/*
 * sstar_fb_cus.c - Sigmastar
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

#include <common.h>
#include <command.h>
#include <fastboot.h>
#include <fastboot-internal.h>
#include <nand.h>
#include "drv_part.h"
#include "sstar_fb_cus.h"

#define TRACE 1
#if TRACE
#define cus_trace(msg...) \
	do { \
			printf("FastBoot Cus: "msg); \
	} while (0)
#else
#define cus_trace(msg...)
#endif

#if defined (CONFIG_SSTAR_NAND)
static int cmd_nand_erase(const char *partname);
static int cmd_nand_write(const char *partname, void *download_buffer, unsigned int file_size);
static int cmd_nand_probe_info(const char *partname, struct part_info *part_info);
#elif defined (CONFIG_SSTAR_NOR)
static int cmd_sf_erase(const char *partname);
static int cmd_sf_write(const char *partname, void *download_buffer ,unsigned int file_size);
static int cmd_sf_probe_info(const char *partname, struct part_info *part_info);
#endif

#define PRE_PART_OPS(name, type)\
        {name, cmd_##type##_erase, cmd_##type##_write, cmd_##type##_probe_info}

const part_ops_t part_ops_infos[] =
{
#if defined (CONFIG_SSTAR_NAND)
	PRE_PART_OPS("CIS",	       nand),
	PRE_PART_OPS("BOOT",       nand), PRE_PART_OPS("BOOT_BAK",   nand),
	PRE_PART_OPS("KERNEL",     nand), PRE_PART_OPS("RECOVERY",   nand),
	PRE_PART_OPS("rootfs",     nand),
	PRE_PART_OPS("MISC",       nand),
	PRE_PART_OPS("ubia",       nand),
#elif defined (CONFIG_SSTAR_NOR)
	PRE_PART_OPS("BOOT",       sf),
	PRE_PART_OPS("KERNEL",     sf),
	PRE_PART_OPS("rootfs",     sf),
	PRE_PART_OPS("misc",       sf),
	PRE_PART_OPS("miservice",  sf),
	PRE_PART_OPS("customer",   sf),
#endif
};


#if defined (CONFIG_SSTAR_NAND)
static int cmd_nand_write_cis(const char *partname, void *download_buffer, unsigned int file_size)
{
    char cmd_write_cis[100] = "\0";
	int  i;
	int  ret;
	int  cis_offset_tbl[] = {0x0, 0x20000, 0x40000, 0x60000, 0x80000, 0xA0000, 0xC0000, 0xE0000, 0x100000, 0x120000};

	for (i = 0; i < ARRAY_SIZE(cis_offset_tbl); i++) {
	    sprintf(cmd_write_cis, "nand write 0x%p 0x%x 0x%x", download_buffer, cis_offset_tbl[i], file_size);
		cus_trace("[%s] command (%s)\n", __func__, cmd_write_cis);

		ret = run_command(cmd_write_cis, 0);
		if (ret) {
			cus_trace("[%s] command (%s) fail\n", __func__, cmd_write_cis);
			break;
		}
	}

    return ret;
}

static inline int cmd_nand_erase(const char *partname)
{
    char cmd_erase_partition[100] = "\0";

    sprintf(cmd_erase_partition, "nand erase.part %s", partname);
	cus_trace("[%s] command (%s)\n", __func__, cmd_erase_partition);

    return run_command(cmd_erase_partition, 0);
}

static inline int cmd_nand_write(const char *partname, void *download_buffer, unsigned int file_size)
{
    char cmd_write_partition[100] = "\0";

	if (!strcmp(partname, "CIS"))
		return cmd_nand_write_cis(partname, download_buffer, file_size);

    sprintf(cmd_write_partition, "nand write.e 0x%p %s 0x%x", download_buffer, partname, file_size);
	cus_trace("[%s] command (%s)\n", __func__, cmd_write_partition);

    return run_command(cmd_write_partition, 0);
}

static int set_dev(int dev)
{
#if 0
	if (dev < 0 || dev >= CONFIG_SYS_MAX_NAND_DEVICE ||
	    !nand_info[dev].name) {
		puts("No such device\n");
		return -1;
	}

	if (nand_curr_device == dev)
		return 0;

	printf("Device %d: %s", dev, nand_info[dev].name);
	puts("... is now current device\n");
	nand_curr_device = dev;

#ifdef CONFIG_SYS_NAND_SELECT_DEVICE
	board_nand_select_device(nand_info[dev].priv, dev);
#endif
#endif
	return 0;
}

static int get_nand_part_info(const char *partname, struct part_info *part_info)
{
#ifdef CONFIG_CMD_MTDPARTS
	struct mtd_device *dev;
	struct part_info *part;
	u8 pnum;
	int ret;

	ret = mtdparts_init();
	if (ret)
		return ret;

	ret = find_dev_and_part(partname, &dev, &pnum, &part);
	if (ret)
		return ret;

	if (dev->id->type != MTD_DEV_TYPE_NAND) {
		puts("not a NAND device\n");
		return -1;
	}
#if 0
	*off = part->offset;
	*size = part->size;
	*maxsize = part->size;
	*idx = dev->id->num;
#else
    part_info->size = part->size;
    cus_trace("%s part %s size %llx\n ", __func__, partname, part_info->size);
#endif

	ret = set_dev(dev->id->num);
	if (ret)
		return ret;

	return 0;
#else
	puts("offset is not a number\n");
	return -1;
#endif
}

static inline int cmd_nand_probe_info(const char *partname, struct part_info *part_info)
{
    return get_nand_part_info(partname, part_info);//only get total size of ubi part
}
#elif defined (CONFIG_SSTAR_NOR)
static inline int cmd_sf_erase(const char *partname)
{
	char cmd_erase_partition[100] = "\0";

	sprintf(cmd_erase_partition, "sf probe 0;sf erase %s", partname);
	cus_trace("[%s] command (%s)\n", __func__, cmd_erase_partition);

	return run_command(cmd_erase_partition, 0);
}

static inline int cmd_sf_write(const char *partname, void *download_buffer, unsigned int file_size)
{
    char cmd_write_partition[100] = "\0";

    sprintf(cmd_write_partition, "sf write 0x%p %s 0x%x", download_buffer, partname, file_size);
	cus_trace("[%s] command (%s)\n", __func__, cmd_write_partition);

    return run_command(cmd_write_partition, 0);
}

static inline int cmd_sf_probe_info(const char *partname, struct part_info *part_info)
{
	char ret;
	struct sstar_part part;

	ret = sstar_part_get(partname, 0, &part);

	if (ret) {
		cus_trace("%s get partition information fail\n", __func__);
		return ret;
	}
	cus_trace("%s partition %s size 0x%x offset 0x%x\n", __func__, part.part_name, part.size, part.offset);

	part_info->size = part.size;

    return 0;
}
#endif
int get_partition_index(const char *partname)
{
    int i;

    for(i = 0;i < ARRAY_SIZE(part_ops_infos); i++)
    {
        if (!strcmp(part_ops_infos[i].partname, partname))
            return i;
    }
    return -1;
}

void fastboot_cus_flash_write(const char *partname, void *download_buffer,
			unsigned int download_bytes, char *response)
{
    int ret, index;
    struct part_info part_info = {};

    index = get_partition_index(partname);
    if (index < 0)
    {
        fastboot_fail("no partition infomation", response);
        return;
    }

    ret = part_ops_infos[index].probe_info(partname, &part_info);
    if (ret < 0)
    {
        fastboot_fail("no such partition", response);
	    return;
    }

    if (part_info.size < download_bytes)
    {
        fastboot_fail("file size is to large", response);
	    return;
    }

    ret = part_ops_infos[index].erase(partname);
    if (ret!=CMD_RET_SUCCESS)
    {
        fastboot_fail("erase fail", response);
	    return;
    }

    ret = part_ops_infos[index].write(partname, download_buffer, download_bytes);
    if (ret!=CMD_RET_SUCCESS)
    {
        fastboot_fail("write fail", response);
	    return;
    }
    fastboot_okay(NULL, response);
}

void fastboot_cus_erase(const char *partname, char *response)
{
    int ret, index;
    struct part_info part_info = {};

    index = get_partition_index(partname);
    if (index < 0)
    {
        fastboot_fail("no such partition", response);
        return;
    }

    ret = part_ops_infos[index].probe_info(partname, &part_info);
    if (ret < 0)
    {
        fastboot_fail("no such partition", response);
	    return;
    }

    ret = part_ops_infos[index].erase(partname);
    if (ret!=CMD_RET_SUCCESS)
    {
        fastboot_fail("no such partition", response);
	    return;
    }
    fastboot_okay(NULL, response);
}

int fastboot_cus_get_part_info(const char *partname,
                struct part_info *part_info, char *response)
{
    int ret, index;
    char *sub;

	cus_trace("[%s] partname: (%s)\n", __func__, partname);
    //find the string "_a" or "_b"
    if(((sub = strpbrk(partname, "_")) && (strpbrk(sub + 1, "a"))) ||
       ((sub = strpbrk(partname, "_")) && (strpbrk(sub + 1, "b"))))
    {
        // no support slot
        return -1;
    }
    index = get_partition_index(partname);
    if (index < 0)
    {
        fastboot_fail("no such partition", response);
	    return -1;
    }

    ret = part_ops_infos[index].probe_info(partname, part_info);
    if (ret < 0)
    {
        fastboot_fail("no such partition", response);
	    return -1;
    }

    fastboot_okay(NULL, response);
    return 0;
}

