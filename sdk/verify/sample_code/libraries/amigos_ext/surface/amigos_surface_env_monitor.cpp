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

#include "amigos_surface_base.h"
#include "amigos_surface_env_monitor.h"

AmigosSurfaceEnvMonitor::AmigosSurfaceEnvMonitor(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, false)
{
}
AmigosSurfaceEnvMonitor::~AmigosSurfaceEnvMonitor()
{
}
void AmigosSurfaceEnvMonitor::_LoadDb()
{
    for (auto it = this->mapModOutputInfo.begin(); it != this->mapModOutputInfo.end(); ++it)
    {
        this->mapEnvMonitorOutputInfo[it->first].strMode    = this->pDb->GetOut<std::string>(it->first, "MODE");
        this->mapEnvMonitorOutputInfo[it->first].strModule  = this->pDb->GetOut<std::string>(it->first, "MODULE");
        this->mapEnvMonitorOutputInfo[it->first].intInPort  = this->pDb->GetOut<unsigned int>(it->first, "IN_PORT");
        this->mapEnvMonitorOutputInfo[it->first].intOutPort = this->pDb->GetOut<unsigned int>(it->first, "OUT_PORT");
    }
}
void AmigosSurfaceEnvMonitor::_UnloadDb()
{
    for (auto it = this->mapModOutputInfo.begin(); it != this->mapModOutputInfo.end(); ++it)
    {
        this->mapEnvMonitorOutputInfo[it->first] = EnvMonitorOutputInfo();
    }
}

