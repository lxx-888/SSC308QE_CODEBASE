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
#include "ss_packet.h"
#include "amigos_log.h"
#include "amigos_module_init.h"
#include "amigos_module_sae.h"

AmigosModuleSae::AmigosModuleSae(const std::string &strInSection)
    : AmigosSurfaceSae(strInSection),
      AmigosModuleBase(this)
{
}
AmigosModuleSae::~AmigosModuleSae() {}
unsigned int AmigosModuleSae::GetModId() const
{
    return uintExtModId;
}
unsigned int AmigosModuleSae::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleSae::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}

void AmigosModuleSae::_Init()
{
    return;
}

void AmigosModuleSae::_Deinit()
{
    return;
}

void AmigosModuleSae::_StartIn(unsigned int inPortId)
{
    MI_U32 ispDevId = this->stSaeInfo.intIspDev;
    MI_U32 ispChnId = this->stSaeInfo.intIspChn;
    MI_ISP_AE_ModeType_e aeType = E_SS_AE_MODE_M;
    MI_ISP_AE_SetExpoMode(ispDevId, ispChnId, &aeType);

    return;
}
void AmigosModuleSae::_StopIn(unsigned int inPortId)
{
    MI_U32 ispDevId = this->stSaeInfo.intIspDev;
    MI_U32 ispChnId = this->stSaeInfo.intIspChn;
    MI_ISP_AE_ModeType_e aeType = E_SS_AE_MODE_A;
    MI_ISP_AE_SetExpoMode(ispDevId, ispChnId, &aeType);

    return;
}


int AmigosModuleSae::_DataReceiver(unsigned int inPortId, stream_packet_obj &packet)
{
    MI_U32 ispDevId = this->stSaeInfo.intIspDev;
    MI_U32 ispChnId = this->stSaeInfo.intIspChn;
    MI_ISP_AE_ExpoValueType_t stIspAeExpoInfo;
    MI_ISP_AE_ExpoInfoType_t *pstAEExpoInfo = NULL;
    memset(&stIspAeExpoInfo,0,sizeof(MI_ISP_AE_ExpoValueType_t));
    if(packet->en_type != EN_USER_META_DATA)
    {
        AMIGOS_ERR("Err packet en_type, must be metadata\n");
        return -1;
    }
    pstAEExpoInfo = (MI_ISP_AE_ExpoInfoType_t*)packet->meta_data.data;
    stIspAeExpoInfo.u32FNx10 = pstAEExpoInfo->stExpoValueLong.u32FNx10;
    stIspAeExpoInfo.u32ISPGain = pstAEExpoInfo->stExpoValueLong.u32ISPGain;
    stIspAeExpoInfo.u32SensorGain = pstAEExpoInfo->stExpoValueLong.u32SensorGain;
    stIspAeExpoInfo.u32US = pstAEExpoInfo->stExpoValueLong.u32US;
    AMIGOS_INFO("ispgain=%u,sensorgain=%u,shutter=%u\n",pstAEExpoInfo->stExpoValueLong.u32ISPGain,pstAEExpoInfo->stExpoValueLong.u32SensorGain,pstAEExpoInfo->stExpoValueLong.u32US);;

    if(MI_SUCCESS != MI_ISP_AE_SetManualExpo(ispDevId, ispChnId, &stIspAeExpoInfo))
    {
        AMIGOS_ERR("MI_ISP_AE_QueryExposureInfo failed\n");
        return -1;
    }
    return 0;
}

AMIGOS_MODULE_INIT("SAE", AmigosModuleSae);
