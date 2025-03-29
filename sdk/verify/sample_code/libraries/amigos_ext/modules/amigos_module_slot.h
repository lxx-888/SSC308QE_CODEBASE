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

#ifndef __AMIGOS_MODULE_SLOT_H__
#define __AMIGOS_MODULE_SLOT_H__

#include "amigos_module_base.h"
#include "amigos_surface_slot.h"

class AmigosModuleSlot: public AmigosSurfaceSlot, public AmigosModuleBase
{
    public:
        explicit AmigosModuleSlot(const std::string &strInSection);
        ~AmigosModuleSlot() override;
        unsigned int GetModId() const override;
        unsigned int GetInputType(unsigned int port) const override;
        unsigned int GetOutputType(unsigned int port) const override;
    private:
        void _Init() override;
        void _Deinit() override;
        void _DirectBind(unsigned int inPortId, unsigned int prevOutPortId, const AmigosModuleBase *pPrev) override;
        void _DirectUnBind(unsigned int inPortId, unsigned int prevOutPortId, const AmigosModuleBase *pPrev) override;
};
#endif

