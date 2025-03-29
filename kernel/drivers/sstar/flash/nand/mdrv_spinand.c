/*
 * mdrv_spinand.c- Sigmastar
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
#include <mdrv_spinand.h>

static u8 drv_spinand_complete_get_features(struct spinand_handle *handle, u8 command, u8 address, u8 *data, u8 size)
{
    handle->msg.buffer         = data;
    handle->msg.size           = size;
    handle->msg.command        = SPIFLASH_COMMAND_READ_STATUS;
    handle->msg.cmd.cmd        = command;
    handle->msg.cmd.address    = address;
    handle->msg.cmd.addr_bytes = 1;
    handle->msg.cmd.dummy      = 0;

    if (0 != spiflash_transfer(handle->ctrl_id, &handle->msg))
        return ERR_SPINAND_DEVICE_FAILURE;

    return ERR_SPINAND_SUCCESS;
}

static u8 drv_spinand_complete_set_features(struct spinand_handle *handle, u8 command, u32 address, u8 addr_bytes,
                                            u8 *data, u8 size)
{
    handle->msg.buffer         = data;
    handle->msg.size           = size;
    handle->msg.command        = SPIFLASH_COMMAND_WRITE_STATUS;
    handle->msg.cmd.cmd        = command ? command : SPI_NAND_CMD_SF;
    handle->msg.cmd.address    = address;
    handle->msg.cmd.addr_bytes = addr_bytes;
    handle->msg.cmd.dummy      = 0;

    if (0 != spiflash_transfer(handle->ctrl_id, &handle->msg))
        return ERR_SPINAND_DEVICE_FAILURE;

    return ERR_SPINAND_SUCCESS;
}

static u8 drv_spinand_complete_read(struct spinand_handle *handle, u16 col_address, u8 *data, u32 size)
{
    handle->msg.buffer         = data;
    handle->msg.size           = size;
    handle->msg.command        = handle->bdma_to_xzdec_en ? SPIFLASH_COMMAND_READ_TO_XZDEC : SPIFLASH_COMMAND_READ;
    handle->msg.cmd.cmd        = handle->order.rfc;
    handle->msg.cmd.address    = col_address;
    handle->msg.cmd.addr_bytes = handle->order.rfc_addr_bytes;
    handle->msg.cmd.dummy      = handle->order.dummy;

    if (0 != spiflash_transfer(handle->ctrl_id, &handle->msg))
        return ERR_SPINAND_DEVICE_FAILURE;

    return ERR_SPINAND_SUCCESS;
}

static u8 drv_spinand_complete_write(struct spinand_handle *handle, u16 col_address, u8 *data, u32 size)
{
    handle->msg.buffer         = data;
    handle->msg.size           = size;
    handle->msg.command        = SPIFLASH_COMMAND_PROGRAM;
    handle->msg.cmd.cmd        = handle->order.program_load;
    handle->msg.cmd.address    = col_address;
    handle->msg.cmd.addr_bytes = 2;
    handle->msg.cmd.dummy      = 0;

    if (0 != spiflash_transfer(handle->ctrl_id, &handle->msg))
        return ERR_SPINAND_DEVICE_FAILURE;

    return ERR_SPINAND_SUCCESS;
}

static u8 drv_spinand_reset(struct spinand_handle *handle)
{
    return drv_spinand_complete_set_features(handle, SPI_NAND_CMD_RESET, 0, 0, NULL, 0);
}

static u8 drv_spinand_write_enable(struct spinand_handle *handle)
{
    return drv_spinand_complete_set_features(handle, SPI_NAND_CMD_WREN, 0, 0, NULL, 0);
}

static u8 drv_spinand_execute(struct spinand_handle *handle)
{
    return drv_spinand_complete_set_features(handle, SPI_NAND_CMD_PE, 0, 0, NULL, 0);
}

static u8 drv_spinand_program_execute(struct spinand_handle *handle, u32 page_address)
{
    return drv_spinand_complete_set_features(handle, SPI_NAND_CMD_PE, page_address, 3, NULL, 0);
}

static u8 drv_spinand_block_erase(struct spinand_handle *handle, u32 page_address)
{
    return drv_spinand_complete_set_features(handle, SPI_NAND_CMD_BE, page_address, 3, NULL, 0);
}

static u8 drv_spinand_page_data_read(struct spinand_handle *handle, u32 page_address)
{
    return drv_spinand_complete_set_features(handle, SPI_NAND_CMD_PGRD, page_address, 3, NULL, 0);
}

static u8 drv_spinand_next_page_read(struct spinand_handle *handle, flash_cmd_set_t *next)
{
    return drv_spinand_complete_set_features(handle, next->command, next->address, next->address_bytes, NULL, 0);
}

static u8 drv_spinand_check_status(struct spinand_handle *handle, u8 *pstatus)
{
    u8  status  = ERR_SPINAND_SUCCESS;
    u32 timeout = handle->order.max_wait_time;

    do
    {
        status = 0;

        if (ERR_SPINAND_SUCCESS
            != drv_spinand_complete_get_features(handle, SPI_NAND_CMD_GF, SPI_NAND_REG_STAT, &status, 1))
        {
            return ERR_SPINAND_DEVICE_FAILURE;
        }

        FLASH_IMPL_USDELAY(10);

        if (10 < timeout)
        {
            timeout -= 10;
        }
        else
        {
            timeout = 0;
        }

        if (0 == timeout)
        {
            return ERR_SPINAND_TIMEOUT;
        }

    } while (status & SPI_NAND_STAT_OIP);

    if (status & SPI_NAND_STAT_P_FAIL)
    {
        return ERR_SPINAND_P_FAIL;
    }

    if (status & SPI_NAND_STAT_E_FAIL)
    {
        return ERR_SPINAND_E_FAIL;
    }

    if (pstatus)
        *pstatus = status;

    return ERR_SPINAND_SUCCESS;
}

static void drv_spinand_wait_device_available(struct spinand_handle *handle)
{
    drv_spinand_check_status(handle, NULL);
}

static void drv_spinand_setup_access(struct spinand_handle *handle, u8 read, u8 address_bytes, u8 dummy_cycle,
                                     u8 program, u8 random)
{
    if (handle->order.rfc != read || handle->order.dummy != dummy_cycle)
    {
        handle->order.rfc            = read;
        handle->order.dummy          = dummy_cycle;
        handle->order.rfc_addr_bytes = address_bytes ? address_bytes : 2;
    }

    if (handle->order.program_load != program || handle->order.random_load != random)
    {
        handle->order.program_load = program;
        handle->order.random_load  = random;
    }
}

static u8 drv_spinand_wait_status(struct spinand_handle *handle, flash_cmd_set_t *status)
{
    u32 u32_status = 0;
    u32 timeout;

    timeout = handle->order.max_wait_time;

    do
    {
        if (ERR_SPINAND_SUCCESS
            != drv_spinand_complete_get_features(handle, SPI_NAND_CMD_GF, status->address, (u8 *)&u32_status,
                                                 status->data_bytes))
        {
            return ERR_SPINAND_DEVICE_FAILURE;
        }

        timeout--;

        if (0 == timeout)
        {
            return ERR_SPINAND_TIMEOUT;
        }

    } while (status->value & u32_status);

    return ERR_SPINAND_SUCCESS;
}

static void drv_spinand_switch_config(struct spinand_handle *handle, flash_cmd_set_t *pst_config, u8 enabled)
{
    u32 config;

    if (!pst_config->data_bytes)
        return;

    drv_spinand_wait_device_available(handle);

    if (ERR_SPINAND_SUCCESS
        != drv_spinand_complete_get_features(handle, SPI_NAND_CMD_GF, pst_config->address, (u8 *)&config,
                                             pst_config->data_bytes))
    {
        flash_impl_printf("[FLASH] Get buf mode status fail!");
    }

    if (enabled && !(config & pst_config->value))
    {
        config |= pst_config->value;
    }
    else if (!enabled && (config & pst_config->value))
    {
        config &= ~pst_config->value;
    }
    else
    {
        return;
    }

    if (ERR_SPINAND_SUCCESS
        != drv_spinand_complete_set_features(handle, SPI_NAND_CMD_SF, pst_config->address, 1, (u8 *)&config,
                                             pst_config->data_bytes))
    {
        flash_impl_printf("[FLASH] Configure buf mode status fail!");
    }

    drv_spinand_wait_device_available(handle);
}

static u8 drv_spinand_die_sel(struct spinand_handle *handle, u32 page)
{
    u8                  die_id;
    u32                 page_limit;
    spinand_ext_info_t *spinand_ext_info = NULL;
    spinand_info_t *    spinand_info     = NULL;

    if (!handle->sni)
        return ERR_SPINAND_SUCCESS;

    spinand_ext_info = &handle->sni->spinand_ext_info;
    spinand_info     = &handle->sni->spinand_info;

    if (spinand_ext_info->profile.flags & SPINAND_MULTI_DIES)
    {
        page_limit = spinand_ext_info->profile.ext_config.die_config.die_size & ~(spinand_info->blk_page_cnt - 1);

        die_id = (page_limit > page) ? 0 : 1;

        if (handle->die_id == die_id)
        {
            return ERR_SPINAND_SUCCESS;
        }

        handle->die_id = die_id;

        return drv_spinand_complete_set_features(
            handle, spinand_ext_info->profile.ext_config.die_config.die_code.command, die_id, 1, NULL, 0);
    }

    return ERR_SPINAND_SUCCESS;
}

static u16 drv_spinand_offset_wrap(struct spinand_handle *handle, u32 page, u8 access)
{
    u16                    plane_select     = 0;
    spinand_ext_info_t *   spinand_ext_info = NULL;
    spinand_info_t *       spinand_info     = NULL;
    spinand_ext_profile_t *profile          = NULL;

    if (!handle->sni)
        return 0;

    spinand_ext_info = &handle->sni->spinand_ext_info;
    spinand_info     = &handle->sni->spinand_info;
    profile          = NULL;

    profile      = &spinand_ext_info->profile;
    plane_select = SPINAND_PLANE_SELECT & profile->flags;

    if (!(page & spinand_info->blk_page_cnt)) // NAND_PAGES_PER_BLOCK
    {
        return 0;
    }

    if ((!plane_select) && (1 < spinand_info->plane_cnt))
    {
        return (1 << 12);
    }

    if (!plane_select)
    {
        return 0;
    }

    switch (access)
    {
        case SPINAND_READ:
            return ((u16)(profile->access.read.address));
        case SPINAND_PROGRAM:
            return ((u16)(profile->access.program.address));
        case SPINAND_RANDOM:
            return ((u16)(profile->access.random.address));
    }

    return 0;
}

static void drv_spinand_reg_init(struct spinand_handle *handle)
{
    spinand_ext_info_t *  ext_sni  = &handle->sni->spinand_ext_info;
    flash_reg_init_set_t *init_set = &ext_sni->profile.init_set;

    if (!init_set->wcommand || !init_set->address1)
        return;

    if (ERR_SPINAND_SUCCESS
        != drv_spinand_complete_set_features(handle, SPI_NAND_CMD_SF, init_set->address1, 1,
                                             (u8 *)(&(init_set->value1)), 1))
    {
        flash_impl_printf("[FLASH] Configure buf mode status fail!");
    }

    drv_spinand_wait_device_available(handle);

    if (!init_set->address2)
        return;

    if (ERR_SPINAND_SUCCESS
        != drv_spinand_complete_set_features(handle, SPI_NAND_CMD_SF, init_set->address2, 1,
                                             (u8 *)(&(init_set->value2)), 1))
    {
        flash_impl_printf("[FLASH] Configure buf mode status fail!");
    }

    drv_spinand_wait_device_available(handle);
}

static u8 drv_spinand_check_ecc_default(struct spinand_handle *handle)
{
    u8                     ret     = ERR_SPINAND_SUCCESS;
    u8                     status  = 0;
    spinand_ext_profile_t *profile = NULL;

    ret = drv_spinand_check_status(handle, &status);

    if (ERR_SPINAND_TIMEOUT <= ret)
        return ret;

    status &= ECC_STATUS_MASK;

    if (ECC_NOT_CORRECTED == status)
    {
        return ERR_SPINAND_ECC_NOT_CORRECTED;
    }

    if (!handle->sni)
        return ERR_SPINAND_SUCCESS;

    profile = &handle->sni->spinand_ext_info.profile;

    if (ECC_STATUS_RESERVED == status)
    {
        if (profile->flags & SPINAND_ECC_RESERVED_NONE_CORRECTED)
        {
            return ERR_SPINAND_ECC_NOT_CORRECTED;
        }

        if (profile->flags & SPINAND_ECC_RESERVED_CORRECTED)
        {
            return ERR_SPINAND_ECC_CORRECTED;
        }
    }

    return ERR_SPINAND_SUCCESS;
}

static u8 drv_spinand_check_ecc_by_sni(struct spinand_handle *handle)
{
    u8                     ret;
    u8                     status;
    spinand_ext_info_t *   spinand_ext_info = &handle->sni->spinand_ext_info;
    spinand_ext_profile_t *profile          = &spinand_ext_info->profile;
    spinand_ecc_config_t * ecc_config       = &profile->ecc_config;
    flash_cmd_set_t *      ecc_status       = &ecc_config->ecc_status;

    ret = drv_spinand_check_status(handle, &status);

    if (ERR_SPINAND_TIMEOUT <= ret)
        return ret;

    status &= spinand_ext_info->profile.ecc_config.ecc_status_mask;

    if (!status)
        return ERR_SPINAND_SUCCESS;
    else if (spinand_ext_info->profile.ecc_config.ecc_not_correct_status == status)
        return ERR_SPINAND_ECC_NOT_CORRECTED;
    else if ((ecc_config->ecc_type & SPINAND_RESERVED_NONE_CORRECTED)
             && status == spinand_ext_info->profile.ecc_config.ecc_reserved)
        return ERR_SPINAND_ECC_NOT_CORRECTED;

    if (ecc_config->ecc_type & SPINAND_THRESHOLD)
    {
        if (ERR_SPINAND_SUCCESS
            != drv_spinand_complete_get_features(handle, ecc_status->command, (u8)(ecc_status->address), &status, 1))
            return ERR_SPINAND_DEVICE_FAILURE;

        if ((status & ecc_status->value) != ecc_status->value)
            return ERR_SPINAND_SUCCESS;
    }
    else if (ecc_config->ecc_type & SPINAND_BITFLIP)
    {
        if (ERR_SPINAND_SUCCESS
            != drv_spinand_complete_get_features(handle, ecc_status->command, (u8)(ecc_status->address), &status, 1))
            return ERR_SPINAND_DEVICE_FAILURE;

        if ((status & ecc_status->value) < spinand_ext_info->profile.ecc_config.bitflip_threshold)
            return ERR_SPINAND_SUCCESS;
    }

    return ERR_SPINAND_ECC_CORRECTED;
}

static u8 drv_spinand_check_ecc(struct spinand_handle *handle)
{
    spinand_ext_info_t *ext_info = NULL;

    if (!handle->sni)
        return drv_spinand_check_ecc_default(handle);

    ext_info = &handle->sni->spinand_ext_info;

    if (ext_info->profile.ecc_config.ecc_en)
        return drv_spinand_check_ecc_by_sni(handle);

    return drv_spinand_check_ecc_default(handle);
}

#if defined(CONFIG_FLASH_SOC_ECC)
static u8 drv_spinand_soc_ecc_read(struct spinand_handle *handle, u32 page, u16 column, u8 *data, u32 size)
{
    u32                   data_offset = 0;
    u32                   read_size   = handle->ecc.page_size + handle->ecc.oob_size;
    u8 *                  data_buf    = handle->ecc.fcie_buffer;
    spinand_ext_info_t *  ext_info    = NULL;
    spinand_ecc_config_t *ecc_config  = NULL;

    do
    {
        if (column >= handle->ecc.page_size) // only oob, bypass ecc decode
        {
            data_offset = column;
            data_buf    = data;
            read_size   = size;
        }

        if (ERR_SPINAND_SUCCESS
            != drv_spinand_complete_read(handle, data_offset | drv_spinand_offset_wrap(handle, page, SPINAND_READ),
                                         data_buf, read_size))
        {
            return ERR_SPINAND_DEVICE_FAILURE;
        }

        if (column < handle->ecc.page_size)
            break;

        return ERR_SPINAND_SUCCESS;
    } while (0);

    if (FCIE_SUCCESS != sstar_fcie_ecc_decode(handle->fcie_if, &handle->ecc))
    {
        return ERR_SPINAND_DEVICE_FAILURE;
    }

    flash_impl_memcpy(data, handle->ecc.fcie_buffer + column, size);

    if (!handle->ecc.ecc_status)
        return ERR_SPINAND_SUCCESS;
    else if (handle->ecc.ecc_status == 0x02)
        return ERR_SPINAND_ECC_NOT_CORRECTED;
    else if (handle->sni)
    {
        ext_info   = &handle->sni->spinand_ext_info;
        ecc_config = &ext_info->profile.ecc_config;

        if (ecc_config->ecc_en && (handle->ecc.ecc_bitflip_cnt >= ecc_config->bitflip_threshold))
            return ERR_SPINAND_ECC_CORRECTED;
    }

    return ERR_SPINAND_SUCCESS;
}

static u8 drv_spinand_soc_ecc_write(struct spinand_handle *handle, u32 page, u16 column, u8 *data, u32 size)
{
    u32 data_offset = 0;
    u32 write_size  = handle->ecc.page_size + handle->ecc.oob_size;
    u8 *data_buf    = handle->ecc.fcie_buffer;

    do
    {
        if (column >= handle->ecc.page_size) // only oob, bypass ecc decode
        {
            data_offset = column;
            data_buf    = data;
            write_size  = size;
        }

        if (column < handle->ecc.page_size)
        {
            flash_impl_memset(handle->ecc.fcie_buffer, 0xFF, write_size);
            flash_impl_memcpy(handle->ecc.fcie_buffer + column, data, size);

            if (FCIE_SUCCESS != sstar_fcie_ecc_encode(handle->fcie_if, &handle->ecc))
                break;
        }

        if (ERR_SPINAND_SUCCESS
            != drv_spinand_complete_write(handle, data_offset | drv_spinand_offset_wrap(handle, page, SPINAND_READ),
                                          data_buf, write_size))
            break;

        return ERR_SPINAND_SUCCESS;
    } while (0);

    return ERR_SPINAND_DEVICE_FAILURE;
}
#endif

static u8 drv_spinand_read_from_cache(struct spinand_handle *handle, u32 page, u16 column, u8 *data, u32 size)
{
    if (ERR_SPINAND_SUCCESS
        != drv_spinand_complete_read(handle, column | drv_spinand_offset_wrap(handle, page, SPINAND_READ), data, size))
        return ERR_SPINAND_DEVICE_FAILURE;

    return ERR_SPINAND_SUCCESS;
}

static u8 drv_spinand_write_to_cache(struct spinand_handle *handle, u32 page, u16 column, u8 *data, u32 size)
{
    if (ERR_SPINAND_SUCCESS
        != drv_spinand_complete_write(handle, column | drv_spinand_offset_wrap(handle, page, SPINAND_PROGRAM), data,
                                      size))
    {
        return ERR_SPINAND_DEVICE_FAILURE;
    }

    return ERR_SPINAND_SUCCESS;
}

static u32 drv_spinand_read_pages(struct spinand_handle *handle, u32 page, u8 *data, u32 size)
{
    u32             load_size  = 0;
    u32             bytes_left = size;
    spinand_info_t *info       = NULL;

    if (!handle->sni)
        return 0;

    info = &handle->sni->spinand_info;

    while (0 != bytes_left)
    {
        load_size = (bytes_left > info->page_byte_cnt) ? info->page_byte_cnt : bytes_left;

        if (ERR_SPINAND_ECC_NOT_CORRECTED <= mdrv_spinand_page_read(handle, page, 0, data, load_size))
        {
            break;
        }

        bytes_left -= load_size;
        data += load_size;
        page++;
    }

    if (0 != bytes_left)
    {
        size -= bytes_left;
    }

    return size;
}

static u32 drv_spinand_read_pages_without_buf_mode(struct spinand_handle *handle, u32 page, u8 *data, u32 size)
{
    u32                 bytes_load;
    u32                 bytes_left;
    u32                 page_load;
    u32                 page_count;
    u32                 page_limit;
    spinand_ext_info_t *spinand_ext_info = &handle->sni->spinand_ext_info;
    spinand_info_t *    spinand_info     = &handle->sni->spinand_info;
    spinand_cr_mode_t * pst_cr_mode      = &spinand_ext_info->profile.access.cr_mode;

    if (spinand_info->blk_page_cnt >= size)
    {
        return drv_spinand_read_pages(handle, page, data, size);
    }

    bytes_left = size;
    bytes_load = bytes_left;
    page_load  = page;

    if (spinand_ext_info->profile.flags & SPINAND_MULTI_DIES)
    {
        drv_spinand_die_sel(handle, 0);
    }

    drv_spinand_switch_config(handle, &pst_cr_mode->cr_profile.none_buf_mode.none_buf_mode_code, 0);

    if (spinand_ext_info->profile.flags & SPINAND_MULTI_DIES)
    {
        page_limit = spinand_ext_info->profile.ext_config.die_config.die_size & ~(spinand_info->blk_page_cnt - 1);
        page_count = size >> flash_impl_count_bits(spinand_info->page_byte_cnt);

        if (page_limit < (page + page_count) || page_limit <= page)
        {
            drv_spinand_die_sel(handle, page_limit);
            drv_spinand_switch_config(handle, &pst_cr_mode->cr_profile.none_buf_mode.none_buf_mode_code, 0);

            if (page_limit < (page + page_count) && page_limit > page)
            {
                bytes_load = (page_limit - page) << flash_impl_count_bits(spinand_info->page_byte_cnt);
            }
        }
    }

    drv_spinand_setup_access(
        handle, pst_cr_mode->cr_profile.none_buf_mode.load.command,
        pst_cr_mode->cr_profile.none_buf_mode.load.address_bytes, pst_cr_mode->cr_profile.none_buf_mode.load.dummy,
        spinand_ext_info->profile.access.program.command, spinand_ext_info->profile.access.random.command);

    do
    {
        if (ERR_SPINAND_ECC_NOT_CORRECTED <= mdrv_spinand_page_data_read(handle, page_load))
        {
            break;
        }

        if (ERR_SPINAND_SUCCESS != drv_spinand_read_from_cache(handle, page_load, 0, data, bytes_load))
        {
            break;
        }

        if (ERR_SPINAND_ECC_NOT_CORRECTED == drv_spinand_check_ecc(handle))
        {
            break;
        }

        bytes_left = bytes_left - bytes_load;
        bytes_load = bytes_left;
        if (spinand_ext_info->profile.flags & SPINAND_MULTI_DIES)
        {
            page_load = page_limit;
        }
    } while (bytes_load);

    drv_spinand_setup_access(
        handle, spinand_ext_info->profile.access.read.command, 0, spinand_ext_info->profile.access.read.dummy,
        spinand_ext_info->profile.access.program.command, spinand_ext_info->profile.access.random.command);

    if (spinand_ext_info->profile.flags & SPINAND_MULTI_DIES)
    {
        if (page_limit < (page + page_count) || page_limit <= page)
        {
            drv_spinand_die_sel(handle, page_limit);
            drv_spinand_switch_config(handle, &pst_cr_mode->cr_profile.none_buf_mode.none_buf_mode_code, 1);
        }
        drv_spinand_die_sel(handle, 0);
    }

    drv_spinand_switch_config(handle, &pst_cr_mode->cr_profile.none_buf_mode.none_buf_mode_code, 1);

    if (0 != bytes_load)
    {
        size -= bytes_load;
    }

    return size;
}

static u32 drv_spinand_read_pages_with_buf_mode(struct spinand_handle *handle, u32 page, u8 *data, u32 size)
{
    spinand_buf_mode_t mode;

    u32 page_end;
    u32 read_end;
    u32 bytes_left;
    u32 load_size;

    u8 block_end_with_last;
    u8 wait_device_avail;
    u8 wait_next;
    u8 wait_next_after_read;
    u8 end_with_reset;

    spinand_ext_info_t *spinand_ext_info = &handle->sni->spinand_ext_info;
    spinand_info_t *    spinand_info     = &handle->sni->spinand_info;

    mode                 = spinand_ext_info->profile.access.cr_mode.cr_profile.buf_mode;
    block_end_with_last  = mode.check_flag & SPINAND_CR_BLOCK_WITH_LAST;
    wait_device_avail    = mode.check_flag & SPINAND_CR_NEXT_STATUS;
    wait_next            = mode.check_flag & SPINAND_CR_BUSY_AFTER_NEXT;
    wait_next_after_read = mode.check_flag & SPINAND_CR_BUSY_AFTER_READ;
    end_with_reset       = mode.check_flag & SPINAND_CR_END_WITH_REST;

    bytes_left = size;
    page_end = page + ((size + spinand_info->page_byte_cnt - 1) >> flash_impl_count_bits(spinand_info->page_byte_cnt));
    read_end = page_end;

    drv_spinand_wait_device_available(handle);

restart_page_read:
    load_size = 0;
    read_end  = page_end;

    if (block_end_with_last)
    {
        read_end = (page + spinand_info->blk_page_cnt) & ~(spinand_info->blk_page_cnt - 1);

        if (read_end > page_end)
        {
            read_end = page_end;
        }
    }

    if (ERR_SPINAND_ECC_NOT_CORRECTED <= mdrv_spinand_page_data_read(handle, page++)) // has read first page
    {
        return 0;
    }

    mode.next_page.address = 0;

    while (read_end > page)
    {
        // prepare next page
        mode.next_page.address = page;

        drv_spinand_next_page_read(handle, &mode.next_page);

        if (wait_next)
        {
            drv_spinand_wait_status(handle, &mode.check_busy);
        }

        if (wait_device_avail)
        {
            drv_spinand_wait_device_available(handle);
        }
        // load current page
        drv_spinand_read_from_cache(handle, (page - 1), 0, data, spinand_info->page_byte_cnt);

        if (wait_next_after_read)
        {
            drv_spinand_wait_status(handle, &mode.check_busy);
        }

        if (ERR_SPINAND_ECC_NOT_CORRECTED == drv_spinand_check_ecc(handle))
        {
            break;
        }

        bytes_left -= spinand_info->page_byte_cnt;
        data += spinand_info->page_byte_cnt;
        page++;
    }

    if (0 != mode.next_page.address)
    {
        drv_spinand_next_page_read(handle, &mode.last_page);

        if (wait_next)
        {
            drv_spinand_wait_status(handle, &mode.check_busy);
        }

        if (wait_device_avail)
        {
            drv_spinand_wait_device_available(handle);
        }
    }

    load_size = (bytes_left > spinand_info->page_byte_cnt) ? spinand_info->page_byte_cnt : bytes_left;

    drv_spinand_read_from_cache(handle, (page - 1), 0, data, load_size);

    if (0 != mode.next_page.address)
    {
        if (wait_next_after_read)
        {
            drv_spinand_wait_status(handle, &mode.check_busy);
        }

        if (end_with_reset)
        {
            if (ERR_SPINAND_SUCCESS != mdrv_spinand_reset(handle))
            {
                size -= bytes_left;
                return size;
            }
        }
    }

    if (read_end != page)
    {
        flash_impl_printf("[FLASH] buf_mode, meet ecc not corrected!\r\n");
        return 0;
    }

    bytes_left -= load_size;
    data += load_size;

    if (0 == bytes_left)
    {
        return size;
    }

    if (read_end < page_end)
    {
        goto restart_page_read;
    }

    return 0;
}

u8 mdrv_spinand_info(struct spinand_handle *handle, flash_nand_info_t *pst_flash_nand_info)
{
    spinand_info_t *info = NULL;

    if (!handle->sni)
        return ERR_SPINAND_INVALID;

    info = &handle->sni->spinand_info;

    memcpy((void *)(pst_flash_nand_info->id), (const void *)(info->id), info->id_byte_cnt);
    pst_flash_nand_info->id_byte_cnt  = info->id_byte_cnt;
    pst_flash_nand_info->blk_page_cnt = info->blk_page_cnt;
    pst_flash_nand_info->blk_cnt      = info->blk_cnt;
    pst_flash_nand_info->sector_size  = info->sector_byte_cnt;
    pst_flash_nand_info->page_size    = info->page_byte_cnt;
    pst_flash_nand_info->oob_size     = info->spare_byte_cnt;
    pst_flash_nand_info->block_size   = info->page_byte_cnt * info->blk_page_cnt;
    pst_flash_nand_info->capacity     = pst_flash_nand_info->block_size * info->blk_cnt;
    pst_flash_nand_info->bakcnt       = info->bakcnt;
    pst_flash_nand_info->bakofs       = info->bakofs;
    pst_flash_nand_info->bl0pba       = info->bl0pba;
    pst_flash_nand_info->bl1pba       = info->bl1pba;
    pst_flash_nand_info->blpinb       = info->blpinb;

    return ERR_SPINAND_SUCCESS;
}

void mdrv_spinand_setup_by_default(struct spinand_handle *handle)
{
    handle->fcie_if              = 0;
    handle->soc_ecc_en           = 0;
    handle->bdma_to_xzdec_en     = 0;
    handle->order.rfc            = SPI_NAND_CMD_RFC;
    handle->order.rfc_addr_bytes = 2;
    handle->order.dummy          = SPI_NAND_CMD_RFC_DUMMY;
    handle->order.program_load   = SPI_NAND_CMD_PL;
    handle->order.random_load    = SPI_NAND_CMD_RPL;
    handle->order.cr_mode        = SPINAND_NO_CR;
    handle->order.max_wait_time  = 500000;
    handle->msg.bdma_en          = 1;

#if defined(CONFIG_FLASH_SOC_ECC)
    handle->soc_ecc_en = hal_fcie_is_soc_ecc();
#endif
}

u8 mdrv_spinand_setup_by_sni(struct spinand_handle *handle)
{
    spinand_info_t *         info        = NULL;
    spinand_ext_info_t *     ext_info    = NULL;
    spinand_ext_profile_t *  ext_profile = NULL;
    spinand_access_config_t *access      = NULL;
    spinand_cr_mode_t *      pst_cr_mode = NULL;
    struct spiflash_config   config;

    if (!handle->sni)
        return ERR_SPINAND_INVALID;

    info        = &handle->sni->spinand_info;
    ext_info    = &handle->sni->spinand_ext_info;
    ext_profile = &ext_info->profile;
    access      = &ext_profile->access;

    handle->fcie_if             = 0;
    handle->soc_ecc_en          = 0;
    handle->bdma_to_xzdec_en    = 0;
    handle->order.rfc           = SPI_NAND_CMD_RFC;
    handle->order.dummy         = SPI_NAND_CMD_RFC_DUMMY;
    handle->order.program_load  = SPI_NAND_CMD_PL;
    handle->order.random_load   = SPI_NAND_CMD_RPL;
    handle->order.cr_mode       = SPINAND_NO_CR;
    handle->order.max_wait_time = ext_profile->max_wait_time;
    handle->msg.bdma_en         = !info->riu_read;

#if defined(CONFIG_FLASH_SOC_ECC)
    handle->soc_ecc_en = hal_fcie_is_soc_ecc();
#endif

    drv_spinand_setup_access(handle, access->read.command, access->read.address_bytes, access->read.dummy,
                             access->program.command, access->random.command);

    config.cmd        = handle->order.rfc;
    config.rate       = info->max_clk;
    config.have_phase = info->have_phase;
    config.phase      = info->phase;
    config.cs_select  = handle->msg.cs_select;
    spiflash_setup(handle->ctrl_id, &config);

    if (handle->msg.bdma_en && !handle->soc_ecc_en)
    {
        if (SPINAND_NONE_BUF_MODE == access->cr_mode.cr_type)
            handle->order.cr_mode = SPINAND_NONE_BUF_MODE;
        else if (SPINAND_BUF_MODE == access->cr_mode.cr_type)
            handle->order.cr_mode = SPINAND_BUF_MODE;
    }

    if (handle->order.cr_mode == SPINAND_NONE_BUF_MODE)
    {
        pst_cr_mode = &ext_info->profile.access.cr_mode;
        config.cmd  = pst_cr_mode->cr_profile.none_buf_mode.load.command;
        spiflash_setup(handle->ctrl_id, &config);
    }

    return ERR_SPINAND_SUCCESS;
}

u8 mdrv_spinand_hardware_init(struct spinand_handle *handle)
{
    spinand_ext_info_t *   ext_info    = NULL;
    spinand_ext_profile_t *ext_profile = NULL;

    if (!handle->sni)
        return ERR_SPINAND_INVALID;

    handle->die_id = 0;
    mdrv_spinand_setup_by_sni(handle);

    flash_impl_printf_hex("[SPINAND] RFC use command 0x", handle->order.rfc, "\r\n");
    flash_impl_printf_hex("[SPINAND] dummy clock 0x", handle->order.dummy, "\r\n");
    flash_impl_printf_hex("[SPINAND] Program with command 0x", handle->order.program_load, ".\r\n");
    flash_impl_printf_hex("[SPINAND] Random with command 0x", handle->order.random_load, ".\r\n");

    if (handle->order.cr_mode == SPINAND_NONE_BUF_MODE)
        flash_impl_printf("[FLASH] Load without buffer mode\r\n");
    else if (handle->order.cr_mode == SPINAND_BUF_MODE)
        flash_impl_printf("[FLASH] Load with buffer mode\r\n");

    if (handle->msg.bdma_en)
        flash_impl_printf("[FLASH] BDMA mode.\r\n");
    else
        flash_impl_printf("[FLASH] RIU mode.\r\n");

    if (handle->soc_ecc_en)
        flash_impl_printf("[FLASH] Use Soc Ecc.\r\n");

    ext_info    = &handle->sni->spinand_ext_info;
    ext_profile = &ext_info->profile;

    drv_spinand_reg_init(handle);

    if (ext_profile->flags & SPINAND_NEED_QE)
    {
        drv_spinand_switch_config(handle, &ext_info->profile.access.qe_status, 1);
    }

    if (ext_profile->ecc_config.ecc_type & SPINAND_THRESHOLD)
    {
        drv_spinand_switch_config(handle, &ext_info->profile.ecc_config.threshold, 1);
    }

    mdrv_spinand_unlock_whole_block(handle);

    return ERR_SPINAND_SUCCESS;
}

u8 mdrv_spinand_reset(struct spinand_handle *handle)
{
    if (ERR_SPINAND_SUCCESS != drv_spinand_reset(handle))
        return ERR_SPINAND_TIMEOUT;

    if (ERR_SPINAND_TIMEOUT == drv_spinand_check_status(handle, NULL))
        return ERR_SPINAND_DEVICE_FAILURE;

    return ERR_SPINAND_SUCCESS;
}

u8 mdrv_spinand_is_id_match(struct spinand_handle *handle, u8 *id, u8 id_len)
{
    u8 i;
    u8 device_id[8];

    if (ERR_SPINAND_SUCCESS != mdrv_spinand_read_id(handle, device_id, id_len))
        return 0;

    for (i = 0; id_len > i; i++)
    {
        if (device_id[i] != id[i])
        {
            return 0;
        }
    }

    return 1;
}

u8 mdrv_spinand_read_id(struct spinand_handle *handle, u8 *data, u8 size)
{
    return drv_spinand_complete_get_features(handle, SPI_NAND_CMD_RDID, 0, data, size);
}

u8 mdrv_spinand_read_status(struct spinand_handle *handle, u8 *pstatus)
{
    return drv_spinand_check_status(handle, pstatus);
}

u8 mdrv_spinand_setup(struct spinand_handle *handle)
{
    spinand_info_t *       info             = NULL;
    spinand_ext_info_t *   spinand_ext_info = NULL;
    spinand_ext_profile_t *profile          = NULL;
    struct spiflash_config config           = {0};

    if (!handle->sni)
        return ERR_SPINAND_INVALID;

    info             = &handle->sni->spinand_info;
    spinand_ext_info = &handle->sni->spinand_ext_info;
    profile          = &spinand_ext_info->profile;

    config.cmd        = handle->order.rfc;
    config.rate       = info->max_clk;
    config.have_phase = info->have_phase;
    config.phase      = info->phase;
    config.cs_select  = handle->msg.cs_select;
    spiflash_setup(handle->ctrl_id, &config);

    if (profile->flags & SPINAND_NEED_QE)
    {
        drv_spinand_switch_config(handle, &spinand_ext_info->profile.access.qe_status, 1);
    }

    if (profile->ecc_config.ecc_type & SPINAND_THRESHOLD)
    {
        drv_spinand_switch_config(handle, &spinand_ext_info->profile.ecc_config.threshold, 1);
    }

    if (profile->flags & SPINAND_ALL_LOCK)
    {
        drv_spinand_switch_config(handle, &profile->ext_config.protect_status.blockStatus.blocks, 0);
    }

    return ERR_SPINAND_SUCCESS;
}

u8 mdrv_spinand_unlock_whole_block(struct spinand_handle *handle)
{
    spinand_ext_info_t *ext_sni        = NULL;
    spinand_protect_t * protect_status = NULL;

    if (!handle->sni)
        return ERR_SPINAND_INVALID;

    ext_sni        = &handle->sni->spinand_ext_info;
    protect_status = &ext_sni->profile.ext_config.protect_status;

    if (ext_sni->profile.flags & SPINAND_MULTI_DIES)
    {
        drv_spinand_die_sel(handle, 0);
    }

    if (SPINAND_ALL_LOCK & ext_sni->profile.flags)
    {
        drv_spinand_switch_config(handle, &protect_status->blockStatus.blocks, 0);
    }

    if (ext_sni->profile.flags & SPINAND_MULTI_DIES)
    {
        drv_spinand_die_sel(handle, ext_sni->profile.ext_config.die_config.die_size);
        drv_spinand_switch_config(handle, &protect_status->blockStatus.blocks, 0);
        drv_spinand_die_sel(handle, 0);
    }

    return ERR_SPINAND_SUCCESS;
}

u8 mdrv_spinand_page_read_raw(struct spinand_handle *handle, u32 page, u16 column, u8 *data, u32 size)
{
    spinand_ext_info_t *spinand_ext_info = NULL;

    if (!handle->sni)
        return ERR_SPINAND_INVALID;

    spinand_ext_info = &handle->sni->spinand_ext_info;

    if (!handle->soc_ecc_en)
        drv_spinand_switch_config(handle, &spinand_ext_info->profile.ext_config.ecc_enabled, 0);

    if (ERR_SPINAND_TIMEOUT <= mdrv_spinand_page_data_read(handle, page))
        return ERR_SPINAND_DEVICE_FAILURE;

    if (ERR_SPINAND_SUCCESS != drv_spinand_read_from_cache(handle, page, column, data, size))
        return ERR_SPINAND_DEVICE_FAILURE;

    if (!handle->soc_ecc_en)
        drv_spinand_switch_config(handle, &spinand_ext_info->profile.ext_config.ecc_enabled, 1);

    return ERR_SPINAND_SUCCESS;
}

u8 mdrv_spinand_page_program_raw(struct spinand_handle *handle, u32 page, u16 column, u8 *data, u32 size)
{
    spinand_ext_info_t *spinand_ext_info = NULL;

    if (!handle->sni)
        return ERR_SPINAND_INVALID;

    spinand_ext_info = &handle->sni->spinand_ext_info;

    if (!handle->soc_ecc_en)
        drv_spinand_switch_config(handle, &spinand_ext_info->profile.ext_config.ecc_enabled, 0);

    drv_spinand_die_sel(handle, page);

    if (ERR_SPINAND_SUCCESS != drv_spinand_write_enable(handle))
        return ERR_SPINAND_DEVICE_FAILURE;

    drv_spinand_wait_device_available(handle);

    if (ERR_SPINAND_SUCCESS != drv_spinand_write_to_cache(handle, page, column, data, size))
        return ERR_SPINAND_DEVICE_FAILURE;

    drv_spinand_wait_device_available(handle);

    if (ERR_SPINAND_SUCCESS != drv_spinand_program_execute(handle, page))
        return ERR_SPINAND_DEVICE_FAILURE;

    if (ERR_SPINAND_P_FAIL == drv_spinand_check_status(handle, NULL))
    {
        flash_impl_printf("[FLASH] Program fail!\r\n");
        return ERR_SPINAND_P_FAIL;
    }

    if (!handle->soc_ecc_en)
        drv_spinand_switch_config(handle, &spinand_ext_info->profile.ext_config.ecc_enabled, 1);

    return ERR_SPINAND_SUCCESS;
}

u8 mdrv_spinand_page_read(struct spinand_handle *handle, u32 page, u16 column, u8 *data, u32 size)
{
    u8 status = ERR_SPINAND_SUCCESS;

#if defined(CONFIG_FLASH_SOC_ECC)
    if (handle->soc_ecc_en)
    {
        if (ERR_SPINAND_TIMEOUT <= (status = mdrv_spinand_page_data_read(handle, page)))
        {
            return status;
        }

        status = drv_spinand_soc_ecc_read(handle, page, column, data, size);
    }
    else
#endif
    {
        if (ERR_SPINAND_TIMEOUT <= (status = mdrv_spinand_page_data_read(handle, page)))
        {
            return status;
        }

        if (ERR_SPINAND_SUCCESS != drv_spinand_read_from_cache(handle, page, column, data, size))
        {
            return ERR_SPINAND_DEVICE_FAILURE;
        }
    }

    return status;
}

u8 mdrv_spinand_page_program(struct spinand_handle *handle, u32 page, u16 column, u8 *data, u32 size)
{
    if (!handle->sni)
        return ERR_SPINAND_INVALID;

    drv_spinand_die_sel(handle, page);

    if (ERR_SPINAND_SUCCESS != drv_spinand_write_enable(handle))
        return ERR_SPINAND_DEVICE_FAILURE;

    drv_spinand_wait_device_available(handle);

#if defined(CONFIG_FLASH_SOC_ECC)
    if (handle->soc_ecc_en)
    {
        if (ERR_SPINAND_SUCCESS != drv_spinand_soc_ecc_write(handle, page, column, data, size))
            return ERR_SPINAND_DEVICE_FAILURE;
    }
    else
#endif
    {
        if (ERR_SPINAND_SUCCESS != drv_spinand_write_to_cache(handle, page, column, data, size))
            return ERR_SPINAND_DEVICE_FAILURE;
    }

    drv_spinand_wait_device_available(handle);

    if (ERR_SPINAND_SUCCESS != drv_spinand_program_execute(handle, page))
        return ERR_SPINAND_DEVICE_FAILURE;

    if (ERR_SPINAND_P_FAIL == drv_spinand_check_status(handle, NULL))
    {
        flash_impl_printf("[FLASH] Program fail!\r\n");
        return ERR_SPINAND_P_FAIL;
    }

    return ERR_SPINAND_SUCCESS;
}

u8 mdrv_spinand_block_erase(struct spinand_handle *handle, u32 page)
{
    u32             block = 0;
    spinand_info_t *info  = NULL;

    if (!handle->sni)
        return ERR_SPINAND_INVALID;

    info = &handle->sni->spinand_info;

    block = page >> flash_impl_count_bits(info->blk_page_cnt);

    drv_spinand_die_sel(handle, page);

    if (ERR_SPINAND_SUCCESS != drv_spinand_write_enable(handle))
        return ERR_SPINAND_DEVICE_FAILURE;

    drv_spinand_wait_device_available(handle);

    if (ERR_SPINAND_SUCCESS != drv_spinand_block_erase(handle, page))
        return ERR_SPINAND_DEVICE_FAILURE;

    if (ERR_SPINAND_E_FAIL == drv_spinand_check_status(handle, NULL))
    {
        flash_impl_printf_hex("[FLASH] Erase fail @ block 0x", block, "!\r\n");
        return ERR_SPINAND_E_FAIL;
    }

    return ERR_SPINAND_SUCCESS;
}

u8 mdrv_spinand_is_support_otp(struct spinand_handle *handle)
{
    spinand_ext_configuration_t *spinand_ext_configuration = NULL;

    if (!handle->sni)
        return 0;

    spinand_ext_configuration = &handle->sni->spinand_ext_configuration;

    if (spinand_ext_configuration->otp_config.otp_en)
        return 1;

    return 0;
}

u8 mdrv_spinand_get_otp_layout(struct spinand_handle *handle, u32 *start, u32 *length, u8 mode)
{
    spinand_ext_configuration_t *spinand_ext_configuration = NULL;
    spinand_otp_config_t *       otp_config                = NULL;
    spinand_otp_info *           factory                   = NULL;
    spinand_otp_info *           user                      = NULL;

    if (!handle->sni)
        return ERR_SPINAND_INVALID;

    spinand_ext_configuration = &handle->sni->spinand_ext_configuration;
    otp_config                = &spinand_ext_configuration->otp_config;
    factory                   = &otp_config->factory;
    user                      = &otp_config->user;

    if (!otp_config->otp_en)
        return ERR_SPINAND_DEVICE_FAILURE;

    if (mode)
    {
        *start  = user->start;
        *length = user->length;
    }
    else
    {
        *start  = factory->start;
        *length = factory->length;
    }

    return ERR_SPINAND_SUCCESS;
}

u8 mdrv_spinand_set_otp_mode(struct spinand_handle *handle, u8 enabled)
{
    spinand_ext_info_t *         spinand_ext_info          = NULL;
    spinand_ext_configuration_t *spinand_ext_configuration = NULL;

    if (!handle->sni)
        return ERR_SPINAND_INVALID;

    spinand_ext_info          = &handle->sni->spinand_ext_info;
    spinand_ext_configuration = &handle->sni->spinand_ext_configuration;

    if (!spinand_ext_configuration->otp_config.otp_en)
    {
        return ERR_SPINAND_INVALID;
    }

    drv_spinand_switch_config(handle, &spinand_ext_info->profile.ext_config.otp.otp_enabled, enabled);

    return ERR_SPINAND_SUCCESS;
}

u8 mdrv_spinand_get_otp_lock(struct spinand_handle *handle)
{
    u32                 otp_status;
    spinand_ext_info_t *spinand_ext_info = NULL;
    spinand_otp_t *     pst_otp;

    if (!handle->sni)
        return 0;

    spinand_ext_info = &handle->sni->spinand_ext_info;
    pst_otp          = &spinand_ext_info->profile.ext_config.otp;

    if (ERR_SPINAND_SUCCESS
        != drv_spinand_complete_get_features(handle, SPI_NAND_CMD_GF, pst_otp->otp_lock.address, (u8 *)&otp_status,
                                             pst_otp->otp_enabled.data_bytes))
    {
        flash_impl_printf("[FLASH_ERR]get features failed\r\n");
        return 0;
    }

    if (otp_status & pst_otp->otp_lock.value)
    {
        return 1;
    }

    return 0;
}

u8 mdrv_spinand_set_otp_lock(struct spinand_handle *handle)
{
    u8                  status           = ERR_SPINAND_SUCCESS;
    spinand_ext_info_t *spinand_ext_info = NULL;
    spinand_otp_t *     pst_otp          = NULL;

    if (!handle->sni)
        return ERR_SPINAND_INVALID;

    spinand_ext_info = &handle->sni->spinand_ext_info;
    pst_otp          = &spinand_ext_info->profile.ext_config.otp;

    if (!mdrv_spinand_set_otp_mode(handle, 1))
    {
        flash_impl_printf("[FLASH] enable opt fail\r\n");
        return ERR_SPINAND_DEVICE_FAILURE;
    }

    do
    {
        if (ERR_SPINAND_SUCCESS != (status = drv_spinand_write_enable(handle)))
            break;

        drv_spinand_switch_config(handle, &pst_otp->otp_lock, 1);

        if (ERR_SPINAND_SUCCESS != (status = drv_spinand_execute(handle)))
            break;

        if (ERR_SPINAND_TIMEOUT <= (status = drv_spinand_check_status(handle, NULL)))
        {
            flash_impl_printf("[FLASH] write otp timeout fail\r\n");
            break;
        }
    } while (0);

    if (!mdrv_spinand_set_otp_mode(handle, 0))
    {
        flash_impl_printf("[FLASH] disable opt fail\r\n");
        return ERR_SPINAND_DEVICE_FAILURE;
    }

    return status;
}

u32 mdrv_spinand_read_otp(struct spinand_handle *handle, u32 page, u16 column, u8 *data, u32 size)
{
    u32                          read_size;
    spinand_info_t *             spinand_info              = NULL;
    spinand_ext_info_t *         spinand_ext_info          = NULL;
    spinand_ext_configuration_t *spinand_ext_configuration = NULL;
    spinand_access_config_t *    access                    = NULL;
    spinand_otp_config_t *       otp_config                = NULL;

    if (!handle->sni)
        return ERR_SPINAND_INVALID;

    spinand_info              = &handle->sni->spinand_info;
    spinand_ext_info          = &handle->sni->spinand_ext_info;
    spinand_ext_configuration = &handle->sni->spinand_ext_configuration;
    access                    = &spinand_ext_info->profile.access;
    otp_config                = &spinand_ext_configuration->otp_config;

    if (!mdrv_spinand_set_otp_mode(handle, 1))
    {
        flash_impl_printf("[FLASH] enable opt fail\r\n");
        return ERR_SPINAND_DEVICE_FAILURE;
    }

    drv_spinand_setup_access(handle, otp_config->otpread.command, otp_config->otpread.address_bytes,
                             otp_config->otpread.dummy, access->program.command, access->random.command);

    read_size = 0;

    if (column & (spinand_info->page_byte_cnt - 1))
    {
        read_size = spinand_info->page_byte_cnt - column;
        read_size = (read_size > size) ? size : read_size;

        mdrv_spinand_page_read(handle, page, column, data, read_size);

        page += 1;
        data += read_size;
        size -= read_size;
    }

    read_size += drv_spinand_read_pages(handle, page, data, size);

    drv_spinand_setup_access(handle, access->read.command, 0, access->read.dummy, access->program.command,
                             access->random.command);

    if (!mdrv_spinand_set_otp_mode(handle, 0))
    {
        flash_impl_printf("[FLASH] disable opt fail\r\n");
        return ERR_SPINAND_DEVICE_FAILURE;
    }

    return read_size;
}

u32 mdrv_spinand_write_otp(struct spinand_handle *handle, u32 page, u16 column, u8 *data, u32 size)
{
    u8                           status                    = ERR_SPINAND_SUCCESS;
    u32                          left_write_size           = size;
    u32                          write_size                = 0;
    spinand_info_t *             spinand_info              = NULL;
    spinand_ext_info_t *         spinand_ext_info          = NULL;
    spinand_ext_configuration_t *spinand_ext_configuration = NULL;
    spinand_access_config_t *    access                    = NULL;
    spinand_otp_config_t *       otp_config                = NULL;

    if (!handle->sni)
        return ERR_SPINAND_INVALID;

    spinand_info              = &handle->sni->spinand_info;
    spinand_ext_info          = &handle->sni->spinand_ext_info;
    spinand_ext_configuration = &handle->sni->spinand_ext_configuration;
    access                    = &spinand_ext_info->profile.access;
    otp_config                = &spinand_ext_configuration->otp_config;

    if (!mdrv_spinand_set_otp_mode(handle, 1))
    {
        flash_impl_printf("[FLASH] enable opt fail\r\n");
        return ERR_SPINAND_DEVICE_FAILURE;
    }

    drv_spinand_setup_access(handle, access->read.command, access->read.address_bytes, access->read.dummy,
                             otp_config->otpprogram.command, access->random.command);

    while (left_write_size)
    {
        column &= (spinand_info->page_byte_cnt - 1);
        write_size = spinand_info->page_byte_cnt - column;
        write_size = (write_size > left_write_size) ? left_write_size : write_size;

        if (ERR_SPINAND_SUCCESS != (status = mdrv_spinand_page_program(handle, page, column, data, write_size)))
            break;

        page += 1;
        column += write_size;
        data += write_size;
        left_write_size -= write_size;
    }

    drv_spinand_setup_access(handle, access->read.command, 0, access->read.dummy, access->program.command,
                             access->random.command);

    if (!mdrv_spinand_set_otp_mode(handle, 0))
    {
        flash_impl_printf("[FLASH] disable opt fail\r\n");
        return ERR_SPINAND_DEVICE_FAILURE;
    }

    return write_size;
}

u8 mdrv_spinand_is_support_ubibbm(struct spinand_handle *handle)
{
    spinand_ext_info_t *spinand_ext_info = NULL;

    if (!handle->sni)
        return 0;

    spinand_ext_info = &handle->sni->spinand_ext_info;

    if (!spinand_ext_info->profile.ecc_config.ecc_en)
        return 0;

    if (SPINAND_ENABLE_UBI_BBM & spinand_ext_info->profile.ecc_config.ecc_type)
        return 1;

    return 0;
}

u8 mdrv_spinand_set_ecc_mode(struct spinand_handle *handle, u8 enabled)
{
    spinand_ext_info_t *spinand_ext_info = NULL;

    if (!handle->sni)
        return ERR_SPINAND_INVALID;

    spinand_ext_info = &handle->sni->spinand_ext_info;

    drv_spinand_switch_config(handle, &spinand_ext_info->profile.ext_config.ecc_enabled, enabled);

    return ERR_SPINAND_SUCCESS;
}

u8 mdrv_spinand_soc_ecc_init(struct spinand_handle *handle, u8 ecc_path, u8 ecc_mode, u8 *fcie_buf)
{
    spinand_ext_info_t *ext_info = NULL;

    if (!fcie_buf)
        return ERR_SPINAND_INVALID;

    handle->ecc.path        = ecc_path;
    handle->ecc.mode        = ecc_mode;
    handle->ecc.fcie_buffer = fcie_buf;

    if (handle->sni)
    {
        ext_info         = &handle->sni->spinand_ext_info;
        handle->ecc.mode = ext_info->profile.ecc_config.ecc_mode;
    }

#if defined(CONFIG_FLASH_SOC_ECC)
    if (FCIE_SUCCESS != sstar_fcie_ecc_set_config(handle->fcie_if, &handle->ecc))
    {
        return ERR_SPINAND_DEVICE_FAILURE;
    }
#else
    return ERR_SPINAND_DEVICE_FAILURE;
#endif

    handle->ecc.sector_cnt = handle->ecc.page_size >> flash_impl_count_bits(handle->ecc.sector_size);

    return ERR_SPINAND_SUCCESS;
}

u8 mdrv_spinand_page_data_read(struct spinand_handle *handle, u32 page)
{
    drv_spinand_die_sel(handle, page);

    if (ERR_SPINAND_SUCCESS != drv_spinand_page_data_read(handle, page))
        return ERR_SPINAND_DEVICE_FAILURE;

    return drv_spinand_check_ecc(handle);
}

u8 mdrv_spinand_block_isbad(struct spinand_handle *handle, u32 page)
{
    u8                       bad         = 0x00;
    u8                       bdma_en     = 0;
    spinand_info_t *         info        = NULL;
    spinand_ext_info_t *     ext_info    = NULL;
    spinand_ext_profile_t *  ext_profile = NULL;
    spinand_access_config_t *access      = NULL;

    if (!handle->sni)
        return 0;

    info                = &handle->sni->spinand_info;
    ext_info            = &handle->sni->spinand_ext_info;
    ext_profile         = &ext_info->profile;
    access              = &ext_profile->access;
    bdma_en             = handle->msg.bdma_en;
    handle->msg.bdma_en = 0;
    drv_spinand_setup_access(handle, SPI_NAND_CMD_RFC, 0, SPI_NAND_CMD_RFC_DUMMY, SPI_NAND_CMD_PL, SPI_NAND_CMD_RPL);

    if (ERR_SPINAND_TIMEOUT <= mdrv_spinand_page_read(handle, page, info->page_byte_cnt, &bad, 1))
    {
        handle->msg.bdma_en = bdma_en;
        drv_spinand_setup_access(handle, access->read.command, access->read.address_bytes, access->read.dummy,
                                 access->program.command, access->random.command);
        return 0;
    }

    handle->msg.bdma_en = bdma_en;
    drv_spinand_setup_access(handle, access->read.command, access->read.address_bytes, access->read.dummy,
                             access->program.command, access->random.command);

    if (0xFF != bad)
    {
        return 1;
    }

    return 0;
}

u32 mdrv_spinand_pages_read(struct spinand_handle *handle, u32 page, u8 *data, u32 size)
{
    if (!handle->sni)
        return ERR_SPINAND_INVALID;

    if (handle->order.cr_mode == SPINAND_NONE_BUF_MODE)
        return drv_spinand_read_pages_without_buf_mode(handle, page, data, size);
    else if (handle->order.cr_mode == SPINAND_BUF_MODE)
        return drv_spinand_read_pages_with_buf_mode(handle, page, data, size);
    else
        return drv_spinand_read_pages(handle, page, data, size);
}

#if defined(CONFIG_FLASH_XZDEC)
u8 mdrv_spinand_page_read_to_xzdec(struct spinand_handle *handle, u32 u32_page, u16 u16_column, u8 *pu8_data,
                                   u32 u32_size)
{
    if (!handle->sni)
        return ERR_SPINAND_INVALID;

    handle->bdma_to_xzdec_en = 1;

    if (ERR_SPINAND_ECC_NOT_CORRECTED <= mdrv_spinand_page_read(handle, u32_page, u16_column, pu8_data, u32_size))
    {
        handle->bdma_to_xzdec_en = 0;
        return ERR_SPINAND_DEVICE_FAILURE;
    }

    handle->bdma_to_xzdec_en = 0;

    return ERR_SPINAND_SUCCESS;
}

u32 mdrv_spinand_pages_read_to_xzdec(struct spinand_handle *handle, u32 u32_page, u8 *pu8_data, u32 u32_size)
{
    u32 actual = 0;

    if (!handle->sni)
        return ERR_SPINAND_INVALID;

    handle->bdma_to_xzdec_en = 1;

    actual = mdrv_spinand_pages_read(handle, u32_page, pu8_data, u32_size);

    handle->bdma_to_xzdec_en = 0;

    return actual;
}
EXPORT_SYMBOL_GPL(mdrv_spinand_page_read_to_xzdec);
EXPORT_SYMBOL_GPL(mdrv_spinand_pages_read_to_xzdec);
#endif

EXPORT_SYMBOL_GPL(mdrv_spinand_info);
EXPORT_SYMBOL_GPL(mdrv_spinand_setup_by_default);
EXPORT_SYMBOL_GPL(mdrv_spinand_setup_by_sni);
EXPORT_SYMBOL_GPL(mdrv_spinand_hardware_init);
EXPORT_SYMBOL_GPL(mdrv_spinand_reset);
EXPORT_SYMBOL_GPL(mdrv_spinand_is_id_match);
EXPORT_SYMBOL_GPL(mdrv_spinand_read_id);
EXPORT_SYMBOL_GPL(mdrv_spinand_read_status);
EXPORT_SYMBOL_GPL(mdrv_spinand_setup);
EXPORT_SYMBOL_GPL(mdrv_spinand_unlock_whole_block);
EXPORT_SYMBOL_GPL(mdrv_spinand_page_read_raw);
EXPORT_SYMBOL_GPL(mdrv_spinand_page_program_raw);
EXPORT_SYMBOL_GPL(mdrv_spinand_page_read);
EXPORT_SYMBOL_GPL(mdrv_spinand_page_program);
EXPORT_SYMBOL_GPL(mdrv_spinand_block_erase);
EXPORT_SYMBOL_GPL(mdrv_spinand_is_support_otp);
EXPORT_SYMBOL_GPL(mdrv_spinand_get_otp_layout);
EXPORT_SYMBOL_GPL(mdrv_spinand_set_otp_mode);
EXPORT_SYMBOL_GPL(mdrv_spinand_get_otp_lock);
EXPORT_SYMBOL_GPL(mdrv_spinand_set_otp_lock);
EXPORT_SYMBOL_GPL(mdrv_spinand_read_otp);
EXPORT_SYMBOL_GPL(mdrv_spinand_write_otp);
EXPORT_SYMBOL_GPL(mdrv_spinand_is_support_ubibbm);
EXPORT_SYMBOL_GPL(mdrv_spinand_set_ecc_mode);
EXPORT_SYMBOL_GPL(mdrv_spinand_soc_ecc_init);
EXPORT_SYMBOL_GPL(mdrv_spinand_page_data_read);
EXPORT_SYMBOL_GPL(mdrv_spinand_block_isbad);
EXPORT_SYMBOL_GPL(mdrv_spinand_pages_read);
