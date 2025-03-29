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

#ifndef __AMIGOS_MODULE_SCL_BASE_H__
#define __AMIGOS_MODULE_SCL_BASE_H__

#include "amigos_module_mi_base.h"
#include "amigos_surface_base.h"
#include "mi_scl_datatype.h"

class AmigosModuleSclBase: public AmigosModuleMiBase
{
public:
    explicit AmigosModuleSclBase(AmigosSurfaceBase *pSurface);
    ~AmigosModuleSclBase() override;
protected:
    void _CreateDeviceResource(unsigned int devId);
    void _DestroyDeviceResource(unsigned int devId);
    bool _CreateDevice(unsigned int devId, MI_SCL_DevAttr_t &stDevAttr);
    void _DestroyDevice(unsigned int devId);
private:
    static std::map<unsigned int, unsigned int> mapSclCreateDev;
};
#endif

