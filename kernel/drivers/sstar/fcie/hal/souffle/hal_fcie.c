/*
 * hal_fcie.c- Sigmastar
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
#include <hal_fcie.h>

#define ECC_BCH_512B_8_BITS  0x00
#define ECC_BCH_512B_16_BITS 0x02
#define ECC_BCH_512B_24_BITS 0x04
#define ECC_BCH_512B_32_BITS 0x06
#define ECC_BCH_512B_40_BITS 0x08

#define ECC_BCH_1024B_8_BITS  0x01
#define ECC_BCH_1024B_16_BITS 0x03
#define ECC_BCH_1024B_24_BITS 0x05
#define ECC_BCH_1024B_32_BITS 0x07
#define ECC_BCH_1024B_40_BITS 0x09

/* HAL_FCIE0_NC_BOOT_MODE */
#define BIT_IMI_SEL (1 << 2)
/* HAL_FCIE0_NC_FCIE_RST */
#define BIT_FCIE_SOFT_RST (1 << 0)
#define BIT_RST_MIU_STS   (1 << 1)
#define BIT_RST_MIE_STS   (1 << 2)
#define BIT_RST_MCU_STS   (1 << 3)
#define BIT_RST_ECC_STS   (1 << 4)
#define BIT_RST_STS_MASK  (BIT_RST_MIE_STS | BIT_RST_MCU_STS | BIT_RST_ECC_STS)
/* HAL_FCIE0_NC_AUXREG_ADR */
// AUX Reg Address
#define AUXADR_CMDREG8  0x08
#define AUXADR_CMDREG9  0x09
#define AUXADR_CMDREGA  0x0A
#define AUXADR_ADRSET   0x0B
#define AUXADR_RPTCNT   0x18 // Pepeat Count
#define AUXADR_RAN_CNT  0x19
#define AUXADR_RAN_POS  0x1A // offset
#define AUXADR_ST_CHECK 0x1B
#define AUXADR_IDLE_CNT 0x1C
#define AUXADR_INSTQUE  0x20
/* HAL_FCIE0_NC_AUXREG_DAT */
// OP Code: Action
#define ACT_WAITRB    0x80
#define ACT_CHKSTATUS 0x81
#define ACT_WAIT_IDLE 0x82
#define ACT_WAIT_MMA  0x83
#define ACT_BREAK     0x88
#define ACT_SER_DOUT  0x90 /* for column addr == 0 */
#define ACT_RAN_DOUT  0x91 /* for column addr != 0 */
//#define ACT_WR_REDU       0x92
//#define ACT_LUT_DWLOAD    0x93
//#define ACT_LUT_DWLOAD1   0x94
#define ACT_SER_DIN 0x98 /* for column addr == 0 */
#define ACT_RAN_DIN 0x99 /* for column addr != 0 */
//#define ACT_RD_REDU       0x9A
//#define ACT_LUT_UPLOAD    0x9B
#define ACT_PAGECOPY_US 0xA0
#define ACT_PAGECOPY_DS 0xA1
#define ACT_REPEAT      0xB0
/* HAL_FCIE0_NC_CTRL */
#define BIT_NC_JOB_START       (1 << 0)
#define BIT_NC_ADMA_EN         (1 << 1)
#define BIT_NC_ZDEC_EN         (1 << 3)
#define BIT_NC_NF2ZDEC_PTR_CLR (1 << 4)
/* HAL_FCIE0_NC_PART_MODE */
#define BIT_PARTIAL_MODE_EN    (1 << 0)
#define BITS_SECTOR_COUNT_MASK ((1 << 7) - 2)
/* HAL_FCIE0_NC_SPARE */
#define BIT_SPARE_DEST       (1 << 8)
#define BIT_SPARE_ECC_BYPASS (1 << 10)
/* HAL_FCIE0_NC_ECC_CTRL */
#define BIT_BYPASS_ECC(x)      (x << 10)
#define BIT_ERROR_NONE_STOP(x) (x << 7)
/* HAL_FCIE0_NC_ECC_STAT0 */
#define BIT_ECC_FLAG     (1 << 0)
#define BITS_ECC_CORRECT ((1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6))
/* HAL_FCIE0_NC_ECC_STAT2 */
#define BITS_ECC_MASK         ((1 << 0) | (1 << 1))
#define BIT_ECC_UNCORRECTABLE (1 << 0)
/* HAL_FCIE0_NC_DDR_CTRL */
#define BIT_NC_32B_MODE (1 << 2)
/* HAL_FCIE0_NC_MIE_EVENT */
#define BIT_NC_JOB_END      (1 << 0)
#define BIT_NC_R2N_ECCCOR   (1 << 1)
#define BIT_NC_SECURE_ALERT (1 << 2)
#define BIT_NC_JOB_ABORT    (1 << 3)
/* HAL_FCIE0_NC_FORCE_INT */
#define BIT_F_NC_JOB_INT        (1 << 0)
#define BIT_F_NC_R2N_ECCCOR_INT (1 << 1)
#define BIT_NC_SECURE_ALERT_INT (1 << 2)
#define BIT_NC_JOB_ABORT_INT    (1 << 3)
/* HAL_FCIE0_NC_FUN_CTL */
#define BIT_NC_R2N_MODE (1 << 1)
#define BIT_NC_SPI_MODE (1 << 4)
/* HAL_FCIE0_NC_ECC_MODE_SEL */
#define BIT_DATA_MODE  (1 << 0)
#define BIT_AXI_LENGTH ((1 << 4) | (1 << 5))
/* HAL_FCIE0_NC_ECC_SPARE_SIZE */
#define BIT_SPARE_SIZE     0xFF
#define BIT_ADDR_LATCH     (1 << 8)
#define BIT_ECC_DIR        (1 << 9)
#define BIT_WRITE_MIU_BUSY (1 << 10)
#define FCIE_SPI_MODE      (1 << 4)
#define FCIR_DIR_WRITE     (1 << 9)
#define FCIR_DIR_READ      (0 << 9)

#define FCIE_WAIT_WRITE_TIME (1 * 1000 * 1000) // us
#define FCIE_WAIT_READ_TIME  (100 * 1000)      // us

#define HAL_FCIE0_NC_BOOT_MODE(hal)           ((hal->fcie0_base) + ((0x37) << 2))
#define HAL_FCIE0_NC_FCIE_RST(hal)            ((hal->fcie0_base) + ((0x3F) << 2))
#define HAL_FCIE0_NC_AUXREG_ADR(hal)          ((hal->fcie0_base) + ((0x43) << 2))
#define HAL_FCIE0_NC_AUXREG_DAT(hal)          ((hal->fcie0_base) + ((0x44) << 2))
#define HAL_FCIE0_NC_CTRL(hal)                ((hal->fcie0_base) + ((0x45) << 2))
#define HAL_FCIE0_NC_PART_MODE(hal)           ((hal->fcie0_base) + ((0x47) << 2))
#define HAL_FCIE0_NC_SPARE(hal)               ((hal->fcie0_base) + ((0x48) << 2))
#define HAL_FCIE0_NC_SPARE_SIZE(hal)          ((hal->fcie0_base) + ((0x49) << 2))
#define HAL_FCIE0_NC_ECC_CTRL(hal)            ((hal->fcie0_base) + ((0x50) << 2))
#define HAL_FCIE0_NC_ECC_STAT0(hal)           ((hal->fcie0_base) + ((0x51) << 2))
#define HAL_FCIE0_NC_ECC_STAT2(hal)           ((hal->fcie0_base) + ((0x53) << 2))
#define HAL_FCIE0_NC_DDR_CTRL(hal)            ((hal->fcie0_base) + ((0x58) << 2))
#define HAL_FCIE0_NC_MIE_EVENT(hal)           ((hal->fcie0_base) + ((0x60) << 2))
#define HAL_FCIE0_NC_MIE_INT_EN(hal)          ((hal->fcie0_base) + ((0x61) << 2))
#define HAL_FCIE0_NC_FUN_CTL(hal)             ((hal->fcie0_base) + ((0x63) << 2))
#define HAL_FCIE0_NC_RDATA_DMA_ADR0(hal)      ((hal->fcie0_base) + ((0x64) << 2))
#define HAL_FCIE0_NC_RDATA_DMA_ADR1(hal)      ((hal->fcie0_base) + ((0x65) << 2))
#define HAL_FCIE0_NC_WDATA_DMA_ADR0(hal)      ((hal->fcie0_base) + ((0x66) << 2))
#define HAL_FCIE0_NC_WDATA_DMA_ADR1(hal)      ((hal->fcie0_base) + ((0x67) << 2))
#define HAL_FCIE0_NC_RSPARE_DMA_ADR0(hal)     ((hal->fcie0_base) + ((0x68) << 2))
#define HAL_FCIE0_NC_RSPARE_DMA_ADR1(hal)     ((hal->fcie0_base) + ((0x69) << 2))
#define HAL_FCIE0_NC_WSPARE_DMA_ADR0(hal)     ((hal->fcie0_base) + ((0x6A) << 2))
#define HAL_FCIE0_NC_WSPARE_DMA_ADR1(hal)     ((hal->fcie0_base) + ((0x6B) << 2))
#define HAL_FCIE0_NC_DATA_BASE_ADDR_MSB(hal)  ((hal->fcie0_base) + ((0x76) << 2))
#define HAL_FCIE0_NC_SPARE_BASE_ADDR_MSB(hal) ((hal->fcie0_base) + ((0x77) << 2))
#define HAL_FCIE0_NC_ECC_DATA_ADDR_L(hal)     ((hal->fcie0_base) + ((0x78) << 2))
#define HAL_FCIE0_NC_ECC_DATA_ADDR_H(hal)     ((hal->fcie0_base) + ((0x79) << 2))
#define HAL_FCIE0_NC_ECC_SPARE_ADDR_L(hal)    ((hal->fcie0_base) + ((0x7A) << 2))
#define HAL_FCIE0_NC_ECC_SPARE_ADDR_H(hal)    ((hal->fcie0_base) + ((0x7B) << 2))
#define HAL_FCIE0_NC_ECC_MODE_SEL(hal)        ((hal->fcie0_base) + ((0x7C) << 2))
#define HAL_FCIE0_NC_ECC_SPARE_SIZE(hal)      ((hal->fcie0_base) + ((0x7D) << 2))

#define HAL_FCIE3_NC_BOOT_MODE(hal) ((hal->fcie3_base) + ((0x30) << 2))

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

#define HAL_OTP_BASE                 (0x1F203E00)
#define HAL_OTP_SPINAND_SOC_ECC      (HAL_OTP_BASE + (0x3E << 2))
#define HAL_OTP_SPINAND_SOC_ECC_MASK (0x0003)

#define HAL_FCIE_ECC_CONFIG_MAX_CNT (6)

static const struct hal_fcie_ecc_config fcie_ecc_config_list[] = {
    {2048, 64, 512, 8, 4, 16},  {2048, 128, 512, 8, 4, 32}, {2048, 128, 512, 16, 4, 32},
    {4096, 128, 512, 8, 8, 16}, {4096, 256, 512, 8, 8, 32}, {4096, 256, 512, 16, 8, 32}};

static u8 hal_fcie_ecc_get_page_mode(u16 page_size)
{
    u8 page_mode;

    switch (page_size)
    {
        case 512:
            page_mode = 0;
            break;
        case 2048:
            page_mode = 0x01;
            break;
        case 4096:
            page_mode = 0x02;
            break;
        case 8192:
            page_mode = 0x03;
            break;
        case (16 << 10):
            page_mode = 0x04;
            break;
        case (32 << 10):
            page_mode = 0x05;
            break;
        default:
            page_mode = 0;
    }

    return page_mode;
}

static u8 hal_fcie_ecc_get_mode(u16 sector_size, u8 max_correct_bits)
{
    u8 ecc_mode = ECC_BCH_512B_8_BITS;

    if (512 == sector_size)
    {
        switch (max_correct_bits)
        {
            case 8:
                ecc_mode = ECC_BCH_512B_8_BITS;
                break;
            case 16:
                ecc_mode = ECC_BCH_512B_16_BITS;
                break;
            case 24:
                ecc_mode = ECC_BCH_512B_24_BITS;
                break;
            case 32:
                ecc_mode = ECC_BCH_512B_32_BITS;
                break;
            case 40:
                ecc_mode = ECC_BCH_512B_40_BITS;
                break;
        }
    }
    else if (1024 == sector_size)
    {
        switch (max_correct_bits)
        {
            case 8:
                ecc_mode = ECC_BCH_1024B_8_BITS;
                break;
            case 16:
                ecc_mode = ECC_BCH_1024B_16_BITS;
                break;
            case 24:
                ecc_mode = ECC_BCH_1024B_24_BITS;
                break;
            case 32:
                ecc_mode = ECC_BCH_1024B_32_BITS;
                break;
            case 40:
                ecc_mode = ECC_BCH_1024B_40_BITS;
                break;
        }
    }

    return ecc_mode;
}

static u16 hal_fcie_ecc_get_data_mode(u16 sector_size)
{
    u16 data_mode;

    switch (sector_size)
    {
        case 512:
            data_mode = 0x00;
            break;
        case 1024:
            data_mode = 0x01;
            break;
        default:
            data_mode = 0x00;
    }

    return data_mode;
}

static u8 hal_fcie_job_is_done(struct hal_fcie *hal, u32 u32_timeout)
{
    u32 u32_count;

    for (u32_count = 0; u32_count < u32_timeout; u32_count++)
    {
        if ((HAL_READ_WORD(HAL_FCIE0_NC_MIE_EVENT(hal)) & BIT_NC_JOB_END)
            && !(HAL_READ_WORD(HAL_FCIE0_NC_ECC_SPARE_SIZE(hal)) & BIT_WRITE_MIU_BUSY))
        {
            return 1;
        }

        FLASH_IMPL_USDELAY(1);
    }

    flash_impl_printf("[FCIE] wait done timeout !!!\r\n");
    return 0;
}

u8 hal_fcie_is_soc_ecc(void)
{
    u16 value = 0;

    value = HAL_READ_WORD(HAL_OTP_SPINAND_SOC_ECC);
    value &= HAL_OTP_SPINAND_SOC_ECC_MASK;

    if (value == 0x00 || value == 0x03)
        return 1;

    return 0;
}

u8 hal_fcie_ecc_get_config(u8 mode, const struct hal_fcie_ecc_config **config)
{
    if (mode < HAL_FCIE_ECC_CONFIG_MAX_CNT)
    {
        *config = &fcie_ecc_config_list[mode];

        return HAL_FCIE_SUCCESS;
    }

    return HAL_FCIE_INVALID;
}

u8 hal_fcie_ecc_reset(struct hal_fcie *hal)
{
    u16 u16_cnt;

    // soft reset
    HAL_WRITE_WORD(HAL_FCIE0_NC_FCIE_RST(hal), ~BIT_FCIE_SOFT_RST); /* active low */

    // As reset is active, Check Reset Status from 0 -> 1
    u16_cnt = 0;
    do
    {
        FLASH_IMPL_USDELAY(1);

        if (0x1000 == u16_cnt++)
        {
            return HAL_FCIE_TIMEOUT;
        }

    } while (BIT_RST_STS_MASK != (HAL_READ_WORD(HAL_FCIE0_NC_FCIE_RST(hal)) & BIT_RST_STS_MASK));

    HAL_WRITE_WORD(HAL_FCIE0_NC_FCIE_RST(hal), BIT_FCIE_SOFT_RST);

    // Restore reset bit, check reset status from 1 -> 0
    u16_cnt = 0;
    do
    {
        FLASH_IMPL_USDELAY(1);

        if (0x1000 == u16_cnt++)
        {
            return HAL_FCIE_TIMEOUT;
        }

    } while (0 != (HAL_READ_WORD(HAL_FCIE0_NC_FCIE_RST(hal)) & BIT_RST_STS_MASK));

    return HAL_FCIE_SUCCESS;
}

u8 hal_fcie_ecc_setup(struct hal_fcie *hal, u8 mode)
{
    u8                                page_mode = 0;
    u8                                ecc_mode  = 0;
    u16                               data_mode = 0;
    const struct hal_fcie_ecc_config *config    = NULL;

    if (mode >= HAL_FCIE_ECC_CONFIG_MAX_CNT)
        return HAL_FCIE_INVALID;

    config = &fcie_ecc_config_list[mode];

    page_mode = hal_fcie_ecc_get_page_mode(config->page_size);
    ecc_mode  = hal_fcie_ecc_get_mode(config->sector_size, config->max_correct_bits);
    data_mode = hal_fcie_ecc_get_data_mode(config->sector_size);

    HAL_WRITE_WORD(HAL_FCIE0_NC_ECC_CTRL(hal), page_mode | (ecc_mode << 3));
    HAL_WRITE_WORD(HAL_FCIE0_NC_SPARE_SIZE(hal), config->oob_size);
    HAL_WRITE_WORD(HAL_FCIE0_NC_SPARE(hal), config->ecc_bytes);
    HAL_WRITE_WORD(HAL_FCIE0_NC_FUN_CTL(hal), 0x1 | BIT_NC_SPI_MODE);
    HAL_WRITE_WORD(HAL_FCIE0_NC_DDR_CTRL(hal), BIT_NC_32B_MODE);
    HAL_WRITE_WORD(HAL_FCIE0_NC_MIE_INT_EN(hal), 0x1);
    HAL_WRITE_WORD(HAL_FCIE0_NC_ECC_MODE_SEL(hal), data_mode);
    HAL_WRITE_WORD(HAL_FCIE0_NC_ECC_SPARE_SIZE(hal), config->ecc_bytes);

    HAL_WRITE_WORD(HAL_FCIE0_NC_BOOT_MODE(hal), 0);
    HAL_WRITE_WORD(HAL_FCIE3_NC_BOOT_MODE(hal), (HAL_READ_WORD(HAL_FCIE3_NC_BOOT_MODE(hal)) | (~(1 << 4))));

    hal->page_size = config->page_size;
    hal->oob_size  = config->oob_size;

    return HAL_FCIE_SUCCESS;
}

u8 hal_fcie_ecc_encode(struct hal_fcie *hal, u8 path, u8 sector_cnt, u8 *fcie_buf)
{
    u64 phys_addr = 0;
    u64 data_addr = 0;
    u64 oob_addr  = 0;

    flash_impl_mem_flush((void *)fcie_buf, hal->page_size + hal->oob_size);
    flash_impl_miupipe_flush();

    HAL_WRITE_WORD(HAL_FCIE0_NC_ECC_SPARE_SIZE(hal),
                   (HAL_READ_WORD(HAL_FCIE0_NC_ECC_SPARE_SIZE(hal)) & ~BIT_ECC_DIR) | FCIR_DIR_WRITE);

    if (path == FCIE_ECC_PATH_IMI)
    {
        HAL_WRITE_WORD(HAL_FCIE0_NC_BOOT_MODE(hal), BIT_IMI_SEL);
        HAL_WRITE_WORD(HAL_FCIE3_NC_BOOT_MODE(hal), (HAL_READ_WORD(HAL_FCIE3_NC_BOOT_MODE(hal)) | (1 << 4)));
    }
    else
    {
        HAL_WRITE_WORD(HAL_FCIE0_NC_BOOT_MODE(hal), 0);
        HAL_WRITE_WORD(HAL_FCIE3_NC_BOOT_MODE(hal), (HAL_READ_WORD(HAL_FCIE3_NC_BOOT_MODE(hal)) | (~(1 << 4))));
    }

    phys_addr = flash_impl_virt_to_phys((void *)fcie_buf);
    data_addr = flash_impl_phys_to_miu(phys_addr);
    oob_addr  = data_addr + hal->page_size;

    HAL_WRITE_WORD(HAL_FCIE0_NC_ECC_DATA_ADDR_L(hal), (data_addr >> 4) & 0xFFFF);
    HAL_WRITE_WORD(HAL_FCIE0_NC_ECC_DATA_ADDR_H(hal), (data_addr >> 20) & 0xFFFF);
    HAL_WRITE_WORD(HAL_FCIE0_NC_ECC_SPARE_ADDR_L(hal), (oob_addr >> 4) & 0xFFFF);
    HAL_WRITE_WORD(HAL_FCIE0_NC_ECC_SPARE_ADDR_H(hal), (oob_addr >> 20) & 0xFFFF);
    HAL_WRITE_WORD(HAL_FCIE0_NC_ECC_SPARE_SIZE(hal), HAL_READ_WORD(HAL_FCIE0_NC_ECC_SPARE_SIZE(hal)) | BIT_ADDR_LATCH);

    HAL_WRITE_WORD(HAL_FCIE0_NC_WDATA_DMA_ADR0(hal), data_addr & 0xFFFF);
    HAL_WRITE_WORD(HAL_FCIE0_NC_WDATA_DMA_ADR1(hal), data_addr >> 16);
    HAL_WRITE_WORD(HAL_FCIE0_NC_DATA_BASE_ADDR_MSB(hal), ((data_addr >> 32) & 0x0f) << 8);
    HAL_WRITE_WORD(HAL_FCIE0_NC_WSPARE_DMA_ADR0(hal), oob_addr & 0xFFFF);
    HAL_WRITE_WORD(HAL_FCIE0_NC_WSPARE_DMA_ADR1(hal), oob_addr >> 16);
    HAL_WRITE_WORD(HAL_FCIE0_NC_SPARE_BASE_ADDR_MSB(hal), ((oob_addr >> 32) & 0x0f) << 8);

    HAL_WRITE_WORD(HAL_FCIE0_NC_PART_MODE(hal), BIT_PARTIAL_MODE_EN | ((sector_cnt - 1) << 1));

    HAL_WRITE_WORD(HAL_FCIE0_NC_AUXREG_ADR(hal), AUXADR_INSTQUE);
    HAL_WRITE_WORD(HAL_FCIE0_NC_AUXREG_DAT(hal), (ACT_BREAK << 8) | ACT_SER_DOUT);
    HAL_WRITE_WORD(HAL_FCIE0_NC_CTRL(hal), HAL_READ_WORD(HAL_FCIE0_NC_CTRL(hal)) | BIT_NC_JOB_START);

    if (!hal_fcie_job_is_done(hal, FCIE_WAIT_READ_TIME))
    {
        return HAL_FCIE_TIMEOUT;
    }

    flash_impl_mem_invalidate((void *)fcie_buf, hal->page_size + hal->oob_size);

    HAL_WRITE_WORD(HAL_FCIE0_NC_MIE_EVENT(hal), (HAL_READ_WORD(HAL_FCIE0_NC_MIE_EVENT(hal)) & BIT_NC_JOB_END));
    HAL_WRITE_WORD(HAL_FCIE0_NC_PART_MODE(hal), 0);

    return HAL_FCIE_SUCCESS;
}

u8 hal_fcie_ecc_decode(struct hal_fcie *hal, u8 path, u8 sector_cnt, u8 *fcie_buf)
{
    u64 phys_addr = 0;
    u64 data_addr = 0;
    u64 oob_addr  = 0;

    flash_impl_mem_flush((void *)fcie_buf, hal->page_size + hal->oob_size);
    flash_impl_miupipe_flush();

    HAL_WRITE_WORD(HAL_FCIE0_NC_ECC_SPARE_SIZE(hal),
                   (HAL_READ_WORD(HAL_FCIE0_NC_ECC_SPARE_SIZE(hal)) & ~BIT_ECC_DIR) | FCIR_DIR_READ);

    if (path == FCIE_ECC_PATH_IMI)
    {
        HAL_WRITE_WORD(HAL_FCIE0_NC_BOOT_MODE(hal), BIT_IMI_SEL);
        HAL_WRITE_WORD(HAL_FCIE3_NC_BOOT_MODE(hal), (HAL_READ_WORD(HAL_FCIE3_NC_BOOT_MODE(hal)) | (1 << 4)));
    }
    else
    {
        HAL_WRITE_WORD(HAL_FCIE0_NC_BOOT_MODE(hal), 0);
        HAL_WRITE_WORD(HAL_FCIE3_NC_BOOT_MODE(hal), (HAL_READ_WORD(HAL_FCIE3_NC_BOOT_MODE(hal)) | (~(1 << 4))));
    }

    phys_addr = flash_impl_virt_to_phys((void *)fcie_buf);
    data_addr = flash_impl_phys_to_miu(phys_addr);
    oob_addr  = data_addr + hal->page_size;

    HAL_WRITE_WORD(HAL_FCIE0_NC_ECC_DATA_ADDR_L(hal), (data_addr >> 4) & 0xFFFF);
    HAL_WRITE_WORD(HAL_FCIE0_NC_ECC_DATA_ADDR_H(hal), (data_addr >> 20) & 0xFFFF);
    HAL_WRITE_WORD(HAL_FCIE0_NC_ECC_SPARE_ADDR_L(hal), (oob_addr >> 4) & 0xFFFF);
    HAL_WRITE_WORD(HAL_FCIE0_NC_ECC_SPARE_ADDR_H(hal), (oob_addr >> 20) & 0xFFFF);
    HAL_WRITE_WORD(HAL_FCIE0_NC_ECC_SPARE_SIZE(hal), HAL_READ_WORD(HAL_FCIE0_NC_ECC_SPARE_SIZE(hal)) | BIT_ADDR_LATCH);

    HAL_WRITE_WORD(HAL_FCIE0_NC_RDATA_DMA_ADR0(hal), data_addr & 0xFFFF);
    HAL_WRITE_WORD(HAL_FCIE0_NC_RDATA_DMA_ADR1(hal), (data_addr >> 16) & 0xFFFF);
    HAL_WRITE_WORD(HAL_FCIE0_NC_DATA_BASE_ADDR_MSB(hal), (data_addr >> 32) & 0x0f);
    HAL_WRITE_WORD(HAL_FCIE0_NC_RSPARE_DMA_ADR0(hal), oob_addr & 0xFFFF);
    HAL_WRITE_WORD(HAL_FCIE0_NC_RSPARE_DMA_ADR1(hal), (oob_addr >> 16) & 0xFFFF);
    HAL_WRITE_WORD(HAL_FCIE0_NC_SPARE_BASE_ADDR_MSB(hal), (oob_addr >> 32) & 0x0f);

    HAL_WRITE_WORD(HAL_FCIE0_NC_PART_MODE(hal), BIT_PARTIAL_MODE_EN | ((sector_cnt - 1) << 1));

    HAL_WRITE_WORD(HAL_FCIE0_NC_AUXREG_ADR(hal), AUXADR_INSTQUE);
    HAL_WRITE_WORD(HAL_FCIE0_NC_AUXREG_DAT(hal), (ACT_BREAK << 8) | ACT_SER_DIN);
    HAL_WRITE_WORD(HAL_FCIE0_NC_CTRL(hal), HAL_READ_WORD(HAL_FCIE0_NC_CTRL(hal)) | BIT_NC_JOB_START);

    if (!hal_fcie_job_is_done(hal, FCIE_WAIT_READ_TIME))
    {
        return HAL_FCIE_TIMEOUT;
    }

    flash_impl_mem_invalidate((void *)fcie_buf, hal->page_size + hal->oob_size);

    HAL_WRITE_WORD(HAL_FCIE0_NC_MIE_EVENT(hal), (HAL_READ_WORD(HAL_FCIE0_NC_MIE_EVENT(hal)) & BIT_NC_JOB_END));
    HAL_WRITE_WORD(HAL_FCIE0_NC_PART_MODE(hal), 0);

    return HAL_FCIE_SUCCESS;
}

u8 hal_fcie_ecc_get_status(struct hal_fcie *hal, u8 *ecc_status, u8 *ecc_bitflip_cnt)
{
    *ecc_status      = HAL_READ_BYTE(HAL_FCIE0_NC_ECC_STAT2(hal)) & 0x03;
    *ecc_bitflip_cnt = (HAL_READ_BYTE(HAL_FCIE0_NC_ECC_STAT0(hal)) & BITS_ECC_CORRECT) >> 1;

    return HAL_FCIE_SUCCESS;
}
