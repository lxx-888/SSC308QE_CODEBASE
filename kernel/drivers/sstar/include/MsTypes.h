/*
 * MsTypes.h- Sigmastar
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
#ifndef _MS_TYPES_H_
#define _MS_TYPES_H_

#ifdef CONFIG_ENABLE_MENUCONFIG
#include "autoconf.h"
#endif

#include <stddef.h>
//-------------------------------------------------------------------------------------------------
//  System Data Type
//-------------------------------------------------------------------------------------------------
#define SUPPORT_ANDROID_L_PHYVIR_TYPE

#if defined(UFO_PUBLIC_HEADER_300)
/// data type unsigned char, data length 1 byte
typedef unsigned char MS_U8; // 1 byte
/// data type unsigned short, data length 2 byte
typedef unsigned short MS_U16; // 2 bytes
/// data type unsigned int, data length 4 byte
typedef unsigned long MS_U32; // 4 bytes
/// data type unsigned int, data length 8 byte
typedef unsigned long long MS_U64; // 8 bytes
/// data type signed char, data length 1 byte
typedef signed char MS_S8; // 1 byte
/// data type signed short, data length 2 byte
typedef signed short MS_S16; // 2 bytes
/// data type signed int, data length 4 byte
typedef signed long MS_S32; // 4 bytes
/// data type signed int, data length 8 byte
typedef signed long long MS_S64; // 8 bytes
/// data type float, data length 4 byte
typedef float MS_FLOAT; // 4 bytes
/// data type pointer content
typedef MS_U32 MS_VIRT; // 8 bytes
/// data type hardware physical address
typedef MS_U32 MS_PHYADDR; // 8 bytes
/// data type 64bit physical address
typedef MS_U32 MS_PHY; // 8 bytes
/// data type size_t
typedef size_t MS_SIZE; // 8 bytes
/// definition for MS_BOOL
typedef unsigned char MS_BOOL; // 1 byte
/// print type  MPRI_DEC
#define MPRI_DEC "%ld"
/// print type  MPRI_UDEC
#define MPRI_UDEC "%lu"
/// print type  MPRI_HEX
#define MPRI_HEX "%lx"
#elif defined(UFO_PUBLIC_HEADER_212)
/// data type unsigned char, data length 1 byte
typedef unsigned char MS_U8; // 1 byte
/// data type unsigned short, data length 2 byte
typedef unsigned short MS_U16; // 2 bytes
/// data type unsigned int, data length 4 byte
typedef unsigned long MS_U32; // 4 bytes
/// data type unsigned int, data length 8 byte
typedef unsigned long long MS_U64; // 8 bytes
/// data type signed char, data length 1 byte
typedef signed char MS_S8; // 1 byte
/// data type signed short, data length 2 byte
typedef signed short MS_S16; // 2 bytes
/// data type signed int, data length 4 byte
typedef signed long MS_S32; // 4 bytes
/// data type signed int, data length 8 byte
typedef signed long long MS_S64; // 8 bytes
/// data type float, data length 4 byte
typedef float MS_FLOAT; // 4 bytes
/// data type pointer content
typedef MS_U32 MS_VIRT; // 4 bytes
/// data type hardware physical address
typedef MS_U32 MS_PHYADDR; // 8 bytes
/// data type 64bit physical address
typedef MS_U32 MS_PHY; // 4 bytes
                       /// data type null pointer
/// definition for MS_BOOL
typedef unsigned char MS_BOOL;
/// data type size_t
typedef MS_U32 MS_SIZE; // 4 bytes
#else
#if defined(__aarch64__)
/// data type unsigned char, data length 1 byte
typedef unsigned char MS_U8; // 1 byte
/// data type unsigned short, data length 2 byte
typedef unsigned short MS_U16; // 2 bytes
/// data type unsigned int, data length 4 byte
typedef unsigned int MS_U32; // 4 bytes
/// data type unsigned int, data length 8 byte
typedef unsigned long long MS_U64; // 8 bytes
/// data type signed char, data length 1 byte
typedef signed char MS_S8; // 1 byte
/// data type signed short, data length 2 byte
typedef signed short MS_S16; // 2 bytes
/// data type signed int, data length 4 byte
typedef signed int MS_S32; // 4 bytes
/// data type signed int, data length 8 byte
typedef signed long long MS_S64; // 8 bytes
/// data type float, data length 4 byte
typedef float MS_FLOAT; // 4 bytes
/// data type pointer content
typedef size_t MS_VIRT; // 8 bytes
/// data type hardware physical address
typedef size_t MS_PHYADDR; // 8 bytes
/// data type size_t
typedef size_t MS_SIZE; // 8 bytes
/// data type 64bit physical address
typedef MS_U64 MS_PHY; // 8 bytes
/// definition for MS_BOOL
typedef unsigned char MS_BOOL; // 1 byte
                               /// print type  MPRI_PHY
#define MPRI_PHY  "%x"
                               /// print type  MPRI_VIRT
#define MPRI_VIRT "%tx"
#elif defined(MSOS_TYPE_NUTTX)
/// data type unsigned char, data length 1 byte
typedef unsigned char MS_U8; // 1 byte
/// data type unsigned short, data length 2 byte
typedef unsigned short MS_U16; // 2 bytes
/// data type unsigned int, data length 4 byte
typedef unsigned long MS_U32; // 4 bytes
/// data type unsigned int, data length 8 byte
typedef unsigned long long MS_U64; // 8 bytes
/// data type signed char, data length 1 byte
typedef signed char MS_S8; // 1 byte
/// data type signed short, data length 2 byte
typedef signed short MS_S16; // 2 bytes
/// data type signed int, data length 4 byte
typedef signed long MS_S32; // 4 bytes
/// data type signed int, data length 8 byte
typedef signed long long MS_S64; // 8 bytes
/// data type float, data length 4 byte
typedef float MS_FLOAT; // 4 bytes
/// data type pointer content
typedef MS_U32 MS_VIRT; // 8 bytes
/// data type hardware physical address
typedef MS_U32 MS_PHYADDR; // 8 bytes
/// data type 64bit physical address
typedef MS_U32 MS_PHY; // 8 bytes
/// data type size_t
typedef MS_U32 MS_SIZE; // 8 bytes
/// definition for MS_BOOL
typedef unsigned char MS_BOOL; // 1 byte
                               /// print type  MPRI_PHY
#define MPRI_PHY  "%x"
                               /// print type  MPRI_PHY
#define MPRI_VIRT "%tx"
#else
#if (defined(CONFIG_PURE_SN) || defined(CONFIG_MBOOT))
/// data type unsigned char, data length 1 byte
typedef unsigned char MS_U8; // 1 byte
/// data type unsigned short, data length 2 byte
typedef unsigned short MS_U16; // 2 bytes
/// data type unsigned int, data length 4 byte
typedef unsigned int MS_U32; // 4 bytes
/// data type unsigned int, data length 8 byte
typedef unsigned long long MS_U64; // 8 bytes
/// data type signed char, data length 1 byte
typedef signed char MS_S8; // 1 byte
/// data type signed short, data length 2 byte
typedef signed short MS_S16; // 2 bytes
/// data type signed int, data length 4 byte
typedef signed int MS_S32; // 4 bytes
/// data type signed int, data length 8 byte
typedef signed long long MS_S64; // 8 bytes
/// data type float, data length 4 byte
typedef float MS_FLOAT; // 4 bytes
/// data type pointer content
typedef MS_U32 MS_VIRT; // 8 bytes
/// data type hardware physical address
typedef MS_U32 MS_PHYADDR; // 8 bytes
/// data type 64bit physical address
typedef MS_U32 MS_PHY; // 8 bytes
/// data type size_t
typedef MS_U32 MS_SIZE; // 8 bytes
/// definition for MS_BOOL
typedef unsigned char MS_BOOL; // 1 byte
                               /// print type  MPRI_PHY
#define MPRI_PHY  "%x"
                               /// print type  MPRI_PHY
#define MPRI_VIRT "%tx"
#else
/// data type unsigned char, data length 1 byte
typedef unsigned char MS_U8; // 1 byte
/// data type unsigned short, data length 2 byte
typedef unsigned short MS_U16; // 2 bytes
/// data type unsigned int, data length 4 byte
typedef unsigned int MS_U32; // 4 bytes
/// data type unsigned int, data length 8 byte
typedef unsigned long long MS_U64; // 8 bytes
/// data type signed char, data length 1 byte
typedef signed char MS_S8; // 1 byte
/// data type signed short, data length 2 byte
typedef signed short MS_S16; // 2 bytes
/// data type signed int, data length 4 byte
typedef signed int MS_S32; // 4 bytes
/// data type signed int, data length 8 byte
typedef signed long long MS_S64; // 8 bytes
/// data type float, data length 4 byte
typedef float MS_FLOAT; // 4 bytes
/// data type pointer content
typedef size_t MS_VIRT; // 8 bytes
/// data type hardware physical address
typedef MS_U64 MS_PHYADDR; // 8 bytes
/// data type 64bit physical address
typedef MS_U64 MS_PHY; // 8 bytes
/// data type size_t
typedef size_t MS_SIZE; // 8 bytes
/// definition for MS_BOOL
typedef unsigned char MS_BOOL; // 1 byte
                               /// print type  MPRI_PHY
#define MPRI_PHY  "%x"
                               /// print type  MPRI_PHY
#define MPRI_VIRT "%tx"
#endif
#endif
#endif
#ifdef NULL
#undef NULL
#endif
#define NULL 0

//-------------------------------------------------------------------------------------------------
//  Software Data Type
//-------------------------------------------------------------------------------------------------

/// definition for VOID
typedef void VOID;
/// definition for FILEID
typedef MS_S32 FILEID;

//[TODO] use MS_U8, ... instead
// data type for 8051 code
// typedef MS_U16                      WORD;
// typedef MS_U8                       BYTE;

#ifndef MSOS_TYPE_LINUX_KERNEL
#ifndef true
/// definition for true
#define true 1
/// definition for false
#define false 0
#endif
#endif // End undef MSOS_TYPE_LINUX_KERNEL

#if !defined(TRUE) && !defined(FALSE)
/// definition for TRUE
#define TRUE 1
/// definition for FALSE
#define FALSE 0
#endif

#if defined(ENABLE) && (ENABLE != 1)
#warning ENALBE is not 1
#else
#define ENABLE 1
#endif

#if defined(DISABLE) && (DISABLE != 0)
#warning DISABLE is not 0
#else
#define DISABLE 0
#endif

/// Define MS FB Format, to share with GE,GOP
///  FIXME THE NAME NEED TO BE REFINED, AND MUST REMOVE UNNESSARY FMT
typedef enum
{
    /// color format I1
    E_MS_FMT_I1 = 0x0,
    /// color format I2
    E_MS_FMT_I2 = 0x1,
    /// color format I4
    E_MS_FMT_I4 = 0x2,
    /// color format palette 256(I8)
    E_MS_FMT_I8 = 0x4,
    /// color format blinking display
    E_MS_FMT_FaBaFgBg2266 = 0x6,
    /// color format for blinking display format
    E_MS_FMT_1ABFgBg12355 = 0x7,
    /// color format RGB565
    E_MS_FMT_RGB565 = 0x8,
    /// color format ARGB1555
    /// @note <b>[URANUS] <em>ARGB1555 is only RGB555</em></b>
    E_MS_FMT_ARGB1555 = 0x9,
    /// color format ARGB4444
    E_MS_FMT_ARGB4444 = 0xa,
    /// color format ARGB1555 DST
    E_MS_FMT_ARGB1555_DST = 0xc,
    /// color format YUV422
    E_MS_FMT_YUV422 = 0xe,
    /// color format ARGB8888
    E_MS_FMT_ARGB8888 = 0xf,
    /// color format RGBA5551
    E_MS_FMT_RGBA5551 = 0x10,
    /// color format RGBA4444
    E_MS_FMT_RGBA4444 = 0x11,
    /// Start of New color define
    /// color format BGRA5551
    E_MS_FMT_BGRA5551 = 0x12,
    /// color format ABGR1555
    E_MS_FMT_ABGR1555 = 0x13,
    /// color format ABGR4444
    E_MS_FMT_ABGR4444 = 0x14,
    /// color format BGRA4444
    E_MS_FMT_BGRA4444 = 0x15,
    /// color format BGR565
    E_MS_FMT_BGR565 = 0x16,
    /// color format RGBA8888
    E_MS_FMT_RGBA8888 = 0x1d,
    /// color format BGRA8888
    E_MS_FMT_BGRA8888 = 0x1e,
    /// color format ABGR8888
    E_MS_FMT_ABGR8888 = 0x1f,
    /// color format AYUV8888
    E_MS_FMT_AYUV8888 = 0x20,

    E_MS_FMT_GENERIC = 0xFFFF,

} MS_ColorFormat;

typedef union _MSIF_Version
{
    struct _DDI
    {
        MS_U8  tag[4];
        MS_U8  type[2];
        MS_U16 customer;
        MS_U16 model;
        MS_U16 chip;
        MS_U8  cpu;
        MS_U8  name[4];
        MS_U8  version[2];
        MS_U8  build[2];
        MS_U8  change[8];
        MS_U8  os;
    } DDI;
    struct _MW
    {
        MS_U8  tag[4];
        MS_U8  type[2];
        MS_U16 customer;
        MS_U16 mod;
        MS_U16 chip;
        MS_U8  cpu;
        MS_U8  name[4];
        MS_U8  version[2];
        MS_U8  build[2];
        MS_U8  changelist[8];
        MS_U8  os;
    } MW;
    struct _APP
    {
        MS_U8 tag[4];
        MS_U8 type[2];
        MS_U8 id[4];
        MS_U8 quality;
        MS_U8 version[4];
        MS_U8 time[6];
        MS_U8 changelist[8];
        MS_U8 reserve[3];
    } APP;
} MSIF_Version;

typedef struct _MS_SW_VERSION_INFO
{
    char UtopiaBspVersion[8]; // Utopia BSP Version
    char MajorVersion[4];     // Major Version Number
    char MinorVersion[4];     // Minor Version Number
    char ChangeList_API[16];  // Sync Perforce Change List Number in API Folder
    char ChangeList_DRV[16];  // Sync Perforce Change List Number in DRV Folder
    char ChangeList_HAL[16];  // Sync Perforce Change List Number in HAL Folder

} MS_SW_VERSION_INFO;

typedef struct _MS_SW_VERSION_NUM
{
    char   UtopiaBspVersion[32];
    MS_U32 UtopiaVerNum;
} MS_SW_VERSION_NUM;

/*
 * function pointers
 */
typedef MS_U32 (*FUtopiaOpen)(void** ppInstance, const void* const pAttribute);
typedef MS_U32 (*FUtopiaIOctl)(void* pInstance, MS_U32 u32Cmd, void* const pArgs);
typedef MS_U32 (*FUtopiaClose)(void* pInstance);
typedef MS_U32 (*FUtopiaSTR)(MS_U32 u32PowerState, void* pModule);
typedef MS_U32 (*FUtopiaMdbIoctl)(MS_U32 cmd, const void* const pArgs);

#ifndef MSOS_TYPE_DEF_MSOSATTRIBUTE
typedef enum
{
    E_MSOS_PRIORITY, ///< Priority-order suspension
    E_MSOS_FIFO,     ///< FIFO-order suspension
} MsOSAttribute;
#define MSOS_TYPE_DEF_MSOSATTRIBUTE
#endif

#define DLL_PACKED __attribute__((__packed__))

#endif // _MS_TYPES_H_
