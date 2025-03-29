/*
 * drv_sdmmc_common.c- Sigmastar
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
 * FileName drv_sdmmc_common.c
 *     @author jeremy.wang (2012/01/10)
 * Desc:
 *     This layer between Linux SD Driver layer and IP Hal layer.
 *     (1) The goal is we don't need to change any Linux SD Driver code, but we can handle here.
 *     (2) You could define Function/Ver option for using, but don't add Project option here.
 *     (3) You could use function option by Project option, but please add to ms_sdmmc.h
 *
 ***************************************************************************************************************/

#include "hal_sdmmc_base.h"
#if (D_OS == D_OS__LINUX)
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include "drv_sdmmc_lnx.h"
#elif (D_OS == D_OS__RTK)
#ifdef CAM_OS_RTK
#include "rtk_cfg.h"
#endif
#include "drv_sdmmc_rtk.h"
#include <drv_sysdesc.h>
#include "hal_int_ctrl_pub.h"
#include "initcall.h"
#endif

#include "cam_os_wrapper.h"
#include "hal_sdmmc_timer.h"
#include "hal_sdmmc_platform.h"
#include "hal_sdmmc_platform_pri_config.h"

#include "hal_sdmmc_v5.h"
#include "hal_sdmmc_intr.h"

#if 0 // #ifdef CONFIG_CAM_CLK
#include "drv_camclk_Api.h"
#endif

#define FORCE_HAL_CLK (1) // Set 1 to use HAL driver rather than DTB. Turn this on for debugging.
//***********************************************************************************************************
// Config Setting (Internal)
//***********************************************************************************************************
#define EN_SDMMC_TRFUNC       (FALSE)
#define EN_SDMMC_TRSDIO       (FALSE)
#define EN_SDMMC_BRO_DMA      (TRUE)
#define EN_SDMMC_DCACHE_FLUSH (TRUE)

#define EN_SDMMC_NOCDZ_NDERR (FALSE)

#define MAX_SEG_CNT 128

#define ENABLE_EMMC_PRE_DEFINED_BLK_CNT 1
//***********************************************************************************************************
#if defined(CONFIG_OF)

#if 0 // #ifdef CONFIG_CAM_CLK
void* gp_clkSlot[3] = {NULL};
#else
extern struct clk *gp_clkSlot[SDMMC_NUM_TOTAL];
#endif

#endif

// Global Variable for All Slot:
//-----------------------------------------------------------------------------------------------------------
#if (D_OS == D_OS__RTK)
extern struct sstar_mmc_priv *gp_mmc_priv[SDMMC_NUM_TOTAL];
#endif

volatile CardType geCardType[SDMMC_NUM_TOTAL]       = {CARD_TYPE_SD, CARD_TYPE_SD, CARD_TYPE_SD};
U32_T             gu32_SdmmcClk[SDMMC_NUM_TOTAL]    = {0};
U32_T             gu32_SdmmcStatus[SDMMC_NUM_TOTAL] = {EV_OTHER_ERR, EV_OTHER_ERR, EV_OTHER_ERR};
S32_T             gu32_SdmmcCurCMD[SDMMC_NUM_TOTAL] = {-1, -1, -1};

CamOsMutex_t sdmmc_mutex[SDMMC_NUM_TOTAL];

// Trace Funcion
//-----------------------------------------------------------------------------------------------------------
#if (EN_SDMMC_TRFUNC)
#define pr_sd_err(fmt, arg...)  CamOsPrintf(KERN_EMERG fmt, ##arg)
#define pr_sd_main(fmt, arg...) CamOsPrintf(KERN_EMERG fmt, ##arg)
#define pr_sd_dbg(fmt, arg...)  CamOsPrintf(KERN_EMERG fmt, ##arg)
#else
#define pr_sd_err(fmt, arg...)  CamOsPrintf(KERN_EMERG fmt, ##arg)
#define pr_sd_main(fmt, arg...) // CamOsPrintf(fmt, ##arg)
#define pr_sd_dbg(fmt, arg...)  // CamOsPrintf(KERN_EMERG fmt, ##arg)
#endif

#if (EN_SDMMC_TRSDIO)
#define pr_sdio_main(fmt, arg...) CamOsPrintf(fmt, ##arg)
#else
#define pr_sdio_main(fmt, arg...)
#endif

#define CamOsMemFullFlush(ptr, len)        \
    do                                     \
    {                                      \
        CamOsMemFlush((void *)(ptr), len); \
        CamOsMiuPipeFlush();               \
    } while (0)

// Switch PAD
//------------------------------------------------------------------------------------------------
static void _SwitchPAD(struct sstar_mmc_priv *p_mmc_priv)
{
#if (PADMUX_SET == PADMUX_SET_BY_REG)
    BOOL_T bIsFakeCdz = p_mmc_priv->u8_fakeCdz;

    Hal_CARD_ConfigSdPad(p_mmc_priv->mmc_PMuxInfo);
    Hal_CARD_ConfigPowerPad(p_mmc_priv->mmc_PMuxInfo);
    if (!bIsFakeCdz)
    {
        Hal_CARD_ConfigCdzPad(p_mmc_priv->mmc_PMuxInfo);
    }
    Hal_CARD_InitPADPin(p_mmc_priv->mmc_PMuxInfo);
#endif
}

static void sstar_sdmmc_reg_bank_set(IpOrder eIP, struct sstar_mmc_priv *p_mmc_priv)
{
    Hal_CREG_SET_REG_BANK(eIP, p_mmc_priv);
    Hal_CREG_SET_PWR_SAVE_REG_BANK(eIP, p_mmc_priv);
    Hal_CREG_SET_CIFD_REG_BANK(eIP, p_mmc_priv);
    Hal_CREG_SET_PLL_REG_BANK(eIP, p_mmc_priv);
}

void sstar_sdmmc_enable(struct sstar_mmc_priv *p_mmc_priv)
{
    sstar_sdmmc_reg_bank_set(p_mmc_priv->mmc_PMuxInfo.u8_ipOrder, p_mmc_priv);

    Hal_CARD_IPOnceSetting(p_mmc_priv->mmc_PMuxInfo);
    _SwitchPAD(p_mmc_priv);

#if (D_OS == D_OS__LINUX)
    if (geCardType[p_mmc_priv->mmc_PMuxInfo.u8_ipOrder] == CARD_TYPE_EMMC)
    {
#if defined(ENABLE_EMMC_POWER_SAVING_MODE) && ENABLE_EMMC_POWER_SAVING_MODE
        Hal_SDMMC_PowerSavingModeVerify(p_mmc_priv->mmc_PMuxInfo.u8_ipOrder);
#endif
    }

    if (geCardType[p_mmc_priv->mmc_PMuxInfo.u8_ipOrder] == CARD_TYPE_EMMC)
        Hal_eMMC_HardWare_Reset(p_mmc_priv->mmc_PMuxInfo);
#endif
}

// Set Power
//------------------------------------------------------------------------------------------------
void sstar_sdmmc_setpower(struct sstar_mmc_priv *p_mmc_priv, U8_T u8PowerMode)
{
    IpOrder eIP = p_mmc_priv->mmc_PMuxInfo.u8_ipOrder;

    if (u8PowerMode == MMC_POWER_OFF)
    {
        //
        Hal_SDMMC_ClkCtrl(eIP, FALSE, 0);

#if defined(CONFIG_SUPPORT_SD30)
        // Add this for some linux version (Non 3.3V enable flow)
        if (p_mmc_priv->u8_supportSD30)
            Hal_CARD_SetPADVdd(p_mmc_priv->mmc_PMuxInfo, EV_NORVOL, WT_POWERUP);
#endif
        //
        Hal_CARD_PowerOff(p_mmc_priv->mmc_PMuxInfo, 0);

        //
        Hal_Timer_mSleep(p_mmc_priv->u32_pwerOffDelay);
    }
    else if (u8PowerMode == MMC_POWER_UP)
    {
        //
        Hal_CARD_PowerOn(p_mmc_priv->mmc_PMuxInfo, 0);

        Hal_Timer_uDelay(10); // For power-up waveform looks fine.

#if defined(CONFIG_SUPPORT_SD30)
        // Add this for some linux version (Non 3.3V enable flow)
        if (p_mmc_priv->u8_supportSD30)
            Hal_CARD_SetPADVdd(p_mmc_priv->mmc_PMuxInfo, EV_NORVOL, WT_POWERUP);
#endif
        //
        Hal_CARD_DrvCtrlPin(p_mmc_priv->mmc_PMuxInfo, p_mmc_priv->mmc_pinDrv);

        //
        Hal_Timer_mSleep(WT_POWERUP);
    }
    else if (u8PowerMode == MMC_POWER_ON)
    {
        //
        Hal_SDMMC_ClkCtrl(eIP, TRUE, 0);
        Hal_SDMMC_Reset(eIP);

        //
        Hal_Timer_mSleep(p_mmc_priv->u32_pwerOnDelay);
    }
}

//------------------------------------------------------------------------------------------------
U32_T sstar_sdmmc_setclock(struct sstar_mmc_priv *p_mmc_priv, unsigned int u32ReffClk)
{
    U32_T   u32RealClk = 0;
    IpOrder eIP        = p_mmc_priv->mmc_PMuxInfo.u8_ipOrder;

    if (u32ReffClk)
    {
        u32RealClk = Hal_CARD_FindClockSetting(eIP, (U32_T)u32ReffClk);

        gu32_SdmmcClk[eIP] = u32RealClk;

        Hal_SDMMC_ClkCtrl(eIP, FALSE, 0); // disable clock first to avoid unexpected clock rate output
#if (!FORCE_HAL_CLK) && (defined(CONFIG_OF))
#ifdef CONFIG_CAM_CLK
        //
#else
        clk_set_rate(gp_clkSlot[eIP], u32RealClk);
#endif
#else
        Hal_CARD_SetClock(&p_mmc_priv->mmc_PMuxInfo, u32RealClk);
#endif
        Hal_SDMMC_ClkCtrl(eIP, TRUE, 0); // enable clock

        Hal_SDMMC_SetNrcDelay(eIP, u32RealClk);
    }

    return u32RealClk;
}

//------------------------------------------------------------------------------------------------
void sstar_sdmmc_setbuswidth(struct sstar_mmc_priv *p_mmc_priv, U8_T u8BusWidth)
{
    IpOrder eIP = p_mmc_priv->mmc_PMuxInfo.u8_ipOrder;

    switch (u8BusWidth)
    {
        case MMC_BUS_WIDTH_1:
            Hal_SDMMC_SetDataWidth(eIP, EV_BUS_1BIT);
            break;
        case MMC_BUS_WIDTH_4:
            Hal_SDMMC_SetDataWidth(eIP, EV_BUS_4BITS);
            break;
        case MMC_BUS_WIDTH_8:
            Hal_SDMMC_SetDataWidth(eIP, EV_BUS_8BITS);
            break;
    }
}

//------------------------------------------------------------------------------------------------
void sstar_sdmmc_setbustiming(struct sstar_mmc_priv *p_mmc_priv, U8_T u8BusTiming)
{
    switch (u8BusTiming)
    {
        case MMC_TIMING_UHS_SDR12:
        case MMC_TIMING_LEGACY:
            /****** For Default Speed ******/
            Hal_CARD_SetBustiming(&p_mmc_priv->mmc_PMuxInfo, EV_BUS_DEF);
            break;

#if defined(CONFIG_SUPPORT_SD30) || defined(CONFIG_SUPPORT_EMMC50)
        case MMC_TIMING_UHS_SDR25:
        case MMC_TIMING_SD_HS:
        case MMC_TIMING_MMC_HS:
            Hal_CARD_SetBustiming(&p_mmc_priv->mmc_PMuxInfo, EV_BUS_HS);
            break;

        case MMC_TIMING_UHS_SDR50:
        case MMC_TIMING_UHS_SDR104:
            Hal_CARD_SetBustiming(&p_mmc_priv->mmc_PMuxInfo, u8BusTiming);
            break;
        case MMC_TIMING_MMC_HS200:
            Hal_CARD_SetBustiming(&p_mmc_priv->mmc_PMuxInfo, EV_BUS_HS200);
            break;
        case MMC_TIMING_MMC_HS400:
            Hal_CARD_SetBustiming(&p_mmc_priv->mmc_PMuxInfo, EV_BUS_HS400);
            break;
#else
        case MMC_TIMING_UHS_SDR25:
        case MMC_TIMING_SD_HS:
        case MMC_TIMING_MMC_HS:
        case MMC_TIMING_UHS_SDR50:
        case MMC_TIMING_UHS_SDR104:
        case MMC_TIMING_MMC_HS200:
            /****** For High Speed ******/
            Hal_CARD_SetBustiming(&p_mmc_priv->mmc_PMuxInfo, EV_BUS_HS);
            break;
#endif
#if defined(CONFIG_SUPPORT_SD30) || defined(CONFIG_SUPPORT_EMMC50)
        case MMC_TIMING_UHS_DDR50:
            Hal_CARD_SetBustiming(&p_mmc_priv->mmc_PMuxInfo, EV_BUS_DDR50);
            break;
#endif
        default:
            /****** For 300KHz IP Issue but not for Default Speed ******/
            Hal_CARD_SetBustiming(&p_mmc_priv->mmc_PMuxInfo, EV_BUS_LOW);
            break;
    }
}

//------------------------------------------------------------------------------------------------
static U16_T _PreDataBufferProcess(TransEmType eTransType, struct mmc_data *data, struct sstar_sdmmc_slot *sdmmchost,
                                   volatile dma_addr_t *ptr_AddrArr, BOOL_T *pbIsMCGOff)
{
#if (D_OS == D_OS__RTK)
    ptr_AddrArr[0] = (volatile dma_addr_t)(volatile dma_addr_t *)data->pu8Buf;
    CamOsMemFullFlush((void *)ptr_AddrArr[0], data->sg[0].length);
    return 1;
#else
    struct scatterlist *p_sg = 0;
    U8_T u8Dir = ((data->flags & MMC_DATA_READ) ? DMA_FROM_DEVICE : DMA_TO_DEVICE);
    U16_T u16sg_idx = 0;
#if (!EN_SDMMC_BRO_DMA)
    U32_T *pSGbuf = 0;
    U32_T u32TranBytes = 0;
    U32_T u32TotalSize = data->blksz * data->blocks;
    unsigned *pDMAbuf = sdmmchost->dma_buffer;

#else
    U16_T     u16SubBCnt = 0;
    U32_T     u32SubLen  = 0;
    BOOL_T    bEnd       = (FALSE);
    unsigned *pADMAbuf   = sdmmchost->adma_buffer;
    U8_T      u8MIUSel   = 0;

#endif

    if (eTransType == EV_CIF)
    {
        p_sg = &data->sg[0];
        ptr_AddrArr[0] = (dma_addr_t)(uintptr_t)(page_address(sg_page(p_sg)) + p_sg->offset);

        if (sdmmchost->p_mmc_priv->u8_cifdMCGOff)
        {
            *pbIsMCGOff = (CARD_REG(A_CLK_EN_REG((IpOrder)sdmmchost->ipOrder)) & R_MCG_DISABLE) ? TRUE : FALSE;
            if (!(*pbIsMCGOff))
                CARD_REG_SETBIT(A_CLK_EN_REG((IpOrder)sdmmchost->ipOrder), R_MCG_DISABLE);
        }
        return 1;
    }

#if (EN_SDMMC_BRO_DMA)
    for (u16sg_idx = 0; u16sg_idx < data->sg_len; u16sg_idx++)
#else
    if (data->sg_len == 1)
#endif
    {
        p_sg = &data->sg[u16sg_idx];

        // dma_map_page will flush cache in DMA_TO_DEVICE or invalidate cache in DMA_FROM_DEVICE. (only L1,L2)
        p_sg->dma_address =
            dma_map_page(&sdmmchost->parent_sdmmc->pdev->dev, sg_page(p_sg), p_sg->offset, p_sg->length, u8Dir);

        if (dma_mapping_error(&sdmmchost->parent_sdmmc->pdev->dev, p_sg->dma_address)) // Add to avoid unmap warning!
            return 0;

        if ((p_sg->dma_address == 0) || (p_sg->dma_address == ~0)) // Mapping Error!
            return 0;

        ptr_AddrArr[u16sg_idx] = (dma_addr_t)p_sg->dma_address;
    }

#if (EN_SDMMC_BRO_DMA)
    if (eTransType == EV_ADMA)
    {
        for (u16sg_idx = 0; u16sg_idx < data->sg_len; u16sg_idx++)
        {
            if (u16sg_idx == ((data->sg_len) - 1))
                bEnd = (TRUE);

            u32SubLen = data->sg[u16sg_idx].length;
            u16SubBCnt = (U16_T)(u32SubLen / data->blksz);
            Hal_SDMMC_ADMASetting((volatile void *)pADMAbuf, u16sg_idx, u32SubLen, u16SubBCnt,
                                  Hal_CARD_TransMIUAddr((dma_addr_t)ptr_AddrArr[u16sg_idx], &u8MIUSel), u8MIUSel, bEnd);
        }

        // Flush L3
        // 1. For sg_buffer DMA_TO_DEVICE.
        // 2. For sg_buffer DMA_FROM_DEVICE(invalidate L1,L2 is not enough).
        // 3. For ADMA descriptor(non-cache).
        Chip_Flush_MIU_Pipe();

        ptr_AddrArr[0] = (dma_addr_t)sdmmchost->adma_phy_addr;
        return 1;
    }
    else
    {
        // Flush L3
        // 1. For sg_buffer DMA_TO_DEVICE.
        // 2. For sg_buffer DMA_FROM_DEVICE(invalidate L1,L2 is not enough).
        Chip_Flush_MIU_Pipe();

        return (U16_T)data->sg_len;
    }
#else
    else
    {
        if (data->flags & MMC_DATA_WRITE) // SGbuf => DMA buf
        {
            while (u16sg_idx < data->sg_len)
            {
                p_sg = &data->sg[u16sg_idx];

                pSGbuf       = kmap_atomic(sg_page(p_sg)) + p_sg->offset;
                u32TranBytes = min(u32TotalSize, p_sg->length);
                memcpy(pDMAbuf, pSGbuf, u32TranBytes);
                u32TotalSize -= u32TranBytes;
                pDMAbuf += (u32TranBytes >> 2);
                kunmap_atomic(pSGbuf);

                u16sg_idx++;
            }
        }

        // Flush L3
        // 1. For sg_buffer DMA_TO_DEVICE.
        // 2. For sg_buffer DMA_FROM_DEVICE(invalidate L1,L2 is not enough).
        Chip_Flush_MIU_Pipe();

        ptr_AddrArr[0] = (dma_addr_t)sdmmchost->dma_phy_addr;
    }

    return 1;

#endif
#endif
}

//------------------------------------------------------------------------------------------------
static void _PostDataBufferProcess(TransEmType eTransType, struct mmc_data *data, struct sstar_sdmmc_slot *sdmmchost,
                                   volatile dma_addr_t *ptr_AddrArr, BOOL_T bIsMCGOff)
{
#if (D_OS == D_OS__RTK)
    if (data->flags & MMC_DATA_READ)
        CamOsMemInvalidate((void *)ptr_AddrArr[0], data->sg[0].length);
#else
    struct scatterlist *p_sg = 0;
    U8_T u8Dir = ((data->flags & MMC_DATA_READ) ? DMA_FROM_DEVICE : DMA_TO_DEVICE);
    U16_T u16sg_idx = 0;

#if (!EN_SDMMC_BRO_DMA)
    U32_T *pSGbuf = 0;
    U32_T u32TranBytes = 0;
    U32_T u32TotalSize = data->blksz * data->blocks;
    unsigned *pDMAbuf = sdmmchost->dma_buffer;
#endif

    if (eTransType == EV_CIF)
    {
        if (sdmmchost->p_mmc_priv->u8_cifdMCGOff)
        {
            if (!bIsMCGOff)
                CARD_REG_CLRBIT(A_CLK_EN_REG((IpOrder)sdmmchost->ipOrder), R_MCG_DISABLE);
        }
        return;
    }

#if (EN_SDMMC_BRO_DMA)

    for (u16sg_idx = 0; u16sg_idx < data->sg_len; u16sg_idx++)
    {
        p_sg = &data->sg[u16sg_idx];
        dma_unmap_page(&sdmmchost->parent_sdmmc->pdev->dev, p_sg->dma_address, p_sg->length, u8Dir);
    }

#else

    if (data->sg_len == 1)
    {
        p_sg = &data->sg[0];
        dma_unmap_page(&sdmmchost->parent_sdmmc->pdev->dev, p_sg->dma_address, p_sg->length, u8Dir);
    }
    else
    {
        if (data->flags & MMC_DATA_READ) // SGbuf => DMA buf
        {
            for (u16sg_idx = 0; u16sg_idx < data->sg_len; u16sg_idx++)
            {
                p_sg = &data->sg[u16sg_idx];

                pSGbuf       = kmap_atomic(sg_page(p_sg)) + p_sg->offset;
                u32TranBytes = min(u32TotalSize, p_sg->length);
                memcpy(pSGbuf, pDMAbuf, u32TranBytes);
                u32TotalSize -= u32TranBytes;
                pDMAbuf += (u32TranBytes >> 2);

                kunmap_atomic(pSGbuf);
            }
        }
    }

#endif
#endif
}

//------------------------------------------------------------------------------------------------
static U32_T _TransArrToUInt(U8_T u8Sep1, U8_T u8Sep2, U8_T u8Sep3, U8_T u8Sep4)
{
    return ((((uint)u8Sep1) << 24) | (((uint)u8Sep2) << 16) | (((uint)u8Sep3) << 8) | ((uint)u8Sep4));
}

//------------------------------------------------------------------------------------------------
static SDMMCRspEmType _TransRspType(unsigned int u32Rsp)
{
    switch (u32Rsp)
    {
        case MMC_RSP_NONE:
            return EV_NO;
        case MMC_RSP_R1:
            // case MMC_RSP_R5:
            // case MMC_RSP_R6:
            // case MMC_RSP_R7:
            return EV_R1;
        case MMC_RSP_R1B:
            return EV_R1B;
        case MMC_RSP_R2:
            return EV_R2;
        case MMC_RSP_R3:
            // case MMC_RSP_R4:
            return EV_R3;
        default:
            return EV_R1;
    }
}

//------------------------------------------------------------------------------------------------
static BOOL_T _PassPrintCMD(SlotEmType eSlot, U8_T u32Cmd, U32_T u32Arg, BOOL_T bSDIODev)
{
    if (bSDIODev)
    {
        if (u32Cmd == SD_SEND_IF_COND)
        {
            return (TRUE);
        }
        else if (u32Cmd == SD_IO_RW_DIRECT)
        {
            if ((u32Arg == 0x00000C00) || (u32Arg == 0x80000C08))
                return (TRUE);
        }
        return (FALSE);
    }

    // SD Use
    switch (u32Cmd)
    {
        /*case MMC_SEND_OP_COND:   // MMC  =>Cmd_1
        case SD_IO_SEND_OP_COND: // SDIO =>Cmd_5
        case SD_SEND_IF_COND:    // SD   =>Cmd_8
        case SD_IO_RW_DIRECT:    // SDIO =>Cmd_52
        case MMC_SEND_STATUS:    // SD   =>CMD13
        case MMC_APP_CMD:*/        // SD   =>Cmd55
        case SD_SEND_IF_COND: // SD   =>Cmd_8
#if defined(CONFIG_SUPPORT_SD30)
        case MMC_SEND_TUNING_BLOCK: // SD => Cmd19
#endif
#if defined(CONFIG_SUPPORT_EMMC50)
        case MMC_SEND_TUNING_BLOCK_HS200: // SD => Cmd19
#endif
            return (TRUE);
            break;
    }

    return (FALSE);
}

//------------------------------------------------------------------------------------------------
static int _RequestEndProcess(CmdEmType eCmdType, RspErrEmType eErrType, IpOrder eIP, struct mmc_data *data)
{
    int          nErr = 0;
    ErrGrpEmType eErrGrp;

    if (eErrType == EV_STS_OK)
    {
        pr_sdio_main("_[%01X]", Hal_SDMMC_GetDATBusLevel(eIP));
        pr_sd_main("@\n");
    }
    else
    {
        pr_sd_main("=> (Err: 0x%04X)", (U16_T)eErrType);
        nErr = (U32_T)eErrType;

        if (eCmdType != EV_CMDRSP)
        {
            eErrGrp = Hal_SDMMC_ErrGroup(eErrType);

            switch ((U16_T)eErrGrp)
            {
                case EV_EGRP_TOUT:
                    nErr = -ETIMEDOUT;
                    break;

                case EV_EGRP_COMM:
                    nErr = -EILSEQ;
                    break;
            }
        }
    }

    if (eErrType == EV_STS_OK)
        return nErr;

    /****** (2) Special Error Process for Stop Wait Process ******/
    if (eErrType == EV_SWPROC_ERR && data && EN_SDMMC_NOCDZ_NDERR)
    {
        data->bytes_xfered = data->blksz * data->blocks;
        nErr               = 0;
        pr_sd_main("_Pass");
    }

    pr_sd_main("\n");

    return nErr;
}

//------------------------------------------------------------------------------------------------
BOOL_T IsAdmaMode(struct sstar_mmc_priv *p_mmc_priv)
{
#if (D_OS == D_OS__RTK)
    return FALSE;
#endif
    // Using ADMA in default.
    return p_mmc_priv->u8_transMode;
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: sstar_sdmmc_request
 *     @author jeremy.wang (2011/5/19)
 * Desc: Request funciton for any commmand
 *
 * @param mmc_param : linux--->p_mmc_host: mmc_host structure pointer,  rtk--> slotnum
 * @param p_mmc_req :  mmc_request structure pointer
 ----------------------------------------------------------------------------------------------------------*/
void sstar_sdmmc_request(struct mmc_host *p_mmc_host, struct mmc_request *p_mmc_req)
{
#if (D_OS == D_OS__LINUX)
    struct sstar_sdmmc_slot *p_sdmmc_slot = mmc_priv(p_mmc_host);
    SlotEmType               eSlot        = (SlotEmType)p_sdmmc_slot->slotNo;
    IpOrder                  eIP          = (IpOrder)p_sdmmc_slot->ipOrder;
    struct sstar_mmc_priv *  p_mmc_priv   = p_sdmmc_slot->p_mmc_priv;
#elif (D_OS == D_OS__RTK)
    struct sstar_sdmmc_slot *p_sdmmc_slot = NULL;
    SlotEmType eSlot = p_mmc_host->slotNo;
    IpOrder eIP = gp_mmc_priv[eSlot]->mmc_PMuxInfo.u8_ipOrder;
    struct sstar_mmc_priv *p_mmc_priv = gp_mmc_priv[eSlot];
#endif
    struct mmc_command *sbc  = p_mmc_req->sbc;
    struct mmc_command *cmd  = p_mmc_req->cmd;
    struct mmc_command *stop = p_mmc_req->stop;
    struct mmc_data *   data = p_mmc_req->data;

    RspStruct *  eRspSt;
    RspErrEmType eErr     = EV_STS_OK;
    CmdEmType    eCmdType = EV_CMDRSP;
#if (EN_SDMMC_BRO_DMA)
    TransEmType eTransType = (IsAdmaMode(p_mmc_priv)) ? EV_ADMA : EV_DMA;
#else
    TransEmType eTransType = EV_DMA;
#endif
    BOOL_T              bCloseClock = FALSE;
    BOOL_T              bSDIODev    = FALSE;
    BOOL_T              bIsMCGOff   = FALSE;
    U8_T                u8CMD       = 0;
    U32_T               u32Arg      = 0;
    U32_T               u32SubLen   = 0;
    U16_T               u16BlkSize = 0, u16BlkCnt = 0, u16SubBlkCnt = 0;
    U16_T               u16ProcCnt = 0, u16Idx = 0;
    volatile dma_addr_t ptr_Addr[MAX_SEG_CNT];

    CamOsMutexLock(&sdmmc_mutex[eSlot]);

#if defined(ENABLE_EMMC_PRE_DEFINED_BLK_CNT) && ENABLE_EMMC_PRE_DEFINED_BLK_CNT
    /****** Send SET_BLOCK_COUNT Cmd ******/
    if (sbc)
    {
        u8CMD  = (U8_T)sbc->opcode;
        u32Arg = (U32_T)sbc->arg;
        pr_sd_main(">> [sdmmc_%u]_CMD_%u (0x%08X)", eSlot, u8CMD, u32Arg);

        Hal_SDMMC_SetCmdToken(eIP, u8CMD, u32Arg);
        Hal_SDMMC_SetSDIOIntBeginSetting(eIP, u8CMD, u32Arg, EV_CMDRSP, 0);
        eErr = Hal_SDMMC_SendCmdAndWaitProcess(eIP, EV_EMP, EV_CMDRSP, _TransRspType(mmc_resp_type(sbc)), TRUE);
        Hal_SDMMC_SetSDIOIntEndSetting(eIP, eErr, 0);

        sbc->error = _RequestEndProcess(EV_CMDRSP, eErr, eIP, data);

        eRspSt       = Hal_SDMMC_GetRspToken(eIP);
        sbc->resp[0] = _TransArrToUInt(eRspSt->u8ArrRspToken[1], eRspSt->u8ArrRspToken[2], eRspSt->u8ArrRspToken[3],
                                       eRspSt->u8ArrRspToken[4]);

        if (sbc->error)
            pr_sd_err(">> [sdmmc_%u] Err: #Cmd_%u(0x%08X) => (E: 0x%04X)(S: 0x%08X)__(L:%u)\n", eSlot, u8CMD, u32Arg,
                      (U16_T)eErr, sbc->resp[0], eRspSt->u32ErrLine);
    }
#endif

    u8CMD  = (U8_T)cmd->opcode;
    u32Arg = (U32_T)cmd->arg;
#if (D_OS == D_OS__LINUX)
    bSDIODev = (BOOL_T)p_sdmmc_slot->sdioFlag;
    if (!p_sdmmc_slot->mmc->card)
        Hal_SDMMC_StopProcessCtrl(eIP, FALSE);
#endif
    pr_sdio_main("_[%01X]_", Hal_SDMMC_GetDATBusLevel());
    pr_sd_main(">> [sdmmc_%u] CMD_%u (0x%08X)\n", eSlot, u8CMD, u32Arg);

    Hal_SDMMC_SetCmdToken(eIP, u8CMD, u32Arg);

    /****** Simple SD command *******/
    if (!data)
    {
        Hal_SDMMC_SetSDIOIntBeginSetting(eIP, u8CMD, u32Arg, EV_CMDRSP, 0);
        eErr = Hal_SDMMC_SendCmdAndWaitProcess(eIP, EV_EMP, EV_CMDRSP, _TransRspType(mmc_resp_type(cmd)), TRUE);
        Hal_SDMMC_SetSDIOIntEndSetting(eIP, eErr, 0);
    }
    else // R/W SD Command
    {
        u16BlkSize   = (U16_T)data->blksz;
        u16BlkCnt    = (U16_T)data->blocks;
        u32SubLen    = (U32_T)data->sg[0].length;
        u16SubBlkCnt = (U16_T)(u32SubLen / u16BlkSize);

        if ((u8CMD == 17) || (u8CMD == 18) || (u8CMD == 24) || (u8CMD == 25))
        {
            gu32_SdmmcCurCMD[eIP] = u8CMD;
            gu32_SdmmcStatus[eIP] = EV_STS_OK;
        }

        eCmdType    = ((data->flags & MMC_DATA_READ) ? EV_CMDREAD : EV_CMDWRITE);
        bCloseClock = ((stop) ? FALSE : TRUE);

        pr_sd_main("__[Sgl: %u] (TB: %u)(BSz: %u)\n", (U16_T)data->sg_len, u16BlkCnt, u16BlkSize);

        u16ProcCnt = _PreDataBufferProcess(eTransType, data, p_sdmmc_slot, ptr_Addr, &bIsMCGOff);
        if (u16ProcCnt == 0)
        {
            pr_sd_err("\n>> [sdmmc_%u] Err: DMA Mapping Addr Error!\n", eSlot);
            eErr = EV_OTHER_ERR;
            goto LABEL_SD_ERR;
        }
        else if (u16ProcCnt == 1)
        {
            u32SubLen    = u16BlkSize * u16BlkCnt;
            u16SubBlkCnt = u16BlkCnt;
        }
        pr_sd_dbg("\n____[0] =>> (SBCnt: %u)__[Addr: 0x%llx]", u16SubBlkCnt, (U64_T)ptr_Addr[0]);

        Hal_SDMMC_TransCmdSetting(eIP, eTransType, u16SubBlkCnt, u16BlkSize,
                                  Hal_CARD_TransMIUAddr((dma_addr_t)ptr_Addr[0], NULL),
                                  (volatile U8_T *)(uintptr_t)ptr_Addr[0]);
        Hal_SDMMC_SetSDIOIntBeginSetting(eIP, u8CMD, u32Arg, eCmdType, u16BlkCnt);
        eErr =
            Hal_SDMMC_SendCmdAndWaitProcess(eIP, eTransType, eCmdType, _TransRspType(mmc_resp_type(cmd)), bCloseClock);

        if (((U16_T)eErr) == EV_STS_OK)
        {
            data->bytes_xfered += u32SubLen;

            /****** Broken DMA *******/
            for (u16Idx = 1; u16Idx < u16ProcCnt; u16Idx++)
            {
                u32SubLen    = (U32_T)data->sg[u16Idx].length;
                u16SubBlkCnt = (U16_T)(u32SubLen / u16BlkSize);
                pr_sd_dbg("\n____[%u] =>> (SBCnt: %u)__[Addr: 0x%llx]", u16Idx, u16SubBlkCnt, (U64_T)ptr_Addr[u16Idx]);

                Hal_SDMMC_TransCmdSetting(eIP, eTransType, u16SubBlkCnt, u16BlkSize,
                                          Hal_CARD_TransMIUAddr(ptr_Addr[u16Idx], NULL),
                                          (volatile U8_T *)(uintptr_t)ptr_Addr[u16Idx]);
                eErr = Hal_SDMMC_RunBrokenDmaAndWaitProcess(eIP, eCmdType);

                if ((U16_T)eErr)
                    break;
                data->bytes_xfered += u32SubLen;
            }
        }
        else
        {
            if ((u8CMD == 17) || (u8CMD == 18) || (u8CMD == 24) || (u8CMD == 25))
                gu32_SdmmcStatus[eIP] = eErr;
        }

        Hal_SDMMC_SetSDIOIntEndSetting(eIP, eErr, u16BlkCnt);
        _PostDataBufferProcess(eTransType, data, p_sdmmc_slot, ptr_Addr, bIsMCGOff);
    }

LABEL_SD_ERR:
    cmd->error = _RequestEndProcess(eCmdType, eErr, eIP, data);

    if (data)
        data->error = cmd->error;

    eRspSt       = Hal_SDMMC_GetRspToken(eIP);
    cmd->resp[0] = _TransArrToUInt(eRspSt->u8ArrRspToken[1], eRspSt->u8ArrRspToken[2], eRspSt->u8ArrRspToken[3],
                                   eRspSt->u8ArrRspToken[4]);
    if (eRspSt->u8RspSize == 0x10)
    {
        cmd->resp[1] = _TransArrToUInt(eRspSt->u8ArrRspToken[5], eRspSt->u8ArrRspToken[6], eRspSt->u8ArrRspToken[7],
                                       eRspSt->u8ArrRspToken[8]);
        cmd->resp[2] = _TransArrToUInt(eRspSt->u8ArrRspToken[9], eRspSt->u8ArrRspToken[10], eRspSt->u8ArrRspToken[11],
                                       eRspSt->u8ArrRspToken[12]);
        cmd->resp[3] =
            _TransArrToUInt(eRspSt->u8ArrRspToken[13], eRspSt->u8ArrRspToken[14], eRspSt->u8ArrRspToken[15], 0);
    }

    /****** Print Error Message******/
    if (!data && cmd->error && !_PassPrintCMD(eSlot, u8CMD, u32Arg, bSDIODev)) // Cmd Err but Pass Print Some Cmds
    {
        if (cmd->error == -EILSEQ)
        {
            pr_sd_err(">> [sdmmc_%u] Warn: #Cmd_%u (0x%08X)=>(E: 0x%04X)(S: 0x%08X)__(L:%u)\n", eSlot, u8CMD, u32Arg,
                      (U16_T)eErr, cmd->resp[0], eRspSt->u32ErrLine);
        }
        else
        {
            pr_sd_err(">> [sdmmc_%u] Err: #Cmd_%u (0x%08X)=>(E: 0x%04X)(S: 0x%08X)__(L:%u)\n", eSlot, u8CMD, u32Arg,
                      (U16_T)eErr, cmd->resp[0], eRspSt->u32ErrLine);
        }
    }
    else if (data && data->error && !_PassPrintCMD(eSlot, u8CMD, u32Arg, bSDIODev)) // Data Err
    {
        if (data->error == -EILSEQ)
        {
            pr_sd_err(">> [sdmmc_%u] Warn: #Cmd_%u (0x%08X)=>(E: 0x%04X)(S: 0x%08X)__(L:%u)(B:%u/%u)(I:%u/%u)\n", eSlot,
                      u8CMD, u32Arg, (U16_T)eErr, cmd->resp[0], eRspSt->u32ErrLine, u16SubBlkCnt, u16BlkCnt, u16Idx,
                      u16ProcCnt);
        }
        else
        {
            pr_sd_err(">> [sdmmc_%u] Err: #Cmd_%u (0x%08X)=>(E: 0x%04X)(S: 0x%08X)__(L:%u)(B:%u/%u)(I:%u/%u)\n", eSlot,
                      u8CMD, u32Arg, (U16_T)eErr, cmd->resp[0], eRspSt->u32ErrLine, u16SubBlkCnt, u16BlkCnt, u16Idx,
                      u16ProcCnt);
        }
    }

    /****** Send Stop Cmd ******/
    if ((stop)
#if defined(ENABLE_EMMC_PRE_DEFINED_BLK_CNT) && ENABLE_EMMC_PRE_DEFINED_BLK_CNT
        && !sbc
#endif
    )
    {
        u8CMD  = (U8_T)stop->opcode;
        u32Arg = (U32_T)stop->arg;
        pr_sd_main(">> [sdmmc_%u]_CMD_%u (0x%08X)", eSlot, u8CMD, u32Arg);

        Hal_SDMMC_SetCmdToken(eIP, u8CMD, u32Arg);
        Hal_SDMMC_SetSDIOIntBeginSetting(eIP, u8CMD, u32Arg, EV_CMDRSP, 0);
        eErr = Hal_SDMMC_SendCmdAndWaitProcess(eIP, EV_EMP, EV_CMDRSP, _TransRspType(mmc_resp_type(stop)), TRUE);
        Hal_SDMMC_SetSDIOIntEndSetting(eIP, eErr, 0);

        stop->error = _RequestEndProcess(EV_CMDRSP, eErr, eIP, data);

        eRspSt        = Hal_SDMMC_GetRspToken(eIP);
        stop->resp[0] = _TransArrToUInt(eRspSt->u8ArrRspToken[1], eRspSt->u8ArrRspToken[2], eRspSt->u8ArrRspToken[3],
                                        eRspSt->u8ArrRspToken[4]);

        if (stop->error)
            pr_sd_err(">> [sdmmc_%u] Err: #Cmd_12 => (E: 0x%04X)(S: 0x%08X)__(L:%u)\n", eSlot, (U16_T)eErr,
                      stop->resp[0], eRspSt->u32ErrLine);
    }

    CamOsMutexUnlock(&sdmmc_mutex[eSlot]);
#if (D_OS == D_OS__LINUX)
    mmc_request_done(p_mmc_host, p_mmc_req);
#endif
}
