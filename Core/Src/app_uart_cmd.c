#include <stdio.h>

#include "main.h"
#include "app_uart_cmd.h"

static uint8_t rx_buf[2];
static uint8_t line_buf[64];

extern UART_HandleTypeDef huart2;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) // "override:在中斷服務程式（ISR）裡，執行收 2 個字元" 2026/07/11 [ADD] by s895025.
{
  printf("Receive input char: %c%c\r\n", rx_buf[0], rx_buf[1]); 
  HAL_UART_Receive_IT(huart, rx_buf, 2);
}

void App_UART_Receive(void) // "啟動中斷式接收，收 2 個字元" 2026/07/12 [ADD] by s895025.
{
  HAL_UART_Receive_IT(&huart2, rx_buf, 2);
}

