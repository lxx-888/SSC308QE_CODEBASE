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

#ifndef __AMIGOS_MODULE_ENV_MONITOR_H__
#define __AMIGOS_MODULE_ENV_MONITOR_H__

#include <map>
#include <string>
#include "amigos_env.h"
#include "amigos_module_base.h"
#include "amigos_surface_env_monitor.h"

class AmigosModuleEnvMonitor : public AmigosSurfaceEnvMonitor, public AmigosModuleBase
{
private:
    static void *EnvMonitorReader(struct ss_thread_buffer *thread_buf);

private:
    struct EnvMonitorReaderDesc
    {
        LinkerGroup  *linker;
        std::string  strModule;
        unsigned int inPort;
        unsigned int outPort;
        void         *threadHandle;
        void (*formatting)(AmigosEnv &env, wchar_t *str, unsigned int n, unsigned int inPort,
                           unsigned int outPortId);
        AmigosEnv *env;
        EnvMonitorReaderDesc() : linker(nullptr), threadHandle(nullptr), formatting(nullptr), env(nullptr) {}
    };

public:
    explicit AmigosModuleEnvMonitor(const std::string &strInSection);
    ~AmigosModuleEnvMonitor() override;

    unsigned int GetModId() const override;
    unsigned int GetInputType(unsigned int port) const override;
    unsigned int GetOutputType(unsigned int port) const override;

protected:
    void _Init() override;
    void _Deinit() override;
    int  _Connected(unsigned int outPortId, unsigned int ref) override;
    int  _Disconnected(unsigned int outPortId, unsigned int ref) override;

private:
    std::map<unsigned int, EnvMonitorReaderDesc> map_timer_desc;
};

#endif /* __AMIGOS_MODULE_ENV_MONITOR_H__ */
