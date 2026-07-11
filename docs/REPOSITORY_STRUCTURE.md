# 仓库结构

```text
stm32u5-smartwatch/
├─ .github/workflows/             GitHub 源码完整性检查
├─ Core/
│  ├─ Inc/                        应用与 BSP 头文件
│  ├─ Src/                        主程序、RTOS、业务模块、BSP
│  ├─ cJSON/                      JSON 库，当前业务未形成完整联网闭环
│  └─ esp8266/                    ESP8266 旧版代码资产，当前未接入主业务
├─ Drivers/                       CMSIS 与 STM32U5 HAL
├─ LVGL/
│  ├─ UI/                         SquareLine 生成的五页 UI
│  ├─ porting/                    显示和输入设备端口
│  └─ src/                        LVGL 8.3 源码
├─ MDK-ARM/                       Keil 工程文件，不含编译输出
├─ Middlewares/                   FreeRTOS 与 CMSIS-RTOS2
├─ docs/                          架构、构建、状态、审查和路线
├─ scripts/                       仓库验证和 Git 推送脚本
├─ STM32U5_LVGL_Inteligentlight.ioc
├─ README.md
├─ CHANGELOG.md
├─ .gitignore
└─ .gitattributes
```

## 为什么不把代码移动到 `firmware/`

原 Keil 工程中已有大量相对路径。直接把 `Core/`、`Drivers/`、`LVGL/` 等目录整体移动到二级目录，会造成 297 个工程引用路径失效。因此本版本优先保证工程可打开、可继续编译，而不是为了目录“好看”进行破坏性搬迁。

## 未包含内容

- Keil `Objects/Listings` 和历史 `.axf/.hex/.map`
- LVGL 官方 demos
- 本机用户配置
- 独立原理图与 PCB 工程

硬件资料应后续放入单独的 `hardware/` 目录，但不要在未提供真实文件时创建虚假的原理图占位内容。
