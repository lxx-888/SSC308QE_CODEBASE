/*
 * mi_common_datatype.h- Sigmastar
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

#ifndef _MI_COMMON_DATATYPE_H_
#define _MI_COMMON_DATATYPE_H_

//-------------------------------------------------------------------------------------------------
//  System Data Type
//-------------------------------------------------------------------------------------------------
/// data type unsigned char, data length 1 byte
typedef unsigned char MI_U8; // 1 byte
/// data type unsigned short, data length 2 byte
typedef unsigned short MI_U16; // 2 bytes
/// data type unsigned int, data length 4 byte
typedef unsigned int MI_U32; // 4 bytes
/// data type unsigned int, data length 8 byte
typedef unsigned long long MI_U64; // 8 bytes
/// data type signed char, data length 1 byte
typedef signed char MI_S8; // 1 byte
/// data type signed short, data length 2 byte
typedef signed short MI_S16; // 2 bytes
/// data type signed int, data length 4 byte
typedef signed int MI_S32; // 4 bytes
/// data type signed int, data length 8 byte
typedef signed long long MI_S64; // 8 bytes
/// data type float, data length 4 byte
typedef float MI_FLOAT; // 4 bytes
/// data type 64bit physical address
typedef unsigned long long MI_PHY; // 8 bytes
/// data type pointer content
typedef unsigned long MI_VIRT; // 4 bytes when 32bit toolchain, 8 bytes when 64bit toolchain.

typedef unsigned char MI_BOOL;

//#ifdef __SIZEOF_INT128__
// typedef __uint128_t MI_VIRTx2; // MI_VIRT length x2
//#else
typedef MI_U64 MI_VIRTx2; // MI_VIRT length x2
//#endif

typedef void*  MI_PTR;
typedef MI_U64 MI_PTR64;

typedef enum
{
    E_MI_MODULE_ID_IVE      = 0,
    E_MI_MODULE_ID_VDF      = 1,
    E_MI_MODULE_ID_VENC     = 2,
    E_MI_MODULE_ID_RGN      = 3,
    E_MI_MODULE_ID_AI       = 4,
    E_MI_MODULE_ID_AO       = 5,
    E_MI_MODULE_ID_VIF      = 6,
    E_MI_MODULE_ID_VPE      = 7,
    E_MI_MODULE_ID_VDEC     = 8,
    E_MI_MODULE_ID_SYS      = 9,
    E_MI_MODULE_ID_FB       = 10,
    E_MI_MODULE_ID_HDMI     = 11,
    E_MI_MODULE_ID_DIVP     = 12,
    E_MI_MODULE_ID_GFX      = 13,
    E_MI_MODULE_ID_VDISP    = 14,
    E_MI_MODULE_ID_DISP     = 15,
    E_MI_MODULE_ID_OS       = 16,
    E_MI_MODULE_ID_IAE      = 17,
    E_MI_MODULE_ID_MD       = 18,
    E_MI_MODULE_ID_OD       = 19,
    E_MI_MODULE_ID_SHADOW   = 20,
    E_MI_MODULE_ID_WARP     = 21,
    E_MI_MODULE_ID_UAC      = 22,
    E_MI_MODULE_ID_LDC      = 23,
    E_MI_MODULE_ID_SD       = 24,
    E_MI_MODULE_ID_PANEL    = 25,
    E_MI_MODULE_ID_CIPHER   = 26,
    E_MI_MODULE_ID_SNR      = 27,
    E_MI_MODULE_ID_WLAN     = 28,
    E_MI_MODULE_ID_IPU      = 29,
    E_MI_MODULE_ID_MIPITX   = 30,
    E_MI_MODULE_ID_GYRO     = 31,
    E_MI_MODULE_ID_JPD      = 32,
    E_MI_MODULE_ID_ISP      = 33,
    E_MI_MODULE_ID_SCL      = 34,
    E_MI_MODULE_ID_WBC      = 35,
    E_MI_MODULE_ID_DSP      = 36,
    E_MI_MODULE_ID_PCIE     = 37,
    E_MI_MODULE_ID_DUMMY    = 38,
    E_MI_MODULE_ID_NIR      = 39,
    E_MI_MODULE_ID_DPU      = 40,
    E_MI_MODULE_ID_HVP      = 41,
    E_MI_MODULE_ID_HDMIRX   = 42,
    E_MI_MODULE_ID_PSPI     = 43,
    E_MI_MODULE_ID_IQSERVER = 44,
    // E_MI_MODULE_ID_SED  = 29,
    E_MI_MODULE_ID_DEBUG = 45,
    E_MI_MODULE_ID_MAX,
} MI_ModuleId_e;

#endif ///_MI_COMMON_DATATYPE_H_
