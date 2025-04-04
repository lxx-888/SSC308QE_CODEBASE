/*
 * eMMC_platform_config.c- Sigmastar
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

#include "eMMC_linux.h"
#include "eMMC.h"
#include "eMMC_hal.h"
#include "eMMC_platform.h"
#include "eMMC_err_codes.h"
#include "gpio.h"
#include "drv_gpio.h"

#define REG_BANK_FCIE0_IP_0 GET_REG_ADDR(RIU_BASE, 0x20980) // Bank: 0x1413
#define REG_BANK_FCIE1_IP_0 GET_REG_ADDR(RIU_BASE, 0x20A00) // Bank: 0x1414
#define REG_BANK_FCIE2_IP_0 GET_REG_ADDR(RIU_BASE, 0x20A80) // Bank: 0x1415

#define REG_BANK_FCIE0_IP_1 GET_REG_ADDR(RIU_PM_BASE, 0x2100) // Bank: 0x42
#define REG_BANK_FCIE1_IP_1 GET_REG_ADDR(RIU_PM_BASE, 0x2180) // Bank: 0x43
#define REG_BANK_FCIE2_IP_1 GET_REG_ADDR(RIU_PM_BASE, 0x2200) // Bank: 0x44

#define REG_BANK_FCIE0_IP_2 GET_REG_ADDR(RIU_BASE, 0x20B00) // Bank: 0x1416
#define REG_BANK_FCIE1_IP_2 GET_REG_ADDR(RIU_BASE, 0x20B80) // Bank: 0x1417
#define REG_BANK_FCIE2_IP_2 GET_REG_ADDR(RIU_BASE, 0x20C00) // Bank: 0x1418

#define REG_BANK_FCIE0_IP_3 GET_REG_ADDR(RIU_BASE, 0x20D00) // Bank: 0x141A
#define REG_BANK_FCIE1_IP_3 GET_REG_ADDR(RIU_BASE, 0x20D80) // Bank: 0x141B
#define REG_BANK_FCIE2_IP_3 GET_REG_ADDR(RIU_BASE, 0x20E00) // Bank: 0x141C

#define REG_BANK_SDPLL GET_REG_ADDR(RIU_BASE, 0x20C80) // Bank: 0x1419

//---------------------------------------------------------------------------------
#define REG_CLK_SOURCE_IP0 GET_REG_ADDR(CLKGEN0_BASE, 0x43)
#define REG_CLK_SOURCE_IP1 GET_REG_ADDR(PM_CLKGEN_BANK, 0x24)
#define REG_CLK_SOURCE_IP2 GET_REG_ADDR(CLKGEN2_BASE, 0x24)
#define REG_CLK_SOURCE_IP3 GET_REG_ADDR(CLKGEN2_BASE, 0x25)

//--------------------------------clock gen------------------------------------
#define REG_BANK_SC_GP_CTRL (0x1133 - 0x1000) * 0x80 // sc_gp_ctrl BK:x1133 reg block
#define SC_GP_CTRL_BASE     GET_REG_ADDR(RIU_BASE, REG_BANK_SC_GP_CTRL)
#define reg_sc_gp_ctrl      GET_REG_ADDR(SC_GP_CTRL_BASE, 0x25)

#define BIT_CKG_SD_VALUE_IP_0 (BIT7)
#define BIT_CKG_SD_VALUE_IP_1 (BIT5)
#define BIT_CKG_SD_VALUE_IP_2 (BIT4)
#define BIT_CKG_SD_VALUE_IP_3 (BIT5)

//---------------------------------------------------------------------------------
#define CLK1_48M  48000000
#define CLK1_43M2 43200000
#define CLK1_40M  40000000
#define CLK1_36M  36000000
#define CLK1_32M  32000000
#define CLK1_20M  20000000
#define CLK1_12M  12000000
#define CLK1_300K 300000
#define CLK1_0    0

extern U32 gu32_clk_driving[EMMC_NUM_TOTAL];
extern U32 gu32_cmd_driving[EMMC_NUM_TOTAL];
extern U32 gu32_data_driving[EMMC_NUM_TOTAL];

volatile U32 EMMC_GET_REG_BANK(U8 eIP, U8 u8Bank)
{
    U32 pIPBANKArr[EMMC_NUM_TOTAL][3] = {
        {(REG_BANK_FCIE0_IP_0), (REG_BANK_FCIE1_IP_0), (REG_BANK_FCIE2_IP_0)},
        {(REG_BANK_FCIE0_IP_1), (REG_BANK_FCIE1_IP_1), (REG_BANK_FCIE2_IP_1)},
        {(REG_BANK_FCIE0_IP_2), (REG_BANK_FCIE1_IP_2), (REG_BANK_FCIE2_IP_2)},
        {(REG_BANK_FCIE0_IP_3), (REG_BANK_FCIE1_IP_3), (REG_BANK_FCIE2_IP_3)},
    };

    return pIPBANKArr[eIP][u8Bank];
}

volatile U32 EMMC_GET_CLK_REG(U8 eIP)
{
    U32 IPClkRegArr[EMMC_NUM_TOTAL] = {REG_CLK_SOURCE_IP0, REG_CLK_SOURCE_IP1, REG_CLK_SOURCE_IP2, REG_CLK_SOURCE_IP3};
    return IPClkRegArr[eIP];
}

volatile U32 EMMC_GET_PLL_REG(U8 eIP)
{
    U32 IPPLLRegArr[EMMC_NUM_TOTAL] = {REG_BANK_SDPLL, REG_BANK_SDPLL, REG_BANK_SDPLL, REG_BANK_SDPLL};
    return IPPLLRegArr[eIP];
}

volatile U16 EMMC_GET_BOOT_CLK(U8 eIP)
{
    U32 IPClkRegArr[EMMC_NUM_TOTAL] = {BIT_CKG_SD_VALUE_IP_0, BIT_CKG_SD_VALUE_IP_1, BIT_CKG_SD_VALUE_IP_2,
                                       BIT_CKG_SD_VALUE_IP_3};
    return IPClkRegArr[eIP];
}

volatile U16 IP_BIT_FUNC_ENABLE(U8 eIP)
{
    U16 IPFuncEnableArr[EMMC_NUM_TOTAL] = {BIT_SDIO_MOD, BIT_SDIO_MOD, BIT_SDIO_MOD, BIT_SDIO_MOD};
    return IPFuncEnableArr[eIP];
}

U16 eMMC_Find_Clock_Reg(eMMC_IP_EmType emmc_ip, U32 u32_Clk)
{
    U8  u8LV                          = 0;
    U16 u16_ClkParam                  = BIT_FCIE_CLK_300K;
    U32 u32RealClk                    = 0;
    U32 u32ClkArr[EMMC_NUM_TOTAL][16] = {{CLK1_48M, CLK1_43M2, CLK1_40M, CLK1_36M, CLK1_32M, CLK1_20M, CLK1_12M,
                                          CLK1_300K, CLK1_0, CLK1_0, CLK1_0, CLK1_0, CLK1_0, CLK1_0, CLK1_0, CLK1_0},
                                         {CLK1_48M, CLK1_43M2, CLK1_40M, CLK1_36M, CLK1_32M, CLK1_20M, CLK1_12M,
                                          CLK1_300K, CLK1_0, CLK1_0, CLK1_0, CLK1_0, CLK1_0, CLK1_0, CLK1_0, CLK1_0},
                                         {CLK1_48M, CLK1_43M2, CLK1_40M, CLK1_36M, CLK1_32M, CLK1_20M, CLK1_12M,
                                          CLK1_300K, CLK1_0, CLK1_0, CLK1_0, CLK1_0, CLK1_0, CLK1_0, CLK1_0, CLK1_0},
                                         {CLK1_48M, CLK1_43M2, CLK1_40M, CLK1_36M, CLK1_32M, CLK1_20M, CLK1_12M,
                                          CLK1_300K, CLK1_0, CLK1_0, CLK1_0, CLK1_0, CLK1_0, CLK1_0, CLK1_0, CLK1_0}};

    for (; u8LV < 16; u8LV++)
    {
        if ((u32_Clk >= u32ClkArr[emmc_ip][u8LV]) || (u8LV == 15) || (u32ClkArr[emmc_ip][u8LV + 1] == 0))
        {
            u32RealClk = u32ClkArr[emmc_ip][u8LV];
            break;
        }
    }

    switch (u32RealClk)
    {
        case CLK1_0:
        case CLK1_300K:
            u16_ClkParam = BIT_FCIE_CLK_300K;
            break;
        case CLK1_12M:
            u16_ClkParam = BIT_FCIE_CLK_12M;
            break;
        case CLK1_20M:
            u16_ClkParam = BIT_FCIE_CLK_20M;
            break;
        case CLK1_32M:
            u16_ClkParam = BIT_FCIE_CLK_32M;
            break;
        case CLK1_36M:
            u16_ClkParam = BIT_FCIE_CLK_36M;
            break;
        case CLK1_40M:
            u16_ClkParam = BIT_FCIE_CLK_40M;
            break;
        case CLK1_43M2:
            u16_ClkParam = BIT_FCIE_CLK_43_2M;
            break;
        case CLK1_48M:
            u16_ClkParam = BIT_FCIE_CLK_48M;
            break;
    }

    return u16_ClkParam;
}

U32 _eMMC_pll_setting(eMMC_IP_EmType emmc_ip, U16 u16_ClkParam)
{
    return eMMC_ST_SUCCESS;
}

U32 eMMC_clock_setting(eMMC_IP_EmType emmc_ip, U16 u16_ClkParam)
{
    eMMC_PlatformResetPre();

    switch (u16_ClkParam)
    {
        case BIT_FCIE_CLK_300K:
            g_eMMCDrv[emmc_ip].u32_ClkKHz = 300;
            break;
        case BIT_CLK_XTAL_12M:
            g_eMMCDrv[emmc_ip].u32_ClkKHz = 12000;
            break;
        case BIT_FCIE_CLK_20M:
            g_eMMCDrv[emmc_ip].u32_ClkKHz = 20000;
            break;
        case BIT_FCIE_CLK_32M:
            g_eMMCDrv[emmc_ip].u32_ClkKHz = 32000;
            break;
        case BIT_FCIE_CLK_36M:
            g_eMMCDrv[emmc_ip].u32_ClkKHz = 36000;
            break;
        case BIT_FCIE_CLK_40M:
            g_eMMCDrv[emmc_ip].u32_ClkKHz = 40000;
            break;
        case BIT_FCIE_CLK_43_2M:
            g_eMMCDrv[emmc_ip].u32_ClkKHz = 43200;
            break;
        case BIT_FCIE_CLK_48M:
            g_eMMCDrv[emmc_ip].u32_ClkKHz = 48000;
            break;
        default:
            eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "eMMC Err: %Xh\n", eMMC_ST_ERR_INVALID_PARAM);
            return eMMC_ST_ERR_INVALID_PARAM;
    }

    if (emmc_ip == IP_EMMC0)
    {
        // clr gating
        REG_FCIE_CLRBIT(EMMC_GET_CLK_REG(emmc_ip), BIT0);
        // clr bit6 for glitch free select
        REG_FCIE_CLRBIT(EMMC_GET_CLK_REG(emmc_ip), BIT_FCIE_CLK_SRC_SEL);
        // clr sel
        REG_FCIE_CLRBIT(EMMC_GET_CLK_REG(emmc_ip), BIT_CLKGEN_FCIE_MASK);

        // eMMC_debug(0, 1, "clock %dk\n", g_eMMCDrv[emmc_ip].u32_ClkKHz);
        REG_FCIE_SETBIT(EMMC_GET_CLK_REG(emmc_ip), u16_ClkParam << 2);

        REG_FCIE_SETBIT(EMMC_GET_CLK_REG(emmc_ip), BIT_FCIE_CLK_SRC_SEL); // glitch free
        REG_FCIE_SETBIT(EMMC_GET_CLK_REG(emmc_ip), BIT7);                 // select sd_clk
        REG_FCIE_SETBIT(
            reg_sc_gp_ctrl,
            EMMC_GET_BOOT_CLK(emmc_ip)); // reg_ckg_sd(BK:x1133_x25) [B3]SDIO30 [B7]SD30  -> 0:clk_boot 1:clk_sd
    }
    else if (emmc_ip == IP_EMMC1)
    {
        // clr gating
        REG_FCIE_CLRBIT(EMMC_GET_CLK_REG(emmc_ip), BIT11);
        // clr bit15 for glitch free select
        REG_FCIE_CLRBIT(EMMC_GET_CLK_REG(emmc_ip), BIT15); // BK:0x0e(pm reg_ckg_sd)[B14] [0:1]/[sd_clk]

        // clr sel
        REG_FCIE_CLRBIT(EMMC_GET_CLK_REG(emmc_ip), BIT14 | BIT13);

        switch (u16_ClkParam)
        {
            case BIT_FCIE_CLK_48M: // 48000KHz-real is 49.15MHz
                break;
            case BIT_FCIE_CLK_32M:                                 // 32000KHz-real is 32.768MHz
                REG_FCIE_SETBIT(EMMC_GET_CLK_REG(emmc_ip), BIT13); // 1
                break;
            case BIT_FCIE_CLK_300K:                                // 300KHz
                REG_FCIE_SETBIT(EMMC_GET_CLK_REG(emmc_ip), BIT14); // 2
                break;
        }

        if (u16_ClkParam != BIT_CLK_XTAL_12M)
            REG_FCIE_SETBIT(EMMC_GET_CLK_REG(emmc_ip), BIT15); // select sd_clk
    }

    g_eMMCDrv[emmc_ip].u16_ClkRegVal = u16_ClkParam;

    eMMC_PlatformResetPost(emmc_ip);

    return eMMC_ST_SUCCESS;
}

void _eMMC_set_bustiming(eMMC_IP_EmType emmc_ip, U32 u32_FCIE_IF_Type)
{
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "eMMC%d >> [_eMMC_set_bustiming] %s mode. <<\r\n", emmc_ip,
               (u32_FCIE_IF_Type == FCIE_eMMC_BYPASS)              ? "BYPASS"
               : (u32_FCIE_IF_Type == FCIE_eMMC_SDR)               ? "SDR 8-bit macro"
               : (u32_FCIE_IF_Type == FCIE_MODE_8BITS_MACRO_DDR52) ? "DDR"
               : (u32_FCIE_IF_Type == FCIE_eMMC_HS200)             ? "HS200"
               : (u32_FCIE_IF_Type == FCIE_eMMC_HS400)             ? "HS400"
               : (u32_FCIE_IF_Type == FCIE_eMMC_5_1_AFIFO)         ? "HS400 5.1"
                                                                   : "Unknow");
#if 0
    REG_FCIE(reg_emmcpll_0x6a(emmc_ip)) = (g_eMMCDrv[emmc_ip].u8_BUS_WIDTH >> 1);
    REG_FCIE(reg_emmcpll_0x73(emmc_ip)) = V_PLL_RX_BPS_EN_INIT;
    REG_FCIE(reg_emmcpll_0x74(emmc_ip)) = V_PLL_ATOP_BYP_RX_EN_INIT;
    // REG_FCIE(FCIE_DDR_MODE(emmc_ip)) = 0x0;
#endif
    //  fcie
    REG_FCIE_CLRBIT(FCIE_DDR_MODE(emmc_ip), BIT_BYPASS_MODE_MASK | BIT_CIFD_MODE_MASK | BIT_MACRO_MODE_MASK);

    switch (u32_FCIE_IF_Type)
    {
        case FCIE_eMMC_BYPASS:
            REG_FCIE_SETBIT(FCIE_DDR_MODE(emmc_ip), BIT_BYPASS_MODE_MASK);
            break;
        case FCIE_eMMC_SDR:
            // fcie
            REG_FCIE_SETBIT(FCIE_DDR_MODE(emmc_ip), BIT_PAD_IN_SEL_SD | BIT_FALL_LATCH);
            g_eMMCDrv[emmc_ip].u32_DrvFlag |= DRV_FLAG_SPEED_HIGH;
            break;

        default:
            eMMC_debug(1, 1, "eMMC%d Err: wrong parameter for switch pad func\n", emmc_ip);
    }
}

U32 eMMC_pads_switch(eMMC_IP_EmType emmc_ip, U8 u8_PAD, U32 u32_FCIE_IF_Type)
{
    uintptr_t ptr_reg  = 0;
    U16       u16_mask = 0, u16_4bitmode = 0, u16_8bitmode = 0;

    if (emmc_ip == IP_EMMC0)
    {
        REG_FCIE_CLRBIT(reg_all_pad_in, BIT_ALL_PAD_IN);
        if ((u8_PAD == 0) || (u8_PAD == 2))
        {
            ptr_reg      = GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x61);
            u16_mask     = BIT12 | BIT11 | BIT10 | BIT9 | BIT8 | BIT5 | BIT4 | BIT1 | BIT0;
            u16_4bitmode = BIT10;
        }
        else if (u8_PAD == 1)
        {
            ptr_reg      = GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x61);
            u16_mask     = BIT12 | BIT11 | BIT10 | BIT9 | BIT8 | BIT4 | BIT5 | BIT1 | BIT0;
            u16_4bitmode = BIT12;
        }
    }
    else if (emmc_ip == IP_EMMC1)
    {
        REG_FCIE_CLRBIT(GET_REG_ADDR(PM_PADTOP_BANK, 0x60), BIT0);
        if (u8_PAD == 0)
        {
            ptr_reg      = GET_REG_ADDR(PM_PADTOP_BANK, 0x52);
            u16_mask     = BIT4 | BIT5 | BIT8 | BIT9 | BIT13 | BIT12;
            u16_4bitmode = BIT8;
        }
        else if (u8_PAD == 1)
        {
            ptr_reg      = GET_REG_ADDR(PM_PADTOP_BANK, 0x52);
            u16_mask     = BIT4 | BIT5 | BIT8 | BIT9 | BIT13 | BIT12;
            u16_4bitmode = BIT9;
        }
        else if (u8_PAD == 2)
        {
            ptr_reg      = GET_REG_ADDR(PM_PADTOP_BANK, 0x52);
            u16_mask     = BIT4 | BIT5 | BIT8 | BIT9 | BIT13 | BIT12;
            u16_4bitmode = BIT9 | BIT8;
        }
    }

    REG_FCIE_CLRBIT(ptr_reg, u16_mask);
    switch (g_eMMCDrv[emmc_ip].u16_of_buswidth)
    {
        case 1:
            REG_FCIE_SETBIT(ptr_reg, u16_4bitmode);
            break;
        case 4:
            REG_FCIE_SETBIT(ptr_reg, u16_4bitmode);
            break;
        case 8:
            if (u16_8bitmode)
                REG_FCIE_SETBIT(ptr_reg, u16_8bitmode);
            else
            {
                pr_err(">> [emmc_%d] don't support 8X mode, using 4X mode\n", emmc_ip);
                REG_FCIE_SETBIT(ptr_reg, u16_4bitmode);
            }
            break;
        default:
            pr_err(">> [emmc] Err: wrong buswidth config: %u!\n", g_eMMCDrv[emmc_ip].u16_of_buswidth);
            REG_FCIE_SETBIT(ptr_reg, u16_8bitmode);
            break;
    };

    g_eMMCDrv[emmc_ip].u8_PadType = u32_FCIE_IF_Type;
    _eMMC_set_bustiming(emmc_ip, u32_FCIE_IF_Type);

    return eMMC_ST_SUCCESS;
}

void eMMC_Read_Timeout_Set(eMMC_IP_EmType emmc_ip)
{
    U8 u8Pad = gu8_emmcPADOrderSlot[emmc_ip];

    if (emmc_ip == IP_EMMC0)
    {
        if ((u8Pad == 0) || (u8Pad == 2))
        {
            REG_FCIE_SETBIT(GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x3a), BIT2 | BIT3 | BIT4 | BIT5);
            REG_FCIE_CLRBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0x6D), BIT2);
            REG_FCIE_SETBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0x6D), BIT1 | BIT3);
            REG_FCIE_CLRBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0x6E), BIT2);
            REG_FCIE_SETBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0x6E), BIT1 | BIT3);
            REG_FCIE_CLRBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0x6F), BIT2);
            REG_FCIE_SETBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0x6F), BIT1 | BIT3);
            REG_FCIE_CLRBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0x70), BIT2);
            REG_FCIE_SETBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0x70), BIT1 | BIT3);
        }
        else if (u8Pad == 1)
        {
            REG_FCIE_SETBIT(GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x3a), BIT2 | BIT3 | BIT4 | BIT5);
            REG_FCIE_CLRBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0x9), BIT2);
            REG_FCIE_SETBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0x9), BIT1 | BIT3);
            REG_FCIE_CLRBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0xA), BIT2);
            REG_FCIE_SETBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0xA), BIT1 | BIT3);
            REG_FCIE_CLRBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0xD), BIT2);
            REG_FCIE_SETBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0xD), BIT1 | BIT3);
            REG_FCIE_CLRBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0xE), BIT2);
            REG_FCIE_SETBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0xE), BIT1 | BIT3);
        }
    }
    else if (emmc_ip == IP_EMMC1)
    {
        if (u8Pad == 0)
        {
            REG_FCIE_CLRBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x30), BIT2);
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x30), BIT1 | BIT3);
            REG_FCIE_CLRBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x31), BIT2);
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x31), BIT1 | BIT3);
            REG_FCIE_CLRBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x32), BIT2);
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x32), BIT1 | BIT3);
            REG_FCIE_CLRBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x33), BIT2);
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x33), BIT1 | BIT3);
        }
        else if (u8Pad == 1)
        {
            REG_FCIE_CLRBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x19), BIT2);
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x19), BIT1 | BIT3);
            REG_FCIE_CLRBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x1A), BIT2);
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x1A), BIT1 | BIT3);
            REG_FCIE_CLRBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x26), BIT2);
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x26), BIT1 | BIT3);
            REG_FCIE_CLRBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x25), BIT2);
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x25), BIT1 | BIT3);
        }
        else if (u8Pad == 2)
        {
            REG_FCIE_CLRBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x15), BIT2);
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x15), BIT1 | BIT3);
            REG_FCIE_CLRBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x16), BIT2);
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x16), BIT1 | BIT3);
            REG_FCIE_CLRBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x0E), BIT2);
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x0E), BIT1 | BIT3);
            REG_FCIE_CLRBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x0F), BIT2);
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x0F), BIT1 | BIT3);
        }
    }
}

void eMMC_Read_Timeout_Clear(eMMC_IP_EmType emmc_ip)
{
    U8 u8Pad = gu8_emmcPADOrderSlot[emmc_ip];

    if (emmc_ip == IP_EMMC0)
    {
        if ((u8Pad == 0) || (u8Pad == 2))
        {
            REG_FCIE_SETBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0x6D), BIT2);
            REG_FCIE_CLRBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0x6D), BIT1 | BIT3);
            REG_FCIE_SETBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0x6E), BIT2);
            REG_FCIE_CLRBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0x6E), BIT1 | BIT3);
            REG_FCIE_SETBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0x6F), BIT2);
            REG_FCIE_CLRBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0x6F), BIT1 | BIT3);
            REG_FCIE_SETBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0x70), BIT2);
            REG_FCIE_CLRBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0x70), BIT1 | BIT3);
            REG_FCIE_CLRBIT(GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x3a), BIT2 | BIT3 | BIT4 | BIT5);
        }
        else if (u8Pad == 1)
        {
            REG_FCIE_SETBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0x9), BIT2);
            REG_FCIE_CLRBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0x9), BIT1 | BIT3);
            REG_FCIE_SETBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0xA), BIT2);
            REG_FCIE_CLRBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0xA), BIT1 | BIT3);
            REG_FCIE_SETBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0xD), BIT2);
            REG_FCIE_CLRBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0xD), BIT1 | BIT3);
            REG_FCIE_SETBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0xE), BIT2);
            REG_FCIE_CLRBIT(GET_REG_ADDR(REG_BANK_PADGPIO, 0xE), BIT1 | BIT3);
            REG_FCIE_CLRBIT(GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x3a), BIT2 | BIT3 | BIT4 | BIT5);
        }
    }
    else if (emmc_ip == IP_EMMC1)
    {
        if (u8Pad == 0)
        {
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x30), BIT2);
            REG_FCIE_CLRBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x30), BIT1 | BIT3);
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x31), BIT2);
            REG_FCIE_CLRBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x31), BIT1 | BIT3);
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x32), BIT2);
            REG_FCIE_CLRBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x32), BIT1 | BIT3);
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x33), BIT2);
            REG_FCIE_CLRBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x33), BIT1 | BIT3);
        }
        else if (u8Pad == 1)
        {
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x19), BIT2);
            REG_FCIE_CLRBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x19), BIT1 | BIT3);
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x1A), BIT2);
            REG_FCIE_CLRBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x1A), BIT1 | BIT3);
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x26), BIT2);
            REG_FCIE_CLRBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x26), BIT1 | BIT3);
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x25), BIT2);
            REG_FCIE_CLRBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x25), BIT1 | BIT3);
        }
        else if (u8Pad == 2)
        {
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x15), BIT2);
            REG_FCIE_CLRBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x15), BIT1 | BIT3);
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x16), BIT2);
            REG_FCIE_CLRBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x16), BIT1 | BIT3);
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x0E), BIT2);
            REG_FCIE_CLRBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x0E), BIT1 | BIT3);
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x0F), BIT2);
            REG_FCIE_CLRBIT(GET_REG_ADDR(PM_PADGPIO_BANK, 0x0F), BIT1 | BIT3);
        }
    }
}

void eMMC_Write_Timeout_Set(eMMC_IP_EmType emmc_ip) {}

void eMMC_Write_Timeout_Clear(eMMC_IP_EmType emmc_ip) {}

void eMMC_driving_control(eMMC_IP_EmType emmc_ip, U8 u8_PAD, EMMCPinDrv_T st_EmmcPindrv)
{
#if 0 // for fpga test
    U32 u32_clkpin, u32_cmdpin, u32_datapin[4] = {0}, i;

    if (emmc_ip == IP_EMMC0)
    {
        if ((u8_PAD == 0) || (u8_PAD == 2))
        {
            u32_clkpin     = 107;
            u32_cmdpin     = 108;
            u32_datapin[0] = 109;
            u32_datapin[1] = 110;
            u32_datapin[2] = 111;
            u32_datapin[3] = 112;
        }
        else if (u8_PAD == 1)
        {
            u32_clkpin     = 11;
            u32_cmdpin     = 12;
            u32_datapin[0] = 10;
            u32_datapin[1] = 9;
            u32_datapin[2] = 14;
            u32_datapin[3] = 13;
        }
    }
    else if (emmc_ip == IP_EMMC1)
    {
        if (u8_PAD == 0)
        {
            u32_clkpin     = 46;
            u32_cmdpin     = 47;
            u32_datapin[0] = 48;
            u32_datapin[1] = 49;
            u32_datapin[2] = 50;
            u32_datapin[3] = 51;
        }
        else if (u8_PAD == 1)
        {
            u32_clkpin     = 27;
            u32_cmdpin     = 28;
            u32_datapin[0] = 26;
            u32_datapin[1] = 25;
            u32_datapin[2] = 38;
            u32_datapin[3] = 37;
        }
        else if (u8_PAD == 2)
        {
            u32_clkpin     = 19;
            u32_cmdpin     = 20;
            u32_datapin[0] = 21;
            u32_datapin[1] = 22;
            u32_datapin[2] = 14;
            u32_datapin[3] = 15;
        }
    }

    if (st_EmmcPindrv.eDrvClk >= 0)
        sstar_gpio_drv_set(u32_clkpin, st_EmmcPindrv.eDrvClk & 0x7);

    if (st_EmmcPindrv.eDrvCmd >= 0)
        sstar_gpio_drv_set(u32_cmdpin, st_EmmcPindrv.eDrvCmd & 0x7);

    if (st_EmmcPindrv.eDrvData >= 0)
        for (i = 0; i < g_eMMCDrv[emmc_ip].u16_of_buswidth; i++)
            sstar_gpio_drv_set(u32_datapin[i], st_EmmcPindrv.eDrvData & 0x7);
#endif
}

void eMMC_Init_Padpin(eMMC_IP_EmType emmc_ip, U8 u8_PAD)
{
    if (emmc_ip == IP_EMMC0)
    {
        if (u8_PAD == 0)
        {
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0x6D)) = 0x54; // PAD_EMMC_D0
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0x6E)) = 0x54; // PAD_EMMC_D1
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0x6F)) = 0x54; // PAD_EMMC_D2
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0x70)) = 0x54; // PAD_EMMC_D3

            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0x6B)) = 0x64; // PAD_EMMC_CLK
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0x6C)) = 0x54; // PAD_EMMC_CMD
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0x32)) = 0x54; // PAD_EMMC_RST
        }
        else if (u8_PAD == 1)
        {
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0xA)) = 0x54; // PAD_EMMC_D0
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0x9)) = 0x54; // PAD_EMMC_D1
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0xE)) = 0x54; // PAD_EMMC_D2
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0xD)) = 0x54; // PAD_EMMC_D3

            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0xB)) = 0x64; // PAD_EMMC_CLK
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0xC)) = 0x54; // PAD_EMMC_CMD
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0xF)) = 0x54; // PAD_EMMC_RST
        }
        else if (u8_PAD == 2)
        {
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0x6D)) = 0x54; // PAD_EMMC_D0
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0x6E)) = 0x54; // PAD_EMMC_D1
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0x6F)) = 0x54; // PAD_EMMC_D2
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0x70)) = 0x54; // PAD_EMMC_D3

            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0x6B)) = 0x64; // PAD_EMMC_CLK
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0x6C)) = 0x54; // PAD_EMMC_CMD
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0x5B)) = 0x54; // PAD_EMMC_RST
        }
    }
    else if (emmc_ip == IP_EMMC1)
    {
        if (u8_PAD == 0)
        {
            REG_FCIE(GET_REG_ADDR(PM_PADGPIO_BANK, 0x30)) = 0x54; // PAD_EMMC_D0
            REG_FCIE(GET_REG_ADDR(PM_PADGPIO_BANK, 0x31)) = 0x54; // PAD_EMMC_D1
            REG_FCIE(GET_REG_ADDR(PM_PADGPIO_BANK, 0x32)) = 0x54; // PAD_EMMC_D2
            REG_FCIE(GET_REG_ADDR(PM_PADGPIO_BANK, 0x33)) = 0x54; // PAD_EMMC_D3

            REG_FCIE(GET_REG_ADDR(PM_PADGPIO_BANK, 0x2E)) = 0x64; // PAD_EMMC_CLK
            REG_FCIE(GET_REG_ADDR(PM_PADGPIO_BANK, 0x2F)) = 0x54; // PAD_EMMC_CMD
            REG_FCIE(GET_REG_ADDR(PM_PADGPIO_BANK, 0x24)) = 0x54; // PAD_EMMC_RST
        }
        else if (u8_PAD == 1)
        {
            REG_FCIE(GET_REG_ADDR(PM_PADGPIO_BANK, 0x1A)) = 0x54; // PAD_EMMC_D0
            REG_FCIE(GET_REG_ADDR(PM_PADGPIO_BANK, 0x19)) = 0x54; // PAD_EMMC_D1
            REG_FCIE(GET_REG_ADDR(PM_PADGPIO_BANK, 0x25)) = 0x54; // PAD_EMMC_D2
            REG_FCIE(GET_REG_ADDR(PM_PADGPIO_BANK, 0x26)) = 0x54; // PAD_EMMC_D3

            REG_FCIE(GET_REG_ADDR(PM_PADGPIO_BANK, 0x1B)) = 0x64; // PAD_EMMC_CLK
            REG_FCIE(GET_REG_ADDR(PM_PADGPIO_BANK, 0x1C)) = 0x54; // PAD_EMMC_CMD
            REG_FCIE(GET_REG_ADDR(PM_PADGPIO_BANK, 0x4))  = 0x54; // PAD_EMMC_RST
        }
        else if (u8_PAD == 2)
        {
            REG_FCIE(GET_REG_ADDR(PM_PADGPIO_BANK, 0x15)) = 0x54; // PAD_EMMC_D0
            REG_FCIE(GET_REG_ADDR(PM_PADGPIO_BANK, 0x16)) = 0x54; // PAD_EMMC_D1
            REG_FCIE(GET_REG_ADDR(PM_PADGPIO_BANK, 0x0E)) = 0x54; // PAD_EMMC_D2
            REG_FCIE(GET_REG_ADDR(PM_PADGPIO_BANK, 0x0F)) = 0x54; // PAD_EMMC_D3

            REG_FCIE(GET_REG_ADDR(PM_PADGPIO_BANK, 0x13)) = 0x64; // PAD_EMMC_CLK
            REG_FCIE(GET_REG_ADDR(PM_PADGPIO_BANK, 0x14)) = 0x54; // PAD_EMMC_CMD
            REG_FCIE(GET_REG_ADDR(PM_PADGPIO_BANK, 0x0D)) = 0x54; // PAD_EMMC_RST
        }
    }
}

void eMMC_RST_L(eMMC_IP_EmType emmc_ip)
{
    U8 u8Pad = gu8_emmcPADOrderSlot[emmc_ip];
    if (emmc_ip == IP_EMMC0)
    {
        if ((u8Pad == 0) || (u8Pad == 2))
        {
            REG_FCIE_SETBIT(GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x61), BIT8);
        }
        else if (u8Pad == 1)
        {
        }
    }
    if (emmc_ip == IP_EMMC1)
    {
        if (u8Pad == 0)
        {
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADTOP_BANK, 0x52), BIT4);
        }
        else if (u8Pad == 1)
        {
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADTOP_BANK, 0x52), BIT5);
        }
        else if (u8Pad == 2)
        {
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADTOP_BANK, 0x52), BIT5 | BIT4);
        }
    }

    REG_FCIE_SETBIT(FCIE_BOOT_CONFIG(emmc_ip), BIT_EMMC_RSTZ_EN | BIT_EMMC_RSTZ);
    REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG(emmc_ip), BIT_EMMC_RSTZ);
}

void eMMC_RST_H(eMMC_IP_EmType emmc_ip)
{
    U8 u8Pad = gu8_emmcPADOrderSlot[emmc_ip];

    if (emmc_ip == IP_EMMC0)
    {
        if ((u8Pad == 0) || (u8Pad == 2))
        {
            REG_FCIE_SETBIT(GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x61), BIT8);
        }
        else if (u8Pad == 1)
        {
        }
    }
    if (emmc_ip == IP_EMMC1)
    {
        if (u8Pad == 0)
        {
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADTOP_BANK, 0x52), BIT4);
        }
        else if (u8Pad == 1)
        {
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADTOP_BANK, 0x52), BIT5);
        }
        else if (u8Pad == 2)
        {
            REG_FCIE_SETBIT(GET_REG_ADDR(PM_PADTOP_BANK, 0x52), BIT5 | BIT4);
        }
    }

    REG_FCIE_SETBIT(FCIE_BOOT_CONFIG(emmc_ip), BIT_EMMC_RSTZ_EN);
    REG_FCIE_SETBIT(FCIE_BOOT_CONFIG(emmc_ip), BIT_EMMC_RSTZ);
    REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG(emmc_ip), BIT_EMMC_RSTZ_EN);
}

dma_addr_t eMMC_Platform_Trans_Dma_Addr(eMMC_IP_EmType emmc_ip, dma_addr_t dma_DMAAddr, U32 *u32MiuSel)
{
    U32 miusel;
    if (u32MiuSel == NULL)
        u32MiuSel = &miusel;

    if (dma_DMAAddr >= 0X1000000000)
    {
        dma_DMAAddr -= 0x1000000000;
        *u32MiuSel = 0;
    }
    else
#ifdef MSTAR_MIU2_BUS_BASE
        if (dma_DMAAddr >= MSTAR_MIU2_BUS_BASE) // MIU2
    {
        dma_DMAAddr -= MSTAR_MIU2_BUS_BASE;
        *u32MiuSel = 2;
    }
    else
#endif
#ifdef MSTAR_MIU1_BUS_BASE
        if (dma_DMAAddr >= MSTAR_MIU1_BUS_BASE) // MIU1
    {
        dma_DMAAddr -= MSTAR_MIU1_BUS_BASE;
        *u32MiuSel = 1;
    }
    else // MIU0
#endif
        if (dma_DMAAddr >= IMI_PHYS) // IMI
    {
        dma_DMAAddr -= IMI_PHYS;
        *u32MiuSel = 1;
    }
    else // MIU0
    {
        dma_DMAAddr -= UL(CONFIG_MIU0_BUS_BASE); // MSTAR_MIU0_BUS_BASE;
        *u32MiuSel = 0;
    }

    return dma_DMAAddr;
}
