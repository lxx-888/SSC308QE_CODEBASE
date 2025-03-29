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

#include "amigos_module_init.h"
#include "amigos_module_hseg.h"
#include "amigos_module_mi_base.h"
#include "ss_thread.h"
#include "algo_hseg_api.h"


SS_ENUM_CAST_STR(ALGO_HsegBgbMode_e,
{
    { E_ALOG_BGBLUR_MODE_BLUR, "blur" },
    { E_ALGO_BGBLUR_MODE_REPLACE, "replace" },
});

SS_ENUM_CAST_STR(ALGO_BgBlurMaskOp_e,
{
    { E_ALGO_BGB_MASK_OP_DILATE, "dilate" },
    { E_ALGO_BGB_MASK_OP_NONE, "none" },
    { E_ALGO_BGB_MASK_OP_ERODE, "erode" },
});

constexpr const static char*      HSEG_THREAD_SENDTODLA = "_SendToDla";
constexpr const static char*      HSEG_THREAD_DOBGBLUR = "_BgBlur";
constexpr const static unsigned int     HSEG_INPORT_ORI = 0;
constexpr const static unsigned int     HSEG_INPORT_REP = 1;
constexpr const static unsigned int     HSEG_INPORT_SEND = 2;

AmigosModuleHseg::AmigosModuleHseg(const std::string &strSection)
    : AmigosSurfaceHseg(strSection)
    , AmigosModuleMiBase(this)
    , hsegHandle(nullptr)
    , outThreadHandle(nullptr)
    , dlaThreadHandle(nullptr)
    , oriPacketObj(nullptr)
    , repPacketObj(nullptr)
    , oriLinker(false, 8)
    , repLinker(false, 8)
{
}

AmigosModuleHseg::~AmigosModuleHseg()
{
}

void AmigosModuleHseg::SetCtrlParam(std::string& strParam, AMIGOS_MOD_HSEG_ParamType_t type)
{
    switch (type)
    {
    case E_MOD_HSEG_PARAM_TYPE_MODE:
        blurCtrlParam.bgblurMode = ss_enum_cast<ALGO_HsegBgbMode_e>::from_str(strParam);
        break;
    case E_MOD_HSEG_PARAM_TYPE_OP:
        blurCtrlParam.maskOp = ss_enum_cast<ALGO_BgBlurMaskOp_e>::from_str(strParam);
        break;
    case E_MOD_HSEG_PARAM_TYPE_THR:
        blurCtrlParam.maskThredhold = atoi(strParam.c_str());
        break;
    case E_MOD_HSEG_PARAM_TYPE_LV:
        blurCtrlParam.blurLevel = atoi(strParam.c_str());
        break;
    case E_MOD_HSEG_PARAM_TYPE_STAGE:
        blurCtrlParam.scalingStage = atoi(strParam.c_str());
        break;
    default:
        AMIGOS_ERR("reset ctrl param failed! type is : %d, param: %s\n", type, strParam.c_str());
        return;
    }
    AMIGOS_INFO("reset ctrl param type : %d, param: %s\n", type, strParam.c_str());
}

void AmigosModuleHseg::_Init()
{
    int iRet = 0;
    InitHsegParam_t initParam;
    memset(&initParam, 0, sizeof(InitHsegParam_t));
    if (stHsegInfo.strIpuPath.length() + 1 > IPU_MAX_LENGTH || stHsegInfo.strModelPath.length() + 1 > MODEL_MAX_LENGTH)
    {
        AMIGOS_ERR("hseg init failed! ipu path or model path is too long,\n ipu path: %s, \n model Path: %s\n",  stHsegInfo.strIpuPath.c_str(), stHsegInfo.strModelPath.c_str());
        return;
    }
    memcpy(initParam.ipu_firware_bin, stHsegInfo.strIpuPath.c_str(), stHsegInfo.strIpuPath.length() + 1);
    memcpy(initParam.seg_model_path, stHsegInfo.strModelPath.c_str(), stHsegInfo.strModelPath.length() + 1);
    iRet = ALGO_HSEG_CreateHandle(&hsegHandle);
    if (0 != iRet)
    {
        AMIGOS_ERR("create handle failed! ret: %d\n", iRet);
        return;
    }
    iRet = ALGO_HSEG_Init(hsegHandle, initParam);
    if (0 != iRet)
    {
        AMIGOS_ERR("create handle failed! ret: %d\n", iRet);
        ALGO_HSEG_ReleaseHandle(hsegHandle);
        hsegHandle = nullptr;
        return;
    }
    blurCtrlParam.bgblurMode = ss_enum_cast<ALGO_HsegBgbMode_e>::from_str(stHsegInfo.strHsegMode);
    blurCtrlParam.maskThredhold = (MI_U8) stHsegInfo.uiMaskThr;
    blurCtrlParam.blurLevel = (MI_U8) stHsegInfo.uiBlurLv;
    blurCtrlParam.scalingStage = (MI_U8) stHsegInfo.uiScalingStage;
    blurCtrlParam.maskOp = ss_enum_cast<ALGO_BgBlurMaskOp_e>::from_str(stHsegInfo.strMaskOp);
    AMIGOS_INFO("hseg init ok\n");
}

void AmigosModuleHseg::_Deinit()
{
    if (nullptr != hsegHandle)
    {
        ALGO_HSEG_DeInit(hsegHandle);
        ALGO_HSEG_ReleaseHandle(hsegHandle);
        hsegHandle = nullptr;
    }
}

unsigned int AmigosModuleHseg::GetModId() const
{
    return uintExtModId;
}

unsigned int AmigosModuleHseg::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}

unsigned int AmigosModuleHseg::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}

ss_linker_base *AmigosModuleHseg::_CreateInputNegativeLinker(unsigned int inPortId)
{
    if (inPortId == HSEG_INPORT_SEND)
    {
        return new LinkerAsyncNegative(false, 8);
    }
    if (inPortId == HSEG_INPORT_ORI)
    {
        return &oriLinker;
    }
    if (inPortId == HSEG_INPORT_REP)
    {
        return &repLinker;
    }
    return nullptr;
}

void AmigosModuleHseg::_DestroyInputNegativeLinker(unsigned int inPortId)
{
    auto iter = this->mapPortIn.find(inPortId);
    if (iter == this->mapPortIn.end())
    {
        AMIGOS_ERR("Cannot find input port %d InInfo\n", inPortId);
        return;
    }
    if (inPortId == HSEG_INPORT_SEND)
    {
        delete iter->second.negative;
    }
    iter->second.negative = nullptr;
}

stream_packer *AmigosModuleHseg::_CreateInputStreamPacker(unsigned int inPortId, bool &bFast)
{
    bFast = false;
    return new StreamPackerSysMma();
}

int AmigosModuleHseg::_InConnect(unsigned int inPortId)
{
    auto it = mapPortIn.find(inPortId);
    if (mapPortIn.end() == it)
    {
        AMIGOS_ERR("hseg chn has no this input(%d) port!!!\n", inPortId);
        return -1;
    }
    if (HSEG_INPORT_ORI == inPortId)
    {
        auto stStreamPacket = it->second.get_packet_info();
        if ((EN_RAW_FRAME_DATA_PA != stStreamPacket.en_type &&  EN_RAW_FRAME_DATA != stStreamPacket.en_type) ||
                (RAW_FORMAT_YUV422_YUYV != stStreamPacket.raw_vid_i.plane_info[0].fmt && RAW_FORMAT_YUV420SP != stStreamPacket.raw_vid_i.plane_info[0].fmt)
                || (stStreamPacket.raw_vid_i.plane_info[0].width < 640 || stStreamPacket.raw_vid_i.plane_info[0].width > 7680)
                || (stStreamPacket.raw_vid_i.plane_info[0].height < 360 || stStreamPacket.raw_vid_i.plane_info[0].height > 4320))
        {
            AMIGOS_ERR("check chn packet failed! get prev packet(not support):type : %d! fmt: %d, resolution: (%d, %d)\n",
                       stStreamPacket.en_type, stStreamPacket.raw_vid_i.plane_info[0].fmt, stStreamPacket.raw_vid_i.plane_info[0].width, stStreamPacket.raw_vid_i.plane_info[0].height);
            return -1;
        }
    }
    AMIGOS_INFO("hseg start in port: %d\n", inPortId);
    if (HSEG_INPORT_SEND != inPortId)
    {
        return 0;
    }
    ss_thread_attr ss_attr;
    memset(&ss_attr, 0, sizeof(ss_thread_attr));
    ss_attr.do_signal          = nullptr;
    ss_attr.in_buf.buf         = this;
    ss_attr.monitor_cycle_sec  = 0;
    ss_attr.monitor_cycle_nsec = 0;
    ss_attr.is_reset_timer     = 0;
    ss_attr.in_buf.size        = 0;
    std::string strThreadName  = this->GetModIdStr() + HSEG_THREAD_SENDTODLA;
    ss_attr.do_monitor         = DoSendToDla;
    snprintf(ss_attr.thread_name, 128, "%s", strThreadName.c_str());
    dlaThreadHandle = ss_thread_open(&ss_attr);
    if (!dlaThreadHandle)
    {
        AMIGOS_ERR("Monitor open thread: %s failed! return error!\n", strThreadName.c_str());
        return -1;
    }
    ss_thread_start_monitor(dlaThreadHandle);
    AMIGOS_INFO("Monitor open thread: %s ok\n", strThreadName.c_str());
    return 0;
}

int AmigosModuleHseg::_InDisconnect(unsigned int inPortId)
{
    if (HSEG_INPORT_SEND != inPortId)
    {
        oriLinker.FlushPacket();
        return 0;
    }
    if (!dlaThreadHandle)
    {
        AMIGOS_ERR("Dla thread handle error!\n");
        return -1;
    }
    ss_thread_stop(dlaThreadHandle);
    ss_thread_close(dlaThreadHandle);
    dlaThreadHandle = nullptr;
    AMIGOS_INFO("hseg close thread: %d\n", inPortId);
    return 0;
}

int AmigosModuleHseg::_Connected(unsigned int outPortId, unsigned int ref)
{
    if (0 != outPortId || ref > 0)
    {
        return 0;
    }
    auto iter = this->mapPortOut.find(outPortId);
    if (this->mapPortOut.end() == iter)
    {
        AMIGOS_ERR("Port %d has not base sturcture.\n", outPortId);
        return -1;
    }
    if (!iter->second.positive.empty())
    {
        threadDesc.pModule = this;
        threadDesc.linker  = &iter->second.positive;
        threadDesc.packer  = &iter->second.outPacker;
        ss_thread_attr ss_attr;
        memset(&ss_attr, 0, sizeof(ss_thread_attr));
        ss_attr.do_signal          = nullptr;
        ss_attr.in_buf.buf         = &threadDesc;
        ss_attr.monitor_cycle_sec  = 0;
        ss_attr.monitor_cycle_nsec = 0;
        ss_attr.is_reset_timer     = 0;
        ss_attr.in_buf.size        = 0;
        ss_attr.do_monitor         = DoBgBlur;
        std::string strThreadName  = this->GetModIdStr() + HSEG_THREAD_DOBGBLUR;
        snprintf(ss_attr.thread_name, 128, "%s", strThreadName.c_str());
        outThreadHandle = ss_thread_open(&ss_attr);
        if (!outThreadHandle)
        {
            AMIGOS_ERR("Monitor open thread: %s failed! return error!\n", strThreadName.c_str());
            return -1;
        }
        ss_thread_start_monitor(outThreadHandle);
    }
    return 0;
}

int AmigosModuleHseg::_Disconnected(unsigned int outPortId, unsigned int ref)
{
    if (0 != outPortId || ref > 0)
    {
        return 0;
    }
    auto iter = this->mapPortOut.find(outPortId);
    if (this->mapPortOut.end() == iter)
    {
        AMIGOS_ERR("Port %d has not base sturcture.\n", outPortId);
        return -1;
    }
    if (!iter->second.positive.empty())
    {
        if (!outThreadHandle)
        {
            AMIGOS_ERR("Port %d has no output thread.\n", outPortId);
            return -1;
        }
        ss_thread_stop(outThreadHandle);
        ss_thread_close(outThreadHandle);
        outThreadHandle    = nullptr;
        threadDesc.pModule = nullptr;
        threadDesc.linker  = nullptr;
    }
    oriLinker.FlushPacket();
    repLinker.FlushPacket();
    return 0;
}

stream_packet_info AmigosModuleHseg::_GetStreamInfo(unsigned int outPortId)
{
    unsigned int inPortId = HSEG_INPORT_ORI;
    stream_packet_info info;

    auto it = mapPortIn.find(inPortId);
    if (mapPortIn.end() == it)
    {
        AMIGOS_ERR("hseg chn has no this input(%d) port!!!\n", inPortId);
        return info;
    }
    return it->second.get_packet_info();
}

bool AmigosModuleHseg::_NeedResetStreamTraverse(unsigned int outPortId, unsigned int bindType,
                                                unsigned int &inPortId)
{
    auto iter = mapPortIn.find(HSEG_INPORT_ORI);

    if (iter == mapPortIn.end())
    {
        AMIGOS_ERR("Port %d has not base sturcture.\n", HSEG_INPORT_ORI);
        return false;
    }
    inPortId = HSEG_INPORT_ORI;
    return true;
}
stream_packet_obj AmigosModuleHseg::_DequeueFromInside(unsigned int outPortId, unsigned int ms)
{
    StreamPackerSysMma mmaPacker;
    return this->_KickPacket(&mmaPacker, outPortId, ms);
}
stream_packet_obj AmigosModuleHseg::_KickPacket(stream_packer *packer, unsigned int outPortId, unsigned int ms)
{
    int iRet = 0;
    int iSwapTmp = 0;
    MI_BOOL bResultFlag = 0;
    stream_packet_info stStreamPacket;
    AlgoHsegInputInfo_t  hsegInputInfo;
    AlgoHsegInputInfo_t srcOri, srcRep;

    if (nullptr == hsegHandle)
    {
        return nullptr;
    }
    memset(&srcOri, 0, sizeof(AlgoHsegInputInfo_t));
    memset(&srcRep, 0, sizeof(AlgoHsegInputInfo_t));
    memset(&hsegInputInfo, 0, sizeof(AlgoHsegInputInfo_t));
    ALGO_HsegBgBlurCtrl_t stCtrl;
    iSwapTmp = blurCtrlParam.bgblurMode;
    stCtrl.bgblur_mode = (ALGO_HsegBgbMode_e)iSwapTmp;
    stCtrl.mask_thredhold = (MI_U8) blurCtrlParam.maskThredhold;
    stCtrl.blur_level = (MI_U8) blurCtrlParam.blurLevel;
    stCtrl.scaling_stage = (MI_U8) blurCtrlParam.scalingStage;
    iSwapTmp = blurCtrlParam.maskOp;
    stCtrl.maskOp = (ALGO_BgBlurMaskOp_e)iSwapTmp;
    auto itOut = mapPortOut.find(outPortId);
    if (mapPortOut.end() == itOut)
    {
        return nullptr;
    }
    auto BackBuf = [&]()
    {
        oriPacketObj = nullptr;
        repPacketObj = nullptr;
    };
    if (nullptr == oriPacketObj)
    {
        oriPacketObj = oriLinker.WaitPacket(ms);
        if (nullptr == oriPacketObj)
        {
            return nullptr;
        }
    }
    if (nullptr == repPacketObj)
    {
        repPacketObj = repLinker.WaitPacket(ms);
        if (nullptr == repPacketObj)
        {
            return nullptr;
        }
    }

    stream_packet_obj oriPa, oriVa;
    stream_packet_obj repPa, repVa;
    stream_packet_obj outPa, outVa;

    iRet = stream_packet_info::raw_convert_vpa_packet(oriPacketObj, oriPa, oriVa);
    if (iRet != 0)
    {
        AMILOG_ERR << "RAW Convert failed! ORI: " << oriPacketObj->get_type() <<  COLOR_ENDL;
        return nullptr;
    }
    iRet = stream_packet_info::raw_convert_vpa_packet(repPacketObj, repPa, repVa);
    if (iRet != 0)
    {
        AMILOG_ERR << "RAW Convert failed! REP: " << repPacketObj->get_type() << COLOR_ENDL;
        return nullptr;
    }

    stStreamPacket.en_type   = EN_RAW_FRAME_DATA;
    stStreamPacket.raw_vid_i = oriPacketObj->raw_vid_i;
    auto outPacket = packer->make(stStreamPacket);
    if (nullptr == outPacket)
    {
        AMIGOS_ERR("Get out port(%d) buf failed\n", itOut->first);
        return nullptr;
    }
    outPacket->set_time_stamp(oriPacketObj->get_time_stamp());
    iRet = stream_packet_info::raw_convert_vpa_packet(outPacket, outPa, outVa);
    if (iRet != 0)
    {
        AMILOG_ERR << "RAW Convert failed! OUT Packet: " << outPacket->get_type() << COLOR_ENDL;
        return nullptr;
    }
    for (int i = 0; i < MAX_INPUT_NUM && i < 3; i++)
    {
        srcOri.bufsize            += oriPa->raw_vid_pa.plane_data[0].size[i];
        srcOri.pt_tensor_data[i]  = oriVa->raw_vid.plane_data[0].data[i];
        srcOri.phy_tensor_addr[i] = oriPa->raw_vid_pa.plane_data[0].phy[i];

        srcRep.bufsize            += repPa->raw_vid_pa.plane_data[0].size[i];
        srcRep.pt_tensor_data[i]  = repVa->raw_vid.plane_data[0].data[i];
        srcRep.phy_tensor_addr[i] = repPa->raw_vid_pa.plane_data[0].phy[i];

        hsegInputInfo.bufsize            += outPa->raw_vid_pa.plane_data[0].size[i];
        hsegInputInfo.pt_tensor_data[i]  = outVa->raw_vid.plane_data[0].data[i];
        hsegInputInfo.phy_tensor_addr[i] = outPa->raw_vid_pa.plane_data[0].phy[i];
    }
    srcOri.width     = oriPa->raw_vid_i.plane_info[0].width;
    srcOri.height    = oriPa->raw_vid_i.plane_info[0].height;
    srcOri.data_type = (RAW_FORMAT_YUV422_YUYV == oriPa->raw_vid_i.plane_info[0].fmt) ? E_ALGO_YUV422_YUYV : E_ALOG_YUV420SP;

    srcRep.width     = repPa->raw_vid_i.plane_info[0].width;
    srcRep.height    = repPa->raw_vid_i.plane_info[0].height;
    srcRep.data_type = srcOri.data_type;

    hsegInputInfo.width     = srcOri.width;
    hsegInputInfo.height    = srcOri.height;
    hsegInputInfo.data_type = srcOri.data_type;
    iRet = ALGO_HSEG_SegmentAndBlurBackgroud(hsegHandle, &srcOri, &srcRep, &hsegInputInfo, stCtrl, &bResultFlag);
    if (0 != iRet)
    {
        AMIGOS_WRN("ALGO_HSEG_SegmentAndBlurBackgroud run failed! ret: %d, flag: %d\n", iRet, bResultFlag);
        return nullptr;
    }
    if (1 != bResultFlag)
    {
        return nullptr;
    }
    BackBuf();
    return outPacket;
}
void *AmigosModuleHseg::DoBgBlur(struct ss_thread_buffer *thread_buf)
{
    HsegThreadDesc *pDesc = (HsegThreadDesc *)thread_buf->buf;
    auto packet = pDesc->pModule->_KickPacket(pDesc->packer, 0, 10);
    if (!packet)
    {
        return nullptr;
    }
    pDesc->linker->enqueue(packet);
    return nullptr;
}

void AmigosModuleHseg::HsegSendToDla()
{
    if (nullptr == hsegHandle)
    {
        return;
    }
    int iRet = 0;
    AlgoHsegInputInfo_t stBufInfo;
    memset(&stBufInfo, 0, sizeof(AlgoHsegInputInfo_t));
    auto itSend = mapPortIn.find((unsigned int)HSEG_INPORT_SEND);
    if (nullptr == itSend->second.negative)
    {
        return;
    }
    LinkerAsyncNegative *asyncLinker = static_cast<LinkerAsyncNegative *>(itSend->second.negative);
    auto packet = asyncLinker->WaitPacket(100);
    if (nullptr == packet)
    {
        return;
    }
    stream_packet_obj pa, va;
    iRet = stream_packet_info::raw_convert_vpa_packet(packet, pa, va);
    if (iRet != 0)
    {
        AMIGOS_ERR("Packet %s can not covert pa or va raw data\n", packet->get_type().c_str());
        return;
    }
    for (int i = 0; i < 3 && i < MAX_INPUT_NUM; i++)
    {
        stBufInfo.bufsize += pa->raw_vid_pa.plane_data[0].size[i];
        stBufInfo.pt_tensor_data[i] = va->raw_vid.plane_data[0].data[i];
        stBufInfo.phy_tensor_addr[i] = pa->raw_vid_pa.plane_data[0].phy[i];
    }
    stBufInfo.width = pa->raw_vid_i.plane_info[0].width;
    stBufInfo.height = pa->raw_vid_i.plane_info[0].height;
    stBufInfo.data_type = (RAW_FORMAT_YUV422_YUYV == pa->raw_vid_i.plane_info[0].fmt) ? E_ALGO_YUV422_YUYV : E_ALOG_YUV420SP;
    iRet = ALGO_HSEG_SendInput(hsegHandle, &stBufInfo);
    if (0 != iRet)
    {
        AMIGOS_ERR("ALGO_HSEG_SendInput run failed! ret: %d\n", iRet);
    }
    return;
}
void* AmigosModuleHseg::DoSendToDla(struct ss_thread_buffer *thread_buf)
{
    AmigosModuleHseg *pModule = (AmigosModuleHseg *)thread_buf->buf;
    if (nullptr != pModule)
    {
        pModule->HsegSendToDla();
    }
    return nullptr;
}
AMIGOS_MODULE_INIT("HSEG", AmigosModuleHseg);

