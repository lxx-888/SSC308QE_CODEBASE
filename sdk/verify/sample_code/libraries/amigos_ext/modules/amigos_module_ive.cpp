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
#include <sys/select.h>
#include <cstring>
#include <unistd.h>
#include "ss_enum_cast.hpp"
#include "amigos_module_init.h"
#include "amigos_module_ive.h"

constexpr const static unsigned int    IVE_INPORT_ORI = 0;
constexpr const static unsigned int    IVE_INPORT_REP = 1;

SS_ENUM_CAST_STR(MI_IVE_BgBlurMode_e,
{
    { E_MI_IVE_BGBLUR_MODE_BLUR, "blur" },
    { E_MI_IVE_BGBLUR_MODE_REPLACE, "replace" },
    { E_MI_IVE_BGBLUR_MODE_MOSAIC, "mosaic" },
    { E_MI_IVE_BGBLUR_MODE_BLUR_AND_MOSAIC, "blur_mosaic" },
});
SS_ENUM_CAST_STR(MI_IVE_BgBlurMaskOp_e,
{
    { E_MI_IVE_BGBLUR_MASK_OP_DILATE, "dilate" },
    { E_MI_IVE_BGBLUR_MASK_OP_NONE, "none" },
    { E_MI_IVE_BGBLUR_MASK_OP_ERODE, "erode" },
});

static int SetPacketToIveImage(stream_packet_obj &packet, MI_IVE_SrcImage_t &stImg)
{
    stream_packet_obj pa = stream_packet_base::convert(packet, EN_RAW_FRAME_DATA_PA);
    if (!pa)
    {
        AMIGOS_ERR("Packet %s error, can not convert to PA.\n", packet->get_type().c_str());
        return -1;
    }
    switch (pa->raw_vid_i.plane_info[0].fmt)
    {
    case RAW_FORMAT_YUV420SP:
        stImg.eType = E_MI_IVE_IMAGE_TYPE_YUV420SP;
        break;
    case RAW_FORMAT_YUV422_YUYV:
        stImg.eType = E_MI_IVE_IMAGE_TYPE_YUV422_YUYV;
        break;
    default:
        AMIGOS_ERR("ive bgblur failed input data fmt err: %d\n", pa->raw_vid_i.plane_info[0].fmt);
        return -1;
    }
    stImg.u16Width  = pa->raw_vid_i.plane_info[0].width;
    stImg.u16Height = pa->raw_vid_i.plane_info[0].height;
    for (int i  = 0; i < 3; i++)
    {
        stImg.apu8VirAddr[i] = (MI_U8 *)NULL;
        stImg.aphyPhyAddr[i] = pa->raw_vid_pa.plane_data[0].phy[i];
        stImg.azu16Stride[i] = pa->raw_vid_pa.plane_data[0].stride[i];
    }
    return 0;
}

static int ReadFileData(std::string& strPath, char *pData, unsigned int uiSize)
{
    unsigned int size = 0;
    unsigned int readSize = 0;
    auto readFd = open(strPath.c_str(), O_RDONLY);
    if (readFd < 0)
    {
        AMIGOS_ERR("open File %s failed\n", strPath.c_str());
        return -1;
    }
    off_t curr = lseek(readFd, 0, SEEK_CUR);
    off_t end  = lseek(readFd, 0, SEEK_END);
    size = end - curr;
    if (size > uiSize)
    {
        AMIGOS_WRN("file %s size(%d) is large than raw size(%d), use raw size\n", strPath.c_str(), size, uiSize);
        size = uiSize;
    }
    off_t offset = lseek(readFd, 0, SEEK_SET);
    readSize = (unsigned int)read(readFd, pData, size);
    if (size != readSize)
    {
        AMIGOS_ERR("read Bad file : %s, need read size: %d, read size : %d, offset = %d\n", strPath.c_str(), size, readSize, offset);
        close(readFd);
        return -1;
    }
    close(readFd);
    return 0;
}

AmigosModuleIve::LinkerBgBlurRepBgNegative::LinkerBgBlurRepBgNegative(const IveInfo &iveInfo, MI_IVE_HANDLE handle, ss_linker_base *linker)
    : iIveHandle(handle)
    , dstLinker(linker)
    , oriPacket(nullptr)
    , bOnBgBlur(true)
{
    memset(&iveCtrl, 0, sizeof(MI_IVE_BgBlurCtrl_t));
    iveCtrl.eBgBlurMode = ss_enum_cast<MI_IVE_BgBlurMode_e>::from_str(iveInfo.strIveMode);
    iveCtrl.eBgBlurMaskOp = ss_enum_cast<MI_IVE_BgBlurMaskOp_e>::from_str(iveInfo.strMaskOp);
    iveCtrl.u8MaskThr = (MI_U8)iveInfo.uiMaskThr;
    iveCtrl.u8BlurLv = (MI_U8)iveInfo.uiBlurLv;
    iveCtrl.u8ScalingStage = (MI_U8)iveInfo.uiScalingStage;
    iveCtrl.u8SaturationLv = (MI_U8)iveInfo.uiSaturationLv;
    iveCtrl.u8MosaicSize = (MI_U8)iveInfo.uiMosaicSize;
}
int AmigosModuleIve::LinkerBgBlurRepBgNegative::enqueue(stream_packet_obj &packet)
{
    MI_IVE_BgBlurCtrl_t bgCtrl;
    stream_packet_obj ori = oriPacket;
    oriPacket = nullptr;
    if (!ori || !packet)
    {
        return -1;
    }
    if (!bOnBgBlur)
    {
        return this->dstLinker->enqueue(ori);
    }
    {
        std::unique_lock<std::mutex> lock(mutexCtrl);
        bgCtrl = iveCtrl;
    }

    MI_IVE_SrcImage_t stOriImg, stRepImg;
    memset(&stOriImg, 0, sizeof(MI_IVE_SrcImage_t));
    memset(&stRepImg, 0, sizeof(MI_IVE_SrcImage_t));
    if (SetPacketToIveImage(ori, stOriImg) != 0)
    {
        return -1;
    }
    if (SetPacketToIveImage(packet, stRepImg) != 0)
    {
        return -1;
    }
    int ret = MI_IVE_BGBlur(iIveHandle, &srcYImage.miIveImg, &srcUvImage.miIveImg, &stOriImg, &stRepImg, &stOriImg, &bgCtrl);
    if (MI_SUCCESS != ret)
    {
        AMIGOS_ERR("MI_IVE_BGBlur() return ERROR 0x%X\n", ret);
        return -1;
    }
    return this->dstLinker->enqueue(ori);
}

void AmigosModuleIve::LinkerBgBlurRepBgNegative::SetCtrlParam(std::string& strParam, AMIGOS_MOD_IVE_ParamType_t type)
{
    std::unique_lock<std::mutex> lock(mutexCtrl);
    switch (type)
    {
    case E_MOD_IVE_PARAM_TYPE_MODE:
        iveCtrl.eBgBlurMode = ss_enum_cast<MI_IVE_BgBlurMode_e>::from_str(strParam);
        break;
    case E_MOD_IVE_PARAM_TYPE_OP:
        iveCtrl.eBgBlurMaskOp = ss_enum_cast<MI_IVE_BgBlurMaskOp_e>::from_str(strParam);
        break;
    case E_MOD_IVE_PARAM_TYPE_THR:
        iveCtrl.u8MaskThr = atoi(strParam.c_str());
        break;
    case E_MOD_IVE_PARAM_TYPE_LV:
        iveCtrl.u8BlurLv = atoi(strParam.c_str());
        break;
    case E_MOD_IVE_PARAM_TYPE_STAGE:
        iveCtrl.u8ScalingStage = atoi(strParam.c_str());
        break;
    case E_MOD_IVE_PARAM_TYPE_SATURATIONLV:
        iveCtrl.u8SaturationLv = atoi(strParam.c_str());
        break;
    case E_MOD_IVE_PARAM_TYPE_MOSAICSIZE:
        iveCtrl.u8MosaicSize = atoi(strParam.c_str());
        break;
    default:
        AMIGOS_ERR("reset ctrl param failed! type is : %d, param: %s\n", type, strParam.c_str());
        return;
    }
    AMIGOS_INFO("reset ctrl param type : %d, param: %s\n", type, strParam.c_str());
}

void AmigosModuleIve::LinkerBgBlurRepBgNegative::DebugBgblurOnOff(std::string& strOnOff)
{
    AMIGOS_INFO("debug Bgblur OnOff: %s\n", strOnOff.c_str());
    if ("on" == strOnOff)
    {
        bOnBgBlur = true;
        return;
    }
    bOnBgBlur = false;
}

void AmigosModuleIve::SetCtrlParam(std::string& strParam, LinkerBgBlurRepBgNegative::AMIGOS_MOD_IVE_ParamType_t type)
{
    if (nullptr == this->bgBlurRepLinker)
    {
        AMIGOS_ERR("ive port: %d, has no input port negative!, Set param failed!\n", IVE_INPORT_REP);
        return;
    }
    this->bgBlurRepLinker->SetCtrlParam(strParam, type);
}

void AmigosModuleIve::DebugIveOnOff(std::string& strOnOff)
{
    if ("bgblur" == stIveInfo.strIveType && nullptr != this->bgBlurRepLinker)
    {
        this->bgBlurRepLinker->DebugBgblurOnOff(strOnOff);
    }
}

AmigosModuleIve::AmigosModuleIve(const std::string &strSection)
    : AmigosSurfaceIve(strSection)
    , AmigosModuleMiBase(this)
    , iIveHandle(MI_IVE_HANDLE_MAX)
    , bgBlurOriLinker(nullptr)
    , bgBlurRepLinker(nullptr)
{
}

AmigosModuleIve::~AmigosModuleIve()
{
}

void AmigosModuleIve::_Init()
{
    iIveHandle = MI_IVE_Create_Handle();
    if (iIveHandle < 0 || iIveHandle >= MI_IVE_HANDLE_MAX)
    {
        AMIGOS_ERR("Ive init failed, ret: %x!\n", iIveHandle);
        return;
    }
    if ("bgblur" == stIveInfo.strIveType)
    {
        auto it = mapPortOut.begin();
        if (mapPortOut.end() == it)
        {
            AMIGOS_ERR("ive bgblur has no output port!, init failed!\n");
            return;
        }
        this->bgBlurRepLinker =  new LinkerBgBlurRepBgNegative(this->stIveInfo, iIveHandle, &it->second.positive);
        if (nullptr == this->bgBlurRepLinker)
        {
            AMIGOS_ERR("new rep linker failed!\n");
            return;
        }
        this->bgBlurOriLinker =  new LinkerBgBlurOriNegative(this->bgBlurRepLinker);
        if (nullptr == this->bgBlurOriLinker)
        {
            AMIGOS_ERR("new ori linker failed!\n");
            delete this->bgBlurRepLinker;
            this->bgBlurRepLinker = nullptr;
            return;
        }
    }
    AMIGOS_INFO("ive init ok, handle: %d, ive Type: %s\n", iIveHandle, stIveInfo.strIveType.c_str());
}

void AmigosModuleIve::_Deinit()
{
    if (nullptr != this->bgBlurRepLinker)
    {
        delete this->bgBlurRepLinker;
        this->bgBlurRepLinker = nullptr;
    }
    if (nullptr != this->bgBlurOriLinker)
    {
        delete this->bgBlurOriLinker;
        this->bgBlurOriLinker = nullptr;
    }
    if (iIveHandle < 0 || iIveHandle >= MI_IVE_HANDLE_MAX)
    {
        AMIGOS_ERR("Ive deinit failed, handle: %x!\n", iIveHandle);
        return;
    }
    MI_IVE_Destroy(iIveHandle);
    AMIGOS_INFO("ive deinit ok, handle: %d\n", iIveHandle);
    iIveHandle = MI_IVE_HANDLE_MAX;
}

unsigned int AmigosModuleIve::GetModId() const
{
    return uintExtModId;
}

unsigned int AmigosModuleIve::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}

unsigned int AmigosModuleIve::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}

ss_linker_base *AmigosModuleIve::_CreateInputNegativeLinker(unsigned int inPortId)
{
    if (iIveHandle < 0 || iIveHandle >= MI_IVE_HANDLE_MAX)
    {
        AMIGOS_ERR("Ive _CreateInputNegativeLinker failed, handle: %x!\n", iIveHandle);
        return nullptr;
    }
    if ("bgblur" ==  stIveInfo.strIveType)
    {
        if (IVE_INPORT_ORI == inPortId)
        {
            return new AmigosModuleBase::LinkerBypass(this->bgBlurOriLinker);
        }
        if (IVE_INPORT_REP == inPortId)
        {
            return new AmigosModuleBase::LinkerBypass(this->bgBlurRepLinker);
        }
    }
    return nullptr;
}
void AmigosModuleIve::_Start()
{
    if ("bgblur" == stIveInfo.strIveType)
    {
        if (nullptr == this->bgBlurRepLinker)
        {
            AMIGOS_ERR("new linker failed!\n");
            return;
        }
        auto itRep = mapPortIn.find(IVE_INPORT_REP);
        if (mapPortIn.end() == itRep)
        {
            AMIGOS_ERR("ive bgblur port: %d, has no input port!, Start failed!\n", IVE_INPORT_REP);
            return;
        }
        auto info = itRep->second.get_packet_info();
        info.raw_vid_i.plane_num = 1;
        info.raw_vid_i.plane_info[0].fmt = RAW_FORMAT_I8;
        stream_packet_obj packetY = stream_packet_base::make<StreamPacketSysMma>(info.raw_vid_i);
        //Default SysMma packet using the RAW PA packet, and it should be converted to VA packet.
        packetY = stream_packet_base::convert(packetY, EN_RAW_FRAME_DATA);
        if (!packetY)
        {
            AMIGOS_ERR("new Y mma buf failed! ive bgblur Start failed!\n");
            return;
        }
        if (0 != ReadFileData(stIveInfo.strYMaskPath, (char *)packetY->raw_vid.plane_data[0].data[0], packetY->raw_vid.plane_data[0].size[0]))
        {
            AMIGOS_ERR("read Y map buf failed! ive bgblur start failed!\n");
            return;
        }
        info.raw_vid_i.plane_info[0].width /= 2;
        info.raw_vid_i.plane_info[0].height /= 2;
        stream_packet_obj packetUv = stream_packet_base::make<StreamPacketSysMma>(info.raw_vid_i);
        //Default SysMma packet using the RAW PA packet, and it should be converted to VA packet.
        packetUv = stream_packet_base::convert(packetUv, EN_RAW_FRAME_DATA);
        if (!packetUv)
        {
            AMIGOS_ERR("new UV mma buf failed! ive bgblur Start failed!\n");
            return;
        }
        if (0 != ReadFileData(stIveInfo.strUvMaskPath, (char *)packetUv->raw_vid.plane_data[0].data[0], packetUv->raw_vid.plane_data[0].size[0]))
        {
            AMIGOS_ERR("read uv mask buf failed! ive bgblur Start failed!\n");
            return;
        }
        this->bgBlurRepLinker->SetImage(packetY, packetUv);
    }
}

bool AmigosModuleIve::_NeedResetStreamTraverse(unsigned int outPortId, unsigned int bindType,
        unsigned int &inPortId)
{
    auto it = mapPortIn.find(IVE_INPORT_ORI);
    if (mapPortIn.end() == it)
    {
        AMIGOS_ERR("ive chn has no this input(%d) port!!!\n", IVE_INPORT_ORI);
        return false;
    }
    inPortId = IVE_INPORT_ORI;
    return true;
}

stream_packet_info AmigosModuleIve::_GetStreamInfo(unsigned int outPortId)
{
    if ("bgblur" ==  stIveInfo.strIveType)
    {
        auto it = mapPortIn.find(IVE_INPORT_ORI);
        if (mapPortIn.end() == it)
        {
            AMIGOS_ERR("ive chn has no this input(%d) port!!!\n", IVE_INPORT_ORI);
            return stream_packet_info();
        }
        return it->second.get_packet_info();
    }
    return stream_packet_info();
}

AMIGOS_MODULE_INIT("IVE", AmigosModuleIve);
