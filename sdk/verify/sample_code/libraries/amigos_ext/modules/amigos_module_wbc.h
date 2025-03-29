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

#ifndef __AMIGOS_MODULE_WBC_H__
#define __AMIGOS_MODULE_WBC_H__

#include "amigos_module_mi_base.h"
#include "amigos_surface_wbc.h"

class AmigosModuleWbc: public AmigosSurfaceWbc, public AmigosModuleMiBase
{
    public:
        explicit AmigosModuleWbc(const std::string &strInSection);
        ~AmigosModuleWbc() override;
        unsigned int GetModId() const override;
        unsigned int GetInputType(unsigned int port) const override;
        unsigned int GetOutputType(unsigned int port) const override;
    protected:
        void _Init() override;
        void _Deinit() override;
};
#endif

