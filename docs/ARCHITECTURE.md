# 软件架构

## 1. 启动链路

```text
Reset
  -> HAL_Init
  -> SystemClock_Config
  -> CubeMX 外设初始化
  -> osKernelInitialize
  -> MX_FREERTOS_Init
  -> osKernelStart
```

工程入口：

- `Core/Src/main.c`
- `Core/Src/app_freertos.c`

## 2. 任务划分

```text
LVGLTask
  ├─ LVGL 初始化
  ├─ ILI9341 显示端口
  ├─ FT6336 输入端口
  ├─ 五页 UI 初始化
  ├─ 接收各业务队列
  └─ 执行 lv_timer_handler

HealthTask
  └─ MAX30102 初始化、FIFO 采样、波形、BPM/实验性 SpO2

SensorTask
  ├─ SHT20 温湿度采集
  └─ SportApp_Tick1s 演示运动数据

SystemTask
  └─ RTC 时间读取和表盘更新
```

所有 LVGL 控件修改统一在 `LVGLTask` 内执行。其他任务只向消息队列投递数据，避免跨线程直接调用 LVGL。

## 3. 数据流

```text
SHT20 --------> SensorTask -----> Sensor UI Queue ---+
MAX30102 -----> HealthTask -----> Health UI Queue ---+--> LVGLTask --> UI
RTC ----------> SystemTask -----> Clock UI Queue ----+
Sport demo ---> SensorTask -----> Sport UI Queue ----+
```

## 4. I²C1 总线

FT6336、SHT20、MAX30102 共用 I²C1。V3 已统一通过 `i2c1Mutex` 串行访问：

```text
FT6336 ----+
SHT20 -----+--> i2c1Mutex --> HAL_I2C --> I²C1
MAX30102 --+
```

后续接入 AP3216C 时也必须经过同一总线互斥层。

## 5. 显示链路

```text
LVGL draw buffer
  -> lv_port_disp
  -> RGB565 DMA buffer
  -> SPI1
  -> GPDMA
  -> ILI9341 320 x 240
```

当前使用两个 `320 x 20` LVGL 绘图缓冲和一个同尺寸 DMA 缓冲。

## 6. UI 页面

```text
Home <-> Health <-> Sport <-> Weather <-> Call <-> Home
```

页面代码位于 `LVGL/UI/screens/`，统一导航逻辑位于 `Core/Src/app_freertos.c` 及 UI 事件文件中。
