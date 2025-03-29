/*
 * mdrv_spinor.c- Sigmastar
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

#include <drv_flash_os_impl.h>
#include <mdrv_spinor.h>

static u8 drv_spinor_complete_read_status(struct spinor_handle *handle, u8 cmd, u8 *pstatus, u8 size)
{
    handle->msg.buffer         = pstatus;
    handle->msg.size           = size;
    handle->msg.command        = SPIFLASH_COMMAND_READ_STATUS;
    handle->msg.cmd.cmd        = cmd;
    handle->msg.cmd.address    = 0;
    handle->msg.cmd.addr_bytes = 0;
    handle->msg.cmd.dummy      = 0;

    if (0 != spiflash_transfer(handle->ctrl_id, &handle->msg))
    {
        return ERR_SPINOR_DEVICE_FAILURE;
    }

    return ERR_SPINOR_SUCCESS;
}

static u8 drv_spinor_complete_write_status(struct spinor_handle *handle, u8 cmd, u8 *pstatus, u8 size)
{
    handle->msg.buffer         = pstatus;
    handle->msg.size           = size;
    handle->msg.command        = SPIFLASH_COMMAND_WRITE_STATUS;
    handle->msg.cmd.cmd        = cmd;
    handle->msg.cmd.address    = 0;
    handle->msg.cmd.addr_bytes = 0;
    handle->msg.cmd.dummy      = 0;

    if (0 != spiflash_transfer(handle->ctrl_id, &handle->msg))
    {
        return ERR_SPINOR_DEVICE_FAILURE;
    }

    return ERR_SPINOR_SUCCESS;
}

static u8 drv_spinor_complete_program_page(struct spinor_handle *handle, u8 cmd, u32 address, u8 *data, u32 size)
{
    handle->msg.buffer         = data;
    handle->msg.size           = size;
    handle->msg.command        = SPIFLASH_COMMAND_PROGRAM;
    handle->msg.cmd.cmd        = cmd;
    handle->msg.cmd.address    = address;
    handle->msg.cmd.addr_bytes = handle->order.address_byte;
    handle->msg.cmd.dummy      = 0;

    if (0 != spiflash_transfer(handle->ctrl_id, &handle->msg))
    {
        return ERR_SPINOR_DEVICE_FAILURE;
    }

    return ERR_SPINOR_SUCCESS;
}

static u8 drv_spinor_complete_read(struct spinor_handle *handle, u32 address, u8 *data, u32 size)
{
    handle->msg.buffer         = data;
    handle->msg.size           = size;
    handle->msg.command        = handle->bdma_to_xzdec_en ? SPIFLASH_COMMAND_READ_TO_XZDEC : SPIFLASH_COMMAND_READ;
    handle->msg.cmd.cmd        = handle->order.read_data;
    handle->msg.cmd.address    = address;
    handle->msg.cmd.addr_bytes = handle->order.address_byte;
    handle->msg.cmd.dummy      = handle->order.dummy;

    if (0 != spiflash_transfer(handle->ctrl_id, &handle->msg))
    {
        return ERR_SPINOR_DEVICE_FAILURE;
    }

    return ERR_SPINOR_SUCCESS;
}

static u8 drv_spinor_complete_erase(struct spinor_handle *handle, u8 command, u32 address)
{
    handle->msg.buffer         = NULL;
    handle->msg.size           = 0;
    handle->msg.command        = SPIFLASH_COMMAND_WRITE_STATUS;
    handle->msg.cmd.cmd        = command;
    handle->msg.cmd.address    = address;
    handle->msg.cmd.addr_bytes = handle->order.address_byte;
    handle->msg.cmd.dummy      = 0;

    if (0 != spiflash_transfer(handle->ctrl_id, &handle->msg))
    {
        return ERR_SPINOR_DEVICE_FAILURE;
    }

    return ERR_SPINOR_SUCCESS;
}

static u8 drv_spinor_write_enable(struct spinor_handle *handle)
{
    return drv_spinor_complete_write_status(handle, SPI_NOR_CMD_WREN, NULL, 0);
}

static u8 drv_spinor_enable_reset(struct spinor_handle *handle)
{
    return drv_spinor_complete_write_status(handle, SPI_NOR_CMD_EN_RESET, NULL, 0);
}

static u8 drv_spinor_reset_device(struct spinor_handle *handle)
{
    return drv_spinor_complete_write_status(handle, SPI_NOR_CMD_RESET, NULL, 0);
}

static u8 drv_spinor_use_ext(struct spinor_handle *handle, u8 ext)
{
    u8 u8_retry = 5;
    u8 u8_rcmd;

    do
    {
        if (ERR_SPINOR_SUCCESS != drv_spinor_write_enable(handle))
        {
            break;
        }

        if (ERR_SPINOR_SUCCESS != drv_spinor_complete_write_status(handle, SPI_NOR_CMD_WREAR, &ext, 1))
        {
            break;
        }

        if (ERR_SPINOR_SUCCESS != drv_spinor_complete_read_status(handle, SPI_NOR_CMD_RDEAR, &u8_rcmd, 1))
        {
            break;
        }

        if (ext == u8_rcmd)
        {
            return ERR_SPINOR_SUCCESS;
        }
    } while (u8_retry--);

    if (!u8_retry && (ext != 0xFF))
    {
        flash_impl_printf("[FLASH] Set ext address register fail, retry 5\n");
        return ERR_SPINOR_DEVICE_FAILURE;
    }

    return ERR_SPINOR_SUCCESS;
}

static u8 drv_spinor_enter_4byte_address(struct spinor_handle *handle)
{
    return drv_spinor_complete_write_status(handle, SPI_NOR_CMD_ENTER_4BYTE, NULL, 0);
}

static u8 drv_spinor_read_id(struct spinor_handle *handle, u8 *ids, u8 bytes)
{
    return drv_spinor_complete_read_status(handle, SPI_NOR_CMD_RDID, ids, bytes);
}

static u8 drv_spinor_check_status(struct spinor_handle *handle)
{
    u32 left_time;
    u32 status;
    left_time = handle->order.time_wait;
    do
    {
        if (0 == left_time)
        {
            flash_impl_printf_hex("[SPINOR] check timeout! @0x", left_time, " us\r\n");
            return ERR_SPINOR_TIMEOUT;
        }

        if (ERR_SPINOR_SUCCESS != drv_spinor_complete_read_status(handle, SPI_NOR_CMD_RDSR, (u8 *)&status, 1))
        {
            flash_impl_printf("[SPINOR] DRV_SPINOR_read_status1 timeout!\r\n");
            return ERR_SPINOR_TIMEOUT;
        }

        if (0 == (status & SPI_NOR_BUSY))
        {
            break;
        }

        FLASH_IMPL_USDELAY(10);

        if (10 < left_time)
        {
            left_time -= 10;
        }
        else
        {
            left_time = 0;
        }

    } while (1);

    return ERR_SPINOR_SUCCESS;
}

static u8 drv_spinor_write_status(struct spinor_handle *handle, u8 cmd, u8 *pstatus, u8 size)
{
    if (ERR_SPINOR_SUCCESS != drv_spinor_write_enable(handle))
    {
        return ERR_SPINOR_DEVICE_FAILURE;
    }

    if (ERR_SPINOR_SUCCESS != drv_spinor_complete_write_status(handle, cmd, pstatus, size))
    {
        return ERR_SPINOR_DEVICE_FAILURE;
    }

    return drv_spinor_check_status(handle);
}

static u8 drv_spinor_erase(struct spinor_handle *handle, u8 command, u32 address)
{
    u8 ext_addr = 0;

    do
    {
        if (handle->order.address_byte != 4)
        {
            ext_addr = (address < SPI_NOR_16MB) ? 0 : 1;

            if (ext_addr != handle->ext_addr)
            {
                if (ERR_SPINOR_SUCCESS != drv_spinor_use_ext(handle, ext_addr))
                {
                    handle->ext_addr = 0xFF;
                    break;
                }

                handle->ext_addr = ext_addr;
            }
        }

        if (ERR_SPINOR_SUCCESS != drv_spinor_write_enable(handle)
            || ERR_SPINOR_SUCCESS != drv_spinor_complete_erase(handle, command, address))
        {
            break;
        }

        return drv_spinor_check_status(handle);
    } while (0);

    return ERR_SPINOR_DEVICE_FAILURE;
}

static void drv_spinor_switch_config(struct spinor_handle *handle, flash_cmd_set_t *cmt, u8 enabled)
{
    u8  fn;
    u8  status = 0;
    u8  status_bytes;
    u8  read_reg_cmd[] = {SPI_NOR_CMD_RDSR, SPI_NOR_CMD_RDSR2, SPI_NOR_CMD_RDSR3};
    u32 u32_status;

    u32_status   = 0;
    status_bytes = 0;
    for (fn = 0; sizeof(read_reg_cmd) / sizeof(u8) > fn && cmt->data_bytes > status_bytes; fn++)
    {
        if (0x01 & (cmt->address >> fn))
        {
            if (ERR_SPINOR_SUCCESS != drv_spinor_complete_read_status(handle, read_reg_cmd[fn], &status, 1))
            {
                flash_impl_printf("[FLASH_ERR] Read status registers fail!\r\n");
            }
            u32_status |= (status << (status_bytes * 8));
            status_bytes++;
        }
    }

    if (enabled && !(u32_status & cmt->value))
    {
        u32_status |= cmt->value;
    }
    else if (!enabled && (u32_status & cmt->value))
    {
        u32_status &= ~cmt->value;
    }
    else
    {
        return;
    }

    if (ERR_SPINOR_SUCCESS != drv_spinor_write_status(handle, cmt->command, (u8 *)&u32_status, cmt->data_bytes))
    {
        flash_impl_printf("[FLASH_ERR] drv_spinor_write_status fail!\r\n");
    }
}

static void drv_spinor_show_protect_status(struct spinor_handle *handle, spinor_info_t *spinor_info)
{
    u8 status_name_index;

    u32 status_size;
    u32 status;

    spinor_info_t *          pspinor_info;
    spinor_protect_status_t *protect_status;
    flash_cmd_set_t *        cmt;

    const char *pastatus_name[] = {"[SPINOR] complement = 0x", "[SPINOR] top/buttom = 0x", "[SPINOR] blocks = 0x",
                                   "[SPINOR] SRP0 = 0x", "[SPINOR] SRP1 = 0x"};

    pspinor_info   = spinor_info;
    protect_status = &pspinor_info->r_protect_status;

    status_size       = sizeof(spinor_protect_status_t);
    cmt               = (flash_cmd_set_t *)&protect_status->block_status.complement;
    status_name_index = 0;

    while (0 != status_size)
    {
        if (0 < cmt->data_bytes)
        {
            status = 0;

            if (ERR_SPINOR_SUCCESS
                != drv_spinor_complete_read_status(handle, cmt->command, (u8 *)&status, cmt->data_bytes))
            {
                flash_impl_printf("[FLASH_ERR] drv_spinor_complete_read_status fail!\r\n");
            }
            flash_impl_printf_hex(pastatus_name[status_name_index], status & cmt->value, "\r\n");
        }

        cmt++;
        status_size -= sizeof(flash_cmd_set_t);
        status_name_index++;
    }
}

void mdrv_spinor_info(struct spinor_handle *handle, flash_nor_info_t *pst_flash_nor_info)
{
    spinor_info_t *pspinor_info = NULL;

    if (!handle->nri)
        return;

    pspinor_info = &handle->nri->spinor_info;

    pst_flash_nor_info->page_size   = 0;
    pst_flash_nor_info->sector_size = 0;
    pst_flash_nor_info->block_size  = 0;
    pst_flash_nor_info->capacity    = 0;

    if (pspinor_info)
    {
        pst_flash_nor_info->page_size   = pspinor_info->page_byte_cnt;
        pst_flash_nor_info->sector_size = pspinor_info->sector_byte_cnt;
        pst_flash_nor_info->block_size  = pspinor_info->blk_bytes_cnt;
        pst_flash_nor_info->capacity    = pspinor_info->capacity;
    }
}

void mdrv_spinor_setup_by_default(struct spinor_handle *handle)
{
    handle->msg.bdma_en        = 1;
    handle->bdma_to_xzdec_en   = 0;
    handle->ext_addr           = 0xFF;
    handle->order.read_data    = SPI_NOR_CMD_FASTREAD;
    handle->order.address_byte = 3;
    handle->order.dummy        = 0x08;
    handle->order.page_program = SPI_NOR_CMD_PP;
    handle->order.time_wait    = 0x5000000;
}

u8 mdrv_spinor_setup_by_nri(struct spinor_handle *handle)
{
    spinor_info_t *        info = NULL;
    struct spiflash_config config;

    if (!handle->nri)
        return ERR_SPINOR_INVALID;

    info = &handle->nri->spinor_info;

    handle->msg.bdma_en        = !(info->reserved[0] & 0x01);
    handle->bdma_to_xzdec_en   = 0;
    handle->ext_addr           = 0xFF;
    handle->order.read_data    = info->read_data.command;
    handle->order.dummy        = info->read_data.dummy;
    handle->order.address_byte = (info->read_data.address_bytes == 4) ? 4 : 3;
    handle->order.page_program = info->program.command;
    handle->order.time_wait    = info->max_wait_time;

    config.cmd        = handle->order.read_data;
    config.rate       = info->max_clk;
    config.have_phase = info->have_phase;
    config.phase      = info->phase;
    config.cs_select  = handle->msg.cs_select;
    spiflash_setup(handle->ctrl_id, &config);

    return ERR_SPINOR_SUCCESS;
}

u8 mdrv_spinor_hardware_init(struct spinor_handle *handle)
{
    spinor_info_t *    info             = NULL;
    spinor_quad_cfg_t *pst_quad_enabled = NULL;

    if (!handle->nri)
        return ERR_SPINOR_INVALID;

    info             = &handle->nri->spinor_info;
    pst_quad_enabled = &info->qe;

    mdrv_spinor_setup_by_nri(handle);

    flash_impl_show_id(info->id, info->id_byte_cnt);
    flash_impl_printf_hex("[SPINOR] ReadData = 0x", handle->order.read_data, "\r\n");
    flash_impl_printf_hex("[SPINOR] Dummy = 0x", handle->order.dummy, "\r\n");
    flash_impl_printf_hex("[SPINOR] pageProgram = 0x", handle->order.page_program, "\r\n");
    if (handle->msg.bdma_en)
    {
        flash_impl_printf("[FLASH] BDMA mode\r\n");
    }
    else
    {
        flash_impl_printf("[FLASH] RIU mode\r\n");
    }

    if (handle->order.address_byte == 4)
    {
        drv_spinor_enter_4byte_address(handle);
    }

    mdrv_spinor_unlock_whole_flash(handle);

    if (pst_quad_enabled->need_qe)
    {
        drv_spinor_switch_config(handle, &pst_quad_enabled->w_quad_enabled, 1);
    }

    drv_spinor_show_protect_status(handle, info);

    flash_impl_printf("[FLASH] End flash init.\r\n");
    return ERR_SPINOR_SUCCESS;
}

u8 mdrv_spinor_reset(struct spinor_handle *handle)
{
    if (ERR_SPINOR_SUCCESS != drv_spinor_enable_reset(handle))
    {
        return ERR_SPINOR_DEVICE_FAILURE;
    }

    if (ERR_SPINOR_SUCCESS != drv_spinor_reset_device(handle))
    {
        return ERR_SPINOR_DEVICE_FAILURE;
    }

    handle->ext_addr = 0;

    return drv_spinor_check_status(handle);
}

u8 mdrv_spinor_is_nri_match(struct spinor_handle *handle, spinor_info_t *info)
{
    u8 u8_id_matched = 0;
    u8 i;
    u8 device_id[16];

    if (ERR_SPINOR_SUCCESS != drv_spinor_read_id(handle, ((u8 *)&device_id), info->id_byte_cnt))
        return 0;

    if (0 < info->id_byte_cnt)
    {
        u8_id_matched = 1;
        for (i = 0; info->id_byte_cnt > i; i++)
        {
            if (device_id[i] != info->id[i])
            {
                flash_impl_show_id(info->id, info->id_byte_cnt);
                u8_id_matched = 0;
                break;
            }
        }
    }

    return u8_id_matched;
}

u8 mdrv_spinor_unlock_whole_flash(struct spinor_handle *handle)
{
    spinor_info_t *          info                 = NULL;
    spinor_protect_status_t *pst_w_protect_status = NULL;
    flash_cmd_set_t *        pt_w_cmt             = NULL;
    flash_cmd_set_t *        pt_c_cmt             = NULL;

    if (!handle->nri)
        return ERR_SPINOR_INVALID;

    info                 = &handle->nri->spinor_info;
    pst_w_protect_status = &info->w_protect_status;
    pt_w_cmt             = &pst_w_protect_status->block_status.blocks;
    pt_c_cmt             = &pst_w_protect_status->block_status.complement;

    if (0 < pt_w_cmt->data_bytes)
    {
        drv_spinor_switch_config(handle, pt_w_cmt, 0);
    }

    if (0 < pt_c_cmt->data_bytes)
    {
        drv_spinor_switch_config(handle, pt_c_cmt, 0);
    }

    return ERR_SPINOR_SUCCESS;
}

u8 mdrv_spinor_read(struct spinor_handle *handle, u32 address, u8 *data, u32 size)
{
    u8  ext_addr = 0;
    u32 read_size;

    while (0 != size)
    {
        if (handle->order.address_byte != 4)
        {
            read_size = SPI_NOR_16MB - (address & SPI_NOR_16MB_MASK);

            if (size < read_size)
            {
                read_size = size;
            }

            ext_addr = (address < SPI_NOR_16MB) ? 0 : 1;

            if (ext_addr != handle->ext_addr)
            {
                if (ERR_SPINOR_SUCCESS != drv_spinor_use_ext(handle, ext_addr))
                {
                    handle->ext_addr = 0xFF;
                    break;
                }

                handle->ext_addr = ext_addr;
            }
        }
        else
        {
            read_size = size;
        }

        if (ERR_SPINOR_SUCCESS != drv_spinor_complete_read(handle, address, data, read_size))
        {
            break;
        }

        address += read_size;
        data += read_size;
        size -= read_size;
    }

    if (size)
        return ERR_SPINOR_DEVICE_FAILURE;

    return ERR_SPINOR_SUCCESS;
}

u8 mdrv_spinor_program(struct spinor_handle *handle, u32 address, u8 *data, u32 size)
{
    u8             ext_addr   = 0;
    u16            write_size = 0;
    spinor_info_t *info       = NULL;

    if (!handle->nri)
        return ERR_SPINOR_INVALID;

    info = &handle->nri->spinor_info;

    while (0 < size)
    {
        write_size = info->page_byte_cnt - ((info->page_byte_cnt - 1) & address);

        if (write_size > size)
        {
            write_size = size;
        }

        if (handle->order.address_byte != 4)
        {
            ext_addr = (address < SPI_NOR_16MB) ? 0 : 1;

            if (ext_addr != handle->ext_addr)
            {
                if (ERR_SPINOR_SUCCESS != drv_spinor_use_ext(handle, ext_addr))
                {
                    handle->ext_addr = 0xFF;
                    return ERR_SPINOR_DEVICE_FAILURE;
                }

                handle->ext_addr = ext_addr;
            }
        }

        if (ERR_SPINOR_SUCCESS != drv_spinor_write_enable(handle))
        {
            return ERR_SPINOR_DEVICE_FAILURE;
        }

        if (ERR_SPINOR_SUCCESS
            != drv_spinor_complete_program_page(handle, handle->order.page_program, address, data, write_size))
        {
            return ERR_SPINOR_DEVICE_FAILURE;
        }

        if (ERR_SPINOR_SUCCESS != drv_spinor_check_status(handle))
        {
            return ERR_SPINOR_DEVICE_FAILURE;
        }

        address += write_size;
        data += write_size;
        size -= write_size;
    }

    return ERR_SPINOR_SUCCESS;
}

u8 mdrv_spinor_erase(struct spinor_handle *handle, u32 address, u32 size)
{
    u32            erase_size = 0;
    spinor_info_t *info       = NULL;

    if (!handle->nri)
        return ERR_SPINOR_INVALID;

    info = &handle->nri->spinor_info;

    if (((info->sector_byte_cnt - 1) & address) || ((info->sector_byte_cnt - 1) & size))
    {
        return ERR_SPINOR_INVALID;
    }

    if (info->capacity < address)
    {
        return ERR_SPINOR_INVALID;
    }

    while (0 != size)
    {
        if (!(address & (info->blk_bytes_cnt - 1)) && (size >= info->blk_bytes_cnt))
        {
            if (ERR_SPINOR_SUCCESS != drv_spinor_erase(handle, SPI_NOR_CMD_64BE, address))
            {
                return ERR_SPINOR_DEVICE_FAILURE;
            }

            erase_size = info->blk_bytes_cnt;
        }
        else
        {
            if (ERR_SPINOR_SUCCESS != drv_spinor_erase(handle, SPI_NOR_CMD_SE, address))
            {
                return ERR_SPINOR_DEVICE_FAILURE;
            }

            erase_size = info->sector_byte_cnt;
        }

        address += erase_size;
        size -= erase_size;
    }

    return ERR_SPINOR_SUCCESS;
}

#if defined(CONFIG_FLASH_XZDEC)
u8 mdrv_spinor_read_to_xzdec(struct spinor_handle *handle, u32 u32_address, u8 *pu8_data, u32 u32_size)
{
    u8 status;

    handle->bdma_to_xzdec_en = 1;

    status = mdrv_spinor_read(handle, u32_address, pu8_data, u32_size);

    handle->bdma_to_xzdec_en = 0;

    return status;
}
#endif
