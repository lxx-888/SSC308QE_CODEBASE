/*
 * drv_sdmmc_lnx.h- Sigmastar
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
 * FileName drv_sdmmc_lnx.h
 *     @author jeremy.wang (2012/01/10)
 * Desc:
 *     This file is the header file of ms_sdmmc_lnx.c.
 *
 ***************************************************************************************************************/

#ifndef __SS_SDMMC_LNX_H
#define __SS_SDMMC_LNX_H

#include <linux/cdev.h>
#include <linux/interrupt.h>
#include "hal_sdmmc_base.h"
#include "hal_sdmmc_platform.h"

//***********************************************************************************************************
// Config Setting (Externel)
//***********************************************************************************************************
//***********************************************************************************************************
#define ENABLE_EMMC_PRE_DEFINED_BLK_CNT 1
#define EN_MSYS_REQ_DMEM                (FALSE)
#define ENABLE_EMMC_POWER_SAVING_MODE   0 // psm on <default off>
#if ENABLE_EMMC_POWER_SAVING_MODE
#define ENABLE_EMMC_PSM_TEST 0 // psm test
#else
#define ENABLE_EMMC_PSM_TEST 1 // psm test
#endif

typedef enum
{
    EV_SDMMC1 = 0,
    EV_SDMMC2 = 1,
    EV_SDMMC3 = 2,

} SlotEmType;

typedef enum
{
    EV_MUTEX1  = 0,
    EV_MUTEX2  = 1,
    EV_MUTEX3  = 2,
    EV_NOMUTEX = 3,

} MutexEmType;

struct sstar_mmc_priv
{
    U8_T          u8_slotNo;
    U8_T          u8_transMode;  // dma, adma, cifd
    U8_T          u8_cifdMCGOff; // cifd mcg off
    U8_T          u8_supportRPM; // sd/sdio runtime PM
    U8_T          u8_fakeCdz;
    U8_T          u8_revCdz;
    U8_T          u8_supportCMD23;
    U8_T          u8_sdioUse1Bit;
    U8_T          u8_supportSD30;
    U8_T          u8_supportEMMC50;
    U32_T         u32_maxClk;
    U32_T         u32_pwerOnDelay;
    U32_T         u32_pwerOffDelay;
    U32_T         u32_mieIrqNo;
    U32_T         u32_cdzIrqNo;
    U32_T         pIPBANKArr[2];
    U32_T         pPLLIPBANKArr[2];
    U32_T         pCIFDIPBANKArr[2];
    U32_T         pPWRSAVEIPBANKArr[2];
    MMCPinDrv     mmc_pinDrv;
    MMCClkPhase   mmc_clkPha;
    MMCPadMuxInfo mmc_PMuxInfo;
};

struct sstar_sdmmc_host
{
    struct platform_device * pdev;
    struct sstar_sdmmc_slot *sdmmc_slot;
};

struct sstar_sdmmc_slot
{
    struct mmc_host *mmc;

    unsigned int slotNo;     // Slot No.
    unsigned int ipOrder;    // Slot No.
    unsigned int mieIRQNo;   // MIE IRQ No.
    unsigned int cdzIRQNo;   // CDZ IRQ No.
    unsigned int pwrGPIONo;  // PWR GPIO No.
    unsigned int pmrsaveClk; // Power Saving Clock

    unsigned int initFlag; // First Time Init Flag
    unsigned int sdioFlag; // SDIO Device Flag
    unsigned int strFlag;  // str flag suspend:2/resume:1/normal:0

    unsigned int   currClk;        // Current Clock
    unsigned int   currRealClk;    // Current Real Clock
    unsigned char  currWidth;      // Current Bus Width
    unsigned char  currTiming;     // Current Bus Timning
    unsigned char  currPowrMode;   // Current PowerMode
    unsigned char  currBusMode;    // Current Bus Mode
    unsigned short currVdd;        // Current Vdd
    unsigned char  currDDR;        // Current DDR
    unsigned char  currTimeoutCnt; // Current Timeout Count

    int read_only; // WP
    int card_det;  // Card Detect

    /****** DMA buffer used for transmitting *******/
    u32 *      dma_buffer;
    dma_addr_t dma_phy_addr;

    /****** ADMA buffer used for transmitting *******/
    u32 *      adma_buffer;
    dma_addr_t adma_phy_addr;

    /***** Tasklet for hotplug ******/
    struct tasklet_struct    hotplug_tasklet;
    struct sstar_sdmmc_host *parent_sdmmc;

    struct sstar_mmc_priv *p_mmc_priv;

#ifdef CONFIG_SUPPORT_SDMMC_COMMAND
    struct list_head list;
#endif
}; /* struct ms_sdmmc_hot*/

#endif // End of __SS_SDMMC_LNX_H
