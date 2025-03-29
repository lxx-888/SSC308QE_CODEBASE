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
#include "ss_cmd_base.h"
#include "amigos_module_ldc.h"
#include "mi_ldc.h"

static int alignDown(int originalValue, int alignment)
{
    return originalValue - (originalValue % alignment);
}
static inline int GetFileSize(const char *path)
{
    FILE *fp        = NULL;
    int   totalSize = 0;

    fp = fopen(path, "rb");
    if (!fp)
    {
        AMIGOS_ERR("failed to open:%s \n", path);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    totalSize = ftell(fp);
    fclose(fp);
    return totalSize;
}
static inline int ReadFileToBuf(const char *path, void *pBuf, unsigned int Length)
{
    int   ret = -1, readSize = 0, totalSize = 0, buflen = (int)Length;
    FILE *fp = NULL;

    if (!pBuf)
    {
        return -1;
    }

    fp = fopen(path, "rb");
    if (!fp)
    {
        AMIGOS_ERR("failed to open:%s \n", path);
        goto __exit_err__;
    }

    fseek(fp, 0, SEEK_END);
    totalSize = ftell(fp);
    if (totalSize > buflen || totalSize < 0)
    {
        AMIGOS_ERR("%s file content is less need:%d %d\n", path, totalSize, buflen);
        goto __exit_err__;
    }

    fseek(fp, 0, SEEK_SET);

    readSize = fread(pBuf, 1, totalSize, fp);
    if (readSize != totalSize)
    {
        AMIGOS_ERR("failed to read buf:%s %p %d %d!\n", path, pBuf, readSize, buflen);
        goto __exit_err__;
    }

    fclose(fp);
    return 0;

__exit_err__:
    if (fp)
        fclose(fp);
    return ret;
}

static int LdcStitchSetDistance(vector<string> &in_strs)
{
    AmigosModuleLdc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleLdc, AmigosModuleBase);
    if (!pMyClass)
    {
        AMIGOS_ERR("Pointer Class is Null\n");
        return -1;
    }
    AmigosModuleLdc::LdcInfo                            StitchInfo;
    std::map<unsigned int, AmigosModuleLdc::LdcInInfo>  StitchInInfo;
    std::map<unsigned int, AmigosModuleLdc::LdcOutInfo> StitchOutInfo;

    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    pMyClass->GetInfo(StitchInfo, StitchInInfo, StitchOutInfo);
    int          dev = stInfo.devId;
    int          chn = stInfo.chnId;
    std::string  path;
    void        *addr;
    unsigned int len;
    path = StitchInfo.CalibInfo.path;

    MI_LDC_ChnStitchAttr_t stChnStitchAttr;
    memset(&stChnStitchAttr, 0x00, sizeof(MI_LDC_ChnStitchAttr_t));

    MI_LDC_GetChnStitchAttr(dev, chn, &stChnStitchAttr);
    memset(&stChnStitchAttr.s32Distance, 0x00, sizeof(MI_S32));

    if (path.empty())
    {
        AMIGOS_ERR("File path is NULL");
        return -1;
    }

    // use callib
    len = GetFileSize(path.c_str());
    if (len <= 0)
    {
        AMIGOS_ERR("Get Size is negative num!");
        return -1;
    }
    addr = (void *)malloc(len);
    if (addr == NULL)
    {
        AMIGOS_ERR("No Memory to alloc, ERR!\n");
        return -1;
    }
    ReadFileToBuf(path.c_str(), addr, len);

    stChnStitchAttr.s32Distance              = (MI_S32)ss_cmd_atoi(in_strs[1].c_str());
    stChnStitchAttr.stCalCfg.pCalibCfgAddr   = addr;
    stChnStitchAttr.stCalCfg.u32CalibCfgSize = len;
    MI_S32 s32Ret                            = MI_SUCCESS;
    s32Ret                                   = MI_LDC_SetChnStitchAttr(dev, chn, &stChnStitchAttr);
    if (MI_SUCCESS != s32Ret)
    {
        AMIGOS_ERR("Set Stitch Chn Attr Fail, dev: %d, chn: %d, distance: %d, err: 0x%x\n", dev, chn, stChnStitchAttr.s32Distance, s32Ret);
        free(addr);
        return s32Ret;
    }
    free(addr);
    return 0;
}

static int LdcStitchSetProjType(vector<string> &in_strs)
{
    AmigosModuleLdc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleLdc, AmigosModuleBase);
    if (!pMyClass)
    {
        AMIGOS_ERR("Pointer Class is Null\n");
        return -1;
    }
    AmigosModuleLdc::LdcInfo                            StitchInfo;
    std::map<unsigned int, AmigosModuleLdc::LdcInInfo>  StitchInInfo;
    std::map<unsigned int, AmigosModuleLdc::LdcOutInfo> StitchOutInfo;

    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    pMyClass->GetInfo(StitchInfo, StitchInInfo, StitchOutInfo);
    int          dev = stInfo.devId;
    int          chn = stInfo.chnId;
    std::string  path;
    void        *addr;
    unsigned int len;
    path = StitchInfo.CalibInfo.path;

    MI_LDC_ChnStitchAttr_t stChnStitchAttr;
    memset(&stChnStitchAttr, 0x00, sizeof(MI_LDC_ChnStitchAttr_t));

    MI_LDC_GetChnStitchAttr(dev, chn, &stChnStitchAttr);
    memset(&stChnStitchAttr.eProjType, 0x00, sizeof(MI_S32));

    if (path.empty())
    {
        AMIGOS_ERR("File path is NULL");
        return -1;
    }

    // use callib
    len = GetFileSize(path.c_str());
    if (len <= 0)
    {
        AMIGOS_ERR("Get Size is negative num!");
        return -1;
    }
    addr = (void *)malloc(len);
    if (addr == NULL)
    {
        AMIGOS_ERR("No Memory to alloc, ERR!\n");
        return -1;
    }
    ReadFileToBuf(path.c_str(), addr, len);

    stChnStitchAttr.eProjType                = (MI_LDC_ProjectionMode_e)ss_cmd_atoi(in_strs[1].c_str());
    stChnStitchAttr.stCalCfg.pCalibCfgAddr   = addr;
    stChnStitchAttr.stCalCfg.u32CalibCfgSize = len;
    MI_S32 s32Ret                            = MI_SUCCESS;
    s32Ret                                   = MI_LDC_SetChnStitchAttr(dev, chn, &stChnStitchAttr);
    if (MI_SUCCESS != s32Ret)
    {
        AMIGOS_ERR("Set Stitch Mode Attr Fail, dev: %d, chn: %d, err:0x%x\n", dev, chn, s32Ret);
        free(addr);
        return s32Ret;
    }
    free(addr);
    return 0;
}

static int LdcSetBgColor(vector<string> &in_strs)
{
    AmigosModuleLdc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleLdc, AmigosModuleBase);
    if (!pMyClass)
    {
        AMIGOS_ERR("Pointer Class is Null\n");
        return -1;
    }
    AmigosModuleLdc::LdcInfo                            BgColorInfo;
    std::map<unsigned int, AmigosModuleLdc::LdcInInfo>  BgColorInInfo;
    std::map<unsigned int, AmigosModuleLdc::LdcOutInfo> BgColorOutInfo;

    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    pMyClass->GetInfo(BgColorInfo, BgColorInInfo, BgColorOutInfo);
    int          dev = stInfo.devId;
    int          chn = stInfo.chnId;
    std::string  path;
    void        *addr;
    unsigned int len;
    path = BgColorInfo.CalibInfo.path;

    MI_LDC_ChnLDCAttr_t stChnLDCAttr;
    memset(&stChnLDCAttr, 0x00, sizeof(MI_LDC_ChnLDCAttr_t));

    MI_LDC_GetChnLDCAttr(dev, chn, &stChnLDCAttr);
    memset(&stChnLDCAttr.bBgColor, 0x00, sizeof(bool));
    memset(&stChnLDCAttr.u32BgColor, 0x00, sizeof(MI_U32));

    if (path.empty())
    {
        AMIGOS_ERR("File path is NULL");
        return -1;
    }

    // use callib
    len = GetFileSize(path.c_str());
    if (len <= 0)
    {
        AMIGOS_ERR("Get Size is negative num!");
        return -1;
    }
    addr = (void *)malloc(len);
    if (addr == NULL)
    {
        AMIGOS_ERR("No Memory to alloc, ERR!\n");
        return -1;
    }
    ReadFileToBuf(path.c_str(), addr, len);

    stChnLDCAttr.bBgColor                        = (MI_BOOL)ss_cmd_atoi(in_strs[1].c_str());
    stChnLDCAttr.u32BgColor                      = (MI_U32)ss_cmd_atoi(in_strs[2].c_str());
    stChnLDCAttr.stCalibInfo.pCalibPolyBinAddr   = addr;
    stChnLDCAttr.stCalibInfo.u32CalibPolyBinSize = len;
    MI_S32 s32Ret                                = MI_SUCCESS;
    s32Ret                                       = MI_LDC_SetChnLDCAttr(dev, chn, &stChnLDCAttr);
    if (MI_SUCCESS != s32Ret)
    {
        AMIGOS_ERR("Set LDC Chn Attr Fail, dev: %d, chn: %d, err:0x%x\n", dev, chn, s32Ret);
        free(addr);
        return s32Ret;
    }
    free(addr);

    return 0;
}

static int LdcSetMountMode(vector<string> &in_strs)
{
    AmigosModuleLdc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleLdc, AmigosModuleBase);
    if (!pMyClass)
    {
        AMIGOS_ERR("Pointer Class is Null\n");
        return -1;
    }
    AmigosModuleLdc::LdcInfo                            MountModeInfo;
    std::map<unsigned int, AmigosModuleLdc::LdcInInfo>  MountModeInInfo;
    std::map<unsigned int, AmigosModuleLdc::LdcOutInfo> MountModeOutInfo;

    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    pMyClass->GetInfo(MountModeInfo, MountModeInInfo, MountModeOutInfo);
    int          dev = stInfo.devId;
    int          chn = stInfo.chnId;
    std::string  path;
    void        *addr;
    unsigned int len;
    path = MountModeInfo.CalibInfo.path;

    MI_LDC_ChnLDCAttr_t stChnLDCAttr;
    memset(&stChnLDCAttr, 0x00, sizeof(MI_LDC_ChnLDCAttr_t));

    MI_LDC_GetChnLDCAttr(dev, chn, &stChnLDCAttr);
    memset(&stChnLDCAttr.eMountMode, 0x00, sizeof(MI_LDC_MountMode_e));

    if (path.empty())
    {
        AMIGOS_ERR("File path is NULL");
        return -1;
    }

    // use callib
    len = GetFileSize(path.c_str());
    if (len <= 0)
    {
        AMIGOS_ERR("Get Size is negative num!");
        return -1;
    }
    addr = (void *)malloc(len);
    if (addr == NULL)
    {
        AMIGOS_ERR("No Memory to alloc, ERR!\n");
        return -1;
    }
    ReadFileToBuf(path.c_str(), addr, len);

    stChnLDCAttr.eMountMode                      = (MI_LDC_MountMode_e)ss_cmd_atoi(in_strs[1].c_str());
    stChnLDCAttr.stCalibInfo.pCalibPolyBinAddr   = addr;
    stChnLDCAttr.stCalibInfo.u32CalibPolyBinSize = len;
    MI_S32 s32Ret                                = MI_SUCCESS;
    s32Ret                                       = MI_LDC_SetChnLDCAttr(dev, chn, &stChnLDCAttr);
    if (MI_SUCCESS != s32Ret)
    {
        AMIGOS_ERR("Set LDC Chn Attr Fail, dev: %d, chn: %d, err:0x%x\n", dev, chn, s32Ret);
        free(addr);
        return s32Ret;
    }
    free(addr);

    return 0;
}

static int LdcSetRegionParam(vector<string> &in_strs)
{
    AmigosModuleLdc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleLdc, AmigosModuleBase);
    if (!pMyClass)
    {
        AMIGOS_ERR("Pointer Class is Null\n");
        return -1;
    }
    AmigosModuleLdc::LdcInfo                            RegionParamInfo;
    std::map<unsigned int, AmigosModuleLdc::LdcInInfo>  RegionParamInInfo;
    std::map<unsigned int, AmigosModuleLdc::LdcOutInfo> RegionParamOutInfo;

    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    pMyClass->GetInfo(RegionParamInfo, RegionParamInInfo, RegionParamOutInfo);
    int          dev = stInfo.devId;
    int          chn = stInfo.chnId;
    std::string  path;
    void        *addr;
    unsigned int len;
    path = RegionParamInfo.CalibInfo.path;

    MI_LDC_ChnLDCAttr_t stChnLDCAttr;
    memset(&stChnLDCAttr, 0x00, sizeof(MI_LDC_ChnLDCAttr_t));

    MI_LDC_GetChnLDCAttr(dev, chn, &stChnLDCAttr);

    if (path.empty())
    {
        AMIGOS_ERR("File path is NULL");
        return -1;
    }

    // use callib
    len = GetFileSize(path.c_str());
    if (len <= 0)
    {
        AMIGOS_ERR("Get Size is negative num!");
        return -1;
    }
    addr = (void *)malloc(len);
    if (addr == NULL)
    {
        AMIGOS_ERR("No Memory to alloc, ERR!\n");
        return -1;
    }
    ReadFileToBuf(path.c_str(), addr, len);

    MI_U32 i = (MI_U32)ss_cmd_atoi(in_strs[1].c_str());
    if (i == (MI_U32)-1)
    {
        AMIGOS_ERR("RegionID is invalid");
        free(addr);
        return -1;
    }

    if (i > LDC_MAX_REGION_NUM - 1)
    {
        AMIGOS_ERR("RegionNum should <= 9");
        free(addr);
        return -1;
    }
    stChnLDCAttr.stRegionAttr[i].eRegionMode = (MI_LDC_RegionMode_e)ss_cmd_atoi(in_strs[2].c_str());
    stChnLDCAttr.stRegionAttr[i].stRegionPara.eCropMode     = (MI_LDC_RegionCropMode_e)ss_cmd_atoi(in_strs[3].c_str());
    stChnLDCAttr.stRegionAttr[i].stRegionPara.s32Pan        = (MI_U32)ss_cmd_atoi(in_strs[4].c_str());
    stChnLDCAttr.stRegionAttr[i].stRegionPara.s32Tilt       = (MI_U32)ss_cmd_atoi(in_strs[5].c_str());
    stChnLDCAttr.stRegionAttr[i].stRegionPara.s32ZoomV      = (MI_U32)ss_cmd_atoi(in_strs[6].c_str());
    stChnLDCAttr.stRegionAttr[i].stRegionPara.s32ZoomH      = (MI_U32)ss_cmd_atoi(in_strs[7].c_str());
    stChnLDCAttr.stRegionAttr[i].stRegionPara.s32InRadius   = (MI_U32)ss_cmd_atoi(in_strs[8].c_str());
    stChnLDCAttr.stRegionAttr[i].stRegionPara.s32OutRadius  = (MI_U32)ss_cmd_atoi(in_strs[9].c_str());
    stChnLDCAttr.stRegionAttr[i].stRegionPara.s32FocalRatio = (MI_U32)ss_cmd_atoi(in_strs[10].c_str());
    stChnLDCAttr.stRegionAttr[i].stRegionPara.s32DistortionRatio = (MI_U32)ss_cmd_atoi(in_strs[11].c_str());
    stChnLDCAttr.stRegionAttr[i].stRegionPara.s32OutRotate       = (MI_U32)ss_cmd_atoi(in_strs[12].c_str());
    stChnLDCAttr.stRegionAttr[i].stRegionPara.s32Rotate          = (MI_U32)ss_cmd_atoi(in_strs[13].c_str());
    stChnLDCAttr.stCalibInfo.pCalibPolyBinAddr                   = addr;
    stChnLDCAttr.stCalibInfo.u32CalibPolyBinSize                 = len;
    MI_S32 s32Ret                                                = MI_SUCCESS;
    s32Ret                                                       = MI_LDC_SetChnLDCAttr(dev, chn, &stChnLDCAttr);
    if (MI_SUCCESS != s32Ret)
    {
        AMIGOS_ERR("Set LDC Chn Attr Fail, dev: %d, chn: %d, err:0x%x\n", dev, chn, s32Ret);
        free(addr);
        return s32Ret;
    }
    free(addr);

    return 0;
}

static int LdcNirSetDistance(vector<string> &in_strs)
{
    AmigosModuleLdc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleLdc, AmigosModuleBase);
    if (!pMyClass)
    {
        AMIGOS_ERR("Pointer Class is Null\n");
        return -1;
    }
    AmigosModuleLdc::LdcInfo                            NirInfo;
    std::map<unsigned int, AmigosModuleLdc::LdcInInfo>  NirInInfo;
    std::map<unsigned int, AmigosModuleLdc::LdcOutInfo> NirOutInfo;

    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    pMyClass->GetInfo(NirInfo, NirInInfo, NirOutInfo);
    int          dev = stInfo.devId;
    int          chn = stInfo.chnId;
    std::string  path;
    void        *addr;
    unsigned int len;
    path = NirInfo.CalibInfo.path;

    MI_LDC_ChnNIRAttr_t stChnNirAttr;
    memset(&stChnNirAttr, 0x00, sizeof(MI_LDC_ChnNIRAttr_t));

    MI_LDC_GetChnNIRAttr(dev, chn, &stChnNirAttr);
    memset(&stChnNirAttr.s32Distance, 0x00, sizeof(MI_S32));

    if (path.empty())
    {
        AMIGOS_ERR("File path is NULL");
        return -1;
    }

    // use callib
    len = GetFileSize(path.c_str());
    if (len <= 0)
    {
        AMIGOS_ERR("Get Size is negative num!");
        return -1;
    }
    addr = (void *)malloc(len);
    if (addr == NULL)
    {
        AMIGOS_ERR("No Memory to alloc, ERR!\n");
        return -1;
    }
    ReadFileToBuf(path.c_str(), addr, len);

    stChnNirAttr.s32Distance              = (MI_S32)ss_cmd_atoi(in_strs[1].c_str());
    stChnNirAttr.stCalCfg.pCalibCfgAddr   = addr;
    stChnNirAttr.stCalCfg.u32CalibCfgSize = len;
    MI_S32 s32Ret                         = MI_SUCCESS;
    s32Ret                                = MI_LDC_SetChnNIRAttr(dev, chn, &stChnNirAttr);
    if (MI_SUCCESS != s32Ret)
    {
        AMIGOS_ERR("Set LDC Chn Attr Fail, dev: %d, chn: %d, err:0x%x\n", dev, chn, s32Ret);
        free(addr);
        return s32Ret;
    }
    free(addr);
    return 0;
}

static int LdcDeleteRegion(vector<string> &in_strs)
{
    AmigosModuleLdc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleLdc, AmigosModuleBase);
    if (!pMyClass)
    {
        AMIGOS_ERR("Pointer Class is Null\n");
        return -1;
    }
    AmigosModuleLdc::LdcInfo                            RegionParamInfo;
    std::map<unsigned int, AmigosModuleLdc::LdcInInfo>  RegionParamInInfo;
    std::map<unsigned int, AmigosModuleLdc::LdcOutInfo> RegionParamOutInfo;

    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    pMyClass->GetInfo(RegionParamInfo, RegionParamInInfo, RegionParamOutInfo);
    int          dev = stInfo.devId;
    int          chn = stInfo.chnId;
    std::string  path;
    void        *addr;
    unsigned int len;
    path = RegionParamInfo.CalibInfo.path;

    MI_LDC_ChnLDCAttr_t stChnLDCAttr;
    memset(&stChnLDCAttr, 0x00, sizeof(MI_LDC_ChnLDCAttr_t));

    MI_LDC_GetChnLDCAttr(dev, chn, &stChnLDCAttr);
    if (stChnLDCAttr.u32RegionNum < 1)
    {
        AMIGOS_WRN("Region Num is below 1, no need to delete\n");
        return -1;
    }
    MI_U32 tmpRegionId        = stChnLDCAttr.u32RegionNum - 1;
    stChnLDCAttr.u32RegionNum = tmpRegionId;

    if (path.empty())
    {
        AMIGOS_ERR("File path is NULL");
        return -1;
    }

    // use callib
    len = GetFileSize(path.c_str());
    if (len <= 0)
    {
        AMIGOS_ERR("Get Size is negative num!");
        return -1;
    }
    addr = (void *)malloc(len);
    if (addr == NULL)
    {
        AMIGOS_ERR("No Memory to alloc, ERR!\n");
        return -1;
    }
    ReadFileToBuf(path.c_str(), addr, len);

    memset(&stChnLDCAttr.stRegionAttr[tmpRegionId], 0x00, sizeof(MI_LDC_RegionAttr_t));
    stChnLDCAttr.stCalibInfo.pCalibPolyBinAddr   = addr;
    stChnLDCAttr.stCalibInfo.u32CalibPolyBinSize = len;
    MI_S32 s32Ret                                = MI_SUCCESS;
    s32Ret                                       = MI_LDC_SetChnLDCAttr(dev, chn, &stChnLDCAttr);
    if (MI_SUCCESS != s32Ret)
    {
        AMIGOS_ERR("Set LDC Chn Attr Fail, dev: %d, chn: %d, err:0x%x\n", dev, chn, s32Ret);
        free(addr);
        return s32Ret;
    }
    free(addr);

    return 0;
}

static int LdcSetMultiRegion(vector<string> &in_strs)
{
    AmigosModuleLdc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleLdc, AmigosModuleBase);
    if (!pMyClass)
    {
        AMIGOS_ERR("Pointer Class is Null\n");
        return -1;
    }
    AmigosModuleLdc::LdcInfo                            RegionParamInfo;
    std::map<unsigned int, AmigosModuleLdc::LdcInInfo>  RegionParamInInfo;
    std::map<unsigned int, AmigosModuleLdc::LdcOutInfo> RegionParamOutInfo;

    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    pMyClass->GetInfo(RegionParamInfo, RegionParamInInfo, RegionParamOutInfo);
    int          dev = stInfo.devId;
    int          chn = stInfo.chnId;
    std::string  path;
    void        *addr;
    unsigned int len;
    path = RegionParamInfo.CalibInfo.path;

    MI_LDC_ChnLDCAttr_t stChnLDCAttr;
    memset(&stChnLDCAttr, 0x00, sizeof(MI_LDC_ChnLDCAttr_t));

    MI_LDC_GetChnLDCAttr(dev, chn, &stChnLDCAttr);

    if (path.empty())
    {
        AMIGOS_ERR("File path is NULL");
        return -1;
    }

    // use callib
    len = GetFileSize(path.c_str());
    if (len <= 0)
    {
        AMIGOS_ERR("Get Size is negative num!");
        return -1;
    }
    addr = (void *)malloc(len);
    if (addr == NULL)
    {
        AMIGOS_ERR("No Memory to alloc, ERR!\n");
        return -1;
    }
    ReadFileToBuf(path.c_str(), addr, len);

    // set width and height from sensor for get every region width and height
    stChnLDCAttr.u32RegionNum = (MI_U32)ss_cmd_atoi(in_strs[1].c_str());
    int tmpWidth              = (MI_U32)ss_cmd_atoi(in_strs[2].c_str());
    int tmpHeight             = (MI_U32)ss_cmd_atoi(in_strs[3].c_str());
    for (int i = 0; i < (int)stChnLDCAttr.u32RegionNum; i++)
    {
        stChnLDCAttr.stRegionAttr[i].eRegionMode                     = MI_LDC_REGION_NO_TRANSFORMATION;
        stChnLDCAttr.stRegionAttr[i].stOutRect.u16X                  = (i % 3) * tmpWidth / 3 + 16;
        stChnLDCAttr.stRegionAttr[i].stOutRect.u16Y                  = (i / 3) * tmpHeight / 3 + 2;
        stChnLDCAttr.stRegionAttr[i].stOutRect.u16Width              = alignDown(tmpWidth / 3, 16) - 32;
        stChnLDCAttr.stRegionAttr[i].stOutRect.u16Height             = alignDown(tmpHeight / 3, 2) - 16;
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
            AMIGOS_ERR("The current mode is MAP2BIN, please choose another mode: [1/2/3/5]");
            free(addr);
            return -1;
        }
    }
    stChnLDCAttr.stCalibInfo.pCalibPolyBinAddr   = addr;
    stChnLDCAttr.stCalibInfo.u32CalibPolyBinSize = len;
    MI_S32 s32Ret                                = MI_SUCCESS;
    s32Ret                                       = MI_LDC_SetChnLDCAttr(dev, chn, &stChnLDCAttr);
    if (MI_SUCCESS != s32Ret)
    {
        AMIGOS_ERR("Set LDC Chn Attr Fail, dev: %d, chn: %d, err:0x%x\n", dev, chn, s32Ret);
        free(addr);
        return s32Ret;
    }
    free(addr);

    return 0;
}

static int LdcSetFullRegion(vector<string> &in_strs)
{
    AmigosModuleLdc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleLdc, AmigosModuleBase);
    if (!pMyClass)
    {
        AMIGOS_ERR("Pointer Class is Null\n");
        return -1;
    }
    AmigosModuleLdc::LdcInfo                            RegionParamInfo;
    std::map<unsigned int, AmigosModuleLdc::LdcInInfo>  RegionParamInInfo;
    std::map<unsigned int, AmigosModuleLdc::LdcOutInfo> RegionParamOutInfo;

    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    pMyClass->GetInfo(RegionParamInfo, RegionParamInInfo, RegionParamOutInfo);
    int          dev = stInfo.devId;
    int          chn = stInfo.chnId;
    std::string  path;
    void        *addr;
    unsigned int len;
    path = RegionParamInfo.CalibInfo.path;

    MI_LDC_ChnLDCAttr_t stChnLDCAttr;
    memset(&stChnLDCAttr, 0x00, sizeof(MI_LDC_ChnLDCAttr_t));

    MI_LDC_GetChnLDCAttr(dev, chn, &stChnLDCAttr);

    if (path.empty())
    {
        AMIGOS_ERR("File path is NULL");
        return -1;
    }

    // use callib
    len = GetFileSize(path.c_str());
    if (len <= 0)
    {
        AMIGOS_ERR("Get Size is negative num!");
        return -1;
    }
    addr = (void *)malloc(len);
    if (addr == NULL)
    {
        AMIGOS_ERR("No Memory to alloc, ERR!\n");
        return -1;
    }
    ReadFileToBuf(path.c_str(), addr, len);

    // set width and height from sensor for get every region width and height
    stChnLDCAttr.u32RegionNum                                    = 1;
    int tmpWidth                                                 = (MI_U32)ss_cmd_atoi(in_strs[1].c_str());
    int tmpHeight                                                = (MI_U32)ss_cmd_atoi(in_strs[2].c_str());
    stChnLDCAttr.stRegionAttr[0].eRegionMode                     = (MI_LDC_RegionMode_e)ss_cmd_atoi(in_strs[3].c_str());
    stChnLDCAttr.stRegionAttr[0].stOutRect.u16X                  = 0;
    stChnLDCAttr.stRegionAttr[0].stOutRect.u16Y                  = 0;
    stChnLDCAttr.stRegionAttr[0].stOutRect.u16Width              = alignDown(tmpWidth, 16);
    stChnLDCAttr.stRegionAttr[0].stOutRect.u16Height             = alignDown(tmpHeight, 2);
    stChnLDCAttr.stRegionAttr[0].stRegionPara.eCropMode          = (MI_LDC_RegionCropMode_e)ss_cmd_atoi(in_strs[4].c_str());
    stChnLDCAttr.stRegionAttr[0].stRegionPara.s32Pan             = 0;
    stChnLDCAttr.stRegionAttr[0].stRegionPara.s32Tilt            = 10;
    stChnLDCAttr.stRegionAttr[0].stRegionPara.s32ZoomV           = (MI_U32)ss_cmd_atoi(in_strs[5].c_str());
    stChnLDCAttr.stRegionAttr[0].stRegionPara.s32ZoomH           = (MI_U32)ss_cmd_atoi(in_strs[6].c_str());
    stChnLDCAttr.stRegionAttr[0].stRegionPara.s32InRadius        = 0;
    stChnLDCAttr.stRegionAttr[0].stRegionPara.s32OutRadius       = (MI_U32)ss_cmd_atoi(in_strs[7].c_str());
    stChnLDCAttr.stRegionAttr[0].stRegionPara.s32FocalRatio      = 0;
    stChnLDCAttr.stRegionAttr[0].stRegionPara.s32DistortionRatio = 0;
    stChnLDCAttr.stRegionAttr[0].stRegionPara.s32OutRotate       = 0;
    stChnLDCAttr.stRegionAttr[0].stRegionPara.s32Rotate          = 0;

    if (stChnLDCAttr.stRegionAttr[0].eRegionMode == MI_LDC_REGION_MAP2BIN)
    {
        AMIGOS_ERR("The current mode is MAP2BIN, please choose another mode: [1/2/3/5]");
        free(addr);
        return -1;
    }
    stChnLDCAttr.stCalibInfo.pCalibPolyBinAddr   = addr;
    stChnLDCAttr.stCalibInfo.u32CalibPolyBinSize = len;
    MI_S32 s32Ret                                = MI_SUCCESS;
    s32Ret                                       = MI_LDC_SetChnLDCAttr(dev, chn, &stChnLDCAttr);
    if (MI_SUCCESS != s32Ret)
    {
        AMIGOS_ERR("Set LDC Chn Attr Fail, dev: %d, chn: %d, err:0x%x\n", dev, chn, s32Ret);
        free(addr);
        return s32Ret;
    }
    free(addr);

    return 0;
}
static int LdcAddRegion(vector<string> &in_strs)
{
    AmigosModuleLdc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleLdc, AmigosModuleBase);
    if (!pMyClass)
    {
        AMIGOS_ERR("Pointer Class is Null\n");
        return -1;
    }
    AmigosModuleLdc::LdcInfo                            RegionParamInfo;
    std::map<unsigned int, AmigosModuleLdc::LdcInInfo>  RegionParamInInfo;
    std::map<unsigned int, AmigosModuleLdc::LdcOutInfo> RegionParamOutInfo;

    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    pMyClass->GetInfo(RegionParamInfo, RegionParamInInfo, RegionParamOutInfo);
    int          dev = stInfo.devId;
    int          chn = stInfo.chnId;
    std::string  path;
    void        *addr;
    unsigned int len;
    path = RegionParamInfo.CalibInfo.path;

    static MI_LDC_ChnLDCAttr_t stChnLDCAttr;
    memset(&stChnLDCAttr, 0x00, sizeof(MI_LDC_ChnLDCAttr_t));

    MI_LDC_GetChnLDCAttr(dev, chn, &stChnLDCAttr);
    MI_U32 tmpRegionId        = stChnLDCAttr.u32RegionNum;
    stChnLDCAttr.u32RegionNum = stChnLDCAttr.u32RegionNum + 1;
    if (stChnLDCAttr.u32RegionNum > LDC_MAX_REGION_NUM)
    {
        AMIGOS_ERR("RegionNum should <= 9");
        return -1;
    }

    if (path.empty())
    {
        AMIGOS_ERR("File path is NULL");
        return -1;
    }

    // use callib
    len = GetFileSize(path.c_str());
    if (len <= 0)
    {
        AMIGOS_ERR("Get Size is negative num!");
        return -1;
    }
    addr = (void *)malloc(len);
    if (addr == NULL)
    {
        AMIGOS_ERR("No Memory to alloc, ERR!\n");
        return -1;
    }
    ReadFileToBuf(path.c_str(), addr, len);

    stChnLDCAttr.stRegionAttr[tmpRegionId].eRegionMode         = (MI_LDC_RegionMode_e)ss_cmd_atoi(in_strs[1].c_str());
    stChnLDCAttr.stRegionAttr[tmpRegionId].stOutRect.u16X      = (MI_U32)ss_cmd_atoi(in_strs[2].c_str());
    stChnLDCAttr.stRegionAttr[tmpRegionId].stOutRect.u16Y      = (MI_U32)ss_cmd_atoi(in_strs[3].c_str());
    stChnLDCAttr.stRegionAttr[tmpRegionId].stOutRect.u16Width  = (MI_U32)ss_cmd_atoi(in_strs[4].c_str());
    stChnLDCAttr.stRegionAttr[tmpRegionId].stOutRect.u16Height = (MI_U32)ss_cmd_atoi(in_strs[5].c_str());
    stChnLDCAttr.stRegionAttr[tmpRegionId].stRegionPara.eCropMode =
        (MI_LDC_RegionCropMode_e)ss_cmd_atoi(in_strs[6].c_str());
    stChnLDCAttr.stRegionAttr[tmpRegionId].stRegionPara.s32Pan             = (MI_U32)ss_cmd_atoi(in_strs[7].c_str());
    stChnLDCAttr.stRegionAttr[tmpRegionId].stRegionPara.s32Tilt            = (MI_U32)ss_cmd_atoi(in_strs[8].c_str());
    stChnLDCAttr.stRegionAttr[tmpRegionId].stRegionPara.s32ZoomV           = (MI_U32)ss_cmd_atoi(in_strs[9].c_str());
    stChnLDCAttr.stRegionAttr[tmpRegionId].stRegionPara.s32ZoomH           = (MI_U32)ss_cmd_atoi(in_strs[10].c_str());
    stChnLDCAttr.stRegionAttr[tmpRegionId].stRegionPara.s32InRadius        = (MI_U32)ss_cmd_atoi(in_strs[11].c_str());
    stChnLDCAttr.stRegionAttr[tmpRegionId].stRegionPara.s32OutRadius       = (MI_U32)ss_cmd_atoi(in_strs[12].c_str());
    stChnLDCAttr.stRegionAttr[tmpRegionId].stRegionPara.s32FocalRatio      = (MI_U32)ss_cmd_atoi(in_strs[13].c_str());
    stChnLDCAttr.stRegionAttr[tmpRegionId].stRegionPara.s32DistortionRatio = (MI_U32)ss_cmd_atoi(in_strs[14].c_str());
    stChnLDCAttr.stRegionAttr[tmpRegionId].stRegionPara.s32OutRotate       = (MI_U32)ss_cmd_atoi(in_strs[15].c_str());
    stChnLDCAttr.stRegionAttr[tmpRegionId].stRegionPara.s32Rotate          = (MI_U32)ss_cmd_atoi(in_strs[16].c_str());

    if (stChnLDCAttr.stRegionAttr[tmpRegionId].eRegionMode == MI_LDC_REGION_MAP2BIN)
    {
        AMIGOS_ERR("The current mode is MAP2BIN, please choose another mode: [1/2/3/5]");
        free(addr);
        return -1;
    }

    stChnLDCAttr.stCalibInfo.pCalibPolyBinAddr   = addr;
    stChnLDCAttr.stCalibInfo.u32CalibPolyBinSize = len;
    MI_S32 s32Ret                                = MI_SUCCESS;
    s32Ret                                       = MI_LDC_SetChnLDCAttr(dev, chn, &stChnLDCAttr);
    if (MI_SUCCESS != s32Ret)
    {
        AMIGOS_ERR("Set LDC Chn Attr Fail, dev: %d, chn: %d, err:0x%x\n", dev, chn, s32Ret);
        free(addr);
        return s32Ret;
    }
    free(addr);

    return 0;
}

static int LdcPmfSetCoef(vector<string> &in_strs)
{
    AmigosModuleLdc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleLdc, AmigosModuleBase);
    if (!pMyClass)
    {
        AMIGOS_ERR("Pointer Class is Null\n");
        return -1;
    }
    AmigosModuleLdc::LdcInfo                            PmfParamInfo;
    std::map<unsigned int, AmigosModuleLdc::LdcInInfo>  PmfParamInInfo;
    std::map<unsigned int, AmigosModuleLdc::LdcOutInfo> PmfParamOutInfo;

    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    pMyClass->GetInfo(PmfParamInfo, PmfParamInInfo, PmfParamOutInfo);

    int dev = stInfo.devId;
    int chn = stInfo.chnId;

    MI_LDC_ChnPMFAttr_t stChnPMFAttr;
    memset(&stChnPMFAttr, 0x00, sizeof(MI_LDC_ChnPMFAttr_t));

    MI_LDC_GetChnPMFAttr(dev, chn, &stChnPMFAttr);

    for (int i = 0; i < 9; i++)
    {
        memset(&stChnPMFAttr.as64PMFCoef[i], 0x00, sizeof(MI_S64));
    }
    stChnPMFAttr.as64PMFCoef[0] = (MI_S64)ss_cmd_atoi(in_strs[1].c_str());
    stChnPMFAttr.as64PMFCoef[1] = (MI_S64)ss_cmd_atoi(in_strs[2].c_str());
    stChnPMFAttr.as64PMFCoef[2] = (MI_S64)ss_cmd_atoi(in_strs[3].c_str());
    stChnPMFAttr.as64PMFCoef[3] = (MI_S64)ss_cmd_atoi(in_strs[4].c_str());
    stChnPMFAttr.as64PMFCoef[4] = (MI_S64)ss_cmd_atoi(in_strs[5].c_str());
    stChnPMFAttr.as64PMFCoef[5] = (MI_S64)ss_cmd_atoi(in_strs[6].c_str());
    stChnPMFAttr.as64PMFCoef[6] = (MI_S64)ss_cmd_atoi(in_strs[7].c_str());
    stChnPMFAttr.as64PMFCoef[7] = (MI_S64)ss_cmd_atoi(in_strs[8].c_str());
    stChnPMFAttr.as64PMFCoef[8] = (MI_S64)ss_cmd_atoi(in_strs[9].c_str());

    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret        = MI_LDC_SetChnPMFAttr(dev, chn, &stChnPMFAttr);
    if (MI_SUCCESS != s32Ret)
    {
        AMIGOS_ERR("Set Chn PMF Attr Fail, dev: %d, chn: %d, err: 0x%x\n", dev, chn, s32Ret);
        return s32Ret;
    }

    return 0;
}

MOD_CMDS(AmiCmdLdc)
{
    ADD_CMD("ldc_stitch_set_distance", LdcStitchSetDistance, 1);
    ADD_CMD_HELP("ldc_stitch_set_distance", "[distance]", "example: ldc_stitch_set_distance 1000");
    ADD_CMD("ldc_stitch_set_projtype", LdcStitchSetProjType, 1);
    ADD_CMD_HELP("ldc_stitch_set_projtype", "[type]: 0:rectilinear, 1:cylindrical, 2:spherical",
                 "example: ldc_stitch_set_projtype 0");
    ADD_CMD("ldc_set_bgcolor", LdcSetBgColor, 2);
    ADD_CMD_HELP("ldc_set_bgcolor", "[enable] [color]", "enable [0/1], color [0x000000 - 0xFFFFFF]");
    ADD_CMD("ldc_set_mountmode", LdcSetMountMode, 1);
    ADD_CMD_HELP("ldc_set_mountmode", "[mountmode]", "example: ldc_set_mountmode [1/2/3]");
    ADD_CMD("ldc_set_regionparam", LdcSetRegionParam, 13);
    ADD_CMD_HELP("ldc_set_regionparam",
                 "[regionID] [regionMode] [cropMode] [pan] [tilt] [zoomV] [zoomH] [inRadius] [outRadius] [focalRatio] "
                 "[distortionRatio] [outRotate] [rotate]",
                 "cropmode [0/1/2], inRadiuus [Valid if regionmode == 1], outRotate [Invalid if regionmode == 1], "
                 "rotate [Valid if regionmode == 3]");
    ADD_CMD("ldc_nir_set_distance", LdcNirSetDistance, 1);
    ADD_CMD_HELP("ldc_nir_set_distance", "[distance]", "example: ldc_nir_set_distance 1000");
    ADD_CMD("ldc_delete_region", LdcDeleteRegion, 0);
    ADD_CMD_HELP("ldc_delete_region", "[NULL]", "example: ldc_delete_region, note:fixed clear the last region");
    ADD_CMD("ldc_add_region", LdcAddRegion, 16);
    ADD_CMD_HELP("ldc_add_region",
                 "[regionmode] [X] [Y] [width] [height] [cropMode] [pan] [tilt] [zoomV] [zoomH] [inRadius] [outRadius] "
                 "[focalRatio] [distortionRatio] [outRotate] [rotate]",
                 "regionmode [can not == 4], cropmode [0/1/2], inRadiuus [Valid if regionmode == 1], outRotate "
                 "[Invalid if regionmode == 1], rotate [Valid if regionmode == 3]");
    ADD_CMD("ldc_set_multi_region", LdcSetMultiRegion, 3);
    ADD_CMD_HELP("ldc_set_multi_region", "set mult number region", "[number] [height of snr] [width of snr]");
    ADD_CMD("ldc_set_full_region", LdcSetFullRegion, 7);
    ADD_CMD_HELP("ldc_set_full_region", "set Full region", "[width of snr] [height of snr] [region mode] [crop mode] "
                "[zoomV] [zoomH] [out radius]");
    ADD_CMD("ldc_pmf_set_coef", LdcPmfSetCoef, 9);
    ADD_CMD_HELP(
        "ldc_pmf_set_coef",
        "[PMFCoef[0]] [PMFCoef[1]] [PMFCoef[2]] [PMFCoef[3]] [PMFCoef[4]] [PMFCoef[5]] [PMFCoef[6]] [PMFCoef[7]] "
        "[PMFCoef[8]]",
        "PMFCoef[0]:[-67108864, 67106816], PMFCoef[1]:[-67108864, 67106816], PMFCoef[2]:[-137438953472, 137434759168]",
        "PMFCoef[3]:[-67108864, 67106816], PMFCoef[4]:[-67108864, 67106816], PMFCoef[5]:[-137438953472, 137434759168]",
        "PMFCoef[6]:[-32768, 32767],       PMFCoef[7]:[-32768, 32767],       PMFCoef[8]:(fixed value)33554432");
}
