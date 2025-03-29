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
#ifndef _ST_COMMON_AI_GLASS_H_
#define _ST_COMMON_AI_GLASS_H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include "ptree.h"

typedef struct ST_Common_SensorLightTable_s
{
    int lux;
    unsigned int u32Shutter;
    unsigned int u32Sensorgain;
    unsigned int u32Ispgain;
} ST_Common_SensorLightTable_t;

typedef struct ST_AI_GLASSES_Handler_s
{
    void *pIns;                     // ptree instance
    PTREE_Config_t stConfig;        // ptree config
    int status;                     // Current status (Record or Capture)
} ST_Common_AiGlasses_Handle_t;


void ST_Common_AiGlasses_Init(void);
void ST_Common_AiGlasses_Deinit(void);

ST_Common_AiGlasses_Handle_t *ST_Common_AiGlasses_CreatePipeline(const char *pJsonPath);

void ST_Common_AiGlasses_DestroyPipeline(ST_Common_AiGlasses_Handle_t *pInstance);

int ST_Common_AiGlasses_Capture(ST_Common_AiGlasses_Handle_t *pInstance, const char *pSaveFileName,const char *pSaveThumbnail,
                                unsigned char enableLowPower);

int ST_Common_AiGlasses_StartRecord(ST_Common_AiGlasses_Handle_t *pInstance, const char *pSaveFileName, const char *pSaveThumbnail);
int ST_Common_AiGlasses_StopRecord(ST_Common_AiGlasses_Handle_t *pInstance);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_AI_GLASS_H_
