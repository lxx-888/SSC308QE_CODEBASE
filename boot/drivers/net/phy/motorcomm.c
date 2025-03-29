/*
 * drivers/net/phy/motorcomm.c
 *
 * Driver for Motorcomm PHYs
 *
 * Author: yinghong.zhang<yinghong.zhang@motor-comm.com>
 *
 * Copyright (c) 2019 Motorcomm, Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * Support : Motorcomm Phys:
 *      Giga phys: yt8511, yt8521, yt8531, yt8614, yt8618
 *      100/10 Phys : yt8512, yt8512b, yt8510
 *      Automotive 100Mb Phys : yt8010
 *      Automotive 100/10 hyper range Phys: yt8510
 */

#include <common.h>
#include <phy.h>
#include <linux/delay.h>

#define REG_DEBUG_ADDR_OFFSET           0x1e
#define REG_DEBUG_DATA                  0x1f
#define YT8531_RGMII_CONFIG1            0xa003
#define YT8531_TX_DELAY_MSK             0xf
#define YT8531_TX_DELAY_1ns             0x5
#define YT8531_TX_DELAY                 CONFIG_PHY_YT_8531_TX_DELAY


static int ytphy_probe(struct phy_device *phydev)
{
    return 0;
}

static int ytphy_config(struct phy_device *phydev)
{
    genphy_config_aneg(phydev);
    return 0;
}

static int ytphy_read_ext(struct phy_device *phydev, int regnum)
{
    int oldpage = phy_read(phydev, MDIO_DEVAD_NONE, REG_DEBUG_ADDR_OFFSET);
    int val;

    phy_write(phydev, MDIO_DEVAD_NONE, REG_DEBUG_ADDR_OFFSET, regnum);
    val = phy_read(phydev, MDIO_DEVAD_NONE, REG_DEBUG_DATA);
    phy_write(phydev, MDIO_DEVAD_NONE, REG_DEBUG_ADDR_OFFSET, oldpage);

    return val;
}

static int ytphy_write_ext(struct phy_device *phydev, int regnum, u16 val)
{
    int oldpage = phy_read(phydev, MDIO_DEVAD_NONE, REG_DEBUG_ADDR_OFFSET);

    phy_write(phydev, MDIO_DEVAD_NONE, REG_DEBUG_ADDR_OFFSET, regnum);
    phy_write(phydev, MDIO_DEVAD_NONE, REG_DEBUG_DATA, val);
    phy_write(phydev, MDIO_DEVAD_NONE, REG_DEBUG_ADDR_OFFSET, oldpage);

    return 0;
}

static int ytphy_parse_status(struct phy_device *phydev)
{
    int val;

    genphy_parse_link(phydev);

    if(phydev->speed == SPEED_1000)
    {
        val = ytphy_read_ext(phydev, YT8531_RGMII_CONFIG1);

        //set tx clk delay from 2ns to 1ns
        val = ((val & ~(YT8531_TX_DELAY_MSK)) | YT8531_TX_DELAY);
        ytphy_write_ext(phydev, YT8531_RGMII_CONFIG1, val);
    }
    return 0;
}

static int ytphy_startup(struct phy_device *phydev)
{
    int ret;

    /* Read the Status (2x to make sure link is right) */
    ret = genphy_update_link(phydev);
    if (ret)
        return ret;

    return ytphy_parse_status(phydev);
}

static struct phy_driver YT8531_driver = {
    .name = "YT8531 Gigabit Ethernet",
    .uid = 0x4f51e91b,
    .mask = 0x00000fff,
    .features = PHY_GBIT_FEATURES,
    .probe = &ytphy_probe,
    .config = &ytphy_config,
    .startup = &ytphy_startup,
    .shutdown = &genphy_shutdown,
};

int phy_yt_init(void)
{
    phy_register(&YT8531_driver);

    return 0;
}
