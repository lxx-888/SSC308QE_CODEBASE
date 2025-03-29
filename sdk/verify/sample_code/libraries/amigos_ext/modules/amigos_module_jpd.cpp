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
#include <assert.h>

#include <vector>
#include <string>

#include "mi_sys.h"
#include "mi_jpd.h"
#include "amigos_module_init.h"
#include "amigos_module_jpd.h"
#include "ss_enum_cast.hpp"
#include "ss_packet.h"

std::map<unsigned int, unsigned int> AmigosModuleJpd::mapJpdCreateDev;

AmigosModuleJpd::AmigosModuleJpd(const std::string &strInSection)
    : AmigosSurfaceJpd(strInSection), AmigosModuleMiBase(this)
{
}
AmigosModuleJpd::~AmigosModuleJpd()
{
}
unsigned int AmigosModuleJpd::GetModId() const
{
    return E_MI_MODULE_ID_JPD;
}
unsigned int AmigosModuleJpd::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleJpd::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_KERNEL | E_MOD_PORT_TYPE_USER;
}
void AmigosModuleJpd::_Init()
{
    MI_JPD_InitParam_t    stInitParam = {0};
    MI_JPD_ChnCreatConf_t stChnCreatConf;
    MI_SYS_ChnPort_t      stChnPort;
    std::map<unsigned int, unsigned int>::iterator itMapJpdCreateDev;

    itMapJpdCreateDev = mapJpdCreateDev.find(stModInfo.devId);
    if (itMapJpdCreateDev == mapJpdCreateDev.end())
    {
        MI_JPD_CreateDev(stModInfo.devId, &stInitParam);
        mapJpdCreateDev[stModInfo.devId] = 0;
    }
    mapJpdCreateDev[stModInfo.devId]++;
    stChnCreatConf.ePixelFormat = ss_enum_cast<MI_SYS_PixelFormat_e>::from_str(stJpdInfo.strOutFmt);
    stChnCreatConf.u32StreamBufSize = stJpdInfo.u32StreamBufSize;
    stChnCreatConf.u32MaxPicWidth = stJpdInfo.u32MaxPicWidth;
    stChnCreatConf.u32MaxPicHeight = stJpdInfo.u32MaxPicHeight;
    MI_JPD_CreateChn(stModInfo.devId, stModInfo.chnId, &stChnCreatConf);
    MI_JPD_StartChn(stModInfo.devId, stModInfo.chnId);

    stChnPort.eModId = E_MI_MODULE_ID_JPD;
    stChnPort.u32DevId = stModInfo.devId;
    stChnPort.u32ChnId = stModInfo.chnId;
    stChnPort.u32PortId = 0;
    MI_SYS_SetChnOutputPortDepth(0, &stChnPort, 2, 4);

}
void AmigosModuleJpd::_Deinit()
{
    MI_JPD_StopChn(stModInfo.devId, stModInfo.chnId);
    MI_JPD_DestroyChn(stModInfo.devId, stModInfo.chnId);
    MI_JPD_DestroyDev(stModInfo.devId);
}
int AmigosModuleJpd::_DataReceiver(unsigned int inPortId, stream_packet_obj &packet)
{
    #define MIN(a, b) ((a) < (b) ? (a) : (b))
    MI_U32             u32RequiredLength = 0;
    MI_JPD_StreamBuf_t stRetStreamBuf;
    MI_S32             s32Ret = E_MI_ERR_FAILED;

    if (packet->en_type != EN_VIDEO_CODEC_DATA)
    {
        AMIGOS_ERR("packet type error %d \n", packet->en_type);
        return -1;
    }
    if (packet->es_vid_i.fmt != ES_STREAM_JPEG)
    {
        AMIGOS_ERR("packet format error %d \n", packet->es_vid_i.fmt);
        return -1;
    }
    u32RequiredLength = packet->es_vid_i.packet_info[0].size;
    memset(&stRetStreamBuf, 0x0, sizeof(MI_JPD_StreamBuf_t));
    s32Ret = MI_JPD_GetStreamBuf((MI_VDEC_DEV)stModInfo.devId, stModInfo.chnId, u32RequiredLength, &stRetStreamBuf, 0);
    if(s32Ret != MI_SUCCESS)
    {
        AMIGOS_ERR("can not get jpd stream %d\n",s32Ret);
        return -1;
    }
    memcpy(stRetStreamBuf.pu8HeadVirtAddr, packet->es_vid.packet_data[0].data, MIN(stRetStreamBuf.u32HeadLength, u32RequiredLength));
    if (stRetStreamBuf.u32HeadLength + stRetStreamBuf.u32TailLength < u32RequiredLength)
    {
        AMIGOS_ERR("MI_JPD_GetStreamBuf return wrong value: HeadLen%u TailLen%u RequiredLength%u\n",
                   stRetStreamBuf.u32HeadLength, stRetStreamBuf.u32TailLength, u32RequiredLength);
        MI_JPD_DropStreamBuf((MI_VDEC_DEV)stModInfo.devId, stModInfo.chnId, &stRetStreamBuf);
        return -1;
    }
    else if (stRetStreamBuf.u32TailLength > 0)
    {
        memcpy(stRetStreamBuf.pu8TailVirtAddr, packet->es_vid.packet_data[0].data + stRetStreamBuf.u32HeadLength,
               MIN(stRetStreamBuf.u32TailLength, u32RequiredLength - stRetStreamBuf.u32HeadLength));
    }
    stRetStreamBuf.u32ContentLength = u32RequiredLength;
    s32Ret = MI_JPD_PutStreamBuf((MI_VDEC_DEV)stModInfo.devId, stModInfo.chnId, &stRetStreamBuf);
    if(s32Ret != MI_SYS_SUCCESS)
    {
        AMIGOS_ERR("can not put jpd stream %d\n", s32Ret);
    }
    return 0;
}
ss_linker_base *AmigosModuleJpd::_CreateInputNegativeLinker(unsigned int inportid)
{
    return new LinkerSyncNegative(inportid, this);
}
stream_packer *AmigosModuleJpd::_CreateInputStreamPacker(unsigned int inPortId, bool &bFast)
{
    bFast = false;
    return new StreamPackerNormal();
}
AMIGOS_MODULE_INIT("JPD", AmigosModuleJpd);
