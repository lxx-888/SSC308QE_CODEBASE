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

#ifndef __AMIGOS_MODULE_BASE_H__
#define __AMIGOS_MODULE_BASE_H__

#include <cstdio>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <iostream>
#include <unordered_map>
#include "ss_message.h"
#include "ss_linker.h"
#include "amigos_surface_base.h"
#include "ss_enum_cast.hpp"
#include "ss_packet.h"
#include "amigos_env.h"

#define AMIGOS_DIV_STR_FOR_EACH(__in, __key, __dst)                                                                           \
    for(const char *__cur = (__in), *__next = strstr(__in, (__key));                                                          \
        (__cur != NULL && *__cur != '\0')? ((__next? __dst.assign(__cur, __next - __cur): __dst.assign(__cur)), true): false; \
        __next = __next? (__cur = __next + 1, strstr(__next + 1, __key)): __cur = NULL)

SS_ENUM_CAST_STR(stream_type, {
    { EN_RAW_FRAME_DATA   , "raw"   },
    { EN_VIDEO_CODEC_DATA , "video" },
    { EN_AUDIO_CODEC_DATA , "audio" },
    { EN_USER_META_DATA   , "user"  },
});

SS_ENUM_CAST_STR(es_video_fmt, {
    { ES_STREAM_H264 , "h264" },
    { ES_STREAM_H265 , "h265" },
    { ES_STREAM_JPEG , "jpeg" },
    { ES_STREAM_AV1  , "av1"  },
    { ES_STREAM_VP9  , "vp9"  },
});

SS_ENUM_CAST_STR(es_audio_fmt, {
    { ES_STREAM_PCM      , "pcm"     },
    { ES_STREAM_AAC      , "aac"     },
    { ES_STREAM_WAV      , "wav"     },
    { ES_STREAM_G711U    , "g711u"   },
    { ES_STREAM_G711A    , "g711a"   },
    { ES_STREAM_G726_16  , "g726_16" },
    { ES_STREAM_G726_24  , "g726_24" },
    { ES_STREAM_G726_32  , "g726_32" },
    { ES_STREAM_G726_40  , "g726_40" },
});

SS_ENUM_CAST_STR(raw_bayer_id, {
    { RAW_PIXEL_BAYERID_MAX , "NA"   },
    { RAW_PIXEL_BAYERID_RG  , "rg"   },
    { RAW_PIXEL_BAYERID_GR  , "gr"   },
    { RAW_PIXEL_BAYERID_BG  , "bg"   },
    { RAW_PIXEL_BAYERID_GB  , "gb"   },
    { RAW_PIXEL_RGBIR_R0    , "r0"   },
    { RAW_PIXEL_RGBIR_G0    , "g0"   },
    { RAW_PIXEL_RGBIR_B0    , "b0"   },
    { RAW_PIXEL_RGBIR_G1    , "g1"   },
    { RAW_PIXEL_RGBIR_G2    , "g2"   },
    { RAW_PIXEL_RGBIR_I0    , "i0"   },
    { RAW_PIXEL_RGBIR_G3    , "g3"   },
    { RAW_PIXEL_RGBIR_I1    , "i1"   },
});

SS_ENUM_CAST_STR(raw_data_precision , {
    { RAW_PRECISION_MAX   , "NA"    },
    { RAW_PRECISION_8BPP  , "8bpp"  },
    { RAW_PRECISION_10BPP , "10bpp" },
    { RAW_PRECISION_12BPP , "12bpp" },
    { RAW_PRECISION_14BPP , "14bpp" },
    { RAW_PRECISION_16BPP , "16bpp" },
});


template <>
class ss_enum_cast<raw_video_fmt>
{
public:
    static std::string to_str(const raw_video_fmt &fmt)
    {
        if (fmt >= RAW_FORMAT_BAYER_BASE && fmt <= RAW_FORMAT_BAYER_NUM)
        {
            std::stringstream ss;
            raw_bayer_id       bayer_id = (raw_bayer_id)((fmt - RAW_FORMAT_BAYER_BASE) % RAW_PIXEL_BAYERID_MAX);
            raw_data_precision data_precision =
                (raw_data_precision)((fmt - RAW_FORMAT_BAYER_BASE) / RAW_PIXEL_BAYERID_MAX);
            ss << "bayer_" << ss_enum_cast<raw_bayer_id>::to_str(bayer_id) << "_"
               << ss_enum_cast<raw_data_precision>::to_str(data_precision);
            return ss.str();
        }
        static const std::unordered_map<raw_video_fmt, std::string> m = {
            {RAW_FORMAT_YUV422_YUYV   , "yuyv"     } ,
            {RAW_FORMAT_YUV422_UYVY   , "uyvy"     } ,
            {RAW_FORMAT_YUV422_YVYU   , "yvyu"     } ,
            {RAW_FORMAT_YUV422_VYUY   , "vyuy"     } ,
            {RAW_FORMAT_YUV422SP      , "yuv422sp" } ,
            {RAW_FORMAT_YUV420SP      , "nv12"     } ,
            {RAW_FORMAT_YUV420SP_NV21 , "nv21"     } ,
            {RAW_FORMAT_RGB888        , "rgb888"   } ,
            {RAW_FORMAT_BGR888        , "bgr888"   } ,
            {RAW_FORMAT_ARGB8888      , "argb8888" } ,
            {RAW_FORMAT_ABGR8888      , "abgr8888" } ,
            {RAW_FORMAT_BGRA8888      , "bgra8888" } ,
            {RAW_FORMAT_RGB565        , "rgb565"   } ,
            {RAW_FORMAT_ARGB1555      , "argb1555" } ,
            {RAW_FORMAT_ARGB4444      , "argb4444" } ,
            {RAW_FORMAT_I2            , "i2"       } ,
            {RAW_FORMAT_I4            , "i4"       } ,
            {RAW_FORMAT_I8            , "i8"       } ,
        };
        auto it = m.find(fmt);
        return it != m.end() ? it->second : "Unknown";
    }
    static raw_video_fmt from_str(const std::string &str)
    {
        static const std::unordered_map<std::string, raw_video_fmt> m = {
            {"yuyv"     , RAW_FORMAT_YUV422_YUYV   } ,
            {"uyvy"     , RAW_FORMAT_YUV422_UYVY   } ,
            {"yvyu"     , RAW_FORMAT_YUV422_YVYU   } ,
            {"vyuy"     , RAW_FORMAT_YUV422_VYUY   } ,
            {"yuv422sp" , RAW_FORMAT_YUV422SP      } ,
            {"nv12"     , RAW_FORMAT_YUV420SP      } ,
            {"nv21"     , RAW_FORMAT_YUV420SP_NV21 } ,
            {"rgb888"   , RAW_FORMAT_RGB888        } ,
            {"bgr888"   , RAW_FORMAT_BGR888        } ,
            {"argb8888" , RAW_FORMAT_ARGB8888      } ,
            {"abgr8888" , RAW_FORMAT_ABGR8888      } ,
            {"bgra8888" , RAW_FORMAT_BGRA8888      } ,
            {"rgb565"   , RAW_FORMAT_RGB565        } ,
            {"argb1555" , RAW_FORMAT_ARGB1555      } ,
            {"argb4444" , RAW_FORMAT_ARGB4444      } ,
            {"i2"       , RAW_FORMAT_I2            } ,
            {"i4"       , RAW_FORMAT_I4            } ,
            {"i8"       , RAW_FORMAT_I8            } ,
        };
        auto it = m.find(str);
        return it != m.end() ? it->second : RAW_FORMAT_MAX;
    }
    static raw_video_fmt from_str_bayer(const std::string &str,const std::string &bayer_id,const std::string &precision)
    {
        return raw_video_fmt(RAW_FORMAT_BAYER_BASE
                             + ss_enum_cast<raw_data_precision>::from_str(precision)
                                   * RAW_PIXEL_BAYERID_MAX
                             + ss_enum_cast<raw_bayer_id>::from_str(bayer_id));
    }
};

class AmigosModuleBase
{
protected:
    class LinkerBypass : public ss_linker_base
    {
    public:
        explicit LinkerBypass(ss_linker_base *linker);
        ~LinkerBypass() override;
    private:
        int enqueue(stream_packet_obj &packet) override;
        stream_packet_obj dequeue(unsigned int ms) override;
        ss_linker_base *holdLinker;
    };
    class LinkerPostReader : public ss_linker_base
    {
    public:
        LinkerPostReader();
        ~LinkerPostReader() override;
        void SetTarget(ss_linker_base *tgt);
        bool empty();
        int enqueue(stream_packet_obj &packet) override;
        stream_packet_obj dequeue(unsigned int ms) override;
    private:
        pthread_rwlock_t linkerMutex;
        ss_linker_base *targetLinker;
    };
    class LinkerGroup : public ss_linker_base
    {
    public:
        LinkerGroup();
        ~LinkerGroup() override;
        void AddLinker(ss_linker_base *pLinker);
        void DelLinker(ss_linker_base *pLinker);
        bool empty();
        int enqueue(stream_packet_obj &packet) override;
        stream_packet_obj dequeue(unsigned int ms) override;
    private:
        pthread_rwlock_t linkerMutex;
        std::set<ss_linker_base *> setLinker;
    };
    class LinkerAsyncNegative : public ss_linker_base
    {
    public:
        explicit LinkerAsyncNegative(bool bDropMsg, unsigned int depth);
        virtual ~LinkerAsyncNegative();
        virtual int enqueue(stream_packet_obj &packet) override;
        virtual stream_packet_obj dequeue(unsigned int ms) override;
        stream_packet_obj WaitPacket(unsigned int ms = 100);
        int FlushPacket();
        int EndMonitor();
    private:
        int _Sleep(unsigned int timeOut);
        pthread_mutex_t    linkerMutex;
        pthread_condattr_t linkerCondAttr;
        pthread_cond_t     linkerCond;
        std::list<stream_packet_obj> packetList;
        bool dropMsg;
        unsigned int packetDepth;
        bool endLoop;
    };
    class LinkerSyncNegative : public ss_linker_base
    {
    public:
        explicit LinkerSyncNegative(unsigned int port, AmigosModuleBase *thisModule);
        virtual ~LinkerSyncNegative() override;
        virtual int enqueue(stream_packet_obj &packet) override;
        virtual stream_packet_obj dequeue(unsigned int ms) override;
    private:
        unsigned int inPortId;
        AmigosModuleBase *thisModule;
    };
    class LinkerOutNegative : public ss_linker_base
    {
    public:
        explicit LinkerOutNegative(unsigned int port, AmigosModuleBase *thisModule);
        virtual ~LinkerOutNegative() override;
    private:
        int enqueue(stream_packet_obj &packet) override;
        stream_packet_obj dequeue(unsigned int ms) override;
        std::list<stream_packet_obj> packetList;
        pthread_mutex_t  linkerMutex;
        unsigned int outPortId;
        AmigosModuleBase *thisModule;
    };
    class StreamPackerNormal : public stream_packer
    {
    public:
        explicit StreamPackerNormal();
        virtual ~StreamPackerNormal();
        stream_packet_obj make(const stream_packet_info &packet_info)  override;
    };
    class StreamPackerBypass : public stream_packer
    {
    public:
        explicit StreamPackerBypass(stream_packer *packer);
        virtual ~StreamPackerBypass();
        stream_packet_obj make(const stream_packet_info &packet_info)  override;
    private:
        stream_packer *storedPacker;
    };
    class StreamPackerThrough : public stream_packer
    {
    public:
        explicit StreamPackerThrough();
        virtual ~StreamPackerThrough();
        enum PackerGroupType
        {
            E_GROUP_FAST,
            E_GROUP_SLOW
        };
        void AddPacker(stream_packer *subMaker, enum PackerGroupType type);
        void DelPacker(stream_packer *subMaker);
        stream_packet_obj make(const stream_packet_info &packet_info)  override;
    private:
        pthread_mutex_t makerMutex;
        std::set<stream_packer *> setFastMaker;
        std::set<stream_packer *> setSlowMaker;
    };
    class OutPortInfo : public ss_message
    {
    public:
        explicit OutPortInfo(AmigosModuleBase *pModule, unsigned int outPortId);
        virtual ~OutPortInfo();
        stream_packet_info get_packet_info() override;
        AmigosModuleBase    *pModule;
        unsigned int        outPortId;
        LinkerGroup         positive;
        ss_linker_base      *negative;
        StreamPackerThrough outPacker;
        std::string         strOutIdStr;
        std::map<AmigosModuleBase *, std::set<unsigned int>> mapNext;
        bool               bStart;
    private:
        void connected(unsigned int ref) override;
        void disconnected(unsigned int ref) override;
    };
    class InPortInfo : public ss_message
    {
    public:
        explicit InPortInfo(AmigosModuleBase *pModule, unsigned int inPortId,
                            unsigned int prevOutPortId = -1, AmigosModuleBase *pPrev = nullptr);
        virtual ~InPortInfo();
        stream_packet_info get_packet_info() override;
        unsigned int       BindType();
        AmigosModuleBase *pModule;
        unsigned int     inPortId;
        unsigned int     prevOutPortId;
        AmigosModuleBase *pPrev;
        LinkerPostReader positive;
        ss_linker_base   *negative;
        stream_packer    *packer;
        std::string      strInIdStr;
        bool             bStart;
    private:
        void connected(unsigned int ref) override;
        void disconnected(unsigned int ref) override;
        void _BindUser(OutPortInfo &outInfo);
        void _UnBindUser(OutPortInfo &outInfo);
    };

    class ModuleState
    {
    public:
        const int MODULE_STATE_NONE       = 0x0000;
        const int MODULE_STATE_INITED     = 0x0001;
        const int MODULE_STATE_PREPARED   = 0x0002;
        const int MODULE_STATE_UNPREPARED = 0x0003;
        const int MODULE_STATE_STARTED    = 0x0004;
        const int MODULE_STATE_STOPED     = 0x0005;
        ModuleState() : state(MODULE_STATE_NONE), bDualOsFirstRun(false) {}
        void Init(unsigned int preload);
        void Deinit();
        bool Skip() const;
        bool Check(int state) const;
        bool Change(int nextState);
        int Get() const;
        bool IsPreload() const;
        bool IsMultiProcess() const;
    private:
        const int MODULE_STATE_MASK              = 0x000f;
        const int MODULE_STATE_SECOND_MASK       = 0x0010;
        const int MODULE_STATE_SKIP_MASK         = 0x0020;
        const int MODULE_STATE_PROCESS_SKIP_MASK = 0x0040;
        int state;
        bool bDualOsFirstRun;
    };
    enum
    {
        E_MOD_PORT_TYPE_NONE     = 0x0,
        E_MOD_PORT_TYPE_KERNEL   = 0x1,
        E_MOD_PORT_TYPE_USER     = 0x2,
    };

    const unsigned int uintExtModId = (unsigned int)-1;

private:
    struct DelayPassInfo
    {
        AmigosModuleBase   *pPrev;
        unsigned int       prevOutPortId;
        DelayPassInfo()
            : pPrev(NULL), prevOutPortId(-1)
        {}
    };
    std::map<unsigned int, struct DelayPassInfo> mapDelayPassInfo;
    AmigosSurfaceBase *pSurface;
    void _BindBlock(unsigned int inPortId, AmigosModuleBase::InPortInfo &inInfo);
    void _UnBindBlock(unsigned int inPortId, AmigosModuleBase::InPortInfo &inInfo);
    void _ResetStreamTraverse(unsigned int outPortId, unsigned int bindType, unsigned int width, unsigned int height);

public:
    explicit AmigosModuleBase(AmigosSurfaceBase *pSurface);
    virtual ~AmigosModuleBase();

    AmigosSurfaceBase *GetSurface() const;

    const std::string &GetModIdStr() const;
    const std::string &GetInPortIdStr(unsigned int inPortId) const;
    const std::string &GetOutPortIdStr(unsigned int outPortId) const;

    void SetEnv();

    void Init();
    void Deinit();

    void Bind();
    void UnBind();
    void BindBlock(unsigned int inPortId);
    void UnBindBlock(unsigned int inPortId);

    void Prepare();
    void Unprepare();

    void Start();
    void Stop();

    void StartIn();
    void StartIn(unsigned int inPortId);
    void StopIn();
    void StopIn(unsigned int inPortId);

    void StartOut();
    void StartOut(unsigned int outPortId);
    void StopOut();
    void StopOut(unsigned int outPortId);

    void ResetStream(unsigned int inPortId, unsigned int width, unsigned int height);
    int CreateDelayPass(unsigned int inPortId);
    int DestroyDelayPass(unsigned int inPortId);
    int InitDelayPass(unsigned int inPortId);
    int DeinitDelayPass(unsigned int inPortId);
    int BindDelayPass(unsigned int inPortId);
    int UnbindDelayPass(unsigned int inPortId);
    int StartDelayPass(unsigned int inPortId);
    int StopDelayPass(unsigned int inPortId);

    void CreateIo();
    void DestroyIo();
    void Link(unsigned int inPortId, unsigned int prevOutId, AmigosModuleBase *pPrev);
    void Unlink();
    void Unlink(unsigned int inPortId);

    bool IsPreload() const;
    void DoDualOsPipeCli(const std::string &cmd);
    void DoDualOsModCli(const std::string &cmd);

    virtual unsigned int GetModId()                  const      = 0;
    virtual unsigned int GetInputType(unsigned int port) const  = 0;
    virtual unsigned int GetOutputType(unsigned int port) const = 0;

protected:
    void _SetPrev(unsigned int prevOutId, unsigned int inPortId, AmigosModuleBase *pPrev);
    void _SetNext(unsigned int outPortId, unsigned int nextInPortId, AmigosModuleBase *pNext);
    void _UnsetPrev(unsigned int inPortId);
    void _UnsetNext(unsigned int outPortId, unsigned int nextInPortId, AmigosModuleBase *pNext);

protected:
    virtual void _Init();
    virtual void _Deinit();
    virtual void _ResourceInit();
    virtual void _ResourceDeinit();

    virtual void _DirectBind(unsigned int inPortId, unsigned int prevOutPortId, const AmigosModuleBase *pPrev);
    virtual void _DirectUnBind(unsigned int inPortId, unsigned int prevOutPortId, const AmigosModuleBase *pPrev);

    virtual bool _IsPostReader(unsigned int inPortId);
    virtual bool _IsDelayConnected(unsigned int uintInPort);

    // Sync input.
    virtual int _DataReceiver(unsigned int inPortId, stream_packet_obj &packet);
    // Sync output.
    virtual int _EnqueueOut(unsigned int outPortId, stream_packet_obj &packet);
    virtual int _DequeueOut(unsigned int outPortId, stream_packet_obj &packet);
    virtual stream_packet_obj _DequeueFromInside(unsigned int outPortId, unsigned int ms);

    virtual void _Prepare();
    virtual void _Unprepare();

    virtual void _Start();
    virtual void _Stop();

    virtual void _StartIn(unsigned int inPortId);
    virtual void _StopIn(unsigned int inPortId);

    virtual void _StartOut(unsigned int outPortId);
    virtual void _StopOut(unsigned int outPortId);

    virtual int _Connected(unsigned int outPortId, unsigned int ref);
    virtual int _Disconnected(unsigned int outPortId, unsigned int ref);

    virtual int _InConnect(unsigned int inPortId);
    virtual int _InDisconnect(unsigned int inPortId);

    virtual int _ConnectedTransfer(unsigned int outPortId);
    virtual int _DisconnectedTransfer(unsigned int outPortId);

    virtual bool _NeedResetStreamTraverse(unsigned int outPortId, unsigned int bindType, unsigned int &inPortId);
    virtual void _ResetStreamOut(unsigned int outPortId, unsigned int width, unsigned int height);
    virtual stream_packet_info _GetStreamInfo(unsigned int outPortId);

    virtual bool _NeedMarkDeinitOnRtos();
    virtual bool _NeedMarkStopOnRtos();
    virtual bool _NeedMarkUnprepareOnRtos();
    virtual bool _NeedMarkStopInOnRtos(unsigned int inPortId);
    virtual bool _NeedMarkUnbindOnRtos(unsigned int inPortId);
    virtual bool _NeedMarkStopOutOnRtos(unsigned int outPortId);

    virtual int _CreateDelayPass(unsigned int outPortId, unsigned int postInId, AmigosModuleBase *postIns);
    virtual int _DestroyDelayPass(unsigned int outPortId, unsigned int postInId, AmigosModuleBase *postIns);
    virtual int _InitDelayPass(unsigned int outPortId);
    virtual int _DeinitDelayPass(unsigned int outPortId);
    virtual int _BindDelayPass(unsigned int outPortId);
    virtual int _UnbindDelayPass(unsigned int outPortId);
    virtual int _StartDelayPass(unsigned int outPortId);
    virtual int _StopDelayPass(unsigned int outPortId);

    virtual ss_linker_base *_CreateInputNegativeLinker(unsigned int inPortId);
    virtual ss_linker_base *_CreateOutputNegativeLinker(unsigned int outPortId);
    virtual stream_packer  *_CreateInputStreamPacker(unsigned int inPortId, bool &bFast);
    virtual void _DestroyInputNegativeLinker(unsigned int inPortId);
    virtual void _DestroyOutputNegativeLinker(unsigned int outPortId);
    virtual void _DestroyInputerStreamPacket(unsigned int inPortId);

public:
    static AmigosModuleBase *GetModule(const std::string &strSec);
    std::map<unsigned int, AmigosModuleBase::InPortInfo>  mapPortIn;
    std::map<unsigned int, AmigosModuleBase::OutPortInfo> mapPortOut;

protected:
    AmigosEnv env;
    ModuleState state;

private:
    void _StartInLoop(std::map<unsigned int, AmigosModuleBase::InPortInfo>::iterator &iter);
    void _StopInLoop(std::map<unsigned int, AmigosModuleBase::InPortInfo>::iterator &iter);
    void _StartOutLoop(std::map<unsigned int, AmigosModuleBase::OutPortInfo>::iterator &iter);
    void _StopOutLoop(std::map<unsigned int, AmigosModuleBase::OutPortInfo>::iterator &iter);
    static pthread_mutex_t lifeCycleMutex;
    static std::map<std::string, AmigosModuleBase *> connectMap;
};

#endif
