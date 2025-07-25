# WIT传感器端口层模版

## 概述

本目录包含WIT传感器驱动的端口层模版文件，提供了平台无关的接口定义和实现框架。各MCU平台可以基于这些模版文件快速实现具体的硬件适配。

## 文件说明

| 文件名 | 说明 |
|--------|------|
| `wit_port.h` | 端口层接口定义头文件，包含所有端口层函数声明 |
| `i2c_port.c` | I2C端口层实现模版，包含I2C读写函数框架 |
| `uart_port.c` | UART端口层实现模版，包含串口输出函数框架 |
| `delay_port.c` | 延时端口层实现模版，包含毫秒和微秒延时函数框架 |
| `README.md` | 本说明文档 |

## 接口说明

### I2C接口

```c
int32_t wit_port_i2c_init(void);
int32_t wit_port_i2c_write(uint8_t ucAddr, uint8_t ucReg, uint8_t *p_ucVal, uint32_t uiLen);
int32_t wit_port_i2c_read(uint8_t ucAddr, uint8_t ucReg, uint8_t *p_ucVal, uint32_t uiLen);
```

- `wit_port_i2c_init()`: 初始化I2C外设
- `wit_port_i2c_write()`: 写I2C寄存器
- `wit_port_i2c_read()`: 读I2C寄存器

### UART接口

```c
int32_t wit_port_uart_init(uint32_t uiBaud);
void wit_port_uart_write(uint8_t *p_ucData, uint32_t uiLen);
```

- `wit_port_uart_init()`: 初始化UART外设
- `wit_port_uart_write()`: 发送UART数据

### 延时接口

```c
int32_t wit_port_delay_init(void);
void wit_port_delay_ms(uint16_t ucMs);
void wit_port_delay_us(uint16_t ucUs);
```

- `wit_port_delay_init()`: 初始化延时功能
- `wit_port_delay_ms()`: 毫秒级延时
- `wit_port_delay_us()`: 微秒级延时

## 移植步骤

### 1. 复制模版文件

将`ports/template/`目录下的所有文件复制到目标MCU的端口层目录，例如：

```bash
cp -r ports/template/* ports/stm32f103/
```

### 2. 实现I2C接口

在`i2c_port.c`中实现具体的I2C操作：

- **硬件I2C**: 调用HAL/LL库函数
- **软件I2C**: 实现GPIO位操作时序

### 3. 实现UART接口

在`uart_port.c`中实现具体的UART操作：

- 配置UART参数（波特率、数据位等）
- 实现数据发送功能
- 可选：实现printf重定向

### 4. 实现延时接口

在`delay_port.c`中实现具体的延时功能：

- **毫秒延时**: 基于SysTick或RTOS延时函数
- **微秒延时**: 基于DWT计数器或校准循环

### 5. 编译配置

在编译系统中添加端口层文件：

```makefile
# Makefile示例
SOURCES += ports/$(MCU)/i2c_port.c
SOURCES += ports/$(MCU)/uart_port.c
SOURCES += ports/$(MCU)/delay_port.c

INCLUDES += -Iports/template
```

## 实现要点

### I2C实现要点

1. **时序正确性**: 确保START、STOP、ACK/NACK时序正确
2. **错误处理**: 处理总线忙、设备无响应等错误情况
3. **超时机制**: 避免死等，设置合理的超时时间
4. **地址格式**: 注意7位地址格式，不包含读写位

### UART实现要点

1. **参数配置**: 正确设置波特率、数据位、停止位、校验位
2. **发送完成**: 确保数据完全发送完成再返回
3. **缓冲管理**: 合理管理发送缓冲区
4. **重定向支持**: 可选实现printf重定向功能

### 延时实现要点

1. **精度要求**: 毫秒延时精度±1ms，微秒延时精度±10us
2. **系统兼容**: 考虑RTOS环境下的任务调度
3. **功耗优化**: 可在延时期间进入低功耗模式
4. **编译器优化**: 注意编译器优化对延时精度的影响

## 测试建议

### 单元测试

1. **I2C测试**: 使用示波器验证I2C时序
2. **UART测试**: 验证串口输出内容和格式
3. **延时测试**: 使用定时器测量延时精度

### 集成测试

1. **传感器通信**: 验证与WIT传感器的通信
2. **数据解析**: 验证数据读取和解析正确性
3. **长时间运行**: 验证系统稳定性

## 常见问题

### Q: I2C通信失败怎么办？

A: 检查以下几点：
- GPIO配置是否正确（开漏输出、上拉电阻）
- I2C时钟频率是否合适（通常100kHz或400kHz）
- 设备地址是否正确
- 时序是否符合I2C标准

### Q: UART输出乱码怎么办？

A: 检查以下几点：
- 波特率设置是否正确
- 数据位、停止位、校验位配置是否匹配
- GPIO复用功能是否正确配置
- 时钟源是否稳定

### Q: 延时不准确怎么办？

A: 检查以下几点：
- SysTick配置是否正确
- 系统时钟频率是否准确
- 编译器优化级别是否影响延时精度
- 是否受到中断影响

## 参考资料

- [WIT传感器官方文档](http://wit-motion.cn/)
- [STM32 HAL库参考手册](https://www.st.com/resource/en/user_manual/dm00105879.pdf)
- [I2C总线规范](https://www.nxp.com/docs/en/user-guide/UM10204.pdf)
- [项目API规则文档](../../rules/api-rules.mdc)
