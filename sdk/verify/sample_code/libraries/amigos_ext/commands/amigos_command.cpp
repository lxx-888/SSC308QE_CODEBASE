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
#include "ss_handle.h"
#include "amigos_command.h"

extern ss_log sslog;

template <class T>
map<std::string, T *> ss_handle_template<T>::handle_map;

void AmigosCommand::Install()
{
    MOD_SETUP_IN_EXT(base, mapCmdData);
    ss_handle_template<AmigosCommand>::install(cmdModName, this);
    std::cout << "Preload AmigosCommand," << '\"' << cmdModName << '\"' << std::endl;
}
AmigosCommand *AmigosCommand::GetObj(const std::string &modName)
{
    return ss_handle_template<AmigosCommand>::get(modName);
}
