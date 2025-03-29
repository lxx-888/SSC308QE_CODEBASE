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

#ifndef __JPD_INIT_H__
#define __JPD_INIT_H__

void DrvJpdClkPut(void *pstClk);
void* DrvJpdOfClkGet(void *pDevNode, int s32ClkIdx);
void* DrvJpdOfClkGetByName(void *pDevNode, char *pClkName);
void* DrvJpdOfFindCompatibleNode(void *pDevNode, const char *pu8Type, const char *pu8Compat);
void* DrvJpdOfFindProperty(void *pDevNode, char *pu8Name, int *ps32Cnt);
int DrvJpdMsysOfPropertyReadU32Index(void *pDevNode, const char *pu8PropName, unsigned int u32ClkIdx, unsigned int *pu32Val);

void* DrvJpdCamClkGetHw(void *pClk);
unsigned int DrvJpdCamOfClkGetParentCount(void *pDevNode);
void* DrvJpdCamClkGetParentByIndex(void *pstClkHw, unsigned int u32ClockIdx);
int DrvJpdCamClkSetRate(void *pClk, unsigned int u32ClkRate);
unsigned int DrvJpdCamClkGetRate(void *pClk);
int DrvJpdCamClkSetParent(void *pstClk, void *pParentClk);
void DrvJpdCamClkPrepareEnable(void *pClk);
void DrvJpdCamClkDisableUnprepare(void *pClk);
int DrvJpdCamOfAddressToResource(void *pDevNode, int s32DevIdx, void **ppBase, unsigned int *pu32Size);
unsigned int DrvJpdCamIrqOfParseAndMap(void *pDevNode, int s32DevIdx);
int DrvJpdCamofPropertyReadU32(void *pDevNode, const char *pPropName, unsigned int *pu32Value);

#endif // __JPD_INIT_H__