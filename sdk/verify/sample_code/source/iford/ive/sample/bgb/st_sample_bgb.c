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
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <string.h>

#include "mi_sys.h"
#include "st_common.h"
#include "st_common_ive.h"

#define MODE_REPLACE_Format    E_MI_IVE_IMAGE_TYPE_YUV420SP
#define MODE_REPLACE_BgWidth    640
#define MODE_REPLACE_BgHight    360
#define u16Width_Ori            3840
#define u16Height_Ori           2160

#define srcImageNameOri     "./resource/input/ive/3840x2160_YUV420SP_ori.bin"
#define srcImageNameYMask   "./resource/input/ive/640x360_U8C1_ymask.bin"
#define srcImageNameUVMask  "./resource/input/ive/640x360_U8C1_uvmask.bin"
#define srcImageNamesOri    "./resource/input/ive/640x360_YUV420SP_sori.bin"
#define srcImageNamesBg     "./resource/input/ive/640x360_YUV420SP_bg.bin"
#define OUTPUT_PATH         "./out/ive/"
#define dstImageName        "./out/ive/Output_BGBlur_3840x2160_YUV420SP.bin"

int main()
{
    STCHECKRESULT(ST_Common_Sys_Init());

    MI_S32 ret;
    MI_IVE_HANDLE handle = 0;
    MI_IVE_SrcImage_t stSrcOri, stSrcYMask, stSrcUvMask, stSrcRepBg;
    MI_IVE_DstImage_t stDst;
    MI_IVE_BgBlurCtrl_t ctrl =
    {
        .eBgBlurMode = E_MI_IVE_BGBLUR_MODE_BLUR,           //E_MI_IVE_BGBLUR_MODE_REPLACE
        .u8MaskThr  = 127,
        .u8BlurLv   = 0,
        .u8ScalingStage = 2,
        .eBgBlurMaskOp = E_MI_IVE_BGBLUR_MASK_OP_NONE,
        .u8SaturationLv = 128,
        .u8MosaicSize = 10
    };

    memset(&stDst,      0, sizeof(MI_IVE_DstImage_t));
    memset(&stSrcOri,   0, sizeof(MI_IVE_SrcImage_t));
    memset(&stSrcYMask, 0, sizeof(MI_IVE_SrcImage_t));
    memset(&stSrcUvMask,0, sizeof(MI_IVE_SrcImage_t));
    memset(&stSrcRepBg, 0, sizeof(MI_IVE_SrcImage_t));

    ret = ST_Common_IveCreateHandle(handle);
    if (ret != MI_SUCCESS)
    {
        printf("Could not create IVE handle\n");
        return ret;
    }

    ret = ST_Common_IveAllocateImage(&stSrcOri, E_MI_IVE_IMAGE_TYPE_YUV420SP, ALIGN_UP(u16Width_Ori, 4), u16Width_Ori, u16Height_Ori);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate input buffer 0\n");
        ST_Common_IveDestroy(handle);
        return ret;
    }
    ret = ST_Common_IveAllocateImage(&stSrcYMask, E_MI_IVE_IMAGE_TYPE_U8C1, ALIGN_UP(MODE_REPLACE_BgWidth, 4), MODE_REPLACE_BgWidth,MODE_REPLACE_BgHight);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate input buffer 0\n");
        CHECK_AND_FREE_IMAGE(stSrcOri);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveAllocateImage(&stSrcUvMask, E_MI_IVE_IMAGE_TYPE_U8C1, ALIGN_UP(MODE_REPLACE_BgWidth >> 1, 4), MODE_REPLACE_BgWidth >> 1,MODE_REPLACE_BgHight >> 1);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate input buffer 0\n");
        CHECK_AND_FREE_IMAGE(stSrcYMask);
        CHECK_AND_FREE_IMAGE(stSrcOri);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    if (E_MI_IVE_BGBLUR_MODE_BLUR == ctrl.eBgBlurMode)
    {
        ret = ST_Common_IveAllocateImage(&stSrcRepBg,E_MI_IVE_IMAGE_TYPE_YUV420SP, ALIGN_UP(MODE_REPLACE_BgWidth, 4), MODE_REPLACE_BgWidth,MODE_REPLACE_BgHight);
        if (ret != MI_SUCCESS)
        {
            printf("Can't allocate input buffer 0\n");
            CHECK_AND_FREE_IMAGE(stSrcYMask);
            CHECK_AND_FREE_IMAGE(stSrcUvMask);
            CHECK_AND_FREE_IMAGE(stSrcOri);
            ST_Common_IveDestroy(handle);
            return ret;
        }
        ret = ST_Common_IveInitInputImage(&stSrcRepBg, srcImageNamesOri);
        if (ret != MI_SUCCESS)
        {
            printf("Init input buffer Failed");
            CHECK_AND_FREE_IMAGE(stSrcYMask);
            CHECK_AND_FREE_IMAGE(stSrcUvMask);
            CHECK_AND_FREE_IMAGE(stSrcOri);
            CHECK_AND_FREE_IMAGE(stSrcRepBg);
            ST_Common_IveDestroy(handle);
            return ret;
        }
        printf("[pattern]: %s\n", srcImageNamesOri);
    }
    else
    {
        ret = ST_Common_IveAllocateImage(&stSrcRepBg, E_MI_IVE_IMAGE_TYPE_YUV420SP, ALIGN_UP(MODE_REPLACE_BgWidth, 4), MODE_REPLACE_BgWidth,MODE_REPLACE_BgHight);
        if (ret != MI_SUCCESS)
        {
            printf("Can't allocate input buffer 0\n");
            CHECK_AND_FREE_IMAGE(stSrcYMask);
            CHECK_AND_FREE_IMAGE(stSrcUvMask);
            CHECK_AND_FREE_IMAGE(stSrcOri);
            ST_Common_IveDestroy(handle);
            return ret;
        }
        ret = ST_Common_IveInitInputImage(&stSrcRepBg, srcImageNamesBg);
        if (ret != MI_SUCCESS)
        {
            printf("Init input buffer Failed");
            CHECK_AND_FREE_IMAGE(stSrcYMask);
            CHECK_AND_FREE_IMAGE(stSrcUvMask);
            CHECK_AND_FREE_IMAGE(stSrcOri);
            CHECK_AND_FREE_IMAGE(stSrcRepBg);
            ST_Common_IveDestroy(handle);
            return ret;
        }
        printf("[pattern]: %s\n", srcImageNamesBg);
    }

    ret = ST_Common_IveAllocateImage(&stDst, E_MI_IVE_IMAGE_TYPE_YUV420SP, ALIGN_UP(u16Width_Ori, 4), u16Width_Ori, u16Height_Ori);
    if (ret != MI_SUCCESS)
    {
        printf("Init input buffer Failed");
        CHECK_AND_FREE_IMAGE(stSrcYMask);
        CHECK_AND_FREE_IMAGE(stSrcUvMask);
        CHECK_AND_FREE_IMAGE(stSrcOri);
        CHECK_AND_FREE_IMAGE(stSrcRepBg);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveInitInputImage(&stSrcOri,      srcImageNameOri);
    if (ret != MI_SUCCESS)
    {
        printf("Init input buffer Failed");
        CHECK_AND_FREE_IMAGE(stSrcYMask);
        CHECK_AND_FREE_IMAGE(stSrcUvMask);
        CHECK_AND_FREE_IMAGE(stSrcOri);
        CHECK_AND_FREE_IMAGE(stSrcRepBg);
        CHECK_AND_FREE_IMAGE(stDst);
        ST_Common_IveDestroy(handle);
        return ret;
    }
    ret = ST_Common_IveInitInputImage(&stSrcYMask,    srcImageNameYMask);
    if (ret != MI_SUCCESS)
    {
        printf("Init input buffer Failed");
        CHECK_AND_FREE_IMAGE(stSrcYMask);
        CHECK_AND_FREE_IMAGE(stSrcUvMask);
        CHECK_AND_FREE_IMAGE(stSrcOri);
        CHECK_AND_FREE_IMAGE(stSrcRepBg);
        CHECK_AND_FREE_IMAGE(stDst);
        ST_Common_IveDestroy(handle);
        return ret;
    }
    ret = ST_Common_IveInitInputImage(&stSrcUvMask,   srcImageNameUVMask);
    if (ret != MI_SUCCESS)
    {
        printf("Init input buffer Failed");
        CHECK_AND_FREE_IMAGE(stSrcYMask);
        CHECK_AND_FREE_IMAGE(stSrcUvMask);
        CHECK_AND_FREE_IMAGE(stSrcOri);
        CHECK_AND_FREE_IMAGE(stSrcRepBg);
        CHECK_AND_FREE_IMAGE(stDst);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = MI_IVE_BGBlur(handle, &stSrcYMask, &stSrcUvMask, &stSrcOri, &stSrcRepBg, &stDst, &ctrl);
    if (ret != MI_SUCCESS)
    {
        printf("Run MI_IVE_BGBlur Failed");
        CHECK_AND_FREE_IMAGE(stSrcYMask);
        CHECK_AND_FREE_IMAGE(stSrcUvMask);
        CHECK_AND_FREE_IMAGE(stSrcOri);
        CHECK_AND_FREE_IMAGE(stSrcRepBg);
        CHECK_AND_FREE_IMAGE(stDst);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    printf("[pattern]: %s\n", srcImageNameOri);
    printf("[pattern]: %s\n", srcImageNameYMask);
    printf("[pattern]: %s\n", srcImageNameUVMask);

    ret = ST_Common_CheckMkdirOutFile(OUTPUT_PATH);
    if (ret != MI_SUCCESS)
    {
        printf("Can't mkdir output path %s\n", OUTPUT_PATH);
        return ret;
    }

    ret = ST_Common_IveSaveOutputImage(&stDst, dstImageName);
    if (ret != MI_SUCCESS)
    {
        printf("Can't save data to output file %s\n", dstImageName);
        CHECK_AND_FREE_IMAGE(stSrcYMask);
        CHECK_AND_FREE_IMAGE(stSrcUvMask);
        CHECK_AND_FREE_IMAGE(stSrcOri);
        CHECK_AND_FREE_IMAGE(stSrcRepBg);
        CHECK_AND_FREE_IMAGE(stDst);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    CHECK_AND_FREE_IMAGE(stSrcYMask);
    CHECK_AND_FREE_IMAGE(stSrcUvMask);
    CHECK_AND_FREE_IMAGE(stSrcOri);
    CHECK_AND_FREE_IMAGE(stSrcRepBg);
    CHECK_AND_FREE_IMAGE(stDst);

    ST_Common_IveDestroy(handle);
    return 0;
}



