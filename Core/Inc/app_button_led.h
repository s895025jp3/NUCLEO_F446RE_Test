#ifndef APP_BUTTON_LED_H
#define APP_BUTTON_LED_H

#include <stdint.h>

void App_ButtonLed_Update(void);
void App_ButtonLed_Toggle(void);
void App_ButtonLed_SetBlinking(uint8_t LedState);
void App_ButtonLed_OnPress(void);
void App_ButtonLed_DebugPrint(void); // [DEBUG]

#endif