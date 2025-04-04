/*
 * part_emmc.c- Sigmastar
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
#include <ide.h>
#include "part_emmc.h"
#include <memalign.h>
#include <mmc.h>
#include <linux/math64.h>

/*
#if defined(CONFIG_CMD_IDE) || \
defined(CONFIG_CMD_MG_DISK) || \
defined(CONFIG_CMD_SCSI) || \
defined(CONFIG_CMD_SATA) || \
defined(CONFIG_CMD_USB) || \
defined(CONFIG_CMD_eMMC) || \
defined(CONFIG_SYSTEMACE)
 */
#if 1

/* stdlib.h causes some compatibility problems; should fixe these! -- wd */
#ifndef __ldiv_t_defined
typedef struct
{
    long int quot; /* Quotient */
    long int rem;  /* Remainder    */
} ldiv_t;
extern ldiv_t ldiv(long int __numer, long int __denom);
#define __ldiv_t_defined 1
#endif

static int part_emmc_read_ddb(struct blk_desc *dev_desc, emmc_driver_desc_t *ddb_p);
static int part_emmc_read_pdb(struct blk_desc *dev_desc, int part, emmc_partition_t *pdb_p);

static int part_emmc_write_ddb(struct blk_desc *dev_desc, emmc_driver_desc_t *ddb_p);
static int part_emmc_write_pdb(struct blk_desc *dev_desc, int part, emmc_partition_t *pdb_p);

// extern int get_NVRAM_max_part_count(void);
// extern U32 eMMC_SearchDevNodeStartSector(void);

/*
 * read Driver Descriptor Block
 */

int get_NVRAM_max_part_count(void)
{
    return EMMC_RESERVED_FOR_MAP;
}
static int part_emmc_read_ddb(struct blk_desc *dev_desc, emmc_driver_desc_t *ddb_p)
{
#if 0
    u32 u32_err;

#if defined(eMMC_FCIE_LINUX_DRIVER) && eMMC_FCIE_LINUX_DRIVER
    if(0 == g_eMMCDrv.u32_PartDevNodeStartSector){
        u32_err = eMMC_SearchDevNodeStartSector();
        if(eMMC_ST_SUCCESS != u32_err){
            printf ("** Err, %s **\n", __func__);
            return (-1);
        }
    }
#endif
    printf("read_ddb from: 0x%Xh\n", (unsigned int)g_eMMCDrv.u32_PartDevNodeStartSector);

    u32_err = eMMC_ReadData((uchar*)ddb_p, 512, g_eMMCDrv.u32_PartDevNodeStartSector);
    if(eMMC_ST_SUCCESS != u32_err){
        printf ("** Can't read Driver Desriptor Block **\n");
        return (-1);
    }

    if (ddb_p->signature != EMMC_DRIVER_MAGIC) {
        printf ("** Bad Signature: expected 0x%04x, got 0x%04x\n",
                EMMC_DRIVER_MAGIC, ddb_p->signature);
        return (-1);
    }

    return (0);
#endif

    if (ddb_p->signature == EMMC_DRIVER_MAGIC)
        return (0);

    if (blk_dread(dev_desc, 0, 1, (uchar *)ddb_p) != 1)
    {
        printf("** Can't read Driver Desriptor Block **\n");
        return (-1);
    }

    if (ddb_p->signature != EMMC_DRIVER_MAGIC)
    {
        //        printf ("** Bad Signature: expected 0x%04x, got 0x%04x\n",
        //                    EMMC_DRIVER_MAGIC, ddb_p->signature);

        return (-1);
    }

    return (0);
}

/*
 * read Partition Descriptor Block
 */
static int part_emmc_read_pdb(struct blk_desc *dev_desc, int part_index, emmc_partition_t *pdb_p)
{
#if 0
    u32 u32_err;
    int part=part_index+=1;

    u32_err = eMMC_ReadData((uchar*)pdb_p, 512,
            g_eMMCDrv.u32_PartDevNodeStartSector + part);

    if(eMMC_ST_SUCCESS != u32_err){
        printf ("** Can't read Driver Desriptor Block **\n");
        return (-1);
    }

    //  if (pdb_p->signature != EMMC_PARTITION_MAGIC) {
    //        //printf ("** Bad Signature: "
    //        //        "expected 0x%04x, got 0x%04x\n",
    //        //        EMMC_PARTITION_MAGIC, pdb_p->signature);
    //        return (-1);
    //    }

    return 0;
#endif

    int part = part_index; // block of ppd start from 1

    /*
     * We must always read the descritpor block for
     * partition 1 first since this is the only way to
     * know how many partitions we have.
     */
    if ((part < 1) || (part > get_NVRAM_max_part_count()))
    {
        printf("** Invalid partition %d:%d [%d:1...%d:%d only]\n", dev_desc->devnum, part, dev_desc->devnum,
               dev_desc->devnum, pdb_p->map_count);
        return (-1);
    }

    if (blk_dread(dev_desc, part, 1, (uchar *)pdb_p) != 1)
    {
        printf("** Can't Read Partition Desiptor Block **\n");
        return (-1);
    }

    if (pdb_p->signature != EMMC_PARTITION_MAGIC)
    {
        // printf("** Bad Signature on %d:%d: expected 0x%04x, got 0x%04x\n", dev_desc->devnum, part_index,
        //        EMMC_PARTITION_MAGIC, pdb_p->signature);
        return (-1);
    }
    return (0);
}

static int part_emmc_write_ddb(struct blk_desc *dev_desc, emmc_driver_desc_t *ddb_p)
{
#if 0
    u32 u32_err;

    if(ddb_p->signature != EMMC_DRIVER_MAGIC){
        printf ("** Bad Signature: expected 0x%04x, try to write 0x%04x **\n",
                EMMC_DRIVER_MAGIC, ddb_p->signature);
        return (-1);
    }

    printf("write_ddb into: 0x%Xh\n",(unsigned int) g_eMMCDrv.u32_PartDevNodeStartSector);
    u32_err = eMMC_WriteData((uchar*)ddb_p, 512, g_eMMCDrv.u32_PartDevNodeStartSector);
    if(eMMC_ST_SUCCESS != u32_err){
        printf ("** Can't read Driver Desriptor Block **\n");
        return (-1);
    }

    return (0);
#endif

    if (ddb_p->signature != EMMC_DRIVER_MAGIC)
    {
        printf("** Bad Signature: expected 0x%04x, try to write 0x%04x **\n", EMMC_DRIVER_MAGIC, ddb_p->signature);
        return (-1);
    }

    if (blk_dwrite(dev_desc, 0, 1, (uchar *)ddb_p) != 1)
    {
        printf("** Can't write Driver Desiptor Block **\n");
        return (-1);
    }

    return (0);
}

static int part_emmc_write_pdb(struct blk_desc *dev_desc, int part_index, emmc_partition_t *pdb_p)
{
#if 0
    u32 u32_err;
    int part=part_index+=1;

    printf("part_emmc_write_pdb : 0x%X\r\n",(unsigned int) g_eMMCDrv.u32_PartDevNodeStartSector + part);

    u32_err = eMMC_WriteData((uchar*)pdb_p, 512,
            g_eMMCDrv.u32_PartDevNodeStartSector + part);

    if(eMMC_ST_SUCCESS != u32_err){
        printf ("** Can't read Driver Desriptor Block **\n");
        return (-1);
    }

    return 0;
#endif

    int part = part_index;

    if ((part < 1) || (part > get_NVRAM_max_part_count()))
    {
        printf("** Invalid partition %d:%d [%d:1...%d:%d only]\n", dev_desc->devnum, part, dev_desc->devnum,
               dev_desc->devnum, pdb_p->map_count);
        return (-1);
    }

    if (blk_dwrite(dev_desc, part, 1, (uchar *)pdb_p) != 1)
    {
        printf("** Can't write Driver Desiptor Block **\n");
        return (-1);
    }

    return (0);
}

static int add_NVRAM_pdb(struct blk_desc *dev_desc, emmc_partition_t *pdb_p)
{
#if 0
    int n = 0;
    int part_idx=-1;
    emmc_partition_t exist_pdb;
    BOOLEAN vacancy_found=FALSE;
    //  ulong special_start;
    ulong start_block, nvram_start_sector = 0;//EMMC_PARTITION_START;

    if(0!=get_NVRAM_start_sector(&start_block))
    {
        printf("get EMMC NVRAM partition start_sector failed...\n");
        return -1;
    }

    nvram_start_sector=start_block;

    if(pdb_p->signature != EMMC_PARTITION_MAGIC){
        printf ("** Bad Signature: expected 0x%04x, try to write 0x%04x **\n",
                EMMC_PARTITION_MAGIC, pdb_p->signature);
        return (-1);
    }

    //  special_start = pdb_p->start_block;
    vacancy_found=FALSE;
    part_idx=-1;
    //let us loop to found the correct start_block...
    for (n=0;n<get_NVRAM_max_part_count();n++) {

        if (0!=part_emmc_read_pdb(dev_desc, n, &exist_pdb)) {
            printf ("** Can't read Partition Map on %d:%d **\n",
                    dev_desc->devnum, n);
            return (-1);
        }

        if(exist_pdb.signature == EMMC_PARTITION_MAGIC){
            if(!strcmp((const char *)pdb_p->name, (const char *)exist_pdb.name)){
                printf("** Partition %s existed...**\n", exist_pdb.name);
                return -1;
            }
            //                              if(pdb_p->block_count > exist_pdb.block_count){
            //                                  printf("** The new size of the partition is too big!\n");
            //                                  return (-1);
            //                              }
            //
            //                              if(special_start != 0)
            //                                  pdb_p->start_block = special_start;
            //                              else
            //                                  pdb_p->start_block = exist_pdb.start_block;
            //
            //                              if(pdb_p->start_block + pdb_p->block_count > dev_desc->lba){
            //                                  printf("** Partition exceed emmc capacity**\n");
            //                                  return (-1);
            //                              }
            //
            //                              if(dev_desc->block_write(dev_desc->devnum, n, 1, (ulong *)pdb_p) != 1){
            //                                  printf("** Can't write Driver Desiptor Block **\n");
            //                                  return (-1);
            //                              }
            //                              return (0);
            //                          }
            //
        if(exist_pdb.start_block < start_block)
        {
            if( exist_pdb.start_block < nvram_start_sector)
            {
                printf("** Invalid start_block 0x%X of partition %s **\n",exist_pdb.start_block, exist_pdb.name);
                return -1;
            }

            continue; //check next partition
        }

        if((exist_pdb.start_block-start_block) >= pdb_p->block_count)
        {
            // vacancy found!!
            vacancy_found=TRUE;
        }
        else if(!vacancy_found)
        {
            start_block=exist_pdb.start_block+exist_pdb.block_count;
        }

    }
    else if(-1==part_idx)
    {
        part_idx=n;
        if(vacancy_found)
        {
            break;
        }
    }

}


if(-1 == part_idx){
    printf("** No vacancy record entry in NVRAM partition map...**\n");
    return (-1);
}

pdb_p->start_block = start_block;

if(pdb_p->start_block + pdb_p->block_count > dev_desc->lba)
{
    printf("** Partition exceed EMMC capacity**\n");
    return (-1);
}


if(0!=part_emmc_write_pdb(dev_desc,part_idx,pdb_p) ){
    printf("** Can't write Partition Map Block **\n");
    return (-1);
}
return (0);
#endif

    int              n = 1;
    emmc_partition_t exist_pdb;
    ulong            special_start, start_block = EMMC_PARTITION_START;
    struct mmc *     emmc = find_mmc_device(dev_desc->devnum);
    int              start_rem, ret;

    if (pdb_p->signature != EMMC_PARTITION_MAGIC)
    {
        printf("** Bad Signature: expected 0x%04x, try to write 0x%04x **\n", EMMC_PARTITION_MAGIC, pdb_p->signature);
        return (-1);
    }

    special_start = pdb_p->start_block;

    for (;;)
    {
        /*
         * We must always read the descritpor block for
         * partition 1 first since this is the only way to
         * know how many partitions we have.
         */
        if (blk_dread(dev_desc, n, 1, &exist_pdb) != 1)
        {
            printf("** Can't read Partition Map on %d:%d **\n", dev_desc->devnum, n);
            return (-1);
        }

        if (exist_pdb.signature == EMMC_PARTITION_MAGIC)
        {
            if (!strcmp((const char *)pdb_p->name, (const char *)exist_pdb.name))
            {
                printf("** Try to change a exist partition %s **\n", exist_pdb.name);
                return (-1);
                // if(pdb_p->block_count > exist_pdb.block_count){
                //     printf("** The new size of the partition is too big!\n");
                //     return (-1);
                // }
                //
                // if(special_start != 0)
                //     pdb_p->start_block = special_start;
                // else
                //     pdb_p->start_block = exist_pdb.start_block;
                //
                // if(pdb_p->start_block + pdb_p->block_count > dev_desc->lba){
                //     printf("** Partition exceed emmc capacity**\n");
                //     return (-1);
                // }
                //
                // if(dev_desc->block_write(dev_desc->devnum, n, 1, (ulong *)pdb_p) != 1){
                //     printf("** Can't write Driver Desiptor Block **\n");
                //     return (-1);
                // }
                // memcpy((unsigned char *)(&g_mpart[n-1]), (unsigned char *)(pdb_p), sizeof(emmc_partition2_t));
                // return (0);
            }

            if (exist_pdb.start_block + exist_pdb.block_count > EMMC_PARTITION_START)
            {
                /* skip table partition and env partition*/
                start_block = exist_pdb.start_block + exist_pdb.block_count;
            }
            ++n;
            if (n >= get_NVRAM_max_part_count())
            {
                printf("** Exceed the max number of partition, STOP!**\n");
                return (-1);
            }
        }
        else
        {
            ret = div_u64_rem(start_block, emmc->erase_grp_size, &start_rem);
            if (start_rem)
                start_block = (ret + 1) * emmc->erase_grp_size;

            if (special_start != 0)
                pdb_p->start_block = special_start;
            else
                pdb_p->start_block = start_block;

            if (pdb_p->start_block + pdb_p->block_count > dev_desc->lba)
            {
                printf("** Partition exceed emmc capacity**\n");
                return (-1);
            }

            if (blk_dwrite(dev_desc, n, 1, (uchar *)pdb_p) != 1)
            {
                printf("** Can't write Driver Desiptor Block **\n");
                return (-1);
            }
            return (0);
        }
    }

    return (0);
}

static emmc_partition_t mpart;

static int test_part_emmc(struct blk_desc *dev_desc)
{
    ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev_desc->blksz);
    emmc_driver_desc_t *ddesc = (emmc_driver_desc_t *)buffer;

    memset(ddesc, 0, dev_desc->blksz);
    if (part_emmc_read_ddb(dev_desc, ddesc))
    {
        /* error reading Driver Desriptor Block, or no valid Signature */
        return (-1);
    }

    return (0);
}

static void print_part_emmc(struct blk_desc *dev_desc)
{
    ulong u32_i;
    ulong bytes;
    char  c;

    printf("%20.32s %-18.32s Number    BlkCnt @ Start      Capacity\n", "Type", "Name");
    for (u32_i = 1; u32_i < get_NVRAM_max_part_count(); u32_i++)
    {
        if (part_emmc_read_pdb(dev_desc, u32_i, &mpart))
            return;

        if (mpart.signature == EMMC_PARTITION_MAGIC)
        {
            c     = 'k';
            bytes = mpart.block_count;
            bytes /= (1024 / 512); /* kB; assumes blk_size == 512 */
            if (bytes >= 1024)
            {
                bytes >>= 10;
                c = 'M';
            }
            if (bytes >= 1024)
            {
                bytes >>= 10;
                c = 'G';
            }
            printf("%20.32s %-18.32s %4u  %10u @ %-10u (%3ld%c)\n", mpart.type, mpart.name, (unsigned int)(u32_i),
                   (unsigned int)(mpart.block_count), mpart.start_block, bytes, c);
        }
    }
}

static int get_partition_info_emmc(struct blk_desc *dev_desc, int part_index, struct disk_partition *info)
{
    if (part_index < 0)
    {
        printf("err, part number: %d\n", part_index);
        return -1;
    }

    if (part_emmc_read_pdb(dev_desc, part_index, &mpart))
        return (-1);

    if (mpart.signature != EMMC_PARTITION_MAGIC)
        return 1;

#if CONFIG_IS_ENABLED(PARTITION_UUIDS)
    sprintf(info->uuid, "%08x-%04x-%04x-%04x-%012x", EMMC_DRIVER_MAGIC, EMMC_DRIVER_MAGIC, EMMC_PARTITION_MAGIC,
            EMMC_PARTITION_MAGIC, part_index);
#endif
    info->blksz = 0x200;
    info->start = mpart.start_block;
    info->size  = mpart.block_count;
    memcpy(info->type, mpart.type, sizeof(info->type));
    memcpy(info->name, mpart.name, sizeof(info->name));

    return (0);
}

#if 0
extern char *gpas8_eMMCPartName[];
int add_emmc_partitions(struct blk_desc *dev_desc, struct disk_partition *info)
{
    int ret;
    eMMC_PNI_t *pPartInfo = (eMMC_PNI_t*)gau8_eMMC_PartInfoBuf;
    u16 u16_i, u16_j, u16_pdb_cnt=0;
    u8  u8_i;

    //printf("%X %X\n", sizeof(emmc_driver_desc_t), sizeof(emmc_partition_t));
    printf("add_emmc_partitions\r\n");

    if(part_emmc_read_ddb(dev_desc, &ddesc)){
        printf("Need write new driver description table!\n");



        ddesc.signature = EMMC_DRIVER_MAGIC;
        ddesc.blk_size = 0x200;
        ddesc.blk_count = g_eMMCDrv.u32_SEC_COUNT;
        ret = part_emmc_write_ddb(dev_desc, &ddesc);
        if(ret){
            printf("Error during write new driver description table!\n");
            return ret;
        }
    }


    u8_i = 0;
    for(u16_i=0; u16_i<pPartInfo->u16_PartCnt; u16_i++)
    {
        for(u16_j=0; u16_j<EMMC_RESERVED_FOR_MAP; u16_j++)
        {
            if((u16_j|eMMC_LOGI_PART) == pPartInfo->records[u16_i].u16_PartType
              )//||(u16_i|eMMC_LOGI_PART|eMMC_HIDDEN_PART) == pPartInfo->records[u16_j].u16_PartType)
              {
                  mpart.signature = EMMC_PARTITION_MAGIC;
                  sprintf((char *)mpart.name, "%s",
                          gpas8_eMMCPartName[u8_i]);//, pPartInfo->records[u16_j].u16_PartType);
                  u8_i++;
                  mpart.start_block =
                      pPartInfo->records[u16_i].u16_StartBlk * pPartInfo->u16_BlkPageCnt;
                  mpart.block_count =
                      pPartInfo->records[u16_i].u16_BlkCnt * pPartInfo->u16_BlkPageCnt;

                  u16_pdb_cnt++;
                  ret = part_emmc_write_pdb(dev_desc, u16_pdb_cnt, &mpart);
                  if(ret)
                  {
                      printf("Error during write new partition description table! %Xh\n",
                              u16_pdb_cnt);
                      return ret;
                  }

                  break;
              }
        }
    }
    return 0;
}


int remove_emmc_partitions(struct blk_desc *dev_desc, struct disk_partition *info)
{
    int ret;

    eMMC_PNI_t *pPartInfo = (eMMC_PNI_t*)gau8_eMMC_PartInfoBuf;
    u16 u16_i, u16_pdb_cnt=0;

    if (part_emmc_read_ddb(dev_desc, &ddesc)) {
        printf("Haven't driver description table!\n");
        return (-1);
    }

    for (u16_i=0; u16_i<pPartInfo->u16_PartCnt; u16_i++) {
        if (pPartInfo->records[u16_i].u16_PartType & eMMC_LOGI_PART) {
            mpart.signature = ~EMMC_PARTITION_MAGIC;

            u16_pdb_cnt++;
            ret = part_emmc_write_pdb(dev_desc, u16_pdb_cnt, &mpart);
            if (ret) {
                printf("Error during write new partition description table! %Xh\n",
                        u16_pdb_cnt);
                return ret;
            }
        }
    }

    return 0;
}
#endif

int init_NVRAM_ddb(struct blk_desc *dev_desc)
{
    ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev_desc->blksz);
    emmc_driver_desc_t *ddesc = (emmc_driver_desc_t *)buffer;
    int                 ret;

    ddesc->signature = EMMC_DRIVER_MAGIC;
    ddesc->blk_size  = 0x200;
    ddesc->blk_count = dev_desc->lba;
    ret              = part_emmc_write_ddb(dev_desc, ddesc);
    if (ret)
    {
        printf("Error during write new driver description table!\n");
        return ret;
    }

    return 0;
}

int create_new_NVRAM_partition(struct blk_desc *dev_desc, struct disk_partition *info)
{
#if 0
    int ret;
    //eMMC_PNI_t *pPartInfo = (eMMC_PNI_t*)gau8_eMMC_PartInfoBuf;
    emmc_partition_t pdb_base;

    //printf("%X %X\n", sizeof(emmc_driver_desc_t), sizeof(emmc_partition_t));
    //printf("add_emmc_partitions\r\n");

    if(part_emmc_read_ddb(dev_desc, &ddesc)){
        printf("Need write correct CIS first!!\n");

        return -1;
    }

    pdb_base.signature = EMMC_PARTITION_MAGIC;

    strcpy((char *)pdb_base.name, (const char *)info->name);
    pdb_base.block_count = info->size;
    pdb_base.start_block = info->start;

    ret = add_NVRAM_pdb(dev_desc, &pdb_base);
    if(ret){
        printf("Error during create new partition record!\n");
        return ret;
    }

    return 0;
#endif
    ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev_desc->blksz);
    emmc_driver_desc_t *ddesc = (emmc_driver_desc_t *)buffer;

    int              ret;
    int              flag = 0;
    emmc_partition_t pdb_base;
    struct mmc *     emmc = find_mmc_device(dev_desc->devnum);
    int              size_rem;

    memset(&pdb_base, 0, sizeof(emmc_partition_t));

    flag = part_emmc_read_ddb(dev_desc, ddesc);
    if (flag)
    {
        printf("Need write new driver description table!\n");
        ret = init_NVRAM_ddb(dev_desc);
        if (ret)
        {
            printf("Error during write new driver description table!\n");
            return ret;
        }
    }

    pdb_base.signature = EMMC_PARTITION_MAGIC;
    strcpy((char *)pdb_base.name, (const char *)info->name);
    ret = div_u64_rem(info->size, emmc->erase_grp_size, &size_rem);
    if (size_rem)
        info->size = (ret + 1) * emmc->erase_grp_size;

    pdb_base.block_count = info->size;
    pdb_base.start_block = info->start;
    strcpy((char *)pdb_base.type, BOOT_PART_TYPE);

    ret = add_NVRAM_pdb(dev_desc, &pdb_base);
    if (ret)
    {
        printf("Error during create new partition record!\n");
        return ret;
    }

    return 0;
}

int delete_NVRAM_all_partition(struct blk_desc *dev_desc)
{
    ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev_desc->blksz);
    emmc_driver_desc_t *ddesc = (emmc_driver_desc_t *)buffer;

    int n = 0;
    //  emmc_driver_desc_t ddb;
    emmc_partition_t exist_pdb, pdb;

    memset(&pdb, 0, sizeof(emmc_partition_t));

    if (part_emmc_read_ddb(dev_desc, ddesc))
    {
        printf("Need write correct CIS first!!\n");
        return -1;
    }

    for (n = 1; n < get_NVRAM_max_part_count(); n++)
    {
        if (0 != part_emmc_read_pdb(dev_desc, n, &exist_pdb))
        {
            printf("** Can't read Partition Map on %d:%d for partition remove **\n", dev_desc->devnum, n);
            continue;
        }
        if ((exist_pdb.signature == EMMC_PARTITION_MAGIC) && strcmp((char *)exist_pdb.name, EMMC_UPGRADE_PARTITION))
        {
            if (0 != part_emmc_write_pdb(dev_desc, n, &pdb))
            {
                printf("** failed to remove the partition %s **\n", exist_pdb.name);
                continue;
            }
        }
    }

    return 0;
}

int remove_NVRAM_partition(struct blk_desc *dev_desc, struct disk_partition *info)
{
    ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev_desc->blksz);
    emmc_driver_desc_t *ddesc = (emmc_driver_desc_t *)buffer;

    int n = 0;
    //        emmc_driver_desc_t ddb;
    emmc_partition_t exist_pdb, pdb;

    memset(&pdb, 0, sizeof(emmc_partition_t));

    if (part_emmc_read_ddb(dev_desc, ddesc))
    {
        printf("Need write correct CIS first!!\n");
        return -1;
    }

    for (n = 1; n < get_NVRAM_max_part_count(); n++)
    {
        if (0 != part_emmc_read_pdb(dev_desc, n, &exist_pdb))
        {
            printf("** Can't read Partition Map on %d:%d for partition remove **\n", dev_desc->devnum, n);
            return (-1);
        }
        if (exist_pdb.signature == EMMC_PARTITION_MAGIC)
        {
            if (!strcmp((const char *)info->name, (const char *)exist_pdb.name))
            {
                if (0 != part_emmc_write_pdb(dev_desc, n, &pdb))
                {
                    printf("** failed to remove the partition %s **\n", exist_pdb.name);
                    return (-1);
                }
                return (0);
            }
        }
    }

    if (n == get_NVRAM_max_part_count())
    {
        printf("partition %s not existed...**\n", info->name);
        return (-1);
    }

    return (0);
}

// blk_desc
U_BOOT_PART_TYPE(EMMC) = {
    .name        = "EMMC",
    .part_type   = PART_TYPE_EMMC,
    .max_entries = EMMC_RESERVED_FOR_MAP,
    .get_info    = part_get_info_ptr(get_partition_info_emmc),
    .print       = part_print_ptr(print_part_emmc),
    .test        = test_part_emmc,
};

#endif
