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

#ifndef __PTREE_RGN_PACKET_H__
#define __PTREE_RGN_PACKET_H__

#include "ptree_packet.h"

#define PTREE_RGN_PACKET_COORDINATE_MAX_W (8192)
#define PTREE_RGN_PACKET_COORDINATE_MAX_H (8192)

typedef struct PTREE_RGN_PACKET_Rect_s      PTREE_RGN_PACKET_Rect_t;
typedef struct PTREE_RGN_PACKET_RectsInfo_s PTREE_RGN_PACKET_RectsInfo_t;
typedef struct PTREE_RGN_PACKET_Rects_s     PTREE_RGN_PACKET_Rects_t;

typedef struct PTREE_RGN_PACKET_Point_s     PTREE_RGN_PACKET_Point_t;
typedef struct PTREE_RGN_PACKET_Line_s      PTREE_RGN_PACKET_Line_t;
typedef struct PTREE_RGN_PACKET_LinesInfo_s PTREE_RGN_PACKET_LinesInfo_t;
typedef struct PTREE_RGN_PACKET_Lines_s     PTREE_RGN_PACKET_Lines_t;

typedef struct PTREE_RGN_PACKET_MapInfo_s PTREE_RGN_PACKET_MapInfo_t;
typedef struct PTREE_RGN_PACKET_Map_s     PTREE_RGN_PACKET_Map_t;

struct PTREE_RGN_PACKET_Rect_s
{
    unsigned short x;
    unsigned short y;
    unsigned short w;
    unsigned short h;
};
struct PTREE_RGN_PACKET_RectsInfo_s
{
    PTREE_PACKET_Info_t base;
    unsigned int        count;
};
struct PTREE_RGN_PACKET_Rects_s
{
    PTREE_PACKET_Obj_t           base;
    PTREE_RGN_PACKET_RectsInfo_t info;
    PTREE_RGN_PACKET_Rect_t *    rects;
};

struct PTREE_RGN_PACKET_Point_s
{
    unsigned short x;
    unsigned short y;
};
struct PTREE_RGN_PACKET_Line_s
{
    PTREE_RGN_PACKET_Point_t pt0;
    PTREE_RGN_PACKET_Point_t pt1;
};
struct PTREE_RGN_PACKET_LinesInfo_s
{
    PTREE_PACKET_Info_t base;
    unsigned int        count;
};
struct PTREE_RGN_PACKET_Lines_s
{
    PTREE_PACKET_Obj_t           base;
    PTREE_RGN_PACKET_LinesInfo_t info;
    PTREE_RGN_PACKET_Line_t *    lines;
};

struct PTREE_RGN_PACKET_MapInfo_s
{
    PTREE_PACKET_Info_t base;
    unsigned int        width;
    unsigned int        height;
};
struct PTREE_RGN_PACKET_Map_s
{
    PTREE_PACKET_Obj_t         base;
    PTREE_RGN_PACKET_MapInfo_t info;
    char *                     data;
};

int                  PTREE_RGN_PACKET_RectsInfoInit(PTREE_RGN_PACKET_RectsInfo_t *rectsInfo, unsigned int count);
PTREE_PACKET_Info_t *PTREE_RGN_PACKET_RectsInfoNew(unsigned int count);
int                  PTREE_RGN_PACKET_RectsInit(PTREE_RGN_PACKET_Rects_t *rects, unsigned int count);
PTREE_PACKET_Obj_t * PTREE_RGN_PACKET_RectsNew(unsigned int count);

int                  PTREE_RGN_PACKET_LinesInfoInit(PTREE_RGN_PACKET_LinesInfo_t *rectsInfo, unsigned int count);
PTREE_PACKET_Info_t *PTREE_RGN_PACKET_LinesInfoNew(unsigned int count);
int                  PTREE_RGN_PACKET_LinesInit(PTREE_RGN_PACKET_Lines_t *rects, unsigned int count);
PTREE_PACKET_Obj_t * PTREE_RGN_PACKET_LinesNew(unsigned int count);

int PTREE_RGN_PACKET_MapInfoInit(PTREE_RGN_PACKET_MapInfo_t *rectsInfo, unsigned int width, unsigned int height);
PTREE_PACKET_Info_t *PTREE_RGN_PACKET_MapInfoNew(unsigned int width, unsigned int height);
int                  PTREE_RGN_PACKET_MapInit(PTREE_RGN_PACKET_Map_t *rects, unsigned int width, unsigned int height);
PTREE_PACKET_Obj_t * PTREE_RGN_PACKET_MapNew(unsigned int width, unsigned int height);

#endif /* ifndef __PTREE_RGN_PACKET_H__ */
