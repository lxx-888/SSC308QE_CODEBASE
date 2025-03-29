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
#include <cstring>
#include "amigos_surface_ldc.h"
AmigosSurfaceLdc::AmigosSurfaceLdc(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, true, true) {}

AmigosSurfaceLdc::~AmigosSurfaceLdc() {}

void AmigosSurfaceLdc::_LoadDb()
{
    char                       strLayerName[30];
    LdcInInfo  stLdcInInfo;
    LdcOutInfo stLdcOutInfo;
    //only IDS scene has 3 input,andLdcInInfo use Inport0 res call MI_LDC_SetInputPortAttr();
    auto itMapLdcIn = this->mapModInputInfo.begin();
    stLdcInInfo.width  = this->pDb->GetIn<unsigned int>(itMapLdcIn->second.curLoopId, "VID_W");
    stLdcInInfo.height = this->pDb->GetIn<unsigned int>(itMapLdcIn->second.curLoopId, "VID_H");
    mapLdcIn[itMapLdcIn->first] = stLdcInInfo;
    for (auto itMapLdcOut = this->mapModOutputInfo.begin(); itMapLdcOut != this->mapModOutputInfo.end(); itMapLdcOut++)
    {
        stLdcOutInfo.width      = this->pDb->GetOut<unsigned int>(itMapLdcOut->second.curLoopId, "VID_W");
        stLdcOutInfo.height     = this->pDb->GetOut<unsigned int>(itMapLdcOut->second.curLoopId, "VID_H");
        stLdcOutInfo.strOutType = this->pDb->GetOut<std::string>(itMapLdcOut->second.curLoopId, "OUT_TYPE");
        stLdcOutInfo.strOutFmt  = this->pDb->GetOut<std::string>(itMapLdcOut->second.curLoopId, "OUT_FMT");
        mapLdcOut[itMapLdcOut->first] = stLdcOutInfo;
    }
    stLdcInfo.CalibInfo.clear();
    stLdcInfo.strWorkMode = this->pDb->GetMod<std::string>("WORK_MODE");
    if ("ldc" == stLdcInfo.strWorkMode)
    {
        stLdcInfo.CalibInfo.path                     = this->pDb->GetMod<std::string>("WORK_MODE_PARAM", "CALIB_PATH");
        stLdcInfo.LdcCfg.FisheyeRadius               = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "FISHEYE_RADIUS");
        stLdcInfo.LdcCfg.Center_X_Off                = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "CENTER_X_OFFSET");
        stLdcInfo.LdcCfg.Center_Y_Off                = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "CENTER_Y_OFFSET");
        stLdcInfo.LdcCfg.EnBgColor                   = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "ENABLE_BGCOLOR");
        stLdcInfo.LdcCfg.BgColor                     = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "BGCOLOR");
        stLdcInfo.LdcCfg.MountMode                   = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "MOUNT_MODE");
        stLdcInfo.LdcCfg.RegionNum                   = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "REGION_NUM");
        if(stLdcInfo.LdcCfg.RegionNum == (unsigned int)-1 || stLdcInfo.LdcCfg.RegionNum > 9)  //LDC_MAX_REGION_NUM
        {
            AMIGOS_ERR("RegionNum error!\n");
            return;
        }
        for(unsigned int i = 0;i<stLdcInfo.LdcCfg.RegionNum;i++)
        {
            snprintf(strLayerName, 30, "REGION_%d", i);
            stLdcInfo.LdcCfg.Region[i].RegionMode           = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"REGION_MODE");
            stLdcInfo.LdcCfg.Region[i].OutRect.x            = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"OUT_RECT_X");
            stLdcInfo.LdcCfg.Region[i].OutRect.y            = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"OUT_RECT_Y");
            stLdcInfo.LdcCfg.Region[i].OutRect.width        = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"OUT_RECT_WIDTH");
            stLdcInfo.LdcCfg.Region[i].OutRect.height       = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"OUT_RECT_HEIGHT");
            if(4 == stLdcInfo.LdcCfg.Region[i].RegionMode)    //MI_LDC_REGION_MAP2BIN
            {
                stLdcInfo.LdcCfg.Region[i].map2binPara.mapX = this->pDb->GetMod<std::string>("WORK_MODE_PARAM", strLayerName,"MAP_X");
                stLdcInfo.LdcCfg.Region[i].map2binPara.mapY = this->pDb->GetMod<std::string>("WORK_MODE_PARAM", strLayerName,"MAP_Y");
                stLdcInfo.LdcCfg.Region[i].map2binPara.grid = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"GRID");
            }
            else
            {
               stLdcInfo.LdcCfg.Region[i].Para.CropMode        = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"CROP_MODE");
               stLdcInfo.LdcCfg.Region[i].Para.Pan             = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"PAN");
               stLdcInfo.LdcCfg.Region[i].Para.Tilt            = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"TILT");
               stLdcInfo.LdcCfg.Region[i].Para.ZoomV           = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"ZOOM_V");
               stLdcInfo.LdcCfg.Region[i].Para.ZoomH           = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"ZOOM_H");
               stLdcInfo.LdcCfg.Region[i].Para.InRadius        = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"IN_RADIUS");
               stLdcInfo.LdcCfg.Region[i].Para.OutRadius       = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"OUT_RADIUS");
               stLdcInfo.LdcCfg.Region[i].Para.FocalRatio      = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"FOCAL_RATIO");
               stLdcInfo.LdcCfg.Region[i].Para.DistortionRatio = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"DISTORTION_RATIO");
               stLdcInfo.LdcCfg.Region[i].Para.OutRot          = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"OUT_ROT");
               stLdcInfo.LdcCfg.Region[i].Para.Rot             = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"ROT");
            }
        }
    }
    else if ("lut" == stLdcInfo.strWorkMode)
    {
        stLdcInfo.LutCfg.Width  = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "WIDTH");
        stLdcInfo.LutCfg.Height = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "HEIGHT");
        stLdcInfo.LutCfg.Table_Weight = this->pDb->GetMod<std::string>("WORK_MODE_PARAM", "TABLE_WEIGHT_PATH");
        stLdcInfo.LutCfg.Table_X = this->pDb->GetMod<std::string>("WORK_MODE_PARAM", "TABLE_X_PATH");
        stLdcInfo.LutCfg.Table_Y = this->pDb->GetMod<std::string>("WORK_MODE_PARAM", "TABLE_Y_PATH");
    }
    else if ("dis" == stLdcInfo.strWorkMode)
    {
        stLdcInfo.DisCfg.DisMode      = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "DIS_MODE");
        stLdcInfo.DisCfg.UserSliceNum = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "USER_SLICE_NUM");
        stLdcInfo.DisCfg.FocalLengthX = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "FOCAL_LEN_X");
        stLdcInfo.DisCfg.FocalLengthY = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "FOCAL_LEN_Y");
        stLdcInfo.DisCfg.SceneType = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "SCENE_TYPE");
        stLdcInfo.DisCfg.MotionLevel = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "MOTION_LEVEL");
        stLdcInfo.DisCfg.CropRatio = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "CROP_RATIO");
        stLdcInfo.DisCfg.RotationMatrix = this->pDb->GetMod<std::string>("WORK_MODE_PARAM", "ROTATION_MATRIX");
    }
    else if ("pmf" == stLdcInfo.strWorkMode)
    {
        stLdcInfo.PmfCfg.PmfCoef = this->pDb->GetMod<std::string>("WORK_MODE_PARAM", "PMF_COEF");
    }
    else if ("stitch" == stLdcInfo.strWorkMode)
    {
        stLdcInfo.CalibInfo.path = this->pDb->GetMod<std::string>("WORK_MODE_PARAM", "CALIB_PATH");
        stLdcInfo.StitchCfg.Distance = this->pDb->GetMod<int>("WORK_MODE_PARAM", "DISTANCE");
        stLdcInfo.StitchCfg.ProjType = this->pDb->GetMod<int>("WORK_MODE_PARAM", "PROJ_TYPE");
    }
    else if ("nir" == stLdcInfo.strWorkMode)
    {
        stLdcInfo.CalibInfo.path = this->pDb->GetMod<std::string>("WORK_MODE_PARAM", "CALIB_PATH");
        stLdcInfo.NirCfg.Distance = this->pDb->GetMod<int>("WORK_MODE_PARAM", "DISTANCE");
    }
    else if ("dpu" == stLdcInfo.strWorkMode)
    {
        stLdcInfo.CalibInfo.path = this->pDb->GetMod<std::string>("WORK_MODE_PARAM", "CALIB_PATH");
        stLdcInfo.DpuCfg.Distance = this->pDb->GetMod<int>("WORK_MODE_PARAM", "DISTANCE");
    }
    else if ("ldc_horizontal" == stLdcInfo.strWorkMode)
    {
        stLdcInfo.CalibInfo.path = this->pDb->GetMod<std::string>("WORK_MODE_PARAM", "CALIB_PATH");
        stLdcInfo.LdcHorizontalCfg.DistortionRatio = this->pDb->GetMod<int>("WORK_MODE_PARAM", "DISTORTION_RATIO");
    }
    else if ("dis_ldc" == stLdcInfo.strWorkMode)
    {
        stLdcInfo.CalibInfo.path                     = this->pDb->GetMod<std::string>("WORK_MODE_PARAM", "CALIB_PATH");
        stLdcInfo.LdcCfg.FisheyeRadius               = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "FISHEYE_RADIUS");
        stLdcInfo.LdcCfg.Center_X_Off                = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "CENTER_X_OFFSET");
        stLdcInfo.LdcCfg.Center_Y_Off                = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "CENTER_Y_OFFSET");
        stLdcInfo.LdcCfg.EnBgColor                   = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "ENABLE_BGCOLOR");
        stLdcInfo.LdcCfg.BgColor                     = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "BGCOLOR");
        stLdcInfo.LdcCfg.MountMode                   = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "MOUNT_MODE");
        stLdcInfo.LdcCfg.RegionNum                   = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "REGION_NUM");
        if(stLdcInfo.LdcCfg.RegionNum == (unsigned int)-1 || stLdcInfo.LdcCfg.RegionNum > 9)  //LDC_MAX_REGION_NUM
        {
            AMIGOS_ERR("RegionNum error!\n");
            return;
        }
        for(unsigned int i = 0;i<stLdcInfo.LdcCfg.RegionNum;i++)
        {
            snprintf(strLayerName, 30, "REGION_%d", i);
            stLdcInfo.LdcCfg.Region[i].RegionMode           = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"REGION_MODE");
            stLdcInfo.LdcCfg.Region[i].OutRect.x            = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"OUT_RECT_X");
            stLdcInfo.LdcCfg.Region[i].OutRect.y            = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"OUT_RECT_Y");
            stLdcInfo.LdcCfg.Region[i].OutRect.width        = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"OUT_RECT_WIDTH");
            stLdcInfo.LdcCfg.Region[i].OutRect.height       = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"OUT_RECT_HEIGHT");
            if(4 == stLdcInfo.LdcCfg.Region[i].RegionMode)    //MI_LDC_REGION_MAP2BIN
            {
                stLdcInfo.LdcCfg.Region[i].map2binPara.mapX = this->pDb->GetMod<std::string>("WORK_MODE_PARAM", strLayerName,"MAP_X");
                stLdcInfo.LdcCfg.Region[i].map2binPara.mapY = this->pDb->GetMod<std::string>("WORK_MODE_PARAM", strLayerName,"MAP_Y");
                stLdcInfo.LdcCfg.Region[i].map2binPara.grid = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"GRID");
            }
            else
            {
               stLdcInfo.LdcCfg.Region[i].Para.CropMode        = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"CROP_MODE");
               stLdcInfo.LdcCfg.Region[i].Para.Pan             = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"PAN");
               stLdcInfo.LdcCfg.Region[i].Para.Tilt            = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"TILT");
               stLdcInfo.LdcCfg.Region[i].Para.ZoomV           = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"ZOOM_V");
               stLdcInfo.LdcCfg.Region[i].Para.ZoomH           = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"ZOOM_H");
               stLdcInfo.LdcCfg.Region[i].Para.InRadius        = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"IN_RADIUS");
               stLdcInfo.LdcCfg.Region[i].Para.OutRadius       = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"OUT_RADIUS");
               stLdcInfo.LdcCfg.Region[i].Para.FocalRatio      = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"FOCAL_RATIO");
               stLdcInfo.LdcCfg.Region[i].Para.DistortionRatio = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"DISTORTION_RATIO");
               stLdcInfo.LdcCfg.Region[i].Para.OutRot          = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"OUT_ROT");
               stLdcInfo.LdcCfg.Region[i].Para.Rot             = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", strLayerName,"ROT");
            }
        }
        stLdcInfo.DisCfg.DisMode      = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "DIS_MODE");
        stLdcInfo.DisCfg.UserSliceNum = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "USER_SLICE_NUM");
        stLdcInfo.DisCfg.FocalLengthX = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "FOCAL_LEN_X");
        stLdcInfo.DisCfg.FocalLengthY = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "FOCAL_LEN_Y");
        stLdcInfo.DisCfg.SceneType = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "SCENE_TYPE");
        stLdcInfo.DisCfg.MotionLevel = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "MOTION_LEVEL");
        stLdcInfo.DisCfg.CropRatio = this->pDb->GetMod<unsigned int>("WORK_MODE_PARAM", "CROP_RATIO");
        stLdcInfo.DisCfg.RotationMatrix = this->pDb->GetMod<std::string>("WORK_MODE_PARAM", "ROTATION_MATRIX");
    }
}

void AmigosSurfaceLdc::_UnloadDb()
{
    mapLdcIn.clear();
    mapLdcOut.clear();
    stLdcInfo.Clear();
}

void AmigosSurfaceLdc::GetInfo(LdcInfo &info, std::map<unsigned int, LdcInInfo> &in,
             std::map<unsigned int, LdcOutInfo> &out) const

{
    info = stLdcInfo;
    in   = mapLdcIn;
    out  = mapLdcOut;
}
void AmigosSurfaceLdc::SetInfo(const LdcInfo &info, const std::map<unsigned int, LdcInInfo> &in,
             const std::map<unsigned int,LdcOutInfo> &out)

{
    stLdcInfo = info;
    mapLdcIn = in;
    mapLdcOut = out;
}

