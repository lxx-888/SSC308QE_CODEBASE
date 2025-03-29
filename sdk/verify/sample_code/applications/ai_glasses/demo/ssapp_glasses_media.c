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
#include "ssapp_glasses_misc.h"
#include "ssapp_glasses_media.h"
#include "st_common_ai_glasses.h"

static const char *                  gp_pWholeJsonPath = "./resource/ai_glasses_aestable_whole.json";
static ST_Common_AiGlasses_Handle_t *gp_pstHandle      = NULL;

MI_S32 SSAPP_GLASSES_MEDIA_Init(void)
{
    ST_Common_AiGlasses_Init();
    gp_pstHandle = ST_Common_AiGlasses_CreatePipeline(gp_pWholeJsonPath);
    if (!gp_pstHandle)
    {
        printf("init photo pipe json failed\n");
        return -1;
    }

    return MI_SUCCESS;
}

MI_S32 SSAPP_GLASSES_MEDIA_DeInit(void)
{
    if (!gp_pstHandle)
    {
        printf("Deinit gp_pstHandle is NULL\n");
        return -1;
    }

    ST_Common_AiGlasses_DestroyPipeline(gp_pstHandle);
    ST_Common_AiGlasses_Deinit();
    return MI_SUCCESS;
}

MI_S32 SSAPP_GLASSES_MEDIA_TakePhoto(MI_U8 u8PhotoCnt)
{
    int    cnt                           = 0;
    MI_S32 ret                           = MI_SUCCESS;
    char   filePath[2][FILE_NAME_LENGTH] = {0};

    if (!gp_pstHandle)
    {
        printf("Take photo gp_pstHandle is NULL\n");
        return -1;
    }

    for (cnt = 0; cnt < u8PhotoCnt; cnt++)
    {
        // continue
        snprintf(filePath[0], sizeof(filePath[0]), FTP_SERVER_PATH "ss_photo_%ld.jpg", SSAPP_GLASSES_MISC_GetPts());
        snprintf(filePath[1], sizeof(filePath[1]), FTP_SERVER_PATH "ss_photo_thumbnail_%ld.jpg",
                 SSAPP_GLASSES_MISC_GetPts());
        printf("1photo[%d] name:%s, %s\n", cnt, filePath[0], filePath[1]);
        ret |= ST_Common_AiGlasses_Capture(gp_pstHandle, filePath[0], filePath[1], cnt == u8PhotoCnt - 1);
    }
    return ret;
}

MI_S32 SSAPP_GLASSES_MEDIA_StartRec(void)
{
    char filePath[2][FILE_NAME_LENGTH] = {0};

    if (!gp_pstHandle)
    {
        printf("Start record pstRecordHandle is NULL\n");
        return -1;
    }

    snprintf(filePath[0], sizeof(filePath[0]), FTP_SERVER_PATH "ss_movie_%ld.es", SSAPP_GLASSES_MISC_GetPts());
    snprintf(filePath[1], sizeof(filePath[1]), FTP_SERVER_PATH "ss_movie_thumbnail_%ld.jpg",
             SSAPP_GLASSES_MISC_GetPts());
    printf("movie name:%s, %s\n", filePath[0], filePath[1]);
    return ST_Common_AiGlasses_StartRecord(gp_pstHandle, filePath[0], filePath[1]);
}

MI_S32 SSAPP_GLASSES_MEDIA_StopRec(void)
{
    if (!gp_pstHandle)
    {
        printf("Stop record pstRecordHandle is NULL\n");
        return -1;
    }

    return ST_Common_AiGlasses_StopRecord(gp_pstHandle);
}
