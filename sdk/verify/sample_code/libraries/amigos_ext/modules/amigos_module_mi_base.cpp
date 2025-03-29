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
#include <sys/time.h>
#include <cassert>
#include <cstring>
#include <memory>

#include "amigos_module_base.h"
#include "amigos_surface_base.h"
#include "mi_common_datatype.h"
#include "mi_sys_datatype.h"
#include "ss_packet.h"
#if INTERFACE_SENSOR
#include "mi_sensor.h"
#endif
#include "mi_sys.h"
#include "ss_linker.h"
#include "amigos_module_mi_base.h"
#include "ss_thread.h"

unsigned int AmigosModuleMiBase::intMiModuleRefCnt = 0;
#ifdef INTERFACE_SENSOR
std::map<unsigned int, AmigosModuleMiBase::stSensorDrvInfo_t> AmigosModuleMiBase::mapSnrDrvInfo;
#endif

void *MiSysReader(struct ss_thread_buffer *thread_buf);

static raw_video_fmt _ConvertPixelFormatToRawVideoFmt(MI_SYS_PixelFormat_e fmt)
{
    switch (fmt)
    {
        case E_MI_SYS_PIXEL_FRAME_YUV422_YUYV:
            return RAW_FORMAT_YUV422_YUYV;
        case E_MI_SYS_PIXEL_FRAME_YUV422_UYVY:
            return RAW_FORMAT_YUV422_UYVY;
        case E_MI_SYS_PIXEL_FRAME_YUV422_YVYU:
            return RAW_FORMAT_YUV422_YVYU;
        case E_MI_SYS_PIXEL_FRAME_YUV422_VYUY:
            return RAW_FORMAT_YUV422_VYUY;
        case E_MI_SYS_PIXEL_FRAME_YUV422_PLANAR:
            return RAW_FORMAT_YUV422SP;
        case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420:
            return RAW_FORMAT_YUV420SP;
        case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420_NV21:
            return RAW_FORMAT_YUV420SP_NV21;
        case E_MI_SYS_PIXEL_FRAME_RGB888:
            return RAW_FORMAT_RGB888;
        case E_MI_SYS_PIXEL_FRAME_BGR888:
            return RAW_FORMAT_BGR888;
        case E_MI_SYS_PIXEL_FRAME_ARGB8888:
            return RAW_FORMAT_ARGB8888;
        case E_MI_SYS_PIXEL_FRAME_ABGR8888:
            return RAW_FORMAT_ABGR8888;
        case E_MI_SYS_PIXEL_FRAME_BGRA8888:
            return RAW_FORMAT_BGRA8888;
        case E_MI_SYS_PIXEL_FRAME_RGB565:
            return RAW_FORMAT_RGB565;
        case E_MI_SYS_PIXEL_FRAME_ARGB1555:
            return RAW_FORMAT_ARGB1555;
        case E_MI_SYS_PIXEL_FRAME_ARGB4444:
            return RAW_FORMAT_ARGB4444;
        case E_MI_SYS_PIXEL_FRAME_I2:
            return RAW_FORMAT_I2;
        case E_MI_SYS_PIXEL_FRAME_I4:
            return RAW_FORMAT_I4;
        case E_MI_SYS_PIXEL_FRAME_I8:
            return RAW_FORMAT_I8;
        case E_MI_SYS_PIXEL_FRAME_RGB_BAYER_BASE...E_MI_SYS_PIXEL_FRAME_RGB_BAYER_NUM:
            return raw_video_fmt(RAW_FORMAT_BAYER_BASE + fmt - E_MI_SYS_PIXEL_FRAME_RGB_BAYER_BASE);
        default:
            break;
    }
    return RAW_FORMAT_MAX;
}
static MI_SYS_PixelFormat_e _ConvertRawVideoFmtToPixelFormat(raw_video_fmt fmt)
{
    switch (fmt)
    {
        case RAW_FORMAT_YUV422_YUYV:
            return E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
        case RAW_FORMAT_YUV422_UYVY:
            return E_MI_SYS_PIXEL_FRAME_YUV422_UYVY;
        case RAW_FORMAT_YUV422_YVYU:
            return E_MI_SYS_PIXEL_FRAME_YUV422_YVYU;
        case RAW_FORMAT_YUV422_VYUY:
            return E_MI_SYS_PIXEL_FRAME_YUV422_VYUY;
        case RAW_FORMAT_YUV422SP:
            return E_MI_SYS_PIXEL_FRAME_YUV422_PLANAR;
        case RAW_FORMAT_YUV420SP:
            return E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        case RAW_FORMAT_YUV420SP_NV21:
            return E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420_NV21;
        case RAW_FORMAT_RGB888:
            return E_MI_SYS_PIXEL_FRAME_RGB888;
        case RAW_FORMAT_BGR888:
            return E_MI_SYS_PIXEL_FRAME_BGR888;
        case RAW_FORMAT_ARGB8888:
            return E_MI_SYS_PIXEL_FRAME_ARGB8888;
        case RAW_FORMAT_ABGR8888:
            return E_MI_SYS_PIXEL_FRAME_ABGR8888;
        case RAW_FORMAT_BGRA8888:
            return E_MI_SYS_PIXEL_FRAME_BGRA8888;
        case RAW_FORMAT_RGB565:
            return E_MI_SYS_PIXEL_FRAME_RGB565;
        case RAW_FORMAT_ARGB1555:
            return E_MI_SYS_PIXEL_FRAME_ARGB1555;
        case RAW_FORMAT_ARGB4444:
            return E_MI_SYS_PIXEL_FRAME_ARGB4444;
        case RAW_FORMAT_I2:
            return E_MI_SYS_PIXEL_FRAME_I2;
        case RAW_FORMAT_I4:
            return E_MI_SYS_PIXEL_FRAME_I4;
        case RAW_FORMAT_I8:
            return E_MI_SYS_PIXEL_FRAME_I8;
        case RAW_FORMAT_BAYER_BASE...RAW_FORMAT_BAYER_NUM:
            return MI_SYS_PixelFormat_e(E_MI_SYS_PIXEL_FRAME_RGB_BAYER_BASE + fmt - RAW_FORMAT_BAYER_BASE);
        default:
            break;
    }
    return E_MI_SYS_PIXEL_FRAME_FORMAT_MAX;
}
class AmigosMiSysInPacket: public stream_packet_base
{
public:
    explicit AmigosMiSysInPacket(const struct raw_video_info &raw_i, MI_SYS_ChnPort_t &stChnPort);
    virtual ~AmigosMiSysInPacket() override;
private:
    virtual stream_packet_obj do_convert(stream_packet_obj &self, enum stream_type type) override;
    virtual void update_time_stamp() override;
    class PacketAbnormal: public err_buf
    {
        void show() const override;
    };
    MI_SYS_BufInfo_t stBufInfo;
};
class AmigosMiSysOutPacket: public stream_packet_base
{
public:
    explicit AmigosMiSysOutPacket(MI_SYS_ChnPort_t &stChnPort, unsigned int ms);
    virtual ~AmigosMiSysOutPacket() override;
private:
    virtual stream_packet_obj do_convert(stream_packet_obj &self, enum stream_type type) override;
    virtual void update_time_stamp() override;
    fd_set           read_fds;
    MI_SYS_BufInfo_t stBufInfo;
    class PacketAbnormal: public err_buf
    {
        void show() const override;
    };
};
class AmigosMiSysPacketPa2Va : public stream_packet_base
{
public:
    explicit AmigosMiSysPacketPa2Va(stream_packet_obj pa);
    ~AmigosMiSysPacketPa2Va();
private:
    stream_packet_obj do_convert(stream_packet_obj &self, enum stream_type type) override;
    void update_time_stamp() override;
    stream_packet_obj paObject;
    class PacketAbnormal : public err_buf
    {
        void show() const override;
    };
};
AmigosModuleMiBase::LinkerMiSysIn::LinkerMiSysIn(const MI_SYS_ChnPort_t &chnPort) : ss_linker_base()
{
    this->stChnPort = chnPort;
}
AmigosModuleMiBase::LinkerMiSysIn::~LinkerMiSysIn() {}
int AmigosModuleMiBase::LinkerMiSysIn::enqueue(stream_packet_obj &packet)
{
    stream_packet_obj vaObj = nullptr;
    if (packet->en_type != EN_RAW_FRAME_DATA && packet->en_type != EN_RAW_FRAME_DATA_PA)
    {
        AMIGOS_ERR("Packet type %d is not support.\n", packet->en_type);
        return -1;
    }
    if (packet->raw_vid_i.plane_num < 1 || packet->raw_vid_i.plane_num > RAW_FRAME_MULTI_PLANE_NUM_MAX)
    {
        AMIGOS_ERR("Packet plane %d out of range.\n", packet->raw_vid_i.plane_num);
        return -1;
    }
    auto pa = stream_packet_base::convert(packet, EN_RAW_FRAME_DATA_PA);
    if (pa && pa->raw_vid_pa.private_handle != NULL)
    {
        // If come packet has private_handle, use dup and inject.
        MI_SYS_BUF_HANDLE hBufHandle = 0;
        if (MI_SUCCESS != MI_SYS_DupBuf((MI_SYS_BUF_HANDLE)pa->raw_vid_pa.private_handle, &hBufHandle))
        {
            AMIGOS_ERR("MI_SYS_DupBuf failed\n");
            return -1;
        }
        if (MI_SUCCESS != MI_SYS_ChnPortInjectBuf(hBufHandle, &this->stChnPort))
        {
            AMIGOS_ERR("MI_SYS_ChnPortInjectBuf failed\n");
            return -1;
        }
        return 0;
    }
    MI_SYS_BufConf_t  stBufConf;
    MI_SYS_BufInfo_t  stBufInfo;
    MI_SYS_BUF_HANDLE hBufHandle = 0;

    memset(&stBufConf, 0, sizeof(MI_SYS_BufConf_t));
    memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));

    auto va = stream_packet_base::convert(packet, EN_RAW_FRAME_DATA);
    if (!va)
    {
        AMIGOS_ERR("Convert to va failed!\n");
        return -1;
    }
    if (va->raw_vid_i.plane_num == 1)
    {
        stBufConf.eBufType                  = E_MI_SYS_BUFDATA_FRAME;
        stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
        stBufConf.stFrameCfg.eCompressMode  = E_MI_SYS_COMPRESS_MODE_NONE;
        stBufConf.stFrameCfg.eFormat        = _ConvertRawVideoFmtToPixelFormat(va->raw_vid_i.plane_info[0].fmt);
        stBufConf.stFrameCfg.u16Width       = (MI_U16)va->raw_vid_i.plane_info[0].width;
        stBufConf.stFrameCfg.u16Height      = (MI_U16)va->raw_vid_i.plane_info[0].height;
    }
    else
    {
        stBufConf.eBufType = E_MI_SYS_BUFDATA_MULTIPLANE;
        stBufConf.stMultiPlaneCfg.u8SubPlaneNum = va->raw_vid_i.plane_num;
        for (unsigned int i = 0; i < va->raw_vid_i.plane_num; ++i)
        {
            stBufConf.stMultiPlaneCfg.stFrameCfgs[i].eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
            stBufConf.stMultiPlaneCfg.stFrameCfgs[i].eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
            stBufConf.stMultiPlaneCfg.stFrameCfgs[i].eFormat =
                _ConvertRawVideoFmtToPixelFormat(va->raw_vid_i.plane_info[i].fmt);
            stBufConf.stMultiPlaneCfg.stFrameCfgs[i].u16Width  = va->raw_vid_i.plane_info[i].width;
            stBufConf.stMultiPlaneCfg.stFrameCfgs[i].u16Height = va->raw_vid_i.plane_info[i].height;
        }
    }

    // Get Input Buf
    if (MI_SUCCESS != MI_SYS_ChnInputPortGetBuf(&stChnPort, &stBufConf, &stBufInfo, &hBufHandle, 0))
    {
        AMIGOS_ERR("MI_SYS_ChnInputPortGetBuf failed\n");
        return -1;
    }

    // Copy
    if (va->raw_vid_i.plane_num == 1)
    {
        for (unsigned int i = 0; i < 3; ++i)
        {
            if (va->raw_vid.plane_data[0].data[i] && va->raw_vid.plane_data[0].size[i])
            {
                memcpy(stBufInfo.stFrameData.pVirAddr[i], va->raw_vid.plane_data[0].data[i],
                       va->raw_vid.plane_data[0].size[i]);
            }
        }
    }
    else
    {
        for (unsigned int plane_idx = 0; plane_idx < va->raw_vid_i.plane_num; ++plane_idx)
        {
            for (unsigned int i = 0; i < 2; ++i)
            {
                if (va->raw_vid.plane_data[plane_idx].data[i] && va->raw_vid.plane_data[plane_idx].size[i])
                {
                    memcpy(stBufInfo.stFrameDataMultiPlane.stSubPlanes[plane_idx].pVirAddr[i],
                           va->raw_vid.plane_data[plane_idx].data[i], va->raw_vid.plane_data[plane_idx].size[i]);
                }
            }
        }
    }
    // Put Input Buf
    if (MI_SUCCESS != MI_SYS_ChnInputPortPutBuf(hBufHandle, &stBufInfo, FALSE))
    {
        AMIGOS_ERR("MI_SYS_ChnInputPortPutBuf failed\n");
        return -1;
    }
    return 0;
}
stream_packet_obj AmigosModuleMiBase::LinkerMiSysIn::dequeue(unsigned int ms)
{
    return nullptr;
}
AmigosMiSysInPacket::AmigosMiSysInPacket(const struct raw_video_info &raw_i, MI_SYS_ChnPort_t &chnPort) : stream_packet_base()
{
    MI_SYS_BufConf_t  stBufConf;
    MI_SYS_BUF_HANDLE hBufHandle = 0;
    struct timeval    timeStamp;

    memset(&this->stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));
    memset(&stBufConf, 0, sizeof(MI_SYS_BufConf_t));

    this->en_type = EN_RAW_FRAME_DATA_PA;
    gettimeofday(&timeStamp, NULL);
    stBufConf.u64TargetPts = timeStamp.tv_sec * 1000000 + timeStamp.tv_usec;
    if (raw_i.plane_num == 1)
    {
        stBufConf.eBufType                  = E_MI_SYS_BUFDATA_FRAME;
        stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
        stBufConf.stFrameCfg.eCompressMode  = E_MI_SYS_COMPRESS_MODE_NONE;
        stBufConf.stFrameCfg.eFormat        = _ConvertRawVideoFmtToPixelFormat(raw_i.plane_info[0].fmt);
        stBufConf.stFrameCfg.u16Width       = (MI_U16)raw_i.plane_info[0].width;
        stBufConf.stFrameCfg.u16Height      = (MI_U16)raw_i.plane_info[0].height;
    }
    else
    {
        stBufConf.eBufType = E_MI_SYS_BUFDATA_MULTIPLANE;
        stBufConf.stMultiPlaneCfg.u8SubPlaneNum = raw_i.plane_num;
        for (unsigned int i = 0; i < raw_i.plane_num && i < MI_SYS_MAX_SUB_PLANE_CNT; ++i)
        {
            stBufConf.stMultiPlaneCfg.stFrameCfgs[i].eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
            stBufConf.stMultiPlaneCfg.stFrameCfgs[i].eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
            stBufConf.stMultiPlaneCfg.stFrameCfgs[i].eFormat =
                _ConvertRawVideoFmtToPixelFormat(raw_i.plane_info[i].fmt);
            stBufConf.stMultiPlaneCfg.stFrameCfgs[i].u16Width  = raw_i.plane_info[i].width;
            stBufConf.stMultiPlaneCfg.stFrameCfgs[i].u16Height = raw_i.plane_info[i].height;
        }
    }
    // Get Input Buf
    if (MI_SUCCESS != MI_SYS_ChnInputPortGetBufPa(&chnPort, &stBufConf, &this->stBufInfo, &hBufHandle, 0))
    {
        AMIGOS_ERR("MI_SYS_ChnInputPortGetBuf failed\n");
        throw PacketAbnormal();
    }
    if (this->stBufInfo.eBufType == E_MI_SYS_BUFDATA_FRAME)
    {
        this->raw_vid_i.plane_num             = 1;
        this->raw_vid_i.plane_info[0].fmt     = _ConvertPixelFormatToRawVideoFmt(this->stBufInfo.stFrameData.ePixelFormat);
        this->raw_vid_i.plane_info[0].width   = this->stBufInfo.stFrameData.u16Width;
        this->raw_vid_i.plane_info[0].height  = this->stBufInfo.stFrameData.u16Height;
        this->raw_vid_pa.plane_data[0].stride[0] = this->stBufInfo.stFrameData.u32Stride[0];
        this->raw_vid_pa.plane_data[0].phy[0]    = this->stBufInfo.stFrameData.phyAddr[0];
        this->raw_vid_pa.plane_data[0].stride[1] = this->stBufInfo.stFrameData.u32Stride[1];
        this->raw_vid_pa.plane_data[0].phy[1]    = this->stBufInfo.stFrameData.phyAddr[1];
        this->raw_vid_pa.plane_data[0].stride[2] = this->stBufInfo.stFrameData.u32Stride[2];
        this->raw_vid_pa.plane_data[0].phy[2]    = this->stBufInfo.stFrameData.phyAddr[2];
        stream_packet_info::raw_data_size(this->raw_vid_i.plane_info[0], this->raw_vid_pa.plane_data[0].stride,
                                          this->raw_vid_pa.plane_data[0].size);
    }
    else if (this->stBufInfo.eBufType == E_MI_SYS_BUFDATA_MULTIPLANE)
    {
        this->raw_vid_i.plane_num = this->stBufInfo.stFrameDataMultiPlane.u8SubPlaneNum;
        for (unsigned int i = 0; i < this->stBufInfo.stFrameDataMultiPlane.u8SubPlaneNum && i < RAW_FRAME_MULTI_PLANE_NUM_MAX; ++i)
        {
            this->raw_vid_i.plane_info[i].fmt =
                _ConvertPixelFormatToRawVideoFmt(this->stBufInfo.stFrameDataMultiPlane.stSubPlanes[i].ePixelFormat);
            this->raw_vid_i.plane_info[i].width   = this->stBufInfo.stFrameDataMultiPlane.stSubPlanes[i].u16Width;
            this->raw_vid_i.plane_info[i].height  = this->stBufInfo.stFrameDataMultiPlane.stSubPlanes[i].u16Height;
            this->raw_vid_pa.plane_data[i].stride[0] = this->stBufInfo.stFrameDataMultiPlane.stSubPlanes[i].u16Stride[0];
            this->raw_vid_pa.plane_data[i].phy[0]    = this->stBufInfo.stFrameDataMultiPlane.stSubPlanes[i].phyAddr[0];
            this->raw_vid_pa.plane_data[i].stride[1] = this->stBufInfo.stFrameDataMultiPlane.stSubPlanes[i].u16Stride[1];
            this->raw_vid_pa.plane_data[i].phy[1]    = this->stBufInfo.stFrameDataMultiPlane.stSubPlanes[i].phyAddr[1];
            stream_packet_info::raw_data_size(this->raw_vid_i.plane_info[i], this->raw_vid_pa.plane_data[i].stride,
                                              this->raw_vid_pa.plane_data[i].size);
        }
    }
    this->raw_vid_pa.private_handle = hBufHandle;
    type_name = "sys_in";
}
void AmigosMiSysInPacket::update_time_stamp()
{
    struct timeval stamp;
    stamp.tv_sec  = this->stBufInfo.u64Pts / 1000000;
    stamp.tv_usec = this->stBufInfo.u64Pts % 1000000;
    this->set_time_stamp(stamp);
}
AmigosMiSysInPacket::~AmigosMiSysInPacket()
{
    if (MI_SUCCESS != MI_SYS_ChnInputPortPutBufPa((MI_SYS_BUF_HANDLE)this->raw_vid_pa.private_handle, &this->stBufInfo, TRUE))
    {
        AMIGOS_ERR("MI_SYS_ChnInputPortPutBuf failed\n");
    }
}
AmigosMiSysPacketPa2Va::AmigosMiSysPacketPa2Va(stream_packet_obj pa) : paObject(pa)
{
    if (pa->en_type != EN_RAW_FRAME_DATA_PA)
    {
        AMIGOS_ERR("Object is not RAW PA\n");
        throw PacketAbnormal();
    }
    this->en_type = EN_RAW_FRAME_DATA;
    this->raw_vid_i = pa->raw_vid_i;
    for (unsigned int i = 0; i < pa->raw_vid_i.plane_num; ++i)
    {
        int ret = MI_SYS_Mmap(pa->raw_vid_pa.plane_data[i].phy[0], pa->raw_vid_pa.plane_data[i].size[0] +
                                                                    pa->raw_vid_pa.plane_data[i].size[1] +
                                                                    pa->raw_vid_pa.plane_data[i].size[2],
                                (void **)&this->raw_vid.plane_data[i].data[0], TRUE);
        if (ret != 0)
        {
            AMIGOS_ERR("Mmap pa 0x%llx error! size 0x%x\n", pa->raw_vid_pa.plane_data[0].phy[0], pa->raw_vid_pa.plane_data[0].size[0]);
            goto ERR_MAP;
        }
        this->raw_vid.plane_data[i].stride[0] = pa->raw_vid_pa.plane_data[i].stride[0];
        this->raw_vid.plane_data[i].size[0]   = pa->raw_vid_pa.plane_data[i].size[0];
        this->raw_vid.plane_data[i].stride[1] = pa->raw_vid_pa.plane_data[i].stride[1];
        this->raw_vid.plane_data[i].data[1]   = pa->raw_vid_pa.plane_data[i].size[1] ?
                                                this->raw_vid.plane_data[i].data[0] + pa->raw_vid_pa.plane_data[i].size[0] : nullptr;
        this->raw_vid.plane_data[i].size[1]   = pa->raw_vid_pa.plane_data[i].size[1];
        this->raw_vid.plane_data[i].stride[2] = pa->raw_vid_pa.plane_data[i].stride[2];
        this->raw_vid.plane_data[i].data[2]   = pa->raw_vid_pa.plane_data[i].size[2] ?
                                                this->raw_vid.plane_data[i].data[1] + pa->raw_vid_pa.plane_data[i].size[1] : nullptr;
        this->raw_vid.plane_data[i].size[2]   = pa->raw_vid_pa.plane_data[i].size[2];
    }
    return;
ERR_MAP:
    for (unsigned int i = 0; i < pa->raw_vid_i.plane_num; ++i)
    {
        if (this->raw_vid.plane_data[i].data[0])
        {
            MI_SYS_Munmap(this->raw_vid.plane_data[i].data[0], this->raw_vid.plane_data[i].size[0] +
                                                               this->raw_vid.plane_data[i].size[1] +
                                                               this->raw_vid.plane_data[i].size[2]);
        }
    }
    throw PacketAbnormal();
}
stream_packet_obj AmigosMiSysPacketPa2Va::do_convert(stream_packet_obj &self, enum stream_type type)
{
    // Only support VA to PA.
    if (type != EN_RAW_FRAME_DATA_PA)
    {
        AMIGOS_ERR("Not support converting %d\n", type);
        return nullptr;
    }
    for (unsigned int i = 0; i < this->raw_vid_i.plane_num; ++i)
    {
        if (this->raw_vid.plane_data[i].data[0])
        {
            MI_SYS_FlushInvCache(this->raw_vid.plane_data[i].data[0], this->raw_vid.plane_data[i].size[0] +
                                                                      this->raw_vid.plane_data[i].size[1] +
                                                                      this->raw_vid.plane_data[i].size[2]);
        }
    }
    return paObject;
}
void AmigosMiSysPacketPa2Va::update_time_stamp()
{
    this->set_time_stamp(paObject->get_time_stamp());
}
AmigosMiSysPacketPa2Va::~AmigosMiSysPacketPa2Va()
{
    for (unsigned int i = 0; i < this->raw_vid_i.plane_num; ++i)
    {
        if (this->raw_vid.plane_data[i].data[0])
        {
            MI_SYS_Munmap(this->raw_vid.plane_data[i].data[0], this->raw_vid.plane_data[i].size[0] +
                                                               this->raw_vid.plane_data[i].size[1] +
                                                               this->raw_vid.plane_data[i].size[2]);
        }
    }
}
void AmigosMiSysPacketPa2Va::PacketAbnormal::show() const
{
}
stream_packet_obj AmigosMiSysInPacket::do_convert(stream_packet_obj &self, enum stream_type type)
{
    // Only support PA to VA.
    if (type != EN_RAW_FRAME_DATA)
    {
        AMIGOS_ERR("Not support converting %d\n", type);
        return nullptr;
    }
    return stream_packet_base::make<AmigosMiSysPacketPa2Va>(self);
}
void AmigosMiSysInPacket::PacketAbnormal::show() const
{
}

AmigosMiSysOutPacket::AmigosMiSysOutPacket(MI_SYS_ChnPort_t &stChnPort, unsigned int ms)
{
    struct timeval    tv;
    MI_S32            s32Fd = 0;
    MI_S32            s32Ret = 0;
    MI_SYS_BUF_HANDLE hHandle = 0;

    s32Ret = MI_SYS_GetFd(&stChnPort, &s32Fd);
    if (s32Ret != MI_SUCCESS)
    {
        AMIGOS_ERR("Get fd error! mod %d dev %d chn %d port %d\n", stChnPort.eModId, stChnPort.u32DevId, stChnPort.u32ChnId, stChnPort.u32PortId);
        throw PacketAbnormal();
    }
    FD_ZERO(&this->read_fds);
    FD_SET(s32Fd, &this->read_fds);
    tv.tv_sec  = 0;
    tv.tv_usec = ms * 1000;
    s32Ret = select(s32Fd + 1, &this->read_fds, NULL, NULL, &tv);
    if (s32Ret <= 0)
    {
        MI_SYS_CloseFd(s32Fd);
        throw PacketAbnormal();
    }
    memset(&this->stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));
    s32Ret = MI_SYS_ChnOutputPortGetBufPa(&stChnPort, &this->stBufInfo, &hHandle);
    if (MI_SUCCESS != s32Ret)
    {
        MI_SYS_CloseFd(s32Fd);
        throw PacketAbnormal();
    }
    MI_SYS_CloseFd(s32Fd);
    this->en_type = EN_RAW_FRAME_DATA_PA;
    if (this->stBufInfo.eBufType == E_MI_SYS_BUFDATA_FRAME)
    {
        this->raw_vid_i.plane_num             = 1;
        this->raw_vid_i.plane_info[0].fmt     = _ConvertPixelFormatToRawVideoFmt(this->stBufInfo.stFrameData.ePixelFormat);
        this->raw_vid_i.plane_info[0].width   = this->stBufInfo.stFrameData.u16Width;
        this->raw_vid_i.plane_info[0].height  = this->stBufInfo.stFrameData.u16Height;
        this->raw_vid_pa.plane_data[0].stride[0] = this->stBufInfo.stFrameData.u32Stride[0];
        this->raw_vid_pa.plane_data[0].phy[0]    = this->stBufInfo.stFrameData.phyAddr[0];
        this->raw_vid_pa.plane_data[0].stride[1] = this->stBufInfo.stFrameData.u32Stride[1];
        this->raw_vid_pa.plane_data[0].phy[1]    = this->stBufInfo.stFrameData.phyAddr[1];
        this->raw_vid_pa.plane_data[0].stride[2] = this->stBufInfo.stFrameData.u32Stride[2];
        this->raw_vid_pa.plane_data[0].phy[2]    = this->stBufInfo.stFrameData.phyAddr[2];
        stream_packet_info::raw_data_size(this->raw_vid_i.plane_info[0], this->raw_vid_pa.plane_data[0].stride,
                                          this->raw_vid_pa.plane_data[0].size);
    }
    else if (this->stBufInfo.eBufType == E_MI_SYS_BUFDATA_MULTIPLANE)
    {
        this->raw_vid_i.plane_num = this->stBufInfo.stFrameDataMultiPlane.u8SubPlaneNum;
        for (unsigned int i = 0; i < this->stBufInfo.stFrameDataMultiPlane.u8SubPlaneNum && i < RAW_FRAME_MULTI_PLANE_NUM_MAX; ++i)
        {
            this->raw_vid_i.plane_info[i].fmt =
                _ConvertPixelFormatToRawVideoFmt(this->stBufInfo.stFrameDataMultiPlane.stSubPlanes[i].ePixelFormat);
            this->raw_vid_i.plane_info[i].width   = this->stBufInfo.stFrameDataMultiPlane.stSubPlanes[i].u16Width;
            this->raw_vid_i.plane_info[i].height  = this->stBufInfo.stFrameDataMultiPlane.stSubPlanes[i].u16Height;
            this->raw_vid_pa.plane_data[i].stride[0] = this->stBufInfo.stFrameDataMultiPlane.stSubPlanes[i].u16Stride[0];
            this->raw_vid_pa.plane_data[i].phy[0]    = this->stBufInfo.stFrameDataMultiPlane.stSubPlanes[i].phyAddr[0];
            this->raw_vid_pa.plane_data[i].stride[1] = this->stBufInfo.stFrameDataMultiPlane.stSubPlanes[i].u16Stride[1];
            this->raw_vid_pa.plane_data[i].phy[1]    = this->stBufInfo.stFrameDataMultiPlane.stSubPlanes[i].phyAddr[1];
            stream_packet_info::raw_data_size(this->raw_vid_i.plane_info[i], this->raw_vid_pa.plane_data[i].stride,
                                              this->raw_vid_pa.plane_data[i].size);
        }
    }
    this->raw_vid_pa.private_handle = hHandle;
    type_name = "sys_out";
}
stream_packet_obj AmigosMiSysOutPacket::do_convert(stream_packet_obj &self, enum stream_type type)
{
    // Only support PA to VA.
    if (type != EN_RAW_FRAME_DATA)
    {
        AMIGOS_ERR("Not support converting %d\n", type);
        return nullptr;
    }
    return stream_packet_base::make<AmigosMiSysPacketPa2Va>(self);
}
void AmigosMiSysOutPacket::update_time_stamp()
{
    struct timeval stamp;
    stamp.tv_sec  = this->stBufInfo.u64Pts / 1000000;
    stamp.tv_usec = this->stBufInfo.u64Pts % 1000000;
    this->set_time_stamp(stamp);
}
AmigosMiSysOutPacket::~AmigosMiSysOutPacket()
{
    if (MI_SYS_ChnOutputPortPutBufPa((MI_SYS_BUF_HANDLE)this->raw_vid_pa.private_handle) != MI_SUCCESS)
    {
        AMIGOS_ERR("MI_SYS_ChnOutputPortPutBuf failed\n");
    }
}
void AmigosMiSysOutPacket::PacketAbnormal::show() const
{
}
AmigosModuleMiBase::StreamPacketSysMma::StreamPacketSysMma(const struct raw_video_info &raw_i)
{
    this->en_type = EN_RAW_FRAME_DATA_PA;
    this->raw_vid_i = raw_i;
    for (unsigned int i = 0; i < this->raw_vid_i.plane_num; ++i)
    {
        stream_packet_info::raw_data_stride(raw_i.plane_info[i], this->raw_vid_pa.plane_data[i].stride);
        stream_packet_info::raw_data_size(raw_i.plane_info[i], this->raw_vid_pa.plane_data[i].stride,
                                          this->raw_vid_pa.plane_data[i].size);
        if (MI_SUCCESS != MI_SYS_MMA_Alloc(0, NULL, this->raw_vid_pa.plane_data[i].size[0]
                                           + this->raw_vid_pa.plane_data[i].size[1]
                                           + this->raw_vid_pa.plane_data[i].size[2],
                                           &this->raw_vid_pa.plane_data[i].phy[0]))
        {
            AMIGOS_ERR("Alloc buffer error! SIZE[0] 0x%x, SIZE[1] 0x%x, SIZE 0x%x\n", this->raw_vid_pa.plane_data[i].size[0],
                       this->raw_vid_pa.plane_data[i].size[1], this->raw_vid_pa.plane_data[i].size[2]);
            throw PacketErrBuf();
            return;
        }
        this->raw_vid_pa.plane_data[i].phy[1] = this->raw_vid_pa.plane_data[i].size[1] > 0 ?
            this->raw_vid_pa.plane_data[i].phy[0] + this->raw_vid_pa.plane_data[i].size[0] : 0;
        this->raw_vid_pa.plane_data[i].phy[2] = this->raw_vid_pa.plane_data[i].size[2] > 0 ?
            this->raw_vid_pa.plane_data[i].phy[1] + this->raw_vid_pa.plane_data[i].size[1] : 0;
    }
    this->type_name = "sys_mma";
}
stream_packet_obj AmigosModuleMiBase::StreamPacketSysMma::do_convert(stream_packet_obj &self, enum stream_type type)
{
    // Only support PA to VA.
    if (type != EN_RAW_FRAME_DATA)
    {
        AMIGOS_ERR("Not support converting %d\n", type);
        return nullptr;
    }
    return stream_packet_base::make<AmigosMiSysPacketPa2Va>(self);
}
AmigosModuleMiBase::StreamPacketSysMma::~StreamPacketSysMma()
{
    for (unsigned int i = 0; i < this->raw_vid_i.plane_num; i++)
    {
        if (this->raw_vid_pa.plane_data[i].phy[0])
        {
            MI_SYS_MMA_Free(0, this->raw_vid_pa.plane_data[i].phy[0]);
        }
    }
}
void AmigosModuleMiBase::StreamPacketSysMma::PacketErrBuf::show() const
{
    AMIGOS_ERR("misys alloc buffer error\n");
}
stream_packet_obj AmigosModuleMiBase::StreamPackerSysMma::make(const stream_packet_info &packet_info)
{
    if (packet_info.en_type != EN_RAW_FRAME_DATA && packet_info.en_type != EN_RAW_FRAME_DATA_PA)
    {
        AMIGOS_ERR("Packet type %d is not support.\n", packet_info.en_type);
        return nullptr;
    }
    if (packet_info.raw_vid_i.plane_num < 1 || packet_info.raw_vid_i.plane_num > RAW_FRAME_MULTI_PLANE_NUM_MAX)
    {
        AMIGOS_ERR("Packet plane %d out of range.\n", packet_info.raw_vid_i.plane_num);
        return nullptr;
    }
    stream_packet_obj packet = stream_packet_base::make<StreamPacketSysMma>(packet_info.raw_vid_i);
    return stream_packet_base::convert(packet, packet_info.en_type);
}
AmigosModuleMiBase::StreamPackerMiSysIn::StreamPackerMiSysIn(const MI_SYS_ChnPort_t &chnPort)
    : stChnPort(chnPort)
{
}
AmigosModuleMiBase::StreamPackerMiSysIn::~StreamPackerMiSysIn()
{
}
stream_packet_obj AmigosModuleMiBase::StreamPackerMiSysIn::make(const stream_packet_info &packet_info)
{
    if (packet_info.en_type != EN_RAW_FRAME_DATA && packet_info.en_type != EN_RAW_FRAME_DATA_PA)
    {
        AMIGOS_ERR("Packet type %d is not support.\n", packet_info.en_type);
        return nullptr;
    }
    if (packet_info.raw_vid_i.plane_num < 1 || packet_info.raw_vid_i.plane_num > RAW_FRAME_MULTI_PLANE_NUM_MAX)
    {
        AMIGOS_ERR("Packet plane %d out of range.\n", packet_info.raw_vid_i.plane_num);
        return nullptr;
    }
    stream_packet_obj packet = stream_packet_base::make<AmigosMiSysInPacket>(packet_info.raw_vid_i, this->stChnPort);
    return stream_packet_base::convert(packet, packet_info.en_type);
}
AmigosModuleMiBase::AmigosModuleMiBase(AmigosSurfaceBase *pSurface)
    : AmigosModuleBase(pSurface)
{
    if (AmigosModuleMiBase::intMiModuleRefCnt == 0)
    {
        MI_SYS_Init(0);
        AMIGOS_INFO("MI_SYS_Init\n");
    }
    AmigosModuleMiBase::intMiModuleRefCnt++;
}
AmigosModuleMiBase::~AmigosModuleMiBase()
{
    AmigosModuleMiBase::intMiModuleRefCnt--;
    if (AmigosModuleMiBase::intMiModuleRefCnt == 0)
    {
        MI_SYS_Exit(0);
#if INTERFACE_SENSOR
        if ( mapSnrDrvInfo.size())
        {
            AMIGOS_INFO("~AmigosModuleMiBase, snr size: %d\n", mapSnrDrvInfo.size());
            mapSnrDrvInfo.clear();
        }
#endif
        AMIGOS_INFO("MI_SYS_Exit\n");
    }
}
void AmigosModuleMiBase::_DirectBind(unsigned int inPortId, unsigned int prevOutPortId, const AmigosModuleBase *pPrev)
{
    MI_SYS_ChnPort_t  stSrcChnPort;
    MI_SYS_ChnPort_t  stDstChnPort;

    AmigosSurfaceBase::ModPortInInfo stInPortInfo;
    if (!this->GetSurface()->GetPortInInfo(inPortId, stInPortInfo))
    {
        AMIGOS_ERR("Get curr mod port in info error\n");
        return;
    }

    AmigosSurfaceBase::ModPortOutInfo stPrevOutPortInfo;
    if (!pPrev->GetSurface()->GetPortOutInfo(prevOutPortId, stPrevOutPortInfo))
    {
        AMIGOS_ERR("Get prev mod port out info error\n");
        return;
    }

    memset(&stSrcChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0, sizeof(MI_SYS_ChnPort_t));

    const AmigosSurfaceBase::ModInfo &stSrcModInfo = pPrev->GetSurface()->GetModInfo();
    stSrcChnPort.eModId    = (MI_ModuleId_e)pPrev->GetModId();
    stSrcChnPort.u32DevId  = stSrcModInfo.devId;
    stSrcChnPort.u32ChnId  = stSrcModInfo.chnId;
    stSrcChnPort.u32PortId = prevOutPortId;

    const AmigosSurfaceBase::ModInfo &stDstModInfo = this->GetSurface()->GetModInfo();
    stDstChnPort.eModId    = (MI_ModuleId_e)this->GetModId();
    stDstChnPort.u32DevId  = stDstModInfo.devId;
    stDstChnPort.u32ChnId  = stDstModInfo.chnId;
    stDstChnPort.u32PortId = inPortId;

    // Bind
    if (MI_SUCCESS
        != MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, stPrevOutPortInfo.curFrmRate, stInPortInfo.curFrmRate,
                               (MI_SYS_BindType_e)stInPortInfo.bindType, stInPortInfo.bindPara))
    {
        AMIGOS_ERR("MI_SYS_BindChnPort2 failed\n");
    }
    AMIGOS_INFO("Bind %d-%d-%d-%d -> %d-%d-%d-%d\n", stSrcChnPort.eModId, stSrcChnPort.u32DevId, stSrcChnPort.u32ChnId,
                stSrcChnPort.u32PortId, stDstChnPort.eModId, stDstChnPort.u32DevId, stDstChnPort.u32ChnId,
                stDstChnPort.u32PortId);
}
void AmigosModuleMiBase::_DirectUnBind(unsigned int inPortId, unsigned int prevOutPortId, const AmigosModuleBase *pPrev)
{
    // UnBind
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;

    memset(&stSrcChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0, sizeof(MI_SYS_ChnPort_t));

    const AmigosSurfaceBase::ModInfo &stSrcModInfo = pPrev->GetSurface()->GetModInfo();
    stSrcChnPort.eModId    = (MI_ModuleId_e)pPrev->GetModId();
    stSrcChnPort.u32DevId  = stSrcModInfo.devId;
    stSrcChnPort.u32ChnId  = stSrcModInfo.chnId;
    stSrcChnPort.u32PortId = prevOutPortId;

    const AmigosSurfaceBase::ModInfo &stDstModInfo = this->GetSurface()->GetModInfo();
    stDstChnPort.eModId    = (MI_ModuleId_e)this->GetModId();
    stDstChnPort.u32DevId  = stDstModInfo.devId;
    stDstChnPort.u32ChnId  = stDstModInfo.chnId;
    stDstChnPort.u32PortId = inPortId;

    if (MI_SUCCESS != MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort))
    {
        AMIGOS_ERR("MI_SYS_UnBindChnPort failed\n");
    }
    AMIGOS_INFO("UnBind %d-%d-%d-%d -> %d-%d-%d-%d\n", stSrcChnPort.eModId, stSrcChnPort.u32DevId,
                stSrcChnPort.u32ChnId, stSrcChnPort.u32PortId, stDstChnPort.eModId, stDstChnPort.u32DevId,
                stDstChnPort.u32ChnId, stDstChnPort.u32PortId);
}

int AmigosModuleMiBase::_DataReceiver(unsigned int inPortId, stream_packet_obj &packet)
{
    return 0; // Nothing to do
}

int AmigosModuleMiBase::_Connected(unsigned int outPortId, unsigned int ref)
{
    if (ref > 0)
    {
        // No reader to call positive linker.
        return 0;
    }
    auto iter = this->mapPortOut.find(outPortId);
    if (iter == this->mapPortOut.end())
    {
        return -1;
    }

    MI_SYS_ChnPort_t stChnPort;
    AmigosSurfaceBase::ModPortOutInfo surfaceOutInfo;
    const AmigosSurfaceBase::ModInfo &stModInfo = this->GetSurface()->GetModInfo();

    memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId    = (MI_ModuleId_e)this->GetModId();
    stChnPort.u32DevId  = stModInfo.devId;
    stChnPort.u32ChnId  = stModInfo.chnId;
    stChnPort.u32PortId = outPortId;
    if (!this->GetSurface()->GetPortOutInfo(outPortId, surfaceOutInfo))
    {
        AMIGOS_ERR("GetPortOutInfo failed\n");
        return -1;
    }
    // It go user bind.
    if (surfaceOutInfo.bUserFrc)
    {
        MI_SYS_SetChnOutputPortUserFrc(&stChnPort, -1, surfaceOutInfo.curFrmRate);
    }
    if (!surfaceOutInfo.bUseDepth)
    {
        // Set default user depth if output is not direct bind.
        if (MI_SUCCESS != MI_SYS_SetChnOutputPortDepth(0, &stChnPort, 3, 4))
        {
            AMIGOS_ERR("MI_SYS_SetChnOutputPortDepth failed\n");
        }
    }
    if (iter->second.positive.empty())
    {
        return 0;
    }
    MiBaseReaderDesc desc;
    desc.outPlinker = &iter->second.positive;
    desc.stChnPort  = stChnPort;

    // Open reader thread
    ss_thread_attr ss_attr;
    memset(&ss_attr, 0, sizeof(ss_thread_attr));
    ss_attr.do_signal          = NULL;
    ss_attr.do_monitor         = MiSysReader;
    ss_attr.monitor_cycle_sec  = 0;
    ss_attr.monitor_cycle_nsec = 0;
    ss_attr.is_reset_timer     = 0;
    ss_attr.in_buf.buf         = (void *)&(this->mapReaderDesc[outPortId]);
    ss_attr.in_buf.size        = 0;
    snprintf(ss_attr.thread_name, 128, "%s", GetOutPortIdStr(outPortId).c_str());
    desc.threadHandle = ss_thread_open(&ss_attr);
    if (!desc.threadHandle)
    {
        this->mapReaderDesc.erase(outPortId);
        AMIGOS_ERR("Monitor return error!\n");
        return -1;
    }
    this->mapReaderDesc[outPortId] = desc;
    ss_thread_start_monitor(desc.threadHandle);
    AMIGOS_INFO("SYS Reader %d started!\n", outPortId);
    return 0;
}

int AmigosModuleMiBase::_Disconnected(unsigned int outPortId, unsigned int ref)
{
    if (ref > 0)
    {
        // No reader to call positive linker.
        return 0;
    }
    auto iter = this->mapPortOut.find(outPortId);
    if (iter->second.positive.empty())
    {
        /* Direct bind case + Post reader case. */
        AMIGOS_INFO("SYS Reader is not needed\n");
        return 0;
    }
    // Stop reader thread
    auto iterDesc = this->mapReaderDesc.find(outPortId);
    if (iterDesc == this->mapReaderDesc.end())
    {
        AMIGOS_ERR("Can not find desc in port %d!\n", outPortId);
        return -1;
    }
    if (!iterDesc->second.threadHandle)
    {
        AMIGOS_ERR("Reader thread error, port %d!\n", outPortId);
        return -1;
    }
    ss_thread_stop(iterDesc->second.threadHandle);
    ss_thread_close(iterDesc->second.threadHandle);
    this->mapReaderDesc.erase(iterDesc);
    AMIGOS_INFO("SYS Reader %d stopped!\n", outPortId);
    return 0;
}
void AmigosModuleMiBase::_StartMiIn(unsigned int inPortId)
{
}
void AmigosModuleMiBase::_StopMiIn(unsigned int inPortId)
{
}
void AmigosModuleMiBase::_StartMiOut(unsigned int inPortId)
{
}
void AmigosModuleMiBase::_StopMiOut(unsigned int inPortId)
{
}
void AmigosModuleMiBase::_StartIn(unsigned int inPortId)
{
    auto iter = this->mapPortIn.find(inPortId);
    if (iter == this->mapPortIn.end())
    {
        AMIGOS_ERR("Port %d has not base sturcture.\n", inPortId);
        return;
    }
    this->_StartMiIn(inPortId);
}

void AmigosModuleMiBase::_StopIn(unsigned int inPortId)
{
    auto iter = this->mapPortIn.find(inPortId);
    if (iter == this->mapPortIn.end())
    {
        AMIGOS_ERR("Port %d has not base sturcture.\n", inPortId);
        return;
    }
    this->_StopMiIn(inPortId);
}

void AmigosModuleMiBase::_StartOut(unsigned int outPortId)
{
    auto iter = this->mapPortOut.find(outPortId);
    if (iter == this->mapPortOut.end())
    {
        AMIGOS_ERR("Port %d has not base sturcture.\n", outPortId);
        return;
    }
    this->_StartMiOut(outPortId);
    AmigosSurfaceBase::ModPortOutInfo surfaceOutInfo;
    if (!this->GetSurface()->GetPortOutInfo(outPortId, surfaceOutInfo))
    {
        AMIGOS_ERR("GetPortOutInfo failed\n");
        return;
    }
    MI_SYS_ChnPort_t stChnPort;
    const AmigosSurfaceBase::ModInfo &stModInfo = this->GetSurface()->GetModInfo();
    memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId    = (MI_ModuleId_e)this->GetModId();
    stChnPort.u32DevId  = stModInfo.devId;
    stChnPort.u32ChnId  = stModInfo.chnId;
    stChnPort.u32PortId = outPortId;
    if (surfaceOutInfo.bUseDepth)
    {
        if (MI_SUCCESS != MI_SYS_SetChnOutputPortDepth(0, &stChnPort, surfaceOutInfo.bindDepthUser,
                                                       surfaceOutInfo.bindDepthTotal))
        {
            AMIGOS_ERR("MI_SYS_SetChnOutputPortDepth failed\n");
        }
    }
    if (surfaceOutInfo.bUseExtConf)
    {
        MI_SYS_FrameBufExtraConfig_t stBufExtCfg;
        memset(&stBufExtCfg, 0, sizeof(MI_SYS_FrameBufExtraConfig_t));
        stBufExtCfg.u16BufHAlignment        = surfaceOutInfo.bufHAlign;
        stBufExtCfg.u16BufVAlignment        = surfaceOutInfo.bufVAlign;
        stBufExtCfg.u16BufChromaAlignment   = surfaceOutInfo.bufChromaAlign;
        stBufExtCfg.u16BufCompressAlignment = surfaceOutInfo.bufCompressAlign;
        stBufExtCfg.u16BufExtraSize         = surfaceOutInfo.bufExtraSize;
        stBufExtCfg.bClearPadding           = surfaceOutInfo.bufClearPadding;
        if (MI_SUCCESS != MI_SYS_SetChnOutputPortBufExtConf(&stChnPort, &stBufExtCfg))
        {
            AMIGOS_ERR("MI_SYS_SetChnOutputPortBufExtConf failed\n");
        }
    }
}

void AmigosModuleMiBase::_StopOut(unsigned int outPortId)
{
    auto iter = this->mapPortOut.find(outPortId);
    if (iter == this->mapPortOut.end())
    {
        AMIGOS_ERR("Port %d has not base sturcture.\n", outPortId);
        return;
    }
    this->_StopMiOut(outPortId);
}

int AmigosModuleMiBase::_EnqueueOut(unsigned int outPortId, stream_packet_obj &packet)
{
    // To Do : Sys output enqueue
    // ......
    return 0;
}
int AmigosModuleMiBase::_DequeueOut(unsigned int outPortId, stream_packet_obj &obj)
{
    // To Do : Sys output dequeue
    // ......
    return 0;
}
stream_packet_obj AmigosModuleMiBase::_DequeueFromInside(unsigned int outPortId, unsigned int ms)
{
    MI_SYS_ChnPort_t stChnPort;
    const AmigosSurfaceBase::ModInfo &stMod = this->GetSurface()->GetModInfo();

    memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId    = (MI_ModuleId_e)this->GetModId();
    stChnPort.u32DevId  = stMod.devId;
    stChnPort.u32ChnId  = stMod.chnId;
    stChnPort.u32PortId = outPortId;
    return stream_packet_base::make<AmigosMiSysOutPacket>(stChnPort, ms);
}
ss_linker_base *AmigosModuleMiBase::_CreateInputNegativeLinker(unsigned int inPortId)
{
    MI_SYS_ChnPort_t stChnPort;
    const AmigosSurfaceBase::ModInfo &stMod = this->GetSurface()->GetModInfo();

    memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId    = (MI_ModuleId_e)this->GetModId();
    stChnPort.u32DevId  = stMod.devId;
    stChnPort.u32ChnId  = stMod.chnId;
    stChnPort.u32PortId = inPortId;
    return new LinkerMiSysIn(stChnPort);
}

stream_packer *AmigosModuleMiBase::_CreateInputStreamPacker(unsigned int inPortId, bool &bFast)
{
    MI_SYS_ChnPort_t stChnPort;
    const AmigosSurfaceBase::ModInfo &stMod = this->GetSurface()->GetModInfo();
    memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));

    stChnPort.eModId    = (MI_ModuleId_e)this->GetModId();
    stChnPort.u32DevId  = stMod.devId;
    stChnPort.u32ChnId  = stMod.chnId;
    stChnPort.u32PortId = inPortId;
    bFast = true;
    return new StreamPackerMiSysIn(stChnPort);
}

void *MiSysReader(struct ss_thread_buffer *thread_buf)
{
    AmigosModuleMiBase::MiBaseReaderDesc *pDesc = (AmigosModuleMiBase::MiBaseReaderDesc *)thread_buf->buf;

    assert(pDesc);
    stream_packet_obj packet = stream_packet_base::make<AmigosMiSysOutPacket>(pDesc->stChnPort, 100);
    if (!packet)
    {
        return NULL;
    }
    pDesc->outPlinker->enqueue(packet);
    return NULL;
}

#if INTERFACE_SENSOR
void AmigosModuleMiBase::GetSensorInfo(stSensorDrvInfo_t &stSnrDrvInfo, unsigned int uintSnrId)
{
    std::map<unsigned int, stSensorDrvInfo_t>::iterator itMapSnr;

    itMapSnr = mapSnrDrvInfo.find(uintSnrId);
    if (itMapSnr == mapSnrDrvInfo.end())
    {
        AMIGOS_ERR("Find snr id %d error !\n", uintSnrId);

        return;
    }
    stSnrDrvInfo = itMapSnr->second;
}
void AmigosModuleMiBase::UpdateSensorInfo(stSensorDrvInfo_t &stSnrDrvInfo, unsigned int uintSnrId)
{
    mapSnrDrvInfo[uintSnrId] = stSnrDrvInfo;
}
#endif

