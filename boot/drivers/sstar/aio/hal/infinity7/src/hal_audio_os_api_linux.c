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
#include "hal_audio_os_api.h"
#include "hal_audio_pri_api.h"

#ifdef CONFIG_CAM_CLK
#include "drv_camclk_Api.h"
#endif

// todo
#include "padmux.h"
#include "drv_padmux.h"
#include "drv_puse.h"
#include "hal_audio_common.h"
#include "hal_audio_pri_api.h"

// ------------------------
typedef enum
{
    E_DTS_AMP_GPIO_A_PAD,
    E_DTS_AMP_GPIO_A_EN,
    E_DTS_AMP_GPIO_B_PAD,
    E_DTS_AMP_GPIO_B_EN,
    E_DTS_AMP_GPIO_ARRAY_LEN,

} DTS_AMP_GPIO;

// ------------------------
static unsigned int g_aioIrqId                           = 0;
static U32          g_aAmpGpio[E_DTS_AMP_GPIO_ARRAY_LEN] = {0};

// ------------------------
int HalAudApiDtsInit(void)
{
    int                 ret;
    struct device_node *pDevNode;
    //    S32 nPadMuxMode;
    const char *dtsCompatibleStr = NULL, *dtsAmpGpioStr = NULL, *dtsKeepI2sClkStr = NULL;
    U32         nI2sRxMode          = 0;
    U32         nI2sRxTdmWsPgm      = FALSE;
    U32         nI2sRxTdmWsWidth    = 0;
    U32         nI2sRxTdmWsInv      = 0;
    U32         nI2sRxTdmBckInv     = 0;
    U32         aI2sRxTdmChSwap[3]  = {0};
    U32         nI2sTxTdmWsPgm      = FALSE;
    U32         nI2sTxTdmWsWidth    = 0;
    U32         nI2sTxTdmWsInv      = 0;
    U32         nI2sTxTdmBckInv     = 0;
    U32         aI2sTxTdmChSwap[3]  = {0};
    U32         nI2sTxTdmActiveSlot = 0;
    U32         aHpfStatus[2]       = {0};
    U32         nDmicBckMode8K      = 7;
    U32         nDmicBckMode16K     = 12;
    U32         nDmicBckMode32K     = 15;
    U32         nDmicBckMode48K     = 16;
    U32         u8IndexGPIO         = 0XFFFF;
    // U32 nKeepAdcPowerOn = 0;
    // U32 nKeepDacPowerOn = 0;
#ifdef CONFIG_CAM_CLK
    u32   AioClk;
    void *pvAioclk = NULL;
#else
    struct clk *clk;
#endif

    // ------------------------
    // Get Pre info.
    // ------------------------
    dtsCompatibleStr = "sstar,audio";
    dtsAmpGpioStr    = "amp-pad";
    dtsKeepI2sClkStr = "keep-i2s-clk";

    //
    pDevNode = of_find_compatible_node(NULL, NULL, dtsCompatibleStr);
    if (pDevNode)
    {
        // IRQ
        g_aioIrqId = CamOfIrqToResource(of_node_get(pDevNode), 0, NULL);
        // AMP GPIO
        if (drv_padmux_active())
        {
            ret = CamOfPropertyReadU32Array(pDevNode, dtsAmpGpioStr, g_aAmpGpio, E_DTS_AMP_GPIO_ARRAY_LEN);
            if (ret == 0)
            {
                if (g_aAmpGpio[E_DTS_AMP_GPIO_A_PAD] != PAD_UNKNOWN)
                {
                    sstar_gpio_pad_val_set(g_aAmpGpio[E_DTS_AMP_GPIO_A_PAD], PINMUX_FOR_GPIO_MODE);

                    ret |= HalAudApiAoAmpEnable(FALSE, 0); // Disable (Setting output, too)
                    if (ret != AIO_OK)
                    {
                        ERRMSG("%s[%d], Failed to HalAudApiAoAmpEnable!\n", __FUNCTION__, __LINE__);
                        goto FAIL;
                    }
                }

                if (g_aAmpGpio[E_DTS_AMP_GPIO_B_PAD] != PAD_UNKNOWN)
                {
                    sstar_gpio_pad_val_set(g_aAmpGpio[E_DTS_AMP_GPIO_B_PAD], PINMUX_FOR_GPIO_MODE);

                    ret |= HalAudApiAoAmpEnable(FALSE, 1); // Disable (Setting output, too)
                    if (ret != AIO_OK)
                    {
                        ERRMSG("%s[%d], Failed to HalAudApiAoAmpEnable!\n", __FUNCTION__, __LINE__);
                        goto FAIL;
                    }
                }
            }
            else
            {
                ERRMSG("%s[%d], Failed to gpio_request amp-gpio !\n", __FUNCTION__, __LINE__);
                goto FAIL;
            }
        }
        else
        {
            g_aAmpGpio[E_DTS_AMP_GPIO_A_PAD] = drv_padmux_getpad(MDRV_PUSE_AIO_AMP_PWR); // MDRV_PUSE_AIO_AMP_PWR_0
            g_aAmpGpio[E_DTS_AMP_GPIO_A_EN]  = 1;                                        // Active H or L

            ret |= HalAudApiAoAmpEnable(FALSE, 0); // Disable (Setting output, too)
            if (ret != AIO_OK)
            {
                ERRMSG("%s[%d], Failed to HalAudApiAoAmpEnable!\n", __FUNCTION__, __LINE__);
                goto FAIL;
            }

            g_aAmpGpio[E_DTS_AMP_GPIO_B_PAD] = drv_padmux_getpad(MDRV_PUSE_AIO_AMP_PWR); // MDRV_PUSE_AIO_AMP_PWR_1
            g_aAmpGpio[E_DTS_AMP_GPIO_B_EN]  = 1;                                        // Active H or L

            ret |= HalAudApiAoAmpEnable(FALSE, 1); // Disable (Setting output, too)
            if (ret != AIO_OK)
            {
                ERRMSG("%s[%d], Failed to HalAudApiAoAmpEnable!\n", __FUNCTION__, __LINE__);
                goto FAIL;
            }
        }

        // Add IO driving
        // MCLK0
        u8IndexGPIO = drv_padmux_getpad(MDRV_PUSE_I2S0_MCK);
        if (u8IndexGPIO != 0XFFFF)
        {
            sstar_gpio_drv_set(u8IndexGPIO, 2);
        }

        // MCLK1
        u8IndexGPIO = drv_padmux_getpad(MDRV_PUSE_I2S1_MCK);
        if (u8IndexGPIO != 0XFFFF)
        {
            sstar_gpio_drv_set(u8IndexGPIO, 2);
        }

        // Dmic CLK
        u8IndexGPIO = drv_padmux_getpad(MDRV_PUSE_DMIC1_CLK);
        if (u8IndexGPIO != 0XFFFF)
        {
            sstar_gpio_drv_set(u8IndexGPIO, 4);
        }

        // I2S TXA
        u8IndexGPIO = drv_padmux_getpad(MDRV_PUSE_I2S0_TX_BCK);
        if (u8IndexGPIO != 0XFFFF)
        {
            sstar_gpio_drv_set(u8IndexGPIO, 10);
        }
        u8IndexGPIO = drv_padmux_getpad(MDRV_PUSE_I2S0_TX_WCK);
        if (u8IndexGPIO != 0XFFFF)
        {
            sstar_gpio_drv_set(u8IndexGPIO, 10);
        }
        u8IndexGPIO = drv_padmux_getpad(MDRV_PUSE_I2S0_TX_SDO);
        if (u8IndexGPIO != 0XFFFF)
        {
            sstar_gpio_drv_set(u8IndexGPIO, 10);
        }

        // I2S TXB
        u8IndexGPIO = drv_padmux_getpad(MDRV_PUSE_I2S1_TX_BCK);
        if (u8IndexGPIO != 0XFFFF)
        {
            sstar_gpio_drv_set(u8IndexGPIO, 10);
        }
        u8IndexGPIO = drv_padmux_getpad(MDRV_PUSE_I2S1_TX_WCK);
        if (u8IndexGPIO != 0XFFFF)
        {
            sstar_gpio_drv_set(u8IndexGPIO, 10);
        }
        u8IndexGPIO = drv_padmux_getpad(MDRV_PUSE_I2S1_TX_SDO);
        if (u8IndexGPIO != 0XFFFF)
        {
            sstar_gpio_drv_set(u8IndexGPIO, 10);
        }

        // I2S RXA
        u8IndexGPIO = drv_padmux_getpad(MDRV_PUSE_I2S0_RX_BCK);
        if (u8IndexGPIO != 0XFFFF)
        {
            sstar_gpio_drv_set(u8IndexGPIO, 10);
        }
        u8IndexGPIO = drv_padmux_getpad(MDRV_PUSE_I2S0_RX_WCK);
        if (u8IndexGPIO != 0XFFFF)
        {
            sstar_gpio_drv_set(u8IndexGPIO, 10);
        }
        u8IndexGPIO = drv_padmux_getpad(MDRV_PUSE_I2S0_RX_SDI);
        if (u8IndexGPIO != 0XFFFF)
        {
            sstar_gpio_drv_set(u8IndexGPIO, 10);
        }

        // I2S RXB
        u8IndexGPIO = drv_padmux_getpad(MDRV_PUSE_I2S1_RX_BCK);
        if (u8IndexGPIO != 0XFFFF)
        {
            sstar_gpio_drv_set(u8IndexGPIO, 10);
        }
        u8IndexGPIO = drv_padmux_getpad(MDRV_PUSE_I2S1_RX_WCK);
        if (u8IndexGPIO != 0XFFFF)
        {
            sstar_gpio_drv_set(u8IndexGPIO, 10);
        }
        u8IndexGPIO = drv_padmux_getpad(MDRV_PUSE_I2S1_RX_SDI);
        if (u8IndexGPIO != 0XFFFF)
        {
            sstar_gpio_drv_set(u8IndexGPIO, 10);
        }

        // I2S RXC
        u8IndexGPIO = drv_padmux_getpad(MDRV_PUSE_I2S2_RX_BCK);
        if (u8IndexGPIO != 0XFFFF)
        {
            sstar_gpio_drv_set(u8IndexGPIO, 10);
        }
        u8IndexGPIO = drv_padmux_getpad(MDRV_PUSE_I2S2_RX_WCK);
        if (u8IndexGPIO != 0XFFFF)
        {
            sstar_gpio_drv_set(u8IndexGPIO, 10);
        }
        u8IndexGPIO = drv_padmux_getpad(MDRV_PUSE_I2S2_RX_SDI);
        if (u8IndexGPIO != 0XFFFF)
        {
            sstar_gpio_drv_set(u8IndexGPIO, 10);
        }

        // I2S RXD
        u8IndexGPIO = drv_padmux_getpad(MDRV_PUSE_I2S3_RX_BCK);
        if (u8IndexGPIO != 0XFFFF)
        {
            sstar_gpio_drv_set(u8IndexGPIO, 10);
        }
        u8IndexGPIO = drv_padmux_getpad(MDRV_PUSE_I2S3_RX_WCK);
        if (u8IndexGPIO != 0XFFFF)
        {
            sstar_gpio_drv_set(u8IndexGPIO, 10);
        }
        u8IndexGPIO = drv_padmux_getpad(MDRV_PUSE_I2S3_RX_SDI);
        if (u8IndexGPIO != 0XFFFF)
        {
            sstar_gpio_drv_set(u8IndexGPIO, 10);
        }
        //
        /*
                // PAD MUX
                if (!drv_padmux_active())
                {
                    ret = CamofPropertyReadU32(pDevNode, "i2s-tx-padmux", &nPadMuxMode);
                    if (ret == 0)
                    {
                        if (nPadMuxMode != 0)
                        {
                            if (nPadMuxMode <= (PINMUX_FOR_I2S_TX_MODE_10 - PINMUX_FOR_I2S_TX_MODE_1 + 1))
                            {
                                sstar_gpio_padgroupmode_set(nPadMuxMode - 1 + PINMUX_FOR_I2S_TX_MODE_1);
                            }
                            else
                            {
                                ERRMSG("%s[%d], I2S Tx PadMuxMode Invalid!\n",__FUNCTION__, __LINE__);
                                goto FAIL;
                            }
                        }
                    }
                    else
                    {
                        ERRMSG("%s[%d], Dts Read I2S Tx padmux fail!\n",__FUNCTION__, __LINE__);
                        goto FAIL;
                    }

                    ret = CamofPropertyReadU32(pDevNode, "i2s-rx-padmux", &nPadMuxMode);
                    if (ret == 0)
                    {
                        if (nPadMuxMode != 0)
                        {
                            if (nPadMuxMode <= (PINMUX_FOR_I2S_RX_MODE_11 - PINMUX_FOR_I2S_RX_MODE_1 + 1))
                            {
                                sstar_gpio_padgroupmode_set(nPadMuxMode - 1 + PINMUX_FOR_I2S_RX_MODE_1);
                            }
                            else
                            {
                                ERRMSG("%s[%d], I2S Rx PadMuxMode Invalid!\n",__FUNCTION__, __LINE__);
                                goto FAIL;
                            }
                        }
                    }
                    else
                    {
                        ERRMSG("%s[%d], Dts Read I2S Rx padmux fail!\n",__FUNCTION__, __LINE__);
                        goto FAIL;
                    }

                    ret = CamofPropertyReadU32(pDevNode, "i2s-rxtx-padmux", &nPadMuxMode);
                    if (ret == 0)
                    {
                        if (nPadMuxMode != 0)
                        {
                            if (nPadMuxMode <= (PINMUX_FOR_I2S_RXTX_MODE_7 - PINMUX_FOR_I2S_RXTX_MODE_1 + 1))
                            {
                                sstar_gpio_padgroupmode_set(nPadMuxMode - 1 + PINMUX_FOR_I2S_RXTX_MODE_1);
                            }
                            else
                            {
                                ERRMSG("%s[%d], I2S TRX PadMuxMode Invalid!\n",__FUNCTION__, __LINE__);
                                goto FAIL;
                            }
                        }
                    }
                    else
                    {
                        ERRMSG("%s[%d], Dts Read I2S TRX padmux fail!\n",__FUNCTION__, __LINE__);
                        goto FAIL;
                    }

                    ret = CamofPropertyReadU32(pDevNode, "i2s-mck-padmux", &nPadMuxMode);
                    if (ret == 0)
                    {
                        if (nPadMuxMode != 0)
                        {
                            if (nPadMuxMode <= (PINMUX_FOR_I2S_MCK_MODE_7 - PINMUX_FOR_I2S_MCK_MODE_1 + 1))
                            {
                                sstar_gpio_padgroupmode_set(nPadMuxMode - 1 + PINMUX_FOR_I2S_MCK_MODE_1);
                            }
                            else
                            {
                                ERRMSG("%s[%d], I2S MCK PadMuxMode Invalid!\n",__FUNCTION__, __LINE__);
                                goto FAIL;
                            }
                        }
                    }
                    else
                    {
                        ERRMSG("%s[%d], Dts Read I2S MCK padmux fail!\n",__FUNCTION__, __LINE__);
                        goto FAIL;
                    }

                    ret = CamofPropertyReadU32(pDevNode, "digmic-padmux", &nPadMuxMode);
                    if (ret == 0)
                    {
                        if (nPadMuxMode != 0)
                        {
                            if (nPadMuxMode <= (PINMUX_FOR_DMIC_MODE_9 - PINMUX_FOR_DMIC_MODE_1 + 1))
                            {
                                sstar_gpio_padgroupmode_set(nPadMuxMode - 1 + PINMUX_FOR_DMIC_MODE_1);
                            }
                            else
                            {
                                ERRMSG("%s[%d], I2S DMIC PadMuxMode Invalid!\n",__FUNCTION__, __LINE__);
                                goto FAIL;
                            }
                        }
                    }
                    else
                    {
                        ERRMSG("%s[%d], Dts Read DMIC padmux fail!\n",__FUNCTION__, __LINE__);
                        goto FAIL;
                    }

                    ret = CamofPropertyReadU32(pDevNode, "digmic-4ch-padmux", &nPadMuxMode);
                    if (ret == 0)
                    {
                        if (nPadMuxMode != 0)
                        {
                            if (nPadMuxMode <= (PINMUX_FOR_DMIC_4CH_MODE_10 - PINMUX_FOR_DMIC_4CH_MODE_1 + 1))
                            {
                                sstar_gpio_padgroupmode_set(nPadMuxMode - 1 + PINMUX_FOR_DMIC_4CH_MODE_1);
                            }
                            else
                            {
                                ERRMSG("%s[%d], I2S DMIC_4CH PadMuxMode Invalid!\n",__FUNCTION__, __LINE__);
                                goto FAIL;
                            }
                        }
                    }
                    else
                    {
                        ERRMSG("%s[%d], Dts Read DMIC padmux fail!\n",__FUNCTION__, __LINE__);
                        goto FAIL;
                    }
                }
        */
        ret = CamofPropertyReadU32(pDevNode, "i2s-rx-mode", &nI2sRxMode);
        if (ret == 0)
        {
            ret |= HalAudI2sSetRxMode(nI2sRxMode);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-rx-mode fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        // I2S RX0 TDM
        ret = CamofPropertyReadU32(pDevNode, "i2s-rx0-tdm-ws-pgm", &nI2sRxTdmWsPgm);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmWsPgm(E_AUDIO_TDM_RXA, nI2sRxTdmWsPgm);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-rx0-tdm-ws-pgm fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }
        ret = CamofPropertyReadU32(pDevNode, "i2s-rx0-tdm-ws-width", &nI2sRxTdmWsWidth);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmWsWidth(E_AUDIO_TDM_RXA, (U8)nI2sRxTdmWsWidth);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-rx0-tdm-ws-width fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }
        ret = CamofPropertyReadU32(pDevNode, "i2s-rx0-tdm-ws-inv", &nI2sRxTdmWsInv);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmWsInv(E_AUDIO_TDM_RXA, nI2sRxTdmWsInv);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-rx0-tdm-ws-inv fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }
        ret = CamofPropertyReadU32(pDevNode, "i2s-rx0-tdm-bck-inv", &nI2sRxTdmBckInv);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmBckInv(E_AUDIO_TDM_RXA, nI2sRxTdmBckInv);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-rx0-tdm-bck-inv fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }
        ret = CamOfPropertyReadU32Array(pDevNode, "i2s-rx0-tdm-ch-swap", &aI2sRxTdmChSwap[0], 3);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmChSwap(E_AUDIO_TDM_RXA, (U8)aI2sRxTdmChSwap[1], (U8)aI2sRxTdmChSwap[0],
                                          (U8)aI2sRxTdmChSwap[2]);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-rx0-tdm-ch-swap fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        // I2S RX1 TDM
        ret = CamofPropertyReadU32(pDevNode, "i2s-rx1-tdm-ws-pgm", &nI2sRxTdmWsPgm);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmWsPgm(E_AUDIO_TDM_RXB, nI2sRxTdmWsPgm);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-rx1-tdm-ws-pgm fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        ret = CamofPropertyReadU32(pDevNode, "i2s-rx1-tdm-ws-width", &nI2sRxTdmWsWidth);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmWsWidth(E_AUDIO_TDM_RXB, (U8)nI2sRxTdmWsWidth);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-rx1-tdm-ws-width fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        ret = CamofPropertyReadU32(pDevNode, "i2s-rx1-tdm-ws-inv", &nI2sRxTdmWsInv);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmWsInv(E_AUDIO_TDM_RXB, nI2sRxTdmWsInv);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-rx1-tdm-ws-inv fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        ret = CamofPropertyReadU32(pDevNode, "i2s-rx1-tdm-bck-inv", &nI2sRxTdmBckInv);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmBckInv(E_AUDIO_TDM_RXA, nI2sRxTdmBckInv);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-rx1-tdm-bck-inv fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        ret = CamOfPropertyReadU32Array(pDevNode, "i2s-rx1-tdm-ch-swap", &aI2sRxTdmChSwap[0], 3);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmChSwap(E_AUDIO_TDM_RXB, (U8)aI2sRxTdmChSwap[1], (U8)aI2sRxTdmChSwap[0],
                                          (U8)aI2sRxTdmChSwap[2]);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-rx1-tdm-ch-swap fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        // I2S RX2 TDM
        ret = CamofPropertyReadU32(pDevNode, "i2s-rx2-tdm-ws-pgm", &nI2sRxTdmWsPgm);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmWsPgm(E_AUDIO_TDM_RXC, nI2sRxTdmWsPgm);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-rx2-tdm-ws-pgm fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        ret = CamofPropertyReadU32(pDevNode, "i2s-rx2-tdm-ws-width", &nI2sRxTdmWsWidth);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmWsWidth(E_AUDIO_TDM_RXC, (U8)nI2sRxTdmWsWidth);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-rx2-tdm-ws-width fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        ret = CamofPropertyReadU32(pDevNode, "i2s-rx2-tdm-ws-inv", &nI2sRxTdmWsInv);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmWsInv(E_AUDIO_TDM_RXC, nI2sRxTdmWsInv);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-rx2-tdm-ws-inv fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        ret = CamofPropertyReadU32(pDevNode, "i2s-rx2-tdm-bck-inv", &nI2sRxTdmBckInv);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmBckInv(E_AUDIO_TDM_RXC, nI2sRxTdmBckInv);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-rx2-tdm-bck-inv fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        ret = CamOfPropertyReadU32Array(pDevNode, "i2s-rx2-tdm-ch-swap", &aI2sRxTdmChSwap[0], 3);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmChSwap(E_AUDIO_TDM_RXC, (U8)aI2sRxTdmChSwap[1], (U8)aI2sRxTdmChSwap[0],
                                          (U8)aI2sRxTdmChSwap[2]);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-rx2-tdm-ch-swap fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        // I2S RX3 TDM
        ret = CamofPropertyReadU32(pDevNode, "i2s-rx3-tdm-ws-pgm", &nI2sRxTdmWsPgm);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmWsPgm(E_AUDIO_TDM_RXD, nI2sRxTdmWsPgm);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-rx3-tdm-ws-pgm fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        ret = CamofPropertyReadU32(pDevNode, "i2s-rx3-tdm-ws-width", &nI2sRxTdmWsWidth);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmWsWidth(E_AUDIO_TDM_RXD, (U8)nI2sRxTdmWsWidth);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-rx3-tdm-ws-width fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        ret = CamofPropertyReadU32(pDevNode, "i2s-rx3-tdm-ws-inv", &nI2sRxTdmWsInv);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmWsInv(E_AUDIO_TDM_RXD, nI2sRxTdmWsInv);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-rx3-tdm-ws-inv fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        ret = CamofPropertyReadU32(pDevNode, "i2s-rx3-tdm-bck-inv", &nI2sRxTdmBckInv);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmBckInv(E_AUDIO_TDM_RXD, nI2sRxTdmBckInv);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-rx3-tdm-bck-inv fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        ret = CamOfPropertyReadU32Array(pDevNode, "i2s-rx3-tdm-ch-swap", &aI2sRxTdmChSwap[0], 3);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmChSwap(E_AUDIO_TDM_RXD, (U8)aI2sRxTdmChSwap[1], (U8)aI2sRxTdmChSwap[0],
                                          (U8)aI2sRxTdmChSwap[2]);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-rx3-tdm-ch-swap fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        // I2S TX0 TDM
        ret = CamofPropertyReadU32(pDevNode, "i2s-tx0-tdm-ws-pgm", &nI2sTxTdmWsPgm);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmWsPgm(E_AUDIO_TDM_TXA, nI2sTxTdmWsPgm);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-tx0-tdm-ws-pgm fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        ret = CamofPropertyReadU32(pDevNode, "i2s-tx0-tdm-ws-width", &nI2sTxTdmWsWidth);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmWsWidth(E_AUDIO_TDM_TXA, (U8)nI2sTxTdmWsWidth);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-tx0-tdm-ws-width fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        ret = CamofPropertyReadU32(pDevNode, "i2s-tx0-tdm-ws-inv", &nI2sTxTdmWsInv);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmWsInv(E_AUDIO_TDM_TXA, (U8)nI2sTxTdmWsInv);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-tx0-tdm-ws-inv fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        ret = CamofPropertyReadU32(pDevNode, "i2s-tx0-tdm-bck-inv", &nI2sTxTdmBckInv);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmBckInv(E_AUDIO_TDM_TXA, (U8)nI2sTxTdmBckInv);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-tx0-tdm-bck-inv fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        ret = CamOfPropertyReadU32Array(pDevNode, "i2s-tx0-tdm-ch-swap", &aI2sTxTdmChSwap[0], 3);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmChSwap(E_AUDIO_TDM_TXA, (U8)aI2sTxTdmChSwap[0], (U8)aI2sTxTdmChSwap[1],
                                          (U8)aI2sTxTdmChSwap[2]);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-tx0-tdm-ch-swap fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        ret = CamofPropertyReadU32(pDevNode, "i2s-tx0-tdm-active-slot", &nI2sTxTdmActiveSlot);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTxTdmActiveSlot((U8)nI2sTxTdmActiveSlot);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-tx0-tdm-active-slot fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        // I2S TX1 TDM
        ret = CamofPropertyReadU32(pDevNode, "i2s-tx1-tdm-ws-pgm", &nI2sTxTdmWsPgm);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmWsPgm(E_AUDIO_TDM_TXB, nI2sTxTdmWsPgm);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-tx1-tdm-ws-pgm fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        ret = CamofPropertyReadU32(pDevNode, "i2s-tx1-tdm-ws-width", &nI2sTxTdmWsWidth);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmWsWidth(E_AUDIO_TDM_TXB, (U8)nI2sTxTdmWsWidth);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-tx1-tdm-ws-width fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        ret = CamofPropertyReadU32(pDevNode, "i2s-tx1-tdm-ws-inv", &nI2sTxTdmWsInv);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmWsInv(E_AUDIO_TDM_TXB, (U8)nI2sTxTdmWsInv);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-tx1-tdm-ws-inv fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        ret = CamofPropertyReadU32(pDevNode, "i2s-tx1-tdm-bck-inv", &nI2sTxTdmBckInv);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmBckInv(E_AUDIO_TDM_TXB, (U8)nI2sTxTdmBckInv);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-tx1-tdm-bck-inv fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        ret = CamOfPropertyReadU32Array(pDevNode, "i2s-tx1-tdm-ch-swap", &aI2sTxTdmChSwap[0], 3);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTdmChSwap(E_AUDIO_TDM_TXB, (U8)aI2sTxTdmChSwap[0], (U8)aI2sTxTdmChSwap[1],
                                          (U8)aI2sTxTdmChSwap[2]);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-tx1-tdm-ch-swap fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        ret = CamofPropertyReadU32(pDevNode, "i2s-tx1-tdm-active-slot", &nI2sTxTdmActiveSlot);
        if (ret == 0)
        {
            ret |= HalAudI2sSaveTxTdmActiveSlot((U8)nI2sTxTdmActiveSlot);
            if (ret != AIO_OK)
            {
                goto FAIL;
            }
        }
        else
        {
            ERRMSG("%s[%d], Dts Read i2s-tx1-tdm-active-slot fail!\n", __FUNCTION__, __LINE__);
            goto FAIL;
        }

        // hpf level
        ret = CamOfPropertyReadU32Array(pDevNode, "hpf-adc1-dmic2ch-level", aHpfStatus, 2);
        if (ret == 0)
        {
            if (aHpfStatus[0] == 1)
            {
                HalAudSetHpf(E_AUD_HPF_ADC1_DMIC_2CH, aHpfStatus[1]);
            }
            else
            {
                ERRMSG("%s, not Seting %s for debug!\n", __FUNCTION__, "hpf-adc1-dmic2ch-level");
            }
        }
        else
        {
            ERRMSG("%s, Failed to get %s attr !\n", __FUNCTION__, "hpf-adc1-dmic2ch-level");
        }

        ret = CamOfPropertyReadU32Array(pDevNode, "hpf-dmic4ch-level", aHpfStatus, 2);
        if (ret == 0)
        {
            if (aHpfStatus[0] == 1)
            {
                HalAudSetHpf(E_AUD_HPF_DMIC_4CH, aHpfStatus[1]);
            }
            else
            {
                ERRMSG("%s, not Seting %s for debug!\n", __FUNCTION__, "hpf-dmic4ch-level");
            }
        }
        else
        {
            ERRMSG("%s, Failed to get %s attr !\n", __FUNCTION__, "hpf-dmic4ch-level");
        }

        // dmic_bck_mode
        if ((0 == CamofPropertyReadU32(pDevNode, "dmic-bck-mode-8k", &nDmicBckMode8K))
            && (0 == CamofPropertyReadU32(pDevNode, "dmic-bck-mode-16k", &nDmicBckMode16K))
            && (0 == CamofPropertyReadU32(pDevNode, "dmic-bck-mode-32k", &nDmicBckMode32K))
            && (0 == CamofPropertyReadU32(pDevNode, "dmic-bck-mode-48k", &nDmicBckMode48K)))
        {
            HalAudDigMicBckMode((U8)nDmicBckMode8K, (U8)nDmicBckMode16K, (U8)nDmicBckMode32K, (U8)nDmicBckMode48K);
        }
        else
        {
            ERRMSG("%s, not Seting %s for debug!\n", __FUNCTION__, "dmic-bck-mode-8k");
        }

        //
        of_node_put(pDevNode);
    }
    else
    {
        ERRMSG("%s, Failed to find device node !\n", __FUNCTION__);
        goto FAIL;
    }

    // Enable PLL
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
#else
    clk = of_clk_get(pDevNode, 0);
    if (clk == NULL)
    {
        ERRMSG("Get audio clock fail!\n");
        goto FAIL;
    }

    CamClkPrepareEnable(clk);
    clk_put(clk);
#endif

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudApiAoAmpEnable(BOOL bEnable, S8 s8Ch)
{
    if ((g_aAmpGpio[E_DTS_AMP_GPIO_A_PAD] == PAD_UNKNOWN) && (g_aAmpGpio[E_DTS_AMP_GPIO_B_PAD] == PAD_UNKNOWN))
    {
        // Do nothing
        ERRMSG("AmpGpio chn0 & chn1 = PAD_UNKNOWN in %s\n", __FUNCTION__);
        return AIO_OK;
    }

    // Using sstar_gpio_set_low(nPadNo); sstar_gpio_set_high(nPadNo); ???
    if (bEnable == TRUE)
    {
        // enable gpio for line-out, should after atop enable
        if (s8Ch == 0)
        {
            if (g_aAmpGpio[E_DTS_AMP_GPIO_A_PAD] != PAD_UNKNOWN)
                camdriver_gpio_direction_output(NULL, g_aAmpGpio[E_DTS_AMP_GPIO_A_PAD],
                                                g_aAmpGpio[E_DTS_AMP_GPIO_A_EN]);
        }
        else if (s8Ch == 1)
        {
            if (g_aAmpGpio[E_DTS_AMP_GPIO_B_PAD] != PAD_UNKNOWN)
                camdriver_gpio_direction_output(NULL, g_aAmpGpio[E_DTS_AMP_GPIO_B_PAD],
                                                g_aAmpGpio[E_DTS_AMP_GPIO_B_EN]);
        }
        else
        {
            ERRMSG("AmpGpio channel error, in %s\n", __FUNCTION__);
            goto FAIL;
        }
    }
    else
    {
        // disable gpio for line-out, should before atop disable
        if (s8Ch == 0)
        {
            if (g_aAmpGpio[E_DTS_AMP_GPIO_A_PAD] != PAD_UNKNOWN)
                camdriver_gpio_direction_output(NULL, g_aAmpGpio[E_DTS_AMP_GPIO_A_PAD],
                                                !(g_aAmpGpio[E_DTS_AMP_GPIO_A_EN]));
        }
        else if (s8Ch == 1)
        {
            if (g_aAmpGpio[E_DTS_AMP_GPIO_B_PAD] != PAD_UNKNOWN)
                camdriver_gpio_direction_output(NULL, g_aAmpGpio[E_DTS_AMP_GPIO_B_PAD],
                                                !(g_aAmpGpio[E_DTS_AMP_GPIO_B_EN]));
        }
        else
        {
            ERRMSG("AmpGpio channel error, in %s\n", __FUNCTION__);
            goto FAIL;
        }
    }

    return AIO_OK;

FAIL:

    return AIO_NG;
}

int HalAudApiGetIrqId(unsigned int *pIrqId)
{
    *pIrqId = g_aioIrqId;

    return AIO_OK;
}
