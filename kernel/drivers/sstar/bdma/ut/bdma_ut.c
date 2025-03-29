/*
 * bdma_ut.c - Sigmastar
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

#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <cam_os_wrapper.h>
#include <mdrv_msys_io_st.h>
#include <ms_msys.h>
#include <hal_bdma.h>

static u32 polynomial = 0x04C11DB7;
#define BDMA_UT_TEST_SIZE (0x1000)

static MSYS_DMEM_INFO bdma_ut_src_dmem;
static MSYS_DMEM_INFO bdma_ut_dst_dmem;
static void *         bdma_ut_src = NULL;
static void *         bdma_ut_dst = NULL;
static dma_addr_t     bdma_ut_src_addr;
static dma_addr_t     bdma_ut_dst_addr;

static void *alloc_dmem(const char *name, unsigned int size, MSYS_DMEM_INFO *dmem, dma_addr_t *addr)
{
    memcpy(dmem->name, name, strlen(name) + 1);
    dmem->length = size;
    if (0 != msys_request_dmem(dmem))
    {
        return NULL;
    }
    *addr = dmem->phys;
    return (void *)((uintptr_t)dmem->kvirt);
}

static void free_dmem(MSYS_DMEM_INFO *dmem)
{
    msys_release_dmem(dmem);
}

u8 bit_change(u8 data)
{
    data = (data << 4) | (data >> 4);
    data = ((data << 2) & 0xcc) | ((data >> 2) & 0x33);
    data = ((data << 2) & 0xaa) | ((data >> 2) & 0x55);

    return data;
}

u32 bdma_sw_crc(u8 flag, u32 crc, u32 poly, u8 *buf, u32 size)
{
    u8  data;
    u32 crc_out;
    u32 crc_poly;
    u32 i, j;

    crc_out  = crc;
    crc_poly = poly;
    for (j = 0; j < size; j = j + 1)
    {
        data = buf[j];
        data = flag ? bit_change(data) : data;
        for (i = 0; i <= 7; i = i + 1)
        {
            if (((crc_out >> 31) ^ ((data >> i) & 0x1)) & 0x1)
            {
                crc_out = ((crc_out << 1) >> 1) ^ (crc_poly >> 1);
                crc_out = (crc_out << 1) | 0x1;
            }
            else
            {
                crc_out = (crc_out << 1);
            }
        }
    }

    return crc_out;
}

static void sstar_bdma_ut_callback(void *pdata)
{
    CamOsTsem_t *ptBdmaDoneSem = (CamOsTsem_t *)pdata;
    CamOsTsemUp(ptBdmaDoneSem);
}

extern void get_random_bytes(void *buf, int nbytes);

static void sstar_bdma_ut_crc(u32 length, u8 u8DmaCh)
{
    u32            crc = 0;
    hal_bdma_param tBdmaParam;
    hal_bdma_crc   tBdmaCrcst;
    CamOsTsem_t    tBdmaDoneSem;

    get_random_bytes((void *)bdma_ut_dst, length);
    memset(bdma_ut_dst, 0, BDMA_UT_TEST_SIZE);
    hal_bdma_initialize(u8DmaCh);

    memset(&tBdmaDoneSem, 0, sizeof(CamOsTsem_t));
    CamOsTsemInit(&tBdmaDoneSem, 0);
    memset(&tBdmaParam, 0, sizeof(hal_bdma_param));
    memset(&tBdmaCrcst, 0, sizeof(hal_bdma_crc));

    tBdmaParam.ePathSel             = HAL_BDMA_MIU0_TO_CRC;
    tBdmaParam.pSrcAddr             = CamOsMemPhysToMiu(bdma_ut_src_addr);
    tBdmaParam.pDstAddr             = 0;
    tBdmaParam.bIntMode             = 1;
    tBdmaParam.eDstAddrMode         = HAL_BDMA_ADDR_INC;
    tBdmaParam.u32TxCount           = length;
    tBdmaParam.pfTxCbFunc           = sstar_bdma_ut_callback;
    tBdmaParam.pTxCbParm            = (void *)&tBdmaDoneSem;
    tBdmaParam.pstCrcst             = &tBdmaCrcst;
    tBdmaParam.pstCrcst->reflection = 1;
    tBdmaParam.pstCrcst->polynomial = polynomial;

    if (HAL_BDMA_PROC_DONE != hal_bdma_transfer(u8DmaCh, &tBdmaParam))
    {
        printk("BDMA[%d] hal_bdma_transfer fail, memcpy test fail!!!\n", u8DmaCh);
        return;
    }

    CamOsTsemDown(&tBdmaDoneSem);
    CamOsTsemDeinit(&tBdmaDoneSem);
    tBdmaCrcst.seedvalue = tBdmaParam.pstCrcst->seedvalue;

    crc = bdma_sw_crc(0, 0x0, polynomial, (void *)bdma_ut_src_addr, length);

    if (tBdmaCrcst.seedvalue == crc)
    {
        printk("bdma crc pass \n");
    }
    else
    {
        printk("crc value: 0x%x\n", tBdmaCrcst.seedvalue);
        printk("cal_crc value: 0x%x\n", crc);
        printk("bdma crc fail \n");
    }

    return;
}

static void sstar_bdma_ut_search(u8 u8DmaCh)
{
    hal_bdma_param  tBdmaParam;
    hal_bdma_search tBdmaSearvhst;
    CamOsTsem_t     tBdmaDoneSem;
    u32             i;
    u32             patternaddr;

    for (i = 0; i < BDMA_UT_TEST_SIZE; i++)
        ((u8 *)bdma_ut_src)[i] = i;

    hal_bdma_initialize(u8DmaCh);

    CamOsTsemInit(&tBdmaDoneSem, 0);
    memset(&tBdmaParam, 0, sizeof(hal_bdma_param));
    memset(&tBdmaSearvhst, 0, sizeof(hal_bdma_search));

    tBdmaParam.ePathSel                 = HAL_BDMA_MIU0_SEARCH;
    tBdmaParam.bIntMode                 = 1;
    tBdmaParam.eDstAddrMode             = HAL_BDMA_ADDR_INC;
    tBdmaParam.u32TxCount               = BDMA_UT_TEST_SIZE;
    tBdmaParam.pSrcAddr                 = CamOsMemPhysToMiu(bdma_ut_src_addr);
    tBdmaParam.pDstAddr                 = 0;
    tBdmaParam.pfTxCbFunc               = sstar_bdma_ut_callback;
    tBdmaParam.pTxCbParm                = (void *)&tBdmaDoneSem;
    tBdmaParam.u32Pattern               = 0x14131211;
    tBdmaParam.pstSearchst              = &tBdmaSearvhst;
    tBdmaParam.pstSearchst->antimatch   = 0x0;
    tBdmaParam.pstSearchst->patternmask = 0x0;

    if (HAL_BDMA_PROC_DONE != hal_bdma_transfer(u8DmaCh, &tBdmaParam))
    {
        return;
    }

    if (tBdmaParam.bIntMode)
    {
        CamOsTsemDown(&tBdmaDoneSem);
    }

    CamOsTsemDeinit(&tBdmaDoneSem);

    patternaddr = tBdmaParam.pstSearchst->patternaddr;
    if (patternaddr == 0x0)
        return;

    return;
}

static void sstar_bdma_ut_memcpy(u8 u8DmaCh)
{
    hal_bdma_param tBdmaParam;
    CamOsTsem_t    tBdmaDoneSem;
    u32            i;

    for (i = 0; i < BDMA_UT_TEST_SIZE; i++)
        ((u8 *)bdma_ut_src)[i] = i;

    memset(bdma_ut_dst, 0, BDMA_UT_TEST_SIZE);

    hal_bdma_initialize(u8DmaCh);
    memset(&tBdmaDoneSem, 0, sizeof(CamOsTsem_t));
    CamOsTsemInit(&tBdmaDoneSem, 0);
    memset(&tBdmaParam, 0, sizeof(hal_bdma_param));
    tBdmaParam.ePathSel     = HAL_BDMA_MIU0_TO_MIU0;
    tBdmaParam.pSrcAddr     = CamOsMemPhysToMiu(bdma_ut_src_addr);
    tBdmaParam.pDstAddr     = CamOsMemPhysToMiu(bdma_ut_dst_addr);
    tBdmaParam.bIntMode     = 1;
    tBdmaParam.eDstAddrMode = HAL_BDMA_ADDR_INC;
    tBdmaParam.u32TxCount   = BDMA_UT_TEST_SIZE;
    tBdmaParam.pfTxCbFunc   = sstar_bdma_ut_callback;
    tBdmaParam.pTxCbParm    = (void *)&tBdmaDoneSem;
    tBdmaParam.u32Pattern   = 0;

    if (HAL_BDMA_PROC_DONE != hal_bdma_transfer(u8DmaCh, &tBdmaParam))
    {
        printk("BDMA[%d] hal_bdma_transfer fail, memcpy test fail!!!\n", u8DmaCh);
        return;
    }

    CamOsTsemDown(&tBdmaDoneSem);
    CamOsTsemDeinit(&tBdmaDoneSem);

    if (!memcmp(bdma_ut_src, bdma_ut_dst, BDMA_UT_TEST_SIZE))
    {
        // printk("BDMA[%d] memcpy test pass\n", u8DmaCh);
    }
    else
    {
        printk("BDMA[%d] memcpy test fail!!!\n", u8DmaCh);
        printk("bdma_ut_src buf:\n");
        // CamOsHexdump(bdma_ut_src, BDMA_UT_TEST_SIZE);
        printk("bdma_ut_dst buf:\n");
        // CamOsHexdump(bdma_ut_dst, BDMA_UT_TEST_SIZE);
    }
}

static void sstar_bdma_ut_memcpy_lineoffset(u8 u8DmaCh)
{
    hal_bdma_param       tBdmaParam;
    hal_bdma_line_offset tBdmaLineOfst;
    CamOsTsem_t          tBdmaDoneSem;
    u32                  i;

    for (i = 0; i < 1024; i++)
        if ((i & 0xF) < 12)
            ((u32 *)bdma_ut_src)[i] = 0x1234abcd;
        else
            ((u32 *)bdma_ut_src)[i] = 0x0;

    memset(bdma_ut_dst, 0, BDMA_UT_TEST_SIZE);

    hal_bdma_initialize(u8DmaCh);
    memset(&tBdmaDoneSem, 0, sizeof(CamOsTsem_t));
    CamOsTsemInit(&tBdmaDoneSem, 0);
    memset(&tBdmaParam, 0, sizeof(hal_bdma_param));
    tBdmaParam.ePathSel                  = HAL_BDMA_MIU0_TO_MIU0;
    tBdmaParam.pSrcAddr                  = CamOsMemPhysToMiu(bdma_ut_src_addr);
    tBdmaParam.pDstAddr                  = CamOsMemPhysToMiu(bdma_ut_dst_addr);
    tBdmaParam.bIntMode                  = 1;
    tBdmaParam.eDstAddrMode              = HAL_BDMA_ADDR_INC;
    tBdmaParam.u32TxCount                = BDMA_UT_TEST_SIZE;
    tBdmaParam.pfTxCbFunc                = sstar_bdma_ut_callback;
    tBdmaParam.pTxCbParm                 = (void *)&tBdmaDoneSem;
    tBdmaParam.u32Pattern                = 0;
    tBdmaParam.pstLineOfst               = &tBdmaLineOfst;
    tBdmaParam.pstLineOfst->u32SrcWidth  = 48;
    tBdmaParam.pstLineOfst->u32SrcOffset = 64;
    tBdmaParam.pstLineOfst->u32DstWidth  = 48;
    tBdmaParam.pstLineOfst->u32DstOffset = 64;
    tBdmaParam.bEnLineOfst               = 1;

    if (HAL_BDMA_PROC_DONE != hal_bdma_transfer(u8DmaCh, &tBdmaParam))
    {
        printk("BDMA[%d] hal_bdma_transfer fail, memcpy_lineoffset test fail!!!\n", u8DmaCh);
        return;
    }

    CamOsTsemDown(&tBdmaDoneSem);
    CamOsTsemDeinit(&tBdmaDoneSem);

    if (!memcmp(bdma_ut_src, bdma_ut_dst, BDMA_UT_TEST_SIZE))
    {
        // printk("BDMA[%d] memcpy_lineoffset test pass\n", u8DmaCh);
    }
    else
    {
        printk("BDMA[%d] memcpy_lineoffset test fail!!!\n", u8DmaCh);
        printk("bdma_ut_src buf:\n");
        CamOsHexdump(bdma_ut_src, BDMA_UT_TEST_SIZE);
        printk("bdma_ut_dst buf:\n");
        CamOsHexdump(bdma_ut_dst, BDMA_UT_TEST_SIZE);
    }
}

static void sstar_bdma_ut_pattern_fill(u8 u8DmaCh)
{
    hal_bdma_param tBdmaParam;
    CamOsTsem_t    tBdmaDoneSem;
    u32            i;

    for (i = 0; i < 1024; i++)
        ((u32 *)bdma_ut_src)[i] = 0x1234abcd;

    memset(bdma_ut_dst, 0, BDMA_UT_TEST_SIZE);

    hal_bdma_initialize(u8DmaCh);
    memset(&tBdmaDoneSem, 0, sizeof(CamOsTsem_t));
    CamOsTsemInit(&tBdmaDoneSem, 0);
    memset(&tBdmaParam, 0, sizeof(hal_bdma_param));
    tBdmaParam.ePathSel     = HAL_BDMA_MEM_TO_MIU0;
    tBdmaParam.pSrcAddr     = 0;
    tBdmaParam.pDstAddr     = CamOsMemPhysToMiu(bdma_ut_dst_addr);
    tBdmaParam.bIntMode     = 1;
    tBdmaParam.eDstAddrMode = HAL_BDMA_ADDR_INC;
    tBdmaParam.u32TxCount   = BDMA_UT_TEST_SIZE;
    tBdmaParam.pfTxCbFunc   = sstar_bdma_ut_callback;
    tBdmaParam.pTxCbParm    = (void *)&tBdmaDoneSem;
    tBdmaParam.u32Pattern   = 0x1234abcd;

    if (HAL_BDMA_PROC_DONE != hal_bdma_transfer(u8DmaCh, &tBdmaParam))
    {
        printk("BDMA[%d] hal_bdma_transfer fail, pattern_fill test fail!!!\n", u8DmaCh);
        return;
    }

    CamOsTsemDown(&tBdmaDoneSem);
    CamOsTsemDeinit(&tBdmaDoneSem);

    if (!memcmp(bdma_ut_src, bdma_ut_dst, BDMA_UT_TEST_SIZE))
    {
        // printk("BDMA[%d] pattern_fill test pass\n", u8DmaCh);
    }
    else
    {
        printk("BDMA[%d] pattern_fill test fail!!!\n", u8DmaCh);
        printk("bdma_ut_src buf:\n");
        // CamOsHexdump(bdma_ut_src, BDMA_UT_TEST_SIZE);
        printk("bdma_ut_dst buf:\n");
        // CamOsHexdump(bdma_ut_dst, BDMA_UT_TEST_SIZE);
    }
}

static void sstar_bdma_ut_pattern_fill_lineoffset(u8 u8DmaCh)
{
    hal_bdma_param       tBdmaParam;
    hal_bdma_line_offset tBdmaLineOfst;
    CamOsTsem_t          tBdmaDoneSem;
    u32                  i;

    for (i = 0; i < 1024; i++)
        if ((i & 0xF) < 12)
            ((u32 *)bdma_ut_src)[i] = 0x1234abcd;
        else
            ((u32 *)bdma_ut_src)[i] = 0x0;

    memset(bdma_ut_dst, 0, BDMA_UT_TEST_SIZE);

    hal_bdma_initialize(u8DmaCh);
    memset(&tBdmaDoneSem, 0, sizeof(CamOsTsem_t));
    CamOsTsemInit(&tBdmaDoneSem, 0);
    memset(&tBdmaParam, 0, sizeof(hal_bdma_param));
    tBdmaParam.ePathSel                  = HAL_BDMA_MEM_TO_MIU0;
    tBdmaParam.pSrcAddr                  = 0;
    tBdmaParam.pDstAddr                  = CamOsMemPhysToMiu(bdma_ut_dst_addr);
    tBdmaParam.bIntMode                  = 1;
    tBdmaParam.eDstAddrMode              = HAL_BDMA_ADDR_INC;
    tBdmaParam.u32TxCount                = BDMA_UT_TEST_SIZE;
    tBdmaParam.pfTxCbFunc                = sstar_bdma_ut_callback;
    tBdmaParam.pTxCbParm                 = (void *)&tBdmaDoneSem;
    tBdmaParam.u32Pattern                = 0x1234abcd;
    tBdmaParam.pstLineOfst               = &tBdmaLineOfst;
    tBdmaParam.pstLineOfst->u32SrcWidth  = 48;
    tBdmaParam.pstLineOfst->u32SrcOffset = 64;
    tBdmaParam.pstLineOfst->u32DstWidth  = 48;
    tBdmaParam.pstLineOfst->u32DstOffset = 64;
    tBdmaParam.bEnLineOfst               = 1;

    if (HAL_BDMA_PROC_DONE != hal_bdma_transfer(u8DmaCh, &tBdmaParam))
    {
        printk("BDMA[%d] hal_bdma_transfer fail, pattern_fill_lineoffset test fail!!!\n", u8DmaCh);
        return;
    }

    CamOsTsemDown(&tBdmaDoneSem);
    CamOsTsemDeinit(&tBdmaDoneSem);

    if (!memcmp(bdma_ut_src, bdma_ut_dst, BDMA_UT_TEST_SIZE))
    {
        // printk("BDMA[%d] pattern_fill_lineoffset test pass\n", u8DmaCh);
    }
    else
    {
        printk("BDMA[%d] pattern_fill_lineoffset test fail!!!\n", u8DmaCh);
        printk("bdma_ut_src buf:\n");
        CamOsHexdump(bdma_ut_src, BDMA_UT_TEST_SIZE);
        printk("bdma_ut_dst buf:\n");
        CamOsHexdump(bdma_ut_dst, BDMA_UT_TEST_SIZE);
    }
}

static int __init sstar_bdma_ut_init(void)
{
    u16 i       = 0;
    u8  u8DmaCh = HAL_BDMA_CH0;

    if (!(bdma_ut_src = alloc_dmem("bdma_ut_test_src", BDMA_UT_TEST_SIZE, &bdma_ut_src_dmem, &bdma_ut_src_addr)))
    {
        printk("BDMA[all] alloc bdma_ut_test_src fail!!!\n");
        return 0;
    }

    if (!(bdma_ut_dst = alloc_dmem("bdma_ut_test_dst", BDMA_UT_TEST_SIZE, &bdma_ut_dst_dmem, &bdma_ut_dst_addr)))
    {
        printk("BDMA[all] alloc bdma_ut_test_dst fail!!!\n");
        return 0;
    }

    while (i < 0x10000)
    {
        while (u8DmaCh < HAL_BDMA_CH_NUM)
        {
            sstar_bdma_ut_memcpy(u8DmaCh);
            // sstar_bdma_ut_memcpy_lineoffset(u8DmaCh);
            sstar_bdma_ut_pattern_fill(u8DmaCh);
            // sstar_bdma_ut_pattern_fill_lineoffset(u8DmaCh);
            u8DmaCh++;
        }
        i++;
        u8DmaCh = 0;
    }

    free_dmem(&bdma_ut_src_dmem);
    free_dmem(&bdma_ut_dst_dmem);

    return 0;
}

static void __exit sstar_bdma_ut_exit(void) {}

module_init(sstar_bdma_ut_init);
module_exit(sstar_bdma_ut_exit);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("Sigmastar Bdma Driver UT");
MODULE_LICENSE("GPL");
MODULE_ALIAS("sstar-bdma-ut");
