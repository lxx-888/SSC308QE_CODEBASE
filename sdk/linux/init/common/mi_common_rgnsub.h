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
#ifndef _REGIONSUBSTRACT_H_
#define _REGIONSUBSTRACT_H_

#include "cam_os_wrapper.h"

typedef int           BOOL;
typedef unsigned char BYTE;
typedef void*         PBLOCKHEAP;

typedef struct MI_COMMON_RgnsubHandle_s
{
    CamOsMemCache_t rgnsubCache;
} MI_COMMON_RgnsubHandle_t;

typedef struct _RECT
{
    int left;
    int top;
    int right;
    int bottom;
} RECT;

typedef RECT* PRECT;

typedef struct _CLIPRECT
{
    RECT rc;
    struct _CLIPRECT* next;
    struct _CLIPRECT* prev;
} CLIPRECT;
typedef CLIPRECT* PCLIPRECT;

#define NULLREGION    0x00
#define SIMPLEREGION  0x01
#define COMPLEXREGION 0x02

typedef struct _CLIPRGN
{
    BYTE type;
    BYTE reserved[3];
    RECT rcBound;
    PCLIPRECT head;
    PCLIPRECT tail;
    PBLOCKHEAP heap;
} CLIPRGN;

typedef CLIPRGN* PCLIPRGN;

#define region_for_each_rect(rect, rgn)                            \
    for (rect = ((rgn)->head) ? (&((rgn)->head->rc)) : NULL; rect; \
         rect = (CAM_OS_CONTAINER_OF(rect, CLIPRECT, rc)->next) ? &(CAM_OS_CONTAINER_OF(rect, CLIPRECT, rc)->next->rc) : NULL)

void mi_common_RgnsubDbgDumpRgn(CLIPRGN* region);
BOOL mi_common_RgnsubIsEmptyClipRgn(const CLIPRGN* pRgn);
void mi_common_RgnsubInitClipRgn(PCLIPRGN pRgn, PBLOCKHEAP heap);
void mi_common_RgnsubEmptyClipRgn(MI_COMMON_RgnsubHandle_t *handle, PCLIPRGN pRgn);
BOOL mi_common_RgnsubAddClipRect(MI_COMMON_RgnsubHandle_t *handle, PCLIPRGN region, const RECT *rect);
BOOL mi_common_RgnsubSubtractRgn(MI_COMMON_RgnsubHandle_t *handle, CLIPRGN *rgnD, const CLIPRGN *rgnM, const CLIPRGN *rgnS);
int mi_common_RgnsubInitRgn(MI_COMMON_RgnsubHandle_t *handle);
int mi_common_RgnsubDeinitRgn(MI_COMMON_RgnsubHandle_t *handle);

#endif