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
#include <unistd.h>
#include <string.h>
#include "mi_isp.h"
#include "mi_isp_cus3a_api.h"
#include "amigos_module_init.h"
#include "amigos_module_isp.h"
#include "mi_sys_datatype.h"
#include "mi_isp_datatype.h"
#include "ss_enum_cast.hpp"
#include "CusEarlyInit_para.h"

SS_ENUM_CAST_STR(MI_ISP_AiBnrSourceType_e,
{
    { E_MI_ISP_AI_BNR_SOURCE_TYPE_NONE, "none" },
    { E_MI_ISP_AI_BNR_SOURCE_TYPE_LINEAR, "linear" },
    { E_MI_ISP_AI_BNR_SOURCE_TYPE_HDR_FUSION, "fusion" },
    { E_MI_ISP_AI_BNR_SOURCE_TYPE_HDR_LONG, "long" },
    { E_MI_ISP_AI_BNR_SOURCE_TYPE_MAX, "max" },
});

std::map<unsigned int, unsigned int> AmigosModuleIsp::mapIspCreateDev;

AmigosModuleIsp::AmigosModuleIsp(const std::string &strInSection)
    : AmigosSurfaceIsp(strInSection), AmigosModuleMiBase(this)
{
}
AmigosModuleIsp::~AmigosModuleIsp()
{
}
unsigned int AmigosModuleIsp::GetModId() const
{
    return E_MI_MODULE_ID_ISP;
}
unsigned int AmigosModuleIsp::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_KERNEL | E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleIsp::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_KERNEL | E_MOD_PORT_TYPE_USER;
}
void AmigosModuleIsp::_ResourceInit()
{
    auto itMapIspCreateDev = mapIspCreateDev.find(stModInfo.devId);
    if (itMapIspCreateDev == mapIspCreateDev.end())
    {
        mapIspCreateDev[stModInfo.devId] = 0;
    }
    mapIspCreateDev[stModInfo.devId]++;
}
void AmigosModuleIsp::_ResourceDeinit()
{
    auto itMapIspCreateDev = mapIspCreateDev.find(stModInfo.devId);
    if (itMapIspCreateDev != mapIspCreateDev.end())
    {
        itMapIspCreateDev->second--;
        if (!itMapIspCreateDev->second)
        {
            mapIspCreateDev.erase(itMapIspCreateDev);
        }
    }
}
void AmigosModuleIsp::_Init()
{
    MI_ISP_ChannelAttr_t stChannelIspAttr;
    MI_ISP_ChnParam_t stChannelIspParam;
    MI_SYS_WindowRect_t stCapRect;
    MI_ISP_AiBnrAttr_t  stAiBnrAttr;

    MasterEarlyInitParam_t *pstEarlyInitParam = NULL;
    std::map<unsigned int, IspInInfo>::iterator itMapIspIn;
    std::map<unsigned int, unsigned int>::iterator itMapIspCreateDev;

    itMapIspCreateDev = mapIspCreateDev.find(stModInfo.devId);
    if (itMapIspCreateDev == mapIspCreateDev.end())
    {
        MI_ISP_DevAttr_t stIspDevAttr;

        memset(&stIspDevAttr, 0, sizeof(MI_ISP_DevAttr_t));
        stIspDevAttr.u32DevStitchMask = stIspInfo.uintStitchMask;
        MI_ISP_CreateDevice((MI_ISP_DEV)stModInfo.devId, &stIspDevAttr);
        mapIspCreateDev[stModInfo.devId] = 0;
    }
    mapIspCreateDev[stModInfo.devId]++;
    memset(&stChannelIspAttr, 0, sizeof(MI_ISP_ChannelAttr_t));
    stChannelIspAttr.u32SensorBindId = stIspInfo.uintSensorId;
    stChannelIspAttr.u32Sync3AType = stIspInfo.uintSync3AType;
    if(stIspInfo.uintCustIqEn)
    {
        pstEarlyInitParam = (MasterEarlyInitParam_t*)&stChannelIspAttr.stIspCustIqParam.stVersion.u8Data[0];
        pstEarlyInitParam->u16SnrEarlyFps = stIspInfo.CustIqParam.SnrEarlyFps;
        pstEarlyInitParam->u16SnrEarlyFlicker = stIspInfo.CustIqParam.SnrEarlyFlicker;
        pstEarlyInitParam->u32SnrEarlyShutter = stIspInfo.CustIqParam.SnrEarlyShutter;
        pstEarlyInitParam->u32SnrEarlyGainX1024 = stIspInfo.CustIqParam.SnrEarlyGainX1024;
        pstEarlyInitParam->u32SnrEarlyDGain = stIspInfo.CustIqParam.SnrEarlyDGain;
        pstEarlyInitParam->u16SnrEarlyAwbRGain = stIspInfo.CustIqParam.SnrEarlyAwbRGain;
        pstEarlyInitParam->u16SnrEarlyAwbGGain = stIspInfo.CustIqParam.SnrEarlyAwbGGain;
        pstEarlyInitParam->u16SnrEarlyAwbBGain = stIspInfo.CustIqParam.SnrEarlyAwbBGain;
        stChannelIspAttr.stIspCustIqParam.stVersion.u32Revision = stIspInfo.CustIqParam.Revision;
        stChannelIspAttr.stIspCustIqParam.stVersion.u32Size = sizeof(MasterEarlyInitParam_t);
    }
    if(stIspInfo.uintMutichnEn)
    {
        MI_ISP_CreateChannel((MI_ISP_DEV)stModInfo.devId, MI_ISP_MULTI_CHN_MASK |stModInfo.chnId, &stChannelIspAttr);
    }
    else
    {
        MI_ISP_CreateChannel((MI_ISP_DEV)stModInfo.devId, stModInfo.chnId, &stChannelIspAttr);
    }
    MI_ISP_ApiCmdLoadBinFile(stModInfo.devId, stModInfo.chnId, (char*)stIspInfo.apiFileName.c_str(), 1234);
    if(stIspInfo.uintSubChnIqEn)
    {
        MI_ISP_ApiCmdLoadBinFile(stIspInfo.SubChnIqParam.dev, stIspInfo.SubChnIqParam.chn, (char*)stIspInfo.SubChnIqParam.apiFileName.c_str(), 1234);
    }
    memset(&stChannelIspParam, 0, sizeof(MI_ISP_ChnParam_t));
    stChannelIspParam.eHDRType = (MI_ISP_HDRType_e)stIspInfo.uintHdrType;
    stChannelIspParam.eHDRFusionType = (MI_ISP_HDRFusionType_e)stIspInfo.uintHdrFusionType;
    stChannelIspParam.u16HDRExposureMask = stIspInfo.uintHdrExposureMask;
    stChannelIspParam.e3DNRLevel = (MI_ISP_3DNR_Level_e)stIspInfo.uint3dNrLevel;
    stChannelIspParam.bMirror = (MI_BOOL)stIspInfo.uintIsMirror;
    stChannelIspParam.bFlip = (MI_BOOL) stIspInfo.uintIsFlip;
    stChannelIspParam.eRot = (MI_SYS_Rotate_e)stIspInfo.uintRotation;
    stChannelIspParam.bLdcEnable = stIspInfo.uintIspLdcEn ? TRUE : FALSE;
    MI_ISP_SetChnParam((MI_ISP_DEV)stModInfo.devId, stModInfo.chnId, &stChannelIspParam);
    //ai bnr
    memset(&stAiBnrAttr, 0, sizeof(MI_ISP_AiBnrAttr_t));
    stAiBnrAttr.eAiSourceType = ss_enum_cast<MI_ISP_AiBnrSourceType_e>::from_str(stIspInfo.aiBnrParam.strBnrSrcType);
    if (stIspInfo.uintAibnrEn && E_MI_ISP_AI_BNR_SOURCE_TYPE_NONE != stAiBnrAttr.eAiSourceType)
    {
        MI_ISP_AiIspInit((MI_ISP_DEV)stModInfo.devId, stModInfo.chnId);
        MI_ISP_LoadAiBnrModel((MI_ISP_DEV)stModInfo.devId, stModInfo.chnId, (char*)stIspInfo.aiBnrParam.bnrModelFileName.c_str());
        MI_ISP_SetAiBnrAttr((MI_ISP_DEV)stModInfo.devId, stModInfo.chnId, &stAiBnrAttr);
        AMIGOS_INFO("set ai bnr src type: %d,  modol: %s\n", stAiBnrAttr.eAiSourceType, stIspInfo.aiBnrParam.bnrModelFileName.c_str());
    }
    if(stIspInfo.uintZoomEn)
    {
        MI_ISP_ZoomTable_t stZoomTable;
        MI_ISP_ZoomAttr_t stZoomAttr;
        memset(&stZoomTable, 0x0, sizeof(MI_ISP_ZoomTable_t));
        memset(&stZoomAttr, 0x0, sizeof(MI_ISP_ZoomAttr_t));
        stZoomTable.u32EntryNum = stIspInfo.ZoomParam.TableNum;
        stZoomTable.pVirTableAddr = new MI_ISP_ZoomEntry_t[stZoomTable.u32EntryNum];
        stIspInfo.ZoomParam.pTableAddr = reinterpret_cast<char*>(stZoomTable.pVirTableAddr);
        for(MI_U32 i = 0; i < stZoomTable.u32EntryNum; i++)
        {
            stZoomTable.pVirTableAddr[i].u8ZoomSensorId = stIspInfo.ZoomParam.Table[i].SnrId;
            stZoomTable.pVirTableAddr[i].stCropWin.u16X = stIspInfo.ZoomParam.Table[i].TableX;
            stZoomTable.pVirTableAddr[i].stCropWin.u16Y = stIspInfo.ZoomParam.Table[i].TableY;
            stZoomTable.pVirTableAddr[i].stCropWin.u16Width = stIspInfo.ZoomParam.Table[i].TableW;
            stZoomTable.pVirTableAddr[i].stCropWin.u16Height = stIspInfo.ZoomParam.Table[i].TableH;
        }
        stZoomAttr.u32FromEntryIndex = stIspInfo.ZoomParam.FromEntryIndex;
        stZoomAttr.u32ToEntryIndex = stIspInfo.ZoomParam.ToEntryIndex;
        MI_ISP_LoadPortZoomTable((MI_ISP_DEV)stModInfo.devId, stModInfo.chnId, &stZoomTable);
        MI_ISP_StartPortZoom((MI_ISP_DEV)stModInfo.devId, stModInfo.chnId, &stZoomAttr);
    }
    if(stIspInfo.uintIspLdcEn)
    {
        MI_ISP_LdcAttr_t stLdcAttr;
        stLdcAttr.u32CenterXOffset = stIspInfo.IspLdcParam.CenterX;
        stLdcAttr.u32CenterYOffset = stIspInfo.IspLdcParam.CenterY;
        stLdcAttr.s32Alpha         = stIspInfo.IspLdcParam.Alpha;
        stLdcAttr.s32Beta          = stIspInfo.IspLdcParam.Beta;
        stLdcAttr.u32CropLeft      = stIspInfo.IspLdcParam.CropL;
        stLdcAttr.u32CropRight     = stIspInfo.IspLdcParam.CropR;
        MI_ISP_SetLdcAttr((MI_ISP_DEV)stModInfo.devId, stModInfo.chnId, &stLdcAttr);
    }
    if(stIspInfo.uintIspOverlapEn)
    {
        MI_ISP_Overlap_e eOverlap;
        eOverlap = ss_enum_cast<MI_ISP_Overlap_e>::from_str(stIspInfo.OverLapParam.strOverlap);
        MI_ISP_SetChnOverlapAttr((MI_ISP_DEV)stModInfo.devId, stModInfo.chnId, eOverlap);
    }
    if(stIspInfo.uintSnrMaskNum > 1)
    {
        AMIGOS_INFO("^^___^^ u32Sensor stitch num:%d.\n",stIspInfo.uintSnrMaskNum);
        int j=0,devid=0;
        MI_U32 u32SubChId = 0;
        for(j = 0; j < E_MI_ISP_SENSOR_MAX; j++)
        {
            if(stChannelIspAttr.u32SensorBindId & (0x1 << j))
            {
                for(devid = 0; devid <= MI_ISP_DEV1; devid++)
                {
                    u32SubChId = 0;
                    if(MI_SUCCESS == MI_ISP_GetSubChnId(devid, stModInfo.chnId, (MI_ISP_BindSnrId_e)(0x1 << j), &u32SubChId))
                    {
                        MI_ISP_ChnParam_t tChnParam;
                        MI_U8 mainDev = 0; //subch device only used on iq and set subch param, by mervyn.chen
                        memset(&tChnParam, 0x0, sizeof(tChnParam));
                        MI_ISP_GetChnParam(mainDev, stModInfo.chnId,  &tChnParam);
                        tChnParam.eRot = stChannelIspParam.eRot;
                        MI_ISP_SetSubChnParam(devid, stModInfo.chnId, u32SubChId, &tChnParam);
                    }
                }
            }
        }
    }
    for (itMapIspIn = mapIspIn.begin(); itMapIspIn != mapIspIn.end(); itMapIspIn++)
    {
        if (mapModInputInfo[itMapIspIn->first].bindType != E_MI_SYS_BIND_TYPE_FRAME_BASE)
        {
            continue;
        }
        stCapRect.u16X = itMapIspIn->second.uintIspInCropX;
        stCapRect.u16Y = itMapIspIn->second.uintIspInCropY;
        stCapRect.u16Width = itMapIspIn->second.uintIspInWidth;
        stCapRect.u16Height = itMapIspIn->second.uintIspInHeight;
        if (!stCapRect.u16Width || !stCapRect.u16Height)
            continue;
        MI_ISP_SetInputPortCrop((MI_ISP_DEV)stModInfo.devId, stModInfo.chnId, &stCapRect);
    }
}
void AmigosModuleIsp::_Deinit()
{
    std::map<unsigned int, unsigned int>::iterator itMapIspCreateDev;

    if(stIspInfo.uintZoomEn)
    {
        delete[] reinterpret_cast<MI_ISP_ZoomEntry_t*>(stIspInfo.ZoomParam.pTableAddr);
        stIspInfo.ZoomParam.pTableAddr = NULL;
        MI_ISP_StopPortZoom((MI_ISP_DEV)stModInfo.devId, stModInfo.chnId);
    }
    //ai bnr
    if (stIspInfo.uintAibnrEn && E_MI_ISP_AI_BNR_SOURCE_TYPE_NONE != ss_enum_cast<MI_ISP_AiBnrSourceType_e>::from_str(stIspInfo.aiBnrParam.strBnrSrcType))
    {
        MI_ISP_UnloadAiBnrModel((MI_ISP_DEV)stModInfo.devId, stModInfo.chnId);
    }
    MI_ISP_DestroyChannel((MI_ISP_DEV)stModInfo.devId, (MI_ISP_CHANNEL)stModInfo.chnId);
    itMapIspCreateDev = mapIspCreateDev.find(stModInfo.devId);
    if (itMapIspCreateDev != mapIspCreateDev.end())
    {
        itMapIspCreateDev->second--;
        if (!itMapIspCreateDev->second)
        {
            MI_ISP_DestoryDevice((MI_ISP_DEV)stModInfo.devId);
            mapIspCreateDev.erase(itMapIspCreateDev);
        }
    }
}

void AmigosModuleIsp::_Start()
{
    MI_ISP_StartChannel((MI_ISP_DEV)stModInfo.devId, (MI_ISP_CHANNEL)stModInfo.chnId);
}

void AmigosModuleIsp::_Stop()
{
    MI_ISP_StopChannel((MI_ISP_DEV)stModInfo.devId, (MI_ISP_CHANNEL)stModInfo.chnId);
}

void AmigosModuleIsp::_StartMiOut(unsigned int outPortId)
{
    MI_ISP_OutPortParam_t stIspOutputParam;
    std::map<unsigned int, IspOutInfo>::iterator   itMapIspOut;

    itMapIspOut = mapIspOut.find(outPortId);
    memset(&stIspOutputParam, 0, sizeof(MI_ISP_OutPortParam_t));
    stIspOutputParam.stCropRect.u16X = itMapIspOut->second.uintIspOutCropX;
    stIspOutputParam.stCropRect.u16Y = itMapIspOut->second.uintIspOutCropY;
    stIspOutputParam.stCropRect.u16Width = itMapIspOut->second.uintIspOutCropW;
    stIspOutputParam.stCropRect.u16Height = itMapIspOut->second.uintIspOutCropH;
    stIspOutputParam.ePixelFormat = ss_enum_cast<MI_SYS_PixelFormat_e>::from_str(itMapIspOut->second.strOutFmt);
    stIspOutputParam.eCompressMode = (MI_SYS_CompressMode_e)itMapIspOut->second.uintCompressMode;
    stIspOutputParam.eBufLayout = (MI_ISP_BufferLayout_e)itMapIspOut->second.uintBufLayout;
    MI_ISP_SetOutputPortParam((MI_ISP_DEV)stModInfo.devId, (MI_ISP_CHANNEL)stModInfo.chnId, outPortId, &stIspOutputParam);
    MI_ISP_EnableOutputPort((MI_ISP_DEV)stModInfo.devId, (MI_ISP_CHANNEL)stModInfo.chnId, outPortId);
    AMIGOS_INFO("ISP ENABLE dev%d chn %d port %d\n", stModInfo.devId, stModInfo.chnId, outPortId);
}

void AmigosModuleIsp::_StopMiOut(unsigned int outPortId)
{
    AMIGOS_INFO("ISP DISABLE dev%d chn %d port %d\n", stModInfo.devId, stModInfo.chnId, outPortId);
    MI_ISP_DisableOutputPort((MI_ISP_DEV)stModInfo.devId, (MI_ISP_CHANNEL)stModInfo.chnId, outPortId);
}
stream_packet_info AmigosModuleIsp::_GetStreamInfo(unsigned int outPortId)
{
    stream_packet_info streamInfo;
    auto itMapIspOut = this->mapIspOut.find(outPortId);
    if (itMapIspOut == this->mapIspOut.end())
    {
        AMIGOS_ERR("Can not find isp output port %d info!\n", outPortId);
        return streamInfo;
    }
    streamInfo.en_type = ss_enum_cast<stream_type>::from_str(itMapIspOut->second.strOutType);
    streamInfo.raw_vid_i.plane_num = 1;
    streamInfo.raw_vid_i.plane_info[0].width  = itMapIspOut->second.uintIspOutCropW;
    streamInfo.raw_vid_i.plane_info[0].height = itMapIspOut->second.uintIspOutCropH;
    return streamInfo;
}
#ifdef CONFIG_MI_SYSCALL_L2R_DESTROY_SUPPORT
bool AmigosModuleIsp::_NeedMarkDeinitOnRtos()
{
    return true;
}
bool AmigosModuleIsp::_NeedMarkStopOnRtos()
{
    return true;
}
bool AmigosModuleIsp::_NeedMarkUnbindOnRtos(unsigned int inPortId)
{
    return true;
}
bool AmigosModuleIsp::_NeedMarkStopOutOnRtos(unsigned int outPortId)
{
    return true;
}
#endif

AMIGOS_MODULE_INIT("ISP", AmigosModuleIsp);
