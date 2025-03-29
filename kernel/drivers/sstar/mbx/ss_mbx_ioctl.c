/*
 * ss_mbx_ioctl.c- Sigmastar
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
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/ioctl.h>

#include "ss_mbx.h"
#include "ss_mbx_ioctl.h"
#include "ss_mbx_impl.h"

static dev_t          ss_mbx_devid;
static struct cdev    ss_mbx_cdev;
static struct class * ss_mbx_class;
static struct device *ss_mbx_device;

static int _mbx_file_open(struct inode *inode, struct file *file)
{
    return SS_Mailbox_IMPL_Init();
}

static int _mbx_file_release(struct inode *inode, struct file *file)
{
    return SS_Mailbox_IMPL_Deinit();
}

static long _mbx_file_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret = E_SS_MBX_RET_FAIL;

    switch (cmd)
    {
        case MBX_IOC_CMD_ENABLE_CLASS:
        {
            u8 u8MsgClass = 0;

            if (copy_from_user(&u8MsgClass, (void *)arg, sizeof(u8)))
                break;

            ret = SS_Mailbox_IMPL_Enable(u8MsgClass);
            break;
        }
        case MBX_IOC_CMD_DISABLE_CLASS:
        {
            u8 u8MsgClass = 0;

            if (copy_from_user(&u8MsgClass, (void *)arg, sizeof(u8)))
                break;

            ret = SS_Mailbox_IMPL_Disable(u8MsgClass);
            break;
        }
        case MBX_IOC_CMD_SEND_MSG:
        {
            SS_Mbx_SendMsg_t stSendMsg = {};

            if (copy_from_user(&stSendMsg, (void *)arg, sizeof(SS_Mbx_SendMsg_t)))
                break;

            ret = SS_Mailbox_IMPL_SendMsg(&stSendMsg.stMbxMsg);
            break;
        }
        case MBX_IOC_CMD_RECV_MSG:
        {
            SS_Mbx_RecvMsg_t stRecvMsg = {};

            if (copy_from_user(&stRecvMsg, (void *)arg, sizeof(SS_Mbx_RecvMsg_t)))
                break;

            ret = SS_Mailbox_IMPL_RecvMsg(stRecvMsg.u8MsgClass, &stRecvMsg.stMbxMsg, stRecvMsg.s32WaitMs);
            if (ret == 0)
            {
                ret = copy_to_user((void *)arg, &stRecvMsg, sizeof(SS_Mbx_RecvMsg_t));
            }
            break;
        }
        default:
        {
            printk(KERN_ERR "invaild cmd %d.\n", cmd);
            break;
        }
    }

    return ret;
}

static struct file_operations mbx_ops = {
    .owner          = THIS_MODULE,
    .open           = _mbx_file_open,
    .release        = _mbx_file_release,
    .unlocked_ioctl = _mbx_file_ioctl,
};

int ss_mbx_cdev_init(void)
{
    int ret = 0;

    ret = alloc_chrdev_region(&ss_mbx_devid, 0, MBX_IOC_DEV_COUNT, MBX_IOC_DEVICNAME);
    if (ret < 0)
    {
        printk(KERN_ERR "%s[%d]: alloc devid failed.\n", __FUNCTION__, __LINE__);
        goto err_alloc_chrdev;
    }

    cdev_init(&ss_mbx_cdev, &mbx_ops);
    ss_mbx_cdev.owner = THIS_MODULE;
    ret               = cdev_add(&ss_mbx_cdev, ss_mbx_devid, MBX_IOC_DEV_COUNT);
    if (ret < 0)
    {
        printk(KERN_ERR "%s[%d]: cdev_add failed.\n", __FUNCTION__, __LINE__);
        goto err_cdev_add;
    }

    ss_mbx_class = class_create(THIS_MODULE, MBX_IOC_DEVICNAME);
    if (IS_ERR(ss_mbx_class))
    {
        printk(KERN_ERR "%s[%d]: class create failed.\n", __FUNCTION__, __LINE__);
        goto err_class_create;
    }

    ss_mbx_device = device_create(ss_mbx_class, NULL, ss_mbx_devid, NULL, MBX_IOC_DEVICNAME);
    if (IS_ERR(ss_mbx_device))
    {
        printk(KERN_ERR "%s[%d]: device create failed.\n", __FUNCTION__, __LINE__);
        goto err_device_create;
    }

    printk(KERN_INFO "%s[%d]: mbx_cdev init.\n", __FUNCTION__, __LINE__);
    return 0;

err_device_create:
    class_destroy(ss_mbx_class);
err_class_create:
    cdev_del(&ss_mbx_cdev);
err_cdev_add:
    unregister_chrdev_region(ss_mbx_devid, MBX_IOC_DEV_COUNT);
err_alloc_chrdev:
    return ret;
}

void ss_mbx_cdev_deinit(void)
{
    device_destroy(ss_mbx_class, ss_mbx_devid);
    class_destroy(ss_mbx_class);
    cdev_del(&ss_mbx_cdev);
    unregister_chrdev_region(ss_mbx_devid, MBX_IOC_DEV_COUNT);
    printk(KERN_INFO "%s[%d]: mbx_cdev deinit.\n", __FUNCTION__, __LINE__);
}
