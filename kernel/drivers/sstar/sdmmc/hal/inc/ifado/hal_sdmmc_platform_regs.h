/*
 * hal_sdmmc_platform_regs.h- Sigmastar
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

#define A_RIU_BASE (0x1F000000)
//***********************************************************************************************************
// Platform Register Basic Address
//------------------------------------------------------------------------------------
#define A_PADTOP_BANK   GET_CARD_REG_ADDR(A_RIU_BASE, 0x81E00) // 103Ch
#define A_PADGPIO_BANK  GET_CARD_REG_ADDR(A_RIU_BASE, 0x81F00) // Bank: 0x103E
#define A_PADGPIO2_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0x88200) // Bank: 0x1104
#define A_CHIPTOP_BANK  GET_CARD_REG_ADDR(A_RIU_BASE, 0x80F00) // 101Eh
//#define A_GPI_INT_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0x81E80)//103Dh
#define A_PM_SLEEP_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0x00700) // 0Eh
#define A_PM_GPIO_BANK  GET_CARD_REG_ADDR(A_RIU_BASE, 0x00780) // 0Fh

//#define A_MCM_SC_GP_BANK    GET_CARD_REG_ADDR(A_RIU_BASE, 0x89900)//1132h
#define A_CLKGEN_BANK     GET_CARD_REG_ADDR(A_RIU_BASE, 0x81C00) // 1038h
#define A_CLKGEN2_BANK    GET_CARD_REG_ADDR(A_RIU_BASE, 0x81F80) // 103Fh
#define A_SC_GP_CTRL_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0x89980) // 1133h
#define A_SDPLL_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0C80) // Bank: 0x1419
#define A_EMMCPLL_BANK    GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0D00) // 0x141A

#define A_PM_PAD_TOP_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0x01F80) // 3Fh
#define A_MIU_BANK        GET_CARD_REG_ADDR(A_RIU_BASE, 0x80900) // 1012h

//------------------------------------------------------------------------------------
// Clock Reg Setting
//---------------------------------------------------------------------------------
#define R_CKG_SD_VALUE_IP_0 (BIT07_T)
#define R_CKG_SD_VALUE_IP_1 (BIT06_T)
#define R_CKG_SD_VALUE_IP_2 (BIT05_T)

#define REG_CLK_IP_SD   (0x43)
#define REG_CLK_IP_SDIO (0x45)
#define REG_CLK_IP_FCIE (0x4b)

#define A_CLK_SOURCE_IP0_REG GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SD)
#define A_CLK_SOURCE_IP1_REG GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_SDIO)
#define A_CLK_SOURCE_IP2_REG GET_CARD_REG_ADDR(A_CLKGEN_BANK, REG_CLK_IP_FCIE)
