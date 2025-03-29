/*
 * hal_sdmmc_regs_v5.h- Sigmastar
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
 * FileName hal_sdmmc_regs_v5.h
 *     @author jeremy.wang (2015/06/03)
 * Desc:
 *     This file is the header file for hal_card_regs.h
 *
 *     We add a new header file to describe the meaning positions of fcie5 registers
 *
 ***************************************************************************************************************/

#ifndef __HAL_SDMMC_REGS_V5_H
#define __HAL_SDMMC_REGS_V5_H

//-----------------------------------------------------------------------------------------------------------
// IP_FCIE or IP_SDIO Register Basic Address
//-----------------------------------------------------------------------------------------------------------
#define A_SD_REG_POS(IP)   GET_CARD_BANK(IP)
#define A_SD_PLL_POS(IP)   GET_PLL_BANK(IP)
#define A_SD_CFIFO_POS(IP) GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x20) // Always at FCIE5
#define A_SD_CIFD_POS(IP)  GET_CIFD_BANK(IP)

#define A_SD_CIFD_R_POS(IP) GET_CARD_REG_ADDR(A_SD_CIFD_POS(IP), 0x00)
#define A_SD_CIFD_W_POS(IP) GET_CARD_REG_ADDR(A_SD_CIFD_POS(IP), 0x20)

#define A_MIE_EVENT_REG(IP)      GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x00)
#define A_MIE_INT_ENABLE_REG(IP) GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x01)
#define A_MMA_PRI_REG_REG(IP)    GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x02)
#define A_DMA_ADDR_15_0_REG(IP)  GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x03)
#define A_DMA_ADDR_31_16_REG(IP) GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x04)
#define A_DMA_LEN_15_0_REG(IP)   GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x05)
#define A_DMA_LEN_31_16_REG(IP)  GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x06)
#define A_MIE_FUNC_CTL_REG(IP)   GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x07)
#define A_JOB_BLK_CNT_REG(IP)    GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x08)
#define A_BLK_SIZE_REG(IP)       GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x09)
#define A_CMD_RSP_SIZE_REG(IP)   GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x0A)
#define A_SD_MODE_REG(IP)        GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x0B)
#define A_SD_CTL_REG(IP)         GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x0C)
#define A_SD_STS_REG(IP)         GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x0D)
#define A_BOOT_MOD_REG(IP)       GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x0E)
#define A_DDR_MOD_REG(IP)        GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x0F)
#define A_DDR_TOGGLE_CNT_REG(IP) GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x10)
#define A_SDIO_MODE_REG(IP)      GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x11)
#define A_TEST_MODE_REG(IP)      GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x15)

#define A_WR_SBIT_TIMER_REG(IP)  GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x17)
#define A_RD_SBIT_TIMER_REG(IP)  GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x18)
#define A_DMA_ADDR_35_32_REG(IP) GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x19)

#define A_SDIO_DET_ON(IP) GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x2F)

#define A_CIFD_EVENT_REG(IP)  GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x30)
#define A_CIFD_INT_EN_REG(IP) GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x31)

#define A_BOOT_REG(IP) GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x37)

#define A_DBG_BUS0_REG(IP) GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x38)
#define A_DBG_BUS1_REG(IP) GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x39)
#define A_CLK_EN_REG(IP)   GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x3E)
#define A_FCIE_RST_REG(IP) GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x3F)

#define A_CFIFO_OFFSET(IP, OFFSET)  GET_CARD_REG_ADDR(A_SD_CFIFO_POS(IP), OFFSET)
#define A_CIFD_R_OFFSET(IP, OFFSET) GET_CARD_REG_ADDR(A_SD_CIFD_R_POS(IP), OFFSET)
#define A_CIFD_W_OFFSET(IP, OFFSET) GET_CARD_REG_ADDR(A_SD_CIFD_W_POS(IP), OFFSET)

#define A_PWR_SAVE_CTRL_REG(IP)            GET_CARD_REG_ADDR(GET_CARD_BANK(IP), 0x35)
#define A_SD_PWR_SAVE_FIFO_POS(IP)         GET_PWR_SAVE_BANK(IP)
#define A_PWR_SAVE_FIFO_OFFSET(IP, OFFSET) GET_CARD_REG_ADDR(A_SD_PWR_SAVE_FIFO_POS(IP), OFFSET)

#if defined(CONFIG_SUPPORT_SD30) || defined(CONFIG_SUPPORT_EMMC50)
//-----------------------------------------------------------------------------------------------------------
// SD_PLL Register Basic Address
//-----------------------------------------------------------------------------------------------------------

#define A_PLL_CLKPH_SKEW(IP)     GET_CARD_REG_ADDR(A_SD_PLL_POS(IP), 0x03)
#define A_PLL_EMMC_TEST(IP)      GET_CARD_REG_ADDR(A_SD_PLL_POS(IP), 0x1A)
#define A_PLL_ECO_EN(IP)         GET_CARD_REG_ADDR(A_SD_PLL_POS(IP), 0x1C)
#define A_PLL_EMMC_EN(IP)        GET_CARD_REG_ADDR(A_SD_PLL_POS(IP), 0x68)
#define A_PLL_SKEW_SUM(IP)       GET_CARD_REG_ADDR(A_SD_PLL_POS(IP), 0x69)
#define A_PLL_IO_BUS_WID(IP)     GET_CARD_REG_ADDR(A_SD_PLL_POS(IP), 0x6A)
#define A_PLL_DQS_PAGE_NO(IP)    GET_CARD_REG_ADDR(A_SD_PLL_POS(IP), 0x6B)
#define A_PLL_DQS_SUM(IP)        GET_CARD_REG_ADDR(A_SD_PLL_POS(IP), 0x6C)
#define A_PLL_DQS_IO_MODE(IP)    GET_CARD_REG_ADDR(A_SD_PLL_POS(IP), 0x6D)
#define A_PLL_RST_SUM(IP)        GET_CARD_REG_ADDR(A_SD_PLL_POS(IP), 0x6F)
#define A_PLL_AFIFO_SUM(IP)      GET_CARD_REG_ADDR(A_SD_PLL_POS(IP), 0x70)
#define A_PLL_TX_BPS_EN(IP)      GET_CARD_REG_ADDR(A_SD_PLL_POS(IP), 0x71)
#define A_PLL_RX_BPS_EN(IP)      GET_CARD_REG_ADDR(A_SD_PLL_POS(IP), 0x73)
#define A_PLL_ATOP_BYP_RX_EN(IP) GET_CARD_REG_ADDR(A_SD_PLL_POS(IP), 0x74)
#endif

//***********************************************************************************************************
//***********************************************************************************************************
//-----------------------------------------------------------------------------------------------------------
// Reg Static Init Setting
//-----------------------------------------------------------------------------------------------------------
#define V_MIE_PATH_INIT       0
#define V_MMA_PRI_INIT        (R_MIU_R_PRIORITY | R_MIU_W_PRIORITY | R_MIU_BUS_BURST8)
#define V_MIE_INT_EN_INIT     (R_DATA_END_IEN | R_CMD_END_IEN | R_SDIO_INT_IEN | R_BUSY_END_IEN)
#define V_MIE_EN_INIT_NO_SDIO (R_DATA_END_IEN | R_CMD_END_IEN | R_BUSY_END_IEN)
#define V_RSP_SIZE_INIT       0
#define V_CMD_SIZE_INIT       (5 << 8)
#define V_SD_CTL_INIT         0
#define V_SD_MODE_INIT        (R_CLK_EN) // Add
#define V_SDIO_MODE_INIT      (1 << 3)   // Low level trigger from Joe's email. Was 0 before.
#define V_DDR_MODE_INIT       0

#if defined(CONFIG_SUPPORT_SD30) || defined(CONFIG_SUPPORT_EMMC50)
#define V_PLL_SKEW_SUM_INIT       ((4 << 4) & R_TUNE_SHOT_OFFSET_MASK)
#define V_PLL_RX_BPS_EN_INIT      0x3F
#define V_PLL_ATOP_BYP_RX_EN_INIT (R_PLL_ATOP_BYP_RX_EN)
#endif

// Mask Range
//-----------------------------------------------------------------------------------------------------------
#define M_SD_ERRSTS   (R_DAT_RD_CERR | R_DAT_WR_CERR | R_DAT_WR_TOUT | R_CMD_NORSP | R_CMDRSP_CERR | R_DAT_RD_TOUT)
#define M_SD_MIEEVENT (R_DATA_END | R_CMD_END | R_ERR_STS | R_BUSY_END_INT | R_R2N_RDY_INT)
#define M_RST_STS     (R_RST_MIU_STS | R_RST_MIE_STS | R_RST_MCU_STS)

// Mask Reg Value
//-----------------------------------------------------------------------------------------------------------
#define M_REG_STSERR(IP)     (CARD_REG(A_SD_STS_REG(IP)) & M_SD_ERRSTS)
#define M_REG_SDMIEEvent(IP) (CARD_REG(A_MIE_EVENT_REG(IP)) & M_SD_MIEEVENT)
#define M_REG_GETDAT0(IP)    (CARD_REG(A_SD_STS_REG(IP)) & R_DAT0)
#define M_REG_IMISEL(IP)     (CARD_REG(A_BOOT_REG(IP)) & R_IMI_SEL)

//***********************************************************************************************************
//***********************************************************************************************************
//============================================
// MIE_EVENT: offset 0x00
//============================================
#define R_DATA_END     BIT00_T
#define R_CMD_END      BIT01_T
#define R_ERR_STS      BIT02_T
#define R_SDIO_INT     BIT03_T
#define R_BUSY_END_INT BIT04_T
#define R_R2N_RDY_INT  BIT05_T
#define R_CARD_CHANGE  BIT06_T
#define R_CARD2_CHANGE BIT07_T

//============================================
// MIE_INT_EN: offset 0x01
//============================================
#define R_DATA_END_IEN     BIT00_T
#define R_CMD_END_IEN      BIT01_T
#define R_ERR_STS_IEN      BIT02_T
#define R_SDIO_INT_IEN     BIT03_T
#define R_BUSY_END_IEN     BIT04_T
#define R_R2N_RDY_INT_IEN  BIT05_T
#define R_CARD_CHANGE_IEN  BIT06_T
#define R_CARD2_CHANGE_IEN BIT07_T

//============================================
// MMA_PRI_REG: offset 0x02
//============================================
#define R_MIU_R_PRIORITY BIT00_T
#define R_MIU_W_PRIORITY BIT01_T

#define R_MIU1_SELECT BIT02_T
#define R_MIU2_SELECT BIT03_T
#define R_MIU3_SELECT (BIT03_T | BIT02_T)

#define R_MIU_BUS_BURST2 BIT04_T
#define R_MIU_BUS_BURST4 BIT05_T
#define R_MIU_BUS_BURST8 (BIT05_T | BIT04_T)

//============================================
// MIE_FUNC_CTL: offset 0x07
//============================================
#define R_EMMC_EN   BIT00_T
#define R_SD_EN     BIT01_T
#define R_SDIO_MODE BIT02_T

//============================================
// MIE_FUNC_CTL: offset 0x08
//============================================
#define M_REG_SD_JOB_BLK_CNT_MASK (BIT13_T - 1)

//============================================
// SD_MODE: offset 0x0B
//============================================
#define R_CLK_EN          BIT00_T
#define R_BUS_WIDTH_4     BIT01_T
#define R_BUS_WIDTH_8     BIT02_T
#define R_DEST_R2N        BIT04_T
#define R_DATASYNC        BIT05_T
#define R_DMA_RD_CLK_STOP BIT07_T
#define R_DIS_WR_BUSY_CHK BIT08_T
#define R_STOP_BLK        BIT09_T

//============================================
// SD_CTL: offset 0x0C
//============================================
#define R_RSPR2_EN    BIT00_T
#define R_RSP_EN      BIT01_T
#define R_CMD_EN      BIT02_T
#define R_DTRX_EN     BIT03_T
#define R_JOB_DIR     BIT04_T
#define R_ADMA_EN     BIT05_T
#define R_JOB_START   BIT06_T
#define R_CHK_CMD     BIT07_T
#define R_BUSY_DET_ON BIT08_T
#define R_ERR_DET_ON  BIT09_T

//============================================
// SD_STS: offset 0x0D
//============================================
#define R_DAT_RD_CERR BIT00_T
#define R_DAT_WR_CERR BIT01_T
#define R_DAT_WR_TOUT BIT02_T
#define R_CMD_NORSP   BIT03_T
#define R_CMDRSP_CERR BIT04_T
#define R_DAT_RD_TOUT BIT05_T
#define R_CARD_BUSY   BIT06_T
#define R_DAT0        BIT08_T
#define R_DAT1        BIT09_T
#define R_DAT2        BIT10_T
#define R_DAT3        BIT11_T
#define R_DAT4        BIT12_T
#define R_DAT5        BIT13_T
#define R_DAT6        BIT14_T
#define R_DAT7        BIT15_T

//============================================
// BOOT_MOD:offset 0x0E
//============================================
#define R_BOOT_MODE BIT02_T

//============================================
// DDR_MOD: offset 0x0F
//============================================
#define R_PAD_IN_BYPASS  BIT00_T
#define R_PAD_IN_RDY_SEL BIT01_T
#define R_PRE_FULL_SEL0  BIT02_T
#define R_PRE_FULL_SEL1  BIT03_T
#define R_DDR_MACRO_EN   BIT07_T
#define R_DDR_EN         BIT08_T
#define R_PAD_CLK_SEL    BIT10_T
#define R_PAD_IN_SEL_IP  BIT11_T
#define R_DDR_MACRO32_EN BIT12_T
#define R_PAD_IN_SEL     BIT13_T
#define R_FALL_LATCH     BIT14_T
#define R_PAD_IN_MASK    BIT15_T
#define R_CIFD_MODE_MASK (R_PAD_IN_RDY_SEL | R_PRE_FULL_SEL0 | R_PRE_FULL_SEL1)
#define R_MACRO_MODE_MASK \
    (R_DDR_MACRO_EN | R_DDR_EN | R_PAD_CLK_SEL | R_DDR_MACRO32_EN | R_PAD_IN_SEL | R_FALL_LATCH | R_PAD_IN_MASK)

//============================================
// SDIO_MOD: offset 0x11
//============================================
#define R_SDIO_INT_MOD0      BIT00_T
#define R_SDIO_INT_MOD1      BIT01_T
#define R_SDIO_INT_MOD_SW_EN BIT02_T
#define R_SDIO_DET_INT_SRC   BIT03_T
#define R_SDIO_INT_TUNE0     BIT04_T
#define R_SDIO_INT_TUNE1     BIT05_T
#define R_SDIO_INT_TUNE2     BIT06_T
#define R_SDIO_INT_TUNE_CLR0 BIT07_T
#define R_SDIO_INT_TUNE_CLR1 BIT08_T
#define R_SDIO_INT_TUNE_CLR2 BIT09_T
#define R_SDIO_RDWAIT_EN     BIT11_T
#define R_SDIO_BLK_GAP_DIS   BIT12_T
#define R_SDIO_INT_STOP_DMA  BIT13_T
#define R_SDIO_INT_TUNE_SW   BIT14_T
#define R_SDIO_INT_ASYN_EN   BIT15_T

//============================================
// TEST_MOD: offset 0x15
//============================================
#define R_SDDR1         BIT00_T
#define R_SD_DEBUG_MOD0 BIT01_T
#define R_SD_DEBUG_MOD1 BIT02_T
#define R_SD_DEBUG_MOD2 BIT03_T
#define R_BIST_MODE     BIT04_T

//============================================
// WR_SBIT_TIMER: offset 0x17
//============================================
#define R_WR_SBIT_TIMER_EN BIT15_T

//============================================
// RD_SBIT_TIMER: offset 0x18
//============================================
#define R_RD_SBIT_TIMER_EN BIT15_T

//============================================
// SDIO_DET_ON: offset 0x2F
//============================================
#define R_SDIO_DET_ON     BIT00_T
#define R_SDIO_DET_ON_SEL BIT01_T

//============================================
// CIFD_EVENT: offset 0x30
//============================================
#define R_WBUF_FULL       BIT00_T
#define R_WBUF_EMPTY_TRIG BIT01_T
#define R_RBUF_FULL_TRIG  BIT02_T
#define R_RBUF_EMPTY      BIT03_T

//============================================
// CIFD_INT_EN: offset 0x31
//============================================
#define R_WBUF_FULL_IEN    BIT00_T
#define R_RBUF_EMPTY_IEN   BIT01_T
#define R_F_WBUF_FULL_INT  BIT08_T
#define R_F_RBUF_EMPTY_INT BIT09_T

//============================================
// CIFD_INT_EN: offset 0x35
//============================================
/* FCIE_PWR_SAVE_CTL 0x35 */
#define BIT_POWER_SAVE_MODE        (1 << 0)
#define BIT_SD_POWER_SAVE_RIU      (1 << 1)
#define BIT_POWER_SAVE_MODE_INT_EN (1 << 2)
#define BIT_SD_POWER_SAVE_RST      (1 << 3)
#define BIT_POWER_SAVE_INT_FORCE   (1 << 4)
#define BIT_RIU_SAVE_EVENT         (1 << 5)
#define BIT_RST_SAVE_EVENT         (1 << 6)
#define BIT_BAT_SAVE_EVENT         (1 << 7)
#define BIT_BAT_SD_POWER_SAVE_MASK (1 << 8)
#define BIT_RST_SD_POWER_SAVE_MASK (1 << 9)
#define BIT_POWER_SAVE_MODE_INT    (1 << 15)

#define PWR_BAT_CLASS (0x1 << 13) /* Battery lost class */
#define PWR_RST_CLASS (0x1 << 12) /* Reset Class */

/* Command Type */
#define PWR_CMD_WREG (0x0 << 9) /* Write data */
#define PWR_CMD_RDCP (0x1 << 9) /* Read and cmp data. If mismatch, HW retry */
#define PWR_CMD_WAIT (0x2 << 9) /* Wait idle, max. 128T */
#define PWR_CMD_WINT (0x3 << 9) /* Wait interrupt */
#define PWR_CMD_STOP (0x7 << 9) /* Stop */

/* RIU Bank */
#define PWR_CMD_BK0 (0x0 << 7)
#define PWR_CMD_BK1 (0x1 << 7)
#define PWR_CMD_BK2 (0x2 << 7)
#define PWR_CMD_BK3 (0x3 << 7)

//============================================
// BOOT_MODE:offset 0x37
//============================================
#define R_NAND_BOOT_EN        BIT00_T
#define R_BOOTSRAM_ACCESS_SEL BIT01_T
#define R_IMI_SEL             BIT02_T

//============================================
// CIFD_INT_EN: offset 0x39
//============================================
#define R_DEBUG_MOD0 BIT08_T
#define R_DEBUG_MOD1 BIT09_T
#define R_DEBUG_MOD2 BIT10_T
#define R_DEBUG_MOD3 BIT11_T

//============================================
// FCIE_RST:offset 0x3E
//============================================
#define R_FCIE_CLK_EN  BIT00_T
#define R_TEST_CLK     BIT01_T
#define R_TEST_MIU_STS BIT02_T
#define R_TEST_MIE_STS BIT03_T
#define R_TEST_MCU_STS BIT04_T
#define R_TEST_ECC_STS BIT05_T
#define R_MCG_DISABLE  BIT14_T

//============================================
// FCIE_RST:offset 0x3F
//============================================
#define R_FCIE_SOFT_RST BIT00_T
#define R_RST_MIU_STS   BIT01_T
#define R_RST_MIE_STS   BIT02_T
#define R_RST_MCU_STS   BIT03_T
#define R_RST_ECC_STS   BIT04_T

// SD PLL Used
//============================================
// SD PLL: CLKPH_SKEW: offset 0x03
//============================================
#define R_PLL_CLKPH_SKEW1_MASK (BIT03_T | BIT03_T | BIT03_T | BIT00_T)
#define R_PLL_CLKPH_SKEW2_MASK (BIT07_T | BIT06_T | BIT05_T | BIT04_T)
#define R_PLL_CLKPH_SKEW3_MASK (BIT11_T | BIT10_T | BIT09_T | BIT08_T)
#define R_PLL_CLKPH_SKEW4_MASK (BIT15_T | BIT14_T | BIT13_T | BIT12_T)

//============================================
// SD PLL: EMMC_TEST: offset 0x1A
//============================================
#define R_PLL_PAD_DRV         BIT00_T
#define R_PLL_G_RX_W_OEN_DOUT BIT03_T
#define R_PLL_G_RX_W_OEN_COUT BIT04_T
#define R_PLL_C2_EN           BIT10_T

//============================================
// SD PLL: ECO_EN: offset 0x1C
//============================================
#define R_PLL_32BIF_RX_ECO_EN BIT08_T
#define R_PLL_1X_SYN_ECO_EN   BIT09_T

//============================================
// SD PLL: EMMC_EN: offset 0x68
//============================================
#define R_PLL_EMMC_EN BIT00_T

//============================================
// SD PLL: SKEW_SUM: offset 0x69
//============================================
#define R_PLL_CLK_DIG_INV       BIT03_T
#define R_TUNE_SHOT_OFFSET_MASK (BIT07_T | BIT06_T | BIT05_T | BIT04_T)
#define R_CLK_SKEW_INV          BIT11_T

//============================================
// SD PLL: IO_BUS_WID: offset 0x6A
//============================================
#define R_PLL_IO_BUS_WID_MASK (BIT01_T | BIT00_T)

//============================================
// SD PLL: DQS_SUM: offset 0x6C
//============================================
#define R_PLL_DQS_MODE_1T      BIT01_T
#define R_PLL_SKEW_INV         BIT07_T
#define R_PLL_DQS_MODE_MASK    (BIT02_T | BIT01_T | BIT00_T)
#define R_PLL_DQS_DLY_SEL_MASK (BIT07_T | BIT06_T | BIT05_T | BIT04_T)
#define SKEW_INV_OFFSET        7

//============================================
// SD PLL: DDR_IO_MODE: offset 0x6D
//============================================
#define R_PLL_DDR_IO_MODE BIT00_T

//============================================
// SD PLL: RST_SUM: offset 0x6F
//============================================
#define R_PLL_MACRO_SW_RSTZ BIT00_T
#define R_PLL_DQS_CNT_RSTN  BIT01_T
#define R_PLL_OSP_SW_RSTZ   BIT02_T

//============================================
// SD PLL: AFIFO_SUM: offset 0x70
//============================================
#define R_PLL_SEL_FLASH_32BIF BIT08_T
#define R_PLL_RX_AFIFO_EN     BIT10_T
#define R_PLL_RSP_AFIFO_EN    BIT11_T

//============================================
// SD PLL: TX_BPS_EN: offset 0x71
//============================================
#define R_PLL_TX_BPS_EN_MASK (BIT05_T | BIT04_T | BIT03_T | BIT02_T | BIT01_T | BIT00_T)

//============================================
// SD PLL: RX_BPS_EN: offset 0x73
//============================================
#define R_PLL_RX_BPS_EN_MASK (BIT05_T | BIT04_T | BIT03_T | BIT02_T | BIT01_T | BIT00_T)

//============================================
// SD PLL: ATOP_BYP_RX_EN: offset 0x74
//============================================
#define R_PLL_ATOP_BYP_RX_EN BIT15_T

#endif // End of __HAL_SDMMC_REGS_V5_H
