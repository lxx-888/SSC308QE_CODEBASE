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
#ifndef __PTREE_API_AESTABLE_H__
#define __PTREE_API_AESTABLE_H__

enum PTREE_API_AESTABLE_CmdTable_e
{
    E_PTREE_API_AESTABLE_CMD_SET_START_MODE,
    E_PTREE_API_AESTABLE_CMD_GET_RUN_MODE,
    E_PTREE_API_AESTABLE_CMD_SET_RUN_MODE,
    E_PTREE_API_AESTABLE_CMD_SET_DEBUG_MODE,
};
enum PTREE_API_AESTABLE_RunMode_e
{
    E_PTREE_API_AESTABLE_RUN_MODE_SHOT   = 0, /* Captrue */
    E_PTREE_API_AESTABLE_RUN_MODE_RECORD = 1, /* Recode mode */
};
enum PTREE_API_AESTABLE_StartMode_e
{
    E_PTREE_API_AESTABLE_START_MODE_FORCE = 0, /* Starting frame counting from the first frame. */
    E_PTREE_API_AESTABLE_START_MODE_AUTO  = 1  /* Starting frame counting after ae stable. */
};

typedef struct PTREE_API_AESTABLE_CaptureParam_s
{
    unsigned char usingLowPower; /* Enable low powner mode for capture. */
    unsigned int  caputreCount;  /* Counting for capture after ae stable. */
    unsigned int  stableCount;   /* Counting down after ae stable. */
} PTREE_API_AESTABLE_CaptureParam_t;

typedef struct PTREE_API_AESTABLE_RecordParam_s
{
    unsigned int stableCount; /* Counting down after ae stable. */
} PTREE_API_AESTABLE_RecordParam_t;

typedef struct PTREE_API_AESTABLE_RunModeParam_s
{
    enum PTREE_API_AESTABLE_RunMode_e mode;
    union
    {
        PTREE_API_AESTABLE_CaptureParam_t captureParam;
        PTREE_API_AESTABLE_RecordParam_t  recordParam;
    };
} PTREE_API_AESTABLE_RunModeParam_t;
#endif /* __PTREE_API_H__ */
