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

/*
 * The full name of 'ptree' is 'Pipeline tree', which use the idea of 'Amigos'
 * for reference and auther is 'pedro.peng' from Sigmastar.
 */

#ifndef __PTREE_API_ISP_H__
#define __PTREE_API_ISP_H__

enum PTREE_API_ISP_CMD_e
{
    E_PTREE_API_ISP_CMD_CUS3A_SET_AE_PARAM,
    E_PTREE_API_ISP_CMD_MAX
};

typedef struct PTREE_API_ISP_Cus3ASetAeParam_s
{
    unsigned int size;               // struct size
    unsigned int change;             // if true, apply this result to hw register
    unsigned int shutter;            // Shutter in ns
    unsigned int sensorGain;         // Sensor gain, 1X = 1024
    unsigned int ispGain;            // ISP gain, 1X = 1024
    unsigned int shutterHdrShort;    // Shutter in ns
    unsigned int sensorGainHdrShort; // Sensor gain, 1X = 1024
    unsigned int ispGainHdrShort;    // ISP gain, 1X = 1024
    unsigned int i4BVx16384;         // Bv * 16384 in APEX system, EV = Av + Tv = Sv + Bv
    unsigned int avgY;               // frame brightness
    unsigned int hdrRatio;           // hdr ratio, 1X = 1024, compatible 2F
    unsigned int hdrRatio1;          // hdr ratio, 1X = 1024, for 3F
    unsigned int fNx10;              // F number * 10
    unsigned int debandFPS;          // Target fps when running auto debanding
    unsigned int weightY;            // frame brightness with ROI weight
    unsigned int gmBlendRatio;       // Adaptive Gamma Blending Ratio from AE
} __attribute__((packed, aligned(1))) PTREE_API_ISP_Cus3ASetAeParam_t;

typedef struct PTREE_API_ISP_Cus3AGetDoAeCount_s
{
    unsigned int currentAECnt; // AE done count
} PTREE_API_ISP_Cus3AGetDoAeCount_t;

#endif //__PTREE_API_ISP_H__
