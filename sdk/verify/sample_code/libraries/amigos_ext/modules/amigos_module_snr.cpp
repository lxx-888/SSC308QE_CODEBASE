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
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "mi_common.h"
#include "mi_vif.h"
#include "mi_sensor.h"
#include "amigos_module_init.h"
#include "amigos_module_snr.h"

AmigosModuleSnr::AmigosModuleSnr(const std::string &strInSection)
    : AmigosSurfaceSnr(strInSection), AmigosModuleMiBase(this)
{
}
AmigosModuleSnr::~AmigosModuleSnr()
{
}
unsigned int AmigosModuleSnr::GetModId() const
{
    return E_MI_MODULE_ID_SNR;
}
unsigned int AmigosModuleSnr::GetInputType(unsigned int port) const
{
    return 0;
}
unsigned int AmigosModuleSnr::GetOutputType(unsigned int port) const
{
    return 0;
}
void AmigosModuleSnr::_ResourceInit()
{
    AmigosModuleMiBase::stSensorDrvInfo_t stSnrDrvInfo;
    MI_SNR_GetPadInfo((MI_SNR_PADID)stSnrInfo.intSensorId, &stSnrDrvInfo.stPadInfo);
    MI_SNR_GetPlaneInfo((MI_SNR_PADID)stSnrInfo.intSensorId, 0, &stSnrDrvInfo.stSnrPlaneInfo);
    UpdateSensorInfo(stSnrDrvInfo, (unsigned int)stSnrInfo.intSensorId);
}
void AmigosModuleSnr::_Init()
{
    MI_U32 u32ResCount =0;
    MI_U8 u8ResIndex =0;
    MI_SNR_Res_t stRes;
    AmigosModuleMiBase::stSensorDrvInfo_t stSnrDrvInfo;

    memset(&stRes, 0x0, sizeof(MI_SNR_Res_t));
    if(stSnrInfo.intHdrType == (int)E_MI_VIF_HDR_TYPE_OFF
       || stSnrInfo.intHdrType == (int)E_MI_VIF_HDR_TYPE_LI)
        MI_SNR_SetPlaneMode((MI_SNR_PADID)stSnrInfo.intSensorId, FALSE);
    else
        MI_SNR_SetPlaneMode((MI_SNR_PADID)stSnrInfo.intSensorId, TRUE);

    MI_SNR_QueryResCount((MI_SNR_PADID)stSnrInfo.intSensorId, &u32ResCount);
    for(u8ResIndex = 0; u8ResIndex < u32ResCount; u8ResIndex++)
    {
        MI_SNR_GetRes((MI_SNR_PADID)stSnrInfo.intSensorId, u8ResIndex, &stRes);
        AMIGOS_INFO("Sensor supported resolution: index %d, Crop(%d,%d,%d,%d), outputsize(%d,%d), maxfps %d, minfps %d, ResDesc %s\n",
        u8ResIndex,
        stRes.stCropRect.u16X, stRes.stCropRect.u16Y, stRes.stCropRect.u16Width,stRes.stCropRect.u16Height,
        stRes.stOutputSize.u16Width, stRes.stOutputSize.u16Height,
        stRes.u32MaxFps,stRes.u32MinFps,
        stRes.strResDesc);
    }
    if(stSnrInfo.intSensorRes >= (int)u32ResCount)
    {
        AMILOG_ERR << "Choice err res " << stSnrInfo.intSensorRes << " >= cnt " << u32ResCount << ',' << std::endl
                   << "Please check your sensor's driver which doesn't exist resolution id " << stSnrInfo.intSensorRes << '.' << std::endl
                   << "Besides you can change 'SNR_RES' in section '" << this->GetSurface()->GetModInfo().sectionName
                   << "' of your json file to match the right resolution id if u insist on using current sensor driver." << COLOR_ENDL;
        assert(0);
    }
    AMIGOS_INFO("You choose sensor res is %d\n", stSnrInfo.intSensorRes);
    MI_SNR_SetRes((MI_SNR_PADID)stSnrInfo.intSensorId, (MI_U32)stSnrInfo.intSensorRes);
    MI_SNR_SetFps((MI_SNR_PADID)stSnrInfo.intSensorId, stSnrInfo.intSensorFps);
    MI_SNR_SetOrien((MI_SNR_PADID)stSnrInfo.intSensorId, stSnrInfo.intSensorMirror,stSnrInfo.intSensorFlip);
    MI_SNR_Enable((MI_SNR_PADID)stSnrInfo.intSensorId);
    MI_SNR_GetPadInfo((MI_SNR_PADID)stSnrInfo.intSensorId, &stSnrDrvInfo.stPadInfo);
    MI_SNR_GetPlaneInfo((MI_SNR_PADID)stSnrInfo.intSensorId, 0, &stSnrDrvInfo.stSnrPlaneInfo);
    UpdateSensorInfo(stSnrDrvInfo, (unsigned int)stSnrInfo.intSensorId);
}
void AmigosModuleSnr::_Deinit()
{
    MI_SNR_Disable((MI_SNR_PADID)stSnrInfo.intSensorId);
}

#ifdef CONFIG_MI_SYSCALL_L2R_DESTROY_SUPPORT
bool AmigosModuleSnr::_NeedMarkDeinitOnRtos()
{
    return true;
}
#endif

AMIGOS_MODULE_INIT("SNR", AmigosModuleSnr);
