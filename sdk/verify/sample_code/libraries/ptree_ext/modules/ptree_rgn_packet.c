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

#include "ptree_rgn_packet.h"
#include "ssos_def.h"
#include "ssos_io.h"
#include "ssos_list.h"
#include "ssos_mem.h"
#include "ptree_packet.h"
#include "ptree_log.h"

#define CHECK_POINTER(_ptr, _err)            \
    (                                        \
        {                                    \
            if (!(_ptr))                     \
            {                                \
                PTREE_ERR("%s NULL", #_ptr); \
                _err;                        \
            }                                \
        })

static void _PTREE_RGN_PACKET_RectsInfoFree(PTREE_PACKET_Info_t *info);
static void _PTREE_RGN_PACKET_LinesInfoFree(PTREE_PACKET_Info_t *info);
static void _PTREE_RGN_PACKET_MapInfoFree(PTREE_PACKET_Info_t *info);

static int _PTREE_RGN_PACKET_RectsInfoEqual(const PTREE_PACKET_Info_t *self, const PTREE_PACKET_Info_t *other);
static int _PTREE_RGN_PACKET_LinesInfoEqual(const PTREE_PACKET_Info_t *self, const PTREE_PACKET_Info_t *other);
static int _PTREE_RGN_PACKET_MapInfoEqual(const PTREE_PACKET_Info_t *self, const PTREE_PACKET_Info_t *other);

static void _PTREE_RGN_PACKET_RectsDestruct(PTREE_PACKET_Obj_t *packet);
static void _PTREE_RGN_PACKET_LinesDestruct(PTREE_PACKET_Obj_t *packet);
static void _PTREE_RGN_PACKET_MapDestruct(PTREE_PACKET_Obj_t *packet);

static void _PTREE_RGN_PACKET_RectsFree(PTREE_PACKET_Obj_t *packet);
static void _PTREE_RGN_PACKET_LinesFree(PTREE_PACKET_Obj_t *packet);
static void _PTREE_RGN_PACKET_MapFree(PTREE_PACKET_Obj_t *packet);

PTREE_PACKET_TYPE_DEFINE(rgn);

PTREE_PACKET_INFO_TYPE_DEFINE(rects);
PTREE_PACKET_INFO_TYPE_DEFINE(lines);
PTREE_PACKET_INFO_TYPE_DEFINE(map);

static const PTREE_PACKET_InfoOps_t  G_PTREE_RGN_PACKET_RECTS_INFO_OPS  = {.equal = _PTREE_RGN_PACKET_RectsInfoEqual};
static const PTREE_PACKET_InfoHook_t G_PTREE_RGN_PACKET_RECTS_INFO_HOOK = {};
static const PTREE_PACKET_InfoHook_t G_PTREE_RGN_PACKET_RECTS_INFO_DYN_HOOK = {
    .free = _PTREE_RGN_PACKET_RectsInfoFree,
};

static const PTREE_PACKET_InfoOps_t  G_PTREE_RGN_PACKET_LINES_INFO_OPS  = {.equal = _PTREE_RGN_PACKET_LinesInfoEqual};
static const PTREE_PACKET_InfoHook_t G_PTREE_RGN_PACKET_LINES_INFO_HOOK = {};
static const PTREE_PACKET_InfoHook_t G_PTREE_RGN_PACKET_LINES_INFO_DYN_HOOK = {
    .free = _PTREE_RGN_PACKET_LinesInfoFree,
};

static const PTREE_PACKET_InfoOps_t  G_PTREE_RGN_PACKET_MAP_INFO_OPS      = {.equal = _PTREE_RGN_PACKET_MapInfoEqual};
static const PTREE_PACKET_InfoHook_t G_PTREE_RGN_PACKET_MAP_INFO_HOOK     = {};
static const PTREE_PACKET_InfoHook_t G_PTREE_RGN_PACKET_MAP_INFO_DYN_HOOK = {
    .free = _PTREE_RGN_PACKET_MapInfoFree,
};

static const PTREE_PACKET_ObjOps_t  G_PTREE_RGN_PACKET_RECTS_OPS  = {};
static const PTREE_PACKET_ObjHook_t G_PTREE_RGN_PACKET_RECTS_HOOK = {
    .destruct = _PTREE_RGN_PACKET_RectsDestruct,
    .free     = NULL,
};
static const PTREE_PACKET_ObjHook_t G_PTREE_RGN_PACKET_RECTS_DYN_HOOK = {
    .destruct = _PTREE_RGN_PACKET_RectsDestruct,
    .free     = _PTREE_RGN_PACKET_RectsFree,
};

static const PTREE_PACKET_ObjOps_t  G_PTREE_RGN_PACKET_LINES_OPS  = {};
static const PTREE_PACKET_ObjHook_t G_PTREE_RGN_PACKET_LINES_HOOK = {
    .destruct = _PTREE_RGN_PACKET_LinesDestruct,
    .free     = NULL,
};
static const PTREE_PACKET_ObjHook_t G_PTREE_RGN_PACKET_LINES_DYN_HOOK = {
    .destruct = _PTREE_RGN_PACKET_LinesDestruct,
    .free     = _PTREE_RGN_PACKET_LinesFree,
};

static const PTREE_PACKET_ObjOps_t  G_PTREE_RGN_PACKET_MAP_OPS  = {};
static const PTREE_PACKET_ObjHook_t G_PTREE_RGN_PACKET_MAP_HOOK = {
    .destruct = _PTREE_RGN_PACKET_MapDestruct,
    .free     = NULL,
};
static const PTREE_PACKET_ObjHook_t G_PTREE_RGN_PACKET_MAP_DYN_HOOK = {
    .destruct = _PTREE_RGN_PACKET_MapDestruct,
    .free     = _PTREE_RGN_PACKET_MapFree,
};

static void _PTREE_RGN_PACKET_RectsInfoFree(PTREE_PACKET_Info_t *info)
{
    SSOS_MEM_Free(CONTAINER_OF(info, PTREE_RGN_PACKET_RectsInfo_t, base));
}
static void _PTREE_RGN_PACKET_LinesInfoFree(PTREE_PACKET_Info_t *info)
{
    SSOS_MEM_Free(CONTAINER_OF(info, PTREE_RGN_PACKET_LinesInfo_t, base));
}
static void _PTREE_RGN_PACKET_MapInfoFree(PTREE_PACKET_Info_t *info)
{
    SSOS_MEM_Free(CONTAINER_OF(info, PTREE_RGN_PACKET_MapInfo_t, base));
}

static int _PTREE_RGN_PACKET_RectsInfoEqual(const PTREE_PACKET_Info_t *self, const PTREE_PACKET_Info_t *other)
{
    PTREE_RGN_PACKET_RectsInfo_t *rectsInfoSelf  = CONTAINER_OF(self, PTREE_RGN_PACKET_RectsInfo_t, base);
    PTREE_RGN_PACKET_RectsInfo_t *rectsInfoOther = CONTAINER_OF(other, PTREE_RGN_PACKET_RectsInfo_t, base);
    return rectsInfoSelf->count == rectsInfoOther->count;
}
static int _PTREE_RGN_PACKET_LinesInfoEqual(const PTREE_PACKET_Info_t *self, const PTREE_PACKET_Info_t *other)
{
    PTREE_RGN_PACKET_LinesInfo_t *linessInfoSelf  = CONTAINER_OF(self, PTREE_RGN_PACKET_LinesInfo_t, base);
    PTREE_RGN_PACKET_LinesInfo_t *linessInfoOther = CONTAINER_OF(other, PTREE_RGN_PACKET_LinesInfo_t, base);
    return linessInfoSelf->count == linessInfoOther->count;
}
static int _PTREE_RGN_PACKET_MapInfoEqual(const PTREE_PACKET_Info_t *self, const PTREE_PACKET_Info_t *other)
{
    PTREE_RGN_PACKET_MapInfo_t *mapInfoSelf  = CONTAINER_OF(self, PTREE_RGN_PACKET_MapInfo_t, base);
    PTREE_RGN_PACKET_MapInfo_t *mapInfoOther = CONTAINER_OF(other, PTREE_RGN_PACKET_MapInfo_t, base);
    return mapInfoSelf->width == mapInfoOther->width && mapInfoSelf->height == mapInfoOther->height;
}

static void _PTREE_RGN_PACKET_RectsDestruct(PTREE_PACKET_Obj_t *packet)
{
    PTREE_RGN_PACKET_Rects_t *rects = CONTAINER_OF(packet, PTREE_RGN_PACKET_Rects_t, base);
    if (rects->rects)
    {
        SSOS_MEM_Free(rects->rects);
        rects->rects = NULL;
    }
}
static void _PTREE_RGN_PACKET_LinesDestruct(PTREE_PACKET_Obj_t *packet)
{
    PTREE_RGN_PACKET_Lines_t *lines = CONTAINER_OF(packet, PTREE_RGN_PACKET_Lines_t, base);
    if (lines->lines)
    {
        SSOS_MEM_Free(lines->lines);
        lines->lines = NULL;
    }
}
static void _PTREE_RGN_PACKET_MapDestruct(PTREE_PACKET_Obj_t *packet)
{
    PTREE_RGN_PACKET_Map_t *map = CONTAINER_OF(packet, PTREE_RGN_PACKET_Map_t, base);
    if (map->data)
    {
        SSOS_MEM_Free(map->data);
        map->data = NULL;
    }
}

static void _PTREE_RGN_PACKET_RectsFree(PTREE_PACKET_Obj_t *packet)
{
    SSOS_MEM_Free(CONTAINER_OF(packet, PTREE_RGN_PACKET_Rects_t, base));
}
static void _PTREE_RGN_PACKET_LinesFree(PTREE_PACKET_Obj_t *packet)
{
    SSOS_MEM_Free(CONTAINER_OF(packet, PTREE_RGN_PACKET_Lines_t, base));
}
static void _PTREE_RGN_PACKET_MapFree(PTREE_PACKET_Obj_t *packet)
{
    SSOS_MEM_Free(CONTAINER_OF(packet, PTREE_RGN_PACKET_Map_t, base));
}

static int _PTREE_RGN_PACKET_RectsInfoInit(PTREE_RGN_PACKET_RectsInfo_t *rectsInfo, unsigned int count)
{
    int ret;
    ret = PTREE_PACKET_InfoInit(&rectsInfo->base, &G_PTREE_RGN_PACKET_RECTS_INFO_OPS, PTREE_PACKET_INFO_TYPE(rects));
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("PTREE_PACKET_InfoInit failed");
        return ret;
    }
    rectsInfo->count = count;
    return SSOS_DEF_OK;
}
static int _PTREE_RGN_PACKET_LinesInfoInit(PTREE_RGN_PACKET_LinesInfo_t *linesInfo, unsigned int count)
{
    int ret;
    ret = PTREE_PACKET_InfoInit(&linesInfo->base, &G_PTREE_RGN_PACKET_LINES_INFO_OPS, PTREE_PACKET_INFO_TYPE(lines));
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("PTREE_PACKET_InfoInit failed");
        return ret;
    }
    linesInfo->count = count;
    return SSOS_DEF_OK;
}
static int _PTREE_RGN_PACKET_MapInfoInit(PTREE_RGN_PACKET_MapInfo_t *mapInfo, unsigned int width, unsigned int height)
{
    int ret;
    ret = PTREE_PACKET_InfoInit(&mapInfo->base, &G_PTREE_RGN_PACKET_MAP_INFO_OPS, PTREE_PACKET_INFO_TYPE(map));
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("PTREE_PACKET_InfoInit failed");
        return ret;
    }
    mapInfo->width  = width;
    mapInfo->height = height;
    return SSOS_DEF_OK;
}

static int _PTREE_RGN_PACKET_RectsInit(PTREE_RGN_PACKET_Rects_t *rects, unsigned int count)
{
    int ret = SSOS_DEF_OK;
    ret     = PTREE_RGN_PACKET_RectsInfoInit(&rects->info, count);
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("PTREE_RGN_PACKET_RectsInfoInit Failed");
        return ret;
    }
    ret = PTREE_PACKET_Init(&rects->base, &G_PTREE_RGN_PACKET_RECTS_OPS, &rects->info.base, PTREE_PACKET_TYPE(rgn));
    if (ret != SSOS_DEF_OK)
    {
        PTREE_PACKET_InfoDel(&rects->info.base);
        PTREE_ERR("PTREE_RGN_PACKET_RectsInfoInit Failed");
        return ret;
    }
    if (!count)
    {
        return SSOS_DEF_OK;
    }
    rects->rects = SSOS_MEM_Alloc(sizeof(PTREE_RGN_PACKET_Rect_t) * count);
    if (!rects->rects)
    {
        PTREE_PACKET_Del(&rects->base);
        PTREE_PACKET_InfoDel(&rects->info.base);
        PTREE_ERR("Alloc err");
        return SSOS_DEF_ENOMEM;
    }
    memset(rects->rects, 0, sizeof(PTREE_RGN_PACKET_Rect_t) * count);
    return SSOS_DEF_OK;
}
static int _PTREE_RGN_PACKET_LinesInit(PTREE_RGN_PACKET_Lines_t *lines, unsigned int count)
{
    int ret = SSOS_DEF_OK;
    ret     = PTREE_RGN_PACKET_LinesInfoInit(&lines->info, count);
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("PTREE_RGN_PACKET_LinesInfoInit Failed");
        return ret;
    }
    ret = PTREE_PACKET_Init(&lines->base, &G_PTREE_RGN_PACKET_LINES_OPS, &lines->info.base, PTREE_PACKET_TYPE(rgn));
    if (ret != SSOS_DEF_OK)
    {
        PTREE_PACKET_InfoDel(&lines->info.base);
        PTREE_ERR("PTREE_RGN_PACKET_LinesInfoInit Failed");
        return ret;
    }
    if (!count)
    {
        return SSOS_DEF_OK;
    }
    lines->lines = SSOS_MEM_Alloc(sizeof(PTREE_RGN_PACKET_Line_t) * count);
    if (!lines->lines)
    {
        PTREE_PACKET_Del(&lines->base);
        PTREE_PACKET_InfoDel(&lines->info.base);
        PTREE_ERR("Alloc err");
        return SSOS_DEF_ENOMEM;
    }
    memset(lines->lines, 0, sizeof(PTREE_RGN_PACKET_Line_t) * count);
    return SSOS_DEF_OK;
}
static int _PTREE_RGN_PACKET_MapInit(PTREE_RGN_PACKET_Map_t *map, unsigned int width, unsigned int height)
{
    int ret = SSOS_DEF_OK;
    ret     = PTREE_RGN_PACKET_MapInfoInit(&map->info, width, height);
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("PTREE_RGN_PACKET_MapInfoInit Failed");
        return ret;
    }
    ret = PTREE_PACKET_Init(&map->base, &G_PTREE_RGN_PACKET_MAP_OPS, &map->info.base, PTREE_PACKET_TYPE(rgn));
    if (ret != SSOS_DEF_OK)
    {
        PTREE_PACKET_InfoDel(&map->info.base);
        PTREE_ERR("PTREE_RGN_PACKET_MapInfoInit Failed");
        return ret;
    }
    if (!width || !height)
    {
        return SSOS_DEF_OK;
    }
    map->data = SSOS_MEM_Alloc(sizeof(PTREE_RGN_PACKET_Line_t) * width * height);
    if (!map->data)
    {
        PTREE_PACKET_Del(&map->base);
        PTREE_PACKET_InfoDel(&map->info.base);
        PTREE_ERR("Alloc err");
        return SSOS_DEF_ENOMEM;
    }
    memset(map->data, 0, sizeof(PTREE_RGN_PACKET_Line_t) * width * height);
    return SSOS_DEF_OK;
}
int PTREE_RGN_PACKET_RectsInfoInit(PTREE_RGN_PACKET_RectsInfo_t *rectsInfo, unsigned int count)
{
    int ret = SSOS_DEF_OK;
    CHECK_POINTER(rectsInfo, return SSOS_DEF_EINVAL);
    ret = _PTREE_RGN_PACKET_RectsInfoInit(rectsInfo, count);
    if (ret != SSOS_DEF_OK)
    {
        return ret;
    }
    PTREE_PACKET_InfoRegister(&rectsInfo->base, &G_PTREE_RGN_PACKET_RECTS_INFO_HOOK);
    return SSOS_DEF_OK;
}
PTREE_PACKET_Info_t *PTREE_RGN_PACKET_RectsInfoNew(unsigned int count)
{
    PTREE_RGN_PACKET_RectsInfo_t *rectsInfo = NULL;

    rectsInfo = SSOS_MEM_Alloc(sizeof(PTREE_RGN_PACKET_RectsInfo_t));
    if (!rectsInfo)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(rectsInfo, 0, sizeof(PTREE_RGN_PACKET_RectsInfo_t));
    if (SSOS_DEF_OK != _PTREE_RGN_PACKET_RectsInfoInit(rectsInfo, count))
    {
        SSOS_MEM_Free(rectsInfo);
        return NULL;
    }
    PTREE_PACKET_InfoRegister(&rectsInfo->base, &G_PTREE_RGN_PACKET_RECTS_INFO_DYN_HOOK);
    return &rectsInfo->base;
}
int PTREE_RGN_PACKET_RectsInit(PTREE_RGN_PACKET_Rects_t *rects, unsigned int count)
{
    int ret = SSOS_DEF_OK;
    CHECK_POINTER(rects, return SSOS_DEF_EINVAL);

    ret = _PTREE_RGN_PACKET_RectsInit(rects, count);
    if (ret != SSOS_DEF_OK)
    {
        return ret;
    }

    PTREE_PACKET_Register(&rects->base, &G_PTREE_RGN_PACKET_RECTS_HOOK);
    return SSOS_DEF_OK;
}
PTREE_PACKET_Obj_t *PTREE_RGN_PACKET_RectsNew(unsigned int count)
{
    PTREE_RGN_PACKET_Rects_t *rects = NULL;

    rects = SSOS_MEM_Alloc(sizeof(PTREE_RGN_PACKET_Rects_t));
    if (!rects)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(rects, 0, sizeof(PTREE_RGN_PACKET_Rects_t));

    if (SSOS_DEF_OK != _PTREE_RGN_PACKET_RectsInit(rects, count))
    {
        SSOS_MEM_Free(rects);
        return NULL;
    }

    PTREE_PACKET_Register(&rects->base, &G_PTREE_RGN_PACKET_RECTS_DYN_HOOK);
    return &rects->base;
}

int PTREE_RGN_PACKET_LinesInfoInit(PTREE_RGN_PACKET_LinesInfo_t *linesInfo, unsigned int count)
{
    int ret = SSOS_DEF_OK;
    CHECK_POINTER(linesInfo, return SSOS_DEF_EINVAL);
    ret = _PTREE_RGN_PACKET_LinesInfoInit(linesInfo, count);
    if (ret != SSOS_DEF_OK)
    {
        return ret;
    }
    PTREE_PACKET_InfoRegister(&linesInfo->base, &G_PTREE_RGN_PACKET_LINES_INFO_HOOK);
    return SSOS_DEF_OK;
}
PTREE_PACKET_Info_t *PTREE_RGN_PACKET_LinesInfoNew(unsigned int count)
{
    PTREE_RGN_PACKET_LinesInfo_t *linesInfo = NULL;

    linesInfo = SSOS_MEM_Alloc(sizeof(PTREE_RGN_PACKET_LinesInfo_t));
    if (!linesInfo)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(linesInfo, 0, sizeof(PTREE_RGN_PACKET_LinesInfo_t));
    if (SSOS_DEF_OK != _PTREE_RGN_PACKET_LinesInfoInit(linesInfo, count))
    {
        SSOS_MEM_Free(linesInfo);
        return NULL;
    }
    PTREE_PACKET_InfoRegister(&linesInfo->base, &G_PTREE_RGN_PACKET_LINES_INFO_DYN_HOOK);
    return &linesInfo->base;
}
int PTREE_RGN_PACKET_LinesInit(PTREE_RGN_PACKET_Lines_t *lines, unsigned int count)
{
    int ret = SSOS_DEF_OK;
    CHECK_POINTER(lines, return SSOS_DEF_EINVAL);

    ret = _PTREE_RGN_PACKET_LinesInit(lines, count);
    if (ret != SSOS_DEF_OK)
    {
        return ret;
    }

    PTREE_PACKET_Register(&lines->base, &G_PTREE_RGN_PACKET_LINES_HOOK);
    return SSOS_DEF_OK;
}
PTREE_PACKET_Obj_t *PTREE_RGN_PACKET_LinesNew(unsigned int count)
{
    PTREE_RGN_PACKET_Lines_t *lines = NULL;

    lines = SSOS_MEM_Alloc(sizeof(PTREE_RGN_PACKET_Lines_t));
    if (!lines)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(lines, 0, sizeof(PTREE_RGN_PACKET_Lines_t));

    if (SSOS_DEF_OK != _PTREE_RGN_PACKET_LinesInit(lines, count))
    {
        SSOS_MEM_Free(lines);
        return NULL;
    }

    PTREE_PACKET_Register(&lines->base, &G_PTREE_RGN_PACKET_LINES_DYN_HOOK);
    return &lines->base;
}

int PTREE_RGN_PACKET_MapInfoInit(PTREE_RGN_PACKET_MapInfo_t *mapInfo, unsigned int width, unsigned int height)
{
    int ret = SSOS_DEF_OK;
    CHECK_POINTER(mapInfo, return SSOS_DEF_EINVAL);
    ret = _PTREE_RGN_PACKET_MapInfoInit(mapInfo, width, height);
    if (ret != SSOS_DEF_OK)
    {
        return ret;
    }
    PTREE_PACKET_InfoRegister(&mapInfo->base, &G_PTREE_RGN_PACKET_MAP_INFO_HOOK);
    return SSOS_DEF_OK;
}
PTREE_PACKET_Info_t *PTREE_RGN_PACKET_MapInfoNew(unsigned int width, unsigned int height)
{
    PTREE_RGN_PACKET_MapInfo_t *mapInfo = NULL;

    mapInfo = SSOS_MEM_Alloc(sizeof(PTREE_RGN_PACKET_MapInfo_t));
    if (!mapInfo)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(mapInfo, 0, sizeof(PTREE_RGN_PACKET_MapInfo_t));
    if (SSOS_DEF_OK != _PTREE_RGN_PACKET_MapInfoInit(mapInfo, width, height))
    {
        SSOS_MEM_Free(mapInfo);
        return NULL;
    }
    PTREE_PACKET_InfoRegister(&mapInfo->base, &G_PTREE_RGN_PACKET_MAP_INFO_DYN_HOOK);
    return &mapInfo->base;
}
int PTREE_RGN_PACKET_MapInit(PTREE_RGN_PACKET_Map_t *map, unsigned int width, unsigned int height)
{
    int ret = SSOS_DEF_OK;
    CHECK_POINTER(map, return SSOS_DEF_EINVAL);

    ret = _PTREE_RGN_PACKET_MapInit(map, width, height);
    if (ret != SSOS_DEF_OK)
    {
        return ret;
    }

    PTREE_PACKET_Register(&map->base, &G_PTREE_RGN_PACKET_MAP_HOOK);
    return SSOS_DEF_OK;
}
PTREE_PACKET_Obj_t *PTREE_RGN_PACKET_MapNew(unsigned int width, unsigned int height)
{
    PTREE_RGN_PACKET_Map_t *map = NULL;

    map = SSOS_MEM_Alloc(sizeof(PTREE_RGN_PACKET_Map_t));
    if (!map)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(map, 0, sizeof(PTREE_RGN_PACKET_Map_t));

    if (SSOS_DEF_OK != _PTREE_RGN_PACKET_MapInit(map, width, height))
    {
        SSOS_MEM_Free(map);
        return NULL;
    }

    PTREE_PACKET_Register(&map->base, &G_PTREE_RGN_PACKET_MAP_DYN_HOOK);
    return &map->base;
}
