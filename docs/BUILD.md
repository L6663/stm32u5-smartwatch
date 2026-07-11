# 构建说明

## 开发环境

- STM32CubeMX
- Keil MDK-ARM
- ARM Compiler 6
- STM32U5 Device Family Pack
- ST-Link

## 工程入口

- CubeMX：`STM32U5_LVGL_Inteligentlight.ioc`
- Keil：`MDK-ARM/STM32U5_LVGL_Inteligentlight.uvprojx`

工程文件名保留现有拼写，以保持 CubeMX、Keil 和源码之间的相对路径关系。

## 编译与烧录

1. 使用 Keil 打开 `.uvprojx` 工程；
2. 确认芯片和 Device Pack；
3. 选择 ARM Compiler 6；
4. 执行 `Rebuild all target files`；
5. 连接 ST-Link；
6. 下载程序并复位开发板。

## 串口参数

```text
115200 baud
8 data bits
No parity
1 stop bit
```

USART1 用于输出系统启动与模块初始化信息。
