/*
 * hal_xzdec.c - Sigmastar
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

#include <drv_flash_os_impl.h>
#include <drv_part.h>
#include <hal_xzdec.h>

#define HAL_XZDEC_SXZ_HEADER_MAGIC          0x61747373 //"sstarXZ"
#define HAL_XZDEC_SXZ_HEADER_MAGIC_SIZE     7
#define HAL_XZDEC_SXZ_HEADER_BLOCK_NUM_SIZE 2
#define HAL_XZDEC_FOOTER_MAGIC              "SZ"
#define HAL_XZDEC_FOOTER_MAGIC_SIZE         2
#define HAL_XZDEC_XZ_HEADER_MAGIC           0x587A37FD //"\3757zXZ"
#define HAL_XZDEC_XZ_HEADER_MAGIC_SIZE      6
#define HAL_XZDEC_XZ_FOOTER_MAGIC           "YZ"
#define HAL_XZDEC_XZ_FOOTER_MAGIC_SIZE      2

#define HAL_XZDEC_READ_BYTE(_reg_) (*(volatile unsigned char *)(FLASH_IMPL_IO_ADDRESS(_reg_)))
#define HAL_XZDEC_WRITE_BYTE(_reg_, _val_) \
    ((*(volatile unsigned char *)(FLASH_IMPL_IO_ADDRESS(_reg_))) = (unsigned char)(_val_))
#define HAL_XZDEC_WRITE_BYTE_MASK(_reg_, _val_, mask)              \
    ((*(volatile unsigned char *)(FLASH_IMPL_IO_ADDRESS(_reg_))) = \
         ((*(volatile unsigned char *)(FLASH_IMPL_IO_ADDRESS(_reg_))) & ~(mask)) | ((unsigned char)(_val_) & (mask)))
#define HAL_XZDEC_READ_WORD(_reg_) (*(volatile unsigned short *)(FLASH_IMPL_IO_ADDRESS(_reg_)))
#define HAL_XZDEC_WRITE_WORD(_reg_, _val_) \
    ((*(volatile unsigned short *)(FLASH_IMPL_IO_ADDRESS(_reg_))) = (unsigned short)(_val_))
#define HAL_XZDEC_WRITE_WORD_MASK(_reg_, _val_, mask)                             \
    ((*(volatile unsigned short *)(FLASH_IMPL_IO_ADDRESS(_reg_))) =               \
         ((*(volatile unsigned short *)(FLASH_IMPL_IO_ADDRESS(_reg_))) & ~(mask)) \
         | ((unsigned short)(_val_) & (mask)))

#define HAL_XZDEC_CHANNEL_OFSSET(_channel_) (_channel_ << 7)

#define HAL_XZDEC_CTRL0(hal)                  ((hal->xzdec_bank) + ((0x00) << 2))
#define HAL_XZDEC_SW_RST(hal)                 ((hal->xzdec_bank) + ((0x01) << 2))
#define HAL_XZDEC_XZ_SIZE_L(hal)              ((hal->xzdec_bank) + ((0x02) << 2))
#define HAL_XZDEC_XZ_SIZE_H(hal)              ((hal->xzdec_bank) + ((0x03) << 2))
#define HAL_XZDEC_IRQ_MASK(hal)               ((hal->xzdec_bank) + ((0x10) << 2))
#define HAL_XZDEC_IRQ_FORCE(hal)              ((hal->xzdec_bank) + ((0x11) << 2))
#define HAL_XZDEC_IRQ_CLR(hal)                ((hal->xzdec_bank) + ((0x12) << 2))
#define HAL_XZDEC_IRQ_FINAL_STATUS(hal)       ((hal->xzdec_bank) + ((0x13) << 2))
#define HAL_XZDEC_IRQ_RAW_STATUS(hal)         ((hal->xzdec_bank) + ((0x14) << 2))
#define HAL_XZDEC_HEADER_CHECK_RESULT(hal)    ((hal->xzdec_bank) + ((0x1A) << 2))
#define HAL_XZDEC_DATA_CHECK_RESULT(hal)      ((hal->xzdec_bank) + ((0x1B) << 2))
#define HAL_XZDEC_SXZ_MAGIC_CHECK_RESULT(hal) ((hal->xzdec_bank) + ((0x1C) << 2))
#define HAL_XZDEC_FILTER_ID_ERROR(hal)        ((hal->xzdec_bank) + ((0x1C) << 2))
#define HAL_XZDEC_PROPERTIES_SIZE_ERROR(hal)  ((hal->xzdec_bank) + ((0x1D) << 2))
#define HAL_XZDEC_DICT_SIZE_ERROR(hal)        ((hal->xzdec_bank) + ((0x1D) << 2))
#define HAL_XZDEC_DICT_OVER_RANGE(hal)        ((hal->xzdec_bank) + ((0x1E) << 2))
#define HAL_XZDEC_CTRL_BYTE_ERROR(hal)        ((hal->xzdec_bank) + ((0x1E) << 2))
#define HAL_XZDEC_MEM_EN(hal)                 ((hal->xzdec_bank) + ((0x31) << 2))
#define HAL_XZDEC_BDMA_CLOCK_GATE_EN(hal)     ((hal->xzdec_bank) + ((0x32) << 2))

#define HAL_XZDEC_WDMA_CTRL0(hal)        ((hal->xzdec_sgw_bank) + ((0x00) << 2))
#define HAL_XZDEC_WDMA_CTRL1(hal)        ((hal->xzdec_sgw_bank) + ((0x01) << 2))
#define HAL_XZDEC_WDMA_WADDR_L(hal)      ((hal->xzdec_sgw_bank) + ((0x06) << 2))
#define HAL_XZDEC_WDMA_WADDR_H(hal)      ((hal->xzdec_sgw_bank) + ((0x07) << 2))
#define HAL_XZDEC_WDMA_WREQTH_WSIZE(hal) ((hal->xzdec_sgw_bank) + ((0x0A) << 2))
#define HAL_XZDEC_WDMA_WQOSTH(hal)       ((hal->xzdec_sgw_bank) + ((0x0B) << 2))

#define HAL_XZDEC_RDMA_CTRL0(hal)        ((hal->xzdec_sgr_bank) + ((0x00) << 2))
#define HAL_XZDEC_RDMA_CTRL1(hal)        ((hal->xzdec_sgr_bank) + ((0x01) << 2))
#define HAL_XZDEC_RDMA_RADDR_L(hal)      ((hal->xzdec_sgr_bank) + ((0x06) << 2))
#define HAL_XZDEC_RDMA_RADDR_H(hal)      ((hal->xzdec_sgr_bank) + ((0x07) << 2))
#define HAL_XZDEC_RDMA_RSIZE_L(hal)      ((hal->xzdec_sgr_bank) + ((0x08) << 2))
#define HAL_XZDEC_RDMA_RSIZE_H(hal)      ((hal->xzdec_sgr_bank) + ((0x09) << 2))
#define HAL_XZDEC_RDMA_RREQTH_RSIZE(hal) ((hal->xzdec_sgr_bank) + ((0x0A) << 2))
#define HAL_XZDEC_RDMA_RQOSTH(hal)       ((hal->xzdec_sgr_bank) + ((0x0B) << 2))
#define HAL_XZDEC_RDMA_CTRL14(hal)       ((hal->xzdec_sgr_bank) + ((0x14) << 2))

#define HAL_XZDEC2_WDMA_CTRL0(hal)        ((hal->xzdec2_sgw_bank) + ((0x00) << 2))
#define HAL_XZDEC2_WDMA_CTRL1(hal)        ((hal->xzdec2_sgw_bank) + ((0x01) << 2))
#define HAL_XZDEC2_WDMA_WADDR_L(hal)      ((hal->xzdec2_sgw_bank) + ((0x06) << 2))
#define HAL_XZDEC2_WDMA_WADDR_H(hal)      ((hal->xzdec2_sgw_bank) + ((0x07) << 2))
#define HAL_XZDEC2_WDMA_WREQTH_WSIZE(hal) ((hal->xzdec2_sgw_bank) + ((0x0A) << 2))
#define HAL_XZDEC2_WDMA_WQOSTH(hal)       ((hal->xzdec2_sgw_bank) + ((0x0B) << 2))

static void hal_xzdec_reset(struct hal_xzdec *hal)
{
    HAL_XZDEC_WRITE_WORD(HAL_XZDEC_SW_RST(hal), 0x7);
    FLASH_IMPL_USDELAY(1);
    HAL_XZDEC_WRITE_WORD(HAL_XZDEC_SW_RST(hal), 0x3);
    FLASH_IMPL_USDELAY(1);
    HAL_XZDEC_WRITE_WORD(HAL_XZDEC_SW_RST(hal), 0x0);
}

static s32 hal_xzdec_wait_done(struct hal_xzdec *hal, u8 core_num, u32 timeout_msec)
{
    ulong starttime = get_timer(0);

    while (get_timer(starttime) < timeout_msec)
    {
        if ((HAL_XZDEC_READ_WORD(HAL_XZDEC_IRQ_FINAL_STATUS(hal)) >> (core_num - 1)) & 0x1)
        {
            return HAL_XZDEC_SUCCESS;
        }
    }

    flash_impl_printf("[XZDEC] wait done timeout !!!\r\n");

    return -HAL_XZDEC_DONE_FAIL;
}

static s32 hal_xzdec_check_result(struct hal_xzdec *hal, hal_xzdec_compress_type type, u8 block_cnt)
{
    u8  result_shift = 0;
    u16 result       = 0;

    while (block_cnt--)
    {
        result_shift = block_cnt << 1;
        result       = (HAL_XZDEC_READ_WORD(HAL_XZDEC_HEADER_CHECK_RESULT(hal)) >> result_shift) & 0x3;

        if (result != 0x3)
        {
            flash_impl_printf("[XZDEC] check  header fail !!!\r\n");
            return -HAL_XZDEC_CHECK_HEADER_FAIL;
        }

        result = (HAL_XZDEC_READ_WORD(HAL_XZDEC_DATA_CHECK_RESULT(hal)) >> result_shift) & 0x3;

        if (result != 0x3)
        {
            flash_impl_printf("[XZDEC] check  data fail !!!\r\n");
            return -HAL_XZDEC_CHECK_DATA_FAIL;
        }
    }

    if (type == HAL_XZDEC_COMPRESS_SXZ)
    {
        if (0xF != (HAL_XZDEC_READ_WORD(HAL_XZDEC_SXZ_MAGIC_CHECK_RESULT(hal)) & 0xF))
        {
            flash_impl_printf("[XZDEC] check  sxz magic fail !!!\r\n");
            return -HAL_XZDEC_CHECK_SXZ_MAGIC_FAIL;
        }
    }

    return HAL_XZDEC_SUCCESS;
}

static s32 hal_xzdec_get_single_block_size(u8 *start, u8 *end, u32 *size)
{
    *size = 0;

    if (start > end)
        return -HAL_XZDEC_ERR_BLOCK_SIZE;

    *size = *end;

    while (end > start)
    {
        end--;
        *size = (*size << 7) | ((*end) & 0x7F);
    }

    return HAL_XZDEC_SUCCESS;
}

s32 hal_xzdec_decode(struct hal_xzdec *hal, hal_xzdec_data_src select, hal_xzdec_blocks_info *info, hal_xzdec_ops *ops)
{
    u8                      i;
    s32                     ret            = 0;
    u32                     src_size       = 0;
    u32                     align_buf_size = 0;
    u64                     align_buf_base = 0;
    u64                     align_dst_base = 0;
    struct flash_bdma_param param;

    src_size       = info->xz_file_size;
    align_buf_size = info->blocks[0].compress_align_size;
    align_buf_base = flash_impl_phys_to_miu(flash_impl_virt_to_phys((void *)(unsigned long)hal->xzdec_buf)) >> 4;
    align_dst_base = flash_impl_phys_to_miu(flash_impl_virt_to_phys((void *)(unsigned long)ops->dst)) >> 4;

    flash_impl_mem_invalidate((void *)(unsigned long)hal->xzdec_buf, src_size);
    flash_impl_mem_invalidate((void *)(unsigned long)ops->dst, info->xz_dec_size);

    HAL_XZDEC_WRITE_WORD_MASK(HAL_XZDEC_MEM_EN(hal), 0x0001, 0x0001);
    HAL_XZDEC_WRITE_WORD_MASK(HAL_XZDEC_BDMA_CLOCK_GATE_EN(hal), 0x0001, 0x0001);

    if (info->type == HAL_XZDEC_COMPRESS_XZ)
    {
        HAL_XZDEC_WRITE_WORD(HAL_XZDEC_WDMA_CTRL1(hal), 0x0800);
        HAL_XZDEC_WRITE_WORD(HAL_XZDEC_WDMA_WREQTH_WSIZE(hal), 0x300B);
        HAL_XZDEC_WRITE_WORD(HAL_XZDEC_WDMA_WQOSTH(hal), 0x000B);
        HAL_XZDEC_WRITE_WORD(HAL_XZDEC_WDMA_WADDR_L(hal), align_buf_base & 0xFFFF);
        HAL_XZDEC_WRITE_WORD(HAL_XZDEC_WDMA_WADDR_H(hal), (align_buf_base >> 16) & 0xFFFF);
        HAL_XZDEC_WRITE_WORD(HAL_XZDEC_WDMA_CTRL0(hal), 0x0011);

        HAL_XZDEC_WRITE_WORD(HAL_XZDEC_RDMA_CTRL0(hal), 0x0000);
        HAL_XZDEC_WRITE_WORD(HAL_XZDEC_RDMA_CTRL1(hal), 0x0800);
        HAL_XZDEC_WRITE_WORD(HAL_XZDEC_RDMA_RREQTH_RSIZE(hal), 0x300B);
        HAL_XZDEC_WRITE_WORD(HAL_XZDEC_RDMA_RQOSTH(hal), 0x000B);
        HAL_XZDEC_WRITE_WORD(HAL_XZDEC_RDMA_RADDR_L(hal), align_buf_base & 0xFFFF);
        HAL_XZDEC_WRITE_WORD(HAL_XZDEC_RDMA_RADDR_H(hal), (align_buf_base >> 16) & 0xFFFF);
        HAL_XZDEC_WRITE_WORD(HAL_XZDEC_RDMA_RSIZE_L(hal), align_buf_size & 0xFFFF);
        HAL_XZDEC_WRITE_WORD(HAL_XZDEC_RDMA_RSIZE_H(hal), (align_buf_size >> 16) & 0xFFFF);
        HAL_XZDEC_WRITE_WORD(HAL_XZDEC_RDMA_CTRL14(hal), 0x0010);
        HAL_XZDEC_WRITE_WORD(HAL_XZDEC_RDMA_CTRL0(hal), 0x0011);

        HAL_XZDEC_WRITE_WORD(HAL_XZDEC2_WDMA_CTRL1(hal), 0x0800);
        HAL_XZDEC_WRITE_WORD(HAL_XZDEC2_WDMA_WREQTH_WSIZE(hal), 0x300B);
        HAL_XZDEC_WRITE_WORD(HAL_XZDEC2_WDMA_WQOSTH(hal), 0x000B);
        HAL_XZDEC_WRITE_WORD(HAL_XZDEC2_WDMA_WADDR_L(hal), align_dst_base & 0xFFFF);
        HAL_XZDEC_WRITE_WORD(HAL_XZDEC2_WDMA_WADDR_H(hal), (align_dst_base >> 16) & 0xFFFF);
        HAL_XZDEC_WRITE_WORD(HAL_XZDEC2_WDMA_CTRL0(hal), 0x0011);

        HAL_XZDEC_WRITE_WORD(HAL_XZDEC_IRQ_MASK(hal), 0x00FE);
        HAL_XZDEC_WRITE_WORD(HAL_XZDEC_XZ_SIZE_L(hal), src_size & 0xFFFF);
        HAL_XZDEC_WRITE_WORD(HAL_XZDEC_XZ_SIZE_H(hal), (src_size >> 16) & 0xFFFF);
        HAL_XZDEC_WRITE_WORD(HAL_XZDEC_CTRL0(hal), 0x0007);
    }
    else if (info->type == HAL_XZDEC_COMPRESS_SXZ)
    {
        for (i = 0; i < info->block_cnt; i++)
        {
            align_buf_size = info->blocks[i].compress_align_size;

            HAL_XZDEC_WRITE_WORD(HAL_XZDEC_WDMA_CTRL1(hal) + HAL_XZDEC_CHANNEL_OFSSET(i), 0x0800 | (i << 12));
            HAL_XZDEC_WRITE_WORD(HAL_XZDEC_WDMA_WREQTH_WSIZE(hal) + HAL_XZDEC_CHANNEL_OFSSET(i), 0x300B);
            HAL_XZDEC_WRITE_WORD(HAL_XZDEC_WDMA_WQOSTH(hal) + HAL_XZDEC_CHANNEL_OFSSET(i), 0x000B);
            HAL_XZDEC_WRITE_WORD(HAL_XZDEC_WDMA_WADDR_L(hal) + HAL_XZDEC_CHANNEL_OFSSET(i), align_buf_base & 0xFFFF);
            HAL_XZDEC_WRITE_WORD(HAL_XZDEC_WDMA_WADDR_H(hal) + HAL_XZDEC_CHANNEL_OFSSET(i),
                                 (align_buf_base >> 16) & 0xFFFF);
            HAL_XZDEC_WRITE_WORD(HAL_XZDEC_WDMA_CTRL0(hal) + HAL_XZDEC_CHANNEL_OFSSET(i), 0x0011);

            HAL_XZDEC_WRITE_WORD(HAL_XZDEC_RDMA_CTRL0(hal) + HAL_XZDEC_CHANNEL_OFSSET(i), 0x0000);
            HAL_XZDEC_WRITE_WORD(HAL_XZDEC_RDMA_CTRL1(hal) + HAL_XZDEC_CHANNEL_OFSSET(i), 0x0800 | (i << 12));
            HAL_XZDEC_WRITE_WORD(HAL_XZDEC_RDMA_RREQTH_RSIZE(hal) + HAL_XZDEC_CHANNEL_OFSSET(i), 0x300B);
            HAL_XZDEC_WRITE_WORD(HAL_XZDEC_RDMA_RQOSTH(hal) + HAL_XZDEC_CHANNEL_OFSSET(i), 0x000B);
            HAL_XZDEC_WRITE_WORD(HAL_XZDEC_RDMA_RADDR_L(hal) + HAL_XZDEC_CHANNEL_OFSSET(i), align_buf_base & 0xFFFF);
            HAL_XZDEC_WRITE_WORD(HAL_XZDEC_RDMA_RADDR_H(hal) + HAL_XZDEC_CHANNEL_OFSSET(i),
                                 (align_buf_base >> 16) & 0xFFFF);
            HAL_XZDEC_WRITE_WORD(HAL_XZDEC_RDMA_RSIZE_L(hal) + HAL_XZDEC_CHANNEL_OFSSET(i), align_buf_size & 0xFFFF);
            HAL_XZDEC_WRITE_WORD(HAL_XZDEC_RDMA_RSIZE_H(hal) + HAL_XZDEC_CHANNEL_OFSSET(i),
                                 (align_buf_size >> 16) & 0xFFFF);
            HAL_XZDEC_WRITE_WORD(HAL_XZDEC_RDMA_CTRL14(hal) + HAL_XZDEC_CHANNEL_OFSSET(i), 0x0010);
            HAL_XZDEC_WRITE_WORD(HAL_XZDEC_RDMA_CTRL0(hal) + HAL_XZDEC_CHANNEL_OFSSET(i), 0x0011);

            HAL_XZDEC_WRITE_WORD(HAL_XZDEC2_WDMA_CTRL1(hal) + HAL_XZDEC_CHANNEL_OFSSET(i), 0x0800 | (i << 12));
            HAL_XZDEC_WRITE_WORD(HAL_XZDEC2_WDMA_WREQTH_WSIZE(hal) + HAL_XZDEC_CHANNEL_OFSSET(i), 0x300B);
            HAL_XZDEC_WRITE_WORD(HAL_XZDEC2_WDMA_WQOSTH(hal) + HAL_XZDEC_CHANNEL_OFSSET(i), 0x000B);
            HAL_XZDEC_WRITE_WORD(HAL_XZDEC2_WDMA_WADDR_L(hal) + HAL_XZDEC_CHANNEL_OFSSET(i), align_dst_base & 0xFFFF);
            HAL_XZDEC_WRITE_WORD(HAL_XZDEC2_WDMA_WADDR_H(hal) + HAL_XZDEC_CHANNEL_OFSSET(i),
                                 (align_dst_base >> 16) & 0xFFFF);
            HAL_XZDEC_WRITE_WORD(HAL_XZDEC2_WDMA_CTRL0(hal) + HAL_XZDEC_CHANNEL_OFSSET(i), 0x0011);

            align_buf_base += align_buf_size;
            align_dst_base += info->blocks[i].uncompress_align_size;
        }

        HAL_XZDEC_WRITE_WORD(HAL_XZDEC_IRQ_MASK(hal), 0x00FF & (~(0x1 << (info->block_cnt - 1))));
        HAL_XZDEC_WRITE_WORD(HAL_XZDEC_CTRL0(hal), 0x0003);
    }

    switch (select)
    {
        case HAL_XZDEC_DATA_SRC_MIU:
            flash_impl_mem_flush((void *)(unsigned long)ops->src, src_size);
            flash_impl_miupipe_flush();
            param.path         = FLASH_BDMA_MIU2XZDEC;
            param.src          = (u8 *)(unsigned long)ops->src;
            param.dst          = NULL;
            param.size         = src_size;
            param.interrupt_en = 0;
            if (FLASH_BDMA_SUCCESS != flash_impl_bdma_transfer(&param))
                ret = -HAL_XZDEC_DEVICE_FAILURE;
            break;
        case HAL_XZDEC_DATA_SRC_SPI:
#if defined(CONFIG_SSTAR_NAND) || defined(CONFIG_SSTAR_NOR)
            if (src_size != sstar_part_load_to_xzdec(ops->src_part, (u32)ops->src, NULL, src_size))
                ret = -HAL_XZDEC_DEVICE_FAILURE;
#else
            ret = -HAL_XZDEC_DEVICE_FAILURE;
#endif
            break;
    }

    flash_impl_mem_invalidate((void *)(unsigned long)ops->dst, info->xz_dec_size);

    if (0 != ret)
        goto out;

    if (hal->calbak_xzdec_waitdone)
    {
        if (0 != hal->calbak_xzdec_waitdone(hal))
            ret = -HAL_XZDEC_DONE_FAIL;
        else
            ret = HAL_XZDEC_SUCCESS;
    }
    else
    {
        ret = hal_xzdec_wait_done(hal, info->block_cnt, HAL_XZDEC_WAIT_TIMEOUT_MS);
    }

    if (HAL_XZDEC_SUCCESS != ret)
        goto out;

    ret = hal_xzdec_check_result(hal, info->type, info->block_cnt);

out:

    HAL_XZDEC_WRITE_WORD_MASK(HAL_XZDEC_MEM_EN(hal), 0x0000, 0x0001);
    HAL_XZDEC_WRITE_WORD_MASK(HAL_XZDEC_BDMA_CLOCK_GATE_EN(hal), 0x0000, 0x0001);

    hal_xzdec_reset(hal);

    return ret;
}

s32 hal_xzdec_get_blocks_info(void *xz_head, hal_xzdec_blocks_info *info)
{
    int i     = 0;
    int num   = 0;
    u8 *start = NULL;
    u8 *end   = NULL;

    if (*(u32 *)xz_head == HAL_XZDEC_SXZ_HEADER_MAGIC)
        info->type = HAL_XZDEC_COMPRESS_SXZ;
    else if (*(u32 *)xz_head == HAL_XZDEC_XZ_HEADER_MAGIC)
        info->type = HAL_XZDEC_COMPRESS_XZ;
    else
    {
        return -HAL_XZDEC_ERR_MAGIC;
    }

    if (info->type == HAL_XZDEC_COMPRESS_SXZ)
    {
        info->block_cnt = *(u8 *)(xz_head + HAL_XZDEC_SXZ_HEADER_MAGIC_SIZE);
    }
    else if (info->type == HAL_XZDEC_COMPRESS_XZ)
    {
        info->block_cnt = 1;
    }

    if (info->type == HAL_XZDEC_COMPRESS_SXZ)
    {
        start                 = xz_head + HAL_XZDEC_SXZ_HEADER_MAGIC_SIZE + HAL_XZDEC_SXZ_HEADER_BLOCK_NUM_SIZE;
        end                   = start;
        info->xz_dec_buf_size = 0;
        info->xz_dec_size     = 0;
        for (i = 0; i < (info->block_cnt * 2); i++) // for sxz, compsize0,uncom size0,compsize1,uncom size1,...
        {
            while (*end >> 7 != 0) // Variable length encoding
            {
                end++;
            }

            num = i >> 1; // block number

            if (i & 0x1)
            {
                if (HAL_XZDEC_SUCCESS
                    != hal_xzdec_get_single_block_size(start, end, &(info->blocks[num].uncompress_size)))
                {
                    return -HAL_XZDEC_ERR_BLOCK_SIZE;
                }

                info->blocks[num].uncompress_align_size = info->blocks[num].uncompress_size >> 4;
                info->xz_dec_size += info->blocks[num].uncompress_size;
            }
            else
            {
                if (HAL_XZDEC_SUCCESS
                    != hal_xzdec_get_single_block_size(start, end, &(info->blocks[num].compress_size)))
                {
                    return -HAL_XZDEC_ERR_BLOCK_SIZE;
                }

                info->blocks[num].compress_align_size = ((info->blocks[num].compress_size + 0x1F) >> 5) << 1;
                info->xz_dec_buf_size += info->blocks[num].compress_size;
            }

            end++;
            start = end;
        }
    }
    else if (info->type == HAL_XZDEC_COMPRESS_XZ)
    {
        info->blocks[0].compress_size       = info->xz_file_size;
        info->blocks[0].compress_align_size = ((info->blocks[0].compress_size + 0x1F) >> 5) << 1;
        info->xz_dec_buf_size               = info->xz_file_size;
        info->xz_dec_size                   = info->xz_file_size;
    }

    return HAL_XZDEC_SUCCESS;
}
