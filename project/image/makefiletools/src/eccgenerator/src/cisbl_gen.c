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

// ==================================
// retrun the byte count for the Key
int GetKeyInfo(void *buf, int count, FILE *stream, char *Key);

// ==================================
char g_StrBuf[MSG_LEN];
FILE * gpf_ini=NULL;
#define UNFD_PART_UBOOT     6

U32 u32_Hash2Size;

char *out_filename = NULL;
char *sni_filename = NULL;
char *in_filename = NULL;
char *g_ecc_type = NULL;

// ==================================
U32 drvNAND_CheckSum(U8 *pu8_Data, U16 u16_ByteCnt)
{
	U32 u32_Sum = 0;

	while (u16_ByteCnt--)
		u32_Sum += *pu8_Data++;

	return u32_Sum;
}


static __inline void dump_mem_line(unsigned char *buf, int cnt)
{
	int i;

	printf( " 0x%08lx: ", (U32)buf);
	for (i= 0; i < cnt; i++)
		printf( "%02X ", buf[i]);

	printf( " | ");

	for (i = 0; i < cnt; i++)
		printf( "%c", (buf[i] >= 32 && buf[i] < 128) ? buf[i] : '.');

	printf( "\n");
}

void dump_mem(unsigned char *buf, int cnt)
{
	int i;

	for (i= 0; i < cnt; i+= 16)
		dump_mem_line(buf + i, 16);
}


/*
=====================================================================================
=====================================================================================
*/
/* 0: ok
     1: fail  */
int parse_nand_id(void)
{
	/* nand id format : NANDID: 2C DA 90 95 6 ....*/
	int i, tmp;

	;
	if(0 == (tmp = GetKeyInfo(g_StrBuf, (int)MSG_LEN, gpf_ini, KEY_NAND_ID)))
	{
		printf("Err: no NAND ID in %s\n", SETTING_INI);
		return 1;
	}
	
    printf("\nNAND ID: ");
	spi_nand.u8_IDByteCnt = 0;
	for(i=0; i<tmp; i++)
	{
		if(('0'<=g_StrBuf[i] && g_StrBuf[i]<='9') ||
		   ('A'<=g_StrBuf[i] && g_StrBuf[i]<='F') || 
		   ('a'<=g_StrBuf[i] && g_StrBuf[i]<='f'))
		{
			spi_nand.au8_ID[spi_nand.u8_IDByteCnt] = 
				strtol(&g_StrBuf[i], NULL, 16);
			
			printf("%X ", spi_nand.au8_ID[spi_nand.u8_IDByteCnt]);
			spi_nand.u8_IDByteCnt++;
			i++;			
		}
	}
	printf("\n");
    printf("u8_IDByteCnt: %d\n", spi_nand.u8_IDByteCnt);
	return 0;
}


int create_blank_page(char ** buffer)
{
	if(*buffer == NULL)
	{
		*buffer = (char*) malloc((spi_nand.u16_PageByteCnt + spi_nand.u16_SpareByteCnt)  * sizeof(char));
		if(*buffer == NULL)
		{
			printf("[%s] memory allocate fails\n", __func__);
			return 1;
		}
	}
	memset(*buffer, 0xff, spi_nand.u16_PageByteCnt + spi_nand.u16_SpareByteCnt);
	return 0;
}

int create_blank_block(char **buffer)
{
	if(*buffer == NULL)
	{
		*buffer= (char *)malloc(spi_nand.u16_BlkPageCnt * (spi_nand.u16_PageByteCnt + spi_nand.u16_SpareByteCnt) * sizeof(char));
		if(*buffer == NULL)
		{
			printf("[%s] Memory Allocate fails\n", __func__);
			return 1;
		}
	}
	memset(*buffer, 0xFF, spi_nand.u16_BlkPageCnt * (spi_nand.u16_PageByteCnt + spi_nand.u16_SpareByteCnt) * sizeof(char));
	return 0;
}

int main(int argc, char *const argv[])
{
	U32 u32_i;
    int result;

    while ((result = getopt(argc, argv, "i:o:s:e:")) != -1)
    {
        switch (result)
        {
            case 's':
            {
                sni_filename = optarg;
            }
            break;
            case 'o':
            {
                out_filename = optarg;
            }
            break;
            case 'i':
            {
                in_filename = optarg;
            }
            break;
            case 'e':
            {
                g_ecc_type = optarg;
            }
            break;
            default:
                break;
        }
    }

	// ========================================
	// initialize
    // ========================================
    #if 1

	if(SPINAND_SearchSNI())
		return 1;

	// default should have the ppm file, 
	for(u32_i=0; u32_i<spi_nand.u16_BlkPageCnt; u32_i++)
		    ga_tPairedPageMap[u32_i].u16_LSB = u32_i;

	// ========================================
	// handle image files
    // ========================================
    if(init_ECC_for_spinand())
		return 1;

	//SPINAND_WriteCISPage();
	SPINAND_WriteHashPages();
    //NAND_Write_PhyImages();
	//NAND_Write_UnfdImage();
	//NAND_Write_UBIImages();
	
    if(gpf_ini)
		fclose(gpf_ini);
	if(gpu8_NandPageBuf)
		free(gpu8_NandPageBuf);

	#else
	if(write_cis_to_block())
		return 1;

	if(argc > 1 && atoi(argv[1]) == 1)
	{
		printf("Generate CIS binary only\n");
		return 0;
	}

	if(NAND_WriteBL_UBOOT())
		return 1;
    #endif
	
	return 0;
}

/*
buf:       Storage location for data.
count:   Maximum number of items to be read.
stream: Pointer to FILE structure.
Key:      string of Key to get info.
return:  byte count read in buf, 0 means fail.
*/
int GetKeyInfo(void *buf, int count, FILE *stream, char *Key)
{
	int s32_OriPos, s32_KeyLen, s32_i, s32_CopyLen, s32_CopyCnt, s32_tmp=1;
	char *pStr;

    fseek(stream, 0, SEEK_SET);
	s32_OriPos = ftell(stream);
	s32_KeyLen = strlen(Key);
	//printf("KeyLen: %u, FilePos: %u\n", s32_KeyLen, s32_OriPos);
	
	do{
		pStr = fgets(buf, count, stream);	
		
		if(feof(stream)) // search Key from the beginning of stream
		{
		    rewind(stream);
			continue;
		}

		//printf("%i %s", s32_tmp, pStr);  s32_tmp++;

		for(s32_i=0; s32_i<s32_KeyLen; s32_i++)
		{
			if('#'==pStr[s32_i] || Key[s32_i]!=pStr[s32_i])
				break;
		}

		if(s32_i == s32_KeyLen)
		{
			s32_CopyLen = strlen(pStr)-s32_KeyLen > count ?
				count : strlen(pStr)-s32_KeyLen;

			s32_CopyCnt = 0;
			
			for(s32_i=0; s32_i<s32_CopyLen; s32_i++)
			{
				if(((':'==pStr[s32_KeyLen+s32_i] || ' '==pStr[s32_KeyLen+s32_i]) && 0==s32_CopyCnt)
					|| '\n'==pStr[s32_KeyLen+s32_i] || '\r'==pStr[s32_KeyLen+s32_i])
					continue; // filter ':'  ' '  '\n'  '\r'
					
				pStr[s32_CopyCnt] = pStr[s32_KeyLen+s32_i];
				s32_CopyCnt++;
			}

			pStr[s32_CopyCnt] = '\0';
			//printf("copy %s", pStr);			
			return s32_CopyCnt;
		}
	}while(ftell(stream) != s32_OriPos);

	return 0;
}

