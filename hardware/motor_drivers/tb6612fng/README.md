# TB6612FNG双路电机驱动芯片驱动库 (2轮驱动专用版)

## 概述

TB6612FNG是一款高性能双路电机驱动芯片，支持PWM速度控制和方向控制。本驱动库专为2轮驱动小车设计，提供了简洁高效的电机控制接口，特别适合PID控制集成。

## 硬件特性

### TB6612FNG芯片规格
- **工作电压**: VM: 2.7V-10.8V, VCC: 2.7V-5.5V
- **输出电流**: 连续1.2A，峰值3.2A (每通道)
- **PWM频率**: 支持1kHz-20kHz (可配置)
- **控制方式**: PWM + 方向控制
- **适用场景**: 2轮驱动小车，PID控制系统

### 引脚说明
```
TB6612FNG引脚定义:
├── 电源引脚
│   ├── VM    - 电机电源输入 (2.7V-10.8V)
│   ├── VCC   - 逻辑电源输入 (2.7V-5.5V)
│   └── GND   - 电源地
├── 控制引脚
│   ├── PWMA  - 电机A PWM控制输入
│   ├── AIN1  - 电机A方向控制1
│   ├── AIN2  - 电机A方向控制2
│   ├── PWMB  - 电机B PWM控制输入
│   ├── BIN1  - 电机B方向控制1
│   ├── BIN2  - 电机B方向控制2
│   └── STBY  - 待机控制 (低电平待机)
└── 输出引脚
    ├── AO1   - 电机A输出1
    ├── AO2   - 电机A输出2
    ├── BO1   - 电机B输出1
    └── BO2   - 电机B输出2
```

## 软件架构

### 分层设计
```
应用层 (App)
    ↓
TB6612FNG驱动层 (Hardware)
    ↓
端口适配层 (Ports)
    ↓
HAL库 (Drivers)
```

### 文件结构
```
hardware/motor_drivers/tb6612fng/
├── tb6612fng.h         # 驱动库头文件
├── tb6612fng.c         # 驱动库实现
└── README.md           # 本说明文档
```

## API接口 (简化版)

### 初始化和配置

#### tb6612_init()
```c
tb6612_error_t tb6612_init(const tb6612_config_t *config);
```
初始化TB6612FNG驱动，配置PWM频率等基本参数。

#### tb6612_deinit()
```c
tb6612_error_t tb6612_deinit(void);
```
反初始化驱动，释放硬件资源。

### 基础电机控制

#### tb6612_set_direction()
```c
tb6612_error_t tb6612_set_direction(tb6612_motor_t motor, tb6612_direction_t direction);
```
设置电机转向：
- `TB6612_STOP`: 停止
- `TB6612_FORWARD`: 前进
- `TB6612_BACKWARD`: 后退

#### tb6612_set_speed()
```c
tb6612_error_t tb6612_set_speed(tb6612_motor_t motor, uint16_t speed_percent);
```
设置电机速度，范围0-100%。

#### tb6612_set_motor_pair()
```c
tb6612_error_t tb6612_set_motor_pair(uint16_t speed_a, tb6612_direction_t dir_a,
                                     uint16_t speed_b, tb6612_direction_t dir_b);
```
同时控制两个电机，适合PID控制调用。

### 2轮驱动专用函数

#### tb6612_move_forward()
```c
tb6612_error_t tb6612_move_forward(uint16_t speed);
```
小车前进，两轮同速前转。

#### tb6612_move_backward()
```c
tb6612_error_t tb6612_move_backward(uint16_t speed);
```
小车后退，两轮同速后转。

#### tb6612_turn_left()
```c
tb6612_error_t tb6612_turn_left(uint16_t speed);
```
小车左转，右轮前转，左轮停止。

#### tb6612_turn_right()
```c
tb6612_error_t tb6612_turn_right(uint16_t speed);
```
小车右转，左轮前转，右轮停止。

### 状态查询

#### tb6612_is_initialized()
```c
bool tb6612_is_initialized(void);
```
检查驱动是否已初始化。

## 使用示例

### 2轮驱动小车控制
```c
#include "tb6612fng.h"

int main(void)
{
    // 使用默认配置初始化
    if (tb6612_init(NULL) != TB6612_OK) {
        printf("TB6612FNG初始化失败!\n");
        return -1;
    }

    // 小车前进，速度50%
    tb6612_move_forward(50);
    HAL_Delay(2000);

    // 小车左转，速度30%
    tb6612_turn_left(30);
    HAL_Delay(1000);

    // 小车后退，速度40%
    tb6612_move_backward(40);
    HAL_Delay(2000);

    // 停止所有电机
    tb6612_stop_all();

    return 0;
}
```

### PID控制集成示例
```c
// PID控制器输出直接调用电机控制
void pid_motor_control(float left_output, float right_output)
{
    uint16_t left_speed = (uint16_t)fabs(left_output);
    uint16_t right_speed = (uint16_t)fabs(right_output);

    tb6612_direction_t left_dir = (left_output >= 0) ? TB6612_FORWARD : TB6612_BACKWARD;
    tb6612_direction_t right_dir = (right_output >= 0) ? TB6612_FORWARD : TB6612_BACKWARD;

    tb6612_set_motor_pair(left_speed, left_dir, right_speed, right_dir);
}
```

### 自定义配置
```c
tb6612_config_t custom_config = {
    .pwm_frequency = 15000,              // 15kHz PWM频率
    .pwm_resolution = 12,                // 12位PWM分辨率
    .max_duty_cycle = 90,                // 最大占空比90%
    .min_duty_cycle = 10                 // 最小占空比10%
};

tb6612_init(&custom_config);
```

## 错误处理

### 错误码说明
- `TB6612_OK`: 操作成功
- `TB6612_ERROR_INVALID_PARAM`: 无效参数
- `TB6612_ERROR_NOT_INITIALIZED`: 未初始化
- `TB6612_ERROR_HARDWARE_FAULT`: 硬件故障

### 错误处理示例
```c
tb6612_error_t ret = tb6612_set_speed(TB6612_MOTOR_A, 75);
if (ret != TB6612_OK) {
    switch (ret) {
        case TB6612_ERROR_INVALID_PARAM:
            printf("参数错误\n");
            break;
        case TB6612_ERROR_NOT_INITIALIZED:
            printf("驱动未初始化\n");
            break;
        case TB6612_ERROR_HARDWARE_FAULT:
            printf("硬件故障\n");
            tb6612_stop(TB6612_MOTOR_A);
            break;
        default:
            printf("未知错误: %d\n", ret);
            break;
    }
}
```

## 移植说明

### 端口层接口 (简化版)
驱动库通过以下端口层接口与硬件交互，需要在具体平台实现：

```c
// 端口层初始化
tb6612_error_t motor_port_init(const tb6612_config_t *config);
tb6612_error_t motor_port_deinit(void);

// 电机控制接口
tb6612_error_t motor_port_set_direction(tb6612_motor_t motor, tb6612_direction_t direction);
tb6612_error_t motor_port_set_speed(tb6612_motor_t motor, uint16_t speed_percent);
```

### STM32F407移植
STM32F407平台的端口层实现位于 `ports/stm32f407/motor_port.c`，包括：
- GPIO控制引脚配置 (PC4, PC5, PB0, PB1)
- PWM定时器配置 (TIM1_CH1, TIM1_CH2)

## 注意事项

1. **电源设计**: 确保VM电源能提供足够的电流，建议使用去耦电容
2. **散热设计**: 大电流应用时注意TB6612FNG的散热
3. **PWM频率**: 建议使用10kHz-15kHz，平衡性能和效率
4. **2轮驱动**: 专为2轮+万向轮小车设计，左右轮独立控制
5. **PID集成**: API设计便于PID控制器直接调用

## 版本历史

- **v1.0.0** (2025-07-25): 2轮驱动专用简化版本，支持PID控制集成

## 技术支持

如有问题或建议，请参考项目文档或联系开发团队。
