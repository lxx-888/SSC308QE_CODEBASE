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
#include "ss_graphic.h"
#include "ss_graphic_factory.h"

SsGraphic* SsGraphicFactory::Create(PixelFormat_e ePixelFmt, uint32_t width, uint32_t height, void *ptr)
{
    SsGraphic * pSsGraphic = nullptr;
    switch (ePixelFmt) {
        case E_BUFFER_NODE_YUV422:
            pSsGraphic = new YUV422_SsGraphic(width, height, ptr);
            break;
        case E_BUFFER_NODE_ARGB8888:
            pSsGraphic = new ARGB8888_SsGraphic(width, height, ptr);
            break;
        case E_BUFFER_NODE_RGB565:
            pSsGraphic = new RGB565_SsGraphic(width, height, ptr);
            break;
        case E_BUFFER_NODE_ARGB1555:
            pSsGraphic = new ARGB1555_SsGraphic(width, height, ptr);
            break;
        case E_BUFFER_NODE_ARGB4444:
            pSsGraphic = new ARGB4444_SsGraphic(width, height, ptr);
            break;
        case E_BUFFER_NODE_I2:
            pSsGraphic = new I2_SsGraphic(width, height, ptr);
            break;
        case E_BUFFER_NODE_I4:
            pSsGraphic = new I4_SsGraphic(width, height, ptr);
            break;
        case E_BUFFER_NODE_I8:
            pSsGraphic = new I8_SsGraphic(width, height, ptr);
            break;
        case E_BUFFER_NODE_YUV420SP:
            pSsGraphic = new YUV420SP_SsGraphic(width, height, ptr);
            break;
        case E_BUFFER_NODE_STREAM:
            pSsGraphic = new Stream_SsGraphic(width * height, ptr);
            break;
        default:
            break;
    }
    return pSsGraphic;
}

