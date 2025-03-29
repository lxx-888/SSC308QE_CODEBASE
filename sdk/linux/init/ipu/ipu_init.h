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
#ifndef __IPU_INIT_H__
#define __IPU_INIT_H__

typedef struct MI_IPU_Freq_Vol_s {
    unsigned int freq_mhz;
    unsigned int voltage_mv;
} MI_IPU_Freq_Vol_t;

typedef int (*scaling_freq_func)(unsigned int);
typedef struct MI_IPU_Common_Callback_s {
    scaling_freq_func scaling_freq;
} MI_IPU_Common_Callback_t;

int MI_IPU_ParseIrq(unsigned int *pu32Virq, int len);
int MI_IPU_GetPllClk(void **ppPll, int index);
int MI_IPU_LinuxInit(MI_IPU_Common_Callback_t *pstCommonCallback);
#if USE_IPU_OPP_TABLE == 1
MI_S32 MI_IPU_Parse_Freq_Vol_Table(void);
MI_S32 MI_IPU_Destroy_Freq_Vol_Table(void);
#endif
void MI_IPU_Iounmap_Wrapper(void *addr);

#endif
