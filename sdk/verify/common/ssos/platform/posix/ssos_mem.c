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

#include <stdlib.h>
#include "ssos_mem.h"

void *SSOS_MEM_Alloc(unsigned long size)
{
    return malloc(size);
}

void SSOS_MEM_Free(void *ptr)
{
    free(ptr);
}
void *SSOS_MEM_AlignAlloc(unsigned long size, unsigned char align)
{
    (void)align;
    return malloc(size);
}

void SSOS_MEM_AlignFree(void *ptr)
{
    free(ptr);
}
