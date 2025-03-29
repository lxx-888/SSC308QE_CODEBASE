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

#ifndef __PTREE_GL_H__
#define __PTREE_GL_H__

typedef struct PTREE_GL_Obj_s    PTREE_GL_Obj_t;
typedef struct PTREE_GL_Canvas_s PTREE_GL_Canvas_t;

typedef unsigned int PTREE_GL_Color_t;
typedef unsigned int PTREE_GL_Coord_t;

struct PTREE_GL_Canvas_s
{
    char *           data;
    PTREE_GL_Coord_t width;
    PTREE_GL_Coord_t height;
    PTREE_GL_Coord_t stride;
    unsigned char    bpp;
};

struct PTREE_GL_Obj_s
{
    PTREE_GL_Canvas_t canvas;
    void (*fillBpp)(void *, unsigned int, unsigned int, unsigned int);
};

int PTREE_GL_Init(PTREE_GL_Obj_t *gl, const PTREE_GL_Canvas_t *canvas);

void PTREE_GL_FillColor(PTREE_GL_Obj_t *gl, PTREE_GL_Color_t color);

void PTREE_GL_SetPixel(PTREE_GL_Obj_t *gl, PTREE_GL_Color_t color, PTREE_GL_Coord_t x, PTREE_GL_Coord_t y);

void PTREE_GL_FillRect(PTREE_GL_Obj_t *gl, PTREE_GL_Color_t color, PTREE_GL_Coord_t x, PTREE_GL_Coord_t y,
                       PTREE_GL_Coord_t w, PTREE_GL_Coord_t h);

void PTREE_GL_DrawFrame(PTREE_GL_Obj_t *gl, PTREE_GL_Color_t color, PTREE_GL_Coord_t x, PTREE_GL_Coord_t y,
                        PTREE_GL_Coord_t w, PTREE_GL_Coord_t h, PTREE_GL_Coord_t thickness);

void PTREE_GL_DrawLine(PTREE_GL_Obj_t *gl, PTREE_GL_Color_t color, PTREE_GL_Coord_t x0, PTREE_GL_Coord_t y0,
                       PTREE_GL_Coord_t x1, PTREE_GL_Coord_t y1, PTREE_GL_Coord_t thickness);

void PTREE_GL_DrawBitmap(PTREE_GL_Obj_t *gl, PTREE_GL_Color_t color, PTREE_GL_Coord_t x, PTREE_GL_Coord_t y,
                         const char *bitmap, PTREE_GL_Coord_t w, PTREE_GL_Coord_t h, PTREE_GL_Coord_t size);

void PTREE_GL_DrawDotMatrix(PTREE_GL_Obj_t *gl, PTREE_GL_Color_t color, const char *dotMatrix, PTREE_GL_Coord_t w,
                            PTREE_GL_Coord_t h, PTREE_GL_Coord_t size);

#endif /* ifndef __PTREE_GL_H__ */
