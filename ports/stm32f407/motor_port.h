/**
 * @file motor_port.h
 * @brief STM32F407平台电机驱动端口层头文件
 * @details 本文件定义了TB6612FNG电机驱动在STM32F407平台上的端口层接口。
 *          端口层负责将硬件无关的驱动接口适配到具体的STM32F407硬件平台。
 * @author Augment Agent
 * @date 2025-07-25
 * @version 1.0.0
 * 
 * @note 硬件连接说明:
 *       STM32F407ZGT6 → TB6612FNG 引脚连接:
 *       ├── 电机A (Motor A)
 *       │   ├── PE9  → PWMA  (TIM1_CH1, PWM输出)
 *       │   ├── PC4  → AIN1  (GPIO输出, 方向控制)
 *       │   └── PC5  → AIN2  (GPIO输出, 方向控制)
 *       ├── 电机B (Motor B)
 *       │   ├── PE11 → PWMB  (TIM1_CH2, PWM输出)
 *       │   ├── PB0  → BIN1  (GPIO输出, 方向控制)
 *       │   └── PB1  → BIN2  (GPIO输出, 方向控制)
 *       └── 控制逻辑
 *           ├── STBY → 3.3V (常高，使能芯片)
 *           ├── VM   → 电机电源 (6-15V)
 *           └── VCC  → 3.3V (逻辑电源)
 */

#ifndef MOTOR_PORT_H__
#define MOTOR_PORT_H__

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "stm32f407_port_config.h"
#include "../../hardware/motor_drivers/tb6612fng/tb6612fng.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                              端口层接口函数                                */
/* ========================================================================== */

/**
 * @brief 初始化电机端口层
 * @param config TB6612FNG配置参数指针
 * @return tb6612_error_t 错误码
 * @retval TB6612_OK 初始化成功
 * @retval TB6612_ERROR_INVALID_PARAM 配置参数无效
 * @retval TB6612_ERROR_HARDWARE_FAULT 硬件初始化失败
 * 
 * @note 此函数会初始化GPIO控制引脚和PWM定时器
 */
tb6612_error_t motor_port_init(const tb6612_config_t *config);

/**
 * @brief 反初始化电机端口层
 * @return tb6612_error_t 错误码
 * @retval TB6612_OK 反初始化成功
 * 
 * @note 此函数会停止PWM输出，释放GPIO资源
 */
tb6612_error_t motor_port_deinit(void);

/**
 * @brief 设置电机方向
 * @param motor 电机标识
 * @param direction 电机方向
 * @return tb6612_error_t 错误码
 * @retval TB6612_OK 设置成功
 * @retval TB6612_ERROR_INVALID_PARAM 参数无效
 * 
 * @note 根据TB6612FNG真值表控制AIN1/AIN2或BIN1/BIN2引脚
 */
tb6612_error_t motor_port_set_direction(tb6612_motor_t motor, tb6612_direction_t direction);

/**
 * @brief 设置电机速度
 * @param motor 电机标识
 * @param speed_percent 速度百分比 (0-100)
 * @return tb6612_error_t 错误码
 * @retval TB6612_OK 设置成功
 * @retval TB6612_ERROR_INVALID_PARAM 参数无效
 * 
 * @note 通过PWM占空比控制电机速度，0%为停止，100%为最大速度
 */
tb6612_error_t motor_port_set_speed(tb6612_motor_t motor, uint16_t speed_percent);

/* ========================================================================== */
/*                              PWM端口层接口                                */
/* ========================================================================== */

/**
 * @brief 初始化PWM端口层
 * @param frequency PWM频率 (Hz)
 * @return int32_t 错误码
 * @retval 0 初始化成功
 * @retval <0 初始化失败
 */
int32_t pwm_port_init(uint32_t frequency);

/**
 * @brief 设置PWM占空比
 * @param channel PWM通道 (1=电机A, 2=电机B)
 * @param duty_percent 占空比百分比 (0-100)
 * @return int32_t 错误码
 * @retval 0 设置成功
 * @retval <0 设置失败
 */
int32_t pwm_port_set_duty(uint8_t channel, uint16_t duty_percent);

/**
 * @brief 设置PWM频率
 * @param frequency PWM频率 (Hz)
 * @return int32_t 错误码
 * @retval 0 设置成功
 * @retval <0 设置失败
 */
int32_t pwm_port_set_frequency(uint32_t frequency);

/**
 * @brief 启动PWM输出
 * @param channel PWM通道 (1=电机A, 2=电机B)
 * @return int32_t 错误码
 * @retval 0 启动成功
 * @retval <0 启动失败
 */
int32_t pwm_port_start(uint8_t channel);

/**
 * @brief 停止PWM输出
 * @param channel PWM通道 (1=电机A, 2=电机B)
 * @return int32_t 错误码
 * @retval 0 停止成功
 * @retval <0 停止失败
 */
int32_t pwm_port_stop(uint8_t channel);

/* ========================================================================== */
/*                              GPIO端口层接口                               */
/* ========================================================================== */

/**
 * @brief 初始化GPIO端口层
 * @return int32_t 错误码
 * @retval 0 初始化成功
 * @retval <0 初始化失败
 * @note GPIO引脚配置由STM32CubeMX完成，此函数仅设置初始状态
 *       需要在CubeMX中配置PC4,PC5,PB0,PB1为GPIO_Output模式
 */
int32_t gpio_port_init(void);

/**
 * @brief 设置GPIO引脚状态
 * @param port GPIO端口
 * @param pin GPIO引脚
 * @param state 引脚状态 (0=低电平, 1=高电平)
 * @return int32_t 错误码
 * @retval 0 设置成功
 * @retval <0 设置失败
 */
int32_t gpio_port_set_pin(GPIO_TypeDef* port, uint16_t pin, uint8_t state);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_PORT_H__ */
