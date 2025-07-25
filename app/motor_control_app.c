/**
 * @file motor_control_app.c
 * @brief TB6612FNG电机控制应用层实现
 * @details 本文件实现了2轮驱动电机控制应用层的完整功能。
 *          遵循简洁设计原则，提供基础的电机控制功能。
 * @author Augment Agent
 * @date 2025-07-25
 * @version 1.0.0
 */

#include "motor_control_app.h"
#include "../hardware/motor_drivers/tb6612fng/tb6612fng.h"
#include <string.h>
#include <stdlib.h>

/* ========================================================================== */
/*                              私有变量定义                                  */
/* ========================================================================== */

/**
 * @brief 电机应用状态全局变量
 */
static motor_app_status_t g_motor_app_status = {0};

/* ========================================================================== */
/*                              私有函数声明                                  */
/* ========================================================================== */

/**
 * @brief 更新电机状态信息
 * @param motor 电机标识 (0=A, 1=B)
 * @param speed 速度 (0-100)
 * @param direction 方向 (-1:后退, 0:停止, 1:前进)
 */
static void update_motor_status(uint8_t motor, uint16_t speed, int8_t direction);

/**
 * @brief 参数有效性检查
 * @param speed 速度参数
 * @return bool 检查结果
 * @retval true 参数有效
 * @retval false 参数无效
 */
static bool is_valid_speed(uint16_t speed);

/* ========================================================================== */
/*                              应用层API接口实现                            */
/* ========================================================================== */

/**
 * @brief 初始化电机控制应用
 */
int32_t motor_app_init(void)
{
    /* 检查是否已经初始化 */
    if (g_motor_app_status.initialized) {
        return 0;  /* 已初始化，直接返回成功 */
    }
    
    /* 清零状态结构体 */
    memset(&g_motor_app_status, 0, sizeof(motor_app_status_t));
    
    /* 初始化TB6612FNG驱动层 */
    if (tb6612_init(NULL) != TB6612_OK) {
        return -1;  /* 驱动初始化失败 */
    }
    
    /* 设置初始化状态 */
    g_motor_app_status.initialized = true;
    g_motor_app_status.motor_a_enabled = true;
    g_motor_app_status.motor_b_enabled = true;
    
    /* 确保电机初始状态为停止 */
    tb6612_stop_all();
    
    return 0;  /* 初始化成功 */
}

/**
 * @brief 反初始化电机控制应用
 */
int32_t motor_app_deinit(void)
{
    /* 检查是否已初始化 */
    if (!g_motor_app_status.initialized) {
        return 0;  /* 未初始化，直接返回成功 */
    }
    
    /* 停止所有电机 */
    tb6612_stop_all();
    
    /* 反初始化TB6612FNG驱动层 */
    tb6612_deinit();
    
    /* 清零状态结构体 */
    memset(&g_motor_app_status, 0, sizeof(motor_app_status_t));
    
    return 0;  /* 反初始化成功 */
}

/**
 * @brief 检查电机应用是否已初始化
 */
bool motor_app_is_initialized(void)
{
    return g_motor_app_status.initialized;
}

/**
 * @brief 获取电机应用状态
 */
int32_t motor_app_get_status(motor_app_status_t *status)
{
    /* 参数检查 */
    if (status == NULL) {
        return -1;
    }
    
    if (!g_motor_app_status.initialized) {
        return -1;
    }
    
    /* 复制状态信息 */
    memcpy(status, &g_motor_app_status, sizeof(motor_app_status_t));
    
    return 0;
}

/* ========================================================================== */
/*                              简化版电机控制接口实现                        */
/* ========================================================================== */

/**
 * @brief 统一控制双电机（简化版）
 */
int32_t motor_app_control_motors(const motor_control_t *control)
{
    /* 参数检查 */
    if (control == NULL) {
        return -1;
    }
    
    if (!g_motor_app_status.initialized) {
        return -1;
    }
    
    /* 处理左轮控制 */
    uint16_t left_speed = (uint16_t)abs(control->left_speed);
    tb6612_direction_t left_dir = TB6612_STOP;
    if (control->left_speed > 0) {
        left_dir = TB6612_FORWARD;
    } else if (control->left_speed < 0) {
        left_dir = TB6612_BACKWARD;
    }
    
    /* 处理右轮控制 */
    uint16_t right_speed = (uint16_t)abs(control->right_speed);
    tb6612_direction_t right_dir = TB6612_STOP;
    if (control->right_speed > 0) {
        right_dir = TB6612_FORWARD;
    } else if (control->right_speed < 0) {
        right_dir = TB6612_BACKWARD;
    }
    
    /* 调用TB6612FNG驱动层接口 */
    if (tb6612_set_motor_pair(left_speed, left_dir, right_speed, right_dir) != TB6612_OK) {
        return -1;
    }
    
    /* 更新状态信息 */
    update_motor_status(0, left_speed, (control->left_speed > 0) ? 1 : ((control->left_speed < 0) ? -1 : 0));
    update_motor_status(1, right_speed, (control->right_speed > 0) ? 1 : ((control->right_speed < 0) ? -1 : 0));
    
    return 0;
}

/* ========================================================================== */
/*                              2轮驱动运动控制接口实现                      */
/* ========================================================================== */

/**
 * @brief 小车前进
 */
int32_t motor_app_move_forward(uint16_t speed)
{
    /* 参数检查 */
    if (!is_valid_speed(speed)) {
        return -1;
    }
    
    if (!g_motor_app_status.initialized) {
        return -1;
    }
    
    /* 调用TB6612FNG驱动层接口 */
    if (tb6612_move_forward(speed) != TB6612_OK) {
        return -1;
    }
    
    /* 更新状态信息 */
    update_motor_status(0, speed, 1);  /* 电机A前进 */
    update_motor_status(1, speed, 1);  /* 电机B前进 */
    
    return 0;
}

/**
 * @brief 小车后退
 */
int32_t motor_app_move_backward(uint16_t speed)
{
    /* 参数检查 */
    if (!is_valid_speed(speed)) {
        return -1;
    }
    
    if (!g_motor_app_status.initialized) {
        return -1;
    }
    
    /* 调用TB6612FNG驱动层接口 */
    if (tb6612_move_backward(speed) != TB6612_OK) {
        return -1;
    }
    
    /* 更新状态信息 */
    update_motor_status(0, speed, -1);  /* 电机A后退 */
    update_motor_status(1, speed, -1);  /* 电机B后退 */
    
    return 0;
}

/**
 * @brief 小车左转
 */
int32_t motor_app_turn_left(uint16_t speed)
{
    /* 参数检查 */
    if (!is_valid_speed(speed)) {
        return -1;
    }
    
    if (!g_motor_app_status.initialized) {
        return -1;
    }
    
    /* 调用TB6612FNG驱动层接口 */
    if (tb6612_turn_left(speed) != TB6612_OK) {
        return -1;
    }
    
    /* 更新状态信息 */
    update_motor_status(0, speed, -1);  /* 电机A后退 */
    update_motor_status(1, speed, 1);   /* 电机B前进 */
    
    return 0;
}

/**
 * @brief 小车右转
 */
int32_t motor_app_turn_right(uint16_t speed)
{
    /* 参数检查 */
    if (!is_valid_speed(speed)) {
        return -1;
    }
    
    if (!g_motor_app_status.initialized) {
        return -1;
    }
    
    /* 调用TB6612FNG驱动层接口 */
    if (tb6612_turn_right(speed) != TB6612_OK) {
        return -1;
    }
    
    /* 更新状态信息 */
    update_motor_status(0, speed, 1);   /* 电机A前进 */
    update_motor_status(1, speed, -1);  /* 电机B后退 */
    
    return 0;
}

/**
 * @brief 停止所有电机
 */
int32_t motor_app_stop_all(void)
{
    if (!g_motor_app_status.initialized) {
        return -1;
    }
    
    /* 调用TB6612FNG驱动层接口 */
    if (tb6612_stop_all() != TB6612_OK) {
        return -1;
    }
    
    /* 更新状态信息 */
    update_motor_status(0, 0, 0);  /* 电机A停止 */
    update_motor_status(1, 0, 0);  /* 电机B停止 */
    
    return 0;
}

/* ========================================================================== */
/*                              私有函数实现                                  */
/* ========================================================================== */

/**
 * @brief 更新电机状态信息
 */
static void update_motor_status(uint8_t motor, uint16_t speed, int8_t direction)
{
    if (motor == 0) {  /* 电机A */
        g_motor_app_status.current_speed_a = speed;
        g_motor_app_status.current_dir_a = direction;
    } else if (motor == 1) {  /* 电机B */
        g_motor_app_status.current_speed_b = speed;
        g_motor_app_status.current_dir_b = direction;
    }
}

/**
 * @brief 参数有效性检查
 */
static bool is_valid_speed(uint16_t speed)
{
    return (speed <= 100);
}

/* ========================================================================== */
/*                              基础测试接口实现                              */
/* ========================================================================== */

/**
 * @brief 基础控制功能测试
 */
int32_t motor_app_test_basic_control(void)
{
    if (!g_motor_app_status.initialized) {
        return -1;
    }

    /* 测试单电机控制 */
    motor_control_t test_control;

    /* 测试左轮前进 */
    test_control.left_speed = 30;
    test_control.right_speed = 0;
    if (motor_app_control_motors(&test_control) != 0) {
        return -1;
    }

    /* 停止 */
    motor_app_stop_all();

    /* 测试右轮前进 */
    test_control.left_speed = 0;
    test_control.right_speed = 30;
    if (motor_app_control_motors(&test_control) != 0) {
        return -1;
    }

    /* 停止 */
    motor_app_stop_all();

    return 0;  /* 测试通过 */
}

/**
 * @brief 2轮驱动运动测试
 */
int32_t motor_app_test_2wheel_motion(void)
{
    if (!g_motor_app_status.initialized) {
        return -1;
    }

    /* 测试前进 */
    if (motor_app_move_forward(40) != 0) {
        return -1;
    }
    motor_app_stop_all();

    /* 测试后退 */
    if (motor_app_move_backward(40) != 0) {
        return -1;
    }
    motor_app_stop_all();

    /* 测试左转 */
    if (motor_app_turn_left(30) != 0) {
        return -1;
    }
    motor_app_stop_all();

    /* 测试右转 */
    if (motor_app_turn_right(30) != 0) {
        return -1;
    }
    motor_app_stop_all();

    return 0;  /* 测试通过 */
}
