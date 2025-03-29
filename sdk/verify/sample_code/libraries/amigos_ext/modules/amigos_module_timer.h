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

#ifndef __AMIGOS_MODULE_TIMER_H__
#define __AMIGOS_MODULE_TIMER_H__

#include <map>
#include <string>
#include "amigos_module_base.h"
#include "amigos_surface_timer.h"

class AmigosModuleTimer : public AmigosSurfaceTimer, public AmigosModuleBase
{
private:
    static void *TimerReader(struct ss_thread_buffer *thread_buf);
private:
    struct TimerReaderDesc
    {
        void        *threadHandle;
        LinkerGroup *linker;
        TimerReaderDesc() : threadHandle(nullptr), linker(nullptr)
        {}
    };

public:
    explicit AmigosModuleTimer(const std::string &strInSection);
    ~AmigosModuleTimer() override;

    unsigned int GetModId() const override;
    unsigned int GetInputType(unsigned int port) const override;
    unsigned int GetOutputType(unsigned int port) const override;

protected:
    void _Init() override;
    void _Deinit() override;
    int  _Connected(unsigned int outPortId, unsigned int ref) override;
    int  _Disconnected(unsigned int outPortId, unsigned int ref) override;

private:
    std::map<unsigned int, TimerReaderDesc> map_timer_desc;
};

#endif /* __AMIGOS_MODULE_TIMER_H__ */
