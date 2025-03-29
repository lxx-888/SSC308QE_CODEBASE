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
#ifndef _SS_GRAPHIC_H_
#define _SS_GRAPHIC_H_

#include <stdint.h>
#include <map>
#include <string>
#include "ss_cmd_base.h"

typedef enum {
    E_BUFFER_NODE_ARGB8888 = 0,
    E_BUFFER_NODE_ARGB1555,
    E_BUFFER_NODE_ARGB4444,
    E_BUFFER_NODE_RGB565,
    E_BUFFER_NODE_I8,
    E_BUFFER_NODE_I4,
    E_BUFFER_NODE_I2,
    E_BUFFER_NODE_YUV420SP,
    E_BUFFER_NODE_YUV422,
    E_BUFFER_NODE_STREAM,
    E_BUFFER_NODE_MAX,
} PixelFormat_e;

typedef enum {
    E_COLOR_MODE_ARGB8888 = 0,
    E_COLOR_MODE_DIRECT,
} ColorMode_e;

class SsGraphic
{
public:
    SsGraphic(uint32_t Width, uint32_t Height, uint8_t BytePreData, uint8_t PixelPreData, void *ptr) :
        data((char*)ptr),
        u32Width(Width),
        u32Height(Height),
        u8BytePreData(BytePreData),
        u8PixelPreData(PixelPreData)
    {
        if (nullptr == data && 0 != this->get_size()) {
            return;
        }
    }
    virtual ~SsGraphic()
    {
        data = nullptr;
    }

public:
    virtual PixelFormat_e get_fmt()const = 0;
    inline uint32_t get_width()const {
        return this->u32Width;
    }
    inline uint32_t get_height()const {
        return this->u32Height;
    }
    inline char* get_data()const {
        return data;
    }
    virtual size_t get_size()const {
        return this->u32Width * this->u32Height * this->u8BytePreData / this->u8PixelPreData;
    }

    virtual void load_palette(uint32_t *pu32PaletteTbl, uint32_t u32PaletteNum);
    virtual void cal_color_key(int color, uint8_t &r, uint8_t &g, uint8_t &b) {}
    void cal_color_key(uint8_t &r, uint8_t &g, uint8_t &b);

    virtual void load_file(std::string filepath);
    virtual void save_file(std::string filepath);
    virtual void copy_to(const void **ppAddr);
    virtual void copy_from(const void **ppAddr);
    virtual void copy_from(const void **ppAddr, const uint32_t *psize, uint32_t count) {
    }
    virtual void set_pixel(int pixel, uint32_t x, uint32_t y) = 0;
    virtual void fill_rect(int color, ColorMode_e mode, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    virtual void fill_ellipse(int color, ColorMode_e mode, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    virtual void draw_line(int color, ColorMode_e mode, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1);
    virtual void draw_bitmap(int color, ColorMode_e mode, uint32_t x, uint32_t y, uint8_t *bitmap, uint32_t width, uint32_t height);
    virtual void draw_font(int color, ColorMode_e mode, uint32_t x, uint32_t y, std::string str, uint32_t font_size);
    inline void fill_color(int color, ColorMode_e mode) {
        this->fill_rect(color, mode, 0, 0, this->u32Width, this->u32Height);
    }

protected:
    virtual int convert_color_to_pixel(int color);

    void convert_color_to_argb(uint32_t color, uint8_t &a, uint8_t &r, uint8_t &g, uint8_t &b);
    void convert_color_to_yuv(uint32_t color, uint8_t &y, uint8_t &u, uint8_t &v);
    void convert_color_to_index(uint32_t color, size_t length, uint8_t &index);
    template <class Color_t>
    void _fill_rect(void *ptr, Color_t color, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t x_align = 1,
                    uint8_t y_align = 1)
    {
        if (ptr == nullptr) {
            return;
        }
        Color_t *dest       = (Color_t *)ptr;
        uint32_t max_width  = this->u32Width / x_align;
        uint32_t max_height = this->u32Height / y_align;
        x                   = uint32_t(x / x_align);
        y                   = uint32_t(y / y_align);
        w                   = uint32_t(w / x_align);
        h                   = uint32_t(h / y_align);

        for (uint32_t line = y; line < ( y + h > max_height ? max_height : y + h ); ++line) {
            std::fill_n(dest + line * max_width + x, x + w > max_width ? (max_width - x) : w, color);
        }
    }

    template <class Color_t>
    void _set_pixel(void *ptr, Color_t color, uint32_t x, uint32_t y, uint8_t x_align = 1, uint8_t y_align = 1)
    {
        if (ptr == nullptr) {
            return;
        }

        uint32_t max_width  = this->u32Width / x_align;
        uint32_t max_height = this->u32Height / y_align;
        x                   = uint32_t(x / x_align);
        y                   = uint32_t(y / y_align);
        if (x >= max_width || y >= max_height) {
            return;
        }

        Color_t *dest = (Color_t*)ptr;

        *( dest + y * max_width + x ) = color;
    }

protected:
    char *data;
    uint32_t u32Width;
    uint32_t u32Height;
    uint8_t u8BytePreData;
    uint8_t u8PixelPreData;
    uint32_t u32PaletteNum;
    uint32_t au32PaletteTbl[256];
};

class YUV420SP_SsGraphic : public SsGraphic
{
public:
    YUV420SP_SsGraphic(uint32_t width, uint32_t height, void *ptr)
        : SsGraphic(( width >> 1 ) << 1, ( height >> 1 ) << 1, 6, 4, ptr) {

    }
    virtual ~YUV420SP_SsGraphic() {

    }
    PixelFormat_e get_fmt()const {
        return E_BUFFER_NODE_YUV420SP;
    }
    virtual void copy_to(const void **ppAddr);
    virtual void copy_from(const void **ppAddr);
    void set_pixel(int pixel, uint32_t x, uint32_t y);
    void fill_rect(int color, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
};

class YUV422_SsGraphic : public SsGraphic
{
public:
    YUV422_SsGraphic(uint32_t width, uint32_t height, void *ptr)
        : SsGraphic(( width >> 1 ) << 1, height, 4, 2, ptr) {

    }
    virtual ~YUV422_SsGraphic() {

    }
    PixelFormat_e get_fmt()const {
        return E_BUFFER_NODE_YUV422;
    }
    int convert_color_to_pixel(int color);
    void set_pixel(int pixel, uint32_t x, uint32_t y);
    void fill_rect(int color, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
};

class ARGB8888_SsGraphic : public SsGraphic
{
public:
    ARGB8888_SsGraphic(uint32_t width, uint32_t height, void *ptr) : SsGraphic(width, height, 4, 1, ptr) {

    }
    virtual ~ARGB8888_SsGraphic() {

    }
    PixelFormat_e get_fmt()const {
        return E_BUFFER_NODE_ARGB8888;
    }
    void set_pixel(int pixel, uint32_t x, uint32_t y);
    void fill_rect(int color, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
    void cal_color_key(int color, uint8_t &r, uint8_t &g, uint8_t &b);
};

class RGB565_SsGraphic : public SsGraphic
{
public:
    RGB565_SsGraphic(uint32_t width, uint32_t height, void *ptr) : SsGraphic(width, height, 2, 1, ptr) {

    }
    virtual ~RGB565_SsGraphic() {

    }
    PixelFormat_e get_fmt()const {
        return E_BUFFER_NODE_RGB565;
    }
    int convert_color_to_pixel(int color);
    void set_pixel(int pixel, uint32_t x, uint32_t y);
    void fill_rect(int color, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
    void cal_color_key(int color, uint8_t &r, uint8_t &g, uint8_t &b);
};

class ARGB1555_SsGraphic : public SsGraphic
{
public:
    ARGB1555_SsGraphic(uint32_t width, uint32_t height, void *ptr) : SsGraphic(width, height, 2, 1, ptr) {

    }
    virtual ~ARGB1555_SsGraphic() {

    }
    PixelFormat_e get_fmt()const {
        return E_BUFFER_NODE_ARGB1555;
    }
    int convert_color_to_pixel(int color);
    void set_pixel(int pixel, uint32_t x, uint32_t y);
    void fill_rect(int color, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
    void cal_color_key(int color, uint8_t &r, uint8_t &g, uint8_t &b);
};

class ARGB4444_SsGraphic : public SsGraphic
{
public:
    ARGB4444_SsGraphic(uint32_t width, uint32_t height, void *ptr) : SsGraphic(width, height, 2, 1, ptr) {

    }
    virtual ~ARGB4444_SsGraphic() {

    }
    PixelFormat_e get_fmt()const {
        return E_BUFFER_NODE_ARGB4444;
    }
    int convert_color_to_pixel(int color);
    void set_pixel(int pixel, uint32_t x, uint32_t y);
    void fill_rect(int color, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
    void cal_color_key(int color, uint8_t &r, uint8_t &g, uint8_t &b);
};

class I2_SsGraphic : public SsGraphic
{
public:
    I2_SsGraphic(uint32_t width, uint32_t height, void *ptr)
        : SsGraphic(( width >> 2 ) << 2, height, 1, 4, ptr) {

    }
    virtual ~I2_SsGraphic() {

    }
    PixelFormat_e get_fmt()const {
        return E_BUFFER_NODE_I2;
    }
    int convert_color_to_pixel(int color);
    void load_palette(uint32_t *pu32PaletteTbl, uint32_t u32PaletteNum);
    void set_pixel(int pixel, uint32_t x, uint32_t y);
    void cal_color_key(int color, uint8_t &r, uint8_t &g, uint8_t &b);
};

class I4_SsGraphic : public SsGraphic
{
public:
    I4_SsGraphic(uint32_t width, uint32_t height, void *ptr)
        : SsGraphic(( width >> 1 ) << 1, height, 1, 2, ptr) {

    }
    virtual ~I4_SsGraphic() {

    }
    PixelFormat_e get_fmt()const {
        return E_BUFFER_NODE_I4;
    }
    int convert_color_to_pixel(int color);
    void load_palette(uint32_t *pu32PaletteTbl, uint32_t u32PaletteNum);
    void set_pixel(int pixel, uint32_t x, uint32_t y);
    void cal_color_key(int color, uint8_t &r, uint8_t &g, uint8_t &b);
};

class I8_SsGraphic : public SsGraphic
{
public:
    I8_SsGraphic(uint32_t width, uint32_t height, void *ptr) : SsGraphic(width, height, 1, 1, ptr) {

    }
    virtual ~I8_SsGraphic() {

    }
    PixelFormat_e get_fmt()const {
        return E_BUFFER_NODE_I8;
    }
    int convert_color_to_pixel(int color);
    void load_palette(uint32_t *pu32PaletteTbl, uint32_t u32PaletteNum);
    void set_pixel(int pixel, uint32_t x, uint32_t y);
    void fill_rect(int color, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
    void cal_color_key(int color, uint8_t &r, uint8_t &g, uint8_t &b);
};

class Stream_SsGraphic : public SsGraphic
{
public:
    Stream_SsGraphic(uint32_t size, void *ptr) : SsGraphic(size, 1, 1, 1, ptr) {
        used_size = 0;
    }
    virtual ~Stream_SsGraphic() {

    }
    PixelFormat_e get_fmt()const {
        return E_BUFFER_NODE_STREAM;
    }
    inline size_t get_size()const {
        return this->used_size;
    }
    void set_pixel(int pixel, uint32_t x, uint32_t y);
    void fill_rect(int color, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
    void copy_from(const void **ppAddr, const uint32_t *psize, uint32_t count);
    void save_file(std::string filepath);
    void load_file(std::string filepath);
private:
    uint32_t used_size;
};
#endif //_SS_GRAPHIC_H_
