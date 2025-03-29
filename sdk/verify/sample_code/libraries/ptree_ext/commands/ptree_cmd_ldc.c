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

#include "mi_common.h"
#include "mi_common_datatype.h"
#include "ssos_io.h"
#include "ptree_log.h"
#include "ptree_mod.h"
#include "ptree_cmd.h"
#include "ssos_mem.h"
#include "ptree_sur_ldc.h"
#include "ssos_list.h"
#include "mi_ldc.h"
#include "mi_ldc_datatype.h"
#include "ptree_maker.h"

static int _PTREE_CMD_LDC_AlginDown(int originalValue, int alignment)
{
    return originalValue - (originalValue % alignment);
}
static inline int _PTREE_CMD_LDC_GetFileSize(const char *path)
{
    SSOS_IO_File_t fd        = NULL;
    int            totalSize = 0;

    fd = SSOS_IO_FileOpen(path, SSOS_IO_O_RDONLY, 0644);
    if (!fd)
    {
        PTREE_ERR("failed to open:%s ", path);
        return -1;
    }

    totalSize = SSOS_IO_FileSeek(fd, 0, SSOS_IO_SEEK_END);
    SSOS_IO_FileClose(fd);
    return totalSize;
}
static inline int _PTREE_CMD_LDC_ReadFileToBuf(const char *path, void *pBuf, unsigned int length)
{
    int            ret = -1, readSize = 0, totalSize = 0, buflen = (int)length;
    SSOS_IO_File_t fd = NULL;

    if (!pBuf)
    {
        return -1;
    }

    fd = SSOS_IO_FileOpen(path, SSOS_IO_O_RDONLY, 0644);
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
    return ret;
}

int PTREE_CMD_LDC_StrCmdSetBgColor(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int                   dev  = pstModObj->info->devId;
    int                   chn  = pstModObj->info->chnId;
    PTREE_SUR_LDC_Info_t *info = NULL;
    char *                path = NULL;
    void *                addr = NULL;
    unsigned int          len;
    MI_U32                ret;
    MI_LDC_ChnLDCAttr_t   stChnLDCAttr;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s", (const char *)pstCmd->cmdId, paraCnt,
              (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1]);

    memset(&stChnLDCAttr, 0x00, sizeof(MI_LDC_ChnLDCAttr_t));
    MI_LDC_GetChnLDCAttr(dev, chn, &stChnLDCAttr);
    memset(&stChnLDCAttr.bBgColor, 0x00, sizeof(MI_BOOL));
    memset(&stChnLDCAttr.u32BgColor, 0x00, sizeof(MI_U32));

    info = CONTAINER_OF(pstModObj->info, PTREE_SUR_LDC_Info_t, base.base);
    path = info->calib.path;

    if (strlen(path) == 0)
    {
        PTREE_ERR("File path is NULL");
        return -1;
    }
    len = _PTREE_CMD_LDC_GetFileSize(path);
    if (len <= 0)
    {
        PTREE_ERR("Get Size is negative num!");
        return -1;
    }
    addr = SSOS_MEM_AlignAlloc(len, 6); // 2^6 byte align
    if (addr == NULL)
    {
        PTREE_ERR("No Memory to alloc, ERR!");
        return -1;
    }
    _PTREE_CMD_LDC_ReadFileToBuf(path, addr, len);

    stChnLDCAttr.bBgColor                        = (MI_BOOL)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    stChnLDCAttr.u32BgColor                      = (MI_U32)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    stChnLDCAttr.stCalibInfo.pCalibPolyBinAddr   = addr;
    stChnLDCAttr.stCalibInfo.u32CalibPolyBinSize = len;
    ret                                          = MI_LDC_SetChnLDCAttr(dev, chn, &stChnLDCAttr);
    if (MI_SUCCESS != ret)
    {
        PTREE_ERR("Set LDC Chn Attr Fail!");
        SSOS_MEM_AlignFree(addr);
        return ret;
    }
    SSOS_MEM_AlignFree(addr);
    return 0;
}

int PTREE_CMD_LDC_StrCmdSetMountMode(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int                   dev  = pstModObj->info->devId;
    int                   chn  = pstModObj->info->chnId;
    PTREE_SUR_LDC_Info_t *info = NULL;
    char *                path = NULL;
    void *                addr = NULL;
    unsigned int          len;
    MI_U32                ret;
    MI_LDC_ChnLDCAttr_t   stChnLDCAttr;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s", (const char *)pstCmd->cmdId, paraCnt, (const char *)pstCmd->cmdPara[0]);

    memset(&stChnLDCAttr, 0x00, sizeof(MI_LDC_ChnLDCAttr_t));
    MI_LDC_GetChnLDCAttr(dev, chn, &stChnLDCAttr);
    memset(&stChnLDCAttr.eMountMode, 0x00, sizeof(MI_LDC_MountMode_e));

    info = CONTAINER_OF(pstModObj->info, PTREE_SUR_LDC_Info_t, base.base);
    path = info->calib.path;

    if (strlen(path) == 0)
    {
        PTREE_ERR("File path is NULL");
        return -1;
    }
    len = _PTREE_CMD_LDC_GetFileSize(path);
    if (len <= 0)
    {
        PTREE_ERR("Get Size is negative num!");
        return -1;
    }
    addr = SSOS_MEM_AlignAlloc(len, 6); // 2^6 byte align
    if (addr == NULL)
    {
        PTREE_ERR("No Memory to alloc, ERR!");
        return -1;
    }
    _PTREE_CMD_LDC_ReadFileToBuf(path, addr, len);

    stChnLDCAttr.eMountMode                      = (MI_LDC_MountMode_e)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    stChnLDCAttr.stCalibInfo.pCalibPolyBinAddr   = addr;
    stChnLDCAttr.stCalibInfo.u32CalibPolyBinSize = len;
    ret                                          = MI_LDC_SetChnLDCAttr(dev, chn, &stChnLDCAttr);
    if (MI_SUCCESS != ret)
    {
        PTREE_ERR("Set LDC Chn Attr Fail!");
        SSOS_MEM_AlignFree(addr);
        return ret;
    }
    SSOS_MEM_AlignFree(addr);
    return 0;
}

int PTREE_CMD_LDC_StrCmdSetRegionParam(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int                   dev  = pstModObj->info->devId;
    int                   chn  = pstModObj->info->chnId;
    PTREE_SUR_LDC_Info_t *info = NULL;
    char *                path = NULL;
    void *                addr = NULL;
    unsigned int          len;
    MI_U32                ret;
    MI_LDC_ChnLDCAttr_t   stChnLDCAttr;
    int                   i = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG(
        "Cmd Str %s, ParaCnt %d, p0->%s, p1->%s, p2->%s, p3->%s, p4->%s, p5->%s, p6->%s, p7->%s, p8->%s, p9->%s, "
        "p10->%s, p11->%s, p12->%s",
        (const char *)pstCmd->cmdId, paraCnt, (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1],
        (const char *)pstCmd->cmdPara[2], (const char *)pstCmd->cmdPara[3], (const char *)pstCmd->cmdPara[4],
        (const char *)pstCmd->cmdPara[5], (const char *)pstCmd->cmdPara[6], (const char *)pstCmd->cmdPara[7],
        (const char *)pstCmd->cmdPara[8], (const char *)pstCmd->cmdPara[9], (const char *)pstCmd->cmdPara[10],
        (const char *)pstCmd->cmdPara[11], (const char *)pstCmd->cmdPara[12]);

    memset(&stChnLDCAttr, 0x00, sizeof(MI_LDC_ChnLDCAttr_t));
    MI_LDC_GetChnLDCAttr(dev, chn, &stChnLDCAttr);

    info = CONTAINER_OF(pstModObj->info, PTREE_SUR_LDC_Info_t, base.base);
    path = info->calib.path;

    if (strlen(path) == 0)
    {
        PTREE_ERR("File path is NULL");
        return -1;
    }
    len = _PTREE_CMD_LDC_GetFileSize(path);
    if (len <= 0)
    {
        PTREE_ERR("Get Size is negative num!");
        return -1;
    }
    addr = SSOS_MEM_AlignAlloc(len, 6); // 2^6 byte align
    if (addr == NULL)
    {
        PTREE_ERR("No Memory to alloc, ERR!");
        return -1;
    }
    _PTREE_CMD_LDC_ReadFileToBuf(path, addr, len);

    if (i < 0 || i > LDC_MAX_REGION_NUM - 1)
    {
        PTREE_ERR("RegionID is invalid");
        SSOS_MEM_AlignFree(addr);
        return -1;
    }
    stChnLDCAttr.stRegionAttr[i].eRegionMode = (MI_LDC_RegionMode_e)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    stChnLDCAttr.stRegionAttr[i].stRegionPara.eCropMode =
        (MI_LDC_RegionCropMode_e)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[2]);
    stChnLDCAttr.stRegionAttr[i].stRegionPara.s32Pan        = (MI_U32)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[3]);
    stChnLDCAttr.stRegionAttr[i].stRegionPara.s32Tilt       = (MI_U32)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[4]);
    stChnLDCAttr.stRegionAttr[i].stRegionPara.s32ZoomV      = (MI_U32)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[5]);
    stChnLDCAttr.stRegionAttr[i].stRegionPara.s32ZoomH      = (MI_U32)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[6]);
    stChnLDCAttr.stRegionAttr[i].stRegionPara.s32InRadius   = (MI_U32)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[7]);
    stChnLDCAttr.stRegionAttr[i].stRegionPara.s32OutRadius  = (MI_U32)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[8]);
    stChnLDCAttr.stRegionAttr[i].stRegionPara.s32FocalRatio = (MI_U32)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[9]);
    stChnLDCAttr.stRegionAttr[i].stRegionPara.s32DistortionRatio =
        (MI_U32)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[10]);
    stChnLDCAttr.stRegionAttr[i].stRegionPara.s32OutRotate = (MI_U32)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[11]);
    stChnLDCAttr.stRegionAttr[i].stRegionPara.s32Rotate    = (MI_U32)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[12]);
    stChnLDCAttr.stCalibInfo.pCalibPolyBinAddr             = addr;
    stChnLDCAttr.stCalibInfo.u32CalibPolyBinSize           = len;
    ret                                                    = MI_LDC_SetChnLDCAttr(dev, chn, &stChnLDCAttr);
    if (MI_SUCCESS != ret)
    {
        PTREE_ERR("Set LDC Chn Attr Fail!");
        SSOS_MEM_AlignFree(addr);
        return ret;
    }
    SSOS_MEM_AlignFree(addr);

    return 0;
}

int PTREE_CMD_LDC_StrCmdSetMultiRegion(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int                   dev  = pstModObj->info->devId;
    int                   chn  = pstModObj->info->chnId;
    PTREE_SUR_LDC_Info_t *info = NULL;
    char *                path = NULL;
    void *                addr = NULL;
    unsigned int          len;
    MI_U32                ret;
    MI_LDC_ChnLDCAttr_t   stChnLDCAttr;
    int                   width, height;
    int                   i;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s", (const char *)pstCmd->cmdId, paraCnt, (const char *)pstCmd->cmdPara[0]);

    memset(&stChnLDCAttr, 0x00, sizeof(MI_LDC_ChnLDCAttr_t));
    MI_LDC_GetChnLDCAttr(dev, chn, &stChnLDCAttr);

    info = CONTAINER_OF(pstModObj->info, PTREE_SUR_LDC_Info_t, base.base);
    path = info->calib.path;

    if (strlen(path) == 0)
    {
        PTREE_ERR("File path is NULL");
        return -1;
    }
    len = _PTREE_CMD_LDC_GetFileSize(path);
    if (len <= 0)
    {
        PTREE_ERR("Get Size is negative num!");
        return -1;
    }
    addr = SSOS_MEM_AlignAlloc(len, 6); // 2^6 byte align
    if (addr == NULL)
    {
        PTREE_ERR("No Memory to alloc, ERR!");
        return -1;
    }
    _PTREE_CMD_LDC_ReadFileToBuf(path, addr, len);

    stChnLDCAttr.u32RegionNum = (MI_U32)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    width                     = (MI_U32)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    height                    = (MI_U32)SSOS_IO_Atoi((const char *)pstCmd->cmdPara[2]);
    for (i = 0; i < (int)stChnLDCAttr.u32RegionNum; i++)
    {
        stChnLDCAttr.stRegionAttr[i].eRegionMode                     = MI_LDC_REGION_NO_TRANSFORMATION;
        stChnLDCAttr.stRegionAttr[i].stOutRect.u16X                  = (i % 3) * width / 3 + 16;
        stChnLDCAttr.stRegionAttr[i].stOutRect.u16Y                  = (i / 3) * height / 3 + 2;
        stChnLDCAttr.stRegionAttr[i].stOutRect.u16Width              = _PTREE_CMD_LDC_AlginDown(width / 3, 16) - 32;
        stChnLDCAttr.stRegionAttr[i].stOutRect.u16Height             = _PTREE_CMD_LDC_AlginDown(height / 3, 2) - 16;
        stChnLDCAttr.stRegionAttr[i].stRegionPara.eCropMode          = MI_LDC_REGION_CROP_NONE;
        stChnLDCAttr.stRegionAttr[i].stRegionPara.s32Pan             = 0;
        stChnLDCAttr.stRegionAttr[i].stRegionPara.s32Tilt            = 10;
        stChnLDCAttr.stRegionAttr[i].stRegionPara.s32ZoomV           = 550;
        stChnLDCAttr.stRegionAttr[i].stRegionPara.s32ZoomH           = 360;
        stChnLDCAttr.stRegionAttr[i].stRegionPara.s32InRadius        = 0;
        stChnLDCAttr.stRegionAttr[i].stRegionPara.s32OutRadius       = 550;
        stChnLDCAttr.stRegionAttr[i].stRegionPara.s32FocalRatio      = 0;
        stChnLDCAttr.stRegionAttr[i].stRegionPara.s32DistortionRatio = 0;
        stChnLDCAttr.stRegionAttr[i].stRegionPara.s32OutRotate       = 0;
        stChnLDCAttr.stRegionAttr[i].stRegionPara.s32Rotate          = 0;

        if (stChnLDCAttr.stRegionAttr[i].eRegionMode == MI_LDC_REGION_MAP2BIN)
        {
            PTREE_ERR("The current mode is MAP2BIN, please choose another mode: [1/2/3/5]");
            SSOS_MEM_AlignFree(addr);
            return -1;
        }
    }
    stChnLDCAttr.stCalibInfo.pCalibPolyBinAddr   = addr;
    stChnLDCAttr.stCalibInfo.u32CalibPolyBinSize = len;
    ret                                          = MI_LDC_SetChnLDCAttr(dev, chn, &stChnLDCAttr);
    if (MI_SUCCESS != ret)
    {
        PTREE_ERR("Set LDC Chn Attr Fail! ");
        SSOS_MEM_AlignFree(addr);
        return ret;
    }
    SSOS_MEM_AlignFree(addr);

    return 0;
}

PTREE_MAKER_CMD_INIT(LDC, PTREE_CMD_StrCompare, PTREE_CMD_StrHashVal,
                     {(unsigned long)"ldc_set_bgcolor", PTREE_CMD_LDC_StrCmdSetBgColor, 2},
                     {(unsigned long)"ldc_set_mountmode", PTREE_CMD_LDC_StrCmdSetMountMode, 1},
                     {(unsigned long)"ldc_set_regionparam", PTREE_CMD_LDC_StrCmdSetRegionParam, 13},
                     {(unsigned long)"ldc_set_multi_region", PTREE_CMD_LDC_StrCmdSetMultiRegion, 3})
