/*
 * drv_sdmmc_rtk.h- Sigmastar
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
/***************************************************************************************************************
 *
 * FileName drv_sdmmc_rtk.h
 *     @author jeremy.wang (2012/01/10)
 * Desc:
 *     This layer between Linux SD Driver layer and IP Hal layer.
 *     (1) The goal is we don't need to change any Linux SD Driver code, but we can handle here.
 *     (2) You could define Function/Ver option for using, but don't add Project option here.
 *     (3) You could use function option by Project option, but please add to ms_sdmmc.h
 *
 ***************************************************************************************************************/

#ifndef __SS_SDMMC_RTK_H
#define __SS_SDMMC_RTK_H

#if defined(__KERNEL__)
#include <linux/cdev.h>
#include <linux/interrupt.h>
#endif

#ifdef CAM_OS_RTK
#include "cam_os_wrapper.h"
#include "rtk_cfg.h"
#endif

#include "hal_sdmmc_base.h"
#include "hal_sdmmc_platform.h"

//***********************************************************************************************************
// Config Setting (Externel)
//***********************************************************************************************************

extern CamOsWorkQueue cdzWorkQueue[], hotplugWorkQueue[];

//***********************************************************************************************************
//***********************************************************************************************************
//------------------------------------------------------------------------------
// MMC/SD Command Index
//------------------------------------------------------------------------------
/* SD commands                           type  argument     response */
/* class 0 */
/* This is basically the same command as for MMC with some quirks. */
#define SD_SEND_RELATIVE_ADDR 3  /* bcr                     R6  */
#define SD_SEND_IF_COND       8  /* bcr  [11:0] See below   R7  */
#define SD_SWITCH_VOLTAGE     11 /* ac                      R1  */

/* class 10 */
#define SD_SWITCH 6 /* adtc [31:0] See below   R1  */

/* class 5 */
#define SD_ERASE_WR_BLK_START 32 /* ac   [31:0] data addr   R1  */
#define SD_ERASE_WR_BLK_END   33 /* ac   [31:0] data addr   R1  */

/* Application commands */
#define SD_APP_SET_BUS_WIDTH    6  /* ac   [1:0] bus width    R1  */
#define SD_APP_SD_STATUS        13 /* adtc                    R1  */
#define SD_APP_SEND_NUM_WR_BLKS 22 /* adtc                    R1  */
#define SD_APP_OP_COND          41 /* bcr  [31:0] OCR         R3  */
#define SD_APP_SEND_SCR         51 /* adtc                    R1  */

//***********************************************************************************************************
/* SDIO commands                         type  argument     response */
#define SD_IO_SEND_OP_COND 5  /* bcr  [23:0] OCR         R4  */
#define SD_IO_RW_DIRECT    52 /* ac   [31:0] See below   R5  */
#define SD_IO_RW_EXTENDED  53 /* adtc [31:0] See below   R5  */

//***********************************************************************************************************
/* Standard MMC commands (4.1)           type  argument     response */
/* class 1 */
#define MMC_GO_IDLE_STATE       0  /* bc                          */
#define MMC_SEND_OP_COND        1  /* bcr  [31:0] OCR         R3  */
#define MMC_ALL_SEND_CID        2  /* bcr                     R2  */
#define MMC_SET_RELATIVE_ADDR   3  /* ac   [31:16] RCA        R1  */
#define MMC_SET_DSR             4  /* bc   [31:16] RCA            */
#define MMC_SLEEP_AWAKE         5  /* ac   [31:16] RCA 15:flg R1b */
#define MMC_SWITCH              6  /* ac   [31:0] See below   R1b */
#define MMC_SELECT_CARD         7  /* ac   [31:16] RCA        R1  */
#define MMC_SEND_EXT_CSD        8  /* adtc                    R1  */
#define MMC_SEND_CSD            9  /* ac   [31:16] RCA        R2  */
#define MMC_SEND_CID            10 /* ac   [31:16] RCA        R2  */
#define MMC_READ_DAT_UNTIL_STOP 11 /* adtc [31:0] dadr        R1  */
#define MMC_STOP_TRANSMISSION   12 /* ac                      R1b */
#define MMC_SEND_STATUS         13 /* ac   [31:16] RCA        R1  */
#define MMC_BUS_TEST_R          14 /* adtc                    R1  */
#define MMC_GO_INACTIVE_STATE   15 /* ac   [31:16] RCA            */
#define MMC_BUS_TEST_W          19 /* adtc                    R1  */
#define MMC_SPI_READ_OCR        58 /* spi                  spi_R3 */
#define MMC_SPI_CRC_ON_OFF      59 /* spi  [0:0] flag      spi_R1 */

/* class 2 */
#define MMC_SET_BLOCKLEN            16 /* ac   [31:0] block len   R1  */
#define MMC_READ_SINGLE_BLOCK       17 /* adtc [31:0] data addr   R1  */
#define MMC_READ_MULTIPLE_BLOCK     18 /* adtc [31:0] data addr   R1  */
#define MMC_SEND_TUNING_BLOCK       19 /* adtc                    R1  */
#define MMC_SEND_TUNING_BLOCK_HS200 21 /* adtc R1  */

/* class 3 */
#define MMC_WRITE_DAT_UNTIL_STOP 20 /* adtc [31:0] data addr   R1  */

/* class 4 */
#define MMC_SET_BLOCK_COUNT      23 /* adtc [31:0] data addr   R1  */
#define MMC_WRITE_BLOCK          24 /* adtc [31:0] data addr   R1  */
#define MMC_WRITE_MULTIPLE_BLOCK 25 /* adtc                    R1  */
#define MMC_PROGRAM_CID          26 /* adtc                    R1  */
#define MMC_PROGRAM_CSD          27 /* adtc                    R1  */

/* class 6 */
#define MMC_SET_WRITE_PROT  28 /* ac   [31:0] data addr   R1b */
#define MMC_CLR_WRITE_PROT  29 /* ac   [31:0] data addr   R1b */
#define MMC_SEND_WRITE_PROT 30 /* adtc [31:0] wpdata addr R1  */

/* class 5 */
#define MMC_ERASE_GROUP_START 35 /* ac   [31:0] data addr   R1  */
#define MMC_ERASE_GROUP_END   36 /* ac   [31:0] data addr   R1  */
#define MMC_ERASE             38 /* ac                      R1b */

/* class 9 */
#define MMC_FAST_IO      39 /* ac   <Complex>          R4  */
#define MMC_GO_IRQ_STATE 40 /* bcr                     R5  */

/* class 7 */
#define MMC_LOCK_UNLOCK 42 /* adtc                    R1b */

/* class 8 */
#define MMC_APP_CMD 55 /* ac   [31:16] RCA        R1  */
#define MMC_GEN_CMD 56 /* adtc [0] RD/WR          R1  */

/* class 11 */
#define MMC_QUE_TASK_PARAMS    44 /* ac   [20:16] task id    R1  */
#define MMC_QUE_TASK_ADDR      45 /* ac   [31:0] data addr   R1  */
#define MMC_EXECUTE_READ_TASK  46 /* adtc [20:16] task id    R1  */
#define MMC_EXECUTE_WRITE_TASK 47 /* adtc [20:16] task id    R1  */
#define MMC_CMDQ_TASK_MGMT     48 /* ac   [20:16] task id    R1b */

//***********************************************************************************************************
//***********************************************************************************************************
//------------------------------------------------------------------------------
#define ENOENT    2
#define ENOMEM    12  /* Out of Memory */
#define EINVAL    22  /* Invalid argument */
#define ENOSPC    28  /* No space left on device */
#define EILSEQ    84  /* Illegal byte sequence */
#define ETIMEDOUT 110 /* Connection timed out */

typedef enum
{
    EV_SDMMC1 = 0,
    EV_SDMMC2 = 1,
    EV_SDMMC3 = 2,

} SlotEmType;

typedef enum
{
    EV_MUTEX1  = 0,
    EV_MUTEX2  = 1,
    EV_MUTEX3  = 2,
    EV_NOMUTEX = 3,

} MutexEmType;

struct sstar_mmc_priv
{
    U8_T          u8_slotNo;
    U8_T          u8_transMode; // dma, adma, cifd
    U8_T          u8_fakeCdz;
    U8_T          u8_revCdz;
    U8_T          u8_intCdz;
    U8_T          u8_remmovable;
    U8_T          u8_sdioUse;
    U8_T          u8_supportCMD23;
    U8_T          u8_sdioUse1Bit;
    U8_T          u8_supportSD30;
    U8_T          u8_supportEMMC50;
    U32_T         u32_maxClk;
    U32_T         u32_pwerOnDelay;
    U32_T         u32_pwerOffDelay;
    U32_T         u32_mieIrqNo;
    U32_T         u32_cdzIrqNo;
    U32_T         pIPBANKArr[2];
    MMCPinDrv     mmc_pinDrv;
    MMCClkPhase   mmc_clkPha;
    MMCPadMuxInfo mmc_PMuxInfo;
};

struct sstar_sdmmc_host
{
    struct platform_device * pdev;
    struct sstar_sdmmc_slot *sdmmc_slot;
};
struct mmc_host
{
    U8_T slotNo; // Slot No.
};

struct sstar_sdmmc_slot
{
    struct mmc_host *mmc;

    U32_T slotNo;     // Slot No.
    U32_T mieIRQNo;   // MIE IRQ No.
    U32_T cdzIRQNo;   // CDZ IRQ No.
    U32_T pwrGPIONo;  // PWR GPIO No.
    U32_T pmrsaveClk; // Power Saving Clock

    U32_T initFlag; // First Time Init Flag
    U32_T sdioFlag; // SDIO Device Flag

    U32_T currClk;        // Current Clock
    U32_T currRealClk;    // Current Real Clock
    U8_T  currWidth;      // Current Bus Width
    U8_T  currTiming;     // Current Bus Timning
    U8_T  currPowrMode;   // Current PowerMode
    U8_T  currBusMode;    // Current Bus Mode
    U16_T currVdd;        // Current Vdd
    U8_T  currDDR;        // Current DDR
    U8_T  currTimeoutCnt; // Current Timeout Count

    int                    read_only; // WP
    int                    card_det;  // Card Detect
    struct sstar_mmc_priv *p_mmc_priv;

}; /* struct ms_sdmmc_hot*/

struct scatterlist
{
    unsigned long page_link;
    unsigned int  offset;
    unsigned int  length;
    dma_addr_t    dma_address;
#ifdef CONFIG_NEED_SG_DMA_LENGTH
    unsigned int dma_length;
#endif
};

struct mmc_command
{
    U32_T opcode;
    U32_T arg;
    U32_T rspType;
#define MMC_CMD23_ARG_REL_WR  (1 << 31)
#define MMC_CMD23_ARG_PACKED  ((0 << 31) | (1 << 30))
#define MMC_CMD23_ARG_TAG_REQ (1 << 29)
    U32_T resp[4];
    U32_T flags; /* expected response type */
#define MMC_RSP_PRESENT (1 << 0)
#define MMC_RSP_136     (1 << 1) /* 136 bit response */
#define MMC_RSP_CRC     (1 << 2) /* expect valid crc */
#define MMC_RSP_BUSY    (1 << 3) /* card may send busy */
#define MMC_RSP_OPCODE  (1 << 4) /* response contains opcode */

#define MMC_CMD_MASK (3 << 5) /* non-SPI command type */
#define MMC_CMD_AC   (0 << 5)
#define MMC_CMD_ADTC (1 << 5)
#define MMC_CMD_BC   (2 << 5)
#define MMC_CMD_BCR  (3 << 5)

#define MMC_RSP_SPI_S1   (1 << 7)  /* one status byte */
#define MMC_RSP_SPI_S2   (1 << 8)  /* second byte */
#define MMC_RSP_SPI_B4   (1 << 9)  /* four data bytes */
#define MMC_RSP_SPI_BUSY (1 << 10) /* card may send busy */

/*
 * These are the native response types, and correspond to valid bit
 * patterns of the above flags.  One additional valid pattern
 * is all zeros, which means we don't expect a response.
 */
#define MMC_RSP_NONE (0)
#define MMC_RSP_R1   (MMC_RSP_PRESENT | MMC_RSP_CRC | MMC_RSP_OPCODE)
#define MMC_RSP_R1B  (MMC_RSP_PRESENT | MMC_RSP_CRC | MMC_RSP_OPCODE | MMC_RSP_BUSY)
#define MMC_RSP_R2   (MMC_RSP_PRESENT | MMC_RSP_136 | MMC_RSP_CRC)
#define MMC_RSP_R3   (MMC_RSP_PRESENT)
#define MMC_RSP_R4   (MMC_RSP_PRESENT)
#define MMC_RSP_R5   (MMC_RSP_PRESENT | MMC_RSP_CRC | MMC_RSP_OPCODE)
#define MMC_RSP_R6   (MMC_RSP_PRESENT | MMC_RSP_CRC | MMC_RSP_OPCODE)
#define MMC_RSP_R7   (MMC_RSP_PRESENT | MMC_RSP_CRC | MMC_RSP_OPCODE)

/* Can be used by core to poll after switch to MMC HS mode */
#define MMC_RSP_R1_NO_CRC (MMC_RSP_PRESENT | MMC_RSP_OPCODE)

#define mmc_resp_type(cmd) \
    ((cmd)->flags & (MMC_RSP_PRESENT | MMC_RSP_136 | MMC_RSP_CRC | MMC_RSP_BUSY | MMC_RSP_OPCODE))

/*
 * These are the SPI response types for MMC, SD, and SDIO cards.
 * Commands return R1, with maybe more info.  Zero is an error type;
 * callers must always provide the appropriate MMC_RSP_SPI_Rx flags.
 */
#define MMC_RSP_SPI_R1  (MMC_RSP_SPI_S1)
#define MMC_RSP_SPI_R1B (MMC_RSP_SPI_S1 | MMC_RSP_SPI_BUSY)
#define MMC_RSP_SPI_R2  (MMC_RSP_SPI_S1 | MMC_RSP_SPI_S2)
#define MMC_RSP_SPI_R3  (MMC_RSP_SPI_S1 | MMC_RSP_SPI_B4)
#define MMC_RSP_SPI_R4  (MMC_RSP_SPI_S1 | MMC_RSP_SPI_B4)
#define MMC_RSP_SPI_R5  (MMC_RSP_SPI_S1 | MMC_RSP_SPI_S2)
#define MMC_RSP_SPI_R7  (MMC_RSP_SPI_S1 | MMC_RSP_SPI_B4)

#define mmc_spi_resp_type(cmd) ((cmd)->flags & (MMC_RSP_SPI_S1 | MMC_RSP_SPI_BUSY | MMC_RSP_SPI_S2 | MMC_RSP_SPI_B4))

/*
 * These are the command types.
 */
#define mmc_cmd_type(cmd) ((cmd)->flags & MMC_CMD_MASK)

    unsigned int retries; /* max number of retries */
    int          error;   /* command error */

    /*
     * Standard errno values are used for errors, but some have specific
     * meaning in the MMC layer:
     *
     * ETIMEDOUT    Card took too long to respond
     * EILSEQ       Basic format problem with the received or sent data
     *              (e.g. CRC check failed, incorrect opcode in response
     *              or bad end bit)
     * EINVAL       Request cannot be performed because of restrictions
     *              in hardware and/or the driver
     * ENOMEDIUM    Host can determine that the slot is empty and is
     *              actively failing requests
     */

    unsigned int        busy_timeout; /* busy detect timeout in ms */
    struct mmc_data *   data;         /* data segment associated with cmd */
    struct mmc_request *mrq;          /* associated request */
};

struct mmc_data
{
    unsigned int   timeout_ns;   /* data timeout (in ns, max 80ms) */
    unsigned int   timeout_clks; /* data timeout (in clocks) */
    unsigned int   blksz;        /* data block size */
    unsigned int   blocks;       /* number of blocks */
    unsigned int   blk_addr;     /* block address */
    int            error;        /* data error */
    unsigned int   flags;
    volatile char *pu8Buf;

#define MMC_DATA_WRITE BIT08_T
#define MMC_DATA_READ  BIT09_T
/* Extra flags used by CQE */
#define MMC_DATA_QBR BIT10_T         /* CQE queue barrier*/
#define MMC_DATA_PRIO       BIT11_T) /* CQE high priority */
#define MMC_DATA_REL_WR     BIT12_T  /* Reliable write */
#define MMC_DATA_DAT_TAG    BIT13_T  /* Tag request */
#define MMC_DATA_FORCED_PRG BIT14_T  /* Forced programming */

    unsigned int bytes_xfered;

    struct mmc_command *stop; /* stop command */
    struct mmc_request *mrq;  /* associated request */

    unsigned int        sg_len;      /* size of scatter list */
    int                 sg_count;    /* mapped sg entries */
    struct scatterlist *sg;          /* I/O scatter list */
    s32                 host_cookie; /* host private data */
};

struct mmc_request
{
    struct mmc_command *sbc; /* SET_BLOCK_COUNT for multiblock */
    struct mmc_command *cmd;
    struct mmc_data *   data;
    struct mmc_command *stop;
};

struct msSt_sd_switch_caps
{
    U32_T hs_max_dtr;
    U32_T uhs_max_dtr;
#define HIGN_SPEED_MODE_MAX_CLK_FREQUENCY 50000000
#define UHS_SDR104_MODE_MAX_CLK_FREQUENCY 208000000
#define UHS_SDR50_MODE_MAX_CLK_FREQUENCY  100000000
#define UHS_DDR50_MODE_MAX_CLK_FREQUENCY  50000000
#define UHS_SDR25_MODE_MAX_CLK_FREQUENCY  UHS_DDR50_MAX_DTR
#define UHS_SDR12_MODE_MAX_CLK_FREQUENCY  25000000
    U32_T sd3BusMode;
#define BUS_SPEED_UHS_SDR12  0
#define BUS_SPEED_HIGH_SPEED 1
#define BUS_SPEED_UHS_SDR25  1
#define BUS_SPEED_UHS_SDR50  2
#define BUS_SPEED_UHS_SDR104 3
#define BUS_SPEED_UHS_DDR50  4

#define SD_MODE_HIGH_SPEED (1 << BUS_SPEED_HIGH_SPEED)
#define SD_MODE_UHS_SDR12  (1 << BUS_SPEED_UHS_SDR12)
#define SD_MODE_UHS_SDR25  (1 << BUS_SPEED_UHS_SDR25)
#define SD_MODE_UHS_SDR50  (1 << BUS_SPEED_UHS_SDR50)
#define SD_MODE_UHS_SDR104 (1 << BUS_SPEED_UHS_SDR104)
#define SD_MODE_UHS_DDR50  (1 << BUS_SPEED_UHS_DDR50)
    U32_T sd3DrvType;
#define SD_DRIVER_STRENGTH_TYPE_B 0x01
#define SD_DRIVER_STRENGTH_TYPE_A 0x02
#define SD_DRIVER_STRENGTH_TYPE_C 0x04
#define SD_DRIVER_STRENGTH_TYPE_D 0x08
    U32_T sd3CurrLimit;
#define SD_SET_CURRENT_LIMIT_TIMING_200 0
#define SD_SET_CURRENT_LIMIT_TIMING_400 1
#define SD_SET_CURRENT_LIMIT_TIMING_600 2
#define SD_SET_CURRENT_LIMIT_TIMING_800 3
#define SD_NO_CHANGE                    (-1)

#define SD_MAX_CURRENT_TIMING_200 (1 << SD_SET_CURRENT_LIMIT_TIMING_200)
#define SD_MAX_CURRENT_TIMING_400 (1 << SD_SET_CURRENT_LIMIT_TIMING_400)
#define SD_MAX_CURRENT_TIMING_600 (1 << SD_SET_CURRENT_LIMIT_TIMING_600)
#define SD_MAX_CURRENT_TIMING_800 (1 << SD_SET_CURRENT_LIMIT_TIMING_800)
};

struct msSt_sd_scr
{
    U8_T sd_vsn;
    U8_T sd_spec3;
    U8_T bus_widths;
#define SD_BUS_WIDTH_1_SCR (1 << 0)
#define SD_BUS_WIDTH_4_SCR (1 << 2)
    U8_T cmds;
#define SD_CMD20_SUPPORT_SCR (1 << 0)
#define SD_CMD23_SUPPORT_SCR (1 << 1)
};

struct msSt_sdio_cccr
{
    U32_T sdio_vsn;
    U32_T sd_vsn;
    U32_T multi_block : 1, low_speed : 1, wide_bus : 1, high_power : 1, high_speed : 1, disable_cd : 1;
};

struct msSt_sdio_cis
{
    U16_T vendor;
    U16_T device;
    U16_T blksize;
    U32_T max_dtr;
};

struct msSt_sdio_func_tuple
{
    struct msSt_sdio_func_tuple *next;
    U8_T                         code;
    U8_T                         size;
    U8_T                         data[0];
};

struct msSt_sdio_func;
struct msSt_mmc_sdio;

typedef void(sdio_irq_handler_t)(struct msSt_sdio_func *);

struct msSt_sdio_func
{
    struct msSt_mmc_sdio *card;
    sdio_irq_handler_t *  irq_handler;
    U32_T                 num; // function number

    U8_T class; // standard interface class
    U16_T vendor;
    U16_T device;

    unsigned max_blksize;
    U32_T    cur_blksize;

    unsigned enable_timeout;

    U32_T state;
#define SDIO_STATE_PRESENT (1 << 0)

    U8_T *tmpbuf;

    unsigned     num_info;
    const char **info;

    struct msSt_sdio_func_tuple *tuples;
};

typedef int(tpl_parse_t)(struct msSt_mmc_sdio *, struct msSt_sdio_func *, const U8_T *, unsigned);

struct msSt_cis_tpl
{
    U8_T         code;
    U8_T         min_size;
    tpl_parse_t *parse;
};

#define SDIO_MAX_FUNCS 7

struct msSt_mmc_sdio
{
    U32_T ocr;
    U32_T rca;
    U32_T type;
    U32_T state;
#define MMC_SDIO_STATE_PRESENT     (BIT00_T)
#define MMC_SDIO_STATE_READONLY    (BIT01_T)
#define MMC_SDIO_STATE_BLOCKADDR   (BIT02_T)
#define MMC_SDIO_CARD_SDXC         (BIT03_T)
#define MMC_SDIO_CARD_REMOVED      (BIT04_T)
#define MMC_SDIO_STATE_DOING_BKOPS (BIT05_T)
#define MMC_SDIO_STATE_SUSPENDED   (BIT06_T)
    U32_T quirks;
#define MMC_SDIO_QUIRK_LENIENT_FN0         (BIT00_T)
#define MMC_SDIO_QUIRK_BLKSZ_FOR_BYTE_MODE (BIT01_T)

#define MMC_SDIO_QUIRK_NONSTD_SDIO (BIT02_T)

#define MMC_SDIO_QUIRK_NONSTD_FUNC_IF       (BIT04_T)
#define MMC_SDIO_QUIRK_DISABLE_CD           (BIT05_T)
#define MMC_SDIO_QUIRK_INAND_CMD38          (BIT06_T)
#define MMC_SDIO_QUIRK_BLK_NO_CMD23         (BIT07_T)
#define MMC_SDIO_QUIRK_BROKEN_BYTE_MODE_512 (BIT08_T)

#define MMC_SDIO_QUIRK_LONG_READ_TIME        (BIT09_T)
#define MMC_SDIO_QUIRK_SEC_ERASE_TRIM_BROKEN (BIT10_T)
#define MMC_SDIO_QUIRK_BROKEN_IRQ_POLLING    (BIT11_T)
#define MMC_SDIO_QUIRK_TRIM_BROKEN           (BIT12_T)
#define MMC_SDIO_QUIRK_BROKEN_HPI            (BIT13_T)

    U32_T erase_size;
    U32_T erase_shift;
    U32_T pref_erase;
    U32_T eg_boundary;
    U8_T  erased_byte;

    struct msSt_sd_switch_caps sw_caps;
    struct msSt_sd_scr         scr;

    U32_T                        sdio_funcs;
    struct msSt_sdio_cccr        cccr;
    struct msSt_sdio_cis         cis;
    struct msSt_sdio_func *      sdio_func[SDIO_MAX_FUNCS];
    struct msSt_sdio_func *      sdio_single_irq;
    unsigned                     num_info;
    const char **                info;
    struct msSt_sdio_func_tuple *tuples;
    U8_T                         func_enable;
    U32_T                        curr_blksize;

    U32_T sd_bus_speed;
    U32_T mmc_avail_type;
    U32_T drive_strength;
};

#define SDIO_CCCR_SDIO_REV 0x00

#define SDIO_CCCR_SPEC_VER_1_00 0
#define SDIO_CCCR_SPEC_VER_1_10 1
#define SDIO_CCCR_SPEC_VER_1_20 2
#define SDIO_CCCR_SPEC_VER_3_00 3

#define SDIO_SDIO_SPEC_VER_1_00 0
#define SDIO_SDIO_SPEC_VER_1_10 1
#define SDIO_SDIO_SPEC_VER_1_20 2
#define SDIO_SDIO_SPEC_VER_2_00 3
#define SDIO_SDIO_SPEC_VER_3_00 4

#define SDIO_CCCR_SD_SPEC_REV 0x01

#define SDIO_SD_SPEC_VER_1_01 0
#define SDIO_SD_SPEC_VER_1_10 1
#define SDIO_SD_SPEC_VER_2_00 2
#define SDIO_SD_SPEC_VER_3_00 3

#define SDIO_CCCR_IO_ENABLE 0x02
#define SDIO_CCCR_IO_READY  0x03

#define SDIO_CCCR_INT_ENABLE  0x04
#define SDIO_CCCR_INT_PENDING 0x05

#define SDIO_CCCR_IO_ABORT 0x06

#define SDIO_CCCR_BUS_IF_CTRL 0x07

#define SDIO_BUS_WIDTH_MASK     (BIT01_T | BIT00_T)
#define SDIO_BUS_WIDTH_1BIT     0x00
#define SDIO_BUS_WIDTH_RESERVED BIT00_T
#define SDIO_BUS_WIDTH_4BIT     BIT01_T

#define SDIO_BUS_ASYNC_INT BIT05_T

#define SDIO_BUS_CD_DISABLE BIT07_T

#define SDIO_CCCR_CARD_CAPS 0x08

#define SDIO_CCCR_CARD_CAP_SDC  BIT00_T
#define SDIO_CCCR_CARD_CAP_SMB  BIT01_T
#define SDIO_CCCR_CARD_CAP_SRW  BIT02_T
#define SDIO_CCCR_CARD_CAP_SBS  BIT03_T
#define SDIO_CCCR_CARD_CAP_S4MI BIT04_T
#define SDIO_CCCR_CARD_CAP_E4MI BIT05_T
#define SDIO_CCCR_CARD_CAP_LSC  BIT06_T
#define SDIO_CCCR_CARD_CAP_4BLS BIT07_T

#define SDIO_CCCR_COMMON_CIS_POINTER 0x09

/* Following 4 regs are valid only if SBS is set */
#define SDIO_CCCR_BUS_SUSPEND 0x0c
#define SDIO_CCCR_FUNC_SELx   0x0d
#define SDIO_CCCR_FLAG_EXECx  0x0e
#define SDIO_CCCR_FLAG_READYx 0x0f

#define SDIO_CCCR_FN0_BLKSIZE 0x10

#define SDIO_CCCR_POWER_CTRL 0x12

#define SDIO_POWER_CTRL_SMPC BIT00_T
#define SDIO_POWER_CTRL_EMPC BIT01_T

#define SDIO_CCCR_SPEED 0x13

#define SDIO_SPEED_SHS             BIT00_T
#define SDIO_SPEED_BSS_SHIFT_INDEX 0x01
#define SDIO_SPEED_BSS_MASK        (7 << SDIO_SPEED_BSS_SHIFT_INDEX)
#define SDIO_SPEED_MODE_SDR12      (0 << SDIO_SPEED_BSS_SHIFT_INDEX)
#define SDIO_SPEED_MODE_SDR25      (1 << SDIO_SPEED_BSS_SHIFT_INDEX)
#define SDIO_SPEED_MODE_SDR50      (2 << SDIO_SPEED_BSS_SHIFT_INDEX)
#define SDIO_SPEED_MODE_SDR104     (3 << SDIO_SPEED_BSS_SHIFT_INDEX)
#define SDIO_SPEED_MODE_DDR50      (4 << SDIO_SPEED_BSS_SHIFT_INDEX)
#define SDIO_SPEED_EHS             BIT01_T

#define SDIO_CCCR_UHS   0x14
#define SDIO_UHS_SDR50  BIT00_T
#define SDIO_UHS_SDR104 BIT01_T
#define SDIO_UHS_DDR50  BIT02_T

#define SDIO_CCCR_DRV_STRENGTH            0x15
#define SDIO_SDTx_MASK                    0x07
#define SDIO_DRIVE_SDTA                   (BIT00_T)
#define SDIO_DRIVE_SDTC                   (BIT01_T)
#define SDIO_DRIVE_SDTD                   (BIT02_T)
#define SDIO_DRIVE_DTSx_MASK              (BIT00_T | BIT01_T)
#define SDIO_DRIVE_STRENGTH_SHIF_DTSxT    4
#define SDIO_DTSx_SET_DRV_STRENGTH_TYPE_B (0 << SDIO_DRIVE_STRENGTH_SHIF_DTSxT)
#define SDIO_DTSx_SET_DRV_STRENGTH_TYPE_A (1 << SDIO_DRIVE_STRENGTH_SHIF_DTSxT)
#define SDIO_DTSx_SET_DRV_STRENGTH_TYPE_C (2 << SDIO_DRIVE_STRENGTH_SHIF_DTSxT)
#define SDIO_DTSx_SET_DRV_STRENGTH_TYPE_D (3 << SDIO_DRIVE_STRENGTH_SHIF_DTSxT)
/*
 * Function Basic Registers (FBR)
 */

#define SDIO_FBR_BASE_ADDR(f) ((f)*0x100)

#define SDIO_FBR_STD_SDIO_FUNC_IF 0x00

#define SDIO_FBR_SUPPORTS_CSA BIT06_T
#define SDIO_FBR_ENABLE_CSA   BIT07_T

#define SDIO_FBR_EXT_STD_SDIO_FUNC_IF 0x01

#define SDIO_FBR_POWER 0x02

#define SDIO_FBR_SUPPORT_POWER_SELECTION BIT00_T
#define SDIO_FBR_ENABLE_POWER_SELECTION  BIT01_T

#define SDIO_FBR_CIS_POINT 0x09

#define SDIO_FBR_CSA_POINT 0x0C

#define SDIO_FBR_CSA_DATA_ACCESS 0x0F

#define SDIO_FBR_IO_BLKSIZE 0x10

extern unsigned char MS_SD_SET_IOS(struct sstar_mmc_priv *p_mmc_priv, struct msSt_SD_IOS *pstSD_IOS,
                                   unsigned char bRefresh);
extern BOOL_T        MS_SD_Card_Detect(unsigned char u8Slot);
// extern unsigned short MS_SD_Request(unsigned char u8Slot, struct msSt_SD_Req *pstReqest);
// extern unsigned short MS_SDIO_Request(unsigned char u8Slot, struct msSt_SD_Req *pstReq);
extern BOOL_T MS_SD_Check_SDSlot(U8_T u8Slot);
extern int    sd_deinit(void);
// close power,clock,timing,set removeflag
extern void sd_card_remove(struct sstar_mmc_priv *p_mmc_priv);
#endif // End of __SS_SDMMC_RTK_H
