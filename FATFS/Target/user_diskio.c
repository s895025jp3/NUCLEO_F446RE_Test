/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * @file    user_diskio.c
  * @brief   This file includes a diskio driver skeleton to be completed by the user.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
 /* USER CODE END Header */

#ifdef USE_OBSOLETE_USER_CODE_SECTION_0
/*
 * Warning: the user section 0 is no more in use (starting from CubeMx version 4.16.0)
 * To be suppressed in the future.
 * Kept to ensure backward compatibility with previous CubeMx versions when
 * migrating projects.
 * User code previously added there should be copied in the new user sections before
 * the section contents can be deleted.
 */
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */
#endif

/* USER CODE BEGIN DECL */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "ff_gen_drv.h"

#include <stdio.h> // "為了 使用 printf 進行序列埠除錯輸出" 2026/07/05 [ADD] by s895025.
#include "main.h"
#include "fatfs.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;
static uint8_t CardType;

/* CS 控制小工具 ---------------------------------------------------------------
   SD 卡規格要求每次 CS 狀態改變後要多送 8 個 clock(1 個 0xFF), 卡片才有時間
   把 DO 腳驅動起來(拉低前)或釋放成高阻抗(拉高後)。少了這個 dummy byte, 單獨
   呼叫一次通常還是會成功, 但連續多次 transaction(例如 f_close 會連續寫 FAT
   table + 目錄項目)就容易失敗。 */
static void SD_Deselect(void)
{
  uint8_t tx_dummy = 0xFF;
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);
  HAL_SPI_Transmit(&hspi3, &tx_dummy, 1, HAL_MAX_DELAY); // CS拉高後補8個clock, 讓卡片釋放DO
}

static void SD_Select(void)
{
  uint8_t tx_dummy = 0xFF;
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi3, &tx_dummy, 1, HAL_MAX_DELAY); // CS拉低後補8個clock, 讓卡片把DO驅動起來
}

/* 等卡片脫離 busy: 卡片忙的時候 MISO 持續輸出 0x00, 讀到 0xFF 才代表可以接受
   下一個指令。送指令前一定要先等, 否則前一次寫入還沒結束就送新指令會被忽略。 */
static int SD_WaitReady(uint32_t timeout_ms)
{
  uint8_t tx_dummy = 0xFF;
  uint8_t rx = 0x00;
  uint32_t start = HAL_GetTick();

  do {
    HAL_SPI_TransmitReceive(&hspi3, &tx_dummy, &rx, 1, HAL_MAX_DELAY);
    if (rx == 0xFF) return 1; // 卡片已就緒
  } while (HAL_GetTick() - start < timeout_ms);

  return 0; // 逾時, 卡片還在忙
}

/* USER CODE END DECL */

/* Private function prototypes -----------------------------------------------*/
DSTATUS USER_initialize (BYTE pdrv);
DSTATUS USER_status (BYTE pdrv);
DRESULT USER_read (BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
#if _USE_WRITE == 1
  DRESULT USER_write (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
  DRESULT USER_ioctl (BYTE pdrv, BYTE cmd, void *buff);
#endif /* _USE_IOCTL == 1 */

Diskio_drvTypeDef  USER_Driver =
{
  USER_initialize,
  USER_status,
  USER_read,
#if  _USE_WRITE
  USER_write,
#endif  /* _USE_WRITE == 1 */
#if  _USE_IOCTL == 1
  USER_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes a Drive
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_initialize (
	BYTE pdrv           /* Physical drive nmuber to identify the drive */
)
{
  /* USER CODE BEGIN INIT */
  // SPI
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET); // CS 拉高: 卡片尚未被選取, 才能送 dummy clock
  uint8_t dummy[10];
  memset(dummy, 0xFF, sizeof(dummy));
  HAL_SPI_Transmit(&hspi3, dummy, 10, HAL_MAX_DELAY); // 送≥74個clock(10 byte=80 clock), 讓卡片完成開機、切換到SPI mode

  SD_Select(); // CS拉低: 選取卡片, 準備送指令

  uint8_t cmd0[6] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95}; // CMD0 (GO_IDLE_STATE): 0x40=start+cmd index0, 中間4 byte固定填0(無參數), 0x95=cmd0專用CRC7+stop bit
  HAL_SPI_Transmit(&hspi3, cmd0, 6, HAL_MAX_DELAY);

  uint8_t tx_dummy = 0xFF;
  uint8_t r1_cmd0 = 0xFF;
  for (int i = 0; i < 8; i++){
      HAL_SPI_TransmitReceive(&hspi3, &tx_dummy, &r1_cmd0, 1, HAL_MAX_DELAY); // 送出 0xFF 當作 dummy byte, 同時讀取卡片透過 MISO 回傳 cmd0 的 response byte
      if (r1_cmd0 != 0xFF) break; // 收到非 0xFF, 代表卡片已回應
  }

  //if (r1_cmd0 != 0x01) printf("CMD0 failed: %02X\r\n", r1_cmd0);
  //printf("CMD0=%02X\n", r1_cmd0);

  uint8_t cmd8[6] = {0x48, 0x00, 0x00, 0x01, 0xAA, 0x87};
  HAL_SPI_Transmit(&hspi3, cmd8, 6, HAL_MAX_DELAY);

  uint8_t r1_cmd8 = 0xFF;
  uint8_t cmd8_echo[4];
  for (int j = 0; j < 8; j++){
      HAL_SPI_TransmitReceive(&hspi3, &tx_dummy, &r1_cmd8, 1, HAL_MAX_DELAY); // 回傳cmd8的 response byte
      if (r1_cmd8 != 0xFF) break;
  }
  HAL_SPI_TransmitReceive(&hspi3, dummy, cmd8_echo, 4, HAL_MAX_DELAY); // 同時讀取卡片透過 MISO 回傳的 response 4 byte

  //printf("CMD8=%02X\n", r1_cmd8);
  //for (int i = 0; i < 4; i++) printf("%02X ", cmd8_echo[i]);
  //printf("\r\n");

  uint8_t r1_cmd41 = 0xFF;
  uint8_t r1_cmd55 = 0xFF;
  for (int k = 0; k < 1000; k++) { // 外層: 重試整組 CMD55+CMD41
    uint8_t cmd55[6] ={0x77, 0x00, 0x00, 0x00, 0x00, 0x01};
    HAL_SPI_Transmit(&hspi3, cmd55, 6, HAL_MAX_DELAY);

    for (int m = 0; m < 8; m++){
        HAL_SPI_TransmitReceive(&hspi3, &tx_dummy, &r1_cmd55, 1, HAL_MAX_DELAY); // 回傳cmd55的 response byte
        if (r1_cmd55 != 0xFF) break;
    }

    uint8_t cmd41[6] ={0x69, 0x40, 0x00, 0x00, 0x00, 0x01};
    HAL_SPI_Transmit(&hspi3, cmd41, 6, HAL_MAX_DELAY);


    for (int n = 0; n < 8; n++){
        HAL_SPI_TransmitReceive(&hspi3, &tx_dummy, &r1_cmd41, 1, HAL_MAX_DELAY); // 回傳cmd41的 response byte
        if (r1_cmd41 != 0xFF) break;
    }

    if (r1_cmd41 == 0x00) break;  // 卡片離開idle, 初始化完成

    //printf("ACMD41=%02X (retry %d)\r\n", r1_cmd41, k); // 這次重試失敗, 印出目前狀態繼續下一輪

  }
  //printf("ACMD41 final=%02X\r\n", r1_cmd41);

  uint8_t cmd58[6] = {0x7A, 0x00, 0x00, 0x00, 0x00, 0x01};
  HAL_SPI_Transmit(&hspi3, cmd58, 6, HAL_MAX_DELAY);

  uint8_t r1_cmd58 = 0xFF;
  uint8_t cmd58_echo[4];
  for (int x = 0; x < 8; x++){
      HAL_SPI_TransmitReceive(&hspi3, &tx_dummy, &r1_cmd58, 1, HAL_MAX_DELAY); // 回傳cmd58的 response byte
      if (r1_cmd58 != 0xFF) break;
  }
  HAL_SPI_TransmitReceive(&hspi3, dummy, cmd58_echo, 4, HAL_MAX_DELAY); // 同時讀取卡片透過 MISO 回傳的 response 4 byte
  //printf("CMD58=%02X\n", r1_cmd58);
  
  if (cmd58_echo[0] & 0x40) {
    CardType = 2;  // CCS=1, SDHC/SDXC
  } else {
    CardType = 1;  // CCS=0, SDSC
  }

  //for (int i = 0; i < 4; i++) printf("%02X ", cmd58_echo[i]);
  //printf("\r\n");

  SD_Deselect();

  if (r1_cmd0 == 0x01 && r1_cmd8 == 0x01 && r1_cmd41 == 0x00 && r1_cmd58 == 0x00)
  {
    Stat = 0; // 全部成功, 清除 STA_NOINIT, 代表磁碟已就緒
    printf("SD card init: OK (CardType=%d)\r\n", CardType);
  } else {
    Stat = STA_NOINIT;
    printf("SD card init: FAILED (CMD0=%02X CMD8=%02X ACMD41=%02X CMD58=%02X)\r\n",
           r1_cmd0, r1_cmd8, r1_cmd41, r1_cmd58);
  }

  return Stat;
  /* USER CODE END INIT */
}

/**
  * @brief  Gets Disk Status
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_status (
	BYTE pdrv       /* Physical drive number to identify the drive */
)
{
  /* USER CODE BEGIN STATUS */
    return Stat;
  /* USER CODE END STATUS */
}

/**
  * @brief  Reads Sector(s)
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT USER_read (
	BYTE pdrv,      /* Physical drive nmuber to identify the drive */
	BYTE *buff,     /* Data buffer to store read data */
	DWORD sector,   /* Sector address in LBA */
	UINT count      /* Number of sectors to read */
)
{
  /* USER CODE BEGIN READ */

  SD_Select(); // CS拉低: 選取卡片, 準備送指令

  if (!SD_WaitReady(500)) { // 送指令前先確認卡片不在busy狀態
    SD_Deselect();
    printf("USER_read: card busy before CMD17\r\n");
    return RES_ERROR;
  }

  // SDHC/SDXC(CardType==2) 直接用 sector number 定址
  // SDSC(CardType==1) 要換算成 byte 位址(sector * 512)
  uint32_t addr_cmd17 = (CardType == 2) ? sector : sector * 512;

  uint8_t cmd17[6];
  cmd17[0] = 0x51;
  cmd17[1] = (uint8_t)(addr_cmd17 >> 24);
  cmd17[2] = (uint8_t)(addr_cmd17 >> 16);
  cmd17[3] = (uint8_t)(addr_cmd17 >> 8);
  cmd17[4] = (uint8_t)(addr_cmd17);
  cmd17[5] = 0x01;

  // 第一段: 讀 R1 (1 byte)
  uint8_t tx_dummy = 0xFF;
  uint8_t r1_cmd17 = 0xFF;

  HAL_SPI_Transmit(&hspi3, cmd17, 6, HAL_MAX_DELAY);

  for (int i = 0; i < 8; i++){
    HAL_SPI_TransmitReceive(&hspi3, &tx_dummy, &r1_cmd17, 1, HAL_MAX_DELAY); // 送出 0xFF 當作 dummy byte, 同時讀取卡片透過 MISO 回傳 cmd17 的 response byte
    if (r1_cmd17 != 0xFF) break; // 收到非 0xFF, 代表卡片已回應
  }
  
  if (r1_cmd17 != 0x00) {
    SD_Deselect();
    return RES_ERROR;
  }
  // 成功的話不動CS, 繼續往下走

  // 第二段: 等 Data Start Token (0xFE), 重試次數要多一點
  uint8_t token_read = 0xFF;

  for (int i = 0; i < 1000; i++){
    HAL_SPI_TransmitReceive(&hspi3, &tx_dummy, &token_read, 1, HAL_MAX_DELAY);
    if (token_read != 0xFF) break;
  }

  if (token_read != 0xFE) {
    SD_Deselect();
    return RES_ERROR;
  }
  
  // 第三段: 讀真正的 512 byte 資料進 buff
  uint8_t dummy_512[512];
  memset(dummy_512, 0xFF, sizeof(dummy_512));

  HAL_SPI_TransmitReceive(&hspi3, dummy_512, buff, 512, HAL_MAX_DELAY);

  // 讀2 byte CRC丟棄, CS拉高, return RES_OK
  uint8_t crc_read[2];
  HAL_SPI_TransmitReceive(&hspi3, dummy_512, crc_read, 2, HAL_MAX_DELAY);  // dummy_512 已經是 512byte 的 0xFF, 前 2byte 夠用

  SD_Deselect();

  return RES_OK;
  /* USER CODE END READ */
}

/**
  * @brief  Writes Sector(s)
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
#if _USE_WRITE == 1
DRESULT USER_write (
	BYTE pdrv,          /* Physical drive nmuber to identify the drive */
	const BYTE *buff,   /* Data to be written */
	DWORD sector,       /* Sector address in LBA */
	UINT count          /* Number of sectors to write */
)
{
  /* USER CODE BEGIN WRITE */
  /* USER CODE HERE */

  SD_Select(); // CS拉低: 選取卡片, 準備送指令

  if (!SD_WaitReady(500)) { // 送指令前先確認上一次寫入已經結束, 卡片不在busy
    SD_Deselect();
    printf("USER_write: card busy before CMD24\r\n");
    return RES_ERROR;
  }

  // 若 SDSC(CardType==1) 要換算成 byte 位址(sector * 512)
  uint32_t addr_cmd24 = (CardType == 2) ? sector : sector * 512;

  uint8_t cmd24[6];
  cmd24[0] = 0x58;
  cmd24[1] = (uint8_t)(addr_cmd24 >> 24);
  cmd24[2] = (uint8_t)(addr_cmd24 >> 16);
  cmd24[3] = (uint8_t)(addr_cmd24 >> 8);
  cmd24[4] = (uint8_t)(addr_cmd24);
  cmd24[5] = 0x01;

  // 第一段: 讀 R1 (1 byte)
  uint8_t tx_dummy = 0xFF;
  uint8_t w1_cmd24 = 0xFF;

  HAL_SPI_Transmit(&hspi3, cmd24, 6, HAL_MAX_DELAY);

  for (int i = 0; i < 8; i++){
    HAL_SPI_TransmitReceive(&hspi3, &tx_dummy, &w1_cmd24, 1, HAL_MAX_DELAY); // 送出 0xFF 當作 dummy byte, 同時讀取卡片透過 MISO 回傳 cmd17 的 response byte
    if (w1_cmd24 != 0xFF) break; // 收到非 0xFF, 代表卡片已回應
  }
  
  if (w1_cmd24 != 0x00) {
    SD_Deselect();
    printf("USER_write: CMD24 R1 failed: %02X\r\n", w1_cmd24);
    return RES_ERROR;
  }

  // 第二段: 送 Data Start Token (0xFE)
  // 規格要求 R1 跟資料封包之間至少隔 1 個 byte(N_WR), 先送一個 dummy 再送 token
  HAL_SPI_Transmit(&hspi3, &tx_dummy, 1, HAL_MAX_DELAY);

  uint8_t token_write = 0xFE;
  HAL_SPI_Transmit(&hspi3, &token_write, 1, HAL_MAX_DELAY); // 主動送出 Data Token —— 送 1 個 byte 0xFE

  // 第三段: 送 buff 512 byte 資料 
  HAL_SPI_Transmit(&hspi3, (uint8_t*)buff, 512, HAL_MAX_DELAY);

  // 送 2 byte CRC丟棄, CS拉高, return RES_OK
  uint8_t crc_write[2]={0xFF, 0xFF};
  HAL_SPI_Transmit(&hspi3, crc_write,  2, HAL_MAX_DELAY); // dummy_512 已經是 512byte 的 0xFF, 前 2byte 夠用

  // 第四段: 讀 Data Response Token, 判斷卡片有沒有接受這筆資料
  uint8_t data_response = 0xFF;
  HAL_SPI_TransmitReceive(&hspi3, &tx_dummy, &data_response, 1, HAL_MAX_DELAY);

  if ((data_response & 0x1F) != 0x05) {
    SD_Deselect();
    printf("USER_write: data rejected: %02X\r\n", data_response);
    return RES_ERROR; // 卡片拒絕了這筆資料(CRC error / write error)
  }

  // 第五段: 等待卡片把資料真的寫進flash(busy), MISO 持續輸出 0x00 代表還在忙
  if (!SD_WaitReady(500)) {
    SD_Deselect();
    printf("USER_write: busy timeout\r\n");
    return RES_ERROR; // 逾時還在忙, 視為失敗
  }

  SD_Deselect();

  return RES_OK;
  /* USER CODE END WRITE */
}
#endif /* _USE_WRITE == 1 */

/**
  * @brief  I/O control operation
  * @param  pdrv: Physical drive number (0..)
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
#if _USE_IOCTL == 1
DRESULT USER_ioctl (
	BYTE pdrv,      /* Physical drive nmuber (0..) */
	BYTE cmd,       /* Control code */
	void *buff      /* Buffer to send/receive control data */
)
{
  /* USER CODE BEGIN IOCTL */
    DRESULT res = RES_ERROR;

    if (Stat & STA_NOINIT) return RES_NOTRDY;
    switch (cmd)
    {
      case CTRL_SYNC:
        // 沒有額外的暫存資料要 flush, 直接回成功
        res = RES_OK;
        break;

      case GET_SECTOR_SIZE:
        // buff 是 void*, 要轉型成 WORD* 才能寫入
        // 標準 SD 卡 sector 固定 512 bytes
        *(WORD*)buff = 512;
        res = RES_OK;
        break;

      default:
        res = RES_PARERR;   // 不支援的 command
        break;
    }

    return res;
  /* USER CODE END IOCTL */
}
#endif /* _USE_IOCTL == 1 */

