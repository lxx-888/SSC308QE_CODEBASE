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

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <cstring>
#include "ss_graph.h"

#define SWAP(a, b)             \
    do                         \
    {                          \
        typeof(a) temp = (a);  \
        (a)            = (b);  \
        (b)            = temp; \
    } while (0)

static void fill_2bpp(void *first, size_t n, size_t offset, unsigned int val)
{
    uint8_t *dst = (uint8_t *)first;
    dst += (offset >> 2);
    val &= 0x3;
    val = (val << 6) | (val << 4) | (val << 2) | val;

    static const struct {
        unsigned char mask0;
        unsigned char mask1;
    } shift_map[] = {
        [0] = {0xff, 0x00},
        [1] = {0x03, 0xfc},
        [2] = {0x0f, 0xf0},
        [3] = {0x3f, 0xc0},
    };

    uint8_t bit_shift = offset & 0x3;
    if (bit_shift)
    {
        if (4 - bit_shift > (int)n)
        {
            *dst = (*dst & (shift_map[bit_shift].mask0 | shift_map[bit_shift + n].mask1))
                   | (val & (shift_map[bit_shift].mask1 ^ shift_map[bit_shift + n].mask1));
            return;
        }
        *dst = (*dst & shift_map[bit_shift].mask0) | (val & shift_map[bit_shift].mask1);
        ++dst;
        n -= 4 - bit_shift;
    }

    size_t bytes = n >> 2;
    if (bytes)
    {
        memset(dst, val, bytes);
        dst += bytes;
    }

    bit_shift = n & 0x3;
    if (bit_shift)
    {
        *dst = (*dst & shift_map[bit_shift].mask1) | (val & shift_map[bit_shift].mask0);
    }
}
static void fill_4bpp(void *first, size_t n, size_t offset, unsigned int val)
{
    if (!n)
    {
        return;
    }
    uint8_t *dst = (uint8_t *)first;
    dst += (offset >> 1);
    val &= 0xf;
    val = (val << 4) | val;
    if (offset & 0x1)
    {
        *dst = (*dst & 0xf) | (val & 0xf0);
        ++dst;
        --n;
    }
    size_t bytes = n >> 1;
    if (bytes)
    {
        memset(dst, val, bytes);
        dst += bytes;
    }
    if (n & 0x1)
    {
        *dst = (*dst & 0xf0) | (val & 0xf);
    }
}
static void fill_8bpp(void *first, size_t n, size_t offset, unsigned int val)
{
    uint8_t *dst = (uint8_t *)first;
    dst += offset;
    memset(dst, val, n);
}
static void fill_16bpp(void *first, size_t n, size_t offset, unsigned int val)
{
    uint16_t *dst = (uint16_t *)first;
    dst += offset;
    for (size_t i = 0; i < n; ++i)
    {
        dst[i] = val;
    }
}
static void fill_32bpp(void *first, size_t n, size_t offset, unsigned int val)
{
    uint32_t *dst = (uint32_t *)first;
    dst += offset;
    for (size_t i = 0; i < n; ++i)
    {
        dst[i] = val;
    }
}

static const std::unordered_map<unsigned int, SS_Graph::FillBpp> g_map_fill = {
    {2, fill_2bpp}, {4, fill_4bpp}, {8, fill_8bpp}, {16, fill_16bpp}, {32, fill_32bpp},
};

SS_Graph::Graph::Graph(const CanvasDesc &desc, const Canvas &canvas) : desc(desc), canvas(canvas)
{
    this->planeNum = 0;
    for (unsigned int i = 0; i < SS_GRAPH_CANVAS_PLANE_MAX_NUM; ++i)
    {
        if (!canvas.data[i])
        {
            this->planeNum = i;
            break;
        }
        auto it = g_map_fill.find(desc.plane[i].bpp);
        if (it != g_map_fill.end())
        {
            this->fillbpp[i] = it->second;
        }
        else
        {
            this->fillbpp[i] = nullptr;
        }
    }
}

SS_Graph::Graph::~Graph() {}

void SS_Graph::Graph::FillColor(const unsigned int plane_idx, const Color &c)
{
    for (size_t i = 0; i < this->canvas.height / this->desc.plane[plane_idx].v_sample; ++i)
    {
        this->fillbpp[plane_idx](this->canvas.data[plane_idx] + this->canvas.stride[plane_idx] * i,
                                 this->canvas.width / this->desc.plane[plane_idx].h_sample, 0, c);
    }
}
void SS_Graph::Graph::SetPixel(const unsigned int plane_idx, const Color &c, const AbsCoord &x, const AbsCoord &y)
{
    if (x >= this->canvas.width)
    {
        return;
    }
    if (y >= this->canvas.height)
    {
        return;
    }
    this->fillbpp[plane_idx](this->canvas.data[plane_idx]
                                 + this->canvas.stride[plane_idx] * y / this->desc.plane[plane_idx].v_sample,
                             1, x / this->desc.plane[plane_idx].h_sample, c);
}
void SS_Graph::Graph::FillRect(const unsigned int plane_idx, const Color &c, const AbsCoord &x, const AbsCoord &y,
                               const AbsCoord &w, const AbsCoord &h)
{
    if (x >= this->canvas.width)
    {
        return;
    }
    if (y >= this->canvas.height)
    {
        return;
    }
    AbsCoord x0 = x;
    AbsCoord y0 = y;
    AbsCoord x1 = x + w;
    AbsCoord y1 = y + h;
    if (x1 >= this->canvas.width)
    {
        x1 = this->canvas.width - 1;
    }
    if (y1 >= this->canvas.height)
    {
        y1 = this->canvas.height - 1;
    }
    x0 /= this->desc.plane[plane_idx].h_sample;
    x1 /= this->desc.plane[plane_idx].h_sample;
    y0 /= this->desc.plane[plane_idx].v_sample;
    y1 /= this->desc.plane[plane_idx].v_sample;
    for (size_t i = y0; i <= y1; ++i)
    {
        this->fillbpp[plane_idx](this->canvas.data[plane_idx] + i * this->canvas.stride[plane_idx], x1 - x0, x0, c);
    }
}
void SS_Graph::Graph::FillEllipse(const unsigned int plane_idx, const Color &c, const AbsCoord &inx,
                                  const AbsCoord &iny, const AbsCoord &inw, const AbsCoord &inh)
{
    if (inx >= this->canvas.width || iny >= this->canvas.height)
        return;
    unsigned int real_w = inx + inw >= this->canvas.width ? this->canvas.width - inx - 1 : inw;
    unsigned int real_h = iny + inh >= this->canvas.height ? this->canvas.height - iny - 1 : inh;

    real_w /= this->desc.plane[plane_idx].h_sample;
    real_h /= this->desc.plane[plane_idx].v_sample;

    int width_h  = real_w >> 1;
    int height_h = real_h >> 1;
    int origin_x = (inx / this->desc.plane[plane_idx].h_sample) + width_h;
    int origin_y = (iny / this->desc.plane[plane_idx].v_sample) + height_h;

    long long hh = height_h * height_h;
    long long ww = width_h * width_h;
    long long hhww = hh * ww;

    int x0 = width_h;
    int dx = 0;

    this->fillbpp[plane_idx](this->canvas.data[plane_idx] + origin_y * this->canvas.stride[plane_idx], x0 << 1,
                             origin_x - x0, c);

    for (int y = 1; y <= height_h; y++)
    {
        int x1 = x0 - (dx - 1);
        for ( ; x1 > 0; x1--)
            if (x1*x1*hh + y*y*ww <= hhww)
                break;
        dx = x0 - x1;
        x0 = x1;

        this->fillbpp[plane_idx](this->canvas.data[plane_idx] + (origin_y + y) * this->canvas.stride[plane_idx],
                                 x0 << 1, origin_x - x0, c);
        this->fillbpp[plane_idx](this->canvas.data[plane_idx] + (origin_y - y) * this->canvas.stride[plane_idx],
                                 x0 << 1, origin_x - x0, c);
    }
}
void SS_Graph::Graph::DrawFrame(const unsigned int plane_idx, const Color &c, const AbsCoord &x, const AbsCoord &y,
                                const AbsCoord &w, const AbsCoord &h, const AbsCoord &thickness)
{
    if (x >= this->canvas.width)
    {
        return;
    }
    if (y >= this->canvas.height)
    {
        return;
    }
    AbsCoord x0 = x;
    AbsCoord y0 = y;
    AbsCoord x1 = x + w;
    AbsCoord y1 = y + h;
    if (x1 >= this->canvas.width)
    {
        x1 = this->canvas.width - 1;
    }
    if (y1 >= this->canvas.height)
    {
        y1 = this->canvas.height - 1;
    }
    AbsCoord line_h = thickness > (w >> 1) ? (w >> 1) : thickness;
    AbsCoord line_w = thickness > (h >> 1) ? (h >> 1) : thickness;
    line_w /= this->desc.plane[plane_idx].h_sample;
    line_h /= this->desc.plane[plane_idx].v_sample;
    x0 /= this->desc.plane[plane_idx].h_sample;
    y0 /= this->desc.plane[plane_idx].v_sample;
    x1 /= this->desc.plane[plane_idx].h_sample;
    y1 /= this->desc.plane[plane_idx].v_sample;
    for (size_t i = 0; i < line_h; ++i)
    {
        this->fillbpp[plane_idx](this->canvas.data[plane_idx] + (y + i) * this->canvas.stride[plane_idx], x1 - x0, x0,
                                 c);
        this->fillbpp[plane_idx](this->canvas.data[plane_idx] + (y1 - i) * this->canvas.stride[plane_idx], x1 - x0, x0,
                                 c);
    }
    for (size_t i = y + line_h; i <= y1 - line_h; ++i)
    {
        this->fillbpp[plane_idx](this->canvas.data[plane_idx] + i * this->canvas.stride[plane_idx], line_w, x0, c);
        this->fillbpp[plane_idx](this->canvas.data[plane_idx] + i * this->canvas.stride[plane_idx], line_w, x1 - line_w,
                                 c);
    }
}
void SS_Graph::Graph::DrawLine(const unsigned int plane_idx, const Color &c, const AbsCoord &x0, const AbsCoord &y0,
                               const AbsCoord &x1, const AbsCoord &y1, const AbsCoord &thickness)
{
    int real_x0 = x0 >= this->canvas.width ? this->canvas.width - 1 : x0;
    int real_x1 = x1 >= this->canvas.width ? this->canvas.width - 1 : x1;
    int real_y0 = y0 >= this->canvas.height ? this->canvas.height - 1 : y0;
    int real_y1 = y1 >= this->canvas.height ? this->canvas.height - 1 : y1;

    real_x0 = real_x0 / this->desc.plane[plane_idx].h_sample;
    real_x1 = real_x1 / this->desc.plane[plane_idx].h_sample;
    real_y0 = real_y0 / this->desc.plane[plane_idx].v_sample;
    real_y1 = real_y1 / this->desc.plane[plane_idx].v_sample;

    int dx = real_x1 - real_x0;
    int dy = real_y1 - real_y0;

    if (std::abs(dx) > std::abs(dy))
    {
        if (dx < 0)
        {
            SWAP(real_x0, real_x1);
            SWAP(real_y0, real_y1);
            dx = -dx;
            dy = -dy;
        }
        int e   = -dx;
        int x   = real_x0;
        int y   = real_y0;
        int dx2 = dx << 1;
        int dy2 = dy << 1;
        if (dy > 0)
        {
            for (x = real_x0; x < real_x1; ++x)
            {
                for (size_t i = 0; i < thickness; ++i)
                {
                    if (y + i >= this->canvas.height / this->desc.plane[plane_idx].v_sample)
                    {
                        break;
                    }
                    this->fillbpp[plane_idx](this->canvas.data[plane_idx] + (y + i) * this->canvas.stride[plane_idx], 1,
                                             x, c);
                }
                e += dy2;
                if (e > 0)
                {
                    ++y;
                    e -= dx2;
                }
            }
        }
        else
        {
            for (x = real_x0; x < real_x1; ++x)
            {
                for (size_t i = 0; i < thickness; ++i)
                {
                    if (y + i >= this->canvas.height / this->desc.plane[plane_idx].v_sample)
                    {
                        break;
                    }
                    this->fillbpp[plane_idx](this->canvas.data[plane_idx] + (y + i) * this->canvas.stride[plane_idx], 1,
                                             x, c);
                }
                e -= dy2;
                if (e > 0)
                {
                    --y;
                    e -= dx2;
                }
            }
        }
    }
    else
    {
        if (dy < 0)
        {
            SWAP(real_x0, real_x1);
            SWAP(real_y0, real_y1);
            dx = -dx;
            dy = -dy;
        }
        int e   = -dy;
        int x   = real_x0;
        int y   = real_y0;
        int dx2 = dx << 1;
        int dy2 = dy << 1;
        if (dx > 0)
        {
            for (y = real_y0; y < real_y1; ++y)
            {
                int n = x + thickness >= this->canvas.width / this->desc.plane[plane_idx].h_sample
                            ? this->canvas.width / this->desc.plane[plane_idx].h_sample - x
                            : thickness;
                this->fillbpp[plane_idx](this->canvas.data[plane_idx] + y * this->canvas.stride[plane_idx], n, x, c);
                e += dx2;
                if (e > 0)
                {
                    ++x;
                    e -= dy2;
                }
            }
        }
        else
        {
            for (y = real_y0; y < real_y1; ++y)
            {
                int n = x + thickness >= this->canvas.width / this->desc.plane[plane_idx].h_sample
                            ? this->canvas.width / this->desc.plane[plane_idx].h_sample - x
                            : thickness;
                this->fillbpp[plane_idx](this->canvas.data[plane_idx] + y * this->canvas.stride[plane_idx], n, x, c);
                e -= dx2;
                if (e > 0)
                {
                    --x;
                    e -= dy2;
                }
            }
        }
    }
}
void SS_Graph::Graph::DrawBitmap(const unsigned int plane_idx, const Color &c, const AbsCoord &x, const AbsCoord &y,
                                 const char *bitmap, const AbsCoord &w, const AbsCoord &h)
{
    unsigned int scan_w = w > this->canvas.width - x ? this->canvas.width - x : w;
    unsigned int scan_h = h > this->canvas.height - y ? this->canvas.height - y : h;
    for (unsigned int i = 0, line = y; i < scan_h; i += this->desc.plane[plane_idx].v_sample, ++line)
    {
        for (unsigned int j = 0, offset = x; j < scan_w; j += this->desc.plane[plane_idx].h_sample, ++offset)
        {
            if (*(bitmap + i * w + j))
            {
                this->fillbpp[plane_idx](this->canvas.data[plane_idx] + line * this->canvas.stride[plane_idx], 1, offset, c);
            }
        }
    }
}

void SS_Graph::Graph::DrawDotMatrix(const unsigned int plane_idx, const Color &c, const char *dotMatrix, const AbsCoord &w,
                                    const AbsCoord &h, const AbsCoord &size)
{
    AbsCoord row, col;
    char *   currLine;
    AbsCoord leftPadding, topPadding;
    AbsCoord blockW, blockH;
    AbsCoord realSize = size;
    if (!dotMatrix || !size || w > this->canvas.width || h > this->canvas.height)
    {
        return;
    }
    blockW      = this->canvas.width / w;
    blockH      = this->canvas.height / h;
    realSize    = realSize > blockW ? blockW : realSize;
    realSize    = realSize > blockH ? blockH : realSize;
    leftPadding = (blockW - realSize) >> 1;
    topPadding  = (blockH - realSize) >> 1;

    currLine = this->canvas.data[plane_idx] + this->canvas.stride[plane_idx] * topPadding;
    for (row = 0; row < h; ++row)
    {
        char *   firstLine = currLine;
        AbsCoord offset    = leftPadding;
        for (col = 0; col < w; ++col)
        {
            if (dotMatrix[col])
            {
                AbsCoord i;
                currLine = firstLine;
                for (i = 0; i < realSize; ++i)
                {
                    this->fillbpp[plane_idx](currLine, realSize, offset, c);
                    currLine += this->canvas.stride[plane_idx];
                }
            }
            offset += blockW;
        }
        currLine = firstLine + this->canvas.stride[plane_idx] * blockH;
        dotMatrix += w;
    }
}

void SS_Graph::Graph::FillColor(const Color &c)
{
    for (unsigned int i = 0; i < this->planeNum; ++i)
    {
        if (this->fillbpp[i])
        {
            Color real_c = c;
            if (desc.plane[i].color_convert)
            {
                real_c = desc.plane[i].color_convert(c, desc.plane[i].user_data);
            }
            this->FillColor(i, real_c);
        }
    }
}

void SS_Graph::Graph::SetPixel(const Color &c, const AbsCoord &x, const AbsCoord &y)
{
    for (unsigned int i = 0; i < this->planeNum; ++i)
    {
        if (this->fillbpp[i])
        {
            Color real_c = c;
            if (desc.plane[i].color_convert)
            {
                real_c = desc.plane[i].color_convert(c, desc.plane[i].user_data);
            }
            this->SetPixel(i, real_c, x, y);
        }
    }
}

void SS_Graph::Graph::FillRect(const Color &c, const AbsCoord &x, const AbsCoord &y, const AbsCoord &w, const AbsCoord &h)
{
    for (unsigned int i = 0; i < this->planeNum; ++i)
    {
        if (this->fillbpp[i])
        {
            Color real_c = c;
            if (desc.plane[i].color_convert)
            {
                real_c = desc.plane[i].color_convert(c, desc.plane[i].user_data);
            }
            this->FillRect(i, real_c, x, y, w, h);
        }
    }
}
void SS_Graph::Graph::FillEllipse(const Color &c, const AbsCoord &x, const AbsCoord &y, const AbsCoord &w, const AbsCoord &h)
{
    for (unsigned int i = 0; i < this->planeNum; ++i)
    {
        if (this->fillbpp[i])
        {
            Color real_c = c;
            if (desc.plane[i].color_convert)
            {
                real_c = desc.plane[i].color_convert(c, desc.plane[i].user_data);
            }
            this->FillEllipse(i, real_c, x, y, w, h);
        }
    }
}
void SS_Graph::Graph::DrawFrame(const Color &c, const AbsCoord &x, const AbsCoord &y, const AbsCoord &w,
                                const AbsCoord &h, const AbsCoord &thickness)
{
    for (unsigned int i = 0; i < this->planeNum; ++i)
    {
        if (this->fillbpp[i])
        {
            Color real_c = c;
            if (desc.plane[i].color_convert)
            {
                real_c = desc.plane[i].color_convert(c, desc.plane[i].user_data);
            }
            this->DrawFrame(i, real_c, x, y, w, h, thickness);
        }
    }
}
void SS_Graph::Graph::DrawLine(const Color &c, const AbsCoord &x0, const AbsCoord &y0, const AbsCoord &x1,
                               const AbsCoord &y1, const AbsCoord &thickness)
{
    for (unsigned int i = 0; i < this->planeNum; ++i)
    {
        if (this->fillbpp[i])
        {
            Color real_c = c;
            if (desc.plane[i].color_convert)
            {
                real_c = desc.plane[i].color_convert(c, desc.plane[i].user_data);
            }
            this->DrawLine(i, real_c, x0, y0, x1, y1, thickness);
        }
    }
}
void SS_Graph::Graph::DrawBitmap(const Color &c, const AbsCoord &x, const AbsCoord &y, const char *bitmap,
                                 const AbsCoord &w, const AbsCoord &h)
{
    for (unsigned int i = 0; i < this->planeNum; ++i)
    {
        if (this->fillbpp[i])
        {
            Color real_c = c;
            if (desc.plane[i].color_convert)
            {
                real_c = desc.plane[i].color_convert(c, desc.plane[i].user_data);
            }
            this->DrawBitmap(i, real_c, x, y, bitmap, w, h);
        }
    }
}
void SS_Graph::Graph::DrawDotMatrix(const Color &c, const char *dotMatrix, const AbsCoord &w, const AbsCoord &h,
                                    const AbsCoord &size)
{
    for (unsigned int i = 0; i < this->planeNum; ++i)
    {
        if (this->fillbpp[i])
        {
            Color real_c = c;
            if (desc.plane[i].color_convert)
            {
                real_c = desc.plane[i].color_convert(c, desc.plane[i].user_data);
            }
            this->DrawDotMatrix(i, real_c, dotMatrix, w, h, size);
        }
    }
}
