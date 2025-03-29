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

#ifndef __PTREE_MOD_RGN_METADATA_H__
#define __PTREE_MOD_RGN_METADATA_H__

#define PTREE_MOD_RGN_METADATA_COORDINATE_MAX_W (8192)
#define PTREE_MOD_RGN_METADATA_COORDINATE_MAX_H (8192)

#define PTREE_MOD_RGN_METADATA_TEXT_MAX_SIZE   (128)
#define PTREE_MOD_RGN_METADATA_POLY_VECTEX_MAX (6)

enum PTREE_MOD_RGN_METADATA_Type_e
{
    E_PTREE_MOD_RGN_METADATA_TEXTS      = 0,
    E_PTREE_MOD_RGN_METADATA_LINES      = 1,
    E_PTREE_MOD_RGN_METADATA_RECT_AREAS = 2,
    E_PTREE_MOD_RGN_METADATA_FRAMES     = E_PTREE_MOD_RGN_METADATA_RECT_AREAS,
    E_PTREE_MOD_RGN_METADATA_COVERS     = E_PTREE_MOD_RGN_METADATA_RECT_AREAS,
    E_PTREE_MOD_RGN_METADATA_POLYS      = 3,
};

enum PTREE_MOD_RGN_METADATA_State_e
{
    E_PTREE_MOD_RGN_METADATA_STATUS_OFF = 0,
    E_PTREE_MOD_RGN_METADATA_STATUS_ON  = 1,
};

typedef struct PTREE_MOD_RGN_METADATA_Point_s
{
    unsigned short x;
    unsigned short y;
} PTREE_MOD_RGN_METADATA_Point_t;

typedef struct PTREE_MOD_RGN_METADATA_Rect_s
{
    unsigned short x;
    unsigned short y;
    unsigned short w;
    unsigned short h;
} PTREE_MOD_RGN_METADATA_Rect_t;

typedef struct PTREE_MOD_RGN_METADATA_Line_s
{
    PTREE_MOD_RGN_METADATA_Point_t      pt0;
    PTREE_MOD_RGN_METADATA_Point_t      pt1;
    enum PTREE_MOD_RGN_METADATA_State_e state;
} PTREE_MOD_RGN_METADATA_Line_t;

typedef struct PTREE_MOD_RGN_METADATA_Lines_s
{
    unsigned int                  count;
    PTREE_MOD_RGN_METADATA_Line_t lines[0];
} PTREE_MOD_RGN_METADATA_Lines_t;

typedef struct PTREE_MOD_RGN_METADATA_Text_s
{
    PTREE_MOD_RGN_METADATA_Point_t pt;
    char                           str[PTREE_MOD_RGN_METADATA_TEXT_MAX_SIZE];
} PTREE_MOD_RGN_METADATA_Text_t;

typedef struct PTREE_MOD_RGN_METADATA_Texts_s
{
    unsigned int                  count;
    PTREE_MOD_RGN_METADATA_Text_t texts[0];
} PTREE_MOD_RGN_METADATA_Texts_t;

typedef struct PTREE_MOD_RGN_METADATA_RectArea_s
{
    PTREE_MOD_RGN_METADATA_Rect_t       rect;
    enum PTREE_MOD_RGN_METADATA_State_e state;
} PTREE_MOD_RGN_METADATA_RectArea_t;
typedef PTREE_MOD_RGN_METADATA_RectArea_t PTREE_MOD_RGN_METADATA_Frame_t;
typedef PTREE_MOD_RGN_METADATA_RectArea_t PTREE_MOD_RGN_METADATA_Cover_t;

typedef struct PTREE_MOD_RGN_METADATA_RectAreas_s
{
    unsigned int                      count;
    PTREE_MOD_RGN_METADATA_RectArea_t areas[0];
} PTREE_MOD_RGN_METADATA_RectAreas_t;
typedef PTREE_MOD_RGN_METADATA_RectAreas_t PTREE_MOD_RGN_METADATA_Frames_t;
typedef PTREE_MOD_RGN_METADATA_RectAreas_t PTREE_MOD_RGN_METADATA_Covers_t;

typedef struct PTREE_MOD_RGN_METADATA_Poly_s
{
    PTREE_MOD_RGN_METADATA_Point_t vectexArr[PTREE_MOD_RGN_METADATA_POLY_VECTEX_MAX];
    unsigned char                  vectexNum;
} PTREE_MOD_RGN_METADATA_Poly_t;

typedef struct PTREE_MOD_RGN_METADATA_Polys_s
{
    unsigned int                  count;
    PTREE_MOD_RGN_METADATA_Poly_t polys[0];
} PTREE_MOD_RGN_METADATA_Polys_t;

#endif /* ifndef __PTREE_MOD_RGN_METADATA_H__ */
