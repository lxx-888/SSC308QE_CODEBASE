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

#if (defined(ECC_LIB_FCIE5) && ECC_LIB_FCIE5)
enum
{
    BCH_8_512 = 0,
    BCH_16_512,
    BCH_24_1024,
    BCH_32_1024,
    BCH_40_1024,
    BCH_60_1024,
    BCH_4_512,
    BCH_12_512,
    BCH_20_512,
    BCH_24_512,    
};
#else
enum
{
    BCH_4_512 = 0,
    BCH_8_512,
    BCH_12_512,
    BCH_16_512,
    BCH_20_512,
    BCH_24_512,
    BCH_24_1024,
    BCH_32_1024,
    BCH_40_1024,
    RS_4_512,
    BCH_4_512_V2,
    BCH_8_512_V2,
    BCH_12_512_V2,
    BCH_16_512_V2,
    BCH_20_512_V2,
    BCH_24_512_V2,
    BCH_24_1024_V2,
    BCH_32_1024_V2,
    BCH_40_1024_V2,
    ECC_MODE_MAX,
};
#endif

#if (defined(ECC_LIB_FCIE5) && ECC_LIB_FCIE5)
int NAND_ECC_TYPE_CIS2LIB[] = 
{
	0, // RS
	0, // BCH 1b
    BCH_8_512,
    BCH_8_512,
    BCH_12_512,
    BCH_16_512,
    BCH_20_512,
    BCH_24_512,
    BCH_24_1024,
    BCH_32_1024,
    BCH_40_1024
};
#else
int NAND_ECC_TYPE_CIS2LIB[] = 
{
	0, // RS
	0, // BCH 1b
    BCH_4_512,
    BCH_8_512,
    BCH_12_512,
    BCH_16_512,
    BCH_20_512,
    BCH_24_512,
    BCH_24_1024,
    BCH_32_1024,
    BCH_40_1024
};
#endif
int *pNAND_ECC_CODELEN;
int (*pFn_ecc_encode)(int , int , unsigned char *, unsigned char *, int , unsigned char *, char);
//int (*pFn_ecc_encode)(int, unsigned char *, unsigned char *, int, unsigned char *, char);

U32 gu32_TotalPageByteCnt;
U8  *gpu8_NandPageBuf=NULL;

// ===============================================
// libBCH_bench_fcie3.a
//   -  4b/512B ok
//   -  8b/512B NG
//   - 12b/512B U16 byte swap
//   - 12b/512B ok
// ===============================================

int init_ECC_for_spinand(void)
{
#if 1
    pNAND_ECC_CODELEN = NAND_ECC_CODELEN_v5;
    pFn_ecc_encode = encode_fcie;
#else
	int tmp;

	if(0==(tmp = GetKeyInfo(g_StrBuf, (int)MSG_LEN, gpf_ini, KEY_ECC_VER)))
	{
		printf("Err: no Key: %s\n", KEY_ECC_VER);
		return 1;
	}

	printf("\nFCIE%s\n", g_StrBuf);
	if('3'==g_StrBuf[0])
	{
		#if ECC_LIB_FCIE3
		pNAND_ECC_CODELEN = NAND_ECC_CODELEN_v3;
		pFn_ecc_encode = encode_fcie;//encode;
		#else
        printf("[%s] Err: NOT support FCIE3 ECC \n", __func__);
		return 1;
		#endif
	}
	else if('4'==g_StrBuf[0])
	{
		#if ECC_LIB_FCIE4
		pNAND_ECC_CODELEN = NAND_ECC_CODELEN_v4;
		pFn_ecc_encode = encode_fcie;//encode_fcie4;
		#else
        printf("[%s] Err: NOT support FCIE4 ECC \n", __func__);
		return 1;
		#endif
	}
	else if('5'==g_StrBuf[0])
	{
		#if ECC_LIB_FCIE5
		pNAND_ECC_CODELEN = NAND_ECC_CODELEN_v5;
		pFn_ecc_encode = encode_fcie;
		#else
        printf("[%s] Err: NOT support FCIE5 ECC \n", __func__);
		return 1;
		#endif
	}    
	else
	{
		printf("Err: unknown FCIE ver.: %s\n", g_StrBuf);
		return 1;
	}
#endif

	if (0==gu16_SectorByteCnt || 0==gu16_SectorSpareByteCnt)
	{
		spinand_init_sector_param();
	}
		

	gu32_TotalPageByteCnt = spi_nand.u16_PageByteCnt + spi_nand.u16_SpareByteCnt;
	gpu8_NandPageBuf = (U8*)malloc(gu32_TotalPageByteCnt);
	if(NULL == gpu8_NandPageBuf)
	{
		printf("Err: malloc for gpu8_NandPageBuf failed\n");
		return 1;
	}	

	return 0;	
}




