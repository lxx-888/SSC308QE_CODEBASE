/*
 * boot_jpd.c- Sigmastar
 *
 * Copyright (c) [2019~2020] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include <common.h>
#include <command.h>
#include <malloc.h>

#if defined(CONFIG_SSTAR_JPD)
#define JPD_TIME_DEBUG 1
#define ALIGNMENT_NEED 64

#ifndef ALIGN_UP
#define ALIGN_UP(val, alignment) ((((val) + (alignment)-1) / (alignment)) * (alignment))
#endif

#if defined(CONFIG_JPD_SW)
#include "jinclude.h"
#include "jpeglib.h"
#elif defined(CONFIG_JPD_HW)
#include "mhal_jpd.h"

#define MAX_DEC_HEIGHT 8640
#define MAX_DEC_WIDTH  10000
typedef struct MHal_JPD_Addr_s
{
    void *       InputAddr;
    void *       InputpVirAddr;
    unsigned int InputSize;

    void *       OutputAddr;
    void *       OutpVirAddr;
    unsigned int OutputSize;

    void *       InterAddr;
    void *       InterVirAddr;
    unsigned int InterSize;

} MHal_JPD_Addr_t;

#endif
#endif

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define BOOT_JPD_DBG_LEVEL_ERR  0x01
#define BOOT_JPD_DBG_LEVEL_INFO 0x02
#define BOOT_JPD_DBG_LEVEL_JPD  0x04

#define BOOT_JPD_DBG(dbglv, _fmt, _args...) \
    do                                      \
        if (dbglv <= g_u32BootJpdDbgLevel)  \
        {                                   \
            printf(_fmt, ##_args);          \
        }                                   \
    while (0)

#define BOOTLOGO_VIRTUAL_ADDRESS_OFFSET 0x20000000

//-------------------------------------------------------------------------------------------------
//  structure & Enu
//-------------------------------------------------------------------------------------------------

typedef enum
{
    EN_LOGO_ROTATE_NONE,
    EN_LOGO_ROTATE_90,
    EN_LOGO_ROTATE_180,
    EN_LOGO_ROTATE_270
} LogoRotation_e;

extern void invalidate_dcache_range(unsigned long start, unsigned long stop);
extern void flush_dcache_range(unsigned long start, unsigned long stop);

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
u32 g_u32BootJpdDbgLevel = BOOT_JPD_DBG_LEVEL_JPD; // | BOOT_JPD_DBG_LEVEL_INFO | BOOT_JPD_DBG_LEVEL_JPD;

//-------------------------------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------------------------------

#define YUV444_TO_YUV420_PIXEL_MAPPING(y_dst_addr, uv_dst_addr, dst_x, dst_y, dst_stride, src_addr, src_x, src_y, \
                                       src_w, src_h)                                                              \
    do                                                                                                            \
    {                                                                                                             \
        for (src_y = 0; src_y < src_h; src_y++)                                                                   \
        {                                                                                                         \
            for (src_x = 0; src_x < src_w; src_x++)                                                               \
            {                                                                                                     \
                *((char *)((char *)(y_dst_addr) + (dst_y) * (dst_stride) + (dst_x))) =                            \
                    *((char *)((char *)(src_addr) + (src_y) * (src_w * 3) + (src_x * 3)));                        \
                if ((src_y & 0x01) && (src_x & 0x01))                                                             \
                {                                                                                                 \
                    *((short *)((char *)(uv_dst_addr) + ((dst_y - 1) >> 1) * (dst_stride) + (dst_x - 1))) =       \
                        *((short *)((char *)(src_addr) + (src_y) * (src_w * 3) + (src_x * 3) + 1));               \
                }                                                                                                 \
            }                                                                                                     \
        }                                                                                                         \
    } while (0)

void _BootLogoYuv444ToYuv420(u8 *pu8InBuf, u8 *pu8OutBuf, u16 *pu16Width, u16 *pu16Height, LogoRotation_e eRot)
{
    u16 x, y;

    u8 *pu8DesY = NULL, *pu8DesUV = NULL;
    u8 *pu8SrcYUV = NULL;

    pu8SrcYUV = pu8InBuf;

    pu8DesY  = pu8OutBuf;
    pu8DesUV = pu8DesY + (*pu16Width) * (*pu16Height);

    BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_JPD, "%s %d:: 444 To 420, In:%lx, Out:%lx, Width:%d, Height:%d\n", __FUNCTION__,
                 __LINE__, (unsigned long)pu8InBuf, (unsigned long)pu8OutBuf, *pu16Width, *pu16Height);

    switch (eRot)
    {
        case EN_LOGO_ROTATE_NONE:
        {
            YUV444_TO_YUV420_PIXEL_MAPPING(pu8DesY, pu8DesUV, x, y, *pu16Width, pu8SrcYUV, x, y, *pu16Width,
                                           *pu16Height);
        }
        break;
        case EN_LOGO_ROTATE_90:
        {
            YUV444_TO_YUV420_PIXEL_MAPPING(pu8DesY, pu8DesUV, *pu16Height - y, x, *pu16Height, pu8SrcYUV, x, y,
                                           *pu16Width, *pu16Height);
            *pu16Width ^= *pu16Height;
            *pu16Height ^= *pu16Width;
            *pu16Width ^= *pu16Height;
        }
        break;
        case EN_LOGO_ROTATE_180:
        {
            YUV444_TO_YUV420_PIXEL_MAPPING(pu8DesY, pu8DesUV, (*pu16Width - x), (*pu16Height - y - 1), *pu16Width,
                                           pu8SrcYUV, x, y, *pu16Width, *pu16Height);
        }
        break;
        case EN_LOGO_ROTATE_270:
        {
            YUV444_TO_YUV420_PIXEL_MAPPING(pu8DesY, pu8DesUV, y, (*pu16Width - x - 1), *pu16Height, pu8SrcYUV, x, y,
                                           *pu16Width, *pu16Height);
            *pu16Width ^= *pu16Height;
            *pu16Height ^= *pu16Width;
            *pu16Width ^= *pu16Height;
        }
        break;
        default:
            return;
    }
}

#if defined(CONFIG_JPD_HW)
#define YUV422_TO_YUV420_PIXEL_MAPPING(y_dst_addr, uv_dst_addr, dst_x, dst_y, dst_stride, src_addr, src_x, src_y, \
                                       src_w, src_h)                                                              \
    do                                                                                                            \
    {                                                                                                             \
        for (src_y = 0; src_y < src_h; src_y++)                                                                   \
        {                                                                                                         \
            for (src_x = 0; src_x < src_w; src_x++)                                                               \
            {                                                                                                     \
                *(((y_dst_addr) + (dst_y) * (dst_stride) + (dst_x))) =                                            \
                    *(((src_addr) + (src_y) * (src_w * 2) + (src_x * 2)));                                        \
                if (dst_stride == src_w)                                                                          \
                {                                                                                                 \
                    if ((src_y & 0x01))                                                                           \
                    {                                                                                             \
                        *(((uv_dst_addr) + ((dst_y - 1) >> 1) * (dst_stride) + (dst_x))) =                        \
                            *(((src_addr) + (src_y) * (src_w * 2) + (src_x * 2) + 1));                            \
                    }                                                                                             \
                }                                                                                                 \
                else                                                                                              \
                {                                                                                                 \
                    if ((src_y & 0x01))                                                                           \
                    {                                                                                             \
                        if (src_x & 0x01)                                                                         \
                        {                                                                                         \
                            *(((uv_dst_addr) + ((dst_y) >> 1) * (dst_stride) + (dst_x))) =                        \
                                *(((src_addr) + (src_y) * (src_w * 2) + (src_x * 2) + 1));                        \
                        }                                                                                         \
                    }                                                                                             \
                    else                                                                                          \
                    {                                                                                             \
                        if ((src_x % 2) == 0)                                                                     \
                        {                                                                                         \
                            *(((uv_dst_addr) + ((dst_y) >> 1) * (dst_stride) + (dst_x))) =                        \
                                *(((src_addr) + (src_y) * (src_w * 2) + (src_x * 2) + 1));                        \
                        }                                                                                         \
                    }                                                                                             \
                }                                                                                                 \
            }                                                                                                     \
        }                                                                                                         \
    } while (0)

void _BootLogoYuv422ToYuv420(u8 *pu8InBuf, u8 *pu8OutBuf, u16 *pu16Width, u16 *pu16Height, LogoRotation_e eRot)
{
    u16 x = 0, y = 0;

    u8 *pu8DesY = NULL, *pu8DesUV = NULL;
    u8 *pu8SrcYUV = NULL;

    pu8SrcYUV = pu8InBuf;

    pu8DesY  = pu8OutBuf;
    pu8DesUV = pu8DesY + (*pu16Width) * (*pu16Height);

    BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_JPD, "%s %d:: 422 To 420, In:%lx, Out:%lx, Width:%d, Height:%d\n", __FUNCTION__,
                 __LINE__, (unsigned long)pu8InBuf, (unsigned long)pu8OutBuf, *pu16Width, *pu16Height);

    switch (eRot)
    {
        case EN_LOGO_ROTATE_NONE:
        {
            YUV422_TO_YUV420_PIXEL_MAPPING(pu8DesY, pu8DesUV, x, y, *pu16Width, pu8SrcYUV, x, y, *pu16Width,
                                           *pu16Height);
        }
        break;
        case EN_LOGO_ROTATE_90:
        {
            YUV422_TO_YUV420_PIXEL_MAPPING(pu8DesY, pu8DesUV, (*pu16Height - y), x, *pu16Height, pu8SrcYUV, x, y,
                                           *pu16Width, *pu16Height);
            *pu16Width ^= *pu16Height;
            *pu16Height ^= *pu16Width;
            *pu16Width ^= *pu16Height;
        }
        break;
        case EN_LOGO_ROTATE_180:
        {
            YUV422_TO_YUV420_PIXEL_MAPPING(pu8DesY, pu8DesUV, (*pu16Width - x), (*pu16Height - y - 1), *pu16Width,
                                           pu8SrcYUV, x, y, *pu16Width, *pu16Height);
        }
        break;
        case EN_LOGO_ROTATE_270:
        {
            YUV422_TO_YUV420_PIXEL_MAPPING(pu8DesY, pu8DesUV, y, (*pu16Width - x - 1), *pu16Height, pu8SrcYUV, x, y,
                                           *pu16Width, *pu16Height);
            *pu16Width ^= *pu16Height;
            *pu16Height ^= *pu16Width;
            *pu16Width ^= *pu16Height;
        }
        break;
        default:
            return;
    }
}
#endif

static void _BootJpdYuvCtrl(unsigned long u32InBufSize, unsigned long u32InBuf, unsigned long u32OutBufSize,
                            unsigned long u32OutBuf, u16 *pu16OutWidth, u16 *pu16OutHeight, LogoRotation_e eRot)
{
#if defined(CONFIG_SSTAR_JPD)

#if defined(CONFIG_JPD_SW)

#if JPD_TIME_DEBUG
    unsigned long u32TimerStart    = 0;
    unsigned long u32TimerDecode   = 0;
    unsigned long u32Timer444To420 = 0;
    unsigned long u32TotalTime     = 0;

    u32TimerStart = get_timer(0);
#endif

    // Variables for the source jpg
    unsigned long u32JpgSize;
    u8 *          pu8JpgBuffer;

    // Variables for the decompressor itself
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr         jerr;

    // Variables for the output buffer, and how long each row is
    unsigned long u32BmpSize;
    u8 *          pu8BmpBuffer;

    unsigned long u32Yuv420Size;
    u8 *          pu8Yuv420Buffer;

    u16 u16RowStride, u16Width, u16Height, u16PixelSize;

    int rc; //, i, j;

    u32JpgSize = u32InBufSize;

    pu8JpgBuffer = (unsigned char *)u32InBuf;

    BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_JPD, "%s %d::  Create Decompress struct\n", __FUNCTION__, __LINE__);
    // Allocate a new decompress struct, with the default error handler.
    // The default error handler will exit() on pretty much any issue,
    // so it's likely you'll want to replace it or supplement it with
    // your own.
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_JPD, "%s %d::  Set memory buffer as source\n", __FUNCTION__, __LINE__);
    // Configure this decompressor to read its data from a memory
    // buffer starting at unsigned char *pu8JpgBuffer, which is u32JpgSize
    // long, and which must contain a complete jpg already.
    //
    // If you need something fancier than this, you must write your
    // own data source manager, which shouldn't be too hard if you know
    // what it is you need it to do. See jpeg-8d/jdatasrc.c for the
    // implementation of the standard jpeg_mem_src and jpeg_stdio_src
    // managers as examples to work from.
    jpeg_mem_src(&cinfo, pu8JpgBuffer, u32JpgSize);

    BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_JPD, "%s %d::  Read the JPEG header\n", __FUNCTION__, __LINE__);
    // Have the decompressor scan the jpeg header. This won't populate
    // the cinfo struct output fields, but will indicate if the
    // jpeg is valid.
    rc = jpeg_read_header(&cinfo, TRUE);

    if (rc != 1)
    {
        BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_ERR, "%s %d:: File does not seem to be a normal JPEG\n", __FUNCTION__,
                     __LINE__);
        return;
    }

    BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_JPD, "%s %d::  Initiate JPEG decompression\n", __FUNCTION__, __LINE__);

    // output color space is yuv444 packet
    cinfo.out_color_space = JCS_YCbCr;

    jpeg_start_decompress(&cinfo);

    u16Width     = cinfo.output_width;
    u16Height    = cinfo.output_height;
    u16PixelSize = cinfo.output_components;

    BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_JPD, "%s %d::  Image is %d by %d with %d components\n", __FUNCTION__, __LINE__,
                 u16Width, u16Height, u16PixelSize);

    u32BmpSize   = u16Width * u16Height * u16PixelSize;
    pu8BmpBuffer = (u8 *)malloc(u32BmpSize);

    if (pu8BmpBuffer == NULL)
    {
        BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_ERR, "%s %d:: malloc fail\n", __FUNCTION__, __LINE__);
        return;
    }

    BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_JPD, "%s %d:: BmpBuffer: 0x%lx\n", __FUNCTION__, __LINE__,
                 (unsigned long)pu8BmpBuffer);
    u32Yuv420Size   = u16Width * u16Height * 3 / 2;
    pu8Yuv420Buffer = (unsigned char *)(u32OutBuf);

    if (u32Yuv420Size > u32OutBufSize)
    {
        free(pu8BmpBuffer);
        BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_ERR, "%s %d::  Yuv420 OutBufSize not enough! expected=%lx, but get=%lx\n",
                     __FUNCTION__, __LINE__, u32Yuv420Size, u32OutBufSize);
        return;
    }

    u16RowStride = u16Width * u16PixelSize;

    BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_JPD, "%s %d:: Start reading scanlines\n", __FUNCTION__, __LINE__);
    while (cinfo.output_scanline < cinfo.output_height)
    {
        unsigned char *buffer_array[1];
        buffer_array[0] = pu8BmpBuffer + (cinfo.output_scanline) * u16RowStride;

        jpeg_read_scanlines(&cinfo, buffer_array, 1);
    }

    BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_JPD, "%s %d:: Done reading scanlines\n", __FUNCTION__, __LINE__);
    jpeg_finish_decompress(&cinfo);

    jpeg_destroy_decompress(&cinfo);

#if JPD_TIME_DEBUG
    u32TimerDecode = get_timer(0);
#endif
    BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_JPD, "%s %d:: End of decompression\n", __FUNCTION__, __LINE__);
    _BootLogoYuv444ToYuv420(pu8BmpBuffer, pu8Yuv420Buffer, &u16Width, &u16Height, eRot);
    *pu16OutWidth  = u16Width;
    *pu16OutHeight = u16Height;

#if JPD_TIME_DEBUG
    u32Timer444To420 = get_timer(0);
    u32TotalTime     = get_timer(u32TimerStart);
    // print the time consuming of JPD_SW
    // get_timer(base) return how many HZ passed since input base
    // here CONFIG_SYS_HZ = 1000, and time measurement unit is ms.
    printf(
        "==================JPD_SW statics==================\n  \
        decode time used    = %ld ms\n  \
        444To420 time used  = %ld ms\n  \
        total time used     = %ld ms\n",
        (u32TimerDecode - u32TimerStart) * 1000 / CONFIG_SYS_HZ,
        (u32Timer444To420 - u32TimerDecode) * 1000 / CONFIG_SYS_HZ, (u32TotalTime)*1000 / CONFIG_SYS_HZ);
#endif

    free(pu8BmpBuffer);

#elif defined(CONFIG_JPD_HW)

    MS_U32 u32DevId            = 0;
    MS_U16 u16Width            = 0;
    MS_U16 u16Height           = 0;
    MS_U32 u32Yuv420Size       = 0;

#if JPD_TIME_DEBUG
    MS_U32 u32TimerStart       = 0;
    MS_U32 u32TimerPrepare     = 0;
    MS_U32 u32TimerExtraHeader = 0;
    MS_U32 u32TimerDecode      = 0;
    MS_U32 u32Timer422To420    = 0;
    MS_U32 u32TotalTime        = 0;

    u32TimerStart = get_timer(0);
#endif

    void *pYuv422OutputAddr = NULL;
    u8 *  pu8Yuv420Buffer   = NULL;

    MHAL_JPD_DEV_HANDLE  pstJpdDevHandle = NULL; ///< might be void* or MS_U32
    MHAL_JPD_INST_HANDLE pstJpdInstCtx   = NULL;

    MHAL_JPD_InputBuf_t  stInputBuf;
    MHAL_JPD_JpgInfo_t   stJpgInfo;
    MHAL_JPD_JpgFrame_t  stJpgFrame;
    MHal_JPD_Addr_t      stJpdAddr;
    MHAL_JPD_MaxDecRes_t stJpdMaxinfo;
    MHAL_JPD_Status_e    eJpdStatusFlag;

    memset(&stInputBuf, 0x00, sizeof(stInputBuf));
    memset(&stJpgInfo, 0x00, sizeof(stJpgInfo));
    memset(&stJpgFrame, 0x00, sizeof(stJpgFrame));
    memset(&stJpdAddr, 0x00, sizeof(stJpdAddr));
    memset(&stJpdMaxinfo, 0x00, sizeof(stJpdMaxinfo));
    memset(&eJpdStatusFlag, 0x00, sizeof(eJpdStatusFlag));

    // InterBuffer allocate when create device

    // InputBuffer
    stJpdAddr.InputAddr     = (void *)u32InBuf;
    stJpdAddr.InputpVirAddr = stJpdAddr.InputAddr;
    stJpdAddr.InputSize     = u32InBufSize;

    // YUV422Buffer
    // We allocate YUV422Buffer when extra the width and height of image.

    if (0 != MHAL_JPD_CreateDevice(u32DevId, &pstJpdDevHandle))
    {
        BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_ERR, "%s %d:: MHAL_JPD_CreateDevice fail.\n", __FUNCTION__, __LINE__);
        return;
    }

    if (0 != MHAL_JPD_CreateInstance(pstJpdDevHandle, &pstJpdInstCtx))
    {
        BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_ERR, "%s %d:: MHAL_JPD_CreateInstance fail.\n", __FUNCTION__, __LINE__);
        return;
    }

    stJpdMaxinfo.u16MaxHeight    = MAX_DEC_HEIGHT;
    stJpdMaxinfo.u16MaxWidth     = MAX_DEC_WIDTH;
    stJpdMaxinfo.u16ProMaxHeight = MAX_DEC_HEIGHT;
    stJpdMaxinfo.u16ProMaxWidth  = MAX_DEC_WIDTH;

    if (0 != MHAL_JPD_SetParam(pstJpdInstCtx, E_JPD_PARAM_MAX_DEC_RES, &stJpdMaxinfo)) // set msg to sysinfo
    {
        BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_ERR, "%s %d:: MHAL_JPD_SetParam MAX_DEC_RES fail.\n", __FUNCTION__, __LINE__);
        return;
    }

    // We do not use Addr2 now
    stInputBuf.pVirtBufAddr1  = stJpdAddr.InputpVirAddr;
    stInputBuf.pVirtBufAddr2  = stJpdAddr.InputpVirAddr + stJpdAddr.InputSize;
    stInputBuf.u64PhyBufAddr1 = (MS_PHY)stJpdAddr.InputAddr;
    stInputBuf.u64PhyBufAddr2 = (MS_PHY)(stJpdAddr.InputAddr + stJpdAddr.InputSize);
    stInputBuf.u32BufSize1    = stJpdAddr.InputSize;
    stInputBuf.u32BufSize2    = 0;

    flush_dcache_range((unsigned long)(stInputBuf.pVirtBufAddr1),
                       (unsigned long)(stInputBuf.pVirtBufAddr1 + stJpdAddr.InputSize));

#if JPD_TIME_DEBUG
    u32TimerPrepare = get_timer(0);
#endif
    BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_JPD, "%s %d:: Start Extract Jpg Header Info.\n", __FUNCTION__, __LINE__);
    if (0 != MHAL_JPD_ExtractJpgInfo(pstJpdInstCtx, &stInputBuf, &stJpgInfo))
    {
        BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_ERR, "%s %d:: MHAL_JPD_Extract Jpg Info fail.\n", __FUNCTION__, __LINE__);
        return;
    }

    BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_JPD, "%s %d:: Get JpgInfo w = %d , h = %d.\n", __FUNCTION__, __LINE__,
                 stJpgInfo.u16AlignedWidth, stJpgInfo.u16AlignedHeight);
    stJpgFrame.stInputBuf.pVirtBufAddr1  = stJpdAddr.InputpVirAddr;
    stJpgFrame.stInputBuf.pVirtBufAddr2  = stJpdAddr.InputpVirAddr + stJpdAddr.InputSize;
    stJpgFrame.stInputBuf.u64PhyBufAddr1 = (MS_PHY)stJpdAddr.InputAddr;
    stJpgFrame.stInputBuf.u64PhyBufAddr2 = (MS_PHY)(stJpdAddr.InputAddr + stJpdAddr.InputSize);
    stJpgFrame.stInputBuf.u32BufSize1    = stJpdAddr.InputSize;
    stJpgFrame.stInputBuf.u32BufSize2    = 0;

    // Get aligned width and height
    u16Width       = stJpgInfo.u16AlignedWidth;
    u16Height      = stJpgInfo.u16AlignedHeight;
    *pu16OutWidth  = stJpgInfo.u16AlignedWidth;
    *pu16OutHeight = stJpgInfo.u16AlignedHeight;
    u32Yuv420Size  = u16Width * u16Height * 3 / 2;

    // YUV422Buffer
    stJpdAddr.OutputSize = stJpgInfo.u16AlignedWidth * stJpgInfo.u16AlignedHeight * 2;
    pYuv422OutputAddr    = malloc(stJpdAddr.OutputSize + ALIGNMENT_NEED);
    if (!pYuv422OutputAddr)
    {
        BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_ERR, "%s %d:: Can not allocate pYuv422OutputAddr.\n", __FUNCTION__, __LINE__);
        free(pYuv422OutputAddr);
        return;
    }
    memset(pYuv422OutputAddr, 0x0, stJpdAddr.OutputSize);
    // Align output buffer, we use 32 bytes alignment
    stJpdAddr.OutputAddr  = (void *)ALIGN_UP((unsigned long)pYuv422OutputAddr, ALIGNMENT_NEED);
    stJpdAddr.OutpVirAddr = stJpdAddr.OutputAddr;

    stJpgFrame.stOutputBuf.pVirtBufAddr[0]  = stJpdAddr.OutpVirAddr;
    stJpgFrame.stOutputBuf.u64PhyBufAddr[0] = (MS_PHY)stJpdAddr.OutputAddr;
    stJpgFrame.stOutputBuf.u32BufSize       = stJpdAddr.OutputSize;

    stJpgFrame.stOutputBuf.eScaleDownMode  = E_MHAL_JPD_SCALE_DOWN_ORG;
    stJpgFrame.stOutputBuf.eOutputFmt      = E_MHAL_JPD_OUTPUT_FMT_YUV422;
    stJpgFrame.stOutputBuf.u32BufStride[0] = stJpgInfo.u16AlignedWidth * 2;
    stJpgFrame.stOutputBuf.eOutputMode     = E_MHAL_JPD_OUTPUT_FRAME;
    stJpgFrame.stOutputBuf.u32LineNum      = 0;
    stJpgFrame.stOutputBuf.bProtectEnable  = 0;
    stJpgFrame.stOutputBuf.pCmdQ           = NULL;

#if JPD_TIME_DEBUG
    u32TimerExtraHeader                    = get_timer(0);
#endif
    BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_JPD, "%s %d:: Start Decode One Frame.\n", __FUNCTION__, __LINE__);
    if (0 != MHAL_JPD_StartDecodeOneFrame(pstJpdInstCtx, &stJpgFrame))
    {
        BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_ERR, "%s %d:: MHAL_JPD_StartDecodeOneFrame fail.\n", __FUNCTION__, __LINE__);
        return;
    }

    BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_JPD, "Input buffer addr         = 0x%lx \n", u32InBuf);
    BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_JPD, "u32InBufSize              = 0x%lx \n", u32InBufSize);
    BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_JPD, "Yuv420Buffer addr         = 0x%lx \n", u32OutBuf);
    BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_JPD, "Yuv420Buffer size in need = 0x%x  \n", u32Yuv420Size);
    BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_JPD, "Get u32OutBufSize         = 0x%lx \n", u32OutBufSize);
    BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_JPD, "Yuv422 Output buffer addr = 0x%lx \n", (unsigned long)stJpdAddr.OutputAddr);
    BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_JPD, "Yuv422 OutBufSize         = 0x%x  \n", stJpdAddr.OutputSize);

    if (MHAL_JPD_CheckDecodeStatus(pstJpdInstCtx, &eJpdStatusFlag) == 0)
    {
        BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_JPD, "%s:%d JPD Done.\n", __FUNCTION__, __LINE__);
    }
    else
    {
        BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_ERR, "%s:%d JPD Fail!!!\n", __FUNCTION__, __LINE__);
    }

#if JPD_TIME_DEBUG
    u32TimerDecode = get_timer(0);
#endif

    if (u32Yuv420Size > u32OutBufSize)
    {
        free(pYuv422OutputAddr);
        BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_ERR, "%s %d:: Yuv420 OutBufSize not enough! expected=%x, but get=%lx\n",
                     __FUNCTION__, __LINE__, u32Yuv420Size, u32OutBufSize);
        return;
    }
    pu8Yuv420Buffer = (u8 *)(u32OutBuf);
    _BootLogoYuv422ToYuv420(pYuv422OutputAddr, pu8Yuv420Buffer, &u16Width, &u16Height, eRot);

    flush_dcache_range((unsigned long)(pu8Yuv420Buffer),
                       (unsigned long)(pu8Yuv420Buffer + u16Width * u16Height * 3 / 2));

    MHAL_JPD_DestroyInstance(pstJpdInstCtx);
    MHAL_JPD_DestroyDevice(pstJpdDevHandle);
    free(pYuv422OutputAddr);

#if JPD_TIME_DEBUG
    u32Timer422To420 = get_timer(0);
    u32TotalTime     = get_timer(u32TimerStart);

    // print the time consuming of JPD_HW
    // get_timer(base) return how many HZ passed since input base
    // here CONFIG_SYS_HZ = 1000, and time measurement unit is ms.
    printf(
        "==================JPD_HW statics==================\n  \
        prepare time used       = %d ms\n  \
        extra header time used  = %d ms\n  \
        decode time used        = %d ms\n  \
        422To420 time used      = %d ms\n  \
        total time used         = %d ms\n",
        (u32TimerPrepare - u32TimerStart) * 1000 / CONFIG_SYS_HZ,
        (u32TimerExtraHeader - u32TimerPrepare) * 1000 / CONFIG_SYS_HZ,
        (u32TimerDecode - u32TimerExtraHeader) * 1000 / CONFIG_SYS_HZ,
        (u32Timer422To420 - u32TimerDecode) * 1000 / CONFIG_SYS_HZ, (u32TotalTime)*1000 / CONFIG_SYS_HZ);
#endif

#endif
#endif
}

#if defined(CONFIG_SSTAR_JPD)
int bootJpd(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    if (argc != 6)
    {
        BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_ERR,
                     "usage: bootjpd [inBufAddr] [inBufSize] [ImgWidth] [ImgHeight] [rotate]\n");
        return -1;
    }

    unsigned long  u32InBufAddr = (unsigned long)simple_strtoul(argv[1], NULL, 0);
    unsigned long  u32InBufSize = (unsigned long)simple_strtoul(argv[2], NULL, 0);
    u16            u16ImgWidth  = (u16)simple_strtoul(argv[3], NULL, 0);
    u16            u16ImgHeight = (u16)simple_strtoul(argv[4], NULL, 0);
    LogoRotation_e eRotate      = (LogoRotation_e)simple_strtoul(argv[5], NULL, 0);

    unsigned long u32OutBufSize = ALIGN_UP(u16ImgWidth, 16) * ALIGN_UP(u16ImgHeight, 16) * 3 / 2;

    // Write jpeg es to inBufAddr by tftp when Ut test
    void *pOutBufAddr = malloc(u32OutBufSize);
    if (!pOutBufAddr)
    {
        BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_ERR, "%s %d:: Can not allocate pOutBuf, Size = %lx.\n", __FUNCTION__, __LINE__,
                     u32OutBufSize);
        free(pOutBufAddr);
        return -1;
    }
    BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_JPD, "Inbuffer addr = 0x%lx \n", u32InBufAddr);
    BOOT_JPD_DBG(BOOT_JPD_DBG_LEVEL_JPD, "pOutBufAddr = 0x%p \n", pOutBufAddr);
    _BootJpdYuvCtrl(u32InBufSize, u32InBufAddr, u32OutBufSize, (unsigned long)(pOutBufAddr), &u16ImgWidth,
                    &u16ImgHeight, eRotate);

    // Do not free pOutBufAddr for Ut check.
    return 0;
}

U_BOOT_CMD(bootjpd, CONFIG_SYS_MAXARGS, 1, bootJpd, "bootjpd [inBufAddr] [inBufSize] [ImgWidth] [ImgHeight] [rotate]\n",
           NULL);

#endif
