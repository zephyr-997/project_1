/**
 * @file tb6612fng.h
 * @brief TB6612FNG双路电机驱动芯片驱动库头文件
 * @details 本文件定义了TB6612FNG电机驱动芯片的完整驱动接口，支持双路电机独立控制。
 *          TB6612FNG是一款高性能双路电机驱动芯片，支持PWM速度控制和方向控制。
 *          适用于直流电机、步进电机等多种电机类型的驱动控制。
 * @author Augment Agent
 * @date 2025-07-25
 * @version 1.0.0
 * 
 * @note 硬件连接说明:
 *       - PWMA: PWM信号输入，控制电机A速度
 *       - PWMB: PWM信号输入，控制电机B速度  
 *       - AIN1/AIN2: 电机A方向控制引脚
 *       - BIN1/BIN2: 电机B方向控制引脚
 *       - STBY: 待机控制引脚，低电平进入待机模式
 *       - VM: 电机电源输入 (2.7V-10.8V)
 *       - VCC: 逻辑电源输入 (2.7V-5.5V)
 */

#ifndef TB6612FNG_H__
#define TB6612FNG_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                              版本信息                                      */
/* ========================================================================== */

#define TB6612FNG_VERSION_MAJOR    1        /**< 主版本号 */
#define TB6612FNG_VERSION_MINOR    0        /**< 次版本号 */
#define TB6612FNG_VERSION_PATCH    0        /**< 补丁版本号 */

/* ========================================================================== */
/*                              错误码定义                                    */
/* ========================================================================== */

/**
 * @brief TB6612FNG错误码枚举
 */
typedef enum {
    TB6612_OK = 0,                          /**< 操作成功 */
    TB6612_ERROR = -1,                      /**< 通用错误 */
    TB6612_ERROR_INVALID_PARAM = -2,        /**< 无效参数 */
    TB6612_ERROR_NOT_INITIALIZED = -3,      /**< 未初始化 */
    TB6612_ERROR_HARDWARE_FAULT = -4,       /**< 硬件故障 */
    TB6612_ERROR_OVERCURRENT = -5,          /**< 过流保护 */
    TB6612_ERROR_OVERTEMPERATURE = -6,      /**< 过温保护 */
    TB6612_ERROR_TIMEOUT = -7               /**< 操作超时 */
} tb6612_error_t;

/* ========================================================================== */
/*                              数据结构定义                                  */
/* ========================================================================== */

/**
 * @brief 电机标识枚举
 */
typedef enum {
    TB6612_MOTOR_A = 0,                     /**< 电机A */
    TB6612_MOTOR_B = 1,                     /**< 电机B */
    TB6612_MOTOR_MAX                        /**< 电机数量上限 */
} tb6612_motor_t;

/**
 * @brief 电机方向枚举
 * @note 根据TB6612FNG真值表定义:
 *       - STOP: IN1=0, IN2=0 (短路制动)
 *       - FORWARD: IN1=1, IN2=0 (正转)
 *       - BACKWARD: IN1=0, IN2=1 (反转)
 *       - BRAKE: IN1=1, IN2=1 (短路制动)
 */
typedef enum {
    TB6612_STOP = 0,                        /**< 停止 (短路制动) */
    TB6612_FORWARD,                         /**< 前进 (正转) */
    TB6612_BACKWARD,                        /**< 后退 (反转) */
    TB6612_BRAKE                            /**< 刹车 (短路制动) */
} tb6612_direction_t;

/**
 * @brief 电机状态枚举
 */
typedef enum {
    TB6612_STATE_IDLE = 0,                  /**< 空闲状态 */
    TB6612_STATE_RUNNING,                   /**< 运行状态 */
    TB6612_STATE_BRAKING,                   /**< 制动状态 */
    TB6612_STATE_FAULT                      /**< 故障状态 */
} tb6612_state_t;

/**
 * @brief TB6612FNG配置结构体
 */
typedef struct {
    uint16_t pwm_frequency;                 /**< PWM频率 (Hz), 范围: 1000-20000 */
    uint8_t  pwm_resolution;                /**< PWM分辨率 (位), 范围: 8-16 */
    uint16_t max_duty_cycle;                /**< 最大占空比 (%), 范围: 1-100 */
    uint16_t min_duty_cycle;                /**< 最小占空比 (%), 范围: 0-99 */
} tb6612_config_t;

/**
 * @brief 电机状态信息结构体
 */
typedef struct {
    tb6612_direction_t direction;           /**< 当前方向 */
    tb6612_state_t state;                   /**< 当前状态 */
    uint16_t speed_percent;                 /**< 当前速度百分比 (0-100) */
} tb6612_motor_status_t;

/**
 * @brief TB6612FNG驱动状态结构体
 */
typedef struct {
    bool initialized;                       /**< 初始化状态 */
    tb6612_config_t config;                 /**< 配置参数 */
    tb6612_motor_status_t motor_status[TB6612_MOTOR_MAX]; /**< 电机状态 */
} tb6612_driver_t;

/* ========================================================================== */
/*                              函数声明                                      */
/* ========================================================================== */



/**
 * @brief 初始化TB6612FNG驱动
 * @param config 配置参数指针，如果为NULL则使用默认配置
 * @return tb6612_error_t 错误码
 * @retval TB6612_OK 初始化成功
 * @retval TB6612_ERROR_INVALID_PARAM 配置参数无效
 * @retval TB6612_ERROR_HARDWARE_FAULT 硬件初始化失败
 * 
 * @note 此函数会初始化PWM输出、GPIO控制引脚等硬件资源
 */
tb6612_error_t tb6612_init(const tb6612_config_t *config);

/**
 * @brief 反初始化TB6612FNG驱动
 * @return tb6612_error_t 错误码
 * @retval TB6612_OK 反初始化成功
 * 
 * @note 此函数会释放所有硬件资源，停止所有电机
 */
tb6612_error_t tb6612_deinit(void);

/**
 * @brief 设置电机方向
 * @param motor 电机标识
 * @param direction 电机方向
 * @return tb6612_error_t 错误码
 * @retval TB6612_OK 设置成功
 * @retval TB6612_ERROR_INVALID_PARAM 参数无效
 * @retval TB6612_ERROR_NOT_INITIALIZED 驱动未初始化
 */
tb6612_error_t tb6612_set_direction(tb6612_motor_t motor, tb6612_direction_t direction);

/**
 * @brief 设置电机速度
 * @param motor 电机标识
 * @param speed_percent 速度百分比 (0-100)
 * @return tb6612_error_t 错误码
 * @retval TB6612_OK 设置成功
 * @retval TB6612_ERROR_INVALID_PARAM 参数无效
 * @retval TB6612_ERROR_NOT_INITIALIZED 驱动未初始化
 * 
 * @note 速度为0时电机停止，速度越大转速越快
 */
tb6612_error_t tb6612_set_speed(tb6612_motor_t motor, uint16_t speed_percent);

/**
 * @brief 停止指定电机
 * @param motor 电机标识
 * @return tb6612_error_t 错误码
 * @retval TB6612_OK 停止成功
 * @retval TB6612_ERROR_INVALID_PARAM 参数无效
 * @retval TB6612_ERROR_NOT_INITIALIZED 驱动未初始化
 */
tb6612_error_t tb6612_stop(tb6612_motor_t motor);



/**
 * @brief 停止所有电机
 * @return tb6612_error_t 错误码
 * @retval TB6612_OK 停止成功
 * @retval TB6612_ERROR_NOT_INITIALIZED 驱动未初始化
 */
tb6612_error_t tb6612_stop_all(void);



/**
 * @brief 检查驱动是否已初始化
 * @return bool 初始化状态
 * @retval true 已初始化
 * @retval false 未初始化
 */
bool tb6612_is_initialized(void);

/* ========================================================================== */
/*                              高级控制函数                                  */
/* ========================================================================== */

/**
 * @brief 双电机协调控制
 * @param speed_a 电机A速度百分比 (0-100)
 * @param dir_a 电机A方向
 * @param speed_b 电机B速度百分比 (0-100)
 * @param dir_b 电机B方向
 * @return tb6612_error_t 错误码
 * @retval TB6612_OK 设置成功
 * @retval TB6612_ERROR_INVALID_PARAM 参数无效
 * @retval TB6612_ERROR_NOT_INITIALIZED 驱动未初始化
 *
 * @note 此函数可以同时控制两个电机，确保动作的同步性
 */
tb6612_error_t tb6612_set_motor_pair(uint16_t speed_a, tb6612_direction_t dir_a,
                                     uint16_t speed_b, tb6612_direction_t dir_b);

/* ========================================================================== */
/*                              2轮驱动专用函数                              */
/* ========================================================================== */

/**
 * @brief 2轮小车前进
 * @param speed 速度百分比 (0-100)
 * @return tb6612_error_t 错误码
 * @retval TB6612_OK 设置成功
 * @retval TB6612_ERROR_INVALID_PARAM 参数无效
 * @retval TB6612_ERROR_NOT_INITIALIZED 驱动未初始化
 */
tb6612_error_t tb6612_move_forward(uint16_t speed);

/**
 * @brief 2轮小车后退
 * @param speed 速度百分比 (0-100)
 * @return tb6612_error_t 错误码
 * @retval TB6612_OK 设置成功
 * @retval TB6612_ERROR_INVALID_PARAM 参数无效
 * @retval TB6612_ERROR_NOT_INITIALIZED 驱动未初始化
 */
tb6612_error_t tb6612_move_backward(uint16_t speed);

/**
 * @brief 2轮小车左转
 * @param speed 速度百分比 (0-100)
 * @return tb6612_error_t 错误码
 * @retval TB6612_OK 设置成功
 * @retval TB6612_ERROR_INVALID_PARAM 参数无效
 * @retval TB6612_ERROR_NOT_INITIALIZED 驱动未初始化
 */
tb6612_error_t tb6612_turn_left(uint16_t speed);

/**
 * @brief 2轮小车右转
 * @param speed 速度百分比 (0-100)
 * @return tb6612_error_t 错误码
 * @retval TB6612_OK 设置成功
 * @retval TB6612_ERROR_INVALID_PARAM 参数无效
 * @retval TB6612_ERROR_NOT_INITIALIZED 驱动未初始化
 */
tb6612_error_t tb6612_turn_right(uint16_t speed);

#ifdef __cplusplus
}
#endif

#endif /* TB6612FNG_H__ */
