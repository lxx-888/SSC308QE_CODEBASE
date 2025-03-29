/*
 * gyro_ioctl.c- Sigmastar
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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/kthread.h>
#include <linux/stat.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/license.h>
#include <linux/cdev.h>

#include "gyro.h"
#include "gyro_internal.h"
#include "gyro_ioctl.h"
#include "ms_msys.h"

typedef struct gyro_ioctldev_res_s
{
    dev_t            devid;
    struct cdev      cdev;
    struct device *  device;
    struct gyro_dev *p_gyrodev;
} gyro_ioctldev_res_t;

typedef struct gyro_ioctlfs_res_s
{
    dev_t devid;
    struct class *class;
    gyro_ioctldev_res_t iocdev[MAX_SUPPORT_DEV_NUM];
} gyro_ioctlfs_res_t;

static gyro_ioctlfs_res_t *gp_ioctlfsres = NULL;

static int _gyro_ioctl_cmd_set_sample_rate(struct gyro_dev *dev, void *uarg)
{
    struct gyro_arg_sample_rate arg = {};
    if (copy_from_user(&arg, uarg, sizeof(arg)))
    {
        return -EFAULT;
    }
    return gyro_set_sample_rate(dev, arg);
}

static int _gyro_ioctl_cmd_set_gyro_range(struct gyro_dev *dev, void *uarg)
{
    struct gyro_arg_gyro_range arg = {};
    if (copy_from_user(&arg, uarg, sizeof(arg)))
    {
        return -EFAULT;
    }
    return gyro_set_gyro_range(dev, arg);
}

static int _gyro_ioctl_cmd_set_accel_range(struct gyro_dev *dev, void *uarg)
{
    struct gyro_arg_accel_range arg = {};
    if (copy_from_user(&arg, uarg, sizeof(arg)))
    {
        return -EFAULT;
    }
    return gyro_set_accel_range(dev, arg);
}

static int _gyro_ioctl_cmd_get_sample_rate(struct gyro_dev *dev, void *uarg)
{
    int                         ret = 0;
    struct gyro_arg_sample_rate arg = {};
    ret                             = gyro_get_sample_rate(dev, &arg);
    if (ret != 0)
    {
        return ret;
    }
    GYRO_INFO("gyro_get_sample_rate %u", arg.rate);
    ret = copy_to_user(uarg, &arg, sizeof(arg));
    return ret ? -EFAULT : 0;
}

static int _gyro_ioctl_cmd_get_gyro_range(struct gyro_dev *dev, void *uarg)
{
    int                        ret = 0;
    struct gyro_arg_gyro_range arg = {};
    ret                            = gyro_get_gyro_range(dev, &arg);
    if (ret != 0)
    {
        return ret;
    }
    GYRO_INFO("gyro_get_gyro_range %u", arg.range);
    ret = copy_to_user(uarg, &arg, sizeof(arg));
    return ret ? -EFAULT : 0;
}

static int _gyro_ioctl_cmd_get_gyro_sensitivity(struct gyro_dev *dev, void *uarg)
{
    int                         ret = 0;
    struct gyro_arg_sensitivity arg = {};
    ret                             = gyro_get_gyro_sensitivity(dev, &arg);
    if (ret != 0)
    {
        return ret;
    }
    GYRO_INFO("_gyro_ioctl_cmd_get_gyro_sensitivity %u %u", arg.num, arg.den);
    ret = copy_to_user(uarg, &arg, sizeof(arg));
    return ret ? -EFAULT : 0;
}

static int _gyro_ioctl_cmd_get_accel_range(struct gyro_dev *dev, void *uarg)
{
    int                         ret = 0;
    struct gyro_arg_accel_range arg = {};
    ret                             = gyro_get_accel_range(dev, &arg);
    if (ret != 0)
    {
        return ret;
    }
    GYRO_INFO("gyro_get_accel_range %u", arg.range);
    ret = copy_to_user(uarg, &arg, sizeof(arg));
    return ret ? -EFAULT : 0;
}

static int _gyro_ioctl_cmd_get_accel_sensitivity(struct gyro_dev *dev, void *uarg)
{
    int                         ret = 0;
    struct gyro_arg_sensitivity arg = {};
    ret                             = gyro_get_accel_sensitivity(dev, &arg);
    if (ret != 0)
    {
        return ret;
    }
    GYRO_INFO("gyro_get_gyro_sensitivity %u %u", arg.num, arg.den);
    ret = copy_to_user(uarg, &arg, sizeof(arg));
    return ret ? -EFAULT : 0;
}

static int _gyro_ioctl_cmd_get_group_delay(struct gyro_dev *dev, void *uarg)
{
    int                         ret = 0;
    struct gyro_arg_group_delay arg = {};
    ret                             = gyro_get_group_delay(dev, &arg);
    if (ret != 0)
    {
        return ret;
    }
    GYRO_INFO("gyro_get_group_delay %u us", arg.delay_us);
    ret = copy_to_user(uarg, &arg, sizeof(arg));
    return ret ? -EFAULT : 0;
}

static int _gyro_ioctl_cmd_read_fifocnt(struct gyro_dev *dev, void *uarg)
{
    int ret = 0;
    u16 arg = 0;
    ret     = gyro_read_fifocnt(dev, &arg);
    if (ret != 0)
    {
        return ret;
    }

    ret = copy_to_user(uarg, &arg, sizeof(arg));
    return ret ? -EFAULT : 0;
}

static int _gyro_ioctl_cmd_read_fifodata(struct gyro_dev *dev, void *uarg)
{
    __u8 *                pfifo_data = NULL;
    int                   ret        = 0;
    gyro_fifo_data_info_t arg        = {};

    ret = copy_from_user(&arg, uarg, sizeof(arg));
    if (ret != 0)
    {
        goto err_copy_to_user;
    }

    if (unlikely(arg.pfifo_data == NULL))
    {
        GYRO_ERR("ill attr: pfifo_data[0x0]");
        return -EFAULT;
    }

    pfifo_data = kzalloc(arg.data_cnt, GFP_KERNEL | GFP_DMA);
    if (pfifo_data == NULL)
    {
        GYRO_ERR("no mem");
        return -ENOMEM;
    }

    ret = gyro_read_fifodata(dev, pfifo_data, arg.data_cnt);
    if (ret != 0)
    {
        goto err_read_fifdata;
    }

    ret = copy_to_user(arg.pfifo_data, pfifo_data, arg.data_cnt);
    if (ret != 0)
    {
        goto err_copy_to_user;
    }

err_copy_to_user:
err_read_fifdata:
    kfree(pfifo_data);
    return ret ? -EFAULT : 0;
}

static int _gyro_ioctl_cmd_read_gyro_xyz(struct gyro_dev *dev, void *uarg)
{
    int                      ret = 0;
    struct gyro_arg_gyro_xyz arg = {};
    ret                          = gyro_read_gyro_xyz(dev, &arg);
    if (ret != 0)
    {
        return ret;
    }
    GYRO_INFO("gyro_read_gyro_xyz %u %u %u", arg.x, arg.y, arg.z);
    ret = copy_to_user(uarg, &arg, sizeof(arg));
    return ret ? -EFAULT : 0;
}

static int _gyro_ioctl_cmd_read_accel_xyz(struct gyro_dev *dev, void *uarg)
{
    int                       ret = 0;
    struct gyro_arg_accel_xyz arg = {};
    ret                           = gyro_read_accel_xyz(dev, &arg);
    if (ret != 0)
    {
        return ret;
    }
    GYRO_INFO("gyro_read_accel_xyz %u %u %u", arg.x, arg.y, arg.z);
    ret = copy_to_user(uarg, &arg, sizeof(arg));
    return ret ? -EFAULT : 0;
}

static int _gyro_ioctl_cmd_read_temp(struct gyro_dev *dev, void *uarg)
{
    int                  ret = 0;
    struct gyro_arg_temp arg = {};
    ret                      = gyro_read_temp(dev, &arg);
    if (ret != 0)
    {
        return ret;
    }
    GYRO_INFO("gyro_read_temp %u", arg.temp);
    ret = copy_to_user(uarg, &arg, sizeof(arg));
    return ret ? -EFAULT : 0;
}

static int _gyro_ioctl_cmd_whoami_verify(struct gyro_dev *dev, void *uarg)
{
    return gyro_whoami_verify(dev);
}

static int _gyro_ioctl_cmd_set_dev_mode(struct gyro_dev *dev, void *uarg)
{
    int                      ret = 0;
    gyro_arg_dev_mode_info_t arg = {};

    if (copy_from_user(&arg, uarg, sizeof(arg)))
    {
        return -EFAULT;
    }

    ret = gyro_set_dev_mode(dev, arg.dev_mode, &arg.fifo_info);
    if (ret != 0)
    {
        return ret;
    }

    if (copy_to_user(&((gyro_arg_dev_mode_info_t *)uarg)->fifo_info, &arg.fifo_info, sizeof(arg.fifo_info)))
    {
        GYRO_ERR("copy the fifo info to user fali");
        return -EFAULT;
    }

    return 0;
}

static int _gyro_ioctl_cmd_reset_fifo(struct gyro_dev *dev, void *uarg)
{
    return gyro_reset_fifo(dev);
}

static int _gyro_file_open(struct inode *pinode, struct file *pfile)
{
    unsigned char    u8index   = 0;
    struct gyro_dev *p_gyrodev = NULL;

    for (u8index = 0; u8index < MAX_SUPPORT_DEV_NUM; u8index++)
    {
        if (gp_ioctlfsres->iocdev[u8index].cdev.dev == pinode->i_cdev->dev)
        {
            p_gyrodev           = gp_ioctlfsres->iocdev[u8index].p_gyrodev;
            pfile->private_data = (void *)p_gyrodev;
            return gyro_enable((gyro_handle)p_gyrodev);
        }
    }

    GYRO_ERR("can't find the match gyrodev");
    pfile->private_data = NULL;
    return -1;
}

static int _gyro_file_release(struct inode *pinode, struct file *pfile)
{
    if (pfile->private_data)
    {
        gyro_disable((gyro_handle)pfile->private_data);
    }

    return 0;
}

static long _gyro_file_ioctl(struct file *pfile, unsigned int cmd, unsigned long arg)
{
    struct gyro_dev *dev  = (struct gyro_dev *)pfile->private_data;
    void *           uarg = (void *)arg;
    u8               nr   = 0;

    /* !!! Warnning, Don't change the order of the array @cmd_ops members!! */
    static int (*cmd_ops[E_GYRO_IOC_CMD_COUNT])(struct gyro_dev *, void *) = {
        /* set config or info */
        _gyro_ioctl_cmd_set_sample_rate,
        _gyro_ioctl_cmd_set_gyro_range,
        _gyro_ioctl_cmd_set_accel_range,
        _gyro_ioctl_cmd_set_dev_mode,

        /* get config or info */
        _gyro_ioctl_cmd_get_sample_rate,
        _gyro_ioctl_cmd_get_gyro_range,
        _gyro_ioctl_cmd_get_gyro_sensitivity,
        _gyro_ioctl_cmd_get_accel_range,
        _gyro_ioctl_cmd_get_accel_sensitivity,
        _gyro_ioctl_cmd_read_fifocnt,
        _gyro_ioctl_cmd_read_fifodata,
        _gyro_ioctl_cmd_read_gyro_xyz,
        _gyro_ioctl_cmd_read_accel_xyz,
        _gyro_ioctl_cmd_read_temp,
        _gyro_ioctl_cmd_whoami_verify,
        _gyro_ioctl_cmd_get_group_delay,

        /* hw ops */
        _gyro_ioctl_cmd_reset_fifo,
    };

    if (_IOC_TYPE(cmd) != GYRO_IOC_MAGIC)
    {
        GYRO_ERR("command type %x, need %x", _IOC_TYPE(cmd), GYRO_IOC_MAGIC);
        return -EINVAL;
    }

    nr = _IOC_NR(cmd);
    if (nr >= E_GYRO_IOC_CMD_COUNT)
    {
        GYRO_ERR("command nr %d, need %d ~ %d", nr, 0, E_GYRO_IOC_CMD_COUNT - 1);
        return -EINVAL;
    }

    return cmd_ops[nr](dev, uarg);
}

static struct file_operations gyro_ops = {
    .owner          = THIS_MODULE,
    .open           = _gyro_file_open,
    .release        = _gyro_file_release,
    .unlocked_ioctl = _gyro_file_ioctl,
};

static int _create_class(void)
{
    int ret = 0;

    /* alloc char device numbers */
    ret = alloc_chrdev_region(&gp_ioctlfsres->devid, 0, MAX_SUPPORT_DEV_NUM, GYRO_IOC_DEVICNAME);
    if (ret < 0)
    {
        GYRO_ERR("alloc devid failed");
        goto err_alloc_chrdev;
    }

    /* create device node */
    gp_ioctlfsres->class = class_create(THIS_MODULE, GYRO_IOC_DEVICNAME);
    if (IS_ERR(gp_ioctlfsres->class))
    {
        GYRO_ERR("class create failed");
        ret = IS_ERR(gp_ioctlfsres->class);
        goto err_alloc_class;
    }

    return ret;

err_alloc_class:
    unregister_chrdev_region(gp_ioctlfsres->devid, MAX_SUPPORT_DEV_NUM);
err_alloc_chrdev:
    return ret;
}

static void _destroy_class(void)
{
    if (gp_ioctlfsres->class)
    {
        unregister_chrdev_region(gp_ioctlfsres->devid, MAX_SUPPORT_DEV_NUM);
        class_destroy(gp_ioctlfsres->class);
        gp_ioctlfsres->class = NULL;
    }
}

static int _create_ioctlfs(struct gyro_dev *pdev)
{
#define STR_LENS 10
    int                  ret            = 0;
    char                 name[STR_LENS] = {"\0"};
    gyro_ioctldev_res_t *p_ioctldev_res = &gp_ioctlfsres->iocdev[pdev->u8_devid];

    /* init cdev */
    cdev_init(&p_ioctldev_res->cdev, &gyro_ops);
    p_ioctldev_res->cdev.owner = THIS_MODULE;
    p_ioctldev_res->devid      = MKDEV(MAJOR(gp_ioctlfsres->devid), pdev->u8_devid);
    ret                        = cdev_add(&p_ioctldev_res->cdev, p_ioctldev_res->devid, 1);
    if (ret < 0)
    {
        GYRO_ERR("cdev_add failed");
        goto err_cdev_add;
    }

    snprintf(name, STR_LENS, GYRO_IOC_DEVICNAME "%u", pdev->u8_devid);
    p_ioctldev_res->device = device_create(gp_ioctlfsres->class, NULL, p_ioctldev_res->devid, (void *)pdev, name);
    if (IS_ERR(p_ioctldev_res->device))
    {
        ret = IS_ERR(p_ioctldev_res->device);
        GYRO_ERR("device create failed");
        goto err_device_create;
    }

    p_ioctldev_res->p_gyrodev = pdev;
    GYRO_DBG("gyro_cdev init [%u-%u]", pdev->u8_devid, p_ioctldev_res->devid);
    return 0;

err_device_create:
    p_ioctldev_res->device = NULL;
    cdev_del(&p_ioctldev_res->cdev);
err_cdev_add:
    return ret;
}

static void _destroy_ioctlfs(gyro_ioctldev_res_t *p_ioctldev_res)
{
    if (p_ioctldev_res->device)
    {
        device_destroy(gp_ioctlfsres->class, p_ioctldev_res->devid);
        cdev_del(&p_ioctldev_res->cdev);
        GYRO_DBG("gyro_cdev deinit [%u]", p_ioctldev_res->devid);
    }
}

int gyro_ioctlfs_init(void)
{
    int              ret   = GYRO_SUCCESS;
    gyro_res_t *     p_res = NULL;
    struct gyro_dev *p_dev = NULL;
    struct gyro_dev *p_pos = NULL;

    BUG_ON(gp_ioctlfsres);

    p_res = get_gyro_reslist(E_DEV_RES);
    if (unlikely(!p_res))
    {
        ret = -1;
        GYRO_ERR("get gyro list fail");
        goto _exit_func;
    }

    gp_ioctlfsres = kcalloc(1, sizeof(gyro_ioctlfs_res_t), GFP_KERNEL);
    if (gp_ioctlfsres == NULL)
    {
        ret = -1;
        GYRO_ERR("no mem for ioctlfs resource");
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
        // not care about the result and lock the dev res
        _create_ioctlfs(p_dev);
    }
    R_UNLOCK_LIST(p_res);

    return ret;

_err_create_class:
    kfree(gp_ioctlfsres);
    gp_ioctlfsres = NULL;
_exit_func:
    return ret;
}

void gyro_ioctlfs_deinit(void)
{
    unsigned char u8_devid = 0;

    if (gp_ioctlfsres == NULL)
    {
        return;
    }

    for (u8_devid = 0; u8_devid < MAX_SUPPORT_DEV_NUM; u8_devid++)
    {
        _destroy_ioctlfs(&gp_ioctlfsres->iocdev[u8_devid]);
    }

    _destroy_class();

    kfree(gp_ioctlfsres);
    gp_ioctlfsres = NULL;
}

MODULE_LICENSE("GPL");
