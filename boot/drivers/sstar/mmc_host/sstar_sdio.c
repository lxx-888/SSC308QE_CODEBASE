/*
 * sstar_sdio.c - Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
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

#include <mmc.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <errno.h>

#include <linux/bug.h>
#include <linux/delay.h>
#include "hal_card_base.h"
#include "hal_sdmmc_v5.h"
#include "hal_card_platform.h"
#include "hal_card_timer.h"
#include <memalign.h>
#include "sstar_sdio.h"

/*
 * SDIO function devices
 */
struct sdio_func
{
    struct sdio_card *  card;        /* the card this device belongs to */
    sdio_irq_handler_t *irq_handler; /* IRQ callback */
    unsigned int        num;         /* function number */

    unsigned char class;   /* standard interface class */
    unsigned short vendor; /* vendor id */
    unsigned short device; /* device id */

    unsigned max_blksize; /* maximum block size */
    unsigned cur_blksize; /* current block size */

    unsigned enable_timeout; /* max enable timeout in msec */

    unsigned int state;             /* function state */
#define SDIO_STATE_PRESENT (1 << 0) /* present in sysfs */

    u8 *tmpbuf; /* DMA:able scratch buffer */

    u8           major_rev; /* major revision number */
    u8           minor_rev; /* minor revision number */
    unsigned     num_info;  /* number of info strings */
    const char **info;      /* info strings */

    struct sdio_func_tuple *tuples;
};

struct sstar_mmc_plat
{
    struct mmc_config cfg;
    struct mmc        mmc;
    struct sdio_card  card;
};

int sstar_sdio_bind(struct udevice *dev, struct mmc *mmc, const struct mmc_config *cfg)
{
    if (!mmc_get_ops(dev))
        return -ENOSYS;

    mmc->cfg  = cfg;
    mmc->priv = dev;
    /* the following chunk was from mmc_register() */

    /* Setup dsr related values */
    mmc->dsr_imp = 0;
    mmc->dsr     = 0xffffffff;

    mmc->dev             = dev;
    mmc->user_speed_mode = MMC_MODES_END;
    return 0;
}

static int sdio_io_rw_direct_host(struct mmc *mmc, int write, unsigned fn, unsigned addr, u8 in, u8 *out)
{
    struct mmc_cmd cmd = {};
    int            err;

    if (fn > 7)
        return -EINVAL;

    /* sanity check */
    if (addr & ~0x1FFFF)
        return -EINVAL;

    cmd.cmdidx = SD_IO_RW_DIRECT;
    cmd.cmdarg = write ? 0x80000000 : 0x00000000;
    cmd.cmdarg |= fn << 28;
    cmd.cmdarg |= (write && out) ? 0x08000000 : 0x00000000;
    cmd.cmdarg |= addr << 9;
    cmd.cmdarg |= in;
    cmd.resp_type = MMC_RSP_R5;

    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err)
    {
        return err;
    }

    if (cmd.response[0] & R5_ERROR)
        return -EIO;
    if (cmd.response[0] & R5_FUNCTION_NUMBER)
        return -EINVAL;
    if (cmd.response[0] & R5_OUT_OF_RANGE)
        return -ERANGE;

    if (out)
        *out = cmd.response[0] & 0xFF;

    return 0;
}

int sdio_reset(struct mmc *mmc)
{
    int ret;
    u8  abort;

    /* SDIO Simplified Specification V2.0, 4.4 Reset for SDIO */
    ret = sdio_io_rw_direct_host(mmc, 0, 0, SDIO_CCCR_ABORT, 0, &abort);
    if (ret)
        abort = 0x08;
    else
        abort |= 0x08;

    return sdio_io_rw_direct_host(mmc, 1, 0, SDIO_CCCR_ABORT, abort, NULL);
}

int sdio_send_io_op_cond(struct mmc *mmc, u32 ocr, u32 *rocr)
{
    struct mmc_cmd cmd = {};
    int            i, err = 0;

    cmd.cmdidx    = SD_IO_SEND_OP_COND;
    cmd.cmdarg    = ocr;
    cmd.resp_type = MMC_RSP_R4;

    for (i = 100; i; i--)
    {
        mdelay(10);
        err = mmc_send_cmd(mmc, &cmd, NULL);
        if (err)
            break;

        /* if we're just probing, do a single pass */
        if (ocr == 0)
            break;

        if (cmd.response[0] & MMC_CARD_BUSY)
            break;

        err = -ETIMEDOUT;
    }

    if (rocr)
        *rocr = cmd.response[0];

    return err;
}

u32 sdio_select_voltage(struct mmc *mmc, u32 ocr)
{
    int bit;

    /*
     * Sanity check the voltages that the card claims to
     * support.
     */
    if (ocr & 0x7F)
    {
        pr_warn("card claims to support voltages below defined range\n");
        ocr &= ~0x7F;
    }

    ocr &= mmc->cfg->voltages;
    if (!ocr)
    {
        pr_warn("no support for card's volts\n");
        return 0;
    }

    bit = fls(ocr) - 1;
    ocr &= 3 << bit;
    if (bit != fls(mmc->cfg->voltages) - 1)
        pr_warn("exceeding card's volts\n");

    return ocr;
}

static int sdio_set_signal_voltage(struct mmc *mmc, uint signal_voltage)
{
    int err;

    if (mmc->signal_voltage == signal_voltage)
        return 0;

    mmc->signal_voltage = signal_voltage;
    err                 = mmc_set_ios(mmc);
    if (err)
        pr_debug("unable to set voltage (err %d)\n", err);

    return err;
}

static uint sdio_mode2freq(struct mmc *mmc, enum bus_mode mode)
{
    static const int freqs[] = {
        [MMC_LEGACY] = 25000000,     [MMC_HS] = 26000000,      [SD_HS] = 50000000,       [MMC_HS_52] = 52000000,
        [MMC_DDR_52] = 52000000,     [UHS_SDR12] = 25000000,   [UHS_SDR25] = 50000000,   [UHS_SDR50] = 100000000,
        [UHS_DDR50] = 50000000,      [UHS_SDR104] = 208000000, [MMC_HS_200] = 200000000, [MMC_HS_400] = 200000000,
        [MMC_HS_400_ES] = 200000000,
    };

    if (mode == MMC_LEGACY)
        return mmc->legacy_speed;
    else if (mode >= MMC_MODES_END)
        return 0;
    else
        return freqs[mode];
}

static inline bool sdio_is_mode_ddr(enum bus_mode mode)
{
    if (mode == MMC_DDR_52)
        return true;
#if CONFIG_IS_ENABLED(MMC_UHS_SUPPORT)
    else if (mode == UHS_DDR50)
        return true;
#endif
#if CONFIG_IS_ENABLED(MMC_HS400_SUPPORT)
    else if (mode == MMC_HS_400)
        return true;
#endif
#if CONFIG_IS_ENABLED(MMC_HS400_ES_SUPPORT)
    else if (mode == MMC_HS_400_ES)
        return true;
#endif
    else
        return false;
}

const char *sdio_mode_name(enum bus_mode mode)
{
    static const char *const names[] = {
        [MMC_LEGACY]    = "MMC legacy",
        [MMC_HS]        = "MMC High Speed (26MHz)",
        [SD_HS]         = "SD High Speed (50MHz)",
        [UHS_SDR12]     = "UHS SDR12 (25MHz)",
        [UHS_SDR25]     = "UHS SDR25 (50MHz)",
        [UHS_SDR50]     = "UHS SDR50 (100MHz)",
        [UHS_SDR104]    = "UHS SDR104 (208MHz)",
        [UHS_DDR50]     = "UHS DDR50 (50MHz)",
        [MMC_HS_52]     = "MMC High Speed (52MHz)",
        [MMC_DDR_52]    = "MMC DDR52 (52MHz)",
        [MMC_HS_200]    = "HS200 (200MHz)",
        [MMC_HS_400]    = "HS400 (200MHz)",
        [MMC_HS_400_ES] = "HS400ES (200MHz)",
    };

    if (mode >= MMC_MODES_END)
        return "Unknown mode";
    else
        return names[mode];
}

static int sdio_select_mode(struct mmc *mmc, enum bus_mode mode)
{
    mmc->selected_mode = mode;
    mmc->tran_speed    = sdio_mode2freq(mmc, mode);
    mmc->ddr_mode      = sdio_is_mode_ddr(mode);
    pr_debug("selecting mode %s (freq : %d MHz)\n", sdio_mode_name(mode), mmc->tran_speed / 1000000);
    return 0;
}

void sdio_set_bus_width(struct mmc *mmc, unsigned int width)
{
    mmc->bus_width = width;
    mmc_set_ios(mmc);
}

static void sdio_set_initial_state(struct mmc *mmc)
{
    int err;

    /* First try to set 3.3V. If it fails set to 1.8V */
    err = sdio_set_signal_voltage(mmc, MMC_SIGNAL_VOLTAGE_330);
    if (err != 0)
        err = sdio_set_signal_voltage(mmc, MMC_SIGNAL_VOLTAGE_180);
    if (err != 0)
        pr_warn("mmc: failed to set signal voltage\n");

    sdio_select_mode(mmc, MMC_LEGACY);
    sdio_set_bus_width(mmc, 1);
    mmc_set_clock(mmc, 0, MMC_CLK_ENABLE);
}

static int sdio_go_idle(struct mmc *mmc)
{
    struct mmc_cmd cmd;
    int            err;

    udelay(1000);

    cmd.cmdidx    = MMC_CMD_GO_IDLE_STATE;
    cmd.cmdarg    = 0;
    cmd.resp_type = MMC_RSP_NONE;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err)
        return err;

    udelay(2000);

    return 0;
}

int sdio_get_op_cond(struct mmc *mmc, bool quiet)
{
    bool uhs_en = supports_uhs(mmc->cfg->host_caps);
    int  err;
    u32  ocr, rocr, ocr_card;

    if (mmc->has_init)
        return 0;

    if (uhs_en)
    {
        pr_warn("host did not support uhs now!\n");
    }
    mmc->ddr_mode = 0;

    sdio_set_initial_state(mmc);

    // reset card
    sdio_reset(mmc);
    /* go idle */
    sdio_go_idle(mmc);

    // cmd5
    err = sdio_send_io_op_cond(mmc, 0, &ocr);
    if (err)
        return err;

    ocr_card = ocr;
    // host & device ocr
    ocr = sdio_select_voltage(mmc, ocr);
    if (!ocr)
        return err;

    // confirm
    err = sdio_send_io_op_cond(mmc, ocr, &rocr);
    if (err)
        return err;

    mmc->cardtype = MMC_TYPE_SDIO;
    mmc->ocr      = ocr_card;

    return err;
}

int sdio_start_init(struct mmc *mmc)
{
    bool no_card;
    int  err = 0;

    /*
     * all hosts are capable of 1 bit bus-width and able to use the legacy
     * timings.
     */
    mmc->host_caps = mmc->cfg->host_caps | MMC_CAP(MMC_LEGACY) | MMC_MODE_1BIT;

#if CONFIG_IS_ENABLED(DM_MMC)
    mmc_deferred_probe(mmc);
#endif
#if !defined(CONFIG_MMC_BROKEN_CD)
    no_card = mmc_getcd(mmc) == 0;
#else
    no_card = 0;
#endif
#if !CONFIG_IS_ENABLED(DM_MMC)
    /* we pretend there's no card when init is NULL */
    no_card = no_card || (mmc->cfg->ops->init == NULL);
#endif
    if (no_card)
    {
        mmc->has_init = 0;
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
        pr_err("MMC: no card present\n");
#endif
        return -ENOMEDIUM;
    }

    err = sdio_get_op_cond(mmc, false);

    if (!err)
        mmc->init_in_progress = 1;

    return err;
}

int sdio_send_relative_addr(struct mmc *mmc, unsigned int *rca)
{
    int            err;
    struct mmc_cmd cmd = {};

    cmd.cmdidx    = SD_SEND_RELATIVE_ADDR;
    cmd.cmdarg    = 0;
    cmd.resp_type = MMC_RSP_R6 | MMC_CMD_BCR;

    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err)
        return err;

    *rca = cmd.response[0] >> 16;

    return 0;
}

static int sdio_select_card(struct mmc *mmc)
{
    struct mmc_cmd cmd = {};

    cmd.cmdidx    = MMC_CMD_SELECT_CARD;
    cmd.cmdarg    = mmc->rca << 16;
    cmd.resp_type = MMC_RSP_R1 | MMC_CMD_AC;

    return mmc_send_cmd(mmc, &cmd, NULL);
}

static int sdio_read_cccr(struct mmc *mmc)
{
    struct sstar_mmc_plat *plat = dev_get_plat(mmc->dev);
    struct sdio_card *     card = &plat->card;
    int                    ret;
    int                    cccr_vsn;
    unsigned char          data;
    unsigned char          speed;
    pr_debug("sdio: sdio_read_cccr >>>>>>>>\n");

    ret = sdio_io_rw_direct_host(mmc, 0, 0, SDIO_CCCR_CCCR, 0, &data);
    if (ret)
        goto out;

    cccr_vsn = data & 0x0f;

    if (cccr_vsn > SDIO_CCCR_REV_3_00)
    {
        pr_err("sdio: unrecognised CCCR structure version %d\n", cccr_vsn);
        return -EINVAL;
    }

    card->cccr.sdio_vsn = (data & 0xf0) >> 4;

    ret = sdio_io_rw_direct_host(mmc, 0, 0, SDIO_CCCR_CAPS, 0, &data);
    if (ret)
        goto out;

    if (data & SDIO_CCCR_CAP_SMB)
        card->cccr.multi_block = 1;
    if (data & SDIO_CCCR_CAP_LSC)
        card->cccr.low_speed = 1;
    if (data & SDIO_CCCR_CAP_4BLS)
        card->cccr.wide_bus = 1;

    if (cccr_vsn >= SDIO_CCCR_REV_1_10)
    {
        ret = sdio_io_rw_direct_host(mmc, 0, 0, SDIO_CCCR_POWER, 0, &data);
        if (ret)
            goto out;

        if (data & SDIO_POWER_SMPC)
            card->cccr.high_power = 1;
    }

    if (cccr_vsn >= SDIO_CCCR_REV_1_20)
    {
        ret = sdio_io_rw_direct_host(mmc, 0, 0, SDIO_CCCR_SPEED, 0, &speed);
        if (ret)
            goto out;

        card->scr.sda_spec3        = 0;
        card->sw_caps.sd3_bus_mode = 0;
        card->sw_caps.sd3_drv_type = 0;

        /* if no uhs mode ensure we check for high speed */
        if (!card->sw_caps.sd3_bus_mode)
        {
            if (speed & SDIO_SPEED_SHS)
            {
                card->cccr.high_speed    = 1;
                card->sw_caps.hs_max_dtr = 50000000;
            }
            else
            {
                card->cccr.high_speed    = 0;
                card->sw_caps.hs_max_dtr = 25000000;
            }
        }
    }

    card->mmc       = mmc;
    card->card_type = MMC_TYPE_SDIO;

    ret = sdio_io_rw_direct_host(mmc, 0, 0, SDIO_CCCR_IO_ENABLE, 0, &data);
    if (ret)
        goto out;

    ret = sdio_io_rw_direct_host(mmc, 0, 0, SDIO_CCCR_IO_READY, 0, &data);
    if (ret)
        goto out;

    card->func_enable = data;

    ret = sdio_io_rw_direct_host(mmc, 0, 0, SDIO_CCCR_FN0_BLKSIZE, 0, &data);
    if (ret)
        goto out;

    card->curr_blksize = data;

    ret = sdio_io_rw_direct_host(mmc, 0, 0, SDIO_CCCR_FN0_BLKSIZE + 1, 0, &data);
    if (ret)
        goto out;

    card->curr_blksize += (data << 8);

    pr_debug("sdio: sdio_read_cccr <<<<<<\n");
out:
    return ret;
}

static const unsigned char speed_val[16] = {0, 10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80};
static const unsigned int  speed_unit[8] = {10000, 100000, 1000000, 10000000, 0, 0, 0, 0};

typedef int(tpl_parse_t)(struct sdio_card *, struct sdio_func *, const unsigned char *, unsigned);

struct cis_tpl
{
    unsigned char code;
    unsigned char min_size;
    tpl_parse_t * parse;
};

static int cis_tpl_parse(struct sdio_card *card, struct sdio_func *func, const char *tpl_descr,
                         const struct cis_tpl *tpl, int tpl_count, unsigned char code, const unsigned char *buf,
                         unsigned size)
{
    int i, ret;

    /* look for a matching code in the table */
    for (i = 0; i < tpl_count; i++, tpl++)
    {
        if (tpl->code == code)
            break;
    }
    if (i < tpl_count)
    {
        if (size >= tpl->min_size)
        {
            if (tpl->parse)
                ret = tpl->parse(card, func, buf, size);
            else
                ret = -EILSEQ; /* known tuple, not parsed */
        }
        else
        {
            /* invalid tuple */
            ret = -EINVAL;
        }
        if (ret && ret != -EILSEQ && ret != -ENOENT)
        {
            pr_err("sdio: bad %s tuple 0x%02x (%u bytes)\n", tpl_descr, code, size);
        }
    }
    else
    {
        /* unknown tuple */
        ret = -ENOENT;
    }

    return ret;
}

#define SDIO_READ_CIS_TIMEOUT_MS (10 * 1000 * 1000) /* 10s */

static int cistpl_vers_1(struct sdio_card *card, struct sdio_func *func, const unsigned char *buf, unsigned size)
{
    u8       major_rev, minor_rev;
    unsigned i, nr_strings;
    char **  buffer, *string;

    if (size < 2)
        return 0;

    major_rev = buf[0];
    minor_rev = buf[1];

    /* Find all null-terminated (including zero length) strings in
       the TPLLV1_INFO field. Trailing garbage is ignored. */
    buf += 2;
    size -= 2;

    nr_strings = 0;
    for (i = 0; i < size; i++)
    {
        if (buf[i] == 0xff)
            break;
        if (buf[i] == 0)
            nr_strings++;
    }
    if (nr_strings == 0)
        return 0;

    size = i;

    buffer = malloc(sizeof(char *) * nr_strings + size);
    if (!buffer)
        return -ENOMEM;

    string = (char *)(buffer + nr_strings);

    for (i = 0; i < nr_strings; i++)
    {
        buffer[i] = string;
        strcpy(string, buf);
        string += strlen(string) + 1;
        buf += strlen(buf) + 1;
    }

    if (func)
    {
        func->major_rev = major_rev;
        func->minor_rev = minor_rev;
        func->num_info  = nr_strings;
        func->info      = (const char **)buffer;
    }
    else
    {
        card->major_rev = major_rev;
        card->minor_rev = minor_rev;
        card->num_info  = nr_strings;
        card->info      = (const char **)buffer;
    }

    return 0;
}

static int cistpl_manfid(struct sdio_card *card, struct sdio_func *func, const unsigned char *buf, unsigned size)
{
    unsigned int vendor, device;

    /* TPLMID_MANF */
    vendor = buf[0] | (buf[1] << 8);

    /* TPLMID_CARD */
    device = buf[2] | (buf[3] << 8);

    if (func)
    {
        func->vendor = vendor;
        func->device = device;
    }
    else
    {
        card->cis.vendor = vendor;
        card->cis.device = device;
    }

    return 0;
}

static int cistpl_funce_common(struct sdio_card *card, struct sdio_func *func, const unsigned char *buf, unsigned size)
{
    /* Only valid for the common CIS (function 0) */
    if (func)
        return -EINVAL;

    /* TPLFE_FN0_BLK_SIZE */
    card->cis.blksize = buf[1] | (buf[2] << 8);

    /* TPLFE_MAX_TRAN_SPEED */
    card->cis.max_dtr = speed_val[(buf[3] >> 3) & 15] * speed_unit[buf[3] & 7];

    return 0;
}

static int cistpl_funce_func(struct sdio_card *card, struct sdio_func *func, const unsigned char *buf, unsigned size)
{
    unsigned vsn;
    unsigned min_size;

    /* Only valid for the individual function's CIS (1-7) */
    if (!func)
        return -EINVAL;

    /*
     * This tuple has a different length depending on the SDIO spec
     * version.
     */
    vsn      = func->card->cccr.sdio_vsn;
    min_size = (vsn == SDIO_SDIO_REV_1_00) ? 28 : 42;

    if (size == 28 && vsn == SDIO_SDIO_REV_1_10)
    {
        pr_warn("sdio: card has broken SDIO 1.1 CIS, forcing SDIO 1.0\n");
        vsn = SDIO_SDIO_REV_1_00;
    }
    else if (size < min_size)
    {
        return -EINVAL;
    }

    /* TPLFE_MAX_BLK_SIZE */
    func->max_blksize = buf[12] | (buf[13] << 8);

    /* TPLFE_ENABLE_TIMEOUT_VAL, present in ver 1.1 and above */
    if (vsn > SDIO_SDIO_REV_1_00)
        func->enable_timeout = (buf[28] | (buf[29] << 8)) * 10;
    else
        func->enable_timeout = 100000; // jiffies_to_msecs(HZ);

    return 0;
}

static const struct cis_tpl cis_tpl_funce_list[] = {
    {0x00, 4, cistpl_funce_common},
    {0x01, 0, cistpl_funce_func},
    {0x04, 1 + 1 + 6, /* CISTPL_FUNCE_LAN_NODE_ID */},
};

static int cistpl_funce(struct sdio_card *card, struct sdio_func *func, const unsigned char *buf, unsigned size)
{
    if (size < 1)
        return -EINVAL;

    return cis_tpl_parse(card, func, "CISTPL_FUNCE", cis_tpl_funce_list, ARRAY_SIZE(cis_tpl_funce_list), buf[0], buf,
                         size);
}

/* Known TPL_CODEs table for CIS tuples */
static const struct cis_tpl cis_tpl_list[] = {
    {0x15, 3, cistpl_vers_1}, {0x20, 4, cistpl_manfid},         {0x21, 2, /* cistpl_funcid */},
    {0x22, 0, cistpl_funce},  {0x91, 2, /* cistpl_sdio_std */},
};

static int sdio_read_cis(struct mmc *mmc, struct sdio_func *func)
{
    struct sstar_mmc_plat *plat = dev_get_plat(mmc->dev);
    struct sdio_card *     card = &plat->card;

    int ret;
    struct sdio_func_tuple *this, **prev;
    unsigned i, ptr = 0;
    card->tuples = NULL;

    /*
     * Note that this works for the common CIS (function number 0) as
     * well as a function's CIS * since SDIO_CCCR_CIS and SDIO_FBR_CIS
     * have the same offset.
     */
    for (i = 0; i < 3; i++)
    {
        unsigned char x, fn;

        if (func)
            fn = func->num;
        else
            fn = 0;

        ret = sdio_io_rw_direct_host(mmc, 0, 0, SDIO_FBR_BASE(fn) + SDIO_FBR_CIS + i, 0, &x);
        if (ret)
            return ret;
        ptr |= x << (i * 8);
    }

    if (func)
        prev = &func->tuples;
    else
        prev = &card->tuples;

    if (*prev)
        return -EINVAL;

    do
    {
        unsigned char tpl_code, tpl_link;
        unsigned long timeout = timer_get_us() + SDIO_READ_CIS_TIMEOUT_MS;

        ret = sdio_io_rw_direct_host(mmc, 0, 0, ptr++, 0, &tpl_code);
        if (ret)
            break;

        /* 0xff means we're done */
        if (tpl_code == 0xff)
            break;

        /* null entries have no link field or data */
        if (tpl_code == 0x00)
            continue;

        ret = sdio_io_rw_direct_host(mmc, 0, 0, ptr++, 0, &tpl_link);
        if (ret)
            break;

        /* a size of 0xff also means we're done */
        if (tpl_link == 0xff)
            break;

        this = malloc(sizeof(*this) + tpl_link);
        if (!this)
            return -ENOMEM;

        for (i = 0; i < tpl_link; i++)
        {
            ret = sdio_io_rw_direct_host(mmc, 0, 0, ptr + i, 0, &this->data[i]);
            if (ret)
                break;
        }
        if (ret)
        {
            free(this);
            break;
        }

        /* Try to parse the CIS tuple */
        ret = cis_tpl_parse(card, func, "CIS", cis_tpl_list, ARRAY_SIZE(cis_tpl_list), tpl_code, this->data, tpl_link);
        if (ret == -EILSEQ || ret == -ENOENT)
        {
            /*
             * The tuple is unknown or known but not parsed.
             * Queue the tuple for the function driver.
             */
            this->next = NULL;
            this->code = tpl_code;
            this->size = tpl_link;
            *prev      = this;
            prev       = &this->next;

            if (ret == -ENOENT)
            {
                if (time_after(timer_get_us(), timeout))
                    break;
                /* warn about unknown tuples */
                pr_warn(
                    "sdio: queuing unknown"
                    " CIS tuple 0x%02x (%u bytes)\n",

                    tpl_code, tpl_link);
            }

            /* keep on analyzing tuples */
            ret = 0;
        }
        else
        {
            /*
             * We don't need the tuple anymore if it was
             * successfully parsed by the SDIO core or if it is
             * not going to be queued for a driver.
             */
            free(this);
        }

        ptr += tpl_link;
    } while (!ret);

    /*
     * Link in all unknown tuples found in the common CIS so that
     * drivers don't have to go digging in two places.
     */
    if (func)
        *prev = card->tuples;

    return ret;
}

int sdio_read_common_cis(struct mmc *mmc)
{
    return sdio_read_cis(mmc, NULL);
}

static int sdio_switch_hs(struct mmc *mmc, int enable)
{
    struct sstar_mmc_plat *plat = dev_get_plat(mmc->dev);
    struct sdio_card *     card = &plat->card;

    int ret;
    u8  speed;

    if (!(mmc->host_caps & MMC_CAP_SD_HIGHSPEED))
        return 0;

    if (!card->cccr.high_speed)
        return 0;

    ret = sdio_io_rw_direct_host(mmc, 0, 0, SDIO_CCCR_SPEED, 0, &speed);
    if (ret)
        return ret;

    if (enable)
        speed |= SDIO_SPEED_EHS;
    else
        speed &= ~SDIO_SPEED_EHS;

    ret = sdio_io_rw_direct_host(mmc, 1, 0, SDIO_CCCR_SPEED, speed, NULL);
    if (ret)
        return ret;

    return 1;
}

static int sdio_enable_hs(struct mmc *mmc)
{
    return sdio_switch_hs(mmc, true);
}

void sdio_set_timing(struct mmc *mmc, unsigned int timing)
{
    mmc->card_caps |= MMC_CAP(timing);
    mmc_set_ios(mmc);
}

static unsigned sdio_get_max_clock(struct mmc *mmc)
{
    struct sstar_mmc_plat *plat = dev_get_plat(mmc->dev);
    struct sdio_card *     card = &plat->card;

    unsigned max_dtr;

    if (mmc->card_caps == MMC_CAP_SD_HIGHSPEED)
    {
        /*
         * The SDIO specification doesn't mention how
         * the CIS transfer speed register relates to
         * high-speed, but it seems that 50 MHz is
         * mandatory.
         */
        max_dtr = 50000000;
    }
    else
    {
        max_dtr = card->cis.max_dtr;
    }

    return max_dtr;
}

static int sdio_enable_wide(struct mmc *mmc)
{
    struct sstar_mmc_plat *plat = dev_get_plat(mmc->dev);
    struct sdio_card *     card = &plat->card;

    int ret;
    u8  ctrl;

    if (!(mmc->host_caps & MMC_MODE_4BIT))
        return 0;

    if (card->cccr.low_speed && !card->cccr.wide_bus)
        return 0;

    ret = sdio_io_rw_direct_host(mmc, 0, 0, SDIO_CCCR_IF, 0, &ctrl);
    if (ret)
        return ret;

    if ((ctrl & SDIO_BUS_WIDTH_MASK) == SDIO_BUS_WIDTH_RESERVED)
        pr_warn("sdio: SDIO_CCCR_IF is invalid: 0x%02x\n", ctrl);

    /* set as 4-bit bus width */
    ctrl &= ~SDIO_BUS_WIDTH_MASK;
    ctrl |= SDIO_BUS_WIDTH_4BIT;

    ret = sdio_io_rw_direct_host(mmc, 1, 0, SDIO_CCCR_IF, ctrl, NULL);
    if (ret)
        return ret;

    return 1;
}

static int sdio_enable_4bit_bus(struct mmc *mmc)
{
    int err;

    err = sdio_enable_wide(mmc);
    if (err <= 0)
        return err;

    sdio_set_bus_width(mmc, MMC_BUS_WIDTH_4);

    return 0;
}

struct sdio_func *sdio_alloc_func(struct sdio_card *card)
{
    struct sdio_func *func;

    func = malloc(sizeof(struct sdio_func));
    if (!func)
        return ERR_PTR(-ENOMEM);

    /*
     * allocate buffer separately to make sure it's properly aligned for
     * DMA usage (incl. 64 bit DMA)
     */
    func->tmpbuf = malloc(4);
    if (!func->tmpbuf)
    {
        free(func);
        return ERR_PTR(-ENOMEM);
    }

    func->card   = card;
    func->tuples = NULL;
    return func;
}

static inline int sdio_card_nonstd_func_interface(const struct sdio_card *c)
{
    return c->quirks & MMC_QUIRK_NONSTD_FUNC_IF;
}

static int sdio_read_fbr(struct sdio_func *func)
{
    int           ret;
    unsigned char data = 0;

    if (sdio_card_nonstd_func_interface(func->card))
    {
        func->class = 0x00;
        return 0;
    }

    ret = sdio_io_rw_direct_host(func->card->mmc, 0, 0, SDIO_FBR_BASE(func->num) + SDIO_FBR_STD_IF, 0, &data);
    if (ret)
        goto out;

    data &= 0x0f;

    if (data == 0x0f)
    {
        ret = sdio_io_rw_direct_host(func->card->mmc, 0, 0, SDIO_FBR_BASE(func->num) + SDIO_FBR_STD_IF_EXT, 0, &data);
        if (ret)
            goto out;
    }

    func->class = data;

    ret = sdio_io_rw_direct_host(func->card->mmc, 0, 0, SDIO_FBR_BASE(func->num) + SDIO_FBR_IO_BLKSIZE, 0, &data);
    if (ret)
        goto out;
    func->cur_blksize = data;

    ret = sdio_io_rw_direct_host(func->card->mmc, 0, 0, SDIO_FBR_BASE(func->num) + SDIO_FBR_IO_BLKSIZE + 1, 0, &data);
    if (ret)
        goto out;
    func->cur_blksize += data << 8;

out:
    return ret;
}

int sdio_read_func_cis(struct sdio_func *func)
{
    int ret;

    ret = sdio_read_cis(func->card->mmc, func);
    if (ret)
        return ret;

    /*
     * Vendor/device id is optional for function CIS, so
     * copy it from the card structure as needed.
     */
    if (func->vendor == 0)
    {
        func->vendor = func->card->cis.vendor;
        func->device = func->card->cis.device;
    }

    return 0;
}

static void sdio_remove_func(struct sdio_func *func)
{
    if (!func)
        return;

    if (func->tmpbuf)
        free(func->tmpbuf);

    free(func);
}

static int sdio_init_func(struct sdio_card *card, unsigned int fn)
{
    int               ret;
    struct sdio_func *func;

    if (WARN_ON(fn > SDIO_MAX_FUNCS))
        return -EINVAL;

    func = sdio_alloc_func(card);
    if (IS_ERR(func))
        return PTR_ERR(func);

    func->num = fn;

    if (!(card->quirks & MMC_QUIRK_NONSTD_SDIO))
    {
        ret = sdio_read_fbr(func);
        if (ret)
            goto fail;

        ret = sdio_read_func_cis(func);
        if (ret)
            goto fail;
    }
    else
    {
        func->vendor      = func->card->cis.vendor;
        func->device      = func->card->cis.device;
        func->max_blksize = func->card->cis.blksize;
    }

    card->sdio_func[fn - 1] = func;

    return 0;

fail:
    sdio_remove_func(func);
    return ret;
}

static int sdio_init_cardfunc(struct mmc *mmc)
{
    struct sstar_mmc_plat *plat = dev_get_plat(mmc->dev);
    struct sdio_card *     card = &plat->card;
    int                    err, i, funcs;

    funcs            = (mmc->ocr & 0x70000000) >> 28;
    card->sdio_funcs = 0;
    pr_debug("sdio_init_cardfunc >>>>>");
    /*
     * Initialize (but don't add) all present functions.
     */
    for (i = 0; i < funcs; i++, card->sdio_funcs++)
    {
        err = sdio_init_func(card, i + 1);
        if (err)
            return err;
    }
    pr_debug("sdio_init_cardfunc <<<<<<<");

    return 0;
}

static int sdio_complete_init(struct mmc *mmc)
{
    int          err = 0;
    unsigned int rca;
    mmc->init_in_progress = 0;

    // set rca
    err = sdio_send_relative_addr(mmc, &rca);
    if (err)
        goto fail;
    mmc->rca = (ushort)rca;
    // select card
    err = sdio_select_card(mmc);
    if (err)
        goto fail;

    err = sdio_read_cccr(mmc);
    if (err)
        goto fail;

    err = sdio_read_common_cis(mmc);
    if (err)
        goto fail;
    err = sdio_enable_hs(mmc);
    if (err > 0)
        sdio_set_timing(mmc, MMC_TIMING_SD_HS);
    else if (err)
        goto fail;

    mmc_set_clock(mmc, sdio_get_max_clock(mmc), false);

    err = sdio_enable_4bit_bus(mmc);
    if (err)
        goto fail;

    err = sdio_init_cardfunc(mmc);
    if (err)
        goto fail;

fail:
    if (err)
        mmc->has_init = 0;
    else
        mmc->has_init = 1;
    return err;
}

int sdio_init(struct mmc *mmc)
{
    int err = 0;
#if CONFIG_IS_ENABLED(DM_MMC)
    struct mmc_uclass_priv *upriv = dev_get_uclass_priv(mmc->dev);

    upriv->mmc = mmc;
#endif
    if (mmc->has_init)
        return 0;

    if (!mmc->init_in_progress)
        err = sdio_start_init(mmc);

    if (!err)
        err = sdio_complete_init(mmc);
    if (err)
        pr_info("%s: %d\n", __func__, err);

    return err;
}

int sdio_set_block_size(struct sdio_func *func, unsigned blksz)
{
    int ret;

    if (blksz == func->cur_blksize)
    {
        return 0;
    }

    if (blksz > 512)
    {
        return -EINVAL;
    }

    if (blksz == 0)
    {
        blksz = min(func->max_blksize, 512u);
        blksz = min(blksz, 512u);
    }

    ret =
        sdio_io_rw_direct_host(func->card->mmc, 1, 0, SDIO_FBR_BASE(func->num) + SDIO_FBR_BLKSIZE, blksz & 0xff, NULL);
    if (ret)
        return ret;
    ret = sdio_io_rw_direct_host(func->card->mmc, 1, 0, SDIO_FBR_BASE(func->num) + SDIO_FBR_BLKSIZE + 1,
                                 (blksz >> 8) & 0xff, NULL);
    if (ret)
        return ret;
    func->cur_blksize = blksz;
    return 0;
}

// Read single byte
int sdio_read_byte(u8 devidx, u8 func, u32 addr, u8 *r_buf)
{
    struct sstar_mmc_plat *plat;
    struct sdio_card *     card;

    int         err;
    struct mmc *mmc;

    if (get_mmc_num() <= 0)
    {
        pr_err("No sdio device available\n");
        return 1;
    }

    mmc = find_mmc_device(devidx);
    if (!mmc)
    {
        printf("no sdio device at slot %x\n", devidx);
        return SDIO_RET_FAILURE;
    }

    if (!(mmc->has_init))
    {
        printf("Do sdio init first!\n");
        if (sdio_init(mmc))
            return SDIO_RET_FAILURE;
        mmc->has_init = 1;
    }

    plat = dev_get_plat(mmc->dev);
    card = &plat->card;

    if (func < 0 || func > card->sdio_funcs)
    {
        printf("[SDIO]%s - ERROR: function %d is out of range, and total is %d !\n", __func__, func, card->sdio_funcs);
        return -1;
    }

    err = sdio_io_rw_direct_host(mmc, 0, func, addr, 0, r_buf);
    if (err)
    {
        return -1;
    }

    return 0;
}

// Write single byte
int sdio_write_byte(u8 devidx, u8 func, u32 addr, u8 w_value)
{
    struct sstar_mmc_plat *plat;
    struct sdio_card *     card;

    int         err;
    struct mmc *mmc;

    if (get_mmc_num() <= 0)
    {
        pr_err("No sdio device available\n");
        return 1;
    }

    mmc = find_mmc_device(devidx);
    if (!mmc)
    {
        printf("no sdio device at slot %x\n", devidx);
        return SDIO_RET_FAILURE;
    }

    if (!(mmc->has_init))
    {
        printf("Do sdio init first!\n");
        if (sdio_init(mmc))
            return SDIO_RET_FAILURE;
        mmc->has_init = 1;
    }

    plat = dev_get_plat(mmc->dev);
    card = &plat->card;

    if (func < 0 || func > card->sdio_funcs)
    {
        printf("[SDIO]%s - ERROR: function %d is out of range, and total is %d !\n", __func__, func, card->sdio_funcs);
        return -1;
    }

    err = sdio_io_rw_direct_host(mmc, 1, func, addr, w_value, NULL);
    if (err)
    {
        return -1;
    }

    return 0;
}

int sdio_enable_func(struct sdio_func *func)
{
    int err, i = 0;
    u8  reg;
    //  unsigned long timeout;

    if (!func)
        return -EINVAL;

    if ((func->card->func_enable) & (1 << func->num))
    {
        return 0;
    }

    err = sdio_io_rw_direct_host(func->card->mmc, 0, 0, SDIO_CCCR_IO_ENABLE, 0, &reg);
    if (err)
    {
        goto err;
    }

    reg |= 1 << func->num;
    err = sdio_io_rw_direct_host(func->card->mmc, 1, 0, SDIO_CCCR_IO_ENABLE, reg, NULL);
    if (err)
    {
        goto err;
    }

    while (1)
    {
        err = sdio_io_rw_direct_host(func->card->mmc, 0, 0, SDIO_CCCR_IO_READY, 0, &reg);
        if (err)
            goto err;
        if (reg & (1 << func->num))
            break;

        if (i++ > 100)
            return -1;
        msleep(10);
    }
    func->card->func_enable |= (1 << func->num);
    return 0;

err:
    printf("SDIO: Failed to enable device %d\n", func->num);
    return err;
}

int sdio_io_rw_extended(struct sdio_card *card, int write, unsigned fn, unsigned addr, int incr_addr, u8 *buf,
                        unsigned blocks, unsigned blksz)
{
    struct mmc_cmd  cmd;
    struct mmc_data data;
    int             err;

    WARN_ON(blksz == 0);

    /* sanity check */
    if (addr & ~0x1FFFF)
        return -EINVAL;

    cmd.cmdidx = SD_IO_RW_EXTENDED;
    cmd.cmdarg = write ? 0x80000000 : 0x00000000;
    cmd.cmdarg |= fn << 28;
    cmd.cmdarg |= incr_addr ? 0x04000000 : 0x00000000;
    cmd.cmdarg |= addr << 9;
    if (blocks == 0)
        cmd.cmdarg |= (blksz == 512) ? 0 : blksz; /* byte mode */
    else
        cmd.cmdarg |= 0x08000000 | blocks; /* block mode */
    cmd.resp_type = MMC_RSP_R5;

    data.blocksize = blksz;
    /* Code in host drivers/fwk assumes that "blocks" always is >=1 */
    data.blocks = blocks ? blocks : 1;
    data.flags  = write ? MMC_DATA_WRITE : MMC_DATA_READ;
    if (write)
        data.src = buf;
    else
        data.dest = buf;

    err = mmc_send_cmd(card->mmc, &cmd, &data);
    if (err)
    {
        printf("sdio data transfer failed\n");
        return err;
    }
    /*
    else if (cmd.response[0] & R5_ERROR)
        err = -EIO;
    else if (cmd.response[0] & R5_FUNCTION_NUMBER)
        err = -EINVAL;
    else if (cmd.response[0] & R5_OUT_OF_RANGE)
        err = -ERANGE;
    */
    return err;
}

static int sdio_io_rw_ext_helper(struct sdio_func *func, int write, unsigned addr, int incr_addr, u8 *buf,
                                 unsigned size)
{
    unsigned remainder = size;
    unsigned max_blocks;
    int      ret;
    if (!func || func->card->sdio_funcs > 7)
        return -EINVAL;

    /* Do the bulk of the transfer using block mode (if supported). */
    if (func->card->cccr.multi_block)
    {
        /* Blocks per command is limited by host count, host transfer
         * size and the maximum for IO_RW_EXTENDED of 511 blocks. */
        max_blocks = min(func->card->mmc->cfg->b_max, 511u);

        while (func->cur_blksize && remainder >= func->cur_blksize)
        {
            unsigned blocks;

            blocks = remainder / func->cur_blksize;
            if (blocks > max_blocks)
                blocks = max_blocks;
            size = blocks * func->cur_blksize;

            ret = sdio_io_rw_extended(func->card, write, func->num, addr, incr_addr, buf, blocks, func->cur_blksize);
            if (ret)
                return ret;

            remainder -= size;
            buf += size;
            if (incr_addr)
                addr += size;
        }
    }

    /* Write the remainder using byte mode. */
    while (remainder > 0)
    {
        size = min(remainder, 512u);

        /* Indicate byte mode by setting "blocks" = 0 */
        ret = sdio_io_rw_extended(func->card, write, func->num, addr, incr_addr, buf, 0, size);
        if (ret)
            return ret;

        remainder -= size;
        buf += size;
        if (incr_addr)
            addr += size;
    }
    return 0;
}

int sdio_read_multi(u8 devidx, u8 func, u32 addr, u32 count, u8 *r_buf)
{
    struct sstar_mmc_plat *plat;
    struct sdio_card *     card;

    int         err;
    struct mmc *mmc;

    if (get_mmc_num() <= 0)
    {
        pr_err("No sdio device available\n");
        return 1;
    }

    mmc = find_mmc_device(devidx);
    if (!mmc)
    {
        printf("no sdio device at slot %x\n", devidx);
        return SDIO_RET_FAILURE;
    }

    if (!(mmc->has_init))
    {
        printf("Do sdio init first!\n");
        if (sdio_init(mmc))
            return SDIO_RET_FAILURE;
        mmc->has_init = 1;
    }

    plat = dev_get_plat(mmc->dev);
    card = &plat->card;

    if (func < 0 || func > card->sdio_funcs)
    {
        printf("[SDIO]%s - ERROR: function %d is out of range, and total is %d !\n", __func__, func, card->sdio_funcs);
        return -1;
    }

    // 1 ~ 512
    if ((count < 1) || (count > SDIO_MAX_BLOCK_COUNT * SDIO_MAX_BLOCK_SIZE))
    {
        printf("[SDIO]%s - ERR, count = %d\n", __func__, count);
        return -1;
    }

    //
    if (count == (SDIO_MAX_BLOCK_COUNT * SDIO_MAX_BLOCK_SIZE))
    {
        count = 0;
    }

    if (func && (func <= card->sdio_funcs))
    {
        if (sdio_enable_func(card->sdio_func[func - 1]))
            return -1;
    }
    //

    err = sdio_io_rw_ext_helper(card->sdio_func[func - 1], 0, addr, 0, r_buf, count);
    if (err)
    {
        return -1;
    }

    return 0;
}

int sdio_write_multi(u8 devidx, u8 func, u32 addr, u32 count, u8 *w_buf)
{
    struct sstar_mmc_plat *plat;
    struct sdio_card *     card;

    int         err;
    struct mmc *mmc;

    if (get_mmc_num() <= 0)
    {
        pr_err("No sdio device available\n");
        return 1;
    }

    mmc = find_mmc_device(devidx);
    if (!mmc)
    {
        printf("no sdio device at slot %x\n", devidx);
        return SDIO_RET_FAILURE;
    }

    if (!(mmc->has_init))
    {
        printf("Do sdio init first!\n");
        if (sdio_init(mmc))
            return SDIO_RET_FAILURE;
        mmc->has_init = 1;
    }

    plat = dev_get_plat(mmc->dev);
    card = &plat->card;

    if (func < 0 || func > card->sdio_funcs)
    {
        printf("[SDIO]%s - ERROR: function %d is out of range, and total is %d !\n", __func__, func, card->sdio_funcs);
        return -1;
    }
    // 1 ~ 512
    if ((count < 1) || (count > SDIO_MAX_BLOCK_COUNT * SDIO_MAX_BLOCK_SIZE))
    {
        printf("[SDIO]%s - ERR, count = %d\n", __func__, count);
        return -1;
    }

    //
    if (count == (SDIO_MAX_BLOCK_COUNT * SDIO_MAX_BLOCK_SIZE))
    {
        count = 0;
    }
    if (func && (func <= card->sdio_funcs))
    {
        if (sdio_enable_func(card->sdio_func[func - 1]))
            return -1;
    }
    //
    err = sdio_io_rw_ext_helper(card->sdio_func[func - 1], 1, addr, 0, w_buf, count);
    if (err)
    {
        return -1;
    }

    return 0;
}

void sstar_sdio_rw_test(u8 devidx, struct mmc *mmc)
{
#if 1
    struct sstar_mmc_plat *plat = dev_get_plat(mmc->dev);
    struct sdio_card *     card = &plat->card;

    unsigned int   test_loop = 0, i = 0;
    unsigned short ret;
    u8 *           r_buf, *w_buf;
    u8             func;
    u32            addr;
    u32            count;
    u32            blk_size;
#endif

#if 1
    //
    r_buf = malloc_cache_aligned(512 * 2);
    w_buf = malloc_cache_aligned(512 * 2);

    while (test_loop < 1)
    {
        printf("--- SDIO Test Loop %d ---\r\n", test_loop);

        func = 0;
        addr = 0x13;
        ret  = sdio_read_byte(devidx, func, addr, r_buf);
        if (ret)
        {
            printf("- MSSDIO_ReadByte ERROR\r\n");
        }
        else
        {
            printf("- MSSDIO_ReadByte [0] = 0x%x\r\n", r_buf[0]);
        }

        //###############################################
        //          MSSDIO_WriteByte Test
        //###############################################
        mdelay(20);
        w_buf[0] = r_buf[0];

        func = 0;
        addr = 0x13;
        ret  = sdio_write_byte(devidx, func, addr, w_buf[0]);
        if (ret)
        {
            printf("- MSSDIO_WriteByte ERROR\r\n");
        }
        else
        {
            printf("- MSSDIO_WriteByte PASS\r\n");
        }
#endif

#if 1
        //###############################################
        //          MSSDIO_WriteMultibyte Test
        //###############################################
        mdelay(20);
        func  = 1;
        addr  = 40; // 16
        count = 4;

        for (i = 0; i < count; i++)
        {
            w_buf[i] = test_loop + i;
        }

        ret = sdio_write_multi(devidx, func, addr, count, w_buf);
        if (ret)
        {
            printf("- MSSDIO_WriteByteMulti ERROR\r\n");
        }
        else
        {
            printf("- MSSDIO_WriteByteMulti PASS\r\n");
        }
#endif

#if 1
        //###############################################
        //          MSSDIO_ReadByteMulti Test
        //###############################################
        mdelay(20);
        func  = 1;
        addr  = (1 << 15);
        count = 4;

        ret = sdio_read_multi(devidx, func, addr, count, r_buf);
        if (ret)
        {
            printf("- MSSDIO_ReadByteMulti ERROR\r\n");
        }
        else
        {
            printf("- MSSDIO_ReadByteMulti PASS\r\n");
            for (i = 0; i < count; i++)
            {
                printf("- [%d] = 0x%x\r\n", i, r_buf[i]);
            }
        }
#endif

#if 1
        //###############################################
        //          MSSDIO_WriteBlockMulti Test
        //###############################################
        mdelay(20);
        func     = 1;
        addr     = (1 << 15); // 1;
        count    = 1;         // 12; // 12 2;
        blk_size = 512;

        for (i = 0; i < (count * blk_size); i++)
        {
            w_buf[i] = test_loop + i;
        }

        if ((func && (func <= card->sdio_funcs) && (blk_size != card->sdio_func[func - 1]->cur_blksize))
            || ((func == 0) && blk_size != card->curr_blksize)) // func 0
        {
            if (sdio_set_block_size(card->sdio_func[func - 1], blk_size))
            {
                printf("- MSSDIO_WriteBlockMulti SETBZ ERROR\r\n");
                goto fail;
            }
        }

        ret = sdio_write_multi(devidx, func, addr, (count * blk_size), w_buf);
        if (ret)
        {
            printf("- MSSDIO_WriteBlockMulti ERROR\r\n");
        }
        else
        {
            printf("- MSSDIO_WriteBlockMulti PASS\r\n");
        }
#endif

#if 1
        //###############################################
        //          MSSDIO_ReadBlockMulti Test
        //###############################################
        mdelay(20);
        func     = 1;
        addr     = 4 + 16 + 32; // 1;
        count    = 1;
        blk_size = 512;

        if ((func && (func <= card->sdio_funcs) && (blk_size != card->sdio_func[func - 1]->cur_blksize))
            || ((func == 0) && blk_size != card->curr_blksize)) // func 0
        {
            if (sdio_set_block_size(card->sdio_func[func - 1], blk_size))
            {
                printf("- MSSDIO_WriteBlockMulti SETBZ ERROR\r\n");
                goto fail;
            }
        }

        ret = sdio_read_multi(devidx, func, addr, (count * blk_size), r_buf);
        if (ret)
        {
            printf("- MSSDIO_ReadBlockMulti ERROR\r\n");
        }
        else
        {
            printf("- MSSDIO_ReadBlockMulti PASS\r\n");
            for (i = 0; i < (count * blk_size); i++)
            {
                pr_debug("- [%d] = 0x%x\r\n", i, r_buf[i]);
            }
        }
#endif

#if 1
        //###############################################
        //     MSSDIO_WriteBlockMulti and set blksz Test
        //###############################################
        mdelay(20);
        func     = 1;
        addr     = (1 << 15); // 1;
        count    = 2;         // 12; // 12 2;
        blk_size = 256;

        for (i = 0; i < (count * blk_size); i++)
        {
            w_buf[i] = test_loop + i;
        }

        if ((func && (func <= card->sdio_funcs) && (blk_size != card->sdio_func[func - 1]->cur_blksize))
            || ((func == 0) && blk_size != card->curr_blksize)) // func 0
        {
            if (sdio_set_block_size(card->sdio_func[func - 1], blk_size))
            {
                printf("- MSSDIO_WriteBlockMulti SETBZ ERROR\r\n");
                goto fail;
            }
        }

        ret = sdio_write_multi(devidx, func, addr, (count * blk_size), w_buf);
        if (ret)
        {
            printf("- MSSDIO_WriteBlockMulti AND SETBZ ERROR\r\n");
        }
        else
        {
            printf("- MSSDIO_WriteBlockMulti AND SETBZ PASS\r\n");
        }
#endif

#if 1
        //###############################################
        // MSSDIO_ReadBlockMulti and set blocksize Test
        //###############################################
        mdelay(20);
        func     = 1;
        addr     = 4 + 16 + 32; // 1;
        count    = 2;
        blk_size = 256;

        if ((func && (func <= card->sdio_funcs) && (blk_size != card->sdio_func[func - 1]->cur_blksize))
            || ((func == 0) && blk_size != card->curr_blksize)) // func 0
        {
            if (sdio_set_block_size(card->sdio_func[func - 1], blk_size))
            {
                printf("- MSSDIO_ReadBlockMulti SETBZ ERROR\r\n");
                goto fail;
            }
        }

        ret = sdio_read_multi(devidx, func, addr, (count * blk_size), r_buf);
        if (ret)
        {
            printf("- MSSDIO_ReadBlockMulti AND SETBZ ERROR\r\n");
        }
        else
        {
            printf("- MSSDIO_ReadBlockMulti AND SETBZ PASS\r\n");
            for (i = 0; i < (count * blk_size); i++)
            {
                pr_debug("- [%d] = 0x%x\r\n", i, r_buf[i]);
            }
        }
#endif

        //
        test_loop++;
    };

fail:
    free(r_buf);
    free(w_buf);
}
