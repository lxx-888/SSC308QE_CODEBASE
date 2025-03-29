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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "mi_sys.h"
#include "mi_sensor.h"
#include "amigos_module_init.h"
#include "amigos_module_snr9931.h"
#include "st_dh9931.h"

AmigosModuleSnr9931::AmigosModuleSnr9931(const std::string &strInSection)
    : AmigosSurfaceSnr9931(strInSection), AmigosModuleMiBase(this)
{
    AMIGOS_INFO("func: %s\n", __FUNCTION__);
}
AmigosModuleSnr9931::~AmigosModuleSnr9931()
{
    AMIGOS_INFO("func: %s\n", __FUNCTION__);
}
unsigned int AmigosModuleSnr9931::GetModId() const
{
    return 0;
}
unsigned int AmigosModuleSnr9931::GetInputType(unsigned int port) const
{
    return 0;
}
unsigned int AmigosModuleSnr9931::GetOutputType(unsigned int port) const
{
    return 0;
}
void AmigosModuleSnr9931::_Init()
{
    MI_U8 u8ChipIndex=0;
    MI_U32 u32ResCount = 0;
    MI_U8 u8ResIndex =0;
    cus_camsensor_res stRes;
    AmigosModuleMiBase::stSensorDrvInfo_t stSnrDrvInfo;

    memset(&stRes, 0x0, sizeof(cus_camsensor_res));
    memset(&stSnrDrvInfo, 0, sizeof(stSensorDrvInfo_t));

    for (auto it = mapSnr9931Info.begin(); it != mapSnr9931Info.end(); it++)
    {
        u8ChipIndex = it->second.intADIndex;
        DHC_DH9931_Init(u8ChipIndex);
        Cus_GetVideoResNum(u8ChipIndex, &u32ResCount);
        for(u8ResIndex = 0; u8ResIndex < u32ResCount; u8ResIndex++)
        {
            Cus_GetVideoRes(u8ChipIndex, u8ResIndex, &stRes);
            AMIGOS_INFO("index %d, Crop(%d,%d,%d,%d), outputsize(%d,%d), maxfps %d, minfps %d, ResDesc %s\n",
                        u8ResIndex,
                        stRes.stCropRect.u16X, stRes.stCropRect.u16Y, stRes.stCropRect.u16Width,stRes.stCropRect.u16Height,
                        stRes.stOutputSize.u16Width, stRes.stOutputSize.u16Height,
                        stRes.u32MaxFps,stRes.u32MinFps,
                        stRes.strResDesc);
        }
        if(it->second.intSensorRes >= (int)u32ResCount)
        {
            AMIGOS_ERR("choice err res %d > =cnt %d\n", it->second.intSensorRes, u32ResCount);
            assert(0);
        }
        AMIGOS_INFO("You choose sensor res is %d\n", it->second.intSensorRes);
        Cus_SetVideoRes(u8ChipIndex, it->second.intSensorRes);
        Cus_GetSnrPadInfo(u8ChipIndex, &stSnrDrvInfo.stPadInfo);
        Cus_GetSnrPlaneInfo(u8ChipIndex, 0, &stSnrDrvInfo.stSnrPlaneInfo);
        UpdateSensorInfo(stSnrDrvInfo, (unsigned int)it->second.intSensorId);
    }
    Cus_SnrEnable();
}
void AmigosModuleSnr9931::_Deinit()
{
    MI_U8 u8ChipIndex=0;
    for (auto it = mapSnr9931Info.begin(); it != mapSnr9931Info.end(); it++)
    {
        u8ChipIndex = it->second.intADIndex;
        DHC_DH9931_DeInit((MI_SNR_PADID)it->second.intSensorId);
        Cus_SnrDisable(u8ChipIndex);
    }
}
AMIGOS_MODULE_INIT("SNR9931", AmigosModuleSnr9931);
