/*
 * sstar_sdio.h - Sigmastar
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

#ifndef __SSTAR_SDIO_H
#define __SSTAR_SDIO_H

#define MMC_CAP2_NO_SDIO (1 << 0)
#define MMC_CAP2_NO_SD   (1 << 1)
#define MMC_CAP2_NO_MMC  (1 << 2)
#define MMC_BUS_WIDTH_1  1
#define MMC_BUS_WIDTH_4  4
#define MMC_BUS_WIDTH_8  8

#define MMC_CMD_MASK (3 << 5) /* non-SPI command type */
#define MMC_CMD_AC   (0 << 5)
#define MMC_CMD_ADTC (1 << 5)
#define MMC_CMD_BC   (2 << 5)
#define MMC_CMD_BCR  (3 << 5)

/* Can the host do 4 bit transfers */
#define MMC_CAP_4_BIT_DATA (1 << 0)
/* Can do MMC high-speed timing */
#define MMC_CAP_MMC_HIGHSPEED (1 << 1)
/* Can do SD high-speed timing */
#define MMC_CAP_SD_HIGHSPEED (1 << 2)

/* SDIO commands                         type  argument     response */
#define SD_SEND_RELATIVE_ADDR 3  /* bcr                     R6  */
#define SD_IO_SEND_OP_COND    5  /* bcr  [23:0] OCR         R4  */
#define SD_IO_RW_DIRECT       52 /* ac   [31:0] See below   R5  */
#define SD_IO_RW_EXTENDED     53 /* adtc [31:0] See below   R5  */

/*
 * SD_IO_RW_EXTENDED argument format:
 *
 *      [31] R/W flag
 *      [30:28] Function number
 *      [27] Block mode
 *      [26] Increment address
 *      [25:9] Register address
 *      [8:0] Byte/block count
 */

#define R4_18V_PRESENT    (1 << 24)
#define R4_MEMORY_PRESENT (1 << 27)

#define MMC_TYPE_MMC      0 /* MMC card */
#define MMC_TYPE_SD       1 /* SD card */
#define MMC_TYPE_SDIO     2 /* SDIO card */
#define MMC_TYPE_SD_COMBO 3 /* SD combo (IO+mem) card */

#define R5_COM_CRC_ERROR       (1 << 15) /* er, b */
#define R5_ILLEGAL_COMMAND     (1 << 14) /* er, b */
#define R5_ERROR               (1 << 11) /* erx, c */
#define R5_FUNCTION_NUMBER     (1 << 9)  /* er, c */
#define R5_OUT_OF_RANGE        (1 << 8)  /* er, c */
#define R5_STATUS(x)           (x & 0xCB00)
#define R5_IO_CURRENT_STATE(x) ((x & 0x3000) >> 12) /* s, b */

/*
 * Card Common Control Registers (CCCR)
 */

#define SDIO_CCCR_CCCR 0x00

#define SDIO_CCCR_REV_1_00 0 /* CCCR/FBR Version 1.00 */
#define SDIO_CCCR_REV_1_10 1 /* CCCR/FBR Version 1.10 */
#define SDIO_CCCR_REV_1_20 2 /* CCCR/FBR Version 1.20 */
#define SDIO_CCCR_REV_3_00 3 /* CCCR/FBR Version 3.00 */

#define SDIO_SDIO_REV_1_00 0 /* SDIO Spec Version 1.00 */
#define SDIO_SDIO_REV_1_10 1 /* SDIO Spec Version 1.10 */
#define SDIO_SDIO_REV_1_20 2 /* SDIO Spec Version 1.20 */
#define SDIO_SDIO_REV_2_00 3 /* SDIO Spec Version 2.00 */
#define SDIO_SDIO_REV_3_00 4 /* SDIO Spec Version 3.00 */

#define SDIO_CCCR_SD 0x01

#define SDIO_SD_REV_1_01 0 /* SD Physical Spec Version 1.01 */
#define SDIO_SD_REV_1_10 1 /* SD Physical Spec Version 1.10 */
#define SDIO_SD_REV_2_00 2 /* SD Physical Spec Version 2.00 */
#define SDIO_SD_REV_3_00 3 /* SD Physical Spev Version 3.00 */

#define SDIO_CCCR_IOEx 0x02
#define SDIO_CCCR_IORx 0x03

#define SDIO_CCCR_IENx 0x04 /* Function/Master Interrupt Enable */
#define SDIO_CCCR_INTx 0x05 /* Function Interrupt Pending */

#define SDIO_CCCR_ABORT 0x06 /* function abort/card reset */

#define SDIO_CCCR_IF 0x07 /* bus interface controls */

#define SDIO_BUS_WIDTH_MASK     0x03 /* data bus width setting */
#define SDIO_BUS_WIDTH_1BIT     0x00
#define SDIO_BUS_WIDTH_RESERVED 0x01
#define SDIO_BUS_WIDTH_4BIT     0x02
#define SDIO_BUS_ECSI           0x20 /* Enable continuous SPI interrupt */
#define SDIO_BUS_SCSI           0x40 /* Support continuous SPI interrupt */

#define SDIO_BUS_ASYNC_INT 0x20

#define SDIO_BUS_CD_DISABLE 0x80 /* disable pull-up on DAT3 (pin 1) */

#define SDIO_CCCR_CAPS 0x08

#define SDIO_CCCR_CAP_SDC  0x01 /* can do CMD52 while data transfer */
#define SDIO_CCCR_CAP_SMB  0x02 /* can do multi-block xfers (CMD53) */
#define SDIO_CCCR_CAP_SRW  0x04 /* supports read-wait protocol */
#define SDIO_CCCR_CAP_SBS  0x08 /* supports suspend/resume */
#define SDIO_CCCR_CAP_S4MI 0x10 /* interrupt during 4-bit CMD53 */
#define SDIO_CCCR_CAP_E4MI 0x20 /* enable ints during 4-bit CMD53 */
#define SDIO_CCCR_CAP_LSC  0x40 /* low speed card */
#define SDIO_CCCR_CAP_4BLS 0x80 /* 4 bit low speed card */

#define SDIO_CCCR_CIS 0x09 /* common CIS pointer (3 bytes) */

/* Following 4 regs are valid only if SBS is set */
#define SDIO_CCCR_SUSPEND 0x0c
#define SDIO_CCCR_SELx    0x0d
#define SDIO_CCCR_EXECx   0x0e
#define SDIO_CCCR_READYx  0x0f

#define SDIO_CCCR_BLKSIZE 0x10

#define SDIO_CCCR_POWER 0x12

#define SDIO_POWER_SMPC 0x01 /* Supports Master Power Control */
#define SDIO_POWER_EMPC 0x02 /* Enable Master Power Control */

#define SDIO_CCCR_IO_ENABLE   0x02
#define SDIO_CCCR_IO_READY    0x03
#define SDIO_CCCR_FN0_BLKSIZE 0x10

#define SDIO_CCCR_INT_ENABLE  0x04
#define SDIO_CCCR_INT_PENDING 0x05

#define SDIO_CCCR_SPEED 0x13

#define SDIO_SPEED_SHS       0x01 /* Supports High-Speed mode */
#define SDIO_SPEED_BSS_SHIFT 1
#define SDIO_SPEED_BSS_MASK  (7 << SDIO_SPEED_BSS_SHIFT)
#define SDIO_SPEED_SDR12     (0 << SDIO_SPEED_BSS_SHIFT)
#define SDIO_SPEED_SDR25     (1 << SDIO_SPEED_BSS_SHIFT)
#define SDIO_SPEED_SDR50     (2 << SDIO_SPEED_BSS_SHIFT)
#define SDIO_SPEED_SDR104    (3 << SDIO_SPEED_BSS_SHIFT)
#define SDIO_SPEED_DDR50     (4 << SDIO_SPEED_BSS_SHIFT)
#define SDIO_SPEED_EHS       SDIO_SPEED_SDR25 /* Enable High-Speed */

#define SDIO_CCCR_UHS   0x14
#define SDIO_UHS_SDR50  0x01
#define SDIO_UHS_SDR104 0x02
#define SDIO_UHS_DDR50  0x04

#define SDIO_CCCR_DRIVE_STRENGTH 0x15
#define SDIO_SDTx_MASK           0x07
#define SDIO_DRIVE_SDTA          (1 << 0)
#define SDIO_DRIVE_SDTC          (1 << 1)
#define SDIO_DRIVE_SDTD          (1 << 2)
#define SDIO_DRIVE_DTSx_MASK     0x03
#define SDIO_DRIVE_DTSx_SHIFT    4
#define SDIO_DTSx_SET_TYPE_B     (0 << SDIO_DRIVE_DTSx_SHIFT)
#define SDIO_DTSx_SET_TYPE_A     (1 << SDIO_DRIVE_DTSx_SHIFT)
#define SDIO_DTSx_SET_TYPE_C     (2 << SDIO_DRIVE_DTSx_SHIFT)
#define SDIO_DTSx_SET_TYPE_D     (3 << SDIO_DRIVE_DTSx_SHIFT)
/*
 * Function Basic Registers (FBR)
 */

#define SDIO_FBR_BASE(f) ((f)*0x100) /* base of function f's FBRs */

#define SDIO_FBR_STD_IF     0x00
#define SDIO_FBR_IO_BLKSIZE 0x10

#define SDIO_FBR_SUPPORTS_CSA 0x40 /* supports Code Storage Area */
#define SDIO_FBR_ENABLE_CSA   0x80 /* enable Code Storage Area */

#define SDIO_FBR_STD_IF_EXT 0x01

#define SDIO_FBR_POWER 0x02

#define SDIO_FBR_POWER_SPS 0x01 /* Supports Power Selection */
#define SDIO_FBR_POWER_EPS 0x02 /* Enable (low) Power Selection */

#define SDIO_FBR_CIS 0x09 /* CIS pointer (3 bytes) */

#define SDIO_FBR_CSA 0x0C /* CSA pointer (3 bytes) */

#define SDIO_FBR_CSA_DATA 0x0F

#define SDIO_FBR_BLKSIZE 0x10 /* block size (2 bytes) */

#define MMC_CARD_BUSY 0x80000000 /* Card Power up status bit */

#define SDIO_MAX_FUNCS           7
#define MMC_QUIRK_NONSTD_SDIO    (1 << 2) /* non-standard SDIO card */
#define MMC_QUIRK_NONSTD_FUNC_IF (1 << 4)

#define SDIO_MAX_BLOCK_COUNT 512
#define SDIO_MAX_BLOCK_SIZE  512

//====================================================================
struct sdio_cccr
{
    unsigned int sdio_vsn;
    unsigned int sd_vsn;
    unsigned int multi_block : 1, low_speed : 1, wide_bus : 1, high_power : 1, high_speed : 1, disable_cd : 1;
};

struct sd_scr
{
    unsigned char sda_vsn;
    unsigned char sda_spec3;
    unsigned char sda_spec4;
    unsigned char sda_specx;
    unsigned char bus_widths;
#define SD_SCR_BUS_WIDTH_1 (1 << 0)
#define SD_SCR_BUS_WIDTH_4 (1 << 2)
    unsigned char cmds;
#define SD_SCR_CMD20_SUPPORT (1 << 0)
#define SD_SCR_CMD23_SUPPORT (1 << 1)
};

struct sd_switch_caps
{
    unsigned int hs_max_dtr;
    unsigned int uhs_max_dtr;
#define HIGH_SPEED_MAX_DTR    50000000
#define UHS_SDR104_MAX_DTR    208000000
#define UHS_SDR50_MAX_DTR     100000000
#define UHS_DDR50_MAX_DTR     50000000
#define UHS_SDR25_MAX_DTR     UHS_DDR50_MAX_DTR
#define UHS_SDR12_MAX_DTR     25000000
#define DEFAULT_SPEED_MAX_DTR UHS_SDR12_MAX_DTR
    unsigned int sd3_bus_mode;
#define UHS_SDR12_BUS_SPEED  0
#define HIGH_SPEED_BUS_SPEED 1
#define UHS_SDR25_BUS_SPEED  1
#define UHS_SDR50_BUS_SPEED  2
#define UHS_SDR104_BUS_SPEED 3
#define UHS_DDR50_BUS_SPEED  4

#define SD_MODE_HIGH_SPEED (1 << HIGH_SPEED_BUS_SPEED)

    unsigned int sd3_drv_type;
#define SD_DRIVER_TYPE_B 0x01
#define SD_DRIVER_TYPE_A 0x02
#define SD_DRIVER_TYPE_C 0x04
#define SD_DRIVER_TYPE_D 0x08
    unsigned int sd3_curr_limit;
#define SD_SET_CURRENT_LIMIT_200 0
#define SD_SET_CURRENT_LIMIT_400 1
#define SD_SET_CURRENT_LIMIT_600 2
#define SD_SET_CURRENT_LIMIT_800 3
#define SD_SET_CURRENT_NO_CHANGE (-1)

#define SD_MAX_CURRENT_200 (1 << SD_SET_CURRENT_LIMIT_200)
#define SD_MAX_CURRENT_400 (1 << SD_SET_CURRENT_LIMIT_400)
#define SD_MAX_CURRENT_600 (1 << SD_SET_CURRENT_LIMIT_600)
#define SD_MAX_CURRENT_800 (1 << SD_SET_CURRENT_LIMIT_800)
};

struct sdio_func;
typedef void(sdio_irq_handler_t)(struct sdio_func *);
#define msleep(a) udelay(a * 1000)

/*
 * SDIO function CIS tuple (unknown to the core)
 */
struct sdio_func_tuple
{
    struct sdio_func_tuple *next;
    unsigned char           code;
    unsigned char           size;
    unsigned char           data[];
};

struct sdio_cis
{
    unsigned short vendor;
    unsigned short device;
    unsigned short blksize;
    unsigned int   max_dtr;
};

struct sdio_card
{
    struct mmc *            mmc; /* the host this device belongs to */
    unsigned int            card_type;
    u8                      func_enable;
    unsigned int            sdio_funcs; /* number of SDIO functions */
    unsigned int            curr_blksize;
    struct sdio_cccr        cccr;                      /* common card info */
    struct sdio_cis         cis;                       /* common tuple info */
    struct sd_scr           scr;                       /* extra SD information */
    struct sd_switch_caps   sw_caps;                   /* switch (CMD6) caps */
    struct sdio_func_tuple *tuples;                    /* unknown common tuples */
    struct sdio_func *      sdio_func[SDIO_MAX_FUNCS]; /* SDIO functions (devices) */
    u8                      major_rev;                 /* major revision number */
    u8                      minor_rev;                 /* minor revision number */
    unsigned                num_info;                  /* number of info strings */
    const char **           info;                      /* info strings */
    unsigned int            quirks;                    /* card quirks */
};

enum sdio_command_ret_t
{
    SDIO_RET_SUCCESS,    /* 0 = Success */
    SDIO_RET_FAILURE,    /* 1 = Failure */
    SDIO_RET_USAGE = -1, /* Failure, please report 'usage' error */
};

// =================================================================
#ifdef CONFIG_SSTAR_SUPPORT_SDIO
int  sdio_init(struct mmc *mmc);
int  sdio_set_block_size(struct sdio_func *func, unsigned blksz);
int  sdio_read_byte(u8 devidx, u8 func, u32 addr, u8 *r_buf);
int  sdio_write_byte(u8 devidx, u8 func, u32 addr, u8 w_value);
int  sdio_read_multi(u8 devidx, u8 func, u32 addr, u32 count, u8 *r_buf);
int  sdio_write_multi(u8 devidx, u8 func, u32 addr, u32 count, u8 *w_buf);
void sstar_sdio_rw_test(u8 devidx, struct mmc *mmc);
#endif
#endif
