/*
 * sstar-dwc3-debugfs.c- Sigmastar
 *
 * Copyright (c) [2019~2021] SigmaStar Technology.
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
#include <linux/slab.h>
#include <linux/ptrace.h>
#include <linux/types.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/usb.h>
#include <linux/pm_runtime.h>
#include <io.h>
#include <xhci.h>
#include <hub.h>
#include <linux/platform_device.h>
#include <sstar-dwc3-of-simple.h>
#include <xhci.h>

#define U0           0x0
#define U1           0x1
#define U2           0x2
#define U3           0x3
#define SS_DIS       0x4
#define RX_DET       0x5
#define SS_INACT     0x6
#define POLL         0x7
#define RECOV        0x8
#define HRESET       0x9
#define COMPLY       0xa
#define LPBK         0xb
#define RESUME_RESET 0xf

#define INIT       0x0
#define POWER2     0x1
#define RESET_R    0x2
#define ACTIVE0    0x3
#define ACTIVE1    0x4
#define QUIET      0x5
#define PIPE_RESET 0x6

#define RESET_P       0x0
#define POWER0        0x1
#define LFPS          0x2
#define RXEQ          0x3
#define ACTIVE        0x4
#define CONFIGURATION 0x5
#define IDLE          0x6

extern u32 global_debug_ltssm[256];
extern u8  ltssm_index;

static ssize_t sstar_ltssm_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
    return 0;
}

static int sstar_ltssm_show(struct seq_file *s, void *unused)
{
    struct dwc3_of_simple *of_simple = s->private;
    unsigned long          flags;
    int                    index, end, state;
    char *                 state_name, *sub;

    spin_lock_irqsave(&of_simple->dwc->lock, flags);
    end = ltssm_index;
    spin_unlock_irqrestore(&of_simple->dwc->lock, flags);

    for (index = 0; end > index; index++)
    {
        state = (global_debug_ltssm[index] & 0x3c00000) >> 22;

        switch (state)
        {
            case U0:
                state_name = "U0";
                break;
            case U1:
                state_name = "U1";
                break;
            case U2:
                state_name = "U2";
                break;
            case U3:
                state_name = "U3";
                break;
            case SS_DIS:
                state_name = "SS_DIS";
                break;
            case RX_DET:
                state_name = "RX_DET";
                break;
            case SS_INACT:
                state_name = "SS_INACT";
                break;
            case POLL:
                state_name = "POLL";
                break;
            case RECOV:
                state_name = "RECOVERY";
                break;
            case HRESET:
                state_name = "HRESET";
                break;
            case COMPLY:
                state_name = "COMPLY";
                break;
            case RESUME_RESET:
                state_name = "RESUME_RESET";
                break;
            default:
                state_name = "UNKNOWN";
        }
        sub = "";
        if (RX_DET == state)
        {
            state = (global_debug_ltssm[index] & 0x3c0000) >> 18;
            switch (state)
            {
                case INIT:
                    sub = ".INIT";
                    break;
                case POWER2:
                    sub = ".POWER";
                    break;
                case RESET_R:
                    sub = ".RESET";
                    break;
                case ACTIVE0:
                    sub = ".ACTIVE";
                    break;
                case ACTIVE1:
                    sub = ".ACTIVE1";
                    break;
                case QUIET:
                    sub = ".QUIET";
                    break;
                case PIPE_RESET:
                    sub = ".PIPE_RESET";
                    break;
            }
        }
        else if (POLL == state)
        {
            state = (global_debug_ltssm[index] & 0x3c0000) >> 18;
            switch (state)
            {
                case RESET_P:
                    sub = ".RESET";
                    break;
                case POWER0:
                    sub = ".POWER";
                    break;
                case LFPS:
                    sub = ".LFPS";
                    break;
                case RXEQ:
                    sub = ".RXEQ";
                    break;
                case ACTIVE:
                    sub = ".ACTIVE";
                    break;
                case CONFIGURATION:
                    sub = ".CONFIGURATION";
                    break;
                case IDLE:
                    sub = ".IDLE";
                    break;
            }
        }

        seq_printf(s, "global_debug_ltssm[%d] = 0x%08x, %s%s\r\n", index, global_debug_ltssm[index], state_name, sub);
    }
    return 0;
}

static int sstar_ltssm_open(struct inode *inode, struct file *file)
{
    return single_open(file, sstar_ltssm_show, inode->i_private);
}

static const struct file_operations sstar_ltssm_fops = {
    .open    = sstar_ltssm_open,
    .write   = sstar_ltssm_write,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static int sstar_dwc3_show(struct seq_file *s, void *unused)
{
    return 0;
}

static int sstar_dwc3_open(struct inode *inode, struct file *file)
{
    return single_open(file, sstar_dwc3_show, inode->i_private);
}

static ssize_t sstar_dwc3_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
    struct seq_file *      s         = file->private_data;
    struct dwc3_of_simple *of_simple = s->private;
    struct usb_hcd *       hcd       = platform_get_drvdata(of_simple->dwc->xhci);
    struct xhci_hcd *      xhci      = hcd_to_xhci(hcd);
    struct usb_device *    hdev      = xhci->shared_hcd->self.root_hub;
    int                    pipe_ctrl;
    unsigned long          flags;
    char                   buf[32] = {0};
    u32                    compl_enabled;
    int                    ret;

    if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
        return -EFAULT;

    ret = kstrtouint(buf, 0, &compl_enabled);
    if (ret)
    {
        return ret;
    }

    pm_runtime_get_sync(&hdev->dev);
    if (!usb_control_msg(hdev, usb_sndctrlpipe(hdev, 0), USB_REQ_SET_FEATURE, USB_RT_PORT, USB_PORT_FEAT_LINK_STATE,
                         1 | (XDEV_COMP_MODE << 3), NULL, 0, 1000))
    {
        spin_lock_irqsave(&of_simple->dwc->lock, flags);
        pipe_ctrl = dwc3_readl(of_simple->dwc->regs, DWC3_GUSB3PIPECTL(0));
        dev_info(&of_simple->dwc->xhci->dev, "%s compl_enabled = %d, pipe_ctrl = 0x%x\r\n", __func__, compl_enabled,
                 pipe_ctrl);
        pipe_ctrl &= (~BIT(30));
        pipe_ctrl |= (compl_enabled << 30);
        dev_info(&of_simple->dwc->xhci->dev, "%s pipe_ctrl = 0x%x\r\n", __func__, pipe_ctrl);
        dwc3_writel(of_simple->dwc->regs, DWC3_GUSB3PIPECTL(0), pipe_ctrl);
        spin_unlock_irqrestore(&of_simple->dwc->lock, flags);
    }
    pm_runtime_put_sync(&hdev->dev);
    return count;
}

static const struct file_operations sstar_dwc3_fops = {
    .open    = sstar_dwc3_open,
    .write   = sstar_dwc3_write,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

void sstar_dwc3_debugfs_exit(struct dwc3_of_simple *of_simple)
{
    debugfs_remove_recursive(of_simple->root);
}

void sstar_dwc3_debugfs_init(struct dwc3_of_simple *of_simple)
{
    of_simple->root = debugfs_create_dir(dev_name(of_simple->dev), usb_debug_root);
    if (USB_DR_MODE_HOST == of_simple->dwc->dr_mode)
    {
        debugfs_create_file("compliance-enabled", 0644, of_simple->root, of_simple, &sstar_dwc3_fops);
    }

    debugfs_create_file("ltssm-state", 0644, of_simple->root, of_simple, &sstar_ltssm_fops);
}
