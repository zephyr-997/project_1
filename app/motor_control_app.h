/**
 * @file motor_control_app.h
 * @brief TB6612FNG电机控制应用层接口定义
 * @details 本文件定义了2轮驱动电机控制应用层的完整接口，遵循简洁设计原则。
 *          提供基础的电机控制功能，包括前进、后退、转向、停止等运动控制。
 *          专为2轮驱动小车设计，预留PID控制接口扩展点。
 * @author Augment Agent
 * @date 2025-07-25
 * @version 1.0.0
 * 
 * @note 设计原则：
 *       - 基本功能优先：优先实现核心电机控制功能
 *       - 代码简洁：每个函数不超过30行，避免过度设计
 *       - 逻辑清晰：保持线性逻辑，避免复杂的跳转
 *       - 渐进式开发：先实现基本功能，再考虑功能扩展
 * 
 * @usage 在Keil5项目中的使用方法：
 *        1. 将motor_control_app.c添加到项目源文件
 *        2. 将motor_control_app.h添加到项目头文件
 *        3. 确保TB6612FNG驱动层和端口层已正确配置
 *        4. 调用motor_app_init()初始化应用
 */

#ifndef MOTOR_CONTROL_APP_H__
#define MOTOR_CONTROL_APP_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                              数据结构定义                                  */
/* ========================================================================== */

/**
 * @brief 电机控制结构体（简化版）
 * @details 用于统一控制双电机的速度和方向
 */
typedef struct {
    int16_t left_speed;     /**< 左轮速度 (-100 到 +100) */
    int16_t right_speed;    /**< 右轮速度 (-100 到 +100) */
} motor_control_t;

/**
 * @brief 电机应用状态结构体
 * @details 记录电机应用的当前状态信息
 */
typedef struct {
    bool initialized;           /**< 初始化状态 */
    bool motor_a_enabled;       /**< 电机A使能状态 */
    bool motor_b_enabled;       /**< 电机B使能状态 */
    uint16_t current_speed_a;   /**< 电机A当前速度 (0-100) */
    uint16_t current_speed_b;   /**< 电机B当前速度 (0-100) */
    int8_t current_dir_a;       /**< 电机A当前方向 (-1:后退, 0:停止, 1:前进) */
    int8_t current_dir_b;       /**< 电机B当前方向 (-1:后退, 0:停止, 1:前进) */
} motor_app_status_t;

/* ========================================================================== */
/*                              应用层API接口                                 */
/* ========================================================================== */

/**
 * @brief 初始化电机控制应用
 * @return int32_t 错误码
 * @retval 0 初始化成功
 * @retval -1 初始化失败
 * 
 * @note 此函数会初始化TB6612FNG驱动层和端口层
 */
int32_t motor_app_init(void);

/**
 * @brief 反初始化电机控制应用
 * @return int32_t 错误码
 * @retval 0 反初始化成功
 * @retval -1 反初始化失败
 * 
 * @note 此函数会停止所有电机并释放资源
 */
int32_t motor_app_deinit(void);

/**
 * @brief 检查电机应用是否已初始化
 * @return bool 初始化状态
 * @retval true 已初始化
 * @retval false 未初始化
 */
bool motor_app_is_initialized(void);

/**
 * @brief 获取电机应用状态
 * @param status 输出参数，存储电机应用状态的指针
 * @return int32_t 错误码
 * @retval 0 获取成功
 * @retval -1 获取失败（参数无效或未初始化）
 */
int32_t motor_app_get_status(motor_app_status_t *status);

/* ========================================================================== */
/*                              简化版电机控制接口                            */
/* ========================================================================== */

/**
 * @brief 统一控制双电机（简化版）
 * @param control 电机控制参数指针
 * @return int32_t 错误码
 * @retval 0 控制成功
 * @retval -1 控制失败
 * 
 * @note 正值表示前进，负值表示后退，0表示停止
 *       函数会自动处理方向转换和速度设置
 */
int32_t motor_app_control_motors(const motor_control_t *control);

/* ========================================================================== */
/*                              2轮驱动运动控制接口                          */
/* ========================================================================== */

/**
 * @brief 小车前进
 * @param speed 前进速度 (0-100)
 * @return int32_t 错误码
 * @retval 0 控制成功
 * @retval -1 控制失败
 */
int32_t motor_app_move_forward(uint16_t speed);

/**
 * @brief 小车后退
 * @param speed 后退速度 (0-100)
 * @return int32_t 错误码
 * @retval 0 控制成功
 * @retval -1 控制失败
 */
int32_t motor_app_move_backward(uint16_t speed);

/**
 * @brief 小车左转
 * @param speed 转向速度 (0-100)
 * @return int32_t 错误码
 * @retval 0 控制成功
 * @retval -1 控制失败
 */
int32_t motor_app_turn_left(uint16_t speed);

/**
 * @brief 小车右转
 * @param speed 转向速度 (0-100)
 * @return int32_t 错误码
 * @retval 0 控制成功
 * @retval -1 控制失败
 */
int32_t motor_app_turn_right(uint16_t speed);

/**
 * @brief 停止所有电机
 * @return int32_t 错误码
 * @retval 0 停止成功
 * @retval -1 停止失败
 */
int32_t motor_app_stop_all(void);

/* ========================================================================== */
/*                              基础测试接口                                  */
/* ========================================================================== */

/**
 * @brief 基础控制功能测试
 * @return int32_t 测试结果
 * @retval 0 测试通过
 * @retval -1 测试失败
 * 
 * @note 测试基本的电机控制功能
 */
int32_t motor_app_test_basic_control(void);

/**
 * @brief 2轮驱动运动测试
 * @return int32_t 测试结果
 * @retval 0 测试通过
 * @retval -1 测试失败
 * 
 * @note 测试2轮驱动的各种运动模式
 */
int32_t motor_app_test_2wheel_motion(void);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_CONTROL_APP_H__ */
