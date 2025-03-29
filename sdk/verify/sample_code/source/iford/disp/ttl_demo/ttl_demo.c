/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized diSclosure,
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
#include "st_common_disp.h"
#include "mi_disp_datatype.h"



static MI_DISP_DEV u32DispDevId = 0;
static ST_Common_InputFile_Attr_t gstInputFileattr;

static MI_S32 STUB_TtlInit()
{
    STCHECKRESULT(ST_Common_Sys_Init());

    MI_DISP_DEV DispDev = u32DispDevId;
    /*----------------------------------------------------------------*/
    MI_DISP_PubAttr_t stDispPubAttr;
    memset(&stDispPubAttr, 0x0, sizeof(MI_DISP_PubAttr_t));
    ST_Common_DispGetDefaultPubAttr(&stDispPubAttr);
    stDispPubAttr.u32BgColor = YUYV_BLACK;
    stDispPubAttr.eIntfType = E_MI_DISP_INTF_TTL;
    stDispPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;
    /*----------------------------------------------------------------*/

    MI_DISP_LAYER DispLayer = 0;
    /*----------------------------------------------------------------*/
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    memset(&stLayerAttr, 0x0, sizeof(MI_DISP_VideoLayerAttr_t));
    ST_Common_DispGetDefaultLayerAttr(&stLayerAttr);
    /******** Canvas size of the video layer *******/
    stLayerAttr.stVidLayerSize.u16Width = 1024;
    stLayerAttr.stVidLayerSize.u16Height = 600;
    /******** Display resolution *******************/
    stLayerAttr.stVidLayerDispWin.u16X = 0;
    stLayerAttr.stVidLayerDispWin.u16Y = 0;
    stLayerAttr.stVidLayerDispWin.u16Width = 1024;
    stLayerAttr.stVidLayerDispWin.u16Height = 600;
    /******** Pixel format of the video layer*******/
    stLayerAttr.ePixFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    /*----------------------------------------------------------------*/

    MI_DISP_INPUTPORT LayerInputPort = 0;
    /*----------------------------------------------------------------*/
    MI_DISP_InputPortAttr_t stInputPortAttr;
    memset(&stInputPortAttr, 0x0, sizeof(MI_DISP_InputPortAttr_t));
    ST_Common_DispGetDefaultInputPortAttr(&stInputPortAttr);
    stInputPortAttr.u16SrcWidth = 1024;
    stInputPortAttr.u16SrcHeight = 600;
    stInputPortAttr.stDispWin.u16X = 0;
    stInputPortAttr.stDispWin.u16Y = 0;
    stInputPortAttr.stDispWin.u16Width = 1024;
    stInputPortAttr.stDispWin.u16Height = 600;
    stInputPortAttr.eDecompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
    /*----------------------------------------------------------------*/
    /* dev init */
    STCHECKRESULT(ST_Common_DispEnableDev(DispDev,  &stDispPubAttr));
    /* layer init */
    STCHECKRESULT(ST_Common_DispEnableLayer(DispDev, DispLayer, &stLayerAttr));
    /* inputport init */
    STCHECKRESULT(ST_Common_DispEnableInputPort(DispLayer, LayerInputPort, &stInputPortAttr));

    gstInputFileattr.bThreadExit = FALSE;
    gstInputFileattr.u32Width = 1024;
    gstInputFileattr.u32Height = 600;
    gstInputFileattr.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    sprintf(gstInputFileattr.InputFilePath,"./resource/input/YUV420SP_1024_600.yuv");
    gstInputFileattr.stModuleInfo.eModId = E_MI_MODULE_ID_DISP; //15
    gstInputFileattr.stModuleInfo.u32DevId = 0;
    gstInputFileattr.stModuleInfo.u32ChnId = 0;
    gstInputFileattr.stModuleInfo.u32PortId = 0;
    gstInputFileattr.u32Fps = 40;
    pthread_create(&gstInputFileattr.pPutDatathread, NULL, ST_Common_PutInputDataThread, (void *)&gstInputFileattr);

    return MI_SUCCESS;
}

static MI_S32 STUB_TtlUninit()
{
    MI_DISP_DEV DispDev = 0;
    MI_DISP_LAYER DispLayer = 0;
    MI_DISP_INPUTPORT LayerInputPort = 0;
    gstInputFileattr.bThreadExit = TRUE;
    pthread_join(gstInputFileattr.pPutDatathread,NULL);

    STCHECKRESULT(ST_Common_DispDisableInputPort(DispLayer, LayerInputPort));
    STCHECKRESULT(ST_Common_DispDisableVideoLayer(DispLayer, DispDev));
    STCHECKRESULT(ST_Common_DispDisableDev(DispDev));

    return MI_SUCCESS;
}

void ST_Disp_Usage(void)
{
    printf("Usage:./prog_disp_ttl_demo) single YUV420 on ttl\n");
    return;
}

int main()
{
    if(STUB_TtlInit() != MI_SUCCESS )
    {
        printf("STUB_TtlInit failed error !!!");
        return -1 ;
    }

    ST_Common_Pause();

    STCHECKRESULT(STUB_TtlUninit());

    return 0;
}