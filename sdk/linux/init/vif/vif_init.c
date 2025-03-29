/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/kdev_t.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/miscdevice.h>
#include <linux/buffer_head.h>
#include <asm/string.h>
#include <asm/uaccess.h>

#include <ms_platform.h>
#include <ms_msys.h>
#include <cam_os_wrapper.h>
#include <cam_device_wrapper.h>
#include <cam_sysfs.h>
#include "mi_common_internal.h"

void drv_vif_isr(u32 irq);
void _vif_module_verify(const char *pCmdStr);

static struct miscdevice       g_stVifDev;
static struct device_node     *pstDevNode = NULL;
static struct platform_device *pdev       = NULL;

void *vif_ioremap(u32 addr, u32 size)
{
    return ioremap(addr, size);
}

int vif_unmap(void *addr)
{
    iounmap(addr);
    return 0;
}

int vif_inirq(void)
{
    return in_irq();
}

void *vif_dev_get(void)
{
    return (void *)pstDevNode;
}

int vif_of_property_readu32(const char *propname, u32 *out_value)
{
    if (pstDevNode == NULL)
    {
        printk("[%s %d]ERROR: VIF pstDevNode is NULL!\n", __func__, __LINE__);
        return -1;
    }
    else
    {
        return CamofPropertyReadU32(pstDevNode, propname, out_value);
    }
}

static irqreturn_t vif_module_isr(int irq, void *priv)
{
    drv_vif_isr(irq);

    return IRQ_HANDLED;
}

int vif_irq_of_parse_and_map(int cnt)
{
    if (pstDevNode == NULL)
    {
        printk("[%s %d]ERROR: VIF pstDevNode is NULL!\n", __func__, __LINE__);
        return -1;
    }
    else
    {
        return CamIrqOfParseAndMap(pstDevNode, cnt);
    }
}

int vif_request_irq(unsigned int virq, const char *name, void *dev)
{
    return request_irq(virq, vif_module_isr, IRQF_TRIGGER_NONE, name, dev);
}

void vif_free_irq(unsigned int virq, void *dev)
{
    free_irq(virq, dev);
}

int vif_devnode_init(void)
{
    pstDevNode = of_find_compatible_node(NULL, NULL, "sstar,vif");
    if (pstDevNode == NULL)
    {
        printk("[%s %d]ERROR: VIF of dts node failed!\n", __func__, __LINE__);
        return -ENODEV;
    }

    pdev = of_find_device_by_node(pstDevNode);
    if (pdev == NULL)
    {
        printk("[%s %d]ERROR: VIF of dts node failed!\n", __func__, __LINE__);
        of_node_put(pstDevNode);
        return -ENODEV;
    }

    return 0;
}

static int vif_open(struct inode *inode, struct file *fp)
{
    return 0;
}

static int vif_release(struct inode *inode, struct file *fp)
{
    return 0;
}
static ssize_t vif_read(struct file *fp, char __user *buf, size_t size, loff_t *ppos)
{
    return 0;
}

static ssize_t vif_write(struct file *fp, const char __user *buf, size_t size, loff_t *ppos)
{
    char  cmd[32];
    char *pcKBuf = NULL;
    pcKBuf       = vzalloc(size);

    if (!copy_from_user((void *)pcKBuf, (void __user *)buf, size))
    {
        sscanf(pcKBuf, "%s", cmd);
        printk("[%s] CMD : %s\n", __FUNCTION__, cmd);
    }
    else
    {
        printk("[%s] copy user data failed!!!\n", __FUNCTION__);
        goto VIF_WRITE_EXIT;
    }

    _vif_module_verify(pcKBuf);

VIF_WRITE_EXIT:
    if (pcKBuf)
    {
        vfree(pcKBuf);
        pcKBuf = NULL;
    }

    return size;
}

static long vif_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
    return 0;
}

struct file_operations vif_ops = {
    .owner          = THIS_MODULE,
    .open           = vif_open,
    .release        = vif_release,
    .read           = vif_read,
    .write          = vif_write,
    .unlocked_ioctl = vif_ioctl,
};

int vif_register_misc_device(void)
{
    if (pdev == NULL)
    {
        printk("[%s %d]ERROR: VIF pdev is NULL!\n", __func__, __LINE__);
        return -1;
    }

    g_stVifDev.minor  = MISC_DYNAMIC_MINOR;
    g_stVifDev.name   = "vif";
    g_stVifDev.fops   = &vif_ops;
    g_stVifDev.parent = &pdev->dev;
    if (misc_register(&g_stVifDev))
    {
        printk("[%s %d]ERROR: VIF miscdevice register failed!\n", __func__, __LINE__);
        return -1;
    }

    return 0;
}

void vif_unregister_misc_device(void)
{
    misc_deregister(&g_stVifDev);
}

#pragma message("compile mi-lite")
DECLEAR_MODULE_INIT_EXIT

MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
MI_MODULE_LICENSE();
