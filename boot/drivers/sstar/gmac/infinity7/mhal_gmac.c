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

//#define GMAC_FPGA             (1)
#define GAMC_DRIVING_UPDATEALL 0xff
#define GMAC_MAX_GEAR          16
#define GMAC_DRIVING_REG_MASK  0x780
#define GMAC_DRIVING_REG_SHIFT 7

#define GMAC1_BANK_ADDR_OFFSET   0x0A
#define rebase_based             0x1F000000
#define GET_REG_ADDR8(x, y)      ((x) + ((y) << 1) - ((y)&1))
#define BASE_REG_CLKGEN_2_PA     (ulong) GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x103F00)
#define BASE_REG_INTR_CTRL1_1_PA (ulong) GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x101700)
#define BASE_REG_GMACPLL_PA      (ulong) GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x103A00)
#define BASE_REG_PAD_GPIO2_PA    (ulong) GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x110400)
#define BASE_REG_GMAC0_PA        (ulong) GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x151000)
#define BASE_REG_GMAC1_PA        (ulong) GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x151100)
#define BASE_REG_NET_GP_CTRL_PA  (ulong) GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x151200)
#define BASE_REG_X32_GMAC0_PA    (ulong) GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x1A5000)
#define BASE_REG_X32_GMAC1_PA    (ulong) GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x1A5A00)

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

u8 GMAC_Driving_Setting[GMAC_NUM][GMAC_MAX_IO] = {{
                                                      7, // RGMII0_MDC
                                                      7, // RGMII0_MDIO
                                                      7, // RGMII0_TX_CLK
                                                      7, // RGMII0_TX_D0
                                                      7, // RGMII0_TX_D1
                                                      7, // RGMII0_TX_D2
                                                      7, // RGMII0_TX_D3
                                                      7  // RGMII0_TX_CTL
                                                  },
                                                  {
                                                      7, // RGMII1_MDC
                                                      7, // RGMII1_MDIO
                                                      7, // RGMII1_TX_CLK
                                                      7, // RGMII1_TX_D0
                                                      7, // RGMII1_TX_D1
                                                      7, // RGMII1_TX_D2
                                                      7, // RGMII1_TX_D3
                                                      7  // RGMII1_TX_CTL
                                                  }};

u8 GMAC_Driving_Offset[GMAC_NUM][GMAC_MAX_IO] = {{0x68, 0x69, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F},
                                                 {0x78, 0x79, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F}};

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

static void mhal_gmac_gpio_reset(u8 gmacId, int reset_io)
{
    ulong bank   = BASE_REG_PAD_GPIO2_PA;
    ulong offset = 0x66;

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
            // GPIO 231
            bank   = BASE_REG_PAD_GPIO2_PA;
            offset = 0x67;
            OUTREGMSK16(GET_REG_ADDR(REG_ADDR_BASE_PADTOP, 0x42), 0x2000, 0x2000);
        }
        else
        {
            // GPIO 230
            bank   = BASE_REG_PAD_GPIO2_PA;
            offset = 0x66;
            OUTREGMSK16(GET_REG_ADDR(REG_ADDR_BASE_PADTOP, 0x42), 0x1000, 0x1000);
        }

#endif
    }
    else if (gmacId == GMAC1)
    {
#ifdef GMAC_FPGA
        OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x31), 0x0000, 0x000F);
        mdelay(1);
        OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x31), 0x0002, 0x000F);
#else
        if (reset_io == 1)
        {
            // GPIO 247
            bank   = BASE_REG_PAD_GPIO2_PA;
            offset = 0x77;
            OUTREGMSK16(GET_REG_ADDR(REG_ADDR_BASE_PADTOP, 0x43), 0x2000, 0x2000);
        }
        else
        {
            // GPIO 246
            bank   = BASE_REG_PAD_GPIO2_PA;
            offset = 0x76;
            OUTREGMSK16(GET_REG_ADDR(REG_ADDR_BASE_PADTOP, 0x43), 0x1000, 0x1000);
        }

#endif
    }

    // Output Low
    OUTREGMSK16(GET_REG_ADDR(bank, offset), 0x0008, 0x000F);
    mdelay(20);
    // Output High
    OUTREGMSK16(GET_REG_ADDR(bank, offset), 0x000A, 0x000F);
}
static void mhal_gmac_update_driving(u8 gmacId, u8 io_idx)
{
    if (io_idx == GAMC_DRIVING_UPDATEALL)
    { // Update All IO
        int i = 0;

        for (i = 0; i < GMAC_MAX_IO; i++)
        {
            OUTREGMSK16(GET_REG_ADDR(BASE_REG_PAD_GPIO2_PA, GMAC_Driving_Offset[gmacId][i]),
                        (u16)(GMAC_Driving_Setting[gmacId][i] << GMAC_DRIVING_REG_SHIFT), GMAC_DRIVING_REG_MASK);
        }
    }
    else
    { // Update one IO by io_idx
        OUTREGMSK16(GET_REG_ADDR(BASE_REG_PAD_GPIO2_PA, GMAC_Driving_Offset[gmacId][io_idx]),
                    (u16)(GMAC_Driving_Setting[gmacId][io_idx] << GMAC_DRIVING_REG_SHIFT), GMAC_DRIVING_REG_MASK);
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
        mhal_gmac_update_driving(GMAC1, GAMC_DRIVING_UPDATEALL);
        // padtop 103c -> RGMII 1.8V
        // OUTREG16(GET_REG_ADDR(REG_ADDR_BASE_PADTOP,0x04), (INREG16(GET_REG_ADDR(REG_ADDR_BASE_PADTOP,0x04)) | 0x02));
#if (GMAC_DBG)
        printk("[%s][%d] SS GMAC1 Setting : Interface=%u\r\n", __func__, __LINE__, interface);
#endif
        if (interface == PHY_INTERFACE_MODE_MII)
        {
            OUTREG16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x01), 0x0000);
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
            // RX clock (REFCLK/2) invert : Using falling edge of RX 25MHz clock to pack 4bit data for DWMAC
            OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x02), BIT7, BIT7);
        }
        else if (interface == PHY_INTERFACE_MODE_RGMII)
        {
            // synopsys x32 bank (10M : xA003,100M : xE003, 1000M : ?)
            // OUTREG32(GET_REG_ADDR(BASE_REG_X32_GMAC1_PA,0x00),0xA000);
            // MAC via RGMII
            OUTREG16(GET_REG_ADDR(BASE_REG_GMAC1_PA, 0x01), 0x0001);
            // padtop 103c
            OUTREGMSK16(GET_REG_ADDR(REG_ADDR_BASE_PADTOP, 0x41), BIT1, BIT1);
            // chiptop 101e
            OUTREGMSK16(GET_REG_ADDR(SSTAR_BASE_REG_CHIPTOP_PA, 0x50), 0x0000, 0xFF00);
        }
        mhal_gmac_gpio_reset(GMAC1, reset_io);
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
            // RX clock (REFCLK/2) invert : Using falling edge of RX 25MHz clock to pack 4bit data for DWMAC
            OUTREGMSK16(GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x02), BIT7, BIT7);
        }
        else if (interface == PHY_INTERFACE_MODE_RGMII)
        {
            // synopsys x32 bank (10M : xA003,100M : xE003, 1000M : ?)
            // OUTREG32(GET_REG_ADDR(BASE_REG_X32_GMAC0_PA,0x00),0xA000);
            // MAC via RGMII
            OUTREG16(GET_REG_ADDR(BASE_REG_GMAC0_PA, 0x01), 0x0001);
            // padtop 103c
            OUTREGMSK16(GET_REG_ADDR(REG_ADDR_BASE_PADTOP, 0x41), BIT0, BIT0);
            // chiptop 101e All padin
            OUTREGMSK16(GET_REG_ADDR(SSTAR_BASE_REG_CHIPTOP_PA, 0x50), 0x0000, 0xFF00);
        }

        mhal_gmac_gpio_reset(GMAC0, reset_io);
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
    // no need
    return 0;
}

ulong dwmac_to_sigma_rebase(void __iomem *addr)
{
    u32           bank = 0, axi_offset = 0, offset = 0;
    u32           reg = (unsigned long)addr;
    u32           bank_offset;
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
void MDrv_GMAC_DumpReg(u32 bank)
{
    int  i;
    u32 *addr;
    u32  content_x32;

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

u8 MHal_GMAC_ReadReg8(u32 bank, u32 reg)
{
    u8 val;

    val = INREG8(GET_REG_ADDR8((ulong)GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, (bank << 8)), reg));
    return val;
}

void MHal_GMAC_WritReg8(u32 bank, u32 reg, u8 val)
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
