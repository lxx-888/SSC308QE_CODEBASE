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
#include "ss_graphic.h"
#include "ss_graphic_factory.h"
#include "ss_buffer.h"
#include "font.h"


static int graphic_create_buffer(std::vector<std::string> &in_strs)
{
    const string &handle = in_strs[1];
    uint32_t frame = ss_cmd_atoi(in_strs[2].c_str());
    PixelFormat_e format;
    format = SsBuffer::PixelFmt_from_str(in_strs[3]);
    uint32_t u32Width = ss_cmd_atoi(in_strs[4].c_str());
    uint32_t u32Height = ss_cmd_atoi(in_strs[5].c_str());

    SsBuffer *buf = SsBuffer::GetInstance(handle);
    if(nullptr != buf)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "This handle has create!\n");
        return -1;
    }

    buf = SsBuffer::Create(handle, format, u32Width, u32Height, frame, nullptr);
    if(nullptr == buf)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Create failed!\n");
        return -1;
    }

    return 0;
}

static int graphic_destroy_buffer(std::vector<std::string> &in_strs)
{
    const string &handle = in_strs[1];

    SsBuffer *buf = SsBuffer::GetInstance(handle);
    if(nullptr == buf)
    {
        return 0;
    }
    ss_handle::destroy(handle);
    return 0;
}

static int graphic_set_color_mode(std::vector<std::string> &in_strs)
{
    const string &handle = in_strs[1];

    SsBuffer *buf = SsBuffer::GetInstance(handle);
    if(nullptr == buf)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "This handle has not create!\n");
        return -1;
    }
    buf->set_color_mode(SsBuffer::ColorMode_e_from_str(in_strs[2]));
    return 0;
}

static int graphic_init_font(std::vector<std::string> &in_strs)
{
    std::string filePath = in_strs[1].c_str();

    Font::GetInstance()->Init(filePath.c_str());
    return 0;
}

static int graphic_fill_color(std::vector<std::string> &in_strs)
{
    const string &handle = in_strs[1];
    int color = ss_cmd_atoi(in_strs[2].c_str());
    uint32_t frame_start = ss_cmd_atoi(in_strs[3].c_str());
    uint32_t frame_end = ss_cmd_atoi(in_strs[4].c_str());

    SsBuffer *buf = SsBuffer::GetInstance(handle);
    if(nullptr == buf)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "This handle has not create!\n");
        return -1;
    }
    return buf->fill_color(color, frame_start, frame_end);
}

static int graphic_fill_rect(std::vector<std::string> &in_strs)
{
    const string &handle = in_strs[1];
    int color = ss_cmd_atoi(in_strs[2].c_str());
    uint32_t x = ss_cmd_atoi(in_strs[3].c_str());
    uint32_t y = ss_cmd_atoi(in_strs[4].c_str());
    uint32_t width = ss_cmd_atoi(in_strs[5].c_str());
    uint32_t height = ss_cmd_atoi(in_strs[6].c_str());
    uint32_t frame_start = ss_cmd_atoi(in_strs[7].c_str());
    uint32_t frame_end = ss_cmd_atoi(in_strs[8].c_str());

    SsBuffer *buf = SsBuffer::GetInstance(handle);
    if(nullptr == buf)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "This handle has not create!\n");
        return -1;
    }
    return buf->fill_rect(color, frame_start, frame_end, x, y, width, height);
}
static int graphic_fill_ellipse(std::vector<std::string> &in_strs)
{
    const string &handle = in_strs[1];
    int color = ss_cmd_atoi(in_strs[2].c_str());
    uint32_t x = ss_cmd_atoi(in_strs[3].c_str());
    uint32_t y = ss_cmd_atoi(in_strs[4].c_str());
    uint32_t width = ss_cmd_atoi(in_strs[5].c_str());
    uint32_t height = ss_cmd_atoi(in_strs[6].c_str());
    uint32_t frame_start = ss_cmd_atoi(in_strs[7].c_str());
    uint32_t frame_end = ss_cmd_atoi(in_strs[8].c_str());

    SsBuffer *buf = SsBuffer::GetInstance(handle);
    if(nullptr == buf)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "This handle has not create!\n");
        return -1;
    }
    return buf->fill_ellipse(color, frame_start, frame_end, x, y, width, height);
}
static int graphic_draw_line(std::vector<std::string> &in_strs)
{
    const string &handle = in_strs[1];
    int color = ss_cmd_atoi(in_strs[2].c_str());
    uint32_t x0 = ss_cmd_atoi(in_strs[3].c_str());
    uint32_t y0 = ss_cmd_atoi(in_strs[4].c_str());
    uint32_t x1 = ss_cmd_atoi(in_strs[5].c_str());
    uint32_t y1 = ss_cmd_atoi(in_strs[6].c_str());
    uint32_t frame_start = ss_cmd_atoi(in_strs[7].c_str());
    uint32_t frame_end = ss_cmd_atoi(in_strs[8].c_str());

    SsBuffer *buf = SsBuffer::GetInstance(handle);
    if(nullptr == buf)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "This handle has not create!\n");
        return -1;
    }
    return buf->draw_line(color, frame_start, frame_end, x0, y0, x1, y1);
}
static int graphic_draw_font(std::vector<std::string> &in_strs)
{
    const string &handle = in_strs[1];
    int color = ss_cmd_atoi(in_strs[2].c_str());
    uint32_t x = ss_cmd_atoi(in_strs[3].c_str());
    uint32_t y = ss_cmd_atoi(in_strs[4].c_str());
    std::string str = in_strs[5].c_str();
    uint32_t font_size = ss_cmd_atoi(in_strs[6].c_str());
    uint32_t frame_start = ss_cmd_atoi(in_strs[7].c_str());
    uint32_t frame_end = ss_cmd_atoi(in_strs[8].c_str());

    SsBuffer *buf = SsBuffer::GetInstance(handle);
    if(nullptr == buf)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "This handle has not create!\n");
        return -1;
    }
    return buf->draw_font(color, frame_start, frame_end, x, y, str, font_size);
}
static int graphic_load_file(std::vector<std::string> &in_strs)
{
    const string &handle = in_strs[1];
    uint32_t frame_start = ss_cmd_atoi(in_strs[3].c_str());
    uint32_t frame_end = ss_cmd_atoi(in_strs[4].c_str());

    SsBuffer *buf = SsBuffer::GetInstance(handle);
    if(nullptr == buf) {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "This handle has not create!\n");
        return -1;
    }
    return buf->load_file(in_strs[2].c_str(), frame_start, frame_end);
}
static int graphic_md5_hash(std::vector<std::string> &in_strs)
{
    const string &handle = in_strs[1];
    std::string hash = in_strs[2].c_str();

    SsBuffer *buf = SsBuffer::GetInstance(handle);
    if(nullptr == buf) {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "This handle has create!\n");
        return -1;
    }
    std::string hash_ret = buf->md5_hash();
    sslog << "md5:" << hash_ret << endl;
    return (hash_ret.compare(hash) == 0) ? 0 : -1;
}
static int graphic_save_file(std::vector<std::string> &in_strs)
{
    const string &handle = in_strs[1];

    SsBuffer *buf = SsBuffer::GetInstance(handle);
    if(nullptr == buf) {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "This handle has create!\n");
        return -1;
    }
    return buf->save_file(in_strs[2].c_str());
}

MOD_CMDS(graphic) {
    ADD_CMD("graphic_create_buffer", graphic_create_buffer, 5);
    ADD_CMD_HELP("graphic_create_buffer", "[handle] [frame] [format] [width] [height]", "format: argb8888 argb1555 argb4444 rgb565 i8 i4 i2 yuv422(yuyv) yuv420sp(nv12)");
    ADD_CMD("graphic_destroy_buffer", graphic_destroy_buffer, 1);
    ADD_CMD_HELP("graphic_destroy_buffer", "[handle]", " ");
    ADD_CMD("graphic_set_color_mode", graphic_set_color_mode, 2);
    ADD_CMD_HELP("graphic_set_color_mode", "[handle] [color_mode]", "color_mode: argb8888 direct");
    ADD_CMD("graphic_init_font", graphic_init_font, 1);
    ADD_CMD_HELP("graphic_init_font", "[filePath]", "ttf file");
    ADD_CMD("graphic_fill_color", graphic_fill_color, 4);
    ADD_CMD_HELP("graphic_fill_color", "[handle] [color] [frame_start] [frame_end]", "color format: ARGB8888");
    ADD_CMD("graphic_fill_rect", graphic_fill_rect, 8);
    ADD_CMD_HELP("graphic_fill_rect", "[handle] [color] [x] [y] [width] [height] [frame_start] [frame_end]", "color format: ARGB8888");
    ADD_CMD("graphic_fill_ellipse", graphic_fill_ellipse, 8);
    ADD_CMD_HELP("graphic_fill_ellipse", "[handle] [color] [x] [y] [width] [height] [frame_start] [frame_end]", "color format: ARGB8888");
    ADD_CMD("graphic_draw_line", graphic_draw_line, 8);
    ADD_CMD_HELP("graphic_draw_line", "[handle] [color] [x0] [y0] [x1] [y1] [frame_start] [frame_end]", "color format: ARGB8888");
    ADD_CMD("graphic_draw_font", graphic_draw_font, 8);
    ADD_CMD_HELP("graphic_draw_font", "[handle] [color] [x] [y] [string] [font_height] [frame_start] [frame_end]", "color format: ARGB8888");
    ADD_CMD("graphic_load_file", graphic_load_file, 4);
    ADD_CMD_HELP("graphic_load_file", "[handle] [filePath] [frame_start] [frame_end]", " ");
    ADD_CMD("graphic_md5_hash", graphic_md5_hash, 2);
    ADD_CMD_HELP("graphic_md5_hash", "[handle] [md5]", " ");
    ADD_CMD("graphic_save_file", graphic_save_file, 2);
    ADD_CMD_HELP("graphic_save_file", "[handle] [filePath]", " ");
}

