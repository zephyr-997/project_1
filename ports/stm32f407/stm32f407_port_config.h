/**
 * @file stm32f407_port_config.h
 * @brief STM32F407端口层配置文件
 * @details 本文件定义STM32F407平台端口层所需的配置参数，配合CubeMX生成的初始化代码使用
 * @author Augment Agent
 * @date 2025-07-25
 */

#ifndef STM32F407_PORT_CONFIG_H__
#define STM32F407_PORT_CONFIG_H__

#include "stm32f4xx_hal.h"
#include "main.h"  /* 包含CubeMX生成的定义 */

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                              端口层配置                                    */
/* ========================================================================== */

/*
 * 注意：以下配置需要与CubeMX生成的配置保持一致
 * 请根据您的CubeMX配置修改这些定义
 */

/* 系统时钟频率 (Hz) - 请根据您的时钟配置修改 */
#ifndef SYSTEM_CLOCK_FREQ
#define SYSTEM_CLOCK_FREQ           168000000UL
#endif

/* I2C超时配置 */
#define WIT_I2C_TIMEOUT             1000UL      /* 1秒超时 */

/* UART配置 */
#define WIT_UART_BAUDRATE           115200UL    /* UART波特率 */
#define WIT_UART_TIMEOUT            1000UL      /* 1秒超时 */

/* ========================================================================== */
/*                              延时配置                                      */
/* ========================================================================== */

/* DWT配置 (用于微秒延时) */
#define WIT_USE_DWT_DELAY           1           /* 使能DWT延时 */

/* 微秒延时校准因子 */
#define WIT_DELAY_US_FACTOR         (SYSTEM_CLOCK_FREQ / 1000000UL)

/* ========================================================================== */
/*                              外设句柄声明                                  */
/* ========================================================================== */

/*
 * 注意：这些句柄应该在CubeMX生成的代码中已经定义
 * 请确保您的CubeMX配置中包含了相应的外设
 * 如果句柄名称不同，请修改以下声明
 */

/* I2C句柄 - 请根据您的CubeMX配置修改句柄名称 */
extern I2C_HandleTypeDef hi2c1;

/* UART句柄 - 请根据您的CubeMX配置修改句柄名称 */
extern UART_HandleTypeDef huart1;

#ifdef __cplusplus
}
#endif

#endif /* STM32F407_PORT_CONFIG_H__ */
