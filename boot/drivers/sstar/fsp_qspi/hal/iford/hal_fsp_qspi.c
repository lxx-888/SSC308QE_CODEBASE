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
#if (defined CONFIG_GPIO_SUPPORT) || (defined CONFIG_SSTAR_GPIO)
#include <drv_gpio.h>
#endif

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

/* FSP_REG_SWITCH */
#define HAL_FSP_SWITCH_MODE_EN            (0x1)
#define HAL_FSP_SWITCH_2ND_MODE_EN        (0x1 << 4)
#define HAL_FSP_SWITCH_2ND_MODE_START_CNT (0xF)
#define HAL_FSP_SWITCH_2ND_MODE_SEL       (0x3)

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

#define HAL_FSP_REG_SWITCH_MODE(hal)        ((hal->fsp_base) + ((0x00) << 2))
#define HAL_FSP_REG_SWITCH_MODE_CNT(hal)    ((hal->fsp_base) + ((0x01) << 2))
#define HAL_FSP_REG_SWITCH_MODE_SELECT(hal) ((hal->fsp_base) + ((0x02) << 2))
#define HAL_FSP_REG_HIGH_SPEED_DIV2_EN(hal) ((hal->fsp_base) + ((0x03) << 2))
#define HAL_FSP_REG_WRITE_SIZE(hal)         ((hal->fsp_base) + ((0x6A) << 2))
#define HAL_FSP_REG_READ_SIZE(hal)          ((hal->fsp_base) + ((0x6B) << 2))
#define HAL_FSP_REG_TRIGGER(hal)            ((hal->fsp_base) + ((0x6D) << 2))
#define HAL_FSP_REG_CTRL01(hal)             ((hal->fsp_base) + ((0x6C) << 2))
#define HAL_FSP_REG_DONE(hal)               ((hal->fsp_base) + ((0x6E) << 2))
#define HAL_FSP_REG_CLEAR_DONE(hal)         ((hal->fsp_base) + ((0x6F) << 2))
#define HAL_FSP_REG_CTRL23(hal)             ((hal->fsp_base) + ((0x75) << 2))
#define HAL_FSP_REG_CTRL4(hal)              ((hal->fsp_base) + ((0x76) << 2))
#define HAL_FSP_REG_WBF_SIZE_OUTSIDE(hal)   ((hal->fsp_base) + ((0x78) << 2))
#define HAL_FSP_REG_WBF_OUTSIDE(hal)        ((hal->fsp_base) + ((0x79) << 2))

#define HAL_QSPI_REG_SW_MODE(hal)                 ((hal->qspi_base) + ((0x0A) << 2))
#define HAL_QSPI_REG_FORCE_INPUT_IN_INTERVAL(hal) ((hal->qspi_base) + ((0x40) << 2))
#define HAL_QSPI_REG_DEF_DTR_1_4_4_L(hal)         ((hal->qspi_base) + ((0x42) << 2))
#define HAL_QSPI_REG_SPI_DTR_HIGHSPEED_MODE(hal)  ((hal->qspi_base) + ((0x47) << 2))
#define HAL_QSPI_REG_TIMEOUT_CNT_VALUE(hal)       ((hal->qspi_base) + ((0x66) << 2))
#define HAL_QSPI_REG_TIMEOUT_CNT_CTRL(hal)        ((hal->qspi_base) + ((0x67) << 2))
#define HAL_QSPI_REG_CFG_CKG_SPI(hal)             ((hal->qspi_base) + ((0x70) << 2))
#define HAL_QSPI_REG_MODE_SEL(hal)                ((hal->qspi_base) + ((0x72) << 2))
#define HAL_QSPI_REG_DEBUG_BUS_1(hal)             ((hal->qspi_base) + ((0x77) << 2))
#define HAL_QSPI_REG_CHIP_SELECT(hal)             ((hal->qspi_base) + ((0x7A) << 2))
#define HAL_QSPI_REG_FUNC_SETTING_DEF0(hal)       ((hal->qspi_base) + ((0x7D) << 2))

#define HAL_PIU_NONPM_BASE       (0x1F200E00)
#define HAL_SW_SPI_MASK_ACK_EN   (HAL_PIU_NONPM_BASE + (0x1a << 2))
#define HAL_SPI_BURST_CTRL_DEBUG (HAL_PIU_NONPM_BASE + (0x34 << 2))

#define HAL_ISP_MCG_BASE (0x1F201800)
#define HAL_ISP_MCG_CTRL (HAL_ISP_MCG_BASE + (0x48 << 2))

#define HAL_OTP_BASE                       (0x1F203E00)
#define HAL_OTP_CA53_BOOT_FROM_PM_SPI      (HAL_OTP_BASE + (0x30 << 2))
#define HAL_OTP_CA53_BOOT_FROM_PM_SPI_MASK (0x0003)

#define HAL_CLKGEN_BASE        (0x1F207000)
#define HAL_CLKGEN_REG_CKG_SPI (HAL_CLKGEN_BASE + (0x32 << 2))

#define HAL_SPI_CTRL_BASE        (0x1F202A00)
#define HAL_SPI_CTRL_REG_CKG_SPI (HAL_SPI_CTRL_BASE + (0x2A << 2))
#define HAL_QSPI_MCG_CTRL        (HAL_SPI_CTRL_BASE + (0x2c << 2))

#define HAL_PAD_TOP                  (0x1F207800)
#define HAL_PAD_TOP_REG_QSPICZ2_MODE (HAL_PAD_TOP + (0x7D << 2))

#define HAL_EMMCPLL_BASE      (0x1F207200)
#define HAL_EMMCPLL_REG_CLKPH (HAL_EMMCPLL_BASE + (0x31 << 2))
#define HAL_EMMCPLL_REG_TEST  (HAL_EMMCPLL_BASE + (0x39 << 2))

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

static void spipll_set(u32 synthesizer, u8 input_div_first, u8 loop_div_first, u8 loop_div_second, u8 pdiv)
{
    OUTREG16(GET_REG8_ADDR(RIU_BASE_ADDR, 0x00103900), synthesizer & 0xFFFF);
    OUTREG16(GET_REG8_ADDR(RIU_BASE_ADDR, 0x00103902), (synthesizer >> 16) & 0xFF);
    OUTREG16(GET_REG8_ADDR(RIU_BASE_ADDR, 0x0010396c), 0x0000);
    OUTREG16(GET_REG8_ADDR(RIU_BASE_ADDR, 0x00103964), 0x0001);
    OUTREG16(GET_REG8_ADDR(RIU_BASE_ADDR, 0x00103966), input_div_first & 0x3);
    OUTREG16(GET_REG8_ADDR(RIU_BASE_ADDR, 0x103968), loop_div_first & 0x3);
    OUTREG16(GET_REG8_ADDR(RIU_BASE_ADDR, 0x10396a), loop_div_second & 0xFF);
    OUTREG16(GET_REG8_ADDR(RIU_BASE_ADDR, 0x10396e), pdiv);
    OUTREG16(GET_REG8_ADDR(RIU_BASE_ADDR, 0x103962), 0x0001);
}

static void spipll_104m_init(void)
{
    spipll_set(0x427627, 0, 0x2, 0x0, 0x0);
}

static void spipll_133m_init(void)
{
    spipll_set(0x19fc27, 0, 0x0, 0x4, 0x2);
}

static void spipll_160m_init(void)
{
    spipll_set(0x159999, 0, 0, 0x2, 0x0);
}

static void spipll_172m_init(void)
{
    spipll_set(0x1417D0, 0, 0, 0x2, 0x0);
}

static void spipll_208m_init(void)
{
    spipll_set(0x213b13, 0, 0, 0x4, 0x0);
}

static void spipll_216m_init(void)
{
    spipll_set(0x200000, 0, 0, 0x4, 0x0);
}

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

    if (!cs_timeout_en)
    {
        hal_fsp_qspi_set_timeout(hal, 1, cs_timeout_value);
    }

    while (HAL_QSPI_REG_WAIT_SELECT_MODE & HAL_READ_WORD(HAL_QSPI_REG_DEBUG_BUS_1(hal)))
        ;

    if (!cs_timeout_en)
    {
        hal_fsp_qspi_set_timeout(hal, cs_timeout_en, cs_timeout_value);
    }
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

static u8 hal_fsp_qspi_cmd_is_switch_mode(u8 cmd)
{
    switch (cmd)
    {
        case 0x32:
        case 0x34:
            return 2;
        default:
            return 0;
    }
}

static void hal_fsp_qspi_set_switch_mode(struct fsp_qspi_hal *hal, struct fsp_qspi_command *cmd, u8 enable)
{
    if (!hal->use_sw_cs && hal_fsp_qspi_cmd_to_access_mode(cmd->cmd) && enable)
    {
        HAL_WRITE_WORD_MASK(HAL_FSP_REG_SWITCH_MODE(hal), 0x1, HAL_FSP_SWITCH_MODE_EN);
        HAL_WRITE_WORD_MASK(HAL_FSP_REG_SWITCH_MODE(hal), (0x1 << 4), HAL_FSP_SWITCH_2ND_MODE_EN);
        HAL_WRITE_WORD_MASK(HAL_FSP_REG_SWITCH_MODE_CNT(hal), cmd->addr_bytes + 0x1, HAL_FSP_SWITCH_2ND_MODE_START_CNT);
        HAL_WRITE_WORD_MASK(HAL_FSP_REG_SWITCH_MODE_SELECT(hal), hal_fsp_qspi_cmd_is_switch_mode(cmd->cmd),
                            HAL_FSP_SWITCH_2ND_MODE_SEL);
    }
    else if (!hal->use_sw_cs && !enable)
    {
        HAL_WRITE_WORD_MASK(HAL_FSP_REG_SWITCH_MODE(hal), 0x0, HAL_FSP_SWITCH_MODE_EN);
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
    u8                        clk_src = 0;
    u8                        rate    = 0;
    struct fsp_qspi_ctrl_hal *ctrl    = &hal->ctrl[hal->cs_select];

    rate = hal_fsp_qspi_is_dtr_cmd(cmd) ? ctrl->rate_dtr : ctrl->rate_str;

    if (rate >= 133)
    {
        clk_src = 1;
        if (rate != 133)
        {
            HAL_WRITE_BYTE_MASK(HAL_QSPI_REG_SPI_DTR_HIGHSPEED_MODE(hal), 0x1, 0x1);
            HAL_WRITE_BYTE_MASK(HAL_FSP_REG_HIGH_SPEED_DIV2_EN(hal), 0x1, 0x1);
        }
    }
    else
    {
        clk_src = 0;
        HAL_WRITE_BYTE_MASK(HAL_QSPI_REG_SPI_DTR_HIGHSPEED_MODE(hal), 0x0, 0x1);
        HAL_WRITE_BYTE_MASK(HAL_FSP_REG_HIGH_SPEED_DIV2_EN(hal), 0x0, 0x1);
    }

    if (hal->set_rate)
    {
        hal->set_rate(hal, rate, clk_src);
        return;
    }

    if (hal->fsp_base == 0x1F201A00)
    {
        if (rate >= 208)
        {
            HAL_WRITE_BYTE(HAL_SPI_CTRL_REG_CKG_SPI, 0x04);
            HAL_WRITE_BYTE_MASK(HAL_SPI_CTRL_REG_CKG_SPI, (0x1 << 4), (0x1 << 4));
        }
        else if (rate >= 172)
        {
            HAL_WRITE_BYTE(HAL_SPI_CTRL_REG_CKG_SPI, 0x04);
            HAL_WRITE_BYTE_MASK(HAL_SPI_CTRL_REG_CKG_SPI, (0x1 << 4), (0x1 << 4));
        }
        else if (rate >= 160)
        {
            HAL_WRITE_BYTE(HAL_SPI_CTRL_REG_CKG_SPI, 0x04);
            HAL_WRITE_BYTE_MASK(HAL_SPI_CTRL_REG_CKG_SPI, (0x1 << 4), (0x1 << 4));
        }
        else if (rate >= 133)
        {
            HAL_WRITE_BYTE(HAL_SPI_CTRL_REG_CKG_SPI, 0x04);
            HAL_WRITE_BYTE_MASK(HAL_SPI_CTRL_REG_CKG_SPI, (0x1 << 4), (0x1 << 4));
            HAL_WRITE_BYTE_MASK(HAL_QSPI_REG_SPI_DTR_HIGHSPEED_MODE(hal), 0, 0x1);
        }
        else if (rate >= 108)
        {
            HAL_WRITE_BYTE(HAL_CLKGEN_REG_CKG_SPI, 0x08);
            HAL_WRITE_BYTE(HAL_SPI_CTRL_REG_CKG_SPI, 0x00);
            HAL_WRITE_BYTE_MASK(HAL_SPI_CTRL_REG_CKG_SPI, (0x1 << 4), (0x1 << 4));
        }
        else if (rate >= 104)
        {
            HAL_WRITE_BYTE(HAL_SPI_CTRL_REG_CKG_SPI, 0x04);
            HAL_WRITE_BYTE_MASK(HAL_SPI_CTRL_REG_CKG_SPI, (0x1 << 4), (0x1 << 4));
        }
        else if (rate >= 86)
        {
            HAL_WRITE_BYTE(HAL_CLKGEN_REG_CKG_SPI, 0x04);
            HAL_WRITE_BYTE(HAL_SPI_CTRL_REG_CKG_SPI, 0x00);
            HAL_WRITE_BYTE_MASK(HAL_SPI_CTRL_REG_CKG_SPI, (0x1 << 4), (0x1 << 4));
        }
        else if (rate >= 54)
        {
            HAL_WRITE_BYTE(HAL_CLKGEN_REG_CKG_SPI, 0x0C);
            HAL_WRITE_BYTE(HAL_SPI_CTRL_REG_CKG_SPI, 0x00);
            HAL_WRITE_BYTE_MASK(HAL_SPI_CTRL_REG_CKG_SPI, (0x1 << 4), (0x1 << 4));
        }
    }
}

u8 hal_fsp_qspi_is_boot_storage(struct fsp_qspi_hal *hal)
{
    return (hal->fsp_base == 0x1F201A00) ? 1 : 0;
}

void hal_fsp_qspi_init(struct fsp_qspi_hal *hal)
{
    FLASH_IMPL_UNUSED_VAR(hal);
    /* make spi pin output mode */
    HAL_WRITE_WORD_MASK(HAL_QSPI_REG_FORCE_INPUT_IN_INTERVAL(hal), 0x0000, 0x0010);

    /*  Fix flash address must 16 align */
    HAL_WRITE_WORD_MASK(HAL_SW_SPI_MASK_ACK_EN, 0x1, 0x1);
    HAL_WRITE_WORD_MASK(HAL_SPI_BURST_CTRL_DEBUG, 0x1, 0x1);

    HAL_WRITE_WORD_MASK(HAL_QSPI_MCG_CTRL, 0x001, 0x101);
    HAL_WRITE_WORD_MASK(HAL_ISP_MCG_CTRL, 0x0, 0x01);

    HAL_WRITE_BYTE_MASK(HAL_QSPI_REG_SPI_DTR_HIGHSPEED_MODE(hal), 0x0, 0x1);
    HAL_WRITE_BYTE_MASK(HAL_FSP_REG_HIGH_SPEED_DIV2_EN(hal), 0x0, 0x1);
}

void hal_fsp_qspi_set_rate(struct fsp_qspi_hal *hal, u8 cs_select, u8 rate, u8 cmd)
{
    struct fsp_qspi_ctrl_hal *ctrl = NULL;

    if (cs_select > (hal->cs_num + hal->cs_ext_num))
    {
        return;
    }

    ctrl             = &hal->ctrl[cs_select];
    hal->cs_select   = cs_select;
    ctrl->setup_rate = rate;
    ctrl->setup_cmd  = cmd;

    if (!ctrl->rate_dtr && hal_fsp_qspi_is_dtr_cmd(cmd))
    {
        if (rate >= 108)
        {
            spipll_216m_init();
            ctrl->need_autok = 1;
            flash_impl_printf("SPI 108M\r\n");
            ctrl->rate_dtr = 216;
            ctrl->rate_str = 216;
        }
        else if (rate >= 104)
        {
            spipll_208m_init();
            ctrl->need_autok = 1;
            flash_impl_printf("SPI 104M\r\n");
            ctrl->rate_dtr = 208;
            ctrl->rate_str = 208;
        }
        else if (rate >= 86)
        {
            spipll_172m_init();
            ctrl->need_autok = 1;
            flash_impl_printf("SPI 86M\r\n");
            ctrl->rate_dtr = 172;
            ctrl->rate_str = 172;
        }
        else if (rate >= 80)
        {
            spipll_160m_init();
            ctrl->need_autok = 1;
            flash_impl_printf("SPI 80M\r\n");
            ctrl->rate_dtr = 160;
            ctrl->rate_str = 160;
        }
        else if (rate >= 54)
        {
            flash_impl_printf("SPI 54M\r\n");
            ctrl->rate_dtr = 108;
            ctrl->rate_str = 108;
        }
        else
        {
            flash_impl_printf("SPI 54M\r\n");
            ctrl->rate_dtr = 108;
            ctrl->rate_str = 108;
        }
    }
    else if (!ctrl->rate_str)
    {
        if (rate >= 133)
        {
            spipll_133m_init();
            flash_impl_printf("SPI 133M\r\n");
            ctrl->rate_str = 133;
        }
        else if (rate >= 108)
        {
            flash_impl_printf("SPI 108M\r\n");
            ctrl->rate_str = 108;
        }
        else if (rate >= 104)
        {
            spipll_104m_init();
            flash_impl_printf("SPI 104M\r\n");
            ctrl->rate_str = 104;
        }
        else if (rate >= 86)
        {
            flash_impl_printf("SPI 86M\r\n");
            ctrl->rate_str = 86;
        }
        else if (rate >= 54)
        {
            flash_impl_printf("SPI 54M\r\n");
            ctrl->rate_str = 54;
        }
        else
        {
            flash_impl_printf("SPI 54M\r\n");
            ctrl->rate_str = 54;
        }
    }
    else
        flash_impl_printf("spi clk already initialized\r\n");

    hal_fsp_qspi_select_clk_src(hal, cmd);
}

void hal_fsp_qspi_set_phase(struct fsp_qspi_hal *hal, u8 phase)
{
    u8                        polority = 0;
    u8                        cycle    = 0;
    struct fsp_qspi_ctrl_hal *ctrl     = &hal->ctrl[hal->cs_select];

    ctrl->have_phase = 1;
    ctrl->phase      = phase;

    while (phase >= 17)
    {
        cycle++;
        phase -= 17;
    }

    while (phase >= 9)
    {
        polority++;
        phase -= 9;
    }

    HAL_WRITE_WORD_MASK(HAL_EMMCPLL_REG_CLKPH, phase, 0xF);
    HAL_WRITE_WORD_MASK(HAL_EMMCPLL_REG_TEST, (polority << 8), (0x1 << 8));
    HAL_WRITE_BYTE_MASK(HAL_QSPI_REG_SPI_DTR_HIGHSPEED_MODE(hal), (polority << 7), (0x1 << 7));
    HAL_WRITE_BYTE_MASK(HAL_QSPI_REG_SPI_DTR_HIGHSPEED_MODE(hal), (cycle << 4), (0x3 << 4));
}

u8 hal_fsp_qspi_try_phase(struct fsp_qspi_hal *hal, u8 (*parrtern_check)(void))
{
    u8 phase     = 0;
    u8 phase_min = 0x80;
    u8 phase_max = 0;

    do
    {
        hal_fsp_qspi_set_phase(hal, phase);

        if (parrtern_check())
        {
            if (phase_min & 0x80)
            {
                phase_min = phase;
                phase_max = phase_min;
            }
        }
        else
        {
            if (!(phase_min & 0x80))
            {
                phase_max = phase - 1;
                break;
            }
        }

        phase++;

        if (phase > 0x44)
        {
            break;
        }
    } while (1);

    if (!(phase_min & 0x80))
    {
        flash_impl_printf_hex("phase_min = 0x", phase_min, "\r\n");
        flash_impl_printf_hex("phase_max = 0x", phase_max, "\r\n");

        phase = phase_min + phase_max;
        phase = phase >> 1;

        hal_fsp_qspi_set_phase(hal, phase);
    }

    return phase;
}

void hal_fsp_qspi_set_timeout(struct fsp_qspi_hal *hal, u8 enable, u32 val)
{
    if (enable)
    {
        HAL_WRITE_WORD(HAL_QSPI_REG_TIMEOUT_CNT_VALUE(hal), (val & 0xFFFF));
        HAL_WRITE_WORD(HAL_QSPI_REG_TIMEOUT_CNT_CTRL(hal), ((val >> 16) & 0xFF) | HAL_QSPI_REG_SPI_TIMEOUT_EN);
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

void hal_fsp_qspi_cz2_enable(void)
{
    HAL_WRITE_BYTE_MASK(HAL_PAD_TOP_REG_QSPICZ2_MODE, 0x1, 0x1);
}

void hal_fsp_qspi_pull_cs(struct fsp_qspi_hal *hal, u8 pull_high)
{
    u8 cs_ctrl;

    cs_ctrl = HAL_READ_BYTE(HAL_QSPI_REG_SW_MODE(hal));

    if (hal->cs_select < hal->cs_num)
    {
        if (1 == pull_high)
        {
            HAL_WRITE_BYTE(HAL_QSPI_REG_SW_MODE(hal), HAL_QSPI_REG_SPI_SW_CS_PULL_HIGH | cs_ctrl);
        }
        else
        {
            HAL_WRITE_BYTE(HAL_QSPI_REG_SW_MODE(hal), ~HAL_QSPI_REG_SPI_SW_CS_PULL_HIGH & cs_ctrl);
        }
    }
    else
    {
        if (hal->cs_ext_num)
        {
            if (1 == pull_high)
            {
#if (defined CONFIG_GPIO_SUPPORT) || (defined CONFIG_SSTAR_GPIO)
                sstar_gpio_set_high(hal->cs_ext[hal->cs_select - hal->cs_num]);
#endif
                HAL_WRITE_BYTE(HAL_QSPI_REG_SW_MODE(hal), HAL_QSPI_REG_SPI_SW_CS_PULL_HIGH | cs_ctrl);
            }
            else
            {
#if (defined CONFIG_GPIO_SUPPORT) || (defined CONFIG_SSTAR_GPIO)
                sstar_gpio_set_low(hal->cs_ext[hal->cs_select - hal->cs_num]);
#endif
                HAL_WRITE_BYTE(HAL_QSPI_REG_SW_MODE(hal), HAL_QSPI_REG_SPI_SW_CS_PULL_HIGH | cs_ctrl);
            }
        }
    }
}

void hal_fsp_qspi_chip_select(struct fsp_qspi_hal *hal, u8 cs_select, u8 cmd)
{
    u8                        cur_cs = 0;
    struct fsp_qspi_ctrl_hal *ctrl   = NULL;

    if (cs_select < (hal->cs_num + hal->cs_ext_num))
    {
        hal->cs_select = cs_select;
        ctrl           = &hal->ctrl[cs_select];

        if (!hal->cs_ext_num)
        {
            cur_cs = HAL_READ_WORD(HAL_QSPI_REG_CHIP_SELECT(hal));
            if (cur_cs != cs_select)
            {
                HAL_WRITE_WORD_MASK(HAL_QSPI_REG_CHIP_SELECT(hal), cs_select, HAL_QSPI_CHIP_SELECT_MASK);
            }
        }

        hal_fsp_qspi_select_clk_src(hal, cmd);

        if (ctrl->have_phase)
            hal_fsp_qspi_set_phase(hal, ctrl->phase);
    }
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
    struct flash_bdma_param param;

    param.interrupt_en = 0;
    param.path         = (hal->fsp_base == 0x1F002C00) ? FLASH_BDMA_MIU2PMSPI : FLASH_BDMA_MIU2SPI;
    param.size         = size;
    param.src          = buf;
    param.dst          = NULL;

    flash_impl_mem_flush((void *)buf, size);
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
        hal_fsp_qspi_set_switch_mode(hal, cmd, 1);

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
        write_size = hal_fsp_qspi_enable_outside_wbf(hal, HAL_FSP_WBF_OUTSIDE_SRC_BDMA, 0, replaced_size, size);

        param.callback_parm = NULL;
        param.callback      = NULL;

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
    hal_fsp_qspi_set_switch_mode(hal, cmd, 0);

    return write_size;
}

u32 hal_fsp_qspi_bdma_read(struct fsp_qspi_hal *hal, struct fsp_qspi_command *cmd, u8 *buf, u32 size)
{
    struct flash_bdma_param param;

    do
    {
        flash_impl_mem_invalidate((void *)buf, size);

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

        flash_impl_mem_invalidate((void *)buf, size);

        hal_fsp_qspi_select_read_mode(hal, 0x0B, cmd->addr_bytes, 8);
    } while (0);

    return size;
}

#if defined(CONFIG_FLASH_XZDEC)
u32 hal_fsp_qspi_bdma_read_to_xzdec(struct fsp_qspi_hal *hal, struct fsp_qspi_command *cmd, u8 *buf, u32 size)
{
    struct flash_bdma_param param;

    do
    {
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

        hal_fsp_qspi_select_read_mode(hal, 0x0B, cmd->addr_bytes, 8);
    } while (0);

    return size;
}
#endif
