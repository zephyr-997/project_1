/**
 * @file jy61p_app.c
 * @brief JY61P陀螺仪传感器应用层实现
 * @details 本文件实现了基于JY61P陀螺仪传感器的应用层逻辑，遵循平台无关设计原则。
 *          仅依赖drivers层API，不包含任何MCU特定代码。适用于Keil5等开发环境。
 * @author Augment Agent
 * @date 2025-07-25
 * @note 根据api-rules.md规范重构，实现软件层级分离，便于移植
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "wit_c_sdk.h"
#include "jy61p_app.h"

/* JY61P端口层接口声明 - 由具体端口层实现 */
extern int32_t wit_port_i2c_init(void);
extern int32_t wit_port_i2c_write(uint8_t ucAddr, uint8_t ucReg, uint8_t *p_ucVal, uint32_t uiLen);
extern int32_t wit_port_i2c_read(uint8_t ucAddr, uint8_t ucReg, uint8_t *p_ucVal, uint32_t uiLen);
extern int32_t wit_port_uart_init(uint32_t uiBaud);
extern void wit_port_uart_write(uint8_t *p_ucData, uint32_t uiLen);
extern int32_t wit_port_delay_init(void);
extern void wit_port_delay_ms(uint16_t ucMs);
extern void wit_port_delay_us(uint16_t ucUs);

/* ========================================================================== */
/*                              应用层数据结构                                */
/* ========================================================================== */



/**
 * @brief JY61P应用状态结构
 */
typedef struct {
    volatile uint8_t data_update_flags;  /**< 数据更新标志 */
    volatile uint8_t cmd_received;       /**< 接收到的命令 */
    jy61p_data_t sensor_data;            /**< JY61P传感器数据 */
    uint8_t sensor_found;                /**< 传感器是否找到 */
    uint8_t sensor_addr;                 /**< 传感器I2C地址 */
} jy61p_app_context_t;

/* ========================================================================== */
/*                              数据更新标志定义                              */
/* ========================================================================== */

#define ACC_UPDATE      0x01    /**< 加速度数据更新标志 */
#define GYRO_UPDATE     0x02    /**< 陀螺仪数据更新标志 */
#define ANGLE_UPDATE    0x04    /**< 角度数据更新标志 */
#define MAG_UPDATE      0x08    /**< 磁场数据更新标志 */
#define READ_UPDATE     0x80    /**< 读取操作更新标志 */

/* ========================================================================== */
/*                              全局变量                                      */
/* ========================================================================== */

static jy61p_app_context_t g_app_ctx = {0};  /**< JY61P应用上下文 */

/* ========================================================================== */
/*                              函数声明                                      */
/* ========================================================================== */

static int32_t jy61p_app_init(void);
static int32_t jy61p_sensor_scan(void);
static void jy61p_sensor_data_process(uint32_t uiReg, uint32_t uiRegNum);
static void jy61p_delay_ms(uint16_t ucMs);
static void jy61p_cmd_process(void);
static void jy61p_show_help(void);
static void jy61p_data_convert_and_print(void);

/* ========================================================================== */
/*                              主函数                                        */
/* ========================================================================== */

/**
 * @brief JY61P应用程序主函数
 * @return 程序退出码
 */
int jy61p_app_main(void)
{
    printf("\r\n");
    printf("************************ JY61P Gyroscope Application ***********************\r\n");
    printf("*                     Platform Independent Version                        *\r\n");
    printf("*                        Based on api-rules.md                           *\r\n");
    printf("**************************************************************************\r\n");
    
    // 初始化应用
    if (jy61p_app_init() != 0) {
        printf("ERROR: JY61P application initialization failed!\r\n");
        return -1;
    }
    
    // 扫描并连接传感器
    if (jy61p_sensor_scan() != 0) {
        printf("ERROR: No JY61P found! Please check connections.\r\n");
        return -1;
    }
    
    printf("JY61P initialized successfully at address 0x%02X\r\n", g_app_ctx.sensor_addr);
    jy61p_show_help();
    
    // 主循环
    while (1) {
        // 读取传感器数据 (从AX开始读取12个寄存器)
        WitReadReg(AX, 12);
        
        // 延时500ms
        wit_port_delay_ms(500);
        
        // 处理用户命令
        jy61p_cmd_process();
        
        // 处理传感器数据
        jy61p_data_convert_and_print();
    }
    
    return 0;
}

/* ========================================================================== */
/*                              应用初始化                                    */
/* ========================================================================== */

/**
 * @brief JY61P应用初始化
 * @return 0: 成功, 其他: 失败
 */
static int32_t jy61p_app_init(void)
{
    // 初始化端口层
    if (wit_port_delay_init() != 0) {
        printf("ERROR: Delay port initialization failed!\r\n");
        return -1;
    }
    
    if (wit_port_uart_init(115200) != 0) {
        printf("ERROR: UART port initialization failed!\r\n");
        return -1;
    }
    
    if (wit_port_i2c_init() != 0) {
        printf("ERROR: I2C port initialization failed!\r\n");
        return -1;
    }
    
    // 初始化JY61P SDK
    WitInit(WIT_PROTOCOL_I2C, 0x50);  // JY61P默认地址0x50
    WitI2cFuncRegister(wit_port_i2c_write, wit_port_i2c_read);
    WitRegisterCallBack(jy61p_sensor_data_process);
    WitDelayMsRegister(jy61p_delay_ms);
    
    // 初始化应用上下文
    memset(&g_app_ctx, 0, sizeof(g_app_ctx));
    g_app_ctx.cmd_received = 0xFF;  // 无效命令
    
    printf("JY61P application initialized successfully.\r\n");
    return 0;
}

/* ========================================================================== */
/*                              传感器扫描                                    */
/* ========================================================================== */

/**
 * @brief 自动扫描I2C总线上的JY61P传感器
 * @return 0: 找到传感器, -1: 未找到传感器
 */
static int32_t jy61p_sensor_scan(void)
{
    printf("Scanning I2C bus for JY61P sensors...\r\n");
    
    // 遍历I2C地址从0到0x7E
    for (uint8_t addr = 0; addr < 0x7F; addr++) {
        // 使用当前地址初始化SDK
        WitInit(WIT_PROTOCOL_I2C, addr);
        
        // 重试2次
        for (int retry = 0; retry < 2; retry++) {
            g_app_ctx.data_update_flags = 0;
            
            // 尝试读取3个加速度寄存器
            WitReadReg(AX, 3);
            wit_port_delay_ms(10);  // 等待传感器响应
            
            // 如果数据更新标志被置位，说明找到了传感器
            if (g_app_ctx.data_update_flags != 0) {
                g_app_ctx.sensor_found = 1;
                g_app_ctx.sensor_addr = addr;
                printf("Found JY61P at I2C address: 0x%02X\r\n", addr);
                return 0;
            }
        }
    }
    
    printf("No JY61P found on I2C bus.\r\n");
    return -1;
}

/* ========================================================================== */
/*                              传感器数据处理                                */
/* ========================================================================== */

/**
 * @brief JY61P传感器数据处理回调函数
 * @param uiReg 更新的起始寄存器地址
 * @param uiRegNum 更新的寄存器数量
 * @note 此函数由WIT SDK在数据准备好后自动调用
 */
static void jy61p_sensor_data_process(uint32_t uiReg, uint32_t uiRegNum)
{
    for (uint32_t i = 0; i < uiRegNum; i++) {
        switch (uiReg) {
            case AZ:  // Z轴加速度更新时，认为整组加速度数据都已更新
                g_app_ctx.data_update_flags |= ACC_UPDATE;
                break;
            case GZ:  // Z轴角速度更新时，认为整组角速度数据都已更新
                g_app_ctx.data_update_flags |= GYRO_UPDATE;
                break;
            case HZ:  // Z轴磁场更新时，认为整组磁场数据都已更新
                g_app_ctx.data_update_flags |= MAG_UPDATE;
                break;
            case Yaw: // 偏航角更新时，认为整组角度数据都已更新
                g_app_ctx.data_update_flags |= ANGLE_UPDATE;
                break;
            default:
                g_app_ctx.data_update_flags |= READ_UPDATE;
                break;
        }
        uiReg++;
    }
}

/**
 * @brief 延时函数封装
 * @param ucMs 延时毫秒数
 */
static void jy61p_delay_ms(uint16_t ucMs)
{
    wit_port_delay_ms(ucMs);
}

/* ========================================================================== */
/*                              数据转换和打印                                */
/* ========================================================================== */

/**
 * @brief JY61P数据转换和打印
 */
static void jy61p_data_convert_and_print(void)
{
    if (g_app_ctx.data_update_flags == 0) {
        return;  // 无数据更新
    }

    // 转换原始数据为物理量
    for (int i = 0; i < 3; i++) {
        // 加速度(g) = (原始值 / 32768) * 16
        g_app_ctx.sensor_data.acc[i] = sReg[AX + i] / 32768.0f * 16.0f;
        // 角速度(°/s) = (原始值 / 32768) * 2000
        g_app_ctx.sensor_data.gyro[i] = sReg[GX + i] / 32768.0f * 2000.0f;
        // 角度(°) = (原始值 / 32768) * 180
        g_app_ctx.sensor_data.angle[i] = sReg[Roll + i] / 32768.0f * 180.0f;
        // 磁场数据直接使用原始值
        g_app_ctx.sensor_data.mag[i] = sReg[HX + i];
    }

    // 温度数据
    g_app_ctx.sensor_data.temp = sReg[TEMP];

    // 根据更新标志打印相应数据
    if (g_app_ctx.data_update_flags & ACC_UPDATE) {
        printf("ACC : %.3f %.3f %.3f (g)\r\n",
               g_app_ctx.sensor_data.acc[0],
               g_app_ctx.sensor_data.acc[1],
               g_app_ctx.sensor_data.acc[2]);
        g_app_ctx.data_update_flags &= ~ACC_UPDATE;
    }

    if (g_app_ctx.data_update_flags & GYRO_UPDATE) {
        printf("GYRO: %.3f %.3f %.3f (°/s)\r\n",
               g_app_ctx.sensor_data.gyro[0],
               g_app_ctx.sensor_data.gyro[1],
               g_app_ctx.sensor_data.gyro[2]);
        g_app_ctx.data_update_flags &= ~GYRO_UPDATE;
    }

    if (g_app_ctx.data_update_flags & ANGLE_UPDATE) {
        printf("ANGLE: %.3f %.3f %.3f (°)\r\n",
               g_app_ctx.sensor_data.angle[0],
               g_app_ctx.sensor_data.angle[1],
               g_app_ctx.sensor_data.angle[2]);
        g_app_ctx.data_update_flags &= ~ANGLE_UPDATE;
    }

    if (g_app_ctx.data_update_flags & MAG_UPDATE) {
        printf("MAG : %d %d %d (raw)\r\n",
               g_app_ctx.sensor_data.mag[0],
               g_app_ctx.sensor_data.mag[1],
               g_app_ctx.sensor_data.mag[2]);
        g_app_ctx.data_update_flags &= ~MAG_UPDATE;
    }
}

/* ========================================================================== */
/*                              命令处理                                      */
/* ========================================================================== */

/**
 * @brief 处理JY61P用户命令
 */
static void jy61p_cmd_process(void)
{
    // 这里应该从串口接收命令，暂时使用全局变量模拟
    // 实际实现中需要在串口中断中调用 jy61p_cmd_data_received()

    switch (g_app_ctx.cmd_received) {
        case 'a':  // 加速度计校准
            printf("Starting accelerometer calibration...\r\n");
            if (WitStartAccCali() != WIT_HAL_OK) {
                printf("ERROR: Accelerometer calibration failed!\r\n");
            } else {
                printf("Accelerometer calibration started successfully.\r\n");
            }
            break;

        case 'm':  // 开始磁场校准
            printf("Starting magnetometer calibration...\r\n");
            if (WitStartMagCali() != WIT_HAL_OK) {
                printf("ERROR: Magnetometer calibration start failed!\r\n");
            } else {
                printf("Magnetometer calibration started. Send 'e' to end.\r\n");
            }
            break;

        case 'e':  // 结束磁场校准
            printf("Ending magnetometer calibration...\r\n");
            if (WitStopMagCali() != WIT_HAL_OK) {
                printf("ERROR: Magnetometer calibration end failed!\r\n");
            } else {
                printf("Magnetometer calibration ended successfully.\r\n");
            }
            break;

        case 'u':  // 设置带宽为5Hz
            printf("Setting bandwidth to 5Hz...\r\n");
            if (WitSetBandwidth(BANDWIDTH_5HZ) != WIT_HAL_OK) {
                printf("ERROR: Set bandwidth failed!\r\n");
            } else {
                printf("Bandwidth set to 5Hz successfully.\r\n");
            }
            break;

        case 'U':  // 设置带宽为256Hz
            printf("Setting bandwidth to 256Hz...\r\n");
            if (WitSetBandwidth(BANDWIDTH_256HZ) != WIT_HAL_OK) {
                printf("ERROR: Set bandwidth failed!\r\n");
            } else {
                printf("Bandwidth set to 256Hz successfully.\r\n");
            }
            break;

        case 'B':  // 设置JY61P串口波特率为115200
            printf("Setting JY61P UART baud to 115200...\r\n");
            if (WitSetUartBaud(WIT_BAUD_115200) != WIT_HAL_OK) {
                printf("ERROR: Set baud rate failed!\r\n");
            } else {
                printf("JY61P UART baud rate set to 115200 successfully.\r\n");
            }
            break;

        case 'b':  // 设置JY61P串口波特率为9600
            printf("Setting JY61P UART baud to 9600...\r\n");
            if (WitSetUartBaud(WIT_BAUD_9600) != WIT_HAL_OK) {
                printf("ERROR: Set baud rate failed!\r\n");
            } else {
                printf("JY61P UART baud rate set to 9600 successfully.\r\n");
            }
            break;

        case 'h':  // 显示帮助信息
            jy61p_show_help();
            break;

        case 0xFF:  // 无效命令，不处理
            return;

        default:
            printf("Unknown command: '%c'. Send 'h' for help.\r\n", g_app_ctx.cmd_received);
            break;
    }

    // 命令处理完毕，复位命令变量
    g_app_ctx.cmd_received = 0xFF;
}

/**
 * @brief 显示JY61P帮助信息
 */
static void jy61p_show_help(void)
{
    printf("\r\n");
    printf("************************** JY61P Gyroscope Help ************************\r\n");
    printf("*                        Command Reference                             *\r\n");
    printf("**************************************************************************\r\n");
    printf("Commands (send via UART with \\r\\n):\r\n");
    printf("  a\\r\\n  - Start accelerometer calibration\r\n");
    printf("  m\\r\\n  - Start magnetometer calibration\r\n");
    printf("  e\\r\\n  - End magnetometer calibration\r\n");
    printf("  u\\r\\n  - Set bandwidth to 5Hz\r\n");
    printf("  U\\r\\n  - Set bandwidth to 256Hz\r\n");
    printf("  b\\r\\n  - Set JY61P UART baud to 9600\r\n");
    printf("  B\\r\\n  - Set JY61P UART baud to 115200\r\n");
    printf("  h\\r\\n  - Show this help information\r\n");
    printf("**************************************************************************\r\n");
    printf("Data Format:\r\n");
    printf("  ACC : X Y Z (g)      - Acceleration in gravity units\r\n");
    printf("  GYRO: X Y Z (°/s)    - Angular velocity in degrees per second\r\n");
    printf("  ANGLE: X Y Z (°)     - Euler angles in degrees\r\n");
    printf("  MAG : X Y Z (raw)    - Magnetic field raw values\r\n");
    printf("**************************************************************************\r\n");
    printf("\r\n");
}

/* ========================================================================== */
/*                              串口命令接收                                  */
/* ========================================================================== */

/**
 * @brief 处理串口接收的数据，用于JY61P命令解析
 * @param ucData 接收到的单个字节数据
 * @note 此函数应在串口接收中断中被调用
 */
void jy61p_cmd_data_received(uint8_t ucData)
{
    static uint8_t s_ucData[50];
    static uint8_t s_ucRxCnt = 0;

    s_ucData[s_ucRxCnt++] = ucData;

    // 确保接收到至少3个字节，因为命令格式为 "命令符 + \r\n"
    if (s_ucRxCnt < 3) {
        return;
    }

    // 防止缓冲区溢出
    if (s_ucRxCnt >= 50) {
        s_ucRxCnt = 0;
        return;
    }

    // 检查是否接收到完整的命令结束符 "\r\n"
    if (s_ucRxCnt >= 3) {
        if ((s_ucData[s_ucRxCnt - 2] == '\r') && (s_ucData[s_ucRxCnt - 1] == '\n')) {
            // 将命令符存入全局命令变量
            g_app_ctx.cmd_received = s_ucData[s_ucRxCnt - 3];

            // 清空接收缓冲区
            memset(s_ucData, 0, 50);
            s_ucRxCnt = 0;
        }
    }
}

/* ========================================================================== */
/*                              应用层API接口                                 */
/* ========================================================================== */

/**
 * @brief 获取当前JY61P传感器数据
 * @param data 输出参数，存储传感器数据
 * @return 0: 成功, -1: 失败
 */
int32_t jy61p_get_sensor_data(jy61p_data_t *data)
{
    if (data == NULL) {
        return -1;
    }

    if (!g_app_ctx.sensor_found) {
        return -1;
    }

    memcpy(data, &g_app_ctx.sensor_data, sizeof(jy61p_data_t));
    return 0;
}

/**
 * @brief 检查JY61P传感器是否已连接
 * @return 1: 已连接, 0: 未连接
 */
uint8_t jy61p_is_sensor_connected(void)
{
    return g_app_ctx.sensor_found;
}

/**
 * @brief 获取JY61P传感器I2C地址
 * @return 传感器I2C地址，如果未找到传感器则返回0xFF
 */
uint8_t jy61p_get_sensor_address(void)
{
    return g_app_ctx.sensor_found ? g_app_ctx.sensor_addr : 0xFF;
}
