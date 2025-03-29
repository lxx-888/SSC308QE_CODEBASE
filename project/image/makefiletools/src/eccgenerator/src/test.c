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

#if 1
void Test_Main(void)
{
	U8 *pu8_Data, *pu8_Spare, *pu8_ECC;
	U32 u32_i;


	printf("%u %u %u\n", nand.u16_PageByteCnt, nand.u16_SpareByteCnt, nand.u16_SpareByteCnt);
	
	pu8_Data = (U8*)malloc(nand.u16_PageByteCnt);
	pu8_Spare = (U8*)malloc(nand.u16_SpareByteCnt+0x20);
	pu8_ECC = (U8*)malloc(nand.u16_SpareByteCnt+0x20);
	if(NULL==pu8_Data || NULL==pu8_Spare || NULL==pu8_ECC)
	{
		printf("Err: pu8_Data:%Xh pu8_Spare:%Xh pu8_ECC:%Xh \n", 			
			(int)pu8_Data, (int)pu8_Spare, (int)pu8_ECC);		
		return;
	}

	// test ECC
	for(u32_i=0; u32_i<nand.u16_PageByteCnt; u32_i++)
	    pu8_Data[u32_i] = 0xFF;
	for(u32_i=0; u32_i<nand.u16_SpareByteCnt; u32_i++)
	{
	    pu8_Spare[u32_i] = 0xFF;
		pu8_ECC[u32_i] = 0xFF;
	}
    
	gen_ecc_with_sector_size(pu8_Data, pu8_Spare, nand.u16_ECCType);

	for(u32_i=0; u32_i<gu16_SectorSpareByteCnt; u32_i++)
    {
		#if 0
        if(u32_i%2==0)
			printf("%d =%02X\n",u32_i, pu8_Spare[u32_i+1]);
		else
			printf("%d =%02X\n",u32_i, pu8_Spare[u32_i-1]);
		#else
		if(u32_i%8==0)
			printf("\n"); 
	    printf(" %02X ", *pu8_Spare);
	    pu8_Spare++;
		#endif
    }
	printf("\n");
	
}
#endif

