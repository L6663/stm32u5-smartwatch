# STM32U5 SmartWatch

基于 **STM32U575、FreeRTOS 与 LVGL 8.3** 开发的智能手表嵌入式原型项目。

项目围绕智能穿戴设备的典型功能展开，完成了图形显示、触摸交互、实时时钟、环境采集、健康采集和多页面应用框架，重点体现 STM32 外设驱动、FreeRTOS 多任务设计、LVGL 图形界面开发及软硬件联调能力。

## 项目效果

系统包含五个主要页面：

```text
Home → Health → Sport → Weather → Call
```

- **Home**：时间、日期、表盘、温湿度、步数、电量与天气概览
- **Health**：心率、血氧、波形、最大值与最小值
- **Sport**：运动模式、时长、距离、热量与运动心率
- **Weather**：地点、日期、温度、天气状态与图标
- **Call**：联系人信息及来电交互界面

## 硬件平台

| 模块 | 方案 |
|---|---|
| 主控 | STM32U575 |
| 显示 | ILI9341，320×240 |
| 触摸 | FT6336 |
| 温湿度 | SHT20 |
| 健康采集 | MAX30102 |
| 实时时钟 | STM32 RTC |
| 无线扩展 | ESP8266 |

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
- Keil MDK

## 系统架构

```text
┌──────────────────────────────────────┐
│            LVGL 图形界面层            │
│ Home / Health / Sport / Weather / Call│
└──────────────────▲───────────────────┘
                   │ 消息队列
┌──────────────────┴───────────────────┐
│              应用业务层               │
│ ClockApp / HealthApp / SensorApp     │
│ SportApp                             │
└──────────────────▲───────────────────┘
                   │
┌──────────────────┴───────────────────┐
│              FreeRTOS 层             │
│ LVGLTask / HealthTask / SensorTask   │
│ SystemTask / Queue / Mutex           │
└──────────────────▲───────────────────┘
                   │
┌──────────────────┴───────────────────┐
│               驱动层                 │
│ LCD / Touch / RTC / Sensor           │
│ SPI / GPDMA / I²C / TIM / UART       │
└──────────────────▲───────────────────┘
                   │
┌──────────────────┴───────────────────┐
│               硬件层                 │
│ STM32U575 / ILI9341 / FT6336         │
│ SHT20 / MAX30102                     │
└──────────────────────────────────────┘
```

## 核心设计

### 1. SPI + GPDMA 异步刷新

LVGL 使用双绘图缓冲，屏幕数据通过 SPI1 和 GPDMA 发送。DMA 完成后再通知 LVGL 释放当前绘图缓冲，从而减少 CPU 等待时间并提升界面响应速度。

### 2. LVGL 单任务访问

所有界面控件统一由 `LVGLTask` 更新，其他任务通过消息队列传递数据，避免多任务直接访问 LVGL 对象。

### 3. I²C 多设备共享

FT6336、SHT20 与 MAX30102 共用 I²C1，通过 FreeRTOS 互斥锁完成总线资源管理。

### 4. 模块化应用结构

时间、健康、环境与运动功能分别封装为独立应用模块，便于维护和继续扩展。

## FreeRTOS 任务

| 任务 | 职责 |
|---|---|
| `LVGLTask` | LVGL 初始化、页面交互、控件更新和界面刷新 |
| `HealthTask` | MAX30102 采集与健康数据处理 |
| `SensorTask` | 温湿度、运动与天气数据维护 |
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
├── docs/                         项目展示文档
├── STM32U5_LVGL_Inteligentlight.ioc
└── README.md
```

## 工程入口

- CubeMX：`STM32U5_LVGL_Inteligentlight.ioc`
- Keil：`MDK-ARM/STM32U5_LVGL_Inteligentlight.uvprojx`
- 主程序：`Core/Src/main.c`
- RTOS：`Core/Src/app_freertos.c`
- 显示适配：`LVGL/porting/lv_port_disp.c`
- 触摸适配：`LVGL/porting/lv_port_indev.c`
- UI：`LVGL/UI/`

## 项目文档

<<<<<<< HEAD
- [软件架构](docs/ARCHITECTURE.md)
- [仓库结构](docs/REPOSITORY_STRUCTURE.md)
- [构建与烧录](docs/BUILD_AND_FLASH.md)
- [功能完成度](docs/FEATURE_STATUS.md)
- [V3 审查报告](docs/PROJECT_AUDIT_V3.md)
- [后续路线](docs/ROADMAP.md)

=======
- [项目总览](docs/PROJECT_OVERVIEW.md)
- [系统架构](docs/ARCHITECTURE.md)
- [功能展示](docs/FEATURES.md)
- [构建说明](docs/BUILD.md)
- [开发规划](docs/ROADMAP.md)
>>>>>>> cbc77f5 (docs: simplify repository presentation)

## 后续扩展

<<<<<<< HEAD
```bash
python scripts/verify_repo.py
```

当前包验证目标：

- Keil 工程引用无缺失；
- 不包含 Keil 构建输出；
- 不包含单文件超过 GitHub 100 MB 限制的内容；
- 保留 `.ioc`、`.uvprojx`、驱动、RTOS、LVGL、UI 和业务源码。


=======
- IMU 计步与姿态识别
- 电池电量采集
- ESP8266 网络通信
- 在线天气与网络校时
- 来电与消息通知
- 低功耗管理
- 健康算法参数优化
>>>>>>> cbc77f5 (docs: simplify repository presentation)
