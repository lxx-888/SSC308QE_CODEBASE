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

#ifndef __AMIGOS_MODULE_ISP_H__
#define __AMIGOS_MODULE_ISP_H__

#include "amigos_module_mi_base.h"
#include "amigos_surface_isp.h"
#include "mi_isp_datatype.h"

SS_ENUM_CAST_STR(MI_ISP_HDRType_e, {
    { E_MI_ISP_HDR_TYPE_OFF        , "off"     },
    { E_MI_ISP_HDR_TYPE_VC         , "vc"      },
    { E_MI_ISP_HDR_TYPE_DOL        , "dol"     },
    { E_MI_ISP_HDR_TYPE_LI         , "li"      },
});

SS_ENUM_CAST_STR(MI_ISP_HDRFusionType_e, {
    { E_MI_ISP_HDR_FUSION_TYPE_NONE        , "off"     },
    { E_MI_ISP_HDR_FUSION_TYPE_2TO1        , "2to1"    },
    { E_MI_ISP_HDR_FUSION_TYPE_3TO1        , "3to1"    },
});

SS_ENUM_CAST_STR(MI_ISP_Overlap_e,
{
    { E_MI_ISP_OVERLAP_NONE, "none" },
    { E_MI_ISP_OVERLAP_128, "128" },
    { E_MI_ISP_OVERLAP_256, "256" },
    { E_MI_ISP_OVERLAP_MAX, "max" },
});

class AmigosModuleIsp: public AmigosSurfaceIsp, public AmigosModuleMiBase
{
    public:
        explicit AmigosModuleIsp(const std::string &strInSection);
        ~AmigosModuleIsp() override;
        unsigned int GetModId() const override;
        unsigned int GetInputType(unsigned int port) const override;
        unsigned int GetOutputType(unsigned int port) const override;
    private:
        void _Init() override;
        void _Deinit() override;
        void _Start() override;
        void _Stop() override;
        void _StartMiOut(unsigned int outPortId) override;
        void _StopMiOut(unsigned int outPortId) override;
        void _ResourceInit() override;
        void _ResourceDeinit() override;
#ifdef CONFIG_MI_SYSCALL_L2R_DESTROY_SUPPORT
        bool _NeedMarkDeinitOnRtos() override;
        bool _NeedMarkStopOnRtos() override;
        bool _NeedMarkUnbindOnRtos(unsigned int inPortId) override;
        bool _NeedMarkStopOutOnRtos(unsigned int outPortId) override;
#endif
        stream_packet_info _GetStreamInfo(unsigned int outPortId);

private:
        static std::map<unsigned int, unsigned int> mapIspCreateDev;
};
#endif

