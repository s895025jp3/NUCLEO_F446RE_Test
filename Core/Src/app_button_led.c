#include <stdio.h>

#include "main.h"
#include "app_button_led.h"
#include "app_sdlog.h"
#include "cmsis_os.h"

extern osSemaphoreId_t ButtonSemHandle;  // 定義在 main.c，由 CubeMX 產生

static volatile uint8_t s_led_blinking = 0; // 加上 static，外部無法存取
static volatile uint32_t s_last_time = 0; // 加上 volatile，ButtonTask(xISR) 寫入，避免主迴圈讀到快取的舊值

// [DEBUG] 量測彈跳用，測完可刪除
static volatile uint32_t s_isr_count   = 0; // EXTI 實際觸發次數
static volatile uint32_t s_press_count = 0; // 防彈跳後確認的有效按壓次數

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_pin) // "在中斷服務程式（ISR）裡，執行閃爍，現在 ISR 只負責 give" 2026/11/01 [MOD] by s895025.
{
  if (GPIO_pin == GPIO_PIN_13) // GPIO_PIN_13 參考 Drivers\BSP\STM32F4xx-Nucleo\stm32f4xx_nucleo.h
  {
    s_isr_count++; // [DEBUG]
    osSemaphoreRelease(ButtonSemHandle);
  }
}

void App_ButtonLed_Update(void) // "主迴圈每輪呼叫，依 flag 決定 LED 閃爍或關閉" 2026/07/11 [ADD] by s895025.
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

void App_ButtonLed_Toggle(void)   // 給 LedTask 用，週期由 osDelay 決定
{
  if (s_led_blinking == 1)  BSP_LED_Toggle(LED2);
  else                      BSP_LED_Off(LED2);
}


void App_ButtonLed_SetBlinking(uint8_t LedState) // "供外部 UART 指令，設定閃爍模式" 2026/07/12 [ADD] by s895025.
{
  if (LedState == 0)
    s_led_blinking = 0;
  else if (LedState == 1)
    s_led_blinking = 1;
}


void App_ButtonLed_OnPress(void) // "給 ButtonTask 呼叫，確認按下後才處理" 2026/11/01 [ADD] by s895025.
{
  s_led_blinking = !s_led_blinking;
  if (s_led_blinking == 1)
    s_last_time = HAL_GetTick(); // 重設時間戳，避免切換瞬間立刻觸發閃爍

  App_SdLog_RequestWrite();
  s_press_count++; // [DEBUG]
}

void App_ButtonLed_DebugPrint(void) // [DEBUG] 計數有變化才印，由 UartCmdTask 呼叫 (ButtonTask stack 太小，不在那裡 printf)
{
  static uint32_t s_printed_isr = 0;
  uint32_t isr = s_isr_count;

  if (isr != s_printed_isr)
  {
    s_printed_isr = isr;
    printf("[BTN] EXTI=%lu, Press=%lu\r\n", isr, s_press_count);
  }
}