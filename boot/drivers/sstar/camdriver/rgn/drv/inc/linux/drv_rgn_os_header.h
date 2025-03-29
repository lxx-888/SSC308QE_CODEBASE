/*
 * drv_rgn_os_header.h - Sigmastar
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

#ifndef __DRV_RGN_OS_HEADER_H__
#define __DRV_RGN_OS_HEADER_H__

#include <linux/pfn.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/vmalloc.h> /* seems do not need this */
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <asm/string.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>

#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/poll.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/blkdev.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/wait.h>

#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>

#include "cam_os_wrapper.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define RGN_IO_OFFSET MS_IO_OFFSET

#define RGN_IO_ADDRESS(x) ((x) + RGN_IO_OFFSET)
//#define __io_address(n)       ((void __iomem __force *)RGN_IO_ADDRESS(n))

/* read register by byte */
#define rgn_readb(a) (*(volatile unsigned char *)RGN_IO_ADDRESS(a))

/* read register by word */
#define rgn_readw(a) (*(volatile unsigned short *)RGN_IO_ADDRESS(a))

/* read register by long */
#define rgn_readl(a) (*(volatile unsigned int *)RGN_IO_ADDRESS(a))

/* write register by byte */
#define rgn_writeb(v, a) (*(volatile unsigned char *)RGN_IO_ADDRESS(a) = (v))

/* write register by word */
#define rgn_writew(v, a) (*(volatile unsigned short *)RGN_IO_ADDRESS(a) = (v))

/* write register by long */
#define rgn_writel(v, a) (*(volatile unsigned int *)RGN_IO_ADDRESS(a) = (v))

#define READ_BYTE(x)     rgn_readb(x)
#define READ_WORD(x)     rgn_readw(x)
#define READ_LONG(x)     rgn_readl(x)
#define WRITE_BYTE(x, y) rgn_writeb((u8)(y), x)
#define WRITE_WORD(x, y) rgn_writew((u16)(y), x)
#define WRITE_LONG(x, y) rgn_writel((u32)(y), x)

#ifndef bool
#define bool u8
#endif

//-------------------------------------------------------------------------------------------------
//  Structure & Emu
//-------------------------------------------------------------------------------------------------
typedef struct
{
    CamOsTsem_t tSemCfg;
} DrvRgnOsTsemConfig_t;

#endif
