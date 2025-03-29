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
// Include files
/*=============================================================*/

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
#ifdef CONFIG_OPTEE
#include <linux/arm-smccc.h>
#include <optee_msg.h>
#include <optee_smc.h>
#endif

#define HAL_BDMA_GROUP_NUM                   3
#define HAL_BDMA_CHANNEL_NUM                 4
#define HAL_BDMA_CHANNEL_TO_GROUP(x)         ((x) / (HAL_BDMA_CHANNEL_NUM))
#define HAL_BDMA_CHANNEL_TO_GROUP_CHANNEL(x) ((x) % (HAL_BDMA_CHANNEL_NUM))

#define OPTEE_SMC_FAST_TZMISC_PREFIX 0xFD00

#define OPTEE_SMC_FUNCID_TZMISC (OPTEE_SMC_FAST_TZMISC_PREFIX + 0x00)
#define OPTEE_SMC_TZMISC        OPTEE_SMC_FAST_CALL_VAL(OPTEE_SMC_FUNCID_TZMISC)

#define OPTEE_SMC_TZMISC_SET_REG_SELECT_SPI_NONPM 0
#define OPTEE_SMC_TZMISC_SET_REG_SELECT_SPI_PM    1

////////////////////////////////////////////////////////////////////////////////
// Global variable
////////////////////////////////////////////////////////////////////////////////

struct hal_bdma
{
    volatile int             group;
    volatile bool            free;
    volatile bool            init;
    volatile bdma_register * base;
    CamOsTsem_t              sem;
    volatile hal_bdma_param *param;
};

typedef enum spi_domain
{
    PM    = 0,
    NONPM = 1,
} spi_domain_t;

static struct hal_bdma hal_bdma_ctrl[HAL_BDMA_GROUP_NUM][HAL_BDMA_CHANNEL_NUM];

/*=============================================================*/
// Local function definition
/*=============================================================*/

static int hal_bdma_clear_irq_and_get_channel(int group)
{
    int i;

    for (i = 0; i < HAL_BDMA_CHANNEL_NUM; i++)
    {
        volatile bdma_register *bdma_base = hal_bdma_ctrl[group][i].base;
        if (bdma_base->reg_ch0_int_bdma)
        {
            bdma_base->reg_ch0_int_bdma = 0x1;
            bdma_base->reg_ch0_int_en   = 0x0;
            return i;
        }
    }
    return -1;
}

static irqreturn_t hal_bdma_interrupt(int irq, void *priv)
{
    int chn;
    int group = *(int *)priv;

    chn = hal_bdma_clear_irq_and_get_channel(group);
    if (chn < 0)
    {
        CamOsPrintf("[BDMA_IRQ] Should never happen !\n");
        return IRQ_NONE;
    }

    if (NULL != hal_bdma_ctrl[group][chn].param->pfTxCbFunc)
    {
        hal_bdma_ctrl[group][chn].param->pfTxCbFunc(hal_bdma_ctrl[group][chn].param->pTxCbParm);
    }

    hal_bdma_ctrl[group][chn].free = TRUE;
    CamOsTsemUp(&hal_bdma_ctrl[group][chn].sem);

    return IRQ_HANDLED;
}

static void hal_bdma_select_spi(spi_domain_t domain)
{
    if (Chip_Get_Revision() >= CHIP_VERSION_UO2)
    {
        if (domain == NONPM)
            SETREG16(BASE_REG_CHIPTOP_PA + REG_ID_45, BIT0);
        else
            CLRREG16(BASE_REG_CHIPTOP_PA + REG_ID_45, BIT0);
    }
    else if (Chip_Get_Revision() == CHIP_VERSION_UO1)
    {
#ifdef CONFIG_OPTEE
        if ((INREG16(BASE_REG_OTP2_PA + REG_ID_30) & BIT9) >> 9)
        {
            struct arm_smccc_res res;

            if (domain == NONPM)
                arm_smccc_smc(OPTEE_SMC_TZMISC, OPTEE_SMC_TZMISC_SET_REG_SELECT_SPI_NONPM, 0, 0, 0, 0, 0, 0, &res);
            else
                arm_smccc_smc(OPTEE_SMC_TZMISC, OPTEE_SMC_TZMISC_SET_REG_SELECT_SPI_PM, 0, 0, 0, 0, 0, 0, &res);

            if (res.a0 != OPTEE_SMC_RETURN_OK)
                CamOsPrintf("set reg_select_spi failed!\n");
        }
        else
#endif
        {
            if (domain == NONPM)
                SETREG16(BASE_REG_TZMISC + REG_ID_00, BIT0);
            else
                CLRREG16(BASE_REG_TZMISC + REG_ID_00, BIT0);
        }
    }
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
    u32                     timeout   = 0x00FFFFFF;
    bool                    ret       = FALSE;
    int                     group     = HAL_BDMA_CHANNEL_TO_GROUP(channel);
    int                     chn       = HAL_BDMA_CHANNEL_TO_GROUP_CHANNEL(channel);
    volatile bdma_register *bdma_base = hal_bdma_ctrl[group][chn].base;

    if (!hal_bdma_ctrl[group][chn].init)
    {
        return HAL_BDMA_PROC_DONE;
    }

    // Polling mode
    if (!bdma_param->bIntMode)
    {
        while (--timeout)
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
    else
    {
        // Interrupt mode
    }

    return HAL_BDMA_PROC_DONE;
}

//------------------------------------------------------------------------------
//  Function    : hal_bdma_initialize
//  Description :
//------------------------------------------------------------------------------
hal_bdma_err hal_bdma_initialize(u8 channel)
{
    void *ptr   = NULL;
    int   group = HAL_BDMA_CHANNEL_TO_GROUP(channel);

    if (!hal_bdma_ctrl[group][0].init)
    {
        struct device_node *dev_node = NULL;
        char                compatible[16];
        int                 irq = 0;
        int                 i;
        unsigned int        reg_base[4];
        hal_bdma_ctrl[group][0].group = group;

        CamOsSnprintf(compatible, sizeof(compatible), "sstar,bdma%d", group);
        dev_node = of_find_compatible_node(NULL, NULL, compatible);

        if (!dev_node)
        {
            return HAL_BDMA_ERROR;
        }

        /* Register interrupt handler */
        irq = irq_of_parse_and_map(dev_node, 0);

        if (0 != request_irq(irq, hal_bdma_interrupt, IRQF_SHARED, "BdmaIsr", (void *)&hal_bdma_ctrl[group][0].group))
        {
            CamOsPrintf("[BDMA] request_irq [%d] Fail\r\n", irq);
            return HAL_BDMA_ERROR;
        }
        else
        {
            // CamOsPrintf("[BDMA] request_irq [%d] OK\r\n", irq);
        }

        for (i = 0; i < HAL_BDMA_CHANNEL_NUM; i++)
        {
            ptr = CamOsMemAlloc(sizeof(hal_bdma_param));

            if (!ptr)
            {
                CamOsPrintf("[BDMA] CamOsMemAlloc Fail\r\n");
                return HAL_BDMA_ERROR;
            }

            hal_bdma_ctrl[group][i].param = ptr;
            // hal_bdma_ctrl[group][i].base = (struct KeBdma_s *)(unsigned long)(of_iomap(dev_node, 0)+(0x80*i));
            hal_bdma_ctrl[group][i].init = TRUE;
            /* Initial semaphore */
            CamOsTsemInit(&hal_bdma_ctrl[group][i].sem, 1);

            // if (of_property_read_u32(dev_node, "reg", &reg_base))
            if (of_property_read_u32_array(dev_node, "reg", reg_base, 2))
            {
                CamOsPrintf("Can't get I/O resource regs for BDMA%d\n", group);
                return -1;
            }

            hal_bdma_ctrl[group][i].base = (bdma_register *)(IO_ADDRESS((unsigned long)reg_base[0] + (0x80 * i)));
        }
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
    int                     group     = HAL_BDMA_CHANNEL_TO_GROUP(channel);
    int                     chn       = HAL_BDMA_CHANNEL_TO_GROUP_CHANNEL(channel);
    volatile bdma_register *bdma_base = hal_bdma_ctrl[group][chn].base;

    if (!hal_bdma_ctrl[group][chn].init)
    {
        return HAL_BDMA_NO_INIT;
    }

    CamOsTsemDown(&hal_bdma_ctrl[group][chn].sem);

    hal_bdma_ctrl[group][chn].free = FALSE;
    memcpy((void *)hal_bdma_ctrl[group][chn].param, (const void *)bdma_param, sizeof(hal_bdma_param));

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
            break;
        case HAL_BDMA_MIU0_TO_MIU1:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_MIU0 | REG_BDMA_CH1_MIU1;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_MIU1_TO_MIU0:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_MIU1 | REG_BDMA_CH1_MIU0;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_MIU1_TO_MIU1:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_MIU1 | REG_BDMA_CH1_MIU1;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
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
            break;
        case HAL_BDMA_IMI_TO_MIU1:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_IMI | REG_BDMA_CH1_MIU1;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
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
            break;
        case HAL_BDMA_MEM_TO_MIU1:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MEM_FILL;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH1_MIU1;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_4BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_MEM_TO_IMI:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MEM_FILL;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH1_IMI;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_4BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_PM_SPI_TO_MIU0:
            hal_bdma_select_spi(PM);
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_SPI;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH1_MIU0;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_8BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_SPI_TO_MIU0:
            hal_bdma_select_spi(NONPM);
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_SPI;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH1_MIU0;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_8BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_MIU0_TO_PM_SPI:
            hal_bdma_select_spi(PM);
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SDT_FSP;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH1_MIU0;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_4BYTE;
            break;
        case HAL_BDMA_MIU0_TO_SPI:
            hal_bdma_select_spi(NONPM);
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
            break;
        case HAL_BDMA_MSPI1_TO_MIU:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MSPI1;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_DST_MIU_IMI_CH0;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_MIU0;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
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
        case HAL_BDMA_MSPI2_TO_MIU:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MSPI2;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_DST_MIU_IMI_CH0;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_MIU0;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_MSPI3_TO_MIU:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MSPI3;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_DST_MIU_IMI_CH0;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_MIU0;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_MIU_TO_MSPI2:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_DST_MSPI2;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_MIU0;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_MIU_TO_MSPI3:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_DST_MSPI3;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_MIU0;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_MIU0_TO_CM4_IMI:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_MIU0 | REG_BDMA_CH1_PM_IMI;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_MIU1_TO_CM4_IMI:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_MIU1 | REG_BDMA_CH1_PM_IMI;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_CM4_IMI_TO_MIU0:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_PM_IMI | REG_BDMA_CH1_MIU0;
            bdma_base->reg_ch0_src_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            bdma_base->reg_ch0_dst_dw      = REG_BDMA_DATA_DEPTH_16BYTE;
            break;
        case HAL_BDMA_CM4_IMI_TO_MIU1:
            bdma_base->reg_ch0_src_sel     = REG_BDMA_SRC_MIU_IMI_CH0;
            bdma_base->reg_ch0_dst_sel     = REG_BDMA_SRC_MIU_IMI_CH1;
            bdma_base->reg_ch0_replace_miu = REG_BDMA_CH0_PM_IMI | REG_BDMA_CH1_MIU1;
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

    bdma_base->reg_ch0_dst_a0 = (U16)((bdma_param->pDstAddr) & 0xFFFF);
    bdma_base->reg_ch0_dst_a1 = (U16)(((bdma_param->pDstAddr) >> 16) & 0xFFFF);
#ifdef CONFIG_PHYS_ADDR_T_64BIT
    bdma_base->reg_ch0_dst_a_msb = (U16)(((bdma_param->pDstAddr) >> 32) & 0xF);
#endif

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
