/**
 * @file tb6612fng.c
 * @brief TB6612FNG双路电机驱动芯片驱动库实现
 * @details 本文件实现了TB6612FNG电机驱动芯片的完整驱动功能，包括初始化、
 *          方向控制、速度控制、状态查询等功能。采用平台无关设计，通过
 *          端口层接口与具体硬件平台交互。
 * @author Augment Agent
 * @date 2025-07-25
 * @version 1.0.0
 */

#include "tb6612fng.h"
#include <string.h>
#include <stdio.h>

/* ========================================================================== */
/*                              私有宏定义                                    */
/* ========================================================================== */

#define TB6612_MAX_SPEED_PERCENT 100        /**< 最大速度百分比 */
#define TB6612_MIN_SPEED_PERCENT 0          /**< 最小速度百分比 */

/* 默认配置参数 */
#define TB6612_DEFAULT_PWM_FREQ         10000   /**< 默认PWM频率 10kHz */
#define TB6612_DEFAULT_PWM_RESOLUTION   10      /**< 默认PWM分辨率 10位 */
#define TB6612_DEFAULT_MAX_DUTY         95      /**< 默认最大占空比 95% */
#define TB6612_DEFAULT_MIN_DUTY         5       /**< 默认最小占空比 5% */

/* ========================================================================== */
/*                              私有变量                                      */
/* ========================================================================== */

/**
 * @brief TB6612FNG驱动器全局状态
 */
static tb6612_driver_t g_tb6612_driver = {0};

/* ========================================================================== */
/*                              端口层接口声明                                */
/* ========================================================================== */

/**
 * @brief 端口层接口声明
 * @note 这些函数由具体的端口层实现，如ports/stm32f407/motor_port.c
 */
extern tb6612_error_t motor_port_init(const tb6612_config_t *config);
extern tb6612_error_t motor_port_deinit(void);
extern tb6612_error_t motor_port_set_direction(tb6612_motor_t motor, tb6612_direction_t direction);
extern tb6612_error_t motor_port_set_speed(tb6612_motor_t motor, uint16_t speed_percent);



/* ========================================================================== */
/*                              私有函数声明                                  */
/* ========================================================================== */

static bool tb6612_is_valid_motor(tb6612_motor_t motor);
static bool tb6612_is_valid_direction(tb6612_direction_t direction);
static bool tb6612_is_valid_speed(uint16_t speed_percent);
static bool tb6612_is_valid_config(const tb6612_config_t *config);
static void tb6612_set_default_config(tb6612_config_t *config);


/* ========================================================================== */
/*                              公共函数实现                                  */
/* ========================================================================== */



/**
 * @brief 初始化TB6612FNG驱动
 */
tb6612_error_t tb6612_init(const tb6612_config_t *config)
{
    tb6612_error_t ret = TB6612_OK;
    
    /* 检查是否已经初始化 */
    if (g_tb6612_driver.initialized) {
        return TB6612_OK;
    }
    
    /* 清零驱动器状态 */
    memset(&g_tb6612_driver, 0, sizeof(tb6612_driver_t));
    
    /* 设置配置参数 */
    if (config != NULL) {
        if (!tb6612_is_valid_config(config)) {
            return TB6612_ERROR_INVALID_PARAM;
        }
        memcpy(&g_tb6612_driver.config, config, sizeof(tb6612_config_t));
    } else {
        /* 使用默认配置 */
        tb6612_set_default_config(&g_tb6612_driver.config);
    }
    
    /* 初始化端口层 */
    ret = motor_port_init(&g_tb6612_driver.config);
    if (ret != TB6612_OK) {
        return ret;
    }
    
    /* 初始化电机状态 */
    for (int i = 0; i < TB6612_MOTOR_MAX; i++) {
        g_tb6612_driver.motor_status[i].direction = TB6612_STOP;
        g_tb6612_driver.motor_status[i].state = TB6612_STATE_IDLE;
        g_tb6612_driver.motor_status[i].speed_percent = 0;
    }

    /* 设置初始化标志 */
    g_tb6612_driver.initialized = true;
    
    return TB6612_OK;
}

/**
 * @brief 反初始化TB6612FNG驱动
 */
tb6612_error_t tb6612_deinit(void)
{
    if (!g_tb6612_driver.initialized) {
        return TB6612_ERROR_NOT_INITIALIZED;
    }
    
    /* 停止所有电机 */
    tb6612_stop_all();
    
    /* 反初始化端口层 */
    motor_port_deinit();
    
    /* 清零驱动器状态 */
    memset(&g_tb6612_driver, 0, sizeof(tb6612_driver_t));
    
    return TB6612_OK;
}

/**
 * @brief 设置电机方向
 */
tb6612_error_t tb6612_set_direction(tb6612_motor_t motor, tb6612_direction_t direction)
{
    tb6612_error_t ret = TB6612_OK;
    
    /* 参数检查 */
    if (!g_tb6612_driver.initialized) {
        return TB6612_ERROR_NOT_INITIALIZED;
    }
    
    if (!tb6612_is_valid_motor(motor)) {
        return TB6612_ERROR_INVALID_PARAM;
    }
    
    if (!tb6612_is_valid_direction(direction)) {
        return TB6612_ERROR_INVALID_PARAM;
    }
    
    /* 调用端口层设置方向 */
    ret = motor_port_set_direction(motor, direction);
    if (ret != TB6612_OK) {
        return ret;
    }
    
    /* 更新电机状态 */
    g_tb6612_driver.motor_status[motor].direction = direction;
    
    /* 根据方向更新电机状态 */
    if (direction == TB6612_STOP || direction == TB6612_BRAKE) {
        g_tb6612_driver.motor_status[motor].state = TB6612_STATE_IDLE;
        g_tb6612_driver.motor_status[motor].speed_percent = 0;
    } else {
        g_tb6612_driver.motor_status[motor].state = TB6612_STATE_RUNNING;
    }
    
    return TB6612_OK;
}

/**
 * @brief 设置电机速度
 */
tb6612_error_t tb6612_set_speed(tb6612_motor_t motor, uint16_t speed_percent)
{
    tb6612_error_t ret = TB6612_OK;
    
    /* 参数检查 */
    if (!g_tb6612_driver.initialized) {
        return TB6612_ERROR_NOT_INITIALIZED;
    }
    
    if (!tb6612_is_valid_motor(motor)) {
        return TB6612_ERROR_INVALID_PARAM;
    }
    
    if (!tb6612_is_valid_speed(speed_percent)) {
        return TB6612_ERROR_INVALID_PARAM;
    }
    
    /* 调用端口层设置速度 */
    ret = motor_port_set_speed(motor, speed_percent);
    if (ret != TB6612_OK) {
        return ret;
    }
    
    /* 更新电机状态 */
    g_tb6612_driver.motor_status[motor].speed_percent = speed_percent;
    
    /* 根据速度更新电机状态 */
    if (speed_percent == 0) {
        g_tb6612_driver.motor_status[motor].state = TB6612_STATE_IDLE;
    } else {
        g_tb6612_driver.motor_status[motor].state = TB6612_STATE_RUNNING;
    }
    
    return TB6612_OK;
}

/**
 * @brief 停止指定电机
 */
tb6612_error_t tb6612_stop(tb6612_motor_t motor)
{
    return tb6612_set_direction(motor, TB6612_STOP);
}



/**
 * @brief 停止所有电机
 */
tb6612_error_t tb6612_stop_all(void)
{
    tb6612_error_t ret = TB6612_OK;
    
    if (!g_tb6612_driver.initialized) {
        return TB6612_ERROR_NOT_INITIALIZED;
    }
    
    /* 停止所有电机 */
    for (int i = 0; i < TB6612_MOTOR_MAX; i++) {
        tb6612_error_t motor_ret = tb6612_stop((tb6612_motor_t)i);
        if (motor_ret != TB6612_OK) {
            ret = motor_ret;  /* 记录最后一个错误 */
        }
    }
    
    return ret;
}



/**
 * @brief 检查驱动是否已初始化
 */
bool tb6612_is_initialized(void)
{
    return g_tb6612_driver.initialized;
}

/* ========================================================================== */
/*                              高级控制函数实现                              */
/* ========================================================================== */

/**
 * @brief 双电机协调控制
 */
tb6612_error_t tb6612_set_motor_pair(uint16_t speed_a, tb6612_direction_t dir_a,
                                     uint16_t speed_b, tb6612_direction_t dir_b)
{
    tb6612_error_t ret = TB6612_OK;

    /* 参数检查 */
    if (!g_tb6612_driver.initialized) {
        return TB6612_ERROR_NOT_INITIALIZED;
    }

    if (!tb6612_is_valid_speed(speed_a) || !tb6612_is_valid_speed(speed_b)) {
        return TB6612_ERROR_INVALID_PARAM;
    }

    if (!tb6612_is_valid_direction(dir_a) || !tb6612_is_valid_direction(dir_b)) {
        return TB6612_ERROR_INVALID_PARAM;
    }

    /* 同时设置两个电机的方向 */
    ret = tb6612_set_direction(TB6612_MOTOR_A, dir_a);
    if (ret != TB6612_OK) {
        return ret;
    }

    ret = tb6612_set_direction(TB6612_MOTOR_B, dir_b);
    if (ret != TB6612_OK) {
        return ret;
    }

    /* 同时设置两个电机的速度 */
    ret = tb6612_set_speed(TB6612_MOTOR_A, speed_a);
    if (ret != TB6612_OK) {
        return ret;
    }

    ret = tb6612_set_speed(TB6612_MOTOR_B, speed_b);
    if (ret != TB6612_OK) {
        return ret;
    }

    return TB6612_OK;
}



/* ========================================================================== */
/*                              2轮驱动专用函数实现                          */
/* ========================================================================== */

/**
 * @brief 2轮小车前进
 */
tb6612_error_t tb6612_move_forward(uint16_t speed)
{
    return tb6612_set_motor_pair(speed, TB6612_FORWARD, speed, TB6612_FORWARD);
}

/**
 * @brief 2轮小车后退
 */
tb6612_error_t tb6612_move_backward(uint16_t speed)
{
    return tb6612_set_motor_pair(speed, TB6612_BACKWARD, speed, TB6612_BACKWARD);
}

/**
 * @brief 2轮小车左转
 */
tb6612_error_t tb6612_turn_left(uint16_t speed)
{
    return tb6612_set_motor_pair(0, TB6612_STOP, speed, TB6612_FORWARD);
}

/**
 * @brief 2轮小车右转
 */
tb6612_error_t tb6612_turn_right(uint16_t speed)
{
    return tb6612_set_motor_pair(speed, TB6612_FORWARD, 0, TB6612_STOP);
}

/* ========================================================================== */
/*                              私有函数实现                                  */
/* ========================================================================== */

/**
 * @brief 检查电机标识是否有效
 * @param motor 电机标识
 * @return bool 有效性
 */
static bool tb6612_is_valid_motor(tb6612_motor_t motor)
{
    return (motor >= TB6612_MOTOR_A && motor < TB6612_MOTOR_MAX);
}

/**
 * @brief 检查电机方向是否有效
 * @param direction 电机方向
 * @return bool 有效性
 */
static bool tb6612_is_valid_direction(tb6612_direction_t direction)
{
    return (direction >= TB6612_STOP && direction <= TB6612_BRAKE);
}

/**
 * @brief 检查速度百分比是否有效
 * @param speed_percent 速度百分比
 * @return bool 有效性
 */
static bool tb6612_is_valid_speed(uint16_t speed_percent)
{
    return (speed_percent >= TB6612_MIN_SPEED_PERCENT &&
            speed_percent <= TB6612_MAX_SPEED_PERCENT);
}

/**
 * @brief 检查配置参数是否有效
 * @param config 配置参数指针
 * @return bool 有效性
 */
static bool tb6612_is_valid_config(const tb6612_config_t *config)
{
    if (config == NULL) {
        return false;
    }

    /* 检查PWM频率范围 */
    if (config->pwm_frequency < 1000 || config->pwm_frequency > 20000) {
        return false;
    }

    /* 检查PWM分辨率范围 */
    if (config->pwm_resolution < 8 || config->pwm_resolution > 16) {
        return false;
    }

    /* 检查占空比范围 */
    if (config->max_duty_cycle > 100 || config->min_duty_cycle >= config->max_duty_cycle) {
        return false;
    }

    return true;
}

/**
 * @brief 设置默认配置参数
 * @param config 配置参数指针
 */
static void tb6612_set_default_config(tb6612_config_t *config)
{
    if (config == NULL) {
        return;
    }

    config->pwm_frequency = TB6612_DEFAULT_PWM_FREQ;
    config->pwm_resolution = TB6612_DEFAULT_PWM_RESOLUTION;
    config->max_duty_cycle = TB6612_DEFAULT_MAX_DUTY;
    config->min_duty_cycle = TB6612_DEFAULT_MIN_DUTY;
}


