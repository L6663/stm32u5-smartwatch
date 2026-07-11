# STM32U5 SmartWatch Prototype

基于 **STM32U575、FreeRTOS、LVGL 8.3 和 SquareLine Studio** 的智能手表 Alpha 原型工程。

> 当前版本：`v0.3.0-alpha-review`。项目已经完成显示、触摸、RTC、任务通信、环境采集和健康采集的基础链路，但仍包含模拟数据和未接入模块，不能表述为完整成品或量产产品。

## 当前能力

- STM32U575 HAL/CubeMX 与 Keil MDK 工程
- ILI9341 320×240，SPI1 + GPDMA 异步刷新
- FT6336 电容触摸
- FreeRTOS 四任务、消息队列和 I²C1 互斥访问
- Home / Health / Sport / Weather / Call 五页 LVGL UI
- RTC 时间与指针表盘更新
- SHT20 温湿度采集
- MAX30102 采样、波形和 BPM 框架
- 页面滑动、健康测量和运动交互框架

## 未完成或模拟功能

| 模块 | 状态 |
|---|---|
| 步数 | 随机演示数据，未接入 IMU |
| 电量 | 固定 86%，未接入 ADC/电量计 |
| 天气 | 本地轮换数据，未联网 |
| 运动距离/热量/运动心率 | 模拟模型 |
| 血氧 | 实验性估算，未完成硬件标定 |
| 来电页面 | 静态 UI，按钮未接业务逻辑 |
| ESP8266 | 代码资产存在，未形成当前联网闭环 |
| AP3216C | 驱动存在，未纳入当前应用数据流 |
| 低功耗和网络校时 | 未完成 |

完整说明见 [`docs/FEATURE_STATUS.md`](docs/FEATURE_STATUS.md)。

## V3 关键修复

1. FT6336、SHT20、MAX30102 统一通过 `i2c1Mutex` 访问 I²C1。
2. FT6336 触点数据改为一次连续读取，减少事务并避免拆帧。
3. 修正横屏触摸坐标可能得到 `x=320` 的越界问题。
4. LVGL Tick 按真实 RTOS Tick 差值推进。
5. SensorTask 和 ClockApp 使用绝对周期调度。
6. 增加触摸 I²C 错误和互斥锁超时计数。

## 快速入口

- CubeMX：`STM32U5_LVGL_Inteligentlight.ioc`
- Keil：`MDK-ARM/STM32U5_LVGL_Inteligentlight.uvprojx`
- RTOS：`Core/Src/app_freertos.c`
- 显示/触摸端口：`LVGL/porting/`
- UI：`LVGL/UI/`

## 仓库文档

- [软件架构](docs/ARCHITECTURE.md)
- [仓库结构](docs/REPOSITORY_STRUCTURE.md)
- [构建与烧录](docs/BUILD_AND_FLASH.md)
- [功能完成度](docs/FEATURE_STATUS.md)
- [V3 审查报告](docs/PROJECT_AUDIT_V3.md)
- [后续路线](docs/ROADMAP.md)


## 上传前验证

```bash
python scripts/verify_repo.py
```

当前包验证目标：

- Keil 工程引用无缺失；
- 不包含 Keil 构建输出；
- 不包含单文件超过 GitHub 100 MB 限制的内容；
- 保留 `.ioc`、`.uvprojx`、驱动、RTOS、LVGL、UI 和业务源码。


