/*
 * hal_card_platform_common.c- Sigmastar
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

#include "hal_card_platform.h"
#include "hal_card_timer.h"
#include "gpio.h"
#include "drv_gpio.h"
#include "drv_padmux.h"
#include "drv_puse.h"
#include "padmux.h"
#include "hal_card_platform_pri_config.h"

#if (PADMUX_SET == PADMUX_SET_BY_FUNC)
static U32_T gu32_SDPuse[3][13] = {
    {MDRV_PUSE_SDIO0_PWR, MDRV_PUSE_SDIO0_CDZ, MDRV_PUSE_SDIO0_CLK, MDRV_PUSE_SDIO0_CMD, MDRV_PUSE_SDIO0_D0,
     MDRV_PUSE_SDIO0_D1, MDRV_PUSE_SDIO0_D2, MDRV_PUSE_SDIO0_D3},
    {MDRV_PUSE_SDIO1_PWR, MDRV_PUSE_SDIO1_CDZ, MDRV_PUSE_SDIO1_CLK, MDRV_PUSE_SDIO1_CMD, MDRV_PUSE_SDIO1_D0,
     MDRV_PUSE_SDIO1_D1, MDRV_PUSE_SDIO1_D2, MDRV_PUSE_SDIO1_D3},
    {MDRV_PUSE_EMMC_PWR, MDRV_PUSE_EMMC_RST, MDRV_PUSE_EMMC_CLK, MDRV_PUSE_EMMC_CMD, MDRV_PUSE_EMMC_D0,
     MDRV_PUSE_EMMC_D1, MDRV_PUSE_EMMC_D2, MDRV_PUSE_EMMC_D3, MDRV_PUSE_EMMC_D4, MDRV_PUSE_EMMC_D5, MDRV_PUSE_EMMC_D6,
     MDRV_PUSE_EMMC_D7, MDRV_PUSE_EMMC_RST},
};
U8_T Hal_CARD_PadmuxGetting(MMCPadMuxInfo *mmc_PMuxInfo)
{
    U32_T u32_SdPadPin[12];
    U32_T i, j, k;

    if (drv_padmux_active())
    {
        i = mmc_PMuxInfo->u8_ipOrder;
        for (j = 0; j < 12; j++)
        {
            // sd0 pad pin getting
            u32_SdPadPin[j] = drv_padmux_getpad(gu32_SDPuse[i][j]);
            if (u32_SdPadPin[j] == PAD_UNKNOWN)
            {
                if (j == 0 || j == 1)
                    continue;

                pr_err("Fail to get pad(%#x) ip(%#x_%d)  form padmux !\n", gu32_SDPuse[i][j], i, j);
                // may no need to control power by software or no need reset/cdz pin

                break;
            }
        }
        if (j >= 8)
        {
            mmc_PMuxInfo->u32_PinPWR    = u32_SdPadPin[0];
            mmc_PMuxInfo->u32_PinCdzRst = u32_SdPadPin[1];
            mmc_PMuxInfo->u32_PinCLK    = u32_SdPadPin[2];
            mmc_PMuxInfo->u32_PinCMD    = u32_SdPadPin[3];
            for (k = 0; k < 8; k++)
                mmc_PMuxInfo->u32_PinDAT[k] = u32_SdPadPin[4 + k];

            if (j == 12)
                mmc_PMuxInfo->u8_busWidth = 8;
            else
                mmc_PMuxInfo->u8_busWidth = 4;

            mmc_PMuxInfo->u32_Mode = drv_padmux_getmode(gu32_SDPuse[i][2]);
            if (mmc_PMuxInfo->u32_PinPWR != PAD_UNKNOWN)
                mmc_PMuxInfo->u32_PwrMode = drv_padmux_getmode(gu32_SDPuse[i][0]);
            if (mmc_PMuxInfo->u32_PinCdzRst != PAD_UNKNOWN)
                mmc_PMuxInfo->u32_CdzRstMode = drv_padmux_getmode(gu32_SDPuse[i][1]);
            return 0;
        }
        return 1;
    }
    printf("padmux not active!\n");
    return 1;
}

void Hal_CARD_ConfigSdPad(MMCPadMuxInfo mmc_PMuxInfo)
{
    if (0 != drv_padmux_active())
    {
        sstar_gpio_padgroupmode_set(mmc_PMuxInfo.u32_Mode);
    }
}
//***********************************************************************************************************
// PAD Setting for Card Platform
//***********************************************************************************************************
void Hal_CARD_InitPADPin(MMCPadMuxInfo mmc_PMuxInfo)
{
    U8_T i;

    sstar_gpio_pull_up(mmc_PMuxInfo.u32_PinCdzRst);

    sstar_gpio_pull_up(mmc_PMuxInfo.u32_PinCMD);
    for (i = 0; i < mmc_PMuxInfo.u8_busWidth; i++)
    {
        sstar_gpio_pull_up(mmc_PMuxInfo.u32_PinDAT[i]);
    }
}

void Hal_CARD_PullPADPin(MMCPadMuxInfo mmc_PMuxInfo, PinPullEmType ePinPull)
{
    U8_T i;
    if (Hal_Card_CheckIseMMC(mmc_PMuxInfo))
        return;

    if (ePinPull == EV_PULLDOWN)
    {
        sstar_gpio_pad_val_set(mmc_PMuxInfo.u32_PinCLK, PINMUX_FOR_GPIO_MODE);
        sstar_gpio_set_low(mmc_PMuxInfo.u32_PinCLK);
        sstar_gpio_pad_oen(mmc_PMuxInfo.u32_PinCLK);

        sstar_gpio_pad_val_set(mmc_PMuxInfo.u32_PinCMD, PINMUX_FOR_GPIO_MODE);
        sstar_gpio_pull_off(mmc_PMuxInfo.u32_PinCMD);
        sstar_gpio_set_low(mmc_PMuxInfo.u32_PinCMD);
        sstar_gpio_pad_oen(mmc_PMuxInfo.u32_PinCMD);

        for (i = 0; i < mmc_PMuxInfo.u8_busWidth; i++)
        {
            sstar_gpio_pad_val_set(mmc_PMuxInfo.u32_PinDAT[i], PINMUX_FOR_GPIO_MODE);
            sstar_gpio_pull_off(mmc_PMuxInfo.u32_PinDAT[i]);
            sstar_gpio_set_low(mmc_PMuxInfo.u32_PinDAT[i]);
            sstar_gpio_pad_oen(mmc_PMuxInfo.u32_PinDAT[i]);
        }
    }
    else if (ePinPull == EV_PULLUP)
    {
        sstar_gpio_pad_odn(mmc_PMuxInfo.u32_PinCLK);

        sstar_gpio_pull_up(mmc_PMuxInfo.u32_PinCMD);
        sstar_gpio_pad_odn(mmc_PMuxInfo.u32_PinCMD);
        for (i = 0; i < mmc_PMuxInfo.u8_busWidth; i++)
        {
            sstar_gpio_pull_up(mmc_PMuxInfo.u32_PinDAT[i]);
            sstar_gpio_pad_odn(mmc_PMuxInfo.u32_PinDAT[i]);
        }
        sstar_gpio_padgroupmode_set(mmc_PMuxInfo.u32_Mode);
    }
}

void Hal_CARD_DrvCtrlPin(MMCPadMuxInfo mmc_PMuxInfo, MMCPinDrv mmc_pinDrv)
{
    U8_T i;

    // driving control switch
    if (mmc_pinDrv.eDrvClk != DRV_NOSET)
        sstar_gpio_drv_set(mmc_PMuxInfo.u32_PinCLK, mmc_pinDrv.eDrvClk);

    if (mmc_pinDrv.eDrvCmd != DRV_NOSET)
        sstar_gpio_drv_set(mmc_PMuxInfo.u32_PinCMD, mmc_pinDrv.eDrvCmd);

    if (mmc_pinDrv.eDrvData != DRV_NOSET)
    {
        for (i = 0; i < mmc_PMuxInfo.u8_busWidth; i++)
            sstar_gpio_drv_set(mmc_PMuxInfo.u32_PinDAT[i], mmc_pinDrv.eDrvData);
    }
}

void Hal_CARD_ConfigPowerPad(MMCPadMuxInfo mmc_PMuxInfo)
{
    if (Hal_Card_CheckIseMMC(mmc_PMuxInfo))
        return;
    if (mmc_PMuxInfo.u32_PinPWR != PAD_UNKNOWN)
    {
        sstar_gpio_pad_val_set(mmc_PMuxInfo.u32_PinPWR, mmc_PMuxInfo.u32_PwrMode);
        sstar_gpio_pad_oen(mmc_PMuxInfo.u32_PinPWR);
    }
}

void Hal_CARD_PowerOff(MMCPadMuxInfo mmc_PMuxInfo, U16_T u16DelayMs)
{
    if (Hal_Card_CheckIseMMC(mmc_PMuxInfo))
        return;
    if (mmc_PMuxInfo.u32_PinPWR != PAD_UNKNOWN)
        sstar_gpio_set_high(mmc_PMuxInfo.u32_PinPWR);

    Hal_Timer_mSleep(u16DelayMs);
}

void Hal_CARD_PowerOn(MMCPadMuxInfo mmc_PMuxInfo, U16_T u16DelayMs)
{
    if (Hal_Card_CheckIseMMC(mmc_PMuxInfo))
        return;
    if (mmc_PMuxInfo.u32_PinPWR != PAD_UNKNOWN)
        sstar_gpio_set_low(mmc_PMuxInfo.u32_PinPWR);

    Hal_Timer_mSleep(u16DelayMs);
}

void Hal_CARD_ConfigCdzPad(MMCPadMuxInfo mmc_PMuxInfo)
{
    if (Hal_Card_CheckIseMMC(mmc_PMuxInfo))
        return;
    if (0 != drv_padmux_active())
    {
        if (mmc_PMuxInfo.u32_PinCdzRst != PAD_UNKNOWN)
        {
            sstar_gpio_pad_val_set(mmc_PMuxInfo.u32_PinCdzRst, mmc_PMuxInfo.u32_CdzRstMode);
            sstar_gpio_pad_odn(mmc_PMuxInfo.u32_PinCdzRst);
        }
    }
}

BOOL_T Hal_CARD_GetCdzState(MMCPadMuxInfo mmc_PMuxInfo)
{
    U8_T nLv = 0;
    if (Hal_Card_CheckIseMMC(mmc_PMuxInfo))
        return TRUE;
    if (mmc_PMuxInfo.u32_PinCdzRst != PAD_UNKNOWN)
        sstar_gpio_pad_read(mmc_PMuxInfo.u32_PinCdzRst, &nLv);

    if (!nLv) // Low Active
        return TRUE;
    else
        return FALSE;
}

#endif
