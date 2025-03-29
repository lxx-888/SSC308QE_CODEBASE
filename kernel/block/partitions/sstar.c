/*
 * sstar.c- Sigmastar
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

#include <linux/ctype.h>
#include "check.h"
#include "sstar.h"

int sstar_emmc_partition(struct parsed_partitions *state)
{
    int slot = 1;
    Sector sect;
    unsigned char *data;
    int blk, blocks_in_map;
    unsigned secsize;
    struct sstar_emmc_partition *part;
    struct sstar_emmc_driver_desc *md;
       printk("sstar_emmc_partition()\n");
    /* Get 0th block and look at the first partition map entry. */
    md = read_part_sector(state, 0, &sect);
    if (!md)
        return -1;

    if (md->signature != SSTAR_EMMC_DRIVER_MAGIC) {
            //can not found the partiton map!
            printk("0x%x\n",md->signature);
            put_dev_sector(sect);
            return 0;
    }

    if(md->version == SSTAR_EMMC_PARTITIONTABLE_VERSION2 || md->version == SSTAR_EMMC_PARTITIONTABLE_VERSION3)
        blocks_in_map = md->drvr_cnt;
    else
        blocks_in_map = SSTAR_EMMC_RESERVED_FOR_MAP;

    //  secsize = be16_to_cpu(md->block_size);
       secsize = 512;
    put_dev_sector(sect);
    data = read_part_sector(state, secsize/512, &sect);
    if (!data)
        return -1;
    part = (struct sstar_emmc_partition *) (data + secsize%512);
    if (part->signature != SSTAR_EMMC_PARTITION_MAGIC) {
        put_dev_sector(sect);
        return 0;		/* not a emmc disk */
    }
    printk(" [emmc]");
    for (blk = 1; blk <= blocks_in_map; ++blk) {
        int pos = blk * secsize;
        put_dev_sector(sect);
        data = read_part_sector(state, pos/512, &sect);
        if (!data)
            return -1;
        part = (struct sstar_emmc_partition *) (data + pos%512);
        if (part->signature != SSTAR_EMMC_PARTITION_MAGIC)
            break;
             printk("Start_block=%d, block_count=%d\n",part->start_block,part->block_count);
        put_partition(state, slot,
            (part->start_block) * (secsize/512),
            (part->block_count) * (secsize/512));
        strcpy(state->parts[slot].info.volname, part->name); /* put parsed partition name into state */

        if (!strncasecmp(part->type, "Linux_RAID", 10))
            state->parts[slot].flags = ADDPART_FLAG_RAID;

        ++slot;
    }

    put_dev_sector(sect);
    printk("\n");
    return 1;
}
