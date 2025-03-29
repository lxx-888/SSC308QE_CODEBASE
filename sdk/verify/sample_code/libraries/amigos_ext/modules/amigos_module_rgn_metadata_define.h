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

#ifndef __AMIGOS_RGN_METADATA_DEFINE_H__
#define __AMIGOS_RGN_METADATA_DEFINE_H__

#define AMIGOS_RGN_METADATA_COORDINATE_MAX_W (8192)
#define AMIGOS_RGN_METADATA_COORDINATE_MAX_H (8192)

#define AMIGOS_RGN_METADATA_TEXT_MAX_SIZE   (128)
#define AMIGOS_RGN_METADATA_POLY_VECTEX_MAX (6)

enum AmigosRgnMetaDataType
{
    E_META_DATA_TEXTS      = 0,
    E_META_DATA_LINES      = 1,
    E_META_DATA_RECT_AREAS = 2,
    E_META_DATA_FRAMES     = E_META_DATA_RECT_AREAS,
    E_META_DATA_COVERS     = E_META_DATA_RECT_AREAS,
    E_META_DATA_POLYS      = 3,
    E_META_DATA_MAP        = 3,
};

enum AmigosRgnMetaDataState
{
    E_META_DATA_STATUS_OFF = 0,
    E_META_DATA_STATUS_ON  = 1,
};

struct AmigosRgnMetaDataPoint
{
    unsigned short x;
    unsigned short y;
};

struct AmigosRgnMetaDataRect
{
    unsigned short x;
    unsigned short y;
    unsigned short w;
    unsigned short h;
};

struct AmigosRgnMetaDataLine
{
    AmigosRgnMetaDataPoint pt0;
    AmigosRgnMetaDataPoint pt1;
    AmigosRgnMetaDataState state;
};

struct AmigosRgnMetaDataLines
{
    unsigned int          count;
    AmigosRgnMetaDataLine lines[0];
};

struct AmigosRgnMetaDataText
{
    AmigosRgnMetaDataPoint pt;
    wchar_t                str[AMIGOS_RGN_METADATA_TEXT_MAX_SIZE];
};

struct AmigosRgnMetaDataTexts
{
    unsigned int          count;
    AmigosRgnMetaDataText texts[0];
};

struct AmigosRgnMetaDataRectArea
{
    AmigosRgnMetaDataRect  rect;
    AmigosRgnMetaDataState state;
};
using AmigosRgnMetaDataFrame = AmigosRgnMetaDataRectArea;
using AmigosRgnMetaDataCover = AmigosRgnMetaDataRectArea;

struct AmigosRgnMetaDataRectAreas
{
    unsigned int              count;
    AmigosRgnMetaDataRectArea areas[0];
};
using AmigosRgnMetaDataFrames = AmigosRgnMetaDataRectAreas;
using AmigosRgnMetaDataCovers = AmigosRgnMetaDataRectAreas;

struct AmigosRgnMetaDataPoly
{
    AmigosRgnMetaDataPoint vectexArr[AMIGOS_RGN_METADATA_POLY_VECTEX_MAX];
    unsigned char          vectexNum;
};

struct AmigosRgnMetaDataPolys
{
    unsigned int          count;
    AmigosRgnMetaDataPoly polys[0];
};

struct AmigosRgnMetaDataMap
{
    unsigned short w;
    unsigned short h;
    char           data[0];
};

#endif /* __AMIGOS_RGN_METADATA_DEFINE_H__ */
