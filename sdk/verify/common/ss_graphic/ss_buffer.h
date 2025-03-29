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

#ifndef _SS_BUFFER_H_
#define _SS_BUFFER_H_
#include "ss_graphic_factory.h"
#include "ss_graphic.h"
#include "ss_convert.h"
#include "ss_handle.h"
#include "ss_default_palette.h"

class SsBuffer :public ss_handle
{
private:
    SsBuffer(PixelFormat_e format, uint32_t width, uint32_t height, uint32_t frame, void *ptr = nullptr) :
        u32Frame(frame)
    {
        uint32_t u32Size = 0;
        switch(format)
        {
            case E_BUFFER_NODE_ARGB8888:
                u32Size = 4 * width * height;
            break;
            case E_BUFFER_NODE_ARGB1555:
            case E_BUFFER_NODE_ARGB4444:
            case E_BUFFER_NODE_RGB565:
            case E_BUFFER_NODE_YUV422:
                u32Size = 2 * width * height;
            break;
            case E_BUFFER_NODE_YUV420SP:
                u32Size = 1.5 * width * height;
            break;
            case E_BUFFER_NODE_I8:
                u32Size = width * height;
            break;
            case E_BUFFER_NODE_I4:
                u32Size = width * height / 2;
            break;
            case E_BUFFER_NODE_I2:
                u32Size = width * height / 4;
            break;
            case E_BUFFER_NODE_STREAM:
                u32Size = width * height * 10;
            break;
            default:
                return;
        }
        bNeedFreeBuffer = 0;
        data = (char *)ptr;
        if(nullptr == ptr)
        {
            data = new char[u32Frame * u32Size];
            bNeedFreeBuffer = 1;
        }
        nodes = new SsGraphic*[u32Frame];
        this->mode = E_COLOR_MODE_ARGB8888;
        for(uint32_t i = 0; i < u32Frame; i++)
        {
            nodes[i] = SsGraphicFactory::Create(format, width, height, data + (i * u32Size));
            nodes[i]->load_palette(default_colors_palette, 256);
            nodes[i]->fill_color(0x0, this->mode);
        }
        this->size = u32Size;
    }
    virtual ~SsBuffer()
    {
        if(nullptr != data && bNeedFreeBuffer)
        {
            delete [] data;
        }
        delete [] nodes;
        data = nullptr;
        nodes = nullptr;
    }
public:
    STR_ENUM(PixelFormat_e, PixelFmt_, {
            {E_BUFFER_NODE_ARGB8888, "argb8888"},
            {E_BUFFER_NODE_ARGB4444, "argb4444"},
            {E_BUFFER_NODE_ARGB1555, "argb1555"},
            {E_BUFFER_NODE_RGB565, "rgb565"},
            {E_BUFFER_NODE_I8, "i8"},
            {E_BUFFER_NODE_I4, "i4"},
            {E_BUFFER_NODE_I2, "i2"},
            {E_BUFFER_NODE_YUV420SP, "yuv420sp"},
            {E_BUFFER_NODE_YUV422, "yuv422"},
            {E_BUFFER_NODE_STREAM, "stream"}
            });
    STR_ENUM(ColorMode_e, ColorMode_e_, {
            {E_COLOR_MODE_ARGB8888, "argb8888"},
            {E_COLOR_MODE_DIRECT, "direct"}
            });
    static SsBuffer *Create(const string &handle, PixelFormat_e format, uint32_t width, uint32_t height, uint32_t frame, void *ptr = nullptr);
    static SsBuffer *Create(const string &handle, uint32_t size, uint32_t frame);
    static SsBuffer *GetInstance(const string &handle);
    SsGraphic *get_node(uint32_t frame) {
        if(frame >= this->u32Frame) {
            return nullptr;
        }
        return this->nodes[frame];
    }
    uint32_t get_frame() {
        return this->u32Frame;
    }
    std::string md5_hash();
    void set_color_mode(ColorMode_e mode);
    int fill_color(int color, uint32_t frame_start, uint32_t frame_end);
    int fill_rect(int color, uint32_t frame_start, uint32_t frame_end, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    int fill_ellipse(int color, uint32_t frame_start, uint32_t frame_end, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    int draw_line(int color, uint32_t frame_start, uint32_t frame_end, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1);
    int draw_font(int color, uint32_t frame_start, uint32_t frame_end, uint32_t x, uint32_t y, std::string str, uint32_t font_size);
    int save_file(std::string filepath);
    int load_file(std::string filepath, uint32_t frame_start, uint32_t frame_end);

private:
    char *data;
    uint32_t size;
    uint32_t u32Frame;
    SsGraphic **nodes;
    bool bNeedFreeBuffer;
    ColorMode_e mode;
};

#endif //_SS_BUFFER_H_
