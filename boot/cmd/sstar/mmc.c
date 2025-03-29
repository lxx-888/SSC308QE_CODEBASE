/*
 * mmc.c- Sigmastar
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
#include <mmc.h>
#include <log.h>
#include <asm/arch/mach/io.h>
#include <asm/arch/mach/platform.h>
#include <sstar_sys_utility.h>
#include <stdlib.h>
#include "part_emmc.h"
#include "include/cmd_sstar_common.h"
#include <ipl.h>
#include <memalign.h>
#include <div64.h>
#include <asm/gpio.h>
#include <linux/delay.h>
#ifdef CONFIG_DM
#include <dm.h>
#endif

#ifdef CONFIG_SSTAR_SUPPORT_SDIO
#include "../../drivers/sstar/mmc_host/inc/sstar_sdio.h"
struct sstar_mmc_plat
{
    struct mmc_config cfg;
    struct mmc        mmc;
    struct sdio_card  card;
};
#endif

#define msleep(a) udelay(a * 1000)

// Global Function
#define BUF_SIZE 8192

// emmc backup
#define RECOVERY_PART_NUM        (3)
#define RECOVERY_DATA_COPY_ADDR  (0x21000000)
#define RECOVERY_DATA_COPY_BATCH (0x1400000)

unsigned char tmp_buf[0x200];
ulong         used_blk;

/*
 * store the recovery parameter
 */
typedef struct
{
    char parta[100]; /* partition name to copy to */
    char partb[100];
    int  offset; /* offset to copy the data, block unit */
    int  size;   /* size to copy the data, block unit*/
} SS_RecoveryPrm;

extern int mmc_get_curr_device(void);

void show_speed(unsigned long time, unsigned long bytes)
{
    uint64_t speed; /* KiB/s */
    speed = (long long)bytes * 1000;
    do_div(speed, time * 1024);
    printf("show speed: %d KiB/s, ", (int)speed);
}

static u32 do_mmc_empty_check(const void *buf, u32 len, u32 empty_flag)
{
    int i;

    if ((!len) || (len & 511))
        return -1;

    for (i = (len >> 2) - 1; i >= 0; i--)
        if (((const uint32_t *)buf)[i] != empty_flag)
            break;

    /* The resulting length must be aligned to the minimum flash I/O size */
    len = ALIGN((i + 1) << 2, 512);
    return len;
}

static u32 do_mmc_write_emptyskip(struct mmc *mmc, s32 start_blk, u32 cnt_blk, const void *buf, u32 empty_skip)
{
    u32 n = 0;

    if (1 == empty_skip)
    { // 1 indicates skipping empty area, 0 means writing all the data
        u32       nn, empty_flag, rcnt, wcnt, cur_cnt = cnt_blk;
        int       boffset = start_blk;
        uintptr_t doffset = (uintptr_t)buf;

        if (mmc->ext_csd[181] != 0)
            empty_flag = 0;
        else
            empty_flag = 0xffffffff;

        while (cur_cnt > 0)
        {
            if (cur_cnt >= 0x800)
                wcnt = 0x800;
            else
                wcnt = cur_cnt;

            rcnt = do_mmc_empty_check((void *)doffset, (wcnt << 9), empty_flag);
            if (-1 == rcnt)
            {
                printf("The block num(0x%x) is wrong!", wcnt);
                return 0;
            }
            rcnt >>= 9;
            if (rcnt == 0)
            {
                boffset += wcnt;
                doffset += wcnt << 9;
                cur_cnt -= wcnt;
                n += wcnt;
                continue;
            }
            nn = blk_dwrite(mmc_get_blk_desc(mmc), boffset, rcnt, (void *)doffset);
            if (nn == rcnt)
                n += wcnt;
            else
            {
                n += nn;
                printf("Only 0x%x blk written to blk 0x%x\n, need 0x%x", nn, boffset, rcnt);
                return n;
            }

            boffset += wcnt;
            doffset += wcnt << 9;
            cur_cnt -= wcnt;
        }
    }
    else
        n = blk_dwrite(mmc_get_blk_desc(mmc), start_blk, cnt_blk, buf);

    return n;
}

// parm: direct 0-read, 1-write
static int mmc_boot_part_rw(u8 *data_buf, u32 data_byte_cnt, u32 blk_addr, u8 part_no, u8 direct)
{
    int              ret;
    struct mmc *     emmc;
    struct blk_desc *emmc_dev;
    u8               curr_dev = mmc_get_curr_device();

    emmc     = find_mmc_device(curr_dev);
    emmc_dev = mmc_get_blk_desc(emmc);

    ret = blk_select_hwpart_devnum(IF_TYPE_MMC, curr_dev, part_no);
    if (ret)
    {
        printf("switch to partitions #%d, %s\n", part_no, (!ret) ? "OK" : "ERROR");
        return ret;
    }

    if (direct)
        ret = blk_dwrite(emmc_dev, blk_addr, data_byte_cnt, data_buf);
    else
        ret = blk_dread(emmc_dev, blk_addr, data_byte_cnt, data_buf);

    ret = blk_select_hwpart_devnum(IF_TYPE_MMC, curr_dev, 0);
    if (ret)
    {
        printf("switch to partitions #0, %s\n", (!ret) ? "OK" : "ERROR");
        return ret;
    }
    return ret;
}

extern int mmc_uda_wp_op(struct mmc *mmc, u32 u32_addr, u8 u8_mode);
extern int mmc_get_uda_wp_status(struct mmc *mmc, u8 *pu8_buf, u32 u32_addr, u8 u8_mode);

// Set the target memory in the user partition to be write-protected
static u32 mmc_user_write_protect_option(struct mmc *mmc, u32 data_address, u32 blk_cnt, u8 mode)
{
    u32 u32_err = 0;
    u32 u32_start;
    ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, 8);

    // check if data_byte_cnt is 512B boundary
    if (data_address + blk_cnt > lldiv(mmc->capacity, mmc->read_bl_len))
    {
        pr_err("eMMC Err: invalid data range, %Xh > %Xh \n", data_address + blk_cnt,
               (u32)lldiv(mmc->capacity, mmc->read_bl_len));
        return -1;
    }

    if (mmc->ext_csd[171] & (BIT2 | BIT0))
    {
        pr_err("eMMC is not in tmp WP mode (%X)\n", mmc->ext_csd[171] & (BIT2 | BIT0));
        return -1;
    }

    switch (mode)
    {
        case 0:
        case 1:
            u32_start = ((data_address / mmc->hc_wp_grp_size) * mmc->hc_wp_grp_size);
            while (u32_start <= (data_address + blk_cnt - 1))
            {
                u32_err = mmc_uda_wp_op(mmc, u32_start, mode ? 0 : 1);
                if (u32_err)
                    return u32_err;

                u32_start += mmc->hc_wp_grp_size;
            }
            break;

        case 2:
        case 3:
            u32_err = mmc_get_uda_wp_status(mmc, buffer, data_address, (mode == 2) ? 1 : 0);
            if (u32_err)
                return -1;

            if (mode == 2)
                u32_err = (buffer[3] & BIT0);
            else
                u32_err = (buffer[7] & 0x03);

            break;

        default:
            pr_err("mmc mode(%d) is not support.\n", mode);
            break;
    }

    return u32_err;
}

/**
 * remove blank character in string str
 *
 * @param str	string should not be const
 * @return void
 */
static void remove_blank_char(char *str)
{
    char *str_c = str;
    int   i, j = 0;
    for (i = 0; str[i] != '\0'; i++)
    {
        if (str[i] != ' ')
            str_c[j++] = str[i];
    }
    str_c[j] = '\0';
    str      = str_c;
}

/**
 * Copy data from emmc to ram, then write to other parts of the emmc
 *
 * @param blkOffset	block offset of the data to be copied
 * @param size		size of the data to be copied,bytes unit
 * @part			partition name of the emmc to write data
 * @return 0 if ok, others on error
 */
static int recovery_write_copy_data(u32 blkOffset, u32 size, char *part)
{
    int  ret;
    int  i;
    int  batch_count;
    int  blk_size;
    int  blk_batch;
    int  blk_remain;
    int  offset;
    char command[100];

    blk_size  = ALIGN(size, 512) / 512;
    blk_batch = RECOVERY_DATA_COPY_BATCH / 512;

    batch_count = blk_size / blk_batch;
    blk_remain  = blk_size % blk_batch;

    /* erase the partition */
    sprintf(command, "emmc erase.p %s", part);
    ret = run_command(command, 0);

    /* copy the batch block data */
    for (i = 0; i < batch_count; i++)
    {
        offset = blkOffset + i * blk_batch;
        sprintf(command, "emmc read %x %x %x", RECOVERY_DATA_COPY_ADDR, offset, RECOVERY_DATA_COPY_BATCH);
        ret = run_command(command, 0);
        if (ret)
        {
            return ret;
        }

        offset = i * blk_batch;
        sprintf(command, "emmc write.p.continue %x %s %x %x", RECOVERY_DATA_COPY_ADDR, part, offset,
                RECOVERY_DATA_COPY_BATCH);
        ret = run_command(command, 0);
        if (ret)
        {
            return ret;
        }
    }

    /* copy the remain block data */
    if (0 != blk_remain)
    {
        offset = blkOffset + i * blk_batch;
        sprintf(command, "emmc read %x %x %x", RECOVERY_DATA_COPY_ADDR, offset, blk_remain * 512);
        ret = run_command(command, 0);
        if (ret)
        {
            return ret;
        }

        offset = i * blk_batch;
        sprintf(command, "emmc write.p.continue %x %s %x %x", RECOVERY_DATA_COPY_ADDR, part, offset,
                (blk_remain * 512));
        ret = run_command(command, 0);
        if (ret)
        {
            return ret;
        }
    }

    return 0;
}

/**
 * Check whether need to recovery the system by check the a gpio pin
 * If needed,copy the recovery data to systems zone (in emmc)
 *
 * @return 0 if ok, others on error
 */
int recovery_check(int force_recovery)
{
    int            flag = 0; /* record the valid paramter */
    int            loop;
    int            len;
    char *         env;
    MS_GPIO_NUM    pin_num;
    int            pin_level;
    int            level1, level2;
    char           str_temp[256];
    char *         str;
    char *         token;
    char           delim[]                            = ";";
    char *         recover_prm_str[RECOVERY_PART_NUM] = {"kernel=", "rootfs=", "user="};
    SS_RecoveryPrm rec_prm[RECOVERY_PART_NUM];
    char           temp1[100], temp2[100];
    int            ret;

    /* parse the recovery pin parameter */
    env = env_get("reckey");

    if (force_recovery)
        flag++;
    else if (NULL != env)
    {
        memset(str_temp, 0, sizeof(str_temp));
        memset(temp1, 0, sizeof(temp1));
        memset(temp2, 0, sizeof(temp2));

        strcpy(str_temp, env);
        sscanf(str_temp, "%[^=]=%d,%[^=]=%d", temp1, &pin_num, temp2, &pin_level);

        /* check the pin*/
        ret = gpio_request(pin_num, "reckey");
        if (ret && ret != -EBUSY)
        {
            printf("gpio: requesting pin %u failed\n", pin_num);
            return -1;
        }
        gpio_direction_input(pin_num);
        level1 = gpio_get_value(pin_num);
        if (pin_level != level1)
        {
            return 1;
        }

        mdelay(10);

        level2 = gpio_get_value(pin_num);
        if (pin_level != level2)
        {
            return 1;
        }

        flag += 1; /* valid pin */
    }

    /* parse the recovery offset and size parameter */
    env = env_get("recargs");
    if (NULL != env)
    {
        memset(str_temp, 0, sizeof(str_temp));
        strcpy(str_temp, env);
        remove_blank_char(str_temp);
        str = str_temp;

        loop = 0;
        for (token = strsep(&str, delim); token != NULL; token = strsep(&str, delim))
        {
            len = strlen(recover_prm_str[loop]);
            if (!strncmp(recover_prm_str[loop], token, len))
            {
                memset(temp1, 0, sizeof(temp1));
                memset(rec_prm[loop].parta, 0, sizeof(rec_prm[loop].parta));
                memset(rec_prm[loop].partb, 0, sizeof(rec_prm[loop].partb));

                sscanf(token, "%[^=]=%x,%x,%[^','],%s", temp1, &rec_prm[loop].offset, &rec_prm[loop].size,
                       rec_prm[loop].parta, rec_prm[loop].partb);

                loop++;
                if (loop >= RECOVERY_PART_NUM)
                {
                    break;
                }
            }
        }

        /* valid recovery parameter */
        if (RECOVERY_PART_NUM == loop)
        {
            flag += 1;
        }
    }

    if (2 == flag)
    {
        printf("recovery system start...\n");

        /* copy data from recovery to system*/
        for (loop = 0; loop < RECOVERY_PART_NUM; loop++)
        {
            recovery_write_copy_data(rec_prm[loop].offset, rec_prm[loop].size, rec_prm[loop].parta);
            recovery_write_copy_data(rec_prm[loop].offset, rec_prm[loop].size, rec_prm[loop].partb);
        }

        return 0;
    }

    return -1;
}

extern int mmc_switch(struct mmc *mmc, u8 set, u8 index, u8 value);

int emmc_wrrel_setting(struct mmc *mmc, u8 option, u8 value_set)
{
    int err;
    u8  wr_rel_set;
    ALLOC_CACHE_ALIGN_BUFFER(u8, ext_csd, MMC_MAX_BLOCK_LEN);

    if (IS_SD(mmc) || (mmc->version < MMC_VERSION_4_41))
    {
        pr_err("eMMC >= 4.4 required for enhanced user data area\n");
        return 2;
    }

    err = mmc_send_ext_csd(mmc, ext_csd);
    if (err)
        return EIO;

    /* The default value of EXT_CSD_WR_REL_SET is device
     * dependent, the values can only be changed if the
     * EXT_CSD_HS_CTRL_REL bit is set. The values can be
     * changed only once and before partitioning is completed. */
    wr_rel_set = ext_csd[EXT_CSD_WR_REL_SET];
    if (option)
    {
        if (value_set)
            wr_rel_set = 0x1F;
        else
            wr_rel_set &= ~EXT_CSD_WR_DATA_REL_USR;
    }
    else
    {
        printf("The UDA reliable writing function of the card is %s\n",
               (wr_rel_set & EXT_CSD_WR_DATA_REL_USR) ? "enabled" : "disabled");
        return 0;
    }

    if ((ext_csd[EXT_CSD_RST_N_FUNCTION] != 1) && (ext_csd[EXT_CSD_RST_N_FUNCTION] == 0))
    {
        err = mmc_set_rst_n_function(mmc, 1); // hw reset is permanently enabled
        if (err)
            return EIO;
    }

    if (wr_rel_set == ext_csd[EXT_CSD_WR_REL_SET])
    {
        printf("The UDA reliable writing function of the card is already %s\n",
               (wr_rel_set & EXT_CSD_WR_DATA_REL_USR) ? "enabled" : "disabled");

        return 2;
    }

    if (wr_rel_set != ext_csd[EXT_CSD_WR_REL_SET] && !(ext_csd[EXT_CSD_WR_REL_PARAM] & EXT_CSD_HS_CTRL_REL))
    {
        pr_err(
            "Card does not support host controlled partition write "
            "reliability settings\n");
        printf("The reliable writing value of the card is 0x%x\n", ext_csd[EXT_CSD_WR_REL_SET]);
        return 2;
    }

    if (ext_csd[EXT_CSD_PARTITION_SETTING] & EXT_CSD_PARTITION_SETTING_COMPLETED)
    {
        pr_err("Card has been partitioned already, and cannot be configured for write reliability anymore\n");
        printf("The reliable writing value of the card is 0x%x\n", ext_csd[EXT_CSD_WR_REL_SET]);
        return 2;
    }

    /* Partitioning requires high-capacity size definitions */
    if (!mmc->hc_wp_grp_size)
    {
        pr_err("Card does not define HC WP group size\n");
    }
    else if (!(ext_csd[EXT_CSD_ERASE_GROUP_DEF] & 0x01))
    {
        err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_ERASE_GROUP_DEF, 1);
        if (err)
            return EIO;

        ext_csd[EXT_CSD_ERASE_GROUP_DEF] = 1;
#if CONFIG_IS_ENABLED(MMC_WRITE)
        /* update erase group size to be high-capacity */
        mmc->erase_grp_size = ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * 1024;
#endif
    }

    /* The WR_REL_SET is a write-once register but shall be
     * written before setting PART_SETTING_COMPLETED. As it is
     * write-once we can only write it when completing the
     * partitioning. */
    if (wr_rel_set != ext_csd[EXT_CSD_WR_REL_SET])
    {
        err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_WR_REL_SET, wr_rel_set);
        if (err)
            return EIO;
    }

    /* Setting PART_SETTING_COMPLETED confirms the partition
     * configuration but it only becomes effective after power
     * cycle, so we do not adjust the partition related settings
     * in the mmc struct. */

    err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_PARTITION_SETTING, EXT_CSD_PARTITION_SETTING_COMPLETED);
    if (err)
        return EIO;

    return 0;
}

static int do_emmc_wrrel(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    u8          wr_rel_set;
    struct mmc *emmc;
    int         err            = -1;
    int         emmc_dev_index = mmc_get_curr_device();

    if (emmc_dev_index < 0)
    {
        if (get_mmc_num() > 0)
            emmc_dev_index = 0;
        else
        {
            puts("No MMC device available\n");
            return 1;
        }
    }

    emmc = find_mmc_device(emmc_dev_index);

    if (!emmc)
    {
        printf("no mmc device at slot %x\n", emmc_dev_index);
        return CMD_RET_FAILURE;
    }

    if (!(emmc->has_init))
    {
        printf("Do mmc init first!\n");
        if (mmc_init(emmc))
            return CMD_RET_FAILURE;
        emmc->has_init = 1;
    }

    if (argc < 2)
    {
        return CMD_RET_USAGE;
    }

    if (!strcmp(argv[1], "check"))
    {
        if (!emmc_wrrel_setting(emmc, 0, 0))
            return CMD_RET_SUCCESS;
        else
            return CMD_RET_FAILURE;
    }
    else if (!strcmp(argv[1], "on") || !strcmp(argv[1], "off"))
    {
        bool fastboot = false;

        wr_rel_set = !strcmp(argv[1], "on") ? 1 : 0;

        err = emmc_wrrel_setting(emmc, 1, wr_rel_set);
        if (!err)
        {
            printf("\t Set user partition write reliability: %s successfully\n", argv[1]);
            if (argc >= 3 && (!strcmp(argv[2], "fastboot")))
            {
                fastboot = true;
            }
            else
            {
                while (1)
                {
                    printf(
                        "Write reliabilty has been %s, please power off, then power on the device once to make "
                        "effect\n",
                        wr_rel_set ? "enabled" : "disabled");
                    printf("Please notice the current data in emmc maybe lost after power cycle\n");
                    msleep(3 * 1000);
                }
            }
            return fastboot ? CMD_RET_FAILURE : CMD_RET_SUCCESS;
        }
        else if (err == 2)
        {
            return CMD_RET_SUCCESS;
        }
        else
        {
            return CMD_RET_FAILURE;
        }
    }

    return CMD_RET_USAGE;
}
U_BOOT_CMD(emmc_wrrel, CONFIG_SYS_MAXARGS, 0, do_emmc_wrrel, "Set write reliability for eMMC UDA ",
           "emmc_wrrel optition <mode> \n"
           "option: \n"
           "       check - Check if EMMC UDA supports write reliability \n"
           "       on    - enable write reliability for eMMC UDA \n"
           "       off   - disable write reliability for eMMC UDA \n");

int do_emmc(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    int                   emmc_dev_index = mmc_get_curr_device();
    struct mmc *          emmc;
    struct blk_desc *     emmc_dev;
    struct disk_partition dpt;
    unsigned long         timer_diff = 0, base_timer = 0;

    if (argc < 2)
        return CMD_RET_USAGE;

    // emmc_dev_index = CONFIG_SSTAR_EMMC_DEV_INDEX;     //emmc_dev_index set to 0

    if (emmc_dev_index < 0)
    {
        if (get_mmc_num() > 0)
            emmc_dev_index = 0;
        else
        {
            puts("No MMC device available\n");
            return 1;
        }
    }

    emmc = find_mmc_device(emmc_dev_index);

    if (!emmc)
    {
        printf("no mmc device at slot %x\n", emmc_dev_index);
        return 1;
    }

    if (!(emmc->has_init))
    {
        printf("Do mmc init first!\n");
        mmc_init(emmc);
        emmc->has_init = 1;
    }

    emmc_dev = blk_get_devnum_by_type(IF_TYPE_MMC, emmc_dev_index);

    if ((emmc_dev == NULL) || (emmc_dev->type == DEV_TYPE_UNKNOWN))
    {
        printf("no mmc device found!\n");
        return 1;
    }

    if (PART_TYPE_EMMC != emmc_dev->part_type)
    {
        printf("dev %d is not EMMC base partition!(part_type:%d)\n", emmc_dev_index, emmc_dev->part_type);
        // TBD:
        if (PART_TYPE_UNKNOWN != emmc_dev->part_type)
            return 1;
    }

#if 0
    if (strncmp(argv[1], "ipverify", 8) == 0) {
        eMMC_IPVerify_Main();
        while(1);
    }
#endif
    if (strncmp(argv[1], "create", 6) == 0)
    {
        strcpy((char *)&dpt.name, argv[2]);
        if (simple_strtoull(argv[3], NULL, 16) == 0x01)
            dpt.size = emmc_dev->lba - used_blk - 0xD000;
        else
        {
            dpt.size = ALIGN(simple_strtoull(argv[3], NULL, 16), 512) / 512;
            used_blk += dpt.size;
        }
        // printf("1111 emmc_dev->lba=%lx\n", emmc_dev->lba);
        // printf("1111 dpt.size=%lx\n", dpt.size);
        if (argc > 4)
            dpt.start = ALIGN(simple_strtoull(argv[4], NULL, 16), 512) / 512;
        else
            dpt.start = 0;

        if (create_new_NVRAM_partition(emmc_dev, &dpt) == 0)
        {
            printf("Add new NVRAM Partition %s success!\n", dpt.name);
            return 0;
        }
        return 1;
    }
    else if (strncmp(argv[1], "remove", 6) == 0)
    {
        strcpy((char *)&dpt.name, (const char *)argv[2]);
        if (remove_NVRAM_partition(emmc_dev, &dpt) == 0)
        {
            printf("Remove partition %s success!\n", dpt.name);
            return 0;
        }
        return 1;
    }
    else if (strncmp(argv[1], "rmgpt", 5) == 0)
    {
        if (delete_NVRAM_all_partition(emmc_dev) == 0)
        {
            printf("delete all partition success!\n");
            return 0;
        }
        return 0;
    }
    else if (strcmp(argv[1], "unlzo") == 0)
    {
        int ret = 0;

        if (argc < 5)
        {
            printf("Usage:\n%s\n", cmdtp->usage);
            return 1;
        }
#if 0
        ret = do_unlzo(emmc, argc, argv);
#else
        printf("unsupport unlzo\n");
        return 1;
#endif

        return ret;
    }
#if 0
    else if (strncmp(argv[1], "slc", 3) == 0)
    {
        unsigned long long size;
        int                reliable_write, ret;

        if (argc < 4)
        {
            printf("Usage:\n%s\n", cmdtp->usage);
            return 0;
        }

        size           = simple_strtoul(argv[2], NULL, 16);
        reliable_write = simple_strtoul(argv[3], NULL, 16);
        if ((reliable_write != 0) && (reliable_write != 1))
        {
            printf("Reliable write enable can only be set to be 0 or 1!!!\n");
            return 0;
        }
        if ((size == 0) && (reliable_write == 0))
        {
            printf("Both of slc size and reliable write configuration are zero, please input proper values!!!\n");
            return 0;
        }

        ret = mmc_slc_mode(emmc, size, reliable_write);

        return ret;
    }
#endif
    else if (strncmp(argv[1], "part", 5) == 0)
    {
        part_print(emmc_dev);
        return 0;
    }
    else if (strncmp(argv[1], "read", 4) == 0)
    {
        void *addr = (void *)simple_strtoul(argv[2], NULL, 16);
        u32   n, n2, cnt, size, tail = 0, partnum = 1;
        s32   blk         = -1;
        char *cmdtail     = strchr(argv[1], '.');
        char *cmdlasttail = strrchr(argv[1], '.');

        size = simple_strtoul(argv[4], NULL, 16);
        cnt  = ALIGN(size, 512) / 512;

        if ((cmdtail) && (!strncmp(cmdtail, ".p", 2)))
        {
            for (partnum = 1; partnum < get_NVRAM_max_part_count(); partnum++)
            {
                int res = part_get_info(emmc_dev, partnum, &dpt);

                if (res > 0)
                    continue;
                else if (res < 0)
                    break;

                if (!strcmp(argv[3], (const char *)dpt.name))
                {
                    blk = dpt.start;
                    if (!strncmp(cmdlasttail, ".continue", 9))
                    {
                        blk += simple_strtoul(argv[4], NULL, 16);
                        size = simple_strtoul(argv[5], NULL, 16);
                        cnt  = ALIGN(size, 512) / 512;
                    }
                    break;
                }
            }
        }
#if 0
        else if ((cmdtail) && (!strncmp(cmdtail, ".cpart", 6)))
        {
            blk  = simple_strtoul(argv[4], NULL, 16);
            size = simple_strtoul(argv[5], NULL, 16);
            cnt  = ALIGN(size, 512) / 512;
        }
#endif
        else if ((cmdtail) && (!strncmp(cmdtail, ".boot", 5)))
        {
            addr = (void *)simple_strtoul(argv[3], NULL, 16);
            blk  = simple_strtoul(argv[4], NULL, 16);
            size = simple_strtoul(argv[5], NULL, 16);
            cnt  = ALIGN(size, 512) / 512;
        }
        else
            blk = simple_strtoul(argv[3], NULL, 16);

        if (blk < 0)
        {
            printf("ERR:Please check the blk# or partiton name!\n");
            return 1;
        }

        /* unaligned size is allowed */
        if ((cnt << 9) > size)
        {
            cnt--;
            tail = size - (cnt << 9);
        }

        printf("\nMMC read: dev # %d, block # %d, count %d ... ", emmc_dev_index, blk, cnt);

#if defined(MMC_SPEED_TEST) && MMC_SPEED_TEST
        FCIE_HWTimer_Start();
#endif

#if 0
        n = mmc->block_dev.block_read(curr_device, blk, cnt, addr);
#else
        if ((cmdtail) && (!strncmp(cmdtail, ".boot", 5)))
        {
            if (strncmp(argv[2], "1", 1) == 0)
                n = mmc_boot_part_rw(addr, cnt << 9, blk, 1, 0);
            else if (strncmp(argv[2], "2", 1) == 0)
                n = mmc_boot_part_rw(addr, cnt << 9, blk, 2, 0);
            else
            {
                printf("mmc access boot partition parameter not found!\n");
                return 1;
            }
            n = (n == 0) ? cnt : -1;

            if (tail)
            {
                if (strncmp(argv[2], "1", 1) == 0)
                    n2 = mmc_boot_part_rw(tmp_buf, 512, (blk + cnt), 1, 0);
                else if (strncmp(argv[2], "2", 1) == 0)
                    n2 = mmc_boot_part_rw(tmp_buf, 512, (blk + cnt), 2, 0);
                else
                {
                    printf("mmc access boot partition parameter not found!\n");
                    return 1;
                }

                n2 = (n2 == 0) ? 1 : -1;
                memcpy(((unsigned char *)addr + (cnt << 9)), tmp_buf, tail);
                n += n2;
                cnt++;
            }
        }
#if 0
        else if ((cmdtail) && (!strncmp(cmdtail, ".cpart", 6)))
        {
            u16 u16_PartType = 0;
            if (strncmp(argv[3], "uboot", 5) == 0)
                u16_PartType = eMMC_PART_EBOOT;
            else if (strncmp(argv[3], "emmcenv", 7) == 0)
                u16_PartType = eMMC_PART_ENV;

            if (u16_PartType)
            {
                //              n = eMMC_ReadPartition(u16_PartType, addr, blk, cnt, 0);
                n = (n == 0) ? cnt : -1;
            }
            else
            {
                printf("Invalid Partition name for PNI partition\n");
                return -1;
            }
        }
#endif
        else
        {
            base_timer = get_timer(0);
            if (cnt > 0)
                n = blk_dread(emmc_dev, blk, cnt, addr);
            else if (cnt == 0)
                n = 0;

            if (tail)
            {
                n2 = blk_dread(emmc_dev, (blk + cnt), 1, tmp_buf);
                n2 = (n2 == 1) ? 1 : -1;
                memcpy(((unsigned char *)addr + (cnt << 9)), tmp_buf, tail);
                n += n2;
                cnt++;
            }
            timer_diff = get_timer(base_timer);
            show_speed(timer_diff, (unsigned long)(n * 512));
        }
#endif

        env_set_hex("filesize", size);
        printf("%d blocks read: %s\n", n, (n == cnt) ? "OK" : "ERROR");

        return (n == cnt) ? 0 : 1;
    }
    else if (strncmp(argv[1], "write", 5) == 0)
    {
        // if(argc < 5)
        //     return cmd_usage(cmdtp);
        void *addr = (void *)simple_strtoul(argv[2], NULL, 16);
        u32   n, cnt, partnum = 0, empty_skip = 0, cont = 0;
        char *cmdtail     = strchr(argv[1], '.');
        char *cmdlasttail = strrchr(argv[1], '.');
        s32   blk         = -1;

        cnt = ALIGN(simple_strtoul(argv[4], NULL, 16), 512) / 512;

        if ((cmdtail) && (!strncmp(cmdtail, ".p", 2)))
        {
            for (partnum = 1; partnum < get_NVRAM_max_part_count(); partnum++)
            {
                int res = part_get_info(emmc_dev, partnum, &dpt);

                if (res > 0)
                    continue;
                else if (res < 0)
                    break;

                if (!strcmp(argv[3], (const char *)dpt.name))
                {
                    blk = dpt.start;
                    if (!strncmp(cmdlasttail, ".continue", 9))
                    {
                        blk += simple_strtoul(argv[4], NULL, 16);
                        cnt  = ALIGN(simple_strtoul(argv[5], NULL, 16), 512) / 512;
                        cont = 1;
                    }
                    break;
                }
            }
        }
#if 0
        else if ((cmdtail) && (!strncmp(cmdtail, ".cpart", 6)))
        {
            // argv[3] = type, argv[4] = startblk in partition, argv[5] = blkcnt
            blk = simple_strtoul(argv[4], NULL, 16);
            cnt = ALIGN(simple_strtoul(argv[5], NULL, 16), 512) / 512;
        }
#endif
        else if ((cmdtail) && (!strncmp(cmdtail, ".boot", 5)))
        {
            addr = (void *)simple_strtoul(argv[3], NULL, 16);
            blk  = simple_strtoul(argv[4], NULL, 16);
            cnt  = ALIGN(simple_strtoul(argv[5], NULL, 16), 512) / 512;
        }
        else
            blk = simple_strtoul(argv[3], NULL, 16);

        if (partnum == get_NVRAM_max_part_count())
        {
            printf("ERR:Can not found partiton with name %s!\n", argv[3]);
            return 1;
        }

        if (blk < 0)
        {
            printf("ERR:Please check the blk# or partiton name!\n");
            return 1;
        }

        printf("\nMMC write: dev # %d, block # %d, count %d ... ", emmc_dev_index, blk, cnt);

#if defined(MMC_SPEED_TEST) && MMC_SPEED_TEST
        FCIE_HWTimer_Start();
#endif

#if 0
        n = mmc->block_dev.block_write(curr_device, blk, cnt, addr);
#else
        if ((cmdtail) && (!strncmp(cmdtail, ".boot", 5)))
        {
            if (strncmp(argv[2], "1", 1) == 0)
                n = mmc_boot_part_rw(addr, cnt << 9, blk, 1, 1);
            else if (strncmp(argv[2], "2", 1) == 0)
                n = mmc_boot_part_rw(addr, cnt << 9, blk, 2, 1);
            else
            {
                printf("mmc access boot partition parameter not found!\n");
                return 1;
            }

            n = (n == 0) ? cnt : -1;
        }
#if 0
        else if ((cmdtail) && (!strncmp(cmdtail, ".cpart", 6)))
        {
            u16 u16_PartType = 0;

            if (strncmp(argv[3], "uboot", 5) == 0)
                u16_PartType = eMMC_PART_EBOOT;
            else if (strncmp(argv[3], "emmcenv", 7) == 0)
                u16_PartType = eMMC_PART_ENV;

            if (u16_PartType)
            {
                //              n = eMMC_WritePartition(u16_PartType, addr, blk, cnt, 0);
                n = (n == 0) ? cnt : -1;
            }
            else
            {
                printf("Invalid Partition name for PNI partition\n");
                return -1;
            }
        }
#endif
        else
        {
            if ((argc == 6) && (cont == 0))
                empty_skip = simple_strtoul(argv[5], NULL, 16);

            if ((argc == 7) && (cont == 1))
                empty_skip = simple_strtoul(argv[6], NULL, 16);
            base_timer = get_timer(0);
            n          = do_mmc_write_emptyskip(emmc, blk, cnt, addr, empty_skip);
            timer_diff = get_timer(base_timer);
            show_speed(timer_diff, (unsigned long)(n * 512));
        }
#endif

        printf("%d blocks written: %s\n", n, (n == cnt) ? "OK" : "ERROR");
        return (n == cnt) ? 0 : 1;
    }
    else if (strncmp(argv[1], "erase", 5) == 0)
    {
        u32   boot_partition = 0; // default user area partition
        u32   n, cnt = 0, partnum = 0;
        char *cmdtail    = strchr(argv[1], '.');
        u64   erase_size = 0;
        s32   start      = -1;

        if (argc == 4)
        {
            start      = simple_strtoul(argv[2], NULL, 16);
            erase_size = simple_strtoul(argv[3], NULL, 16); // Bytes
            if ((erase_size <= 0) || (erase_size & 0x1FF))
            {
                printf("invalied erase size, must aligned to 512 bytes\n");
                return 1;
            }

            cnt = erase_size >> 9; // /unit 512B
        }

        if ((cmdtail) && (!strncmp(cmdtail, ".p", 2)))
        {
            if (argc != 3) // not specify partition name
                return cmd_usage(cmdtp);

            for (partnum = 1; partnum < get_NVRAM_max_part_count(); partnum++)
            {
                int res = part_get_info(emmc_dev, partnum, &dpt);

                if (res > 0)
                    continue;
                else if (res < 0)
                    break;

                if (!strcmp(argv[2], (const char *)dpt.name))
                {
                    start = dpt.start;
                    cnt   = dpt.size; // block number
                    break;
                }
            }

            if (cnt == 0)
            {
                printf("ERR:invalid parameter, please check partiton name!\n");
                return 1;
            }
        }
        else if (argc == 2)
        { // erase all blocks in user area partiiton
            start = 0;
            cnt   = emmc_dev->lba;
        }

        if (start < 0)
        {
            printf("ERR:invalid parameter, please check the blk# or partiton name!\n");
            return 1;
        }

        if (cnt <= 0)
        {
            printf("ERR:invalid parameter, Please check size!\n");
            return 1;
        }

        //        if(((!boot_partition)&&(cnt > emmc->block_dev.lba))
        //        ||((boot_partition)&&(cnt>g_eMMCDrv.u32_BOOT_SEC_COUNT)))
        //            printf("ERR:invalid parameter, please check the size#!\n");

        printf("\nMMC erase: dev # %d, %s part, block # %d, count %d ... \n", emmc_dev_index,
               boot_partition ? "boot" : "user area", start, cnt);

        if (!boot_partition)
        {
            n = blk_derase(emmc_dev, start, cnt);
        }
        //        else {
        //
        //            if (strncmp(argv[2], "1", 1) == 0)
        //                n = eMMC_EraseBootPart(start, start + cnt - 1, 1);
        //            else if (strncmp(argv[2], "2", 1) == 0)
        //                n = eMMC_EraseBootPart(start, start + cnt - 1, 2);
        //            else
        //            {
        //                printf("mmc access boot partition parameter not found!\n");
        //                return 1;
        //            }
        //
        //            n = (n == 0) ? cnt : -1;
        //        }

        printf("%d blocks erase: %s\n", n, (n == cnt) ? "OK" : "ERROR");
        return (n == cnt) ? 0 : 1;
    }
    else if (strncmp(argv[1], "recovery", 8) == 0)
        recovery_check(1);
    else
        return CMD_RET_USAGE;

    return 1;
}

U_BOOT_CMD(emmc, CONFIG_SYS_MAXARGS, 1, do_emmc, "EMMC function on NVRAM base partition",
           "emmc create [name] [size] - create mmc partition [name]\n"
           "emmc remove [name] - remove mmc partition [name]\n"
           "emmc rmgpt - clean all mmc partition table\n"
           "emmc part - list partitions \n"
           "emmc slc size relwr - set slc in the front of user area,  0xffffffff means max slc size\n"
           "emmc unlzo Src_Address Src_Length Partition_Name [empty_skip:0-disable,1-enable]- decompress lzo file and "
           "write to mmc partition \n"
           "emmc read.p addr partition_name size\n"
           "emmc read.p.continue addr partition_name offset size\n"
           "emmc write.p addr partition_name size [empty_skip:0-disable,1-enable]\n"
           "emmc write.p.continue addr partition_name offset size [empty_skip:0-disable,1-enable]\n"
           "emmc erase.p partition_name\n");

static int do_emmc_get_cust(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    u32  cust_address, ipl_size, boot_part_address;
    char get_cust_address[10];

    if (argc > 2)
    {
        printf("error: Illegality parameter! \n");
        goto error;
    }

    boot_part_address = (unsigned int)simple_strtoul(argv[1], NULL, 16);
    if (boot_part_address < 0X20000000)
    {
        printf("error: Illegality BOOTPART.bin Address! \n");
        goto error;
    }

    if (image_get_ipl_magic((uintptr_t)boot_part_address) != IPL__HEADER_CHAR)
    {
        printf("error: BOOPART image is broken! wrong ipl head: 0x%08x\n",
               image_get_ipl_magic((uintptr_t)boot_part_address));
        goto error;
    }

    ipl_size     = image_get_ipl_size((uintptr_t)boot_part_address);
    cust_address = boot_part_address + ipl_size + RSA_SIG_LEN;

    if (image_get_ipl_magic((uintptr_t)cust_address) != IPLK_HEADER_CHAR)
    {
        printf("error: BOOPART image is broken! wrong cust head: 0x%08x\n",
               image_get_ipl_magic((uintptr_t)cust_address));
        goto error;
    }

    sprintf(get_cust_address, "%08x", cust_address);
    env_set("cust_address", get_cust_address);

    return 0;

error:
    printf("=================================================================\n");
    return -1;
}

U_BOOT_CMD(emmc_get_cust, CONFIG_SYS_MAXARGS, 1, do_emmc_get_cust, "Find cust address in BOOT_PART.bin", "");

int do_emmc_wp(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    u32         address, size = 1;
    u8          mode;
    int         emmc_dev_index = mmc_get_curr_device();
    struct mmc *mmc;

    if (emmc_dev_index < 0)
    {
        if (get_mmc_num() > 0)
            emmc_dev_index = 0;
        else
        {
            printf("No MMC device available\n");
            return 1;
        }
    }

    mmc = find_mmc_device(emmc_dev_index);

    if (!mmc)
    {
        printf("no mmc device at slot %x\n", emmc_dev_index);
        return 1;
    }

    if (!(mmc->has_init))
    {
        printf("Do mmc init first!\n");
        mmc_init(mmc);
    }

    if ((argc != 3) && (argc != 4))
        return CMD_RET_USAGE;

    address = (unsigned int)simple_strtoul(argv[2], NULL, 16);
    if (argc == 4)
        size = (unsigned int)simple_strtoul(argv[3], NULL, 16);

    if (strncmp(argv[1], "-s", 8) == 0)
        mmc_user_write_protect_option(mmc, address, size, 0);
    else if (strncmp(argv[1], "-c", 8) == 0)
        mmc_user_write_protect_option(mmc, address, size, 1);
    else if (strncmp(argv[1], "-q", 8) == 0)
    {
        mode = mmc_user_write_protect_option(mmc, address, size, 2);
        printf("eMMC USER part address(0x%08x)'s write protection is %d\n", address, mode);
    }
    else if (strncmp(argv[1], "-qt", 8) == 0)
    {
        mode = mmc_user_write_protect_option(mmc, address, size, 3);
        printf("eMMC USER part address(0x%08x)'s write protection type is %d\n", address, mode);
    }
    else
        return CMD_RET_USAGE;

    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(emmc_wp, CONFIG_SYS_MAXARGS, 0, do_emmc_wp, "eMMC write protection related",
           " Set/Clear/Ask the eMMC memory's write protection\n"
           " emmc_wp -o [address] <size>\n"
           " -o:\n"
           "    s  : Set the address of write group is write protected\n"
           "    c  : Clear the address of write group from write protected\n"
           "    q  : Ask the address of write group if is in write protected? (0:invalid, 1:valid)\n"
           "    qt : Ask the address of write group's write protection type. (0:not protected, 1:temporary, "
           "2:power-on, 3:permanent)\n"
           " [address] and <size> is in bytes or block, it's depend on eMMC's HC\n");

/**
 * If can't boot kernel with default env,it will try the backup one
 *
 * @return 0 if ok, others on error
 */
static int recovery_to_backup(char *env_name_boot_cmd, char *env_name_boot_arg)
{
    char *env;

    printf("Cover \"bootcmd\" with %s, and cover \"bootargs\" with %s, then try backup one...\n", env_name_boot_cmd,
           env_name_boot_arg);

    env = env_get(env_name_boot_cmd);
    if (NULL != env)
    {
        env_set("bootcmd", env);
    }

    env = env_get(env_name_boot_arg);
    if (NULL != env)
    {
        env_set("bootargs", env);
    }

    run_command("saveenv", 0);

    run_command("reset", 0);

    return 0;
}

int do_restart_bk(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    char env_name_boot_cmd[32] = {'b', 'o', 'o', 't', 'c', 'm', 'd', 'b', 'p'};
    char env_name_boot_arg[32] = {'b', 'o', 'o', 't', 'a', 'r', 'g', 's', 'b', 'p'};

    if (argc == 3)
    {
        strcpy(env_name_boot_cmd, argv[1]);
        strcpy(env_name_boot_arg, argv[2]);
    }
    else if ((argc == 2) || (argc > 3))
        return CMD_RET_USAGE;

    recovery_to_backup(env_name_boot_cmd, env_name_boot_arg);

    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(bootbp, CONFIG_SYS_MAXARGS, 0, do_restart_bk, "retry start linux with 'bootcmdbp' and 'bootargsbp'",
           "<bootcmdbp> <bootargsbp>\n"
           "  <bootcmdbp>   -optional, the env name of backup bootcmd. default is \"bootcmdbp\".\n"
           "  <bootargsbp>  -optional, the env name of backup bootargs. default is \"bootargsbp\".\n");

// load script from SD casd
int do_dstar(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    char targFile[255];
    char devPart[8] = {'0', ':', '1'};
    ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, BUF_SIZE);
    int i;

    if ((argc != 1) && (argc != 3) && (argc != 5))
        return CMD_RET_USAGE;

    strcpy(targFile, "auto_update.txt");

    for (i = 1; i <= 2; i++)
    {
        if (argc >= 1 + i * 2)
        {
            if (!strncmp(argv[2 * i - 1], "-f", 2))
                strcpy(targFile, argv[2 * i]);
            else if (!strncmp(argv[2 * i - 1], "-i", 2))
                strcpy(devPart, argv[2 * i]);
            else
                return CMD_RET_USAGE;
        }
    }

    char *script_buf;
    char *next_line;

#if defined(CONFIG_LMB) && defined(CONFIG_SSTAR_ESTAR_LOAD_SCRIPT_ADDR)
    script_buf = (char *)CONFIG_SSTAR_ESTAR_LOAD_SCRIPT_ADDR;
#else
    script_buf = buffer;
#endif

    sprintf(script_buf, "fatload mmc %s %llX %s", devPart, (U64)(uintptr_t)script_buf, targFile);
    printf("\n>> %s \n", script_buf);
    if (run_command(script_buf, 0))
        return CMD_RET_FAILURE;

#if defined(CONFIG_LMB) && defined(CONFIG_SSTAR_ESTAR_LOAD_SCRIPT_ADDR)
    memcpy((void *)buffer, (void *)script_buf, BUF_SIZE);
#endif

    script_buf = buffer;

    while ((next_line = get_script_next_line(&script_buf)) != NULL)
    {
        printf("\n>> %s \n", next_line);
        if (run_command(next_line, 0))
            return CMD_RET_FAILURE;
    }

    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(dstar, CONFIG_SYS_MAXARGS, 1, do_dstar, "script via SD/MMC with FAT filesystem",
           "<-i dev[:part]> <-f update_file>\n"
           "  -i    Select the target mmc device and part, default is \"0:1\".\n"
           "  -f    Select the update script file in SD card, default is \"auto_update.txt\".\n");

int do_sdstar(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    char targFile[255], env_buf[255];
    char devPart[8] = {'0', ':', '1'};
    ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, BUF_SIZE);
    int i;

    if ((argc != 1) && (argc != 3) && (argc != 5))
        return CMD_RET_USAGE;

    strcpy(targFile, "SigmastarUpgradeSD.bin");

    for (i = 1; i <= 2; i++)
    {
        if (argc >= 1 + i * 2)
        {
            if (!strncmp(argv[2 * i - 1], "-f", 2))
                strcpy(targFile, argv[2 * i]);
            else if (!strncmp(argv[2 * i - 1], "-i", 2))
                strcpy(devPart, argv[2 * i]);
            else
                return CMD_RET_USAGE;
        }
    }

    char *script_buf;
    char *next_line;

#if defined(CONFIG_LMB) && defined(CONFIG_SSTAR_ESTAR_LOAD_SCRIPT_ADDR)
    script_buf = (char *)CONFIG_SSTAR_ESTAR_LOAD_SCRIPT_ADDR;
#else
    script_buf = buffer;
#endif

    sprintf(env_buf, "setenv SdUpgradeImage %s", targFile);
    if (run_command(env_buf, 0))
        return CMD_RET_FAILURE;

    sprintf(script_buf, "fatload mmc %s %llX %s 0x%x 0x0", devPart, (U64)(uintptr_t)script_buf, targFile, BUF_SIZE);
    printf("\n>> %s \n", script_buf);
    if (run_command(script_buf, 0))
        return CMD_RET_FAILURE;

    memset(env_buf, 0, sizeof(env_buf));
    sprintf(env_buf, "setenv MMC_Dev %s", devPart);
    if (run_command(env_buf, 0))
        return CMD_RET_FAILURE;

#if defined(CONFIG_LMB) && defined(CONFIG_SSTAR_ESTAR_LOAD_SCRIPT_ADDR)
    memcpy((void *)buffer, (void *)script_buf, BUF_SIZE);
#endif

    script_buf = buffer;
    while ((next_line = get_script_next_line(&script_buf)) != NULL)
    {
        printf("\n>> %s \n", next_line);
        if (run_command(next_line, 0))
            return CMD_RET_FAILURE;
    }

    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(sdstar, CONFIG_SYS_MAXARGS, 1, do_sdstar, "package via SD/MMC with FAT filesystem",
           "<-i dev[:part]> <-f update_file>\n"
           "  -i    Select the target mmc device and part, default is \"0:1\".\n"
           "  -f    Select the update package file in SD card, default is \"SigmastarUpgradeSD.bin\".\n");

int do_emmcstar(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    u8 dev = 0;
    ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, BUF_SIZE);

    if ((argc != 1) && (argc != 2))
        return CMD_RET_USAGE;

    if (argc == 2)
        dev = strtoul(argv[1], NULL, 10);

    char *script_buf;
    char *next_line;

#if defined(CONFIG_LMB) && defined(CONFIG_SSTAR_ESTAR_LOAD_SCRIPT_ADDR)
    script_buf = (char *)CONFIG_SSTAR_ESTAR_LOAD_SCRIPT_ADDR;
#else
    script_buf = buffer;
#endif

    sprintf(script_buf, "mmc dev %d; emmc read.p.continue %llX %s 0x0 0x4000", dev, (U64)(uintptr_t)script_buf,
            EMMC_UPGRADE_PARTITION);
    printf("\n>> %s \n", script_buf);
    if (run_command(script_buf, 0))
        return CMD_RET_FAILURE;

#if defined(CONFIG_LMB) && defined(CONFIG_SSTAR_ESTAR_LOAD_SCRIPT_ADDR)
    memcpy((void *)buffer, (void *)script_buf, BUF_SIZE);
#endif

    script_buf = buffer;
    while ((next_line = get_script_next_line(&script_buf)) != NULL)
    {
        printf("\n>> %s \n", next_line);
        if (run_command(next_line, 0))
            return CMD_RET_FAILURE;
    }

    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(emmcstar, CONFIG_SYS_MAXARGS, 1, do_emmcstar, "script via emmc package in the \"upgrade\" partition",
           "<dev>\n"
           "  dev    Optional, select the target mmc device, default is 0.\n");

#ifdef CONFIG_SSTAR_SUPPORT_SDIO

static int do_sstarsdio(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    struct mmc *mmc;
    int         mmc_dev_index = (int)simple_strtoull(argv[2], NULL, 16);

    if (mmc_dev_index < 0)
    {
        if (get_mmc_num() > 0)
            mmc_dev_index = 0;
        else
        {
            puts("No MMC device available\n");
            return 1;
        }
    }

    mmc = find_mmc_device(mmc_dev_index);
    if (!mmc)
    {
        printf("no device at slot %x\n!\n", mmc_dev_index);
        return CMD_RET_FAILURE;
    }

    if (argc < 3)
    {
        return CMD_RET_USAGE;
    }

    if (!strcmp(argv[1], "init"))
    {
        return sdio_init(mmc);
    }
    else if (!strcmp(argv[1], "rw"))
    {
        sstar_sdio_rw_test(mmc_dev_index, mmc);
        return 0;
    }
    else
        return cmd_usage(cmdtp);
}
U_BOOT_CMD(sdiotest, CONFIG_SYS_MAXARGS, 0, do_sstarsdio, "sstar sdio test",
           "sdio_test optition <index> \n"
           "option: \n"
           "       rw   - sdio read/write test \n"
           "       init - sdio init test \n"
           "index: 0 1 3\n");
#endif
