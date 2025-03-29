/*
 * hal_card_platform.h- Sigmastar
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
 * FileName hal_card_platform.h
 *     @author jeremy.wang (2012/01/10)
 * Desc:
 *     This file is the header file of hal_card_platform_XX.c.
 *     Every project has the same header file.
 *
 ***************************************************************************************************************/

#ifndef __HAL_CARD_PLATFORM_H
#define __HAL_CARD_PLATFORM_H

#include "hal_card_regs.h"

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
    PhaseType eClkPha_TX;
    PhaseType eClkPha_RX;
    U8_T      u8_eightPhaEn;
    PhaseType u8_eightPha_TX;
    PhaseType u8_eightPha_RX;
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

U8_T Hal_CARD_PadmuxGetting(MMCPadMuxInfo *mmc_PMuxInfo);
void Hal_CARD_IPOnceSetting(IpOrder eIP);

// PAD Setting for Card Platform
//----------------------------------------------------------------------------------------------------------
void   Hal_CARD_InitPADPin(MMCPadMuxInfo mmc_PMuxInfo);
BOOL_T Hal_CARD_GetPadInfoCdzPad(IpOrder eIP, U32_T *nPadNum);
BOOL_T Hal_CARD_GetPadInfoPowerPad(IpOrder eIP, U32_T *nPadNum);
void   Hal_CARD_ConfigSdPad(MMCPadMuxInfo mmc_PMuxInfo);
void   Hal_CARD_ConfigPowerPad(MMCPadMuxInfo mmc_PMuxInfo);
void   Hal_CARD_PullPADPin(MMCPadMuxInfo mmc_PMuxInfo, PinPullEmType ePinPull);
void   Hal_CARD_DrvCtrlPin(MMCPadMuxInfo mmc_PMuxInfo, MMCPinDrv mmc_pinDrv);
BOOL_T Hal_Check_ClkCmd_Interrelate(IpOrder eIP, PadOrder ePAD);

// Clock Setting for Card Platform
//----------------------------------------------------------------------------------------------------------
void  Hal_CARD_SetClock(IpOrder eIP, MMCClkPhase *pmmc_clkPha, U32_T u32ClkFromIPSet);
U32_T Hal_CARD_FindClockSetting(IpOrder eIP, U32_T u32ReffClk);
#ifdef CONFIG_PM_SLEEP
void Hal_CARD_devpm_GetClock(IpOrder eIP, U32_T *pu32PmIPClk, U32_T *pu32PmBlockClk);
void Hal_CARD_devpm_setClock(IpOrder eIP, U32_T u32PmIPClk, U32_T u32PmBlockClk);
#endif

// Power and Voltage Setting for Card Platform
//----------------------------------------------------------------------------------------------------------
void   Hal_CARD_PowerOn(MMCPadMuxInfo mmc_PMuxInfo, U16_T u16DelayMs);
void   Hal_CARD_PowerOff(MMCPadMuxInfo mmc_PMuxInfo, U16_T u16DelayMs);
BOOL_T Hal_CARD_SetPADVdd(IpOrder eIP, PadOrder ePAD, PADVddEmType ePADVdd, U16_T u16DelayMs);

// Card Detect and GPIO Setting for Card Platform
//----------------------------------------------------------------------------------------------------------
void   Hal_CARD_ConfigCdzPad(MMCPadMuxInfo mmc_PMuxInfo);
BOOL_T Hal_CARD_GetCdzState(MMCPadMuxInfo mmc_PMuxInfo);
U32_T  Hal_CARD_CheckCdzMode(IpOrder eIP);

// MIU Setting for Card Platform
//----------------------------------------------------------------------------------------------------------
dma_addr_t Hal_CARD_TransMIUAddr(dma_addr_t ptr_Addr, U8_T *pu8MIUSel);
void       eMMC_hardware_reset(IpOrder eIP, PadOrder ePAD, U16_T u16DelayMs);
BOOL_T     Hal_Card_CheckIseMMC(MMCPadMuxInfo mmc_PMuxInfo);

#endif // End of __HAL_CARD_PLATFORM_H
