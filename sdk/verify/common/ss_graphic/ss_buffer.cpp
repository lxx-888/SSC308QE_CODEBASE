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
#include <iostream>
#include <fstream>
#include <cstring>
#include <assert.h>
#include "md5.hpp"
#include "ss_buffer.h"
#include "ss_graphic_factory.h"
#include "ss_graphic.h"


SsBuffer *SsBuffer::Create(const string &handle, PixelFormat_e format, uint32_t width, uint32_t height, uint32_t frame, void *ptr)
{
    SsBuffer *buf = new SsBuffer(format, width, height, frame, ptr);
    assert(buf);
    if(!ss_handle::install(handle, buf))
    {
        delete buf;
        return NULL;
    }
    return buf;
}
SsBuffer *SsBuffer::Create(const string &handle, uint32_t size, uint32_t frame)
{
    SsBuffer *buf = new SsBuffer(E_BUFFER_NODE_STREAM, size, 1, frame, nullptr);
    assert(buf);
    if(!ss_handle::install(handle, buf))
    {
        delete buf;
        return NULL;
    }
    return buf;
}
SsBuffer *SsBuffer::GetInstance(const string &handle)
{
    SsBuffer *buf = dynamic_cast <SsBuffer *>(SsBuffer::get(handle));
    if(nullptr == buf) {
        return nullptr;
    }
    return buf;
}
std::string SsBuffer::md5_hash()
{
    return md5::md5_hash_buffer((unsigned char*)this->data, this->size);
}
int32_t SsBuffer::fill_color(int color, uint32_t frame_start, uint32_t frame_end)
{
    if(frame_start > frame_end || frame_end > this->u32Frame) {
        return -1;
    }
    for (uint32_t i = frame_start; i <= frame_end; i++) {
        this->get_node(i)->fill_color(color, mode);
    }
    return 0;
}
void SsBuffer::set_color_mode(ColorMode_e mode)
{
    this->mode = mode;
}
int SsBuffer::fill_rect(int color, uint32_t frame_start, uint32_t frame_end, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    if(frame_start > frame_end || frame_end > this->u32Frame) {
        return -1;
    }
    for (uint32_t i = frame_start; i <= frame_end; i++) {
        this->get_node(i)->fill_rect(color, this->mode, x, y, width, height);
    }
    return 0;
}
int SsBuffer::fill_ellipse(int color, uint32_t frame_start, uint32_t frame_end, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    if(frame_start > frame_end || frame_end > this->u32Frame) {
        return -1;
    }
    for (uint32_t i = frame_start; i <= frame_end; i++) {
        this->get_node(i)->fill_ellipse(color, this->mode, x, y, width, height);
    }
    return 0;
}
int SsBuffer::draw_line(int color, uint32_t frame_start, uint32_t frame_end, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1)
{
    if(frame_start > frame_end || frame_end > this->u32Frame) {
        return -1;
    }
    for (uint32_t i = frame_start; i <= frame_end; i++) {
        this->get_node(i)->draw_line(color, this->mode, x0, y0, x1, y1);
    }
    return 0;
}
int SsBuffer::draw_font(int color, uint32_t frame_start, uint32_t frame_end, uint32_t x, uint32_t y, std::string str, uint32_t font_size)
{
    if(frame_start > frame_end || frame_end > this->u32Frame) {
        return -1;
    }
    for (uint32_t i = frame_start; i <= frame_end; i++) {
        this->get_node(i)->draw_font(color, this->mode, x, y, str, font_size);
    }
    return 0;
}
int SsBuffer::save_file(std::string filepath)
{
    std::ofstream fout(filepath, std::ios::out & std::ios::binary & std::ios::app);
    if(!fout.is_open()) {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Missing file!\n");
        return -1;
    }
    for (uint32_t i = 0; i < this->u32Frame; i++) {
        fout.write(this->get_node(i)->get_data(), this->get_node(i)->get_size());
    }
    fout.close();
    return 0;
}

int SsBuffer::load_file(std::string filepath, uint32_t frame_start, uint32_t frame_end)
{
    std::ifstream fin(filepath, std::ios::in & std::ios::binary & std::ios::app);
    if (!fin.is_open()) {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Missing file!\n");
        return -1;
    }
    for (uint32_t i = frame_start; i < (frame_end < this->u32Frame ? frame_end : this->u32Frame); i++) {
        fin.read(this->get_node(i)->get_data(), this->get_node(i)->get_size());
    }
    fin.close();
    return 0;
}
