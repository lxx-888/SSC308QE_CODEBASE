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

#ifndef __AMIGOS_SURFACE_ISP_H__
#define __AMIGOS_SURFACE_ISP_H__

#include <cstring>
#include "amigos_surface_base.h"

#define ZOOM_TABLE_MAX_NUM (8)

class AmigosSurfaceIsp : public AmigosSurfaceBase
{
public:

    struct stIspTableCfg
    {
        unsigned int       SnrId;
        unsigned int       TableX;
        unsigned int       TableY;
        unsigned int       TableW;
        unsigned int       TableH;
    };

    struct stIspZoomCfg
    {
        unsigned int       FromEntryIndex;
        unsigned int       ToEntryIndex;
        unsigned int       TableNum;
        char*              pTableAddr;
        struct stIspTableCfg Table[ZOOM_TABLE_MAX_NUM];

        void clear()
        {
            FromEntryIndex = 0;
            ToEntryIndex   = 0;
            TableNum = 0;
            for(int i = 0; i < ZOOM_TABLE_MAX_NUM; i++)
            {
                pTableAddr = NULL;
                Table[i].SnrId = 0;
                Table[i].TableX = 0;
                Table[i].TableY = 0;
                Table[i].TableW = 0;
                Table[i].TableH = 0;
            }
        }
    };

    struct stCustIqCfg
    {
        unsigned int         Revision;
        unsigned short       SnrEarlyFps;
        unsigned short       SnrEarlyFlicker;
        unsigned short       SnrEarlyAwbRGain;
        unsigned short       SnrEarlyAwbGGain;
        unsigned short       SnrEarlyAwbBGain;
        unsigned long        SnrEarlyShutter;
        unsigned long        SnrEarlyGainX1024;
        unsigned long        SnrEarlyDGain;

        void clear()
        {
            Revision          = 0;
            SnrEarlyFps       = 0;
            SnrEarlyFlicker   = 0;
            SnrEarlyShutter   = 0;
            SnrEarlyGainX1024 = 0;
            SnrEarlyDGain     = 0;
            SnrEarlyAwbRGain  = 0;
            SnrEarlyAwbGGain  = 0;
            SnrEarlyAwbBGain  = 0;
        }
    };

    struct stSubChnIqCfg
    {
        unsigned int        dev;
        unsigned int        chn;
        std::string         apiFileName;

        void clear()
        {
            dev       = 0;
            chn       = 0;
            apiFileName.clear();
        }
    };
    struct stOverLapCfg
    {
        std::string strOverlap;
        void clear()
        {
            strOverlap.clear();
        }
    };
    struct stIspLdcCfg
    {
        unsigned int        CenterX;
        unsigned int        CenterY;
        unsigned int        Alpha;
        unsigned int        Beta;
        unsigned int        CropL;
        unsigned int        CropR;

        void clear()
        {
            CenterX       = 0;
            CenterY       = 0;
            Alpha         = 0;
            Beta          = 0;
            CropL         = 0;
            CropR         = 0;
        }
    };
    struct stAiBnrCfg
    {
        std::string strBnrSrcType;
        std::string bnrModelFileName;
        void clear()
        {
            bnrModelFileName.clear();
            strBnrSrcType.clear();
        }
    };

    struct IspInfo
    {
        unsigned int uintHdrType;
        unsigned int uintHdrFusionType;
        unsigned int uintHdrExposureMask;
        unsigned int uintSensorId;
        unsigned int uintRotation;
        unsigned int uintIsMirror;
        unsigned int uintIsFlip;
        unsigned int uint3dNrLevel;
        unsigned int uintSync3AType;
        unsigned int uintStitchMask;
        unsigned int uintSnrMaskNum;
        unsigned int uintZoomEn;
        unsigned int uintCustIqEn;
        unsigned int uintSubChnIqEn;
        unsigned int uintIspLdcEn;
        unsigned int uintMutichnEn;
        unsigned int uintIspOverlapEn;
        unsigned int uintAibnrEn;
        struct stIspZoomCfg ZoomParam;
        struct stCustIqCfg CustIqParam;
        struct stSubChnIqCfg SubChnIqParam;
        struct stIspLdcCfg IspLdcParam;
        std::string apiFileName;
        struct stOverLapCfg OverLapParam;
        struct stAiBnrCfg aiBnrParam;

        IspInfo()
        {
            Clear();
        }
        void Clear()
        {
            uintHdrType    = 0;
            uintHdrFusionType = 0;
            uintHdrExposureMask = 0;
            uintSensorId   = 0;
            uintRotation   = 0;
            uintIsMirror   = 0;
            uintIsFlip     = 0;
            uint3dNrLevel  = 0;
            uintSync3AType = 0;
            uintStitchMask = 0;
            uintSnrMaskNum = 0;
            uintZoomEn     = 0;
            uintCustIqEn   = 0;
            uintSubChnIqEn = 0;
            uintIspLdcEn   = 0;
            uintAibnrEn    = 0;
            ZoomParam.clear();
            CustIqParam.clear();
            SubChnIqParam.clear();
            IspLdcParam.clear();
            apiFileName.clear();
            aiBnrParam.clear();
        }
    };

    struct IspInInfo
    {
        unsigned int uintIspInCropX;
        unsigned int uintIspInCropY;
        unsigned int uintIspInWidth;
        unsigned int uintIspInHeight;
        IspInInfo ()
            : uintIspInCropX(0), uintIspInCropY(0), uintIspInWidth(0), uintIspInHeight(0)
        {}
    };

    struct IspOutInfo
    {
        unsigned int uintIspOutCropX;
        unsigned int uintIspOutCropY;
        unsigned int uintIspOutCropW;
        unsigned int uintIspOutCropH;
        unsigned int uintCompressMode;
        unsigned int uintBufLayout;
        std::string strOutType;
        std::string strOutFmt;
        IspOutInfo()
            : uintIspOutCropX(0), uintIspOutCropY(0), uintIspOutCropW(0), uintIspOutCropH(0),
              uintCompressMode(0),uintBufLayout(0)
        {}
    };

    explicit AmigosSurfaceIsp(const std::string &strInSection);
    virtual ~AmigosSurfaceIsp();
    void GetInfo(IspInfo &info) const;
    void SetInfo(const IspInfo &info);
    void GetInInfo(unsigned int portId, IspInInfo &stIn) const;
    void SetInInfo(unsigned int portId, const IspInInfo &stIn);
    void GetOutInfo(unsigned int portId, IspOutInfo &stOut) const;
    void SetOutInfo(unsigned int portId, const IspOutInfo &stOut);

protected:
    IspInfo                            stIspInfo;
    std::map<unsigned int, IspInInfo>  mapIspIn;
    std::map<unsigned int, IspOutInfo> mapIspOut;
private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};
#endif //__AMIGOS_SURFACE_ISP_H__
