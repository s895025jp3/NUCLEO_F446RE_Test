#include <stdio.h>

#include "main.h"
#include "app_bmp180.h"

HAL_StatusTypeDef Bmp180_ChipId_status;
HAL_StatusTypeDef Bmp180_CalibCoeffs_status;

uint8_t Bmp180_ChipId;

uint8_t  Bmp180_AC1_raw8[2];
int16_t  Bmp180_AC1_raw16;

void App_Bmp180_ReadData()
{
    App_Bmp180_ReadChipId();
    App_Bmp180_ReadCalibCoeffs();
}

void App_Bmp180_ReadChipId()
{
    Bmp180_ChipId_status = HAL_I2C_Mem_Read(&hi2c1, 0x77 << 1, 0xD0, I2C_MEMADD_SIZE_8BIT, &Bmp180_ChipId, 1, 100);// 讀取 Register Address 的 reset state(唯讀,用於驗證通訊)
    if (Bmp180_ChipId_status == HAL_OK)                                                                            // I2C_MEMADD_SIZE_8BIT:存取的暫存器位址大小
    {                                                                                                              // 1:要讀的資料內容大小        
        printf("Bmp180 chip-id: %02X\r\n", Bmp180_ChipId);
    }
}

void App_Bmp180_ReadCalibCoeffs()
{
    Bmp180_CalibCoeffs_status = HAL_I2C_Mem_Read(&hi2c1, 0x77 << 1, 0xAA, I2C_MEMADD_SIZE_8BIT, Bmp180_AC1_raw8, 2, 100);// 讀取 AC1 校正係數(0xAA/0xAB, MSB/LSB, 唯讀)
    
    if (Bmp180_CalibCoeffs_status == HAL_OK)
    {
        Bmp180_AC1_raw16 = Bmp180_AC1_raw8[0]<<8 | Bmp180_AC1_raw8[1];
        printf("Bmp180 calib-coeffs: %04X\r\n", Bmp180_AC1_raw16);
    }
}