# STM32F407ZGT6 应用层 (APP Layer)

## 概述

本目录包含STM32F407ZGT6平台的应用层实现，遵循分层架构设计原则。应用层仅依赖硬件驱动层和端口层的API接口，不包含任何MCU特定代码，实现了完整的软件层级分离。**专门针对Keil5开发环境优化，便于移植和集成。**

## 文件结构

```
app/
├── jy61p_app.c              # JY61P陀螺仪传感器应用实现
├── jy61p_app.h              # JY61P陀螺仪传感器应用接口
├── motor_control_app.c      # TB6612FNG电机控制应用实现
├── motor_control_app.h      # TB6612FNG电机控制应用接口
├── motor_control_example.c  # 电机控制使用示例代码
└── README.md                # 本说明文档（包含完整使用指南）
```

## 应用模块说明

### 1. JY61P陀螺仪传感器应用
- **文件**: `jy61p_app.c/h`
- **功能**: JY61P陀螺仪传感器数据采集和处理
- **状态**: ✅ 已完成
- **特性**: 自动扫描、实时数据读取、串口命令控制

### 2. TB6612FNG电机控制应用 (新增)
- **文件**: `motor_control_app.c/h`
- **功能**: 2轮驱动电机控制应用
- **状态**: ✅ 已完成
- **特性**: 基础运动控制、简化接口、状态管理

## 主要特性

### 1. Keil5友好设计
- ✅ 模块化源文件，便于添加到Keil项目
- ✅ 清晰的头文件接口，包含所有必要声明
- ✅ 不依赖复杂的构建系统，直接编译即可

### 2. 平台无关设计
- ✅ 不包含任何MCU头文件（如`stm32xxx.h`）
- ✅ 仅依赖硬件驱动层和端口层接口
- ✅ 支持STM32F103、STM32F407等多种MCU

### 3. 功能完整性
- ✅ JY61P传感器：自动扫描、实时数据读取、串口命令控制
- ✅ TB6612FNG电机：2轮驱动控制、运动控制、状态管理
- ✅ 友好的用户界面和帮助信息
- ✅ 完善的错误处理和状态反馈

### 4. 接口简洁化
- ✅ 每个模块对外暴露核心API函数
- ✅ 隐藏内部实现细节
- ✅ 支持数据查询和状态检查
- ✅ 遵循"基本功能优先、代码简洁"原则

## API接口说明

### JY61P陀螺仪传感器接口

#### 数据获取
```c
#include "jy61p_app.h"

// 获取JY61P传感器数据
jy61p_data_t data;
if (jy61p_get_sensor_data(&data) == 0) {
    printf("加速度: %.3f, %.3f, %.3f g\n", data.acc[0], data.acc[1], data.acc[2]);
}

// 检查JY61P传感器连接状态
if (jy61p_is_sensor_connected()) {
    printf("JY61P已连接，地址: 0x%02X\n", jy61p_get_sensor_address());
}
```

#### 命令处理
```c
// 在串口中断中调用
void UART_IRQHandler(void) {
    if (UART_GetITStatus(UART1, UART_IT_RXNE) != RESET) {
        uint8_t data = UART_ReceiveData(UART1);
        jy61p_cmd_data_received(data);  // 处理接收到的JY61P命令
    }
}
```

### TB6612FNG电机控制接口

#### 初始化和管理
```c
#include "motor_control_app.h"

// 初始化电机控制应用
if (motor_app_init() != 0) {
    printf("电机应用初始化失败!\n");
    return -1;
}

// 检查初始化状态
if (motor_app_is_initialized()) {
    printf("电机应用已初始化\n");
}

// 获取电机状态
motor_app_status_t status;
if (motor_app_get_status(&status) == 0) {
    printf("电机A: 速度=%d, 方向=%d\n",
           status.current_speed_a, status.current_dir_a);
}
```

#### 2轮驱动运动控制
```c
// 基础运动控制
motor_app_move_forward(50);    // 前进，速度50%
motor_app_move_backward(40);   // 后退，速度40%
motor_app_turn_left(30);       // 左转，速度30%
motor_app_turn_right(30);      // 右转，速度30%
motor_app_stop_all();          // 停止所有电机

// 统一控制接口
motor_control_t control;
control.left_speed = 60;       // 左轮前进60%
control.right_speed = -40;     // 右轮后退40%
motor_app_control_motors(&control);
```

## 功能特性详细说明

### JY61P陀螺仪传感器功能

#### 支持的命令
| 命令 | 功能 | 说明 |
|------|------|------|
| `a\r\n` | 加速度计校准 | 开始加速度计校准过程 |
| `m\r\n` | 磁力计校准开始 | 开始磁力计校准，需要转动传感器 |
| `e\r\n` | 磁力计校准结束 | 结束磁力计校准过程 |
| `u\r\n` | 设置低带宽 | 设置输出带宽为5Hz |
| `U\r\n` | 设置高带宽 | 设置输出带宽为256Hz |
| `b\r\n` | 低波特率 | 设置JY61P串口为9600bps |
| `B\r\n` | 高波特率 | 设置JY61P串口为115200bps |
| `h\r\n` | 帮助信息 | 显示命令帮助和数据格式说明 |

#### 数据格式
应用层自动将传感器原始数据转换为标准物理单位：
```
ACC : X Y Z (g)      - 加速度，单位：重力加速度
GYRO: X Y Z (°/s)    - 角速度，单位：度每秒
ANGLE: X Y Z (°)     - 欧拉角，单位：度
MAG : X Y Z (raw)    - 磁场原始值
```

### TB6612FNG电机控制功能

#### 基础功能
- ✅ 双电机独立控制
- ✅ 2轮驱动运动控制（前进/后退/左转/右转/停止）
- ✅ 简化的电机控制接口
- ✅ 应用状态管理
- ✅ 基础功能测试

#### 设计特点
- **简洁设计**: 每个函数不超过30行，接口简单易用
- **错误处理**: 完善的参数检查和错误返回机制
- **状态管理**: 实时跟踪电机运行状态
- **扩展性**: 预留PID控制接口扩展点

## 使用示例

### JY61P传感器使用示例

#### 基本集成（推荐）
```c
#include "jy61p_app.h"

int main(void) {
    // 系统初始化（时钟、GPIO等）
    SystemInit();

    // JY61P应用层会自动初始化所有必要组件
    return main();  // 调用JY61P应用层主函数
}
```

#### 自定义集成（高级用法）
```c
#include "jy61p_app.h"

int main(void) {
    // 系统初始化
    SystemInit();

    // 在你的主循环中
    while(1) {
        jy61p_data_t data;
        if (jy61p_get_sensor_data(&data) == 0) {
            // 处理JY61P传感器数据
            process_jy61p_data(&data);
        }
        HAL_Delay(100);
    }
}

// 在串口中断中调用
void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        uint8_t byte = USART_ReceiveData(USART1);
        jy61p_cmd_data_received(byte);
    }
}
```

### TB6612FNG电机控制使用示例

#### 基础使用示例
```c
#include "motor_control_app.h"

int main(void)
{
    /* 系统初始化 */
    HAL_Init();
    SystemClock_Config();

    /* 初始化电机应用 */
    if (motor_app_init() != 0) {
        printf("电机应用初始化失败!\n");
        return -1;
    }

    /* 小车前进2秒 */
    motor_app_move_forward(50);
    HAL_Delay(2000);

    /* 小车左转1秒 */
    motor_app_turn_left(30);
    HAL_Delay(1000);

    /* 停止所有电机 */
    motor_app_stop_all();

    return 0;
}
```

#### 统一控制接口示例
```c
void demo_unified_control(void)
{
    motor_control_t control;

    /* 小车前进 */
    control.left_speed = 60;
    control.right_speed = 60;
    motor_app_control_motors(&control);
    HAL_Delay(2000);

    /* 小车原地左转 */
    control.left_speed = -40;
    control.right_speed = 40;
    motor_app_control_motors(&control);
    HAL_Delay(1000);

    /* 停止 */
    control.left_speed = 0;
    control.right_speed = 0;
    motor_app_control_motors(&control);
}
```

## Keil5集成方法

### 1. 添加文件到Keil项目

**步骤1：创建文件组**
在Project窗口中右键"Target 1"，选择"Manage Project Items"：
1. 添加组"APP"
2. 添加组"Drivers"
3. 添加组"Ports"
4. 添加组"Hardware"

**步骤2：添加源文件**
- **APP组**: `app/jy61p_app.c`, `app/motor_control_app.c`
- **Drivers组**: `drivers/wit_c_sdk/wit_c_sdk.c`
- **Ports组**: `ports/stm32f407/*.c` (根据MCU选择)
- **Hardware组**: `hardware/motor_drivers/tb6612fng/tb6612fng.c`

**步骤3：添加头文件路径**
在"Options for Target" -> "C/C++" -> "Include Paths"中添加：
- `app`
- `drivers/wit_c_sdk`
- `ports/stm32f407`
- `hardware/motor_drivers/tb6612fng`

### 2. 编译配置

**预处理器定义**:
```
STM32F407xx
USE_HAL_DRIVER
```

**优化设置**:
- Optimization: Level 1 (-O1)
- 勾选"One ELF Section per Function"

## 依赖关系

```
应用层依赖关系:
├── app/jy61p_app.c
│   ├── drivers/wit_c_sdk/wit_c_sdk.h
│   ├── drivers/wit_c_sdk/REG.h
│   └── ports/stm32f407/wit_port.h
└── app/motor_control_app.c
    ├── hardware/motor_drivers/tb6612fng/tb6612fng.h
    └── ports/stm32f407/motor_port.h (通过tb6612fng.c调用)
```

## 硬件连接说明

### JY61P传感器连接（STM32F407ZGT6）
```
JY61P引脚    STM32F407引脚    功能
VCC         3.3V           电源正极
GND         GND            电源负极
SDA         PB9 (I2C1_SDA) I2C数据线
SCL         PB8 (I2C1_SCL) I2C时钟线
```

### TB6612FNG电机驱动连接
```
TB6612FNG引脚  STM32F407引脚    功能
VM            电机电源(6-15V)  电机电源输入
VCC           3.3V           逻辑电源输入
GND           GND            电源地
STBY          3.3V           待机控制(常高)
PWMA          PE9 (TIM1_CH1) 电机A PWM控制
AIN1          PC4            电机A方向控制1
AIN2          PC5            电机A方向控制2
PWMB          PE11(TIM1_CH2) 电机B PWM控制
BIN1          PB0            电机B方向控制1
BIN2          PB1            电机B方向控制2
AO1/AO2       电机A正负极     电机A输出
BO1/BO2       电机B正负极     电机B输出
```

### 串口连接（用于数据输出和调试）
```
USB-TTL     STM32F407引脚    功能
VCC         3.3V           电源（可选）
GND         GND            地线
RX          PA9 (USART1_TX) 接收MCU发送的数据
TX          PA10(USART1_RX) 发送命令到MCU（可选）
```

## 测试验证

### 1. 编译测试
- [ ] Keil5项目编译无错误无警告
- [ ] 所有依赖文件正确包含
- [ ] 头文件路径配置正确
- [ ] 预处理器定义正确

### 2. JY61P传感器功能测试
- [ ] JY61P传感器自动扫描成功
- [ ] 数据实时输出正常（通过串口查看）
- [ ] 所有命令响应正确
- [ ] 校准功能工作正常
- [ ] `jy61p_get_sensor_data()` 返回有效数据
- [ ] `jy61p_is_sensor_connected()` 状态正确

### 3. TB6612FNG电机控制功能测试
- [ ] 电机应用初始化成功
- [ ] 基础运动控制功能正常（前进/后退/左转/右转/停止）
- [ ] 统一控制接口工作正确
- [ ] 电机状态监控准确
- [ ] `motor_app_test_basic_control()` 测试通过
- [ ] `motor_app_test_2wheel_motion()` 测试通过

### 4. 硬件测试
- [ ] 在实际STM32F407ZGT6硬件上运行正常
- [ ] I2C通信稳定，JY61P数据读取正常
- [ ] PWM输出正确，电机转向和速度控制准确
- [ ] 串口输出正确，调试信息清晰

## 故障排除

### 常见编译问题

1. **找不到头文件**
   ```
   错误：cannot open source input file "wit_c_sdk.h" 或 "tb6612fng.h"
   解决：检查Include Paths是否正确添加了相应路径
   ```

2. **未定义的函数**
   ```
   错误：undefined symbol wit_port_i2c_init 或 motor_port_init
   解决：确保端口层源文件已添加到项目中
   ```

3. **重复定义main函数**
   ```
   错误：multiple definition of 'main'
   解决：如果使用自定义main，不要同时包含应用层中的main函数
   ```

### 常见运行问题

#### JY61P传感器相关
1. **JY61P扫描失败**
   - 检查I2C连接和上拉电阻（通常4.7kΩ）
   - 确认JY61P供电正常（3.3V或5V）
   - 验证端口层I2C实现正确

2. **数据输出异常**
   - 检查串口配置和波特率（默认115200）
   - 确认端口层UART实现正确
   - 验证printf重定向到串口

#### TB6612FNG电机控制相关
1. **电机不转动**
   - 检查TB6612FNG供电（VM电源6-15V，VCC逻辑电源3.3V）
   - 确认STBY引脚连接到3.3V（使能芯片）
   - 验证PWM输出和GPIO控制引脚连接

2. **电机转向错误**
   - 检查AIN1/AIN2和BIN1/BIN2引脚连接
   - 确认电机接线正确（AO1/AO2, BO1/BO2）
   - 验证方向控制逻辑

3. **电机速度控制异常**
   - 检查PWM频率设置（建议10kHz）
   - 确认TIM1定时器配置正确
   - 验证占空比计算逻辑

### 调试方法

1. **使用Keil调试器**：设置断点跟踪函数调用
2. **串口输出调试**：添加printf语句查看程序流程
3. **逻辑分析仪**：检查I2C时序和PWM波形
4. **示波器**：测量PWM输出波形和电机驱动信号

## 快速开始指南

### 5分钟快速集成
1. **创建Keil5项目**（选择STM32F407ZGT6芯片）
2. **添加文件**：将应用层源文件添加到项目
3. **配置路径**：添加头文件包含路径
4. **连接硬件**：按照硬件连接说明连接传感器和电机驱动
5. **编译下载**：编译项目并下载到MCU
6. **查看输出**：通过串口助手查看数据输出和电机控制效果

### 预期输出示例

#### JY61P传感器输出
```
************************ JY61P Gyroscope Application ***********************
*                     Platform Independent Version                        *
**************************************************************************
JY61P application initialized successfully.
Scanning I2C bus for JY61P sensors...
Found JY61P at I2C address: 0x50
JY61P initialized successfully at address 0x50

ACC : 0.123 -0.456 0.987 (g)
GYRO: 1.234 -2.345 0.123 (°/s)
ANGLE: 12.34 -23.45 1.23 (°)
MAG : 123 -456 789 (raw)
```

#### TB6612FNG电机控制输出
```
=== TB6612FNG电机控制应用示例 ===
系统初始化完成
电机应用初始化成功
小车前进...
小车左转...
停止所有电机
基础控制示例完成
```

## 扩展规划

### 预留扩展点
- **JY61P传感器**: 多传感器支持、数据融合算法
- **TB6612FNG电机**: PID控制器接口、速度闭环控制、轨迹跟踪

### 后续版本计划
- **v1.1**: 添加PID控制接口和传感器数据融合
- **v1.2**: 添加速度闭环控制和轨迹跟踪功能
- **v1.3**: 添加多传感器支持和高级控制算法

## 版本历史

- **v1.0** (2025-07-25): 初始版本，包含JY61P传感器和TB6612FNG电机控制应用

## 技术支持

如有问题请参考：
- **项目架构文档**: `docs/项目架构.md`
- **开发任务文档**: `docs/开发任务.md`
- **TB6612FNG驱动文档**: `hardware/motor_drivers/tb6612fng/README.md`
- **端口层文档**: `ports/stm32f407/README.md`
