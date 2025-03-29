/*
 * hal_bdma.c- Sigmastar
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

#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/interrupt.h>
#include <ms_platform.h>
#include <ms_types.h>
#include <registers.h>
#include <kernel_bdma.h>
#include <hal_bdma.h>
#include <cam_os_wrapper.h>

#define HAL_BDMA_GROUP_NUM                   3
#define HAL_BDMA_CHANNEL_NUM                 4
#define HAL_BDMA_CHANNEL_TO_GROUP(x)         ((x) / (HAL_BDMA_CHANNEL_NUM))
#define HAL_BDMA_CHANNEL_TO_GROUP_CHANNEL(x) ((x) % (HAL_BDMA_CHANNEL_NUM))

struct hal_bdma
{
    bool            free;
    bool            init;
    bdma_register * base;
    CamOsTsem_t     sem;
    hal_bdma_param *param;
    char            name[16];
};

static struct hal_bdma hal_bdma_ctrl[HAL_BDMA_GROUP_NUM][HAL_BDMA_CHANNEL_NUM];

static irqreturn_t hal_bdma_interrupt(int irq, void *priv)
{
    struct hal_bdma *       sstar_bdma = (struct hal_bdma *)priv;
    volatile bdma_register *g_ptKeBdma = sstar_bdma->base;

    if (g_ptKeBdma->reg_ch0_int_bdma)
    {
        g_ptKeBdma->reg_ch0_int_bdma = 0x1;
        g_ptKeBdma->reg_ch0_int_en   = 0x0;

        if (sstar_bdma->param->ePathSel == HAL_BDMA_MIU0_TO_CRC)
        {
            if (sstar_bdma->param->pstCrcst)
            {
                sstar_bdma->param->pstCrcst->seedvalue =
                    (g_ptKeBdma->reg_ch0_cmd1_low | (g_ptKeBdma->reg_ch0_cmd1_high << 16));
            }
        }

        if (sstar_bdma->param->ePathSel == HAL_BDMA_MIU0_SEARCH)
        {
            if (g_ptKeBdma->reg_ch0_result0 == 0x0)
            {
                if (sstar_bdma->param->pstSearchst)
                {
                    sstar_bdma->param->pstSearchst->patternaddr =
                        (g_ptKeBdma->reg_ch0_src_a0 | (g_ptKeBdma->reg_ch0_src_a1 << 16));
                }
            }
            else
            {
                if (sstar_bdma->param->pstSearchst)
                {
                    sstar_bdma->param->pstSearchst->patternaddr = 0x0;
                }
            }
        }

        if (NULL != sstar_bdma->param->pfTxCbFunc)
        {
            sstar_bdma->param->pfTxCbFunc(sstar_bdma->param->pTxCbParm);
        }

        sstar_bdma->free = TRUE;
        CamOsTsemUp(&sstar_bdma->sem);
    }

    return IRQ_HANDLED;
}

//------------------------------------------------------------------------------
//  Function    : hal_bdma_wait_transfer_done
//  Description :
//------------------------------------------------------------------------------
/**
 * @brief BDMA wait transfer data done
 *
 * @param [in]  bdma_param      BDMA configuration parameter
 *
 * @return hal_bdma_err BDMA error code
 */
static hal_bdma_err hal_bdma_wait_transfer_done(u8 channel, hal_bdma_param *bdma_param)
{
    ktime_t                 timeout    = 5000; // 5sec
    ktime_t                 start_time = ktime_get();
    bool                    ret        = FALSE;
    int                     group      = HAL_BDMA_CHANNEL_TO_GROUP(channel);
    int                     chn        = HAL_BDMA_CHANNEL_TO_GROUP_CHANNEL(channel);
    volatile bdma_register *bdma_base  = hal_bdma_ctrl[group][chn].base;

    if (!hal_bdma_ctrl[group][chn].init)
    {
        return HAL_BDMA_PROC_DONE;
    }

    // Polling mode
    if (!bdma_param->bIntMode)
    {
        while (ktime_ms_delta(ktime_get(), start_time) < timeout)
        {
            // Check done
            if (bdma_base->reg_ch0_done == 0x1)
            {
                ret = TRUE;
                break;
            }
        }

        // Clear done
        bdma_base->reg_ch0_done = 0x1;

        hal_bdma_ctrl[group][chn].free = TRUE;
        CamOsTsemUp(&hal_bdma_ctrl[group][chn].sem);

        if (ret == FALSE)
        {
            CamOsPrintf("Wait BDMA Done Fail\r\n");
            return HAL_BDMA_POLLING_TIMEOUT;
        }
    }

    return HAL_BDMA_PROC_DONE;
}

//------------------------------------------------------------------------------
//  Function    : hal_bdma_initialize
//  Description :
//------------------------------------------------------------------------------
hal_bdma_err hal_bdma_initialize(u8 channel)
{
    u8 group    = HAL_BDMA_CHANNEL_TO_GROUP(channel);
    u8 group_ch = HAL_BDMA_CHANNEL_TO_GROUP_CHANNEL(channel);

    if (!hal_bdma_ctrl[group][group_ch].init)
    {
        u32                 irq_num = 0;
        char                compatible[16];
        struct resource     resource;
        struct device_node *dev_node = NULL;

        CamOsSnprintf(compatible, sizeof(compatible), "sstar,bdma%d", channel);

        dev_node = of_find_compatible_node(NULL, NULL, compatible);
        if (!dev_node)
        {
            return HAL_BDMA_ERROR;
        }

        /* Register interrupt handler */
        irq_num = irq_of_parse_and_map(dev_node, 0);
        if (!irq_num)
        {
            CamOsPrintf("[BDMA] irq_of_parse_and_map [%d] Fail\n", irq_num);
            return HAL_BDMA_ERROR;
        }

        CamOsSnprintf(hal_bdma_ctrl[group][group_ch].name, sizeof(hal_bdma_ctrl[group][group_ch].name), "bdma%d",
                      channel);

        hal_bdma_ctrl[group][group_ch].param = CamOsMemAlloc(sizeof(hal_bdma_param));
        if (!hal_bdma_ctrl[group][group_ch].param)
        {
            CamOsPrintf("[BDMA] CamOsMemAlloc Fail\r\n");
            return HAL_BDMA_ERROR;
        }

        /* Initial semaphore */
        CamOsTsemInit(&hal_bdma_ctrl[group][group_ch].sem, 1);
        if (of_address_to_resource(dev_node, 0, &resource))
        {
            CamOsPrintf("Can't get I/O resource regs for BDMA%d\n", group);
            if (hal_bdma_ctrl[group][group_ch].param)
            {
                CamOsMemRelease((void *)hal_bdma_ctrl[group][group_ch].param);
                hal_bdma_ctrl[group][group_ch].param = NULL;
            }
            return HAL_BDMA_ERROR;
        }
        hal_bdma_ctrl[group][group_ch].base = (bdma_register *)(IO_ADDRESS((unsigned long)resource.start));

        if (0
            != request_irq(irq_num, hal_bdma_interrupt, IRQF_SHARED | IRQF_ONESHOT, hal_bdma_ctrl[group][group_ch].name,
                           (void *)&hal_bdma_ctrl[group][group_ch]))
        {
            CamOsPrintf("[BDMA] request_irq [%d] Fail\n", irq_num);
            if (hal_bdma_ctrl[group][group_ch].param)
            {
                CamOsMemRelease((void *)hal_bdma_ctrl[group][group_ch].param);
                hal_bdma_ctrl[group][group_ch].param = NULL;
            }
            return HAL_BDMA_ERROR;
        }

        hal_bdma_ctrl[group][group_ch].init = TRUE;
    }

    return HAL_BDMA_PROC_DONE;
}
EXPORT_SYMBOL(hal_bdma_initialize);

//------------------------------------------------------------------------------
//  Function    : hal_bdma_transfer
//  Description :
//------------------------------------------------------------------------------
/**
 * @brief BDMA starts to transfer data
 *
 * @param [in]  bdma_param      BDMA configuration parameter
 *
 * @return hal_bdma_err BDMA error code
 */
hal_bdma_err hal_bdma_transfer(u8 channel, hal_bdma_param *bdma_param)
{
    bool                    wlast_loss = FALSE;
    int                     group      = HAL_BDMA_CHANNEL_TO_GROUP(channel);
    int                     chn        = HAL_BDMA_CHANNEL_TO_GROUP_CHANNEL(channel);
    volatile bdma_register *bdma_base  = hal_bdma_ctrl[group][chn].base;

    if (!hal_bdma_ctrl[group][chn].init)
    {
        return HAL_BDMA_NO_INIT;
    }

    CamOsTsemDown(&hal_bdma_ctrl[group][chn].sem);

    hal_bdma_ctrl[group][chn].free = FALSE;
    memcpy(hal_bdma_ctrl[group][chn].param, bdma_param, sizeof(hal_bdma_param));

    bdma_base->reg_ch0_busy     = 0x1;
    bdma_base->reg_ch0_int_bdma = 0x1;
    bdma_base->reg_ch0_done     = 0x1;

    switch (bdma_param->ePathSel)
    {
        case HAL_BDMA_MIU0_TO_MIU0:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_MIU0 | REG_BDMA_CH1_MIU0;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            wlast_loss                     = TRUE;
            break;
        case HAL_BDMA_MIU0_TO_MIU1:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_MIU0 | REG_BDMA_CH1_MIU1;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            wlast_loss                     = TRUE;
            break;
        case HAL_BDMA_MIU1_TO_MIU0:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_MIU1 | REG_BDMA_CH1_MIU0;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            wlast_loss                     = TRUE;
            break;
        case HAL_BDMA_MIU1_TO_MIU1:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_MIU1 | REG_BDMA_CH1_MIU1;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            wlast_loss                     = TRUE;
            break;
        case HAL_BDMA_MIU0_TO_IMI:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_MIU0 | REG_BDMA_CH1_IMI;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_MIU1_TO_IMI:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_MIU1 | REG_BDMA_CH1_IMI;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_IMI_TO_MIU0:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_IMI | REG_BDMA_CH1_MIU0;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            wlast_loss                     = TRUE;
            break;
        case HAL_BDMA_IMI_TO_MIU1:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_IMI | REG_BDMA_CH1_MIU1;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            wlast_loss                     = TRUE;
            break;
        case HAL_BDMA_IMI_TO_IMI:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_IMI | REG_BDMA_CH1_IMI;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_MEM_TO_MIU0:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MEM_FILL;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH1_MIU0;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_4BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            wlast_loss                     = TRUE;
            break;
        case HAL_BDMA_MEM_TO_MIU1:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MEM_FILL;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH1_MIU1;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_4BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            wlast_loss                     = TRUE;
            break;
        case HAL_BDMA_MEM_TO_IMI:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MEM_FILL;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH1_IMI;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_4BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_SPI_TO_MIU0:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_SPI;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH1_MIU0;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_8BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_MIU0_TO_SPI:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SDT_FSP;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH1_MIU0;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_4BYTE;
            break;
        case HAL_BDMA_SPI_TO_MIU1:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_SPI;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH1_MIU1;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_8BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_SPI_TO_IMI:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_SPI;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH1_IMI;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_8BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_MSPI0_TO_MIU:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MSPI0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_DST_MIU_IMI_CH0;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_MIU0;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            wlast_loss                     = TRUE;
            break;
        case HAL_BDMA_MSPI1_TO_MIU:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MSPI1;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_DST_MIU_IMI_CH0;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_MIU0;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            wlast_loss                     = TRUE;
            break;
        case HAL_BDMA_MIU_TO_MSPI0:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_DST_MSPI0;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_MIU0;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_MIU_TO_MSPI1:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_DST_MSPI1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_MIU0;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_PSRAM_TO_MIU0:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_PSRAM | REG_BDMA_CH1_MIU0;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            wlast_loss                     = TRUE;
            break;
        case HAL_BDMA_MIU0_TO_PSRAM:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_MIU0 | REG_BDMA_CH1_PSRAM;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_MIU0_SEARCH:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_DST_SEARCH;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_MIU0;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_1BYTE;
            break;
        case HAL_BDMA_MIU0_TO_CRC:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_DST_CRC;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_MIU0;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_1BYTE;
            break;
        case HAL_BDMA_MIU0_TO_CM4_IMI:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_MIU0 | REG_BDMA_CH1_IMI;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_CM4_IMI_TO_MIU0:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_IMI | REG_BDMA_CH1_MIU0;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        default:
            return HAL_BDMA_PROC_DONE;
            break;
    }

    // Set Source / Destination Address
    if ((HAL_BDMA_MEM_TO_MIU0 == bdma_param->ePathSel) || (HAL_BDMA_MEM_TO_MIU1 == bdma_param->ePathSel)
        || (HAL_BDMA_MEM_TO_IMI == bdma_param->ePathSel))
    {
        bdma_base->reg_ch0_cmd0_low  = (U16)(bdma_param->u32Pattern & 0xFFFF);
        bdma_base->reg_ch0_cmd0_high = (U16)(bdma_param->u32Pattern >> 16);
        bdma_base->reg_ch0_src_a0    = (U16)((0x0000) & 0xFFFF);
        bdma_base->reg_ch0_src_a1    = (U16)((0x0000) & 0xFFFF);
#ifdef CONFIG_PHYS_ADDR_T_64BIT
        bdma_base->reg_ch0_src_a_msb = (U16)((0x0000) & 0xF);
#endif
    }
    else
    {
        bdma_base->reg_ch0_src_a0 = (U16)((bdma_param->pSrcAddr) & 0xFFFF);
        bdma_base->reg_ch0_src_a1 = (U16)(((bdma_param->pSrcAddr) >> 16) & 0xFFFF);
#ifdef CONFIG_PHYS_ADDR_T_64BIT
        bdma_base->reg_ch0_src_a_msb = (U16)(((bdma_param->pSrcAddr) >> 32) & 0xF);
#endif
    }

    if (HAL_BDMA_MIU0_TO_CRC == bdma_param->ePathSel)
    {
        if (bdma_param->pstCrcst)
        {
            bdma_base->reg_ch0_cfg       = (bdma_param->pstCrcst->reflection ? 0x1 : 0x0);
            bdma_base->reg_ch0_cmd0_low  = (U16)(bdma_param->pstCrcst->polynomial & 0xFFFF);
            bdma_base->reg_ch0_cmd0_high = (U16)(bdma_param->pstCrcst->polynomial >> 16);
            bdma_base->reg_ch0_cmd1_low  = (U16)(0x0000);
            bdma_base->reg_ch0_cmd1_high = (U16)(0x0000);
            bdma_base->reg_ch0_dst_a0    = (U16)(0x0000);
            bdma_base->reg_ch0_dst_a1    = (U16)(0x0000);
#ifdef CONFIG_PHYS_ADDR_T_64BIT
            bdma_base->reg_ch0_dst_a_msb = (U16)(0x0000 & 0xF);
#endif
        }
        else
        {
            return HAL_BDMA_ERROR;
        }
    }
    else if (HAL_BDMA_MIU0_SEARCH == bdma_param->ePathSel)
    {
        if (bdma_param->pstSearchst)
        {
            bdma_base->reg_ch0_cfg =
                ((bdma_param->pstSearchst->antimatch ? BIT0 : 0) | (bdma_param->pstSearchst->mobf ? BIT1 : 0));
            bdma_base->reg_ch0_cmd0_low  = (bdma_param->u32Pattern & 0xFFFF);
            bdma_base->reg_ch0_cmd0_high = (bdma_param->u32Pattern >> 16);
            bdma_base->reg_ch0_cmd1_low  = (bdma_param->pstSearchst->patternmask & 0xFFFF);
            bdma_base->reg_ch0_cmd1_high = (bdma_param->pstSearchst->patternmask >> 16);
            bdma_base->reg_ch0_cmd2_low  = (bdma_param->pstSearchst->mobfkey & 0xFFFF);
            bdma_base->reg_ch0_cmd2_high = (bdma_param->pstSearchst->mobfkey >> 16);
            bdma_base->reg_ch0_dst_a0    = (U16)(0x0000);
            bdma_base->reg_ch0_dst_a1    = (U16)(0x0000);
#ifdef CONFIG_PHYS_ADDR_T_64BIT
            bdma_base->reg_ch0_dst_a_msb = (U16)(0x0000 & 0xF);
#endif
        }
        else
        {
            return HAL_BDMA_ERROR;
        }
    }
    else
    {
        bdma_base->reg_ch0_dst_a0 = (U16)((bdma_param->pDstAddr) & 0xFFFF);
        bdma_base->reg_ch0_dst_a1 = (U16)(((bdma_param->pDstAddr) >> 16) & 0xFFFF);
#ifdef CONFIG_PHYS_ADDR_T_64BIT
        bdma_base->reg_ch0_dst_a_msb = (U16)(((bdma_param->pDstAddr) >> 32) & 0xF);
#endif
    }

    // Set Transfer Size
    bdma_base->reg_ch0_size0 = (U16)(bdma_param->u32TxCount & 0xFFFF);
    bdma_base->reg_ch0_size1 = (U16)(bdma_param->u32TxCount >> 16);

    /* Set LineOffset Attribute */
    if (bdma_param->bEnLineOfst == TRUE)
    {
        bdma_base->reg_ch0_src_width_low   = (U16)(bdma_param->pstLineOfst->u32SrcWidth & 0xFFFF);
        bdma_base->reg_ch0_src_width_high  = (U16)(bdma_param->pstLineOfst->u32SrcWidth >> 16);
        bdma_base->reg_ch0_src_offset_low  = (U16)(bdma_param->pstLineOfst->u32SrcOffset & 0xFFFF);
        bdma_base->reg_ch0_src_offset_high = (U16)(bdma_param->pstLineOfst->u32SrcOffset >> 16);
        bdma_base->reg_ch0_dst_width_low   = (U16)(bdma_param->pstLineOfst->u32DstWidth & 0xFFFF);
        bdma_base->reg_ch0_dst_width_high  = (U16)(bdma_param->pstLineOfst->u32DstWidth >> 16);
        bdma_base->reg_ch0_dst_offset_low  = (U16)(bdma_param->pstLineOfst->u32DstOffset & 0xFFFF);
        bdma_base->reg_ch0_dst_offset_high = (U16)(bdma_param->pstLineOfst->u32DstOffset >> 16);
        bdma_base->reg_ch0_offset_en       = 1;
    }
    else if (wlast_loss == TRUE)
    {
        bdma_base->reg_ch0_src_width_low   = 0x400;
        bdma_base->reg_ch0_src_width_high  = 0x0;
        bdma_base->reg_ch0_src_offset_low  = 0x400;
        bdma_base->reg_ch0_src_offset_high = 0x0;
        bdma_base->reg_ch0_dst_width_low   = 0x400;
        bdma_base->reg_ch0_dst_width_high  = 0x0;
        bdma_base->reg_ch0_dst_offset_low  = 0x400;
        bdma_base->reg_ch0_dst_offset_high = 0x0;
        bdma_base->reg_ch0_offset_en       = 1;
    }
    else
    {
        bdma_base->reg_ch0_offset_en = 0;
    }

    // Set Interrupt Enable
    if (bdma_param->bIntMode)
    {
        bdma_base->reg_ch0_int_en = 1;
    }
    else
    {
        bdma_base->reg_ch0_int_en = 0;
    }

    // Trigger
    bdma_base->reg_ch0_trig = 0x1;

    // Polling mode
    if (!bdma_param->bIntMode)
    {
        hal_bdma_wait_transfer_done(channel, bdma_param);
    }

    return HAL_BDMA_PROC_DONE;
}
EXPORT_SYMBOL(hal_bdma_transfer);
