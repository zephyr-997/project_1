/**
 * @file motor_port_test.c
 * @brief STM32F407电机端口层测试文件
 * @details 本文件提供了TB6612FNG电机驱动端口层的基础测试功能。
 *          用于验证GPIO控制、PWM输出等硬件接口的正确性。
 * @author Augment Agent
 * @date 2025-07-25
 * @version 1.0.0
 * 
 * @note 测试说明:
 *       1. 本文件仅用于开发阶段的功能验证
 *       2. 测试前请确保硬件连接正确
 *       3. 测试时请注意电机安全，避免意外转动
 */

#include "motor_port.h"
#include "../../hardware/motor_drivers/tb6612fng/tb6612fng.h"
#include <stdio.h>

/* ========================================================================== */
/*                              测试配置                                      */
/* ========================================================================== */

#define TEST_PWM_FREQUENCY      10000       /* 测试PWM频率 10kHz */
#define TEST_SPEED_LOW          30          /* 低速测试 30% */
#define TEST_SPEED_HIGH         70          /* 高速测试 70% */
#define TEST_DELAY_MS           1000        /* 测试延时 1秒 */

/* ========================================================================== */
/*                              测试函数声明                                  */
/* ========================================================================== */

static int32_t test_gpio_control(void);
static int32_t test_pwm_output(void);
static int32_t test_motor_direction(void);
static int32_t test_motor_speed(void);
static int32_t test_motor_integration(void);
static void test_delay_ms(uint32_t ms);

/* ========================================================================== */
/*                              公共测试接口                                  */
/* ========================================================================== */

/**
 * @brief 运行所有电机端口层测试
 * @return int32_t 测试结果
 * @retval 0 所有测试通过
 * @retval <0 测试失败
 */
int32_t motor_port_run_all_tests(void)
{
    int32_t result = 0;
    
    printf("=== TB6612FNG电机端口层测试开始 ===\r\n");
    
    /* 测试1: GPIO控制测试 */
    printf("测试1: GPIO控制测试...\r\n");
    result = test_gpio_control();
    if (result != 0) {
        printf("GPIO控制测试失败: %ld\r\n", result);
        return result;
    }
    printf("GPIO控制测试通过\r\n");
    
    /* 测试2: PWM输出测试 */
    printf("测试2: PWM输出测试...\r\n");
    result = test_pwm_output();
    if (result != 0) {
        printf("PWM输出测试失败: %ld\r\n", result);
        return result;
    }
    printf("PWM输出测试通过\r\n");
    
    /* 测试3: 电机方向控制测试 */
    printf("测试3: 电机方向控制测试...\r\n");
    result = test_motor_direction();
    if (result != 0) {
        printf("电机方向控制测试失败: %ld\r\n", result);
        return result;
    }
    printf("电机方向控制测试通过\r\n");
    
    /* 测试4: 电机速度控制测试 */
    printf("测试4: 电机速度控制测试...\r\n");
    result = test_motor_speed();
    if (result != 0) {
        printf("电机速度控制测试失败: %ld\r\n", result);
        return result;
    }
    printf("电机速度控制测试通过\r\n");
    
    /* 测试5: 电机集成测试 */
    printf("测试5: 电机集成测试...\r\n");
    result = test_motor_integration();
    if (result != 0) {
        printf("电机集成测试失败: %ld\r\n", result);
        return result;
    }
    printf("电机集成测试通过\r\n");
    
    printf("=== 所有测试通过! ===\r\n");
    return 0;
}

/**
 * @brief 基础功能测试
 * @return int32_t 测试结果
 */
int32_t motor_port_basic_test(void)
{
    tb6612_config_t config = {
        .pwm_frequency = TEST_PWM_FREQUENCY,
        .pwm_resolution = 10,
        .max_duty_cycle = 95,
        .min_duty_cycle = 5
    };
    
    printf("基础功能测试开始...\r\n");
    
    /* 初始化端口层 */
    if (motor_port_init(&config) != TB6612_OK) {
        printf("端口层初始化失败\r\n");
        return -1;
    }
    
    /* 测试电机A前进 */
    printf("电机A前进测试...\r\n");
    motor_port_set_direction(TB6612_MOTOR_A, TB6612_FORWARD);
    motor_port_set_speed(TB6612_MOTOR_A, TEST_SPEED_LOW);
    test_delay_ms(TEST_DELAY_MS);
    
    /* 停止电机A */
    motor_port_set_speed(TB6612_MOTOR_A, 0);
    motor_port_set_direction(TB6612_MOTOR_A, TB6612_STOP);
    test_delay_ms(500);
    
    /* 测试电机B前进 */
    printf("电机B前进测试...\r\n");
    motor_port_set_direction(TB6612_MOTOR_B, TB6612_FORWARD);
    motor_port_set_speed(TB6612_MOTOR_B, TEST_SPEED_LOW);
    test_delay_ms(TEST_DELAY_MS);
    
    /* 停止电机B */
    motor_port_set_speed(TB6612_MOTOR_B, 0);
    motor_port_set_direction(TB6612_MOTOR_B, TB6612_STOP);
    
    /* 反初始化 */
    motor_port_deinit();
    
    printf("基础功能测试完成\r\n");
    return 0;
}

/* ========================================================================== */
/*                              私有测试函数                                  */
/* ========================================================================== */

/**
 * @brief GPIO控制测试
 * @note GPIO配置由STM32CubeMX完成，此测试验证GPIO状态设置功能
 */
static int32_t test_gpio_control(void)
{
    /* 设置GPIO初始状态（配置由CubeMX完成） */
    if (gpio_port_init() != 0) {
        return -1;
    }

    /* 测试所有方向控制引脚的状态切换 */
    gpio_port_set_pin(TB6612_AIN1_PORT, TB6612_AIN1_PIN, 1);
    test_delay_ms(100);
    gpio_port_set_pin(TB6612_AIN1_PORT, TB6612_AIN1_PIN, 0);

    gpio_port_set_pin(TB6612_AIN2_PORT, TB6612_AIN2_PIN, 1);
    test_delay_ms(100);
    gpio_port_set_pin(TB6612_AIN2_PORT, TB6612_AIN2_PIN, 0);

    gpio_port_set_pin(TB6612_BIN1_PORT, TB6612_BIN1_PIN, 1);
    test_delay_ms(100);
    gpio_port_set_pin(TB6612_BIN1_PORT, TB6612_BIN1_PIN, 0);

    gpio_port_set_pin(TB6612_BIN2_PORT, TB6612_BIN2_PIN, 1);
    test_delay_ms(100);
    gpio_port_set_pin(TB6612_BIN2_PORT, TB6612_BIN2_PIN, 0);

    return 0;
}

/**
 * @brief PWM输出测试
 */
static int32_t test_pwm_output(void)
{
    /* 初始化PWM */
    if (pwm_port_init(TEST_PWM_FREQUENCY) != 0) {
        return -1;
    }
    
    /* 测试不同占空比 */
    uint16_t duty_cycles[] = {10, 30, 50, 70, 90};
    uint8_t num_tests = sizeof(duty_cycles) / sizeof(duty_cycles[0]);
    
    for (uint8_t i = 0; i < num_tests; i++) {
        /* 设置占空比 */
        pwm_port_set_duty(1, duty_cycles[i]);
        pwm_port_set_duty(2, duty_cycles[i]);
        
        /* 启动PWM */
        pwm_port_start(1);
        pwm_port_start(2);
        
        test_delay_ms(200);
        
        /* 停止PWM */
        pwm_port_stop(1);
        pwm_port_stop(2);
        
        test_delay_ms(100);
    }
    
    return 0;
}

/**
 * @brief 电机方向控制测试
 */
static int32_t test_motor_direction(void)
{
    tb6612_config_t config = {
        .pwm_frequency = TEST_PWM_FREQUENCY,
        .pwm_resolution = 10,
        .max_duty_cycle = 95,
        .min_duty_cycle = 5
    };
    
    /* 初始化端口层 */
    if (motor_port_init(&config) != TB6612_OK) {
        return -1;
    }
    
    /* 测试所有方向 */
    tb6612_direction_t directions[] = {TB6612_FORWARD, TB6612_BACKWARD, TB6612_BRAKE, TB6612_STOP};
    uint8_t num_directions = sizeof(directions) / sizeof(directions[0]);
    
    for (uint8_t motor = 0; motor < TB6612_MOTOR_MAX; motor++) {
        for (uint8_t i = 0; i < num_directions; i++) {
            if (motor_port_set_direction((tb6612_motor_t)motor, directions[i]) != TB6612_OK) {
                motor_port_deinit();
                return -2;
            }
            test_delay_ms(100);
        }
    }
    
    motor_port_deinit();
    return 0;
}

/**
 * @brief 电机速度控制测试
 */
static int32_t test_motor_speed(void)
{
    tb6612_config_t config = {
        .pwm_frequency = TEST_PWM_FREQUENCY,
        .pwm_resolution = 10,
        .max_duty_cycle = 95,
        .min_duty_cycle = 5
    };
    
    /* 初始化端口层 */
    if (motor_port_init(&config) != TB6612_OK) {
        return -1;
    }
    
    /* 测试不同速度 */
    uint16_t speeds[] = {0, 25, 50, 75, 100};
    uint8_t num_speeds = sizeof(speeds) / sizeof(speeds[0]);
    
    for (uint8_t motor = 0; motor < TB6612_MOTOR_MAX; motor++) {
        for (uint8_t i = 0; i < num_speeds; i++) {
            if (motor_port_set_speed((tb6612_motor_t)motor, speeds[i]) != TB6612_OK) {
                motor_port_deinit();
                return -2;
            }
            test_delay_ms(200);
        }
    }
    
    motor_port_deinit();
    return 0;
}

/**
 * @brief 电机集成测试
 */
static int32_t test_motor_integration(void)
{
    tb6612_config_t config = {
        .pwm_frequency = TEST_PWM_FREQUENCY,
        .pwm_resolution = 10,
        .max_duty_cycle = 95,
        .min_duty_cycle = 5
    };
    
    /* 初始化端口层 */
    if (motor_port_init(&config) != TB6612_OK) {
        return -1;
    }
    
    /* 双电机协调测试 */
    motor_port_set_direction(TB6612_MOTOR_A, TB6612_FORWARD);
    motor_port_set_direction(TB6612_MOTOR_B, TB6612_FORWARD);
    motor_port_set_speed(TB6612_MOTOR_A, TEST_SPEED_LOW);
    motor_port_set_speed(TB6612_MOTOR_B, TEST_SPEED_LOW);
    test_delay_ms(TEST_DELAY_MS);
    
    /* 停止所有电机 */
    motor_port_set_speed(TB6612_MOTOR_A, 0);
    motor_port_set_speed(TB6612_MOTOR_B, 0);
    motor_port_set_direction(TB6612_MOTOR_A, TB6612_STOP);
    motor_port_set_direction(TB6612_MOTOR_B, TB6612_STOP);
    
    motor_port_deinit();
    return 0;
}

/**
 * @brief 测试延时函数
 * @param ms 延时毫秒数
 */
static void test_delay_ms(uint32_t ms)
{
    /* 使用HAL库延时函数 */
    HAL_Delay(ms);
}
