/**
 * @file jy61p_app.h
 * @brief JY61P陀螺仪传感器应用层接口定义
 * @details 本文件定义了应用层对外提供的接口，遵循平台无关设计原则。
 *          上层模块可以通过这些接口访问JY61P传感器数据和控制传感器行为。
 *          适用于Keil5等开发环境，便于移植。
 * @author Augment Agent
 * @date 2025-07-25
 * @note 根据api-rules.md规范设计，实现接口最小化原则
 * 
 * @usage 在Keil5项目中的使用方法：
 *        1. 将jy61p_app.c添加到项目源文件
 *        2. 将jy61p_app.h添加到项目头文件
 *        3. 确保端口层实现已正确配置
 *        4. 在串口中断中调用jy61p_cmd_data_received()
 */

#ifndef JY61P_APP_H__
#define JY61P_APP_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                              数据结构定义                                  */
/* ========================================================================== */

/**
 * @brief JY61P传感器数据结构s
 * @details 包含JY61P传感器的所有测量数据，已转换为标准物理单位
 */
typedef struct {
    float acc[3];    /**< 三轴加速度 [X, Y, Z] (g) */
    float gyro[3];   /**< 三轴角速度 [X, Y, Z] (°/s) */
    float angle[3];  /**< 三轴角度 [Roll, Pitch, Yaw] (°) */
    int16_t mag[3];  /**< 三轴磁场 [X, Y, Z] (原始值) */
    int16_t temp;    /**< 温度 (原始值) */
} jy61p_data_t;

/* ========================================================================== */
/*                              应用层API接口                                 */
/* ========================================================================== */

/**
 * @brief 获取当前JY61P传感器数据
 * @param data 输出参数，存储JY61P传感器数据的指针
 * @return 0: 成功获取数据, -1: 失败（传感器未连接或参数无效）
 * @note 此函数返回最新的JY61P传感器数据，数据已转换为标准物理单位
 * 
 * @code
 * jy61p_data_t data;
 * if (jy61p_get_sensor_data(&data) == 0) {
 *     printf("Acceleration: %.3f, %.3f, %.3f g\n", data.acc[0], data.acc[1], data.acc[2]);
 * }
 * @endcode
 */
int32_t jy61p_get_sensor_data(jy61p_data_t *data);

/**
 * @brief 检查JY61P传感器是否已连接
 * @return 1: JY61P传感器已连接并正常工作, 0: 传感器未连接
 * @note 此函数可用于检查JY61P传感器状态，确保数据有效性
 */
uint8_t jy61p_is_sensor_connected(void);

/**
 * @brief 获取JY61P传感器I2C地址
 * @return JY61P传感器的I2C地址 (0x00-0x7F), 如果未找到传感器则返回0xFF
 * @note 此函数返回自动扫描找到的JY61P传感器地址，可用于调试和状态显示
 */
uint8_t jy61p_get_sensor_address(void);

/**
 * @brief 处理串口接收的JY61P命令数据
 * @param ucData 接收到的单个字节数据
 * @note 此函数应在串口接收中断中被调用，用于解析JY61P用户命令
 *       支持的命令格式为：单字符 + \r\n
 * 
 * @code
 * // 在串口中断服务程序中调用
 * void UART_IRQHandler(void) {
 *     if (UART_GetITStatus(UART1, UART_IT_RXNE) != RESET) {
 *         uint8_t data = UART_ReceiveData(UART1);
 *         jy61p_cmd_data_received(data);
 *     }
 * }
 * @endcode
 */
void jy61p_cmd_data_received(uint8_t ucData);

/* ========================================================================== */
/*                              应用程序入口                                  */
/* ========================================================================== */

/**
 * @brief JY61P应用程序主函数
 * @return 程序退出码 (0: 正常退出, 其他: 错误退出)
 * @note 此函数是JY61P应用程序的入口点，包含完整的初始化和主循环逻辑
 *       在嵌入式系统中通常不会返回
 */
int jy61p_app_main(void);

/* ========================================================================== */
/*                              使用说明                                      */
/* ========================================================================== */

/**
 * @page jy61p_usage JY61P应用层使用说明
 * 
 * @section jy61p_overview 概述
 * 本应用层实现了JY61P传感器的完整功能，包括：
 * - 自动传感器扫描和连接
 * - 实时数据读取和转换
 * - 串口命令处理
 * - 传感器校准和配置
 * 
 * @section jy61p_integration 集成方法
 * 1. 确保端口层（ports/）已正确实现并初始化
 * 2. 在主程序中调用 jy61p_app_main() 函数
 * 3. 在串口中断中调用 jy61p_cmd_data_received()
 * 
 * @section jy61p_commands 支持的命令
 * - 'a' + \r\n: 开始加速度计校准
 * - 'm' + \r\n: 开始磁力计校准
 * - 'e' + \r\n: 结束磁力计校准
 * - 'u' + \r\n: 设置带宽为5Hz
 * - 'U' + \r\n: 设置带宽为256Hz
 * - 'b' + \r\n: 设置传感器串口波特率为9600
 * - 'B' + \r\n: 设置传感器串口波特率为115200
 * - 'h' + \r\n: 显示帮助信息
 * 
 * @section jy61p_data_format 数据格式
 * 传感器数据通过串口以以下格式输出：
 * - ACC : X Y Z (g) - 加速度，单位为重力加速度
 * - GYRO: X Y Z (°/s) - 角速度，单位为度每秒
 * - ANGLE: X Y Z (°) - 欧拉角，单位为度
 * - MAG : X Y Z (raw) - 磁场原始值
 */

#ifdef __cplusplus
}
#endif

#endif /* JY61P_APP_H__ */
