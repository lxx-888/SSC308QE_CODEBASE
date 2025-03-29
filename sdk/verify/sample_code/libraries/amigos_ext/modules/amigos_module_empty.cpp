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
#include <string.h>
#include <stdio.h>
#include "amigos_module_init.h"
#include "amigos_module_empty.h"
#include "ss_packet.h"

AmigosModuleEmpty::AmigosModuleEmpty(const std::string &strInSection)
    : AmigosSurfaceEmpty(strInSection), AmigosModuleBase(this)
{
}
AmigosModuleEmpty::~AmigosModuleEmpty()
{
    AMIGOS_INFO("func: %s\n", __FUNCTION__);
}
unsigned int AmigosModuleEmpty::GetModId() const
{
    return uintExtModId;
}
unsigned int AmigosModuleEmpty::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleEmpty::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
int AmigosModuleEmpty::_DataReceiver(unsigned int inPortId, stream_packet_obj &packet)
{
    return 0;
}
void AmigosModuleEmpty::_Init()
{
    AMIGOS_INFO("func: %s\n", __FUNCTION__);
}
void AmigosModuleEmpty::_Deinit()
{
    AMIGOS_INFO("func: %s\n", __FUNCTION__);
}
AMIGOS_MODULE_INIT("EMPTY", AmigosModuleEmpty);
