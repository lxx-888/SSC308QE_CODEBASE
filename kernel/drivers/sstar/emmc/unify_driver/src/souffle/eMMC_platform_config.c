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
#include "mstar_mci.h"

#define REG_BANK_FCIE0_IP_0 GET_REG_ADDR(RIU_BASE, 0x20980) // Bank: 0x1413
#define REG_BANK_FCIE1_IP_0 GET_REG_ADDR(RIU_BASE, 0x20A00) // Bank: 0x1414
#define REG_BANK_FCIE2_IP_0 GET_REG_ADDR(RIU_BASE, 0x20A80) // Bank: 0x1415

#define REG_BANK_FCIE0_IP_1 GET_REG_ADDR(RIU_BASE, 0x20800) // Bank: 0x1410
#define REG_BANK_FCIE1_IP_1 GET_REG_ADDR(RIU_BASE, 0x20880) // Bank: 0x1411
#define REG_BANK_FCIE2_IP_1 GET_REG_ADDR(RIU_BASE, 0x20900) // Bank: 0x1412

#define REG_BANK_FCIE0_IP_2 GET_REG_ADDR(RIU_BASE, 0x20B00) // Bank: 0x1416
#define REG_BANK_FCIE1_IP_2 GET_REG_ADDR(RIU_BASE, 0x20B80) // Bank: 0x1417
#define REG_BANK_FCIE2_IP_2 GET_REG_ADDR(RIU_BASE, 0x20C00) // Bank: 0x1418

#define REG_BANK_FCIE0_IP_3 GET_REG_ADDR(RIU_BASE, 0x20D00) // Bank: 0x141A
#define REG_BANK_FCIE1_IP_3 GET_REG_ADDR(RIU_BASE, 0x20D80) // Bank: 0x141B
#define REG_BANK_FCIE2_IP_3 GET_REG_ADDR(RIU_BASE, 0x20E00) // Bank: 0x141C

//---------------------------------------------------------------------------------
#define REG_CLK_SOURCE_IP0 GET_REG_ADDR(CLKGEN0_BASE, 0x43)
#define REG_CLK_SOURCE_IP1 GET_REG_ADDR(CLKGEN0_BASE, 0x4b)
#define REG_CLK_SOURCE_IP2 GET_REG_ADDR(CLKGEN2_BASE, 0x45)
#define REG_CLK_SOURCE_IP3 GET_REG_ADDR(CLKGEN2_BASE, 0x25)

//--------------------------------clock gen------------------------------------
#define REG_BANK_SC_GP_CTRL (0x1135 - 0x1000) * 0x80 // sc_gp_ctrl BK:x1135 reg block
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

extern U32            gu32_clk_driving[EMMC_NUM_TOTAL];
extern U32            gu32_cmd_driving[EMMC_NUM_TOTAL];
extern U32            gu32_data_driving[EMMC_NUM_TOTAL];
extern volatile U32   gu32_SupportEmmc50[EMMC_NUM_TOTAL];
extern struct clk_hw *CLK_fcie_1x_p[EMMC_NUM_TOTAL];
extern struct clk_hw *CLK_fcie_2x_p[EMMC_NUM_TOTAL];
#if defined(MSTAR_EMMC_CONFIG_OF)
extern struct clk_data *clkdata;
#endif

volatile U32 EMMC_GET_REG_BANK(U8 eIP, U8 u8Bank)
{
    U32 pIPBANKArr[EMMC_NUM_TOTAL][3] = {
        {(REG_BANK_FCIE0_IP_0), (REG_BANK_FCIE1_IP_0), (REG_BANK_FCIE2_IP_0)},
        {(REG_BANK_FCIE0_IP_1), (REG_BANK_FCIE1_IP_1), (REG_BANK_FCIE2_IP_1)},
        {(REG_BANK_FCIE0_IP_2), (REG_BANK_FCIE1_IP_2), (REG_BANK_FCIE2_IP_2)},
    };

    return pIPBANKArr[eIP][u8Bank];
}

volatile U32 EMMC_GET_CLK_REG(U8 eIP)
{
    U32 IPClkRegArr[EMMC_NUM_TOTAL] = {REG_CLK_SOURCE_IP0, REG_CLK_SOURCE_IP1, REG_CLK_SOURCE_IP2};
    return IPClkRegArr[eIP];
}

volatile U32 EMMC_GET_PLL_REG(U8 eIP)
{
    U32 IPPLLRegArr[EMMC_NUM_TOTAL] = {REG_BANK_FCIE0_IP_3, REG_BANK_FCIE0_IP_3, REG_BANK_FCIE2_IP_3};
    return IPPLLRegArr[eIP];
}

volatile U16 EMMC_GET_BOOT_CLK(U8 eIP)
{
    U32 IPClkRegArr[EMMC_NUM_TOTAL] = {BIT_CKG_SD_VALUE_IP_0, BIT_CKG_SD_VALUE_IP_1, BIT_CKG_SD_VALUE_IP_2};
    return IPClkRegArr[eIP];
}

volatile U16 IP_BIT_FUNC_ENABLE(U8 eIP)
{
    U16 IPFuncEnableArr[EMMC_NUM_TOTAL] = {BIT_SDIO_MOD, BIT_EMMC_EN, BIT_SDIO_MOD};
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

U32 eMMC_clock_setting(eMMC_IP_EmType emmc_ip, U16 u16_ClkParam)
{
    U32 u32RealClk = 0;
    eMMC_PlatformResetPre();

    if (g_eMMCDrv[emmc_ip].u16_ClkRegVal != u16_ClkParam)
    {
        switch (u16_ClkParam)
        {
            // emmc_pll clock
            case eMMC_PLL_CLK__20M:
            case BIT_FCIE_CLK_20M:
                g_eMMCDrv[emmc_ip].u32_ClkKHz = 20000;
                break;
            case eMMC_PLL_CLK__32M:
            case BIT_FCIE_CLK_32M:
                g_eMMCDrv[emmc_ip].u32_ClkKHz = 32000;
                break;
            case eMMC_PLL_CLK__36M:
            case BIT_FCIE_CLK_36M:
                g_eMMCDrv[emmc_ip].u32_ClkKHz = 36000;
                break;
            case eMMC_PLL_CLK__40M:
            case BIT_FCIE_CLK_40M:
                g_eMMCDrv[emmc_ip].u32_ClkKHz = 40000;
                break;
            case eMMC_PLL_CLK__48M:
            case BIT_FCIE_CLK_48M:
                g_eMMCDrv[emmc_ip].u32_ClkKHz = 48000;
                break;
            case BIT_FCIE_CLK_300K:
                g_eMMCDrv[emmc_ip].u32_ClkKHz = 300;
                break;
            case BIT_CLK_XTAL_12M:
                g_eMMCDrv[emmc_ip].u32_ClkKHz = 12000;
                break;
            case BIT_FCIE_CLK_43_2M:
                g_eMMCDrv[emmc_ip].u32_ClkKHz = 43200;
                break;
            default:
                eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "eMMC Err: %Xh\n", eMMC_ST_ERR_INVALID_PARAM);
                return eMMC_ST_ERR_INVALID_PARAM;
        }

        REG_FCIE_CLRBIT(EMMC_GET_CLK_REG(emmc_ip), BIT1 | BIT0);
        REG_FCIE_SETBIT(EMMC_GET_CLK_REG(emmc_ip), BIT6); // reg_ckg_sdio(BK:x1038_x43) [B5] -> 0:clk_boot 1:clk_sd
        REG_FCIE_CLRBIT(EMMC_GET_CLK_REG(emmc_ip), BIT13 | BIT12 | BIT11 | BIT10 | BIT9 | BIT8);

        if (gu32_SupportEmmc50[emmc_ip])
            REG_FCIE_W(reg_emmcpll_fbdiv(emmc_ip), 0x6);

        REG_FCIE_SETBIT(reg_sc_gp_ctrl, EMMC_GET_BOOT_CLK(emmc_ip));

        u32RealClk = g_eMMCDrv[emmc_ip].u32_ClkKHz * 1000;
        clk_set_rate(clkdata->clk_fcie[emmc_ip], u32RealClk);

        g_eMMCDrv[emmc_ip].u16_ClkRegVal = u16_ClkParam;
    }

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

    //    fcie
    REG_FCIE_CLRBIT(FCIE_DDR_MODE(emmc_ip), BIT_BYPASS_MODE_MASK | BIT_CIFD_MODE_MASK | BIT_MACRO_MODE_MASK);

    switch (u32_FCIE_IF_Type)
    {
        case FCIE_eMMC_BYPASS:
            REG_FCIE_SETBIT(FCIE_DDR_MODE(emmc_ip), BIT_BYPASS_MODE_MASK);
            if (gu32_SupportEmmc50[emmc_ip])
                eMMC_pll_setting(emmc_ip, u32_FCIE_IF_Type);
            break;

        case FCIE_eMMC_SDR:
            // fcie
            REG_FCIE_SETBIT(FCIE_DDR_MODE(emmc_ip), BIT_PAD_IN_SEL_SD | BIT_FALL_LATCH | BIT10);
            if (gu32_SupportEmmc50[emmc_ip])
                eMMC_pll_setting(emmc_ip, u32_FCIE_IF_Type);
            g_eMMCDrv[emmc_ip].u32_DrvFlag |= DRV_FLAG_SPEED_HIGH;
            break;

        case FCIE_MODE_8BITS_MACRO_DDR52:
            eMMC_debug(1, 1, "eMMC%d Err: Do not support!", emmc_ip);
            g_eMMCDrv[emmc_ip].u32_DrvFlag |= DRV_FLAG_DDR_MODE;
            break;

        case FCIE_eMMC_HS200:
            REG_FCIE_W(FCIE_DDR_MODE(emmc_ip), BIT_32BIT_MACRO_EN);
            REG_FCIE_W(GET_REG_ADDR(CLKGEN0_BASE, 0x4D), 0x14);
            eMMC_pll_setting(emmc_ip, u32_FCIE_IF_Type);
            g_eMMCDrv[emmc_ip].u32_DrvFlag |= DRV_FLAG_SPEED_HS200;
            break;

        case FCIE_eMMC_HS400:
            REG_FCIE_W(FCIE_DDR_MODE(emmc_ip), BIT_32BIT_MACRO_EN | BIT_DDR_EN);
            REG_FCIE_W(GET_REG_ADDR(CLKGEN0_BASE, 0x4D), 0x14);
            eMMC_pll_setting(emmc_ip, u32_FCIE_IF_Type);
            g_eMMCDrv[emmc_ip].u32_DrvFlag |= DRV_FLAG_SPEED_HS400;
            break;

        case FCIE_eMMC_5_1_AFIFO:
            eMMC_debug(1, 1, "eMMC%d Err: Do not support!", emmc_ip);
            g_eMMCDrv[emmc_ip].u32_DrvFlag |= DRV_FLAG_SPEED_HIGH;
            break;

        default:
            eMMC_debug(1, 1, "eMMC%d Err: wrong parameter for switch pad func\n", emmc_ip);
    }
}

void eMMC_pll_setting(eMMC_IP_EmType emmc_ip, U32 u32_FCIE_IF_Type)
{
    switch (u32_FCIE_IF_Type)
    {
        case FCIE_eMMC_BYPASS:
        case FCIE_eMMC_SDR:
            clk_set_rate(CLK_fcie_1x_p[emmc_ip]->clk, (FCIE_eMMC_SDR + 1));
            break;
        case FCIE_eMMC_HS200:
            clk_set_parent(clkdata->clk_fcie[emmc_ip], CLK_fcie_1x_p[emmc_ip]->clk);
            clk_set_rate(CLK_fcie_1x_p[emmc_ip]->clk, g_eMMCDrv[emmc_ip].u8_BUS_WIDTH);
            g_eMMCDrv[emmc_ip].u32_ClkKHz = 200000;
            break;

        case FCIE_eMMC_HS400:
            clk_set_parent(clkdata->clk_fcie[emmc_ip], CLK_fcie_2x_p[emmc_ip]->clk);
            clk_set_rate(CLK_fcie_2x_p[emmc_ip]->clk, g_eMMCDrv[emmc_ip].u8_BUS_WIDTH);
            g_eMMCDrv[emmc_ip].u32_ClkKHz = 200000;
            break;

        default:
            eMMC_debug(1, 1, "eMMC%d Err: wrong parameter for switch pad func\n", emmc_ip);
    }
    eMMC_hw_timer_delay(HW_TIMER_DELAY_100us); // asked by Irwin
}

U32 eMMC_pads_switch(eMMC_IP_EmType emmc_ip, U8 u8_PAD, U32 u32_FCIE_IF_Type)
{
    uintptr_t ptr_reg  = 0;
    U16       u16_mask = 0, u16_4bitmode = 0, u16_8bitmode = 0;

    if (emmc_ip == IP_EMMC0)
    {
        ptr_reg      = GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x67);
        u16_mask     = BIT11 | BIT10 | BIT9 | BIT8;
        u16_4bitmode = BIT8;
    }
    else if (emmc_ip == IP_EMMC1)
    {
        ptr_reg      = GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x61);
        u16_mask     = BIT8 | BIT4 | BIT2 | BIT0;
        u16_4bitmode = BIT0;
        u16_8bitmode = BIT2;
    }
    else if (emmc_ip == IP_EMMC2)
    {
        ptr_reg      = GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x67);
        u16_mask     = BIT13 | BIT12;
        u16_4bitmode = BIT12;
    }

    REG_FCIE_CLRBIT(reg_all_pad_in, BIT_ALL_PAD_IN);
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
    if (emmc_ip == IP_EMMC0)
    {
        REG_FCIE_CLRBIT(GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x67), BIT8);
    }
    else if (emmc_ip == IP_EMMC1)
    {
        switch (g_eMMCDrv[emmc_ip].u16_of_buswidth)
        {
            case 4:
                REG_FCIE_CLRBIT(GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x61), BIT0);
                break;
            case 8:
                REG_FCIE_CLRBIT(GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x61), BIT2);
                break;
            default:
                pr_err(">> [emmc] Err: wrong buswidth config: %u!\n", g_eMMCDrv[emmc_ip].u16_of_buswidth);
                break;
        }
    }
    else if (emmc_ip == IP_EMMC2)
    {
        REG_FCIE_CLRBIT(GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x67), BIT12);
    }
}

void eMMC_Read_Timeout_Clear(eMMC_IP_EmType emmc_ip)
{
    if (emmc_ip == IP_EMMC0)
    {
        REG_FCIE_SETBIT(GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x67), BIT8);
    }
    else if (emmc_ip == IP_EMMC1)
    {
        switch (g_eMMCDrv[emmc_ip].u16_of_buswidth)
        {
            case 4:
                REG_FCIE_SETBIT(GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x61), BIT0);
                break;
            case 8:
                REG_FCIE_SETBIT(GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x61), BIT2);
                break;
            default:
                pr_err(">> [emmc] Err: wrong buswidth config: %u!\n", g_eMMCDrv[emmc_ip].u16_of_buswidth);
                break;
        }
    }
    else if (emmc_ip == IP_EMMC2)
    {
        REG_FCIE_SETBIT(GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x67), BIT12);
    }
}

void eMMC_Write_Timeout_Set(eMMC_IP_EmType emmc_ip)
{
    if (emmc_ip == IP_EMMC0)
    {
        REG_FCIE_CLRBIT(GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x67), BIT8);
    }
    else if (emmc_ip == IP_EMMC1)
    {
        REG_FCIE(GET_REG_ADDR(REG_BANK_MIU, 0x14)) = 0x400; // mask fcie read miu
        REG_FCIE(GET_REG_ADDR(REG_BANK_MIU, 0x13)) = 0x17;
        REG_FCIE(GET_REG_ADDR(REG_BANK_MIU, 0x43)) = 0x17;
        eMMC_hw_timer_sleep(HW_TIMER_DELAY_1ms);
    }
    else if (emmc_ip == IP_EMMC2)
    {
        REG_FCIE_CLRBIT(GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x67), BIT12);
    }
}

void eMMC_Write_Timeout_Clear(eMMC_IP_EmType emmc_ip)
{
    if (emmc_ip == IP_EMMC0)
    {
        REG_FCIE_SETBIT(GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x67), BIT8);
    }
    else if (emmc_ip == IP_EMMC1)
    {
        REG_FCIE(GET_REG_ADDR(REG_BANK_MIU, 0x14)) = 0x00; // PAD_EMMC_D0
        REG_FCIE(GET_REG_ADDR(REG_BANK_MIU, 0x13)) = 0x17; // PAD_EMMC_D0
        REG_FCIE(GET_REG_ADDR(REG_BANK_MIU, 0x43)) = 0x17; // PAD_EMMC_D3
    }
    else if (emmc_ip == IP_EMMC2)
    {
        REG_FCIE_SETBIT(GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x67), BIT12);
    }
}

void eMMC_driving_control(eMMC_IP_EmType emmc_ip, U8 u8_PAD, EMMCPinDrv_T st_EmmcPindrv)
{
    U32 u32_clkpin = 0, u32_cmdpin = 0, u32_datapin[8] = {0}, i;

    if (emmc_ip == IP_EMMC0)
    {
    }
    else if (emmc_ip == IP_EMMC1)
    {
        if (u8_PAD == 0)
        {
            u32_clkpin     = PAD_EMMC_CLK;
            u32_cmdpin     = PAD_EMMC_CMD;
            u32_datapin[0] = PAD_EMMC_D0;
            u32_datapin[1] = PAD_EMMC_D1;
            u32_datapin[2] = PAD_EMMC_D2;
            u32_datapin[3] = PAD_EMMC_D3;
            u32_datapin[4] = PAD_EMMC_D4;
            u32_datapin[5] = PAD_EMMC_D5;
            u32_datapin[6] = PAD_EMMC_D6;
            u32_datapin[7] = PAD_EMMC_D7;
        }
    }

    if (st_EmmcPindrv.eDrvClk >= 0)
        sstar_gpio_drv_set(u32_clkpin, st_EmmcPindrv.eDrvClk & 0x7);

    if (st_EmmcPindrv.eDrvCmd >= 0)
        sstar_gpio_drv_set(u32_cmdpin, st_EmmcPindrv.eDrvCmd & 0x7);

    if (st_EmmcPindrv.eDrvData >= 0)
        for (i = 0; i < g_eMMCDrv[emmc_ip].u16_of_buswidth; i++)
            sstar_gpio_drv_set(u32_datapin[i], st_EmmcPindrv.eDrvData & 0x7);
}

void eMMC_Init_Padpin(eMMC_IP_EmType emmc_ip, U8 u8_PAD)
{
    if (emmc_ip == IP_EMMC0)
    {
        if (u8_PAD == 0)
        {
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0x01)) = 0x54; // PAD_EMMC_D0
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0x00)) = 0x54; // PAD_EMMC_D1
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0x05)) = 0x54; // PAD_EMMC_D2
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0x04)) = 0x54; // PAD_EMMC_D3

            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0x06)) = 0x64; // PAD_EMMC_CLK
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0x03)) = 0x54; // PAD_EMMC_CMD
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO, 0x16)) = 0x54; // PAD_EMMC_RST
        }
    }
    else if (emmc_ip == IP_EMMC1)
    {
        if (u8_PAD == 0)
        {
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO2, 0x16)) = 0xA55; // PAD_EMMC_D0
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO2, 0x18)) = 0xA55; // PAD_EMMC_D1
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO2, 0x1A)) = 0xA55; // PAD_EMMC_D2
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO2, 0x14)) = 0xA55; // PAD_EMMC_D3

#if (FCIE_eMMC_SDR == FCIE_MODE_8BITS_MACRO_HIGH_SPEED)              // eMMC 8bit mode
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO2, 0x15)) = 0xA55; // PAD_EMMC_D4
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO2, 0x17)) = 0xA55; // PAD_EMMC_D5
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO2, 0x19)) = 0xA55; // PAD_EMMC_D6
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO2, 0x1B)) = 0xA55; // PAD_EMMC_D7
#endif

            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO2, 0x11)) = 0x254; // PAD_EMMC_CLK
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO2, 0x12)) = 0xA55; // PAD_EMMC_CMD
            REG_FCIE(GET_REG_ADDR(REG_BANK_PADGPIO2, 0x10)) = 0xA55; // PAD_EMMC_RST
        }
    }
}

void eMMC_RST_L(eMMC_IP_EmType emmc_ip)
{
    if (emmc_ip == IP_EMMC0)
    {
        REG_FCIE_SETBIT(GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x7A), BIT0);
    }
    if (emmc_ip == IP_EMMC1)
    {
        REG_FCIE_SETBIT(GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x61), BIT4);
    }
    if (emmc_ip == IP_EMMC2)
    {
        REG_FCIE_SETBIT(GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x68), BIT14);
    }

    REG_FCIE_SETBIT(FCIE_BOOT_CONFIG(emmc_ip), BIT_EMMC_RSTZ_EN);
    REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG(emmc_ip), BIT_EMMC_RSTZ);
}

void eMMC_RST_H(eMMC_IP_EmType emmc_ip)
{
    if (emmc_ip == IP_EMMC0)
    {
        REG_FCIE_SETBIT(GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x7A), BIT0);
    }
    if (emmc_ip == IP_EMMC1)
    {
        REG_FCIE_SETBIT(GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x61), BIT4);
    }
    if (emmc_ip == IP_EMMC2)
    {
        REG_FCIE_SETBIT(GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x68), BIT14);
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
    {
        dma_DMAAddr -= UL(CONFIG_MIU0_BUS_BASE); // MSTAR_MIU0_BUS_BASE;
        *u32MiuSel = 0;
    }

    return dma_DMAAddr;
}
