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
#pragma once
#include <mutex>
#include <atomic>
#include "ss_linker.h"
#include "amigos_log.h"
#include "ss_packet.h"
#include "mi_ive.h"
#include "amigos_module_mi_base.h"
#include "amigos_surface_ive.h"

class AmigosModuleIve final: public AmigosSurfaceIve, public AmigosModuleMiBase
{
public:
    class SourceImage
    {
    public:
        SourceImage() : imagePa(nullptr)
        {
            memset(&miIveImg, 0, sizeof(MI_IVE_SrcImage_t));
        }
        void SetImage(stream_packet_obj pa)
        {
            this->miIveImg.eType          = E_MI_IVE_IMAGE_TYPE_U8C1;
            this->miIveImg.u16Width       = pa->raw_vid_i.plane_info[0].width;
            this->miIveImg.u16Height      = pa->raw_vid_i.plane_info[0].height;
            this->miIveImg.apu8VirAddr[0] = (MI_U8 *)NULL;
            this->miIveImg.aphyPhyAddr[0] = pa->raw_vid_pa.plane_data[0].phy[0];
            this->miIveImg.azu16Stride[0] = pa->raw_vid_pa.plane_data[0].stride[0];
            this->imagePa = pa;
        }
        ~SourceImage() {}
        MI_IVE_SrcImage_t miIveImg;

    private:
        stream_packet_obj imagePa;
    };
    class LinkerBgBlurRepBgNegative final: public ss_linker_base
    {
    public:
        typedef enum
        {
            E_MOD_IVE_PARAM_TYPE_MODE,
            E_MOD_IVE_PARAM_TYPE_OP,
            E_MOD_IVE_PARAM_TYPE_THR,
            E_MOD_IVE_PARAM_TYPE_LV,
            E_MOD_IVE_PARAM_TYPE_STAGE,
            E_MOD_IVE_PARAM_TYPE_SATURATIONLV,
            E_MOD_IVE_PARAM_TYPE_MOSAICSIZE
        } AMIGOS_MOD_IVE_ParamType_t;
        LinkerBgBlurRepBgNegative(const IveInfo &iveInfo, MI_IVE_HANDLE handle, ss_linker_base *linker);
        virtual ~LinkerBgBlurRepBgNegative() {};
        int enqueue(stream_packet_obj &packet) override;
        stream_packet_obj dequeue(unsigned int ms) override
        {
            return nullptr;
        }
        void SetSrcPacket(stream_packet_obj &packet)
        {
            this->oriPacket = packet;
        }
        void SetCtrlParam(std::string& strParam, AMIGOS_MOD_IVE_ParamType_t type);
        void SetImage(stream_packet_obj &yPacket, stream_packet_obj &uvPacket)
        {
            srcYImage.SetImage(stream_packet_base::convert(yPacket, EN_RAW_FRAME_DATA_PA));
            srcUvImage.SetImage(stream_packet_base::convert(uvPacket, EN_RAW_FRAME_DATA_PA));
        }
        void DebugBgblurOnOff(std::string& strOnOff);
    private:
        MI_IVE_HANDLE       iIveHandle;
        ss_linker_base      *dstLinker;
        stream_packet_obj   oriPacket;
        MI_IVE_BgBlurCtrl_t iveCtrl;
        std::mutex          mutexCtrl;
        std::atomic<bool>   bOnBgBlur;
        SourceImage         srcYImage;
        SourceImage         srcUvImage;
    };

    class LinkerBgBlurOriNegative final: public ss_linker_base
    {
    public:
        LinkerBgBlurOriNegative(LinkerBgBlurRepBgNegative *linker) : repLinker(linker)
        {
        };
        virtual ~LinkerBgBlurOriNegative() {};
        int enqueue(stream_packet_obj &packet) override
        {
            repLinker->SetSrcPacket(packet);
            return 0;
        };
        stream_packet_obj dequeue(unsigned int time_out_ms)
        {
            return nullptr;
        };
    protected:
        LinkerBgBlurRepBgNegative *repLinker;
    };
public:

    explicit AmigosModuleIve(const std::string &strSection);
    ~AmigosModuleIve() override;

    unsigned int GetModId() const override;
    unsigned int GetInputType(unsigned int port) const override;
    unsigned int GetOutputType(unsigned int port) const override;
    void SetCtrlParam(std::string& strParam, LinkerBgBlurRepBgNegative::AMIGOS_MOD_IVE_ParamType_t type);
    void DebugIveOnOff(std::string& strOnOff);

protected:
    void _Init() override;
    void _Deinit() override;

    void _StartOut(unsigned int outPortId) override {};
    void _StopOut(unsigned int outPortId) override {};
    int _Connected(unsigned int outPortId, unsigned int ref) override
    {
        return 0;
    };
    int _Disconnected(unsigned int outPortId, unsigned int ref) override
    {
        return 0;
    };

    void _StartIn(unsigned int inPortId) override {};
    void _StopIn(unsigned int inPortId) override {};

private:
    void _Start() override;
    ss_linker_base *_CreateInputNegativeLinker(unsigned int inPortId) override;
    virtual bool _NeedResetStreamTraverse(unsigned int outPortId, unsigned int bindType, unsigned int &inPortId) override;
    stream_packet_info _GetStreamInfo(unsigned int outPortId) override;

private:
    MI_IVE_HANDLE iIveHandle;
    LinkerBgBlurOriNegative   *bgBlurOriLinker;
    LinkerBgBlurRepBgNegative *bgBlurRepLinker;
};
