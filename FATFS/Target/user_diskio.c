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

  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET); // CS拉低: 選取卡片, 準備送指令

  uint8_t cmd0[6] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95}; // CMD0 (GO_IDLE_STATE): 0x40=start+cmd index0, 中間4 byte固定填0(無參數), 0x95=cmd0專用CRC7+stop bit
  HAL_SPI_Transmit(&hspi3, cmd0, 6, HAL_MAX_DELAY);

  uint8_t tx_dummy = 0xFF;
  uint8_t r1_cmd0 = 0xFF;
  for (int i = 0; i < 8; i++){
      HAL_SPI_TransmitReceive(&hspi3, &tx_dummy, &r1_cmd0, 1, HAL_MAX_DELAY); // 送出 0xFF 當作 dummy byte, 同時讀取卡片透過 MISO 回傳 cmd0 的 response byte
      if (r1_cmd0 != 0xFF) break; // 收到非 0xFF, 代表卡片已回應
  }
  if (r1_cmd0 != 0x01)
    printf("CMD0 failed: %02X\r\n", r1_cmd0);
  printf("CMD0=%02X\n", r1_cmd0);

  uint8_t cmd8[6] = {0x48, 0x00, 0x00, 0x01, 0xAA, 0x87};
  HAL_SPI_Transmit(&hspi3, cmd8, 6, HAL_MAX_DELAY);

  uint8_t r1_cmd8 = 0xFF;
  uint8_t cmd8_echo[4];
  for (int j = 0; j < 8; j++){
      HAL_SPI_TransmitReceive(&hspi3, &tx_dummy, &r1_cmd8, 1, HAL_MAX_DELAY); // 回傳cmd8的 response byte
      if (r1_cmd8 != 0xFF) break;
  }
  HAL_SPI_TransmitReceive(&hspi3, dummy, cmd8_echo, 4, HAL_MAX_DELAY); // 同時讀取卡片透過 MISO 回傳的 response 4 byte

  printf("CMD8=%02X\n", r1_cmd8);
  for (int i = 0; i < 4; i++)
    printf("%02X ", cmd8_echo[i]);
  printf("\r\n");


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

    printf("ACMD41=%02X (retry %d)\r\n", r1_cmd41, k); // 這次重試失敗, 印出目前狀態繼續下一輪

  }
  printf("ACMD41 final=%02X\r\n", r1_cmd41);

  uint8_t cmd58[6] = {0x7A, 0x00, 0x00, 0x00, 0x00, 0x01};
  HAL_SPI_Transmit(&hspi3, cmd58, 6, HAL_MAX_DELAY);

  uint8_t r1_cmd58 = 0xFF;
  uint8_t cmd58_echo[4];
  for (int x = 0; x < 8; x++){
      HAL_SPI_TransmitReceive(&hspi3, &tx_dummy, &r1_cmd58, 1, HAL_MAX_DELAY); // 回傳cmd58的 response byte
      if (r1_cmd58 != 0xFF) break;
  }
  HAL_SPI_TransmitReceive(&hspi3, dummy, cmd58_echo, 4, HAL_MAX_DELAY); // 同時讀取卡片透過 MISO 回傳的 response 4 byte
  printf("CMD58=%02X\n", r1_cmd58);
  
  if (cmd58_echo[0] & 0x40) {
    CardType = 2;  // CCS=1, SDHC/SDXC
  } else {
    CardType = 1;  // CCS=0, SDSC
  }

  for (int i = 0; i < 4; i++)
    printf("%02X ", cmd58_echo[i]);
  printf("\r\n");

  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);

  if (r1_cmd0 == 0x01 && r1_cmd8 == 0x01 && r1_cmd41 == 0x00 && r1_cmd58 == 0x00)
  {
    Stat = 0; // 全部成功, 清除 STA_NOINIT, 代表磁碟已就緒
  } else {
    Stat = STA_NOINIT;
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
    return res;
  /* USER CODE END IOCTL */
}
#endif /* _USE_IOCTL == 1 */

