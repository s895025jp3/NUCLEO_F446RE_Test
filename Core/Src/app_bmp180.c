#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "app_bmp180.h"

uint16_t Bmp180_UT_buf16;

typedef struct
{
    int16_t Bmp180_AC1;
    int16_t Bmp180_AC2;
    int16_t Bmp180_AC3;
    uint16_t Bmp180_AC4;
    uint16_t Bmp180_AC5;
    uint16_t Bmp180_AC6;
    int16_t Bmp180_B1;
    int16_t Bmp180_B2;
    int16_t Bmp180_MB;
    int16_t Bmp180_MC;
    int16_t Bmp180_MD;
} Bmp180_raw_calibcoeffs;
Bmp180_raw_calibcoeffs calib_data;

void App_Bmp180_ReadData()
{
    App_Bmp180_ReadChipId();
    //App_Bmp180_ReadCalibCoeffs();
    App_Bmp180_ReadCalibCoeffs_ALL();
    App_Bmp180_Read_UTvalue();
    App_Bmp180_Calculate_Tvalue();
}

void App_Bmp180_ReadChipId()
{
    uint8_t Bmp180_ChipId;
    HAL_StatusTypeDef Bmp180_ChipId_status;

    Bmp180_ChipId_status = HAL_I2C_Mem_Read(&hi2c1, 0x77 << 1, 0xD0, I2C_MEMADD_SIZE_8BIT, &Bmp180_ChipId, 1, 100);// 讀取 Register Address 的 reset state(唯讀,用於驗證通訊)
    if (Bmp180_ChipId_status == HAL_OK)                                                                            // I2C_MEMADD_SIZE_8BIT:存取的暫存器位址大小
    {                                                                                                              // 1:要讀的資料內容大小        
        printf("Bmp180 chip-id: %02X\r\n", Bmp180_ChipId);
    }
}

void App_Bmp180_ReadCalibCoeffs()
{
    uint8_t  Bmp180_AC1_raw8[2];
    int16_t  Bmp180_AC1_16;
    HAL_StatusTypeDef Bmp180_CalibCoeffs_status;

    Bmp180_CalibCoeffs_status = HAL_I2C_Mem_Read(&hi2c1, 0x77 << 1, 0xAA, I2C_MEMADD_SIZE_8BIT, Bmp180_AC1_raw8, 2, 100);// 讀取 AC1 校正係數(0xAA/0xAB, MSB/LSB, 唯讀)
    
    if (Bmp180_CalibCoeffs_status == HAL_OK)
    {
        Bmp180_AC1_16 = Bmp180_AC1_raw8[0]<<8 | Bmp180_AC1_raw8[1];
        printf("Bmp180 calib-coeffs: %04X\r\n", Bmp180_AC1_16);
    }
}

void App_Bmp180_ReadCalibCoeffs_ALL()
{
    uint8_t  Bmp180_raw_buf[22];
    HAL_StatusTypeDef Bmp180_CalibCoeffs_status_all;

    Bmp180_CalibCoeffs_status_all = HAL_I2C_Mem_Read(&hi2c1, 0x77 << 1, 0xAA, I2C_MEMADD_SIZE_8BIT, Bmp180_raw_buf, 22, 100);// 讀取 全部 校正係數(0xAA/0xBF, MSB/LSB, 唯讀)
    
    if (Bmp180_CalibCoeffs_status_all == HAL_OK)
    {
        calib_data.Bmp180_AC1 = Bmp180_raw_buf[0]<<8 | Bmp180_raw_buf[1];
        calib_data.Bmp180_AC2 = Bmp180_raw_buf[2]<<8 | Bmp180_raw_buf[3];
        calib_data.Bmp180_AC3 = Bmp180_raw_buf[4]<<8 | Bmp180_raw_buf[5];
        calib_data.Bmp180_AC4 = Bmp180_raw_buf[6]<<8 | Bmp180_raw_buf[7];
        calib_data.Bmp180_AC5 = Bmp180_raw_buf[8]<<8 | Bmp180_raw_buf[9];
        calib_data.Bmp180_AC6 = Bmp180_raw_buf[10]<<8 | Bmp180_raw_buf[11];
        calib_data.Bmp180_B1 = Bmp180_raw_buf[12]<<8 | Bmp180_raw_buf[13];
        calib_data.Bmp180_B2 = Bmp180_raw_buf[14]<<8 | Bmp180_raw_buf[15];
        calib_data.Bmp180_MB = Bmp180_raw_buf[16]<<8 | Bmp180_raw_buf[17];
        calib_data.Bmp180_MC = Bmp180_raw_buf[18]<<8 | Bmp180_raw_buf[19];
        calib_data.Bmp180_MD = Bmp180_raw_buf[20]<<8 | Bmp180_raw_buf[21];
   
        // printf("Bmp180_AC1= %04X\r\n",  calib_data.Bmp180_AC1);
        // printf("Bmp180_AC2= %04X\r\n",  calib_data.Bmp180_AC2);
        // printf("Bmp180_AC3= %04X\r\n",  calib_data.Bmp180_AC3);
        // printf("Bmp180_AC4= %04X\r\n",  calib_data.Bmp180_AC4);
        // printf("Bmp180_AC5= %04X\r\n",  calib_data.Bmp180_AC5);
        // printf("Bmp180_AC6= %04X\r\n",  calib_data.Bmp180_AC6);
        // printf("Bmp180_B1= %04X\r\n",  calib_data.Bmp180_B1);
        // printf("Bmp180_B2= %04X\r\n",  calib_data.Bmp180_B2);
        // printf("Bmp180_MB= %04X\r\n",  calib_data.Bmp180_MB);
        // printf("Bmp180_MC= %04X\r\n",  calib_data.Bmp180_MC);
        // printf("Bmp180_MD= %04X\r\n",  calib_data.Bmp180_MD);
    }
}

void App_Bmp180_Read_UTvalue()
{
    uint8_t Bmp180_UT_buf8[2];
    uint8_t Bmp180_UT_cmd = 0x2E;

    HAL_I2C_Mem_Write(&hi2c1, 0x77 << 1, 0xF4, I2C_MEMADD_SIZE_8BIT, &Bmp180_UT_cmd, 1, 100);// 寫入 Uncompensated Temperature value
    HAL_Delay(5);
    HAL_I2C_Mem_Read(&hi2c1, 0x77 << 1, 0xF6, I2C_MEMADD_SIZE_8BIT, Bmp180_UT_buf8, 2, 100);// 讀取 Uncompensated Temperature value
    
    Bmp180_UT_buf16 = Bmp180_UT_buf8[0]<<8 | Bmp180_UT_buf8[1];
    printf("Bmp180 Uncompensated Temperature= %04X\r\n", Bmp180_UT_buf16);
}

void App_Bmp180_Calculate_Tvalue()
{
    // 計算實際溫度公式
    // X1 = (UT - AC6) * AC5 / 2^15
    // X2 = MC * 2^11 / (X1 + MD)
    // B5 = X1 + X2
    // T  = (B5 + 8) / 2^4 (單位是 0.1°C)

    int32_t X1, X2 , B5, T; // int32_t = long, 寫 int32_t 更明確 

    X1 = ((Bmp180_UT_buf16 - calib_data.Bmp180_AC6) * calib_data.Bmp180_AC5) / (1 << 15);
    X2 = (calib_data.Bmp180_MC * (1 << 11)) / (X1 + calib_data.Bmp180_MD);
    B5 = X1 + X2 ;
    T = (B5 + 8) / (1 << 4);
    if (T > 0)
        printf("Bmp180 Temperature= %d => %d.%d °C\r\n", T, T/10, T%10);
    else
        printf("Bmp180 Temperature= %d => -%d.%d °C\r\n", T, abs(T)/10, abs(T)%10);
}