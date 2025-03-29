/*
 * io.h- Sigmastar
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

#ifndef _ASM_ARCH_IO_H_
#define _ASM_ARCH_IO_H_

//------------------------------------------------------------------------------
//  Include Files
//------------------------------------------------------------------------------

/*------------------------------------------------------------------------------
    Constant
-------------------------------------------------------------------------------*/

/* max of IO Space */
#define IO_SPACE_LIMIT 0xffffffff

/* Constants of CEDRIC RIU */
#define IO_PHYS         0x1F000000 //sync with platform.h
#define IO_OFFSET       0x00000000
#define IO_SIZE         0x00400000
#define IO_VIRT         (IO_PHYS + IO_OFFSET)
#define io_p2v(pa)      ((pa) + IO_OFFSET)
#define io_v2p(va)      ((va) - IO_OFFSET)
#define IO_ADDRESS(x)   io_p2v(x)

/* read register by byte */
#define sstar_readb(a) (*(volatile unsigned char *)IO_ADDRESS(a))

/* read register by word */
#define sstar_readw(a) (*(volatile unsigned short *)IO_ADDRESS(a))

/* read register by long */
#define sstar_readl(a) (*(volatile unsigned int *)IO_ADDRESS(a))

/* write register by byte */
#define sstar_writeb(v,a) (*(volatile unsigned char *)IO_ADDRESS(a) = (v))

/* write register by word */
#define sstar_writew(v,a) (*(volatile unsigned short *)IO_ADDRESS(a) = (v))

/* write register by long */
#define sstar_writel(v,a) (*(volatile unsigned int *)IO_ADDRESS(a) = (v))

#define BIT_0       (1<<0)
#define BIT_1       (1<<1)
#define BIT_2       (1<<2)
#define BIT_3       (1<<3)
#define BIT_4       (1<<4)
#define BIT_5       (1<<5)
#define BIT_6       (1<<6)
#define BIT_7       (1<<7)
#define BIT_8       (1<<8)
#define BIT_9       (1<<9)
#define BIT_10      (1<<10)
#define BIT_11      (1<<11)
#define BIT_12      (1<<12)
#define BIT_13      (1<<13)
#define BIT_14      (1<<14)
#define BIT_15      (1<<15)

#endif /* _ASM_ARCH_IO_H_ */
