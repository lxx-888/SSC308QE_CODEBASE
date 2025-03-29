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

#include "mi_common_datatype.h"
#include "mi_ldc_datatype.h"
#include "mi_ldc.h"
#include "mi_sys_datatype.h"
#include "mi_sys.h"
#include "ssos_def.h"
#include "ssos_mem.h"
#include "ptree_log.h"
#include "ptree_packet_raw.h"
#include "ptree_mod_sys.h"
#include "ptree_mod_ldc.h"
#include "ptree_sur_ldc.h"
#include "ptree_sur_sys.h"
#include "ptree_maker.h"

#define MMA_ALLOC_BUF(pphy, ppvaddr, __u32Size, __exit_func__)           \
    do                                                                   \
    {                                                                    \
        *pphy    = 0UL;                                                  \
        *ppvaddr = NULL;                                                 \
        s32Ret   = MI_SYS_MMA_Alloc(0, NULL, __u32Size, pphy);           \
        if (s32Ret)                                                      \
        {                                                                \
            PTREE_ERR("failed to MI_SYS_MMA_Alloc Buf: %d ", __u32Size); \
            goto __exit_func__;                                          \
        }                                                                \
        s32Ret = MI_SYS_Mmap(*pphy, __u32Size, ppvaddr, false);          \
        if (s32Ret)                                                      \
        {                                                                \
            PTREE_ERR("failed to map Buf");                              \
            goto __exit_func__;                                          \
        }                                                                \
    } while (0)

#define MMA_FREE_BUF(phy, pvaddr, __u32Size)  \
    do                                        \
    {                                         \
        if (pvaddr && __u32Size)              \
            MI_SYS_Munmap(pvaddr, __u32Size); \
        if (phy)                              \
            MI_SYS_MMA_Free(0, phy);          \
        phy    = 0UL;                         \
        pvaddr = NULL;                        \
    } while (0)

#define PTREE_MOD_LDC_DEV_NUM (1)

typedef struct PTREE_MOD_LDC_Obj_s    PTREE_MOD_LDC_Obj_t;
typedef struct PTREE_MOD_LDC_InObj_s  PTREE_MOD_LDC_InObj_t;
typedef struct PTREE_MOD_LDC_OutObj_s PTREE_MOD_LDC_OutObj_t;

struct PTREE_MOD_LDC_Obj_s
{
    PTREE_MOD_SYS_Obj_t base;
};
struct PTREE_MOD_LDC_InObj_s
{
    PTREE_MOD_SYS_InObj_t base;
};
struct PTREE_MOD_LDC_OutObj_s
{
    PTREE_MOD_SYS_OutObj_t base;
};

static int                 _PTREE_MOD_LDC_Init(PTREE_MOD_SYS_Obj_t *sysMod);
static int                 _PTREE_MOD_LDC_Deinit(PTREE_MOD_SYS_Obj_t *sysMod);
static int                 _PTREE_MOD_LDC_Start(PTREE_MOD_SYS_Obj_t *sysMod);
static int                 _PTREE_MOD_LDC_Stop(PTREE_MOD_SYS_Obj_t *sysMod);
static PTREE_MOD_InObj_t * _PTREE_MOD_LDC_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static PTREE_MOD_OutObj_t *_PTREE_MOD_LDC_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static void                _PTREE_MOD_LDC_Free(PTREE_MOD_SYS_Obj_t *sysMod);

static int                _PTREE_MOD_LDC_InStart(PTREE_MOD_SYS_InObj_t *sysModIn);
static void               _PTREE_MOD_LDC_InFree(PTREE_MOD_SYS_InObj_t *sysModIn);
static PTREE_MOD_InObj_t *_PTREE_MOD_LDC_InNew(PTREE_MOD_SYS_Obj_t *sysMod, unsigned int loopId);

static int  _PTREE_MOD_LDC_OutStart(PTREE_MOD_SYS_OutObj_t *sysModOut);
static void _PTREE_MOD_LDC_OutGetPacketInfo(PTREE_MOD_SYS_OutObj_t *sysModOut, PTREE_PACKET_RAW_RawInfo_t *rawInfo);
static void _PTREE_MOD_LDC_OutFree(PTREE_MOD_SYS_OutObj_t *sysModOut);
static PTREE_MOD_OutObj_t *_PTREE_MOD_LDC_OutNew(PTREE_MOD_SYS_Obj_t *sysMod, unsigned int loopId);

static const PTREE_MOD_SYS_Ops_t G_PTREE_MOD_LDC_SYS_OPS = {
    .init         = _PTREE_MOD_LDC_Init,
    .deinit       = _PTREE_MOD_LDC_Deinit,
    .start        = _PTREE_MOD_LDC_Start,
    .stop         = _PTREE_MOD_LDC_Stop,
    .createModIn  = _PTREE_MOD_LDC_CreateModIn,
    .createModOut = _PTREE_MOD_LDC_CreateModOut,
};
static const PTREE_MOD_SYS_Hook_t G_PTREE_MOD_LDC_SYS_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_LDC_Free,
};

static const PTREE_MOD_SYS_InOps_t G_PTREE_MOD_LDC_SYS_IN_OPS = {
    .start = _PTREE_MOD_LDC_InStart,
};
static const PTREE_MOD_SYS_InHook_t G_PTREE_MOD_LDC_SYS_IN_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_LDC_InFree,
};

static const PTREE_MOD_SYS_OutOps_t G_PTREE_MOD_LDC_SYS_OUT_OPS = {
    .start         = _PTREE_MOD_LDC_OutStart,
    .getPacketInfo = _PTREE_MOD_LDC_OutGetPacketInfo,
};
static const PTREE_MOD_SYS_OutHook_t G_PTREE_MOD_LDC_SYS_OUT_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_LDC_OutFree,
};

static unsigned int g_ldcCreateDev[PTREE_MOD_LDC_DEV_NUM] = {0};

static void * g_pVirMap2BinX[LDC_MAX_REGION_NUM], *g_pVirMap2BinY[LDC_MAX_REGION_NUM];
static MI_PHY g_phyMap2BinX[LDC_MAX_REGION_NUM], g_phyMap2BinY[LDC_MAX_REGION_NUM];

static inline int _PTREE_MOD_LDC_GetFileSize(const char *path)
{
    SSOS_IO_File_t fd        = NULL;
    int            totalSize = 0;

    fd = SSOS_IO_FileOpen(path, SSOS_IO_O_RDONLY, 444);
    if (!fd)
    {
        PTREE_ERR("failed to open:%s ", path);
        return -1;
    }

    totalSize = SSOS_IO_FileSeek(fd, 0, SSOS_IO_SEEK_END);
    SSOS_IO_FileClose(fd);
    return totalSize;
}
static inline int _PTREE_MOD_LDC_ReadFileToBuf(const char *path, void *pBuf, unsigned int length)
{
    int  s32Ret = -1, readSize = 0, totalSize = 0, buflen = (int)length;
    int *fd = NULL;

    if (!pBuf)
    {
        return -1;
    }

    fd = SSOS_IO_FileOpen(path, SSOS_IO_O_RDONLY, 444);
    if (!fd)
    {
        PTREE_ERR("failed to open:%s ", path);
        goto __exit_err__;
    }

    totalSize = SSOS_IO_FileSeek(fd, 0, SSOS_IO_SEEK_END);
    if (totalSize > buflen)
    {
        PTREE_ERR("%s file content is less need:%d %d", path, totalSize, buflen);
        goto __exit_err__;
    }

    SSOS_IO_FileSeek(fd, 0, SSOS_IO_SEEK_SET);

    readSize = SSOS_IO_FileRead(fd, pBuf, totalSize);
    if (readSize != totalSize)
    {
        PTREE_ERR("failed to read buf:%s %p %d %d!", path, pBuf, readSize, buflen);
        goto __exit_err__;
    }

    SSOS_IO_FileClose(fd);
    return 0;

__exit_err__:
    if (fd)
    {
        SSOS_IO_FileClose(fd);
    }
    return s32Ret;
}
static int _PTREE_MOD_LDC_LdcModeInit(PTREE_SUR_LDC_Info_t *ldcInfo, void *calibAddr)
{
    int                 s32Ret;
    MI_LDC_ChnLDCAttr_t ldcChnAttr;
    unsigned int        u32Idx = 0;
    memset(&ldcChnAttr, 0, sizeof(MI_LDC_ChnLDCAttr_t));
    ldcChnAttr.bBgColor                        = ldcInfo->ldcCfg.enBgColor == 1 ? TRUE : FALSE;
    ldcChnAttr.u32BgColor                      = ldcInfo->ldcCfg.bgColor;
    ldcChnAttr.eMountMode                      = (MI_LDC_MountMode_e)ldcInfo->ldcCfg.mountMode;
    ldcChnAttr.stCalibInfo.u32CalibPolyBinSize = ldcInfo->calib.len;
    ldcChnAttr.stCalibInfo.pCalibPolyBinAddr   = calibAddr;
    ldcChnAttr.stCalibInfo.s32CenterXOffset    = ldcInfo->ldcCfg.centerXOff;
    ldcChnAttr.stCalibInfo.s32CenterYOffset    = ldcInfo->ldcCfg.centerYOff;
    ldcChnAttr.stCalibInfo.s32FisheyeRadius    = ldcInfo->ldcCfg.fisheyeRadius;
    ldcChnAttr.u32RegionNum                    = ldcInfo->ldcCfg.regionNum;
    for (u32Idx = 0; u32Idx < ldcChnAttr.u32RegionNum && u32Idx < LDC_MAX_REGION_NUM; u32Idx++)
    {
        ldcChnAttr.stRegionAttr[u32Idx].eRegionMode    = (MI_LDC_RegionMode_e)ldcInfo->ldcCfg.region[u32Idx].regionMode;
        ldcChnAttr.stRegionAttr[u32Idx].stOutRect.u16X = ldcInfo->ldcCfg.region[u32Idx].x;
        ldcChnAttr.stRegionAttr[u32Idx].stOutRect.u16Y = ldcInfo->ldcCfg.region[u32Idx].y;
        ldcChnAttr.stRegionAttr[u32Idx].stOutRect.u16Width  = ldcInfo->ldcCfg.region[u32Idx].width;
        ldcChnAttr.stRegionAttr[u32Idx].stOutRect.u16Height = ldcInfo->ldcCfg.region[u32Idx].height;
        if (ldcChnAttr.stRegionAttr[0].eRegionMode == MI_LDC_REGION_MAP2BIN)
        {
            ldcChnAttr.stRegionAttr[u32Idx].stRegionMapInfo.u32Grid = ldcInfo->ldcCfg.region[u32Idx].map2bin.grid;
            ldcChnAttr.stRegionAttr[u32Idx].stRegionMapInfo.u32XmapSize =
                _PTREE_MOD_LDC_GetFileSize(ldcInfo->ldcCfg.region[u32Idx].map2bin.mapX);
            ldcChnAttr.stRegionAttr[u32Idx].stRegionMapInfo.u32YmapSize =
                _PTREE_MOD_LDC_GetFileSize(ldcInfo->ldcCfg.region[u32Idx].map2bin.mapY);
            if (ldcChnAttr.stRegionAttr[u32Idx].stRegionMapInfo.u32XmapSize == (unsigned int)-1
                || ldcChnAttr.stRegionAttr[u32Idx].stRegionMapInfo.u32YmapSize == (unsigned int)-1)
            {
                PTREE_ERR("region[%u] Xmap OR Ymap file err", u32Idx);
                goto __exit_ldc_err;
            }
            MMA_ALLOC_BUF(&g_phyMap2BinX[u32Idx], &g_pVirMap2BinX[u32Idx],
                          ldcChnAttr.stRegionAttr[u32Idx].stRegionMapInfo.u32XmapSize, __exit_ldc_err);
            MMA_ALLOC_BUF(&g_phyMap2BinY[u32Idx], &g_pVirMap2BinY[u32Idx],
                          ldcChnAttr.stRegionAttr[u32Idx].stRegionMapInfo.u32YmapSize, __exit_ldc_err);
            // PTREE_ERR("Xmap bufsize=%d.Ymap
            // bufsize=%d",ldcChnAttr.stRegionAttr[u32Idx].stRegionMapInfo.u32XmapSize,ldcChnAttr.stRegionAttr[u32Idx].stRegionMapInfo.u32YmapSize);
            ldcChnAttr.stRegionAttr[u32Idx].stRegionMapInfo.pXmapAddr = g_pVirMap2BinX[u32Idx];
            ldcChnAttr.stRegionAttr[u32Idx].stRegionMapInfo.pYmapAddr = g_pVirMap2BinY[u32Idx];
        }
        else
        {
            ldcChnAttr.stRegionAttr[u32Idx].stRegionPara.eCropMode =
                (MI_LDC_RegionCropMode_e)ldcInfo->ldcCfg.region[u32Idx].para.cropMode;
            ldcChnAttr.stRegionAttr[u32Idx].stRegionPara.s32DistortionRatio =
                ldcInfo->ldcCfg.region[u32Idx].para.distortionRatio;
            ldcChnAttr.stRegionAttr[u32Idx].stRegionPara.s32FocalRatio = ldcInfo->ldcCfg.region[u32Idx].para.focalRatio;
            ldcChnAttr.stRegionAttr[u32Idx].stRegionPara.s32InRadius   = ldcInfo->ldcCfg.region[u32Idx].para.inRadius;
            ldcChnAttr.stRegionAttr[u32Idx].stRegionPara.s32OutRadius  = ldcInfo->ldcCfg.region[u32Idx].para.outRadius;
            ldcChnAttr.stRegionAttr[u32Idx].stRegionPara.s32OutRotate  = ldcInfo->ldcCfg.region[u32Idx].para.outRot;
            ldcChnAttr.stRegionAttr[u32Idx].stRegionPara.s32Pan        = ldcInfo->ldcCfg.region[u32Idx].para.pan;
            ldcChnAttr.stRegionAttr[u32Idx].stRegionPara.s32Tilt       = ldcInfo->ldcCfg.region[u32Idx].para.tilt;
            ldcChnAttr.stRegionAttr[u32Idx].stRegionPara.s32ZoomH      = ldcInfo->ldcCfg.region[u32Idx].para.zoomH;
            ldcChnAttr.stRegionAttr[u32Idx].stRegionPara.s32ZoomV      = ldcInfo->ldcCfg.region[u32Idx].para.zoomV;
        }
    }
    MI_LDC_SetChnLDCAttr((MI_LDC_DEV)ldcInfo->base.base.devId, (MI_LDC_CHN)ldcInfo->base.base.chnId, &ldcChnAttr);
    return MI_SUCCESS;
__exit_ldc_err:
    PTREE_ERR("__exit_ldc_err");
    for (u32Idx = 0; u32Idx < ldcChnAttr.u32RegionNum && u32Idx < LDC_MAX_REGION_NUM; u32Idx++)
    {
        MMA_FREE_BUF(g_phyMap2BinX[u32Idx], g_pVirMap2BinX[u32Idx],
                     ldcChnAttr.stRegionAttr[u32Idx].stRegionMapInfo.u32XmapSize);
        MMA_FREE_BUF(g_phyMap2BinY[u32Idx], g_pVirMap2BinY[u32Idx],
                     ldcChnAttr.stRegionAttr[u32Idx].stRegionMapInfo.u32YmapSize);
    }
    return -1;
}
static int _PTREE_MOD_LDC_StitchModeInit(PTREE_SUR_LDC_Info_t *ldcInfo, void *calibAddr)
{
    MI_LDC_ChnStitchAttr_t stitchChnAttr;
    memset(&stitchChnAttr, 0, sizeof(MI_LDC_ChnStitchAttr_t));
    stitchChnAttr.eProjType                = (MI_LDC_ProjectionMode_e)ldcInfo->stitchCfg.projType;
    stitchChnAttr.s32Distance              = ldcInfo->stitchCfg.distance;
    stitchChnAttr.stCalCfg.u32CalibCfgSize = ldcInfo->calib.len;
    stitchChnAttr.stCalCfg.pCalibCfgAddr   = calibAddr;

    MI_LDC_SetChnStitchAttr((MI_LDC_DEV)ldcInfo->base.base.devId, (MI_LDC_CHN)ldcInfo->base.base.chnId, &stitchChnAttr);
    return MI_SUCCESS;
}
static int _PTREE_MOD_LDC_DisModeInit(PTREE_SUR_LDC_Info_t *ldcInfo)
{
    int                 i = 0;
    MI_LDC_ChnDISAttr_t stChnDISAttr;
    memset(&stChnDISAttr, 0, sizeof(MI_LDC_ChnDISAttr_t));
    stChnDISAttr.eMode = (MI_LDC_DISMode_e)ldcInfo->disCfg.disMode;
    if (MI_LDC_DIS_GME_6DOF == stChnDISAttr.eMode || MI_LDC_DIS_GME_8DOF == stChnDISAttr.eMode)
    {
        stChnDISAttr.eSceneType   = (MI_LDC_DISSceneType_e)ldcInfo->disCfg.sceneType;
        stChnDISAttr.eMotionLevel = (MI_LDC_DISMotionLevel_e)ldcInfo->disCfg.motionLevel;
        stChnDISAttr.u8CropRatio  = ldcInfo->disCfg.cropRatio;
    }
    else if (MI_LDC_DIS_GYRO == stChnDISAttr.eMode)
    {
        for (i = 0; i < LDC_MAXTRIX_NUM; i++)
        {
            stChnDISAttr.as32RotationMatrix[i] = ldcInfo->disCfg.rotationMatrix[i];
        }
        stChnDISAttr.u32UserSliceNum = ldcInfo->disCfg.userSliceNum;
        stChnDISAttr.u32FocalLengthX = ldcInfo->disCfg.focalLengthX;
        stChnDISAttr.u32FocalLengthY = ldcInfo->disCfg.focalLengthY;
    }
    MI_LDC_SetChnDISAttr((MI_LDC_DEV)ldcInfo->base.base.devId, (MI_LDC_CHN)ldcInfo->base.base.chnId, &stChnDISAttr);
    return MI_SUCCESS;
}

static int _PTREE_MOD_LDC_Init(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_LDC_Info_t *ldcInfo   = CONTAINER_OF(sysMod->base.info, PTREE_SUR_LDC_Info_t, base.base);
    void *                calibAddr = NULL;
    MI_LDC_ChnAttr_t      ldcChnAttr;

    if (!g_ldcCreateDev[ldcInfo->base.base.devId])
    {
        MI_LDC_DevAttr_t ldcDevAttr;
        memset(&ldcDevAttr, 0, sizeof(MI_LDC_DevAttr_t));
        if (MI_SUCCESS != MI_LDC_CreateDevice(ldcInfo->base.base.devId, &ldcDevAttr))
        {
            PTREE_ERR("MI_LDC_CreateDevice failed");
            return SSOS_DEF_FAIL;
        }
    }
    ++g_ldcCreateDev[ldcInfo->base.base.devId];

    memset(&ldcChnAttr, 0, sizeof(MI_LDC_ChnAttr_t));

    ldcChnAttr.eWorkMode = (MI_LDC_WorkMode_e)ldcInfo->workMode;
    if (sysMod->base.info->inCnt > 0)
    {
        PTREE_SUR_SYS_InInfo_t *sysInInfo = CONTAINER_OF(sysMod->base.arrModIn[0]->info, PTREE_SUR_SYS_InInfo_t, base);
        ldcChnAttr.eInputBindType         = sysInInfo->bindType;
    }

    if (MI_SUCCESS
        != MI_LDC_CreateChannel((MI_LDC_DEV)ldcInfo->base.base.devId, (MI_LDC_CHN)ldcInfo->base.base.chnId,
                                &ldcChnAttr))
    {
        PTREE_ERR("MI_LDC_CreateChannel error");
        return SSOS_DEF_FAIL;
    }

    if (strlen(ldcInfo->calib.path) != 0)
    {
        ldcInfo->calib.len = _PTREE_MOD_LDC_GetFileSize(ldcInfo->calib.path);
        if (ldcInfo->calib.len == (unsigned int)-1)
        {
            PTREE_ERR("Open file error!");
            return SSOS_DEF_FAIL;
        }
        calibAddr = SSOS_MEM_AlignAlloc(ldcInfo->calib.len, 6); // 2^6 byte align
        if (calibAddr == NULL)
        {
            PTREE_ERR("No Memory to alloc, ERR!");
            return SSOS_DEF_FAIL;
        }
        _PTREE_MOD_LDC_ReadFileToBuf(ldcInfo->calib.path, calibAddr, ldcInfo->calib.len);
    }

    switch (ldcChnAttr.eWorkMode)
    {
        case MI_LDC_WORKMODE_LDC:
            _PTREE_MOD_LDC_LdcModeInit(ldcInfo, calibAddr);
            break;
        case MI_LDC_WORKMODE_STITCH:
            _PTREE_MOD_LDC_StitchModeInit(ldcInfo, calibAddr);
            break;
        case MI_LDC_WORKMODE_DIS:
            _PTREE_MOD_LDC_DisModeInit(ldcInfo);
            break;
        case MI_LDC_WORKMODE_DIS_LDC:
            _PTREE_MOD_LDC_LdcModeInit(ldcInfo, calibAddr);
            _PTREE_MOD_LDC_DisModeInit(ldcInfo);
            break;
        default:
            PTREE_ERR("Work Mode ERR!");
            break;
    }

    SSOS_MEM_AlignFree(calibAddr);
    calibAddr = NULL;
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_LDC_Deinit(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_LDC_Info_t *ldcInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_LDC_Info_t, base.base);

    MI_LDC_DestroyChannel((MI_LDC_DEV)ldcInfo->base.base.devId, (MI_LDC_CHN)ldcInfo->base.base.chnId);

    --g_ldcCreateDev[ldcInfo->base.base.devId];
    if (!g_ldcCreateDev[ldcInfo->base.base.devId])
    {
        MI_LDC_DestroyDevice(ldcInfo->base.base.devId);
    }

    return SSOS_DEF_OK;
}
static int _PTREE_MOD_LDC_Start(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_LDC_Info_t *ldcInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_LDC_Info_t, base.base);

    if (MI_SUCCESS != MI_LDC_StartChannel(ldcInfo->base.base.devId, ldcInfo->base.base.chnId))
    {
        PTREE_ERR("MI_LDC_StartChannel failed");
        return SSOS_DEF_FAIL;
    }

    return SSOS_DEF_OK;
}
static int _PTREE_MOD_LDC_Stop(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_LDC_Info_t *ldcInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_LDC_Info_t, base.base);

    MI_LDC_StopChannel(ldcInfo->base.base.devId, ldcInfo->base.base.chnId);

    return SSOS_DEF_OK;
}
static PTREE_MOD_InObj_t *_PTREE_MOD_LDC_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_SYS_Obj_t *sysMod = CONTAINER_OF(mod, PTREE_MOD_SYS_Obj_t, base);
    (void)loopId;
    return _PTREE_MOD_LDC_InNew(sysMod, loopId);
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_LDC_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_SYS_Obj_t *sysMod = CONTAINER_OF(mod, PTREE_MOD_SYS_Obj_t, base);
    (void)loopId;
    return _PTREE_MOD_LDC_OutNew(sysMod, loopId);
}
static void _PTREE_MOD_LDC_Free(PTREE_MOD_SYS_Obj_t *sysMod)
{
    SSOS_MEM_Free(CONTAINER_OF(sysMod, PTREE_MOD_LDC_Obj_t, base));
}

static int _PTREE_MOD_LDC_InStart(PTREE_MOD_SYS_InObj_t *sysModIn)
{
    PTREE_SUR_LDC_InInfo_t *ldcInInfo = CONTAINER_OF(sysModIn->base.info, PTREE_SUR_LDC_InInfo_t, base.base);
    PTREE_SUR_Info_t *      info      = sysModIn->base.thisMod->info;
    MI_LDC_InputPortAttr_t  ldcInputAttr;

    memset(&ldcInputAttr, 0, sizeof(MI_LDC_InputPortAttr_t));

    if (!ldcInInfo->width || !ldcInInfo->height)
    {
        return SSOS_DEF_OK;
    }
    ldcInputAttr.u16Width  = ldcInInfo->width;
    ldcInputAttr.u16Height = ldcInInfo->height;

    if (MI_SUCCESS != MI_LDC_SetInputPortAttr((MI_LDC_DEV)info->devId, (MI_LDC_CHN)info->chnId, &ldcInputAttr))
    {
        PTREE_ERR("MI_LDC_SetInputPortAttr error");
        return SSOS_DEF_FAIL;
    }
    return SSOS_DEF_OK;
}
static void _PTREE_MOD_LDC_InFree(PTREE_MOD_SYS_InObj_t *sysModIn)
{
    SSOS_MEM_Free(CONTAINER_OF(sysModIn, PTREE_MOD_LDC_InObj_t, base));
}
static PTREE_MOD_InObj_t *_PTREE_MOD_LDC_InNew(PTREE_MOD_SYS_Obj_t *sysMod, unsigned int loopId)
{
    PTREE_MOD_LDC_InObj_t *ldcModIn = NULL;

    ldcModIn = SSOS_MEM_Alloc(sizeof(PTREE_MOD_LDC_InObj_t));
    if (!ldcModIn)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(ldcModIn, 0, sizeof(PTREE_MOD_LDC_InObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_InObjInit(&ldcModIn->base, &G_PTREE_MOD_LDC_SYS_IN_OPS, sysMod, loopId))
    {
        SSOS_MEM_Free(ldcModIn);
        return NULL;
    }
    PTREE_MOD_SYS_InObjRegister(&ldcModIn->base, &G_PTREE_MOD_LDC_SYS_IN_HOOK);
    return &ldcModIn->base.base;
}

static int _PTREE_MOD_LDC_OutStart(PTREE_MOD_SYS_OutObj_t *sysModOut)
{
    PTREE_SUR_LDC_OutInfo_t *ldcOutInfo = CONTAINER_OF(sysModOut->base.info, PTREE_SUR_LDC_OutInfo_t, base.base);
    PTREE_SUR_Info_t *       info       = sysModOut->base.thisMod->info;
    MI_LDC_OutputPortAttr_t  ldcOutputAttr;

    memset(&ldcOutputAttr, 0, sizeof(MI_LDC_OutputPortAttr_t));

    ldcOutputAttr.u16Width  = ldcOutInfo->width;
    ldcOutputAttr.u16Height = ldcOutInfo->height;
    ldcOutputAttr.ePixelFmt = (MI_SYS_PixelFormat_e)ldcOutInfo->videoFormat;
    if (MI_SUCCESS != MI_LDC_SetOutputPortAttr((MI_LDC_DEV)info->devId, (MI_LDC_CHN)info->chnId, &ldcOutputAttr))
    {
        PTREE_ERR("MI_LDC_SetOutputPortAttr error");
        return SSOS_DEF_FAIL;
    }

    return SSOS_DEF_OK;
}
static void _PTREE_MOD_LDC_OutGetPacketInfo(PTREE_MOD_SYS_OutObj_t *sysModOut, PTREE_PACKET_RAW_RawInfo_t *rawInfo)
{
    PTREE_SUR_LDC_OutInfo_t *ldcOutInfo = CONTAINER_OF(sysModOut->base.info, PTREE_SUR_LDC_OutInfo_t, base.base);
    rawInfo->fmt                        = PTREE_MOD_SYS_SysFmtToPtreeFmt(ldcOutInfo->videoFormat);
    rawInfo->width                      = ldcOutInfo->width;
    rawInfo->height                     = ldcOutInfo->height;
}
static void _PTREE_MOD_LDC_OutFree(PTREE_MOD_SYS_OutObj_t *sysModOut)
{
    SSOS_MEM_Free(CONTAINER_OF(sysModOut, PTREE_MOD_LDC_OutObj_t, base));
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_LDC_OutNew(PTREE_MOD_SYS_Obj_t *sysMod, unsigned int loopId)
{
    PTREE_MOD_LDC_OutObj_t *ldcModOut = NULL;

    ldcModOut = SSOS_MEM_Alloc(sizeof(PTREE_MOD_LDC_OutObj_t));
    if (!ldcModOut)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(ldcModOut, 0, sizeof(PTREE_MOD_LDC_OutObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_OutObjInit(&ldcModOut->base, &G_PTREE_MOD_LDC_SYS_OUT_OPS, sysMod, loopId))
    {
        SSOS_MEM_Free(ldcModOut);
        return NULL;
    }
    PTREE_MOD_SYS_OutObjRegister(&ldcModOut->base, &G_PTREE_MOD_LDC_SYS_OUT_HOOK);
    return &ldcModOut->base.base;
}

PTREE_MOD_Obj_t *PTREE_MOD_LDC_New(PARENA_Tag_t *tag)
{
    PTREE_MOD_LDC_Obj_t *ldcMod = NULL;

    ldcMod = SSOS_MEM_Alloc(sizeof(PTREE_MOD_LDC_Obj_t));
    if (!ldcMod)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(ldcMod, 0, sizeof(PTREE_MOD_LDC_Obj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_ObjInit(&ldcMod->base, &G_PTREE_MOD_LDC_SYS_OPS, tag, E_MI_MODULE_ID_LDC))
    {
        goto ERR_MOD_SYS_INIT;
    }

    if (ldcMod->base.base.info->devId >= PTREE_MOD_LDC_DEV_NUM)
    {
        PTREE_ERR("Dev id %d is not support, max number is %d", ldcMod->base.base.info->devId, PTREE_MOD_LDC_DEV_NUM);
        goto ERR_DEV_OUT_OF_RANGE;
    }

    PTREE_MOD_SYS_ObjRegister(&ldcMod->base, &G_PTREE_MOD_LDC_SYS_HOOK);
    return &ldcMod->base.base;

ERR_DEV_OUT_OF_RANGE:
    PTREE_MOD_ObjDel(&ldcMod->base.base);
ERR_MOD_SYS_INIT:
    SSOS_MEM_Free(ldcMod);
ERR_MEM_ALLOC:
    return NULL;
}

PTREE_MAKER_MOD_INIT(LDC, PTREE_MOD_LDC_New);
