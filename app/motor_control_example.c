/**
 * @file motor_control_example.c
 * @brief TB6612FNG电机控制应用使用示例
 * @details 本文件展示了如何在main.c中集成和使用电机控制应用。
 *          提供了多种使用场景的示例代码，便于开发者参考。
 * @author Augment Agent
 * @date 2025-07-25
 * @version 1.0.0
 * 
 * @note 使用方法：
 *       1. 将相关代码复制到你的main.c文件中
 *       2. 根据实际需求选择合适的示例
 *       3. 确保已正确初始化系统时钟和HAL库
 */

#include "motor_control_app.h"
#include "main.h"  /* STM32CubeMX生成的头文件 */
#include <stdio.h>

/* ========================================================================== */
/*                              示例1：基础使用                              */
/* ========================================================================== */

/**
 * @brief 基础电机控制示例
 * @details 演示最简单的电机控制使用方法
 */
void example_basic_motor_control(void)
{
    printf("=== 基础电机控制示例 ===\n");
    
    /* 初始化电机应用 */
    if (motor_app_init() != 0) {
        printf("错误：电机应用初始化失败!\n");
        return;
    }
    printf("电机应用初始化成功\n");
    
    /* 小车前进2秒 */
    printf("小车前进...\n");
    motor_app_move_forward(50);
    HAL_Delay(2000);
    
    /* 小车左转1秒 */
    printf("小车左转...\n");
    motor_app_turn_left(30);
    HAL_Delay(1000);
    
    /* 小车后退1秒 */
    printf("小车后退...\n");
    motor_app_move_backward(40);
    HAL_Delay(1000);
    
    /* 小车右转1秒 */
    printf("小车右转...\n");
    motor_app_turn_right(30);
    HAL_Delay(1000);
    
    /* 停止所有电机 */
    printf("停止所有电机\n");
    motor_app_stop_all();
    
    printf("基础控制示例完成\n\n");
}

/* ========================================================================== */
/*                              示例2：统一控制接口                          */
/* ========================================================================== */

/**
 * @brief 统一控制接口示例
 * @details 演示使用motor_control_t结构体进行精确控制
 */
void example_unified_control(void)
{
    printf("=== 统一控制接口示例 ===\n");
    
    motor_control_t control;
    
    /* 小车前进 */
    printf("小车前进（双轮同速）...\n");
    control.left_speed = 60;
    control.right_speed = 60;
    motor_app_control_motors(&control);
    HAL_Delay(2000);
    
    /* 小车原地左转 */
    printf("小车原地左转...\n");
    control.left_speed = -40;
    control.right_speed = 40;
    motor_app_control_motors(&control);
    HAL_Delay(1000);
    
    /* 小车弧形右转 */
    printf("小车弧形右转...\n");
    control.left_speed = 70;
    control.right_speed = 30;
    motor_app_control_motors(&control);
    HAL_Delay(1500);
    
    /* 停止 */
    printf("停止\n");
    control.left_speed = 0;
    control.right_speed = 0;
    motor_app_control_motors(&control);
    
    printf("统一控制示例完成\n\n");
}

/* ========================================================================== */
/*                              示例3：状态监控                              */
/* ========================================================================== */

/**
 * @brief 状态监控示例
 * @details 演示如何获取和显示电机状态信息
 */
void example_status_monitor(void)
{
    printf("=== 状态监控示例 ===\n");
    
    motor_app_status_t status;
    
    /* 检查初始化状态 */
    if (motor_app_is_initialized()) {
        printf("电机应用已初始化\n");
    } else {
        printf("电机应用未初始化\n");
        return;
    }
    
    /* 启动电机并监控状态 */
    printf("启动电机前进...\n");
    motor_app_move_forward(75);
    
    /* 获取并显示状态 */
    if (motor_app_get_status(&status) == 0) {
        printf("电机状态信息：\n");
        printf("  电机A: 使能=%s, 速度=%d, 方向=%d\n", 
               status.motor_a_enabled ? "是" : "否",
               status.current_speed_a, 
               status.current_dir_a);
        printf("  电机B: 使能=%s, 速度=%d, 方向=%d\n", 
               status.motor_b_enabled ? "是" : "否",
               status.current_speed_b, 
               status.current_dir_b);
    }
    
    HAL_Delay(1000);
    motor_app_stop_all();
    
    printf("状态监控示例完成\n\n");
}

/* ========================================================================== */
/*                              示例4：功能测试                              */
/* ========================================================================== */

/**
 * @brief 功能测试示例
 * @details 演示如何使用内置的测试功能
 */
void example_function_test(void)
{
    printf("=== 功能测试示例 ===\n");
    
    /* 基础控制功能测试 */
    printf("执行基础控制功能测试...\n");
    if (motor_app_test_basic_control() == 0) {
        printf("✅ 基础控制功能测试通过\n");
    } else {
        printf("❌ 基础控制功能测试失败\n");
    }
    
    HAL_Delay(500);
    
    /* 2轮驱动运动测试 */
    printf("执行2轮驱动运动测试...\n");
    if (motor_app_test_2wheel_motion() == 0) {
        printf("✅ 2轮驱动运动测试通过\n");
    } else {
        printf("❌ 2轮驱动运动测试失败\n");
    }
    
    printf("功能测试示例完成\n\n");
}

/* ========================================================================== */
/*                              示例5：错误处理                              */
/* ========================================================================== */

/**
 * @brief 错误处理示例
 * @details 演示如何进行错误处理和异常恢复
 */
void example_error_handling(void)
{
    printf("=== 错误处理示例 ===\n");
    
    /* 测试未初始化时的错误处理 */
    motor_app_deinit();  /* 先反初始化 */
    
    printf("测试未初始化状态下的调用...\n");
    int ret = motor_app_move_forward(50);
    if (ret != 0) {
        printf("✅ 正确检测到未初始化错误\n");
    }
    
    /* 重新初始化 */
    if (motor_app_init() == 0) {
        printf("重新初始化成功\n");
    }
    
    /* 测试无效参数的错误处理 */
    printf("测试无效参数...\n");
    ret = motor_app_move_forward(150);  /* 超出范围的速度 */
    if (ret != 0) {
        printf("✅ 正确检测到参数错误\n");
    }
    
    /* 测试NULL指针的错误处理 */
    printf("测试NULL指针...\n");
    ret = motor_app_control_motors(NULL);
    if (ret != 0) {
        printf("✅ 正确检测到NULL指针错误\n");
    }
    
    printf("错误处理示例完成\n\n");
}

/* ========================================================================== */
/*                              主函数示例                                   */
/* ========================================================================== */

/**
 * @brief 主函数示例
 * @details 展示如何在main函数中集成电机控制应用
 * @note 这是一个完整的main函数示例，可以直接使用
 */
void motor_control_main_example(void)
{
    printf("=== TB6612FNG电机控制应用示例 ===\n\n");
    
    /* 系统初始化（这部分通常由STM32CubeMX生成） */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_TIM1_Init();
    MX_USART1_UART_Init();
    
    printf("系统初始化完成\n");
    
    /* 运行各种示例 */
    example_basic_motor_control();
    HAL_Delay(1000);
    
    example_unified_control();
    HAL_Delay(1000);
    
    example_status_monitor();
    HAL_Delay(1000);
    
    example_function_test();
    HAL_Delay(1000);
    
    example_error_handling();
    
    printf("=== 所有示例运行完成 ===\n");
    
    /* 清理资源 */
    motor_app_deinit();
    printf("应用已清理，程序结束\n");
}

/* ========================================================================== */
/*                              简化版main函数                               */
/* ========================================================================== */

/**
 * @brief 简化版main函数示例
 * @details 最简单的集成方式，适合快速验证功能
 */
#if 0  /* 取消注释以使用此main函数 */
int main(void)
{
    /* HAL库初始化 */
    HAL_Init();
    SystemClock_Config();
    
    /* 外设初始化 */
    MX_GPIO_Init();
    MX_TIM1_Init();
    MX_USART1_UART_Init();
    
    /* 初始化电机应用 */
    if (motor_app_init() != 0) {
        printf("电机应用初始化失败!\n");
        while(1);  /* 停止程序 */
    }
    
    /* 主循环 */
    while (1)
    {
        /* 简单的运动序列 */
        motor_app_move_forward(50);
        HAL_Delay(2000);
        
        motor_app_turn_left(30);
        HAL_Delay(1000);
        
        motor_app_move_backward(40);
        HAL_Delay(1000);
        
        motor_app_turn_right(30);
        HAL_Delay(1000);
        
        motor_app_stop_all();
        HAL_Delay(2000);
    }
}
#endif
