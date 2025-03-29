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
#define ECC_LIB_FCIE3   0
#define ECC_LIB_FCIE4   0
#define ECC_LIB_FCIE5   1


// ===============================================
#define SBOOT_SIZE    0x10000
#define FILENAME_LEN  0x1000

#define OUTDIR	      "./output/"
#define INDIR 	      "./input/"
#define SETDIR        "./setting/"
/*
 Input files:
*/
#define SETTING_INI   SETDIR"user_setting.ini"

#define NANDID        SETDIR"NANDID.txt"
#define NNIFILE       SETDIR"NANDINFO.nni"
#define PPMFILE	      SETDIR"PAIRPAGEMAP_v2.ppm"
#define PNIFILE       SETDIR"PARTITION_v2_default.pni"

#define MBOOTFILE     INDIR"mboot.bin"

/*
 Output files: CIS.bin, Boot1.bin(including sboot/ sboot backup/ uboot.bin) and Boot2.bin (uboot.bin.bin Backup)
*/
#define CISBIN	      OUTDIR"CIS.bin"
#define BOOT1BIN      OUTDIR"boot1_from_blk0xa.bin"
#define BOOT2BIN      OUTDIR"boot2.bin"

/*
 key in SETTING_INI file
*/
#define KEY_NAND_ID     "NANDID" // NAND Flash ID
#define KEY_CIS         "CIS" // nni file
#define KEY_SNI         "SNI" // sni file
#define KEY_PNI         "PNI" // pni file
#define KEY_PPM         "PPM" // ppm file
#define KEY_ECC_VER     "FCIE" // FCIE3 or FCIE4

#define KEY_PHY_PART    "PHY" // unfd physical partitions 
#define KEY_UNFD_LGI    "UNFD_LGI" // unfd logical partitions
#define UNFD_LOGI_PART	0x8000 // bit-or if the partition needs Wear-Leveling

#define KEY_UBI_IMG	    "MTD_UBI"	// mtd partition for ubi attaching
#define KEY_MTD_PART    "MTD_PARTS" // mtd string for parsing mtd partition

#define KEY_HASH0		"HASH0"
#define KEY_HASH1		"HASH1"

#define KEY_ECC_TYPE	"ECC_TYPE"

