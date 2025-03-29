/*
 * drv_padmux.c- Sigmastar
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

#define LOG_CATEGORY UCLASS_PINCTRL

/*-----------------------------------------------------------------------------
  Include Files
  ------------------------------------------------------------------------------*/
#include <common.h>
#include <log.h>
#ifdef CONFIG_DM
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#endif
#include "asm/types.h"
#include "asm/arch/mach/sstar_types.h"
#include "asm/arch/mach/platform.h"
#include "asm/arch/mach/io.h"
#include "padmux.h"
#include "drv_puse.h"
#include "drv_gpio.h"
#include "gpio.h"

#define PADINFO_NAME "schematic"

typedef struct
{
    U32 u32PadId;
    U32 u32Mode;
    U32 u32Puse;
} __attribute__((__packed__)) pad_info_t;

typedef struct
{
    U32         pad_num;
    pad_info_t *pad_info;
} padmux_plat;

static padmux_plat *m_padmux = NULL;

int drv_padmux_active(void)
{
    return (m_padmux && m_padmux->pad_info);
}

int drv_padmux_getpad(int puse)
{
    int i;

    if ((MDRV_PUSE_NA == puse) || !m_padmux)
        return PAD_UNKNOWN;

    for (i = 0; i < m_padmux->pad_num; i++)
    {
        if (m_padmux->pad_info[i].u32Puse == puse)
            return m_padmux->pad_info[i].u32PadId;
    }
    return PAD_UNKNOWN;
}

int drv_padmux_getmode(int puse)
{
    int i;

    if (MDRV_PUSE_NA == puse)
        return PINMUX_FOR_UNKNOWN_MODE;

    for (i = 0; i < m_padmux->pad_num; i++)
    {
        if (m_padmux->pad_info[i].u32Puse == puse)
            return m_padmux->pad_info[i].u32Mode;
    }
    return PINMUX_FOR_UNKNOWN_MODE;
}

int drv_padmux_getpuse(int ip, int ch, int pad)
{
    int puse;
    puse = (ip) + (ch << 8) + (pad);
    return puse;
}

static int drv_padmux_of_to_plat(struct udevice *dev)
{
    padmux_plat *plat = dev_get_plat(dev);
    u32 *        pads;
    int          size = 0;
    int          ret  = 0;

    plat->pad_num  = 0;
    plat->pad_info = NULL;

    dev_read_prop(dev, PADINFO_NAME, &size);
    if (size)
    {
        pads = malloc(size);
        if (!pads)
            return -ENOMEM;

        ret = dev_read_u32_array(dev, PADINFO_NAME, pads, size / sizeof(u32));
        if (ret)
        {
            free(pads);
            dev_err(dev, "failed to read pads info. from dts\n");
            return ret;
        }

        plat->pad_num  = size / sizeof(pad_info_t);
        plat->pad_info = (pad_info_t *)pads;

        return 0;
    }

    dev_err(dev, "failed to read property %s\n", PADINFO_NAME);
    return 0;
}

static int drv_padmux_probe(struct udevice *dev)
{
    padmux_plat *plat = dev_get_plat(dev);

    if (plat->pad_num && plat->pad_info)
    {
        int i;

        log_debug("======= Padmux =======\n");
        log_debug("ID\tMode\tPuse\n");
        for (i = 0; i < plat->pad_num; i++)
        {
            pad_info_t *pad = &plat->pad_info[i];

            log_debug("%04d\t0x%04x\t0x%08x\n", pad->u32PadId, pad->u32Mode, pad->u32Puse);
            sstar_gpio_pad_val_set((U8)pad->u32PadId, pad->u32Mode);
        }
        log_debug("======================\n");

        m_padmux = plat;
    }

    return 0;
}

static const struct udevice_id sstar_padmux_ids[] = {{.compatible = "sstar,padmux"}, {}};

U_BOOT_DRIVER(padmux_sstar) = {
    .name       = "padmux_sstar",
    .id         = UCLASS_NOP,
    .of_match   = sstar_padmux_ids,
    .of_to_plat = drv_padmux_of_to_plat,
    .plat_auto  = sizeof(padmux_plat),
    .probe      = drv_padmux_probe,
    .flags      = DM_FLAG_PRE_RELOC,
};

int sstar_padmux_init(void)
{
    struct udevice *dev;
    int             ret;
    int             retval = 0;

    for (uclass_first_device(UCLASS_NOP, &dev); dev; uclass_find_next_device(&dev))
    {
        if (dev->driver == DM_DRIVER_GET(padmux_sstar))
        {
            ret = device_probe(dev);
            if (ret)
            {
                printf("Failed to probe device %s err: %d\n", dev->name, ret);
                retval = ret;
            }
        }
    }

    return retval;
}
