/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

//#include <linux/module.h>
//#include <linux/interrupt.h>
//#include <linux/of_irq.h>
//#include <linux/gpio.h>
//#include <linux/clk.h>
//#include <linux/proc_fs.h>
//#include <linux/seq_file.h>
//#include <linux/uaccess.h>
//#include <asm/uaccess.h>
//#include <linux/mutex.h>
//#include <linux/of_platform.h>
//#include <linux/kernel.h>


#include "cam_clkgen.h"
#include "cam_sysfs.h"
#include "mi_common_internal.h"
#include "aio_init.h"
#include "drv_clock.h"

struct MHAL_AUDIO_InternalApi_t;

struct MHAL_AUDIO_InternalApi_t * MHAL_AUDIO_GetInternalApi(void);

MI_S32 _GetU32InfoFromDtsByType(InfoType_e eType, MI_U32 *pu32Value)
{
    MI_S32              s32Ret = AIO_INIT_OK;
    struct device_node *pDevNode = NULL;
    MI_U32              u32Value = 0;
    char                *pKey = NULL;

    if (!pu32Value)
    {
        CamOsPrintf("[%s:%d] null pointer.pu32Value[%px].\n", __func__, __LINE__, pu32Value);
        return E_MI_ERR_NULL_PTR;
    }

    pDevNode = of_find_compatible_node(NULL, NULL, "sstar,audio");
    if (pDevNode)
    {
        switch (eType)
        {
            case E_INFO_TYPE_IRQ_NUM:
            {
                *pu32Value = CamOfIrqToResource(of_node_get(pDevNode), 0, NULL);
            }
            break;

            case E_INFO_TYPE_I2S_RX0_TDM_WS_PGM ... E_INFO_TYPE_ADC_CH_INMUX:
            {
                if (E_INFO_TYPE_I2S_RX0_TDM_WS_PGM == eType)
                {
                    pKey = "i2s-rx0-tdm-ws-pgm";
                }
                else if (E_INFO_TYPE_I2S_RX0_TDM_WS_WIDTH == eType)
                {
                    pKey = "i2s-rx0-tdm-ws-width";
                }
                else if (E_INFO_TYPE_I2S_RX0_TDM_WS_INV == eType)
                {
                    pKey = "i2s-rx0-tdm-ws-inv";
                }
                else if (E_INFO_TYPE_I2S_RX0_TDM_BCK_INV == eType)
                {
                    pKey = "i2s-rx0-tdm-bck-inv";
                }
                else if (E_INFO_TYPE_I2S_TX0_TDM_WS_PGM == eType)
                {
                    pKey = "i2s-tx0-tdm-ws-pgm";
                }
                else if (E_INFO_TYPE_I2S_TX0_TDM_WS_WIDTH == eType)
                {
                    pKey = "i2s-tx0-tdm-ws-width";
                }
                else if (E_INFO_TYPE_I2S_TX0_TDM_WS_INV == eType)
                {
                    pKey = "i2s-tx0-tdm-ws-inv";
                }
                else if (E_INFO_TYPE_I2S_TX0_TDM_BCK_INV == eType)
                {
                    pKey = "i2s-tx0-tdm-bck-inv";
                }
                else if (E_INFO_TYPE_I2S_TX0_TDM_ACTIVE_SLOT == eType)
                {
                    pKey = "i2s-tx0-tdm-active-slot";
                }
                else if (E_INFO_TYPE_DMIC_BCK_EXT_MODE == eType)
                {
                    pKey = "dmic-bck-ext-mode";
                }
                else if (E_INFO_TYPE_KEEP_ADC_POWER_ON == eType)
                {
                    pKey = "keep_adc_power_on";
                }
                else if (E_INFO_TYPE_KEEP_DAC_POWER_ON == eType)
                {
                    pKey = "keep_dac_power_on";
                }
                else if (E_INFO_TYPE_I2S_RX_MODE == eType)
                {
                    pKey = "i2s-rx-mode";
                }
                else if (E_INFO_TYPE_I2S_RX_SHORT_FF_MODE == eType)
                {
                    pKey = "i2s-rx-short-ff-mode";
                }
                else if (E_INFO_TYPE_I2S_TX_SHORT_FF_MODE == eType)
                {
                    pKey = "i2s-tx-short-ff-mode";
                }
                else if (E_INFO_TYPE_DMIC_DELAY == eType)
                {
                    pKey = "dmic-delay";
                }
                else if (E_INFO_TYPE_ADC_CH_INMUX == eType)
                {
                    pKey = "adc-ch-inmux";
                }

                s32Ret = CamofPropertyReadU32(pDevNode, pKey, &u32Value);
                if (s32Ret == 0)
                {
                    *pu32Value = u32Value;
                }
                else
                {
                    CamOsPrintf("[%s:%d] Failed to get %s from dts!\n", __func__, __LINE__, pKey);
                    s32Ret = AIO_INIT_NG;
                }
            }
            break;

            default:
                CamOsPrintf("[%s:%d] Unknown info type %d!\n", __func__, __LINE__, eType);
                s32Ret = AIO_INIT_NG;
                break;
        }

        of_node_put(pDevNode);
    }
    else
    {
        CamOsPrintf("[%s:%d] null pointer.pDevNode[%px].\n", __func__, __LINE__, pDevNode);
        s32Ret = AIO_INIT_NG;
    }

    return s32Ret;
}

MI_S32 _GetArrayInfoFromDtsByType(InfoType_e eType, MI_U32 *pau32Array, MI_U32 u32ArrayItems)
{
    MI_S32              s32Ret = AIO_INIT_OK;
    struct device_node *pDevNode;
    char               *pKey = NULL;

    if (!pau32Array || 0 == u32ArrayItems)
    {
        CamOsPrintf("[%s:%d] null pointer or array size is 0. pau32Array[%px] u32ArrayItems[%d]\n",
            __func__, __LINE__, pau32Array, u32ArrayItems);
        return AIO_INIT_NG;
    }

    pDevNode = of_find_compatible_node(NULL, NULL, "sstar,audio");
    if (pDevNode)
    {
        switch (eType)
        {
            case E_INFO_TYPE_AMP_GPIO:
            {
                pKey = "amp-pad";
            }
            break;

            case E_INFO_TYPE_I2S_RX0_TDM_CH_SWAP:
            {
                pKey = "i2s-rx0-tdm-ch-swap";
            }
            break;

            case E_INFO_TYPE_I2S_TX0_TDM_CH_SWAP:
            {
                pKey = "i2s-tx0-tdm-ch-swap";
            }
            break;

            case E_INFO_TYPE_HPF_ADC1_LEVEL:
            {
                pKey = "hpf-adc1-level";
            }
            break;

            case E_INFO_TYPE_HPF_ADC2_LEVEL:
            {
                pKey = "hpf-adc2-level";
            }
            break;

            case E_INFO_TYPE_HPF_DMIC_LEVEL:
            {
                pKey = "hpf-dmic-level";
            }
            break;

            case E_INFO_TYPE_DMIC_BCK_MODE:
            {
                pKey = "dmic-bck-mode";
            }
            break;

            case E_INFO_TYPE_ADC_OUT_SEL:
            {
                pKey = "adc-out-sel";
            }
            break;

            default:
            return AIO_INIT_NG;
        }

        s32Ret = CamOfPropertyReadU32Array(pDevNode, pKey, pau32Array, u32ArrayItems);
        if (s32Ret)
        {
            CamOsPrintf("[%s:%d] Failed to get %s from dts.\n", __func__, __LINE__, pKey);
            s32Ret = AIO_INIT_NG;
        }
        of_node_put(pDevNode);
    }
    else
    {
        CamOsPrintf("[%s:%d] null pointer.pDevNode[%px].\n", __func__, __LINE__, pDevNode);
        s32Ret = AIO_INIT_NG;
    }

    return s32Ret;
}

MI_S32 AudClkEnable(MI_S8 clk_index, MI_S8 parent_clk_index, MI_BOOL bEn, MI_S64 nFreq)
{
    struct device_node *pDevNode;
    void *              parent_clk = NULL;
    struct clk *        clk;
    const char *        dtsCompatibleStr = NULL;

    dtsCompatibleStr = "sstar,audio";

    pDevNode = of_find_compatible_node(NULL, NULL, dtsCompatibleStr);

    if (!pDevNode)
    {
        CamOsPrintf("Get audio pDevNode fail!");
        return AIO_INIT_NG;
    }

    clk = of_clk_get(pDevNode, clk_index);
    if (IS_ERR(clk))
    {
        CamOsPrintf("Get audio clock[%d] fail!", clk_index);
        of_node_put(pDevNode);
        return AIO_INIT_NG;
    }

    if (bEn)
    {
        if (parent_clk_index >= 0)
        {
            parent_clk = sstar_clk_get_parent_by_index(clk, parent_clk_index);
        }
        if (parent_clk != NULL)
        {
            sstar_clk_set_parent(clk, parent_clk);
        }
        sstar_clk_prepare_enable(clk);
        if (nFreq >= 0)
        {
            sstar_clk_set_rate(parent_clk, nFreq);
        }
    }
    else
    {
        if (sstar_clk_is_enabled(clk))
        {
            if (parent_clk_index >= 0)
            {
                parent_clk = sstar_clk_get_parent_by_index(clk, parent_clk_index);
            }

            if (parent_clk != NULL)
            {
                sstar_clk_set_parent(clk, parent_clk);
                if (nFreq >= 0)
                {
                    sstar_clk_set_rate(parent_clk, nFreq);
                }
            }
            sstar_clk_disable_unprepare(clk);
        }
    }

    of_node_put(pDevNode);
    sstar_clk_put(clk);
    return AIO_INIT_OK;
}

void MHAL_AUDIO_Module_Init(void);
void MHAL_AUDIO_Module_DeInit(void);

static int mi_aio_insmod(void)
{
    CamOsPrintf("module [%s] init\n", "aio");
    MHAL_AUDIO_Module_Init();
    return 0;
}

static void mi_aio_rmmod(void)
{
    CamOsPrintf("module [%s] deinit\n", "aio");
    MHAL_AUDIO_Module_DeInit();
}

DECLEAR_MODULE_INIT_EXIT_EXTRA(mi_aio_insmod, mi_aio_rmmod);

#ifndef COMBINE_MI_MODULE
EXPORT_SYMBOL(MHAL_AUDIO_GetInternalApi);
#endif

MI_MODULE_LICENSE();
MODULE_AUTHOR("Sigmastar");
