/*
 * mhal_gmac.c- Sigmastar
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

#include <common.h>
#include <command.h>
#include "asm/arch/mach/sstar_types.h"
#include "asm/arch/mach/platform.h"
#include "asm/arch/mach/io.h"
#include "phy.h"
#include <linux/delay.h>
#include "mhal_gmac.h"
#include <net.h>
#include <linux/mii.h>
#include <phy.h>
#include <miiphy.h>
#include <console.h>

//#define GMAC_FPGA
#define GAMC_DRIVING_UPDATEALL 0xff
#define GMAC_MAX_GEAR          4
#define GMAC_DRIVING_REG_MASK  0x180
#define GMAC_DRIVING_REG_SHIFT 7

#define GMAC1_BANK_ADDR_OFFSET      0x0A
#define rebase_based                0x1F000000
#define GET_REG_ADDR8(x, y)         ((x) + ((y) << 1) - ((y)&1))
#define GET_BASE_ADDR_BY_BANK(x, y) ((x) + ((y) << 1))
#define SSTAR_BASE_REG_RIU_PA       (0x1F000000)
#define SSTAR_BASE_REG_CHIPTOP_PA   GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x101E00)
#define BASE_REG_CLKGEN_2_PA        (ulong) GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x103F00)
#define BASE_REG_INTR_CTRL1_1_PA    (ulong) GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x101700)
#define BASE_REG_GMACPLL_PA         (ulong) GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x103A00)
#define BASE_REG_PAD_GPIO_PA        (ulong) GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x103E00)
#define BASE_REG_PAD_GPIO2_PA       (ulong) GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x110400)
#define BASE_REG_NET_GP_CTRL_PA     (ulong) GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x151200)
#define BASE_REG_X32_GMAC0_PA       (ulong) GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x1A5000)
#define BASE_REG_X32_GMAC1_PA       (ulong) GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x1A5A00)
#define BASE_REG_GMAC0_PA           (ulong) GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x151000)
#define BASE_REG_GMAC1_PA           (ulong) GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x151100)

#define GMAC0    0
#define GMAC1    1
#define GMAC_NUM 2

#define GMAC_DBG 0

enum
{
    GMAC_MDC,
    GMAC_MDIO,
    GMAC_RGMII_TX_CLK,
    GMAC_RGMII_TX_D0,
    GMAC_RGMII_TX_D1,
    GMAC_RGMII_TX_D2,
    GMAC_RGMII_TX_D3,
    GMAC_RGMII_TX_CTL,
    GMAC_MAX_IO
};

char *Gmac_IOName[GMAC_MAX_IO] = {
    "MDC", "MDIO", "RGMII_TX_CLK", "RGMII_TX_D0", "RGMII_TX_D1", "RGMII_TX_D2", "RGMII_TX_D3", "RGMII_TX_CTL",
};

/*
 * Uboot command :
 * gmac drv <gmac number> <io index> <gear>
 * <gmac number> = 0: gmac0 , 1: gmac1
 * <io index> = from 0 to 8 are mapping to
 * GMAC_MDC,
 * GMAC_MDIO,
 * GMAC_RGMII_TX_CLK,
 * GMAC_RGMII_TX_D0,
 * GMAC_RGMII_TX_D1,
 * GMAC_RGMII_TX_D2,
 * GMAC_RGMII_TX_D3,
 * GMAC_RGMII_TX_CTL,
 * GMAC_RGMII_MCLK
 *
 * ex :
 * input:
 * gmac drv 0 0 3
 * output on console:
 * GMAC[0] : MDC driving changed to gear 3
 *
 * Driving Gear Mapping :
 * 3.3V mode (typical)
 * Gear (DS3, DS2, DS1, DS0) =
 * 0 -> (0, 0) -> 4mA
 * 1 -> (0, 1) -> 8mA
 * 2 -> (1, 0) -> 12mA
 * 3 -> (1, 1) -> 16mA
 * */
uint8_t GMAC_Driving_Setting[GMAC_NUM][GMAC_MAX_IO] = {{
                                                           1, // RGMII0_MDC
                                                           1, // RGMII0_MDIO
                                                           3, // RGMII0_TX_CLK
                                                           2, // RGMII0_TX_D0
                                                           2, // RGMII0_TX_D1
                                                           2, // RGMII0_TX_D2
                                                           2, // RGMII0_TX_D3
                                                           1  // RGMII0_TX_CTL
                                                       },
                                                       {
                                                           1, // RGMII1_MDC
                                                           1, // RGMII1_MDIO
                                                           1, // RGMII1_TX_CLK
                                                           1, // RGMII1_TX_D0
                                                           1, // RGMII1_TX_D1
                                                           1, // RGMII1_TX_D2
                                                           1, // RGMII1_TX_D3
                                                           1  // RGMII1_TX_CTL
                                                       }};

uint8_t GMAC_Driving_Offset[GMAC_NUM][GMAC_MAX_IO] = {{0x34, 0x33, 0x2D, 0x2F, 0x30, 0x31, 0x32, 0x2E},
                                                      {0x64, 0x63, 0x5D, 0x5F, 0x60, 0x61, 0x62, 0x5E}};

/*
 * Usage of proc for GMAC
 * 1.cat /proc/gmac_0/driving
 * 2.echo io_idx gear > /proc/gmac_0/driving
 *
 * ex :
 * input:
 * echo 0 8 > /proc/gmac_0/driving
 * output on console:
 * MDC driving = 8
 *
 * Driving Gear Mapping :
 * 3.3V mode (typical)
 * Gear (DS3, DS2, DS1, DS0) =
 * 0 -> (0, 0, 0, 0) -> 4.4mA
 * 1 -> (0, 0, 0, 1) -> 6.5mA
 * 2 -> (0, 0, 1, 0) -> 8.7mA
 * 3 -> (0, 0, 1, 1) -> 10.9mA
 * 4 -> (0, 1, 0, 0) -> 13.0mA
 * 5 -> (0, 1, 0, 1) -> 15.2mA
 * 6 -> (0, 1, 1, 0) -> 17.4mA
 * 7 -> (0, 1, 1, 1) -> 19.5mA
 * 8 -> (1, 0, 0, 0) -> 21.7mA
 * 9 -> (1, 0, 0, 1) -> 23.9mA
 * 10 -> (1, 0, 1, 0) -> 26.0mA
 * 11 -> (1, 0, 1, 1) -> 28.2mA
 * 12 -> (1, 1, 0, 0) -> 30.3mA
 * 13 -> (1, 1, 0, 1) -> 32.5mA
 * 14 -> (1, 1, 1, 0) -> 34.6mA
 * 15 -> (1, 1, 1, 1) -> 36.8mA
 * */
static char gmac_reset0;
static char gmac_reset1;
uint16_t    G_txc_phase = txc_0_phase;
extern int  G_gmac_calb;

static void mhal_gmac_gpio_reset(uint8_t gmacId, int interface, int reset_io)
{
    uint32_t bank   = BASE_REG_PAD_GPIO2_PA;
    uint32_t offset = 0x26;
    if (reset_io == 0xFF)
        return;

    if (gmacId == GMAC0)
    {
#ifdef GMAC_FPGA
        OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x31), 0x0000, 0x000F);
        mdelay(1);
        OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x31), 0x0002, 0x000F);
#else
        if (reset_io == 1)
        {
            // GPIO 138
            bank   = BASE_REG_PAD_GPIO2_PA;
            offset = 0x0A;
        }
        else
        {
            // GPIO 101
            bank   = BASE_REG_PAD_GPIO2_PA;
            offset = 0x26;
        }
#if PHASE_CALB
        if (!G_gmac_calb)
        {
            // Output Low
            OUTREGMSK16(GET_REG_ADDR(bank, offset), 0x0008, 0x000F);
            mdelay(20);
            // Output High
            OUTREGMSK16(GET_REG_ADDR(bank, offset), 0x000A, 0x000F);
            mdelay(50);
        }
        else
#endif
        {
            // Output Low
            OUTREGMSK16(GET_REG_ADDR(bank, offset), 0x0008, 0x000F);
            if (interface == PHY_INTERFACE_MODE_RGMII)
            {
                mdelay(10);
                // open rgmii0 pad
                OUTREGMSK16(GET_REG_ADDR(REG_ADDR_BASE_PADTOP, 0x41), BIT0, BIT0);
            }
            mdelay(20);
            // Output High
            OUTREGMSK16(GET_REG_ADDR(bank, offset), 0x000A, 0x000F);
            mdelay(300);
        }
#endif
    }
    else if (gmacId == GMAC1)
    {
        if (interface == PHY_INTERFACE_MODE_MII)
        {
            return;
        }
#ifdef GMAC_FPGA
        OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x31), 0x0000, 0x000F);
        mdelay(1);
        OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x31), 0x0002, 0x000F);
#else
        if (reset_io == 1)
        {
            // GPIO 86
            bank   = BASE_REG_PAD_GPIO_PA;
            offset = 0x56;
        }
        else
        {
            // GPIO 85
            bank   = BASE_REG_PAD_GPIO_PA;
            offset = 0x55;
        }
#if PHASE_CALB
        if (!G_gmac_calb)
        {
            // Output Low
            OUTREGMSK16(GET_REG_ADDR(bank, offset), 0x0008, 0x000F);
            mdelay(20);
            // Output High
            OUTREGMSK16(GET_REG_ADDR(bank, offset), 0x000A, 0x000F);
            mdelay(50);
        }
        else
#endif
        {
            // Output Low
            OUTREGMSK16(GET_REG_ADDR(bank, offset), 0x0008, 0x000F);
            if (interface == PHY_INTERFACE_MODE_RGMII)
            {
                mdelay(10);
                // open rgmii1 pad
                OUTREGMSK16(GET_REG_ADDR(REG_ADDR_BASE_PADTOP, 0x41), BIT1, BIT1);
            }
            mdelay(20);
            // Output High
            OUTREGMSK16(GET_REG_ADDR(bank, offset), 0x000A, 0x000F);
            mdelay(300);
        }
#endif
    }
}

static void mhal_gmac_mclk_setting(uint8_t gmacId, int mclk_freq, int mclk_mode)
{
    if (gmacId == GMAC0)
    {
        if (mclk_freq == 0)
        {
            // do nothing
        }
        else
        {
            if (mclk_mode == 1)
            {
                // GPIO159 : disable GPIO mode ,enable output
                OUTREG8(GET_REG_ADDR8(BASE_REG_PAD_GPIO2_PA, 0x4A), 0x0a50);
                // padtop reg_gphy0_ref_mode
                OUTREGMSK16(GET_REG_ADDR(REG_ADDR_BASE_PADTOP, 0x38), 0x0001, BIT1 | BIT0);
                // padtop reg_gmii_sel
                OUTREGMSK16(GET_REG_ADDR(REG_ADDR_BASE_PADTOP, 0x0c), 0x0001, BIT0);
            }
            else if (mclk_mode == 2)
            {
                // GPIO101 : disable GPIO mode ,enable output
                OUTREG8(GET_REG_ADDR8(BASE_REG_PAD_GPIO_PA, 0xCA), 0x0a50);
                // padtop reg_gphy0_ref_mode
                OUTREGMSK16(GET_REG_ADDR(REG_ADDR_BASE_PADTOP, 0x38), 0x0002, BIT1 | BIT0);
                // padtop reg_gmii_sel
                OUTREGMSK16(GET_REG_ADDR(REG_ADDR_BASE_PADTOP, 0x0c), 0x0001, BIT0);
            }
            else
            {
                // default is 1
                // GPIO159 : disable GPIO mode ,enable output
                OUTREG8(GET_REG_ADDR8(BASE_REG_PAD_GPIO2_PA, 0x4A), 0x0a50);
                // padtop reg_gphy0_ref_mode
                OUTREGMSK16(GET_REG_ADDR(REG_ADDR_BASE_PADTOP, 0x38), 0x0001, BIT1 | BIT0);
                // padtop reg_gmii_sel
                OUTREGMSK16(GET_REG_ADDR(REG_ADDR_BASE_PADTOP, 0x0c), 0x0001, BIT0);
            }

#if (CONFIG_SSTAR_CLK == 0)
            if (mclk_freq == 25)
            {
                // CLKGEN2 reg_ckg_gphy0_ref
                OUTREG16(GET_REG_ADDR(BASE_REG_CLKGEN_2_PA, 0x1C), 0x0000);
            }
            else if (mclk_freq == 50)
            {
                // CLKGEN2 reg_ckg_gphy0_ref
                OUTREG16(GET_REG_ADDR(BASE_REG_CLKGEN_2_PA, 0x1C), 0x0004);
            }
#endif
        }
    }
    else if (gmacId == GMAC1)
    {
        if (mclk_freq == 0)
        {
            // do nothing
        }
        else
        {
            if (mclk_mode == 1)
            {
                // GPIO86 : disable GPIO mode ,enable output
                OUTREG8(GET_REG_ADDR8(BASE_REG_PAD_GPIO_PA, 0xAC), 0x0a50);
                // padtop reg_gphy0_ref_mode
                OUTREGMSK16(GET_REG_ADDR(REG_ADDR_BASE_PADTOP, 0x38), 0x0010, BIT4 | BIT5);
            }
            else if (mclk_mode == 2)
            {
                // GPIO85 : disable GPIO mode ,enable output
                OUTREG8(GET_REG_ADDR8(BASE_REG_PAD_GPIO_PA, 0xAA), 0x0a50);
                // padtop reg_gphy0_ref_mode
                OUTREGMSK16(GET_REG_ADDR(REG_ADDR_BASE_PADTOP, 0x38), 0x0020, BIT4 | BIT5);
            }
            else
            {
                // default is 1
                //  GPIO86 : disable GPIO mode ,enable output
                OUTREG8(GET_REG_ADDR8(BASE_REG_PAD_GPIO_PA, 0xAC), 0x0a50);
                // padtop reg_gphy0_ref_mode
                OUTREGMSK16(GET_REG_ADDR(REG_ADDR_BASE_PADTOP, 0x38), 0x0010, BIT4 | BIT5);
            }

#if (CONFIG_SSTAR_CLK == 0)
            if (mclk_freq == 25)
            {
                // CLKGEN2 reg_ckg_gphy0_ref
                OUTREG16(GET_REG_ADDR(BASE_REG_CLKGEN_2_PA, 0x1D), 0x0000);
            }
            else if (mclk_freq == 50)
            {
                // CLKGEN2 reg_ckg_gphy0_ref
                OUTREG16(GET_REG_ADDR(BASE_REG_CLKGEN_2_PA, 0x1D), 0x0004);
            }
#endif
        }
    }
}

static void mhal_gmac_update_driving(uint8_t gmacId, uint8_t io_idx)
{
    if (io_idx == GAMC_DRIVING_UPDATEALL)
    { // Update All IO
        int i = 0;

        for (i = 0; i < GMAC_MAX_IO; i++)
        {
            OUTREGMSK16(GET_REG_ADDR(BASE_REG_PAD_GPIO2_PA, GMAC_Driving_Offset[gmacId][i]),
                        (uint16_t)(GMAC_Driving_Setting[gmacId][i] << GMAC_DRIVING_REG_SHIFT), GMAC_DRIVING_REG_MASK);
        }
    }
    else
    { // Update one IO by io_idx
        OUTREGMSK16(GET_REG_ADDR(BASE_REG_PAD_GPIO2_PA, GMAC_Driving_Offset[gmacId][io_idx]),
                    (uint16_t)(GMAC_Driving_Setting[gmacId][io_idx] << GMAC_DRIVING_REG_SHIFT), GMAC_DRIVING_REG_MASK);
    }
}

int sstar_gamc_adjust_driving(int gmacid, int io, int gear)
{
    if (gmacid >= GMAC_NUM)
        return -1;
    if (io >= GMAC_MAX_IO)
        return -1;
    if (gear >= GMAC_MAX_GEAR)
        return -1;

    printf("GMAC[%u] : %s driving changed to gear %u\n", gmacid, Gmac_IOName[io], gear);
    GMAC_Driving_Setting[gmacid][io] = gear;
    mhal_gmac_update_driving(gmacid, io);

    return 0;
}

int sstar_gmac_probe(phys_addr_t reg, int interface, int mclk_freq, int mclk_mode, int reset_io)
{
    // disable long packet protection for gmac0
    OUTREG16(GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x28), INREG16(GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x28)) & 0x7FFF);
    // disable long packet protection for gmac1
    OUTREG16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x28), INREG16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x28)) & 0x7FFF);

    // turn on gmacpll; wait around 200us for PLL to be stable
    // swch 4
    // wriu   0x00103a02 0x00
    // wriu   0x00103a03 0x00
    OUTREG8(GET_REG_ADDR8(BASE_REG_GMACPLL_PA, 0x02), 0);
    OUTREG8(GET_REG_ADDR8(BASE_REG_GMACPLL_PA, 0x03), 0);

    // OUTREG16(0x1f207402, 0x0000);
    //  turn on d2s & vco_en
    // wriu   0x00103a2c 0xbe
    OUTREG8(GET_REG_ADDR8(BASE_REG_GMACPLL_PA, 0x2c), 0xbe);

    // new add for U02
    // wait 1
    // wriu 0x00103a15 0x40
    mdelay(1);
    OUTREG8(GET_REG_ADDR8(BASE_REG_GMACPLL_PA, 0x15), 0x40);

    if (reg == GMAC1_Base)
    {
        if (interface != PHY_INTERFACE_MODE_MII)
        {
            mhal_gmac_mclk_setting(GMAC1, mclk_freq, mclk_mode);
            mhal_gmac_update_driving(GMAC1, GAMC_DRIVING_UPDATEALL);
        }
        // padtop 103c -> RGMII 1.8V
        // OUTREG16(GET_REG_ADDR(BASE_REG_PADTOP_PA,0x04), (INREG16(GET_REG_ADDR(BASE_REG_PADTOP_PA,0x04)) | 0x02));
#if (GMAC_DBG)
        printk("[%s][%d] SS GMAC1 Setting : Interface=%u\r\n", __func__, __LINE__, interface);
#endif
        if (interface == PHY_INTERFACE_MODE_MII)
        {
            OUTREG16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x01), 0x8000);
        }
        else if (interface == PHY_INTERFACE_MODE_RMII)
        {
            // synopsys x32 bank (10M : xA003,100M : xE003, 1000M : ?)
            // OUTREG32(GET_REG_ADDR(BASE_REG_X32_GMAC1_PA,0x00),0xA000);
            // MAC via RMII
            OUTREG16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x01), 0x8004);
            // padtop 103c
            OUTREGMSK16(GET_REG_ADDR(REG_ADDR_BASE_PADTOP, 0x41), BIT1, BIT1);
            // chiptop 101e
            OUTREGMSK16(GET_REG_ADDR(SSTAR_BASE_REG_CHIPTOP_PA, 0x50), 0x0000, 0xFF00);
        }
        else if (interface == PHY_INTERFACE_MODE_RGMII)
        {
            // synopsys x32 bank (10M : xA003,100M : xE003, 1000M : ?)
            // OUTREG32(GET_REG_ADDR(BASE_REG_X32_GMAC1_PA,0x00),0xA000);
            // MAC via RGMII
            OUTREG16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x01), 0x0001);
            // chiptop 101e
            OUTREGMSK16(GET_REG_ADDR(SSTAR_BASE_REG_CHIPTOP_PA, 0x50), 0x0000, 0xFF00);
        }

        mhal_gmac_gpio_reset(GMAC1, interface, reset_io);
#if GMAC_DBG
        printk("[%s][%d] INREG16(0x1f2A2204)=0x%x\r\n", __func__, __LINE__,
               INREG16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x01)));
#endif
        // 1512 10 bit1 block reg for interrupt entry gmac1
        OUTREG16(GET_REG_ADDR(BASE_REG_NET_GP_CTRL_PA, 0x10),
                 INREG16(GET_REG_ADDR(BASE_REG_NET_GP_CTRL_PA, 0x10)) & (~BIT(1)));
#if (GMAC_DBG)
        printk("[%s][%d] INREG16(0x1f2a2440)=%x\r\n", __func__, __LINE__,
               INREG16(GET_REG_ADDR(BASE_REG_NET_GP_CTRL_PA, 0x10)));
#endif
    }
    else
    {
        mhal_gmac_mclk_setting(GMAC0, mclk_freq, mclk_mode);
        mhal_gmac_update_driving(GMAC0, GAMC_DRIVING_UPDATEALL);
        // padtop 103c -> RGMII 1.8V
        // OUTREG16(GET_REG_ADDR(BASE_REG_PADTOP_PA,0x04), (INREG16(GET_REG_ADDR(BASE_REG_PADTOP_PA,0x04)) | 0x80));
#if (GMAC_DBG)
        printk("[%s][%d] SS GMAC0 Setting : Interface=%u\r\n", __func__, __LINE__, interface);
#endif
        if (interface == PHY_INTERFACE_MODE_MII)
        {
            OUTREG16(GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x01), 0x0000);
        }
        else if (interface == PHY_INTERFACE_MODE_RMII)
        {
            // synopsys x32 bank (10M : xA003,100M : xE003, 1000M : ?)
            // OUTREG32(GET_REG_ADDR(BASE_REG_X32_GMAC0_PA,0x00),0xA000);
            // MAC via RMII
            OUTREG16(GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x01), 0x8004);
            // padtop 103c
            OUTREGMSK16(GET_REG_ADDR(REG_ADDR_BASE_PADTOP, 0x41), BIT0, BIT0);
            // chiptop 101e
            OUTREGMSK16(GET_REG_ADDR(SSTAR_BASE_REG_CHIPTOP_PA, 0x50), 0x0000, 0xFF00);
        }
        else if (interface == PHY_INTERFACE_MODE_RGMII)
        {
            // synopsys x32 bank (10M : xA003,100M : xE003, 1000M : ?)
            // OUTREG32(GET_REG_ADDR(BASE_REG_X32_GMAC0_PA,0x00),0xA000);
            // MAC via RGMII
            OUTREG16(GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x01), 0x0001);
            // chiptop 101e All padin
            OUTREGMSK16(GET_REG_ADDR(SSTAR_BASE_REG_CHIPTOP_PA, 0x50), 0x0000, 0xFF00);
        }

        mhal_gmac_gpio_reset(GMAC0, interface, reset_io);
#if (GMAC_DBG)
        printf("[%s][%d] INREG16(0x1f2A2004)=0x%x\r\n", __func__, __LINE__,
               INREG16(GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x01)));
#endif
        // 1512 10 bit0 block reg for interrupt entry gmac0
        OUTREG16(GET_REG_ADDR(BASE_REG_NET_GP_CTRL_PA, 0x10),
                 INREG16(GET_REG_ADDR(BASE_REG_NET_GP_CTRL_PA, 0x10)) & (~BIT(0)));
#if (GMAC_DBG)
        printf("[%s][%d] INREG16(0x1f2a2440)=%x\r\n", __func__, __LINE__,
               INREG16(GET_REG_ADDR(BASE_REG_NET_GP_CTRL_PA, 0x10)));
#endif
    }
#if (GMAC_DBG)
    // 1017 h6f bit11 clear to 0 for interrupt entry 2 , we got two entry of interrupt
    printf("[%s][%d] INREG16(0x1f202fbc)=%x\r\n", __func__, __LINE__,
           INREG16(GET_REG_ADDR(BASE_REG_INTR_CTRL1_1_PA, 0x6F)));
#endif
    OUTREG16(GET_REG_ADDR(BASE_REG_INTR_CTRL1_1_PA, 0x6F),
             INREG16(GET_REG_ADDR(BASE_REG_INTR_CTRL1_1_PA, 0x6F)) & (~BIT(11)));
#if (GMAC_DBG)
    printf("[%s][%d] INREG16(0x1f202fbc)=%x\r\n", __func__, __LINE__,
           INREG16(GET_REG_ADDR(BASE_REG_INTR_CTRL1_1_PA, 0x6F)));
#endif
    return 0;
}

int sstar_gmac_start_clock(phys_addr_t reg)
{
    int offset = 0x35;

    if (reg == GMAC1_Base)
    {
        offset = 0x36;
    }
    else
    {
        offset = 0x35;
    }

    OUTREGMSK16(GET_REG_ADDR(BASE_REG_CLKGEN_2_PA, offset), 0x0, 0x1);
    return 0;
}

int sstar_gmac_stop_clock(phys_addr_t reg)
{
    int offset = 0x35;

    if (reg == GMAC1_Base)
    {
        offset = 0x36;
    }
    else
    {
        offset = 0x35;
    }

    OUTREGMSK16(GET_REG_ADDR(BASE_REG_CLKGEN_2_PA, offset), 0x1, 0x1);

    return 0;
}

int sstar_gmac_set_rate(phys_addr_t reg, int speed)
{
    int offset = 0x35;

    if (reg == GMAC1_Base)
    {
        offset = 0x36;
    }
    else
    {
        offset = 0x35;
    }

    switch (speed)
    {
        case SPEED_1000:
            // Clk 125M for 1000Mbps
            OUTREG16(GET_REG_ADDR(BASE_REG_CLKGEN_2_PA, offset), 0x0);
            printk("SPEED_1000\n");
            break;
        case SPEED_100:
            // Clk 25M for 100Mbps
            OUTREG16(GET_REG_ADDR(BASE_REG_CLKGEN_2_PA, offset), 0x4);
            printk("SPEED_100\n");
            break;
        case SPEED_10:
            OUTREG16(GET_REG_ADDR(BASE_REG_CLKGEN_2_PA, offset), 0x8);
            printk("SPEED_10\n");
            break;
        default:
            pr_err("invalid speed %d", speed);
            return -EINVAL;
    }
    return 0;
}

int sstar_gmac_get_rate(phys_addr_t reg)
{
    int rate;
    int offset = 0x35;

    if (reg == GMAC1_Base)
    {
        offset = 0x36;
    }
    else
    {
        offset = 0x35;
    }

    rate = INREG16(GET_REG_ADDR(BASE_REG_CLKGEN_2_PA, offset)) >> 2;

    if (rate == 0)
        rate = 125 * 1000 * 1000;
    else if (rate == 1)
        rate = 25 * 1000 * 1000;
    else if (rate == 2)
        rate = 2.5 * 1000 * 1000;

    return rate;
}

int sstar_gmac_set_tx_clk_pad_sel(phys_addr_t regs, int interface, int speed)
{
    uint16_t val;

    ////GMAC offset 0x31,default setting
    if (regs == GMAC1_Base)
    {
        if (interface == PHY_INTERFACE_MODE_RMII)
        {
            switch (speed)
            {
                case SPEED_1000:
                    // without overwrite bit 2,3
                    OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x31), 0x0000, ~GMAC_SEL_MSK);
                    break;
                case SPEED_100:
                    OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x31), 0x2200, ~GMAC_SEL_MSK);
                    break;
                case SPEED_10:
                    OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x31), 0x2200, ~GMAC_SEL_MSK);
                    break;
            }
        }
        else if (interface == PHY_INTERFACE_MODE_RGMII)
        {
            switch (speed)
            {
                case SPEED_1000:
                    OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x31), 0x0000, ~GMAC_SEL_MSK);
                    OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x31), 0x0000, GMAC_RGMII_RXCTL_MODE);
                    break;
                case SPEED_100:
                    OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x31), 0x140, ~GMAC_SEL_MSK);
                    OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x31), GMAC_RGMII_RXCTL_MODE, GMAC_RGMII_RXCTL_MODE);
#if DYN_PHASE_CALB == 0
                    // reference bit 2,3 to calibrate tx clk phase
                    val = (INREG16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x31)) & GMAC_CALB_MSK) >> 2;
                    // only update bit 11.12
                    OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x31), val << 11, GMAC_TX_PHASE_MSK);
#endif
                    break;
                case SPEED_10:
                    OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x31), 0x0280, ~GMAC_SEL_MSK);
                    OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x31), GMAC_RGMII_RXCTL_MODE, GMAC_RGMII_RXCTL_MODE);
                    break;
            }
#if DYN_PHASE_CALB
            // reference bit 2,3 to calibrate tx clk phase
            val = (INREG16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x31)) & GMAC_CALB_MSK) >> 2;
            // only update bit 11.12
            OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x31), val << 11, GMAC_TX_PHASE_MSK);
#endif
        }
    }
    else
    {
        if (interface == PHY_INTERFACE_MODE_RMII)
        {
            switch (speed)
            {
                case SPEED_1000:
                    OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x31), 0x0000, ~GMAC_SEL_MSK);
                    break;
                case SPEED_100:
                    OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x31), 0x2200, ~GMAC_SEL_MSK);
                    break;
                case SPEED_10:
                    OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x31), 0x2200, ~GMAC_SEL_MSK);
                    break;
            }
        }
        else if (interface == PHY_INTERFACE_MODE_RGMII)
        {
            switch (speed)
            {
                case SPEED_1000:
                    OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x31), 0x0000, ~GMAC_SEL_MSK);
                    OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x31), 0x0000, GMAC_RGMII_RXCTL_MODE);
                    break;
                case SPEED_100:
                    OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x31), 0x140, ~GMAC_SEL_MSK);
                    OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x31), GMAC_RGMII_RXCTL_MODE, GMAC_RGMII_RXCTL_MODE);
#if DYN_PHASE_CALB == 0
                    // reference bit 2,3 to calibrate tx clk phase
                    val = (INREG16(GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x31)) & GMAC_CALB_MSK) >> 2;
                    // only update bit 11.12
                    OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x31), val << 11, GMAC_TX_PHASE_MSK);
#endif
                    break;
                case SPEED_10:
                    OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x31), 0x0280, ~GMAC_SEL_MSK);
                    OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x31), GMAC_RGMII_RXCTL_MODE, GMAC_RGMII_RXCTL_MODE);
                    break;
            }
#if DYN_PHASE_CALB
            // reference bit 2,3 to calibrate tx clk phase
            val = (INREG16(GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x31)) & GMAC_CALB_MSK) >> 2;
            // only update bit 11.12
            OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x31), val << 11, GMAC_TX_PHASE_MSK);
#endif
        }
    }

    return 0;
}

#define GMAC1_BANK_ADDR_OFFSET 0x0A
#define rebase_based           0x1F000000

ulong dwmac_to_sigma_rebase(void __iomem *addr)
{
    uint32_t      bank = 0, axi_offset = 0, offset = 0;
    uint32_t      reg = (unsigned long)addr;
    uint32_t      bank_offset;
    unsigned long rebase_addr;

    if (reg >= GMAC1_Base)
        offset = (reg - GMAC1_Base);
    else
        offset = (reg - GMAC0_Base);

    axi_offset = offset >> 8;

    switch (axi_offset)
    {
        case 0x00:
        case 0x01:
            axi_offset = 0x00;
            bank       = 0x1A50;
            break;
        case 0x02:
        case 0x03:
            axi_offset = 0x02;
            bank       = 0x1A51;
            break;
        case 0x07:
        case 0x08:
            axi_offset = 0x07;
            bank       = 0x1A52;
            break;
        case 0x09:
        case 0x0a:
            axi_offset = 0x09;
            bank       = 0x1A53;
            break;
        case 0x0b:
        case 0x0c:
            axi_offset = 0x0b;
            bank       = 0x1A54;
            break;
        case 0x0d:
        case 0x0e:
            axi_offset = 0x0d;
            bank       = 0x1A55;
            break;
        case 0x0f:
        case 0x10:
            axi_offset = 0x0f;
            bank       = 0x1A56;
            break;
        case 0x11:
        case 0x12:
            axi_offset = 0x11;
            bank       = 0x1A57;
            break;
    };

    bank_offset = offset - (axi_offset << 8);
    if (reg >= GMAC1_Base)
        bank += GMAC1_BANK_ADDR_OFFSET;

    rebase_addr = rebase_based + (bank << 9) + bank_offset;

#if 0
	if(bank==0x1A56){
		printf("[RBS] input write addr: 0x%x\r\n",reg);
		printf("[RBS] rebse write addr= 0x%lx \r\n", rebase_addr);
		printf("[RBS] input read addr: 0x%x\r\n",reg);
		printf("[RBS] rebse read addr= 0x%lx \r\n", rebase_addr);
	}
#endif

    return (ulong)rebase_addr;
}

void sstar_gmac_dump_packet(char *packet, int length, int dir /*0 for TX, 1 for RX*/)
{
    int i;

    printf("%s :\n", dir == 0 ? "TX" : "RX");

    for (i = 0; i < length; i++)
    {
        if ((i & 0xF) == 0)
            printf("\n");
        printf("0x%02x ", *(packet + i));
    }

    printf("\n");
}

#if (0)
void MDrv_GMAC_DumpReg(uint32_t bank)
{
    int       i;
    uint32_t *addr;
    uint32_t  content_x32;

    printf("BANK:0x%04X\n", bank);
    for (i = 0; i <= 0x7f; i += 1)
    {
        if (i % 0x8 == 0x0)
            printf("%02X: ", i);
        addr        = (unsigned long)(0x1f000000 + bank * 0x200 + i * 4);
        content_x32 = *(unsigned int *)addr;
        printf("0x%08X ", content_x32);
        if (i % 0x8 == 0x7)
            printf("\n");
    }
    printf("\r\n");
}
#endif

uint8_t MHal_GMAC_ReadReg8(uint32_t bank, uint32_t reg)
{
    uint8_t val;

    val = INREG8(GET_REG_ADDR8((ulong)GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, (bank << 8)), reg));
    return val;
}

void MHal_GMAC_WritReg8(uint32_t bank, uint32_t reg, uint8_t val)
{
    OUTREG8(GET_REG_ADDR8((ulong)GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, (bank << 8)), reg), val);
}

int loopback_test(void *dev, phys_addr_t regs, int pktlen)
{
    char            pass = 0;
    int             ret  = 0;
    uchar *         packet;
    int             i;
    struct udevice *pDev = dev;
    unsigned char   packet_fill[1500];

    // fill pattern
    for (i = 0; i < sizeof(packet_fill); i++)
    {
        packet_fill[i] = i;
    }

    ret = eth_get_ops(pDev)->send(pDev, &packet_fill, pktlen);

    for (int i = 0; i < 10; i++)
    {
        ret = eth_get_ops(pDev)->recv(pDev, 0, &packet);

        if (ret >= 0 && eth_get_ops(pDev)->free_pkt)
            eth_get_ops(pDev)->free_pkt(pDev, packet, ret);

        if (ret != -EAGAIN)
        {
            pass = 1;
            break;
        }
        mdelay(1);
    }

    if (pass == 1)
    {
        printf("P ");
        return 1;
    }
    else
    {
        printf("loopback fail ! \r\n");
        return 0;
    }
}

void sstar_gmac_loopback_test(void *dev, void *mii, int phyaddr, phys_addr_t regs, int pktlen, int speed)
{
    struct mii_dev *pmii = mii;
    u16             reg0bk;
    u16             phy_speed = 0x140;

    reg0bk = pmii->read(pmii, phyaddr, 0, MII_BMCR);

    switch (speed)
    {
        case SPEED_1000:
            phy_speed = 0x140;
            printf("SPEED 1G\n");
            break;
        case SPEED_100:
            phy_speed = 0x2100;
            printf("SPEED 100M\n");
            break;
        case SPEED_10:
            phy_speed = 0x100;
            printf("SPEED 10M\n");
            break;
    }

    // Raising loopback bit
    pmii->write(pmii, phyaddr, 0, MII_BMCR, phy_speed | BMCR_LOOPBACK);
    mdelay(100);

    // printf("readback , MII_BMCR = 0x%x\n", pmii->read(pmii, phyaddr, 0, MII_BMCR));

    while (1)
    {
        int ret;

        ret = loopback_test(dev, regs, pktlen);

        if (ctrlc() || ret == 0)
            break;
    }

    pmii->write(pmii, phyaddr, 0, MII_BMCR, (reg0bk & ~BMCR_LOOPBACK));
}

#if DYN_PHASE_CALB

#include <net.h>
#include <linux/mii.h>
#include <phy.h>
#include <miiphy.h>

int      gNeedCalibrat;
uint16_t gPHYReg0BackUp;

int dyn_loopback(phys_addr_t regs, uint16_t phase)
{
    char            gmac_calb_pass = 0;
    int             ret            = 0;
    uint32_t        reg;
    uchar *         packet;
    struct udevice *current;

    unsigned char packet_fill[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x08, 0x06,
                                   0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
                                   0xc0, 0xa8, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xa8, 0x01, 0x0f};

    if (regs == GMAC1_Base)
    {
        reg = GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x31);
    }
    else
    {
        reg = GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x31);
    }

    // only update bit 11.12
    OUTREGMSK16(reg, phase, GMAC_TX_PHASE_MSK);
    debug("TX Phase : 0x%08x \r\n", INREG16(reg));

    current = eth_get_dev();
    if (!current)
        return -ENODEV;

    ret = eth_get_ops(current)->send(current, &packet_fill, sizeof(packet_fill));

    for (int i = 0; i < 6; i++)
    {
        ret = eth_get_ops(current)->recv(current, 0, &packet);

        if (ret >= 0 && eth_get_ops(current)->free_pkt)
            eth_get_ops(current)->free_pkt(current, packet, ret);

        if (ret != -EAGAIN)
        {
            gmac_calb_pass = 1;
            break;
        }
        mdelay(1);
    }

    if (gmac_calb_pass == 1)
    {
        // printf("Phase %x loopback pass ! \r\n",phase);
        return 1;
    }
    else
    {
        // printf("Phase %x loopback fail ! \r\n",phase);
        return 0;
    }
}

int sstar_gmac_prepare_dyncalibrat(struct phy_device *phy, phys_addr_t regs, int interface, int speed)
{
    uint16_t        val;
    uint32_t        reg;
    uint16_t        phy_speed = 0x2100;
    struct mii_dev *pmii;

    gNeedCalibrat = 0;

    if (interface != PHY_INTERFACE_MODE_RGMII)
        return 0;

    if (regs == GMAC1_Base)
    {
        reg = GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x31);
    }
    else
    {
        reg = GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x31);
    }

    val = (INREG16(reg) & GMAC_RESV_SPEED_MSK) >> 14;
    // Check if we need calibration for current speed
    switch (speed)
    {
        case SPEED_1000:
            if (val == 1)
                return 0;
            phy_speed = 0x140;
            break;
        case SPEED_100:
            if (val == 2)
                return 0;
            phy_speed = 0x2100;
            break;
        case SPEED_10:
            if (val == 3)
                return 0;
            phy_speed = 0x100;
            break;
    }

    gNeedCalibrat  = 1;
    pmii           = mdio_get_current_dev();
    gPHYReg0BackUp = pmii->read(pmii, phy->addr, 0, MII_BMCR);
    // Raising loopback bit
    pmii->write(pmii, phy->addr, 0, MII_BMCR, phy_speed | BIT14 /*Loopback*/);
    mdelay(100);
    return -1;
}

void sstar_gmac_do_dyncalibrat(struct phy_device *phy, phys_addr_t regs, int interface, int speed)
{
    uint32_t        reg;
    struct mii_dev *pmii;

    if (!gNeedCalibrat)
        return;

    pmii = mdio_get_current_dev();

    while (1)
    {
        if (dyn_loopback(regs, txc_0_phase))
        {
            G_txc_phase = txc_0_phase;
            break;
        }
        if (dyn_loopback(regs, txc_180_phase))
        {
            G_txc_phase = txc_180_phase;
            break;
        }
        if (dyn_loopback(regs, txc_90_phase))
        {
            G_txc_phase = txc_90_phase;
            break;
        }
        if (dyn_loopback(regs, txc_270_phase))
        {
            G_txc_phase = txc_270_phase;
            break;
        }
        printf("\r\n\r\n");
        pmii->write(pmii, phy->addr, 0, MII_BMCR, (gPHYReg0BackUp & ~BIT14));
        return;
    }

    if (regs == GMAC1_Base)
    {
        reg = GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x31);
    }
    else
    {
        reg = GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x31);
    }

    OUTREGMSK16(reg, G_txc_phase >> 9, GMAC_CALB_MSK);

    switch (speed)
    {
        case SPEED_1000:
            OUTREGMSK16(reg, 1 << 14, GMAC_RESV_SPEED_MSK);
            break;
        case SPEED_100:
            OUTREGMSK16(reg, 2 << 14, GMAC_RESV_SPEED_MSK);
            break;
        case SPEED_10:
            OUTREGMSK16(reg, 3 << 14, GMAC_RESV_SPEED_MSK);
            break;
    }

    sstar_gmac_set_tx_clk_pad_sel(regs, interface, speed);

    // Clear loopback bit
    pmii->write(pmii, phy->addr, 0, MII_BMCR, (gPHYReg0BackUp & ~BIT14));
}
#endif
