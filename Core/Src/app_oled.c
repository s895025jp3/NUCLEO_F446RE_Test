#include <stdio.h>

#include "app_oled.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"

void App_Oled_Init(void) {
   ssd1306_Init(); 
}

void App_Oled_TestDisplay(void) {
    ssd1306_Fill(Black); // White 點亮原本顏色、Black 關掉原本顏色
    ssd1306_WriteString("Hi", Font_11x18, White);
    ssd1306_UpdateScreen();
}

void App_Oled_TestColorBoundary(void) {
    ssd1306_Fill(Black);

    for (uint8_t y = 0; y < 64; y += 4) {
        ssd1306_Line(0, y, 127, y, White);
    }

    ssd1306_UpdateScreen();
}