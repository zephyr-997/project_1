/**
 * @file example_usage.c
 * @brief STM32F407 WIT传感器端口层使用示例
 * @details 本文件展示如何在STM32F407平台上使用端口层接口与WIT传感器进行通信
 * @author Augment Agent
 * @date 2025-07-25
 */

#include "wit_port.h"
#include "stm32f407_port_config.h"
#include <stdio.h>

/* ========================================================================== */
/*                              示例：端口层初始化                            */
/* ========================================================================== */

/**
 * @brief STM32F407端口层完整初始化
 * @return 0: 成功, 其他: 失败
 * @note 硬件初始化由CubeMX生成的代码完成，此函数仅初始化端口层
 */
int32_t stm32f407_wit_port_init_all(void)
{
    int32_t ret;

    /* 初始化延时功能 */
    ret = wit_port_delay_init();
    if (ret != 0) {
        printf("Error: Delay port initialization failed\r\n");
        return ret;
    }

    /* 初始化UART端口层 */
    ret = wit_port_uart_init(115200);
    if (ret != 0) {
        printf("Error: UART port initialization failed\r\n");
        return ret;
    }

    /* 初始化I2C端口层 */
    ret = wit_port_i2c_init();
    if (ret != 0) {
        printf("Error: I2C port initialization failed\r\n");
        return ret;
    }

    printf("STM32F407 WIT sensor port layer initialized successfully\r\n");
    printf("Note: Hardware initialization should be done by CubeMX generated code\r\n");
    printf("System Clock: %lu Hz\r\n", SYSTEM_CLOCK_FREQ);

    return 0;
}

/**
 * @brief 兼容的端口层初始化函数
 * @return 0: 成功, 其他: 失败
 */
int32_t wit_port_init_all(void)
{
    return stm32f407_wit_port_init_all();
}

/* ========================================================================== */
/*                              示例：I2C通信                                */
/* ========================================================================== */

/**
 * @brief 读取WIT传感器寄存器示例
 * @param sensor_addr 传感器I2C地址
 * @param reg_addr 寄存器地址
 * @param data 读取的数据缓冲区
 * @param len 读取长度
 * @return 1: 成功, 0: 失败
 */
int32_t wit_read_register_example(uint8_t sensor_addr, uint8_t reg_addr, uint8_t *data, uint32_t len)
{
    /* 使用端口层I2C接口读取寄存器 */
    return wit_port_i2c_read(sensor_addr, reg_addr, data, len);
}

/**
 * @brief 写入WIT传感器寄存器示例
 * @param sensor_addr 传感器I2C地址
 * @param reg_addr 寄存器地址
 * @param data 要写入的数据
 * @param len 写入长度
 * @return 1: 成功, 0: 失败
 */
int32_t wit_write_register_example(uint8_t sensor_addr, uint8_t reg_addr, uint8_t *data, uint32_t len)
{
    /* 使用端口层I2C接口写入寄存器 */
    return wit_port_i2c_write(sensor_addr, reg_addr, data, len);
}

/* ========================================================================== */
/*                              示例：UART输出                               */
/* ========================================================================== */

/**
 * @brief 输出调试信息示例
 * @param message 要输出的消息
 */
void wit_debug_print_example(const char *message)
{
    /* 使用端口层UART接口输出调试信息 */
    wit_port_uart_write((uint8_t*)message, strlen(message));
}

/**
 * @brief 输出传感器数据示例
 * @param acc_x X轴加速度
 * @param acc_y Y轴加速度
 * @param acc_z Z轴加速度
 */
void wit_print_sensor_data_example(int16_t acc_x, int16_t acc_y, int16_t acc_z)
{
    char buffer[100];
    int len;
    
    /* 格式化传感器数据 */
    len = sprintf(buffer, "ACC: X=%d, Y=%d, Z=%d\r\n", acc_x, acc_y, acc_z);
    
    /* 输出到UART */
    wit_port_uart_write((uint8_t*)buffer, len);
}

/* ========================================================================== */
/*                              示例：延时使用                                */
/* ========================================================================== */

/**
 * @brief 传感器初始化序列示例
 * @param sensor_addr 传感器I2C地址
 * @return 1: 成功, 0: 失败
 */
int32_t wit_sensor_init_sequence_example(uint8_t sensor_addr)
{
    uint8_t init_data[] = {0x01, 0x02, 0x03};
    
    /* 发送初始化命令 */
    if (!wit_port_i2c_write(sensor_addr, 0x3E, init_data, sizeof(init_data))) {
        return 0;
    }
    
    /* 等待传感器初始化完成 */
    wit_port_delay_ms(100);  /* 延时100ms */
    
    /* 验证初始化结果 */
    uint8_t status;
    if (!wit_port_i2c_read(sensor_addr, 0x3F, &status, 1)) {
        return 0;
    }
    
    return (status == 0x55) ? 1 : 0;  /* 检查状态寄存器 */
}

/* ========================================================================== */
/*                              STM32F407特定示例                            */
/* ========================================================================== */

/**
 * @brief STM32F407性能测试示例
 */
void stm32f407_performance_test(void)
{
    uint32_t start_time, end_time;

    printf("\r\n=== STM32F407 Performance Test ===\r\n");

    /* 测试延时精度 */
    printf("Testing delay accuracy...\r\n");
    start_time = HAL_GetTick();
    wit_port_delay_ms(1000);  /* 延时1秒 */
    end_time = HAL_GetTick();
    printf("1000ms delay actual: %lu ms\r\n", end_time - start_time);

    /* 测试微秒延时 */
    printf("Testing microsecond delay...\r\n");
    start_time = DWT->CYCCNT;
    wit_port_delay_us(100);  /* 延时100微秒 */
    end_time = DWT->CYCCNT;
    printf("100us delay cycles: %lu (expected: %lu)\r\n",
           end_time - start_time, 100 * (SYSTEM_CLOCK_FREQ / 1000000UL));
}

/**
 * @brief STM32F407 I2C总线扫描示例
 */
void stm32f407_i2c_scan(void)
{
    uint8_t addr;
    uint8_t dummy_data;
    int32_t found_count = 0;

    printf("\r\n=== I2C Bus Scan ===\r\n");
    printf("Scanning I2C bus (0x08-0x77)...\r\n");

    for (addr = 0x08; addr <= 0x77; addr++) {
        /* 尝试读取设备 */
        if (wit_port_i2c_read(addr, 0x00, &dummy_data, 1)) {
            printf("Found device at address 0x%02X\r\n", addr);
            found_count++;
        }
        wit_port_delay_ms(1);  /* 短暂延时 */
    }

    printf("Scan complete. Found %d device(s)\r\n", found_count);
}

/**
 * @brief STM32F407系统信息显示
 */
void stm32f407_system_info(void)
{
    printf("\r\n=== STM32F407 System Information ===\r\n");
    printf("CPU: STM32F407VGT6\r\n");
    printf("Core: ARM Cortex-M4F\r\n");
    printf("System Clock: %lu Hz\r\n", SYSTEM_CLOCK_FREQ);
    printf("HAL Version: %lu\r\n", HAL_GetHalVersion());
    printf("Tick Frequency: %lu Hz\r\n", HAL_GetTickFreq());

#if WIT_USE_DWT_DELAY
    printf("DWT Delay: Enabled\r\n");
#else
    printf("DWT Delay: Disabled\r\n");
#endif

    printf("I2C: %s, %lu Hz\r\n", "Hardware", (uint32_t)WIT_I2C_CLOCK_SPEED);
    printf("UART: %s, %lu bps\r\n", "Hardware", (uint32_t)WIT_UART_BAUDRATE);
}

/**
 * @brief I2C时序控制示例
 */
void wit_i2c_timing_example(void)
{
    /* I2C操作之间的短延时 */
    wit_port_delay_us(10);  /* 延时10微秒 */
    
    /* 传感器复位后的等待时间 */
    wit_port_delay_ms(50);  /* 延时50毫秒 */
}

/* ========================================================================== */
/*                              示例：完整的传感器读取                        */
/* ========================================================================== */

/**
 * @brief 完整的传感器数据读取示例
 * @param sensor_addr 传感器I2C地址
 * @return 1: 成功, 0: 失败
 */
int32_t wit_read_sensor_complete_example(uint8_t sensor_addr)
{
    uint8_t data[6];  /* 6字节数据：X、Y、Z轴各2字节 */
    int16_t acc_x, acc_y, acc_z;
    
    /* 读取加速度数据寄存器 */
    if (!wit_port_i2c_read(sensor_addr, 0x34, data, 6)) {
        wit_debug_print_example("Error: Failed to read sensor data\r\n");
        return 0;
    }
    
    /* 解析数据 */
    acc_x = (int16_t)((data[1] << 8) | data[0]);
    acc_y = (int16_t)((data[3] << 8) | data[2]);
    acc_z = (int16_t)((data[5] << 8) | data[4]);
    
    /* 输出数据 */
    wit_print_sensor_data_example(acc_x, acc_y, acc_z);
    
    return 1;
}

/* ========================================================================== */
/*                              示例：错误处理                                */
/* ========================================================================== */

/**
 * @brief 带错误处理的传感器操作示例
 * @param sensor_addr 传感器I2C地址
 * @return 1: 成功, 0: 失败
 */
int32_t wit_sensor_operation_with_error_handling_example(uint8_t sensor_addr)
{
    int32_t retry_count = 3;
    uint8_t device_id;
    
    /* 重试机制 */
    while (retry_count > 0) {
        /* 读取设备ID */
        if (wit_port_i2c_read(sensor_addr, 0x00, &device_id, 1)) {
            if (device_id == 0x50) {  /* 期望的设备ID */
                wit_debug_print_example("Sensor detected successfully\r\n");
                return 1;
            } else {
                wit_debug_print_example("Warning: Unexpected device ID\r\n");
            }
        } else {
            wit_debug_print_example("Error: I2C communication failed\r\n");
        }
        
        /* 重试前延时 */
        wit_port_delay_ms(10);
        retry_count--;
    }
    
    wit_debug_print_example("Error: Sensor detection failed after retries\r\n");
    return 0;
}
