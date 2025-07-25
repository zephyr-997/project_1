/**
 * @file uart_port.c
 * @brief WIT传感器STM32F407 UART端口层实现
 * @details 本文件基于STM32 HAL库实现UART端口层功能，支持串口输出和printf重定向
 * @author Augment Agent
 * @date 2025-07-25
 */

#include "wit_port.h"
#include "stm32f407_port_config.h"
#include <stdio.h>

/* ========================================================================== */
/*                              私有宏定义                                    */
/* ========================================================================== */

#define UART_TIMEOUT_MS         WIT_UART_TIMEOUT   /* UART超时时间(毫秒) */
#define UART_TX_BUFFER_SIZE     256                 /* 发送缓冲区大小 */

/* ========================================================================== */
/*                              私有变量                                      */
/* ========================================================================== */

static uint8_t s_uart_initialized = 0;                     /* UART初始化标志 */
static uint8_t s_tx_buffer[UART_TX_BUFFER_SIZE];           /* 发送缓冲区 */

/* ========================================================================== */
/*                              私有函数声明                                  */
/* ========================================================================== */

static HAL_StatusTypeDef uart_wait_tx_complete(void);
static HAL_StatusTypeDef uart_transmit_data(uint8_t *data, uint16_t size);

/* ========================================================================== */
/*                              公共函数实现                                  */
/* ========================================================================== */

/**
 * @brief UART端口层初始化
 * @param uiBaud 波特率
 * @return 0: 成功, 其他: 失败
 * @note UART硬件初始化由CubeMX生成的代码完成，此函数仅做状态检查和波特率配置
 */
int32_t wit_port_uart_init(uint32_t uiBaud)
{
    /* 参数检查 */
    if (uiBaud == 0) {
        return -1;  /* 无效波特率 */
    }

    if (s_uart_initialized) {
        return 0;  /* 已经初始化 */
    }

    /* 检查UART句柄是否已初始化 */
    if (huart1.Instance == NULL) {
        return -1;  /* UART未初始化，请检查CubeMX配置 */
    }

    /* 如果需要自定义波特率，重新配置 */
    if (uiBaud != huart1.Init.BaudRate) {
        huart1.Init.BaudRate = uiBaud;
        if (HAL_UART_Init(&huart1) != HAL_OK) {
            return -1;
        }
    }

    s_uart_initialized = 1;
    return 0;
}

/**
 * @brief UART发送数据
 * @param p_ucData 要发送的数据指针
 * @param uiLen 数据长度
 */
void wit_port_uart_write(uint8_t *p_ucData, uint32_t uiLen)
{
    /* 参数检查 */
    if (p_ucData == NULL || uiLen == 0) {
        return;
    }

    /* 检查初始化状态 */
    if (!s_uart_initialized) {
        if (wit_port_uart_init(WIT_UART_BAUDRATE) != 0) {
            return;
        }
    }

    /* 发送数据 */
    uart_transmit_data(p_ucData, (uint16_t)uiLen);
}

/* ========================================================================== */
/*                              私有函数实现                                  */
/* ========================================================================== */

/**
 * @brief 等待UART发送完成
 * @return HAL_OK: 成功, 其他: 失败
 */
static HAL_StatusTypeDef uart_wait_tx_complete(void)
{
    uint32_t timeout = UART_TIMEOUT_MS;

    /* 等待UART发送完成 */
    while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == RESET && timeout > 0) {
        HAL_Delay(1);
        timeout--;
    }

    return (timeout > 0) ? HAL_OK : HAL_TIMEOUT;
}

/**
 * @brief UART数据发送
 * @param data 数据指针
 * @param size 数据大小
 * @return HAL_OK: 成功, 其他: 失败
 */
static HAL_StatusTypeDef uart_transmit_data(uint8_t *data, uint16_t size)
{
    HAL_StatusTypeDef status;

    /* 使用HAL库发送数据 */
    status = HAL_UART_Transmit(&huart1, data, size, UART_TIMEOUT_MS);

    return status;
}

/* ========================================================================== */
/*                              重定向支持                                    */
/* ========================================================================== */

#ifdef __GNUC__
/**
 * @brief GCC编译器printf重定向
 * @param file 文件描述符
 * @param ptr 数据指针
 * @param len 数据长度
 * @return 发送的字节数
 */
int _write(int file, char *ptr, int len)
{
    wit_port_uart_write((uint8_t*)ptr, len);
    return len;
}
#endif

#ifdef __CC_ARM
/**
 * @brief Keil MDK编译器printf重定向
 * @param ch 字符
 * @param f 文件指针
 * @return 发送的字符
 */
int fputc(int ch, FILE *f)
{
    uint8_t data = (uint8_t)ch;
    wit_port_uart_write(&data, 1);
    return ch;
}
#endif

#ifdef __ICCARM__
/**
 * @brief IAR编译器printf重定向
 * @param ch 字符
 * @return 发送的字符
 */
int putchar(int ch)
{
    uint8_t data = (uint8_t)ch;
    wit_port_uart_write(&data, 1);
    return ch;
}
#endif
