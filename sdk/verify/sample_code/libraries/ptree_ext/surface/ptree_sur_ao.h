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

#ifndef __PTREE_SUR_AO_H__
#define __PTREE_SUR_AO_H__

#include "ptree_sur_sys.h"
#include "mi_ao_datatype.h"

typedef struct PTREE_SUR_AO_Info_s
{
    PTREE_SUR_SYS_Info_t base;
    unsigned int         u32PeriodSize;
    signed short         s16Volume;
    unsigned int         u32SampleRate;
    unsigned int         u32I2sSyncclock;
    unsigned int         u32I2sTdmslots;
    unsigned int         u32I2sSync;
    unsigned int         u32SyncMode;

    MI_AO_If_e             enAoIf;
    MI_AO_ChannelMode_e    eChannelMode;
    MI_AUDIO_Format_e      eAoFormat;
    MI_AUDIO_SoundMode_e   eAoSoundMode;
    MI_AUDIO_I2sMode_e     eAoI2sMode;
    MI_AUDIO_I2sFormat_e   eAoI2sFormat;
    MI_AUDIO_I2sMclk_e     eAoI2sMclkE;
    MI_AUDIO_I2sBitWidth_e eAoI2sBitWidth;
} PTREE_SUR_AO_Info_t;

#endif //__PTREE_MOD_AO_H__
