# Changelog

## v0.3.0-alpha-review — 2026-07-11

- 审核并重新标注各模块真实完成度。
- 修复 FT6336 与 SHT20/MAX30102 共用 I²C1 时的并发访问风险。
- 将 FT6336 触点寄存器读取合并为一次连续事务。
- 修复横屏触摸坐标边界越界。
- LVGL Tick 改为按真实 RTOS Tick 推进。
- SensorTask 与 ClockApp 改为绝对周期调度。
- 增加项目 README、功能状态表、V3 审查报告和 Git 忽略规则。
