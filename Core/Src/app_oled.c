#include <stdio.h>

#include "app_oled.h"
#include "app_bmp180.h"

#include "ssd1306.h"
#include "ssd1306_fonts.h"

#define OLED_COLOR_ZONE_Y   16

void App_Oled_Init(void) {
   ssd1306_Init(); 
}

void App_Oled_TestDisplay(void) {
    ssd1306_Fill(Black); // White 點亮原本顏色、Black 關掉原本顏色
    ssd1306_WriteString("Hi", Font_11x18, White);
    ssd1306_UpdateScreen();
}

// OLED color-zone boundary test (yellow/blue split at y=16)
void App_Oled_TestColorBoundary(void) {
    ssd1306_Fill(Black);

    for (uint8_t y = 0; y < 64; y += 4) {
        ssd1306_Line(0, y, 127, y, White);
    }

    ssd1306_UpdateScreen();
}

void App_Oled_UpdateWeather(float temperature, float pressure) {
    char buf[16];

    ssd1306_Fill(Black); // 清空畫面(全部像素熄滅),避免殘留上一次顯示的內容
   
    // 黃色區標題
    ssd1306_SetCursor(0, 0); // 黃色區第一行,x=0 靠左對齊,y=0
    ssd1306_WriteString("Weather", Font_6x8, White); // 6x8 字型、White 點亮筆畫

    // 藍色區溫度
    sprintf(buf, "T: %.1f C", temperature);
    ssd1306_SetCursor(0, 20); // 藍色區第一行,x=0 靠左對齊,y=20(避開黃藍分界 y=16, 留間距)
    ssd1306_WriteString(buf, Font_7x10, White); // 7x10 字型、White 點亮筆畫

    // 藍色區氣壓
    sprintf(buf, "P: %.1f hPa", pressure);
    ssd1306_SetCursor(0, 40);
    ssd1306_WriteString(buf, Font_7x10, White);

    ssd1306_UpdateScreen(); // 將緩衝區內容透過 I2C 一次送出, 畫面更新顯示
}

void App_Oled_ReadData(void) {
    int32_t raw_temperature = App_Bmp180_GetTemperature();
    int32_t raw_pressure = App_Bmp180_GetPressure();

    App_Oled_UpdateWeather(raw_temperature/10.f, raw_pressure/100.f);
}