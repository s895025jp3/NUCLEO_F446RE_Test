#ifndef APP_SDLOG_H
#define APP_SDLOG_H

void App_SdLog_Init(void);          // 開機時掛載SD卡+建立/開啟log檔(或每次寫入時開關)
void App_SdLog_WriteOnce(void);     // 讀BMP180 → 更新OLED → 寫一行log
void App_SdLog_RequestWrite(void);  // 供中斷呼叫, 只設旗標
void App_SdLog_Update(void);        // 供主迴圈呼叫, 檢查旗標決定要不要真的寫
#endif