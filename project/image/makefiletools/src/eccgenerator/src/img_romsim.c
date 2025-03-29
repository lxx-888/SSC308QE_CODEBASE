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
#define CISPAGE "page0h.sim08.hex"
#define UNFD_PART_UBOOT     6

extern int create_blank_page(char ** buffer);
extern int gen_ecc_with_page_size(char *SourceBuffer, int ECC_Mode);
extern int GetKeyInfo(void *buf, int count, FILE *stream, char *Key);
extern int gen_ecc_with_sector_size(char *SourceBuffer, char *SpareBuf, int ECC_Mode);

extern char *out_filename;
extern char *in_filename;
extern char *g_ecc_type;

static void inline print_data_to_file(FILE*p, unsigned char *buf, int count)
{
	int i;
	for(i = 0; i < count; i++)
		fprintf(p,"%02X\n", buf[i]);
}

int SPINAND_WriteCISPage(void)
{
    int StrLen, i, count;
    
	FILE *out = NULL;
    FILE *in = NULL;
    
	char *pagebuffer = NULL;
    char file_name[FILENAME_LEN] = {0};
    char bin_name[FILENAME_LEN] = {0};
    
    if (0 == (StrLen = GetKeyInfo(g_StrBuf, (int)MSG_LEN, gpf_ini, KEY_CIS)))
    {
        printf("Err: no Key: %s\n", KEY_CIS);
        return 1;
    }

       
    for(i = 0; i < StrLen; i++) // one loop for one file -> one partition
	{
		// get image file name
		if(' '==g_StrBuf[i])
			continue;
		
		count = get_image_filename(&g_StrBuf[i], file_name, FILENAME_LEN);
        
		if(0 == count)
			break;
		else
			i += count-1;

        sprintf(bin_name, "%s%s", in_filename ? in_filename : OUTDIR, file_name);

        if (NULL == (in = fopen(bin_name, "rb")))
        {
            printf("[%s] Open %s fails\n", __func__, file_name);
            return 1;
        }

        generate_image_filename(file_name, bin_name, strlen(bin_name));
		sprintf(file_name, "%s.ecc.bin", bin_name);
        
        if (NULL == (out = fopen(file_name, "wb")))
    	{
    	    printf("[%s] create %s fail\n", __func__, file_name);
    		return 1;
    	}

         if(create_blank_page(&pagebuffer))
    	{
    		printf("[%s] create blank page fail\n", __func__);
    		return 1;
    	}

        fread(pagebuffer, 1, spi_nand.u16_PageByteCnt + spi_nand.u16_SpareByteCnt, in);
        gen_ecc_with_page_size(pagebuffer, ECC_TYPE_8BIT);
    	fwrite(pagebuffer, 1, spi_nand.u16_PageByteCnt + spi_nand.u16_SpareByteCnt, out);
        
    	fclose(out);
        fclose(in);
    	
    	printf("[%s] Generate %s Sucessfully\n", __func__, file_name);
    }
    
	free(pagebuffer);
    
	return 0;
}

int SPINAND_WriteHashPages(void)
{
	int StrLen;
	int count, i;
    int ecc_type = ECC_TYPE_8BIT;

    char BinFilename[FILENAME_LEN];
    
	FILE * pInFile, *pOutFile;
	
	char ac_InFilename[FILENAME_LEN], ac_OutFilename[FILENAME_LEN];

    if (!strcmp(g_ecc_type, "ECC_TYPE_16BIT"))
    {
        if ((spi_nand.u16_PageByteCnt == 0x800) && (spi_nand.u16_SpareByteCnt >= 128))
        {
            ecc_type = ECC_TYPE_16BIT;
        }

        if ((spi_nand.u16_PageByteCnt == 0x1000) && (spi_nand.u16_SpareByteCnt >= 256))
        {
            ecc_type = ECC_TYPE_16BIT;
        }
    }

    if ((ecc_type == ECC_TYPE_8BIT) && (((spi_nand.u16_SpareByteCnt * spi_nand.u16_SectorByteCnt) / spi_nand.u16_PageByteCnt) < 16))
    {
        sprintf(ac_InFilename, "%s%s%s", "echo Err: flash spare  too small!! > ", out_filename ? out_filename : OUTDIR, "eccgenerator.err");
        system(ac_InFilename);
        return 1;
    }

    StrLen = strlen(in_filename);

	for (i=0; i<StrLen; i++) // one loop for one file -> one partition
	{
		// get image file name
		if(' '==in_filename[i])
			continue;

		count = get_image_filename(&in_filename[i], BinFilename, FILENAME_LEN);
		if(0 == count)
			break;
		else
			i += count-1;

		sprintf(ac_InFilename, "%s%s", out_filename ? out_filename : OUTDIR, BinFilename);

		if(NULL == (pInFile = fopen(ac_InFilename, "rb")))
		{
			printf("\n[%s] Err: open file %s fails\n", __func__, ac_InFilename);
			return 1;
		}

        generate_image_filename(ac_OutFilename, ac_InFilename, strlen(ac_InFilename));

		sprintf(ac_OutFilename,"%s.ecc.bin", ac_InFilename);

		if(NULL == (pOutFile = fopen(ac_OutFilename, "wb")))
		{
			printf("\n[%s] Err: open file %s fails\n", __func__, ac_OutFilename);
			return 1;
		}
		
		memset(gpu8_NandPageBuf, 0xFF,  gu32_TotalPageByteCnt);
		
		while (0 != fread(gpu8_NandPageBuf, 1, spi_nand.u16_PageByteCnt, pInFile))
		{
			gen_ecc_with_page_size(gpu8_NandPageBuf, ecc_type);
			fwrite(gpu8_NandPageBuf, 1, spi_nand.u16_PageByteCnt + spi_nand.u16_SpareByteCnt, pOutFile);
			memset(gpu8_NandPageBuf, 0xFF,  gu32_TotalPageByteCnt);
		}

		fclose(pInFile);
		fclose(pOutFile);
        printf("[%s] Generate %s Sucessfully\n", __func__, ac_OutFilename);
	}
}

