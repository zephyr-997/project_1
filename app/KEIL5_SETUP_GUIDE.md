# JY61P Keil5项目配置指南

## 概述

本指南详细说明如何在Keil5 MDK-ARM开发环境中配置和使用JY61P陀螺仪传感器应用层代码。

## 前提条件

- ✅ Keil5 MDK-ARM已安装
- ✅ 对应MCU的Device Family Pack已安装
- ✅ STM32CubeMX已配置好基础工程（可选）
- ✅ JY61P传感器硬件准备就绪

## 步骤1：创建Keil5项目

### 1.1 新建项目
1. 打开Keil5，选择"Project" -> "New µVision Project"
2. 选择项目保存位置，输入项目名称（如：JY61P_Demo）
3. 在Device选择界面，选择你的MCU型号：
   - STM32F103系列：如STM32F103C8T6
   - STM32F407系列：如STM32F407VGT6

### 1.2 配置启动文件
1. 在弹出的"Manage Run-Time Environment"窗口中
2. 展开"Device"，勾选"Startup"
3. 展开"CMSIS"，勾选"CORE"
4. 点击"OK"

## 步骤2：添加源文件

### 2.1 创建文件组
在Project窗口中右键"Target 1"，选择"Manage Project Items"：
1. 添加组"APP"
2. 添加组"Drivers" 
3. 添加组"Ports"

### 2.2 添加应用层文件
在"APP"组中添加：
- `app/jy61p_app.c`
- `app/jy61p_app.h` (可选，作为参考)

### 2.3 添加驱动层文件
在"Drivers"组中添加：
- `drivers/wit_c_sdk/wit_c_sdk.c`

### 2.4 添加端口层文件
在"Ports"组中添加（根据MCU选择）：
- `ports/stm32f103/i2c_port.c`
- `ports/stm32f103/uart_port.c`
- `ports/stm32f103/delay_port.c`

或者：
- `ports/stm32f407/i2c_port.c`
- `ports/stm32f407/uart_port.c`
- `ports/stm32f407/delay_port.c`

## 步骤3：配置编译选项

### 3.1 设置包含路径
右键项目名称，选择"Options for Target"：
1. 切换到"C/C++"选项卡
2. 在"Include Paths"中添加：
   ```
   ..\app                (包含jy61p_app.h)
   ..\drivers\wit_c_sdk
   ..\ports\stm32f103    (或stm32f407)
   ```

### 3.2 设置预处理器定义
在"Preprocessor Symbols"中添加：

**对于STM32F103：**
```
STM32F103xB
USE_HAL_DRIVER
```

**对于STM32F407：**
```
STM32F407xx
USE_HAL_DRIVER
```

### 3.3 优化设置
1. "Optimization"选择"Level 1 (-O1)"
2. 勾选"One ELF Section per Function"
3. 勾选"Enum Container always int"

## 步骤4：配置调试选项

### 4.1 设置调试器
在"Debug"选项卡中：
1. 选择使用的调试器（如ST-Link Debugger）
2. 点击"Settings"进行详细配置

### 4.2 设置下载选项
在"Utilities"选项卡中：
1. 勾选"Update Target before Debugging"
2. 选择对应的下载算法

## 步骤5：硬件连接

### 5.1 JY61P连接（以STM32F103为例）
```
JY61P引脚    STM32F103引脚    功能
VCC         3.3V           电源正极
GND         GND            电源负极
SDA         PB11           I2C数据线
SCL         PB10           I2C时钟线
```

### 5.2 串口连接（用于数据输出）
```
USB-TTL     STM32F103引脚    功能
VCC         3.3V           电源（可选）
GND         GND            地线
RX          PA9 (USART1_TX) 接收MCU发送的数据
TX          PA10(USART1_RX) 发送命令到MCU（可选）
```

## 步骤6：编译和测试

### 6.1 编译项目
1. 按F7或点击"Build"按钮
2. 检查编译输出，确保无错误无警告

### 6.2 下载程序
1. 连接调试器到MCU
2. 按F8或点击"Download"按钮

### 6.3 运行测试
1. 打开串口助手，设置波特率115200
2. 复位MCU，观察串口输出
3. 应该看到JY61P初始化信息和实时数据

## 预期输出示例

```
************************ JY61P Gyroscope Application ***********************
*                     Platform Independent Version                        *
*                        Based on api-rules.md                           *
**************************************************************************
JY61P application initialized successfully.
Scanning I2C bus for JY61P sensors...
Found JY61P at I2C address: 0x50
JY61P initialized successfully at address 0x50

************************** JY61P Gyroscope Help ************************
*                        Command Reference                             *
**************************************************************************
Commands (send via UART with \r\n):
  a\r\n  - Start accelerometer calibration
  m\r\n  - Start magnetometer calibration
  ...

ACC : 0.123 -0.456 0.987 (g)
GYRO: 1.234 -2.345 0.123 (°/s)
ANGLE: 12.34 -23.45 1.23 (°)
MAG : 123 -456 789 (raw)
```

## 常见问题解决

### 编译错误
1. **找不到头文件**：检查Include Paths配置
2. **未定义符号**：确保所有必要的源文件都已添加
3. **重复定义**：检查是否有重复的源文件

### 运行问题
1. **无串口输出**：检查printf重定向配置
2. **找不到JY61P**：检查I2C连接和上拉电阻
3. **数据异常**：检查JY61P供电和I2C时序

## 高级配置

### 自定义main函数
如果需要自定义main函数，可以：
1. 不使用jy61p_app.c中的main函数
2. 在自己的main.c中调用JY61P API
3. 参考example_integration.c中的示例

### 多传感器支持
可以修改代码支持多个JY61P传感器：
1. 修改扫描函数支持多地址
2. 使用数组存储多个传感器上下文
3. 在数据处理中区分不同传感器

## 总结

按照本指南配置后，你将拥有一个完整的JY61P传感器Keil5项目，可以：
- 自动检测和连接JY61P传感器
- 实时读取和显示传感器数据
- 通过串口命令控制传感器
- 便于移植到不同的STM32平台
