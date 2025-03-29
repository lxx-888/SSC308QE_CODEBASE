/*
 * sstar_usb2_phy_debugfs.c - Sigmastar
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

#include <linux/bitfield.h>
#include <linux/debugfs.h>
#include <linux/of_address.h>
#include "sstar_usb2_phy_debugfs.h"
#include "io.h"

static uint tx_swing = 0x0;
module_param(tx_swing, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(tx_swing, "driver current, in decimal format");

static uint dem_cur = 0x0;
module_param(dem_cur, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(dem_cur, "dem current, in decimal format");

static uint cm_cur = 0x0;
module_param(cm_cur, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(cm_cur, "VCM current, in decimal format");

static int sstar_u2phy_utmi_ls_cross_rise_show(struct seq_file *s, void *unused)
{
    struct sstar_u2phy *priv = s->private;
    struct regmap *     utmi = priv->utmi;
    unsigned long       flags;
    u32                 ls_cross_level;

    spin_lock_irqsave(&priv->lock, flags);
    regmap_read(utmi, (0x49 << 2), &ls_cross_level);
    spin_unlock_irqrestore(&priv->lock, flags);
    seq_printf(s, "ls_cross_rise(0x00~0x7f) = 0x%lx\r\n", FIELD_GET(GENMASK(14, 8), ls_cross_level));

    return 0;
}

static int sstar_u2phy_utmi_ls_cross_rise_open(struct inode *inode, struct file *file)
{
    return single_open(file, sstar_u2phy_utmi_ls_cross_rise_show, inode->i_private);
}

static ssize_t sstar_u2phy_utmi_ls_cross_rise_write(struct file *file, const char __user *ubuf, size_t count,
                                                    loff_t *ppos)
{
    struct seq_file *   s    = file->private_data;
    struct sstar_u2phy *priv = s->private;
    struct regmap *     utmi = priv->utmi;
    unsigned long       flags;
    char                buf[32] = {0};
    u32                 ls_cross_level;
    int                 ret;

    if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
        return -EFAULT;

    ret = kstrtouint(buf, 0, &ls_cross_level);
    if (ret)
    {
        return ret;
    }

    spin_lock_irqsave(&priv->lock, flags);
    regmap_update_bits(utmi, (0x49 << 2), GENMASK(14, 8), FIELD_PREP(GENMASK(14, 8), ls_cross_level));
    spin_unlock_irqrestore(&priv->lock, flags);

    return count;
}

static const struct file_operations sstar_ls_cross_rise_fops = {
    .open    = sstar_u2phy_utmi_ls_cross_rise_open,
    .write   = sstar_u2phy_utmi_ls_cross_rise_write,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static int sstar_u2phy_utmi_ls_cross_fall_show(struct seq_file *s, void *unused)
{
    struct sstar_u2phy *priv = s->private;
    struct regmap *     utmi = priv->utmi;
    unsigned long       flags;
    u32                 ls_cross_level;

    spin_lock_irqsave(&priv->lock, flags);
    regmap_read(utmi, (0x49 << 2), &ls_cross_level);
    spin_unlock_irqrestore(&priv->lock, flags);
    seq_printf(s, "ls_cross_fall(0x00~0x7f) = 0x%lx\r\n", FIELD_GET(GENMASK(6, 0), ls_cross_level));

    return 0;
}

static int sstar_u2phy_utmi_ls_cross_fall_open(struct inode *inode, struct file *file)
{
    return single_open(file, sstar_u2phy_utmi_ls_cross_fall_show, inode->i_private);
}

static ssize_t sstar_u2phy_utmi_ls_cross_fall_write(struct file *file, const char __user *ubuf, size_t count,
                                                    loff_t *ppos)
{
    struct seq_file *   s    = file->private_data;
    struct sstar_u2phy *priv = s->private;
    struct regmap *     utmi = priv->utmi;
    unsigned long       flags;
    char                buf[32] = {0};
    u32                 ls_cross_level;
    int                 ret;

    if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
        return -EFAULT;

    ret = kstrtouint(buf, 0, &ls_cross_level);
    if (ret)
    {
        return ret;
    }

    spin_lock_irqsave(&priv->lock, flags);
    regmap_update_bits(utmi, (0x49 << 2), GENMASK(6, 0), FIELD_PREP(GENMASK(6, 0), ls_cross_level));
    spin_unlock_irqrestore(&priv->lock, flags);

    return count;
}

static const struct file_operations sstar_ls_cross_fall_fops = {
    .open    = sstar_u2phy_utmi_ls_cross_fall_open,
    .write   = sstar_u2phy_utmi_ls_cross_fall_write,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static int sstar_u2phy_utmi_cm_cur_show(struct seq_file *s, void *unused)
{
    struct sstar_u2phy *priv = s->private;
    struct regmap *     utmi = priv->utmi;

    unsigned long flags;
    u32           cm_cur;
    int           bit_masks;

    spin_lock_irqsave(&priv->lock, flags);
    regmap_read(utmi, (0x17 << 2), &cm_cur);
    spin_unlock_irqrestore(&priv->lock, flags);

    bit_masks = BIT(3) | BIT(4) | BIT(5);
    cm_cur    = (bit_masks & cm_cur) >> 3;
    switch (cm_cur)
    {
        case 0x03:
            cm_cur = 105;
            break;
        case 0x02:
            cm_cur = 110;
            break;
        case 0x01:
        case 0x07:
            cm_cur = 115;
            break;
        case 0x00:
        case 0x06:
            cm_cur = 120;
            break;
        case 0x05:
            cm_cur = 125;
            break;
        case 0x04:
            cm_cur = 130;
            break;
        default:
            seq_printf(s, "cm_current: unknown\r\n");
            return 0;
    }
    seq_printf(s, "cm_current: %d%%\r\n", cm_cur);
    return 0;
}

static int sstar_u2phy_utmi_cm_cur_open(struct inode *inode, struct file *file)
{
    return single_open(file, sstar_u2phy_utmi_cm_cur_show, inode->i_private);
}

static ssize_t sstar_u2phy_utmi_cm_cur_set(struct sstar_u2phy *priv, unsigned int cm_cur)
{
    struct regmap *utmi = priv->utmi;
    unsigned long  flags;
    int            bit_masks;

    switch (cm_cur)
    {
        case 105:
            cm_cur = 0x03;
            break;
        case 110:
            cm_cur = 0x02;
            break;
        case 115:
            cm_cur = 0x01;
            break;
        case 120:
            cm_cur = 0x00;
            break;
        case 125:
            cm_cur = 0x05;
            break;
        case 130:
            cm_cur = 0x04;
            break;
        default:
            dev_err(&priv->phy->dev, "Unsupported option: %d\n", cm_cur);
            dev_err(&priv->phy->dev, "CM Current option should be: 105/110/115/120/125/130\n");
            return -EINVAL;
    }

    spin_lock_irqsave(&priv->lock, flags);
    bit_masks = BIT(3) | BIT(4) | BIT(5);
    regmap_update_bits(utmi, (0x17 << 2), bit_masks, (cm_cur << 3));
    spin_unlock_irqrestore(&priv->lock, flags);
    return 0;
}

static ssize_t sstar_u2phy_utmi_cm_cur_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
    struct seq_file *   s       = file->private_data;
    struct sstar_u2phy *priv    = s->private;
    char                buf[32] = {0};
    u32                 cm_cur;
    int                 ret;

    if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
        return -EFAULT;

    ret = kstrtouint(buf, 0, &cm_cur);
    if (ret)
    {
        return ret;
    }

    ret = sstar_u2phy_utmi_cm_cur_set(priv, cm_cur);
    if (ret)
    {
        return ret;
    }

    return count;
}

static const struct file_operations sstar_cm_cur_fops = {
    .open    = sstar_u2phy_utmi_cm_cur_open,
    .write   = sstar_u2phy_utmi_cm_cur_write,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static int sstar_u2phy_utmi_dem_cur_show(struct seq_file *s, void *unused)
{
    struct sstar_u2phy *priv = s->private;
    struct regmap *     utmi = priv->utmi;

    unsigned long flags;
    u32           dem_cur;
    int           bit_masks;

    spin_lock_irqsave(&priv->lock, flags);
    regmap_read(utmi, (0x16 << 2), &dem_cur);
    spin_unlock_irqrestore(&priv->lock, flags);

    bit_masks = (BIT(7) | BIT(8) | BIT(9));
    dem_cur   = (bit_masks & dem_cur) >> 7;
    switch (dem_cur)
    {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
            dem_cur = 100;
            break;
        case 0x04:
            dem_cur = 105;
            break;
        case 0x05:
            dem_cur = 110;
            break;
        case 0x06:
            dem_cur = 115;
            break;
        case 0x07:
            dem_cur = 120;
            break;
        default:
            seq_printf(s, "de_emphasis_current: unknown\r\n");
            return 0;
    }
    seq_printf(s, "de_emphasis_current: %d%%\r\n", dem_cur);
    return 0;
}

static int sstar_u2phy_utmi_dem_cur_open(struct inode *inode, struct file *file)
{
    return single_open(file, sstar_u2phy_utmi_dem_cur_show, inode->i_private);
}

static ssize_t sstar_u2phy_utmi_dem_cur_set(struct sstar_u2phy *priv, unsigned int dem_cur)
{
    struct regmap *utmi = priv->utmi;
    unsigned long  flags;
    int            bit_masks;

    switch (dem_cur)
    {
        case 100:
            dem_cur = 0x00;
            break;
        case 105:
            dem_cur = 0x04;
            break;
        case 110:
            dem_cur = 0x05;
            break;
        case 115:
            dem_cur = 0x06;
            break;
        case 120:
            dem_cur = 0x07;
            break;
        default:
            dev_err(&priv->phy->dev, "Unsupported option: %d\n", dem_cur);
            dev_err(&priv->phy->dev, "De-emphasis Current option should be: 100/105/110/115/120\n");
            return -EINVAL;
    }

    spin_lock_irqsave(&priv->lock, flags);
    bit_masks = (BIT(7) | BIT(8) | BIT(9));
    regmap_update_bits(utmi, (0x16 << 2), bit_masks, (dem_cur << 7));
    spin_unlock_irqrestore(&priv->lock, flags);
    return 0;
}

static ssize_t sstar_u2phy_utmi_dem_cur_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
    struct seq_file *   s       = file->private_data;
    struct sstar_u2phy *priv    = s->private;
    char                buf[32] = {0};
    u32                 dem_cur;
    int                 ret;

    if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
        return -EFAULT;

    ret = kstrtouint(buf, 0, &dem_cur);
    if (ret)
    {
        return ret;
    }

    ret = sstar_u2phy_utmi_dem_cur_set(priv, dem_cur);
    if (ret)
    {
        return ret;
    }

    return count;
}

static const struct file_operations sstar_dem_cur_fops = {
    .open    = sstar_u2phy_utmi_dem_cur_open,
    .write   = sstar_u2phy_utmi_dem_cur_write,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static int sstar_u2phy_utmi_tx_swing_show(struct seq_file *s, void *unused)
{
    struct sstar_u2phy *priv = s->private;
    struct regmap *     utmi = priv->utmi;

    unsigned long flags;
    u32           tx_swing;
    int           bit_masks;

    spin_lock_irqsave(&priv->lock, flags);
    regmap_read(utmi, (0x16 << 2), &tx_swing);
    spin_unlock_irqrestore(&priv->lock, flags);
    bit_masks = (BIT(4) | BIT(5) | BIT(6));
    tx_swing  = (bit_masks & tx_swing) >> 4;

    switch (tx_swing)
    {
        case 0x04:
            tx_swing = 80;
            break;
        case 0x05:
            tx_swing = 85;
            break;
        case 0x06:
            tx_swing = 90;
            break;
        case 0x07:
            tx_swing = 95;
            break;
        case 0x00:
            tx_swing = 100;
            break;
        case 0x01:
            tx_swing = 105;
            break;
        case 0x02:
            tx_swing = 110;
            break;
        case 0x03:
            tx_swing = 115;
            break;
        default:
            seq_printf(s, "tx_swing: unknown\r\n");
            return 0;
    }

    seq_printf(s, "tx_swing: %d%%\r\n", tx_swing);
    return 0;
}

static int sstar_u2phy_utmi_tx_swing_open(struct inode *inode, struct file *file)
{
    return single_open(file, sstar_u2phy_utmi_tx_swing_show, inode->i_private);
}

static ssize_t sstar_u2phy_utmi_tx_swing_set(struct sstar_u2phy *priv, unsigned int tx_swing)
{
    struct regmap *utmi = priv->utmi;
    unsigned long  flags;
    int            bit_masks;

    switch (tx_swing)
    {
        case 80:
            tx_swing = 0x04;
            break;
        case 85:
            tx_swing = 0x05;
            break;
        case 90:
            tx_swing = 0x06;
            break;
        case 95:
            tx_swing = 0x07;
            break;
        case 100:
            tx_swing = 0x00;
            break;
        case 105:
            tx_swing = 0x01;
            break;
        case 110:
            tx_swing = 0x02;
            break;
        case 115:
            tx_swing = 0x03;
            break;
        default:
            dev_err(&priv->phy->dev, "Unsupported option: %d\n", tx_swing);
            dev_err(&priv->phy->dev, "Main Current(TX swing) option should be: 80/85/90/95/100/105/110/115\n");
            return -EINVAL;
    }

    spin_lock_irqsave(&priv->lock, flags);
    bit_masks = (BIT(4) | BIT(5) | BIT(6));
    regmap_update_bits(utmi, (0x16 << 2), bit_masks, (tx_swing << 4));
    spin_unlock_irqrestore(&priv->lock, flags);
    return 0;
}

static ssize_t sstar_u2phy_utmi_tx_swing_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
    struct seq_file *   s       = file->private_data;
    struct sstar_u2phy *priv    = s->private;
    char                buf[32] = {0};
    u32                 tx_swing;
    int                 ret;

    if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
        return -EFAULT;

    ret = kstrtouint(buf, 0, &tx_swing);
    if (ret)
    {
        return ret;
    }

    ret = sstar_u2phy_utmi_tx_swing_set(priv, tx_swing);
    if (ret)
    {
        return ret;
    }

    return count;
}

static const struct file_operations sstar_tx_swing_fops = {
    .open    = sstar_u2phy_utmi_tx_swing_open,
    .write   = sstar_u2phy_utmi_tx_swing_write,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static int sstar_u2phy_utmi_pre_emphasis_show(struct seq_file *s, void *unused)
{
    struct sstar_u2phy *priv = s->private;
    struct regmap *     utmi = priv->utmi;
    unsigned long       flags;
    u32                 val;

    spin_lock_irqsave(&priv->lock, flags);
    regmap_read(utmi, (0x43 << 2), &val);
    spin_unlock_irqrestore(&priv->lock, flags);

    seq_printf(s, "pre_emphasis(0~3): 0x%lx\r\n", FIELD_GET(GENMASK(9, 8), val));
    return 0;
}

static int sstar_u2phy_utmi_pre_emphasis_open(struct inode *inode, struct file *file)
{
    return single_open(file, sstar_u2phy_utmi_pre_emphasis_show, inode->i_private);
}

static ssize_t sstar_u2phy_utmi_pre_emphasis_set(struct sstar_u2phy *priv, unsigned int val)
{
    struct regmap *utmi = priv->utmi;
    unsigned long  flags;

    spin_lock_irqsave(&priv->lock, flags);
    regmap_update_bits(utmi, (0x43 << 2), GENMASK(9, 8), FIELD_PREP(GENMASK(9, 8), val));
    regmap_set_bits(utmi, (0x43 << 2), BIT(10));
    spin_unlock_irqrestore(&priv->lock, flags);
    return 0;
}

static ssize_t sstar_u2phy_utmi_pre_emphasis_write(struct file *file, const char __user *ubuf, size_t count,
                                                   loff_t *ppos)
{
    struct seq_file *   s       = file->private_data;
    struct sstar_u2phy *priv    = s->private;
    char                buf[32] = {0};
    u32                 val;
    int                 ret;

    if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
        return -EFAULT;

    ret = kstrtouint(buf, 0, &val);
    if (ret)
    {
        return ret;
    }

    ret = sstar_u2phy_utmi_pre_emphasis_set(priv, val);
    if (ret)
    {
        return ret;
    }

    return count;
}

static const struct file_operations sstar_pre_emphasis_fops = {
    .open    = sstar_u2phy_utmi_pre_emphasis_open,
    .write   = sstar_u2phy_utmi_pre_emphasis_write,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static int sstar_u2phy_utmi_slew_rate_show(struct seq_file *s, void *unused)
{
    struct sstar_u2phy *priv = s->private;
    struct regmap *     utmi = priv->utmi;
    unsigned long       flags;
    u32                 val;

    spin_lock_irqsave(&priv->lock, flags);
    regmap_read(utmi, (0x44 << 2), &val);
    spin_unlock_irqrestore(&priv->lock, flags);

    seq_printf(s, "slew_rate(0~3): 0x%lx\r\n", FIELD_GET(GENMASK(2, 1), val));
    return 0;
}

static int sstar_u2phy_utmi_slew_rate_open(struct inode *inode, struct file *file)
{
    return single_open(file, sstar_u2phy_utmi_slew_rate_show, inode->i_private);
}

static ssize_t sstar_u2phy_utmi_slew_rate_set(struct sstar_u2phy *priv, unsigned int val)
{
    struct regmap *utmi = priv->utmi;
    unsigned long  flags;

    spin_lock_irqsave(&priv->lock, flags);
    regmap_update_bits(utmi, (0x44 << 2), GENMASK(2, 1), FIELD_PREP(GENMASK(2, 1), val));
    regmap_set_bits(utmi, (0x44 << 2), BIT(3));
    spin_unlock_irqrestore(&priv->lock, flags);
    return 0;
}

static ssize_t sstar_u2phy_utmi_slew_rate_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
    struct seq_file *   s       = file->private_data;
    struct sstar_u2phy *priv    = s->private;
    char                buf[32] = {0};
    u32                 val;
    int                 ret;

    if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
        return -EFAULT;

    ret = kstrtouint(buf, 0, &val);
    if (ret)
    {
        return ret;
    }

    ret = sstar_u2phy_utmi_slew_rate_set(priv, val);
    if (ret)
    {
        return ret;
    }

    return count;
}

static const struct file_operations sstar_slew_rate_fops = {
    .open    = sstar_u2phy_utmi_slew_rate_open,
    .write   = sstar_u2phy_utmi_slew_rate_write,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static int sstar_u2phy_utmi_swing_trim_show(struct seq_file *s, void *unused)
{
    struct sstar_u2phy *priv = s->private;
    struct regmap *     utmi = priv->utmi;
    unsigned long       flags;
    u32                 val;

    spin_lock_irqsave(&priv->lock, flags);
    regmap_read(utmi, (0x44 << 2), &val);
    spin_unlock_irqrestore(&priv->lock, flags);

    seq_printf(s, "swing_trim(0~63): 0x%lx\r\n", FIELD_GET(GENMASK(9, 4), val));
    return 0;
}

static int sstar_u2phy_utmi_swing_trim_open(struct inode *inode, struct file *file)
{
    return single_open(file, sstar_u2phy_utmi_swing_trim_show, inode->i_private);
}

static ssize_t sstar_u2phy_utmi_swing_trim_set(struct sstar_u2phy *priv, unsigned int val)
{
    struct regmap *utmi = priv->utmi;
    unsigned long  flags;

    spin_lock_irqsave(&priv->lock, flags);
    regmap_update_bits(utmi, (0x44 << 2), GENMASK(9, 4), FIELD_PREP(GENMASK(9, 4), val));
    spin_unlock_irqrestore(&priv->lock, flags);
    return 0;
}

static ssize_t sstar_u2phy_utmi_swing_trim_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
    struct seq_file *   s       = file->private_data;
    struct sstar_u2phy *priv    = s->private;
    char                buf[32] = {0};
    u32                 val;
    int                 ret;

    if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
        return -EFAULT;

    ret = kstrtouint(buf, 0, &val);
    if (ret)
    {
        return ret;
    }

    ret = sstar_u2phy_utmi_swing_trim_set(priv, val);
    if (ret)
    {
        return ret;
    }

    return count;
}

static const struct file_operations sstar_swing_trim_fops = {
    .open    = sstar_u2phy_utmi_swing_trim_open,
    .write   = sstar_u2phy_utmi_swing_trim_write,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static int sstar_u2phy_utmi_disc_ref_vol_show(struct seq_file *s, void *unused)
{
    struct sstar_u2phy *priv = s->private;
    struct regmap *     utmi = priv->utmi;
    unsigned long       flags;
    u32                 val;

    spin_lock_irqsave(&priv->lock, flags);
    regmap_read(utmi, (0x41 << 2), &val);
    spin_unlock_irqrestore(&priv->lock, flags);

    seq_printf(s, "disconnect_refer_voltage(0~31): 0x%lx\r\n", FIELD_GET(GENMASK(4, 0), val));
    return 0;
}

static int sstar_u2phy_utmi_disc_ref_vol_open(struct inode *inode, struct file *file)
{
    return single_open(file, sstar_u2phy_utmi_disc_ref_vol_show, inode->i_private);
}

static ssize_t sstar_u2phy_utmi_disc_ref_vol_set(struct sstar_u2phy *priv, unsigned int val)
{
    struct regmap *utmi = priv->utmi;
    unsigned long  flags;

    spin_lock_irqsave(&priv->lock, flags);
    regmap_update_bits(utmi, (0x41 << 2), GENMASK(4, 0), FIELD_PREP(GENMASK(4, 0), val));
    spin_unlock_irqrestore(&priv->lock, flags);
    return 0;
}

static ssize_t sstar_u2phy_utmi_disc_ref_vol_write(struct file *file, const char __user *ubuf, size_t count,
                                                   loff_t *ppos)
{
    struct seq_file *   s       = file->private_data;
    struct sstar_u2phy *priv    = s->private;
    char                buf[32] = {0};
    u32                 val;
    int                 ret;

    if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
        return -EFAULT;

    ret = kstrtouint(buf, 0, &val);
    if (ret)
    {
        return ret;
    }

    ret = sstar_u2phy_utmi_disc_ref_vol_set(priv, val);
    if (ret)
    {
        return ret;
    }

    return count;
}

static const struct file_operations sstar_disc_ref_vol_fops = {
    .open    = sstar_u2phy_utmi_disc_ref_vol_open,
    .write   = sstar_u2phy_utmi_disc_ref_vol_write,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static int sstar_u2phy_utmi_squelch_ref_vol_show(struct seq_file *s, void *unused)
{
    struct sstar_u2phy *priv = s->private;
    struct regmap *     utmi = priv->utmi;
    unsigned long       flags;
    u32                 val;

    spin_lock_irqsave(&priv->lock, flags);
    regmap_read(utmi, (0x4B << 2), &val);
    spin_unlock_irqrestore(&priv->lock, flags);

    seq_printf(s, "squelch_refer_voltage(0~31): 0x%lx\r\n", FIELD_GET(GENMASK(4, 0), val));
    return 0;
}

static int sstar_u2phy_utmi_squelch_ref_vol_open(struct inode *inode, struct file *file)
{
    return single_open(file, sstar_u2phy_utmi_squelch_ref_vol_show, inode->i_private);
}

static ssize_t sstar_u2phy_utmi_squelch_ref_vol_set(struct sstar_u2phy *priv, unsigned int val)
{
    struct regmap *utmi = priv->utmi;
    unsigned long  flags;

    spin_lock_irqsave(&priv->lock, flags);
    regmap_update_bits(utmi, (0x4B << 2), GENMASK(4, 0), FIELD_PREP(GENMASK(4, 0), val));
    spin_unlock_irqrestore(&priv->lock, flags);
    return 0;
}

static ssize_t sstar_u2phy_utmi_squelch_ref_vol_write(struct file *file, const char __user *ubuf, size_t count,
                                                      loff_t *ppos)
{
    struct seq_file *   s       = file->private_data;
    struct sstar_u2phy *priv    = s->private;
    char                buf[32] = {0};
    u32                 val;
    int                 ret;

    if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
        return -EFAULT;

    ret = kstrtouint(buf, 0, &val);
    if (ret)
    {
        return ret;
    }

    ret = sstar_u2phy_utmi_squelch_ref_vol_set(priv, val);
    if (ret)
    {
        return ret;
    }

    return count;
}

static const struct file_operations sstar_squelch_ref_vol_fops = {
    .open    = sstar_u2phy_utmi_squelch_ref_vol_open,
    .write   = sstar_u2phy_utmi_squelch_ref_vol_write,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static int sstar_u2phy_edswitch_show(struct seq_file *s, void *unused)
{
    struct sstar_u2phy *priv = s->private;

    if (priv->ed_hs_switch_on)
        seq_printf(s, "eye-diagram mode: high speed on\n");
    else if (priv->ed_fs_switch_on)
        seq_printf(s, "eye-diagram mode: full speed on\n");
    else
        seq_printf(s, "eye-diagram mode: off\n");

    seq_printf(s, "\nNote: When switch eye-diagram mode on, the host is unable to work\n");
    seq_printf(s, "You must reboot to back to normal\n");

    return 0;
}

static int sstar_u2phy_edswitch_open(struct inode *inode, struct file *file)
{
    return single_open(file, sstar_u2phy_edswitch_show, inode->i_private);
}

static void sstar_u2phy_edswitch_set_v1(struct sstar_u2phy *priv, EYE_DIAGRAM_MODE_E speed)
{
    void __iomem *reg_bank1 = priv->ed_bank1;
    void __iomem *reg_bank2 = priv->ed_bank2;
    void __iomem *reg_bank3 = priv->ed_bank3;
    void __iomem *reg_bank4 = priv->ed_bank4;

    OUTREG8(GET_REG8_ADDR(reg_bank3, 0x00), 0xff);
    OUTREG8(GET_REG8_ADDR(reg_bank3, 0x02), 0x00);
    OUTREG8(GET_REG8_ADDR(reg_bank1, 0x00), 0xb0);
    OUTREG8(GET_REG8_ADDR(reg_bank1, 0x01), 0x10);
    OUTREG8(GET_REG8_ADDR(reg_bank1, 0x04), 0x10);
    OUTREG8(GET_REG8_ADDR(reg_bank1, 0x05), 0x01);
    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x08), 0x0f);
    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x09), 0x04);
    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x00), 0x05);
    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x01), 0x7f);
    OUTREG8(GET_REG8_ADDR(reg_bank4, 0xfe), 0xe1);
    OUTREG8(GET_REG8_ADDR(reg_bank4, 0xff), 0x08);
    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x08), 0x0f);
    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x09), 0x04);
    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x20), 0xa1);
    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x21), 0x80);
    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x22), 0x88);
    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x23), 0x20);

    if (EYE_DIAGRAM_MODE_HIGH_SPEED == speed)
    {
        OUTREG8(GET_REG8_ADDR(reg_bank2, 0x00), 0x03);
        OUTREG8(GET_REG8_ADDR(reg_bank2, 0x01), 0x6b);
        mdelay(1);
    }

    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x00), 0xc3);
    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x01), 0x69);
    mdelay(2);

    if (EYE_DIAGRAM_MODE_HIGH_SPEED == speed)
    {
        OUTREG8(GET_REG8_ADDR(reg_bank2, 0x00), 0x01);
        OUTREG8(GET_REG8_ADDR(reg_bank2, 0x01), 0x00);
    }
    else if (EYE_DIAGRAM_MODE_FULL_SPEED == speed)
    {
        OUTREG8(GET_REG8_ADDR(reg_bank2, 0x02), 0x84);
        OUTREG8(GET_REG8_ADDR(reg_bank2, 0x03), 0x90);
        OUTREG8(GET_REG8_ADDR(reg_bank2, 0x00), 0x01);
        OUTREG8(GET_REG8_ADDR(reg_bank2, 0x01), 0x02);
        OUTREG8(GET_REG8_ADDR(reg_bank2, 0x00), 0x01);
        OUTREG8(GET_REG8_ADDR(reg_bank2, 0x01), 0x00);
        mdelay(1);
    }
    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x3c), 0x01);
    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x3d), 0x00);
    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x3c), 0x00);
    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x3d), 0x00);

    if (EYE_DIAGRAM_MODE_HIGH_SPEED == speed)
        mdelay(2);
    else if (EYE_DIAGRAM_MODE_FULL_SPEED == speed)
        mdelay(5);

    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x10), 0x78);
    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x11), 0x00);
    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x06), 0x43);
    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x07), 0x00);
    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x06), 0x40);
    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x07), 0x00);

    if (EYE_DIAGRAM_MODE_HIGH_SPEED == speed)
    {
        OUTREG8(GET_REG8_ADDR(reg_bank2, 0x00), 0x01);
        OUTREG8(GET_REG8_ADDR(reg_bank2, 0x01), 0x00);
        OUTREG8(GET_REG8_ADDR(reg_bank2, 0x14), 0x00);
        OUTREG8(GET_REG8_ADDR(reg_bank2, 0x15), 0x06);
    }
    else if (EYE_DIAGRAM_MODE_FULL_SPEED == speed)
    {
        OUTREG8(GET_REG8_ADDR(reg_bank2, 0x00), 0xeb);
        OUTREG8(GET_REG8_ADDR(reg_bank2, 0x01), 0x00);
        OUTREG8(GET_REG8_ADDR(reg_bank2, 0x14), 0x40);
        OUTREG8(GET_REG8_ADDR(reg_bank2, 0x15), 0x07);
    }
    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x34), 0x00);
    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x35), 0x00);
    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x32), 0xfe);
    OUTREG8(GET_REG8_ADDR(reg_bank2, 0x33), 0x0b);

    if (EYE_DIAGRAM_MODE_HIGH_SPEED == speed)
        mdelay(2);
    else if (EYE_DIAGRAM_MODE_FULL_SPEED == speed)
        mdelay(170);
}

static void sstar_u2phy_edswitch_set_v2(struct sstar_u2phy *priv, EYE_DIAGRAM_MODE_E speed)
{
    void __iomem *reg_bank1 = priv->ed_bank1;
    void __iomem *reg_bank2 = priv->ed_bank2;
    void __iomem *reg_bank3 = priv->ed_bank3;

    if (EYE_DIAGRAM_MODE_HIGH_SPEED == speed)
    {
        OUTREG8(GET_REG8_ADDR(reg_bank1, 0x00), 0xc0);
        OUTREG8(GET_REG8_ADDR(reg_bank2, 0x00), 0x05);
        OUTREG8(GET_REG8_ADDR(reg_bank2, 0x01), 0x7f);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x08), 0xef);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x09), 0x04);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x0a), 0x00);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x0b), 0x40);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x08), 0xef);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x09), 0x04);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x0a), 0x40);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x0b), 0x40);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x08), 0x6f);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x09), 0x04);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x00), 0x05);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x01), 0x3f);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x00), 0x07);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x01), 0x3f);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x00), 0x47);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x01), 0x3f);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x00), 0x47);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x01), 0x3f);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x00), 0xc7);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x01), 0x3f);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x10), 0x40);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x11), 0x00);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x00), 0xc7);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x01), 0x3f);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x10), 0xc0);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x11), 0x00);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x06), 0x24);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x07), 0x30);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x08), 0x6f);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x09), 0x04);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x0a), 0x40);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x0b), 0x40);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x00), 0xc7);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x01), 0x3b);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x3c), 0x01);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x3d), 0x00);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x3c), 0x00);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x3d), 0x00);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x00), 0xc7);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x01), 0x33);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x00), 0xc7);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x01), 0x32);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x10), 0x78);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x11), 0x00);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x06), 0x43);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x07), 0x00);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x06), 0x40);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x07), 0x00);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x00), 0x01);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x01), 0x00);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x14), 0x00);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x15), 0x06);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x34), 0x00);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x35), 0x00);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x32), 0xfe);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x33), 0x0b);

        dev_info(priv->dev, "eye pattern high speed setup done\n");
    }
    else if (EYE_DIAGRAM_MODE_FULL_SPEED == speed)
    {
        OUTREG8(GET_REG8_ADDR(reg_bank1, 0x00141e00), 0xc0);
        OUTREG8(GET_REG8_ADDR(reg_bank2, 0x00), 0x05);
        OUTREG8(GET_REG8_ADDR(reg_bank2, 0x01), 0x7f);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x08), 0xef);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x09), 0x04);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x0a), 0x00);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x0b), 0x40);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x08), 0xef);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x09), 0x04);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x0a), 0x40);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x0b), 0x40);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x08), 0x6f);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x09), 0x04);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x00), 0x05);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x01), 0x3f);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x00), 0x07);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x01), 0x3f);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x00), 0x0f);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x01), 0x3f);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x00), 0x0f);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x01), 0x3f);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x00), 0x4f);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x01), 0x3f);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x00), 0x4f);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x01), 0x3f);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x00), 0xcf);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x01), 0x3f);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x02), 0x80);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x03), 0x94);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x02), 0x80);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x03), 0x94);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x00), 0xcf);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x01), 0x2f);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x10), 0x40);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x11), 0x08);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x10), 0x40);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x11), 0x0a);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x10), 0x78);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x11), 0x00);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x06), 0x43);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x07), 0x00);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x06), 0x40);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x07), 0x00);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x00), 0xeb);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x01), 0x00);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x14), 0x40);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x15), 0x07);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x34), 0x00);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x35), 0x00);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x32), 0xfe);
        OUTREG8(GET_REG8_ADDR(reg_bank3, 0x33), 0x0b);

        dev_info(priv->dev, "eye pattern full speed setup done\n");
    }
}

static void sstar_u2phy_edswitch_set(struct sstar_u2phy *priv, EYE_DIAGRAM_MODE_E speed)
{
    if (priv->phy_data && priv->phy_data->revision == 1)
    {
        sstar_u2phy_edswitch_set_v1(priv, speed);
    }
    else
    {
        sstar_u2phy_edswitch_set_v2(priv, speed);
    }
}

static ssize_t sstar_u2phy_edswitch_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
    struct seq_file *   s    = file->private_data;
    struct sstar_u2phy *priv = s->private;
    unsigned long       flags;
    char                buf[32] = {0};
    EYE_DIAGRAM_MODE_E  speed   = EYE_DIAGRAM_MODE_UNKNOWN;

    if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
        return -EFAULT;

    if (!strncmp(buf, "hs", 2))
    {
        priv->ed_hs_switch_on = true;
        speed                 = EYE_DIAGRAM_MODE_HIGH_SPEED;
    }
    else if (!strncmp(buf, "fs", 2))
    {
        priv->ed_fs_switch_on = true;
        speed                 = EYE_DIAGRAM_MODE_FULL_SPEED;
    }
    else
    {
        dev_err(&priv->phy->dev, "Unknow eye diagram switch option: %s\n", buf);
        return -EINVAL;
    }

    spin_lock_irqsave(&priv->lock, flags);
    sstar_u2phy_edswitch_set(priv, speed);
    spin_unlock_irqrestore(&priv->lock, flags);

    return count;
}

static const struct file_operations sstar_u2phy_edswitch_fops = {
    .open    = sstar_u2phy_edswitch_open,
    .write   = sstar_u2phy_edswitch_write,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

int sstar_u2phy_edswitch_creat(struct device_node *node, struct phy *phy)
{
    phandle             ph;
    struct device_node *ednode;
    struct device *     dev  = &phy->dev;
    struct sstar_u2phy *priv = (struct sstar_u2phy *)phy_get_drvdata(phy);

    if (!of_property_read_u32(node, "eye-diagram", &ph))
    {
        ednode = of_find_node_by_phandle(ph);
        if (!ednode)
        {
            dev_err(dev, "No specified eye diagram node\n");
            return -EINVAL;
        }

        priv->ed_bank1 = of_iomap(ednode, 0);
        priv->ed_bank2 = of_iomap(ednode, 1);
        priv->ed_bank3 = of_iomap(ednode, 2);
        if (priv->phy_data && priv->phy_data->revision == 1)
        {
            priv->ed_bank4 = of_iomap(ednode, 3);
        }
        priv->has_ed_switch = true;
        dev_info(dev, "Eye diagram bank: %p %p %p %p\n", priv->ed_bank1, priv->ed_bank2, priv->ed_bank3,
                 priv->ed_bank4);
        return 0;
    }
    return -EINVAL;
}

static ssize_t sstar_u2phy_reset_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
    struct seq_file *   s       = file->private_data;
    struct sstar_u2phy *priv    = s->private;
    char                buf[32] = {0};
    u32                 reset   = 0;
    int                 ret;

    if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
        return -EFAULT;

    ret = kstrtouint(buf, 0, &reset);
    if (ret)
    {
        return ret;
    }

    if (reset && priv->phy->ops)
    {
        if (priv->phy->ops->reset)
            priv->phy->ops->reset(priv->phy);

        if (priv->phy->ops->power_off)
            priv->phy->ops->power_off(priv->phy);

        if (priv->phy->ops->power_on)
            priv->phy->ops->power_on(priv->phy);
    }

    return count;
}

static int sstar_u2phy_reset_open(struct inode *inode, struct file *file)
{
    return single_open(file, NULL, inode->i_private);
}

static const struct file_operations sstar_u2phy_reset_fops = {
    .open    = sstar_u2phy_reset_open,
    .write   = sstar_u2phy_reset_write,
    .release = single_release,
};

#ifdef CONFIG_DEBUG_FS
void sstar_u2phy_utmi_debugfs_init(struct sstar_u2phy *priv)
{
    spin_lock_init(&priv->lock);
    priv->root = debugfs_create_dir(dev_name(&priv->phy->dev), usb_debug_root);

    if (priv->phy_data && priv->phy_data->revision == 1)
    {
        debugfs_create_file("tx_swing", 0644, priv->root, priv, &sstar_tx_swing_fops);
        debugfs_create_file("de_emphasis_current", 0644, priv->root, priv, &sstar_dem_cur_fops);
        debugfs_create_file("cm_current", 0644, priv->root, priv, &sstar_cm_cur_fops);
    }
    else
    {
        debugfs_create_file("pre_emphasis", 0644, priv->root, priv, &sstar_pre_emphasis_fops);
        debugfs_create_file("slew_rate", 0644, priv->root, priv, &sstar_slew_rate_fops);
        debugfs_create_file("swing_trim", 0644, priv->root, priv, &sstar_swing_trim_fops);
        debugfs_create_file("disconnect_refer_voltage", 0644, priv->root, priv, &sstar_disc_ref_vol_fops);
        debugfs_create_file("squelch_refer_voltage", 0644, priv->root, priv, &sstar_squelch_ref_vol_fops);
        debugfs_create_file("ls_cross_rise", 0644, priv->root, priv, &sstar_ls_cross_rise_fops);
        debugfs_create_file("ls_cross_fall", 0644, priv->root, priv, &sstar_ls_cross_fall_fops);
    }

    debugfs_create_file("reset", 0644, priv->root, priv, &sstar_u2phy_reset_fops);

    if (priv->has_ed_switch)
        debugfs_create_file("eye_diagram_switch", 0644, priv->root, priv, &sstar_u2phy_edswitch_fops);
}

void sstar_u2phy_utmi_debugfs_exit(void *data)
{
    struct sstar_u2phy *priv = data;

    debugfs_remove_recursive(priv->root);
}
#endif

void sstar_u2phy_utmi_atop_v1_set(struct phy *phy)
{
    struct sstar_u2phy *priv = phy_get_drvdata(phy);
    struct regmap *     utmi = priv->utmi;
    u32                 val[3];

    if (of_property_read_u32_array(phy->dev.of_node, "tx-swing,de-emphasis,cm-current", val, ARRAY_SIZE(val)))
    {
        if (!tx_swing && !dem_cur && !cm_cur)
        {
            /* Default UTMI eye diagram parameter setting */
            regmap_update_bits(utmi, (0x16 << 2), 0x03F0, 0x0210); // tx-swing:[6:4], de-emphasis:[9:7]
            regmap_update_bits(utmi, (0x17 << 2), 0x8138, 0x8100); // cm-current:[5: 3], [15]=1''b1
            dev_info(&phy->dev, "Default UTMI eye diagram parameter setting\n");
        }
        return;
    }

    if (tx_swing != 0)
        val[0] = tx_swing;

    if (dem_cur != 0)
        val[1] = dem_cur;

    if (cm_cur != 0)
        val[2] = cm_cur;

    dev_info(&phy->dev, "Tx-swing:%d%% De-emphasis:%d%% CM-current:%d%%\n", val[0], val[1], val[2]);
    sstar_u2phy_utmi_tx_swing_set(priv, val[0]);
    sstar_u2phy_utmi_dem_cur_set(priv, val[1]);
    sstar_u2phy_utmi_cm_cur_set(priv, val[2]);
    regmap_set_bits(utmi, (0x17 << 2), BIT(15) | BIT(8));

    return;
}

void sstar_u2phy_utmi_atop_set(struct phy *phy)
{
    struct sstar_u2phy *priv = phy_get_drvdata(phy);
    struct regmap *     utmi = priv->utmi;
    u32                 val;

    if (priv->phy_data && priv->phy_data->revision == 1)
    {
        sstar_u2phy_utmi_atop_v1_set(phy);
        return;
    }

    if (!of_property_read_u32(phy->dev.of_node, "pre-emphasis", &val))
    {
        sstar_u2phy_utmi_pre_emphasis_set(priv, val);
        dev_info(&phy->dev, "pre emphasis: 0x%02x\n", val);
    }

    if (!of_property_read_u32(phy->dev.of_node, "slew-rate", &val))
    {
        sstar_u2phy_utmi_slew_rate_set(priv, val);
        dev_info(&phy->dev, "slew rate: 0x%02x\n", val);
    }

    if (!of_property_read_u32(phy->dev.of_node, "swing", &val))
    {
        sstar_u2phy_utmi_swing_trim_set(priv, val);
        dev_info(&phy->dev, "swing trim code: 0x%04x\n", val);
    }

    if (!of_property_read_u32(phy->dev.of_node, "disconnect-refer-vol", &val))
    {
        sstar_u2phy_utmi_disc_ref_vol_set(priv, val);
        dev_info(&phy->dev, "disconnect reference voltage: 0x%04x\n", val);
    }

    if (!of_property_read_u32(phy->dev.of_node, "squelch-refer-vol", &val))
    {
        sstar_u2phy_utmi_squelch_ref_vol_set(priv, val);
        dev_info(&phy->dev, "squelch reference voltage: 0x%04x\n", val);
    }

    if (!of_property_read_u32(phy->dev.of_node, "ls-cross-level", &val))
    {
        regmap_write(utmi, (0x49 << 2), val);
        regmap_read(utmi, (0x49 << 2), &val);
        dev_info(&phy->dev, "ls-cross-level = 0x%04x\r\n", val);
    }

    if (!of_property_read_u32(phy->dev.of_node, "fs-cross-level", &val))
    {
        regmap_write(utmi, (0x42 << 2), val);
        regmap_read(utmi, (0x42 << 2), &val);
        dev_info(&phy->dev, "fs-cross-level = 0x%04x\r\n", val);
    }

    return;
}

MODULE_DESCRIPTION("Sigmastar USB2 PHY debugfs driver");
MODULE_LICENSE("GPL v2");
