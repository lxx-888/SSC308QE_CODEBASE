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

#include "ssos_io.h"
#include "ptree_gl.h"

#define ABS(x) ((x) > 0 ? (x) : -(x));

static void _PTREE_GL_Fill2Bpp(void *first, unsigned int n, unsigned int offset, unsigned int val)
{
    struct
    {
        unsigned char mask0;
        unsigned char mask1;
    } shiftMap[] = {
        [0] = {0xff, 0x00},
        [1] = {0x03, 0xfc},
        [2] = {0x0f, 0xf0},
        [3] = {0x3f, 0xc0},
    };
    unsigned char  bitShift = offset & 0x3;
    unsigned char *dst      = (unsigned char *)first;
    unsigned int   bytes    = n >> 2;

    dst += (offset >> 2);
    val &= 0x3;
    val = (val << 6) | (val << 4) | (val << 2) | val;

    if (bitShift)
    {
        if (4 - bitShift > (int)n)
        {
            *dst = (*dst & (shiftMap[bitShift].mask0 | shiftMap[bitShift + n].mask1))
                   | (val & (shiftMap[bitShift].mask1 ^ shiftMap[bitShift + n].mask1));
            return;
        }
        *dst = (*dst & shiftMap[bitShift].mask0) | (val & shiftMap[bitShift].mask1);
        ++dst;
        n -= 4 - bitShift;
    }

    if (bytes)
    {
        memset(dst, val, bytes);
        dst += bytes;
    }

    bitShift = n & 0x3;
    if (bitShift)
    {
        *dst = (*dst & shiftMap[bitShift].mask1) | (val & shiftMap[bitShift].mask0);
    }
}
static void _PTREE_GL_Fill4Bpp(void *first, unsigned int n, unsigned int offset, unsigned int val)
{
    unsigned char *dst   = (unsigned char *)first;
    unsigned int   bytes = n >> 1;
    dst += (offset >> 1);
    val &= 0xf;
    val = (val << 4) | val;
    if (offset & 0x1)
    {
        *dst = (*dst & 0xf) | (val & 0xf0);
        ++dst;
        --n;
    }
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
static void _PTREE_GL_Fill8Bpp(void *first, unsigned int n, unsigned int offset, unsigned int val)
{
    unsigned char *dst = (unsigned char *)first;
    dst += offset;
    memset(dst, val, n);
}
static void _PTREE_GL_Fill16Bpp(void *first, unsigned int n, unsigned int offset, unsigned int val)
{
    unsigned short *dst = (unsigned short *)first;
    unsigned int    i   = 0;
    dst += offset;
    for (i = 0; i < n; ++i)
    {
        dst[i] = val;
    }
}
static void _PTREE_GL_Fill32Bpp(void *first, unsigned int n, unsigned int offset, unsigned int val)
{
    unsigned int *dst = (unsigned int *)first;
    unsigned int  i   = 0;
    dst += offset;
    for (i = 0; i < n; ++i)
    {
        dst[i] = val;
    }
}

int PTREE_GL_Init(PTREE_GL_Obj_t *gl, const PTREE_GL_Canvas_t *canvas)
{
    if (!gl || !canvas)
    {
        return SSOS_DEF_FAIL;
    }
    gl->canvas = *canvas;
    switch (canvas->bpp)
    {
        case 2:
            gl->fillBpp = _PTREE_GL_Fill2Bpp;
            break;
        case 4:
            gl->fillBpp = _PTREE_GL_Fill4Bpp;
            break;
        case 8:
            gl->fillBpp = _PTREE_GL_Fill8Bpp;
            break;
        case 16:
            gl->fillBpp = _PTREE_GL_Fill16Bpp;
            break;
        case 32:
            gl->fillBpp = _PTREE_GL_Fill32Bpp;
            break;
        default:
            gl->fillBpp = NULL;
            break;
    }
    return SSOS_DEF_OK;
}

void PTREE_GL_FillColor(PTREE_GL_Obj_t *gl, PTREE_GL_Color_t color)
{
    PTREE_GL_Coord_t i;
    char *           dst;
    if (!gl || !gl->canvas.data || !gl->fillBpp)
    {
        return;
    }
    dst = gl->canvas.data;
    for (i = 0; i < gl->canvas.height; ++i)
    {
        gl->fillBpp(dst, gl->canvas.width, 0, color);
        dst += gl->canvas.stride;
    }
}

void PTREE_GL_SetPixel(PTREE_GL_Obj_t *gl, PTREE_GL_Color_t color, PTREE_GL_Coord_t x, PTREE_GL_Coord_t y)
{
    if (!gl || !gl->canvas.data || !gl->fillBpp || x >= gl->canvas.width || y >= gl->canvas.height)
    {
        return;
    }
    gl->fillBpp(gl->canvas.data + y * gl->canvas.stride, 1, x, color);
}

void PTREE_GL_FillRect(PTREE_GL_Obj_t *gl, PTREE_GL_Color_t color, PTREE_GL_Coord_t x, PTREE_GL_Coord_t y,
                       PTREE_GL_Coord_t w, PTREE_GL_Coord_t h)
{
    PTREE_GL_Coord_t i;
    char *           dst;
    if (!gl || !gl->canvas.data || !gl->fillBpp || x >= gl->canvas.width || y >= gl->canvas.height)
    {
        return;
    }
    if (x + w >= gl->canvas.width)
    {
        w = gl->canvas.width - x - 1;
    }
    if (y + h >= gl->canvas.height)
    {
        h = gl->canvas.height - y - 1;
    }
    dst = gl->canvas.data + gl->canvas.stride * y;
    for (i = 0; i < h; ++i)
    {
        gl->fillBpp(dst, w, x, color);
        dst += gl->canvas.stride;
    }
}

void PTREE_GL_DrawFrame(PTREE_GL_Obj_t *gl, PTREE_GL_Color_t color, PTREE_GL_Coord_t x, PTREE_GL_Coord_t y,
                        PTREE_GL_Coord_t w, PTREE_GL_Coord_t h, PTREE_GL_Coord_t thickness)
{
    PTREE_GL_Coord_t i;
    PTREE_GL_Coord_t lineW, lineH;
    char *           dst;
    if (!gl || !gl->canvas.data || !gl->fillBpp || x >= gl->canvas.width || y >= gl->canvas.height)
    {
        return;
    }
    if (x + w >= gl->canvas.width)
    {
        w = gl->canvas.width - x - 1;
    }
    if (y + h >= gl->canvas.height)
    {
        h = gl->canvas.height - y - 1;
    }
    lineW = w >> 1;
    lineH = h >> 1;
    if (thickness < lineW)
    {
        lineW = thickness;
    }
    if (thickness < lineH)
    {
        lineH = thickness;
    }
    dst = gl->canvas.data + gl->canvas.stride * y;
    for (i = 0; i < lineH; ++i)
    {
        gl->fillBpp(dst, w, x, color);
        dst += gl->canvas.stride;
    }
    h = h - lineH - lineH;
    for (i = 0; i < h; ++i)
    {
        gl->fillBpp(dst, lineW, x, color);
        gl->fillBpp(dst, lineW, x + w - lineW, color);
        dst += gl->canvas.stride;
    }
    for (i = 0; i < lineH; ++i)
    {
        gl->fillBpp(dst, w, x, color);
        dst += gl->canvas.stride;
    }
}

void PTREE_GL_DrawLine(PTREE_GL_Obj_t *gl, PTREE_GL_Color_t color, PTREE_GL_Coord_t x0, PTREE_GL_Coord_t y0,
                       PTREE_GL_Coord_t x1, PTREE_GL_Coord_t y1, PTREE_GL_Coord_t thickness)
{
    int   dx, dy;
    int   sx, sy;
    char *currLine;

    x0 = x0 >= gl->canvas.width ? gl->canvas.width - 1 : x0;
    x1 = x1 >= gl->canvas.width ? gl->canvas.width - 1 : x1;
    y0 = y0 >= gl->canvas.height ? gl->canvas.height - 1 : y0;
    y1 = y1 >= gl->canvas.height ? gl->canvas.height - 1 : y1;

    dx = x1 - x0;
    dy = y1 - y0;
    sx = dx > 0 ? 1 : -1;
    sy = dy > 0 ? 1 : -1;
    dx = ABS(dx);
    dy = ABS(dy);

    if (dx > dy)
    {
        int dx2  = dx << 1;
        int dy2  = dy << 1;
        int err  = dx;
        int i    = 0;
        currLine = gl->canvas.data + gl->canvas.stride * y0;
        while (x0 != x1)
        {
            char *firstLine = currLine;
            for (i = 0; i < thickness; ++i)
            {
                if (y0 + i >= gl->canvas.height)
                {
                    break;
                }
                gl->fillBpp(currLine, 1, x0, color);
                currLine += gl->canvas.stride;
            }
            currLine = firstLine;
            err -= dy2;
            if (err < 0)
            {
                y0 += sy;
                currLine += (int)gl->canvas.stride * sy;
                err += dx2;
            }
            x0 += sx;
        }
        for (i = 0; i < thickness; ++i)
        {
            if (y0 + i >= gl->canvas.height)
            {
                break;
            }
            gl->fillBpp(currLine, 1, x0, color);
        }
    }
    else
    {
        int dx2  = dx << 1;
        int dy2  = dy << 1;
        int err  = dy;
        currLine = gl->canvas.data + gl->canvas.stride * y0;
        while (y0 != y1)
        {
            gl->fillBpp(currLine, x0 + thickness >= gl->canvas.width ? gl->canvas.width - x0 : thickness, x0, color);
            err -= dx2;
            if (err < 0)
            {
                x0 += sx;
                err += dy2;
            }
            y0 += sy;
            currLine += (int)gl->canvas.stride * sy;
        }
        gl->fillBpp(currLine, x0 + thickness >= gl->canvas.width ? gl->canvas.width - x0 : thickness, x0, color);
    }
}

void PTREE_GL_DrawBitmap(PTREE_GL_Obj_t *gl, PTREE_GL_Color_t color, PTREE_GL_Coord_t x, PTREE_GL_Coord_t y,
                         const char *bitmap, PTREE_GL_Coord_t w, PTREE_GL_Coord_t h, PTREE_GL_Coord_t size)
{
    PTREE_GL_Coord_t row, col;
    PTREE_GL_Coord_t bitmapStride;
    char *           currLine;
    if (!gl || !gl->canvas.data || !gl->fillBpp || !bitmap || !size)
    {
        return;
    }
    bitmapStride = w;
    if (x + w * size >= gl->canvas.width)
    {
        w = (gl->canvas.width - x - 1) / size;
    }
    if (y + h * size >= gl->canvas.height)
    {
        h = (gl->canvas.height - y - 1) / size;
    }
    currLine = gl->canvas.data;
    for (row = 0; row < h; ++row)
    {
        char *           firstLine = currLine;
        PTREE_GL_Coord_t offset    = 0;
        for (col = 0; col < w; ++col)
        {
            if (bitmap[col])
            {
                PTREE_GL_Coord_t i;
                currLine = firstLine;
                for (i = 0; i < size; ++i)
                {
                    gl->fillBpp(currLine, size, offset, color);
                    currLine += gl->canvas.stride;
                }
            }
            offset += size;
        }
        currLine = firstLine + gl->canvas.stride * size;
        bitmap += bitmapStride;
    }
}

void PTREE_GL_DrawDotMatrix(PTREE_GL_Obj_t *gl, PTREE_GL_Color_t color, const char *dotMatrix, PTREE_GL_Coord_t w,
                            PTREE_GL_Coord_t h, PTREE_GL_Coord_t size)
{
    PTREE_GL_Coord_t row, col;
    char *           currLine;
    PTREE_GL_Coord_t leftPadding, topPadding;
    PTREE_GL_Coord_t blockW, blockH;
    if (!gl || !gl->canvas.data || !gl->fillBpp || !dotMatrix || !size || w > gl->canvas.width || h > gl->canvas.height)
    {
        return;
    }
    blockW      = gl->canvas.width / w;
    blockH      = gl->canvas.height / h;
    size        = size > blockW ? blockW : size;
    size        = size > blockH ? blockH : size;
    leftPadding = (blockW - size) >> 1;
    topPadding  = (blockH - size) >> 1;

    currLine = gl->canvas.data + gl->canvas.stride * topPadding;
    for (row = 0; row < h; ++row)
    {
        char *           firstLine = currLine;
        PTREE_GL_Coord_t offset    = leftPadding;
        for (col = 0; col < w; ++col)
        {
            if (dotMatrix[col])
            {
                PTREE_GL_Coord_t i;
                currLine = firstLine;
                for (i = 0; i < size; ++i)
                {
                    gl->fillBpp(currLine, size, offset, color);
                    currLine += gl->canvas.stride;
                }
            }
            offset += blockW;
        }
        currLine = firstLine + gl->canvas.stride * blockH;
        dotMatrix += w;
    }
}
