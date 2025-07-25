# STM32F407 WIT传感器端口层实现

## 概述

本目录包含基于STM32F407微控制器和HAL库的WIT传感器端口层实现。**配合STM32CubeMX生成的初始化代码使用**，支持硬件I2C通信、UART输出、精确延时等功能。

## 文件说明

| 文件名 | 说明 |
|--------|------|
| `wit_port.h` | 端口层接口定义头文件 |
| `stm32f407_port_config.h` | STM32F407端口层配置文件 |
| `i2c_port.c` | I2C端口层实现(基于HAL_I2C) |
| `uart_port.c` | UART端口层实现(基于HAL_UART) |
| `delay_port.c` | 延时端口层实现(基于HAL_Delay和DWT) |
| `example_usage.c` | 使用示例代码 |
| `README.md` | 本说明文档 |

## 与STM32CubeMX集成

**重要**: 本端口层实现依赖STM32CubeMX生成的硬件初始化代码，不包含硬件初始化功能。

## CubeMX配置要求

### 必需的外设配置

#### I2C配置 (推荐I2C1)
- **模式**: I2C
- **速率**: 100kHz (标准模式)
- **地址长度**: 7位
- **推荐引脚**: PB8(SCL), PB9(SDA)

#### UART配置 (推荐USART1)
- **模式**: 异步通信
- **波特率**: 115200 (可配置)
- **数据位**: 8位
- **停止位**: 1位
- **校验**: 无
- **推荐引脚**: PA9(TX), PA10(RX)

#### 系统配置
- **系统时钟**: 建议168MHz
- **SysTick**: 1kHz (HAL库默认)

### CubeMX配置步骤

1. **创建新工程**: 选择STM32F407ZGT6
2. **配置时钟**: 设置系统时钟为168MHz
3. **配置I2C**: 启用I2C1，设置为100kHz
4. **配置UART**: 启用USART1，设置为115200bps
5. **生成代码**: 选择HAL库，生成Keil/GCC项目

## 功能特性

### I2C功能
- ✅ 硬件I2C支持
- ✅ 自动重试机制
- ✅ 超时保护
- ✅ 错误处理

### UART功能
- ✅ 串口数据输出
- ✅ printf重定向支持
- ✅ 多编译器兼容(GCC/Keil/IAR)
- ✅ 超时保护

### 延时功能
- ✅ 毫秒级延时(基于HAL_Delay)
- ✅ 微秒级延时(基于DWT计数器)
- ✅ 高精度时序控制

## 使用方法

### 1. 硬件连接

```
STM32F407    WIT传感器
I2C_SCL  --> SCL
I2C_SDA  --> SDA
GND      --> GND
3.3V     --> VCC
```

### 2. 软件集成

#### 步骤1: 添加端口层文件到工程
将以下文件添加到您的CubeMX生成的工程中：
- `ports/stm32f407/i2c_port.c`
- `ports/stm32f407/uart_port.c`
- `ports/stm32f407/delay_port.c`

#### 步骤2: 添加包含路径
在工程设置中添加包含路径：`ports/stm32f407`

#### 步骤3: 包含头文件
```c
#include "wit_port.h"
```

#### 步骤4: 初始化端口层
```c
// 在main函数中，CubeMX初始化代码之后
int main(void)
{
    /* CubeMX生成的初始化代码 */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();    // 确保I2C已初始化
    MX_USART1_UART_Init(); // 确保UART已初始化

    /* 初始化WIT传感器端口层 */
    wit_port_delay_init();
    wit_port_uart_init(115200);
    wit_port_i2c_init();

    /* 您的应用代码 */
    while (1) {
        // ...
    }
}
```

#### 使用I2C通信
```c
uint8_t sensor_addr = 0x50;  // 传感器地址
uint8_t reg_addr = 0x34;     // 寄存器地址
uint8_t data[6];             // 数据缓冲区

// 读取传感器数据
if (wit_port_i2c_read(sensor_addr, reg_addr, data, 6)) {
    // 处理读取的数据
} else {
    // 处理读取错误
}
```

#### 使用UART输出
```c
// 直接输出数据
char message[] = "Hello, WIT Sensor!\r\n";
wit_port_uart_write((uint8_t*)message, strlen(message));

// 使用printf(需要重定向支持)
printf("Sensor data: %d, %d, %d\r\n", acc_x, acc_y, acc_z);
```

#### 使用延时功能
```c
// 毫秒延时
wit_port_delay_ms(100);  // 延时100ms

// 微秒延时
wit_port_delay_us(50);   // 延时50us
```

### 3. 编译配置

#### Keil MDK
```c
// 在工程中添加以下文件：
// - ports/stm32f407/*.c
// - STM32F4xx HAL库文件

// 包含路径：
// - ports/stm32f407
// - STM32F4xx HAL库头文件路径
```

#### GCC/Makefile
```makefile
# 源文件
SOURCES += ports/stm32f407/i2c_port.c
SOURCES += ports/stm32f407/uart_port.c
SOURCES += ports/stm32f407/delay_port.c
SOURCES += ports/stm32f407/stm32f407_port_init.c

# 包含路径
INCLUDES += -Iports/stm32f407
INCLUDES += -IDrivers/STM32F4xx_HAL_Driver/Inc

# 链接HAL库
LDFLAGS += -lhal_f4
```

## 配置选项

### 检查外设句柄名称
确保 `stm32f407_port_config.h` 中的句柄名称与您的CubeMX配置一致：

```c
/* 如果您的CubeMX生成的句柄名称不同，请修改这些声明 */
extern I2C_HandleTypeDef hi2c1;    // 改为您的I2C句柄名称
extern UART_HandleTypeDef huart1;  // 改为您的UART句柄名称
```

### 修改超时配置
编辑 `stm32f407_port_config.h` 文件：

```c
// 修改I2C超时时间
#define WIT_I2C_TIMEOUT             2000UL      // 改为2秒

// 修改UART超时时间
#define WIT_UART_TIMEOUT            2000UL      // 改为2秒

// 禁用DWT延时
#define WIT_USE_DWT_DELAY           0           // 使用循环延时
```

### 系统时钟配置
如果您的系统时钟不是168MHz，请修改：

```c
// 修改系统时钟频率
#define SYSTEM_CLOCK_FREQ           84000000UL  // 改为84MHz
```

## 性能参数

| 功能 | 性能指标 |
|------|----------|
| I2C速率 | 100kHz (标准模式) |
| UART波特率 | 115200 bps (可配置) |
| 毫秒延时精度 | ±1ms |
| 微秒延时精度 | ±1us (DWT模式) |
| I2C超时 | 1000ms |
| UART超时 | 1000ms |

## 故障排除

### I2C通信失败
1. 检查硬件连接和上拉电阻
2. 确认设备地址正确
3. 检查I2C时钟频率设置
4. 使用示波器验证时序

### UART输出异常
1. 检查波特率设置
2. 确认GPIO复用功能配置
3. 检查printf重定向实现

### 延时不准确
1. 确认系统时钟配置
2. 检查DWT是否正确初始化
3. 考虑中断对延时的影响

## 兼容性

- **MCU**: STM32F407xx系列
- **HAL库**: STM32F4xx HAL Driver V1.7.0+
- **编译器**: GCC, Keil MDK, IAR EWARM
- **RTOS**: 支持FreeRTOS等实时操作系统

## 注意事项

1. **时钟配置**: 确保系统时钟正确配置为168MHz
2. **中断优先级**: 合理设置SysTick和其他中断优先级
3. **功耗优化**: 可在延时期间进入低功耗模式
4. **线程安全**: 多任务环境下注意资源保护

## 集成检查清单

在使用端口层之前，请确认以下项目：

### CubeMX配置检查
- [ ] I2C外设已启用并正确配置
- [ ] UART外设已启用并正确配置
- [ ] 系统时钟已正确配置
- [ ] HAL库已选择并生成代码

### 代码集成检查
- [ ] 端口层源文件已添加到工程
- [ ] 包含路径已正确设置
- [ ] 外设句柄名称与CubeMX生成的一致
- [ ] 端口层初始化函数已调用

### 功能测试
- [ ] I2C通信正常
- [ ] UART输出正常
- [ ] printf重定向工作(如果需要)
- [ ] 延时功能准确

## 常见问题

### Q: 编译时提示找不到hi2c1或huart1？
A: 请检查：
1. CubeMX是否已生成I2C和UART初始化代码
2. `stm32f407_port_config.h`中的句柄名称是否与CubeMX生成的一致

### Q: I2C通信失败？
A: 请检查：
1. CubeMX中I2C配置是否正确
2. 硬件连接是否正确
3. 上拉电阻是否已连接

### Q: UART无输出？
A: 请检查：
1. CubeMX中UART配置是否正确
2. 波特率设置是否匹配
3. printf重定向是否正确实现

## 更新日志

- **v1.1.0** (2025-07-25): 优化为配合CubeMX使用，移除硬件初始化代码
- **v1.0.0** (2025-07-25): 初始版本，支持基本I2C/UART/延时功能
