/*
 * hal_fsp_qspi.c- Sigmastar
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
#include <hal_fsp_qspi.h>

#define HAL_FSP_USE_SINGLE_CMD 1
#define HAL_FSP_USE_TWO_CMDS   2
#define HAL_FSP_USE_THREE_CMDS 3

#define HAL_FSP_MAX_READ_BUF_CNT      10
#define HAL_FSP_MAX_WRITE_BUF_CNT     15
#define HAL_FSP_WRITE_BUF_JUMP_OFFSET 0x0A

#define HAL_FSP_CHK_NUM_WAITDONE 10000

#define HAL_REG_FSP_WRITE_BUFF         (0x60)
#define HAL_REG_FSP_READ_BUFF          (0x65)
#define HAL_REG_FSP_WRITE_BUFF2        (0x70)
#define HAL_FSP_REG_RW_BUF(hal, index) ((hal->fsp_base) + (((index) << 1) - ((index)&1)))

/* FSP_REG_CTRL01 */
#define HAL_FSP_ENABLE            (0x1 << 0)
#define HAL_FSP_RESET             (0x1 << 1)
#define HAL_FSP_INT               (0x1 << 2)
#define HAL_FSP_AUTOCHECK         (0x1 << 3)
#define HAL_FSP_ENABLE_SECOND_CMD (0x1 << 15)
#define HAL_FSP_ENABLE_THIRD_CMD  (0x1 << 14)

/* FSP_REG_CTRL23 */
#define HAL_FSP_ACCESS_MODE (0x3)
#define HAL_FSP_SINGLE_MODE (0x0)
#define HAL_FSP_DUAL_MODE   (0x2)
#define HAL_FSP_QUAD_MODE   (0x1)
/* FSP_REG_WBF_SIZE_OUTSIDE */
#define HAL_FSP_WBF_SIZE_OUTSIDE_SIZE (0x1FFF)
#define HAL_FSP_WBF_SIZE_OUTSIDE(x)   ((x)&0x1FFF)
/* FSP_REG_WBF_OUTSIDE */
#define HAL_FSP_WBF_REPLACED(x)      (x & 0xFF)
#define HAL_FSP_WBF_MODE(x)          ((x & 0xF) << 8)
#define HAL_FSP_WBF_OUTSIDE_EN       (1 << 12)
#define HAL_FSP_WBF_OUTSIDE_SRC_BDMA (0x00)
#define HAL_FSP_WBF_OUTSIDE_SRC(x)   ((x & 0x3) << 14)

/* QSPI_REG_SW_MODE */
#define HAL_QSPI_REG_SPI_SW_CS_EN        0x01
#define HAL_QSPI_REG_SPI_SW_CS_PULL_HIGH 0x02
/* QSPI_REG_TIMEOUT_CNT_CTRL */
#define HAL_QSPI_REG_SPI_TIMEOUT_EN  0x8000
#define HAL_QSPI_REG_SPI_TIMEOUT_RST 0x4000
/* QSPI_REG_CFG_CKG_SPI */
#define HAL_QSPI_USER_DUMMY_EN    0x10
#define HAL_QSPI_4_BYTE_ADDR_MODE 0x800
/* QSPI_REG_MODE_SEL */
#define HAL_QSPI_NORMAL_MODE 0x00
#define HAL_QSPI_FAST_READ   0x01
#define HAL_QSPI_CMD_3B      0x02
#define HAL_QSPI_CMD_BB      0x03
#define HAL_QSPI_CMD_ED      0x07
#define HAL_QSPI_CMD_6B      0x0A
#define HAL_QSPI_CMD_EB      0x0B
/* QSPI_REG_DEBUG_BUS_1 */
#define HAL_QSPI_REG_WAIT_SELECT_MODE (0x1F << 10)
/* QSPI_REG_CHIP_SELECT */
#define HAL_QSPI_CHIP_SELECT_MASK (0x3)
/* QSPI_REG_FUNC_SETTING_DEF0 */
#define HAL_QSPI_ADDR4_EN 0x4
#define HAL_QSPI_ADDR2_EN 0x800
#define HAL_QSPI_DUMMY_EN 0x1000

#define HAL_FSP_REG_WRITE_SIZE(hal)       ((hal->fsp_base) + ((0x6A) << 2))
#define HAL_FSP_REG_READ_SIZE(hal)        ((hal->fsp_base) + ((0x6B) << 2))
#define HAL_FSP_REG_TRIGGER(hal)          ((hal->fsp_base) + ((0x6D) << 2))
#define HAL_FSP_REG_CTRL01(hal)           ((hal->fsp_base) + ((0x6C) << 2))
#define HAL_FSP_REG_DONE(hal)             ((hal->fsp_base) + ((0x6E) << 2))
#define HAL_FSP_REG_CLEAR_DONE(hal)       ((hal->fsp_base) + ((0x6F) << 2))
#define HAL_FSP_REG_CTRL23(hal)           ((hal->fsp_base) + ((0x75) << 2))
#define HAL_FSP_REG_CTRL4(hal)            ((hal->fsp_base) + ((0x76) << 2))
#define HAL_FSP_REG_WBF_SIZE_OUTSIDE(hal) ((hal->fsp_base) + ((0x78) << 2))
#define HAL_FSP_REG_WBF_OUTSIDE(hal)      ((hal->fsp_base) + ((0x79) << 2))

#define HAL_QSPI_REG_SW_MODE(hal)                ((hal->qspi_base) + ((0x0A) << 2))
#define HAL_QSPI_REG_DEF_DTR_1_4_4_L(hal)        ((hal->qspi_base) + ((0x42) << 2))
#define HAL_QSPI_REG_SPI_DTR_HIGHSPEED_MODE(hal) ((hal->qspi_base) + ((0x47) << 2))
#define HAL_QSPI_REG_TIMEOUT_CNT_VALUE(hal)      ((hal->qspi_base) + ((0x66) << 2))
#define HAL_QSPI_REG_TIMEOUT_CNT_CTRL(hal)       ((hal->qspi_base) + ((0x67) << 2))
#define HAL_QSPI_REG_CFG_CKG_SPI(hal)            ((hal->qspi_base) + ((0x70) << 2))
#define HAL_QSPI_REG_MODE_SEL(hal)               ((hal->qspi_base) + ((0x72) << 2))
#define HAL_QSPI_REG_DEBUG_BUS_1(hal)            ((hal->qspi_base) + ((0x77) << 2))
#define HAL_QSPI_REG_CHIP_SELECT(hal)            ((hal->qspi_base) + ((0x7A) << 2))
#define HAL_QSPI_REG_FUNC_SETTING_DEF0(hal)      ((hal->qspi_base) + ((0x7D) << 2))

#define HAL_PIU_NONPM_BASE       (0x1F200E00)
#define HAL_SW_SPI_MASK_ACK_EN   (HAL_PIU_NONPM_BASE + (0x1a << 2))
#define HAL_SPI_BURST_CTRL_DEBUG (HAL_PIU_NONPM_BASE + (0x34 << 2))

#define HAL_OTP_BASE                       (0x1F203E00)
#define HAL_OTP_CA53_BOOT_FROM_PM_SPI      (HAL_OTP_BASE + (0x30 << 2))
#define HAL_OTP_CA53_BOOT_FROM_PM_SPI_MASK (0x0003)

#define HAL_CLKGEN_BASE        (0x1F207000)
#define HAL_CLKGEN_REG_CKG_SPI (HAL_CLKGEN_BASE + (0x32 << 2))

#define HAL_SPI_CTRL_BASE        (0x1F23A800)
#define HAL_SPI_CTRL_REG_CKG_SPI (HAL_SPI_CTRL_BASE + (0x2A << 2))

#define HAL_READ_BYTE(_reg_) (*(volatile unsigned char *)(FLASH_IMPL_IO_ADDRESS(_reg_)))
#define HAL_WRITE_BYTE(_reg_, _val_) \
    ((*(volatile unsigned char *)(FLASH_IMPL_IO_ADDRESS(_reg_))) = (unsigned char)(_val_))
#define HAL_WRITE_BYTE_MASK(_reg_, _val_, mask)                    \
    ((*(volatile unsigned char *)(FLASH_IMPL_IO_ADDRESS(_reg_))) = \
         ((*(volatile unsigned char *)(FLASH_IMPL_IO_ADDRESS(_reg_))) & ~(mask)) | ((unsigned char)(_val_) & (mask)))
#define HAL_READ_WORD(_reg_) (*(volatile unsigned short *)(FLASH_IMPL_IO_ADDRESS(_reg_)))
#define HAL_WRITE_WORD(_reg_, _val_) \
    ((*(volatile unsigned short *)(FLASH_IMPL_IO_ADDRESS(_reg_))) = (unsigned short)(_val_))
#define HAL_WRITE_WORD_MASK(_reg_, _val_, mask)                                   \
    ((*(volatile unsigned short *)(FLASH_IMPL_IO_ADDRESS(_reg_))) =               \
         ((*(volatile unsigned short *)(FLASH_IMPL_IO_ADDRESS(_reg_))) & ~(mask)) \
         | ((unsigned short)(_val_) & (mask)))

void __attribute__((weak)) spipll_160m_init(void) {}
void __attribute__((weak)) spipll_172m_init(void) {}
void __attribute__((weak)) spipll_208m_init(void) {}
void __attribute__((weak)) spipll_216m_init(void) {}

static void hal_fsp_qspi_reset(struct fsp_qspi_hal *hal, u8 cmd_cnt)
{
    u16 u16_fsp_ctrl0;

    u16_fsp_ctrl0 = HAL_FSP_ENABLE | HAL_FSP_RESET | HAL_FSP_INT;

    if (2 == cmd_cnt)
    {
        u16_fsp_ctrl0 |= HAL_FSP_ENABLE_SECOND_CMD;
    }
    else if (3 == cmd_cnt)
    {
        u16_fsp_ctrl0 |= HAL_FSP_ENABLE_THIRD_CMD;
    }

    HAL_WRITE_WORD(HAL_FSP_REG_CTRL01(hal), u16_fsp_ctrl0);
    HAL_WRITE_WORD(HAL_FSP_REG_CTRL23(hal), 0);
    HAL_WRITE_WORD(HAL_FSP_REG_CTRL4(hal), 0);

    hal->wd_reg         = (HAL_REG_FSP_WRITE_BUFF << 1);
    hal->wd_index       = 0;
    hal->wbf_cmd_size   = 0;
    hal->rbf_reply_size = 0;
}

static u8 hal_fsp_qspi_set_wbf_size(struct fsp_qspi_hal *hal, u8 which, u32 count)
{
    u8 u8_count;

    u8_count = count & 0xFF;
    if (HAL_FSP_MAX_WRITE_BUF_CNT < count)
    {
        u8_count = HAL_FSP_MAX_WRITE_BUF_CNT;
    }

    hal->wbf_cmd_size |= u8_count << (4 * (which - 1));

    return u8_count;
}

static u8 hal_fsp_qspi_set_rbf_size(struct fsp_qspi_hal *hal, u8 which, u32 receive)
{
    u8 u8_rbf_size;

    u8_rbf_size = receive;

    if (HAL_FSP_MAX_READ_BUF_CNT < receive)
    {
        u8_rbf_size = HAL_FSP_MAX_READ_BUF_CNT;
    }

    hal->rbf_reply_size |= ((u8_rbf_size & 0xff) << (4 * (which - 1)));

    return u8_rbf_size;
}

static u8 hal_fsp_qspi_write_wbf(struct fsp_qspi_hal *hal, u8 *buf, u8 size)
{
    u8 bytes_written = 0;

    for (bytes_written = 0; HAL_FSP_MAX_WRITE_BUF_CNT > hal->wd_index && size > bytes_written; bytes_written++)
    {
        if (HAL_FSP_WRITE_BUF_JUMP_OFFSET == hal->wd_index)
        {
            hal->wd_reg = (HAL_REG_FSP_WRITE_BUFF2 << 1);
        }

        HAL_WRITE_BYTE(HAL_FSP_REG_RW_BUF(hal, hal->wd_reg), *buf);
        hal->wd_reg++;
        hal->wd_index++;
        buf++;
    }

    return bytes_written;
}

static u8 hal_fsp_qspi_read_rbf(struct fsp_qspi_hal *hal, u8 *buf, u8 size)
{
    u8 bytes_read;

    for (bytes_read = 0; size > bytes_read && HAL_FSP_MAX_READ_BUF_CNT > bytes_read; bytes_read++)
    {
        *buf = HAL_READ_BYTE(HAL_FSP_REG_RW_BUF(hal, (HAL_REG_FSP_READ_BUFF << 1) + bytes_read));
        buf++;
    }

    return bytes_read;
}

static void hal_fsp_qspi_trigger(struct fsp_qspi_hal *hal)
{
    HAL_WRITE_WORD(HAL_FSP_REG_WRITE_SIZE(hal), hal->wbf_cmd_size);
    HAL_WRITE_WORD(HAL_FSP_REG_READ_SIZE(hal), hal->rbf_reply_size);
    HAL_WRITE_WORD(HAL_FSP_REG_TRIGGER(hal), 0x1);
}

static u8 hal_fsp_qspi_wait_done(struct fsp_qspi_hal *hal)
{
    // consider as it spend very long time to check if FSP done, so it may implment timeout method to improve
    u16 try = 0;

    while (try < HAL_FSP_CHK_NUM_WAITDONE)
    {
        if (HAL_READ_WORD(HAL_FSP_REG_DONE(hal)) & 0x1)
        {
            return 1;
        }

        try++;
    }

    flash_impl_printf("[FSP] wait fsp done timeout!!!\r\n");

    return 0;
}

static void hal_fsp_qspi_clear_trigger(struct fsp_qspi_hal *hal)
{
    HAL_WRITE_WORD(HAL_FSP_REG_CLEAR_DONE(hal), 0x1);
    HAL_WRITE_WORD(HAL_FSP_REG_WRITE_SIZE(hal), 0);
    HAL_WRITE_WORD(HAL_FSP_REG_READ_SIZE(hal), 0);

    hal->wd_reg         = (HAL_REG_FSP_WRITE_BUFF << 1);
    hal->wd_index       = 0;
    hal->wbf_cmd_size   = 0;
    hal->rbf_reply_size = 0;
}

static u8 hal_fsp_qspi_cmd_to_read_mode(u8 cmd)
{
    u8 mode_cmd;

    switch (cmd)
    {
        case 0x03:
            mode_cmd = HAL_QSPI_NORMAL_MODE;
            break;
        case 0x0B:
            mode_cmd = HAL_QSPI_FAST_READ;
            break;
        case 0x3B:
            mode_cmd = HAL_QSPI_CMD_3B;
            break;
        case 0xBB:
            mode_cmd = HAL_QSPI_CMD_BB;
            break;
        case 0x6B:
            mode_cmd = HAL_QSPI_CMD_6B;
            break;
        case 0xEB:
            mode_cmd = HAL_QSPI_CMD_EB;
            break;
        case 0xEE:
        case 0xED:
            mode_cmd = HAL_QSPI_CMD_ED;
            break;
        default:
            mode_cmd = HAL_QSPI_NORMAL_MODE;
    }

    return mode_cmd;
}

static void hal_fsp_qspi_select_read_mode(struct fsp_qspi_hal *hal, u8 cmd, u8 addr_bytes, u8 dummy_cyc)
{
    u16 set              = 0;
    u16 cs_timeout_en    = hal->cs_timeout_en;
    u32 cs_timeout_value = hal->cs_timeout_value;

    set |= (addr_bytes == 2) ? HAL_QSPI_ADDR2_EN : ((addr_bytes == 4) ? HAL_QSPI_ADDR4_EN : 0);
    set |= (dummy_cyc > 0) ? HAL_QSPI_DUMMY_EN : 0;

    HAL_WRITE_WORD(HAL_QSPI_REG_FUNC_SETTING_DEF0(hal), set);

    if (addr_bytes == 4)
    {
        HAL_WRITE_WORD_MASK(HAL_QSPI_REG_CFG_CKG_SPI(hal), HAL_QSPI_4_BYTE_ADDR_MODE, HAL_QSPI_4_BYTE_ADDR_MODE);
    }
    else
    {
        HAL_WRITE_WORD_MASK(HAL_QSPI_REG_CFG_CKG_SPI(hal), 0, HAL_QSPI_4_BYTE_ADDR_MODE);
    }

    if (dummy_cyc > 0)
    {
        HAL_WRITE_WORD_MASK(HAL_QSPI_REG_CFG_CKG_SPI(hal), HAL_QSPI_USER_DUMMY_EN, HAL_QSPI_USER_DUMMY_EN);
        HAL_WRITE_WORD_MASK(HAL_QSPI_REG_CFG_CKG_SPI(hal), (dummy_cyc - 1), 0xF);
        HAL_WRITE_WORD_MASK(HAL_QSPI_REG_CFG_CKG_SPI(hal), (dummy_cyc > 16) ? 0x40 : 0x00, 0x40);
    }
    else
    {
        HAL_WRITE_WORD_MASK(HAL_QSPI_REG_CFG_CKG_SPI(hal), 0, HAL_QSPI_USER_DUMMY_EN);
    }

    HAL_WRITE_WORD(HAL_QSPI_REG_MODE_SEL(hal), hal_fsp_qspi_cmd_to_read_mode(cmd));

    if (cmd == 0xEE)
    {
        HAL_WRITE_WORD_MASK(HAL_QSPI_REG_FUNC_SETTING_DEF0(hal), 0x8, 0x8);
        HAL_WRITE_WORD_MASK(HAL_QSPI_REG_DEF_DTR_1_4_4_L(hal), 0xEE, 0xFF);
    }

    hal_fsp_qspi_set_timeout(hal, 1, 0);
    while (HAL_QSPI_REG_WAIT_SELECT_MODE & HAL_READ_WORD(HAL_QSPI_REG_DEBUG_BUS_1(hal)))
        ;
    hal_fsp_qspi_set_timeout(hal, cs_timeout_en, cs_timeout_value);
}

static u8 hal_fsp_qspi_cmd_to_access_mode(u8 cmd)
{
    switch (cmd)
    {
        case 0x32:
        case 0x6B:
        case 0x34:
        case 0x38:
        case 0xEB:
            return HAL_FSP_QUAD_MODE;
        case 0x3B:
        case 0xBB:
            return HAL_FSP_DUAL_MODE;
        default:
            return HAL_FSP_SINGLE_MODE;
    }
}

static u8 hal_fsp_qspi_is_read_data_cmd(u8 cmd)
{
    switch (cmd)
    {
        case 0x6B:
        case 0xEB:
        case 0x3B:
        case 0xBB:
        case 0x0B:
        case 0x03:
            return 1;
        default:
            return 0;
    }
}

static u8 hal_fsp_qspi_is_program_cmd(u8 cmd)
{
    switch (cmd)
    {
        case 0x32:
        case 0x02:
        case 0x38:
        case 0x34:
            return 1;
        default:
            return 0;
    }
}

static u8 hal_fsp_qspi_is_dtr_cmd(u8 cmd)
{
    switch (cmd)
    {
        case 0xED:
        case 0xEE:
            return 1;
        default:
            return 0;
    }
}

static void hal_fsp_qspi_set_access_mode(struct fsp_qspi_hal *hal, u8 cmd)
{
    HAL_WRITE_WORD_MASK(HAL_FSP_REG_CTRL23(hal), hal_fsp_qspi_cmd_to_access_mode(cmd), HAL_FSP_ACCESS_MODE);
}

static u32 hal_fsp_qspi_enable_outside_wbf(struct fsp_qspi_hal *hal, u8 src, u8 which_wbf, u8 byte_replaced, u32 size)
{
    size = (size > HAL_FSP_WBF_SIZE_OUTSIDE_SIZE) ? HAL_FSP_WBF_SIZE_OUTSIDE_SIZE : size;

    HAL_WRITE_WORD(HAL_FSP_REG_WBF_OUTSIDE(hal), HAL_FSP_WBF_OUTSIDE_SRC(src) | HAL_FSP_WBF_OUTSIDE_EN
                                                     | HAL_FSP_WBF_MODE(which_wbf)
                                                     | HAL_FSP_WBF_REPLACED(byte_replaced));
    HAL_WRITE_WORD(HAL_FSP_REG_WBF_SIZE_OUTSIDE(hal), HAL_FSP_WBF_SIZE_OUTSIDE(size + 1));

    return size;
}

static void hal_fsp_qspi_disable_outside_wbf(struct fsp_qspi_hal *hal)
{
    HAL_WRITE_WORD(HAL_FSP_REG_WBF_OUTSIDE(hal), 0);
    HAL_WRITE_WORD(HAL_FSP_REG_WBF_SIZE_OUTSIDE(hal), 0);
}

static void hal_fsp_qspi_select_clk_src(struct fsp_qspi_hal *hal, u8 cmd)
{
    u8 rate = 0;

    rate = hal_fsp_qspi_is_dtr_cmd(cmd) ? hal->rate_dtr : hal->rate_str;

    if (hal->rate == rate)
    {
        return;
    }
    else
    {
        hal->rate = rate;
    }

    if (hal->set_rate)
    {
        hal->set_rate(hal, rate);
        return;
    }

    if (hal->fsp_base == 0x1F203600)
    {
        if (rate >= 208)
        {
            HAL_WRITE_BYTE(HAL_SPI_CTRL_REG_CKG_SPI, 0x04);
            HAL_WRITE_BYTE_MASK(HAL_SPI_CTRL_REG_CKG_SPI, (0x1 << 4), (0x1 << 4));
            HAL_WRITE_BYTE(HAL_QSPI_REG_SPI_DTR_HIGHSPEED_MODE(hal), 0x11);
        }
        else if (rate >= 172)
        {
            HAL_WRITE_BYTE(HAL_SPI_CTRL_REG_CKG_SPI, 0x04);
            HAL_WRITE_BYTE_MASK(HAL_SPI_CTRL_REG_CKG_SPI, (0x1 << 4), (0x1 << 4));
            HAL_WRITE_BYTE(HAL_QSPI_REG_SPI_DTR_HIGHSPEED_MODE(hal), 0x11);
        }
        else if (rate >= 160)
        {
            HAL_WRITE_BYTE(HAL_SPI_CTRL_REG_CKG_SPI, 0x04);
            HAL_WRITE_BYTE_MASK(HAL_SPI_CTRL_REG_CKG_SPI, (0x1 << 4), (0x1 << 4));
            HAL_WRITE_BYTE(HAL_QSPI_REG_SPI_DTR_HIGHSPEED_MODE(hal), 0x11);
        }
        else if (rate >= 108)
        {
            HAL_WRITE_BYTE(HAL_CLKGEN_REG_CKG_SPI, 0x08);
            HAL_WRITE_BYTE(HAL_SPI_CTRL_REG_CKG_SPI, 0x00);
            HAL_WRITE_BYTE_MASK(HAL_SPI_CTRL_REG_CKG_SPI, (0x1 << 4), (0x1 << 4));
            HAL_WRITE_BYTE(HAL_QSPI_REG_SPI_DTR_HIGHSPEED_MODE(hal), 0x0);
        }
        else if (rate >= 104)
        {
            HAL_WRITE_BYTE(HAL_SPI_CTRL_REG_CKG_SPI, 0x04);
            HAL_WRITE_BYTE_MASK(HAL_SPI_CTRL_REG_CKG_SPI, (0x1 << 4), (0x1 << 4));
            HAL_WRITE_BYTE(HAL_QSPI_REG_SPI_DTR_HIGHSPEED_MODE(hal), 0x0);
        }
        else if (rate >= 86)
        {
            HAL_WRITE_BYTE(HAL_CLKGEN_REG_CKG_SPI, 0x04);
            HAL_WRITE_BYTE(HAL_SPI_CTRL_REG_CKG_SPI, 0x00);
            HAL_WRITE_BYTE_MASK(HAL_SPI_CTRL_REG_CKG_SPI, (0x1 << 4), (0x1 << 4));
            HAL_WRITE_BYTE(HAL_QSPI_REG_SPI_DTR_HIGHSPEED_MODE(hal), 0x0);
        }
        else if (rate >= 54)
        {
            HAL_WRITE_BYTE(HAL_CLKGEN_REG_CKG_SPI, 0x10);
            HAL_WRITE_BYTE(HAL_SPI_CTRL_REG_CKG_SPI, 0x00);
            HAL_WRITE_BYTE_MASK(HAL_SPI_CTRL_REG_CKG_SPI, (0x1 << 4), (0x1 << 4));
            HAL_WRITE_BYTE(HAL_QSPI_REG_SPI_DTR_HIGHSPEED_MODE(hal), 0x0);
        }
    }
}

u8 hal_fsp_qspi_is_boot_storage(struct fsp_qspi_hal *hal)
{
    u16 value = 0;

    value = HAL_READ_WORD(HAL_OTP_CA53_BOOT_FROM_PM_SPI);
    value &= HAL_OTP_CA53_BOOT_FROM_PM_SPI_MASK;

    if ((value == 0x01) || (value == 0x02))
    {
        return (hal->fsp_base == 0x1F002C00) ? 1 : 0;
    }

    return (hal->fsp_base == 0x1F203600) ? 1 : 0;
}

void hal_fsp_qspi_init(struct fsp_qspi_hal *hal)
{
    FLASH_IMPL_UNUSED_VAR(hal);
    // Fix flash address must 16 align
    HAL_WRITE_WORD_MASK(HAL_SW_SPI_MASK_ACK_EN, 0x1, 0x1);
    HAL_WRITE_WORD_MASK(HAL_SPI_BURST_CTRL_DEBUG, 0x1, 0x1);
}

void hal_fsp_qspi_set_rate(struct fsp_qspi_hal *hal, u8 rate, u8 cmd)
{
    if (!hal->rate_dtr && hal_fsp_qspi_is_dtr_cmd(cmd))
    {
        if (rate >= 108)
        {
            spipll_216m_init();
            flash_impl_printf("SPI 108M\r\n");
            hal->rate_dtr = 216;
            hal->rate_str = 108;
        }
        else if (rate >= 104)
        {
            spipll_208m_init();
            flash_impl_printf("SPI 104M\r\n");
            hal->rate_dtr = 208;
            hal->rate_str = 86;
        }
        else if (rate >= 86)
        {
            spipll_172m_init();
            flash_impl_printf("SPI 86M\r\n");
            hal->rate_dtr = 172;
            hal->rate_str = 86;
        }
        else if (rate >= 80)
        {
            spipll_160m_init();
            flash_impl_printf("SPI 80M\r\n");
            hal->rate_dtr = 160;
            hal->rate_str = 54;
        }
        else if (rate >= 54)
        {
            flash_impl_printf("SPI 54M\r\n");
            hal->rate_dtr = 108;
            hal->rate_str = 54;
        }
        else
        {
            flash_impl_printf("SPI 54M\r\n");
            hal->rate_dtr = 108;
            hal->rate_str = 54;
        }
    }
    else if (!hal->rate_str)
    {
        if (rate >= 108)
        {
            flash_impl_printf("SPI 108M\r\n");
            hal->rate_str = 108;
        }
        else if (rate >= 104)
        {
            flash_impl_printf("SPI 104M\r\n");
            hal->rate_str = 104;
        }
        else if (rate >= 86)
        {
            flash_impl_printf("SPI 86M\r\n");
            hal->rate_str = 86;
        }
        else if (rate >= 54)
        {
            flash_impl_printf("SPI 54M\r\n");
            hal->rate_str = 54;
        }
        else
        {
            flash_impl_printf("SPI 54M\r\n");
            hal->rate_str = 54;
        }
    }
    else
        flash_impl_printf("spi clk already initialized\r\n");

    hal_fsp_qspi_select_clk_src(hal, 0);
}

void hal_fsp_qspi_set_timeout(struct fsp_qspi_hal *hal, u8 enable, u32 val)
{
    if (enable)
    {
        HAL_WRITE_WORD(HAL_QSPI_REG_TIMEOUT_CNT_VALUE(hal), (val & 0xFFFF));
        HAL_WRITE_WORD(HAL_QSPI_REG_TIMEOUT_CNT_CTRL(hal), (val & 0xFF) | HAL_QSPI_REG_SPI_TIMEOUT_EN);
    }
    else
    {
        HAL_WRITE_WORD(HAL_QSPI_REG_TIMEOUT_CNT_CTRL(hal), HAL_QSPI_REG_SPI_TIMEOUT_RST);
    }

    hal->cs_timeout_en    = enable;
    hal->cs_timeout_value = val;
}

void hal_fsp_qspi_use_sw_cs(struct fsp_qspi_hal *hal, u8 enabled)
{
    u8 cs_ctrl;

    cs_ctrl = HAL_READ_BYTE(HAL_QSPI_REG_SW_MODE(hal));

    if (1 == enabled)
    {
        HAL_WRITE_BYTE(HAL_QSPI_REG_SW_MODE(hal), HAL_QSPI_REG_SPI_SW_CS_EN | cs_ctrl);
    }
    else
    {
        HAL_WRITE_BYTE(HAL_QSPI_REG_SW_MODE(hal), ~HAL_QSPI_REG_SPI_SW_CS_EN & cs_ctrl);
    }

    hal->use_sw_cs = !!enabled;
}

void hal_fsp_qspi_pull_cs(struct fsp_qspi_hal *hal, u8 pull_high)
{
    u8 cs_ctrl;

    cs_ctrl = HAL_READ_BYTE(HAL_QSPI_REG_SW_MODE(hal));

    if (1 == pull_high)
    {
        HAL_WRITE_BYTE(HAL_QSPI_REG_SW_MODE(hal), HAL_QSPI_REG_SPI_SW_CS_PULL_HIGH | cs_ctrl);
    }
    else
    {
        HAL_WRITE_BYTE(HAL_QSPI_REG_SW_MODE(hal), ~HAL_QSPI_REG_SPI_SW_CS_PULL_HIGH & cs_ctrl);
    }
}

void hal_fsp_qspi_chip_select(struct fsp_qspi_hal *hal, u8 cs_select)
{
    HAL_WRITE_WORD_MASK(HAL_QSPI_REG_CHIP_SELECT(hal), cs_select, HAL_QSPI_CHIP_SELECT_MASK);
}

u32 hal_fsp_qspi_write(struct fsp_qspi_hal *hal, struct fsp_qspi_command *cmd, u8 *buf, u32 size)
{
    u8 write_size                      = 0;
    u8 wbuf[HAL_FSP_MAX_WRITE_BUF_CNT] = {0};
    u8 wbuf_index                      = 0;

    wbuf[wbuf_index++] = cmd->cmd;

    if (cmd->addr_bytes == 1)
    {
        wbuf[wbuf_index++] = (cmd->address) & 0xFF;
    }
    else if (cmd->addr_bytes == 2)
    {
        wbuf[wbuf_index++] = (cmd->address >> 8) & 0xFF;
        wbuf[wbuf_index++] = (cmd->address) & 0xFF;
    }
    else if (cmd->addr_bytes == 3)
    {
        wbuf[wbuf_index++] = (cmd->address >> 16) & 0xFF;
        wbuf[wbuf_index++] = (cmd->address >> 8) & 0xFF;
        wbuf[wbuf_index++] = (cmd->address) & 0xFF;
    }
    else if (cmd->addr_bytes == 4)
    {
        wbuf[wbuf_index++] = (cmd->address >> 24) & 0xFF;
        wbuf[wbuf_index++] = (cmd->address >> 16) & 0xFF;
        wbuf[wbuf_index++] = (cmd->address >> 8) & 0xFF;
        wbuf[wbuf_index++] = (cmd->address) & 0xFF;
    }

    if (cmd->dummy)
    {
        wbuf_index += cmd->dummy >> 3;
    }

    if (wbuf_index > HAL_FSP_MAX_WRITE_BUF_CNT)
        return 0;

    hal_fsp_qspi_reset(hal, HAL_FSP_USE_SINGLE_CMD);

    if (hal->use_sw_cs && size && hal_fsp_qspi_is_program_cmd(cmd->cmd))
    {
        wbuf_index = 0;
        hal_fsp_qspi_set_access_mode(hal, cmd->cmd);
    }
    else
    {
        hal_fsp_qspi_set_access_mode(hal, 0);
    }

    write_size = hal_fsp_qspi_set_wbf_size(hal, 1, size + wbuf_index);

    write_size -= wbuf_index;

    hal_fsp_qspi_write_wbf(hal, wbuf, wbuf_index);
    hal_fsp_qspi_write_wbf(hal, buf, write_size);

    hal_fsp_qspi_trigger(hal);

    if (!hal_fsp_qspi_wait_done(hal))
    {
        return 0;
    }

    hal_fsp_qspi_clear_trigger(hal);

    return write_size;
}

u32 hal_fsp_qspi_read(struct fsp_qspi_hal *hal, struct fsp_qspi_command *cmd, u8 *buf, u32 size)
{
    u32 read_size;

    u8 wbuf[HAL_FSP_MAX_WRITE_BUF_CNT] = {0};
    u8 wbuf_index                      = 0;

    wbuf[wbuf_index++] = cmd->cmd;

    if (cmd->addr_bytes == 1)
    {
        wbuf[wbuf_index++] = (cmd->address) & 0xFF;
    }
    else if (cmd->addr_bytes == 2)
    {
        wbuf[wbuf_index++] = (cmd->address >> 8) & 0xFF;
        wbuf[wbuf_index++] = (cmd->address) & 0xFF;
    }
    else if (cmd->addr_bytes == 3)
    {
        wbuf[wbuf_index++] = (cmd->address >> 16) & 0xFF;
        wbuf[wbuf_index++] = (cmd->address >> 8) & 0xFF;
        wbuf[wbuf_index++] = (cmd->address) & 0xFF;
    }
    else if (cmd->addr_bytes == 4)
    {
        wbuf[wbuf_index++] = (cmd->address >> 24) & 0xFF;
        wbuf[wbuf_index++] = (cmd->address >> 16) & 0xFF;
        wbuf[wbuf_index++] = (cmd->address >> 8) & 0xFF;
        wbuf[wbuf_index++] = (cmd->address) & 0xFF;
    }

    if (cmd->dummy)
    {
        wbuf_index += cmd->dummy >> 3;
    }

    if (wbuf_index > HAL_FSP_MAX_WRITE_BUF_CNT)
        return 0;

    hal_fsp_qspi_reset(hal, HAL_FSP_USE_SINGLE_CMD);

    if (hal->use_sw_cs && size && hal_fsp_qspi_is_read_data_cmd(cmd->cmd))
    {
        wbuf_index = 0;
        hal_fsp_qspi_set_access_mode(hal, cmd->cmd);
    }
    else
    {
        hal_fsp_qspi_set_access_mode(hal, 0);
    }

    hal_fsp_qspi_set_wbf_size(hal, 1, wbuf_index);
    read_size = hal_fsp_qspi_set_rbf_size(hal, 1, size);

    hal_fsp_qspi_write_wbf(hal, wbuf, wbuf_index);

    hal_fsp_qspi_trigger(hal);

    if (!hal_fsp_qspi_wait_done(hal))
    {
        return 0;
    }

    hal_fsp_qspi_read_rbf(hal, buf, read_size);

    hal_fsp_qspi_clear_trigger(hal);

    return read_size;
}

u32 hal_fsp_qspi_bdma_write(struct fsp_qspi_hal *hal, struct fsp_qspi_command *cmd, u8 *buf, u32 size)
{
    u8                      replaced_size   = 0;
    u8                      replaced_buf[8] = {0};
    u32                     write_size      = 0;
    u32                     cache_size      = 0;
    struct flash_bdma_param param;

    param.interrupt_en = 0;
    param.path         = (hal->fsp_base == 0x1F002C00) ? FLASH_BDMA_MIU2PMSPI : FLASH_BDMA_MIU2SPI;
    param.size         = size;
    param.src          = buf;
    param.dst          = NULL;

    cache_size = size + ((u32)buf - FLASH_IMPL_CACHE_LINE_ALIGN_DOWN((u32)buf));
    flash_impl_mem_flush((void *)FLASH_IMPL_CACHE_LINE_ALIGN_DOWN((u32)buf),
                         FLASH_IMPL_CACHE_LINE_ALIGN_UP(cache_size));
    flash_impl_miupipe_flush();

    hal_fsp_qspi_reset(hal, HAL_FSP_USE_SINGLE_CMD);

    if (hal->use_sw_cs)
    {
        hal_fsp_qspi_set_access_mode(hal, cmd->cmd);
        hal_fsp_qspi_set_wbf_size(hal, 1, 0);
    }
    else
    {
        replaced_buf[0] = cmd->cmd;

        if (cmd->addr_bytes == 2)
        {
            replaced_buf[1] = (cmd->address >> 8) & 0xff;
            replaced_buf[2] = cmd->address & 0xff;
            replaced_size   = 3;
        }
        else if (cmd->addr_bytes == 3)
        {
            replaced_buf[1] = (cmd->address >> 16) & 0xff;
            replaced_buf[2] = (cmd->address >> 8) & 0xff;
            replaced_buf[3] = cmd->address & 0xff;
            replaced_size   = 4;
        }
        else if (cmd->addr_bytes == 4)
        {
            replaced_buf[1] = (cmd->address >> 24) & 0xff;
            replaced_buf[2] = (cmd->address >> 16) & 0xff;
            replaced_buf[3] = (cmd->address >> 8) & 0xff;
            replaced_buf[4] = cmd->address & 0xff;
            replaced_size   = 5;
        }

        hal_fsp_qspi_set_access_mode(hal, 0);

        if (replaced_size != hal_fsp_qspi_set_wbf_size(hal, 1, replaced_size))
            return 0;

        hal_fsp_qspi_write_wbf(hal, replaced_buf, replaced_size);
    }

    if (hal->interrupt_en)
    {
        param.interrupt_en  = 1;
        param.callback_parm = hal;
        param.callback      = (void *)hal->bdma_callback;

        if (FLASH_BDMA_SUCCESS != flash_impl_bdma_transfer(&param))
            return 0;

        write_size = hal_fsp_qspi_enable_outside_wbf(hal, HAL_FSP_WBF_OUTSIDE_SRC_BDMA, 0, replaced_size, size);

        hal_fsp_qspi_trigger(hal);

        if (!hal->wait_bdma_done || (0 != hal->wait_bdma_done(hal)))
        {
            flash_impl_printf("[FSP] wait bdma done timeout !!!\r\n");
            return 0;
        }
    }
    else
    {
        if (!hal->use_sw_cs)
            return 0;

        write_size = hal_fsp_qspi_enable_outside_wbf(hal, HAL_FSP_WBF_OUTSIDE_SRC_BDMA, 0, replaced_size, size);

        hal_fsp_qspi_trigger(hal);

        if (FLASH_BDMA_SUCCESS != flash_impl_bdma_transfer(&param))
            return 0;
    }

    if (!hal_fsp_qspi_wait_done(hal))
    {
        return 0;
    }

    hal_fsp_qspi_clear_trigger(hal);
    hal_fsp_qspi_disable_outside_wbf(hal);

    return write_size;
}

u32 hal_fsp_qspi_bdma_read(struct fsp_qspi_hal *hal, struct fsp_qspi_command *cmd, u8 *buf, u32 size)
{
    struct flash_bdma_param param;

    hal_fsp_qspi_select_clk_src(hal, cmd->cmd);

    do
    {
        flash_impl_mem_invalidate((void *)buf, FLASH_IMPL_CACHE_LINE_ALIGN_UP(size));

        hal_fsp_qspi_select_read_mode(hal, cmd->cmd, cmd->addr_bytes, cmd->dummy);

        param.interrupt_en  = hal->interrupt_en;
        param.path          = (hal->fsp_base == 0x1F002C00) ? FLASH_BDMA_PMSPI2MIU : FLASH_BDMA_SPI2MIU;
        param.size          = size;
        param.src           = (u8 *)(unsigned long)cmd->address;
        param.dst           = buf;
        param.callback_parm = hal;
        param.callback      = (void *)((hal->interrupt_en) ? hal->bdma_callback : NULL);

        if (FLASH_BDMA_SUCCESS != flash_impl_bdma_transfer(&param))
        {
            size = 0;
            break;
        }

        if (hal->interrupt_en)
        {
            if (!hal->wait_bdma_done || (0 != hal->wait_bdma_done(hal)))
            {
                flash_impl_printf("[QSPI] wait bdma done timeout !!!\r\n");
                size = 0;
                break;
            }
        }

        flash_impl_mem_invalidate((void *)buf, FLASH_IMPL_CACHE_LINE_ALIGN_UP(size));

        hal_fsp_qspi_select_read_mode(hal, 0x0B, cmd->addr_bytes, 8);
    } while (0);

    hal_fsp_qspi_select_clk_src(hal, 0x0B);

    return size;
}

#if defined(CONFIG_FLASH_XZDEC)
u32 hal_fsp_qspi_bdma_read_to_xzdec(struct fsp_qspi_hal *hal, struct fsp_qspi_command *cmd, u8 *buf, u32 size)
{
    struct flash_bdma_param param;

    hal_fsp_qspi_select_clk_src(hal, cmd->cmd);

    do
    {
        flash_impl_mem_invalidate((void *)buf, FLASH_IMPL_CACHE_LINE_ALIGN_UP(size));

        hal_fsp_qspi_select_read_mode(hal, cmd->cmd, cmd->addr_bytes, cmd->dummy);

        param.interrupt_en  = hal->interrupt_en;
        param.path          = (hal->fsp_base == 0x1F002C00) ? FLASH_BDMA_PMSPI2XZDEC : FLASH_BDMA_SPI2XZDEC;
        param.size          = size;
        param.src           = (u8 *)(unsigned long)cmd->address;
        param.dst           = buf;
        param.callback_parm = hal;
        param.callback      = (void *)((hal->interrupt_en) ? hal->bdma_callback : NULL);

        if (FLASH_BDMA_SUCCESS != flash_impl_bdma_transfer(&param))
        {
            size = 0;
            break;
        }

        if (hal->interrupt_en)
        {
            if (!hal->wait_bdma_done || (0 != hal->wait_bdma_done(hal)))
            {
                flash_impl_printf("[QSPI] wait bdma done timeout !!!\r\n");
                size = 0;
                break;
            }
        }

        flash_impl_mem_invalidate((void *)buf, FLASH_IMPL_CACHE_LINE_ALIGN_UP(size));
        hal_fsp_qspi_select_read_mode(hal, 0x0B, cmd->addr_bytes, 8);
    } while (0);

    hal_fsp_qspi_select_clk_src(hal, 0x0B);

    return size;
}
#endif
