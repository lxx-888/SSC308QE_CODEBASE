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
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "ptree_maker.h"
#include "ptree_db_json.h"
#include "arena.h"
#include "ptree.h"
#include "ptree_api_file.h"
#include "ptree_api_vif.h"
#include "ptree_api_isp.h"
#include "ptree_api_aestable.h"
#include "ptree_api_snr.h"
#include "ptree_api_sys.h"
#include "st_common.h"
#include "st_common_ai_glasses.h"
#include "sensor_light_table_imx681.h"
#include "light_misc_control_api.h"

enum {
    E_AI_GLASSES_CAPTURE = 0,
    E_AI_GLASSES_RECORD,
    E_AI_GLASSES_MAX
};

static int g_fdLightDev = -1;

static inline int _FileNameCheck(const char *pFile)
{
    int s32SurfixOff = 0;

    s32SurfixOff = strlen(pFile);
    if (s32SurfixOff > 6 && !strcmp(pFile + s32SurfixOff - 5, ".json"))
    {
        printf("[PTREE_DB_JSON]:File name (%s) check pass.\n", pFile);
        return 0;
    }

    return -1;
}

static int _PtreeTakeOff(PTREE_Config_t *pstConfig, const char *pJsonPath)
{
    /* Default set as using binary. */
    int s32Ret = 0;

    s32Ret = _FileNameCheck(pJsonPath);
    if (s32Ret == 0)
    {
        pstConfig->pDbInstance = PTREE_DB_JSON_Init(pJsonPath);
        return (pstConfig->pDbInstance && ARENA_Create(&pstConfig->pArenaHandle, 0x10000) == 0) ? s32Ret : -1;
    }

    printf("the file %s is not json format\n", pJsonPath);
    return -1;
}

static int _PtreeDropDown(PTREE_Config_t *pstConfig)
{
    ARENA_Destroy(pstConfig->pArenaHandle);
    PTREE_DB_JSON_Deinit(pstConfig->pDbInstance);
    return 0;
}

static void _EnableLowPowerMode(unsigned long enableLowPower)
{
    PTREE_API_AESTABLE_RunModeParam_t stAeStableParam;

    memset(&stAeStableParam, 0, sizeof(PTREE_API_AESTABLE_CaptureParam_t));
    PTREE_CMD("AESTABLE", 0, 1, E_PTREE_API_AESTABLE_CMD_GET_RUN_MODE, (unsigned long)&stAeStableParam);
    if (stAeStableParam.captureParam.usingLowPower != enableLowPower)
    {
        stAeStableParam.captureParam.usingLowPower = enableLowPower;
        PTREE_CMD("AESTABLE", 0, 1, E_PTREE_API_AESTABLE_CMD_SET_RUN_MODE, (unsigned long)&stAeStableParam);
    }
}

static int _SensorLightInit()
{
    int fd = -1;

    fd = Dev_Light_Misc_Device_GetFd();
    if (fd < 0)
    {
        printf("get light device fd fail\n");
        return -1;
    }

    if (Dev_Light_Misc_Device_Init(fd, 0))
    {
        printf("init light device fail\n");
        Dev_Light_Misc_Device_CloseFd(fd);
        fd = -1;
    }
    else
    {
        Dev_Light_Misc_Device_Get_LightSensor_Value(fd);
    }

    return fd;
}

static void _SensorLightDeinit(int *pFd)
{
    if (*pFd >= 0)
    {
        Dev_Light_Misc_Device_DeInit(*pFd, 0);
        Dev_Light_Misc_Device_CloseFd(*pFd);
        *pFd = -1;
    }
}

static const ST_Common_SensorLightTable_t *_SensorLightLookupItem(int fd, int lightTableIdx)
{
    int lux = 0;
    int i   = 0;
    int min = 0xFFFF;

    const ST_Common_SensorLightTable_t *pLightItem = NULL;

    if (fd < 0)
    {
        printf("Light device has not inited\n");
        return NULL;
    }

    lux = Dev_Light_Misc_Device_Get_LightSensor_Value(fd);
    if (lux <= 0)
    {
        printf("get lux fail, lux: %d\n", lux);
        return NULL;
    }

    for (i = 0; i < (sizeof(g_stLightTable[lightTableIdx]) / sizeof(g_stLightTable[lightTableIdx][0])); i++)
    {
        if (DIFF(lux, g_stLightTable[lightTableIdx][i].lux) < min)
        {
            min = DIFF(lux, g_stLightTable[lightTableIdx][i].lux);
            pLightItem = &g_stLightTable[lightTableIdx][i];
        }
    }

    printf("get lux is %d, use the nearest setting: lux[%d], shutter[%d], sensorGain[%d]\n", lux,
            pLightItem->lux, pLightItem->u32Shutter, pLightItem->u32Sensorgain);

    return (ST_Common_SensorLightTable_t *)pLightItem;
}

static int _SetAeParams(int lightTableIdx)
{
    PTREE_API_VIF_SetShutterGain_t stVifShutterGain = {0};
    const ST_Common_SensorLightTable_t *pLightItem = _SensorLightLookupItem(g_fdLightDev, lightTableIdx);

    if (!pLightItem)
    {
        printf("set ae param fail\n");
        return -1;
    }

    // set vif ae param
    stVifShutterGain.shutterTimeUs = pLightItem->u32Shutter;
    stVifShutterGain.aeGain        = pLightItem->u32Sensorgain;
    PTREE_CMD("VIF", 0, 0, E_PTREE_API_VIF_CMD_SET_SHUTTER_GAIN, (unsigned long)&stVifShutterGain);

    return 0;
}

static int _CheckoutStatus(ST_Common_AiGlasses_Handle_t *pHandle, int nextStatus)
{
    PTREE_ModuleDesc_t               modDesc;
    PTREE_API_SNR_Config_t           snrCfg;
    PTREE_API_SYS_OutputFpsParam_t   sysOutputFps;
    PTREE_API_SYS_OutputDepthParam_t sysOutputDepth;

    if (pHandle->status == nextStatus)
    {
        return 0;
    }

    memset(&modDesc, 0, sizeof(PTREE_ModuleDesc_t));
    memset(&snrCfg, 0, sizeof(PTREE_API_SNR_Config_t));
    memset(&sysOutputFps, 0, sizeof(PTREE_API_SYS_OutputFpsParam_t));
    memset(&sysOutputDepth, 0, sizeof(PTREE_API_SYS_OutputDepthParam_t));

    modDesc.modName = "SNR";
    modDesc.devId   = 0;
    modDesc.chnId   = 0;
    PTREE_IoCtl(&modDesc, E_PTREE_IOCTL_STOP, 0);
    PTREE_IoCtl(&modDesc, E_PTREE_IOCTL_UNPREPARE, 0);
    PTREE_IoCtl(&modDesc, E_PTREE_IOCTL_DEINIT, 0);

    PTREE_CMD("SNR", 0, 0, E_PTREE_API_SNR_CMD_GET_CONFIG, (unsigned long)&snrCfg);

    modDesc.modName = "AESTABLE";
    modDesc.devId   = 0;
    modDesc.chnId   = 0;
    if (nextStatus == E_AI_GLASSES_CAPTURE)
    {
        snrCfg.fps           = 15;
        snrCfg.res           = 0;
        sysOutputFps.fps     = 15;
        sysOutputDepth.bEn   = 1;
        sysOutputDepth.total = 3;
        /* Restart AESTABLE of capture when the last record top it */
        PTREE_IoCtl(&modDesc, E_PTREE_IOCTL_START, 0);
    }
    else if (nextStatus == E_AI_GLASSES_RECORD)
    {
        snrCfg.fps         = 30;
        snrCfg.res         = 2;
        sysOutputFps.fps   = 30;
        sysOutputDepth.bEn = 0;
        /* Modules in PASS will not stop and unbind if the last capture is not low power mode */
        PTREE_IoCtl(&modDesc, E_PTREE_IOCTL_STOP, 0);
    }

    PTREE_CMD("SNR", 0, 0, E_PTREE_API_SNR_CMD_SET_CONFIG, (unsigned long)&snrCfg);
    PTREE_CMD("VIF", 0, 0, E_PTREE_API_SYS_CMD_SET_OUTPUT_FPS, 0, (unsigned long)&sysOutputFps);
    PTREE_CMD("VIF", 0, 0, E_PTREE_API_SYS_CMD_SET_OUTPUT_DEPTH , 0, (unsigned long)&sysOutputDepth);

    modDesc.modName = "SNR";
    modDesc.devId   = 0;
    modDesc.chnId   = 0;
    PTREE_IoCtl(&modDesc, E_PTREE_IOCTL_INIT, 0);
    PTREE_IoCtl(&modDesc, E_PTREE_IOCTL_PREPARE, 0);
    PTREE_IoCtl(&modDesc, E_PTREE_IOCTL_START, 0);

    pHandle->status = nextStatus;
    return 0;
}

/**
 * ST_Common_AiGlasses_Init - Setup ptree modules.
 *
 */
void ST_Common_AiGlasses_Init(void)
{
    PTREE_MAKER_SETUP_API(SNR);
    PTREE_MAKER_SETUP_API(VIF, SYS);
    PTREE_MAKER_MOD_SETUP(ISP);
    PTREE_MAKER_SETUP_API(AESTABLE);
    PTREE_MAKER_MOD_SETUP(SCL);
    PTREE_MAKER_MOD_SETUP(VENC);
    PTREE_MAKER_MOD_SETUP(POOL);
    PTREE_MAKER_MOD_SETUP(LDC);
    PTREE_MAKER_MOD_SETUP(IQ);
    PTREE_MAKER_MOD_SETUP(EMPTY);
    PTREE_MAKER_SETUP_API(FILE);
    PTREE_MAKER_MOD_SETUP(PASS);

    g_fdLightDev = _SensorLightInit();
    if (g_fdLightDev < 0)
    {
        printf("Init light dev fail\n");
    }
}

/**
 * ST_Common_AiGlasses_Deinit - Clear ptree modules.
 */
void ST_Common_AiGlasses_Deinit(void)
{
    _SensorLightDeinit(&g_fdLightDev);
    PTREE_MAKER_Clear();
}


/**
 * ST_Common_AiGlasses_CreatePipeline - Init settings of ptree modules.
 * @pJsonPath: the path of json config file.
 *
 * Init ptree modules from json config file and create ptree instance.
 *
 * Return the point of ptree instance if init success, else return NULL.
 */
ST_Common_AiGlasses_Handle_t *ST_Common_AiGlasses_CreatePipeline(const char *pJsonPath)
{
    ST_Common_AiGlasses_Handle_t *pHandle = NULL;
    int   ret                             = 0;

    if (!pJsonPath)
    {
        printf("invalid param \n");
        return NULL;
    }

    pHandle = (ST_Common_AiGlasses_Handle_t *)malloc(sizeof(ST_Common_AiGlasses_Handle_t));
    memset(pHandle, 0, sizeof(ST_Common_AiGlasses_Handle_t));

    ret = _PtreeTakeOff(&pHandle->stConfig, pJsonPath);
    if (ret)
    {
        free(pHandle);
        return NULL;
    }

    pHandle->pIns = PTREE_CreateInstance(&pHandle->stConfig);
    if (!pHandle->pIns)
    {
        printf("Create instance fail!\n");
        _PtreeDropDown(&pHandle->stConfig);
        free(pHandle);
        pHandle = NULL;
    }
    PTREE_ConstructPipeline(pHandle->pIns);
    PTREE_StartPipeline(pHandle->pIns);

    pHandle->status = E_AI_GLASSES_CAPTURE;

    return pHandle;
}

/**
 * ST_Common_AiGlasses_DestroyPipeline - Destroy ptree instance.
 * @pHandle: the point of pipeline handle.
 *
 * Destory ptree instance and release the resources.
 */
void ST_Common_AiGlasses_DestroyPipeline(ST_Common_AiGlasses_Handle_t *pHandle)
{
    if (pHandle)
    {
        PTREE_StopPipeline(pHandle->pIns);
        PTREE_DestructPipeline(pHandle->pIns);
        PTREE_DestroyInstance(pHandle->pIns);
        _PtreeDropDown(&pHandle->stConfig);
        free(pHandle);
        pHandle = NULL;
    }
}

/**
 * ST_Common_AiGlasses_Capture - Take onw photo
 * @pHandle: the point of pipeline handle.
 * @pSaveFileName: the path to save captured photo.
 * @enableLowPower: if enter into low power mode or not while capturing done.
 *
 * Set saved file name and take one photo
 *
 * Return 0 if capture success, else return -1.
 */
int ST_Common_AiGlasses_Capture(ST_Common_AiGlasses_Handle_t *pHandle, const char *pSaveFileName, const char *pSaveThumbnail,
                                unsigned char enableLowPower)
{
    if (!pHandle || !pHandle->pIns)
    {
        printf("Capture ptree instance has not created yet.\n");
        return -1;
    }

    _CheckoutStatus(pHandle, E_AI_GLASSES_CAPTURE);

    PTREE_CMD("FILE", 0, 0, E_PTREE_API_FILE_CMD_SET_FILE_PATH_WRITE, 1, (unsigned long)pSaveFileName);
    PTREE_CMD("FILE", 0, 0, E_PTREE_API_FILE_CMD_SET_LEFT_FRAME_WRITE, 1, 1);
    if(pSaveThumbnail)
    {
        PTREE_CMD("FILE", 0, 0, E_PTREE_API_FILE_CMD_SET_FILE_PATH_WRITE, 2, (unsigned long)pSaveThumbnail);
        PTREE_CMD("FILE", 0, 0, E_PTREE_API_FILE_CMD_SET_LEFT_FRAME_WRITE, 2, 1);
    }
    _EnableLowPowerMode(enableLowPower);
    _SetAeParams(E_AI_GLASSES_CAPTURE);
    PTREE_CMD("FILE", 0, 0, E_PTREE_API_FILE_CMD_START_FILE_WRITE, 1);
    if(pSaveThumbnail)
    {
        PTREE_CMD("FILE", 0, 0, E_PTREE_API_FILE_CMD_START_FILE_WRITE, 2);
    }
    PTREE_CMD("FILE", 0, 0, E_PTREE_API_FILE_CMD_WAIT_FRAME_WRITE, 1, 3000);
    if(pSaveThumbnail)
    {
        PTREE_CMD("FILE", 0, 0, E_PTREE_API_FILE_CMD_WAIT_FRAME_WRITE, 2, 3000);
    }
    PTREE_CMD("FILE", 0, 0, E_PTREE_API_FILE_CMD_STOP_FILE_WRITE, 1);
    if(pSaveThumbnail)
    {
        PTREE_CMD("FILE", 0, 0, E_PTREE_API_FILE_CMD_STOP_FILE_WRITE, 2);
    }

    return 0;
}

/**
 * ST_Common_AiGlasses_StartRecord - Start recording.
 * @pHandle: the point of pipeline handle.
 * @pSaveFileName: the path to save record video.
 *
 * Create recording pipeline and keep recording until PipelineStopRecord is called.
 *
 * Return 0 if start recording success, else return -1.
 */
int ST_Common_AiGlasses_StartRecord(ST_Common_AiGlasses_Handle_t *pHandle, const char *pSaveFileName, const char *pSaveThumbnail)
{
    if (!pHandle || !pHandle->pIns)
    {
        return -1;
    }

    _CheckoutStatus(pHandle, E_AI_GLASSES_RECORD);

    PTREE_CMD("FILE", 0, 0, E_PTREE_API_FILE_CMD_SET_FILE_PATH_WRITE, 0, (unsigned long)pSaveFileName);
    PTREE_CMD("FILE", 0, 0, E_PTREE_API_FILE_CMD_SET_LEFT_FRAME_WRITE, 0, -1);
    if(pSaveThumbnail)
    {
        PTREE_CMD("FILE", 0, 0, E_PTREE_API_FILE_CMD_SET_FILE_PATH_WRITE, 3, (unsigned long)pSaveThumbnail);
        PTREE_CMD("FILE", 0, 0, E_PTREE_API_FILE_CMD_SET_LEFT_FRAME_WRITE, 3, 1);
    }

    _SetAeParams(E_AI_GLASSES_RECORD);

    PTREE_CMD("FILE", 0, 0, E_PTREE_API_FILE_CMD_START_FILE_WRITE, 0);
    if(pSaveThumbnail)
    {
        PTREE_CMD("FILE", 0, 0, E_PTREE_API_FILE_CMD_START_FILE_WRITE, 3);
    }
    PTREE_CMD("FILE", 0, 0, E_PTREE_API_FILE_CMD_WAIT_FRAME_WRITE, 0, 3000);
    if(pSaveThumbnail)
    {
        PTREE_CMD("FILE", 0, 0, E_PTREE_API_FILE_CMD_WAIT_FRAME_WRITE, 3, 3000);
        PTREE_CMD("FILE", 0, 0, E_PTREE_API_FILE_CMD_STOP_FILE_WRITE, 3);
    }

    return 0;
}

/**
 * ST_Common_AiGlasses_StopRecord - Stop recording.
 * @pHandle: the point of pipeline handle.
 *
 * Stop recording pipeline.
 *
 * Return 0 if stop recording success, else return -1.
 */
int ST_Common_AiGlasses_StopRecord(ST_Common_AiGlasses_Handle_t *pHandle)
{
    if (!pHandle || !pHandle->pIns)
    {
        return -1;
    }

    PTREE_CMD("FILE", 0, 0, E_PTREE_API_FILE_CMD_STOP_FILE_WRITE, 0);

    return 0;
}
