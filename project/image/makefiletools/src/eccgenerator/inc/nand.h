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

#define U32  unsigned int
#define U16  unsigned short
#define U8   unsigned char
#define S32  signed int
#define S16  signed short
#define S8   signed char

//===========================================================
// NAND limitations
//===========================================================
#define NAND_MAX_BLKCNT        (16*1024) // max blk cnt, comes from BBT 2KB x 8bits = 16K blocks.
#define NAND_MAX_BLKPAGECNT    ( 1*1024) // page per blk, comes from PPM 2KB / 4 * 2 = 1K pages.
#define NAND_MAX_PAGEBYTECNT   (16*1024) // page byte cnt, comes from 16KB drvNandPageBuf

//------------------------------------------------------------------

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <BCH_bench_fcie3.h>
#include <BCH_bench_fcie4.h>
#include <BCH_bench.h>

#define NAND_BBT_PAGE_COUNT 8
#define NAND_PARTITAION_BYTE_CNT 0x200
#include <CIS.h>

#include <user_setting.h>
#define MSG_LEN  0x400
extern char g_StrBuf[MSG_LEN];
extern FILE * gpf_ini;
extern U16 gu16_SectorByteCnt;
extern U16 gu16_SectorSpareByteCnt;

extern int  set_output_filename(char *pImgFilename, char *pOutFilename, int count, U16 u16_PartIdx);
extern int  get_image_filename(char *pStr, char *pFilename, int count);
extern int  NAND_Write_PhyImages(void);
extern int  NAND_Write_UnfdImage(void);
extern int	NAND_Write_UBIImage(void);
extern int	NAND_WriteCISPage(void);
extern int NAND_WriteHashPages(void);
extern U8   drvNAND_CheckAll0xFF(U8* pu8_Buf, U32 u32_ByteCnt);

extern void Test_Main(void);

extern int  init_ECC(void);

