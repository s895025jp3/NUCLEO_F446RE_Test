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
| 2026/07/11 | main.c, stm32f4xx_it.c/.h, stm32f4xx_hal_msp.c, .ioc | Add: USART2 interrupt receive (echo single char) | b670748 |
| 2026/07/12 | main.c, app_uart_cmd.c/.h | Refactor: Move UART receive logic into its own module | 91b15bc |
| 2026/07/12 | app_uart_cmd.c | Add: UART echoes full line (handles \r\n edge case) | 981b2c0 |
| 2026/07/12 | app_uart_cmd.c, app_button_led.c/.h | Add: UART commands control LED (led on/off, case-insensitive) | 99ffb9e |
| 2026/07/12 | main.c, .ioc, stm32f4xx_hal_msp.c | Add: I2C1 address scan (PB8/PB9, finds BMP180 at 0x77) | 13b6f05 |
| 2026/07/18 | app_bmp180.c/.h, main.h | Add: Read BMP180 chip-id from I2C1 (register 0xD0, confirms 0x55) | ac434c2 |
| 2026/07/18 | app_bmp180.c | Add: Read BMP180 AC1 calibration coefficient (0xAA/0xAB, MSB+LSB) | c673322 |
| 2026/07/18 | app_bmp180.c | Add: Read BMP180 calibration coefficients (11 params via struct) | 4418fad |
| 2026/07/18 | app_bmp180.c | Add: Read BMP180 uncompensated temperature (UT) from I2C1 | b355176 |
| 2026/07/19 | app_bmp180.c | Add: Calculate BMP180 true temperature | b13f086 |
| 2026/07/19 | app_bmp180.c | Add: Calculate BMP180 true pressure (fix B5 variable shadowing) | 6fe40cb |
| 2026/07/19 | main.c, app_uart_cmd.c | Add: Real-time UART echo and command prompt (>) | 483f19c |
