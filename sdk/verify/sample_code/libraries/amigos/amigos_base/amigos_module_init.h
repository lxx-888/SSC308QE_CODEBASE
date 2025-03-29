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

#ifndef __AMIGOS_MODULE_INIT_H__
#define __AMIGOS_MODULE_INIT_H__

#include "amigos_module_base.h"
#include "amigos_surface_base.h"
class AmigosModuleInit
{
public:
    explicit AmigosModuleInit(void (*preLoad)(void))
    {
        preLoad();
    }
    virtual ~AmigosModuleInit() {}
    static std::map<std::string, AmigosModuleBase *(*)(const std::string &)> initMap;
};

#define AMIGOS_MODULE_INIT(__name, __class)                                             \
    static AmigosModuleBase *__amigos_early_init_load_##__class(const std::string &key) \
    {                                                                                   \
        __class *pModule  = new __class(key);                                           \
        assert(pModule);                                                                \
        return pModule;                                                                 \
    }                                                                                   \
    void __amigos_early_init_preload_##__class()                                        \
    {                                                                                   \
        std::cout << "Preload " << #__class                                       \
        << ',' << #__name << std::endl;                                                 \
        AmigosModuleInit::initMap[__name] = __amigos_early_init_load_##__class;         \
    }

#define AMIGOS_MODULE_SETUP(__mod)                                     \
    extern void __amigos_early_init_preload_AmigosModule##__mod(void); \
    __amigos_early_init_preload_AmigosModule##__mod()


#define AMIGOS_SETUP(__name, __mod, ...) \
        AMIGOS_MODULE_SETUP(__mod);

#endif //__AMIGOS_MODULE_INIT_H__
