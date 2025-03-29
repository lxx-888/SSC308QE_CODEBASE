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

#include "font.h"
#include "ss_graphic.h"

// SsGraphic ///////////////////////////////////////////////////////////////


void SsGraphic::load_file(std::string filepath)
{
    std::ifstream fin(filepath, std::ios::in & std::ios::binary);
    if (!fin.is_open()) {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Missing file!\n");
        std::cout << "Missing file " << filepath << std::endl;
        return ;
    }
    fin.read(this->data, this->get_size());
    fin.close();
}
void SsGraphic::save_file(std::string filepath)
{
    std::ofstream fout(filepath, std::ios::out & std::ios::binary);
    if (!fout.is_open()) {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Missing file!\n");
        return ;
    }
    fout.write(this->data, this->get_size());
    fout.close();
}
void SsGraphic::copy_from(const void **ppAddr)
{
    std::copy((char*)ppAddr[0], (char*)ppAddr[0] + this->get_size(), this->data);
}
void SsGraphic::copy_to(const void **ppAddr)
{
    std::copy(this->data, this->data + this->get_size(), (char*)ppAddr[0]);
}
void SsGraphic::load_palette(uint32_t *pu32PaletteTbl, uint32_t u32PaletteNum)
{
    std::copy(pu32PaletteTbl, pu32PaletteTbl + u32PaletteNum, this->au32PaletteTbl);
    this->u32PaletteNum = u32PaletteNum;
}
void SsGraphic::convert_color_to_argb(uint32_t color, uint8_t &a, uint8_t &r, uint8_t &g, uint8_t &b)
{
    a = ( color & 0xff000000 ) >> 24;
    r = ( color & 0x00ff0000 ) >> 16;
    g = ( color & 0x0000ff00 ) >> 8;
    b = ( color & 0x000000ff ) >> 0;
}
void SsGraphic::convert_color_to_yuv(uint32_t color, uint8_t &y, uint8_t &u, uint8_t &v)
{
    uint8_t r = ( color & 0x00ff0000 ) >> 16;
    uint8_t g = ( color & 0x0000ff00 ) >> 8;
    uint8_t b = ( color & 0x000000ff ) >> 0;
    y = uint8_t(0.299 * r + 0.587 * g + 0.114 * b);
    u = uint8_t(0.5 * r - 0.4187 * g - 0.0813 * b + 128);
    v = uint8_t(-0.1687 * r - 0.3313 * g + 0.5 * b + 128);
}
void SsGraphic::convert_color_to_index(uint32_t color, size_t length, uint8_t &index)
{
    unsigned int min_diff = 0xffffffff;
    unsigned int curr_diff = 0;
    uint8_t da, dr, dg, db;
    uint8_t a, r, g, b;
    convert_color_to_argb(color, da, dr, dg, db);
    length = std::min(sizeof(this->au32PaletteTbl)/sizeof(this->au32PaletteTbl[0]), (size_t)length);
    for (size_t i = 0; i < length; ++i) {
        convert_color_to_argb(this->au32PaletteTbl[i], a, r, g, b);
        curr_diff = std::abs(int(dr - r)) + std::abs(int(dg - g)) + std::abs(int(db - b));
        if (curr_diff < min_diff) {
            min_diff = curr_diff;
            index = i;
        }
    }
}
int SsGraphic::convert_color_to_pixel(int color)
{
    return color;
}
void SsGraphic::fill_rect(int color, ColorMode_e mode, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    int pixel = color;
    if(E_COLOR_MODE_ARGB8888 == mode)
    {
        pixel = this->convert_color_to_pixel(color);
    }
    for (uint32_t row = y; row < y + height; row++) {
        for (uint32_t col = x; col < x + width; col++) {
            this->set_pixel(pixel, col, row);
        }
    }
}
void SsGraphic::fill_ellipse(int color, ColorMode_e mode, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    int origin_x = x + ( width >> 1 );
    int origin_y = y + ( height >> 1 );
    int width_h = width >> 1;
    int height_h = height >> 1;

    long long hh = height_h * height_h;
    long long ww = width_h * width_h;
    long long hhww = hh * ww;

    int x0 = width_h;
    int dx = 0;

    int pixel = color;
    if(E_COLOR_MODE_ARGB8888 == mode)
    {
        pixel = this->convert_color_to_pixel(color);
    }
    for (int x = -width_h; x <= width_h; x++)
        this->set_pixel(pixel, origin_x + x, origin_y);

    for (int y = 1; y <= height_h; y++)
    {
        int x1 = x0 - (dx - 1);
        for ( ; x1 > 0; x1--)
            if (x1*x1*hh + y*y*ww <= hhww)
                break;
        dx = x0 - x1;
        x0 = x1;

        for (int x = -x0; x <= x0; x++)
        {
            this->set_pixel(pixel, origin_x + x, origin_y - y);
            this->set_pixel(pixel, origin_x + x, origin_y + y);
        }
    }
}
void SsGraphic::draw_line(int color, ColorMode_e mode, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int ux = dx > 0 ? 1 : -1;
    int uy = dy > 0 ? 1 : -1;
    int dx2 = dx << 1;
    int dy2 = dy << 1;

    int pixel = color;
    if(E_COLOR_MODE_ARGB8888 == mode)
    {
        pixel = this->convert_color_to_pixel(color);
    }
    if (std::abs(dx) > std::abs(dy)) {
        int e = -dx * ux;
        int x = x0;
        int y = y0;
        for (x = x0; ux * x < int(x1); x += ux) {
            this->set_pixel(pixel, x, y);
            e += dy2 * uy;
            if (e > 0) {
                y += uy;
                e -= dx2 * ux;
            }
        }
    } else {
        int e = -dy * uy;
        int x = x0;
        int y = y0;
        for (y = y0; uy * y < int(y1); y += uy) {
            this->set_pixel(pixel, x, y);
            e += dx2 * ux;
            if (e > 0) {
                x += ux;
                e -= dy2 * uy;
            }
        }
    }
}
void SsGraphic::draw_bitmap(int color, ColorMode_e mode, uint32_t x, uint32_t y, uint8_t *bitmap, uint32_t width, uint32_t height)
{
    int pixel = color;
    if(E_COLOR_MODE_ARGB8888 == mode)
    {
        pixel = this->convert_color_to_pixel(color);
    }
    for (uint32_t i = 0; i < height; ++i) {
        for (uint32_t j = 0; j < width; ++j) {
            if (*(bitmap + i * width + j) & 0x80) {
                this->set_pixel(pixel, j + x, i + y);
            }
        }
    }
}
void SsGraphic::draw_font(int color, ColorMode_e mode, uint32_t x, uint32_t y, std::string str, uint32_t font_size)
{
    unsigned char * bitmap = new unsigned char[this->u32Width * font_size];

    memset(bitmap, 0, this->u32Width * font_size);
    Font::GetInstance()->Str2Bitmap(str, bitmap, this->u32Width, font_size);
    this->draw_bitmap(color, mode, x, y, bitmap, this->u32Width, font_size);
    delete [] bitmap;
}
void SsGraphic::cal_color_key(uint8_t &r, uint8_t &g, uint8_t &b)
{
    this->cal_color_key(0xff000000 | (r << 16) | (g << 8) | b, r, g, b);
}

// YUV420SP_SsGraphic ////////////////////////////////////////////////////////

void YUV420SP_SsGraphic::copy_from(const void **ppAddr)
{
    char *src = (char*)ppAddr[0];
    std::copy(src, src + (this->u32Width * this->u32Height), this->data);
    src = (char*)ppAddr[1];
    std::copy(src + this->u32Width * this->u32Height, src + this->u32Width * this->u32Height / 2, this->data + this->u32Width * u32Height);
}
void YUV420SP_SsGraphic::copy_to(const void **ppAddr)
{
    char *src = (char*)ppAddr[0];
    std::copy(this->data, this->data + this->u32Width * this->u32Height, src);
    src = (char*)ppAddr[1];
    std::copy(this->data + this->u32Width * this->u32Height, this->data + this->u32Width * this->u32Height * 3 / 2, src);
}
void YUV420SP_SsGraphic::set_pixel(int pixel, uint32_t x, uint32_t y)
{
    uint8_t cy, cu, cv;
    convert_color_to_yuv(pixel, cy, cu, cv);
    this->_set_pixel<uint8_t>(this->data, cy, x, y);
    this->_set_pixel<uint16_t>(this->data + this->u32Width * this->u32Height, uint16_t((cu << 8) | cv), x, y, 2, 2);
}
void YUV420SP_SsGraphic::fill_rect(int color, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    uint8_t cy, cu, cv;
    convert_color_to_yuv(color, cy, cu, cv);
    this->_fill_rect<uint8_t>(this->data, cy, x, y, w, h);
    this->_fill_rect<uint16_t>(this->data + this->u32Width * this->u32Height,
            uint16_t((cu << 8) | cv), x, y, w, h, 2, 2);
}

//////// YUV422_SsGraphic /////////////////////////////////
int YUV422_SsGraphic::convert_color_to_pixel(int color)
{
    uint8_t cy, cu, cv;
    convert_color_to_yuv(color, cy, cu, cv);
    return cu << 24 | cy << 16 | cv << 8 | cy;
}
void YUV422_SsGraphic::set_pixel(int pixel, uint32_t x, uint32_t y)
{
    this->_set_pixel<uint32_t>(this->data, uint32_t(pixel), x, y, 2, 1);
}
void YUV422_SsGraphic::fill_rect(int color, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    uint8_t cy, cu, cv;
    convert_color_to_yuv(color, cy, cu, cv);
    this->_fill_rect<uint32_t>(this->data, uint32_t(cu << 24 | cy << 16 | cv << 8 | cy), x, y, w, h, 2);
}

//////// ARGB8888_SsGraphic /////////////////////////////////
void ARGB8888_SsGraphic::set_pixel(int pixel, uint32_t x, uint32_t y)
{
    this->_set_pixel<uint32_t>(this->data, uint32_t(pixel), x, y);
}
void ARGB8888_SsGraphic::fill_rect(int color, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    this->_fill_rect<uint32_t>(this->data, uint32_t(color), x, y, w, h);
}
void ARGB8888_SsGraphic::cal_color_key(int color, uint8_t &r, uint8_t &g, uint8_t &b)
{
    uint8_t a = 0;
    convert_color_to_argb(color, a, r, g, b);
}

//////// RGB565_SsGraphic /////////////////////////////////
int RGB565_SsGraphic::convert_color_to_pixel(int color)
{
    uint8_t a, r, g, b;
    convert_color_to_argb(color, a, r, g, b);
    r = r >> 3;
    g = g >> 2;
    b = b >> 3;
    return r << 11 | g << 5 | b;
}
void RGB565_SsGraphic::set_pixel(int pixel, uint32_t x, uint32_t y)
{
    this->_set_pixel<uint16_t>(this->data, uint16_t(pixel), x, y);
}
void RGB565_SsGraphic::fill_rect(int color, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    this->_fill_rect<uint16_t>(this->data, uint16_t(convert_color_to_pixel(color)), x, y, w, h);
}
void RGB565_SsGraphic::cal_color_key(int color, uint8_t &r, uint8_t &g, uint8_t &b)
{
    uint8_t a = 0;
    convert_color_to_argb(color, a, r, g, b);
    r = ( r & 0xf8 ) | ( (r >> 3) & 0x07 );
    g = ( g & 0xfc ) | ( (g >> 2) & 0x03 );
    b = ( b & 0xf8 ) | ( (b >> 3) & 0x07 );
}

//////// ARGB1555_SsGraphic /////////////////////////////////
int ARGB1555_SsGraphic::convert_color_to_pixel(int color)
{
    uint8_t a, r, g, b;
    convert_color_to_argb(color, a, r, g, b);
    a = a >> 7;
    r = r >> 3;
    g = g >> 3;
    b = b >> 3;
    return a << 15 | r << 10 | g << 5 | b;
}
void ARGB1555_SsGraphic::set_pixel(int pixel, uint32_t x, uint32_t y)
{
    this->_set_pixel<uint16_t>(this->data, uint16_t(pixel), x, y);
}
void ARGB1555_SsGraphic::fill_rect(int color, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    this->_fill_rect<uint16_t>(this->data, uint16_t(convert_color_to_pixel(color)), x, y, w, h);
}
void ARGB1555_SsGraphic::cal_color_key(int color, uint8_t &r, uint8_t &g, uint8_t &b)
{
    uint8_t a = 0;
    convert_color_to_argb(color, a, r, g, b);
    r = ( r & 0xf8 ) | ( (r >> 3) & 0x07 );
    g = ( g & 0xf8 ) | ( (g >> 3) & 0x07 );
    b = ( b & 0xf8 ) | ( (b >> 3) & 0x07 );
}

//////// ARGB4444_SsGraphic /////////////////////////////////
int ARGB4444_SsGraphic::convert_color_to_pixel(int color)
{
    uint8_t a, r, g, b;
    convert_color_to_argb(color, a, r, g, b);
    a = a >> 4;
    r = r >> 4;
    g = g >> 4;
    b = b >> 4;
    return a << 12 | r << 8 | g << 4 | b;
}
void ARGB4444_SsGraphic::set_pixel(int pixel, uint32_t x, uint32_t y)
{
    this->_set_pixel<uint16_t>(this->data, uint16_t(pixel), x, y);
}
void ARGB4444_SsGraphic::fill_rect(int color, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    this->_fill_rect<uint16_t>(this->data, uint16_t(convert_color_to_pixel(color)), x, y, w, h);
}
void ARGB4444_SsGraphic::cal_color_key(int color, uint8_t &r, uint8_t &g, uint8_t &b)
{
    uint8_t a = 0;
    convert_color_to_argb(color, a, r, g, b);
    r = ( r & 0xf0 ) | ( (r >> 4) & 0x0f );
    g = ( g & 0xf0 ) | ( (g >> 4) & 0x0f );
    b = ( b & 0xf0 ) | ( (b >> 4) & 0x0f );
}

//////// I2_SsGraphic /////////////////////////////////
int I2_SsGraphic::convert_color_to_pixel(int color)
{
    uint8_t index;
    convert_color_to_index(color, 4, index);
    return index;
}
void I2_SsGraphic::load_palette(uint32_t *pu32PaletteTbl, uint32_t u32PaletteNum)
{
    if(u32PaletteNum < 4)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Palette is not enough!");
        return;
    }
    std::copy(pu32PaletteTbl, pu32PaletteTbl + u32PaletteNum, this->au32PaletteTbl);
    this->u32PaletteNum = u32PaletteNum;
}
void I2_SsGraphic::set_pixel(int pixel, uint32_t x, uint32_t y)
{
    uint8_t index, val;
    uint8_t shift = 0;
    if (this->data == nullptr || x >= this->u32Width || y >= this->u32Height) {
        return;
    }
    index = (uint8_t)pixel;
    shift = ( x % 4 ) << 1;
    val = *( this->data + y * int( this->u32Width / 4 ) + int(x / 4) ) & ~( 0x3 << shift );
    val |= index << shift;
    this->_set_pixel<uint8_t>(this->data, val, x, y, 4);
}
void I2_SsGraphic::cal_color_key(int color, uint8_t &r, uint8_t &g, uint8_t &b)
{
    uint8_t index, a;
    convert_color_to_index(color, 4, index);
    convert_color_to_argb(this->au32PaletteTbl[index], a, r, g, b);
}

//////// I4_SsGraphic /////////////////////////////////
int I4_SsGraphic::convert_color_to_pixel(int color)
{
    uint8_t index;
    convert_color_to_index(color, 16, index);
    return index;
}
void I4_SsGraphic::load_palette(uint32_t *pu32PaletteTbl, uint32_t u32PaletteNum)
{
    if(u32PaletteNum < 16)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Palette is not enough!");
        return;
    }
    std::copy(pu32PaletteTbl, pu32PaletteTbl + u32PaletteNum, this->au32PaletteTbl);
    this->u32PaletteNum = u32PaletteNum;
}
void I4_SsGraphic::set_pixel(int pixel, uint32_t x, uint32_t y)
{
    uint8_t index, val;
    uint8_t shift = 0;
    index = (uint8_t)pixel;
    if (this->data == nullptr || x >= this->u32Width || y >= this->u32Height) {
        return;
    }
    shift = ( x % 2 ) << 2;
    val = *( this->data + y * int( this->u32Width / 2 ) + int(x / 2) ) & ~( 0xf << shift );
    val |= index << shift;
    this->_set_pixel<uint8_t>(this->data, val, x, y, 2);
}
void I4_SsGraphic::cal_color_key(int color, uint8_t &r, uint8_t &g, uint8_t &b)
{
    uint8_t index, a;
    convert_color_to_index(color, 16, index);
    convert_color_to_argb(this->au32PaletteTbl[index], a, r, g, b);
}

//////// I8_SsGraphic /////////////////////////////////
int I8_SsGraphic::convert_color_to_pixel(int color)
{
    uint8_t index;
    convert_color_to_index(color, 256, index);
    return index;
}
void I8_SsGraphic::load_palette(uint32_t *pu32PaletteTbl, uint32_t u32PaletteNum)
{
    if(u32PaletteNum < 256)
    {
        std::cout << "palette is not enough" << std::endl;
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Palette is not enough!");
        return;
    }
    std::copy(pu32PaletteTbl, pu32PaletteTbl + u32PaletteNum, this->au32PaletteTbl);
    this->u32PaletteNum = u32PaletteNum;
}
void I8_SsGraphic::set_pixel(int pixel, uint32_t x, uint32_t y)
{
    uint8_t index = (uint8_t)pixel;
    this->_set_pixel<uint8_t>(this->data, uint8_t(index), x, y);
}
void I8_SsGraphic::fill_rect(int color, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    this->_fill_rect<uint8_t>(this->data, uint8_t(convert_color_to_pixel(color)), x, y, w, h);
}
void I8_SsGraphic::cal_color_key(int color, uint8_t &r, uint8_t &g, uint8_t &b)
{
    uint8_t index, a;
    convert_color_to_index(color, 256, index);
    convert_color_to_argb(this->au32PaletteTbl[index], a, r, g, b);
}

//////// Stream_SsGraphic /////////////////////////////////
void Stream_SsGraphic::set_pixel(int pixel, uint32_t x, uint32_t y)
{
}
void Stream_SsGraphic::fill_rect(int color, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
}
void Stream_SsGraphic::copy_from(const void **ppAddr, const uint32_t *psize, uint32_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint8_t *src = (uint8_t *)ppAddr[i];
        std::copy(src, src + psize[i], (char*)this->data + this->used_size);
        this->used_size += psize[i];
    }
}
void Stream_SsGraphic::save_file(std::string filepath)
{
    std::ofstream fout(filepath, std::ios::out & std::ios::binary);
    if (!fout.is_open()) {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Missing file!\n");
        return ;
    }
    fout.write(this->data, this->used_size);
    fout.close();
}
void Stream_SsGraphic::load_file(std::string filepath)
{
    std::ifstream fin(filepath, std::ios::in & std::ios::binary);
    if (!fin.is_open()) {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Missing file!\n");
        return ;
    }
    fin.read(this->data, this->used_size);
    fin.close();
}
