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


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;

typedef struct
{
    U8  command;
    U8  address_bytes;
    U16 address;
    U8  dummy;
    U8  reserved[3];
    /* bit[31:24] 0:str mode 1:dtr mode
     * bit[23:0] 1-1-1 1-1-4 1-4-4 .....
     * bit[23:16] = command line - 1
     * bit[15:8] = address line - 1
     * bit[7:0] = data line - 1
     */
    U32 mode;
} spinand_read_set;

typedef struct
{
    U8  command;
    U8  r_command;
    U8  address;
    U8  value;
} spinand_reg_set;
typedef struct
{
    U8 clk;
    U8 dma;
    U8 phase;
    U8 polority;
    U8 cycles_delay;
    /* pad_driving[0] CZ
     * pad_driving[1] CK
     * pad_driving[2] DI
     * pad_driving[3] DO
     * pad_driving[4] WPZ
     * pad_driving[5] HLD
     */
    U8 pad_driving[6];
    U8 reserved[21];
} spinand_ctrl_cfg;

typedef struct
{
    U8  id_bytes;
    U8  id[7];
    U16 spare_bytes;
    U16 page_bytes;
    U16 sector_bytes;
    U16 block_page_cnt;
    U16 block_cnt;
    U8  plane_cnt;
    U8  u8_BL0PBA;
    U8  u8_BL1PBA;
    U8  u8_BLPINB;
    U8  u8_BAKCNT;
    U8  u8_BAKOFS;
    U8  reserved[44];
    spinand_read_set read_set;
    spinand_reg_set  qe;
} spinand_basic_cfg;

typedef struct
{
    U8                magic[4];
    U32               crc32;
    U32               flag;
    spinand_ctrl_cfg  ctrl_cfg;
    spinand_basic_cfg basic_cfg;
} spinand_basic_info;

typedef struct {
    U8   u8_IDByteCnt;
    U8   au8_ID[15];
    U16  u16_SpareByteCnt;
    U16  u16_PageByteCnt;
    U16  u16_BlkPageCnt;
    U16  u16_BlkCnt;
    U16  u16_SectorByteCnt;
    U8   u8PlaneCnt;
    U8   u8WrapConfig;
    U8   U8RIURead;
    U8   u8_MaxClk;
    U8   u8_UBOOTPBA;
    U8   u8_BL0PBA;
    U8   u8_BL1PBA;
    U8   u8_BLPINB;
    U8   u8_BAKCNT;
    U8   u8_BAKOFS;
    U8   u8_HashPBA[3]; //NO USED
    U8   u8_BootIdLoc;
    U8   u8_Reserved[24];//just for aligning to 64bytes + magic[16] = 80bytes
} SPINAND_INFO_t;

typedef struct {
    U8  au8_magic[4];
    U32 u32_crc32;
    U8  u8_IDByteCnt;
    U8  au8_ID[15];
} SPINAND_MAGIC_t;

typedef struct {
    SPINAND_MAGIC_t spinand_magic;
    U8 au8_reserved[0x1BC];
    U8 au8_venderName[16];
    U8 au8_partnumber[16];
} SPINAND_EXT_SNI_t;

const U8 au8_i6f_basicMagicData[] = {0x4D, 0x53, 0x54, 0x41, 0x52, 0x53, 0x45, 0x4D, 0x49, 0x55, 0x53, 0x46, 0x44, 0x43, 0x49, 0x53};
const U8 au8_basicMagicData[] = {0x53, 0x4e, 0x49, 0x46};
const U8 au8_rombasicMagicData[] = {0x53, 0x4e, 0x49, 0x42};

#define SNI_SIZE 768
#define ROM_SNI_SIZE 256
#define FULL_SNI_SIZE 1024

static U32 crc32_tab[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

U32 calc_crc32(U32 crc, void *buf, U32 size)
{
    U8 *p;

    p = (U8*)buf;
    crc = crc ^ ~0U;

    while (size--)
        crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);

    return crc ^ ~0U;
}

U8 FLASH_IMPL_BIT_CHANGE(U8 data)
{
    data = (data << 4) | (data >> 4);
    data = ((data << 2) & 0xcc) | ((data >> 2) & 0x33);
    data = ((data << 2) & 0xaa) | ((data >> 2) & 0x55);

    return data;
}

U32 FLASH_IMPL_BDMA_SW_CRC32(U8 flag, U32 crc, U32 poly, U8 *buf, U32 size)
{
    U8 data;
    U32 crc_out;
    U32 crc_poly;
    U32 i, j;

    crc_out = crc;
    crc_poly = poly;
    for (j = 0;j < size; j = j+1)
    {
        data = buf[j];
        data = flag ? FLASH_IMPL_BIT_CHANGE(data) : data;
        for (i = 0;i <= 7;i = i + 1)
        {
            if (((crc_out >> 31) ^ ((data >> i) & 0x1)) & 0x1)
            {
                crc_out = ((crc_out << 1) >> 1) ^ (crc_poly >> 1);
                crc_out = (crc_out << 1) | 0x1;
            }
            else
            {
                crc_out = (crc_out << 1);
            }
        }
    }

    return crc_out;
}

int main(int argc, char **argv)
{
    U8 u8_command;
    U8 i;
    U8 u8_MaxClk = 0;
    U8 u8_BL0PBA = 0;
    U8 u8_BL1PBA = 0;
    U8 u8_BLPINB = 0;
    U8 u8_BAKCNT = 0;
    U8 u8_BAKOFS = 0;
    U8 u8PlaneCnt = 0;
    U16 u16_PageByteCnt = 0;
    U16 u16_SpareByteCnt = 0;

    int err_var;
    int opt;
    char *output_filename = NULL;
    char *input_filename = NULL;
    int fd;
    FILE *stream = NULL;
    char dst_Buf[FULL_SNI_SIZE] = {0};
    spinand_basic_info *pSpinandbasicinfo = NULL;
    spinand_ctrl_cfg   *pSpinandctrl      = NULL;
    SPINAND_INFO_t     *pSpinandinfo      = NULL;
    SPINAND_INFO_t     *pSpinandinfo_i6f  = NULL;
    SPINAND_MAGIC_t    *pSpinandmagic     = NULL;
    SPINAND_EXT_SNI_t  *pSpinandextsni    = NULL;

    while ((opt = getopt(argc, argv, "o:i:h:a:b:c:d:e:p:s:t:f:")) != -1) {
        switch (opt) {
            case 'a':
                u8_BL0PBA = atoi(optarg);
                break;
            case 'b':
                u8_BL1PBA = atoi(optarg);
                break;
            case 'c':
                u8_BLPINB = atoi(optarg);
                break;
            case 'd':
                u8_BAKCNT = atoi(optarg);
                break;
            case 'e':
                u8_BAKOFS = atoi(optarg);
                break;
            case 'p':
                u16_PageByteCnt = atoi(optarg);
                break;
            case 's':
                u16_SpareByteCnt = atoi(optarg);
                break;
            case 't':
                u8PlaneCnt = atoi(optarg);
                break;
            case 'o':
                output_filename = optarg;
                break;
            case 'i':
                input_filename = optarg;
                break;
            case 'f':
                u8_MaxClk = atoi(optarg);
                break;
            case '?':
                printf("\n -a    u8_BL0PBA \
                    \n -b    u8_BL1PBA \
                    \n -c    u8_BLPINB \
                    \n -d    u8_BAKCNT \
                    \n -e    u8_BAKOFS \
                    \n -f    u8_MaxClk \
                    \n -i    input file \
                    \n -o    output file \
                    \n -h    help \n");
                return 0;
                break;
            default:
                printf("Invalid parameter\n");
                break;
        }
    }

    if(!output_filename || !input_filename)
    {
        err_var = 0;
        goto err;
    }

    fd = open(input_filename, O_RDONLY);
    stream = fopen(output_filename, "w+");

    if(fd < 0 || !stream)
    {
        err_var = 1;
        goto err;
    }

    pSpinandextsni = (SPINAND_EXT_SNI_t *)(&dst_Buf[ROM_SNI_SIZE]);
    pSpinandmagic  = &pSpinandextsni->spinand_magic;
    pSpinandinfo   = (SPINAND_INFO_t *)(dst_Buf + ROM_SNI_SIZE + 8);

    do
    {
        if (FULL_SNI_SIZE != read(fd, dst_Buf, FULL_SNI_SIZE))
        {
            break;
        }

        if (0 != memcmp(au8_basicMagicData, (void *)&dst_Buf[ROM_SNI_SIZE], sizeof(au8_basicMagicData)))
        {
            break;
        }

        if (!memcmp(au8_rombasicMagicData, (void *)&dst_Buf[0], sizeof(au8_rombasicMagicData)))
        {
            pSpinandbasicinfo = (spinand_basic_info *)(dst_Buf);
            pSpinandctrl      = &pSpinandbasicinfo->ctrl_cfg;
            pSpinandbasicinfo->basic_cfg.u8_BL0PBA = u8_BL0PBA;
            pSpinandbasicinfo->basic_cfg.u8_BL1PBA = u8_BL1PBA;
            pSpinandbasicinfo->basic_cfg.u8_BLPINB = u8_BLPINB;
            pSpinandbasicinfo->basic_cfg.u8_BAKCNT = u8_BAKCNT;
            pSpinandbasicinfo->basic_cfg.u8_BAKOFS = u8_BAKOFS;

            if (u8PlaneCnt)
            {
                pSpinandbasicinfo->basic_cfg.plane_cnt = u8PlaneCnt;
            }

            if (u16_PageByteCnt)
            {
                pSpinandbasicinfo->basic_cfg.page_bytes = u16_PageByteCnt;
            }

            if (u16_SpareByteCnt)
            {
                pSpinandbasicinfo->basic_cfg.spare_bytes = u16_SpareByteCnt;
            }

            if((pSpinandinfo->au8_ID[0] == 0xef) && (pSpinandinfo->au8_ID[1] == 0xbc) && (pSpinandinfo->au8_ID[2] == 0x21))
            {
                pSpinandbasicinfo->flag = 0x3;
                pSpinandbasicinfo->basic_cfg.read_set.command = 0x6b;
                pSpinandbasicinfo->basic_cfg.read_set.address_bytes = 0x02;
                pSpinandbasicinfo->basic_cfg.read_set.address = 0x0;
                pSpinandbasicinfo->basic_cfg.read_set.dummy = 0x08;
                pSpinandbasicinfo->basic_cfg.read_set.mode = 0x3;
                pSpinandbasicinfo->basic_cfg.qe.command = 0x1f;
                pSpinandbasicinfo->basic_cfg.qe.r_command = 0x0f;
                pSpinandbasicinfo->basic_cfg.qe.address = 0xb0;
                pSpinandbasicinfo->basic_cfg.qe.value = 0x01;
                pSpinandctrl->clk = 0x56;
                pSpinandctrl->dma = 0x1;
                pSpinandctrl->phase = 0;
                pSpinandctrl->polority = 0;
                pSpinandctrl->cycles_delay = 0;
                pSpinandctrl->pad_driving[0] = 1;
                pSpinandctrl->pad_driving[1] = 2;
                pSpinandctrl->pad_driving[2] = 1;
                pSpinandctrl->pad_driving[3] = 2;
                pSpinandctrl->pad_driving[4] = 1;
                pSpinandctrl->pad_driving[5] = 2;
            }
        }

        if (!memcmp(au8_i6f_basicMagicData, (void *)&dst_Buf[0], sizeof(au8_i6f_basicMagicData)))
        {
            pSpinandinfo_i6f = (SPINAND_INFO_t *)(dst_Buf + 16);
            pSpinandinfo_i6f->u8_BL0PBA = u8_BL0PBA;
            pSpinandinfo_i6f->u8_BL1PBA = u8_BL1PBA;
            pSpinandinfo_i6f->u8_BLPINB = u8_BLPINB;
            pSpinandinfo_i6f->u8_BAKCNT = u8_BAKCNT;
            pSpinandinfo_i6f->u8_BAKOFS = u8_BAKOFS;

            if (u8_MaxClk)
            {
                pSpinandinfo_i6f->u8_MaxClk = u8_MaxClk;
            }

            if (u8PlaneCnt)
            {
                pSpinandinfo_i6f->u8PlaneCnt = u8PlaneCnt;
            }

            if (u16_PageByteCnt)
            {
                pSpinandinfo_i6f->u16_PageByteCnt = u16_PageByteCnt;
            }

            if (u16_SpareByteCnt)
            {
                pSpinandinfo_i6f->u16_SpareByteCnt = u16_SpareByteCnt;
            }
        }

        pSpinandinfo->u8_BL0PBA = u8_BL0PBA;
        pSpinandinfo->u8_BL1PBA = u8_BL1PBA;
        pSpinandinfo->u8_BLPINB = u8_BLPINB;
        pSpinandinfo->u8_BAKCNT = u8_BAKCNT;
        pSpinandinfo->u8_BAKOFS = u8_BAKOFS;

        if (u8_MaxClk)
        {
            pSpinandinfo->u8_MaxClk = u8_MaxClk;
        }

        if (u8PlaneCnt)
        {
            pSpinandinfo->u8PlaneCnt = u8PlaneCnt;
        }

        if (u16_PageByteCnt)
        {
            pSpinandinfo->u16_PageByteCnt = u16_PageByteCnt;
        }

        if (u16_SpareByteCnt)
        {
            pSpinandinfo->u16_SpareByteCnt = u16_SpareByteCnt;
        }

        pSpinandmagic->u32_crc32 = calc_crc32(0, (void *)&dst_Buf[ROM_SNI_SIZE + 8], (U32)(0x300 - 0x8));

        if (!memcmp(au8_rombasicMagicData, (void *)&dst_Buf[0], sizeof(au8_rombasicMagicData)))
        {
            pSpinandbasicinfo->crc32 = FLASH_IMPL_BDMA_SW_CRC32(0, -1, 0xedb88320, (void *)&dst_Buf[8], 0x80 - 0x8);
        }

        fwrite(dst_Buf,1,FULL_SNI_SIZE,stream);
    }while(1);

    close(fd);
    fclose(stream);
    printf(">>>%s\n",output_filename);

    return 0;
err:
    printf("error number %d\n",err_var);
    return -1;
}
