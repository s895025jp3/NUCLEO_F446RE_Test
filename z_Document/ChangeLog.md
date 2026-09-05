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
| 2026/07/26 | ssd1306.c/.h, ssd1306_fonts.c/.h, ssd1306_conf.h | Add: Vendor SSD1306 OLED driver library (afiskon/stm32-ssd1306) | 1fa7fcd |
| 2026/07/26 | app_oled.c/.h, main.c, ssd1306_conf.h, .cproject | Add: Integrate SSD1306 OLED via app_oled module, display text test | aff2f76 |
| 2026/07/26 | app_oled.c/.h, main.c, ssd1306_conf.h | Add: OLED color-zone boundary test helper (yellow/blue split at y=16) | cc419e4 |
| 2026/07/26 | app_bmp180.c/.h, app_oled.c/.h, main.c, .cproject | Add: Display BMP180 temperature/pressure on OLED | 53e280a |
| 2026/08/01 | main.c, .ioc | Add: Enable SPI3, test SPI3 loopback communication | 7ad5462 |
| 2026/08/01 | main.c | Add: Send SD card CMD0 over SPI3, verify R1 idle response | 14e03e9 |
| 2026/08/01 | main.c | Add: Send SD card CMD8, verify R7 response (SDv2 detection) | 540e686 |
| 2026/08/01 | main.c | Add: Send ACMD41 + CMD58 over SPI3, complete SD card SPI init sequence (SDHC confirmed) | e297182 |
| 2026/08/08 | user_diskio.c, main.h | Refactor: Move SD card SPI init sequence into user_diskio.c USER_initialize() | 2ebbd31 |
| 2026/08/08 | user_diskio.c, main.c | Add: Implement USER_read() (CMD17) | 7d1586a |
| 2026/08/08 | user_diskio.c | Add: Implement USER_write() (CMD24) and USER_ioctl() (CTRL_SYNC), verify file read/write via FatFs | e8418f5 |
| 2026/08/08 | app_sdlog.c/.h, app_button_led.c, app_oled.h, main.c | Add: Implement SD card logging (app_sdlog), triggered by button + 5s timer | e85a56f |
| 2026/08/22 | main.c, .ioc, stm32f4xx_it.c/.h, stm32f4xx_hal_msp.c, STM32F446RETX_FLASH.ld | Add: Enable SPI3 DMA (RX Stream0/TX Stream5), verify Tx/Rx DMA complete via interrupt callback | 79750c2 |
| 2026/08/24 | user_diskio.c, main.c | Add: Convert USER_read/USER_write 512-byte SPI3 transfer to DMA, use TxRxCpltCallback for TransmitReceive_DMA completion | e96102d |
| 2026/09/05 | main.c, .ioc, FreeRTOSConfig.h, freertos.c, stm32f4xx_hal_timebase_tim.c, stm32f4xx_it.c/.h, stm32f4xx_hal_msp.c, user_diskio.c, Middlewares/FreeRTOS | Add: Enable FreeRTOS (CMSIS-V2, TIM6 timebase), verify scheduler via LED blink task | eaa10d7 |
