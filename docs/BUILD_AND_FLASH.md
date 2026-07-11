# 构建与烧录

## 环境

- STM32CubeMX
- Keil MDK-ARM
- ARM Compiler 6
- STM32U5 Device Family Pack
- ST-Link

## 工程入口

- CubeMX：`STM32U5_LVGL_Inteligentlight.ioc`
- Keil：`MDK-ARM/STM32U5_LVGL_Inteligentlight.uvprojx`

保留了原工程文件名中的 `Inteligentlight` 拼写，以免破坏已有工程路径和自动生成关系。

## 构建步骤

1. 解压仓库，不要放在过深或包含特殊字符的目录。
2. 使用 Keil 打开 `.uvprojx`。
3. 确认目标器件为 STM32U575 对应型号。
4. 确认使用 ARM Compiler 6。
5. 执行 `Rebuild all target files`。
6. 检查是否为 0 Error，并记录 Warning。
7. 连接 ST-Link 后烧录。
8. USART1 使用 `115200-8-N-1` 查看启动日志。

## 首次回归项目

- 冷启动、复位、断电重启显示是否稳定。
- 五页连续滑动是否卡顿或错页。
- 触摸四角及边缘坐标是否正确。
- MAX30102、SHT20 和触摸同时运行时 I²C 错误计数是否增长。
- RTC 时间是否正常推进。
- 背光 PB10/TIM2 CH3 的实际电气逻辑是否与代码一致。

## 已知边界

此仓库已通过源码引用检查和主机端 Clang C11 语法检查，但未在当前交付环境执行 Keil ARM 完整链接，也未代替真实开发板回归。
