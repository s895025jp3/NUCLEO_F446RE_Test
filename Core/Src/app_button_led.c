#include <stdio.h>

#include "main.h"
#include "app_button_led.h"

static volatile uint8_t s_led_blinking = 0; // 加上 static，外部無法存取
static volatile uint32_t s_last_time = 0; // 加上 volatile，ISR 也會寫入，避免主迴圈讀到快取的舊值

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_pin) // 在中斷服務程式（ISR）裡，執行閃爍
{
  if (GPIO_pin == GPIO_PIN_13) // GPIO_PIN_13 參考 Drivers\BSP\STM32F4xx-Nucleo\stm32f4xx_nucleo.h
  {
    s_led_blinking = !s_led_blinking;
    if (s_led_blinking == 1)
      s_last_time = HAL_GetTick(); // 重設時間戳，避免切換瞬間立刻觸發閃爍
  }
}

void App_ButtonLed_Init(void)
{
  /* Initialize leds */
  BSP_LED_Init(LED2);

  /* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI); // BUTTON_USER:B1
}

void App_ButtonLed_Update(void)
{
   if (s_led_blinking == 1)
  {
    if (HAL_GetTick() - s_last_time >= 500)
    {
      s_last_time = HAL_GetTick();
      BSP_LED_Toggle(LED2);
    }
  }
  else
  {
    BSP_LED_Off(LED2);
  }
}

