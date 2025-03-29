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

#ifndef __AMIGOS_MODULE_IQ_H__
#define __AMIGOS_MODULE_IQ_H__

#include "amigos_module_mi_base.h"
#include "amigos_surface_iq.h"
#include "mi_isp_iq_datatype.h"
#include "mi_isp_ae_datatype.h"

SS_ENUM_CAST_STR(MI_ISP_AE_FlickerType_e, {
    { E_SS_AE_FLICKER_TYPE_DISABLE, "disable" },
    { E_SS_AE_FLICKER_TYPE_60HZ   , "60hz"    },
    { E_SS_AE_FLICKER_TYPE_50HZ   , "50hz"    },
    { E_SS_AE_FLICKER_TYPE_AUTO   , "auto"    },
});

SS_ENUM_CAST_STR(MI_ISP_AE_FlickerDetectType_e, {
    { E_SS_AE_FLICKER_TYPE_DETECT_60HZ   , "60hz"    },
    { E_SS_AE_FLICKER_TYPE_DETECT_50HZ   , "50hz"    },
});

SS_ENUM_CAST_STR(MI_ISP_IQ_CaliItem_e, {
    { E_SS_CALI_ITEM_AWB   , "awb"    },
    { E_SS_CALI_ITEM_OBC   , "obc"    },
    { E_SS_CALI_ITEM_SDC   , "sdc"    },
    { E_SS_CALI_ITEM_ALSC  , "alsc"   },
    { E_SS_CALI_ITEM_LSC   , "lsc"    },
    { E_SS_CALI_ITEM_AWB_EX, "awb_ex" },
    { E_SS_CALI_ITEM_FPN   , "fpn"    },
    { E_SS_CALI_ITEM_NE    , "ne"     },
    { E_SS_CALI_ITEM_AIBNR , "aibnr"  },
});

class AmigosModuleIq: public AmigosSurfaceIq, public AmigosModuleMiBase
{
    public:
        explicit AmigosModuleIq(const std::string &inStrSection);
        ~AmigosModuleIq() override;
        unsigned int GetModId() const override;
        unsigned int GetInputType(unsigned int port) const override;
        unsigned int GetOutputType(unsigned int port) const override;

    private:
        int IspWaitReadyTimeout(int time_ms);
        void _Init() override;
        void _Deinit() override;
        void _Start() override;
        void _Stop() override;
        void _ResourceInit() override;
        void _ResourceDeinit() override;
        static std::map<unsigned int, unsigned int> mapIqCreateDev;
};
#endif

