#ifndef APP_BMP180_H
#define APP_BMP180_H

#include <stdint.h>

extern uint32_t g_i2c_ok;
extern uint32_t g_i2c_fail;

void App_Bmp180_ReadData(void);
void App_Bmp180_ReadChipId(void);
void App_Bmp180_ReadCalibCoeffs(void);
void App_Bmp180_ReadCalibCoeffs_ALL(void);
void App_Bmp180_ReadTemperature(void);
void App_Bmp180_ReadPressure(void);
void App_Bmp180_Read_UTvalue(void);
void App_Bmp180_Calculate_Tvalue(void);
void App_Bmp180_Read_UPvalue(void);
void App_Bmp180_Calculate_Pvalue(void);
int32_t App_Bmp180_GetTemperature(void);
int32_t App_Bmp180_GetPressure(void);

#endif