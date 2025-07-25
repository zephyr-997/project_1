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

/* SysTick频率配置 (Hz) - HAL库默认1kHz */
#ifndef SYSTICK_FREQ
#define SYSTICK_FREQ                1000UL          /* 1kHz, 1ms中断间隔 */
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

/* 微秒延时校准因子 (基于CPU频率计算) */
#define WIT_DELAY_US_FACTOR         (SYSTEM_CLOCK_FREQ / 1000000UL)

/* SysTick相关配置说明:
 * - SYSTICK_FREQ: SysTick定时器频率，HAL库默认1kHz (1ms中断间隔)
 * - 系统时钟168MHz时，SysTick重载值 = 168000000/1000 - 1 = 167999
 * - 这确保了HAL_Delay()函数的1ms精度
 */

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

/* ========================================================================== */
/*                              电机驱动配置                                  */
/* ========================================================================== */

/* TB6612FNG电机驱动配置参数 */
#define TB6612_PWM_FREQUENCY        10000UL     /* 10kHz PWM频率 */
#define TB6612_PWM_RESOLUTION       1000        /* PWM分辨率 (0-1000) */
#define TB6612_MAX_DUTY_CYCLE       95          /* 最大占空比 95% */
#define TB6612_MIN_DUTY_CYCLE       5           /* 最小占空比 5% */
#define TB6612_EMERGENCY_STOP_TIME  10          /* 紧急停止时间 (ms) */

/* TB6612FNG引脚定义 */
#define TB6612_AIN1_PORT            GPIOC       /* PC4 - 电机A方向控制1 */
#define TB6612_AIN1_PIN             GPIO_PIN_4
#define TB6612_AIN2_PORT            GPIOC       /* PC5 - 电机A方向控制2 */
#define TB6612_AIN2_PIN             GPIO_PIN_5
#define TB6612_BIN1_PORT            GPIOB       /* PB0 - 电机B方向控制1 */
#define TB6612_BIN1_PIN             GPIO_PIN_0
#define TB6612_BIN2_PORT            GPIOB       /* PB1 - 电机B方向控制2 */
#define TB6612_BIN2_PIN             GPIO_PIN_1

/* PWM定时器配置 */
#define TB6612_PWM_TIMER            TIM1        /* PWM定时器 */
#define TB6612_PWMA_CHANNEL         TIM_CHANNEL_1  /* 电机A PWM通道 (PE9) */
#define TB6612_PWMB_CHANNEL         TIM_CHANNEL_2  /* 电机B PWM通道 (PE11) */

/* 定时器句柄声明 */
extern TIM_HandleTypeDef htim1;

#ifdef __cplusplus
}
#endif

#endif /* STM32F407_PORT_CONFIG_H__ */
