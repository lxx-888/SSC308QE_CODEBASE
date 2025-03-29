/*
 * part_emmc.h- Sigmastar
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

#ifndef _DISK_PART_EMMC_H_
#define _DISK_PART_EMMC_H_

#include "part.h"

#define PART_TYPE_EMMC        0x40
#define EMMC_RESERVED_FOR_MAP 64
#define EMMC_RESERVED_FOR_KL  8192 /* reserve 4M space for kernel */
#define EMMC_STATUS_BOOTABLE  8    /* partition is bootable */

#define EMMC_DRIVER_MAGIC    0x1630
#define EMMC_PARTITION_MAGIC 0x5840

#define EMMC_PARTITION_START       615200 // ordinary partition data start block
#define EMMC_INNER_PARTITION_START 800    // can't be seen by user from emmc command or in kernel

#define EMMC_UPGRADE_PARTITION "upgrade" // the partition for emmc upragde

/*
 * Driver Descriptor Structure, in block 0.
 * This block is (and shall remain) 512 bytes long.
 * Note that there is an alignment problem for the driver descriptor map!
 */
typedef struct emmc_driver_desc
{
    __u16 signature;     /* expected to be EMMC_DRIVER_MAGIC  */
    __u16 blk_size;      /* block size of device */
    __u32 blk_count;     /* number of blocks on device */
    __u16 dev_type;      /* device type */
    __u16 dev_id;        /* device id */
    __u32 data;          /* reserved */
    __u16 drvr_cnt;      /* number of driver descriptor entries */
    __u16 drvr_map[247]; /* driver descriptor map */
} emmc_driver_desc_t;

/*
 * Device Driver Entry
 * (Cannot be included in emmc_driver_desc because of alignment problems)
 */
typedef struct emmc_driver_entry
{
    __u32 block; /* block number of starting block */
    __u16 size;  /* size of driver, in 512 byte blocks */
    __u16 type;  /* OS Type */
} emmc_driver_entry_t;

/*
 * Each Partition Map entry (in blocks 1 ... N) has this format:
 */
typedef struct EMMC_partition
{
    __u16 signature;     /* expected to be EMMC_PARTITION_MAGIC   */
    __u16 sig_pad;       /* reserved */
    __u32 map_count;     /* blocks in partition map   */
    __u32 start_block;   /* abs. starting block # of partition    */
    __u32 block_count;   /* number of blocks in partition  */
    uchar name[32];      /* partition name   */
    uchar type[32];      /* string type description */
    __u32 data_start;    /* rel block # of first data block   */
    __u32 data_count;    /* number of data blocks  */
    __u32 status;        /* partition status bits  */
    __u32 boot_start;    /* first block of boot code */
    __u32 boot_size;     /* size of boot code, in bytes  */
    __u32 boot_load;     /* boot code load address   */
    __u32 boot_load2;    /* reserved  */
    __u32 boot_entry;    /* boot code entry point */
    __u32 boot_entry2;   /* reserved   */
    __u32 boot_cksum;    /* boot code checksum  */
    uchar processor[16]; /* Type of Processor */
    __u16 part_pad[188]; /* reserved    */
#ifdef CONFIG_ISO_PARTITION
    uchar iso_dummy[2048]; /* Reservere enough room for an ISO partition block to fit */
#endif
} emmc_partition_t __attribute__((aligned(64)));

int get_NVRAM_max_part_count(void);
int ClearDescTable(void);
int create_new_NVRAM_partition(struct blk_desc *dev_desc, struct disk_partition *info);
int delete_NVRAM_all_partition(struct blk_desc *dev_desc);
int remove_NVRAM_partition(struct blk_desc *dev_desc, struct disk_partition *info);

#endif /* _DISK_PART_EMMC_H_ */
