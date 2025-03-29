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
#ifndef __AMIGOS_INSTANCE_H__
#define __AMIGOS_INSTANCE_H__

#include "amigos_module_base.h"
#include "amigos_surface_base.h"

class AmigosInstance
{
    public:
        struct AmigosBlockMap
        {
            std::string modSectionName;
            std::vector<unsigned int> ioPorts[2];
            unsigned int uintX;
            unsigned int uintY;
            unsigned int uintH;
        };
        explicit AmigosInstance(unsigned int myChipId, AmigosDatabaseFactoryBase *factory);
        virtual ~AmigosInstance();
        void LoadDbAll();
        void UnloadDbAll();
        void InitAll();
        void BindAll();
        void StartAll();
        void StopAll();
        void UnbindAll();
        void DeinitAll();
        void GetMapAll(std::vector<AmigosBlockMap> &vectBlockMap);
    private:
        void BuildModTree(AmigosModuleBase *pIns);
        void CreateConnection(unsigned int myChipId);
        void DestroyConnection();
        AmigosModuleBase *Implement(std::string &strKey);
        void ReverseMapAll();
        unsigned int uintTraversePosX;
        unsigned int uintTraversePosY;
        AmigosDatabaseFactoryBase *insFactory;
        std::vector<AmigosModuleBase*> connectOrder;
        std::map<unsigned int, unsigned int> mapSlaveChipIdRef;
};
#endif //__AMIGOS_INSTANCE_H__
