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
#define CELL_TYPE_SLC      0
#define CELL_TYPE_MLC      1

#define ECC_TYPE_RS        1
#define ECC_TYPE_4BIT      2
#define ECC_TYPE_8BIT      3
#define ECC_TYPE_12BIT     4
#define ECC_TYPE_16BIT     5
#define ECC_TYPE_20BIT     6
#define ECC_TYPE_24BIT     7
#define ECC_TYPE_24BIT1KB  8
#define ECC_TYPE_32BIT1KB  9
#define ECC_TYPE_40BIT1KB  10

typedef struct _PAIRED_PAGE_MAP {

    U16 u16_LSB;
    U16 u16_MSB;

} PAIRED_PAGE_MAP_t, *P_PAIRED_PAGE_MAP_t;


typedef struct
{
	U16 u16_StartBlk;		// the start block index
	U16 u16_BlkCnt;			// project team defined
	U16 u16_PartType;		// project team defined
	U16 u16_BackupBlkCnt;	// reserved good blocks count for backup, UNFD internal use.
							// e.g. u16BackupBlkCnt  = u16BlkCnt * 0.03 + 2
} PARTITION_RECORD_t;

typedef struct
{
	U32 u32_ChkSum;
	U16	u16_SpareByteCnt;
	U16	u16_PageByteCnt;
	U16	u16_BlkPageCnt;
	U16	u16_BlkCnt;
	U16 u16_PartCnt;
	U16 u16_UnitByteCnt;
	PARTITION_RECORD_t records[62];

}PARTITION_INFO_t;

typedef struct _DDR_TIMING_GROUP {

	U8	u8_ClkIdx;
	U8	u8_DqsMode;
	U8	u8_DelayCell;
	U8	u8_DdrTiming;

} __attribute__((__packed__)) DDR_TIMING_GROUP_t;

typedef struct {
	U8	 au8_Tag[16];
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
    U8   u8_HashPBA[3][2];
    U8   u8_BootIdLoc;
    U8   u8_Reserved[24];//just for aligning to 64bytes + magic[16] = 80bytes
} __attribute__((aligned(16))) SPINAND_INFO_t;


extern SPINAND_INFO_t nand;
extern SPINAND_INFO_t	spi_nand;
extern PARTITION_INFO_t  partinfo;
extern PAIRED_PAGE_MAP_t ga_tPairedPageMap[512];

extern int NAND_ECC_TYPE_CIS2LIB[];
//extern int NAND_ECC_TYPE_MAPING[]; 
extern int NAND_ECC_CODELEN_v3[];
extern int NAND_ECC_CODELEN_v4[];
extern int NAND_ECC_CODELEN_v5[];
extern int *pNAND_ECC_CODELEN;

extern int (*pFn_ecc_encode)(int, int, unsigned char *, unsigned char *, int, unsigned char *, char);
//extern int (*pFn_ecc_encode)(int, unsigned char *, unsigned char *, int, unsigned char *, char);

extern U32 gu32_TotalPageByteCnt;
extern U8  *gpu8_NandPageBuf;

#define IF_MLC()  (nand.u32_Config&CELL_TYPE_MLC)

extern U32  drvNAND_CompareCISTag(U8 *tag);
extern void dump_part_records(PARTITION_RECORD_t *records, int cnt);
extern void dump_part_info(PARTITION_INFO_t *pPartInfo);
extern void initial_sector_param(void);

extern int  NAND_SearchNNI(void);
extern int  NAND_SearchPNI(void);
extern int  NAND_SearchPPM(void);
