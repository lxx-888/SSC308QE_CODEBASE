/*
 * hal_sdmmc_platform.h- Sigmastar
 *
 * Copyright (c) [2019~2020] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */
/***************************************************************************************************************
 *
 * FileName hal_sdmmc_platform.h
 *     @author jeremy.wang (2012/01/10)
 * Desc:
 *     This file is the header file of hal_card_platform_XX.c.
 *     Every project has the same header file.
 *
 ***************************************************************************************************************/

#ifndef __HAL_SDMMC_PLATFORM_H
#define __HAL_SDMMC_PLATFORM_H

#include "hal_sdmmc_base.h"
#if (D_OS == D_OS__LINUX)
#include "drv_sdmmc_lnx.h"
#elif (D_OS == D_OS__RTK)
#include "drv_sdmmc_rtk.h"
#endif

//***********************************************************************************************************
//  Basic Setting
//------------------------------------------------------------------------------------
//
// PADMUX_SET
#define PADMUX_SET_BY_FUNC (0)
#define PADMUX_SET_BY_REG  (1)

// GPIO_SET
#define GPIO_SET_BY_FUNC (0)
#define GPIO_SET_BY_REG  (1)

#if (D_OS == D_OS__LINUX)
#define PADMUX_SET (PADMUX_SET_BY_FUNC) //(PADMUX_SET_BY_FUNC)
#define GPIO_SET   (GPIO_SET_BY_FUNC)   //(GPIO_SET_BY_FUNC)

#define FORCE_SWITCH_PAD (FALSE)
#else
#define PADMUX_SET (PADMUX_SET_BY_REG)
#define GPIO_SET   (GPIO_SET_BY_REG)

#define FORCE_SWITCH_PAD (TRUE)
#endif

//-----------------------------------------------------------------------------------------------------------
// Clock Level Setting (From High Speed to Low Speed)
//-----------------------------------------------------------------------------------------------------------
#define CLK_G  250000000
#define CLK_F  200000000
#define CLK_E  100000000
#define CLK_DU 50000000
#define CLK_D  48000000
#define CLK_C  43200000 // 43200000
#define CLK_B  40000000
#define CLK_A  36000000 // 36000000
#define CLK_9U 33400000
#define CLK_9  32000000
#define CLK_8  20000000
#define CLK_7  12000000 // 12000000
#define CLK_6  300000   // alway 400KHz for DTS
#define CLK_5  0
#define CLK_4  0
#define CLK_3  0
#define CLK_2  0
#define CLK_1  0
#define CLK_0  0
//-----------------------------------------------------------------------------------------------------------
//***********************************************************************************************************

typedef enum
{
    EV_PULLDOWN,
    EV_PULLUP,

} PinPullEmType;

typedef enum
{
    EV_GPIO_OPT1 = 0,
    EV_GPIO_OPT2 = 1,
    EV_GPIO_OPT3 = 2,
    EV_GPIO_OPT4 = 3,
    EV_GPIO_OPT5 = 4,

} GPIOOptEmType;

typedef enum
{
    EV_NORVOL = 0,
    EV_LOWVOL = 1,
    EV_MINVOL = 2,

} PADVddEmType;

typedef enum
{
    DRV_NOSET = -1,
    DRV_CTRL_0,
    DRV_CTRL_1,
    DRV_CTRL_2,
    DRV_CTRL_3,
    DRV_CTRL_4,
    DRV_CTRL_5,
    DRV_CTRL_6,
    DRV_CTRL_7,
    DRV_CTRL_8,
    DRV_CTRL_9,
    DRV_CTRL_10,
    DRV_CTRL_11,
    DRV_CTRL_12,
    DRV_CTRL_13,
    DRV_CTRL_14,
    DRV_CTRL_15,
} DrvCtrlType;

typedef enum
{
    PHA_NOSET = -1,
    PHA_SEL_0,
    PHA_SEL_1,
    PHA_SEL_2,
    PHA_SEL_3,
} PhaseType;

typedef struct
{
    U8_T      u8_clkPhaEn;
    U8_T      u8_eightPhaEn;
    U8_T      u8_eightPha_TX;
    U8_T      u8_eightPha_RX;
    PhaseType eClkPha_TX;
    PhaseType eClkPha_RX;
} MMCClkPhase;

typedef struct
{
    DrvCtrlType eDrvClk;
    DrvCtrlType eDrvCmd;
    DrvCtrlType eDrvData;
} MMCPinDrv;

typedef struct
{
    U8_T  u8_ipOrder;
    U8_T  u8_padOrder;
    U32_T u32_Mode;
    U32_T u32_PwrMode;
    U32_T u32_CdzRstMode;
    U32_T u32_PinPWR;
    U32_T u32_PinCdzRst;
    U32_T u32_PinCLK;
    U32_T u32_PinCMD;
    U32_T u32_PinDAT[8];
    U8_T  u8_busWidth;
} MMCPadMuxInfo;

#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
typedef u64 dma_addr_t;
#else
typedef u32 dma_addr_t;
#endif

U8_T Hal_CARD_PadmuxGetting(MMCPadMuxInfo *mmc_PMuxInfo);
void Hal_CARD_IPOnceSetting(MMCPadMuxInfo mmc_PMuxInfo);

// PAD Setting for Card Platform
//----------------------------------------------------------------------------------------------------------
void   Hal_CARD_InitPADPin(MMCPadMuxInfo mmc_PMuxInfo);
BOOL_T Hal_CARD_GetPadInfoCdzPad(MMCPadMuxInfo mmc_PMuxInfo);
BOOL_T Hal_CARD_GetPadInfoPowerPad(MMCPadMuxInfo mmc_PMuxInfo);
void   Hal_CARD_ConfigSdPad(MMCPadMuxInfo mmc_PMuxInfo);
void   Hal_CARD_ConfigPowerPad(MMCPadMuxInfo mmc_PMuxInfo);
void   Hal_CARD_PullPADPin(MMCPadMuxInfo mmc_PMuxInfo, PinPullEmType ePinPull);
void   Hal_CARD_DrvCtrlPin(MMCPadMuxInfo mmc_PMuxInfo, MMCPinDrv mmc_PinDrv);
BOOL_T Hal_Check_ClkCmd_Interrelate(IpOrder eIP, PadOrder ePAD);

// Clock & BusTiming Setting for Card Platform
//----------------------------------------------------------------------------------------------------------
void  Hal_CARD_SetBustiming(MMCPadMuxInfo *mmc_PMuxInfo, BusTimingEmType eBusTiming);
void  Hal_CARD_PllSetting(MMCPadMuxInfo *mmc_PMuxInfo, BusTimingEmType eBusTiming);
void  Hal_CARD_SetClock(MMCPadMuxInfo *mmc_PMuxInfo, U32_T u32ClkFromIPSet);
U32_T Hal_CARD_FindClockSetting(IpOrder eIP, U32_T u32ReffClk);
#ifdef CONFIG_PM_SLEEP
void Hal_CARD_devpm_GetClock(IpOrder eIP, U32_T *pu32PmIPClk, U32_T *pu32PmBlockClk);
void Hal_CARD_devpm_setClock(IpOrder eIP, U32_T u32PmIPClk, U32_T u32PmBlockClk);
#endif

// Power and Voltage Setting for Card Platform
//----------------------------------------------------------------------------------------------------------
void   Hal_CARD_PowerOn(MMCPadMuxInfo mmc_PMuxInfo, U16_T u16DelayMs);
void   Hal_CARD_PowerOff(MMCPadMuxInfo mmc_PMuxInfo, U16_T u16DelayMs);
BOOL_T Hal_CARD_SetPADVdd(MMCPadMuxInfo mmc_PMuxInfo, PADVddEmType ePADVdd, U16_T u16DelayMs);

// Card Detect and GPIO Setting for Card Platform
//----------------------------------------------------------------------------------------------------------
void   Hal_CARD_ConfigCdzPad(MMCPadMuxInfo mmc_PMuxInfo); // Hal_CARD_InitGPIO
BOOL_T Hal_CARD_GetCdzState(MMCPadMuxInfo mmc_PMuxInfo);  // Hal_CARD_GetGPIOState
U32_T  Hal_CARD_CheckCdzMode(MMCPadMuxInfo mmc_PMuxInfo);

// MIU Setting for Card Platform
//----------------------------------------------------------------------------------------------------------
dma_addr_t Hal_CARD_TransMIUAddr(dma_addr_t ptr_Addr, U8_T *pu8MIUSel);

// Hard Ware Resetting for eMMC Card
//----------------------------------------------------------------------------------------------------------
void Hal_eMMC_HardWare_Reset(MMCPadMuxInfo mmc_PMuxInfo);

// Error Status Set or Clear
//----------------------------------------------------------------------------------------------------------
void SDMMC_Read_Timeout_Set(MMCPadMuxInfo mmc_PMuxInfo);
void SDMMC_Read_Timeout_Clear(MMCPadMuxInfo mmc_PMuxInfo);
void SDMMC_Write_Timeout_Set(IpOrder eIP);
void SDMMC_Write_Timeout_Clear(IpOrder eIP);
#endif // End of __HAL_SDMMC_PLATFORM_H
