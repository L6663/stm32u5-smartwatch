# 系统架构

## 1. 启动流程

```text
系统复位
   ↓
HAL 初始化
   ↓
GPIO 与 USART1
   ↓
系统时钟配置
   ↓
GPDMA / I²C / SPI / ICACHE / TIM / RTC
   ↓
FreeRTOS 内核与对象创建
   ↓
任务调度启动
   ↓
LVGL 与业务模块运行
```

## 2. 任务架构

| 任务 | 主要职责 |
|---|---|
| LVGLTask | 页面初始化、消息处理、动画与界面刷新 |
| HealthTask | MAX30102 采样与健康数据处理 |
| SensorTask | 温湿度、运动和天气数据维护 |
| SystemTask | RTC 时间、日期和表盘更新 |

## 3. UI 线程模型

LVGL 控件只在 `LVGLTask` 中访问。

```text
HealthTask ──┐
SensorTask ──┼──> 消息队列 ──> LVGLTask ──> 页面控件
SystemTask ──┘
```

这种设计避免多个任务同时修改图形对象，提高系统稳定性。

## 4. 显示链路

```text
LVGL 绘图缓冲
   ↓
RGB565 数据处理
   ↓
独立 DMA 发送缓冲
   ↓
SPI1 + GPDMA
   ↓
ILI9341
```

显示端采用双绘图缓冲和 DMA 完成回调，使 CPU 在屏幕发送期间可以继续执行其他任务。

## 5. 触摸链路

```text
FT6336
   ↓ I²C1
触点读取与坐标转换
   ↓
LVGL 输入设备
   ↓
点击与滑动事件
   ↓
页面切换
```

## 6. I²C 总线管理

```text
FT6336 ─────┐
SHT20 ──────┼──> i2c1Mutex ──> I²C1
MAX30102 ───┘
```

每次访问 I²C1 前先申请互斥锁，通信结束后释放，避免多任务并发访问冲突。

## 7. 主要数据流

### 时间

```text
RTC → SystemTask → Clock Queue → LVGLTask → Home
```

### 温湿度

```text
SHT20 → SensorTask → UI Queue → LVGLTask → Home
```

### 健康数据

```text
MAX30102 → HealthTask → 算法处理 → Health Queue → LVGLTask → Health
```
