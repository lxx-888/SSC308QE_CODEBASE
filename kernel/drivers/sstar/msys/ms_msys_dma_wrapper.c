/*
 * ms_msys_dma_wrapper.c- Sigmastar
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
#include <linux/kernel.h>
//#include <asm/uaccess.h> /* for get_fs*/
#include <linux/miscdevice.h>
#include <linux/dma-mapping.h> /* for dma_alloc_coherent */
#include <linux/slab.h>
#include <linux/swap.h>
#include <linux/delay.h>
#include <linux/seq_file.h>
#include <linux/compaction.h> /*  for sysctl_compaction_handler*/
#include <asm/cacheflush.h>

#include "registers.h"
#include "ms_platform.h"
#include "mdrv_msys_io_st.h"
#include "mdrv_msys_io.h"
#include "platform_msys.h"
#ifdef CONFIG_SSTAR_BDMA
#include "hal_bdma.h"
#endif

#include "cam_os_wrapper.h"

extern struct miscdevice sys_dev;

#if defined(CONFIG_SSTAR_BDMA)
static void msys_bdma_done(void *Parm)
{
    CamOsTsem_t *pstBdmaDoneSem = (CamOsTsem_t *)Parm;
    CamOsTsemUp(pstBdmaDoneSem);
}

int msys_dma_fill(MSYS_DMA_FILL *pstDmaCfg)
{
    hal_bdma_param tBdmaParam;
    u8             u8DmaCh = HAL_BDMA_CH1;
    CamOsTsem_t    tBdmaDoneSem;

    memset(&tBdmaDoneSem, 0, sizeof(CamOsTsem_t));
    CamOsTsemInit(&tBdmaDoneSem, 0);
    memset(&tBdmaParam, 0, sizeof(hal_bdma_param));
    // tBdmaParam.ePathSel  = (pstDmaCfg->phyaddr < ARM_MIU1_BASE_ADDR) ? (HAL_BDMA_MEM_TO_MIU0) :
    // (HAL_BDMA_MEM_TO_MIU1);
    tBdmaParam.ePathSel = HAL_BDMA_MEM_TO_MIU0;

    tBdmaParam.bIntMode     = 1;
    tBdmaParam.eDstAddrMode = HAL_BDMA_ADDR_INC;
    tBdmaParam.u32TxCount   = pstDmaCfg->length;
    tBdmaParam.pSrcAddr     = 0;
    // tBdmaParam.pDstAddr     = (pstDmaCfg->phyaddr < ARM_MIU1_BASE_ADDR) ? (void *)((unsigned long)pstDmaCfg->phyaddr)
    // : (void *)((unsigned long)pstDmaCfg->phyaddr - ARM_MIU1_BASE_ADDR);
    tBdmaParam.pDstAddr   = pstDmaCfg->phyaddr;
    tBdmaParam.pfTxCbFunc = msys_bdma_done;
    tBdmaParam.pTxCbParm  = (void *)&tBdmaDoneSem;
    tBdmaParam.u32Pattern = pstDmaCfg->pattern;

    if (HAL_BDMA_PROC_DONE != hal_bdma_transfer(u8DmaCh, &tBdmaParam))
    {
        return -1;
    }

    if (tBdmaParam.bIntMode)
    {
        CamOsTsemDown(&tBdmaDoneSem);
    }
    CamOsTsemDeinit(&tBdmaDoneSem);

    return 0;
}
EXPORT_SYMBOL(msys_dma_fill);

int msys_dma_copy(MSYS_DMA_COPY *cfg)
{
    hal_bdma_param tBdmaParam;
    u8             u8DmaCh = HAL_BDMA_CH2;
    CamOsTsem_t    tBdmaDoneSem;

    memset(&tBdmaDoneSem, 0, sizeof(CamOsTsem_t));
    CamOsTsemInit(&tBdmaDoneSem, 0);
    memset(&tBdmaParam, 0, sizeof(hal_bdma_param));
    // tBdmaParam.ePathSel     = ((unsigned long)cfg->phyaddr_src < ARM_MIU1_BASE_ADDR) ? (HAL_BDMA_MIU0_TO_MIU0) :
    // (HAL_BDMA_MIU1_TO_MIU0); tBdmaParam.ePathSel     = ((unsigned long)cfg->phyaddr_dst < ARM_MIU1_BASE_ADDR) ?
    // tBdmaParam.ePathSel : tBdmaParam.ePathSel+1;
    tBdmaParam.ePathSel = HAL_BDMA_MIU0_TO_MIU0;

    // CamOsPrintf("tBdmaParam.ePathSel %d\n",tBdmaParam.ePathSel);
    tBdmaParam.pSrcAddr     = cfg->phyaddr_src;
    tBdmaParam.pDstAddr     = cfg->phyaddr_dst;
    tBdmaParam.bIntMode     = 1;
    tBdmaParam.eDstAddrMode = HAL_BDMA_ADDR_INC;
    tBdmaParam.u32TxCount   = cfg->length;
    tBdmaParam.pfTxCbFunc   = msys_bdma_done;
    tBdmaParam.pTxCbParm    = (void *)&tBdmaDoneSem;
    tBdmaParam.u32Pattern   = 0;

    if (HAL_BDMA_PROC_DONE != hal_bdma_transfer(u8DmaCh, &tBdmaParam))
    {
        return -1;
    }

    if (tBdmaParam.bIntMode)
    {
        CamOsTsemDown(&tBdmaDoneSem);
    }
    CamOsTsemDeinit(&tBdmaDoneSem);

    return 0;
}
EXPORT_SYMBOL(msys_dma_copy);

int msys_dma_copy_to_pmimi(MSYS_DMA_COPY *cfg)
{
    hal_bdma_param tBdmaParam;
    u8             u8DmaCh = HAL_BDMA2_CH0;
    CamOsTsem_t    tBdmaDoneSem;

    memset(&tBdmaDoneSem, 0, sizeof(CamOsTsem_t));
    CamOsTsemInit(&tBdmaDoneSem, 0);
    memset(&tBdmaParam, 0, sizeof(hal_bdma_param));
    tBdmaParam.ePathSel = HAL_BDMA_MIU0_TO_IMI;

    tBdmaParam.pSrcAddr     = cfg->phyaddr_src;
    tBdmaParam.pDstAddr     = cfg->phyaddr_dst;
    tBdmaParam.bIntMode     = 0;
    tBdmaParam.eDstAddrMode = HAL_BDMA_ADDR_INC;
    tBdmaParam.u32TxCount   = cfg->length;
    tBdmaParam.pfTxCbFunc   = msys_bdma_done;
    tBdmaParam.pTxCbParm    = (void *)&tBdmaDoneSem;
    tBdmaParam.u32Pattern   = 0;

    if (HAL_BDMA_PROC_DONE != hal_bdma_transfer(u8DmaCh, &tBdmaParam))
    {
        return -1;
    }

    if (tBdmaParam.bIntMode)
    {
        CamOsTsemDown(&tBdmaDoneSem);
    }
    CamOsTsemDeinit(&tBdmaDoneSem);

    return 0;
}
EXPORT_SYMBOL(msys_dma_copy_to_pmimi);

int msys_dma_copy_from_pmimi(MSYS_DMA_COPY *cfg)
{
    hal_bdma_param tBdmaParam;
    u8             u8DmaCh = HAL_BDMA2_CH0;
    CamOsTsem_t    tBdmaDoneSem;

    memset(&tBdmaDoneSem, 0, sizeof(CamOsTsem_t));
    CamOsTsemInit(&tBdmaDoneSem, 0);
    memset(&tBdmaParam, 0, sizeof(hal_bdma_param));
    tBdmaParam.ePathSel = HAL_BDMA_IMI_TO_MIU0;

    tBdmaParam.pSrcAddr     = cfg->phyaddr_src;
    tBdmaParam.pDstAddr     = cfg->phyaddr_dst;
    tBdmaParam.bIntMode     = 0;
    tBdmaParam.eDstAddrMode = HAL_BDMA_ADDR_INC;
    tBdmaParam.u32TxCount   = cfg->length;
    tBdmaParam.pfTxCbFunc   = msys_bdma_done;
    tBdmaParam.pTxCbParm    = (void *)&tBdmaDoneSem;
    tBdmaParam.u32Pattern   = 0;

    if (HAL_BDMA_PROC_DONE != hal_bdma_transfer(u8DmaCh, &tBdmaParam))
    {
        return -1;
    }

    if (tBdmaParam.bIntMode)
    {
        CamOsTsemDown(&tBdmaDoneSem);
    }
    CamOsTsemDeinit(&tBdmaDoneSem);

    return 0;
}
EXPORT_SYMBOL(msys_dma_copy_from_pmimi);

/**
 * msys_dma_copy_general() - for Bdma 0/1/2/3/4 copy data,for miu2miu and mspi
 *
 * @u8DmaCh:   select which bdma channel be use. [0-20)
 * @epath_sel: select bdma copy way.<bdma 1/2/3/4 only for mspi2miu or miu2mspi or miu2miu>
 * @cfg:       param of src/dst/length
 *
 * The relationship between BDMA group and channel
 * group     channel
 * bdma0     <00-03>
 * bdma1     <04-07>
 * bdma2     <08-11>
 * bdma3     <11-15>
 * bdma4     <15-19>
 *
 * @Returns.0-successful
 */
int msys_dma_copy_general(u8 u8DmaCh, int path_sel, MSYS_DMA_COPY *cfg)
{
    hal_bdma_param tBdmaParam;
    CamOsTsem_t    tBdmaDoneSem;

    if (u8DmaCh >= HAL_BDMA_CH_NUM)
        return -1;
    memset(&tBdmaDoneSem, 0, sizeof(CamOsTsem_t));
    CamOsTsemInit(&tBdmaDoneSem, 0);
    hal_bdma_initialize(u8DmaCh);

    memset(&tBdmaParam, 0, sizeof(hal_bdma_param));
    tBdmaParam.ePathSel     = path_sel;
    tBdmaParam.pSrcAddr     = cfg->phyaddr_src;
    tBdmaParam.pDstAddr     = cfg->phyaddr_dst;
    tBdmaParam.bIntMode     = 1;
    tBdmaParam.eDstAddrMode = HAL_BDMA_ADDR_INC;
    tBdmaParam.u32TxCount   = cfg->length;
    tBdmaParam.pfTxCbFunc   = msys_bdma_done;
    tBdmaParam.pTxCbParm    = (void *)&tBdmaDoneSem;
    tBdmaParam.u32Pattern   = 0;

    if (HAL_BDMA_PROC_DONE != hal_bdma_transfer(u8DmaCh, &tBdmaParam))
    {
        return -1;
    }

    if (tBdmaParam.bIntMode)
    {
        CamOsTsemDown(&tBdmaDoneSem);
    }
    CamOsTsemDeinit(&tBdmaDoneSem);

    return 0;
}
EXPORT_SYMBOL(msys_dma_copy_general);

int msys_dma_copy_to_psram(MSYS_DMA_COPY *cfg)
{
    hal_bdma_param tBdmaParam;
    u8             u8DmaCh = HAL_BDMA2_CH0;
    CamOsTsem_t    tBdmaDoneSem;

    memset(&tBdmaDoneSem, 0, sizeof(CamOsTsem_t));
    CamOsTsemInit(&tBdmaDoneSem, 0);
    memset(&tBdmaParam, 0, sizeof(hal_bdma_param));

    tBdmaParam.ePathSel = HAL_BDMA_MIU0_TO_PSRAM;

    tBdmaParam.pSrcAddr     = cfg->phyaddr_src;
    tBdmaParam.pDstAddr     = cfg->phyaddr_dst;
    tBdmaParam.bIntMode     = 0;
    tBdmaParam.eDstAddrMode = HAL_BDMA_ADDR_INC;
    tBdmaParam.u32TxCount   = cfg->length;
    tBdmaParam.pfTxCbFunc   = msys_bdma_done;
    tBdmaParam.pTxCbParm    = (void *)&tBdmaDoneSem;
    tBdmaParam.u32Pattern   = 0;

    if (HAL_BDMA_PROC_DONE != hal_bdma_transfer(u8DmaCh, &tBdmaParam))
    {
        return -1;
    }

    if (tBdmaParam.bIntMode)
    {
        CamOsTsemDown(&tBdmaDoneSem);
    }
    CamOsTsemDeinit(&tBdmaDoneSem);

    return 0;
}
EXPORT_SYMBOL(msys_dma_copy_to_psram);

int msys_dma_copy_from_psram(MSYS_DMA_COPY *cfg)
{
    hal_bdma_param tBdmaParam;
    u8             u8DmaCh = HAL_BDMA2_CH0;
    CamOsTsem_t    tBdmaDoneSem;

    memset(&tBdmaDoneSem, 0, sizeof(CamOsTsem_t));
    CamOsTsemInit(&tBdmaDoneSem, 0);
    memset(&tBdmaParam, 0, sizeof(hal_bdma_param));

    tBdmaParam.ePathSel = HAL_BDMA_PSRAM_TO_MIU0;

    tBdmaParam.pSrcAddr     = cfg->phyaddr_src;
    tBdmaParam.pDstAddr     = cfg->phyaddr_dst;
    tBdmaParam.bIntMode     = 0;
    tBdmaParam.eDstAddrMode = HAL_BDMA_ADDR_INC;
    tBdmaParam.u32TxCount   = cfg->length;
    tBdmaParam.pfTxCbFunc   = msys_bdma_done;
    tBdmaParam.pTxCbParm    = (void *)&tBdmaDoneSem;
    tBdmaParam.u32Pattern   = 0;

    if (HAL_BDMA_PROC_DONE != hal_bdma_transfer(u8DmaCh, &tBdmaParam))
    {
        return -1;
    }

    if (tBdmaParam.bIntMode)
    {
        CamOsTsemDown(&tBdmaDoneSem);
    }
    CamOsTsemDeinit(&tBdmaDoneSem);

    return 0;
}
EXPORT_SYMBOL(msys_dma_copy_from_psram);
#endif

#if defined(CONFIG_SSTAR_BDMA_LINE_OFFSET_ON)
int msys_dma_fill_lineoffset(MSYS_DMA_FILL_BILT *pstDmaCfg)
{
    hal_bdma_param       tBdmaParam;
    hal_bdma_line_offset tBdmaLineOfst;
    u8                   u8DmaCh = HAL_BDMA_CH1;
    CamOsTsem_t          tBdmaDoneSem;

    memset(&tBdmaDoneSem, 0, sizeof(CamOsTsem_t));
    CamOsTsemInit(&tBdmaDoneSem, 0);
    memset(&tBdmaParam, 0, sizeof(hal_bdma_param));
    // tBdmaParam.ePathSel     = (pstDmaCfg->phyaddr < ARM_MIU1_BASE_ADDR) ? (HAL_BDMA_MEM_TO_MIU0) :
    // (HAL_BDMA_MEM_TO_MIU1);
    tBdmaParam.ePathSel = HAL_BDMA_MEM_TO_MIU0;

    tBdmaParam.bIntMode     = 1;
    tBdmaParam.eDstAddrMode = HAL_BDMA_ADDR_INC;
    tBdmaParam.u32TxCount   = pstDmaCfg->length;
    tBdmaParam.pSrcAddr     = 0;
    // tBdmaParam.pDstAddr     = (pstDmaCfg->phyaddr < ARM_MIU1_BASE_ADDR) ? (void *)((unsigned long)pstDmaCfg->phyaddr)
    // : (void *)((unsigned long)pstDmaCfg->phyaddr - ARM_MIU1_BASE_ADDR);
    tBdmaParam.pDstAddr = pstDmaCfg->phyaddr;

    tBdmaParam.pfTxCbFunc = msys_bdma_done;
    tBdmaParam.pTxCbParm  = (void *)&tBdmaDoneSem;
    tBdmaParam.u32Pattern = pstDmaCfg->pattern;

    if (pstDmaCfg->lineofst_dst)
    {
        if (pstDmaCfg->lineofst_dst < pstDmaCfg->width_dst)
        {
            printk(
                "[BDMA] CAUTION: line offset is less than width\n"
                "DstWidth=0x%x  lineofst_dst=0x%x\n",
                pstDmaCfg->width_dst, pstDmaCfg->lineofst_dst);
            dump_stack();
            return -1;
        }

        tBdmaParam.pstLineOfst               = &tBdmaLineOfst;
        tBdmaParam.pstLineOfst->u32SrcWidth  = pstDmaCfg->width_dst;
        tBdmaParam.pstLineOfst->u32SrcOffset = pstDmaCfg->lineofst_dst;
        tBdmaParam.pstLineOfst->u32DstWidth  = pstDmaCfg->width_dst;
        tBdmaParam.pstLineOfst->u32DstOffset = pstDmaCfg->lineofst_dst;

        tBdmaParam.bEnLineOfst = 1;
    }
    else
    {
        tBdmaParam.bEnLineOfst = 0;
        tBdmaParam.pstLineOfst = NULL;
    }

    if (HAL_BDMA_PROC_DONE != hal_bdma_transfer(u8DmaCh, &tBdmaParam))
    {
        return -1;
    }

    if (tBdmaParam.bIntMode)
    {
        CamOsTsemDown(&tBdmaDoneSem);
    }
    CamOsTsemDeinit(&tBdmaDoneSem);
    return 0;
}
EXPORT_SYMBOL(msys_dma_fill_lineoffset);

int msys_dma_copy_lineoffset(MSYS_DMA_BLIT *cfg)
{
    hal_bdma_param       tBdmaParam;
    hal_bdma_line_offset tBdmaLineOfst;
    u8                   u8DmaCh = HAL_BDMA_CH2;
    CamOsTsem_t          tBdmaDoneSem;

    memset(&tBdmaDoneSem, 0, sizeof(CamOsTsem_t));
    CamOsTsemInit(&tBdmaDoneSem, 0);
    memset(&tBdmaParam, 0, sizeof(hal_bdma_param));
    // tBdmaParam.ePathSel     = ((unsigned long)cfg->phyaddr_src < ARM_MIU1_BASE_ADDR) ? (HAL_BDMA_MIU0_TO_MIU0) :
    // (HAL_BDMA_MIU1_TO_MIU0); tBdmaParam.ePathSel     = ((unsigned long)cfg->phyaddr_dst < ARM_MIU1_BASE_ADDR) ?
    // tBdmaParam.ePathSel : tBdmaParam.ePathSel+1;
    tBdmaParam.ePathSel = HAL_BDMA_MIU0_TO_MIU0;

    // tBdmaParam.pSrcAddr     = ((unsigned long)cfg->phyaddr_src < ARM_MIU1_BASE_ADDR) ? (void *)((unsigned
    // long)cfg->phyaddr_src) : (void *)((unsigned long)cfg->phyaddr_src - ARM_MIU1_BASE_ADDR); tBdmaParam.pDstAddr =
    // ((unsigned long)cfg->phyaddr_dst < ARM_MIU1_BASE_ADDR) ? (void *)((unsigned long)cfg->phyaddr_dst) : (void
    // *)((unsigned long)cfg->phyaddr_dst - ARM_MIU1_BASE_ADDR);

    tBdmaParam.pSrcAddr = cfg->phyaddr_src;
    tBdmaParam.pDstAddr = cfg->phyaddr_dst;

    tBdmaParam.bIntMode     = 1;
    tBdmaParam.eDstAddrMode = HAL_BDMA_ADDR_INC;
    tBdmaParam.u32TxCount   = cfg->length;
    tBdmaParam.pfTxCbFunc   = msys_bdma_done;
    tBdmaParam.pTxCbParm    = (void *)&tBdmaDoneSem;
    tBdmaParam.u32Pattern   = 0;

    if (cfg->lineofst_src && cfg->lineofst_dst)
    {
        if ((cfg->lineofst_src < cfg->width_src) || (cfg->lineofst_dst < cfg->width_dst))
        {
            printk(
                "[BDMA] CAUTION: line offset is less than width\n"
                "width_src=0x%x  lineofst_src=0x%x, DstWidth=0x%x  lineofst_dst=0x%x\n",
                cfg->width_src, cfg->lineofst_src, cfg->width_dst, cfg->lineofst_dst);
            dump_stack();
            return -1;
        }

        tBdmaParam.pstLineOfst               = &tBdmaLineOfst;
        tBdmaParam.pstLineOfst->u32SrcWidth  = cfg->width_src;
        tBdmaParam.pstLineOfst->u32SrcOffset = cfg->lineofst_src;
        tBdmaParam.pstLineOfst->u32DstWidth  = cfg->width_dst;
        tBdmaParam.pstLineOfst->u32DstOffset = cfg->lineofst_dst;

        tBdmaParam.bEnLineOfst = 1;
    }
    else
    {
        tBdmaParam.bEnLineOfst = 0;
        tBdmaParam.pstLineOfst = NULL;
    }

    if (HAL_BDMA_PROC_DONE != hal_bdma_transfer(u8DmaCh, &tBdmaParam))
    {
        return -1;
    }

    if (tBdmaParam.bIntMode)
    {
        CamOsTsemDown(&tBdmaDoneSem);
    }
    CamOsTsemDeinit(&tBdmaDoneSem);

    return 0;
}
EXPORT_SYMBOL(msys_dma_copy_lineoffset);
#endif

#if defined(CONFIG_SSTAR_BDMA_BLIT_WRAPPER)
int msys_dma_blit(MSYS_DMA_BLIT *cfg)
{
    hal_bdma_param       tBdmaParam;
    hal_bdma_line_offset tBdmaLineOfst;
    u8                   u8DmaCh = HAL_BDMA_CH3;
    CamOsTsem_t          tBdmaDoneSem;

    memset(&tBdmaDoneSem, 0, sizeof(CamOsTsem_t));
    CamOsTsemInit(&tBdmaDoneSem, 0);
    memset(&tBdmaParam, 0, sizeof(hal_bdma_param));
    // tBdmaParam.ePathSel     = ((unsigned long)cfg->phyaddr_src < ARM_MIU1_BASE_ADDR) ? (HAL_BDMA_MIU0_TO_MIU0) :
    // (HAL_BDMA_MIU1_TO_MIU0); tBdmaParam.ePathSel     = ((unsigned long)cfg->phyaddr_dst < ARM_MIU1_BASE_ADDR) ?
    // tBdmaParam.ePathSel : tBdmaParam.ePathSel+1;
    tBdmaParam.ePathSel = HAL_BDMA_MIU0_TO_MIU0;

    // tBdmaParam.pSrcAddr     = ((unsigned long)cfg->phyaddr_src < ARM_MIU1_BASE_ADDR) ? (void *)((unsigned
    // long)cfg->phyaddr_src) : (void *)((unsigned long)cfg->phyaddr_src - ARM_MIU1_BASE_ADDR); tBdmaParam.pDstAddr =
    // ((unsigned long)cfg->phyaddr_dst < ARM_MIU1_BASE_ADDR) ? (void *)((unsigned long)cfg->phyaddr_dst) : (void
    // *)((unsigned long)cfg->phyaddr_dst - ARM_MIU1_BASE_ADDR);
    tBdmaParam.pSrcAddr = cfg->phyaddr_src;
    tBdmaParam.pDstAddr = cfg->phyaddr_dst;

    tBdmaParam.bIntMode     = 1;
    tBdmaParam.eDstAddrMode = HAL_BDMA_ADDR_INC;
    tBdmaParam.u32TxCount   = cfg->length;
    tBdmaParam.pfTxCbFunc   = msys_bdma_done;
    tBdmaParam.pTxCbParm    = (void *)&tBdmaDoneSem;
    tBdmaParam.u32Pattern   = 0;

    if (cfg->lineofst_src && cfg->lineofst_dst)
    {
        if ((cfg->lineofst_src < cfg->width_src) || (cfg->lineofst_dst < cfg->width_dst))
        {
            printk(
                "[BDMA] CAUTION: line offset is less than width\n"
                "width_src=0x%x  lineofst_src=0x%x, DstWidth=0x%x  lineofst_dst=0x%x\n",
                cfg->width_src, cfg->lineofst_src, cfg->width_dst, cfg->lineofst_dst);
            dump_stack();
            return -1;
        }

        tBdmaParam.pstLineOfst               = &tBdmaLineOfst;
        tBdmaParam.pstLineOfst->u32SrcWidth  = cfg->width_src;
        tBdmaParam.pstLineOfst->u32SrcOffset = cfg->lineofst_src;
        tBdmaParam.pstLineOfst->u32DstWidth  = cfg->width_dst;
        tBdmaParam.pstLineOfst->u32DstOffset = cfg->lineofst_dst;

        tBdmaParam.bEnLineOfst = 1;
    }
    else
    {
        tBdmaParam.bEnLineOfst = 0;
        tBdmaParam.pstLineOfst = NULL;
    }

    if (HAL_BDMA_PROC_DONE != hal_bdma_transfer(u8DmaCh, &tBdmaParam))
    {
        return -1;
    }

    if (tBdmaParam.bIntMode)
    {
        CamOsTsemDown(&tBdmaDoneSem);
    }
    CamOsTsemDeinit(&tBdmaDoneSem);

    return 0;
}
EXPORT_SYMBOL(msys_dma_blit);
#endif

static int __init ms_msys_dma_wrapper_init(void)
{
#if defined(CONFIG_SSTAR_BDMA)
    hal_bdma_initialize(0);
    hal_bdma_initialize(1);
    hal_bdma_initialize(2);
    hal_bdma_initialize(3);
    hal_bdma_initialize(8);
#endif
    return 0;
}
subsys_initcall(ms_msys_dma_wrapper_init)
