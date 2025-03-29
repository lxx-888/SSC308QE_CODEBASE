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

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "ss_packet.h"
#ifdef INTERFACE_SYS
#include "mi_sys.h"
#endif
#include "amigos_module_init.h"
#include "amigos_module_osd.h"

#define _alp(_d) (((_d)&0x000000FF)>>6)
#define _avg(_c00,_c01,_c10,_c11) (((((_c00)+(_c01)+1)>>1)+(((_c10)+(_c11)+1)>>1)+1)>>1)
#define _alpmax(_a, _b) (_alp(_a)>_alp(_b)?(_a):(_b))
#define rgb2y(_r, _g, _b) ((((((306*(_r))>>2)+150*(_g)+29*(_b))>>7)+1)>>1)
#define rgb2u(_r, _g, _b) ((64*(_b)+16384-22*(_r)-42*(_g))>>7)
#define rgb2v(_r, _g, _b) ((64*(_r)+16384-54*(_g)-10*(_b))>>7)
#define rgb2uv(_r, _g, _b) (rgb2v(_r, _g, _b)<<8)|rgb2u(_r, _g, _b) //U:[bit0~7], V:[bit8~15]

//UV Drop Mode
#define uvavg(_c00,_c01,_c10,_c11) (_c00)

//UV EvgMode
//#define uvavg(_c00,_c01,_c10,_c11) _avg(_c00, _c01, _c10, _c11)

//Alpha Drop Mode
#define alpavg(_a00,_a01,_a10,_a11) (_a00)

//Alpha Evg mode
//#define alpavg(_a00,_a01,_a10,_a11) _avg(_a00, _a01, _a10, _a11)

//ECO With This
//#define alpavg(_a00,_a01,_a10,_a11) _alpmax(_alpmax(_a00, _a01), _alpmax(_a10, _a11))

#define blending(_alp,_gop,_base) ((_alp == 255)?(_gop):(((_alp)*(_gop)+(256-(_alp))*(_base))>>8))
//#define blending(_alp,_gop,_base) (((_alp)*(_gop)+(256-(_alp))*(_base))>>8)
//#define blending(_alp,_gop,_base) (_gop)

struct yuv444_2line
{
    unsigned char *yuv[2];
};

struct ayuv_2line
{
    unsigned int *ayuv[2];
};
struct yuv420sp_2line
{
    unsigned char *y[2];
    unsigned char *uv;
};

static inline void osd_blending_2line(const struct ayuv_2line *ayuv, const struct yuv420sp_2line *yuv,
                        unsigned short width, struct yuv420sp_2line *out_yuv)
{
    unsigned char alpha_avg = 0;
    unsigned char u_avg = 0;
    unsigned char v_avg = 0;
    unsigned short i = 0;
    struct ayuv_val
    {
        unsigned char y_val;
        unsigned char u_val;
        unsigned char v_val;
        unsigned char a_val;
    };
    struct ayuv_val *ayuv_val[4];
    for (i = 0; i < width; i += 2)
    {
        ayuv_val[0] = (struct ayuv_val *)&ayuv->ayuv[0][i];
        ayuv_val[1] = (struct ayuv_val *)&ayuv->ayuv[0][i + 1];
        ayuv_val[2] = (struct ayuv_val *)&ayuv->ayuv[1][i];
        ayuv_val[3] = (struct ayuv_val *)&ayuv->ayuv[1][i + 1];
        out_yuv->y[0][i]     = blending(ayuv_val[0]->a_val, ayuv_val[0]->y_val, yuv->y[0][i]);
        out_yuv->y[0][i + 1] = blending(ayuv_val[1]->a_val, ayuv_val[1]->y_val, yuv->y[0][i + 1]);
        out_yuv->y[1][i]     = blending(ayuv_val[2]->a_val, ayuv_val[2]->y_val, yuv->y[1][i]);
        out_yuv->y[1][i + 1] = blending(ayuv_val[3]->a_val, ayuv_val[3]->y_val, yuv->y[1][i + 1]);
        alpha_avg            = alpavg(ayuv_val[0]->a_val, ayuv_val[1]->a_val, ayuv_val[2]->a_val, ayuv_val[3]->a_val);
        u_avg                = uvavg(ayuv_val[0]->u_val, ayuv_val[1]->u_val, ayuv_val[2]->u_val, ayuv_val[3]->u_val);
        v_avg                = uvavg(ayuv_val[0]->v_val, ayuv_val[1]->v_val, ayuv_val[2]->v_val, ayuv_val[3]->v_val);
        out_yuv->uv[i]       = blending(alpha_avg, u_avg, yuv->uv[i]);
        out_yuv->uv[i + 1]   = blending(alpha_avg, v_avg, yuv->uv[i + 1]);
    }
    return;
}
static inline void osd_convert_argb1555_line_2_ayuv444_pixel_alpha(const short *line_buf, unsigned int width, int *a_yuv, unsigned char alpha0, unsigned char alpha1)
{
    unsigned char r_val, g_val, b_val;
    for (unsigned int i = 0; i < width; i++)
    {
        r_val = ((line_buf[i] & 0x7C00) | ((line_buf[i] & 0x7000) >> 5)) >> 7;
        g_val = ((line_buf[i] & 0x3E0) | ((line_buf[i] & 0x380) >> 5)) >> 2;
        b_val = ((line_buf[i] & 0x1F) << 3) | ((line_buf[i] & 0x1C) >> 2);
        a_yuv[i] = (((line_buf[i] & 0x8000)? alpha1: alpha0) << 24) |
                  (rgb2v(r_val, g_val, b_val) << 16) |
                  (rgb2u(r_val, g_val, b_val) << 8) |
                  (rgb2y(r_val, g_val, b_val));
    }
}
static inline void osd_convert_argb4444_line_2_ayuv444_pixel_alpha(const short *line_buf, unsigned int width, int *a_yuv)
{
    unsigned char r_val, g_val, b_val;
    for (unsigned int i = 0; i < width; i++)
    {
        r_val = ((line_buf[i] & 0xF00) | ((line_buf[i] & 0xF00) >> 4)) >> 4;
        g_val = (line_buf[i] & 0xF0) | ((line_buf[i] & 0xF0) >> 4);
        b_val = ((line_buf[i] & 0xF) << 4) | (line_buf[i] & 0xF);
        a_yuv[i] = (((line_buf[i] & 0xF000) | ((line_buf[i] & 0xF000) >> 4)) << 16) |
                  (rgb2v(r_val, g_val, b_val) << 16) |
                  (rgb2u(r_val, g_val, b_val) << 8) |
                  (rgb2y(r_val, g_val, b_val));
    }
}
static inline void osd_convert_argb1555_line_2_ayuv444_constant_alpha(const short *line_buf, unsigned int width, int *a_yuv, unsigned char alpha)
{
    unsigned char r_val, g_val, b_val;
    for (unsigned int i = 0; i < width; i++)
    {
        r_val = ((line_buf[i] & 0x7C00) | ((line_buf[i] & 0x7000) >> 5)) >> 7;
        g_val = ((line_buf[i] & 0x3E0) | ((line_buf[i] & 0x380) >> 5)) >> 2;
        b_val = ((line_buf[i] & 0x1F) << 3) | ((line_buf[i] & 0x1C) >> 2);
        a_yuv[i] = (alpha << 24) |
                  (rgb2v(r_val, g_val, b_val) << 16) |
                  (rgb2u(r_val, g_val, b_val) << 8) |
                  (rgb2y(r_val, g_val, b_val));
    }
}
static inline void osd_convert_argb4444_line_2_ayuv444_constant_alpha(const short *line_buf, unsigned int width, int *a_yuv, unsigned char alpha)
{
    unsigned char r_val, g_val, b_val;
    for (unsigned int i = 0; i < width; i++)
    {
        r_val = ((line_buf[i] & 0xF00) | ((line_buf[i] & 0xF00) >> 4)) >> 4;
        g_val = (line_buf[i] & 0xF0) | ((line_buf[i] & 0xF0) >> 4);
        b_val = ((line_buf[i] & 0xF) << 4) | (line_buf[i] & 0xF);
        a_yuv[i] = (alpha << 24) |
                  (rgb2v(r_val, g_val, b_val) << 16) |
                  (rgb2u(r_val, g_val, b_val) << 8) |
                  (rgb2y(r_val, g_val, b_val));
    }
}
static inline void osd_convert_rgb565_line_2_ayuv444_constant_alpha(const short *line_buf, unsigned int width, int *a_yuv, unsigned char alpha)
{
    unsigned char r_val, g_val, b_val;
    for (unsigned int i = 0; i < width; i++)
    {
        r_val = ((line_buf[i] & 0xF800) | ((line_buf[i] & 0xE000) >> 5)) >> 8;
        g_val = ((line_buf[i] & 0x7E0) | ((line_buf[i] & 0x60) >> 6)) >> 3;
        b_val = ((line_buf[i] & 0x1F) << 3) | ((line_buf[i] & 0x1C) >> 2);
        a_yuv[i] = (alpha << 24) |
                  (rgb2v(r_val, g_val, b_val) << 16) |
                  (rgb2u(r_val, g_val, b_val) << 8) |
                  (rgb2y(r_val, g_val, b_val));
    }
}

static inline void osd_convert_argb8888_line_2_ayuv444_constant_alpha(const int *line_buf, unsigned int width, int *a_yuv, unsigned char alpha)
{
    unsigned char r_val, g_val, b_val;
    for (unsigned int i = 0; i < width; i++)
    {
        r_val = (line_buf[i] & 0xFF0000) >> 16;
        g_val = (line_buf[i] & 0xFF00) >> 8;
        b_val = line_buf[i] & 0xFF;
        a_yuv[i] = (alpha << 24) |
                  (rgb2v(r_val, g_val, b_val) << 16) |
                  (rgb2u(r_val, g_val, b_val) << 8) |
                  (rgb2y(r_val, g_val, b_val));
    }
}
static inline unsigned int get_ms()
{
    struct timespec ts;
    unsigned int ms;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    ms = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
    if(ms == 0)
    {
        ms = 1;
    }
    return ms;
}

AmigosModuleOsd::AmigosModuleOsd(const std::string &strInSection)
    : AmigosSurfaceOsd(strInSection), AmigosModuleBase(this)
{
    AMIGOS_INFO("func: %s\n", __FUNCTION__);
}
AmigosModuleOsd::~AmigosModuleOsd()
{
    AMIGOS_INFO("func: %s\n", __FUNCTION__);
}
unsigned int AmigosModuleOsd::GetModId() const
{
    return uintExtModId;
}
unsigned int AmigosModuleOsd::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleOsd::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
int AmigosModuleOsd::_DataReceiver(unsigned int inPortId, stream_packet_obj &packet)
{
    if (packet->en_type != EN_RAW_FRAME_DATA)
    {
        AMIGOS_ERR("Not support current format!\n");
        return -1;
    }
    switch (packet->raw_vid_i.plane_info[0].fmt)
    {
        case RAW_FORMAT_YUV420SP:
        {
            struct ayuv_2line ayuv;
            struct yuv420sp_2line yuv;
            unsigned int now = get_ms();
            for (auto it = this->listOsdInfo.begin(); it != this->listOsdInfo.end(); ++it)
            {
                if (it->uintOsdPosX >= packet->raw_vid_i.plane_info[0].width
                    || it->uintOsdPosY >= packet->raw_vid_i.plane_info[0].height)
                {
                    continue;
                }
                unsigned int cut_w = (it->uintOsdPosX + it->uintOsdWidth > packet->raw_vid_i.plane_info[0].width) ?
                                      (packet->raw_vid_i.plane_info[0].width - it->uintOsdPosX) : it->uintOsdWidth;
                unsigned int cut_h = (it->uintOsdPosY + it->uintOsdHeight > packet->raw_vid_i.plane_info[0].height) ?
                                      (packet->raw_vid_i.plane_info[0].height - it->uintOsdPosY) : it->uintOsdHeight;
                unsigned int pos_x = it->uintOsdPosX >> 1 << 1;
                unsigned int pos_y = it->uintOsdPosY >> 1 << 1;
                ayuv.ayuv[0] = (unsigned int *)it->pExtData;
                ayuv.ayuv[1] = ayuv.ayuv[0] + it->uintOsdWidth;
                yuv.y[0] = (unsigned char *)(packet->raw_vid.plane_data[0].data[0] + pos_x + pos_y * packet->raw_vid.plane_data[0].stride[0]);
                yuv.y[1] = yuv.y[0] + packet->raw_vid.plane_data[0].stride[0];
                yuv.uv = (unsigned char*)(packet->raw_vid.plane_data[0].data[1] + pos_x + (pos_y >> 1) * packet->raw_vid.plane_data[0].stride[1]);
                osd_blending_2line(&ayuv, &yuv, cut_w, &yuv);
                for (unsigned int i = 2; i < cut_h; i += 2)
                {
                    ayuv.ayuv[0] += (it->uintOsdWidth << 1);
                    ayuv.ayuv[1] = ayuv.ayuv[0] + it->uintOsdWidth;
                    yuv.y[0] += (packet->raw_vid.plane_data[0].stride[0] << 1);
                    yuv.y[1] = yuv.y[0] + packet->raw_vid.plane_data[0].stride[0];
                    yuv.uv += packet->raw_vid.plane_data[0].stride[1];
                    osd_blending_2line(&ayuv, &yuv, cut_w, &yuv);
                }
            }
            std::cout << "Blending Diff: " << get_ms() - now << std::endl;
        }
        break;
        default:
            AMIGOS_ERR("Not support now!\n");
            break;
    }
    unsigned int outPortId = inPortId;
    auto iterPortOut = this->mapPortOut.find(outPortId);
    if (iterPortOut == this->mapPortOut.end())
    {
        AMIGOS_ERR("Port %d has not base sturcture.\n", outPortId);
        return -1;
    }
    iterPortOut->second.positive.enqueue(packet);
    return 0;
}

void AmigosModuleOsd::_Init()
{
    for (auto it = listOsdInfo.begin(); it != listOsdInfo.end(); ++it)
    {
        int fd = open(it->osdFile.c_str(), O_RDONLY);
        if (fd < 0)
        {
            AMIGOS_ERR("Open %s Error.\n", it->osdFile.c_str());
            return;
        }
        it->pExtData = new int[it->uintOsdWidth * it->uintOsdHeight] ;
        assert(it->pExtData);
        if (it->osdFormat == "argb1555")
        {
            short *line_buf = new short[it->uintOsdWidth];
            assert(line_buf);
            if (it->alphaMode == "pixel")
            {
                for (unsigned int i = 0; i < it->uintOsdHeight; i++)
                {
                    read(fd, line_buf, it->uintOsdWidth * 2);
                    osd_convert_argb1555_line_2_ayuv444_pixel_alpha(line_buf, it->uintOsdWidth, (int *)it->pExtData + it->uintOsdWidth * i, it->ucharAlpha0, it->ucharAlpha1);
                }
            }
            else if (it->alphaMode == "constant")
            {
                for (unsigned int i = 0; i < it->uintOsdHeight; i++)
                {
                    read(fd, line_buf, it->uintOsdWidth * 2);
                    osd_convert_argb1555_line_2_ayuv444_constant_alpha(line_buf, it->uintOsdWidth, (int *)it->pExtData + it->uintOsdWidth * i, it->ucharConstantAlpha);
                }
            }
            else
            {
                assert(0);
            }
            delete [] line_buf;
        }
        else if (it->osdFormat == "argb4444")
        {

            short *line_buf = new short[it->uintOsdWidth];
            assert(line_buf);
            if (it->alphaMode == "pixel")
            {
                for (unsigned int i = 0; i < it->uintOsdHeight; i++)
                {
                    read(fd, line_buf, it->uintOsdWidth * 2);
                    osd_convert_argb4444_line_2_ayuv444_pixel_alpha(line_buf, it->uintOsdWidth, (int *)it->pExtData + it->uintOsdWidth * i);
                }
            }
            else if (it->alphaMode == "constant")
            {
                for (unsigned int i = 0; i < it->uintOsdHeight; i++)
                {
                    read(fd, line_buf, it->uintOsdWidth * 2);
                    osd_convert_argb4444_line_2_ayuv444_constant_alpha(line_buf, it->uintOsdWidth, (int *)it->pExtData + it->uintOsdWidth * i, it->ucharConstantAlpha);
                }
            }
            else
            {
                assert(0);
            }
            delete [] line_buf;

        }
        else if (it->osdFormat == "rgb565")
        {
            short *line_buf = new short[it->uintOsdWidth];
            assert(line_buf);
            for (unsigned int i = 0; i < it->uintOsdHeight; i++)
            {
                read(fd, line_buf, it->uintOsdWidth * 2);
                osd_convert_rgb565_line_2_ayuv444_constant_alpha(line_buf, it->uintOsdWidth, (int *)it->pExtData + it->uintOsdWidth * i, it->ucharConstantAlpha);
            }
            delete [] line_buf;
        }
        else if (it->osdFormat == "argb8888")
        {
            if (it->alphaMode == "pixel")
            {
                for (unsigned int i = 0; i < it->uintOsdHeight; i++)
                {
                    read(fd, (int *)it->pExtData + it->uintOsdWidth * i, it->uintOsdWidth * 4);
                }
            }
            else if (it->alphaMode == "constant")
            {
                int *line_buf = new int[it->uintOsdWidth];
                assert(line_buf);
                for (unsigned int i = 0; i < it->uintOsdHeight; i++)
                {
                    read(fd, (int *)line_buf, it->uintOsdWidth * 4);
                    osd_convert_argb8888_line_2_ayuv444_constant_alpha(line_buf, it->uintOsdWidth, (int *)it->pExtData + it->uintOsdWidth * i, it->ucharConstantAlpha);
                }
                delete [] line_buf;
            }

        }
        else if (it->osdFormat == "i8")
        {
        }
        else if (it->osdFormat == "i4")
        {
            if (it->uintOsdWidth / 2)
            {
            }
        }
        else if (it->osdFormat == "i2")
        {
            if (it->uintOsdWidth / 4)
            {
            }
        }
        close(fd);
    }
}
void AmigosModuleOsd::_Deinit()
{
    for (auto it = listOsdInfo.begin(); it != listOsdInfo.end(); ++it)
    {
        delete [](int *)it->pExtData;
        it->pExtData = NULL;
    }
}
AMIGOS_MODULE_INIT("OSD", AmigosModuleOsd);
