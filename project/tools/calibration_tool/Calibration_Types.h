////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2012 SigmaStar Technology Corp.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// SigmaStar Technology Corp. and be kept in strict confidence
// (SigmaStar Confidential Information) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of SigmaStar Confidential
// Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

/*
*    Calibration_Types.h
*
*    Created on: Mar 07, 2018
*        Author: Jeffrey Chou
*/

#ifndef __CALIBRATION_TYPES_H__
#define __CALIBRATION_TYPES_H__

#ifdef __cplusplus
extern "C"
{
#endif
    /// data type unsigned char, data length 1 byte
    typedef unsigned char               u8;                                 // 1 byte
                                                                            /// data type unsigned short, data length 2 byte
    typedef unsigned short              u16;                                // 2 bytes
                                                                            /// data type unsigned int, data length 4 byte
#if INTPTR_MAX == INT32_MAX
    typedef unsigned int                u32;
#else
    typedef unsigned long               u32;                                // 4 bytes
#endif
                                                                            /// data type unsigned int, data length 8 byte
    typedef unsigned long long          u64;                                // 8 bytes
                                                                            /// data type signed char, data length 1 byte
    typedef signed char                 s8;                                 // 1 byte
                                                                            /// data type signed short, data length 2 byte
    typedef signed short                s16;                                // 2 bytes
                                                                            /// data type signed int, data length 4 byte
    typedef signed long                 s32;                                // 4 bytes
                                                                            /// data type signed int, data length 8 byte
    typedef signed long long            s64;                                // 8 bytes

    typedef float                       f32;

    #define TRUE                        (1)
    #define FALSE                       (0)

    #define FAIL                        (-1)
    #define SUCCESS                     (0)
    #define NOT_ENOUNGH_FILES_FOUND     (1)
    #define OPEN_RAW_FILE_FAIL          (2)

    #define FILENAME_PATH_MAX           (256)

    #define min(a,b)                    (((a) >= (b)) ? (b) : (a))
    #define max(a,b)                    (((a) >= (b)) ? (a) : (b))
    #define minmax(v,a,b)               (((v) < (a)) ? (a) : ((v) > (b)) ? (b) : (v))
    #define BIT_GET_VALUE(v, num)       (((v) >> (num)) & 1)
    #define BIT_CTRL_0(v, num)          ((v) & ~(1 << (num)))
    #define BIT_CTRL_1(v, num)          ((v) | (1 << (num)))

#ifdef __cplusplus
}    //end of extern C
#endif

#endif