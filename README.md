# STM32U5 SmartWatch

基于 **STM32U575、FreeRTOS 与 LVGL 8.3** 开发的智能手表嵌入式原型项目。

项目重点展示 STM32 外设驱动、FreeRTOS 多任务设计、LVGL 图形界面开发、消息队列通信、共享总线保护及软硬件联调能力。当前版本已经形成完整的软件框架，但仍包含部分演示数据和待接入功能，因此不定位为量产级智能手表产品。

## 项目演示

当前仓库已完成源码和文档整理。实机主图、五页面截图拼图及页面滑动 GIF 将在后续提交中补充。

## 项目功能

系统包含五个主要页面：

```text
Home → Health → Sport → Weather → Call
```

- **Home**：时间、日期、表盘、温湿度、步数、电量与天气概览
- **Health**：心率、血氧估算、波形、最大值与最小值
- **Sport**：运动模式、时长、距离、热量与运动心率
- **Weather**：地点、日期、温度、天气状态与图标
- **Call**：联系人信息及来电交互界面

## 功能完成度

| 功能 | 当前状态 | 说明 |
|---|---|---|
| ILI9341 图形显示 | 已实现 | 使用 SPI1 与 GPDMA 完成 LVGL 屏幕刷新 |
| FT6336 触摸输入 | 已实现 | 支持点击、滑动及五页面循环切换 |
| FreeRTOS 多任务 | 已实现 | 包含 LVGL、健康、传感器和系统任务 |
| UI 消息队列 | 已实现 | 业务任务通过队列向 LVGLTask 传递数据 |
| I²C 总线互斥 | 已实现 | FT6336、SHT20 与 MAX30102 共享 I²C1 |
| RTC 时间与日期 | 已实现 | 用于数字时间、日期和表盘更新 |
| SHT20 温湿度采集 | 已实现 | 使用非保持模式读取并校验 CRC |
| MAX30102 采集链路 | 已实现 | 包含驱动、FIFO 采样、手指检测和心率处理链路 |
| 血氧显示 | 实验性实现 | 当前为未校准的项目级估算，不可用于医疗诊断 |
| 步数 | 演示数据 | 当前由软件模拟，尚未接入 IMU |
| 天气信息 | 演示数据 | 当前为本地轮换数据，尚未接入在线天气服务 |
| 运动距离、热量和心率 | 演示数据 | 用于验证页面和任务通信结构 |
| 电量显示 | 演示数据 | 尚未接入电池 ADC 或电量计 |
| 来电功能 | 界面框架 | 当前完成页面与交互展示，尚未接入真实通知链路 |
| ESP8266 联网 | 待接入 | 已预留扩展方向，尚未形成 Wi-Fi 通信闭环 |
| 低功耗管理 | 待完善 | 屏幕休眠、触摸唤醒和传感器电源管理仍在规划中 |

## 硬件平台

| 模块 | 方案 |
|---|---|
| 主控 | STM32U575 |
| 显示 | ILI9341，320×240 TFT |
| 触摸 | FT6336 电容触摸 |
| 温湿度 | SHT20 |
| 健康采集 | MAX30102 |
| 实时时钟 | STM32 RTC |
| 调试通信 | USART1 |
| 无线扩展 | 预留 ESP8266 接入方向 |

## 软件技术栈

- STM32 HAL
- FreeRTOS / CMSIS-RTOS2
- LVGL 8.3
- SquareLine Studio
- SPI1 + GPDMA
- I²C
- RTC
- USART
- STM32CubeMX
- Keil MDK-ARM

## 系统架构

```text
┌──────────────────────────────────────┐
│            LVGL 图形界面层           │
│ Home / Health / Sport / Weather / Call│
└──────────────────▲───────────────────┘
                   │ 消息队列
┌──────────────────┴───────────────────┐
│              应用业务层              │
│ ClockApp / HealthApp / SensorApp    │
│ SportApp                            │
└──────────────────▲───────────────────┘
                   │
┌──────────────────┴───────────────────┐
│              FreeRTOS 层            │
│ LVGLTask / HealthTask / SensorTask  │
│ SystemTask / Queue / Mutex          │
└──────────────────▲───────────────────┘
                   │
┌──────────────────┴───────────────────┐
│               驱动层                │
│ LCD / Touch / RTC / Sensor          │
│ SPI / GPDMA / I²C / TIM / UART      │
└──────────────────▲───────────────────┘
                   │
┌──────────────────┴───────────────────┐
│               硬件层                │
│ STM32U575 / ILI9341 / FT6336        │
│ SHT20 / MAX30102                    │
└──────────────────────────────────────┘
```

## 核心设计

### 1. SPI + GPDMA 异步刷新

LVGL 使用双绘图缓冲，屏幕数据通过 SPI1 和 GPDMA 发送。DMA 完成后再通知 LVGL 释放当前绘图缓冲，减少 CPU 等待时间并提升界面响应速度。

### 2. LVGL 单任务访问

所有 LVGL 控件统一由 `LVGLTask` 更新。其他任务只负责采集和业务处理，并通过消息队列向界面任务发送数据，避免多任务并发访问 LVGL 对象。

### 3. I²C 多设备共享

FT6336、SHT20 与 MAX30102 共用 I²C1，各任务访问总线前申请互斥锁，通信完成后释放，避免并发访问造成总线冲突。

### 4. 模块化应用结构

时间、健康、环境与运动功能分别封装为独立应用模块，便于维护、定位问题和继续扩展。

## FreeRTOS 任务

| 任务 | 主要职责 |
|---|---|
| `LVGLTask` | LVGL 初始化、页面交互、控件更新和界面刷新 |
| `HealthTask` | MAX30102 采样与健康数据处理 |
| `SensorTask` | 温湿度、演示步数、演示天气及相关数据维护 |
| `SystemTask` | RTC 时间、日期与表盘更新 |

## 仓库结构

```text
stm32u5-smartwatch/
├── Core/                         应用代码与外设初始化
├── Drivers/                      CMSIS 与 STM32 HAL
├── LVGL/
│   ├── UI/                       SquareLine 生成界面
│   ├── porting/                  显示与触摸适配
│   └── src/                      LVGL 源码
├── Middlewares/                  FreeRTOS 与 CMSIS-RTOS2
├── MDK-ARM/                      Keil 工程
├── docs/                         项目说明文档
├── STM32U5_LVGL_Inteligentlight.ioc
└── README.md
```

> `.ioc` 与 Keil 工程暂时保留原有文件名，以避免破坏现有工程路径。对外项目名称统一使用 STM32U5 SmartWatch。

## 工程入口

- 主程序：`Core/Src/main.c`
- FreeRTOS：`Core/Src/app_freertos.c`
- 显示适配：`LVGL/porting/lv_port_disp.c`
- 触摸适配：`LVGL/porting/lv_port_indev.c`
- UI 文件：`LVGL/UI/`
- CubeMX 工程：`STM32U5_LVGL_Inteligentlight.ioc`
- Keil 工程：`MDK-ARM/STM32U5_LVGL_Inteligentlight.uvprojx`

## 构建与烧录

1. 使用 Keil MDK-ARM 打开工程；
2. 确认安装 STM32U5 Device Family Pack；
3. 使用 ARM Compiler 6；
4. 执行 `Rebuild all target files`；
5. 连接 ST-Link 并下载程序；
6. 复位后通过 USART1 查看启动日志。

串口参数：

```text
115200 baud
8 data bits
No parity
1 stop bit
```

## 项目文档

- [项目总览](docs/PROJECT_OVERVIEW.md)
- [系统架构](docs/ARCHITECTURE.md)
- [功能展示](docs/FEATURES.md)
- [构建说明](docs/BUILD.md)
- [开发规划](docs/ROADMAP.md)

## 后续规划

- 接入 IMU，实现真实计步与姿态识别；
- 接入电池 ADC 或电量计；
- 接入 ESP8266，实现在线天气、网络校时和消息通知；
- 优化 MAX30102 信号处理与参数标定；
- 完善屏幕休眠、触摸唤醒和系统低功耗管理；
- 完成长时间运行、任务栈水位和系统内存测试。

## 说明

本项目用于嵌入式系统学习、工程实践与求职展示。健康相关结果仅用于算法和界面验证，不构成医疗测量结果。
