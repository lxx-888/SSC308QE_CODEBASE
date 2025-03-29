/*
 * gyro_module.c - Sigmastar
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
#include "gyro_core.h"
#include "gyro_internal.h"
#include <linux/init.h>
#include <linux/module.h>

static int __init gyro_init(void)
{
    int ret = 0;

    ret = gyro_internal_init();
    if (ret != GYRO_SUCCESS)
    {
        GYRO_ERR("gyro init fail, ret[%d]", ret);
        return ret;
    }

#ifdef CONFIG_SS_GYRO_SYSFS
    ret = gyro_sysfs_init();
    if (ret < 0)
    {
        GYRO_ERR("gyro_sysfs_init failed");
        goto err_gyro_sysfs_init;
    }
#endif

#ifdef CONFIG_SS_GYRO_IOCTL
    ret = gyro_ioctlfs_init();
    if (ret < 0)
    {
        GYRO_ERR("gyro_ioctlfs_init failed");
        goto err_gyro_ioctl_init;
    }
#endif

    return 0;

#ifdef CONFIG_SS_GYRO_IOCTL
err_gyro_ioctl_init:
#endif

#ifdef CONFIG_SS_GYRO_SYSFS
err_gyro_sysfs_init:
    gyro_internal_deinit();
#endif
    return ret;
}

static void __exit gyro_exit(void)
{
#ifdef CONFIG_SS_GYRO_IOCTL
    gyro_ioctlfs_deinit();
#endif
#ifdef CONFIG_SS_GYRO_SYSFS
    gyro_sysfs_deinit();
#endif
    gyro_internal_deinit();
}

module_init(gyro_init);
module_exit(gyro_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("Gyro sensor driver");
