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

static volatile uint8_t s_lineReady = 0;   // ISR 寫、task 讀 → 必須 volatile
//static uint8_t s_echo_idx = 0;             // 只有 task 用，不用 volatile

extern UART_HandleTypeDef huart2;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) // "override:在中斷服務程式（ISR）裡，執行接收字元" 2026/07/12 [ADD] by s895025.
{                                                       // 資料真正'進來'的時間點，只有 callback 知道
  HAL_UART_Receive_IT(huart, rx_buf, 1);

  if (s_lineReady) return; // task 還沒處理完上一行，這些字元先丟掉

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
      // printf("%c", rx_buf[0]);
      // fflush(stdout);
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
        //printf("\nReceive input string: ");
        // for (int i = 0; i < line_len; i++)
        // {
        //   printf("%c", line_buf[i]);
        // }
        // printf("\r\n");

        line_buf[line_len] = '\0';
        s_lineReady = 1; // 取代搬走的那 5 行

        // App_UART_ProcessCommand(line_len);
        temp_char = 0;
        //line_len = 0;
        // printf("> ");
      }
    }
  }
}

void App_Uart_CmdTask_Poll(void)
{
  if (!s_lineReady) return;

  printf("\nReceive input string: ");
  for (int i = 0; i < line_len; i++)
  {
    printf("%c", line_buf[i]);
  }
  printf("\r\n");

  App_UART_ProcessCommand(line_len);

  line_len = 0;
  s_lineReady = 0;
  printf("> ");
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