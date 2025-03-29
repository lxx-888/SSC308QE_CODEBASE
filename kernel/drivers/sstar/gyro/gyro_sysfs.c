/*
 * gyro_sysfs.c - Sigmastar
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

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/kthread.h>
#include <linux/stat.h>
#include <linux/string.h>
#include <linux/slab.h>

#include "gyro_internal.h"
#include "ms_msys.h"

#define CHECK_RESULT(result, expectation, function, fmt, ...) \
    do                                                        \
    {                                                         \
        if ((expectation) == (function))                      \
        {                                                     \
            printk("[ Success ] " #fmt "\n", ##__VA_ARGS__);  \
        }                                                     \
        else                                                  \
        {                                                     \
            printk("[ Failed  ] " #fmt "\n", ##__VA_ARGS__);  \
            result = !result;                                 \
        }                                                     \
    } while (0)

typedef struct gyro_sysdev_res_s
{
    dev_t          devid;
    struct device *sysfs_dev;
} gyro_sysdev_res_t;

typedef struct gyro_sysfs_res_s
{
    dev_t devid;
    struct class *class;
    gyro_sysdev_res_t sysdev[MAX_SUPPORT_DEV_NUM];
} gyro_sysfs_res_t;

static gyro_sysfs_res_t *gp_sysfsres = NULL;

static void show_one_frame_fifo_data(u8 *data, struct gyro_arg_fifo_info fifo_info)
{
    struct fifo_parse
    {
        char *name;
        u8    start;
        u8    end;
    } fp[] = {
        {"gx", fifo_info.gx_start, fifo_info.gx_end},     {"gy", fifo_info.gy_start, fifo_info.gy_end},
        {"gz", fifo_info.gz_start, fifo_info.gz_end},     {"ax", fifo_info.ax_start, fifo_info.ax_end},
        {"ay", fifo_info.ay_start, fifo_info.ay_end},     {"az", fifo_info.az_start, fifo_info.az_end},
        {"te", fifo_info.temp_start, fifo_info.temp_end},
    };
    unsigned int  i     = 0;
    unsigned char j     = 0;
    int           num   = 0;
    int           shift = 0;

    printk(KERN_DEBUG KERN_CONT "| ");
    for (i = 0; i < sizeof(fp) / sizeof(fp[0]); ++i)
    {
        if (fp[i].start > fifo_info.bytes_pre_data || fp[i].end > fifo_info.bytes_pre_data)
        {
            continue;
        }
        num   = 0;
        shift = fifo_info.is_big_endian ? (fp[i].end - fp[i].start) * 8 : 0;
        for (j = fp[i].start; j <= fp[i].end; ++j)
        {
            pr_info("[%u]-[%#x]\n", j, data[j]);
            if ((fifo_info.is_big_endian == 1 && j == fp[i].start) || (fifo_info.is_big_endian == 0 && j == fp[i].end))
            {
                num |= ((signed char)data[j] << shift);
            }
            else
            {
                num |= ((unsigned char)data[j] << shift);
            }

            shift += fifo_info.is_big_endian ? -8 : 8;
        }
        printk(KERN_DEBUG KERN_CONT "%4s: %6d | ", fp[i].name, num);
    }
    printk(KERN_DEBUG KERN_CONT "\n");
}

static void _gyro_self_check(struct gyro_dev *p_gyrodev)
{
    struct gyro_arg_dev_mode    dev_mode;
    struct gyro_arg_gyro_range  gyro_range;
    struct gyro_arg_accel_range accel_range;
    struct gyro_arg_sample_rate sample_rate;
    struct gyro_arg_fifo_info   fifo_info;
    struct gyro_arg_group_delay group_delay;
    u8 *                        pfifo_data = NULL;
    gyro_handle                 gyrohandle = (void *)p_gyrodev;

    bool         result = true;
    int          i;
    unsigned int test_sample_rate[] = {
        100, 200, 500, 1000, 2000, 4000, 8000, 16000, 200,
    };
    unsigned int test_gyro_range[] = {
        125, 250, 500, 1000, 2000,
    };
    unsigned int             test_accel_range[] = {2, 4, 8, 16};
    struct gyro_arg_dev_mode test_dev_mode[]    = {
        {0, 0},
        {1, GYROSENSOR_ZA_FIFO_EN | GYROSENSOR_YA_FIFO_EN | GYROSENSOR_XA_FIFO_EN | GYROSENSOR_ZG_FIFO_EN
                | GYROSENSOR_YG_FIFO_EN | GYROSENSOR_XG_FIFO_EN | GYROSENSOR_TEMP_FIFO_EN},
        {1, GYROSENSOR_ZA_FIFO_EN | GYROSENSOR_YA_FIFO_EN | GYROSENSOR_XA_FIFO_EN},
        {1, GYROSENSOR_ZG_FIFO_EN | GYROSENSOR_YG_FIFO_EN | GYROSENSOR_XG_FIFO_EN},
    };

    /* enable gyro */
    CHECK_RESULT(result, 0, gyro_enable(gyrohandle), "gyro_enable");
    if (!result)
    {
        return;
    }

    printk("----gyro range test --------------------------------------\n");
    for (i = 0; i < sizeof(test_gyro_range) / sizeof(test_gyro_range[0]); ++i)
    {
        gyro_range.range = test_gyro_range[i];
        CHECK_RESULT(result, 0, gyro_set_gyro_range(gyrohandle, gyro_range), "gyro_set_gyro_range (%u)",
                     test_gyro_range[i]);
        CHECK_RESULT(result, 0, gyro_get_gyro_range(gyrohandle, &gyro_range), "gyro_get_gyro_range");
        CHECK_RESULT(result, true, gyro_range.range == test_gyro_range[i], "read back check.");
        CHECK_RESULT(result, true, result, "result check gyro_range [%u]", test_gyro_range[i]);
    }

    printk("----accel range test --------------------------------------\n");
    for (i = 0; i < sizeof(test_accel_range) / sizeof(test_accel_range[0]); ++i)
    {
        accel_range.range = test_accel_range[i];
        CHECK_RESULT(result, 0, gyro_set_accel_range(gyrohandle, accel_range), "gyro_set_accel_range (%u)",
                     test_accel_range[i]);
        CHECK_RESULT(result, 0, gyro_get_accel_range(gyrohandle, &accel_range), "gyro_get_accel_range");
        CHECK_RESULT(result, true, accel_range.range == test_accel_range[i], "read back check.");
        CHECK_RESULT(result, true, result, "result check accel_range [%u]", test_accel_range[i]);
    }

    printk("----sample rate test --------------------------------------\n");
    for (i = 0; i < sizeof(test_sample_rate) / sizeof(test_sample_rate[0]); ++i)
    {
        sample_rate.rate = test_sample_rate[i];
        CHECK_RESULT(result, 0, gyro_set_sample_rate(gyrohandle, sample_rate), "gyro_set_sample_rate (%u)",
                     test_sample_rate[i]);
        CHECK_RESULT(result, 0, gyro_get_sample_rate(gyrohandle, &sample_rate), "gyro_get_sample_rate");
        CHECK_RESULT(result, true, sample_rate.rate == test_sample_rate[i], "read back check.");
        CHECK_RESULT(result, true, result, "result check sample_rate [%u]", test_sample_rate[i]);
    }

    printk("----gyro_set_dev_mode -------------------------------------\n");
    for (i = 0; i < sizeof(test_dev_mode) / sizeof(test_dev_mode[0]); ++i)
    {
        dev_mode = test_dev_mode[i];
        CHECK_RESULT(result, 0, gyro_set_dev_mode(gyrohandle, dev_mode, &fifo_info), "gyro_set_dev_mode (%d, 0x%x)",
                     dev_mode.fifo_mode, dev_mode.fifo_type);
        printk("\tgx_start, gx_end      %d, %d\n", fifo_info.gx_start, fifo_info.gx_end);
        printk("\tgy_start, gy_end      %d, %d\n", fifo_info.gy_start, fifo_info.gy_end);
        printk("\tgz_start, gz_end      %d, %d\n", fifo_info.gz_start, fifo_info.gz_end);
        printk("\tax_start, ax_end      %d, %d\n", fifo_info.ax_start, fifo_info.ax_end);
        printk("\tay_start, ay_end      %d, %d\n", fifo_info.ay_start, fifo_info.ay_end);
        printk("\taz_start, az_end      %d, %d\n", fifo_info.az_start, fifo_info.az_end);
        printk("\ttemp_start, temp_end  %d, %d\n", fifo_info.temp_start, fifo_info.temp_end);
        printk("\tbytes_pre_data        %d\n", fifo_info.bytes_pre_data);
        printk("\tis_big_endian         %d\n", fifo_info.is_big_endian);
        printk("\tmax_fifo_cnt          %d\n", fifo_info.max_fifo_cnt);
    }

    // To avoid FIFO overflow due to excessively high sampling rates,
    // we'll default to a sampling rate of 1kHz before reading from the FIFO.
    sample_rate.rate = 1000;
    CHECK_RESULT(result, 0, gyro_set_sample_rate(gyrohandle, sample_rate), "gyro_set_sample_rate (%u)",
                 test_sample_rate[i]);
    CHECK_RESULT(result, 0, gyro_get_sample_rate(gyrohandle, &sample_rate), "gyro_get_sample_rate");
    gyro_reset_fifo(gyrohandle);

    printk("----group delay test ---------------------------------------\n");
    CHECK_RESULT(result, 0, gyro_get_group_delay(gyrohandle, &group_delay), "gyro_get_group_delay (%u)us",
                 group_delay.delay_us);
    printk("\tgroup delay[%u]us \n", group_delay.delay_us);

    pfifo_data = kzalloc(fifo_info.max_fifo_cnt, GFP_KERNEL | GFP_DMA);
    if (pfifo_data == NULL)
    {
        GYRO_ERR("no mem for fifo data, size[0x%x]\n", fifo_info.max_fifo_cnt);
        return;
    }

    printk("----gyro fifo test ---------------------------------------\n");
    for (i = 0; i < 10; ++i)
    {
        u16 cnt = 0;
        u16 j   = 0;

        bool bNeedReset = false;
        msleep(30);
        gyro_read_fifocnt(gyrohandle, &cnt);
        if (cnt > 2048)
        {
            cnt = 2048 / fifo_info.bytes_pre_data * fifo_info.bytes_pre_data;
        }

        memset(pfifo_data, 0, cnt);
        if (cnt % fifo_info.bytes_pre_data != 0)
        {
            // fifo_data_info.data_cnt = ALIGN_DOWN(cnt, fifo_info.bytes_pre_data);
            GYRO_WRN("fifocnt[%u] not align [%u]\n", cnt, fifo_info.bytes_pre_data);
            bNeedReset = true;
        }

        gyro_read_fifodata(gyrohandle, pfifo_data, cnt);

        if (bNeedReset == true)
        {
            gyro_reset_fifo(gyrohandle);
        }

        printk(KERN_DEBUG "-------------- loop[%08x] curcnt[%08u] --------------\n", i, cnt);
        for (j = 0; j <= cnt - fifo_info.bytes_pre_data; j += fifo_info.bytes_pre_data)
        {
            show_one_frame_fifo_data(&pfifo_data[j], fifo_info);
        }
    }

    kfree(pfifo_data);
    gyro_disable(gyrohandle);
}

static ssize_t gyro_sysfs_self_check_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct gyro_dev *p_gyrodev = dev_get_drvdata(dev);
    BUG_ON(!p_gyrodev);

    _gyro_self_check(p_gyrodev);
    return scnprintf(buf, PAGE_SIZE, "Show dev[%u] whole log in kmsg\n", p_gyrodev->u8_devid);
}

static DEVICE_ATTR(self_check, S_IRUGO, gyro_sysfs_self_check_show, NULL);

static int _create_class(void)
{
    int ret = 0;

    /* alloc char device numbers */
    ret = alloc_chrdev_region(&gp_sysfsres->devid, 0, MAX_SUPPORT_DEV_NUM, GYRO_DEVICNAME);
    if (ret < 0)
    {
        GYRO_ERR("alloc devid failed");
        goto err_alloc_chrdev;
    }

    gp_sysfsres->class = msys_get_sysfs_class();
err_alloc_chrdev:
    return ret;
}

static void _destroy_class(void)
{
    if (gp_sysfsres->class)
    {
        unregister_chrdev_region(gp_sysfsres->devid, MAX_SUPPORT_DEV_NUM);
        gp_sysfsres->class = NULL;
    }
}

static int _create_sysfs(struct gyro_dev *pdev)
{
#define STR_LENS 6
    int                ret            = 0;
    char               name[STR_LENS] = {"\0"};
    gyro_sysdev_res_t *p_sysdev_res   = &gp_sysfsres->sysdev[pdev->u8_devid];

    snprintf(name, STR_LENS, "gyro%u", pdev->u8_devid);
    p_sysdev_res->devid     = MKDEV(MAJOR(gp_sysfsres->devid), pdev->u8_devid);
    p_sysdev_res->sysfs_dev = device_create(gp_sysfsres->class, NULL, p_sysdev_res->devid, pdev, name);
    if (IS_ERR(p_sysdev_res->sysfs_dev))
    {
        GYRO_ERR("device_create for dev[%u] sysfs failed.", pdev->u8_devid);
        ret = IS_ERR(p_sysdev_res->sysfs_dev);
        goto err_device_create;
    }

    device_create_file(p_sysdev_res->sysfs_dev, &dev_attr_self_check);

    GYRO_DBG("gyro_sysfs init [%u-%u]", pdev->u8_devid, p_sysdev_res->devid);
    return 0;

err_device_create:
    p_sysdev_res->sysfs_dev = NULL;
    return ret;
}

static void _destroy_sysfs(gyro_sysdev_res_t *p_sysdev_res)
{
    if (p_sysdev_res->sysfs_dev)
    {
        device_remove_file(p_sysdev_res->sysfs_dev, &dev_attr_self_check);
        device_destroy(gp_sysfsres->class, p_sysdev_res->devid);
        GYRO_DBG("gyro_sysfs deinit [%u]", p_sysdev_res->devid);
    }
}

int gyro_sysfs_init(void)
{
    int              ret   = GYRO_SUCCESS;
    gyro_res_t *     p_res = NULL;
    struct gyro_dev *p_dev = NULL;
    struct gyro_dev *p_pos = NULL;

    BUG_ON(gp_sysfsres);

    p_res = get_gyro_reslist(E_DEV_RES);
    if (unlikely(!p_res))
    {
        ret = -1;
        GYRO_ERR("get gyro list fail");
        goto _exit_func;
    }

    gp_sysfsres = kcalloc(1, sizeof(gyro_sysfs_res_t), GFP_KERNEL);
    if (gp_sysfsres == NULL)
    {
        ret = -1;
        GYRO_ERR("no mem for sysfs resource");
        goto _exit_func;
    }

    ret = _create_class();
    if (ret != GYRO_SUCCESS)
    {
        goto _err_create_class;
    }

    R_LOCK_LIST(p_res);
    list_for_each_entry_safe(p_dev, p_pos, &p_res->list, list_head)
    {
        // not care about the success and lock the dev res
        ret = _create_sysfs(p_dev);
    }
    R_UNLOCK_LIST(p_res);

    ret = GYRO_SUCCESS;
    return ret;

_err_create_class:
    kfree(gp_sysfsres);
    gp_sysfsres = NULL;
_exit_func:
    return ret;
}

void gyro_sysfs_deinit(void)
{
    unsigned char u8_devid = 0;

    if (gp_sysfsres == NULL)
    {
        return;
    }

    for (u8_devid = 0; u8_devid < MAX_SUPPORT_DEV_NUM; u8_devid++)
    {
        _destroy_sysfs(&gp_sysfsres->sysdev[u8_devid]);
    }

    _destroy_class();

    kfree(gp_sysfsres);
    gp_sysfsres = NULL;
}
