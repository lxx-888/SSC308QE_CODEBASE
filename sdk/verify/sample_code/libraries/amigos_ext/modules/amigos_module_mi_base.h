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

#ifndef __AMIGOS_MODULE_MI_BASE_H__
#define __AMIGOS_MODULE_MI_BASE_H__

#include <map>
#include <string>
#include "mi_common_datatype.h"
#include "mi_sys_datatype.h"
#include "ss_enum_cast.hpp"
#include "ss_packet.h"
#if INTERFACE_SENSOR
#include "mi_sensor.h"
#endif
#include "amigos_surface_base.h"
#include "amigos_module_base.h"
#include "amigos_env.h"

SS_ENUM_CAST_STR(MI_SYS_BayerId_e, {
    { E_MI_SYS_PIXEL_BAYERID_MAX , "NA" },
    { E_MI_SYS_PIXEL_BAYERID_RG  , "rg" },
    { E_MI_SYS_PIXEL_BAYERID_GR  , "gr" },
    { E_MI_SYS_PIXEL_BAYERID_BG  , "bg" },
    { E_MI_SYS_PIXEL_BAYERID_GB  , "gb" },
    { E_MI_SYS_PIXEL_RGBIR_R0    , "r0" },
    { E_MI_SYS_PIXEL_RGBIR_G0    , "g0" },
    { E_MI_SYS_PIXEL_RGBIR_B0    , "b0" },
    { E_MI_SYS_PIXEL_RGBIR_G1    , "g1" },
    { E_MI_SYS_PIXEL_RGBIR_G2    , "g2" },
    { E_MI_SYS_PIXEL_RGBIR_I0    , "i0" },
    { E_MI_SYS_PIXEL_RGBIR_G3    , "g3" },
    { E_MI_SYS_PIXEL_RGBIR_I1    , "i1" },
});

SS_ENUM_CAST_STR(MI_SYS_DataPrecision_e, {
    { E_MI_SYS_DATA_PRECISION_MAX   , "NA"    },
    { E_MI_SYS_DATA_PRECISION_8BPP  , "8bpp"  },
    { E_MI_SYS_DATA_PRECISION_10BPP , "10bpp" },
    { E_MI_SYS_DATA_PRECISION_12BPP , "12bpp" },
    { E_MI_SYS_DATA_PRECISION_14BPP , "14bpp" },
    { E_MI_SYS_DATA_PRECISION_16BPP , "16bpp" },
});

SS_ENUM_CAST_STR(MI_SYS_CompressMode_e, {
    { E_MI_SYS_COMPRESS_MODE_NONE    , "na"   },
    { E_MI_SYS_COMPRESS_MODE_SEG     , "seg"  },
    { E_MI_SYS_COMPRESS_MODE_LINE    , "line" },
    { E_MI_SYS_COMPRESS_MODE_FRAME   , "frame"},
    { E_MI_SYS_COMPRESS_MODE_TO_8BIT , "8bit" },
    { E_MI_SYS_COMPRESS_MODE_TO_6BIT , "6bit" },
    { E_MI_SYS_COMPRESS_MODE_IFC     , "ifc" },
    { E_MI_SYS_COMPRESS_MODE_SFBC0   , "sfbc0"},
    { E_MI_SYS_COMPRESS_MODE_SFBC1   , "sfbc1"},
    { E_MI_SYS_COMPRESS_MODE_SFBC2   , "sfbc2"},
});

template <>
class ss_enum_cast<MI_SYS_PixelFormat_e>
{
public:
    static std::string to_str(const MI_SYS_PixelFormat_e &fmt)
    {
        if (fmt >= E_MI_SYS_PIXEL_FRAME_RGB_BAYER_BASE && fmt <= E_MI_SYS_PIXEL_FRAME_RGB_BAYER_NUM)
        {
            std::stringstream ss;
            MI_SYS_BayerId_e bayer_id = (MI_SYS_BayerId_e)((fmt - RAW_FORMAT_BAYER_BASE) % E_MI_SYS_PIXEL_BAYERID_MAX);
            MI_SYS_DataPrecision_e data_precision =
                (MI_SYS_DataPrecision_e)((fmt - RAW_FORMAT_BAYER_BASE) / E_MI_SYS_PIXEL_BAYERID_MAX);
            ss << "bayer_" << ss_enum_cast<MI_SYS_BayerId_e>::to_str(bayer_id) << "_"
               << ss_enum_cast<MI_SYS_DataPrecision_e>::to_str(data_precision);
            return ss.str();
        }
        static const std::unordered_map<MI_SYS_PixelFormat_e, std::string> m = {
            { E_MI_SYS_PIXEL_FRAME_YUV422_YUYV             , "yuyv"     } ,
            { E_MI_SYS_PIXEL_FRAME_YUV422_UYVY             , "uyvy"     } ,
            { E_MI_SYS_PIXEL_FRAME_YUV422_YVYU             , "yvyu"     } ,
            { E_MI_SYS_PIXEL_FRAME_YUV422_VYUY             , "vyuy"     } ,
            { E_MI_SYS_PIXEL_FRAME_YUV422_PLANAR           , "yuv422sp" } ,
            { E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420      , "nv12"     } ,
            { E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420_NV21 , "nv21"     } ,
            { E_MI_SYS_PIXEL_FRAME_RGB888                  , "rgb888"   } ,
            { E_MI_SYS_PIXEL_FRAME_BGR888                  , "bgr888"   } ,
            { E_MI_SYS_PIXEL_FRAME_ARGB8888                , "argb8888" } ,
            { E_MI_SYS_PIXEL_FRAME_ABGR8888                , "abgr8888" } ,
            { E_MI_SYS_PIXEL_FRAME_BGRA8888                , "bgra8888" } ,
            { E_MI_SYS_PIXEL_FRAME_RGB565                  , "rgb565"   } ,
            { E_MI_SYS_PIXEL_FRAME_ARGB1555                , "argb1555" } ,
            { E_MI_SYS_PIXEL_FRAME_ARGB4444                , "argb4444" } ,
            { E_MI_SYS_PIXEL_FRAME_I2                      , "i2"       } ,
            { E_MI_SYS_PIXEL_FRAME_I4                      , "i4"       } ,
            { E_MI_SYS_PIXEL_FRAME_I8                      , "i8"       } ,
        };
        auto it = m.find(fmt);
        return it != m.end() ? it->second : "Unknown";
    }
    static MI_SYS_PixelFormat_e from_str(const std::string &str)
    {
        static const std::unordered_map<std::string, MI_SYS_PixelFormat_e> m = {
            { "yuyv"     , E_MI_SYS_PIXEL_FRAME_YUV422_YUYV             } ,
            { "uyvy"     , E_MI_SYS_PIXEL_FRAME_YUV422_UYVY             } ,
            { "yvyu"     , E_MI_SYS_PIXEL_FRAME_YUV422_YVYU             } ,
            { "vyuy"     , E_MI_SYS_PIXEL_FRAME_YUV422_VYUY             } ,
            { "yuv422sp" , E_MI_SYS_PIXEL_FRAME_YUV422_PLANAR           } ,
            { "nv12"     , E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420      } ,
            { "nv21"     , E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420_NV21 } ,
            { "rgb888"   , E_MI_SYS_PIXEL_FRAME_RGB888                  } ,
            { "bgr888"   , E_MI_SYS_PIXEL_FRAME_BGR888                  } ,
            { "argb8888" , E_MI_SYS_PIXEL_FRAME_ARGB8888                } ,
            { "abgr8888" , E_MI_SYS_PIXEL_FRAME_ABGR8888                } ,
            { "bgra8888" , E_MI_SYS_PIXEL_FRAME_BGRA8888                } ,
            { "rgb565"   , E_MI_SYS_PIXEL_FRAME_RGB565                  } ,
            { "argb1555" , E_MI_SYS_PIXEL_FRAME_ARGB1555                } ,
            { "argb4444" , E_MI_SYS_PIXEL_FRAME_ARGB4444                } ,
            { "i2"       , E_MI_SYS_PIXEL_FRAME_I2                      } ,
            { "i4"       , E_MI_SYS_PIXEL_FRAME_I4                      } ,
            { "i8"       , E_MI_SYS_PIXEL_FRAME_I8                      } ,
        };
        auto it = m.find(str);
        return it != m.end() ? it->second : E_MI_SYS_PIXEL_FRAME_FORMAT_MAX;
    }
    static MI_SYS_PixelFormat_e from_str_bayer(const std::string &str,const std::string &bayer_id,const std::string &precision)
    {
        return (MI_SYS_PixelFormat_e)(E_MI_SYS_PIXEL_FRAME_RGB_BAYER_BASE
                                      + ss_enum_cast<MI_SYS_DataPrecision_e>::from_str(precision)
                                            * E_MI_SYS_PIXEL_BAYERID_MAX
                                      + ss_enum_cast<MI_SYS_BayerId_e>::from_str(bayer_id));
    }
};

class AmigosModuleMiBase : public AmigosModuleBase
{
    friend void *MiSysReader(struct ss_thread_buffer *thread_buf);
protected:
    class StreamPacketSysMma final : public stream_packet_base
    {
    public:
        class PacketErrBuf : public err_buf
        {
            void show() const override;
        };
        explicit StreamPacketSysMma(const struct raw_video_info &raw_i);
        ~StreamPacketSysMma() override;
    private:
        stream_packet_obj do_convert(stream_packet_obj &self, enum stream_type type) override;
    };
    class StreamPackerSysMma final : public stream_packer
    {
    public:
        StreamPackerSysMma() {}
        ~StreamPackerSysMma() override {}
        stream_packet_obj make(const stream_packet_info &packet_info) override;
    };
private:
    struct MiBaseReaderDesc
    {
        MI_SYS_ChnPort_t stChnPort;
        void             *threadHandle;
        LinkerGroup      *outPlinker;
        MiBaseReaderDesc()
        {
            memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));
            threadHandle = nullptr;
            outPlinker   = nullptr;
        }
    };
    class LinkerMiSysIn : public ss_linker_base
    {
    public:
        explicit LinkerMiSysIn(const MI_SYS_ChnPort_t &chnPort);
        ~LinkerMiSysIn() override;
    private:
        int enqueue(stream_packet_obj &packet) override;
        stream_packet_obj dequeue(unsigned int ms) override;
        MI_SYS_ChnPort_t stChnPort;
    };
    class StreamPackerMiSysIn final : public stream_packer
    {
    public:
        StreamPackerMiSysIn(const MI_SYS_ChnPort_t &chnPort);
        ~StreamPackerMiSysIn() override;
        stream_packet_obj make(const stream_packet_info &packet_info) override;
    private:
        MI_SYS_ChnPort_t stChnPort;
    };
public:

    public:
        explicit AmigosModuleMiBase(AmigosSurfaceBase *pSurface);
        ~AmigosModuleMiBase() override;

    protected:
        void _DirectBind(unsigned int inPortId, unsigned int prevOutPortId, const AmigosModuleBase *pPrev) override;
        void _DirectUnBind(unsigned int inPortId, unsigned int prevOutPortId, const AmigosModuleBase *pPrev) override;

        int _DataReceiver(unsigned int inPortId, stream_packet_obj &packet) override;
        int _Connected(unsigned int outPortId, unsigned int ref) override;
        int _Disconnected(unsigned int outPortId, unsigned int ref) override;

        void _StartIn(unsigned int inPortId) override;
        void _StopIn(unsigned int inPortId) override;

        void _StartOut(unsigned int outPortId) override;
        void _StopOut(unsigned int outPortId) override;

        virtual void _StartMiIn(unsigned int outPortId);
        virtual void _StopMiIn(unsigned int outPortId);
        virtual void _StartMiOut(unsigned int outPortId);
        virtual void _StopMiOut(unsigned int outPortId);

        int _EnqueueOut(unsigned int outPortId, stream_packet_obj &packet) override;
        int _DequeueOut(unsigned int outPortId, stream_packet_obj &packet) override;
        stream_packet_obj _DequeueFromInside(unsigned int outPortId, unsigned int ms) override;

        virtual ss_linker_base *_CreateInputNegativeLinker(unsigned int inPortId) override;
        virtual stream_packer *_CreateInputStreamPacker(unsigned int inPortId, bool &bFast) override;

#if INTERFACE_SENSOR
        typedef struct stSensorDrvInfo_s
        {
            MI_SNR_PADInfo_t  stPadInfo;
            MI_SNR_PlaneInfo_t stSnrPlaneInfo;
        } stSensorDrvInfo_t;
        static void GetSensorInfo(stSensorDrvInfo_t &stSnrDrvInfo, unsigned int uintSnrId);
        static void UpdateSensorInfo(stSensorDrvInfo_t &stSnrDrvInfo, unsigned int uintSnrId);
#endif

    private:
        std::map<unsigned int, MiBaseReaderDesc> mapReaderDesc;
        static unsigned int intMiModuleRefCnt;
#if INTERFACE_SENSOR
        static std::map<unsigned int, stSensorDrvInfo_t> mapSnrDrvInfo;
#endif
};
#endif /* __AMIGOS_MODULE_MI_BASE_H__ */
