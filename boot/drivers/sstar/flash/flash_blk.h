/*
 * flash_blk.h- Sigmastar
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
#ifndef __FLASH_BLK_H__
#define __FLASH_BLK_H__

#include <blk.h>
#include <mtd.h>

#define PART_TYPE_BLK_FLASH    0x77
#define FLASH_RESERVED_FOR_MAP 64
#define PART_TYPE_SPINAND      "SPINAND"
#define PART_TYPE_SPINOR       "SPINOR"

struct sstar_flash_info
{
    u32 flash_con_type;

    struct mtd_info *mtd;

    u32 density; // total size
    /*
     * read() - read from a block device
     *
     * @start:	Start block number to read (0=first)
     * @blkcnt:	Number of blocks to read
     * @buffer:	Destination buffer for data read
     */
    int (*read)(struct udevice *udev, u32 start, u32 blkcnt, void *buffer);
    /*
     * write() - write to a block device
     *
     * @dev:	Device to write to
     * @start:	Start block number to write (0=first)
     * @blkcnt:	Number of blocks to write
     * @buffer:	Source buffer for data to write
     */
    int (*write)(struct udevice *udev, u32 start, u32 blkcnt, const void *buffer);
    /*
     * erase() - erase a section of a block device
     *
     * @dev:	Device to (partially) erase
     * @start:	Start block number to erase (0=first)
     * @blkcnt:	Number of blocks to erase
     */
    int (*erase)(struct udevice *udev, u32 start, u32 blkcnt);
};

#endif /* __FLASH_BLK_H__ */