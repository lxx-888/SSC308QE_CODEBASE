/*
 * drv_sdmmc_lnx.c- Sigmastar
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
 * FileName drv_sdmmc_lnx.c
 *     @author jeremy.wang (2012/01/10)
 * Desc:
 *     This layer between Linux SD Driver layer and IP Hal layer.
 *     (1) The goal is we don't need to change any Linux SD Driver code, but we can handle here.
 *     (2) You could define Function/Ver option for using, but don't add Project option here.
 *     (3) You could use function option by Project option, but please add to ms_sdmmc.h
 *
 ***************************************************************************************************************/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/blkdev.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/version.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/clk-provider.h>
#include <asm/io.h>
#include "cam_os_wrapper.h"

#include "card.h"
#include "core.h"

#include "drv_sdmmc_lnx.h"
#include "drv_sdmmc_common.h"
#include "hal_sdmmc_v5.h"
#include "hal_sdmmc_intr.h"
#include "hal_sdmmc_timer.h"
#include "hal_sdmmc_platform.h"
#include "hal_sdmmc_platform_pri_config.h"

#include "drv_padmux.h"
#include "drv_gpio.h"
#include "padmux.h"

#ifdef CONFIG_CAM_CLK
#include "drv_camclk_Api.h"
#endif

//###########################################################################################################
#if (EN_MSYS_REQ_DMEM)
//###########################################################################################################
#include "../include/ms_msys.h"
//###########################################################################################################
#endif

#if defined(CONFIG_OF)
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/clk.h>
#endif

#define FORCE_HAL_CLK (1) // Set 1 to use HAL driver rather than DTB. Turn this on for debugging.
//***********************************************************************************************************
// Config Setting (Internal)
//***********************************************************************************************************
#define EN_SDMMC_TRFUNC       (FALSE)
#define EN_SDMMC_TRSDIO       (FALSE)
#define EN_SDMMC_BRO_DMA      (TRUE)
#define EN_SDMMC_DCACHE_FLUSH (TRUE)

/****** For Allocation buffer *******/
#define MAX_BLK_SIZE  512  // Maximum Transfer Block Size
#define MAX_BLK_COUNT 1024 // Maximum Transfer Block Count
#define MAX_SEG_CNT   128

/****** For broken DMA *******/
#define MAX_BRO_BLK_COUNT 1024 // Maximum Broken DMA Transfer Block Count

/****** For SD Debounce Setting *******/
#define WT_DB_PLUG      30  // Waiting time for Insert Debounce
#define WT_DB_UNPLUG    30  // Waiting time for Unplug Debounce
#define WT_DB_SW_PLUG   300 // Waiting time for Plug Delay Process
#define WT_DB_SW_UNPLUG 0   // Waiting time for Uplug Delay Process

//***********************************************************************************************************
BOOL_T       gb_Sdio_Dis_Intr_By_IP[SDMMC_NUM_TOTAL] = {FALSE, FALSE, FALSE};
extern U32_T gu32_SdmmcClk[SDMMC_NUM_TOTAL];
extern U32_T gu32_SdmmcStatus[SDMMC_NUM_TOTAL];
extern S32_T gu32_SdmmcCurCMD[SDMMC_NUM_TOTAL];

#ifdef CONFIG_PM_SLEEP
static U16_T gu16_SlotIPClk[SDMMC_NUM_TOTAL]    = {0};
static U16_T gu16_SlotBlockClk[SDMMC_NUM_TOTAL] = {0};
#endif

static const char gu8_mie_irq_name[SDMMC_NUM_TOTAL][20] = {"mie0_irq", "mie1_irq", "mie2_irq"};
static const char gu8_irq_name[SDMMC_NUM_TOTAL][20]     = {"cdz_slot0_irq", "cdz_slot1_irq", "cdz_slot2_irq"};

//
static IntSourceStruct gst_IntSourceSlot[SDMMC_NUM_TOTAL];
static spinlock_t      g_RegLockSlot[SDMMC_NUM_TOTAL];

volatile IpType          geIpTypeIp[SDMMC_NUM_TOTAL] = {IP_0_TYPE, IP_1_TYPE, IP_2_TYPE};
extern volatile CardType geCardType[SDMMC_NUM_TOTAL];

#if defined(CONFIG_OF)

#ifdef CONFIG_CAM_CLK
void *gp_clkSlot[SDMMC_NUM_TOTAL] = {NULL};
#else
struct clk *      gp_clkSlot[SDMMC_NUM_TOTAL];
struct clk *      gp_clksyncSlot[SDMMC_NUM_TOTAL];
struct clk *      gp_clkpmmcuSlot[SDMMC_NUM_TOTAL];
static const char gu8_sd_clk_name[SDMMC_NUM_TOTAL][20]   = {"clk_sdmmc0", "clk_sdmmc1", "clk_sdmmc2"};
static const char gu8_sync_clk_name[SDMMC_NUM_TOTAL][20] = {"syn_clk_sdmmc0", "syn_clk_sdmmc1", "syn_clk_sdmmc2"};
static const char gu8_pm_mcu_name[SDMMC_NUM_TOTAL][20]   = {"pm_mcu_sdmmc0", "pm_mcu_sdmmc1", "pm_mcu_sdmmc2"};

#endif

#endif
#if defined(CONFIG_SUPPORT_SD30) || defined(CONFIG_SUPPORT_EMMC50)
struct clk_hw *CLK_IP_1x_p[SDMMC_NUM_TOTAL];
struct clk_hw *CLK_IP_2x_p[SDMMC_NUM_TOTAL];
#endif
static U16_T gu16_SlotClkEn[SDMMC_NUM_TOTAL] = {0};

U8_T gu8_SlotTuningSts[SDMMC_NUM_TOTAL] = {0};

#ifdef CONFIG_SUPPORT_SDMMC_COMMAND
struct list_head sdmmc_command_list;
LIST_HEAD(sdmmc_command_list);
#endif

#ifdef CONFIG_SUPPORT_SDMMC_UT_VERIFY
extern struct attribute_group sstar_sdmmc_ut_attr_grp;
#endif
extern struct attribute_group sstar_sdmmc_debug_attr_grp;

#ifdef CONFIG_SUPPORT_SDMMC_AT_SMOKE
static char *       sdSpeedMode   = "SD20";
static char *       emmcSpeedMode = "HS";
static char *       transMode     = "adma";
static unsigned int busWidth      = 4;
#endif

// Global Variable for All Slot:
//-----------------------------------------------------------------------------------------------------------
extern CamOsMutex_t sdmmc_mutex[SDMMC_NUM_TOTAL];

// String Name
//-----------------------------------------------------------------------------------------------------------
#define DRIVER_NAME "sstar_sdmmc"
#define DRIVER_DESC "Sstar SD/MMC Card Interface driver"

// Trace Funcion
//-----------------------------------------------------------------------------------------------------------
#if (EN_SDMMC_TRFUNC)
#define pr_sd_err(fmt, arg...)  printk(fmt, ##arg)
#define pr_sd_main(fmt, arg...) printk(fmt, ##arg)
#define pr_sd_dbg(fmt, arg...)  printk(fmt, ##arg)
#else
#define pr_sd_err(fmt, arg...)  printk(fmt, ##arg)
#define pr_sd_main(fmt, arg...) // printk(fmt, ##arg)
#define pr_sd_dbg(fmt, arg...)  // printk(fmt, ##arg)
#endif

#if (EN_SDMMC_TRSDIO)
#define pr_sdio_main(fmt, arg...) printk(fmt, ##arg)
#else
#define pr_sdio_main(fmt, arg...)
#endif

void Hal_CARD_SetGPIOIntAttr(GPIOOptEmType eGPIOOPT, unsigned int irq)
{
#if (D_OS == D_OS__LINUX)
    if (eGPIOOPT == EV_GPIO_OPT1) // clear interrupt
    {
        struct irq_data *sd_irqdata = irq_get_irq_data(irq);
        struct irq_chip *chip       = irq_get_chip(irq);

        chip->irq_ack(sd_irqdata);
    }
    else if (eGPIOOPT == EV_GPIO_OPT2)
    {
    }
    else if (eGPIOOPT == EV_GPIO_OPT3) // sd polarity _HI Trig for remove
    {
        irq_set_irq_type(irq, IRQ_TYPE_EDGE_RISING);
    }
    else if (eGPIOOPT == EV_GPIO_OPT4) // sd polarity _LO Trig for insert
    {
        irq_set_irq_type(irq, IRQ_TYPE_EDGE_FALLING);
    }
    else if (eGPIOOPT == EV_GPIO_OPT5) // Set IRQ_TYPE_NONE.
    {
        irq_set_irq_type(irq, IRQ_TYPE_NONE);
    }
#endif
}

//------------------------------------------------------------------------------------------------
#if defined(CONFIG_SUPPORT_SD30)
// Set Bus Voltage
//------------------------------------------------------------------------------------------------
static unsigned char _SetBusVdd(struct sstar_mmc_priv *p_mmc_priv, U8_T u8Vdd)
{
    IpOrder       eIP  = p_mmc_priv->mmc_PMuxInfo.u8_ipOrder;
    unsigned char bRet = 0;

    if (u8Vdd == MMC_SIGNAL_VOLTAGE_180)
    {
        Hal_SDMMC_ClkCtrl(eIP, FALSE, 5);
        bRet = Hal_CARD_SetPADVdd(p_mmc_priv->mmc_PMuxInfo, EV_LOWVOL, 10);
        Hal_SDMMC_ClkCtrl(eIP, TRUE, 5);
    }
    else
    {
        /****** Simple Setting Here ******/
        bRet = Hal_CARD_SetPADVdd(p_mmc_priv->mmc_PMuxInfo, EV_NORVOL, 0);
    }

    return bRet;
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: sstar_sdmmc_switch_busvdd
 *     @author jeremy.wang (2018/1/8)
 * Desc:
 *
 * @param p_mmc_host :
 * @param p_mmc_ios :
 *
 * @return int  :
 ----------------------------------------------------------------------------------------------------------*/
static int sstar_sdmmc_switch_busvdd(struct mmc_host *p_mmc_host, struct mmc_ios *p_mmc_ios)
{
    struct sstar_sdmmc_slot *p_sdmmc_slot = mmc_priv(p_mmc_host);
    SlotEmType               eSlot        = (SlotEmType)p_sdmmc_slot->slotNo;

    if (p_sdmmc_slot->p_mmc_priv->u8_supportSD30)
    {
        pr_sd_main(">> [sdmmc_%u] Switch BusVdd (%u)\n", eSlot, p_mmc_ios->signal_voltage);

        if (_SetBusVdd(p_sdmmc_slot->p_mmc_priv, p_mmc_ios->signal_voltage))
        {
            pr_err(">> [sdmmc_%u] Err: Single Volt (%u) doesn't ready!\n", eSlot, p_mmc_ios->signal_voltage);
            return 1;
        }
    }

    return 0;
}
#endif

#ifdef CONFIG_SUPPORT_SDMMC_AT_SMOKE
static U8_T sstar_sdmmc_select_transmode(char *ptransMode)
{
    /* 0:dma 1:adma */
    return (strcmp((const char *)ptransMode, "adma") == 0) ? 1 : 0;
}

static void sstar_sdmmc_modify_speedmode_byparam(char *pspeedMode, unsigned int uibusWidth, char *ptransMode,
                                                 struct mmc_host *p_mmc_host)
{
    struct sstar_sdmmc_slot *p_sdmmc_slot = mmc_priv(p_mmc_host);

    if (!strcmp((const char *)pspeedMode, "SD20"))
    {
        p_mmc_host->caps |= (MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED);
        p_mmc_host->caps &= ~(MMC_CAP_UHS_SDR50 | MMC_CAP_UHS_SDR104 | MMC_CAP_UHS_DDR50);
        if (uibusWidth > 4 || uibusWidth < 1) // bus-width could be 1, 4
            uibusWidth = 4;
    }
    else if (!strcmp((const char *)pspeedMode, "SD30"))
    {
#if defined(CONFIG_SUPPORT_SD30)
        p_mmc_host->caps |= (MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED);
        p_mmc_host->caps |= (MMC_CAP_UHS_SDR50 | MMC_CAP_UHS_SDR104 | MMC_CAP_UHS_DDR50);
#endif
        if (uibusWidth != 4) // bus-width only 4
            uibusWidth = 4;
    }
    else if (!strcmp((const char *)pspeedMode, "HS"))
    {
        p_mmc_host->caps |= MMC_CAP_MMC_HIGHSPEED;
        p_mmc_host->caps2 &= ~(MMC_CAP2_HS200_1_8V_SDR | MMC_CAP2_HS400_1_8V);
        if (uibusWidth > 8) // bus-width could be 1, 4, 8
            uibusWidth = 8;
    }
    else if (!strcmp((const char *)pspeedMode, "HS200"))
    {
#if defined(CONFIG_SUPPORT_EMMC50)
        p_mmc_host->caps |= MMC_CAP_MMC_HIGHSPEED;
        p_mmc_host->caps2 |= MMC_CAP2_HS200_1_8V_SDR;
        p_mmc_host->caps2 &= ~MMC_CAP2_HS400_1_8V;
#endif
        if (uibusWidth > 8 || uibusWidth < 4) // bus-width could be 4, 8
            uibusWidth = 8;
    }
    else if (!strcmp((const char *)pspeedMode, "HS400"))
    {
#if defined(CONFIG_SUPPORT_EMMC50)
        p_mmc_host->caps |= MMC_CAP_MMC_HIGHSPEED;
        p_mmc_host->caps2 |= (MMC_CAP2_HS200_1_8V_SDR | MMC_CAP2_HS400_1_8V);
#endif
        if (uibusWidth != 8) // bus-width only 8
            uibusWidth = 8;
    }

    switch (uibusWidth)
    {
        case 8:
            p_mmc_host->caps |= (MMC_CAP_8_BIT_DATA | MMC_CAP_4_BIT_DATA);
            break;
        case 4:
            p_mmc_host->caps &= ~(MMC_CAP_8_BIT_DATA);
            p_mmc_host->caps |= MMC_CAP_4_BIT_DATA;
            break;
        case 1:
            p_mmc_host->caps &= ~(MMC_CAP_8_BIT_DATA | MMC_CAP_4_BIT_DATA);
            break;
        default:
            pr_err("Invalid \"bus-width\" value %u!\n", uibusWidth);
            return;
    }

    p_sdmmc_slot->p_mmc_priv->mmc_PMuxInfo.u8_busWidth = uibusWidth;
    p_sdmmc_slot->p_mmc_priv->u8_transMode             = sstar_sdmmc_select_transmode(ptransMode);
}

static void sstar_sdmmc_select_speedmode(IpOrder eIP, struct mmc_host *p_mmc_host)
{
    switch (geCardType[eIP])
    {
        case CARD_TYPE_SD:
            sstar_sdmmc_modify_speedmode_byparam(sdSpeedMode, busWidth, transMode, p_mmc_host);
            break;
        case CARD_TYPE_EMMC:
            sstar_sdmmc_modify_speedmode_byparam(emmcSpeedMode, busWidth, transMode, p_mmc_host);
            break;
        default:
            break;
    }
}
#endif

//------------------------------------------------------------------------------------------------
BOOL_T _GetCardDetect(struct sstar_mmc_priv *p_mmc_priv)
{
    if (p_mmc_priv->u8_fakeCdz)
    {
        return (TRUE);
    }
    else
    {
        if (p_mmc_priv->u8_revCdz)
            return !Hal_CARD_GetCdzState(p_mmc_priv->mmc_PMuxInfo);
        else
            return Hal_CARD_GetCdzState(p_mmc_priv->mmc_PMuxInfo);
    }

    return (FALSE);
}

static BOOL_T _GetWriteProtect(SlotEmType eSlot)
{
    return FALSE;
}

static BOOL_T _CardDetect_PlugDebounce(struct sstar_mmc_priv *p_mmc_priv, U32_T u32WaitMs, BOOL_T bPrePlugStatus)
{
    BOOL_T bCurrPlugStatus = bPrePlugStatus;
    U32_T  u32DiffTime     = 0;

    while (u32DiffTime < u32WaitMs)
    {
        mdelay(1);
        u32DiffTime++;

        bCurrPlugStatus = _GetCardDetect(p_mmc_priv);

        if (bPrePlugStatus != bCurrPlugStatus)
        {
            /****** Print the Debounce ******/
            /*if(bPrePlugStatus)
                printk("#");
            else
                printk("$");*/
            /*********************************/
            break;
        }
    }
    return bCurrPlugStatus;
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: sstar_sdmmc_cdzint
 *     @author jeremy.wang (2012/5/8)
 * Desc: Int funtion for GPIO Card Detection
 *
 * @param irq :
 * @param p_dev_id :
 *
 * @return irqreturn_t  :
 ----------------------------------------------------------------------------------------------------------*/
static irqreturn_t sstar_sdmmc_cdzint(int irq, void *p_dev_id)
{
    irqreturn_t              irq_t        = IRQ_NONE;
    IntSourceStruct *        pstIntSource = p_dev_id;
    struct sstar_sdmmc_slot *p_sdmmc_slot = pstIntSource->p_data;

    //
    disable_irq_nosync(irq);
    //
    tasklet_schedule(&p_sdmmc_slot->hotplug_tasklet);
    irq_t = IRQ_HANDLED;

    return irq_t;
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: sstar_sdmmc_hotplug
 *     @author jeremy.wang (2012/1/5)
 * Desc: Hotplug function for Card Detection
 *
 * @param data : ms_sdmmc_slot struct pointer
 ----------------------------------------------------------------------------------------------------------*/
static void sstar_sdmmc_hotplug(unsigned long data)
{
    struct sstar_sdmmc_slot *p_sdmmc_slot = (struct sstar_sdmmc_slot *)data;
    IpOrder                  eIP          = (IpOrder)p_sdmmc_slot->ipOrder;
    GPIOOptEmType            eINSOPT      = EV_GPIO_OPT3;
    GPIOOptEmType            eEJTOPT      = EV_GPIO_OPT4;

    if (p_sdmmc_slot->p_mmc_priv->u8_revCdz)
    {
        eINSOPT = EV_GPIO_OPT4;
        eEJTOPT = EV_GPIO_OPT3;
    }

    pr_sd_dbg("\n>> [sdmmc_%u] CDZ... ", p_sdmmc_slot->slotNo);

LABEL_LOOP_HOTPLUG:

    if (_GetCardDetect(p_sdmmc_slot->p_mmc_priv)) // Insert (CDZ)
    {
        if ((FALSE) == _CardDetect_PlugDebounce(p_sdmmc_slot->p_mmc_priv, WT_DB_PLUG, TRUE))
            goto LABEL_LOOP_HOTPLUG;

        mmc_detect_change(p_sdmmc_slot->mmc, msecs_to_jiffies(WT_DB_SW_PLUG));
        pr_sd_dbg("(INS) OK!\n");

        Hal_CARD_SetGPIOIntAttr(EV_GPIO_OPT1, p_sdmmc_slot->cdzIRQNo);
        Hal_CARD_SetGPIOIntAttr(eINSOPT, p_sdmmc_slot->cdzIRQNo);
    }
    else // Remove (CDZ)
    {
        if ((TRUE) == _CardDetect_PlugDebounce(p_sdmmc_slot->p_mmc_priv, WT_DB_UNPLUG, FALSE))
            goto LABEL_LOOP_HOTPLUG;

        if (p_sdmmc_slot->mmc->card)
            mmc_card_set_removed(p_sdmmc_slot->mmc->card);

        Hal_SDMMC_StopProcessCtrl(eIP, TRUE);
        mmc_detect_change(p_sdmmc_slot->mmc, msecs_to_jiffies(WT_DB_SW_UNPLUG));
        pr_sd_dbg("(EJT) OK!\n");

        Hal_CARD_SetGPIOIntAttr(EV_GPIO_OPT1, p_sdmmc_slot->cdzIRQNo);
        Hal_CARD_SetGPIOIntAttr(eEJTOPT, p_sdmmc_slot->cdzIRQNo);

        gu32_SdmmcClk[eIP]    = 0;
        gu32_SdmmcStatus[eIP] = EV_OTHER_ERR;
        gu32_SdmmcCurCMD[eIP] = -1;
    }
    enable_irq(p_sdmmc_slot->cdzIRQNo);
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: sstar_sdmmc_set_ios
 *     @author jeremy.wang (2011/5/19)
 * Desc: Set IO bus Behavior
 *
 * @param p_mmc_host : mmc_host structure pointer
 * @param p_mmc_ios :  mmc_ios  structure pointer
 ----------------------------------------------------------------------------------------------------------*/
static void sstar_sdmmc_set_ios(struct mmc_host *p_mmc_host, struct mmc_ios *p_mmc_ios)
{
    struct sstar_sdmmc_slot *p_sdmmc_slot = mmc_priv(p_mmc_host);
    SlotEmType               eSlot        = (SlotEmType)p_sdmmc_slot->slotNo;

    CamOsMutexLock(&sdmmc_mutex[eSlot]);

    /****** Clock Setting*******/
    if ((p_sdmmc_slot->currClk != p_mmc_ios->clock) && (p_sdmmc_slot->currTiming != MMC_TIMING_MMC_HS200)
        && (p_sdmmc_slot->currTiming != MMC_TIMING_MMC_HS400))
    {
        /* Modified by Spade: enable clk in probe
                #if defined(CONFIG_OF)
                if(p_mmc_ios->clock>0)
                    clk_prepare_enable(gp_clkSlot[eSlot]);
                else
                    clk_disable_unprepare(gp_clkSlot[eSlot]);
                #endif
        */
        p_sdmmc_slot->currClk     = p_mmc_ios->clock;
        p_sdmmc_slot->currRealClk = sstar_sdmmc_setclock(p_sdmmc_slot->p_mmc_priv, p_sdmmc_slot->currClk);

        if ((p_sdmmc_slot->currRealClk == 0) && (p_sdmmc_slot->currClk != 0))
        {
            pr_sd_err(">> [sdmmc_%u] Set IOS => Clk=Error\n", eSlot);
        }
        else if (p_sdmmc_slot->currRealClk <= 400000)
        {
            sstar_sdmmc_setbustiming(p_sdmmc_slot->p_mmc_priv, 0xFF);
        }
        else
        {
            pr_sd_dbg(">> [sdmmc_%u] Set IOS => Clk=%u (Real=%u)\n", eSlot, p_sdmmc_slot->currClk,
                      p_sdmmc_slot->currRealClk);
        }
    }

    /****** Power Switch Setting *******/
    if (p_sdmmc_slot->currPowrMode != p_mmc_ios->power_mode)
    {
        p_sdmmc_slot->currPowrMode = p_mmc_ios->power_mode;
        pr_sd_dbg(">> [sdmmc_%u] Set IOS => Power=%u\n", eSlot, p_sdmmc_slot->currPowrMode);
        sstar_sdmmc_setpower(p_sdmmc_slot->p_mmc_priv, p_sdmmc_slot->currPowrMode);

        if (p_sdmmc_slot->currPowrMode == MMC_POWER_OFF)
        {
            p_sdmmc_slot->initFlag = 0;
            p_sdmmc_slot->sdioFlag = 0;
        }
    }

    /****** Bus Width Setting*******/
    if ((p_sdmmc_slot->currWidth != p_mmc_ios->bus_width) || !p_sdmmc_slot->initFlag)
    {
        p_sdmmc_slot->currWidth = p_mmc_ios->bus_width;
        sstar_sdmmc_setbuswidth(p_sdmmc_slot->p_mmc_priv, p_sdmmc_slot->currWidth);
        pr_sd_dbg(">> [sdmmc_%u] Set IOS => BusWidth=%u\n", eSlot, p_sdmmc_slot->currWidth);
    }

    /****** Bus Timing Setting*******/
    if ((p_sdmmc_slot->currTiming != p_mmc_ios->timing) || !p_sdmmc_slot->initFlag)
    {
        p_sdmmc_slot->currTiming = p_mmc_ios->timing;
        sstar_sdmmc_setbustiming(p_sdmmc_slot->p_mmc_priv, p_sdmmc_slot->currTiming);
        pr_sd_dbg(">> [sdmmc_%u] Set IOS => BusTiming=%u\n", eSlot, p_sdmmc_slot->currTiming);
    }

#if 0
    /****** Voltage Setting *******/
    if( (p_sdmmc_slot->currVdd != p_mmc_ios->signal_voltage) || !p_sdmmc_slot->initFlag)
    {
        p_sdmmc_slot->currVdd = p_mmc_ios->signal_voltage;

        // set voltage function
    }
#endif

    p_sdmmc_slot->initFlag = 1;

    CamOsMutexUnlock(&sdmmc_mutex[eSlot]);
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: sstar_sdmmc_get_ro
 *     @author jeremy.wang (2011/5/19)
 * Desc:  Get SD card read/write permission
 *
 * @param p_mmc_host : mmc_host structure pointer
 *
 * @return int  :  1 = read-only, 0 = read-write.
 ----------------------------------------------------------------------------------------------------------*/
static int sstar_sdmmc_get_ro(struct mmc_host *p_mmc_host)
{
    struct sstar_sdmmc_slot *p_sdmmc_slot = mmc_priv(p_mmc_host);
    SlotEmType               eSlot        = (SlotEmType)p_sdmmc_slot->slotNo;

    CamOsMutexLock(&sdmmc_mutex[eSlot]);

    if (_GetWriteProtect(eSlot)) // For CB2 HW Circuit, WP=>NWP
        p_sdmmc_slot->read_only = 1;
    else
        p_sdmmc_slot->read_only = 0;

    CamOsMutexUnlock(&sdmmc_mutex[eSlot]);

    pr_sd_main(">> [sdmmc_%u] Get RO => (%d)\n", eSlot, p_sdmmc_slot->read_only);

    return p_sdmmc_slot->read_only;
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: sstar_sdmmc_get_cd
 *     @author jeremy.wang (2011/6/17)
 * Desc: Get SD card detection status
 *
 * @param p_mmc_host : mmc_host structure pointer
 *
 * @return int  :  1 = Present
 ----------------------------------------------------------------------------------------------------------*/
static int sstar_sdmmc_get_cd(struct mmc_host *p_mmc_host)
{
    struct sstar_sdmmc_slot *p_sdmmc_slot = mmc_priv(p_mmc_host);

    if (_GetCardDetect(p_sdmmc_slot->p_mmc_priv))
    {
        p_sdmmc_slot->card_det = 1;
        if (!gu16_SlotClkEn[p_sdmmc_slot->ipOrder])
        {
            clk_prepare_enable(gp_clkSlot[p_sdmmc_slot->ipOrder]);
            if (p_sdmmc_slot->p_mmc_priv->u8_supportSD30)
                clk_prepare_enable(gp_clksyncSlot[p_sdmmc_slot->ipOrder]);
            if (!IS_ERR_OR_NULL(gp_clkpmmcuSlot[p_sdmmc_slot->ipOrder]))
                clk_prepare_enable(gp_clkpmmcuSlot[p_sdmmc_slot->ipOrder]);
            gu16_SlotClkEn[p_sdmmc_slot->ipOrder] = 1;
        }
    }
    else
    {
        p_sdmmc_slot->card_det = 0;
        if (gu16_SlotClkEn[p_sdmmc_slot->ipOrder])
        {
            clk_disable_unprepare(gp_clkSlot[p_sdmmc_slot->ipOrder]);
            if (p_sdmmc_slot->p_mmc_priv->u8_supportSD30)
                clk_disable_unprepare(gp_clksyncSlot[p_sdmmc_slot->ipOrder]);
            if (!IS_ERR_OR_NULL(gp_clkpmmcuSlot[p_sdmmc_slot->ipOrder]))
                clk_disable_unprepare(gp_clkpmmcuSlot[p_sdmmc_slot->ipOrder]);
            gu16_SlotClkEn[p_sdmmc_slot->ipOrder] = 0;
        }
    }

    return p_sdmmc_slot->card_det;
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: sstar_sdmmc_init_card
 *     @author jeremy.wang (2012/2/20)
 * Desc:
 *
 * @param p_mmc_host :
 * @param p_mmc_card :
 ----------------------------------------------------------------------------------------------------------*/
static void sstar_sdmmc_init_card(struct mmc_host *p_mmc_host, struct mmc_card *p_mmc_card)
{
#if 0 // Modify this step to ms_sdmmc_init_slot
    struct ms_sdmmc_slot   *p_sdmmc_slot  = mmc_priv(p_mmc_host);
    SlotEmType eSlot = (SlotEmType)p_sdmmc_slot->slotNo;
    IpOrder eIP     = ge_IPOrderSlot[eSlot];

    Hal_SDMMC_SDIODeviceCtrl(eIP, TRUE);
    p_sdmmc_slot->sdioFlag = 1;

    pr_sd_dbg(">> [sdmmc_%u] Found SDIO Device!\n", eSlot);
#endif
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: sstar_sdmmc_enable_sdio_irq
 *     @author jeremy.wang (2012/2/20)
 * Desc:
 *
 * @param p_mmc_host :
 * @param enable :
 ----------------------------------------------------------------------------------------------------------*/
static void sstar_sdmmc_enable_sdio_irq(struct mmc_host *p_mmc_host, int enable)
{
    struct sstar_sdmmc_slot *p_sdmmc_slot = mmc_priv(p_mmc_host);
    IpOrder                  eIP          = p_sdmmc_slot->ipOrder;

    // Remove Original VER_04 spin lock
    // TODO: SMP consideration!!!
    // should add spin lock here?

    Hal_SDMMC_SDIOIntDetCtrl(eIP, (BOOL_T)enable);

    if (enable)
    {
        pr_sdio_main(">> [sdmmc_%u] =========> SDIO IRQ EN=> (%d)\n", p_sdmmc_slot->slotNo, enable);
    }
}

#if defined(CONFIG_SUPPORT_SD30) || defined(CONFIG_SUPPORT_EMMC50)
/*----------------------------------------------------------------------------------------------------------
 *
 * Function: sstar_sdmmc_exec_tuning
 *     @author jeremy.wang (2018/1/9)
 * Desc:
 *
 * @param p_mmc_host :
 * @param opcode :
 *
 * @return int  :
 ----------------------------------------------------------------------------------------------------------*/
#if defined(CONFIG_SUPPORT_EMMC50)
static int sstar_emmc_execute_tuning(struct mmc_host *p_mmc_host, u32 opcode)
{
    unsigned char            u8Phase      = 0;
    signed char              s8retPhase   = 0;
    struct sstar_sdmmc_slot *p_sdmmc_slot = mmc_priv(p_mmc_host);
    unsigned char            u8BusTiming  = p_mmc_host->ios.timing;
    SlotEmType               eSlot        = (SlotEmType)p_sdmmc_slot->slotNo;
    IpOrder                  eIP          = (IpOrder)p_sdmmc_slot->ipOrder;

    // Clean All Pass Phase
    Hal_SDMMC_SavePassPhase(eIP, u8Phase, TRUE);
    gu8_SlotTuningSts[eIP] = 1;

    if (u8BusTiming == MMC_TIMING_MMC_HS200)
    {
        for (u8Phase = 0; u8Phase < 18; u8Phase++)
        {
            Hal_SDMMC_SetPhase(eIP, EV_SDR, u8Phase);

            if (!mmc_send_tuning(p_mmc_host, opcode, NULL))
            {
                if (Hal_SDMMC_SavePassPhase(eIP, u8Phase, FALSE))
                {
                    return 1;
                }

                // eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 0,">> [emmc_%d] SDR Tuning ...... Good Phase (%u)\n", eSlot,
                // u8Phase);
            }
        }

        if ((s8retPhase = Hal_SDMMC_FindFitPhaseSetting(eIP, 17)) < 0)
        {
            pr_err(">> [emmc_%d] Err: Fit Phase Finding (SDR) ...... Ret(%d)!\n", eSlot, s8retPhase);
            return 1;
        }

        Hal_SDMMC_SetPhase(eIP, EV_SDR, (U8)s8retPhase);

        pr_err(">> [emmc_%d] SDR PH(%d), ", eSlot, s8retPhase);
        Hal_SDMMC_Dump_GoodPhases(eIP);
        pr_err("\n");
    }
    else if (u8BusTiming == (MMC_TIMING_UHS_DDR50 | MMC_TIMING_MMC_DDR52))
    {
        for (u8Phase = 0; u8Phase < 7; u8Phase++)
        {
            Hal_SDMMC_SetPhase(eIP, EV_DDR, u8Phase);

            if (!mmc_send_tuning(p_mmc_host, opcode, NULL))
            {
                if (Hal_SDMMC_SavePassPhase(eIP, u8Phase, FALSE))
                {
                    return 1;
                }

                // eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 0,">> [emmc_%d] DDR Tuning ...... Good Phase (%u)\n", eSlot,
                // u8Phase);
            }
        }

        if ((s8retPhase = Hal_SDMMC_FindFitPhaseSetting(eIP, 6)) < 0)
        {
            pr_err(">> [emmc_%d] Err: Fit Phase Finding (DDR) ...... Ret(%d)!\n", eSlot, s8retPhase);
            return 1;
        }

        Hal_SDMMC_SetPhase(eIP, EV_DDR, (U8)s8retPhase);

        pr_err(">> [emmc_%d] DDR PH(%d), ", eSlot, s8retPhase);
        Hal_SDMMC_Dump_GoodPhases(eIP);
        pr_err("\n");
    }
    gu8_SlotTuningSts[eIP] = 0;

    return 0;
}
#else
static int        sstar_emmc_execute_tuning(struct mmc_host *p_mmc_host, u32 opcode)
{
    return 0;
}
#endif

#if defined(CONFIG_SUPPORT_SD30)
static int sstar_sd_execute_tuning(struct mmc_host *p_mmc_host, u32 opcode)
{
    struct sstar_sdmmc_slot *p_sdmmc_slot = mmc_priv(p_mmc_host);
    SlotEmType               eSlot        = (SlotEmType)p_sdmmc_slot->slotNo;
    IpOrder                  eIP          = (IpOrder)p_sdmmc_slot->ipOrder;
    unsigned char            u8BusTiming  = p_mmc_host->ios.timing;
    unsigned char            u8Phase      = 0;
    signed char              s8retPhase   = 0;

    if (eIP == IP_ORDER_1) // SDIO3.0
        return 0;

    // Clean All Pass Phase
    Hal_SDMMC_SavePassPhase(eIP, u8Phase, TRUE);
    gu8_SlotTuningSts[eIP] = 1;

    // Scan SDR Phase
    if ((u8BusTiming == MMC_TIMING_UHS_SDR50) || (u8BusTiming == MMC_TIMING_UHS_SDR104))
    {
        for (u8Phase = 0; u8Phase < 18; u8Phase++)
        {
            Hal_SDMMC_SetPhase(eIP, EV_SDR, u8Phase);

            if (!mmc_send_tuning(p_mmc_host, opcode, NULL))
            {
                if (Hal_SDMMC_SavePassPhase(eIP, u8Phase, FALSE))
                {
                    return 1;
                }

                pr_sd_main(">> [sdmmc_%u] SDR Tuning ...... Good Phase (%u)\n", eSlot, u8Phase);
            }
        }

        if ((s8retPhase = Hal_SDMMC_FindFitPhaseSetting(eIP, 17)) < 0)
        {
            pr_err(">> [sdmmc_%u] Err: Fit Phase Finding (SDR) ...... Ret(%d)!\n", eSlot, s8retPhase);
            return 1;
        }

        Hal_SDMMC_SetPhase(eIP, EV_SDR, (U8_T)s8retPhase);

        pr_err(">> [sdmmc_%u] SDR PH(%d), ", eSlot, s8retPhase);
        Hal_SDMMC_Dump_GoodPhases(eIP);
        pr_err("\n");

    } // Scan DDR Phase
    else if (u8BusTiming == MMC_TIMING_UHS_DDR50)
    {
        for (u8Phase = 0; u8Phase < 7; u8Phase++)
        {
            Hal_SDMMC_SetPhase(eIP, EV_DDR, u8Phase);

            if (!mmc_send_tuning(p_mmc_host, opcode, NULL))
            {
                if (Hal_SDMMC_SavePassPhase(eIP, u8Phase, FALSE))
                {
                    pr_err(">> [sdmmc_%u] Err: Phase Saving (DDR) over MAX_PHASE!\n", eSlot);
                    return 1;
                }

                pr_sd_main(">> [sdmmc_%u] DDR Tuning ...... Good Phase (%u)\n", eSlot, u8Phase);
            }
        }

        if ((s8retPhase = Hal_SDMMC_FindFitPhaseSetting(eIP, 6)) < 0)
        {
            pr_err(">> [sdmmc_%u] Err: Fit Phase Finding (DDR) ...... Ret(%d)!\n", eSlot, s8retPhase);
            return 1;
        }

        pr_sd_main(">> [sdmmc_%u] Exc Tuning => Sel DDR Phase (%d)\n", eSlot, s8retPhase);

        Hal_SDMMC_SetPhase(eIP, EV_DDR, (U8_T)s8retPhase);

        pr_err(">> [sdmmc_%u] ", eSlot);
        Hal_SDMMC_Dump_GoodPhases(eIP);
        pr_err("\n");
    }
    gu8_SlotTuningSts[eIP] = 0;

    return 0;
}
#else
static int sstar_sd_execute_tuning(struct mmc_host *p_mmc_host, u32 opcode)
{
    return 0;
}
#endif

static int sstar_sdmmc_execute_tuning(struct mmc_host *p_mmc_host, u32 opcode)
{
    if (!(p_mmc_host->caps2 & MMC_CAP2_NO_MMC))
        return sstar_emmc_execute_tuning(p_mmc_host, opcode);
    else
        return sstar_sd_execute_tuning(p_mmc_host, opcode);
}

#endif

/**********************************************************************************************************
 * Define Static Global Structs
 **********************************************************************************************************/
/*----------------------------------------------------------------------------------------------------------
 *  st_mmc_ops
 ----------------------------------------------------------------------------------------------------------*/
static const struct mmc_host_ops st_mmc_ops = {
    .request         = sstar_sdmmc_request,
    .set_ios         = sstar_sdmmc_set_ios,
    .get_ro          = sstar_sdmmc_get_ro,
    .get_cd          = sstar_sdmmc_get_cd,
    .init_card       = sstar_sdmmc_init_card,
    .enable_sdio_irq = sstar_sdmmc_enable_sdio_irq,
#if defined(CONFIG_SUPPORT_SD30)
    .start_signal_voltage_switch = sstar_sdmmc_switch_busvdd,
#endif
#if defined(CONFIG_SUPPORT_SD30) || defined(CONFIG_SUPPORT_EMMC50)
    .execute_tuning = sstar_sdmmc_execute_tuning,
#endif
};

#if defined(CONFIG_OF)

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: sstar_sdmmc_dts_init
 *     @author jeremy.wang (2017/3/24)
 * Desc: Device Tree Init
 *
 * @param p_dev : platform device
 ----------------------------------------------------------------------------------------------------------*/
static int sstar_sdmmc_dts_init(struct platform_device *p_dev)
{
    int                      ret;
    U8_T                     ipidx        = 0;
    struct sstar_sdmmc_host *p_sdmmc_host = platform_get_drvdata(p_dev);
    struct sstar_mmc_priv *  p_mmc_priv;

    p_mmc_priv = kzalloc(sizeof(struct sstar_mmc_priv), GFP_KERNEL);

    if (!p_mmc_priv)
    {
        pr_err(">> [mmc] Err: Failed to Allocate p_mmc_priv!\n\n");
        return -ENOMEM;
    }

    p_sdmmc_host->sdmmc_slot->p_mmc_priv = p_mmc_priv;
    memset(&(p_mmc_priv->mmc_PMuxInfo), 0xff, sizeof(MMCPadMuxInfo));

    p_mmc_priv->u8_slotNo = of_alias_get_id(p_dev->dev.of_node, "sdmmc");

    if (of_property_read_u8(p_dev->dev.of_node, "ip-order", &p_mmc_priv->mmc_PMuxInfo.u8_ipOrder))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [ip-order] option!\n");
        goto DTS_FAIL;
    }

#if (PADMUX_SET == PADMUX_SET_BY_FUNC)
    if (Hal_CARD_PadmuxGetting(&(p_mmc_priv->mmc_PMuxInfo)))
    {
        pr_err(">> [sdmmc_%u] Warn: Could not get SD pad group from Padmux dts!\n", p_mmc_priv->u8_slotNo);
        goto DTS_FAIL;
    }
#else
    if (of_property_read_u32(p_dev->dev.of_node, "cdz-pad", &p_mmc_priv->mmc_PMuxInfo.u32_PinCdzRst))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [cdz-pad] option!\n");
        // goto DTS_FAIL;
    }

    if (of_property_read_u32(p_dev->dev.of_node, "pwr-pad", &p_mmc_priv->mmc_PMuxInfo.u32_PinPWR))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [pwr-pad] option!\n");
        // goto DTS_FAIL;
    }

    if (of_property_read_u32(p_dev->dev.of_node, "bus-width", (U32_T *)&p_mmc_priv->mmc_PMuxInfo.u8_busWidth))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [bus-widthd] option!\n");
        goto DTS_FAIL;
    }
#endif

    ret = mmc_of_parse(p_sdmmc_host->sdmmc_slot->mmc);
    if (ret)
    {
        pr_err(">> [sdmmc] Err: Failed to parse host caps\n");
        return ret;
    }

    if (of_property_read_u32_array(p_dev->dev.of_node, "reg", p_mmc_priv->pIPBANKArr, 2))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [reg] option!\n");
        goto DTS_FAIL;
    }

    if (of_property_read_u32_array(p_dev->dev.of_node, "pll-reg", p_mmc_priv->pPLLIPBANKArr, 2))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [pll-reg] option!\n");
        goto DTS_FAIL;
    }

    if (of_property_read_u32_array(p_dev->dev.of_node, "cifd-reg", p_mmc_priv->pCIFDIPBANKArr, 2))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [cifd-reg] option!\n");
        goto DTS_FAIL;
    }

    if (of_property_read_u32_array(p_dev->dev.of_node, "pwr-save-reg", p_mmc_priv->pPWRSAVEIPBANKArr, 2))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [pwr-save-reg] option!\n");
        goto DTS_FAIL;
    }

    if (of_property_read_u8(p_dev->dev.of_node, "pad-order", &p_mmc_priv->mmc_PMuxInfo.u8_padOrder))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [pad-order] option!\n");
        goto DTS_FAIL;
    }

    if (of_property_read_u8(p_dev->dev.of_node, "trans-mode", &p_mmc_priv->u8_transMode))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [trans-mode] option!\n");
        goto DTS_FAIL;
    }

    if (of_property_read_u8(p_dev->dev.of_node, "cifd-mcg-off", &p_mmc_priv->u8_cifdMCGOff))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [cifd-mcg-off] option!\n");
        goto DTS_FAIL;
    }

    if (of_property_read_u8(p_dev->dev.of_node, "support-runtime-pm", &p_mmc_priv->u8_supportRPM))
    {
        pr_warn(">> [sdmmc] Err: Could not get dts [support-runtime-pm] option!\n");
        p_mmc_priv->u8_supportRPM = 0;
    }

    if (of_property_read_u8(p_dev->dev.of_node, "fake-cdz", &p_mmc_priv->u8_fakeCdz))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [fake-cdz] option!\n");
        goto DTS_FAIL;
    }

    if (of_property_read_u8(p_dev->dev.of_node, "rev-cdz", &p_mmc_priv->u8_revCdz))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [rev-cdz] option!\n");
        goto DTS_FAIL;
    }

    if (!(p_sdmmc_host->sdmmc_slot->mmc->caps2 & MMC_CAP2_NO_MMC))
    {
        if (of_property_read_u8(p_dev->dev.of_node, "support-cmd23", &p_mmc_priv->u8_supportCMD23))
        {
            pr_err(">> [sdmmc] Err: Could not get dts [support-cmd23] option!\n");
            goto DTS_FAIL;
        }
    }
    if (of_property_read_u32(p_dev->dev.of_node, "pwr-on-delay", &p_mmc_priv->u32_pwerOnDelay))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [pwr-on-delay] option!\n");
        goto DTS_FAIL;
    }

    if (of_property_read_u32(p_dev->dev.of_node, "pwr-off-delay", &p_mmc_priv->u32_pwerOffDelay))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [pwr-off-delay] option!\n");
        goto DTS_FAIL;
    }

    if (of_property_read_u8(p_dev->dev.of_node, "sdio-use-1bit", &p_mmc_priv->u8_sdioUse1Bit))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [sdio-use-1bit] option!\n");
    }

    if (of_property_read_u32(p_dev->dev.of_node, "clk-driving", (U32_T *)&p_mmc_priv->mmc_pinDrv.eDrvClk))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [clk-driving] option!\n");
    }

    if (of_property_read_u32(p_dev->dev.of_node, "cmd-driving", (U32_T *)&p_mmc_priv->mmc_pinDrv.eDrvCmd))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [cmd-driving] option!\n");
    }

    if (of_property_read_u32(p_dev->dev.of_node, "data-driving", (U32_T *)&p_mmc_priv->mmc_pinDrv.eDrvData))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [data-driving] option!\n");
    }

    if (of_property_read_u8(p_dev->dev.of_node, "en-clk-phase", &p_mmc_priv->mmc_clkPha.u8_clkPhaEn))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [en-clk-phase] option!\n");
    }

    if (of_property_read_u32(p_dev->dev.of_node, "rx-clk-phase", (U32_T *)&p_mmc_priv->mmc_clkPha.eClkPha_RX))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [rx-clk-phase] option!\n");
    }

    if (of_property_read_u32(p_dev->dev.of_node, "tx-clk-phase", (U32_T *)&p_mmc_priv->mmc_clkPha.eClkPha_TX))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [tx-clk-phase] option!\n");
    }

    if (of_property_read_u8(p_dev->dev.of_node, "en-eight-phase", &p_mmc_priv->mmc_clkPha.u8_eightPhaEn))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [en-eight-phase] option!\n");
    }

    if (of_property_read_u8(p_dev->dev.of_node, "rx-eight-phase", &p_mmc_priv->mmc_clkPha.u8_eightPha_RX))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [rx-eight-phase] option!\n");
    }

    if (of_property_read_u8(p_dev->dev.of_node, "tx-eight-phase", &p_mmc_priv->mmc_clkPha.u8_eightPha_TX))
    {
        pr_err(">> [sdmmc] Err: Could not get dts [tx-eight-phase] option!\n");
    }

    ipidx                  = p_mmc_priv->mmc_PMuxInfo.u8_ipOrder;
    p_mmc_priv->u32_maxClk = p_sdmmc_host->sdmmc_slot->mmc->f_max;

    if (p_mmc_priv->u8_sdioUse1Bit)
        p_mmc_priv->mmc_PMuxInfo.u8_busWidth = 1;
    // Check conflict
    if (!(p_sdmmc_host->sdmmc_slot->mmc->caps2 & MMC_CAP2_NO_SDIO))
    {
        if (geIpTypeIp[ipidx] != IP_TYPE_SDIO)
        {
            pr_err(">> mmc_%u, When SDIO is used, IpType need to be IP_TYPE_SDIO, current setting = %u\n", ipidx,
                   (U8_T)geIpTypeIp[ipidx]);
            goto DTS_FAIL;
        }
    }

    // MIE irq depend on which IP
    p_mmc_priv->u32_mieIrqNo = of_irq_get_byname(p_dev->dev.of_node, gu8_mie_irq_name[ipidx]);

    // CDZ irq
    p_mmc_priv->u32_cdzIrqNo = of_irq_get_byname(p_dev->dev.of_node, gu8_irq_name[ipidx]);

    if (!Hal_CARD_CheckCdzMode(p_mmc_priv->mmc_PMuxInfo) || (p_mmc_priv->u32_cdzIrqNo <= 0))
    {
        p_sdmmc_host->sdmmc_slot->cdzIRQNo = sstar_gpio_to_irq(p_mmc_priv->mmc_PMuxInfo.u32_PinCdzRst);
        if (p_sdmmc_host->sdmmc_slot->cdzIRQNo <= 0)
        {
            pr_err(">> [sdmmc] slotNo = %u, cann't get cdz irq number\n", p_mmc_priv->u8_slotNo);
            // continue;
        }
        p_mmc_priv->u32_cdzIrqNo = p_sdmmc_host->sdmmc_slot->cdzIRQNo;
    }
    /**
     * 1.For SD card insert/eject operation, one operation should only trigger one time interrupt, but, actually,
     * one operation will trigger corresponding interrupt twice times at some time because of voltage signal's
     * jitter and linux kernel's "lazy disbale" and "delayed interrupt disable" feature which meaning that no mask/
     * unmask function will be truely called in disable_irq_nosync/enable_irq function respectively.
     * 2.So, for low speed interrupt, such as, SD card insert/eject operation, should disabled it's "lazy disable"
     * feature and use the SD card hotplug routine's debounce founction to handle voltage jitter.
     * 3.For SD card's interrupt line, after disable "lazy disable" feature, mask/unmask function of GPI_INTC will
     * be called in disable_irq_nosync/enable_irq function respectively.
     */
    irq_set_status_flags(p_mmc_priv->u32_cdzIrqNo, IRQ_DISABLE_UNLAZY);

    // Debug
    pr_sd_dbg(">> [sdmmc] SlotNo = %u\n", p_mmc_priv->u8_slotNo);
    pr_sd_dbg(">> [sdmmc] RevCDZ = %u\n", p_mmc_priv->u8_revCdz);
    pr_sd_dbg(">> [sdmmc] IP = %u \n", p_mmc_priv->mmc_PMuxInfo.u8_ipOrder);
    pr_sd_dbg(">> [sdmmc] PAD = %u \n", p_mmc_priv->mmc_PMuxInfo.u8_padOrder);
    pr_sd_dbg(">> [sdmmc] IP_BANK = %x \n", p_mmc_priv->pIPBANKArr[0]);
    pr_sd_dbg(">> [sdmmc] PLL_IP_BANK = %x \n", p_mmc_priv->pPLLIPBANKArr[0]);
    pr_sd_dbg(">> [sdmmc] CIFD_IP_BANK = %x \n", p_mmc_priv->pCIFDIPBANKArr[0]);
    pr_sd_dbg(">> [sdmmc] CIFD_IP_BANK = %x \n", p_mmc_priv->pPWRSAVEIPBANKArr[0]);
    pr_sd_dbg(">> [sdmmc] MaxClk = %u \n", p_mmc_priv->u32_maxClk);
    pr_sd_dbg(">> [sdmmc] BusWidth = %u \n", p_mmc_priv->mmc_PMuxInfo.u8_busWidth);
    pr_sd_dbg(">> [sdmmc] FakeCDZ = %u \n", p_mmc_priv->u8_fakeCdz);
    pr_sd_dbg(">> [sdmmc] SupportCMD23 = %u \n", p_mmc_priv->u8_supportCMD23);
    pr_sd_dbg(">> [sdmmc] CIFDMCGOFF = %u \n", p_mmc_priv->u8_cifdMCGOff);
    pr_sd_dbg(">> [sdmmc] SupportRPM = %u \n", p_mmc_priv->u8_supportRPM);
    pr_sd_dbg(">> [sdmmc] CdzNo = %u \n", p_mmc_priv->mmc_PMuxInfo.u32_PinCdzRst);
    pr_sd_dbg(">> [sdmmc] PwrNo = %u \n", p_mmc_priv->mmc_PMuxInfo.u32_PinPWR);
    pr_sd_dbg(">> [sdmmc] SDIOuse1Bit = %u \n", p_mmc_priv->u8_sdioUse1Bit);
    pr_sd_dbg(">> [sdmmc] PwrOffDelay = %u \n", p_mmc_priv->u32_pwerOffDelay);
    pr_sd_dbg(">> [sdmmc] PwrOnDelay = %u \n", p_mmc_priv->u32_pwerOnDelay);
    pr_sd_dbg(">> [sdmmc] MieIntNo = %u \n", p_mmc_priv->u32_mieIrqNo);
    pr_sd_dbg(">> [sdmmc] CdzIntNo = %u \n", p_mmc_priv->u32_cdzIrqNo);
    pr_sd_dbg(">> [sdmmc] EnClkPhase = %u \n", p_mmc_priv->mmc_clkPha.u8_clkPhaEn);
    pr_sd_dbg(">> [sdmmc] TXClkPhase = %u \n", p_mmc_priv->mmc_clkPha.eClkPha_TX);
    pr_sd_dbg(">> [sdmmc] RXClkPhase = %u \n", p_mmc_priv->mmc_clkPha.eClkPha_RX);
    pr_sd_dbg(">> [sdmmc] ClkDriving = %u \n", p_mmc_priv->mmc_pinDrv.eDrvClk);
    pr_sd_dbg(">> [sdmmc] CmdDriving = %u \n", p_mmc_priv->mmc_pinDrv.eDrvCmd);
    pr_sd_dbg(">> [sdmmc] DataDriving = %u \n", p_mmc_priv->mmc_pinDrv.eDrvData);

#ifdef CONFIG_CAM_CLK
    //
    U32_T SdmmcClk = 0;
    ipidx          = p_mmc_priv->mmc_PMuxInfo.u8_ipOrder;

    /* Get Clock info from DTS */
    SdmmcClk = 0;
    of_property_read_u32_index(p_dev->dev.of_node, "camclk", ipidx, &(SdmmcClk));

    if (!SdmmcClk)
    {
        // printk(KERN_DEBUG "[%s] Fail to get clk!\n", __func__);
        pr_err(">> [sdmmc_%u] Err: Failed to get dts clock tree!\n", p_mmc_priv->u8_slotNo);
        goto DTS_FAIL;
    }

    CamClkRegister("Sdmmc", SdmmcClk, &(gp_clkSlot[ipidx]));
    CamClkSetOnOff(gp_clkSlot[ipidx], 1);
#else

    /* Get Clock info from DTS */
    gp_clkSlot[ipidx] = of_clk_get_by_name(p_dev->dev.of_node, gu8_sd_clk_name[ipidx]);

    if (IS_ERR(gp_clkSlot[ipidx]))
    {
        pr_err(">> [mmc_%u] Err: Failed to get dts clock tree!\n", p_mmc_priv->u8_slotNo);
        goto DTS_FAIL;
    }

    gp_clkpmmcuSlot[ipidx] = of_clk_get_by_name(p_dev->dev.of_node, gu8_pm_mcu_name[ipidx]);

    if (IS_ERR_OR_NULL(gp_clkpmmcuSlot[ipidx]))
    {
        pr_err(">> [mmc_%u] Warn: Failed to get dts pm mcu clock tree!\n", p_mmc_priv->u8_slotNo);
    }

#if defined(CONFIG_SUPPORT_SD30)
    if (p_sdmmc_host->sdmmc_slot->mmc->caps & (MMC_CAP_UHS_SDR104 | MMC_CAP_UHS_DDR50 | MMC_CAP_UHS_SDR50))
    {
        p_mmc_priv->u8_supportSD30 = 1;
        CLK_IP_1x_p[ipidx]         = clk_hw_get_parent_by_index(__clk_get_hw(gp_clkSlot[ipidx]), 14);
        gp_clksyncSlot[ipidx]      = of_clk_get_by_name(p_dev->dev.of_node, gu8_sync_clk_name[ipidx]);
        if (IS_ERR(gp_clksyncSlot[ipidx]))
        {
            pr_err(">> [sdmmc_%u] Err: Failed to get dts syn clock tree!\n", p_mmc_priv->u8_slotNo);
            goto DTS_FAIL;
        }
    }
#else
    p_mmc_priv->u8_supportSD30 = 0;
    p_sdmmc_host->sdmmc_slot->mmc->caps &= ~(MMC_CAP_UHS_SDR104 | MMC_CAP_UHS_DDR50 | MMC_CAP_UHS_SDR50);
#endif

#if defined(CONFIG_SUPPORT_EMMC50)
    if (p_sdmmc_host->sdmmc_slot->mmc->caps2 & (MMC_CAP2_HS400_1_8V | MMC_CAP2_HS200_1_8V_SDR))
    {
        p_mmc_priv->u8_supportEMMC50 = 1;
        CLK_IP_1x_p[ipidx]           = clk_hw_get_parent_by_index(__clk_get_hw(gp_clkSlot[ipidx]), 14);
        CLK_IP_2x_p[ipidx]           = clk_hw_get_parent_by_index(__clk_get_hw(gp_clkSlot[ipidx]), 15);
        gp_clksyncSlot[ipidx]        = of_clk_get_by_name(p_dev->dev.of_node, gu8_sync_clk_name[ipidx]);
        if (IS_ERR(gp_clksyncSlot[ipidx]))
        {
            pr_err(">> [sdmmc_%u] Err: Failed to get dts syn clock tree!\n", p_mmc_priv->u8_slotNo);
            goto DTS_FAIL;
        }
    }
#else
    p_mmc_priv->u8_supportEMMC50 = 0;
    p_sdmmc_host->sdmmc_slot->mmc->caps2 &= ~(MMC_CAP2_HS400_1_8V | MMC_CAP2_HS200_1_8V_SDR);
#endif
#endif

    return 0;
DTS_FAIL:
    kfree(p_mmc_priv);
    return 1;
}

#endif

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: sstar_sdmmc_init_slot
 *     @author jeremy.wang (2015/12/9)
 * Desc: Init Slot Setting
 *
 * @param slotNo : Slot Number
 * @param p_sdmmc_host : ms_sdmmc_host
 *
 * @return int  : Error Status; Return 0 if no error
 ----------------------------------------------------------------------------------------------------------*/
static int sstar_sdmmc_init_slot(struct sstar_sdmmc_host *p_sdmmc_host)
{
    struct sstar_sdmmc_slot *p_sdmmc_slot;
    struct mmc_host *        p_mmc_host;
    struct sstar_mmc_priv *  p_mmc_priv;
    IpOrder                  eIP;
    int                      nRet = 0;

//###########################################################################################################
#if (EN_MSYS_REQ_DMEM)
    //###########################################################################################################
    MSYS_DMEM_INFO mem_info;
//###########################################################################################################
#endif
    //###########################################################################################################

#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
    if (dma_set_mask_and_coherent(&p_sdmmc_host->pdev->dev, DMA_BIT_MASK(64)))
        pr_err("no suitable DMA available \n");
#endif

    /****** (1) Allocte MMC and SDMMC host ******/
    p_mmc_host = mmc_alloc_host(sizeof(struct sstar_sdmmc_slot), &p_sdmmc_host->pdev->dev);

    if (!p_mmc_host)
    {
        pr_err(">> [mmc] Err: Failed to Allocate mmc_host!\n");
        return -ENOMEM;
    }

    /****** (2) SDMMC host setting ******/
    p_sdmmc_slot      = mmc_priv(p_mmc_host);
    p_sdmmc_slot->mmc = p_mmc_host;

    p_sdmmc_host->sdmmc_slot   = p_sdmmc_slot;
    p_sdmmc_slot->parent_sdmmc = p_sdmmc_host;

    /****** (3) Parse SDMMC DTS ******/
#if defined(CONFIG_OF)
    if (sstar_sdmmc_dts_init(p_sdmmc_host->pdev))
    {
        nRet = -EINVAL;
        goto INIT_FAIL_4;
    }
#else
    // CONFIG_OF is necessary
    nRet = 1;
    goto INIT_FAIL_4;
#endif

    p_mmc_priv = p_sdmmc_slot->p_mmc_priv;
    eIP        = p_mmc_priv->mmc_PMuxInfo.u8_ipOrder;

#if (!EN_SDMMC_BRO_DMA)

    //###########################################################################################################
#if !(EN_MSYS_REQ_DMEM)
    //###########################################################################################################
    p_sdmmc_slot->dma_buffer = dma_alloc_coherent(&p_sdmmc_host->pdev->dev, MAX_BLK_COUNT * MAX_BLK_SIZE,
                                                  &p_sdmmc_slot->dma_phy_addr, GFP_KERNEL);
    if (!p_sdmmc_slot->dma_buffer)
    {
        pr_err(">> [sdmmc_%u] Err: Failed to Allocate sdmmc_host DMA buffer\n", p_mmc_priv->u8_slotNo);
        nRet = -ENOMEM;
        goto INIT_FAIL_4;
    }
    //###########################################################################################################
#else
    //###########################################################################################################
    mem_info.length = MAX_BLK_COUNT * MAX_BLK_SIZE;
    strcpy(mem_info.name, "MMC_SGBUF");
    if (msys_request_dmem(&mem_info))
    {
        pr_err(">> [sdmmc_%u] Err: Failed to Allocate sdmmc_host DMA buffer\n", p_mmc_priv->u8_slotNo);
        nRet = -ENOMEM;
        goto INIT_FAIL_4;
    }

    p_sdmmc_slot->dma_phy_addr = (dma_addr_t)mem_info.phys;
    p_sdmmc_slot->dma_buffer   = (U32_T *)((U32_T)mem_info.kvirt);
    //###########################################################################################################
#endif

#else
    if (IsAdmaMode(p_mmc_priv))
    {
        //###########################################################################################################
#if !(EN_MSYS_REQ_DMEM)
        //###########################################################################################################
        p_sdmmc_slot->adma_buffer = dma_alloc_coherent(&p_sdmmc_host->pdev->dev, sizeof(AdmaDescStruct) * MAX_SEG_CNT,
                                                       &p_sdmmc_slot->adma_phy_addr, GFP_KERNEL);
        if (!p_sdmmc_slot->adma_buffer)
        {
            pr_err(">> [sdmmc_%u] Err: Failed to Allocate sdmmc_host ADMA buffer\n", p_mmc_priv->u8_slotNo);
            nRet = -ENOMEM;
            goto INIT_FAIL_4;
        }
        //###########################################################################################################
#else
        //###########################################################################################################
        mem_info.length = sizeof(AdmaDescStruct) * MAX_SEG_CNT;
        sprintf(mem_info.name, "%s%01d", "MMC_ADMABUF", p_mmc_priv->u8_slotNo);
        if (msys_request_dmem(&mem_info))
        {
            pr_err(">> [sdmmc_%u] Err: Failed to Allocate sdmmc_host ADMA buffer\n", p_mmc_priv->u8_slotNo);
            nRet = -ENOMEM;
            goto INIT_FAIL_4;
        }

        p_sdmmc_slot->adma_phy_addr = (dma_addr_t)mem_info.phys;
        p_sdmmc_slot->adma_buffer   = (U32_T *)((U32_T)mem_info.kvirt);

        //###########################################################################################################
#endif
    }
#endif

    p_sdmmc_slot->slotNo     = p_mmc_priv->u8_slotNo;
    p_sdmmc_slot->ipOrder    = p_mmc_priv->mmc_PMuxInfo.u8_ipOrder;
    p_sdmmc_slot->pmrsaveClk = Hal_CARD_FindClockSetting(eIP, 400000);
    p_sdmmc_slot->mieIRQNo   = p_mmc_priv->u32_mieIrqNo;
    p_sdmmc_slot->cdzIRQNo   = p_mmc_priv->u32_cdzIrqNo;
    p_sdmmc_slot->pwrGPIONo  = p_mmc_priv->mmc_PMuxInfo.u32_PinPWR;
    p_sdmmc_slot->initFlag   = 0;
    p_sdmmc_slot->sdioFlag   = 0;

    p_sdmmc_slot->currClk      = 0;
    p_sdmmc_slot->currWidth    = 0;
    p_sdmmc_slot->currTiming   = 0;
    p_sdmmc_slot->currPowrMode = MMC_POWER_OFF;
    p_sdmmc_slot->currVdd      = 0;
    p_sdmmc_slot->currDDR      = 0;

    /***** (4) MMC host setting ******/
    p_mmc_host->ops   = &st_mmc_ops;
    p_mmc_host->f_min = p_sdmmc_slot->pmrsaveClk;
    p_mmc_host->ocr_avail =
        MMC_VDD_32_33 | MMC_VDD_31_32 | MMC_VDD_30_31 | MMC_VDD_29_30 | MMC_VDD_28_29 | MMC_VDD_27_28 | MMC_VDD_165_195;
    if (p_mmc_priv->u8_sdioUse1Bit)
        p_mmc_host->caps &= ~MMC_CAP_4_BIT_DATA;

    // SDIO & EMMC Card are non-removable
    if (p_mmc_host->caps & MMC_CAP_NONREMOVABLE)
    {
        if (!gu16_SlotClkEn[eIP])
        {
            clk_prepare_enable(gp_clkSlot[eIP]);
            if ((p_mmc_priv->u8_supportSD30 == 1) || (p_mmc_priv->u8_supportEMMC50 == 1))
                clk_prepare_enable(gp_clksyncSlot[eIP]);

            if (!IS_ERR_OR_NULL(gp_clkpmmcuSlot[eIP]))
                clk_prepare_enable(gp_clkpmmcuSlot[eIP]);
            gu16_SlotClkEn[eIP] = 1;
        }
    }

#if (EN_SDMMC_BRO_DMA)
    p_mmc_host->max_blk_count = MAX_BRO_BLK_COUNT;
#else
    p_mmc_host->max_blk_count = MAX_BLK_COUNT;
#endif
    p_mmc_host->max_blk_size = MAX_BLK_SIZE;

    p_mmc_host->max_req_size = p_mmc_host->max_blk_count * p_mmc_host->max_blk_size;
    p_mmc_host->max_seg_size = p_mmc_host->max_req_size;

    p_mmc_host->max_segs = MAX_SEG_CNT;

    if (!(p_mmc_host->caps2 & MMC_CAP2_NO_SD))
        geCardType[eIP] = CARD_TYPE_SD;
    else if (!(p_mmc_host->caps2 & MMC_CAP2_NO_SDIO))
        geCardType[eIP] = CARD_TYPE_SDIO;
    else if (!(p_mmc_host->caps2 & MMC_CAP2_NO_MMC))
        geCardType[eIP] = CARD_TYPE_EMMC;

#ifdef CONFIG_SUPPORT_SDMMC_AT_SMOKE
    sstar_sdmmc_select_speedmode(eIP, p_mmc_host);
#endif

#if defined(ENABLE_EMMC_PRE_DEFINED_BLK_CNT) && ENABLE_EMMC_PRE_DEFINED_BLK_CNT
    if (p_mmc_priv->u8_supportCMD23)
        p_mmc_host->caps |= MMC_CAP_CMD23;
#endif

    if (geCardType[eIP] == CARD_TYPE_EMMC)
    {
        p_mmc_host->caps |= MMC_CAP_WAIT_WHILE_BUSY;
    }

    /****** (5) IP Once Setting for Different Platform ******/
    sstar_sdmmc_enable(p_mmc_priv);

    /****** (7) Interrupt Source Setting ******/
    gst_IntSourceSlot[p_mmc_priv->u8_slotNo].slotNo = p_mmc_priv->u8_slotNo;
    gst_IntSourceSlot[p_mmc_priv->u8_slotNo].eIP    = eIP;
    gst_IntSourceSlot[p_mmc_priv->u8_slotNo].p_data = p_sdmmc_slot;

    /*****  (8) Spinlock Init for Reg Protection ******/
    spin_lock_init(&g_RegLockSlot[p_mmc_priv->u8_slotNo]);
    CamOsMutexInit(&sdmmc_mutex[p_mmc_priv->u8_slotNo]);

    /****** (9) Register IP IRQ *******/
#if defined(CONFIG_SSTAR_SDMMC_INTERRUPT_MODE)
    Hal_SDMMC_MIEIntCtrl(eIP, FALSE);

#if 1 // MIE

    nRet = request_irq(p_sdmmc_slot->mieIRQNo, Hal_CARD_INT_MIE, IRQF_TRIGGER_NONE, DRIVER_NAME "_mie",
                       &gst_IntSourceSlot[p_mmc_priv->u8_slotNo]);
    if (nRet)
    {
        pr_err(">> [mmc_%u] Err: Failed to request MIE Interrupt (%u)!\n", p_mmc_priv->u8_slotNo,
               p_sdmmc_slot->mieIRQNo);
        goto INIT_FAIL_3;
    }

    Hal_SDMMC_MIEIntCtrl(eIP, TRUE);

    if (!(p_mmc_host->caps2 & MMC_CAP2_NO_SDIO))
    {
        if (p_mmc_host->caps & MMC_CAP_SDIO_IRQ)
        {
            Hal_CARD_INT_SetMIEIntEn_ForSDIO(eIP, TRUE);
        }
        else
        {
            gb_Sdio_Dis_Intr_By_IP[eIP] = TRUE;
            Hal_CARD_INT_SetMIEIntEn_ForSDIO(eIP, FALSE);
        }
        Hal_SDMMC_SDIODeviceCtrl(eIP, TRUE);
        p_sdmmc_slot->sdioFlag = 1;

        pr_sd_dbg(">> [mmc_%u] Enable SDIO Interrupt Mode! \n", p_mmc_priv->u8_slotNo);
    }
    else
    {
        gb_Sdio_Dis_Intr_By_IP[eIP] = TRUE;
        Hal_SDMMC_SDIODeviceCtrl(eIP, FALSE);
        p_sdmmc_slot->sdioFlag = 0;
    }
#endif

#endif

    // Don't pre power up
    p_mmc_host->caps2 |= MMC_CAP2_NO_PRESCAN_POWERUP;

    // support runtime PM
    if (!(geCardType[eIP] == CARD_TYPE_EMMC))
    {
        if (p_mmc_priv->u8_supportRPM)
        {
            p_mmc_host->caps |= ((geCardType[eIP] == CARD_TYPE_SDIO) ? MMC_CAP_POWER_OFF_CARD : MMC_CAP_AGGRESSIVE_PM);
            p_mmc_host->pm_flags |= ((geCardType[eIP] == CARD_TYPE_SDIO) ? MMC_PM_KEEP_POWER : 0);
            pm_runtime_set_active(&p_sdmmc_host->pdev->dev);
            pm_runtime_set_autosuspend_delay(&p_sdmmc_host->pdev->dev, 50);
            pm_runtime_use_autosuspend(&p_sdmmc_host->pdev->dev);
            pm_runtime_enable(&p_sdmmc_host->pdev->dev);
        }
    }
    nRet = mmc_add_host(p_mmc_host);
    if (nRet)
        goto INIT_FAIL_2;
    // CDZ IRQ
    if (!(p_mmc_host->caps & (MMC_CAP_NEEDS_POLL | MMC_CAP_NONREMOVABLE)))
    {
        tasklet_init(&p_sdmmc_slot->hotplug_tasklet, sstar_sdmmc_hotplug, (unsigned long)p_sdmmc_slot);

        //
        Hal_CARD_SetGPIOIntAttr((_GetCardDetect(p_mmc_priv) ? EV_GPIO_OPT3 : EV_GPIO_OPT4), p_sdmmc_slot->cdzIRQNo);
        Hal_CARD_SetGPIOIntAttr(EV_GPIO_OPT1, p_sdmmc_slot->cdzIRQNo);

        nRet = request_irq(p_sdmmc_slot->cdzIRQNo, sstar_sdmmc_cdzint, IRQF_TRIGGER_NONE, DRIVER_NAME "_cdz",
                           &gst_IntSourceSlot[p_mmc_priv->u8_slotNo]);
        if (nRet)
        {
            pr_err(">> [mmc_%u] Err: Failed to request CDZ Interrupt (%u)!\n", p_mmc_priv->u8_slotNo,
                   p_sdmmc_slot->cdzIRQNo);
            goto INIT_FAIL_1;
        }

        pr_sd_dbg(">> [mmc_%u] Int CDZ use Ext GPIO IRQ: (%u)\n", p_mmc_priv->u8_slotNo, p_sdmmc_slot->cdzIRQNo);

        Hal_CARD_SetGPIOIntAttr(EV_GPIO_OPT2, p_sdmmc_slot->cdzIRQNo);

#if 0 // Make irq wake up system from suspend.
        irq_set_irq_wake(p_sdmmc_slot->cdzIRQNo, TRUE);
#endif
    }

#ifdef CONFIG_SUPPORT_SDMMC_COMMAND
    list_add(&p_sdmmc_slot->list, &sdmmc_command_list);
#endif

    // Return Success
    return 0;

INIT_FAIL_1:
    tasklet_kill(&p_sdmmc_slot->hotplug_tasklet);
    free_irq(p_sdmmc_slot->mieIRQNo, &gst_IntSourceSlot[p_mmc_priv->u8_slotNo]);

    mmc_remove_host(p_mmc_host);
INIT_FAIL_2:
    if (!(geCardType[eIP] == CARD_TYPE_EMMC))
    {
        if (p_mmc_priv->u8_supportRPM)
        {
            pm_runtime_get_sync(&p_sdmmc_host->pdev->dev);
            pm_runtime_disable(&p_sdmmc_host->pdev->dev);
            pm_runtime_put_noidle(&p_sdmmc_host->pdev->dev);
        }
    }

#if defined(CONFIG_SSTAR_SDMMC_INTERRUPT_MODE)
INIT_FAIL_3:
#endif
#if (!EN_SDMMC_BRO_DMA)

//###########################################################################################################
#if !(EN_MSYS_REQ_DMEM)
    //###########################################################################################################
    if (p_sdmmc_slot->dma_buffer)
        dma_free_coherent(&p_sdmmc_host->pdev->dev, MAX_BLK_COUNT * MAX_BLK_SIZE, p_sdmmc_slot->dma_buffer,
                          p_sdmmc_slot->dma_phy_addr);
//###########################################################################################################
#else
    //###########################################################################################################
    mem_info.length = MAX_BLK_COUNT * MAX_BLK_SIZE;
    strcpy(mem_info.name, "SDMMC_SGBUF");
    mem_info.phys = (unsigned long long)p_sdmmc_slot->dma_phy_addr;
    msys_release_dmem(&mem_info);
//###########################################################################################################
#endif

#else

#if !(EN_MSYS_REQ_DMEM)
    if (IsAdmaMode(p_mmc_priv))
    {
        if (p_sdmmc_slot->adma_buffer)
            dma_free_coherent(&p_sdmmc_host->pdev->dev, sizeof(AdmaDescStruct) * MAX_SEG_CNT, p_sdmmc_slot->adma_buffer,
                              p_sdmmc_slot->adma_phy_addr);
    }
#endif

#endif

INIT_FAIL_4:
    mmc_free_host(p_mmc_host);
    return nRet;
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: sstar_sdmmc_probe
 *     @author jeremy.wang (2011/5/18)
 * Desc: Probe Platform Device
 *
 * @param p_dev : platform_device
 *
 * @return int : Error Status; Return 0 if no error.
 ----------------------------------------------------------------------------------------------------------*/
// struct ms_sdmmc_host *p_sdmmc_host;
static int sstar_sdmmc_probe(struct platform_device *p_dev)
{
    struct sstar_sdmmc_host *p_sdmmc_host;
    int                      ret    = 0;
    unsigned int             slotNo = of_alias_get_id(p_dev->dev.of_node, "sdmmc");

    pr_info(">> [sdmmc_%u] sstar_sdmmc_probe \n", slotNo);

    p_sdmmc_host = kzalloc(sizeof(struct sstar_sdmmc_host), GFP_KERNEL);

    if (!p_sdmmc_host)
    {
        pr_err(">> [sdmmc_%u] Err: Failed to Allocate p_sdmmc_host!\n\n", slotNo);
        return -ENOMEM;
    }

    p_sdmmc_host->pdev = p_dev;

    /***** device data setting ******/
    platform_set_drvdata(p_dev, p_sdmmc_host);

    /***** device PM wakeup setting ******/
    device_init_wakeup(&p_dev->dev, 1);

    ret = sstar_sdmmc_init_slot(p_sdmmc_host);
    if (ret != 0)
    {
        pr_err(">> [sdmmc_%u] Err: Failed to init slot!\n", slotNo);
        kfree(p_sdmmc_host);
        return ret;
    }

    // For getting and showing device attributes from/to user space.
#ifdef CONFIG_SUPPORT_SDMMC_UT_VERIFY
    ret = sysfs_create_group(&p_sdmmc_host->pdev->dev.kobj, &sstar_sdmmc_ut_attr_grp);
#endif
    ret = sysfs_create_group(&p_sdmmc_host->pdev->dev.kobj, &sstar_sdmmc_debug_attr_grp);
    return 0;
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: sstar_sdmmc_remove_slot
 *     @author jeremy.wang (2015/12/9)
 * Desc: Remove Slot Setting
 *
 * @param slotNo : Slot Number
 * @param p_sdmmc_host : ms_sdmmc_host
 ----------------------------------------------------------------------------------------------------------*/
static void sstar_sdmmc_remove_slot(unsigned int slotNo, struct sstar_sdmmc_host *p_sdmmc_host)
{
    struct sstar_sdmmc_slot *p_sdmmc_slot = p_sdmmc_host->sdmmc_slot;
    struct mmc_host *        p_mmc_host   = p_sdmmc_slot->mmc;
    SlotEmType               eSlot        = (SlotEmType)slotNo;

//###########################################################################################################
#if (EN_MSYS_REQ_DMEM)
    //###########################################################################################################
    MSYS_DMEM_INFO mem_info;
//###########################################################################################################
#endif
    //###########################################################################################################
    // runtime PM would resume before release buf
    mmc_remove_host(p_mmc_host);
    if (!(geCardType[eSlot] == CARD_TYPE_EMMC))
    {
        if (p_sdmmc_slot->p_mmc_priv->u8_supportRPM)
        {
            pm_runtime_get_sync(&p_sdmmc_host->pdev->dev);
            pm_runtime_disable(&p_sdmmc_host->pdev->dev);
            pm_runtime_put_noidle(&p_sdmmc_host->pdev->dev);
        }
    }

//###########################################################################################################
#if !(EN_MSYS_REQ_DMEM)
    //###########################################################################################################
    if (!IsAdmaMode(p_sdmmc_slot->p_mmc_priv))
    {
        if (p_sdmmc_slot->dma_buffer)
            dma_free_coherent(&p_sdmmc_host->pdev->dev, MAX_BLK_COUNT * MAX_BLK_SIZE, p_sdmmc_slot->dma_buffer,
                              p_sdmmc_slot->dma_phy_addr);
    }
    else
    {
        if (p_sdmmc_slot->adma_buffer)
            dma_free_coherent(&p_sdmmc_host->pdev->dev, sizeof(AdmaDescStruct) * MAX_SEG_CNT, p_sdmmc_slot->adma_buffer,
                              p_sdmmc_slot->adma_phy_addr);
    }
//###########################################################################################################
#else
    //###########################################################################################################
    mem_info.length = MAX_BLK_COUNT * MAX_BLK_SIZE;
    strcpy(mem_info.name, "SDMMC_SGBUF");
    mem_info.phys = (unsigned long long)p_sdmmc_slot->dma_phy_addr;
    msys_release_dmem(&mem_info);
//###########################################################################################################
#endif
    CamOsMutexDestroy(&sdmmc_mutex[eSlot]);

#if defined(CONFIG_SSTAR_SDMMC_INTERRUPT_MODE)
    free_irq(p_sdmmc_slot->mieIRQNo, &gst_IntSourceSlot[eSlot]);
#endif

    if (!(p_mmc_host->caps & (MMC_CAP_NEEDS_POLL | MMC_CAP_NONREMOVABLE)))
    {
        tasklet_kill(&p_sdmmc_slot->hotplug_tasklet);
        if (p_sdmmc_slot->cdzIRQNo)
        {
            free_irq(p_sdmmc_slot->cdzIRQNo, &gst_IntSourceSlot[eSlot]);

            // Set irq type IRQ_TYPE_NONE for of_irq_get_byname() works fine next time.
            // Hal_CARD_SetGPIOIntAttr(EV_GPIO_OPT5, p_sdmmc_slot->cdzIRQNo);
            irq_dispose_mapping(p_sdmmc_slot->cdzIRQNo); // for irq_create_fwspec_mapping
        }
    }

    if (gu16_SlotClkEn[p_sdmmc_slot->ipOrder])
    {
        clk_disable_unprepare(gp_clkSlot[p_sdmmc_slot->ipOrder]);
        if (p_sdmmc_slot->p_mmc_priv->u8_supportSD30 || p_sdmmc_slot->p_mmc_priv->u8_supportEMMC50)
            clk_disable_unprepare(gp_clksyncSlot[p_sdmmc_slot->ipOrder]);

        if (!IS_ERR_OR_NULL(gp_clkpmmcuSlot[p_sdmmc_slot->ipOrder]))
            clk_disable_unprepare(gp_clkpmmcuSlot[p_sdmmc_slot->ipOrder]);
        gu16_SlotClkEn[p_sdmmc_slot->ipOrder] = 0;
    }
#if defined(CONFIG_OF)
#ifdef CONFIG_CAM_CLK
    if (gp_clkSlot[p_sdmmc_slot->ipOrder])
    {
        CamClkUnregister(gp_clkSlot[p_sdmmc_slot->ipOrder]);
        gp_clkSlot[p_sdmmc_slot->ipOrder] = NULL;
    }
#else
    if (gp_clkSlot[p_sdmmc_slot->ipOrder])
    {
        clk_put(gp_clkSlot[p_sdmmc_slot->ipOrder]);
        gp_clkSlot[p_sdmmc_slot->ipOrder] = NULL;
        if (p_sdmmc_slot->p_mmc_priv->u8_supportSD30 || p_sdmmc_slot->p_mmc_priv->u8_supportEMMC50)
        {
            clk_put(gp_clksyncSlot[p_sdmmc_slot->ipOrder]);
            gp_clksyncSlot[p_sdmmc_slot->ipOrder] = NULL;
        }
        if (!IS_ERR_OR_NULL(gp_clkpmmcuSlot[p_sdmmc_slot->ipOrder]))
        {
            clk_put(gp_clkpmmcuSlot[p_sdmmc_slot->ipOrder]);
            gp_clkpmmcuSlot[p_sdmmc_slot->ipOrder] = NULL;
        }
    }
#endif
#endif
    mmc_free_host(p_mmc_host);
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: sstar_sdmmc_remove
 *     @author jeremy.wang (2011/5/18)
 * Desc: Revmoe MMC host
 *
 * @param p_dev :  platform device structure
 *
 * @return int  : Error Status; Return 0 if no error.
 ----------------------------------------------------------------------------------------------------------*/
static int sstar_sdmmc_remove(struct platform_device *p_dev)
{
    struct sstar_sdmmc_host *p_sdmmc_host = platform_get_drvdata(p_dev);

    sstar_sdmmc_remove_slot(p_sdmmc_host->sdmmc_slot->slotNo, p_sdmmc_host);
    pr_sd_dbg(">> [sdmmc_%u] Remove devices...\n", p_sdmmc_host->sdmmc_slot->slotNo);
    //
#ifdef CONFIG_SUPPORT_SDMMC_UT_VERIFY
    sysfs_remove_group(&p_sdmmc_host->pdev->dev.kobj, &sstar_sdmmc_ut_attr_grp);
#endif
    sysfs_remove_group(&p_sdmmc_host->pdev->dev.kobj, &sstar_sdmmc_debug_attr_grp);
    platform_set_drvdata(p_dev, NULL);

    kfree(p_sdmmc_host);

    return 0;
}

#if defined(CONFIG_OF)

#ifdef CONFIG_PM_SLEEP
static int sstar_sdmmc_devpm_prepare(struct device *dev)
{
    struct sstar_sdmmc_host *p_sdmmc_host = dev_get_drvdata(dev);
    struct sstar_mmc_priv *  p_mmc_priv   = p_sdmmc_host->sdmmc_slot->p_mmc_priv;
    if (p_mmc_priv->u8_supportRPM)
    {
        if (!(p_sdmmc_host->sdmmc_slot->mmc->caps & (MMC_CAP_NEEDS_POLL | MMC_CAP_NONREMOVABLE)))
        {
            disable_irq(p_sdmmc_host->sdmmc_slot->cdzIRQNo);
        }
    }
    return 0;
}

static void sstar_sdmmc_devpm_complete(struct device *dev)
{
    struct sstar_sdmmc_host *p_sdmmc_host = dev_get_drvdata(dev);
    struct sstar_mmc_priv *  p_mmc_priv   = p_sdmmc_host->sdmmc_slot->p_mmc_priv;

    if (p_mmc_priv->u8_supportRPM)
    {
        cancel_delayed_work_sync(&p_sdmmc_host->sdmmc_slot->mmc->detect);
        p_sdmmc_host->sdmmc_slot->mmc->rescan_disable = 0;
        if (!(p_sdmmc_host->sdmmc_slot->mmc->caps & (MMC_CAP_NEEDS_POLL | MMC_CAP_NONREMOVABLE)))
        {
            enable_irq(p_sdmmc_host->sdmmc_slot->cdzIRQNo);
        }
    }
}

static int sstar_sdmmc_devpm_suspend(struct device *dev)
{
    int                      tret         = 0;
    unsigned int             TmpIPClk     = 0;
    unsigned int             TmpBlockClk  = 0;
    struct sstar_sdmmc_host *p_sdmmc_host = dev_get_drvdata(dev);
    struct sstar_mmc_priv *  p_mmc_priv   = p_sdmmc_host->sdmmc_slot->p_mmc_priv;

#ifdef CONFIG_CAM_CLK
    CamClkSetOnOff(gp_clkSlot[p_mmc_priv->mmc_PMuxInfo.u8_ipOrder], 0);
#else
    if (gu16_SlotClkEn[p_mmc_priv->mmc_PMuxInfo.u8_ipOrder])
    {
        clk_disable_unprepare(gp_clkSlot[p_mmc_priv->mmc_PMuxInfo.u8_ipOrder]);
        if (p_mmc_priv->u8_supportSD30 | p_mmc_priv->u8_supportEMMC50)
            clk_disable_unprepare(gp_clksyncSlot[p_mmc_priv->mmc_PMuxInfo.u8_ipOrder]);
        if (!IS_ERR_OR_NULL(gp_clkpmmcuSlot[p_mmc_priv->mmc_PMuxInfo.u8_ipOrder]))
            clk_disable_unprepare(gp_clkpmmcuSlot[p_mmc_priv->mmc_PMuxInfo.u8_ipOrder]);
        gu16_SlotClkEn[p_mmc_priv->mmc_PMuxInfo.u8_ipOrder] = 0;
    }

#endif
    // backup current clk
    Hal_CARD_devpm_GetClock(p_mmc_priv->mmc_PMuxInfo.u8_ipOrder, &TmpIPClk, &TmpBlockClk);
    gu16_SlotIPClk[p_mmc_priv->u8_slotNo]    = TmpIPClk;
    gu16_SlotBlockClk[p_mmc_priv->u8_slotNo] = TmpBlockClk;

    if (p_mmc_priv->u8_supportRPM)
    {
        p_sdmmc_host->sdmmc_slot->mmc->pm_flags |=
            ((geCardType[p_mmc_priv->mmc_PMuxInfo.u8_ipOrder] == CARD_TYPE_SDIO) ? MMC_PM_KEEP_POWER : 0);
        p_sdmmc_host->sdmmc_slot->strFlag = 2; // suspend
    }
    pr_sd_dbg(">> [mmc_%u] Suspend device pm... \n", p_mmc_priv->u8_slotNo);

    return tret;
}

static int sstar_sdmmc_devpm_resume(struct device *dev)
{
    struct sstar_sdmmc_host *p_sdmmc_host = dev_get_drvdata(dev);
    struct sstar_mmc_priv *  p_mmc_priv   = p_sdmmc_host->sdmmc_slot->p_mmc_priv;
    struct sstar_sdmmc_slot *p_sdmmc_slot = p_sdmmc_host->sdmmc_slot;
#ifdef CONFIG_CAM_CLK
    int tret = 0;
#else
    int ret = 0, tret = 0;
#endif
    GPIOOptEmType eINSOPT = EV_GPIO_OPT3;
    GPIOOptEmType eEJTOPT = EV_GPIO_OPT4;

    if (p_sdmmc_slot->p_mmc_priv->u8_revCdz)
    {
        eINSOPT = EV_GPIO_OPT4;
        eEJTOPT = EV_GPIO_OPT3;
    }

#ifdef CONFIG_CAM_CLK
    CamClkSetOnOff(gp_clkSlot[p_mmc_priv->mmc_PMuxInfo.u8_ipOrder], 1);
#else
    if (!gu16_SlotClkEn[p_mmc_priv->mmc_PMuxInfo.u8_ipOrder])
    {
        ret = clk_prepare_enable(gp_clkSlot[p_mmc_priv->mmc_PMuxInfo.u8_ipOrder]);
        if (p_mmc_priv->u8_supportSD30 | p_mmc_priv->u8_supportEMMC50)
            ret = clk_prepare_enable(gp_clksyncSlot[p_mmc_priv->mmc_PMuxInfo.u8_ipOrder]);
        if (!IS_ERR_OR_NULL(gp_clkpmmcuSlot[p_mmc_priv->mmc_PMuxInfo.u8_ipOrder]))
            clk_prepare_enable(gp_clkpmmcuSlot[p_mmc_priv->mmc_PMuxInfo.u8_ipOrder]);
        gu16_SlotClkEn[p_mmc_priv->mmc_PMuxInfo.u8_ipOrder] = 1;
    }

    if (ret != 0)
        tret = ret;
#endif
    // clock restore
    Hal_CARD_devpm_setClock(p_mmc_priv->mmc_PMuxInfo.u8_ipOrder, gu16_SlotIPClk[p_mmc_priv->u8_slotNo],
                            gu16_SlotBlockClk[p_mmc_priv->u8_slotNo]);

    // pad restore
    sstar_sdmmc_enable(p_mmc_priv);
    //_SwitchPAD(p_mmc_priv);

    if (!(p_sdmmc_host->sdmmc_slot->mmc->caps & (MMC_CAP_NEEDS_POLL | MMC_CAP_NONREMOVABLE)))
    {
        if (_GetCardDetect(p_mmc_priv))
        {
            Hal_CARD_SetGPIOIntAttr(EV_GPIO_OPT1, p_sdmmc_slot->cdzIRQNo);
            Hal_CARD_SetGPIOIntAttr(eINSOPT, p_sdmmc_slot->cdzIRQNo);
        }
        else
        {
            Hal_CARD_SetGPIOIntAttr(EV_GPIO_OPT1, p_sdmmc_slot->cdzIRQNo);
            Hal_CARD_SetGPIOIntAttr(eEJTOPT, p_sdmmc_slot->cdzIRQNo);
        }
    }

    if (p_mmc_priv->u8_supportRPM)
    {
        p_sdmmc_host->sdmmc_slot->mmc->rescan_disable = 1;
        p_sdmmc_slot->strFlag                         = 1; // resume
    }

    pr_sd_dbg(">> [mmc_%u] Resume device pm...(Ret:%u) \n", p_mmc_priv->u8_slotNo, ret);

    return tret;
}

#else

#define sstar_sdmmc_devpm_prepare  NULL
#define sstar_sdmmc_devpm_complete NULL
#define sstar_sdmmc_devpm_suspend  NULL
#define sstar_sdmmc_devpm_resume   NULL

#endif

static int sstar_sdmmc_devpm_runtime_suspend(struct device *dev)
{
    pr_sd_dbg(">> [sdmmc] Runtime Suspend device pm...\n");
    return 0;
}

static int sstar_sdmmc_devpm_runtime_resume(struct device *dev)
{
    struct sstar_sdmmc_host *p_sdmmc_host = dev_get_drvdata(dev);
    struct sstar_mmc_priv *  p_mmc_priv   = p_sdmmc_host->sdmmc_slot->p_mmc_priv;

    pr_sd_dbg(">> [sdmmc] Runtime Resume device pm...\n");
    if (p_mmc_priv->u8_supportRPM)
    {
        if (p_sdmmc_host->sdmmc_slot->strFlag == 1)
        {
            mmc_detect_change(p_sdmmc_host->sdmmc_slot->mmc, 0);
        }
        p_sdmmc_host->sdmmc_slot->strFlag = 0; // normal
    }
    return 0;
}

#define sstar_sdmmc_suspend NULL
#define sstar_sdmmc_resume  NULL

#else

#ifdef CONFIG_PM_SLEEP

#if 0 //( LINUX_VERSION_CODE < KERNEL_VERSION(3, 11, 0) ) // CONFIG_PM

        /*----------------------------------------------------------------------------------------------------------
         *
         * Function: sstar_sdmmc_suspend
         *     @author jeremy.wang (2011/5/18)
         * Desc: Suspend MMC host
         *
         * @param p_dev :   platform device structure
         * @param state :   Power Management Transition State
         *
         * @return int  :   Error Status; Return 0 if no error.
         ----------------------------------------------------------------------------------------------------------*/
        static int sstar_sdmmc_suspend(struct platform_device *p_dev, pm_message_t state)
        {
            struct ms_sdmmc_host *p_sdmmc_host = platform_get_drvdata(p_dev);
            struct mmc_host      *p_mmc_host;
            unsigned int slotNo = 0;
            int ret = 0, tret = 0;


            for(slotNo=0; slotNo<gu8_SlotNums; slotNo++)
            {
                if(gb_RejectSuspend)
                    return -1;

                p_mmc_host = p_sdmmc_host->sdmmc_slot[slotNo]->mmc;

                if (p_mmc_host)
                {

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
                    ret = mmc_suspend_host(p_mmc_host);
#else
                    ret = mmc_suspend_host(p_mmc_host, state);
#endif
                    pr_sd_dbg(">> [sdmmc_%u] Suspend host...(Ret:%u) \n", slotNo, ret);

                    if(ret!=0)
                        tret = ret;
                }

            }

            return tret;
        }

        /*----------------------------------------------------------------------------------------------------------
         *
         * Function: sstar_sdmmc_resume
         *     @author jeremy.wang (2011/5/18)
         * Desc:   Resume MMC host
         *
         * @param p_dev :   platform device structure
         * @return int  :   Error Status; Return 0 if no error.
         ----------------------------------------------------------------------------------------------------------*/
        static int sstar_sdmmc_resume(struct platform_device *p_dev)
        {
            struct ms_sdmmc_host *p_sdmmc_host = platform_get_drvdata(p_dev);
            struct mmc_host      *p_mmc_host;
            unsigned int slotNo = 0;
            int ret = 0, tret = 0;

            for(slotNo=0; slotNo<gu8_SlotNums; slotNo++)
            {
                p_mmc_host = p_sdmmc_host->sdmmc_slot[slotNo]->mmc;
                if (p_mmc_host)
                {
                    ret = mmc_resume_host(p_mmc_host);
                    pr_sd_dbg(">> [sdmmc_%u] Resume host...(Ret:%u) \n", slotNo, ret);
                    if(ret!=0)
                        tret = ret;
                }
            }

            return tret;
        }

#else

static int sstar_sdmmc_suspend(struct platform_device *p_dev, pm_message_t state)
{
    int ret = 0;
    return ret;
}

static int sstar_sdmmc_resume(struct platform_device *p_dev)
{
    int ret = 0;
    return ret;
}

#endif

#else // !CONFIG_PM

// Current driver does not support following two functions, therefore set them to NULL.
#define sstar_sdmmc_suspend NULL
#define sstar_sdmmc_resume  NULL

#endif // End of CONFIG_PM

#endif

/**********************************************************************************************************
 * Define Static Global Structs
 **********************************************************************************************************/

#if defined(CONFIG_OF)
/*----------------------------------------------------------------------------------------------------------
 *  ms_sdmmc_of_match_table
 ----------------------------------------------------------------------------------------------------------*/
static const struct of_device_id sstar_sdmmc_of_match_table[] = {{.compatible = "sstar,sdmmc"}, {}};

/*----------------------------------------------------------------------------------------------------------
 *  ms_sdmmc_dev_pm_ops
 ----------------------------------------------------------------------------------------------------------*/
static struct dev_pm_ops sstar_sdmmc_dev_pm_ops = {
    .suspend         = sstar_sdmmc_devpm_suspend,
    .resume          = sstar_sdmmc_devpm_resume,
    .prepare         = sstar_sdmmc_devpm_prepare,
    .complete        = sstar_sdmmc_devpm_complete,
    .runtime_suspend = sstar_sdmmc_devpm_runtime_suspend,
    .runtime_resume  = sstar_sdmmc_devpm_runtime_resume,
};

#else

/*----------------------------------------------------------------------------------------------------------
 *  st_ms_sdmmc_device
 ----------------------------------------------------------------------------------------------------------*/
static u64 mmc_dmamask = 0xffffffffUL;
static struct platform_device sstar_sdmmc_pltdev = {
    .name = DRIVER_NAME,
    .id = 0,
    .dev =
        {
            .dma_mask = &mmc_dmamask,
            .coherent_dma_mask = 0xffffffffUL,
        },
};

#endif // End of (defined(CONFIG_OF))

/*----------------------------------------------------------------------------------------------------------
 *  st_ms_sdmmc_driver
 ----------------------------------------------------------------------------------------------------------*/
static struct platform_driver sstar_sdmmc_pltdrv = {
    .remove  = sstar_sdmmc_remove, /*__exit_p(ms_sdmmc_remove)*/
    .suspend = sstar_sdmmc_suspend,
    .resume  = sstar_sdmmc_resume,
    .probe   = sstar_sdmmc_probe,
    .driver =
        {
            .name  = DRIVER_NAME,
            .owner = THIS_MODULE,

#if defined(CONFIG_OF)
            .of_match_table = of_match_ptr(sstar_sdmmc_of_match_table),
            .pm             = &sstar_sdmmc_dev_pm_ops,
#endif

        },
};

/**********************************************************************************************************
 * Init & Exit Modules
 **********************************************************************************************************/

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: sstar_mci_init
 *     @author jeremy.wang (2011/7/18)
 * Desc: Linux Module Function for Init
 *
 * @return s32 __init  :  Error Status; Return 0 if no error.
 ----------------------------------------------------------------------------------------------------------*/
static s32 sstar_sdmmc_init(void)
{
    pr_sd_dbg(KERN_INFO ">> [sdmmc] %s Driver Initializing... \n", DRIVER_NAME);

#if !(defined(CONFIG_OF))
    platform_device_register(&sstar_sdmmc_pltdev);
#endif

    return platform_driver_register(&sstar_sdmmc_pltdrv);
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: sstar_sdmmc_exit
 *     @author jeremy.wang (2011/9/8)
 * Desc: Linux Module Function for Exit
 ----------------------------------------------------------------------------------------------------------*/
static void sstar_sdmmc_exit(void)
{
    platform_driver_unregister(&sstar_sdmmc_pltdrv);
}

module_init(sstar_sdmmc_init);
module_exit(sstar_sdmmc_exit);

#ifdef CONFIG_SUPPORT_SDMMC_AT_SMOKE
module_param(sdSpeedMode, charp, 0644);
module_param(emmcSpeedMode, charp, 0644);
module_param(transMode, charp, 0644);
module_param(busWidth, uint, 0644);
#endif

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_AUTHOR("SSTAR");
