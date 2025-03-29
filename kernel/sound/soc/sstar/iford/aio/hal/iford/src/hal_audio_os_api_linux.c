/*
 * hal_audio_os_api_linux.c - Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
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

// todo

//
#include "cam_os_wrapper.h"

//
#include "cam_sysfs.h"
#include "cam_clkgen.h"
#include "drv_gpio_io.h"

//
#include <linux/interrupt.h>
#include <linux/of_irq.h>
#include <linux/gpio.h>
#include <linux/clk.h>

//
#include "gpio.h"
#include "padmux.h"
#include "drv_gpio.h"
#include "drv_padmux.h"
#include "drv_puse.h"

//
#include "ms_platform.h"

// Hal
#include "hal_audio_common.h"
#include "hal_audio_sys.h"
#include "hal_audio_config.h"
#include "hal_audio_types.h"
#include "hal_audio_api.h"
#include "hal_audio_pri_api.h"
#include "hal_audio_os_api.h"

#ifdef CONFIG_CAM_CLK
#include "drv_camclk_Api.h"
#endif

// ------------------------
typedef enum
{
    E_DTS_AMP_GPIO_A_PAD,
    E_DTS_AMP_GPIO_A_EN,
    E_DTS_AMP_GPIO_B_PAD,
    E_DTS_AMP_GPIO_B_EN,
    E_DTS_HP_GPIO_A_PAD,
    E_DTS_HP_GPIO_A_EN,
    E_DTS_HP_GPIO_B_PAD,
    E_DTS_HP_GPIO_B_EN,
    E_DTS_AMP_GPIO_ARRAY_LEN,

} DTS_AMP_GPIO;

// ------------------------
static volatile int g_nInited                            = 0;
static unsigned int g_aioIrqId                           = 0;
static U32          g_aAmpGpio[E_DTS_AMP_GPIO_ARRAY_LEN] = {0};
DTS_CONFIG_VALUE_un dts_cfg_value[DTS_CONFIG_KEY_TOTAL];

static int _HalAudAoAmpInit(DTS_AMP_GPIO index, int puse)
{
    int ret = 0;

#ifdef CONFIG_SSTAR_PADMUX
    if (drv_padmux_active())
    {
        g_aAmpGpio[index]     = drv_padmux_getpad(puse); // MDRV_PUSE_AIO_AMP_PWR_0
        g_aAmpGpio[index + 1] = 1;                       // Active H or L
    }
#endif

    if (g_aAmpGpio[index] == PAD_UNKNOWN)
    {
        ERRMSG("%s[%d] GPIO_PAD[%d], PUSE[%d] dosen't exist!\n", __FUNCTION__, __LINE__, index, puse);
        return AIO_NG;
    }
#if SSTAR_FOR_FPGA_TEST
    ERRMSG("FPGA not support gpio config!\n");
#else
    camdriver_gpio_request(NULL, g_aAmpGpio[index]);
#ifdef CONFIG_SSTAR_PADMUX
    if (!drv_padmux_active())
#endif
    {
        sstar_gpio_pad_val_set(g_aAmpGpio[index], PINMUX_FOR_GPIO_MODE);
    }
#endif
    ret |= HalAudApiAoAmpEnable(FALSE, index / 2); // Disable (Setting output, too)
    if (ret != AIO_OK)
    {
        ERRMSG("%s[%d] GPIO_PAD[%d], PUSE[%d] AMP Enable error!\n", __FUNCTION__, __LINE__, index, puse);
    }
    return ret;
}

void _HalAudReleaseDts(void)
{
    int           i;
    DTS_CONFIG_st dts_config[DTS_CONFIG_KEY_TOTAL] = {FOREACH_DTS_CONFIG};
    for (i = 0; i < DTS_CONFIG_KEY_TOTAL; i++)
    {
        if (dts_config[i].arraySize > 1 && dts_cfg_value[i].values != NULL)
        {
            kfree(dts_cfg_value[i].values);
        }
    }
}

void _HalAudDumpDts(void)
{
    int           i, j;
    DTS_CONFIG_st dts_config[DTS_CONFIG_KEY_TOTAL] = {FOREACH_DTS_CONFIG};

    for (i = 0; i < DTS_CONFIG_KEY_TOTAL; i++)
    {
        if (dts_config[i].arraySize > 1 && dts_cfg_value[i].values != NULL)
        {
            for (j = 0; j < dts_config[i].arraySize; j++)
            {
                printk("name = %s, dts_cfg_value[%d].array[%d] = %d\n", dts_config[i].name, i, j,
                       dts_cfg_value[i].values[j]);
            }
        }
        else
        {
            printk("name = %s, dts_cfg_value[%d].value = %d\n", dts_config[i].name, i, dts_cfg_value[i].value);
        }
    }
}

int _HalAudPraseDts(struct device_node *pDevNode)
{
    int           size, ret, i;
    DTS_CONFIG_st dts_config[DTS_CONFIG_KEY_TOTAL] = {FOREACH_DTS_CONFIG};
    for (i = DTS_CONFIG_KEY_START; i < DTS_CONFIG_KEY_TOTAL; i++)
    {
        size = dts_config[i].arraySize;

        if ((size == 0)
#ifdef CONFIG_SSTAR_PADMUX
            || (drv_padmux_active() && (NULL != dts_config[i].name) && strstr(dts_config[i].name, "padmux"))
#endif
        )
        {
            continue;
        }
        if (size > 1)
        {
            // Array
            dts_cfg_value[i].values = (U32 *)kmalloc(size * sizeof(U32), GFP_KERNEL);
            if (dts_cfg_value[i].values == NULL)
            {
                continue;
            }
            ret = CamOfPropertyReadU32Array(pDevNode, dts_config[i].name, dts_cfg_value[i].values, size);
            if (ret)
            {
                kfree(dts_cfg_value[i].values);
            }
        }
        else
        {
            ret = CamofPropertyReadU32(pDevNode, dts_config[i].name, &dts_cfg_value[i].value);
            if (ret)
            {
                dts_cfg_value[i].value = dts_config[i].defValue;
            }
        }
        // printk("dts_config[%d] = %s, ret = %d\n", i, dts_config[i].name, ret);
        //  if(ret){
        //	release();
        //	return AIO_NG;
        //  }
    }

    //_HalAudDumpDts();

    return AIO_OK;
}
// ------------------------
int HalAudApiDtsInit(void)
{
    int                 ret = 0;
    struct device_node *pDevNode;
    const char *        dtsCompatibleStr = NULL, *dtsAmpGpioStr = NULL;
#ifdef CONFIG_CAM_CLK
    u32   AioClk;
    void *pvAioclk = NULL;
#endif
    // ------------------------
    // Get Pre info.
    // ------------------------
    dtsCompatibleStr = "sstar,audio";
    dtsAmpGpioStr    = "amp-pad";

    if (g_nInited != 0)
    {
        return AIO_OK;
    }
    g_nInited++;

    //
    pDevNode = of_find_compatible_node(NULL, NULL, dtsCompatibleStr);
    if (pDevNode)
    {
        // IRQ
        g_aioIrqId = CamOfIrqToResource(of_node_get(pDevNode), 0, NULL);

        // debug level
        _HalAudPraseDts(pDevNode);

        // AMP GPIO
#ifdef CONFIG_SSTAR_PADMUX
        if (!drv_padmux_active())
#else
        if (1)
#endif
        {
            ret = CamOfPropertyReadU32Array(pDevNode, dtsAmpGpioStr, g_aAmpGpio, E_DTS_AMP_GPIO_ARRAY_LEN);
            if (ret == 0)
            {
                _HalAudAoAmpInit(E_DTS_AMP_GPIO_A_PAD, 0);
                _HalAudAoAmpInit(E_DTS_AMP_GPIO_B_PAD, 0);
                _HalAudAoAmpInit(E_DTS_HP_GPIO_A_PAD, 0);
                _HalAudAoAmpInit(E_DTS_HP_GPIO_B_PAD, 0);
            }
            else
            {
                ERRMSG("Failed to gpio_request amp-pad !\n");
                // goto FAIL;
            }
        }
        else
        {
            _HalAudAoAmpInit(E_DTS_AMP_GPIO_A_PAD, MDRV_PUSE_AIO_AMP_PWR);
            _HalAudAoAmpInit(E_DTS_AMP_GPIO_B_PAD, MDRV_PUSE_AIO_AMP_PWR);
            //_HalAudAoAmpInit(E_DTS_HP_GPIO_A_PAD, MDRV_PUSE_AIO_HP_PWR);
            //_HalAudAoAmpInit(E_DTS_HP_GPIO_B_PAD, MDRV_PUSE_AIO_HP_PWR);
        }

        // I2S TXA
        HalAudI2sSetDriving(3);
        //
        of_node_put(pDevNode);
    }
    else
    {
        ERRMSG("%s, Failed to find device node !\n", __FUNCTION__);
        goto FAIL;
    }

#ifdef CONFIG_CAM_CLK
    AioClk = 0;
    CamOfPropertyReadU32Index(pDevNode, "camclk", 0, &(AioClk));
    if (!AioClk)
    {
        printk(KERN_DEBUG "[%s] Fail to get clk!\n", __func__);
    }
    else
    {
        CamClkRegister("Aio", AioClk, &(pvAioclk));
        CamClkSetOnOff(pvAioclk, 1);
        CamClkUnregister(pvAioclk);
    }
#endif

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudApiAoAmpEnable(BOOL bEnable, S8 s8Ch)
{
    s8Ch *= 2;
    if (s8Ch >= (E_DTS_AMP_GPIO_ARRAY_LEN - 1) || g_aAmpGpio[s8Ch] == PAD_UNKNOWN)
    {
        ERRMSG("s8Ch %d failed\n", s8Ch);
        return AIO_OK;
    }

#if SSTAR_FOR_FPGA_TEST
    ERRMSG("FPGA not support gpio config!\n");
#else
    camdriver_gpio_direction_output(NULL, g_aAmpGpio[s8Ch], bEnable ? g_aAmpGpio[s8Ch + 1] : !(g_aAmpGpio[s8Ch + 1]));
#endif
    DBGMSG(AUDIO_DBG_LEVEL_PATH, "bEnable[%d] s8Ch[%d]", bEnable, s8Ch);

    return AIO_OK;
}

int HalAudApiAoAmpStateGet(int *bEnable, U8 s8Ch)
{
    s8Ch *= 2;
    if (s8Ch >= (E_DTS_AMP_GPIO_ARRAY_LEN - 1) || g_aAmpGpio[s8Ch] == PAD_UNKNOWN)
    {
        ERRMSG("s8Ch %d failed\n", s8Ch);
        return AIO_OK;
    }
#if SSTAR_FOR_FPGA_TEST
    ERRMSG("FPGA not support gpio config!\n");
    *bEnable = FALSE;
#else
    *bEnable = (camdriver_gpio_get(NULL, g_aAmpGpio[s8Ch]) == g_aAmpGpio[s8Ch + 1]) ? TRUE : FALSE;
#endif

    return AIO_OK;
}

int HalAudApiGetIrqId(unsigned int *pIrqId)
{
    *pIrqId = g_aioIrqId;

    return AIO_OK;
}
