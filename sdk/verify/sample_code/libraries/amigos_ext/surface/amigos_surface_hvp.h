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

#ifndef __AMIGOS_SURFACE_HVP_H__
#define __AMIGOS_SURFACE_HVP_H__

#include "amigos_surface_base.h"

class AmigosSurfaceHvp: public AmigosSurfaceBase
{
    public:
        struct HvpInfo
        {
            unsigned int FrcMode;
            unsigned int BufMaxCnt;
            unsigned int BufMaxWidth;  //OUT_VID_W
            unsigned int BufMaxHeight;  //OUT_VID_H
            unsigned int CompressMode;
            unsigned int FieldType;
            unsigned int InputColor;
            unsigned int ColorDepth;
            unsigned int DMAColor;
            unsigned int DestColor;
            unsigned int OutWidth;
            unsigned int OutHeight;
            unsigned int Fpsx100;
            public:
            HvpInfo()
            {
                Clear();
            }
            void Clear()
            {
                FrcMode      = 0;
                BufMaxCnt    = 0;
                BufMaxWidth  = 0;
                BufMaxHeight = 0;
                CompressMode = 0;
                FieldType    = 0;
                InputColor   = 0;
                ColorDepth   = 0;
                DMAColor     = 0;
                DestColor    = 0;
                OutWidth     = 0;
                OutHeight    = 0;
                Fpsx100      = 0;
            }
        };
        explicit AmigosSurfaceHvp(const std::string &strInSection);
        virtual ~AmigosSurfaceHvp();
    protected:
        HvpInfo stHvpConf;
    private:
        virtual void _LoadDb();
        virtual void _UnloadDb();
};
#endif //__AMIGOS_SURFACE_HVP_H__
