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

int set_output_filename(char *pBinFilename, char *pOutFilename, int count, U16 u16_PartIdx)
{
	int i, j;

	for(i=0; i<count; i++)
		if('.' == pBinFilename[i] || '\0' == pBinFilename[i])
			break;
		else
			pOutFilename[i] = pBinFilename[i];

	for(j=0; j<partinfo.u16_PartCnt; j++)
		//if(0==(PARTTYPE_RESERVED&partinfo.records[j].u16_PartType))
		if(u16_PartIdx == partinfo.records[j].u16_PartType)
			break;
	if(j == partinfo.u16_PartCnt)
		return 0;

	sprintf(&pOutFilename[i], "_from blk %u%s", partinfo.records[j].u16_StartBlk, &pBinFilename[i]);
    
	return strlen(pOutFilename);
}

void generate_image_filename(char *pStr, char *pFilename, int count)
{
	for (; 0 <= count; count--) // loop for each image file
	{
		if('.' == pFilename[count])
			break;
	}
	
	pFilename[count] = '\0';
	memcpy(pStr, pFilename, count);
}


int get_image_filename(char *pStr, char *pFilename, int count)
{
	int byte_cnt=0, i;
	
	for(i=0; i<count; i++) // loop for each image file
	{
		if(' '==pStr[i])
			break;
				
		pFilename[i] = pStr[i];
		byte_cnt++;
	}
	
	pFilename[i] = '\0';

	return byte_cnt;
}
