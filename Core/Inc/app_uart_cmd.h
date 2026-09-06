#ifndef APP_UART_CMD_H
#define APP_UART_CMD_H

#include <stdint.h>

void App_Uart_Receive(void);
void App_UART_ProcessCommand(uint8_t length);
void App_Uart_CmdTask_Poll(void);

#endif