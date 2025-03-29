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
#include <atomic>
#include "amigos_module_mi_base.h"
#include "amigos_surface_hseg.h"

class AmigosModuleHseg final: public AmigosSurfaceHseg, public AmigosModuleMiBase
{
public:

    typedef enum
    {
        E_MOD_HSEG_PARAM_TYPE_MODE,
        E_MOD_HSEG_PARAM_TYPE_OP,
        E_MOD_HSEG_PARAM_TYPE_THR,
        E_MOD_HSEG_PARAM_TYPE_LV,
        E_MOD_HSEG_PARAM_TYPE_STAGE
    } AMIGOS_MOD_HSEG_ParamType_t;

    typedef struct AMIGOS_MOD_HsegBgBlurCtrl_s
    {
        std::atomic<int> bgblurMode;
        std::atomic<int> maskOp;
        std::atomic<unsigned char> maskThredhold;
        std::atomic<unsigned char> blurLevel; //[0-255]
        std::atomic<unsigned char> scalingStage; //[1-15]
        AMIGOS_MOD_HsegBgBlurCtrl_s() : bgblurMode(0), maskOp(0), maskThredhold(0), blurLevel(0), scalingStage(0)
        {
        }
    } AMIGOS_MOD_HsegBgBlurCtrl_t;

    explicit AmigosModuleHseg(const std::string &strSection);
    ~AmigosModuleHseg() override;

    unsigned int GetModId() const override;
    unsigned int GetInputType(unsigned int port) const override;
    unsigned int GetOutputType(unsigned int port) const override;

    void SetCtrlParam(std::string& strParam, AMIGOS_MOD_HSEG_ParamType_t type);

protected:
    void _Init() override;
    void _Deinit() override;
    bool _NeedResetStreamTraverse(unsigned int outPortId, unsigned int bindType, unsigned int &inPortId) override;

private:
    stream_packet_obj _KickPacket(stream_packer *packer, unsigned int outPortId, unsigned int ms);
    void _StartOut(unsigned int outPortId) override {};
    void _StopOut(unsigned int outPortId) override {};
    void _StartIn(unsigned int inPortId) override {};
    void _StopIn(unsigned int inPortId) override {};
    int _InConnect(unsigned int inPortId) override;
    int _InDisconnect(unsigned int inPortId) override;
    int _Connected(unsigned int outPortId, unsigned int ref) override;
    int _Disconnected(unsigned int outPortId, unsigned int ref) override;
    stream_packet_obj _DequeueFromInside(unsigned int outPortId, unsigned int ms) override;
    ss_linker_base *_CreateInputNegativeLinker(unsigned int inPortId) override;
    void _DestroyInputNegativeLinker(unsigned int inPortId) override;
    stream_packer *_CreateInputStreamPacker(unsigned int inPortId, bool &bFast);
    stream_packet_info _GetStreamInfo(unsigned int outPortId) override;

    void HsegSendToDla();
    static void * DoSendToDla(struct ss_thread_buffer *thread_buf);
    static void * DoBgBlur(struct ss_thread_buffer *thread_buf);

private:
    struct HsegThreadDesc
    {
        AmigosModuleHseg *pModule;
        ss_linker_base   *linker;
        stream_packer    *packer;
        HsegThreadDesc()
        {
            pModule = nullptr;
            linker  = nullptr;
        }
    };
    void                        *hsegHandle;
    void                        *outThreadHandle;
    void                        *dlaThreadHandle;
    struct HsegThreadDesc       threadDesc;
    stream_packet_obj           oriPacketObj;
    stream_packet_obj           repPacketObj;
    AMIGOS_MOD_HsegBgBlurCtrl_t blurCtrlParam;
    LinkerAsyncNegative         oriLinker;
    LinkerAsyncNegative         repLinker;
};


