#ifndef WIT_PORT_H__
#define WIT_PORT_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file wit_port.h
 * @brief WIT传感器端口层接口定义
 * @details 本文件定义了WIT传感器驱动所需的所有端口层接口，包括I2C、UART和延时功能。
 *          各MCU平台需要实现这些接口以适配具体的硬件平台。
 * @author Augment Agent
 * @date 2025-07-25
 */

/* ========================================================================== */
/*                              I2C 端口层接口                                */
/* ========================================================================== */

/**
 * @brief I2C端口层初始化
 * @return 0: 成功, 其他: 失败
 */
int32_t wit_port_i2c_init(void);

/**
 * @brief I2C写寄存器
 * @param ucAddr 设备地址 (7位地址，不包含读写位)
 * @param ucReg 寄存器地址
 * @param p_ucVal 要写入的数据指针
 * @param uiLen 数据长度
 * @return 1: 成功, 0: 失败
 * @note 此函数需要实现完整的I2C写时序：START + ADDR + REG + DATA + STOP
 */
int32_t wit_port_i2c_write(uint8_t ucAddr, uint8_t ucReg, uint8_t *p_ucVal, uint32_t uiLen);

/**
 * @brief I2C读寄存器
 * @param ucAddr 设备地址 (7位地址，不包含读写位)
 * @param ucReg 寄存器地址
 * @param p_ucVal 读取数据存储指针
 * @param uiLen 要读取的数据长度
 * @return 1: 成功, 0: 失败
 * @note 此函数需要实现完整的I2C读时序：START + ADDR + REG + RESTART + ADDR+1 + DATA + STOP
 */
int32_t wit_port_i2c_read(uint8_t ucAddr, uint8_t ucReg, uint8_t *p_ucVal, uint32_t uiLen);

/* ========================================================================== */
/*                             UART 端口层接口                               */
/* ========================================================================== */

/**
 * @brief UART端口层初始化
 * @param uiBaud 波特率
 * @return 0: 成功, 其他: 失败
 */
int32_t wit_port_uart_init(uint32_t uiBaud);

/**
 * @brief UART发送数据
 * @param p_ucData 要发送的数据指针
 * @param uiLen 数据长度
 * @note 此函数用于串口数据输出，通常用于调试信息打印
 */
void wit_port_uart_write(uint8_t *p_ucData, uint32_t uiLen);

/* ========================================================================== */
/*                             延时端口层接口                                 */
/* ========================================================================== */

/**
 * @brief 延时端口层初始化
 * @return 0: 成功, 其他: 失败
 */
int32_t wit_port_delay_init(void);

/**
 * @brief 毫秒级延时
 * @param ucMs 延时时间(毫秒)
 * @note 此函数需要提供精确的毫秒级延时，用于I2C时序控制等
 */
void wit_port_delay_ms(uint16_t ucMs);

/**
 * @brief 微秒级延时
 * @param ucUs 延时时间(微秒)
 * @note 此函数用于I2C位时序控制，需要较高精度
 */
void wit_port_delay_us(uint16_t ucUs);

#ifdef __cplusplus
}
#endif

#endif /* WIT_PORT_H__ */
