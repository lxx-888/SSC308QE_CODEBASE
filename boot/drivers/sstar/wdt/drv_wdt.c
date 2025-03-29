/*
 * drv_wdt.c - Sigmastar
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

/*
 * The Watchdog Timer Mode Register can be only written to once. If the
 * timeout need to be set from U-Boot, be sure that the bootstrap doesn't
 * write to this register. Inform Linux to it too
 */

#include <common.h>
#include <wdt.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <sstar_types.h>
#include <platform.h>
#include <io.h>
#ifdef CONFIG_SSTAR_CLK
#include <clk.h>
#endif
#ifdef CONFIG_WDT
#include <dm.h>
#endif
#include <hal_wdt.h>

// #define SSTAR_WDT_DEBUG 1

#ifdef SSTAR_WDT_DEBUG
#define wdt_dbg(fmt, ...) printf("[sstar-wdt] " fmt, ##__VA_ARGS__)
#define wdt_err(fmt, ...) printf("[sstar-wdt] " fmt, ##__VA_ARGS__)
#else
#define wdt_dbg(fmt, ...) \
    do                    \
    {                     \
    } while (0)
#define wdt_err(fmt, ...) printf("[sstar-wdt] " fmt, ##__VA_ARGS__)
#endif

struct sstar_wdt
{
    struct hal_wdt_data hal_data;
    struct clk          clk;
    unsigned long       clk_rate;
};

#ifdef CONFIG_WDT // for DM driver
static int sstar_wdt_reset(struct udevice *dev)
{
    struct sstar_wdt *wdt = dev_get_priv(dev);

    hal_wdt_ping(&wdt->hal_data);

    return 0;
}

static int sstar_wdt_stop(struct udevice *dev)
{
    struct sstar_wdt *wdt = dev_get_priv(dev);

    wdt_dbg("stop\n");
    hal_wdt_stop(&wdt->hal_data);

    return 0;
}

static int sstar_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
    struct sstar_wdt * wdt = dev_get_priv(dev);
    unsigned long long ticks;

    if (timeout_ms < 5000)
    {
        ticks = wdt->clk_rate * 5;
        wdt_dbg("start:%llu ms(%#llx)\n", 5000ULL, ticks);
    }
    else
    {
        ticks = wdt->clk_rate * timeout_ms / 1000;
        wdt_dbg("start:%llu ms(%#llx)\n", timeout_ms, ticks);
    }

    hal_wdt_start(&wdt->hal_data, -1ULL, ticks);
    hal_wdt_ping(&wdt->hal_data);

    return 0;
}

static int sstar_wdt_probe(struct udevice *dev)
{
    struct sstar_wdt *wdt = dev_get_priv(dev);

    wdt->hal_data.reg_base = dev_read_addr_ptr(dev);
    if (!wdt->hal_data.reg_base)
        return -ENOENT;

#ifdef CONFIG_SSTAR_CLK
    if (clk_get_by_index(dev, 0, &wdt->clk))
    {
        wdt_err("get clk fail, using fix clk rate\n");
        wdt->clk_rate = CONFIG_WDT_CLOCK;
    }
    else
    {
        wdt->clk_rate = clk_get_rate(&wdt->clk);
    }
#else
    wdt->clk_rate = CONFIG_WDT_CLOCK;
#endif

    wdt_dbg("base:%p clk_rate:%lu probed.\n", wdt->hal_data.reg_base, wdt->clk_rate);

    return 0;
}

static const struct wdt_ops sstar_wdt_ops = {
    .start = sstar_wdt_start,
    .reset = sstar_wdt_reset,
    .stop  = sstar_wdt_stop,
};

static const struct udevice_id sstar_wdt_ids[] = {{.compatible = "sstar,wdt"}, {}};

U_BOOT_DRIVER(sstar_wdt) = {
    .name      = "sstar_wdt",
    .id        = UCLASS_WDT,
    .of_match  = sstar_wdt_ids,
    .priv_auto = sizeof(struct sstar_wdt),
    .probe     = sstar_wdt_probe,
    .ops       = &sstar_wdt_ops,
    .flags     = DM_FLAG_PRE_RELOC,
};

#else
/*
 * The timer watchdog can be set between
 * 0.5 and 128 Seconds. If not defined
 * in configuration file, sets 128 Seconds
 */
#ifndef CONFIG_WATCHDOG_TIMEOUT_MSECS
#define CONFIG_WATCHDOG_TIMEOUT_MSECS 128000
#endif

static struct sstar_wdt wdt;

void hw_watchdog_reset(void)
{
    hal_wdt_ping(&wdt.hal_data);

    return;
}

void hw_watchdog_init(void)
{
    unsigned long long ticks;

    wdt.hal_data.reg_base = BASE_REG_WDT_PA;
    wdt.clk_rate          = CONFIG_WDT_CLOCK;

    ticks = wdt.clk_rate * CONFIG_WATCHDOG_TIMEOUT_MSECS / 1000;

    wdt_dbg("init&start:%llu ms(%#llx)\n", CONFIG_WATCHDOG_TIMEOUT_MSECS, ticks);

    hal_wdt_start(&wdt.hal_data, -1ULL, ticks);
    hal_wdt_ping(&wdt.hal_data);

    return;
}
#endif // end CONFIG_WDT
