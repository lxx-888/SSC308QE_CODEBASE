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

#ifndef __IVE_INIT_H__
#define __IVE_INIT_H__

// for irq/bankbase
void* DrvIveOfFindCompatibleNode(void *pDevNode, const char *pu8Type, const char *pu8Compat);
int DrvIveCamOfAddressToResource(void *pDevNode, int s32ResourceIdx, unsigned long long *pBankBase);
int DrvIveCamOfIrqToResource(void *pDevNode, int s32ResourceIdx, void *pRes);
#endif // __IVE_INIT_H__