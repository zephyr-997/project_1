/**
 * @file i2c_port.c
 * @brief WIT传感器STM32F407 I2C端口层实现
 * @details 本文件基于STM32 HAL库实现I2C端口层功能，支持硬件I2C通信
 * @author Augment Agent
 * @date 2025-07-25
 */

#include "wit_port.h"
#include "stm32f407_port_config.h"

/* ========================================================================== */
/*                              私有宏定义                                    */
/* ========================================================================== */

#define I2C_TIMEOUT_MS          WIT_I2C_TIMEOUT    /* I2C超时时间(毫秒) */
#define I2C_RETRY_COUNT         3                  /* I2C重试次数 */
#define I2C_DEVICE_ADDR_SHIFT   1                  /* 设备地址左移1位 */

/* ========================================================================== */
/*                              私有变量                                      */
/* ========================================================================== */

static uint8_t s_i2c_initialized = 0;  /* I2C初始化标志 */

/* ========================================================================== */
/*                              私有函数声明                                  */
/* ========================================================================== */

static HAL_StatusTypeDef i2c_wait_ready(void);
static int32_t i2c_mem_write_with_retry(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t size);
static int32_t i2c_mem_read_with_retry(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t size);

/* ========================================================================== */
/*                              公共函数实现                                  */
/* ========================================================================== */

/**
 * @brief I2C端口层初始化
 * @return 0: 成功, 其他: 失败
 * @note I2C硬件初始化由CubeMX生成的代码完成，此函数仅做状态检查
 */
int32_t wit_port_i2c_init(void)
{
    if (s_i2c_initialized) {
        return 0;  /* 已经初始化 */
    }

    /* 检查I2C句柄是否已初始化 */
    if (hi2c1.Instance == NULL) {
        return -1;  /* I2C未初始化，请检查CubeMX配置 */
    }

    /* 等待I2C就绪 */
    if (i2c_wait_ready() != HAL_OK) {
        return -1;
    }

    s_i2c_initialized = 1;
    return 0;
}

/**
 * @brief I2C写寄存器
 * @param ucAddr 设备地址 (7位地址，不包含读写位)
 * @param ucReg 寄存器地址
 * @param p_ucVal 要写入的数据指针
 * @param uiLen 数据长度
 * @return 1: 成功, 0: 失败
 */
int32_t wit_port_i2c_write(uint8_t ucAddr, uint8_t ucReg, uint8_t *p_ucVal, uint32_t uiLen)
{
    /* 参数检查 */
    if (p_ucVal == NULL || uiLen == 0) {
        return 0;
    }

    /* 检查初始化状态 */
    if (!s_i2c_initialized) {
        if (wit_port_i2c_init() != 0) {
            return 0;
        }
    }

    /* 执行I2C写操作(带重试) */
    return i2c_mem_write_with_retry(ucAddr, ucReg, p_ucVal, (uint16_t)uiLen);
}

/**
 * @brief I2C读寄存器
 * @param ucAddr 设备地址 (7位地址，不包含读写位)
 * @param ucReg 寄存器地址
 * @param p_ucVal 读取数据存储指针
 * @param uiLen 要读取的数据长度
 * @return 1: 成功, 0: 失败
 */
int32_t wit_port_i2c_read(uint8_t ucAddr, uint8_t ucReg, uint8_t *p_ucVal, uint32_t uiLen)
{
    /* 参数检查 */
    if (p_ucVal == NULL || uiLen == 0) {
        return 0;
    }

    /* 检查初始化状态 */
    if (!s_i2c_initialized) {
        if (wit_port_i2c_init() != 0) {
            return 0;
        }
    }

    /* 执行I2C读操作(带重试) */
    return i2c_mem_read_with_retry(ucAddr, ucReg, p_ucVal, (uint16_t)uiLen);
}

/* ========================================================================== */
/*                              私有函数实现                                  */
/* ========================================================================== */

/**
 * @brief 等待I2C总线就绪
 * @return HAL_OK: 成功, 其他: 失败
 */
static HAL_StatusTypeDef i2c_wait_ready(void)
{
    uint32_t timeout = I2C_TIMEOUT_MS;

    /* 等待I2C总线空闲 */
    while (__HAL_I2C_GET_FLAG(&hi2c1, I2C_FLAG_BUSY) && timeout > 0) {
        HAL_Delay(1);
        timeout--;
    }

    return (timeout > 0) ? HAL_OK : HAL_TIMEOUT;
}

/**
 * @brief 带重试的I2C内存写操作
 * @param dev_addr 设备地址
 * @param reg_addr 寄存器地址
 * @param data 数据指针
 * @param size 数据大小
 * @return 1: 成功, 0: 失败
 */
static int32_t i2c_mem_write_with_retry(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t size)
{
    HAL_StatusTypeDef status;
    uint32_t retry_count = I2C_RETRY_COUNT;

    while (retry_count > 0) {
        /* 执行I2C内存写操作 */
        status = HAL_I2C_Mem_Write(&hi2c1,
                                   (uint16_t)(dev_addr << I2C_DEVICE_ADDR_SHIFT),
                                   reg_addr,
                                   I2C_MEMADD_SIZE_8BIT,
                                   data,
                                   size,
                                   I2C_TIMEOUT_MS);

        if (status == HAL_OK) {
            return 1;  /* 成功 */
        }

        /* 重试前短暂延时 */
        HAL_Delay(1);
        retry_count--;
    }

    return 0;  /* 失败 */
}

/**
 * @brief 带重试的I2C内存读操作
 * @param dev_addr 设备地址
 * @param reg_addr 寄存器地址
 * @param data 数据指针
 * @param size 数据大小
 * @return 1: 成功, 0: 失败
 */
static int32_t i2c_mem_read_with_retry(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t size)
{
    HAL_StatusTypeDef status;
    uint32_t retry_count = I2C_RETRY_COUNT;

    while (retry_count > 0) {
        /* 执行I2C内存读操作 */
        status = HAL_I2C_Mem_Read(&hi2c1,
                                  (uint16_t)(dev_addr << I2C_DEVICE_ADDR_SHIFT),
                                  reg_addr,
                                  I2C_MEMADD_SIZE_8BIT,
                                  data,
                                  size,
                                  I2C_TIMEOUT_MS);

        if (status == HAL_OK) {
            return 1;  /* 成功 */
        }

        /* 重试前短暂延时 */
        HAL_Delay(1);
        retry_count--;
    }

    return 0;  /* 失败 */
}
