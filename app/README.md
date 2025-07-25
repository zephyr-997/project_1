# JY61P陀螺仪传感器应用层 (APP Layer)

## 概述

本目录包含JY61P陀螺仪传感器的应用层实现，遵循`api-rules.md`中定义的平台无关设计原则。应用层仅依赖`drivers/`层的API接口，不包含任何MCU特定代码，实现了完整的软件层级分离。**专门针对Keil5开发环境优化，便于移植和集成。**

## 文件结构

```
app/
├── jy61p_app.c     # JY61P应用层主程序实现
├── jy61p_app.h     # JY61P应用层对外接口定义
├── README.md       # 本说明文档
└── KEIL5_SETUP_GUIDE.md  # Keil5配置指南
```

## 主要特性

### 1. Keil5友好设计
- ✅ 单一源文件`jy61p_app.c`，便于添加到Keil项目
- ✅ 清晰的头文件`app.h`，包含所有必要接口
- ✅ 不依赖复杂的构建系统，直接编译即可

### 2. 平台无关设计
- ✅ 不包含任何MCU头文件（如`stm32xxx.h`）
- ✅ 仅依赖`drivers/wit_c_sdk/`和`ports/`层接口
- ✅ 支持STM32F103、STM32F407等多种MCU

### 3. 功能完整性
- ✅ 自动JY61P传感器扫描和连接
- ✅ 实时数据读取和单位转换
- ✅ 串口命令解析和处理
- ✅ JY61P传感器校准和配置
- ✅ 友好的用户界面和帮助信息

### 4. 接口最小化
- ✅ 对外仅暴露4个核心API函数
- ✅ 隐藏内部实现细节
- ✅ 支持数据查询和状态检查

## API接口

### 数据获取
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

### 命令处理
```c
// 在串口中断中调用
void UART_IRQHandler(void) {
    if (UART_GetITStatus(UART1, UART_IT_RXNE) != RESET) {
        uint8_t data = UART_ReceiveData(UART1);
        jy61p_cmd_data_received(data);  // 处理接收到的JY61P命令
    }
}
```

## 支持的命令

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

## 数据格式

应用层自动将传感器原始数据转换为标准物理单位：

```
ACC : X Y Z (g)      - 加速度，单位：重力加速度
GYRO: X Y Z (°/s)    - 角速度，单位：度每秒  
ANGLE: X Y Z (°)     - 欧拉角，单位：度
MAG : X Y Z (raw)    - 磁场原始值
```

## Keil5集成方法

### 1. 添加文件到Keil项目

**步骤1：添加源文件**
1. 在Keil5项目中右键点击"Source Group 1"
2. 选择"Add Files to Group 'Source Group 1'"
3. 添加以下文件：
   - `app/jy61p_app.c`
   - `drivers/wit_c_sdk/wit_c_sdk.c`
   - `ports/stm32fxxx/*.c` (根据你的MCU选择)

**步骤2：添加头文件路径**
1. 右键项目名称，选择"Options for Target"
2. 在"C/C++"选项卡中，添加Include Paths：
   - `app`
   - `drivers/wit_c_sdk`
   - `ports/stm32fxxx` (根据你的MCU选择)

### 2. 基本集成（推荐）
```c
#include "jy61p_app.h"

int main(void) {
    // 系统初始化（时钟、GPIO等）
    SystemInit();

    // JY61P应用层会自动初始化所有必要组件
    return main();  // 调用JY61P应用层主函数
}
```

### 3. 自定义集成（高级用法）
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

## 依赖关系

```
app/jy61p_app.c
    ├── jy61p_app.h                      # JY61P应用层接口
    ├── drivers/wit_c_sdk/wit_c_sdk.h    # WIT SDK接口
    ├── drivers/wit_c_sdk/REG.h          # 寄存器定义
    └── ports/*/wit_port.h               # 端口层接口（通过extern声明）
```

## Keil5编译配置

### 必需的源文件
在Keil5项目中添加以下源文件：
```
Source Group 1/
├── jy61p_app.c                    # JY61P应用层主文件
├── wit_c_sdk.c                    # WIT SDK实现
├── i2c_port.c                     # I2C端口层实现
├── uart_port.c                    # UART端口层实现
├── delay_port.c                   # 延时端口层实现
└── main.c (可选)                  # 如果需要自定义main函数
```

### 头文件包含路径
在"Options for Target" -> "C/C++" -> "Include Paths"中添加：
```
../app                 (包含jy61p_app.h)
../drivers/wit_c_sdk
../ports/stm32f103    (或stm32f407，根据MCU选择)
```

### 预处理器定义
根据目标MCU在"Preprocessor Symbols"中添加：
- STM32F103: `STM32F103xB, USE_HAL_DRIVER`
- STM32F407: `STM32F407xx, USE_HAL_DRIVER`

## Keil5测试验证

### 1. 编译测试
- [ ] Keil5项目编译无错误无警告
- [ ] 所有依赖文件正确包含
- [ ] 头文件路径配置正确
- [ ] 预处理器定义正确

### 2. 功能测试
- [ ] JY61P传感器自动扫描成功
- [ ] 数据实时输出正常（通过串口查看）
- [ ] 所有命令响应正确
- [ ] 校准功能工作正常

### 3. 接口测试
- [ ] `jy61p_get_sensor_data()` 返回有效数据
- [ ] `jy61p_is_sensor_connected()` 状态正确
- [ ] `jy61p_cmd_data_received()` 命令解析正确

### 4. 硬件测试
- [ ] 在实际STM32硬件上运行正常
- [ ] I2C通信稳定
- [ ] 串口输出正确

## Keil5故障排除

### 常见编译问题

1. **找不到头文件**
   ```
   错误：cannot open source input file "wit_c_sdk.h"
   解决：检查Include Paths是否正确添加了drivers/wit_c_sdk路径
   ```

2. **未定义的函数**
   ```
   错误：undefined symbol wit_port_i2c_init
   解决：确保端口层源文件已添加到项目中
   ```

3. **重复定义main函数**
   ```
   错误：multiple definition of 'main'
   解决：如果使用自定义main，不要同时包含jy61p_app.c中的main函数
   ```

### 常见运行问题

1. **JY61P扫描失败**
   - 检查I2C连接和上拉电阻（通常4.7kΩ）
   - 确认JY61P供电正常（3.3V或5V）
   - 验证端口层I2C实现正确

2. **数据输出异常**
   - 检查串口配置和波特率（默认115200）
   - 确认端口层UART实现正确
   - 验证printf重定向到串口

3. **命令无响应**
   - 检查串口中断是否正确调用`jy61p_cmd_data_received()`
   - 确认命令格式为`字符+\r\n`
   - 验证UART接收中断使能

### Keil5调试方法

1. **使用Keil调试器**：设置断点跟踪函数调用
2. **串口输出调试**：添加printf语句查看程序流程
3. **逻辑分析仪**：检查I2C时序是否正确

## 快速开始指南

### 5分钟快速集成
1. **创建Keil5项目**（选择对应的STM32芯片）
2. **添加文件**：将`jy61p_app.c`拖入项目
3. **配置路径**：添加头文件包含路径
4. **连接硬件**：JY61P的SDA/SCL连接到MCU的I2C引脚
5. **编译下载**：编译项目并下载到MCU
6. **查看输出**：通过串口助手查看JY61P数据

### 硬件连接示例（STM32F103）
```
JY61P    ->    STM32F103
VCC      ->    3.3V
GND      ->    GND
SDA      ->    PB11 (I2C2_SDA)
SCL      ->    PB10 (I2C2_SCL)
```

## 版本历史

- **v1.0** (2025-07-25): 初始版本，针对Keil5优化，便于移植
