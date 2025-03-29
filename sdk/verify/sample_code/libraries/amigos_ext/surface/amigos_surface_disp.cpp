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

#include "amigos_surface_disp.h"

AmigosSurfaceDisp::AmigosSurfaceDisp(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, true, false) {}

AmigosSurfaceDisp::~AmigosSurfaceDisp() {}

void AmigosSurfaceDisp::_LoadDb()
{
    std::string                strInputCnt;
    int                        intLayerCnt;
    int                        intLayerId;
    int                        i = 0;
    char                       strLayerName[30];
    std::map<unsigned int, unsigned int>mapPortId;
    stDispInfo.strDevType         = this->pDb->GetMod<std::string>("DEV_TYPE");
    stDispInfo.strBackGroundColor = this->pDb->GetMod<std::string>("BK_COLOR");
    stDispInfo.strOutTiming       = this->pDb->GetMod<std::string>("DISP_OUT_TIMING");
    intLayerCnt                   = this->pDb->GetMod<int>("IN_LAYER_CNT");
    if ("panel" == stDispInfo.strDevType)
    {
        stDispInfo.strPnlLinkType     = this->pDb->GetMod<std::string>("MODE_PANEL_PARAM", "PNL_LINK_TYPE");
        AMIGOS_INFO("PNL_LINK_TYPE : %s\n", stDispInfo.strPnlLinkType.c_str());
    }
    AMIGOS_INFO("DEV_TYPE : %s\n", stDispInfo.strDevType.c_str());
    AMIGOS_INFO("BK_COLOR : %s\n", stDispInfo.strBackGroundColor.c_str());
    AMIGOS_INFO("LAYER_CNT : %d\n", intLayerCnt);
    if (intLayerCnt != -1)
    {
        for (i = 0; i < intLayerCnt; i++)
        {
            snprintf(strLayerName, 30, "IN_LAYER_%d", i);
            mapLayerInfo[i].uintId         = this->pDb->GetMod<unsigned int>(strLayerName, "LAYER_ID");
            mapLayerInfo[i].uintRot        = this->pDb->GetMod<unsigned int>(strLayerName, "LAYER_ROT");
            mapLayerInfo[i].uintWidth      = this->pDb->GetMod<unsigned int>(strLayerName, "LAYER_WIDTH");
            mapLayerInfo[i].uintHeight     = this->pDb->GetMod<unsigned int>(strLayerName, "LAYER_HEIGHT");
            mapLayerInfo[i].uintDispWidth  = this->pDb->GetMod<unsigned int>(strLayerName, "LAYER_DISP_WIDTH");
            mapLayerInfo[i].uintDispHeight = this->pDb->GetMod<unsigned int>(strLayerName, "LAYER_DISP_HEIGHT");
            mapLayerInfo[i].uintDispXpos   = this->pDb->GetMod<unsigned int>(strLayerName, "LAYER_DISP_XPOS");
            mapLayerInfo[i].uintDispYpos   = this->pDb->GetMod<unsigned int>(strLayerName, "LAYER_DISP_YPOS");
            AMIGOS_INFO("Layer %d info : LAYER_ROT %d\n", mapLayerInfo[i].uintId, mapLayerInfo[i].uintRot);
            AMIGOS_INFO("Layer %d info : LAYER_WIDTH %d\n", mapLayerInfo[i].uintId, mapLayerInfo[i].uintWidth);
            AMIGOS_INFO("Layer %d info : LAYER_HEIGHT %d\n", mapLayerInfo[i].uintId, mapLayerInfo[i].uintHeight);
            AMIGOS_INFO("Layer %d info : LAYER_DISP_WIDTH %d\n", mapLayerInfo[i].uintId, mapLayerInfo[i].uintDispWidth);
            AMIGOS_INFO("Layer %d info : LAYER_DISP_HEIGHT %d\n", mapLayerInfo[i].uintId,
                        mapLayerInfo[i].uintDispHeight);
            AMIGOS_INFO("Layer %d info : LAYER_DISP_XPOS %d\n", mapLayerInfo[i].uintId, mapLayerInfo[i].uintDispXpos);
            AMIGOS_INFO("Layer %d info : LAYER_DISP_YPOS %d\n", mapLayerInfo[i].uintId, mapLayerInfo[i].uintDispYpos);
            mapPortId[mapLayerInfo[i].uintId] = 0;
        }
    }
    for (auto itMapIn = this->mapModInputInfo.begin(); itMapIn != this->mapModInputInfo.end(); ++itMapIn)
    {
        intLayerId = this->pDb->GetIn<int>(itMapIn->second.curLoopId, "IN_LAYER_ID");
        if (intLayerId != -1)
        {
            if (mapLayerInfo.find(intLayerId) != mapLayerInfo.end())
            {
                DispLayerInputPortInfo     stDispLayerInputPortInfo;
                stDispLayerInputPortInfo.uintSrcWidth    = this->pDb->GetIn<unsigned int>(itMapIn->second.curLoopId, "SRC_WIDTH");
                stDispLayerInputPortInfo.uintSrcHeight   = this->pDb->GetIn<unsigned int>(itMapIn->second.curLoopId, "SRC_HEIGHT");
                stDispLayerInputPortInfo.uintDstWidth    = this->pDb->GetIn<unsigned int>(itMapIn->second.curLoopId, "DST_WIDTH");
                stDispLayerInputPortInfo.uintDstHeight   = this->pDb->GetIn<unsigned int>(itMapIn->second.curLoopId, "DST_HEIGHT");
                stDispLayerInputPortInfo.uintDstXpos     = this->pDb->GetIn<unsigned int>(itMapIn->second.curLoopId, "DST_XPOS");
                stDispLayerInputPortInfo.uintDstYpos     = this->pDb->GetIn<unsigned int>(itMapIn->second.curLoopId, "DST_YPOS");
                stDispLayerInputPortInfo.uintSysChn      = this->pDb->GetIn<unsigned int>(itMapIn->second.curLoopId, "SYS_CHN");
                stDispLayerInputPortInfo.uintLayerPortId = mapPortId[intLayerId]++;
                stDispLayerInputPortInfo.uintLayerId     = intLayerId;
                AMIGOS_INFO("In Layer %d port %d info : SYS_CHN %d\n", intLayerId, itMapIn->first,
                            stDispLayerInputPortInfo.uintSysChn);
                AMIGOS_INFO("In Layer %d port %d info : SRC_WIDTH %d\n", intLayerId, itMapIn->first,
                            stDispLayerInputPortInfo.uintSrcWidth);
                AMIGOS_INFO("In Layer %d port %d info : SRC_HEIGHT %d\n", intLayerId, itMapIn->first,
                            stDispLayerInputPortInfo.uintSrcHeight);
                AMIGOS_INFO("In Layer %d port %d info : DST_WIDTH %d\n", intLayerId, itMapIn->first,
                            stDispLayerInputPortInfo.uintDstWidth);
                AMIGOS_INFO("In Layer %d port %d info : DST_HEIGHT %d\n", intLayerId, itMapIn->first,
                            stDispLayerInputPortInfo.uintDstHeight);
                AMIGOS_INFO("In Layer %d port %d info : DST_XPOS %d\n", intLayerId, itMapIn->first,
                            stDispLayerInputPortInfo.uintDstXpos);
                AMIGOS_INFO("In Layer %d port %d info : DST_YPOS %d\n", intLayerId, itMapIn->first,
                            stDispLayerInputPortInfo.uintDstYpos);
                mapDispInInfo[itMapIn->first] = stDispLayerInputPortInfo;
            }
            else
            {
                AMIGOS_ERR("Layer did not create, pls check your config.\n");
            }
        }
    }
}

void AmigosSurfaceDisp::_UnloadDb()
{
    mapLayerInfo.clear();
    stDispInfo.Clear();
}

void AmigosSurfaceDisp::GetInfo(DispInfo &info, std::map<unsigned int, DispLayerInfo> &layerInfo) const
{
    info      = stDispInfo;
    layerInfo = mapLayerInfo;
}

void AmigosSurfaceDisp::SetInfo(const DispInfo &info, const std::map<unsigned int, DispLayerInfo> &layerInfo)
{
    stDispInfo   = info;
    mapLayerInfo = layerInfo;
}

