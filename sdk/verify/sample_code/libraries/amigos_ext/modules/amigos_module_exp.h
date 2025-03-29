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

#ifndef __AMIGOS_MODULE_EXP_H__
#define __AMIGOS_MODULE_EXP_H__

#include <pthread.h>
#include <list>
#include <map>
#include <set>
#include <string>

#include "ss_exp.h"
#include "ss_filter.h"

#include "amigos_module_base.h"
#include "amigos_surface_exp.h"

class AmigosModuleExp : public AmigosSurfaceExp, public AmigosModuleBase
{
private:
    static int ExpServerOpen(const char *url, void *privateData, unsigned char bRead, SS_EXP_WorkCfg_t *workCfg);
    static int ExpServerClose(void *pUsrData);
    static int ExpServerTransfer(void *pUsrData, SS_EXP_TransferObject_t *transferObj);
    static int ExpServerTransferDone(void *pUsrData, const SS_EXP_TransferObject_t *transferObj);
    static int ExpServerRemoteRead(void *pUsrData, char *pTransData, unsigned int u32DataSize);
    static void *ExpServerRemoteReadAlloc(void *, unsigned int size);
    static void ExpServerRemoteReadFree(void *, void *buf);

    static int ExpClientOpen(const char *url, void *privateData, unsigned char bRead, SS_EXP_WorkCfg_t *workCfg);
    static int ExpClientClose(void *pUsrData);
    static int ExpClientTransfer(void *pUsrData, SS_EXP_TransferObject_t *transferObj);
    static int ExpClientTransferDone(void *pUsrData, const SS_EXP_TransferObject_t *transferObj);
    static int ExpClientRemoteRead(void *pUsrData, char *pTransData, unsigned int u32DataSize);
    static void *ExpClientRemoteReadAlloc(void *pUsrData, unsigned int size);
    static void ExpClientRemoteReadFree(void *pUsrData, void *buf);
    static int ExpClientWorkMsg(void *pUsrData, const char workMsg[SS_EXP_WORK_INFO_MSG_SIZE]);

    class LinkerExpTransferIn : public ss_linker_base
    {
    public:
        explicit LinkerExpTransferIn(unsigned int retryDelay, InPortInfo *info);
        ~LinkerExpTransferIn() override;

        void AddLinker(ss_linker_base *pConnector);
        void DelLinker(ss_linker_base *pConnector);

        void _prepareEnqueue(void);
        int  _tryEnqueue(stream_packet_obj &packet);
        int               enqueue(stream_packet_obj &packet) override;
        stream_packet_obj dequeue(unsigned int ms) override;
        InPortInfo *inInfo;
    private:
        pthread_mutex_t            linkerMutex;
        unsigned int               retryDelay;
        std::set<ss_linker_base *> setLinker;
        std::set<ss_linker_base *> tryLinker;
    };
    class ExpHandShakeInDesc : public LinkerExpTransferIn
    {
    public:
        explicit ExpHandShakeInDesc(unsigned int retryDelay, const std::string &url,
                                    void *ins, InPortInfo *info, unsigned int workId);
        virtual ~ExpHandShakeInDesc();
    private:
        WORK_HANDLE         readWorkHandle;
        WORK_HANDLE         writeWorkHandle;
    };

    class ExpDirectInDesc : public LinkerExpTransferIn
    {
    public:
        explicit ExpDirectInDesc(unsigned int retryDelay, unsigned int socketPort, InPortInfo *info);
        virtual ~ExpDirectInDesc();
    private:
        void *pInstance;
    };
    class ExpTransferDesc
    {
    public:
        explicit ExpTransferDesc(LinkerExpTransferIn *transferLinker);
        ~ExpTransferDesc();
        LinkerExpTransferIn *transferLinker;
        LinkerAsyncNegative  nLinker;
        ss_message          *message;
        stream_packet_obj    packet;
    };
    struct ExpOutDesc
    {
        void                                     *pInstance;
        std::map<unsigned int, ss_linker_base *> linker;
        std::map<unsigned int, stream_packer *>  packer;
        ExpOutDesc() : pInstance(nullptr) {}
    };
    struct ExpTransferOutDesc
    {
        stream_packer     *packer;
        ss_linker_base    *linker;
        ExpOutDesc        *desc;
        stream_packet_info info;
        stream_packet_obj  packet;
        ss_video_packet_filter video_filter;
        ExpTransferOutDesc() : packer(nullptr), linker(nullptr), desc(nullptr) {}
    };
public:
    explicit AmigosModuleExp(const std::string &strInSection);
    ~AmigosModuleExp() override;
    unsigned int GetModId() const override;
    unsigned int GetInputType(unsigned int port) const override;
    unsigned int GetOutputType(unsigned int port) const override;

private:
    void _Init() override;
    void _Deinit() override;

    void _Start() override;
    void _Stop() override;

    void _StartIn(unsigned int inPortId) override;
    void _StopIn(unsigned int inPortId) override;

    void _StartOut(unsigned int outPordId) override;
    void _StopOut(unsigned int outPordId) override;

    int _Connected(unsigned int outPortId, unsigned int ref) override;
    int _Disconnected(unsigned int outPortId, unsigned int ref) override;
    bool _IsDelayConnected(unsigned int uintInPort) override;

    ss_linker_base *_CreateInputNegativeLinker(unsigned int inPortId) override;
    void _DestroyInputNegativeLinker(unsigned int inPortId) override;
    stream_packet_info _GetStreamInfo(unsigned int outPortId) override;
private:
    int  _StartDirectIn(unsigned int inPortId, const ExpInInfo &expInInfo, InPortInfo *expInDesc);
    int  _StartHandShakeIn(unsigned int inPortId, const ExpInInfo &expInInfo, InPortInfo *expInDesc);
    void _StopDirectIn(unsigned int inPortId);
    void _StopHandShakeIn(unsigned int inPortId);

private:

    std::map<unsigned int, void*> mapHandShakeInsDesc;
    // Normally one url ha s2 channels of stream, specifically they are video and audio.
    std::map<unsigned int, std::map<std::string, ExpHandShakeInDesc *>> mapInHandShakeDesc;
    std::map<unsigned int, ExpDirectInDesc *>   mapInDirectDesc;
    std::map<std::string, ExpOutDesc>           mapOutDesc;
    std::set<std::string>                       setAddr;
};

#endif /* __AMIGOS_MODULE_EXP_H__ */
