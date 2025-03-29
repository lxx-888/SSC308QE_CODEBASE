/*
 * drv_sdmmc_verify.c- Sigmastar
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
 * FileName drv_sdmmc_verify.c
 *     @author jeremy.wang (2015/10/01)
 * Desc:
 *     This layer is between UBOOT Common API layer and SDMMC Driver layer.
 *     (1) The goal is we could modify any verification flow but don't modify any sdmmc driver code.
 *     (2) Timer Test, PAD Test, Init Test, CIFD/DMA Test, Burning Test
 *
 ***************************************************************************************************************/
#include <linux/mmc/host.h>
#include "core.h"
#include "mmc_ops.h"
#include "sd_ops.h"
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include "cam_os_wrapper.h"
#include "drv_sdmmc_lnx.h"
#include "drv_sdmmc_verify.h"
#include "drv_sdmmc_command.h"
#include "hal_sdmmc_timer.h"
#include "hal_sdmmc_platform.h"
#include "hal_sdmmc_platform_pri_config.h"
#if defined(CONFIG_SSTAR_SDMMC_INTERRUPT_MODE)
#include "hal_sdmmc_intr.h" //inlcue but may not use
#endif

//***********************************************************************************************************
// Config Setting (Internel)
//***********************************************************************************************************
#define D_DUMPCOMP (TRUE)

#define BURN_START_SECTOR 30000
#define BURN_END_SECTOR   70000

#define BURN_CLK_CHG_CNT 120
#define BURN_CLK_CHG_LVL 3

#define BURN_PAT_CHG_CNT 20
#define BURN_PAT_CHG_LVL 5

static U8_T  gu8Pattern = 0;
static U16_T gu16Ret1 = 0, gu16Ret2 = 0, gu16Ret3 = 0;

#define D_TIMERTEST    "(SDMMC TimerTest)   "
#define D_SETPAD       "(SDMMC SetPAD)      "
#define D_DAT1DET      "(SDMMC DAT1Det)     "
#define D_CARDDET      "(SDMMC CardDet)     "
#define D_SDMMCINIT    "(SDMMC Init)        "
#define D_CIFDWRITE    "(SDMMC CIFD W)      "
#define D_CIFDREAD     "(SDMMC CIFD R)      "
#define D_DMAWRITE     "(SDMMC DMA W)       "
#define D_DMAREAD      "(SDMMC DMA R)       "
#define D_ADMAWRITE    "(SDMMC ADMA W)      "
#define D_ADMAREAD     "(SDMMC ADMA R)      "
#define D_COMPARE      "(COMPARE CONTENT)   "
#define D_WIDEBUS      "(SDMMC SetWideBus)  "
#define D_HIGHBUS      "(SDMMC SetHighBus)  "
#define D_SETBUSCLK    "(SDMMC SetBusClock) "
#define D_SETBUSTIMING "(SDMMC SetBusTiming)"
#define D_TESTPATT     "(SDMMC TestPattern) "
#define D_PSINIT       "(SDMMC PS Init)     "
#define D_PSTEST       "(SDMMC PS Test)     "

#define MAX_BLK_SIZE  512  // Maximum Transfer Block Size
#define MAX_BLK_COUNT 1024 // Maximum Transfer Block Count
#define MAX_SEG_CNT   128

#if 1
#define prtstring(fmt, arg...) printk(KERN_CONT fmt, ##arg)
#else
#define prtstring(fmt, arg...)
#endif
static U8_T *     dma_test_buf0, *dma_test_buf1;
extern dma_addr_t dma_test_addr0, dma_test_addr1;
extern dma_addr_t AdmaScriptsAddr;

static AdmaDescStruct *  AdmaScripts;
static U8_T              u8Width[3] = {1, 4, 8};
extern U8_T              u8MakeStsErr;
extern struct list_head  sdmmc_command_list;
extern volatile CardType geCardType[SDMMC_NUM_TOTAL];
extern CamOsMutex_t      sdmmc_mutex[SDMMC_NUM_TOTAL];

//----------------------------------------------------------------------------------------------------------
void _PRT_LINE(BOOL_T bHidden)
{
    if (!bHidden)
        prtstring("\r\n========================================================================================\r\n");
}

//----------------------------------------------------------------------------------------------------------
void _PRT_ENTER(BOOL_T bHidden)
{
    if (!bHidden)
        prtstring("\r\n");
}

//----------------------------------------------------------------------------------------------------------
void _PRT_MSG(U8_T u8Slot, char arrTestName[], U16_T u16Err, BOOL_T bHidden)
{
    if (u16Err)
    {
        prtstring("[sdmmc_%d]  %s ........ (FAIL)= %04x\n", u8Slot, arrTestName, u16Err);
    }
    else if (!bHidden)
    {
        prtstring("[sdmmc_%d]  %s ........ (PASS)\n", u8Slot, arrTestName);
    }
}

void _PRT_VALUE(U8_T u8Slot, char arrTestName[], U32_T u32Value, BOOL_T bHidden)
{
    if (!bHidden)
    {
        prtstring("[sdmmc_%d]  %s ........  %08x\n", u8Slot, arrTestName, u32Value);
    }
}

void _PRT_STATUS(U8_T u8BusWidth, U8_T u8BusTiming)
{
    printk("\r\n*********************** [ Test %d bit %s I/O ] ************************************\r\n", u8BusWidth,
           (u8BusTiming == 0)   ? "Low Speed"
           : (u8BusTiming == 1) ? "Default Speed"
           : (u8BusTiming == 2) ? "High Speed"
                                : "Unknow");
}

void _DUMP_MEM_ARR(volatile U8_T *u8Arr, U16_T u16Size)
{
    unsigned short u16Pos;

    prtstring("\r\n---------------------------------------------------------------------------------------------\r\n");

    for (u16Pos = 0; u16Pos < u16Size; u16Pos++)
    {
        if (u16Pos % 16 == 0 && u16Pos != 0)
            prtstring("\r\n");

        prtstring("(%02x)", u8Arr[u16Pos]);
    }
    prtstring("\r\n----------------------------------------------------------------------------------------------\r\n");
}

U16_T _MEM_COMPARE(volatile U8_T *u8Arr1, volatile U8_T *u8Arr2, U16_T u16Size)
{
    U16_T u16Pos;

    for (u16Pos = 0; u16Pos < u16Size; u16Pos++)
    {
        if (u8Arr1[u16Pos] != u8Arr2[u16Pos])
            return 1;
    }

    return 0;
}

void BUFFER_DAM_Alloc(struct device *dev)
{
    dma_test_buf0 = dma_alloc_coherent(dev, MAX_BLK_COUNT * MAX_BLK_SIZE, &dma_test_addr0, GFP_KERNEL);
    dma_test_buf1 = dma_alloc_coherent(dev, MAX_BLK_COUNT * MAX_BLK_SIZE, &dma_test_addr1, GFP_KERNEL);
    AdmaScripts   = dma_alloc_coherent(dev, sizeof(AdmaScripts) * MAX_SEG_CNT, &AdmaScriptsAddr, GFP_KERNEL);

    printk(KERN_CONT "dma_test_buf0: phys_addr: %llx | vir_addr: %llx \n", (U64_T)(uintptr_t)dma_test_addr0,
           (U64_T)(uintptr_t)dma_test_buf0);
    printk(KERN_CONT "dma_test_buf1: phys_addr: %llx | vir_addr: %llx \n", (U64_T)(uintptr_t)dma_test_addr1,
           (U64_T)(uintptr_t)dma_test_buf1);
    printk(KERN_CONT "ADMA SCRIPTS : phys_addr: %llx | vir_addr: %llx \n", (U64_T)(uintptr_t)AdmaScriptsAddr,
           (U64_T)(uintptr_t)AdmaScripts);

    return;
}

void BUFFER_DMA_FREE(struct device *dev)
{
    dma_free_coherent(dev, MAX_BLK_COUNT * MAX_BLK_SIZE, dma_test_buf0, dma_test_addr0);
    dma_free_coherent(dev, MAX_BLK_COUNT * MAX_BLK_SIZE, dma_test_buf1, dma_test_addr1);
    dma_free_coherent(dev, sizeof(AdmaScripts) * MAX_SEG_CNT, AdmaScripts, AdmaScriptsAddr);

    return;
}
void _SET_MEM_PATTERN(volatile U8_T *u8Arr, U16_T u16Size, U8_T u8Pattern)
{
    U16_T u16Pos = 0;
    U8_T  ctmp   = 0;
    U8_T  change = 0;

    if (u8Pattern == 0)
    {
        ctmp = 0;
        for (u16Pos = 0; u16Pos < u16Size; u16Pos++)
        {
            if (u16Pos % 16 == 0 && u16Pos != 0)
                ctmp++;

            u8Arr[u16Pos] = (unsigned char)ctmp;
        }
    }
    else if (u8Pattern == 1)
    {
        ctmp = 0;
        for (u16Pos = 0; u16Pos < u16Size; u16Pos++)
        {
            u8Arr[u16Pos] = (unsigned char)ctmp;
            ctmp++;
        }
    }
    else if (u8Pattern == 2)
    {
        ctmp = 0xFF;
        for (u16Pos = 0; u16Pos < u16Size; u16Pos++)
        {
            u8Arr[u16Pos] = (unsigned char)ctmp;
            ctmp--;
        }
    }
    else if (u8Pattern == 3)
    {
        for (u16Pos = 0; u16Pos < u16Size; u16Pos++)
        {
            if ((u16Pos % 0x100) == 0)
            {
                if (change)
                {
                    ctmp   = 0xFF;
                    change = 0;
                }
                else
                {
                    ctmp   = 0x00;
                    change = 1;
                }
            }

            u8Arr[u16Pos] = (unsigned char)ctmp;

            if (change)
                ctmp++;
            else
                ctmp--;
        }
    }
    else if (u8Pattern == 4) // 0xFF, 0x00
    {
        ctmp = 0xFF;
        for (u16Pos = 0; u16Pos <= u16Size; u16Pos++)
        {
            u8Arr[u16Pos] = (unsigned char)ctmp;

            if (ctmp == 0)
                ctmp = 0xFF;
            else
                ctmp = 0x0;
        }
    }
    else if (u8Pattern == 5) // 0x55, 0xAA
    {
        ctmp = 0x55;
        for (u16Pos = 0; u16Pos <= u16Size; u16Pos++)
        {
            u8Arr[u16Pos] = (unsigned char)ctmp;

            if (ctmp == 0xAA)
                ctmp = 0x55;
            else
                ctmp = 0xAA;
        }
    }
}

/*----------------------------------------------------------------------------------------------------------
*
* Function: IPV_SDMMC_TimerTest
*     @author jeremy.wang (2015/10/13)
* Desc: Timer Test for IP Verification
*
* @param u8Sec : Seconds
----------------------------------------------------------------------------------------------------------*/
void IPV_SDMMC_TimerTest(U8_T u8Slot, U8_T u8Sec)
{
    U8_T u8CurSec;

    _PRT_LINE(FALSE);

    _PRT_MSG(u8Slot, D_TIMERTEST, 0, FALSE);
    prtstring("-->");

    for (u8CurSec = 0; (u8CurSec < u8Sec) && (u8CurSec <= 10); u8CurSec++)
    {
        prtstring("(@)");
        Hal_Timer_mDelay(1000);
    }

    _PRT_LINE(FALSE);
}

/*----------------------------------------------------------------------------------------------------------
*
* Function: IPV_SDMMC_CardDetect
*     @author jeremy.wang (2015/10/1)
* Desc: Card Detection for IP Verification
*
* @param u8Slot : Slot ID
----------------------------------------------------------------------------------------------------------*/
void IPV_SDMMC_CardDetect(struct sstar_sdmmc_slot *p_sdmmc_slot)
{
    _PRT_LINE(FALSE);

    gu16Ret1 = !SDMMC_CardDetect(p_sdmmc_slot);
    _PRT_MSG(p_sdmmc_slot->slotNo, D_CARDDET, gu16Ret1, FALSE);

    _PRT_LINE(FALSE);
}

/*----------------------------------------------------------------------------------------------------------
*
* Function: IPV_SDMMC_Init
*     @author jeremy.wang (2015/10/1)
* Desc: SDMMC Init for IP Verification
*
* @param u8Slot : Slot ID
----------------------------------------------------------------------------------------------------------*/
void IPV_SDMMC_Init(struct sstar_sdmmc_slot *p_sdmmc_slot)
{
    _PRT_LINE(FALSE);

    gu16Ret1 = SDMMC_Init(p_sdmmc_slot);
    _PRT_MSG(p_sdmmc_slot->ipOrder, D_SDMMCINIT, gu16Ret1, FALSE);

    _PRT_LINE(FALSE);
}

void IPV_SDMMC_Init_Full(struct sstar_sdmmc_slot *p_sdmmc_slot)
{
    IPV_SDMMC_Init(p_sdmmc_slot);
    IPV_SDMMC_SetHighBus(p_sdmmc_slot);
    IPV_SDMMC_SetBusTiming(p_sdmmc_slot, 2);
    IPV_SDMMC_SetClock(p_sdmmc_slot->p_mmc_priv, 0);
    IPV_SDMMC_SetWideBus(p_sdmmc_slot, p_sdmmc_slot->p_mmc_priv->mmc_PMuxInfo.u8_busWidth);
}

/*----------------------------------------------------------------------------------------------------------
*
* Function: IPV_SDMMC_SetWideBus
*     @author jeremy.wang (2015/10/6)
* Desc: Set Wide Bus (4Bits) for IP Verification
*
* @param p_sdmmc_slot :
* @param u8BusWidth :  1 / 4 / 8
----------------------------------------------------------------------------------------------------------*/
void IPV_SDMMC_SetWideBus(struct sstar_sdmmc_slot *p_sdmmc_slot, U8_T u8BusWidth)
{
    _PRT_LINE(FALSE);

    if (!(p_sdmmc_slot->mmc->caps2 & MMC_CAP2_NO_SD))
        gu16Ret1 = SDMMC_SetWideBus(p_sdmmc_slot->ipOrder);
    else if (!(p_sdmmc_slot->mmc->caps2 & MMC_CAP2_NO_MMC))
        gu16Ret1 = eMMC_SetBusWidth(p_sdmmc_slot->ipOrder, u8BusWidth, 0);
    _PRT_MSG(p_sdmmc_slot->slotNo, D_WIDEBUS, gu16Ret1, FALSE);

    _PRT_LINE(FALSE);
}

/*----------------------------------------------------------------------------------------------------------
*
* Function: IPV_SDMMC_SetHighBus
*     @author jeremy.wang (2015/10/13)
* Desc: Set HighSpeed Bus for IP Verification
*
* @param p_sdmmc_slot :
----------------------------------------------------------------------------------------------------------*/
void IPV_SDMMC_SetHighBus(struct sstar_sdmmc_slot *p_sdmmc_slot)
{
    _PRT_LINE(FALSE);

    if (!(p_sdmmc_slot->mmc->caps2 & MMC_CAP2_NO_SD))
        gu16Ret1 = SDMMC_SwitchHighBus(p_sdmmc_slot);
    else if (!(p_sdmmc_slot->mmc->caps2 & MMC_CAP2_NO_MMC))
        gu16Ret1 = eMMC_SetBusSpeed(p_sdmmc_slot->ipOrder, HIGH_SPEED_BUS_SPEED);
    _PRT_MSG(p_sdmmc_slot->ipOrder, D_HIGHBUS, gu16Ret1, FALSE);

    _PRT_LINE(FALSE);
}

/*----------------------------------------------------------------------------------------------------------
*
* Function: IPV_SDMMC_SetClock
*     @author jeremy.wang (2015/10/13)
* Desc: Set Clock for IP Verification
*
* @param p_mmc_priv :
* @param u32ReffClk :  Clock Hz
----------------------------------------------------------------------------------------------------------*/
void IPV_SDMMC_SetClock(struct sstar_mmc_priv *p_mmc_priv, U32_T u32ReffClk)
{
    U32_T u32RealClk = 0;

    _PRT_LINE(FALSE);

    u32RealClk = SDMMC_SetClock(p_mmc_priv, u32ReffClk);
    _PRT_VALUE(p_mmc_priv->u8_slotNo, D_SETBUSCLK, u32RealClk, FALSE);

    _PRT_LINE(FALSE);
}

/*----------------------------------------------------------------------------------------------------------
*
* Function: IPV_SDMMC_SetBusTiming
*     @author jeremy.wang (2015/10/13)
* Desc: Set Bus Timing for IP Verification
*
* @param p_sdmmc_slot :
* @param u8BusTiming : Bus Timing (Enum Type number)
----------------------------------------------------------------------------------------------------------*/
void IPV_SDMMC_SetBusTiming(struct sstar_sdmmc_slot *p_sdmmc_slot, U8_T u8BusTiming)
{
    _PRT_LINE(FALSE);

    SDMMC_SetBusTiming(p_sdmmc_slot->p_mmc_priv, (BusTimingEmType)u8BusTiming);
    _PRT_VALUE(p_sdmmc_slot->ipOrder, D_SETBUSTIMING, u8BusTiming, FALSE);

    _PRT_LINE(FALSE);
}

U16_T SDMMCTest_Command_Response_CRC_Error(struct sstar_sdmmc_slot *p_sdmmc_slot)
{
    u8MakeStsErr = MAKE_CMD_RSP_ERR;

    SDMMC_ReadData(p_sdmmc_slot, BURN_START_SECTOR, 1, dma_test_buf1);

    u8MakeStsErr = 0;

    return 0;
}

U16_T SDMMCTest_Command_No_Response(struct sstar_sdmmc_slot *p_sdmmc_slot)
{
    u8MakeStsErr = MAKE_CMD_NO_RSP;

    SDMMC_CMD0(p_sdmmc_slot->ipOrder);

    u8MakeStsErr = 0;

    return 0;
}

U16_T SDMMCTest_Read_Timeout_Error(struct sstar_sdmmc_slot *p_sdmmc_slot)
{
    u8MakeStsErr = MAKE_RD_TOUT_ERR;

    SDMMC_Read_Timeout_Set(p_sdmmc_slot->p_mmc_priv->mmc_PMuxInfo);
    SDMMC_ReadData(p_sdmmc_slot, BURN_START_SECTOR, 1, dma_test_buf1);
    SDMMC_Read_Timeout_Clear(p_sdmmc_slot->p_mmc_priv->mmc_PMuxInfo);

    u8MakeStsErr = 0;

    return 0;
}

U16_T SDMMCTest_Write_Timeout_Error(struct sstar_sdmmc_slot *p_sdmmc_slot)
{
    u8MakeStsErr = MAKE_WR_TOUT_ERR;

    SDMMC_Write_Timeout_Set(p_sdmmc_slot->ipOrder);
    SDMMC_WriteData(p_sdmmc_slot, BURN_START_SECTOR, 1, dma_test_buf0);
    SDMMC_Write_Timeout_Clear(p_sdmmc_slot->ipOrder);

    u8MakeStsErr = 0;

    return 0;
}

U16_T SDMMCTest_Write_CRC_Error(struct sstar_sdmmc_slot *p_sdmmc_slot)
{
    u8MakeStsErr = MAKE_WR_CRC_ERR;

    SDMMC_WriteData(p_sdmmc_slot, BURN_START_SECTOR, 1, dma_test_buf0);

    u8MakeStsErr = 0;

    return 0;
}

U16_T SDMMCTest_Read_CRC_Error(struct sstar_sdmmc_slot *p_sdmmc_slot)
{
    u8MakeStsErr = MAKE_RD_CRC_ERR;

    SDMMC_ReadData(p_sdmmc_slot, BURN_START_SECTOR, 1, dma_test_buf1);

    u8MakeStsErr = 0;

    return 0;
}

/*----------------------------------------------------------------------------------------------------------
*
* Function: IPV_SDMMC_CIFD_RW
*     @author jeremy.wang (2015/10/13)
* Desc: CIFD R/W for IP Verification
*
* @param p_sdmmc_slot :
* @param u32SecAddr : Sector Address
* @param bHidden : Hidden Print
----------------------------------------------------------------------------------------------------------*/
void IPV_SDMMC_CIFD_RW(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32SecBlkAddr, BOOL_T bHidden)
{
#if defined(CONFIG_SSTAR_SDMMC_POLLING_MODE)

    _PRT_LINE(bHidden);

    _SET_MEM_PATTERN(dma_test_buf0, 512, gu8Pattern);

    gu16Ret1 = SDMMC_CIF_BLK_W(p_sdmmc_slot, u32SecBlkAddr, dma_test_buf0);
    _PRT_MSG(p_sdmmc_slot->ipOrder, D_CIFDWRITE, gu16Ret1, bHidden);
    _PRT_ENTER(bHidden);

    memset(dma_test_buf1, 0, 512);

    gu16Ret2 = SDMMC_CIF_BLK_R(p_sdmmc_slot, u32SecBlkAddr, dma_test_buf1);
    _PRT_MSG(p_sdmmc_slot->ipOrder, D_CIFDREAD, gu16Ret2, bHidden);
    _PRT_ENTER(bHidden);

    gu16Ret3 = _MEM_COMPARE(dma_test_buf0, dma_test_buf1, 512);
    _PRT_MSG(p_sdmmc_slot->ipOrder, D_COMPARE, gu16Ret3, bHidden);

    _PRT_LINE(bHidden);

#if (D_DUMPCOMP)
    if (gu16Ret3)
    {
        _DUMP_MEM_ARR(dma_test_buf0, 512);
        _DUMP_MEM_ARR(dma_test_buf1, 512);
    }
#endif
#endif
}

/*----------------------------------------------------------------------------------------------------------
*
* Function: IPV_SDMMC_DMA_RW
*     @author jeremy.wang (2015/10/13)
* Desc: DMA R/W for IP Verification
*
* @param p_sdmmc_slot :
* @param u32SecBlkAddr : Sector Address
* @param u16SecCount : Sector Count
* @param bHidden : Hidden Print
----------------------------------------------------------------------------------------------------------*/
void IPV_SDMMC_DMA_RW(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32SecBlkAddr, U16_T u16SecCount, BOOL_T bHidden)
{
    if (u16SecCount > 8)
        return;

    _PRT_LINE(bHidden);

    _SET_MEM_PATTERN(dma_test_buf0, u16SecCount * 512, gu8Pattern);

    gu16Ret1 = SDMMC_WriteData(p_sdmmc_slot, u32SecBlkAddr, u16SecCount, dma_test_buf0);
    _PRT_MSG(p_sdmmc_slot->ipOrder, D_DMAWRITE, gu16Ret1, bHidden);
    _PRT_ENTER(bHidden);

    memset(dma_test_buf1, 0, u16SecCount * 512);

    gu16Ret2 = SDMMC_ReadData(p_sdmmc_slot, u32SecBlkAddr, u16SecCount, dma_test_buf1);
    _PRT_MSG(p_sdmmc_slot->ipOrder, D_DMAREAD, gu16Ret2, bHidden);
    _PRT_ENTER(bHidden);

    gu16Ret3 = _MEM_COMPARE(dma_test_buf0, dma_test_buf1, u16SecCount * 512);
    _PRT_MSG(p_sdmmc_slot->ipOrder, D_COMPARE, gu16Ret3, bHidden);

    _PRT_LINE(bHidden);

#if (D_DUMPCOMP)
    if (gu16Ret3)
    {
        _DUMP_MEM_ARR(dma_test_buf0, u16SecCount * 512);
        _DUMP_MEM_ARR(dma_test_buf1, u16SecCount * 512);
    }
#endif
}

//###########################################################################################################
#if 1 //(D_FCIE_M_VER == D_FCIE_M_VER__05)
//###########################################################################################################

/*----------------------------------------------------------------------------------------------------------
*
* Function: IPV_SDMMC_ADMA_RW
*     @author jeremy.wang (2015/10/13)
* Desc: ADMA R/W for IP Verification
*
* @param p_sdmmc_slot :
* @param u32SecBlkAddr : Sector Address
* @param bHidden : Hidden Print
----------------------------------------------------------------------------------------------------------*/
void IPV_SDMMC_ADMA_RW(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32SecBlkAddr, BOOL_T bHidden)
{
    dma_addr_t auDMAAddr[10];
    U16_T      au16BlkCnt[10];
    U32_T      i, u32seccount = 0;

    _PRT_LINE(bHidden);

    _SET_MEM_PATTERN(dma_test_buf0, 2 * 512, gu8Pattern);

    for (i = 0; i < 10; i++)
    {
        auDMAAddr[i] = dma_test_addr0 + 0x200 * u32seccount;

        au16BlkCnt[i] = 0x1;
        u32seccount += au16BlkCnt[i];
    }

    gu16Ret1 =
        SDMMC_ADMA_BLK_W(p_sdmmc_slot, u32SecBlkAddr << 9, auDMAAddr, au16BlkCnt, 10, (volatile void *)AdmaScripts);
    _PRT_MSG(p_sdmmc_slot->ipOrder, D_ADMAWRITE, gu16Ret1, bHidden);
    _PRT_ENTER(bHidden);
    if (gu16Ret1)
    {
        // while(1);
    }

    memset(dma_test_buf1, 0, 2 * 512);
    u32seccount = 0;

    for (i = 0; i < 10; i++)
    {
        auDMAAddr[i] = dma_test_addr1 + 0x200 * u32seccount;

        au16BlkCnt[i] = 0x5;
        u32seccount += au16BlkCnt[i];
    }

    gu16Ret2 =
        SDMMC_ADMA_BLK_R(p_sdmmc_slot, u32SecBlkAddr << 9, auDMAAddr, au16BlkCnt, 10, (volatile void *)AdmaScripts);
    _PRT_MSG(p_sdmmc_slot->ipOrder, D_ADMAREAD, gu16Ret2, bHidden);
    _PRT_ENTER(bHidden);
    if (gu16Ret2)
    {
        // while(1);
    }

    gu16Ret3 = _MEM_COMPARE(dma_test_buf0, dma_test_buf1, 2 * 512);
    _PRT_MSG(p_sdmmc_slot->ipOrder, D_COMPARE, gu16Ret3, bHidden);

    _PRT_LINE(bHidden);

#if (D_DUMPCOMP)
    if (gu16Ret3)
    {
        _DUMP_MEM_ARR(dma_test_buf0, 2 * 512);
        _DUMP_MEM_ARR(dma_test_buf1, 2 * 512);
        while (1)
            ;
    }
#endif
}

//###########################################################################################################
#endif // End (D_FCIE_M_VER == D_FCIE_M_VER__05)

/*----------------------------------------------------------------------------------------------------------
*
* Function: IPV_SDMMC_BurnRW
*     @author jeremy.wang (2015/10/13)
* Desc: Burning R/W for IP Verification
*
* @param p_sdmmc_slot :
* @param u8TransType : DMA/ADMA/CIFD ...
* @param u32StartSec : Start Sector
* @param u32EndSec : End Sector
----------------------------------------------------------------------------------------------------------*/
void IPV_SDMMC_BurnRW(struct sstar_sdmmc_slot *p_sdmmc_slot, U8_T u8TransType, U32_T u32StartSec, U32_T u32EndSec)
{
    U32_T u32Sec, u32ClkCnt = 0, u32PatCnt = 0, u8DownLvl = 0, u8PatLvl = 0;

    prtstring("\r\n******************************* [Begin Burning ] ***************************************\r\n");

    IPV_SDMMC_SetClock(p_sdmmc_slot->p_mmc_priv, 0); // Set to Maximum Clock
    IPV_SDMMC_TestPattern(p_sdmmc_slot->ipOrder, u8PatLvl);

    if ((u32StartSec == 0) && (u32EndSec == 0))
    {
        u32StartSec = BURN_START_SECTOR;
        u32EndSec   = BURN_END_SECTOR;
    }

    for (u32Sec = u32StartSec; u32Sec < u32EndSec; u32Sec++)
    {
        u32ClkCnt++;
        u32PatCnt++;

        if (u8TransType == 1) // DMA
        {
            prtstring("Verify_DMA_RW (SEC#)=%08x.........\r\n", u32Sec);
            IPV_SDMMC_DMA_RW(p_sdmmc_slot, u32Sec, 2, TRUE);

            if ((gu16Ret1 > 0) || (gu16Ret2 > 0) || (gu16Ret3 > 0))
                return;
        }

//###########################################################################################################
#if 1 //(D_FCIE_M_VER == D_FCIE_M_VER__05)
      //###########################################################################################################
        else if (u8TransType == 2)
        {
            prtstring("Verify_ADMA_RW (SEC#)=%08x.........\r\n", u32Sec);
            IPV_SDMMC_ADMA_RW(p_sdmmc_slot, u32Sec, TRUE);

            if ((gu16Ret1 > 0) || (gu16Ret2 > 0) || (gu16Ret3 > 0))
                return;
        }
//###########################################################################################################
#endif // End (D_FCIE_M_VER == D_FCIE_M_VER__05)

        else
        {
            prtstring("Verify_CIFD_RW (SEC#)=%08x.........\r\n", u32Sec);
            IPV_SDMMC_CIFD_RW(p_sdmmc_slot, u32Sec, TRUE);

            if ((gu16Ret1 > 0) || (gu16Ret2 > 0) || (gu16Ret3 > 0))
                return;
        }

        if (u32ClkCnt >= BURN_CLK_CHG_CNT)
        {
            u32ClkCnt = 0;
            u8DownLvl++;

            if (u8DownLvl > BURN_CLK_CHG_LVL)
                u8DownLvl = 0;

            IPV_SDMMC_SetClock(p_sdmmc_slot->p_mmc_priv, 0);
        }

        if (u32PatCnt >= BURN_PAT_CHG_CNT)
        {
            u32PatCnt = 0;
            u8PatLvl++;

            if (u8PatLvl > BURN_PAT_CHG_LVL)
                u8PatLvl = 0;

            IPV_SDMMC_TestPattern(p_sdmmc_slot->ipOrder, u8PatLvl);
        }

        // Hal_Timer_mDelay(200);
    }

    prtstring("\r\n******************************* [Begin Endg ] *****************************************\r\n");
}

/*----------------------------------------------------------------------------------------------------------
*
* Function: IPV_SDMMC_TestPattern
*     @author jeremy.wang (2015/10/1)
* Desc: Test Pattern for IP Verification
*
* @param u8Slot : Slot ID
* @param u8Pattern : Pattern Number
----------------------------------------------------------------------------------------------------------*/
void IPV_SDMMC_TestPattern(U8_T u8Slot, U8_T u8Pattern)
{
    _PRT_LINE(FALSE);

    gu8Pattern = u8Pattern;
    _PRT_VALUE(u8Slot, D_TESTPATT, (U32_T)gu8Pattern, FALSE);

    _PRT_LINE(FALSE);
}

#if 1
void do_io_std_test(struct sstar_sdmmc_slot *p_sdmmc_slot)
{
    unsigned int i;

    for (i = 6; i < 16; i++)
    {
        // IPV_SDMMC_CIFD_RW(p_sdmmc_slot, i, 0);
        IPV_SDMMC_DMA_RW(p_sdmmc_slot, i, 2, 0);

//###########################################################################################################
#if 1 //(D_FCIE_M_VER == D_FCIE_M_VER__05)
      //###########################################################################################################
        IPV_SDMMC_ADMA_RW(p_sdmmc_slot, i, 0);
//###########################################################################################################
#endif
    }
}

void do_io_std_burntest(struct sstar_sdmmc_slot *p_sdmmc_slot)
{
    IPV_SDMMC_CIFD_RW(p_sdmmc_slot, 6, 0);
    IPV_SDMMC_DMA_RW(p_sdmmc_slot, 6, 2, 0);

//###########################################################################################################
#if 1 //(D_FCIE_M_VER == D_FCIE_M_VER__05)
      //###########################################################################################################
    IPV_SDMMC_ADMA_RW(p_sdmmc_slot, 6, 0);
//###########################################################################################################
#endif

    IPV_SDMMC_BurnRW(p_sdmmc_slot, 0, 300, 350); // CIFD
    IPV_SDMMC_BurnRW(p_sdmmc_slot, 1, 300, 350); // DMA

//###########################################################################################################
#if 1 //(D_FCIE_M_VER == D_FCIE_M_VER__05)
      //###########################################################################################################
    IPV_SDMMC_BurnRW(p_sdmmc_slot, 2, 300, 350); // ADMA
//###########################################################################################################
#endif
}

void IPV_SDMMC_RW_Verify(struct device *dev, U8_T u8Val1)
{
    struct sstar_sdmmc_slot *p_sdmmc_slot;
    struct list_head *       pos;
    U8_T                     u8BusWidth = 0, u8MaxWidth = 0, u8BusTiming = 0;

    list_for_each(pos, &sdmmc_command_list)
    {
        p_sdmmc_slot = list_entry(pos, struct sstar_sdmmc_slot, list);
        if (p_sdmmc_slot->ipOrder == u8Val1)
            break;
        else
            p_sdmmc_slot = NULL;
    }

    if (!p_sdmmc_slot)
    {
        pr_err(">> [sdmmc_%u] Err: No Manual Card Detect\n", u8Val1);
        return;
    }

    BUFFER_DAM_Alloc(dev);

    u8MaxWidth = p_sdmmc_slot->p_mmc_priv->mmc_PMuxInfo.u8_busWidth;

    prtstring("\r\n******************************* [ Test Start ] *****************************************\r\n");

    for (u8BusWidth = 0; u8BusWidth < 3; u8BusWidth++)
    {
        if (u8Width[u8BusWidth] > u8MaxWidth)
            break;

        for (u8BusTiming = 0; u8BusTiming < 3; u8BusTiming++)
        {
            IPV_SDMMC_Init(p_sdmmc_slot);

            _PRT_STATUS(u8Width[u8BusWidth], u8BusTiming);

            if ((u8Width[u8BusWidth] != 1))
                IPV_SDMMC_SetWideBus(p_sdmmc_slot, u8Width[u8BusWidth]);

            if (u8BusTiming == 0)
            {
                do_io_std_test(p_sdmmc_slot);
            }
            else
            {
                if (u8BusTiming == 2)
                    IPV_SDMMC_SetHighBus(p_sdmmc_slot);

                IPV_SDMMC_SetBusTiming(p_sdmmc_slot, u8BusTiming);
                if (u8BusTiming == 1)
                    IPV_SDMMC_SetClock(p_sdmmc_slot->p_mmc_priv, 12000000);
                else
                    IPV_SDMMC_SetClock(p_sdmmc_slot->p_mmc_priv, 0);
                do_io_std_burntest(p_sdmmc_slot);
            }

            if (!((u8Width[u8BusWidth] == u8MaxWidth) && (u8BusTiming == 2)))
            {
                IPV_SDMMC_TimerTest(p_sdmmc_slot->ipOrder, 3);
                IPV_SDMMC_CardDetect(p_sdmmc_slot);
            }
        }
    }

    prtstring("\r\n******************************* [ Test End ] *****************************************\r\n");

    BUFFER_DMA_FREE(dev);
}

void IPV_SDMMC_PowerSavingModeVerify(IpOrder eIP)
{
#if defined(ENABLE_EMMC_PSM_TEST) && ENABLE_EMMC_PSM_TEST
    Hal_SDMMC_PowerSavingModeVerify(eIP);
#endif
}

void SDMMC_SDIOinterrupt(IpOrder eIP)
{
    Hal_SDMMC_SDIOinterrupt(eIP);
}

#endif

//###########################################################################################################
//###########################################################################################################
#if 0
static ssize_t sdmmc_bus_width_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct sstar_sdmmc_host *p_sdmmc_host = dev_get_drvdata(dev);
    U32_T                    err          = 0, temp, width;
    struct mmc_host *        host         = p_sdmmc_host->sdmmc_slot->mmc;
    struct mmc_card *        card         = host->card;

    temp = sscanf(buf, "%d", &width);

    if (temp != 1)
    {
        return -EINVAL;
    }

    mmc_claim_host(host);
    if (geCardType[p_sdmmc_host->sdmmc_slot->ipOrder] == CARD_TYPE_SD)
        err = mmc_app_set_bus_width(card, width); // 1bit:0,4bit:2
    else if (geCardType[p_sdmmc_host->sdmmc_slot->ipOrder] == CARD_TYPE_EMMC)
    {
        if (width == 3)
            host->caps |= MMC_CAP_8_BIT_DATA;
        else if (width == 2)
        {
            host->caps &= ~MMC_CAP_8_BIT_DATA;
            host->caps |= MMC_CAP_4_BIT_DATA;
        }
        else
            printk("width: %d, input value is not match\n", 1 << width);
        err = mmc_select_bus_width(card);
    }
    mmc_release_host(host);
    if (err)
        printk("ERROR: %d, switch bus width: %d fail \n", err, width);
    else
        mmc_set_bus_width(host, width);

    return count;
}
DEVICE_ATTR(sdmmc_bus_width_set, S_IWUSR, NULL, sdmmc_bus_width_store);

static ssize_t sdmmc_clk_timing_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct sstar_sdmmc_host *p_sdmmc_host = dev_get_drvdata(dev);
    U32_T                    err          = 0, temp, clock;
    U8_T *                   status;
    struct mmc_host *        host = p_sdmmc_host->sdmmc_slot->mmc;
    struct mmc_card *        card = host->card;

    status = kmalloc(64, GFP_KERNEL);
    if (!status)
        printk("Err: Failed to Allocate status!\n");

    temp = sscanf(buf, "%d", &clock);

    if (temp != 1)
    {
        return -EINVAL;
    }

    if (12000000 < clock && clock <= 26000000)
    {
        mmc_claim_host(host);
        if (geCardType[p_sdmmc_host->sdmmc_slot->ipOrder] == CARD_TYPE_SD)
            err = mmc_sd_switch(card, 1, 0, UHS_SDR12_BUS_SPEED, status);
        else if (geCardType[p_sdmmc_host->sdmmc_slot->ipOrder] == CARD_TYPE_EMMC)
            err = __mmc_switch(card, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING, EXT_CSD_TIMING_BC,
                               card->ext_csd.generic_cmd6_time, MMC_TIMING_LEGACY, true, true);
        mmc_release_host(host);
        if (err)
            printk("ERROR: switch default speed fail, err: %d \n", err);
        else
            mmc_set_timing(host, MMC_TIMING_LEGACY); // default speed:20M
    }
    else if (26000000 < clock && clock < 50000000)
    {
        mmc_claim_host(host);
        if (geCardType[p_sdmmc_host->sdmmc_slot->ipOrder] == CARD_TYPE_SD)
            err = mmc_sd_switch(card, 1, 0, HIGH_SPEED_BUS_SPEED, status);
        else if (geCardType[p_sdmmc_host->sdmmc_slot->ipOrder] == CARD_TYPE_EMMC)
            err = __mmc_switch(card, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING, EXT_CSD_TIMING_HS,
                               card->ext_csd.generic_cmd6_time, MMC_TIMING_MMC_HS, true, true);
        mmc_release_host(host);
        if (err)
            printk("ERROR: switch high speed fail, err:%d\n", err);
        else
            mmc_set_timing(host,
                           MMC_TIMING_MMC_HS); // high speed:32M,36M,40M,43.2M,48M
    }

    mmc_set_clock(host, clock);

    return count;
}
DEVICE_ATTR(sdmmc_clk_timing_set, S_IWUSR, NULL, sdmmc_clk_timing_store);

static ssize_t sdmmc_inter_polling_store(struct device *dev, struct device_attribute *attr, const char *buf,
                                         size_t count)
{
    U32_T                    u32_temp, u32_inter_en;
    struct sstar_sdmmc_host *p_sdmmc_host = dev_get_drvdata(dev);

    u32_temp = sscanf(buf, "%d", &u32_inter_en);

    if (u32_temp != 1)
    {
        return -EINVAL;
    }
#if defined(CONFIG_SSTAR_SDMMC_INTERRUPT_MODE)
    Hal_SDMMC_MIEIntCtrl(p_sdmmc_host->sdmmc_slot->ipOrder, u32_inter_en); // 1:interrupt,0:polling
    if (Hal_CARD_INT_MIEIntRunning(p_sdmmc_host->sdmmc_slot->ipOrder))
        printk("interrupt mode\n");
    else
        printk("polling mode\n");
#else
    p_sdmmc_host = p_sdmmc_host;
    printk("disabel interrupt, polling mode\n");
#endif
    return count;
}
DEVICE_ATTR(sdmmc_inter_polling_set, S_IWUSR, NULL, sdmmc_inter_polling_store);
#endif

static ssize_t sdmmc_test_err_status_interrupt_store(struct device *dev, struct device_attribute *attr, const char *buf,
                                                     size_t count)
{
    U32_T                    u32_slotNum  = -1, u32_temp;
    struct sstar_sdmmc_host *p_sdmmc_host = dev_get_drvdata(dev);

    u32_temp = sscanf(buf, "%d", &u32_slotNum);

    if (u32_slotNum != p_sdmmc_host->sdmmc_slot->slotNo)
    {
        printk("Err : Please enter current number[ %d ]! \r\n", p_sdmmc_host->sdmmc_slot->slotNo);
        return count;
    }
    CamOsMutexLock(&sdmmc_mutex[u32_slotNum]);

    BUFFER_DAM_Alloc(dev);

    IPV_SDMMC_Init_Full(p_sdmmc_host->sdmmc_slot);
    prtstring("\033[7;31mTest read CRC error int\033[m\r\n");
    SDMMCTest_Read_CRC_Error(p_sdmmc_host->sdmmc_slot);

    IPV_SDMMC_Init_Full(p_sdmmc_host->sdmmc_slot);
    prtstring("\033[7;31mTest write CRC error int\033[m\r\n");
    SDMMCTest_Write_CRC_Error(p_sdmmc_host->sdmmc_slot);

    IPV_SDMMC_Init_Full(p_sdmmc_host->sdmmc_slot);
    prtstring("\033[7;31mTest write timeout int\033[m\r\n");
    SDMMCTest_Write_Timeout_Error(p_sdmmc_host->sdmmc_slot);

    IPV_SDMMC_Init_Full(p_sdmmc_host->sdmmc_slot);
    prtstring("\033[7;31mTest read timeout int\033[m\r\n");
    SDMMCTest_Read_Timeout_Error(p_sdmmc_host->sdmmc_slot); // We have problem again

    IPV_SDMMC_Init_Full(p_sdmmc_host->sdmmc_slot);
    prtstring("\033[7;31mTest commnad no response int\033[m\r\n");
    SDMMCTest_Command_No_Response(p_sdmmc_host->sdmmc_slot);

    IPV_SDMMC_Init_Full(p_sdmmc_host->sdmmc_slot);
    prtstring("\033[7;31mTest commnad response CRC int\033[m\r\n");
    SDMMCTest_Command_Response_CRC_Error(p_sdmmc_host->sdmmc_slot);

    BUFFER_DMA_FREE(dev);

    CamOsMutexUnlock(&sdmmc_mutex[u32_slotNum]);

    return count;
}
DEVICE_ATTR(test_sdmmc_err_status_interrupt, S_IWUSR, NULL, sdmmc_test_err_status_interrupt_store);

static ssize_t sdmmc_test_power_save_mode_store(struct device *dev, struct device_attribute *attr, const char *buf,
                                                size_t count)
{
    U32_T                    u32_slotNum  = -1, u32_temp;
    struct sstar_sdmmc_host *p_sdmmc_host = dev_get_drvdata(dev);

    u32_temp = sscanf(buf, "%d", &u32_slotNum);

    if (u32_slotNum != p_sdmmc_host->sdmmc_slot->slotNo)
    {
        printk("Err : Please enter current number[ %d ]! \r\n", p_sdmmc_host->sdmmc_slot->slotNo);
        return count;
    }
    CamOsMutexLock(&sdmmc_mutex[u32_slotNum]);
    IPV_SDMMC_PowerSavingModeVerify(p_sdmmc_host->sdmmc_slot->ipOrder);
    CamOsMutexUnlock(&sdmmc_mutex[u32_slotNum]);

    return count;
}
DEVICE_ATTR(test_sdmmc_power_save_mode, S_IWUSR, NULL, sdmmc_test_power_save_mode_store);

static ssize_t sdmmc_test_sdio_interrupt_store(struct device *dev, struct device_attribute *attr, const char *buf,
                                               size_t count)
{
    U32_T                    u32_slotNum  = -1, u32_temp;
    struct sstar_sdmmc_host *p_sdmmc_host = dev_get_drvdata(dev);

    u32_temp = sscanf(buf, "%d", &u32_slotNum);

    if (u32_slotNum != p_sdmmc_host->sdmmc_slot->slotNo)
    {
        printk("Err : Please enter current number[ %d ]! \r\n", p_sdmmc_host->sdmmc_slot->slotNo);
        return count;
    }

    CamOsMutexLock(&sdmmc_mutex[u32_slotNum]);
    SDMMC_SDIOinterrupt(p_sdmmc_host->sdmmc_slot->ipOrder);
    CamOsMutexUnlock(&sdmmc_mutex[u32_slotNum]);

    return count;
}
DEVICE_ATTR(test_sdmmc_sdio_interrupt, S_IWUSR, NULL, sdmmc_test_sdio_interrupt_store);

static ssize_t sdmmc_rw_ipverify_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    U32_T                    u32_slotNum  = -1, u32_temp;
    struct sstar_sdmmc_host *p_sdmmc_host = dev_get_drvdata(dev);

    u32_temp = sscanf(buf, "%d", &u32_slotNum);
    if (u32_slotNum != p_sdmmc_host->sdmmc_slot->slotNo)
    {
        printk("Err : Please enter current number[ %d ]! \r\n", p_sdmmc_host->sdmmc_slot->slotNo);
        return count;
    }

    CamOsMutexLock(&sdmmc_mutex[u32_slotNum]);
    IPV_SDMMC_RW_Verify(dev, p_sdmmc_host->sdmmc_slot->ipOrder);
    CamOsMutexUnlock(&sdmmc_mutex[u32_slotNum]);

    return count;
}
DEVICE_ATTR(test_sdmmc_ipverify, S_IWUSR, NULL, sdmmc_rw_ipverify_store);

static struct attribute *sstar_sdmmc_attr[] = {
#if 0
    &dev_attr_sdmmc_bus_width_set.attr,
    &dev_attr_sdmmc_clk_timing_set.attr,
    &dev_attr_sdmmc_inter_polling_set.attr,
#endif
    &dev_attr_test_sdmmc_err_status_interrupt.attr,
    &dev_attr_test_sdmmc_power_save_mode.attr,
    &dev_attr_test_sdmmc_sdio_interrupt.attr,
    &dev_attr_test_sdmmc_ipverify.attr,
    NULL,
};

struct attribute_group sstar_sdmmc_ut_attr_grp = {
    .attrs = sstar_sdmmc_attr,
};
