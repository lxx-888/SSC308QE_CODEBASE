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

#include <pthread.h>
#include <unistd.h>

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <set>
#include <string>

#include <signal.h>
#include "ss_auto_lock.h"
#include "ss_enum_cast.hpp"
#include "ss_linker.h"
#include "ss_packet.h"
#include "ss_thread.h"
#include "ss_exp.h"

#include "amigos_log.h"
#include "amigos_module_init.h"
#include "amigos_surface_exp.h"
#include "amigos_module_exp.h"

#define EXP_LINKER_FIFO_MAX_CNT 15

static void _BrokenPipe(int num)
{
    printf("MASK BROKEN PIPE. Signal Num %d\n", num);
}
static unsigned int _stuToStr(const stream_packet_info &info, const unsigned int &id, char *str)
{
    if (EN_VIDEO_CODEC_DATA == info.en_type)
    {
        snprintf(str, SS_EXP_WORK_INFO_MSG_SIZE, "workid:%d,type:%s,fmt:%s,width:%d,height:%d",
                id, ss_enum_cast<stream_type>::to_str(info.en_type).c_str(),
                ss_enum_cast<es_video_fmt>::to_str(info.es_vid_i.fmt).c_str(),
                info.es_vid_i.width, info.es_vid_i.height);
    }
    else if (EN_AUDIO_CODEC_DATA == info.en_type)
    {
        snprintf(str, SS_EXP_WORK_INFO_MSG_SIZE, "workid:%d,type:%s,fmt:%s,sample:%d,channel:%d",
                id, ss_enum_cast<stream_type>::to_str(info.en_type).c_str(),
                ss_enum_cast<es_audio_fmt>::to_str(info.es_aud_i.fmt).c_str(),
                info.es_aud_i.sample_rate, info.es_aud_i.channels);
    }
    else
    {
        AMIGOS_ERR("Not Support type %d\n", info.en_type);
        return 0;
    }
    return strlen(str);
}
static int _parseStr(const char* str, stream_packet_info &info, unsigned int &id)
{
    char key[5][10];
    char value[5][10];
    int result = sscanf(str, "%[^:]:%[^,],%[^:]:%[^,],%[^:]:%[^,],%[^:]:%[^,],%[^:]:%[^,]",
           key[0], value[0], key[1], value[1], key[2],
           value[2], key[3], value[3], key[4], value[4]);
    if (result != 10)
    {
        AMIGOS_ERR("parser err, please check msg format, msg %s\n", str);
        return -1;
    }
    if (strcmp (key[0], "workid") == 0)
    {
        id = atoi(value[0]);
    }
    if (strcmp(key[1], "type") == 0)
    {
        info.en_type = ss_enum_cast<stream_type>::from_str(value[1]);
    }
    if (info.en_type == EN_VIDEO_CODEC_DATA)
    {
        if (strcmp(key[2], "fmt") == 0)
        {
            info.es_vid_i.fmt = ss_enum_cast<es_video_fmt>::from_str(value[2]);
        }
        if (strcmp(key[3], "width") == 0)
        {
            info.es_vid_i.width = atoi(value[3]);
        }
        if (strcmp(key[4], "height") == 0)
        {
            info.es_vid_i.width = atoi(value[4]);
        }
    }
    else if (info.en_type == EN_AUDIO_CODEC_DATA)
    {
        if (strcmp(key[2], "fmt") == 0)
        {
            info.es_aud_i.fmt = ss_enum_cast<es_audio_fmt>::from_str(value[2]);
        }
        if (strcmp(key[3], "sample") == 0)
        {
            info.es_aud_i.sample_rate = atoi(value[3]);
        }
        if (strcmp(key[4], "channel") == 0)
        {
            info.es_aud_i.channels = atoi(value[4]);
        }
    }
    else
    {
        AMIGOS_ERR("No Support format\n");
        return -1;
    }
    return 0;
}
AmigosModuleExp::ExpHandShakeInDesc::ExpHandShakeInDesc(unsigned int retryDelay, const std::string &url, void *ins,
                                                        InPortInfo *info, unsigned int workId)
    : LinkerExpTransferIn(retryDelay, info), readWorkHandle(NULL), writeWorkHandle(NULL)
{
    SS_EXP_WorkInfo_t workInfo;

    memset(&workInfo, 0, sizeof(SS_EXP_WorkInfo_t));
    stream_packet_info packetInfo = info->get_packet_info();
    unsigned int size = _stuToStr(packetInfo, workId, workInfo.workMsg);
    if (size > sizeof(workInfo.workMsg) || !size)
    {
        AMIGOS_ERR("workMsg oversize or size is zero\n");
        assert(0);
    }
    AMIGOS_INFO("%s\n", workInfo.workMsg);
    workInfo.enWorkMode   = E_SS_EXP_SRC_WRITE_ONLY;
    this->writeWorkHandle = SS_EXP_SourceCreateWork(ins, url.c_str(), this, &workInfo);
    if (NULL == this->writeWorkHandle)
    {
        AMIGOS_ERR("Create write work handle failed\n");
        assert(0);
    }
    workInfo.enWorkMode  = E_SS_EXP_SRC_READ_ONLY;
    this->readWorkHandle = SS_EXP_SourceCreateWork(ins, url.c_str(), this, &workInfo);
    if (NULL == this->readWorkHandle)
    {
        AMIGOS_ERR("Create read work handle failed\n");
        SS_EXP_SourceDestroyWork(this->writeWorkHandle);
        this->writeWorkHandle = NULL;
        assert(0);
    }
}
AmigosModuleExp::ExpHandShakeInDesc::~ExpHandShakeInDesc()
{
    SS_EXP_SourceDestroyWork(this->readWorkHandle);
    SS_EXP_SourceDestroyWork(this->writeWorkHandle);
    this->readWorkHandle  = NULL;
    this->writeWorkHandle = NULL;
}
AmigosModuleExp::ExpDirectInDesc::ExpDirectInDesc(unsigned int retryDelay, unsigned int socketPort, InPortInfo *info)
    : LinkerExpTransferIn(retryDelay, info), pInstance(nullptr)
{
    SS_EXP_SourceAttr_t    sourceAttr;
    SS_EXP_TransferActCb_t transferActCb;

    memset(&sourceAttr, 0, sizeof(SS_EXP_SourceAttr_t));
    memset(&transferActCb, 0, sizeof(SS_EXP_TransferActCb_t));

    sourceAttr.uintPort    = socketPort;
    sourceAttr.bWithNoSync = 1;
    sourceAttr.privateData = this;

    transferActCb.fpSsExpOpen            = ExpServerOpen;
    transferActCb.fpSsExpClose           = ExpServerClose;
    transferActCb.fpSsExpTransfer        = ExpServerTransfer;
    transferActCb.fpSsExpTransferDone    = ExpServerTransferDone;
    transferActCb.fpSsExpRemoteRead      = ExpServerRemoteRead;
    transferActCb.fpSsExpRemoteReadAlloc = ExpServerRemoteReadAlloc;
    transferActCb.fpSsExpRemoteReadFree  = ExpServerRemoteReadFree;

    this->pInstance = SS_EXP_SourceInit(&sourceAttr, &transferActCb);
    if (this->pInstance)
    {
        SS_EXP_SourceStart(this->pInstance);
        return;
    }
    AMIGOS_ERR("SS_EXP_SourceInit Failed.\n");
}
AmigosModuleExp::ExpDirectInDesc::~ExpDirectInDesc()
{
    SS_EXP_SourceStop(this->pInstance);
    SS_EXP_SourceDeinit(this->pInstance);
}

AmigosModuleExp::ExpTransferDesc::ExpTransferDesc(LinkerExpTransferIn *transferLinker)
    : transferLinker(transferLinker), nLinker(false, EXP_LINKER_FIFO_MAX_CNT)
{
    transferLinker->AddLinker(&nLinker);
    transferLinker->inInfo->access();
}
AmigosModuleExp::ExpTransferDesc::~ExpTransferDesc()
{
    // Maybe u are supposed to be strange here, but we should delete linker first to let the 'enqueue'
    // thread go unblockedly and then it will not happen deadlock in calling function 'message->leave()'.
    transferLinker->DelLinker(&nLinker);
    transferLinker->inInfo->leave();
}
AmigosModuleExp::AmigosModuleExp(const std::string &strInSection)
    : AmigosSurfaceExp(strInSection), AmigosModuleBase(this)
{
}

AmigosModuleExp::~AmigosModuleExp() {}

unsigned int AmigosModuleExp::GetModId() const
{
    return this->uintExtModId;
}

unsigned int AmigosModuleExp::GetInputType(unsigned int) const
{
    return E_MOD_PORT_TYPE_USER;
}

unsigned int AmigosModuleExp::GetOutputType(unsigned int) const
{
    return E_MOD_PORT_TYPE_USER;
}

void AmigosModuleExp::_Init()
{
    signal(SIGPIPE, _BrokenPipe);
    auto iter = this->mapPortIn.begin();
    if (iter == this->mapPortIn.end())
    {
        // has no input port, no need to init exp source
        return;
    }
    SS_EXP_SourceAttr_t    sourceAttr;
    SS_EXP_TransferActCb_t transferActCb;
    memset(&sourceAttr, 0, sizeof(SS_EXP_SourceAttr_t));
    memset(&transferActCb, 0, sizeof(SS_EXP_TransferActCb_t));
    for (auto& itInInfo : mapExpInInfo)
    {
        if (E_EXP_IN_MODE_HAND_SHAKE == itInInfo.second.mode)
        {
            auto itExp = mapHandShakeInsDesc.find(itInInfo.second.socketPort);
            if (mapHandShakeInsDesc.end() == itExp)
            {
                memset(&sourceAttr, 0, sizeof(SS_EXP_SourceAttr_t));
                memset(&transferActCb, 0, sizeof(SS_EXP_TransferActCb_t));
                sourceAttr.uintPort    = itInInfo.second.socketPort;
                sourceAttr.bWithNoSync = 0;
                sourceAttr.privateData = this;
                transferActCb.fpSsExpOpen            = ExpServerOpen;
                transferActCb.fpSsExpClose           = ExpServerClose;
                transferActCb.fpSsExpTransfer        = ExpServerTransfer;
                transferActCb.fpSsExpTransferDone    = ExpServerTransferDone;
                transferActCb.fpSsExpRemoteRead      = ExpServerRemoteRead;
                transferActCb.fpSsExpRemoteReadAlloc = ExpServerRemoteReadAlloc;
                transferActCb.fpSsExpRemoteReadFree  = ExpServerRemoteReadFree;
                auto pHandShakeIns = SS_EXP_SourceInit(&sourceAttr, &transferActCb);
                if (!pHandShakeIns)
                {
                    AMIGOS_ERR("SS_EXP_SourceInit Failed \n");
                    continue;
                };
                mapHandShakeInsDesc[itInInfo.second.socketPort] = pHandShakeIns;
            }
        }
    }
    AMIGOS_INFO("exp init OK \n");
}

void AmigosModuleExp::_Deinit()
{
    for (auto &itExp : mapHandShakeInsDesc)
    {
        SS_EXP_SourceDeinit(itExp.second);
    }
    mapHandShakeInsDesc.clear();
    AMIGOS_INFO("exp deinit.\n");
}

void AmigosModuleExp::_Start()
{
    for (auto &itExp : mapHandShakeInsDesc)
    {
        if (itExp.second)
        {
            if (0 != SS_EXP_SourceStart(itExp.second))
            {
                AMIGOS_ERR("SS_EXP_SourceStop Failed. port: %u\n", itExp.first);
            }
        }
    }
    if (mapExpInInfo.size())
    {
        // EXP source.
        const char *interface[] = {"eth0", "eth1", "wlan0", "wlan1", "enp1s0", NULL};
        char        ipAddr[32];
        int         i = 0;
        for (i = 0; interface[i] != NULL; i++)
        {
            if (SS_EXP_GetIpAddr(interface[i], ipAddr, 32) == 0)
            {
                break;
            }
        }
        if (i == sizeof(interface) / sizeof(char *))
        {
            AMIGOS_WRN("Can not get current device's IP.\n");
            snprintf(ipAddr, 32, "0.0.0.0");
        }
        for (auto &it : mapExpInInfo)
        {
            std::stringstream urlAddr;
            if (it.second.mode == E_EXP_IN_MODE_DIRECT)
            {
                urlAddr << "tcp://" << ipAddr << ':' << std::dec << it.second.socketPort;
            }
            else if (it.second.mode == E_EXP_IN_MODE_HAND_SHAKE)
            {
                urlAddr << "exp://" << ipAddr << ':' << std::dec << it.second.socketPort << '/' << it.second.urlSuffix;
            }
            if (setAddr.find(urlAddr.str()) == setAddr.end())
            {
                setAddr.insert(urlAddr.str());
            }
        }
        i = 0;
        for (auto &it : setAddr)
        {
            std::stringstream        ss;
            ss << "EXP_" << this->stModInfo.devId << "_" << this->stModInfo.chnId << "_" << i++;
            AMIGOS_LOG("=================URL=================\n");
            AMIGOS_LOG("%s\n", it.c_str());
            AMIGOS_LOG("=================URL=================\n");
            this->env.Ext("EXP_PREVIEW_WINDOWS")[ss.str()] = it;
        }
    }
    AMIGOS_INFO("exp start.\n");
}

void AmigosModuleExp::_Stop()
{
    for (auto &itExp : mapHandShakeInsDesc)
    {
        if (itExp.second)
        {
            if (0 != SS_EXP_SourceStop(itExp.second))
            {
                AMIGOS_ERR("SS_EXP_SourceStop Failed. port: %u\n", itExp.first);
            }
        }
    }
    if (mapExpInInfo.size())
    {
        int i = 0;
        for (auto it : setAddr)
        {
            std::stringstream        ss;
            ss << "EXP_" << this->stModInfo.devId << "_" << this->stModInfo.chnId << "_" << i++;
            this->env.Ext("EXP_PREVIEW_WINDOWS")[ss.str()] = "";
        }
        setAddr.clear();
    }
    AMIGOS_INFO("exp stop.\n");
}

void AmigosModuleExp::_StartIn(unsigned int inPortId)
{
    auto itExpInInfo = this->mapExpInInfo.find(inPortId);
    if (itExpInInfo == this->mapExpInInfo.end())
    {
        AMIGOS_ERR("Input port %d is not exist\n", inPortId);
        return;
    }

    auto itIn = this->mapPortIn.find(inPortId);
    if (itIn == this->mapPortIn.end())
    {
        AMIGOS_ERR("exp desc %d is not exist\n", inPortId);
        return;
    }

    auto packetInfo = itIn->second.get_packet_info();
    if (EN_VIDEO_CODEC_DATA != packetInfo.en_type && EN_AUDIO_CODEC_DATA != packetInfo.en_type)
    {
        AMIGOS_ERR("en_type %d is not support\n", packetInfo.en_type);
        return;
    }

    const ExpInInfo &expInInfo = itExpInInfo->second;

    int ret = 0;
    if (expInInfo.mode == E_EXP_IN_MODE_HAND_SHAKE)
    {
        ret = this->_StartHandShakeIn(inPortId, expInInfo, &itIn->second);
        if (ret == 0)
        {
            AMIGOS_LOG("====== EXP Server [%d] start <ip>:%d/%s\n", inPortId, itExpInInfo->second.socketPort,
                       itExpInInfo->second.urlSuffix.c_str());
        }
        return;
    }
    if (expInInfo.mode == E_EXP_IN_MODE_DIRECT)
    {
        ret = this->_StartDirectIn(inPortId, expInInfo, &itIn->second);
        if (ret == 0)
        {
            AMIGOS_LOG("====== EXP Server [%d] start <ip>:%d\n", inPortId, itExpInInfo->second.socketPort);
        }
    }
}

void AmigosModuleExp::_StopIn(unsigned int inPortId)
{
    auto itExpInInfo = this->mapExpInInfo.find(inPortId);
    if (itExpInInfo == this->mapExpInInfo.end())
    {
        AMIGOS_ERR("Input port %d is not exist\n", inPortId);
        return;
    }

    const ExpInInfo &expInInfo = itExpInInfo->second;
    if (expInInfo.mode == E_EXP_IN_MODE_HAND_SHAKE)
    {
        this->_StopHandShakeIn(inPortId);
    }
    else if (expInInfo.mode == E_EXP_IN_MODE_DIRECT)
    {
        this->_StopDirectIn(inPortId);
    }
    AMIGOS_LOG("====== EXP Server [%d] stop\n", inPortId);
}

void AmigosModuleExp::_StartOut(unsigned int outPortId)
{
    auto iter = this->mapPortOut.find(outPortId);
    if (iter == this->mapPortOut.end())
    {
        AMIGOS_ERR("Base out port %d not exist\n", outPortId);
        return;
    }
    auto itExpOutInfo = this->mapExpOutInfo.find(outPortId);
    if (itExpOutInfo == this->mapExpOutInfo.end())
    {
        AMIGOS_ERR("Output port %d is not exit\n", outPortId);
        return;
    }

    ExpOutInfo &expOutInfo = itExpOutInfo->second;
    auto &outDesc = this->mapOutDesc[expOutInfo.url];
    if (outDesc.linker.find(expOutInfo.workId) != outDesc.linker.end())
    {
        AMIGOS_ERR("linker has occupied, portid %d\n", outPortId);
        return ;
    }
    outDesc.linker[expOutInfo.workId] = &iter->second.positive;
    outDesc.packer[expOutInfo.workId] = &iter->second.outPacker;
}

void AmigosModuleExp::_StopOut(unsigned int outPortId)
{
    auto iter = this->mapPortOut.find(outPortId);
    if (iter == this->mapPortOut.end())
    {
        AMIGOS_ERR("Base out port %d not exist\n", outPortId);
        return;
    }
    auto itExpOutInfo = this->mapExpOutInfo.find(outPortId);
    if (itExpOutInfo == this->mapExpOutInfo.end())
    {
        AMIGOS_ERR("Output port %d is not exit\n", outPortId);
        return;
    }
    auto &outDesc = this->mapOutDesc[itExpOutInfo->second.url];
    if (outDesc.linker.find(itExpOutInfo->second.workId) == outDesc.linker.end())
    {
        AMIGOS_ERR("linker has not created, portid %d\n", outPortId);
        return ;
    }
    outDesc.linker.erase(itExpOutInfo->second.workId);
    outDesc.packer.erase(itExpOutInfo->second.workId);
}

int AmigosModuleExp::_Connected(unsigned int outPortId, unsigned int ref)
{
    if (ref > 0)
    {
        return 0;
    }
    auto iter = this->mapExpOutInfo.find(outPortId);
    if (iter == this->mapExpOutInfo.end())
    {
        return -1;
    }
    auto it = this->mapOutDesc.find(iter->second.url);
    if (it != this->mapOutDesc.end() && it->second.pInstance)
    {
        AMIGOS_INFO("url %s, SS_EXP_SinkInit_%p noneed init \n", iter->second.url.c_str(),
                it->second.pInstance);
        return 0;
    }
    SS_EXP_SinkAttr_t sinkAttr;
    SS_EXP_TransferActCb_t transferActCb;
    memset(&sinkAttr, 0, sizeof(SS_EXP_SinkAttr_t));
    memset(&transferActCb, 0, sizeof(SS_EXP_TransferActCb_t));

    sinkAttr.pAddress = iter->second.url.c_str();
    sinkAttr.privateData = &this->mapOutDesc[iter->second.url];

    transferActCb.fpSsExpOpen            = ExpClientOpen;
    transferActCb.fpSsExpClose           = ExpClientClose;
    transferActCb.fpSsExpWorkMsg         = ExpClientWorkMsg;
    transferActCb.fpSsExpTransfer        = ExpClientTransfer;
    transferActCb.fpSsExpTransferDone    = ExpClientTransferDone;
    transferActCb.fpSsExpRemoteRead      = ExpClientRemoteRead;
    transferActCb.fpSsExpRemoteReadAlloc = ExpClientRemoteReadAlloc;
    transferActCb.fpSsExpRemoteReadFree  = ExpClientRemoteReadFree;

    this->mapOutDesc[iter->second.url].pInstance = SS_EXP_SinkInit(&sinkAttr, &transferActCb);
    if (!this->mapOutDesc[iter->second.url].pInstance)
    {
        AMIGOS_ERR("SS_EXP_SinkInit Failed \n");
        return -1;
    }
    AMIGOS_INFO("url %s, SS_EXP_SinkInit init OK \n", iter->second.url.c_str());
    return 0;
}
int AmigosModuleExp::_Disconnected(unsigned int outPortId, unsigned int ref)
{
    if (ref > 0)
    {
        return 0;
    }
    auto iter = this->mapExpOutInfo.find(outPortId);
    if (iter == this->mapExpOutInfo.end())
    {
        AMIGOS_ERR("Base out port %d not exist\n", outPortId);
        return -1;
    }
    auto itOutDesc = this->mapOutDesc.find(iter->second.url);
    if (itOutDesc == this->mapOutDesc.end())
    {
        AMIGOS_ERR("Base out port %d not exist\n", outPortId);
        return -1;
    }
    if (itOutDesc->second.pInstance)
    {
        SS_EXP_SinkDeinit(itOutDesc->second.pInstance);
        itOutDesc->second.pInstance = nullptr;
    }
    return 0;
}
bool AmigosModuleExp::_IsDelayConnected(unsigned int)
{
    return true;
}

ss_linker_base *AmigosModuleExp::_CreateInputNegativeLinker(unsigned int inPortId)
{
    auto itExpInInfo = this->mapExpInInfo.find(inPortId);
    if (itExpInInfo == this->mapExpInInfo.end())
    {
        AMIGOS_ERR("Input port %d is not exist\n", inPortId);
        return nullptr;
    }
    const ExpInInfo &expInInfo = itExpInInfo->second;
    if (expInInfo.mode == E_EXP_IN_MODE_HAND_SHAKE)
    {
        auto itDesc = this->mapInHandShakeDesc[expInInfo.workId].find(itExpInInfo->second.urlSuffix);
        if (itDesc == this->mapInHandShakeDesc[expInInfo.workId].end())
        {
            AMIGOS_ERR("In port %d, handshake descriptor error.\n", inPortId);
            return nullptr;
        }
        return itDesc->second;
    }
    if (expInInfo.mode == E_EXP_IN_MODE_DIRECT)
    {
        auto itDesc = this->mapInDirectDesc.find(inPortId);
        if (itDesc == this->mapInDirectDesc.end())
        {
            AMIGOS_ERR("In port %d, handshake descriptor error.\n", inPortId);
            return nullptr;
        }
        return itDesc->second;
    }
    AMIGOS_ERR("Mode error in port %d\n", inPortId);
    return nullptr;
}

void AmigosModuleExp::_DestroyInputNegativeLinker(unsigned int inPortId)
{
    auto iter = this->mapPortIn.find(inPortId);
    if (iter == this->mapPortIn.end())
    {
        AMIGOS_ERR("Cannot find input port %d InInfo\n", inPortId);
        return;
    }
    iter->second.negative = nullptr;
    AMIGOS_INFO("Linker will destroy later\n");
}

int AmigosModuleExp::_StartHandShakeIn(unsigned int inPortId, const ExpInInfo &expInInfo, InPortInfo *inInfo)
{
    auto itExp = mapHandShakeInsDesc.find(expInInfo.socketPort);
    if (mapHandShakeInsDesc.end() == itExp)
    {
        AMIGOS_ERR("Hand shake ins is not inited\n");
        return -1;
    }
    // each work id is for the same type.
    auto iter= this->mapInHandShakeDesc[expInInfo.workId].find(expInInfo.urlSuffix);
    if (iter != this->mapInHandShakeDesc[expInInfo.workId].end())
    {
        AMIGOS_ERR("Input port for url %s is exceeded\n", expInInfo.urlSuffix.c_str());
        return -1;
    }
    auto itInInfo = this->mapModInputInfo.find(inPortId);
    if (itInInfo == this->mapModInputInfo.end())
    {
        AMIGOS_ERR("Input port %d is not exist\n", inPortId);
        return -1;
    }
    unsigned int delay = 0;
    if (!itInInfo->second.curFrmRate)
    {
        AMIGOS_WRN("Input FPS is 0, force using 30fps.\n");
        delay = 1000000 * EXP_LINKER_FIFO_MAX_CNT / 30;
    }
    else
    {
        delay = 1000000 * EXP_LINKER_FIFO_MAX_CNT / itInInfo->second.curFrmRate;
    }
    ExpHandShakeInDesc *desc = new ExpHandShakeInDesc(delay, expInInfo.urlSuffix, itExp->second, inInfo, expInInfo.workId);
    if (!desc)
    {
        AMIGOS_ERR("No buffer for new HandShakeDesc\n");
        return -1;
    }
    this->mapInHandShakeDesc[expInInfo.workId][expInInfo.urlSuffix] = desc;
    AMIGOS_INFO("Insert inport %d url %s\n", inPortId, expInInfo.urlSuffix.c_str());
    return 0;
}
void AmigosModuleExp::_StopHandShakeIn(unsigned int inPortId)
{
    auto itExpInInfo = this->mapExpInInfo.find(inPortId);
    if (itExpInInfo == this->mapExpInInfo.end())
    {
        AMIGOS_ERR("Input port %d is not exist\n", inPortId);
        return;
    }
    auto itExp = mapHandShakeInsDesc.find(itExpInInfo->second.socketPort);
    if (mapHandShakeInsDesc.end() == itExp)
    {
        AMIGOS_ERR("Hand shake ins is not inited\n");
        return;
    }
    auto         itDesc     = this->mapInHandShakeDesc[itExpInInfo->second.workId].find(itExpInInfo->second.urlSuffix);
    if (itDesc == this->mapInHandShakeDesc[itExpInInfo->second.workId].end())
    {
        AMIGOS_ERR("Hand shake desc port %d url %s was not exist\n", inPortId, itExpInInfo->second.urlSuffix.c_str());
        return;
    }
    delete itDesc->second;
    this->mapInHandShakeDesc[itExpInInfo->second.workId].erase(itDesc);
    return;
}
int AmigosModuleExp::_StartDirectIn(unsigned int inPortId, const ExpInInfo &expInInfo, InPortInfo *inInfo)
{
    if (this->mapInDirectDesc.end() != this->mapInDirectDesc.find(inPortId))
    {
        AMIGOS_ERR("Direct desc is exist\n");
        return -1;
    }
    auto itInInfo = this->mapModInputInfo.find(inPortId);
    if (itInInfo == this->mapModInputInfo.end())
    {
        AMIGOS_ERR("Input port %d is not exist\n", inPortId);
        return -1;
    }
    unsigned int delay = 0;
    if (!itInInfo->second.curFrmRate)
    {
        AMIGOS_WRN("Input FPS is 0, force using 30fps.\n");
        delay = 1000000 * EXP_LINKER_FIFO_MAX_CNT / 30;
    }
    else
    {
        delay = 1000000 * EXP_LINKER_FIFO_MAX_CNT / itInInfo->second.curFrmRate;
    }
    ExpDirectInDesc *desc = new ExpDirectInDesc(delay, expInInfo.socketPort, inInfo);
    if (!desc)
    {
        AMIGOS_ERR("No buffer for new DescInDesc\n");
        return -1;
    }
    this->mapInDirectDesc[inPortId] = desc;
    return 0;
}
void AmigosModuleExp::_StopDirectIn(unsigned int inPortId)
{
    auto itInDesc = this->mapInDirectDesc.find(inPortId);
    if (itInDesc == this->mapInDirectDesc.end())
    {
        AMIGOS_ERR("Direct desc is not exist\n");
        return;
    }
    delete itInDesc->second;
    this->mapInDirectDesc.erase(itInDesc);
}

AmigosModuleExp::LinkerExpTransferIn::LinkerExpTransferIn(unsigned int retryDelay, InPortInfo *info)
    : inInfo(info), linkerMutex(PTHREAD_MUTEX_INITIALIZER), retryDelay(retryDelay)
{
}

AmigosModuleExp::LinkerExpTransferIn::~LinkerExpTransferIn() {}

void AmigosModuleExp::LinkerExpTransferIn::AddLinker(ss_linker_base *linker)
{
    ss_auto_lock lock(this->linkerMutex);
    if (this->setLinker.end() != this->setLinker.find(linker))
    {
        return;
    }
    this->setLinker.insert(linker);
}
void AmigosModuleExp::LinkerExpTransferIn::DelLinker(ss_linker_base *linker)
{
    ss_auto_lock lock(this->linkerMutex);
    auto         itConnector = this->setLinker.find(linker);
    if (itConnector != this->setLinker.end())
    {
        this->setLinker.erase(itConnector);
    }
    itConnector = this->tryLinker.find(linker);
    if (itConnector != this->tryLinker.end())
    {
        this->tryLinker.erase(itConnector);
    }
}
void AmigosModuleExp::LinkerExpTransferIn::_prepareEnqueue(void)
{
    ss_auto_lock lock(this->linkerMutex);
    this->tryLinker = this->setLinker;
}
int AmigosModuleExp::LinkerExpTransferIn::_tryEnqueue(stream_packet_obj &packet)
{
    int          ret = 0;
    ss_auto_lock lock(this->linkerMutex);
    for (auto iter = this->tryLinker.begin(); iter != this->tryLinker.end();)
    {
        if (0 == (*iter)->enqueue(packet))
        {
            iter = tryLinker.erase(iter);
        }
        else
        {
            ++iter;
            ret = -1;
        }
    }
    return ret;
}

int AmigosModuleExp::LinkerExpTransferIn::enqueue(stream_packet_obj &packet)
{
    this->_prepareEnqueue();
    int ret = this->_tryEnqueue(packet);
    if (ret == 0)
    {
        return ret;
    }
    usleep(this->retryDelay);
    return this->_tryEnqueue(packet);
}

stream_packet_obj AmigosModuleExp::LinkerExpTransferIn::dequeue(unsigned int ms)
{
    return nullptr;
}

int AmigosModuleExp::ExpServerOpen(const char *url, void *privateData, unsigned char bRead, SS_EXP_WorkCfg_t *workCfg)
{
    if (bRead)
    {
        return 0;
    }
    LinkerExpTransferIn *transferIn = static_cast<LinkerExpTransferIn *>(privateData);
    ExpTransferDesc *transferDesc = new ExpTransferDesc(transferIn);
    if (!transferDesc)
    {
        AMIGOS_ERR("new TransferDesc Failed\n");
        return -1;
    }
    workCfg->pUserData = (void *)transferDesc;
    return 0;
}
int AmigosModuleExp::ExpServerClose(void *pUsrData)
{
    ExpTransferDesc *transferDesc = static_cast<ExpTransferDesc *>(pUsrData);
    delete transferDesc;
    return 0;
}
int AmigosModuleExp::ExpServerTransfer(void *pUsrData, SS_EXP_TransferObject_t *transferObj)
{
    ExpTransferDesc *transferDesc = static_cast<ExpTransferDesc *>(pUsrData);
    transferDesc->packet          = transferDesc->nLinker.WaitPacket();
    if (!transferDesc->packet)
    {
        transferDesc->packet = nullptr;
        return -1;
    }
    if (transferDesc->packet->es_vid_i.packet_count > SS_EXP_PACKET_SLICE_CNT)
    {
        transferDesc->packet = nullptr;
        return -1;
    }
    transferObj->packetCount = transferDesc->packet->es_vid_i.packet_count;
    for (unsigned int i = 0; i < transferDesc->packet->es_vid_i.packet_count; i++)
    {
        transferObj->packetArray[i].packetSize = transferDesc->packet->es_vid_i.packet_info[i].size;
        transferObj->packetArray[i].packetData = transferDesc->packet->es_vid.packet_data[i].data;
    }
    return 0;
}
int AmigosModuleExp::ExpServerTransferDone(void *pUsrData, const SS_EXP_TransferObject_t *)
{
    ExpTransferDesc *transferDesc = static_cast<ExpTransferDesc *>(pUsrData);
    transferDesc->packet          = nullptr;
    return 0;
}
int AmigosModuleExp::ExpServerRemoteRead(void *, char *, unsigned int)
{
    return 0;
}
void *AmigosModuleExp::ExpServerRemoteReadAlloc(void *, unsigned int size)
{
    return new char[size];
}
void AmigosModuleExp::ExpServerRemoteReadFree(void *, void *buf)
{
    delete [] static_cast<char *>(buf);
}
int AmigosModuleExp::ExpClientOpen(const char *url, void *privateData, unsigned char bRead, SS_EXP_WorkCfg_t *workCfg)
{
    ExpOutDesc *thisModule = static_cast<ExpOutDesc *>(privateData);
    auto outDesc = new ExpTransferOutDesc();
    ASSERT(outDesc);
    outDesc->linker    = nullptr;
    outDesc->desc      = thisModule;
    workCfg->pUserData = (void *)outDesc;
    if (!bRead)
    {
        // Cycle for 1 second sleep to do heart beat..
        workCfg->cycleSec  = 1;
        workCfg->cycleNSec = 0;
    }
    return 0;
}
int AmigosModuleExp::ExpClientClose(void *pUsrData)
{
    ExpTransferOutDesc *outDesc = static_cast<ExpTransferOutDesc *>(pUsrData);
    delete outDesc;
    outDesc = nullptr;
    return 0;
}
int AmigosModuleExp::ExpClientWorkMsg(void *pUsrData, const char workMsg[SS_EXP_WORK_INFO_MSG_SIZE])
{
    unsigned int workId = 0;
    ExpTransferOutDesc *outDesc = static_cast<ExpTransferOutDesc *>(pUsrData);
    int result = _parseStr(workMsg, outDesc->info, workId);
    if (result == -1)
    {
        AMIGOS_ERR("Parser Str failed, str: %s\n", workMsg);
        return -1;
    }
    outDesc->linker = outDesc->desc->linker[workId];
    outDesc->packer = outDesc->desc->packer[workId];
    return 0;
}
int AmigosModuleExp::ExpClientTransfer(void *, SS_EXP_TransferObject_t *transferObj)
{
    // Send an empty header is used for testing heart beat every 1s
    transferObj->packetArray[0].packetData = nullptr;
    transferObj->packetArray[0].packetSize = 0;
    return 0;
}
int AmigosModuleExp::ExpClientTransferDone(void *, const SS_EXP_TransferObject_t *)
{
    return 0;
}
int AmigosModuleExp::ExpClientRemoteRead(void *pUsrData, char *pTransData, unsigned int u32DataSize)
{
    ExpTransferOutDesc *outDesc = static_cast<ExpTransferOutDesc *>(pUsrData);
    if (!outDesc->linker)
    {
        AMIGOS_ERR("linker is NULL, ins %p\n", outDesc->desc->pInstance);
        return -1;
    }
    if (!outDesc->packet)
    {
        AMIGOS_ERR("Packet is NULL, ins %p\n", outDesc->desc->pInstance);
        return -1;
    }
    if (!outDesc->video_filter.check(outDesc->packet))
    {
        return 0;
    }
    if (outDesc->linker->enqueue(outDesc->packet) != 0)
    {
        outDesc->video_filter.reset();
    }
    return 0;
}
void *AmigosModuleExp::ExpClientRemoteReadAlloc(void *pUsrData, unsigned int size)
{
    ExpTransferOutDesc *outDesc = static_cast<ExpTransferOutDesc *>(pUsrData);

    if (outDesc->info.en_type == EN_VIDEO_CODEC_DATA)
    {
        outDesc->info.es_vid_i.packet_count = 1;
        outDesc->info.es_vid_i.packet_info[0].size  = size;
        outDesc->info.es_vid_i.packet_info[0].b_end = true;
        outDesc->packet = outDesc->packer->make(outDesc->info);
        return outDesc->packet->es_vid.packet_data[0].data;
    }
    if (outDesc->info.en_type == EN_AUDIO_CODEC_DATA)
    {
        outDesc->info.es_aud_i.packet_count = 1;
        outDesc->info.es_aud_i.packet_info[0].size = size;
        outDesc->packet = outDesc->packer->make(outDesc->info);
        return outDesc->packet->es_aud.packet_data[0].data;
    }
    AMIGOS_ERR("Client packet data type %d is NULL!\n", outDesc->info.en_type);
    return NULL;
}
void AmigosModuleExp::ExpClientRemoteReadFree(void *pUsrData, void *buf)
{
    ExpTransferOutDesc *outDesc = static_cast<ExpTransferOutDesc *>(pUsrData);
    outDesc->packet = nullptr;
}
static unsigned int _GetSampleWidth(std::string str)
{
    if (str == "g711u" || str == "g711a")
    {
        return 8;
    }
    if (str == "g726_16")
    {
        return 2;
    }
    if (str == "g726_24")
    {
        return 3;
    }
    if (str == "g726_32")
    {
        return 4;
    }
    if (str == "g726_40")
    {
        return 5;
    }
    return 16;
}
static unsigned int _GetChannels(std::string str)
{
    if (str == "mono")
    {
        return 1;
    }
    if (str == "stereo")
    {
        return 2;
    }
    if (str == "4ch")
    {
        return 4;
    }
    if (str == "6ch")
    {
        return 6;
    }
    if (str == "8ch")
    {
        return 8;
    }
    return 1;
}
stream_packet_info AmigosModuleExp::_GetStreamInfo(unsigned int outPortId)
{
    struct stream_packet_info streamInfo;
    auto iterOutInfo = this->mapExpOutInfo.find(outPortId);
    if (iterOutInfo == this->mapExpOutInfo.end())
    {
        AMIGOS_ERR("Can't find the output info!\n");
        return stream_packet_info();
    }
    if (iterOutInfo->second.portType == "video")
    {
        streamInfo.en_type         = EN_VIDEO_CODEC_DATA;
        streamInfo.es_vid_i.fmt    = ss_enum_cast<es_video_fmt>::from_str(iterOutInfo->second.fmt);
        streamInfo.es_vid_i.width  = iterOutInfo->second.width;
        streamInfo.es_vid_i.height = iterOutInfo->second.height;
    }
    else if (iterOutInfo->second.portType == "audio")
    {
        streamInfo.en_type               = EN_AUDIO_CODEC_DATA;
        streamInfo.es_aud_i.fmt          = ss_enum_cast<es_audio_fmt>::from_str(iterOutInfo->second.fmt);
        streamInfo.es_aud_i.channels     = _GetChannels(iterOutInfo->second.channel);
        streamInfo.es_aud_i.sample_rate  = iterOutInfo->second.samplerate;
        streamInfo.es_aud_i.sample_width = _GetSampleWidth(iterOutInfo->second.fmt);
    }
    return streamInfo;
}
AMIGOS_MODULE_INIT("EXP", AmigosModuleExp);
