# 韌體修改記錄

## 修改摘要

| 日期 | 檔案 | 說明 | Commit |
|---|---|---|---|
| 2026/07/05 | main.c | Add: Add USART2 printf debug output and reorganize docs | 7d06839 |
| 2026/07/11 | main.c | Add: LED blink in main loop (blocking HAL_Delay) | 30a37a3 |
| 2026/07/11 | main.c | Add: LED toggle on button interrupt (HAL_GPIO_EXTI_Callback) | bca3a6b |
| 2026/07/11 | main.c | Add: Button toggles LED blink mode (blocking HAL_Delay) | fcd36ab |
| 2026/07/11 | main.c | Add: Button toggles LED blink mode (non-blocking HAL_GetTick) | c748033 |
| 2026/07/11 | main.c, app_button_led.c/.h | Refactor: Move button/LED logic into its own module | e01c881 |
