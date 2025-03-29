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
#ifndef __AMIGOS_MODULE_PCIE_H__
#define __AMIGOS_MODULE_PCIE_H__

#include "amigos_module_mi_base.h"
#include "amigos_surface_pcie.h"
#include "ss_connector.h"

class AmigosModulePcie: public AmigosSurfacePcie, public AmigosModuleMiBase
{
    public:
        explicit AmigosModulePcie(const std::string &strInSection);
        ~AmigosModulePcie() override;

        unsigned int GetModId() const override;
        unsigned int GetInputType(unsigned int port) const override;
        unsigned int GetOutputType(unsigned int port) const override;
    private:
        void _Init() override;
        void _Deinit() override;
        struct PcieReaderDesc
        {
            ss_connector_base *pOutConnector;
            ss_connector_base *pPcieRingConnector;
            void              *threadHandle;
            PcieReaderDesc ()
                :pOutConnector(NULL), pPcieRingConnector(NULL), threadHandle(NULL)
            {}
        };
        void BlockBindCheck();
        ss_connector_base *CreateSsRingConnector();
        void DestroySsRingConnector(ss_connector_base *connector);
        bool _IsSyncMode(unsigned int inPortId) const override;
        void _StartOut(unsigned int outPortId) override;
        void _StopOut(unsigned int outPortId) override;
        ss_connector_base *_CreateConnector(unsigned int inPortId, bool &bFast) override;
        void _DestroyConnector(ss_connector_base *connector) override;
        int _Connected(unsigned int outPortId, unsigned int ref) override;
        int _Disconnected(unsigned int outPortId, unsigned int ref) override;

        std::map<unsigned int, struct PcieReaderDesc> mapPcieOutDesc;
        static void * _PcieReader(struct ss_thread_buffer *pstBuf);
        static std::map<unsigned int, unsigned int> mapPcieCreateDev;
};
#endif
