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
#include <nand.h>

SPINAND_INFO_t nand;
SPINAND_INFO_t	spi_nand;
PARTITION_INFO_t partinfo;
extern char *out_filename;
extern char *sni_filename;


PAIRED_PAGE_MAP_t ga_tPairedPageMap[512] = {{0,0}};
#if 0
int NAND_ECC_TYPE_MAPING[] = 
{
	0, 0, 
	BCH_4_512_V2, 
	BCH_8_512_V2, 
	BCH_12_512_V2, 
	BCH_16_512_V2, 
	BCH_20_512_V2, 
	BCH_24_512_V2, 
	BCH_24_1024_V2, 
	BCH_32_1024_V2, 
	BCH_40_1024_V2
};
#endif
int NAND_ECC_CODELEN_v3[] = 
{
	0, 0, 7, 13, 20, 26, 33, 39, 42, 56, 70
};
int NAND_ECC_CODELEN_v4[] = //v.4
{
	0, 0, 8, 14, 20, 26, 34, 40, 42, 56, 70
};
int NAND_ECC_CODELEN_v5[] = //v.5
{
	0, 0, 14, 14, 20, 28, 34, 40, 42, 56, 70
};

U16 gu16_SectorByteCnt = 0;
U16 gu16_SectorSpareByteCnt = 0;

void spinand_init_sector_param(void)
{
	gu16_SectorByteCnt = spi_nand.u16_SectorByteCnt;
	gu16_SectorSpareByteCnt = (gu16_SectorByteCnt * spi_nand.u16_SpareByteCnt) / spi_nand.u16_PageByteCnt;
}

/* return 0: same, 1: different */
U32 drvNAND_CompareCISTag(U8 *tag)
{
	const char *str = "MSTARSEMIUSFDCIS";
	int i = 0;
	
	for (i = 0; i < 16; i++) {
		if (tag[i] != str[i])
			return 1;
	}
    printf("Get CIS header\n");
	return 0;
}

void dump_spi_nand_info(SPINAND_INFO_t *pNandInfo)
{
    int i;
	printf("###############################################\n");
	printf( "#        SPINAND INFO                        #\n");
	printf( "###############################################\n");

	printf( "pNandInfo: 0x%08lx\n", (U32)pNandInfo);
	printf( "au8_Tag          : [");
	for (i = 0; i < 16; i++)
		printf( "%c", pNandInfo->au8_Tag[i]);
	printf( "]\n");

	printf( "u8_IDByteCnt     : 0x%04x\n", pNandInfo->u8_IDByteCnt);

	printf( "au8_ID           : 0x[ ");
	for (i = 0; i < pNandInfo->u8_IDByteCnt; i++)
		printf( "%02X ", pNandInfo->au8_ID[i]);
	printf( "]\n");

    printf( "u16_SpareByteCnt   : 0x%04x\n", pNandInfo->u16_SpareByteCnt);
    printf( "u16_SectorByteCnt  : 0x%04x\n", pNandInfo->u16_SectorByteCnt);
	printf( "u16_PageByteCnt    : 0x%04x\n", pNandInfo->u16_PageByteCnt);
	printf( "u16_BlkPageCnt     : 0x%04x\n", pNandInfo->u16_BlkPageCnt);
	printf( "u16_BlkCnt         : 0x%04x\n", pNandInfo->u16_BlkCnt);
}

int SPINAND_SearchSNI(void)
{
	int sni_size;
	int u32_index;
	
	char sni_list[512];
	char filename[256];
	
	FILE *sni_out = NULL;
	
	SPINAND_INFO_t *pNandInfo;

#if 0
	if (0 == GetKeyInfo(g_StrBuf, (int)MSG_LEN, gpf_ini, KEY_SNI))
	{
		printf("Err: no Key: %s\n", KEY_SNI);
		return 1;
	}
#endif

	if (NULL == (sni_out = fopen(sni_filename, "rb")))
	{
		printf("[%s] Err: open file %s fails\n", __func__, filename);
		return 1;
	}

    printf("[%s] open file %s ok\n", __func__, sni_filename);

	while (512 == fread(sni_list, 1, 512, sni_out))
	{
		pNandInfo = (SPINAND_INFO_t *) sni_list;

		if (0 == drvNAND_CompareCISTag(pNandInfo->au8_Tag))
		{			
			memcpy(&spi_nand, pNandInfo, sizeof(SPINAND_INFO_t));
			spinand_init_sector_param();
            printf("[%s] get correct sni\n", __func__);
            fclose(sni_out);
			return 0;	
		}
		
		fseek(sni_out, 512, SEEK_CUR);
	}
    fclose(sni_out);
	return 1;
}

