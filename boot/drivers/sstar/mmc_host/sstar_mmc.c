/*
 * sstar_mmc.c- Sigmastar
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

#include <mmc.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <errno.h>

#include <asm/cache.h>
#include "hal_card_base.h"
#include "hal_sdmmc_v5.h"
#include "hal_card_platform.h"
#include "hal_card_timer.h"
#include "sstar_sdio.h"

#define MMC_POWER_OFF       0
#define MMC_POWER_UP        1
#define MMC_POWER_ON        2
#define MMC_POWER_UNDEFINED 3

struct sstar_mmc_plat
{
    struct mmc_config cfg;
    struct mmc        mmc;
    struct sdio_card  card;
};

struct sstar_mmc_priv
{
    U8_T          u8_slotNo;
    U8_T          u8_pwerOnDelay;
    U8_T          u8_pwerOffDelay;
    U8_T          u8_devType;  // SD, SDIO ,EMMC
    U8_T          u8_tansMode; // dma, adma, cifd
    U8_T          u8_fakeCdz;
    U8_T          u8_revCdz;
    U8_T          u8_uhsSupport;
    U8_T          u8_sdioUse;
    MMCPinDrv     mmc_pinDrv;
    MMCClkPhase   mmc_clkPha;
    MMCPadMuxInfo mmc_PMuxInfo;
};

#define EN_SDMMC_TRFUNC (FALSE)

#if (EN_SDMMC_TRFUNC)
#define pr_sd_err(fmt, arg...)  printf(fmt, ##arg)
#define pr_sd_main(fmt, arg...) printf(fmt, ##arg)
#define pr_sd_dbg(fmt, arg...)  //
#else
#define pr_sd_err(fmt, arg...)  printf(fmt, ##arg)
#define pr_sd_main(fmt, arg...) // printf(fmt, ##arg)
#define pr_sd_dbg(fmt, arg...)  // printf(fmt, ##arg)
#endif

extern void flush_dcache_range(unsigned long start, unsigned long stop);
extern void invalidate_dcache_range(unsigned long start, unsigned long stop);
//------------------------------------------------------------------------------------------------
static uint _TransArrToUInt(U8_T u8Sep1, U8_T u8Sep2, U8_T u8Sep3, U8_T u8Sep4)
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
        case MMC_RSP_R1: // MMC_RSP_R5, MMC_RSP_R6, MMC_RSP_R7
            return EV_R1;
        case MMC_RSP_R1b:
            return EV_R1B;
        case MMC_RSP_R2:
            return EV_R2;
        case MMC_RSP_R3: // MMC_RSP_R4
            return EV_R3;
    }

    return EV_R1;
}

static int _RequestErrProcess(RspErrEmType eErrType)
{
    ErrGrpEmType eErrGrp;
    int          nErr = 0;

    if (eErrType == EV_STS_OK)
    {
        pr_sd_main("@\n");
        return nErr;
    }
    else
    {
        pr_sd_main("=> (Err: 0x%04X)\n", (U16_T)eErrType);

        nErr = (U32_T)eErrType;

        eErrGrp = Hal_SDMMC_ErrGroup(eErrType);

        switch ((U16_T)eErrGrp)
        {
            case EV_EGRP_TOUT:
                nErr = -ETIMEDOUT;
                break;

            case EV_EGRP_COMM:
                nErr = ECOMM;
                break;
        }
    }

    return nErr;
}

//------------------------------------------------------------------------------------------------
static U32_T _SetClock(IpOrder eIP, MMCClkPhase *pmmc_clkPha, unsigned int u32ReffClk, U8_T u8PassLevel,
                       U8_T u8DownLevel)
{
    U32_T u32RealClk = 0;

    if (u32ReffClk)
    {
        u32RealClk = Hal_CARD_FindClockSetting(eIP, (U32_T)u32ReffClk);
        Hal_CARD_SetClock(eIP, pmmc_clkPha, u32RealClk);
        Hal_SDMMC_SetNrcDelay(eIP, u32RealClk);
    }

    return u32RealClk;
}

// Set Power
//------------------------------------------------------------------------------------------------
static void _SetPower(struct sstar_mmc_priv priv, U8_T u8PowerMode)
{
    MMCPadMuxInfo mmc_PMuxInfo = priv.mmc_PMuxInfo;
    IpOrder       eIP          = mmc_PMuxInfo.u8_ipOrder;
    //    PadOrder      ePAD         = mmc_PMuxInfo.u8_padOrder;

    if (u8PowerMode == MMC_POWER_OFF)
    {
        //
        Hal_SDMMC_ClkCtrl(eIP, FALSE, 0);

        //
        Hal_CARD_PowerOff(mmc_PMuxInfo, 0);

        //
        Hal_CARD_PullPADPin(mmc_PMuxInfo, EV_PULLDOWN);

        //
        Hal_Timer_mSleep(priv.u8_pwerOffDelay);
    }
    else if (u8PowerMode == MMC_POWER_UP)
    {
        //
        Hal_CARD_PowerOn(mmc_PMuxInfo, 0);

        Hal_Timer_uDelay(10); // For power-up waveform looks fine.
        Hal_CARD_PullPADPin(mmc_PMuxInfo, EV_PULLUP);

        //
        Hal_CARD_DrvCtrlPin(mmc_PMuxInfo, priv.mmc_pinDrv);

        //
        Hal_Timer_mSleep(WT_POWERUP);
    }
    else if (u8PowerMode == MMC_POWER_ON)
    {
        //
        Hal_SDMMC_ClkCtrl(eIP, TRUE, 0);
        Hal_SDMMC_Reset(eIP);

        //
        Hal_Timer_mSleep(priv.u8_pwerOnDelay);
    }
}

static int sstar_mmc_dm_send_cmd(struct udevice *dev, struct mmc_cmd *cmd, struct mmc_data *data)
{
    struct sstar_mmc_priv *priv = dev_get_priv(dev);
    // struct mmc *           mmc  = mmc_get_mmc_dev(dev);

    RspStruct *  eRspSt;
    RspErrEmType eErr = EV_STS_OK;
    CmdEmType    eCmdType;
    TransEmType  eTransType = EV_DMA;

    IpOrder eIP = priv->mmc_PMuxInfo.u8_ipOrder;
    U8_T    u8_cmdIdx;
    U16_T   u16_blkSize = 0, u16_blkCnt;
    U32_T   u32_arg     = 0;
    BOOL_T  bCloseClock = FALSE;

    u8_cmdIdx = cmd->cmdidx;
    u32_arg   = cmd->cmdarg;
    volatile U8_T *pu8Buf;
    int            nErr  = 0;
    unsigned long  Csize = 0;

    pr_sd_main("_[mmc_%u] CMD_%u (0x%08X)", priv->u8_slotNo, u8_cmdIdx, u32_arg);

    Hal_SDMMC_SetCmdToken(eIP, u8_cmdIdx, u32_arg);

    if (!data)
    {
        if (MMC_RSP_R1b == cmd->resp_type)
            bCloseClock = FALSE;
        else
            bCloseClock = TRUE;
        eErr = Hal_SDMMC_SendCmdAndWaitProcess(eIP, EV_EMP, EV_CMDRSP, _TransRspType(cmd->resp_type), bCloseClock);
    }
    else
    {
        u16_blkSize = (U16_T)data->blocksize;
        u16_blkCnt  = (U16_T)data->blocks;
        Csize       = u16_blkSize * u16_blkCnt;

        if (u8_cmdIdx == 51)
            Csize = CONFIG_SYS_CACHELINE_SIZE;

        pr_sd_main("__(TB: %u)(BSz: %u)(Csize: %lx)\n", u16_blkCnt, u16_blkSize, Csize);

        if (data->flags & MMC_DATA_READ)
        {
            eCmdType = EV_CMDREAD;
            pu8Buf   = (volatile U8_T *)data->dest;
        }
        else
        {
            eCmdType = EV_CMDWRITE;
            pu8Buf   = (volatile U8_T *)data->src;
        }

        if ((u16_blkCnt > 1) || (MMC_RSP_R1b == cmd->resp_type))
            bCloseClock = FALSE;
        else
            bCloseClock = TRUE;

        flush_dcache_range((uintptr_t)pu8Buf, ALIGN((uintptr_t)(pu8Buf + Csize), ARCH_DMA_MINALIGN));

        Hal_SDMMC_TransCmdSetting(eIP, eTransType, u16_blkCnt, u16_blkSize,
                                  Hal_CARD_TransMIUAddr((uintptr_t)pu8Buf, NULL), pu8Buf);
        eErr = Hal_SDMMC_SendCmdAndWaitProcess(eIP, eTransType, eCmdType, _TransRspType(cmd->resp_type), bCloseClock);

        if (eCmdType == EV_CMDREAD)
            invalidate_dcache_range((uintptr_t)pu8Buf, ALIGN((uintptr_t)(pu8Buf + Csize), ARCH_DMA_MINALIGN));
    }

    nErr = _RequestErrProcess(eErr);

    eRspSt = Hal_SDMMC_GetRspToken(eIP);

    cmd->response[0] = _TransArrToUInt(eRspSt->u8ArrRspToken[1], eRspSt->u8ArrRspToken[2], eRspSt->u8ArrRspToken[3],
                                       eRspSt->u8ArrRspToken[4]);
    cmd->response[1] = _TransArrToUInt(eRspSt->u8ArrRspToken[5], eRspSt->u8ArrRspToken[6], eRspSt->u8ArrRspToken[7],
                                       eRspSt->u8ArrRspToken[8]);
    cmd->response[2] = _TransArrToUInt(eRspSt->u8ArrRspToken[9], eRspSt->u8ArrRspToken[10], eRspSt->u8ArrRspToken[11],
                                       eRspSt->u8ArrRspToken[12]);
    cmd->response[3] =
        _TransArrToUInt(eRspSt->u8ArrRspToken[13], eRspSt->u8ArrRspToken[14], eRspSt->u8ArrRspToken[15], 0);

    return nErr;
}

static int sstar_mmc_dm_set_ios(struct udevice *dev)
{
    struct sstar_mmc_priv *priv = dev_get_priv(dev);
    struct mmc *           mmc  = mmc_get_mmc_dev(dev);

    unsigned int u32RealClk = 0;
    IpOrder      eIP        = priv->mmc_PMuxInfo.u8_ipOrder;

    pr_sd_main("_[sdmmc_%u] mmc_set_ios =>", (U32_T)eIP);

    // Set up Clock
    u32RealClk = _SetClock(eIP, &priv->mmc_clkPha, mmc->clock, 0, 0);

    pr_sd_main("RealClk=%u ", u32RealClk);

    if (mmc->card_caps & MMC_MODE_HS)
    {
        Hal_SDMMC_SetBusTiming(eIP, EV_BUS_HS);
        pr_sd_main("[HS] ");
    }
    else
    {
        if (u32RealClk <= 400000)
        {
            Hal_SDMMC_SetBusTiming(eIP, EV_BUS_LOW);
            pr_sd_main("[LS] ");
        }
        else
        {
            Hal_SDMMC_SetBusTiming(eIP, EV_BUS_DEF);
            pr_sd_main("[DS] ");
        }
    }

    // Set up DataWidth
    if (mmc->bus_width == 1)
        Hal_SDMMC_SetDataWidth(eIP, EV_BUS_1BIT);
    else if (mmc->bus_width == 4)
        Hal_SDMMC_SetDataWidth(eIP, EV_BUS_4BITS);
    else if (mmc->bus_width == 8)
        Hal_SDMMC_SetDataWidth(eIP, EV_BUS_8BITS);

    pr_sd_main(", BusWidth=%u\n", mmc->bus_width);

    return 0;
};

static int sstar_mmc_dm_deferred_probe(struct udevice *dev)
{
    struct sstar_mmc_priv *priv = dev_get_priv(dev);
    // struct mmc *           mmc  = mmc_get_mmc_dev(dev);

    IpOrder eIP = priv->mmc_PMuxInfo.u8_ipOrder;

    pr_sd_main("_[sdmmc_%u] sstar_mmc_dm_deferred_probe. \n", (U32_T)eIP);

    Hal_CARD_IPOnceSetting(eIP);
    if (priv->u8_sdioUse)
    {
        Hal_SDMMC_SDIODeviceCtrl(eIP, true);
    }
    Hal_CARD_ConfigSdPad(priv->mmc_PMuxInfo);
    Hal_CARD_ConfigPowerPad(priv->mmc_PMuxInfo);
    Hal_CARD_ConfigCdzPad(priv->mmc_PMuxInfo);
    Hal_CARD_InitPADPin(priv->mmc_PMuxInfo);
    _SetPower(*priv, MMC_POWER_OFF);
    _SetPower(*priv, MMC_POWER_UP);
    _SetClock(eIP, &priv->mmc_clkPha, 300000, 0, 0);
    _SetPower(*priv, MMC_POWER_ON);

    return 0;
}

static int sstar_mmc_dm_cd(struct udevice *dev)
{
    struct sstar_mmc_priv *priv = dev_get_priv(dev);

    if (priv->u8_fakeCdz)
    {
        return (TRUE);
    }
    else
    {
        if (priv->u8_revCdz)
            return !Hal_CARD_GetCdzState(priv->mmc_PMuxInfo);
        else
            return Hal_CARD_GetCdzState(priv->mmc_PMuxInfo);
    }

    return (FALSE);
}

static int sstar_mmc_dm_wait(struct udevice *dev, int state, int timeout_us)
{
    struct sstar_mmc_priv *priv = dev_get_priv(dev);

    IpOrder eIP = priv->mmc_PMuxInfo.u8_ipOrder;

    return HAL_SDMMC_WaitDat0(eIP, state, timeout_us / 1000);
}

static const struct dm_mmc_ops sstar_msc_ops = {
    .send_cmd       = sstar_mmc_dm_send_cmd,
    .set_ios        = sstar_mmc_dm_set_ios,
    .deferred_probe = sstar_mmc_dm_deferred_probe,
    .get_cd         = sstar_mmc_dm_cd,
    .wait_dat0      = sstar_mmc_dm_wait,
};

static int sstar_mmc_of_to_plat(struct udevice *dev)
{
    struct sstar_mmc_priv *priv = dev_get_priv(dev);
    struct sstar_mmc_plat *plat = dev_get_plat(dev);
    struct mmc_config *    cfg;
    int                    ret;

    cfg = &plat->cfg;

    cfg->name      = "MSC";
    cfg->host_caps = MMC_MODE_HS_52MHz | MMC_MODE_HS;

    ret = mmc_of_parse(dev, cfg);
    if (ret < 0)
    {
        dev_err(dev, "failed to parse host caps\n");
        return ret;
    }

    cfg->f_min = 300000;
    if (cfg->f_max <= 0)
        cfg->f_max = 48000000;

    cfg->voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;
    cfg->b_max    = CONFIG_SYS_MMC_MAX_BLK_COUNT;

    memset(&(priv->mmc_PMuxInfo), 0xff, sizeof(MMCPadMuxInfo));

    priv->u8_slotNo                  = dev_seq(dev);
    priv->mmc_PMuxInfo.u8_ipOrder    = dev_read_u32_default(dev, "ip-order", priv->u8_slotNo);
    priv->mmc_PMuxInfo.u8_padOrder   = dev_read_u32_default(dev, "pad-order", 0);
    priv->u8_pwerOnDelay             = dev_read_u32_default(dev, "pwr-on-delay", 50);
    priv->u8_pwerOffDelay            = dev_read_u32_default(dev, "pwr-off-delay", 50);
    priv->u8_sdioUse                 = dev_read_u32_default(dev, "sdio-use", 0);
    priv->mmc_PMuxInfo.u8_busWidth   = dev_read_u32_default(dev, "bus-width", 4);
    priv->u8_fakeCdz                 = dev_read_u32_default(dev, "fake-cdz", 0);
    priv->u8_revCdz                  = dev_read_u32_default(dev, "rev-cdz", 0);
    priv->mmc_pinDrv.eDrvClk         = dev_read_u32_default(dev, "clk-driving", 0);
    priv->mmc_pinDrv.eDrvCmd         = dev_read_u32_default(dev, "cmd-driving", 0);
    priv->mmc_pinDrv.eDrvData        = dev_read_u32_default(dev, "data-driving", 0);
    priv->mmc_clkPha.u8_clkPhaEn     = dev_read_u32_default(dev, "en-clk-phase", 0);
    priv->mmc_clkPha.eClkPha_RX      = dev_read_u32_default(dev, "rx-clk-phase", 0);
    priv->mmc_clkPha.eClkPha_TX      = dev_read_u32_default(dev, "tx-clk-phase", 0);
    priv->mmc_clkPha.u8_eightPhaEn   = dev_read_u32_default(dev, "en-eight-phase", 0);
    priv->mmc_clkPha.u8_eightPha_RX  = dev_read_u32_default(dev, "rx-eight-phase", 0);
    priv->mmc_clkPha.u8_eightPha_TX  = dev_read_u32_default(dev, "tx-eight-phase", 0);
    priv->mmc_PMuxInfo.u32_PinPWR    = dev_read_u32_default(dev, "pwr-pad", 0);
    priv->mmc_PMuxInfo.u32_PinCdzRst = dev_read_u32_default(dev, "cdz-pad", 0);
    Hal_CARD_PadmuxGetting(&(priv->mmc_PMuxInfo));
    return 0;
}

static int sstar_mmc_bind(struct udevice *dev)
{
    struct sstar_mmc_plat *plat = dev_get_plat(dev);
    return mmc_bind(dev, &plat->mmc, &plat->cfg);
}

static int sstar_mmc_probe(struct udevice *dev)
{
    struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
    struct sstar_mmc_priv * priv  = dev_get_priv(dev);
    struct sstar_mmc_plat * plat  = dev_get_plat(dev);

    plat->mmc.priv = priv;
    upriv->mmc     = &plat->mmc;
    return 0;
}

static const struct udevice_id sstar_mmc_ids[] = {{.compatible = "sstar-mmc"}, {}};

U_BOOT_DRIVER(sstar_mmc_drv) = {
    .name       = "sstar_mmc",
    .id         = UCLASS_MMC,
    .of_match   = sstar_mmc_ids,
    .of_to_plat = sstar_mmc_of_to_plat,
    .bind       = sstar_mmc_bind,
    .probe      = sstar_mmc_probe,
    .priv_auto  = sizeof(struct sstar_mmc_priv),
    .plat_auto  = sizeof(struct sstar_mmc_plat),
    .ops        = &sstar_msc_ops,
};
