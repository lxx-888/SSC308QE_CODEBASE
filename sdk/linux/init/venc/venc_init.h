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

#ifndef __VENC_INIT_H__
#define __VENC_INIT_H__

#define SSTAR_VENC_DEVICE_NODE_NAME "sstar,venc"
#define SSTAR_JPE_DEVICE_NODE_NAME  "sstar,cedric-jpe"

void DrvVencClkPut(void *pstClk);
void* DrvVencOfClkGet(void *pDevNode, int s32ClkIdx);
void* DrvVencOfClkGetByName(void *pDevNode, char *pClkName);
void* DrvVencOfFindCompatibleNode(void *pDevNode, const char *type, const char *compat);
int DrvVencOfDeviceIsAvailable(void *pDevNode);
void* DrvVencOfFindProperty(void *pDevNode, char *name, int *cnt);
int DrvVencMsysOfPropertyReadU32Index(void *pDevNode, const char *propname, unsigned int u32ClkIdx, unsigned int *val);
int DrvVencCallUserModeHelper(char *path, char **argv, char **envp);

void* DrvVencCamClkGetHw(void *pstClk);
void* DrvVencCamClkGetParentByIndex(void *pstClkHw, unsigned int u32ClockIdx);
int DrvVencCamClkSetRate(void *pstClk, unsigned int u32ClkRate);
unsigned int DrvVencCamClkGetRate(void *pstClk);
unsigned int DrvVencCamClkRoundRate(void *pstClkHw, unsigned int u32ClockRateMax);
int DrvVencCamClkSetParent(void *pstClk, void *pstParentClk);
void DrvVencCamClkPrepareEnable(void *pstClk);
void DrvVencCamClkDisableUnprepare(void *pstClk);
int DrvVencCamOfIrqToResource(void *pDevNode, int s32DevIdx, void *pRes);
int DrvVencCamOfAddressToResource(void *pDevNode, int s32DevIdx, void **base, int *size);
unsigned int DrvVencCamIrqOfParseAndMap(void *pDevNode, int s32DevIdx);

#endif // __VENC_INIT_H__