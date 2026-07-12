#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "main.h"
#include "app_uart_cmd.h"
#include "app_button_led.h"

static uint8_t rx_buf[1];
static char line_buf[64];
static uint8_t line_len;
static char temp_char;

extern UART_HandleTypeDef huart2;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) // "override:在中斷服務程式（ISR）裡，執行接收字元" 2026/07/12 [ADD] by s895025.
{                                                       // 資料真正'進來'的時間點，只有 callback 知道
  HAL_UART_Receive_IT(huart, rx_buf, 1);
  if (rx_buf[0] != '\r' && rx_buf[0] != '\n')
  {
    if (line_len >= 63)
    {
      line_buf[line_len] = '\0';
    }
    else
    {
      line_buf[line_len] = rx_buf[0];
      line_len++;
    }
  }
  else
  {
    if ((rx_buf[0] == '\r') || (rx_buf[0] == '\n')) // 判斷是否為行結尾符號
    {
      if (rx_buf[0] == '\r')
        temp_char = rx_buf[0];

      if (!(temp_char == '\r' && rx_buf[0] == '\n')) // Windows：送 \r\n，Linux：送 \n
      {                                              // 處理連續送 \r\n 時，只有 \r 會印出內容，避免 \r\n 重複印出2次
        printf("Receive input string: ");
        for (int i = 0; i < line_len; i++)
        {
          printf("%c", line_buf[i]);
        }
        printf("\r\n");

        line_buf[line_len] = '\0';

        App_UART_ProcessCommand(line_len);

        line_len = 0;
      }
    }
  }
}

void App_Uart_Receive(void) // "啟動中斷式接收字元" 2026/07/12 [ADD] by s895025.
{
  HAL_UART_Receive_IT(&huart2, rx_buf, 1);
}

void App_UART_ProcessCommand(uint8_t length) // "判斷輸入字元並執行命令" 2026/07/12 [ADD] by s895025.
{
  for (int i = 0; i < length; i++)
  {
    line_buf[i] = tolower(line_buf[i]);
  }
  if (strcmp(line_buf, "led on") == 0)
  {
    App_ButtonLed_SetBlinking(1);
  }
  if (strcmp(line_buf, "led off") == 0)
  {
    App_ButtonLed_SetBlinking(0);
  }
}