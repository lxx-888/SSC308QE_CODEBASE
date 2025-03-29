/*
 * drv_sdmmc_rtk.c- Sigmastar
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
 * FileName drv_sdmmc_rtk.c
 *     @author jeremy.wang (2012/01/10)
 * Desc:
 *     This layer between Linux SD Driver layer and IP Hal layer.
 *     (1) The goal is we don't need to change any Linux SD Driver code, but we can handle here.
 *     (2) You could define Function/Ver option for using, but don't add Project option here.
 *     (3) You could use function option by Project option, but please add to ms_sdmmc.h
 *
 ***************************************************************************************************************/

#ifdef CAM_OS_RTK
#include "rtk_cfg.h"
#endif

#include "cam_os_wrapper.h"
#include "drv_sdmmc_rtk.h"
#include "drv_sdmmc_common.h"
#include "hal_sdmmc_timer.h"
#include "hal_sdmmc_platform.h"
#include "hal_sdmmc_platform_pri_config.h"

#include "hal_sdmmc_v5.h"
#include "hal_sdmmc_intr.h"

#include "sdmmc_irq.h"
#include <drv_sysdesc.h>
#include "hal_int_ctrl_pub.h"
#include "initcall.h"

#if 0 // #ifdef CONFIG_CAM_CLK
#include "drv_camclk_Api.h"
#endif

#include "../../../../padmux/drv/pub/drv_padmux.h"

//***********************************************************************************************************
// Config Setting (Internal)
//***********************************************************************************************************
#define EN_SDMMC_TRFUNC (FALSE)

//***********************************************************************************************************
#ifdef CONFIG_PM_SLEEP
static U16_T gu16_SlotIPClk[SDMMC_NUM_TOTAL]    = {0};
static U16_T gu16_SlotBlockClk[SDMMC_NUM_TOTAL] = {0};
#endif

#if defined(CONFIG_OF)

#if 0 // #ifdef CONFIG_CAM_CLK
void* gp_clkSlot[3] = {NULL};
#else
struct clk *gp_clkSlot[SDMMC_NUM_TOTAL];
#endif

#endif

// Global Variable for All Slot:
//-----------------------------------------------------------------------------------------------------------
static struct msSt_SD_SlotInfo _stSDInfo[SDMMC_NUM_TOTAL];
static BOOL_T                  gb_SDReady = FALSE;
struct sstar_mmc_priv *        gp_mmc_priv[SDMMC_NUM_TOTAL];
CamOsMutex_t                   sdmmc_init_mutex[SDMMC_NUM_TOTAL];
volatile BOOL_T                gb_SDCardInitFlag[SDMMC_NUM_TOTAL] = {FALSE};

// workqueue for cdz irq
CamOsWorkQueue cdzWorkQueue[SDMMC_NUM_TOTAL], hotplugWorkQueue[SDMMC_NUM_TOTAL];

CamOsMutex_t sdmmc_mutex[SDMMC_NUM_TOTAL];

//
static IntSourceStruct   gst_IntSourceSlot[3];
volatile IpType          geIpTypeIp[SDMMC_NUM_TOTAL]              = {IP_0_TYPE, IP_1_TYPE, IP_2_TYPE};
BOOL_T                   gb_Sdio_Dis_Intr_By_IP[SDMMC_NUM_TOTAL]  = {FALSE, FALSE, FALSE};
U32_T                    gu32_SlotEnPwrHighValid[SDMMC_NUM_TOTAL] = {0, 0, 0};
extern volatile CardType geCardType[SDMMC_NUM_TOTAL];

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

BOOL_T MS_SD_Card_Detect(unsigned char u8Slot)
{
    if (gp_mmc_priv[u8Slot]->u8_fakeCdz)
    {
        return (TRUE);
    }
    else
    {
        if (gp_mmc_priv[u8Slot]->u8_revCdz)
            return !Hal_CARD_GetCdzState(gp_mmc_priv[u8Slot]->mmc_PMuxInfo);
        else
            return Hal_CARD_GetCdzState(gp_mmc_priv[u8Slot]->mmc_PMuxInfo);
    }

    return (FALSE);
}

unsigned char MS_SD_SET_IOS(struct sstar_mmc_priv *p_mmc_priv, struct msSt_SD_IOS *pstSD_IOS, unsigned char bRefresh)
{
    U8_T bRet   = 0;
    U8_T u8Slot = p_mmc_priv->u8_slotNo;

    /****** Bus Timing Setting*******/
    if ((_stSDInfo[u8Slot].currTiming != pstSD_IOS->timing) || bRefresh)
    {
        _stSDInfo[u8Slot].currTiming = pstSD_IOS->timing;
        sstar_sdmmc_setbustiming(p_mmc_priv, (BusTimingEmType)_stSDInfo[u8Slot].currTiming);
    }

    /****** Clock Setting*******/
    if ((_stSDInfo[u8Slot].currClk != pstSD_IOS->clock) || bRefresh)
    {
        _stSDInfo[u8Slot].currClk     = pstSD_IOS->clock;
        _stSDInfo[u8Slot].currRealClk = sstar_sdmmc_setclock(p_mmc_priv, _stSDInfo[u8Slot].currClk);

        if ((_stSDInfo[u8Slot].currRealClk == 0) && (_stSDInfo[u8Slot].currClk != 0))
        {
            // prtstring("Set IOS => Clk=Error\n\r\n");
        }
        else if (_stSDInfo[u8Slot].currRealClk <= 400000)
        {
            sstar_sdmmc_setbustiming(p_mmc_priv, MMC_TIMING_LEGACY);
        }
        else
        {
            /*
            prtstring("Set IOS => Clk=");
            prtU32Hex(_stSDInfo[u8Slot].currClk);
            prtstring(" (Real=");
            prtU32Hex(_stSDInfo[u8Slot].currRealClk);
            prtstring(")\r\n");
            */
        }
    }

    /****** Power Switch Setting *******/
    if ((_stSDInfo[u8Slot].currPowrMode != pstSD_IOS->power_mode) || bRefresh)
    {
        _stSDInfo[u8Slot].currPowrMode = pstSD_IOS->power_mode;
        sstar_sdmmc_setpower(p_mmc_priv, _stSDInfo[u8Slot].currPowrMode);
    }

    /****** Bus Width Setting*******/
    if ((_stSDInfo[u8Slot].currWidth != pstSD_IOS->bus_width) || bRefresh)
    {
        _stSDInfo[u8Slot].currWidth = pstSD_IOS->bus_width;
        sstar_sdmmc_setbuswidth(p_mmc_priv, (SDMMCBusWidthEmType)_stSDInfo[u8Slot].currWidth);
    }

#if 0 // Always 3.3V
    /****** Pad Voltage Setting*******/
    if( (_stSDInfo[u8Slot].currPadVolt != pstSD_IOS->pad_volt) || bRefresh)
    {
        _stSDInfo[u8Slot].currPadVolt = pstSD_IOS->pad_volt;
        bRet = _SD_SetBusVdd(u8Slot,  (BusVddEmType)_stSDInfo[u8Slot].currPadVolt);
    }
#endif

    return bRet;
}

BOOL_T MS_SD_Check_SDSlot(U8_T u8Slot)
{
    if (gp_mmc_priv[u8Slot] == NULL)
        return FALSE;
    else
        return TRUE;
}

#ifdef CAM_OS_RTK

static unsigned short _SDMMCGetSysDescId(U8_T u8Slot)
{
    switch (u8Slot)
    {
#ifdef SYSDESC_DEV_sdmmc0
        case 0:
            return SYSDESC_DEV_sdmmc0;
#endif
#ifdef SYSDESC_DEV_sdmmc1
        case 1:
            return SYSDESC_DEV_sdmmc1;
#endif
#ifdef SYSDESC_DEV_sdmmc2
        case 2:
            return SYSDESC_DEV_sdmmc2;
#endif
#ifdef SYSDESC_DEV_sdmmc3
        case 3:
            return SYSDESC_DEV_sdmmc3;
#endif
        default:
            return 0xffff;
    }
}

int ms_sdmmc_rtk_dts_init(struct sstar_mmc_priv *p_mmc_priv)
{
    U32_T err;
    U8_T  status = 0, ipidx = 0;
    U8_T  u8Slot = p_mmc_priv->u8_slotNo;

#if 0 // #ifdef CONFIG_CAM_CLK
    U32_T SdmmcClk = 0;
#endif

    if (E_SYS_DESC_PASS != MDrv_SysDesc_Read_U8(_SDMMCGetSysDescId(u8Slot), SYSDESC_PRO_status_u8, &status) || !status)
    {
        pr_sd_err("[SDMMC]-%s is not enabled\n", __func__);
        return 1;
    }

    err = MDrv_SysDesc_Read_U8(_SDMMCGetSysDescId(u8Slot), SYSDESC_PRO_ip_orders_u8,
                               &p_mmc_priv->mmc_PMuxInfo.u8_ipOrder);
    if (err)
    {
        pr_sd_err("[SDMMC]-%s : get dts option [ip_orders_u8] fail %X ...\n", __func__, err);
        return 1;
    }

#if (PADMUX_SET == PADMUX_SET_BY_FUNC)
    if (Hal_CARD_PadmuxGetting(&p_mmc_priv->mmc_PMuxInfo))
    {
        pr_sd_err(">> [SDMMC] Err: Could not get SD pad group from Padmux dts!\n");
        return 1;
    }
#else
    err = MDrv_SysDesc_Read_U32(_SDMMCGetSysDescId(u8Slot), SYSDESC_PRO_cdzs_gpios_u32,
                                &p_mmc_priv->mmc_PMuxInfo.u32_PinCdzRst);
    if (err)
    {
        pr_sd_err("[SDMMC]-%s : get dts option [cdzs_gpios_u32] fail %X ...\n", __func__, err);
        return 1;
    }

    err = MDrv_SysDesc_Read_U32(_SDMMCGetSysDescId(u8Slot), SYSDESC_PRO_pwr_gpios_u32,
                                &p_mmc_priv->mmc_PMuxInfo.u32_PinPWR);
    if (err)
    {
        pr_sd_err("[SDMMC]-%s : get dts option [pwr_gpios_u32] fail %X ...\n", __func__, err);
        return 1;
    }

    err = MDrv_SysDesc_Read_U8(_SDMMCGetSysDescId(u8Slot), SYSDESC_PRO_bus_width_u8,
                               &p_mmc_priv->mmc_PMuxInfo.u8_busWidth);
    if (err)
    {
        pr_sd_err("[SDMMC]-%s : get dts option [bus_width_u8] fail %X ...\n", __func__, err);
        return 1;
    }

#endif
    err = MDrv_SysDesc_Read_U32_Array(_SDMMCGetSysDescId(u8Slot), SYSDESC_PRO_regs_u32_u16, p_mmc_priv->pIPBANKArr, 1);
    if (err)
    {
        pr_sd_err("[SDMMC]-%s : get dts option [regs_u32_u16] fail %X ...\n", __func__, err);
        return 1;
    }

    err = MDrv_SysDesc_Read_U8(_SDMMCGetSysDescId(u8Slot), SYSDESC_PRO_revcdz_u8, &p_mmc_priv->u8_revCdz);
    if (err)
    {
        pr_sd_err("[SDMMC]-%s : get dts option [revcdz_u8] fail %X ...\n", __func__, err);
        return 1;
    }

    err = MDrv_SysDesc_Read_U8(_SDMMCGetSysDescId(u8Slot), SYSDESC_PRO_pad_orders_u8,
                               &p_mmc_priv->mmc_PMuxInfo.u8_padOrder);
    if (err)
    {
        pr_sd_err("[SDMMC]-%s : get dts option [pad_orders_u8] fail %X ...\n", __func__, err);
        return 1;
    }

    err = MDrv_SysDesc_Read_U32(_SDMMCGetSysDescId(u8Slot), SYSDESC_PRO_max_clks_u32, &p_mmc_priv->u32_maxClk);
    if (err)
    {
        pr_sd_err("[SDMMC]-%s : get dts option [max_clks_u32] fail %X ...\n", __func__, err);
        return 1;
    }

    err = MDrv_SysDesc_Read_U8(_SDMMCGetSysDescId(u8Slot), SYSDESC_PRO_intcdzs_u8, &p_mmc_priv->u8_intCdz);
    if (err)
    {
        pr_sd_err("[SDMMC]-%s : get dts option [intcdzs_u8] fail %X ...\n", __func__, err);
        return 1;
    }

    err = MDrv_SysDesc_Read_U8(_SDMMCGetSysDescId(u8Slot), SYSDESC_PRO_fakecdzs_u8, &p_mmc_priv->u8_fakeCdz);
    if (err)
    {
        pr_sd_err("[SDMMC]-%s : get dts option [fakecdzs_u8] fail %X ...\n", __func__, err);
        return 1;
    }

    err =
        MDrv_SysDesc_Read_U32(_SDMMCGetSysDescId(u8Slot), SYSDESC_PRO_pwr_off_delay_u32, &p_mmc_priv->u32_pwerOffDelay);
    if (err)
    {
        pr_sd_err("[SDMMC]-%s : get dts option [pwr_off_delay_u32] fail %X ...\n", __func__, err);
        return 1;
    }

    err = MDrv_SysDesc_Read_U32(_SDMMCGetSysDescId(u8Slot), SYSDESC_PRO_pwr_on_delay_u32, &p_mmc_priv->u32_pwerOnDelay);
    if (err)
    {
        pr_sd_err("[SDMMC]-%s : get dts option [pwr_on_delay_u32] fail %X ...\n", __func__, err);
        return 1;
    }

    err = MDrv_SysDesc_Read_U8(_SDMMCGetSysDescId(u8Slot), SYSDESC_PRO_sdio_use_u8, &p_mmc_priv->u8_sdioUse);
    if (err)
    {
        pr_sd_err("[SDMMC]-%s : get dts option [sdio_use_u8] fail %X ...\n", __func__, err);
        return 1;
    }

    err = MDrv_SysDesc_Read_U8(_SDMMCGetSysDescId(u8Slot), SYSDESC_PRO_removable_u8, &p_mmc_priv->u8_remmovable);
    if (err)
    {
        pr_sd_err("[SDMMC]-%s : get dts option [removable_u8] fail %X ...\n", __func__, err);
        return 1;
    }

#ifdef SYSDESC_PRO_slot_Sdio_Use_1bit_u8
    err = MDrv_SysDesc_Read_U8_Array(_SDMMCGetSysDescId(u8Slot), SYSDESC_PRO_Sdio_Use_1bit_u8,
                                     &p_mmc_priv->u8_sdioUse1Bit);
    if (err)
    {
        pr_sd_err("[SDMMC]-%s : get dts option [Sdio_Use_1bit_u8] fail %X ...\n", __func__, err);
    }
#endif

    err = MDrv_SysDesc_Read_U32(_SDMMCGetSysDescId(u8Slot), SYSDESC_PRO_mie_intr_u32, &p_mmc_priv->u32_mieIrqNo);
    if (err)
    {
        pr_sd_err("[SDMMC]-%s : get dts option [mie_intr_u32] fail %X ...\n", __func__, err);
        return 1;
    }

    err = MDrv_SysDesc_Read_U32(_SDMMCGetSysDescId(u8Slot), SYSDESC_PRO_cdz_intr_u32, &p_mmc_priv->u32_cdzIrqNo);
    if (err)
    {
        pr_sd_err("[SDMMC]-%s : get dts option [cdz_intr_u32] fail %X ...\n", __func__, err);
        return 1;
    }

    //
    ipidx = p_mmc_priv->mmc_PMuxInfo.u8_ipOrder;

    // Check conflict
    if (p_mmc_priv->u8_sdioUse)
    {
        if (geIpTypeIp[ipidx] != IP_TYPE_SDIO)
        {
            // pr_err(">> [sdmmc] slotNo = %u, When SDIO is used, IpType need to be IP_TYPE_SDIO, current setting =
            // %u\n", slotNo, (U8_T)geIpTypeIp[u8_IPOrderSlot[eSlot]]);
            return 1;
        }
    }

#if 1
    // Debug
    pr_sd_dbg(">> [sdmmc] SlotNo = %u\n", p_mmc_priv->u8_slotNo);
    pr_sd_dbg(">> [sdmmc] RevCDZ = %u\n", p_mmc_priv->u8_revCdz);
    pr_sd_dbg(">> [sdmmc] IP     = %u\n", p_mmc_priv->mmc_PMuxInfo.u8_ipOrder);
    pr_sd_dbg(">> [sdmmc] PAD    = %u\n", p_mmc_priv->mmc_PMuxInfo.u8_padOrder);
    pr_sd_dbg(">> [sdmmc] IP_BANK= %x\n", p_mmc_priv->pIPBANKArr[0]);
    pr_sd_dbg(">> [sdmmc] MaxClk = %u\n", p_mmc_priv->u32_maxClk);
    pr_sd_dbg(">> [sdmmc] IntCDZ = %u\n", p_mmc_priv->u8_intCdz);
    pr_sd_dbg(">> [sdmmc] FakeCDZ = %u\n", p_mmc_priv->u8_fakeCdz);
    pr_sd_dbg(">> [sdmmc] CdzNo = %u\n", p_mmc_priv->mmc_PMuxInfo.u32_PinCdzRst);
    pr_sd_dbg(">> [sdmmc] PwrNo = %u\n", p_mmc_priv->mmc_PMuxInfo.u32_PinPWR);
    pr_sd_dbg(">> [sdmmc] PwrOffDelay = %u\n", p_mmc_priv->u32_pwerOffDelay);
    pr_sd_dbg(">> [sdmmc] PwrOnDelay  = %u\n", p_mmc_priv->u32_pwerOnDelay);
    pr_sd_dbg(">> [sdmmc] SdioUse   = %u\n", p_mmc_priv->u8_sdioUse);
    pr_sd_dbg(">> [sdmmc] Removable = %u\n", p_mmc_priv->u8_remmovable);
    pr_sd_dbg(">> [sdmmc] BusWidth  = %u\n", p_mmc_priv->mmc_PMuxInfo.u8_busWidth);
    pr_sd_dbg(">> [sdmmc] MieIntNo  = %u\n", p_mmc_priv->u32_mieIrqNo);
    pr_sd_dbg(">> [sdmmc] CdzIntNo  = %u\n", p_mmc_priv->u32_cdzIrqNo);
#endif
#if 0 // #ifdef CONFIG_CAM_CLK
    //
    for(slotNo =0; slotNo<gu8_SlotNums; slotNo++)
    {
        eSlot = (SlotEmType)slotNo;
        ipidx = (U8_T)ge_IPOrderSlot[eSlot];

        /* Get Clock info from DTS */
        SdmmcClk = 0;
        of_property_read_u32_index(p_dev->dev.of_node,"camclk", ipidx, &(SdmmcClk));

        if (!SdmmcClk)
        {
            //printk(KERN_DEBUG "[%s] Fail to get clk!\n", __func__);
            //pr_err(">> [sdmmc_%u] Err: Failed to get dts clock tree!\n", slotNo);
            return 1;
        }

        CamClkRegister("Sdmmc",SdmmcClk,&(gp_clkSlot[slotNo]));
        CamClkSetOnOff(gp_clkSlot[slotNo], 1);
    }
#endif

    return 0;
}

#endif

static void _SD_InitInfo(U8_T u8Slot)
{
    _stSDInfo[u8Slot].currClk     = 0;
    _stSDInfo[u8Slot].currRealClk = 0;

    _stSDInfo[u8Slot].currPowrMode = MMC_POWER_OFF;
    _stSDInfo[u8Slot].currWidth    = MMC_BUS_WIDTH_1;
    _stSDInfo[u8Slot].currTiming   = MMC_TIMING_LEGACY;
    _stSDInfo[u8Slot].currPadVolt  = SD_PAD_VOLT_330;
    _stSDInfo[u8Slot].read_only    = (FALSE);
    _stSDInfo[u8Slot].card_det     = (FALSE);
}

static int ms_sdmmc_init_slot(struct sstar_mmc_priv *p_mmc_priv)
{
    SlotEmType eSlot = (SlotEmType)p_mmc_priv->u8_slotNo;
    IpOrder    eIP   = p_mmc_priv->mmc_PMuxInfo.u8_ipOrder;
    int        nRet  = 0;

    /****** (2) SDMMC host setting ******/

    // p_sdmmc_slot->slotNo       = slotNo;

    // p_sdmmc_slot->pwrGPIONo    = gu32_PwrNoSlot[eSlot];

    // p_mmc_host->f_max = gu32_MaxClkSlot[eSlot];

    // p_mmc_host->ocr_avail =
    // MMC_VDD_32_33|MMC_VDD_31_32|MMC_VDD_30_31|MMC_VDD_29_30|MMC_VDD_28_29|MMC_VDD_27_28|MMC_VDD_165_195;
    // p_mmc_host->caps = MMC_CAP_4_BIT_DATA|MMC_CAP_MMC_HIGHSPEED|MMC_CAP_SD_HIGHSPEED;

    // p_mmc_host->max_blk_count  = MAX_BLK_COUNT;
    // p_mmc_host->max_blk_size   = MAX_BLK_SIZE;

    // p_mmc_host->max_req_size   = p_mmc_host->max_blk_count  * p_mmc_host->max_blk_size;
    // p_mmc_host->max_seg_size   = p_mmc_host->max_req_size;

    // p_mmc_host->max_segs       = MAX_SEG_CNT;

    _SD_InitInfo(eSlot);

    /****** (5) Init GPIO Setting ******/
    sstar_sdmmc_enable(p_mmc_priv);

    /****** (6) Interrupt Source Setting ******/
    gst_IntSourceSlot[eSlot].slotNo = eSlot;
    gst_IntSourceSlot[eSlot].eIP    = eIP;
    gst_IntSourceSlot[eSlot].p_data = NULL;

    /*****  (7) Spinlock Init for Reg Protection ******/
    CamOsMutexInit(&sdmmc_mutex[eSlot]);
    /****** (8) Register IP IRQ *******/

#if defined(CONFIG_SSTAR_SDMMC_INTERRUPT_MODE)
    Hal_SDMMC_MIEIntCtrl(eIP, FALSE);

#if 1 // MIE

    nRet |= SdmmcIrqRequest(&gst_IntSourceSlot[eSlot], p_mmc_priv->u32_mieIrqNo);
    if (nRet)
    {
        CamOsPrintf("ERROR SdmmcIrq init\r\n");
        goto INIT_FAIL_2;
    }

    Hal_SDMMC_MIEIntCtrl(eIP, TRUE);

    if (p_mmc_priv->u8_sdioUse) // When SDIO Card is used, we enable SDIO int.
    {
        // p_mmc_host->caps |= MMC_CAP_SDIO_IRQ;
        Hal_CARD_INT_SetMIEIntEn_ForSDIO(eIP, TRUE);
        Hal_SDMMC_SDIODeviceCtrl(eIP, TRUE);
#if 0 // ***enable by sdio API***
        Hal_SDMMC_SDIOIntDetCtrl(eIP, TRUE);
#endif
        // p_sdmmc_slot->sdioFlag = 1;

        pr_sd_dbg(">> [sdmmc_%u] Enable SDIO Interrupt Mode! \n", eSlot);
    }
    else
    {
        // p_mmc_host->caps2 = MMC_CAP2_NO_SDIO;
        Hal_SDMMC_SDIODeviceCtrl(eIP, FALSE);
        // p_sdmmc_slot->sdioFlag = 0;
    }
#endif

#else

    if (p_mmc_priv->u8_sdioUse) // When SDIO Card is used, we enable SDIO int.
    {
        Hal_SDMMC_SDIODeviceCtrl(eIP, TRUE);
    }
    else
    {
        Hal_SDMMC_SDIODeviceCtrl(eIP, FALSE);
    }

#endif

    if (p_mmc_priv->u8_intCdz)
    {
        CamOsWorkQueueCreate(&cdzWorkQueue[eSlot], "cdzWorkQueue", 5);
        CamOsWorkQueueCreate(&hotplugWorkQueue[eSlot], "cdzhotplug", 5);

        nRet |= SdmmcCdzIrqRequest(&gst_IntSourceSlot[eSlot], p_mmc_priv->u32_cdzIrqNo);
        if (nRet)
        {
            CamOsPrintf("ERROR SdmmcIrq init\r\n");
            goto INIT_FAIL_3;
        }
    }

    //
    // mmc_add_host(p_mmc_host);

    // Return Success
    return 0;

// INIT_FAIL_1:
INIT_FAIL_3:
    SdmmcCdzIrqFree(&gst_IntSourceSlot[eSlot], p_mmc_priv->u32_cdzIrqNo);
    CamOsWorkQueueDestroy(cdzWorkQueue[eIP]);
    CamOsWorkQueueDestroy(hotplugWorkQueue[eIP]);

#if defined(CONFIG_SSTAR_SDMMC_INTERRUPT_MODE)
INIT_FAIL_2:
    SdmmcIrqFree(&gst_IntSourceSlot[eSlot], p_mmc_priv->u32_mieIrqNo);
#endif

    return nRet;
}

static int ms_sdmmc_probe(struct sstar_mmc_priv **p_mmc_priv, U8_T u8Slot)
{
    int ret = 0;

    p_mmc_priv[u8Slot] = (struct sstar_mmc_priv *)CamOsMemAlloc(sizeof(struct sstar_mmc_priv));

    if (!p_mmc_priv[u8Slot])
    {
        pr_sd_err(">> [mmc] Err: Failed to Allocate p_mmc_priv!\n\n");
        return -1;
    }

    p_mmc_priv[u8Slot]->u8_slotNo = u8Slot;

    if (ms_sdmmc_rtk_dts_init(p_mmc_priv[u8Slot]))
    {
        goto err;
    }

    ret = ms_sdmmc_init_slot(p_mmc_priv[u8Slot]);
    if (ret != 0)
    {
        goto err;
    }

    return 0;

err:
    CamOsMemRelease(p_mmc_priv[u8Slot]);
    p_mmc_priv[u8Slot] = NULL;
    return ret ? ret : 1;
}

static int ms_sdmmc_remove(struct sstar_mmc_priv *p_mmc_priv)
{
    SlotEmType eSlot = (SlotEmType)p_mmc_priv->u8_slotNo;
    IpOrder    eIP   = p_mmc_priv->mmc_PMuxInfo.u8_ipOrder;

    CamOsMutexDestroy(&sdmmc_mutex[eSlot]);

    if (p_mmc_priv->u8_intCdz)
    {
        SdmmcCdzIrqFree(&gst_IntSourceSlot[eSlot], p_mmc_priv->u32_cdzIrqNo);

        CamOsWorkQueueDestroy(cdzWorkQueue[eIP]);
        CamOsWorkQueueDestroy(hotplugWorkQueue[eIP]);
    }

#if (EN_BIND_CARD_INT)
    SdmmcIrqFree(&gst_IntSourceSlot[eSlot], p_mmc_priv->u32_mieIrqNo);
#endif

    return 0;
}

void sd_card_remove(struct sstar_mmc_priv *p_mmc_priv)
{
    struct msSt_SD_IOS SETSDIOS = {0, 0, 0, 0, 0, 0}; //{NULL};
    if (!p_mmc_priv)
    {
        CamOsPrintf("Sdmmc ERR: invalid slot number: %d !\n", p_mmc_priv->u8_slotNo);
        return;
    }

    SETSDIOS.clock      = 400000;
    SETSDIOS.power_mode = MMC_POWER_OFF;
    SETSDIOS.bus_width  = MMC_BUS_WIDTH_1;
    SETSDIOS.timing     = MMC_TIMING_LEGACY;
    SETSDIOS.pad_volt   = SD_PAD_VOLT_330;
    MS_SD_SET_IOS(p_mmc_priv, &SETSDIOS, TRUE);

    gb_SDCardInitFlag[p_mmc_priv->u8_slotNo] = FALSE;
}

// sd driver init
static int sd_init(void)
{
    int  ret;
    U8_T u8Slot;

    if (gb_SDReady)
        return 0;
    gb_SDReady = TRUE;

    for (u8Slot = 0; u8Slot < SDMMC_NUM_TOTAL; u8Slot++)
    {
        ret = CamOsMutexInit(&sdmmc_init_mutex[u8Slot]);
        if (ret != CAM_OS_OK)
        {
            CamOsPrintf("Unable to init sock mutex %s,%d\n", __FUNCTION__, __LINE__);
        }

        gp_mmc_priv[u8Slot] = NULL;
        if (_SDMMCGetSysDescId(u8Slot) != 0xffff)
            ret = ms_sdmmc_probe(gp_mmc_priv, u8Slot);
    }

    return ret;
}

// sd driver deinit
int sd_deinit(void)
{
    int  ret;
    U8_T u8Slot;

    if (!gb_SDReady)
        return 0;
    gb_SDReady = FALSE;

    for (u8Slot = 0; u8Slot < SDMMC_NUM_TOTAL; u8Slot++)
    {
        if (gp_mmc_priv[u8Slot] == NULL)
            continue;

        sd_card_remove(gp_mmc_priv[u8Slot]);
        ret = CamOsMutexDestroy(&sdmmc_init_mutex[u8Slot]);
        if (ret != CAM_OS_OK)
        {
            CamOsPrintf("Unable to init sock mutex %s,%d\n", __FUNCTION__, __LINE__);
        }
        ret = ms_sdmmc_remove(gp_mmc_priv[u8Slot]);
        if (ret != 0)
        {
            CamOsPrintf("Unable to remove sdmmc_%u %s,%d\n", u8Slot, __FUNCTION__, __LINE__);
            return ret;
        }
        CamOsMemRelease(gp_mmc_priv[u8Slot]);
        gp_mmc_priv[u8Slot] = NULL;
    }
    return 0;
}

void sdmmc_init(void)
{
    sd_init();
}
rtos_device_initcall(sdmmc_init);
