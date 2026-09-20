<div align="center">

[English](README.md) | **简体中文**

</div>

# HandBridge-SG90

一个使用 SG90 舵机替换 [AmazingHand](https://github.com/pollen-robotics/AmazingHand) 原舵机的开源控制器与协议桥接项目。

本项目将 AmazingHand 原舵机替换为普通 **SG90 180° 舵机**。项目使用以 **STM32F103C8T6 + PCA9685** 为核心的自制控制板，保留上位机侧串口协议，并将位置、速度和时间指令转换为 SG90 可用的 PWM 信号。

> 本项目是社区兼容方案，与 AmazingHand 原作者或相关厂商无官方隶属关系。AmazingHand 及相关名称的权利归其各自所有者所有。

<p align="center">
  <img src="Docs/myhand.jpg" alt="使用 SG90 舵机改造的 AmazingHand" width="900">
</p>

<p align="center"><em>SG90 舵机与自制控制硬件组成的实机原型。</em></p>

## 项目特性

- 兼容 `0xFF 0xFF` 帧头的 AmazingHand/舵机串口指令格式
- 支持舵机 ID `1 ~ 10`
- 支持 Ping、Read、Write 和 Sync Write 指令
- 将 `0 ~ 1023` 的目标位置映射为 SG90 的 `180° ~ 0°`
- 支持按目标时间或目标速度进行逐度运动
- 使用 PCA9685 输出 50 Hz PWM，可扩展至 16 路
- 提供 EEPROM/SRAM 风格的虚拟寄存器，便于兼容上位机读写
- 支持 10 路 ADC 舵机在线检测输入
- USART1 输出调试日志，USART3 接收控制协议
- 提供自制四层控制板及可直接交给 PCB 厂的生产文件

## 工作原理

```text
AmazingHand 上位机 / 控制器
            │
            │ UART，1 Mbps
            ▼
        自制控制板
      ├─ STM32F103C8T6 协议转换
      ├─ CH340C USB 转串口
      ├─ 虚拟寄存器与 ADC 检测
      └─ PCA9685 PWM 输出
            │
            │ I²C
            ▼
          PCA9685
            │
            │ 50 Hz PWM
            ▼
        SG90 舵机 × 10
```

收到目标位置后，固件按以下关系转换角度：

```text
SG90 角度 = 180 - 目标位置 × 180 / 1023
```

PCA9685 默认使用地址 `0x40`，PWM 频率为 `50 Hz`。当前脉宽计数范围为：

- 0°：`102`
- 180°：`512`
- 安全限幅：`80 ~ 550`

不同批次 SG90 的机械行程可能存在差异。首次通电前建议卸下舵机摇臂或减小行程，确认方向与极限位置后再连接机械结构。

## 硬件需求

- `Docs/PCB` 中提供的自制控制板，或功能等效的 STM32F103C8T6 + PCA9685 电路
- SG90 舵机，最多 10 个
- 稳定的 5 V 舵机电源
- ST-Link 下载器
- USB 数据线，用于板载 CH340C 调试串口
- 可选：外接 USB 转串口模块，用于连接控制端口
- 可选：舵机在线检测电路

## 接线说明

项目 PCB 已集成 STM32F103C8T6、CH340C、3.3 V 稳压器、PCA9685、状态指示灯、SWD、串口接口、10 路 PWM 输出和 10 路 ADC 检测输入。以下表格描述固件层面的连接关系，也适用于使用模块自行搭建电路。

### STM32 与 PCA9685

| STM32F103C8T6 | PCA9685 | 说明 |
| --- | --- | --- |
| PB6 | SCL | I²C1 时钟 |
| PB7 | SDA | I²C1 数据 |
| 3.3V | VCC | PCA9685 逻辑电源 |
| GND | GND | 共地 |
| 外部 5V | V+ | SG90 舵机电源 |

### 串口

| 接口 | 引脚 | 波特率 | 用途 |
| --- | --- | --- | --- |
| USART1 TX/RX | PA9 / PA10 | 115200，8N1 | 调试日志 |
| USART3 TX/RX | PB10 / PB11 | 1000000，8N1 | AmazingHand 控制协议 |

连接控制串口时应交叉连接 TX/RX，并确保双方共地。STM32F103 的串口逻辑电平为 3.3 V，请勿直接接入 RS-232 电平。

板载 CH340C 连接 USART1，用于固件日志与开发调试；AmazingHand 控制协议通过 PB10/PB11 对应的 USART3 处理。

### 舵机通道

当前固件直接使用舵机 ID 作为 PCA9685 通道号：

| 舵机 ID | PCA9685 通道 |
| --- | --- |
| 1 ~ 10 | CH1 ~ CH10 |

因此 `CH0` 当前未使用。SG90 常见线序为：棕色 GND、红色 5 V、橙色 PWM；不同厂商可能不同，请以实际舵机资料为准。

### ADC 在线检测

固件将舵机 ID `1 ~ 10` 分别映射到以下 ADC 输入：

| 舵机 ID | ADC 输入 | MCU 引脚 |
| --- | --- | --- |
| 1 | ADC1_IN0 | PA0 |
| 2 | ADC1_IN1 | PA1 |
| 3 | ADC1_IN2 | PA2 |
| 4 | ADC1_IN3 | PA3 |
| 5 | ADC1_IN4 | PA4 |
| 6 | ADC1_IN5 | PA5 |
| 7 | ADC1_IN6 | PA6 |
| 8 | ADC1_IN7 | PA7 |
| 9 | ADC1_IN8 | PB0 |
| 10 | ADC1_IN9 | PB1 |

当前在线判定范围为约 `8 ~ 250 mV`。普通 SG90 本身通常不提供位置反馈，因此这部分需要配合项目对应的检测硬件；如果你的硬件没有该检测电路，Ping 和 Read 指令可能不会返回数据，需要按实际硬件调整 `Business/control/control.c` 中的 `s_get_servo()`。

## 自制控制板

<p align="center">
  <a href="Docs/PCB/SCH_Schematic1_1-P1_2026-09-20.png">
    <img src="Docs/PCB/SCH_Schematic1_1-P1_2026-09-20.png" alt="控制板原理图" width="1000">
  </a>
</p>

控制板集成了协议转换所需的完整硬件：

- STM32F103C8T6，外接 8 MHz 晶振，系统主频 72 MHz
- PCA9685 16 路 PWM 控制器，其中 10 路用于机械手
- CH340C USB 转串口与自动复位电路
- RT9193-33GB 3.3 V 稳压器和 USB 逻辑电源输入
- 10 路 PWM 输出与 10 路电流/电压检测输入
- SWD 下载、串口、BOOT0、复位和状态指示灯接口
- 独立舵机电源输入及 220 µF 大容量滤波电容

制造资料包含四层板 Gerber、BOM 和贴片坐标文件。当前 BOM 共 29 个物料条目、81 个元件，其中包含 72 个贴片元件和 9 个通孔元件。

| 文件 | 用途 |
| --- | --- |
| [原理图](Docs/PCB/SCH_Schematic1_1-P1_2026-09-20.png) | 完整控制板原理图 |
| [Gerber 生产文件](Docs/PCB/Gerber_PCB1_2026-09-20.zip) | 包含铜层、阻焊、丝印、板框和钻孔数据 |
| [物料清单 BOM](Docs/PCB/BOM_Board1_PCB1_2026-09-20.xlsx) | 元件参数、封装、位号及部分供应商料号 |
| [贴片坐标文件](Docs/PCB/PickAndPlace_PCB1_2026_09_20.xlsx) | SMT 坐标、板层和旋转角度 |

下单前请自行复核原理图、封装、极性、连接器顺序和 PCB 厂商的生产规则。这些文件对应当前原型版本，不提供生产良率保证。

## 供电警告

请勿使用 STM32 开发板的 5 V 引脚或 USB 接口直接为多路 SG90 供电。

- SG90 启动和堵转时会产生较大的瞬时电流
- 多舵机系统应使用独立、足够容量的 5 V 电源
- STM32、PCA9685、舵机电源和控制端必须共地
- 建议在 PCA9685 的 V+ 与 GND 附近增加大容量电解电容
- 首次测试时一次只连接一个舵机，并避免机械堵转

10 路舵机所需电流与负载、舵机批次及动作方式有关，请为电源预留充分余量。

## 开发环境

- MCU：STM32F103C8T6
- 主频：72 MHz
- HAL：STM32F1 HAL Driver
- 配置工具：STM32CubeMX
- 工程文件：Keil MDK-ARM 5
- PCB 设计/导出工具：嘉立创 EDA（EasyEDA/JLCEDA）
- CubeMX 工程：`Servo_Protocol_Conversion.ioc`
- Keil 工程：`MDK-ARM/Servo_Protocol_Conversion.uvprojx`

## 编译与烧录

1. 安装 Keil MDK-ARM 5，并安装 STM32F1 对应的 Device Pack。
2. 打开 `MDK-ARM/Servo_Protocol_Conversion.uvprojx`。
3. 选择 `Servo_Protocol_Conversion` Target 并编译工程。
4. 使用 ST-Link 将程序下载到 STM32F103C8T6。
5. 上电后，通过 USART1 可看到初始化和通信调试信息。

仓库中也包含已生成的固件：

```text
MDK-ARM/Servo_Protocol_Conversion/Servo_Protocol_Conversion.hex
```

> 直接烧录现成 HEX 前，请确认你的 MCU 型号、晶振、引脚和外围硬件与本项目一致。

## 协议概览

基础数据帧格式：

```text
FF FF ID LENGTH COMMAND/ERROR PARAM... CHECKSUM
```

校验和计算方式：

```text
CHECKSUM = ~(ID + LENGTH + COMMAND/ERROR + PARAM...) & 0xFF
```

固件当前识别的 ID 为 `1 ~ 10`，广播 ID 为 `0xFE`。

| 指令 | 指令码 | 说明 |
| --- | --- | --- |
| Ping | `0x01` | 检测舵机是否在线 |
| Read | `0x02` | 读取虚拟寄存器 |
| Write | `0x03` | 写入虚拟寄存器并控制舵机 |
| Sync Write | `0x83` | 同步控制多个舵机 |

主要运动寄存器：

| 地址 | 名称 |
| --- | --- |
| `0x2A ~ 0x2B` | 目标位置 |
| `0x2C ~ 0x2D` | 目标时间 |
| `0x2E ~ 0x2F` | 目标速度 |
| `0x38 ~ 0x39` | 当前位置 |
| `0x3A ~ 0x3B` | 当前速度 |

控制行为：

- 目标速度为 `0`：直接设置目标角度
- 目标时间大于 `0`：按设定时间逐度移动
- 目标时间为 `0` 且目标速度大于 `0`：按速度逐度移动
- ID 为 `0xFE`：向 ID 1 到 10 广播写入

## 目录结构

```text
.
├─ Business/
│  ├─ control/           # 协议到舵机动作的转换逻辑
│  ├─ message_process/   # 串口收包、帧解析和循环缓冲区
│  └─ register/          # 兼容协议的虚拟寄存器
├─ Core/                 # STM32CubeMX 生成的初始化与主程序
├─ Drivers/              # STM32 HAL 和 CMSIS
├─ Functional/
│  ├─ servo/             # PCA9685 / SG90 驱动
│  └─ eeprom/            # 外部 Flash 驱动（当前主流程未启用）
├─ Docs/
│  ├─ myhand.jpg         # 实机原型照片
│  └─ PCB/               # 原理图、Gerber、BOM 和贴片坐标文件
├─ MDK-ARM/              # Keil 工程及构建产物
├─ README.md             # 英文文档（GitHub 默认显示）
├─ README_zh-CN.md       # 简体中文文档
└─ Servo_Protocol_Conversion.ioc
```

## 当前限制

- 当前固件按裸机方式运行，不使用 RTOS
- 仅验证了 STM32F103C8T6、PCA9685 和 SG90 组合
- 位置映射方向固定为 `1023 → 0°`、`0 → 180°`
- 运动过程使用阻塞式延时，一个舵机平滑运动时会暂时阻塞其他指令处理
- 虚拟寄存器位于 RAM，设备复位后会恢复默认值
- ADC 在线检测依赖额外硬件，不能直接视为 SG90 的位置反馈
- SG90 精度、死区和负载能力与原 AmazingHand 舵机不同，动作效果不会完全一致

## 参数调整

常用参数位于以下文件：

- `Functional/servo/pca9685.c`
  - PCA9685 I²C 地址
  - PWM 频率
  - 0°/180° 脉宽和安全限幅
- `Business/control/control.c`
  - 位置到角度的映射方向
  - 速度和时间控制算法
  - ADC 在线检测阈值
- `Core/Src/usart.c`
  - 控制串口与调试串口波特率

修改机械行程前，请先确认舵机不会撞击限位。

## 贡献

欢迎提交 Issue 和 Pull Request，例如：

- 增加非阻塞式多舵机运动控制
- 增加不同舵机的独立方向、零点和行程校准
- 改进协议兼容性和异常帧处理
- 补充原理图、PCB、接线图和实机演示
- 增加 GCC/CMake 或 PlatformIO 构建支持

## 开源许可证

HandBridge-SG90 使用 [MIT License](LICENSE) 开源。

STM32 HAL、CMSIS 等第三方组件遵循其各自目录中的许可证。

## 致谢

- [AmazingHand](https://github.com/pollen-robotics/AmazingHand) 项目及其社区
- STMicroelectronics STM32 HAL / CMSIS
- PCA9685 与 SG90 相关开源资料和社区实践
