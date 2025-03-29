/*
 * sstar_pcie_generic.c - Sigmastar
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
#include <log.h>
#include <env.h>
#include <dm.h>
#include <dm/lists.h>
#include <dm/device_compat.h>
#include <vsprintf.h>

static int sstar_dm_pcie_probe(struct udevice *dev)
{
    ofnode          node;
    int             ret;
    struct udevice *pcie_dev;
    const char *    drv_name = NULL;
    const char *    dev_name = NULL;

    /* Find ep subnode */
    ofnode_for_each_subnode(node, dev_ofnode(dev))
    {
        drv_name = NULL;

        /* find nodes for RC */
        if (ofnode_device_is_compatible(node, "snps,dw-pcie"))
        {
            drv_name = "sstar_pcie";
            dev_dbg(dev, "driver: %s\r\n", drv_name);
        }
        /* find nodes for EP */
        else if (ofnode_device_is_compatible(node, "snps,dw-pcie-ep"))
        {
            drv_name = "sstar_pcie_ep";
            dev_dbg(dev, "driver: %s\r\n", drv_name);
        }
        /* find nodes of PCIe whose mode is specified by bootargs */
        else if (ofnode_device_is_compatible(node, "snps,dw-pcie-earlyparam"))
        {
            u32   portid = 0;
            char *bootargs;
            char *p, *m;
            char  pcieN[8];

            if (ofnode_read_u32(node, "portid", &portid))
                continue;

            bootargs = env_get("bootargs");
            if (!bootargs)
                continue;

            sprintf(pcieN, "pcie%d", portid);
            p = strstr(bootargs, pcieN);
            if (!p)
                continue;

            m = strchr(p, '=');
            if (m)
            {
                m++;
                if (*m == 'r') /* RC */
                    drv_name = "sstar_pcie";
                else if (*m == 'e') /* EP */
                    drv_name = "sstar_pcie_ep";

                dev_dbg(dev, "%s: %s\r\n", pcieN, drv_name);
            }
        }

        if (drv_name)
        {
            dev_name = ofnode_get_name(node);
            ret      = device_bind_driver_to_node(dev, drv_name, dev_name, node, &pcie_dev);
            if (ret)
            {
                dev_err(dev, "unable to bind device %s and driver %s\n", drv_name, dev_name);
            }
        }
    }
    return 0;
}

static int sstar_dm_pcie_remove(struct udevice *dev)
{
    return 0;
}

static const struct udevice_id sstar_dm_pcie_ids[] = {{.compatible = "sstar,dm_pcie"}, {}};

/* dual mode PCIe controller */
U_BOOT_DRIVER(sstar_dm_pcie) = {.name     = "sstar_dm_pcie",
                                .id       = UCLASS_PCI_GENERIC,
                                .of_match = sstar_dm_pcie_ids,
                                .probe    = sstar_dm_pcie_probe,
                                .remove   = sstar_dm_pcie_remove};

int sstar_dm_pcie_init(void)
{
    struct udevice *dev;
    int             ret;

    ret = uclass_first_device(UCLASS_PCI_GENERIC, &dev);

    return ret;
}
