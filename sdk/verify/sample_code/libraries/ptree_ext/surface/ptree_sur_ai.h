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

#ifndef __PTREE_SUR_AI_H__
#define __PTREE_SUR_AI_H__

#include "ptree_sur_sys.h"
#include "mi_ai_datatype.h"

typedef struct PTREE_SUR_AI_Info_s
{
    PTREE_SUR_SYS_Info_t   base;
    unsigned int           u32PeriodSize;
    signed short           s16Volume;
    unsigned int           u32Echo;
    unsigned int           u32SampleRate;
    unsigned int           u32InterLeaved;
    unsigned int           u32I2sSyncclock;
    unsigned int           u32I2sTdmslots;
    unsigned int           u32IfCount;
    MI_AI_If_e             enAiIf[MI_AI_MAX_CHN_NUM / 2];
    MI_AUDIO_Format_e      eAiFormat;
    MI_AUDIO_SoundMode_e   eAiSoundMode;
    MI_AUDIO_I2sMode_e     eAiI2sMode;
    MI_AUDIO_I2sFormat_e   eAiI2sFormat;
    MI_AUDIO_I2sMclk_e     eAiI2sMclkE;
    MI_AUDIO_I2sBitWidth_e eAiI2sBitWidth;
} PTREE_SUR_AI_Info_t;

#endif //__PTREE_MOD_AI_H__
