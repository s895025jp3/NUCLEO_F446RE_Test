#include <stdio.h>

#include "main.h"

HAL_StatusTypeDef Bmp180_status; 
uint8_t Bmp180_value; 


void App_Bmp180_ReadChipId()
{
    Bmp180_status = HAL_I2C_Mem_Read(&hi2c1, 0x77 << 1, 0xD0, I2C_MEMADD_SIZE_8BIT, &Bmp180_value, 1, 100);// 讀取 Register Address 的 reset state(唯讀,用於驗證通訊)
    if (Bmp180_status == HAL_OK)
    {
        printf("Bmp180 chip-id: %02X\r\n", Bmp180_value);
    }
}
