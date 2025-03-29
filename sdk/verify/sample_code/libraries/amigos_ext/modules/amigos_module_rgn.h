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

#ifndef __AMIGOS_MODULE_RGN_H__
#define __AMIGOS_MODULE_RGN_H__

#include <pthread.h>
#include <list>
#include <map>
#include <string>
#include "amigos_module_rgn_metadata_define.h"
#include "mi_common_datatype.h"
#include "ss_linker.h"

#include "mi_rgn_datatype.h"
#include "amigos_surface_rgn.h"
#include "amigos_module_mi_base.h"
#include "ss_font.h"
#include "ss_packet.h"

class AmigosModuleRgn : public AmigosSurfaceRgn, public AmigosModuleMiBase
{
    friend void *ColorInvertProcess(struct ss_thread_buffer *thread_buf);
public:
    enum ThicknessLevel
    {
        E_THICKNESS_LEVEL_THIN = 0,
        E_THICKNESS_LEVEL_NORMAL,
        E_THICKNESS_LEVEL_THICK,
    };

    enum SizeLevel
    {
        E_SIZE_LEVEL_TINY = 0,
        E_SIZE_LEVEL_SMALL,
        E_SIZE_LEVEL_NORMAL,
        E_SIZE_LEVEL_LARGE,
        E_SIZE_LEVEL_HUGE,
    };

private:
    struct RgnTargetTiming
    {
        unsigned int width;
        unsigned int height;

        bool operator==(const RgnTargetTiming &other) const
        {
            return this->width == other.width && this->height == other.height;
        }
        bool operator<(const RgnTargetTiming &other) const
        {
            return this->width * this->height < other.width * other.height;
        }
    };

    struct RgnTargetInfo
    {
        MI_RGN_ChnPort_t stChnPort;
        RgnTargetTiming  timing;

        bool operator==(const RgnTargetInfo &other) const
        {
            return 0 == memcmp(&this->stChnPort, &other.stChnPort, sizeof(MI_RGN_ChnPort_t));
        }
        bool operator<(const RgnTargetInfo &other) const
        {
            return this->stChnPort.eModId != other.stChnPort.eModId ? this->stChnPort.eModId < other.stChnPort.eModId
                   : this->stChnPort.s32DevId != other.stChnPort.s32DevId
                       ? this->stChnPort.s32DevId < other.stChnPort.s32DevId
                   : this->stChnPort.s32ChnId != other.stChnPort.s32ChnId
                       ? this->stChnPort.s32ChnId < other.stChnPort.s32ChnId
                   : this->stChnPort.s32PortId != other.stChnPort.s32PortId
                       ? this->stChnPort.s32PortId < other.stChnPort.s32PortId
                   : this->stChnPort.bInputPort != other.stChnPort.bInputPort
                       ? this->stChnPort.bInputPort < other.stChnPort.bInputPort
                       : false;
        }
    };

    class RgnAdapter
    {
    public:
        explicit RgnAdapter(unsigned int handleGroupId);
        ~RgnAdapter();
        inline bool Create(MI_RGN_Attr_t &attr);
        inline bool Destroy();
        inline bool SetBitmap(MI_RGN_Bitmap_t &bitmap);
        inline bool GetCanvas(MI_RGN_CanvasInfo_t &canvas);
        inline bool UpdateCanvas();
        inline bool Attach(const RgnTargetInfo &target, MI_RGN_ChnPortParam_t &param);
        inline bool Detach(const RgnTargetInfo &target);

    private:
        MI_RGN_HANDLE                                  handle;
        unsigned int                                   handleGroupId;
        std::map<RgnTargetInfo, bool>                  map_attach;
        static std::vector<std::vector<MI_RGN_HANDLE>> handle_pool_table;
        static pthread_mutex_t                         handle_pool_mutex;
        static pthread_mutex_t                         canvas_mutex;
    };
    class RgnHandleRes
    {
    public:
        explicit RgnHandleRes(RgnAdapter *rgn_adapter);
        ~RgnHandleRes();
        bool CreateResource(const struct raw_video_info &info);
    protected:
        RgnAdapter *rgn_adapter;
    private:
        bool is_create;
    };
    class StreamPackerRgnIn: public stream_packer, public RgnHandleRes
    {
    public:
        explicit StreamPackerRgnIn(RgnAdapter *rgn_adapter);
        ~StreamPackerRgnIn() override;
        stream_packet_obj make(const stream_packet_info &packet_info) override;
    private:
    };
    class LinkerRgnIn: public ss_linker_base, public RgnHandleRes
    {
    public:
        explicit LinkerRgnIn(RgnAdapter *adapter);
        ~LinkerRgnIn() override;
        int enqueue(stream_packet_obj &packet) override;
        stream_packet_obj dequeue(unsigned int ms) override;
    };
    class AmigosMiRgnStreamPacket : public stream_packet_base
    {
    public:
        AmigosMiRgnStreamPacket(const stream_packet_info &packet_info, const MI_RGN_CanvasInfo_t &canvas);
        ~AmigosMiRgnStreamPacket() override;
    };

    class RgnControl
    {
    public:
        explicit RgnControl(unsigned int devId);
        virtual ~RgnControl();
        inline bool add_target(const RgnTargetInfo &target);
        inline bool del_target(const RgnTargetInfo &target);
        void process(const std::list<RgnTargetInfo> &target_lst, stream_packet_obj &packet);
        bool set_attr(const std::list<RgnTargetInfo> &target_lst, const AmigosSurfaceRgn::RgnInputInfo &info);

    protected:
        virtual void _add_target(const RgnTargetInfo &target)        = 0;
        virtual void _del_target(const RgnTargetInfo &target)        = 0;
        virtual void _set_param(const RgnTargetInfo &target)         = 0;
        virtual bool _check_packet(stream_packet_obj  &packet) = 0;
        virtual int  _set_attr(const AmigosSurfaceRgn::RgnInputInfo &)
        {
            return -1;
        }
        virtual void _finish() {}
        unsigned int devId;

    private:
        pthread_mutex_t control_mutex;
        bool is_processed;
    };
    class CanvasControl : public RgnControl
    {
    public:
        explicit CanvasControl(unsigned int devId, const AmigosSurfaceRgn::CanvasInfo &canvas_info, RgnAdapter *rgn_adapter);
        ~CanvasControl() override;

    protected:
        void _add_target(const RgnTargetInfo &target) override;
        void _del_target(const RgnTargetInfo &target) override;
        void _set_param(const RgnTargetInfo &target) override;
        bool _check_packet(stream_packet_obj &packet) override;

    private:
        MI_BOOL                  osd_show;
        MI_U32                   osd_layer;
        MI_RGN_OsdChnPortParam_t osd_param;
        RgnAdapter              *rgn_adapter;
    };
    class LineControl : public RgnControl
    {
    private:
        struct LineDesc
        {
            RgnAdapter  *rgn_adapter;
            unsigned int use_count;
            bool is_refresh;
        };

    public:
        explicit LineControl(unsigned int devId, const AmigosModuleRgn::LineInfo &line_info);
        ~LineControl() override;

    protected:
        void _add_target(const RgnTargetInfo &target) override;
        void _del_target(const RgnTargetInfo &target) override;
        void _set_param(const RgnTargetInfo &target) override;
        bool _check_packet(stream_packet_obj &packet) override;
        void _finish() override;
        int _set_attr(const AmigosSurfaceRgn::RgnInputInfo &info) override;
        void _refresh_lines(RgnAdapter *rgn_adapter, bool is_init);

    private:
        MI_RGN_PixelFormat_e pixel_fmt;
        unsigned int         color_off;
        unsigned int         color_on;
        ThicknessLevel       thickness_level;
        ThicknessLevel       thickness_level_last;

        std::map<RgnTargetTiming, LineDesc> map_timing_desc;
        stream_packet_obj                   last, curr;
    };
    class TextControl : public RgnControl
    {
    private:
        static const unsigned int TEXT_MAX_NUM = 64;
        struct TextDesc
        {
            RgnAdapter  *rgn_adapters[TEXT_MAX_NUM];
            unsigned int use_count;
            bool         is_refresh;
        };
    public:
        explicit TextControl(unsigned int devId, const AmigosSurfaceRgn::TextInfo &text_info);
        ~TextControl() override;

    protected:
        void _add_target(const RgnTargetInfo &target) override;
        void _del_target(const RgnTargetInfo &target) override;
        void _set_param(const RgnTargetInfo &target) override;
        bool _check_packet(stream_packet_obj &packet) override;
        void _finish() override;
        int _set_attr(const AmigosSurfaceRgn::RgnInputInfo &info) override;
        void _refresh_text(RgnAdapter *rgn_adapters[], unsigned int size, bool is_init);
        void _move_text(RgnAdapter *rgn_adapters[], const RgnTargetInfo &target, unsigned int size);

    private:
        MI_RGN_PixelFormat_e pixel_fmt;
        unsigned int         color;
        SizeLevel            size_level;
        unsigned int         area_w;
        unsigned int         area_h;
        unsigned int         default_x;
        unsigned int         default_y;

        std::map<unsigned int, TextDesc> map_size_desc;

        stream_packet_obj last, curr;
        SS_Font          *font;
    };
    class OsdFrameControl : public RgnControl
    {
    private:
        struct FrameDesc
        {
            RgnAdapter  *rgn_adapter;
            unsigned int use_count;
            bool is_refresh;
        };

    public:
        explicit OsdFrameControl(unsigned int devId, const AmigosModuleRgn::LineInfo &line_info);
        ~OsdFrameControl() override;

    protected:
        void _add_target(const RgnTargetInfo &target) override;
        void _del_target(const RgnTargetInfo &target) override;
        void _set_param(const RgnTargetInfo &target) override;
        bool _check_packet(stream_packet_obj &packet) override;
        void _finish() override;
        int _set_attr(const AmigosSurfaceRgn::RgnInputInfo &info) override;
        void _refresh_frames(RgnAdapter *rgn_adapter, bool is_init);

    private:
        MI_RGN_PixelFormat_e pixel_fmt;
        unsigned int         color_off;
        unsigned int         color_on;
        ThicknessLevel       thickness_level;
        ThicknessLevel       thickness_level_last;

        std::map<RgnTargetTiming, FrameDesc> map_timing_desc;
        stream_packet_obj                    last, curr;
    };
    class OsdDotMatrixControl : public RgnControl
    {
    private:
        struct DotMatrixDesc
        {
            RgnAdapter  *rgn_adapter;
            unsigned int use_count;
            bool is_refresh;
        };

    public:
        explicit OsdDotMatrixControl(unsigned int devId, const AmigosModuleRgn::DotMatrixInfo &dot_matrix_info);
        ~OsdDotMatrixControl() override;

    protected:
        void _add_target(const RgnTargetInfo &target) override;
        void _del_target(const RgnTargetInfo &target) override;
        void _set_param(const RgnTargetInfo &target) override;
        bool _check_packet(stream_packet_obj &packet) override;
        void _finish() override;
        int _set_attr(const AmigosSurfaceRgn::RgnInputInfo &info) override;
        void _refresh_dotmatrix(RgnAdapter *rgn_adapter, bool is_init);

    private:
        MI_RGN_PixelFormat_e pixel_fmt;
        unsigned int         color;
        SizeLevel            size_level;
        SizeLevel            size_level_last;

        std::map<RgnTargetTiming, DotMatrixDesc> map_timing_desc;
        stream_packet_obj                        last, curr;
    };
    class CoverControl : public RgnControl
    {
    public:
        explicit CoverControl(unsigned int devId, const AmigosSurfaceRgn::CoverInfo &cover_info);
        ~CoverControl() override;

    protected:
        void _add_target(const RgnTargetInfo &target) override;
        void _del_target(const RgnTargetInfo &target) override;
        void _set_param(const RgnTargetInfo &target) override;
        bool _check_packet(stream_packet_obj &packet) override;
        int _set_attr(const AmigosSurfaceRgn::RgnInputInfo &info) override;
        void _move_cover(const RgnTargetInfo &target);

    private:
        MI_RGN_CoverMode_e mode;
        unsigned int       color;
        SizeLevel          block_size_lv;

        static const unsigned int COVER_MAX_NUM = 32;
        RgnAdapter               *rgn_adapters[COVER_MAX_NUM];
        unsigned int              use_count;
        stream_packet_obj         curr;
    };
    class FrameControl : public RgnControl
    {
    public:
        explicit FrameControl(unsigned int devId, const AmigosSurfaceRgn::FrameInfo &frame_info);
        ~FrameControl() override;

    protected:
        void _add_target(const RgnTargetInfo &target) override;
        void _del_target(const RgnTargetInfo &target) override;
        void _set_param(const RgnTargetInfo &target) override;
        bool _check_packet(stream_packet_obj &packet) override;
        int _set_attr(const AmigosSurfaceRgn::RgnInputInfo &info) override;
        void _move_frame(const RgnTargetInfo &target);

    private:
        unsigned int color;
        ThicknessLevel thickness_lv;

        static const unsigned int FRAME_MAX_NUM = 32;
        RgnAdapter               *rgn_adapters[FRAME_MAX_NUM];
        unsigned int              use_count;
        stream_packet_obj         curr;
    };
    class PolyControl : public RgnControl
    {
    public:
        explicit PolyControl(unsigned int devId, const AmigosSurfaceRgn::CoverInfo &poly_info);
        ~PolyControl() override;

    protected:
        void _add_target(const RgnTargetInfo &target) override;
        void _del_target(const RgnTargetInfo &target) override;
        void _set_param(const RgnTargetInfo &target) override;
        bool _check_packet(stream_packet_obj &packet) override;
        int _set_attr(const AmigosSurfaceRgn::RgnInputInfo &info) override;
        void _move_poly(const RgnTargetInfo &target);

    private:
        MI_RGN_CoverMode_e mode;
        unsigned int       color;
        SizeLevel          block_size_lv;

        static const unsigned int POLY_MAX_NUM = 32;
        RgnAdapter               *rgn_adapters[POLY_MAX_NUM];
        unsigned int              use_count;
        stream_packet_obj         curr;
    };

public:
    explicit AmigosModuleRgn(const std::string &strInSection);
    ~AmigosModuleRgn() override;

public:
    unsigned int GetModId() const override;
    unsigned int GetInputType(unsigned int port) const override;
    unsigned int GetOutputType(unsigned int port) const override;

    bool Attach(const AmigosSurfaceRgn::RgnModAttachInfo &attach_info, bool bUserTiming);
    bool Detach(const AmigosSurfaceRgn::RgnModAttachInfo &attach_info);
    bool SetAttr(unsigned int inPortId, const AmigosSurfaceRgn::RgnInputInfo &info);
    bool SetColorInvert(const AmigosSurfaceRgn::RgnInfo &info, bool is_init);
    bool Process(unsigned int inPortId, stream_packet_obj &packet);

protected:
    void _Init() override;
    void _Deinit() override;
    void _Start() override;
    void _Stop() override;

    int  _DataReceiver(unsigned int inPortId, stream_packet_obj &packet) override;
    void _StartIn(unsigned int inPortId) override;
    void _StopIn(unsigned int inPortId) override;

    ss_linker_base *_CreateInputNegativeLinker(unsigned int inPortId) override;
    stream_packer *_CreateInputStreamPacker(unsigned int inPortId, bool &bFast) override;

private:
    bool _ConvertAttachInfoToTargetInfo(const RgnModAttachInfo &attach_info, RgnTargetInfo &target,
                                        bool bUseEnvTiming = true);

private:
    std::map<unsigned int, RgnAdapter *> map_canvas_rgn_adapter;
    std::map<unsigned int, RgnControl *> map_rgn_control;
    MI_RGN_ColorInvertAttr_t             color_invert_attr;
    bool                                 color_invert_thread_alive;
    std::list<RgnTargetInfo>             target_lst;
    pthread_rwlock_t                     rgn_lock;
    void                                *threadHandle;
    static unsigned int                  init_count;
};
#endif /* __AMIGOS_MODULE_RGN_H__ */
