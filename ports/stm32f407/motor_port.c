/**
 * @file motor_port.c
 * @brief STM32F407平台电机驱动端口层实现文件
 * @details 本文件实现了TB6612FNG电机驱动在STM32F407平台上的端口层接口。
 *          包括GPIO控制、PWM输出等硬件相关功能的具体实现。
 * @author Augment Agent
 * @date 2025-07-25
 * @version 1.0.0
 */

#include "motor_port.h"
#include "tim.h"
#include "gpio.h"
#include <string.h>

/* ========================================================================== */
/*                              私有变量                                      */
/* ========================================================================== */

/**
 * @brief PWM端口层状态结构体
 */
typedef struct {
    bool initialized;                   /**< 初始化状态 */
    uint32_t frequency;                 /**< PWM频率 */
    uint16_t period;                    /**< PWM周期值 */
    uint16_t prescaler;                 /**< 预分频值 */
} pwm_port_state_t;

static pwm_port_state_t g_pwm_state = {0};  /**< PWM端口层状态 */

/* ========================================================================== */
/*                              私有函数声明                                  */
/* ========================================================================== */

static int32_t calculate_pwm_parameters(uint32_t frequency, uint16_t *prescaler, uint16_t *period);
static tb6612_error_t configure_motor_gpio(void);

/* ========================================================================== */
/*                              端口层接口实现                                */
/* ========================================================================== */

/**
 * @brief 初始化电机端口层
 */
tb6612_error_t motor_port_init(const tb6612_config_t *config)
{
    tb6612_error_t ret = TB6612_OK;
    
    /* 参数检查 */
    if (config == NULL) {
        return TB6612_ERROR_INVALID_PARAM;
    }
    
    /* 初始化GPIO */
    ret = configure_motor_gpio();
    if (ret != TB6612_OK) {
        return ret;
    }
    
    /* 初始化PWM */
    if (pwm_port_init(config->pwm_frequency) != 0) {
        return TB6612_ERROR_HARDWARE_FAULT;
    }
    
    /* 停止所有PWM输出，确保电机初始状态为停止 */
    pwm_port_stop(1);  /* 电机A */
    pwm_port_stop(2);  /* 电机B */
    
    /* 设置所有方向控制引脚为低电平（停止状态） */
    gpio_port_set_pin(TB6612_AIN1_PORT, TB6612_AIN1_PIN, 0);
    gpio_port_set_pin(TB6612_AIN2_PORT, TB6612_AIN2_PIN, 0);
    gpio_port_set_pin(TB6612_BIN1_PORT, TB6612_BIN1_PIN, 0);
    gpio_port_set_pin(TB6612_BIN2_PORT, TB6612_BIN2_PIN, 0);
    
    return TB6612_OK;
}

/**
 * @brief 反初始化电机端口层
 */
tb6612_error_t motor_port_deinit(void)
{
    /* 停止所有PWM输出 */
    pwm_port_stop(1);
    pwm_port_stop(2);
    
    /* 设置所有GPIO为低电平 */
    gpio_port_set_pin(TB6612_AIN1_PORT, TB6612_AIN1_PIN, 0);
    gpio_port_set_pin(TB6612_AIN2_PORT, TB6612_AIN2_PIN, 0);
    gpio_port_set_pin(TB6612_BIN1_PORT, TB6612_BIN1_PIN, 0);
    gpio_port_set_pin(TB6612_BIN2_PORT, TB6612_BIN2_PIN, 0);
    
    /* 清零PWM状态 */
    memset(&g_pwm_state, 0, sizeof(pwm_port_state_t));
    
    return TB6612_OK;
}

/**
 * @brief 设置电机方向
 */
tb6612_error_t motor_port_set_direction(tb6612_motor_t motor, tb6612_direction_t direction)
{
    /* 参数检查 */
    if (motor >= TB6612_MOTOR_MAX) {
        return TB6612_ERROR_INVALID_PARAM;
    }
    
    if (motor == TB6612_MOTOR_A) {
        /* 电机A方向控制 */
        switch (direction) {
            case TB6612_STOP:
                gpio_port_set_pin(TB6612_AIN1_PORT, TB6612_AIN1_PIN, 0);
                gpio_port_set_pin(TB6612_AIN2_PORT, TB6612_AIN2_PIN, 0);
                break;
            case TB6612_FORWARD:
                gpio_port_set_pin(TB6612_AIN1_PORT, TB6612_AIN1_PIN, 1);
                gpio_port_set_pin(TB6612_AIN2_PORT, TB6612_AIN2_PIN, 0);
                break;
            case TB6612_BACKWARD:
                gpio_port_set_pin(TB6612_AIN1_PORT, TB6612_AIN1_PIN, 0);
                gpio_port_set_pin(TB6612_AIN2_PORT, TB6612_AIN2_PIN, 1);
                break;
            case TB6612_BRAKE:
                gpio_port_set_pin(TB6612_AIN1_PORT, TB6612_AIN1_PIN, 1);
                gpio_port_set_pin(TB6612_AIN2_PORT, TB6612_AIN2_PIN, 1);
                break;
            default:
                return TB6612_ERROR_INVALID_PARAM;
        }
    } else if (motor == TB6612_MOTOR_B) {
        /* 电机B方向控制 */
        switch (direction) {
            case TB6612_STOP:
                gpio_port_set_pin(TB6612_BIN1_PORT, TB6612_BIN1_PIN, 0);
                gpio_port_set_pin(TB6612_BIN2_PORT, TB6612_BIN2_PIN, 0);
                break;
            case TB6612_FORWARD:
                gpio_port_set_pin(TB6612_BIN1_PORT, TB6612_BIN1_PIN, 1);
                gpio_port_set_pin(TB6612_BIN2_PORT, TB6612_BIN2_PIN, 0);
                break;
            case TB6612_BACKWARD:
                gpio_port_set_pin(TB6612_BIN1_PORT, TB6612_BIN1_PIN, 0);
                gpio_port_set_pin(TB6612_BIN2_PORT, TB6612_BIN2_PIN, 1);
                break;
            case TB6612_BRAKE:
                gpio_port_set_pin(TB6612_BIN1_PORT, TB6612_BIN1_PIN, 1);
                gpio_port_set_pin(TB6612_BIN2_PORT, TB6612_BIN2_PIN, 1);
                break;
            default:
                return TB6612_ERROR_INVALID_PARAM;
        }
    }
    
    return TB6612_OK;
}

/**
 * @brief 设置电机速度
 */
tb6612_error_t motor_port_set_speed(tb6612_motor_t motor, uint16_t speed_percent)
{
    uint8_t channel;
    
    /* 参数检查 */
    if (motor >= TB6612_MOTOR_MAX || speed_percent > 100) {
        return TB6612_ERROR_INVALID_PARAM;
    }
    
    /* 确定PWM通道 */
    if (motor == TB6612_MOTOR_A) {
        channel = 1;  /* TIM1_CH1 */
    } else {
        channel = 2;  /* TIM1_CH2 */
    }
    
    /* 设置PWM占空比 */
    if (pwm_port_set_duty(channel, speed_percent) != 0) {
        return TB6612_ERROR_HARDWARE_FAULT;
    }
    
    /* 启动或停止PWM输出 */
    if (speed_percent > 0) {
        pwm_port_start(channel);
    } else {
        pwm_port_stop(channel);
    }
    
    return TB6612_OK;
}

/* ========================================================================== */
/*                              PWM端口层实现                                */
/* ========================================================================== */

/**
 * @brief 初始化PWM端口层
 */
int32_t pwm_port_init(uint32_t frequency)
{
    uint16_t prescaler, period;
    
    /* 参数检查 */
    if (frequency < 1000 || frequency > 20000) {
        return -1;  /* 频率超出范围 */
    }
    
    /* 计算PWM参数 */
    if (calculate_pwm_parameters(frequency, &prescaler, &period) != 0) {
        return -2;  /* 参数计算失败 */
    }
    
    /* 配置定时器参数 */
    htim1.Init.Prescaler = prescaler - 1;
    htim1.Init.Period = period - 1;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    
    /* 重新初始化定时器 */
    if (HAL_TIM_PWM_Init(&htim1) != HAL_OK) {
        return -3;  /* 定时器初始化失败 */
    }
    
    /* 保存状态 */
    g_pwm_state.initialized = true;
    g_pwm_state.frequency = frequency;
    g_pwm_state.period = period;
    g_pwm_state.prescaler = prescaler;
    
    return 0;
}

/**
 * @brief 设置PWM占空比
 */
int32_t pwm_port_set_duty(uint8_t channel, uint16_t duty_percent)
{
    uint32_t tim_channel;
    uint32_t pulse_value;
    
    /* 参数检查 */
    if (!g_pwm_state.initialized || duty_percent > 100) {
        return -1;
    }
    
    /* 确定定时器通道 */
    if (channel == 1) {
        tim_channel = TB6612_PWMA_CHANNEL;
    } else if (channel == 2) {
        tim_channel = TB6612_PWMB_CHANNEL;
    } else {
        return -2;  /* 无效通道 */
    }
    
    /* 计算脉冲值 */
    pulse_value = (g_pwm_state.period * duty_percent) / 100;
    
    /* 设置PWM占空比 */
    __HAL_TIM_SET_COMPARE(&htim1, tim_channel, pulse_value);
    
    return 0;
}

/**
 * @brief 设置PWM频率
 */
int32_t pwm_port_set_frequency(uint32_t frequency)
{
    /* 重新初始化PWM */
    return pwm_port_init(frequency);
}

/**
 * @brief 启动PWM输出
 */
int32_t pwm_port_start(uint8_t channel)
{
    uint32_t tim_channel;

    /* 参数检查 */
    if (!g_pwm_state.initialized) {
        return -1;
    }

    /* 确定定时器通道 */
    if (channel == 1) {
        tim_channel = TB6612_PWMA_CHANNEL;
    } else if (channel == 2) {
        tim_channel = TB6612_PWMB_CHANNEL;
    } else {
        return -2;  /* 无效通道 */
    }

    /* 启动PWM输出 */
    if (HAL_TIM_PWM_Start(&htim1, tim_channel) != HAL_OK) {
        return -3;  /* 启动失败 */
    }

    return 0;
}

/**
 * @brief 停止PWM输出
 */
int32_t pwm_port_stop(uint8_t channel)
{
    uint32_t tim_channel;

    /* 参数检查 */
    if (!g_pwm_state.initialized) {
        return -1;
    }

    /* 确定定时器通道 */
    if (channel == 1) {
        tim_channel = TB6612_PWMA_CHANNEL;
    } else if (channel == 2) {
        tim_channel = TB6612_PWMB_CHANNEL;
    } else {
        return -2;  /* 无效通道 */
    }

    /* 停止PWM输出 */
    if (HAL_TIM_PWM_Stop(&htim1, tim_channel) != HAL_OK) {
        return -3;  /* 停止失败 */
    }

    return 0;
}

/* ========================================================================== */
/*                              GPIO端口层实现                               */
/* ========================================================================== */

/**
 * @brief 初始化GPIO端口层
 * @note GPIO引脚配置由STM32CubeMX自动生成，此函数仅设置初始状态
 */
int32_t gpio_port_init(void)
{
    /*
     * GPIO引脚配置已由STM32CubeMX在MX_GPIO_Init()中完成
     * 这里只需要设置所有引脚的初始状态为低电平
     */

    /* 初始化所有引脚为低电平（停止状态） */
    HAL_GPIO_WritePin(TB6612_AIN1_PORT, TB6612_AIN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(TB6612_AIN2_PORT, TB6612_AIN2_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(TB6612_BIN1_PORT, TB6612_BIN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(TB6612_BIN2_PORT, TB6612_BIN2_PIN, GPIO_PIN_RESET);

    return 0;
}

/**
 * @brief 设置GPIO引脚状态
 */
int32_t gpio_port_set_pin(GPIO_TypeDef* port, uint16_t pin, uint8_t state)
{
    /* 参数检查 */
    if (port == NULL) {
        return -1;
    }

    /* 设置引脚状态 */
    if (state) {
        HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
    }

    return 0;
}

/* ========================================================================== */
/*                              私有函数实现                                  */
/* ========================================================================== */

/**
 * @brief 计算PWM参数
 * @param frequency 目标频率 (Hz)
 * @param prescaler 预分频值指针
 * @param period 周期值指针
 * @return int32_t 错误码
 * @retval 0 计算成功
 * @retval <0 计算失败
 */
static int32_t calculate_pwm_parameters(uint32_t frequency, uint16_t *prescaler, uint16_t *period)
{
    uint32_t timer_clock;
    uint32_t temp_prescaler, temp_period;

    /* 参数检查 */
    if (prescaler == NULL || period == NULL) {
        return -1;
    }

    /* TIM1时钟频率 = APB2时钟频率 * 2 = 84MHz * 2 = 168MHz */
    timer_clock = SYSTEM_CLOCK_FREQ;  /* 168MHz */

    /* 计算最佳预分频和周期值 */
    for (temp_prescaler = 1; temp_prescaler <= 65536; temp_prescaler++) {
        temp_period = timer_clock / (temp_prescaler * frequency);

        /* 检查周期值是否在有效范围内 */
        if (temp_period > 0 && temp_period <= 65536) {
            *prescaler = (uint16_t)temp_prescaler;
            *period = (uint16_t)temp_period;
            return 0;
        }
    }

    return -2;  /* 无法找到合适的参数 */
}

/**
 * @brief 配置电机GPIO
 * @return tb6612_error_t 错误码
 * @note GPIO引脚配置由STM32CubeMX完成，需要在CubeMX中配置以下引脚：
 *       - PC4: GPIO_Output (TB6612_AIN1)
 *       - PC5: GPIO_Output (TB6612_AIN2)
 *       - PB0: GPIO_Output (TB6612_BIN1)
 *       - PB1: GPIO_Output (TB6612_BIN2)
 */
static tb6612_error_t configure_motor_gpio(void)
{
    /* 设置GPIO初始状态（配置由CubeMX完成） */
    if (gpio_port_init() != 0) {
        return TB6612_ERROR_HARDWARE_FAULT;
    }

    return TB6612_OK;
}
