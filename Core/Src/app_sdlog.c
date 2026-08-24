#include <stdio.h>   // 如果要printf除錯訊息

#include "app_sdlog.h"
#include "fatfs.h"   // 取得 USERFatFS, USERPath 這兩個CubeMX生成的變數
#include "app_bmp180.h"
#include "app_oled.h"

static volatile uint8_t s_writeRequested = 0;

void App_SdLog_RequestWrite(void)
{
  s_writeRequested = 1;
}

void App_SdLog_Update(void)
{
    static uint32_t s_lastLogTime = 0;

    if (s_writeRequested || (HAL_GetTick() - s_lastLogTime >= 5000))
    {
        s_writeRequested = 0;
        s_lastLogTime = HAL_GetTick();
        App_SdLog_WriteOnce();
    }
}

void App_SdLog_Init(void) // f_mount
{
    FRESULT fr = f_mount(&USERFatFS, USERPath, 1);
    printf("SD log init: f_mount=%d\r\n", fr);

    // 驗證 寫進 SD 卡的資料是不是真的正確地被永久保存下來
    // FIL f;
    // if (f_open(&f, "log3.csv", FA_READ) == FR_OK) {
    //     char line[64];
    //     while (f_gets(line, sizeof(line), &f)) {
    //         printf("%s", line);
    //     }
    //     f_close(&f);
    // }
}

void App_SdLog_WriteOnce(void) // 讀BMP180 → 更新OLED → 開檔append寫入 → 關檔
{
    FIL file;
    FRESULT fr;

    App_Bmp180_ReadData(); // 讀取 Bmp180

    // 給 OLED 用: 要浮點數才能顯示小數
    float temp = App_Bmp180_GetTemperature() / 10.f;
    float pres = App_Bmp180_GetPressure() / 100.f;
    App_Oled_UpdateWeather(temp, pres);

    // 給寫檔用: FatFs的f_printf不支援%f, 直接用原始整數手動拆整數/小數部分
    int32_t rawTemp = App_Bmp180_GetTemperature();
    int32_t rawPres = App_Bmp180_GetPressure(); 

    // 開檔append寫入
    fr = f_open(&file, "log3.csv", FA_OPEN_APPEND | FA_WRITE);
    printf("f_open(write): %d\r\n", fr);

    //fr = f_printf(&file, "%lu,%.1f,%.1f\r\n", HAL_GetTick(), temp, pres); // FatFs 的 f_printf 不支援 %f
    fr = f_printf(&file, "%lu,%d.%d,%d.%d\r\n", HAL_GetTick(),
              rawTemp / 10, rawTemp % 10,
              rawPres / 100, rawPres % 100);

    fr = f_close(&file);
    printf("f_close: %d\r\n", fr);

    // DMA 讀回驗證: 重新打開同一個檔案, 讀出最後寫入的內容, 印出來肉眼比對
    // FIL rfile;
    // if (f_open(&rfile, "log3.csv", FA_READ) == FR_OK) {
    //     f_lseek(&rfile, f_size(&rfile) > 100 ? f_size(&rfile) - 100 : 0); // 跳到檔案接近結尾處(避免從頭讀整個大檔案)
    //     char rbuf[128];
    //     UINT br;
    //     f_read(&rfile, rbuf, sizeof(rbuf) - 1, &br);
    //     rbuf[br] = '\0';
    //     printf("Readback (last bytes): %s\r\n", rbuf);
    //     f_close(&rfile);
    // }
}