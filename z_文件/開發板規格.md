# NUCLEO-F446RE 開發板規格

## MCU

| 項目 | 內容 |
|---|---|
| 型號 | STM32F446RET6 |
| 核心 | Arm Cortex-M4（含 FPU + DSP 指令集） |
| 封裝 | LQFP64（64 pin） |
| Flash | 512 KB |
| SRAM | 128 KB |
| 最高主頻 | 180 MHz |

## 板載資源（ST Nucleo-64）

- ST-LINK/V2-1 板載燒錄/除錯器，同時作為 USB 虛擬 COM Port（對應 USART2 = PA2/PA3）
- 使用者 LED：LD2（綠色），接 PA5
- 使用者按鈕：B1/USER，接 PC13，本專案設為 EXTI 中斷模式
- Arduino Uno V3 排針 + ST Morpho 全接腳排針

## 時脈

- 內部 HSI（16 MHz）為主要來源
- 32.768 kHz LSE 供 RTC 使用（PC14/PC15 = OSC32_IN/OUT）
- 預留 HSE 腳位（PH0/PH1 = OSC_IN/OUT），Nucleo 板預設不外接晶振

### 目前專案時脈設定（[main.c](../Core/Src/main.c) `SystemClock_Config`）

- 來源：HSI → PLL
- PLLM = 16, PLLN = 336, PLLP = /4, PLLQ = 2, PLLR = 2
- SYSCLK / AHB (HCLK) = **84 MHz**
- APB1 (PCLK1) = HCLK/2 = 42 MHz
- APB2 (PCLK2) = HCLK/1 = 84 MHz
- 電壓調節：Voltage Scale 3
- Flash Latency：2 wait states

> 註：MCU 最高可跑到 180 MHz（需改用外部 HSE 8MHz 並調整 PLL 參數），目前專案為 CubeMX 預設值，僅 84 MHz。

## 來源

- `.ioc` 設定檔：[NUCLEO_F446RE_Test.ioc](../NUCLEO_F446RE_Test.ioc)
- CubeMX Firmware Package：STM32Cube FW_F4 V1.28.3
