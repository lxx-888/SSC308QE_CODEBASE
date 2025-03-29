/*
 * drv_rtcpwc.c - Sigmastar
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
#ifdef CONFIG_DM
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#endif

#include <rtc.h>
#include <hal_rtcpwc.h>

#define DTS_DEFAULT_DATE "default-date"

struct rtc_t
{
    struct hal_rtcpwc_t hal;
};

#ifdef CONFIG_DM_RTC
static struct udevice *rtc_get_udevice(void)
{
    int             rcode = 0;
    struct udevice *dev;

    rcode = uclass_get_device_by_seq(UCLASS_RTC, 0, &dev);
    if (rcode)
    {
        rcode = uclass_get_device(UCLASS_RTC, 0, &dev);
        if (rcode)
        {
            printf("Cannot find RTC: err=%d\n", rcode);
            return NULL;
        }
    }
    return dev;
}
#endif

void sstar_rtc_init(void)
{
#ifdef CONFIG_DM_RTC
    rtc_get_udevice();
#endif
}

void sstar_rtc_poweroff(void)
{
#ifdef CONFIG_DM_RTC
    struct udevice *dev = rtc_get_udevice();
    struct rtc_t *  drv = dev_get_plat(dev);
    hal_rtc_power_off(&drv->hal);
#endif
}

void sstar_rtc_sw3_set(u32 val)
{
    struct udevice *dev = rtc_get_udevice();
    struct rtc_t *  drv = dev_get_plat(dev);

    if ((hal_rtc_get_sw3(&drv->hal) != val) || (hal_rtc_get_sw2(&drv->hal) != RTC_MAGIC_NUMBER))
    {
        hal_rtc_set_sw3(&drv->hal, val);
    }
}

u32 sstar_rtc_sw3_get(void)
{
    struct udevice *dev = rtc_get_udevice();
    struct rtc_t *  drv = dev_get_plat(dev);

    return hal_rtc_get_sw3(&drv->hal);
}

static int sstar_rtc_of_to_plat(struct udevice *dev)
{
    int             ret;
    int             size;
    u32             val = 0;
    fdt_addr_t      base;
    s32             offset   = 0;
    u32             array[2] = {0};
    struct rtc_time tm       = {0};
    struct rtc_t *  drv      = dev_get_plat(dev);

    base = dev_read_addr(dev);
    if (base == FDT_ADDR_T_NONE)
    {
        return -EINVAL;
    }
    drv->hal.rtc_base = (unsigned long)base;

    if (dev_read_prop(dev, DTS_DEFAULT_DATE, &size))
    {
        ret = dev_read_u32_array(dev, DTS_DEFAULT_DATE, (u32 *)&tm, size / sizeof(u32));
        if (ret)
        {
            dev_err(dev, "failed to read " DTS_DEFAULT_DATE " from dts\n");
            return ret;
        }
    }

    ret = dev_read_bool(dev, "io0-hiz");
    if (ret)
    {
        drv->hal.pwc_io0_hiz = 1;
        dev_dbg(dev, "io0-hiz (%d)\n", 1);
    }

    ret = dev_read_u32(dev, "io2-wos", &val);
    if (ret)
    {
        dev_dbg(dev, "dev_read_u32 fail (io2-wos) %d\n", ret);
    }
    else
    {
        drv->hal.pwc_io2_valid = true;
        drv->hal.pwc_io2_cmp   = val;

        ret = dev_read_u32_array(dev, "io2-wos-v", array, 2);
        if (ret)
        {
            dev_dbg(dev, "of_property_read_u32_array fail (io2-wos-v) %d\n", ret);
        }
        else
        {
            drv->hal.pwc_io2_vlsel = array[0];
            drv->hal.pwc_io2_vhsel = array[1];
            dev_dbg(dev, "io2-wos-v (%d %d)\n", array[0], array[1]);
        }
    }

    ret = dev_read_bool(dev, "io3-pu");
    if (ret)
    {
        drv->hal.pwc_io3_pu = 1;
        dev_dbg(dev, "io3-pu (%d)\n", 1);
    }

    ret = dev_read_u32(dev, "io4-enable", &val);
    if (ret)
    {
        dev_dbg(dev, "of_property_read_u32 fail (io4-enable) %d\n", ret);
    }
    else
    {
        drv->hal.pwc_io4_valid = true;
        drv->hal.pwc_io4_value = val;
        dev_dbg(dev, "io4-enable (%d)\n", val);
    }

    ret = dev_read_u32(dev, "io5-enable", &val);
    if (ret)
    {
        dev_dbg(dev, "of_property_read_u32 fail (io5-enable) %d\n", ret);
    }
    else
    {
        drv->hal.pwc_io5_valid = true;
        drv->hal.pwc_io5_value = val;
        dev_dbg(dev, "io5-enable (%d)\n", val);
    }

    ret = dev_read_bool(dev, "io5-no-poweroff");
    if (ret)
    {
        drv->hal.pwc_io5_no_poweroff = 1;
    }

    ret = dev_read_bool(dev, "iso-auto-regen");
    if (ret)
    {
        drv->hal.iso_auto_regen = 1;
        dev_dbg(dev, "iso-auto-regen (%d)\n", 1);
    }

    ret = dev_read_u32(dev, "offset-count", &offset);
    if (ret)
    {
        dev_dbg(dev, "of_property_read_s32 fail (offset-count) %d\n", ret);
    }
    else
    {
        ret = dev_read_bool(dev, "offset-nagative");
        if (ret)
        {
            offset = -offset;
        }

        if (offset > -256 && offset < 256)
        {
            drv->hal.offset_count = (s16)offset;
        }
        else
        {
            drv->hal.offset_count = 0;
        }
        dev_dbg(dev, "offset-count (%d)\n", val);
    }

    return 0;
}

static int sstar_rtc_probe(struct udevice *dev)
{
    struct rtc_t *drv = dev_get_plat(dev);
    dev_dbg(dev, "sstar_rtc_probe\n");
    hal_rtc_init(&drv->hal);
    return 0;
}

static int sstar_rtc_get(struct udevice *dev, struct rtc_time *time)
{
    u32           seconds;
    struct rtc_t *drv = dev_get_plat(dev);
    dev_dbg(dev, "sstar_rtc_get\n");
    hal_rtc_read_time(&drv->hal, &seconds);
    rtc_to_tm(seconds, time);
    return 0;
}

static int sstar_rtc_set(struct udevice *dev, const struct rtc_time *time)
{
    u32           seconds;
    struct rtc_t *drv = dev_get_plat(dev);
    seconds           = rtc_mktime(time);
    dev_dbg(dev, "sstar_rtc_set\n");
    hal_rtc_set_time(&drv->hal, seconds);
    return 0;
}

static struct rtc_ops sstar_rtc_ops = {
    .get = sstar_rtc_get,
    .set = sstar_rtc_set,
};

static const struct udevice_id sstar_rtc_ids[] = {{.compatible = "sstar,rtcpwc"}, {}};

U_BOOT_DRIVER(sstar_rtc) = {
    .name       = "sstar,rtcpwc",
    .id         = UCLASS_RTC,
    .of_match   = sstar_rtc_ids,
    .of_to_plat = sstar_rtc_of_to_plat,
    .plat_auto  = sizeof(struct rtc_t),
    .probe      = sstar_rtc_probe,
    .flags      = DM_FLAG_PRE_RELOC,
    .ops        = &sstar_rtc_ops,
};
