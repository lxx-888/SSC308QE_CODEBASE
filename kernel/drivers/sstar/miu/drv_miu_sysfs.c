/*
 * drv_miu_sysfs.c - Sigmastar
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

#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>

#include <drv_miu.h>
#include "drv_miu_defs.h"

//=================================================================================================
//                                     MIU Sysfs Function
//=================================================================================================

static ssize_t miu_dram_info_show(struct device* dev, struct device_attribute* attr, char* buf)
{
    struct miu_device* mdev       = sstar_to_miu_device(dev);
    const char*        failed_str = "Failed to read dram info!\n";

    if (mdev->dram.callback.read_info(&mdev->dram) < 0)
    {
        CamOsSnprintf(buf, PAGE_SIZE, failed_str);
        return strlen(failed_str);
    }

    return mdev->dram.callback.show_info(&mdev->dram, buf, buf + PAGE_SIZE);
}

DEVICE_ATTR(dram_info, 0444, miu_dram_info_show, NULL);

static ssize_t miu_ip_table_show(struct device* dev, struct device_attribute* attr, char* buf)
{
    struct miu_device* mdev = sstar_to_miu_device(dev);

    return mdev->client.callback.show_ip_table(&mdev->client, buf, buf + PAGE_SIZE);
}

DEVICE_ATTR(ip_table, 0444, miu_ip_table_show, NULL);

static ssize_t miu_client_show(struct file* filp, struct kobject* kobj, struct bin_attribute* attr, char* buf,
                               loff_t offset, size_t count)
{
    struct device*     dev  = container_of(kobj, struct device, kobj);
    struct miu_device* mdev = sstar_to_miu_device(dev);

    return mdev->client.callback.show_client(&mdev->client, offset, buf, buf + PAGE_SIZE);
}

static int miu_get_id_and_rw(struct miu_device* mdev, const char* buf, ssize_t length, u32* id,
                             enum hal_miu_rw_mode* rw)
{
    int  n1, n2;
    char name[15] = {0};
    char mode[5]  = {0};

    if (sscanf(buf, "%s %n", mode, &n1) != 1)
        goto FAIL;

    if (strcmp(mode, "r") == 0)
        *rw = HAL_MIU_RO;
    else if (strcmp(mode, "w") == 0)
        *rw = HAL_MIU_WO;
    // else if (strcmp(mode, "RW") == 0)
    //     *rw = E_MIU_RW;
    else
        goto FAIL;

    if (sscanf(buf + n1, "%s %n", name, &n2) == 1)
    {
        *id = mdev->client.callback.name_to_id(&mdev->client, name, *rw);
        // printk("name=[%s], id=[0x%x] %d %d\n", name, *id, n1, n2);
        if (mdev->client.callback.id_valid(&mdev->client, *id, *rw) != -1)
            return n1 + n2;
    }

    if (sscanf(buf + n1, "%x %n", id, &n2) == 1 && (mdev->client.callback.id_valid(&mdev->client, *id, *rw) != -1))
        return n1 + n2;
    sstar_miu_err("[CLIENT]: Not such IP.\n");
FAIL:

    return 0;
}

static ssize_t miu_client_store(struct file* filp, struct kobject* kobj, struct bin_attribute* attr, char* buf,
                                loff_t off, size_t n)
{
    int                  i;
    int                  flag = 1;
    char                 type[20];
    int                  offset = 0, tmp, ret;
    u32                  id;
    enum hal_miu_rw_mode rw;
    struct hal_miu_ip*   ip;
    struct device*       dev  = container_of(kobj, struct device, kobj);
    struct miu_device*   mdev = sstar_to_miu_device(dev);

    sscanf(buf, "%s", type);
    offset = strlen(type);

    for (i = 0; i < mdev->client.func_num; i++)
    {
        if (strcmp(mdev->client.ip_func[i].name, type))
            continue;

        id  = 0;
        tmp = miu_get_id_and_rw(mdev, buf + offset, n - offset, &id, &rw);
        if (id == -1)
        {
            // printk("client not suppport this command.\r\n");
            break;
        }
        offset += tmp;

        if (tmp)
        {
            ip  = mdev->client.callback.id_to_ip(&mdev->client, id, rw);
            ret = mdev->client.ip_func[i].call(ip, rw, buf + offset, n - offset);
        }

        if (!tmp || ret)
        {
            sstar_miu_info("%s\nUasge:\n", buf + offset);
            sstar_miu_info("\techo %-9s [r/w] [name/id] %s > client\n", mdev->client.ip_func[i].name,
                           mdev->client.ip_func[i].help);
        }
        flag = 0;
        break;
    }

    if (flag)
    {
        sstar_miu_info("Uasge:\n");
        for (i = 0; i < mdev->client.func_num; i++)
        {
            sstar_miu_info("\techo %-9s [r/w] [name/id] %s > client\n", mdev->client.ip_func[i].name,
                           mdev->client.ip_func[i].help);
        }
    }

    return n;
}

BIN_ATTR(client, 0644, miu_client_show, miu_client_store, HAL_MIU_CLIENT_BUF_MAX);

static int miu_bist_get_mask(const char* buf, ssize_t length, bool* mask, int bist_num)
{
    int  i, id, n;
    int  offset = 0;
    char str[15];

    if (sscanf(buf, "%s", str) != 1)
    {
        return -1;
    }

    if (strncmp(str, "all", 4) == 0)
    {
        for (i = 0; i < bist_num; i++)
        {
            mask[i] = true;
        }
    }
    else
    {
        while (offset < length)
        {
            sscanf(buf + offset, "%d %n", &id, &n);

            if (id < 0 || id > bist_num)
            {
                printk("Invalid Bist Num: %d\n", id);
                return -1;
            }
            mask[id] = true;
            offset += n;
        }
    }

    return 0;
}

static ssize_t miu_bist_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t length)
{
    int                i, n;
    int                offset = 0;
    int                ret;
    u32                size;
    struct miu_device* mdev = sstar_to_miu_device(dev);

    char type[10]               = {0};
    char mode[5]                = {0};
    bool mask[HAL_MIU_BIST_NUM] = {0};

    enum hal_miu_rw_mode bist_mode = HAL_MIU_RW;
    bool                 bist_loop = true;

    ret = sscanf(buf, "%s%n", type, &n);
    offset += n;

    if (strncmp(type, "stop", 4) == 0)
    {
        if (miu_bist_get_mask(buf + offset, length - offset, mask, mdev->bist.bist_num) < 0)
        {
            goto BIST_EXIT;
        }
        ret = mdev->bist.callback.stop(&mdev->bist, mask, &mdev->protect);
    }
    else if (strncmp(type, "size", 4) == 0)
    {
        sscanf(buf + offset, "%u", &size);
        ret = mdev->bist.callback.set_size(&mdev->bist, size);
    }
    else if (strncmp(type, "cmd_mode", 8) == 0)
    {
        sscanf(buf + offset, "%u", &size);
        ret = mdev->bist.callback.cmd_mode(&mdev->bist, size);
    }
    else
    {
        if (strncmp(type, "loop", 4) == 0)
            bist_loop = true;
        else if (strncmp(type, "oneshot", 7) == 0)
            bist_loop = false;
        else
            goto BIST_EXIT;

        ret = sscanf(buf + offset, "%s%n", mode, &n);
        offset += n;

        if (strncmp(mode, "rw", 2) == 0)
            bist_mode = HAL_MIU_RW;
        else if (strncmp(mode, "ro", 2) == 0)
            bist_mode = HAL_MIU_RO;
        else if (strncmp(mode, "wo", 2) == 0)
            bist_mode = HAL_MIU_WO;
        else
            goto BIST_EXIT;

        if (miu_bist_get_mask(buf + offset, length - offset, mask, mdev->bist.bist_num) < 0)
        {
            goto BIST_EXIT;
        }
        ret = mdev->bist.callback.start(&mdev->bist, bist_loop, bist_mode, mask, &mdev->protect);
    }

    return ret < 0 ? -1 : length;

BIST_EXIT:
    printk("Usage: echo [type] [mode] [bist] > bist\n");
    printk("\t[type] loop  oneshot\n");
    printk("\t[mode] rw  ro  wo\n");
    printk(KERN_CONT "\t[bist] all");
    for (i = 0; i < mdev->bist.bist_num; i++)
    {
        printk(KERN_CONT "  %d", i);
    }
    printk("\nEx. echo loop rw all > bist\n");
    printk("\nchange size: echo size [size] > bist\n\n");

    return -1;
}

static ssize_t miu_bist_show(struct device* dev, struct device_attribute* attr, char* buf)
{
    struct miu_device* mdev = sstar_to_miu_device(dev);

    return mdev->bist.callback.show(&mdev->bist, buf, buf + PAGE_SIZE);
}

DEVICE_ATTR(bist, 0644, miu_bist_show, miu_bist_store);

static ssize_t miu_bw_show(struct file* filp, struct kobject* kobj, struct bin_attribute* attr, char* buf,
                           loff_t offset, size_t n)
{
    struct device*     dev  = container_of(kobj, struct device, kobj);
    struct miu_device* mdev = sstar_to_miu_device(dev);

    return mdev->bw.callback.show(&mdev->bw, offset, buf, buf + n);
}

static ssize_t miu_bw_store(struct file* filp, struct kobject* kobj, struct bin_attribute* attr, char* buf,
                            loff_t offset, size_t count)
{
#if defined(CONFIG_MIU_BWLA)
    int                round;
    struct device*     dev  = container_of(kobj, struct device, kobj);
    struct miu_device* mdev = sstar_to_miu_device(dev);

    if (sscanf(buf, "%u", &round) != 1)
    {
        return count;
    }
    mdev->bw.callback.store(&mdev->bw, round);
#endif

    return count;
}
BIN_ATTR(bw, 0644, miu_bw_show, miu_bw_store, HAL_MIU_BW_BUF_MAX);

static ssize_t miu_bw_tool_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t length)
{
    struct miu_device* mdev = sstar_to_miu_device(dev);
    int                ret, n;
    char               cmd[10] = {0};

    ret = sscanf(buf, "%s%n", cmd, &n);

    if (strncmp(cmd, "stop", 4) == 0)
    {
        mdev->bw.callback.tool_stop(&mdev->bw);
        printk("miu bw measure stop\r\n");
    }
    else if (strncmp(cmd, "start", 5) == 0)
    {
        mdev->bw.callback.tool_start(&mdev->bw);
        printk("miu bw measure start\r\n");
    }
    else
    {
        // mdev->bw.callback.tool_stop(&mdev->bw);
        printk("Not support this command\r\n");
    }

    return length;
}

static ssize_t miu_bw_tool_show(struct device* dev, struct device_attribute* attr, char* buf)
{
    struct miu_device* mdev = sstar_to_miu_device(dev);

    return mdev->bw.callback.tool_show(&mdev->bw, buf, buf + PAGE_SIZE);
}

DEVICE_ATTR(bw_tool, 0644, miu_bw_tool_show, miu_bw_tool_store);

#if !defined(CONFIG_MIU_BWLA)
static ssize_t miu_bw_enable_show(struct device* dev, struct device_attribute* attr, char* buf)
{
    struct miu_device* mdev = sstar_to_miu_device(dev);

    return mdev->bw.callback.show_enable(&mdev->bw, buf, buf + PAGE_SIZE);
}

static ssize_t miu_bw_enable_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
    int                n      = 0;
    int                offset = 0;
    bool               enable;
    int                value;
    char               cmd[20];
    char               name[20];
    struct miu_device* mdev = sstar_to_miu_device(dev);

    if (sscanf(buf, "%s", cmd) != 1)
    {
        return -1;
    }

    if (strcmp(cmd, "enable") && strcmp(cmd, "disable"))
    {
        return -1;
    }

    offset += strlen(cmd);
    enable = !strcmp(cmd, "enable");

    while (offset < count)
    {
        if (sscanf(buf + offset, "%s %n", name, &n) != 1)
        {
            return -1;
        }

        if (!strcmp(name, "all"))
        {
            mdev->bw.callback.set_enable(&mdev->bw, 0, enable, true);
            break;
        }

        value = mdev->client.callback.name_to_id(&mdev->client, name, HAL_MIU_RW);
        if (value < 0)
        {
            sscanf(buf + offset, "%x %n", &value, &n);
        }
        if (mdev->client.callback.id_valid(&mdev->client, value, HAL_MIU_RW) < 0)
        {
            hal_miu_err("[BW] ip name or id invalid!\n");
            return -1;
        }
        mdev->bw.callback.set_enable(&mdev->bw, value, enable, false);
        offset += n;
    }

    return count;
}

DEVICE_ATTR(bw_enable, 0644, miu_bw_enable_show, miu_bw_enable_store);
#endif

//=================================================================================================
//                                     MIU Sysfs Main
//=================================================================================================

static struct attribute* miu_attrs[] = {
    &dev_attr_dram_info.attr,
    &dev_attr_ip_table.attr,
    &dev_attr_bist.attr,
#if !defined(CONFIG_MIU_BWLA)
    &dev_attr_bw_enable.attr,
#else
    &dev_attr_bw_tool.attr,
#endif
    NULL,
};

static struct bin_attribute* miu_bin_attrs[] = {
    &bin_attr_client,
    &bin_attr_bw,
    NULL,
};

static const struct attribute_group miu_attr_group = {.attrs = miu_attrs, .bin_attrs = miu_bin_attrs};

int sstar_miu_sysfs_init(void)
{
    int                ret;
    struct miu_device* mdev = sstar_get_miu_device();

    mdev->sub_sysfs.name      = "miu";
    mdev->sub_sysfs.dev_name  = "miu";
    mdev->dev_sysfs.kobj.name = "miu0";
    mdev->dev_sysfs.bus       = &mdev->sub_sysfs;
    dev_set_drvdata(&mdev->dev_sysfs, mdev);

    // Since mdev was dynamic allocated, dynamic lock_key has to be registered to eliminate waring at boot time and to
    // prevent false positive report.
    lockdep_register_key(&mdev->sub_sysfs.lock_key);

    ret = subsys_system_register(&mdev->sub_sysfs, NULL);
    if (ret < 0)
    {
        sstar_miu_err("[SYSFS] subsys_system_register failed!\n");
        return ret;
    }

    ret = device_register(&mdev->dev_sysfs);
    if (ret < 0)
    {
        sstar_miu_err("[SYSFS] dev_sysfs register failed!\n");
        return ret;
    }

    ret = sysfs_create_group(&mdev->dev_sysfs.kobj, &miu_attr_group);
    if (ret < 0)
    {
        sstar_miu_err("[SYSFS] miu_attr_group create failed!\n");
        return ret;
    }

    return 0;
}
