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
#include <stdlib.h>
#include <string.h>

typedef enum{
    MODE_NO_SUPPORT = 0,
    NO_ERROR,
    REDU_LONG_NO_MATCH,
    IMCOMPLETE_DECODING,
}RETURN_MESSAGE;

int encode(int ECC_MODE,unsigned char *pSourceData,unsigned char *pSpareArea,int REDU_LENGTH,unsigned char *pOutputData,char debug);
