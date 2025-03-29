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

#ifndef __AMIGOS_SURFACE_BASE_H__
#define __AMIGOS_SURFACE_BASE_H__

#include <string>
#include <vector>
#include <list>
#include <map>
#include "amigos_log.h"
#include "amigos_database.h"

class AmigosSurfaceBase
{
public:
    struct ModInfo
    {
        std::string  modName;
        unsigned int devId;
        unsigned int chnId;
        unsigned int preLoad;
        std::string  sectionName;
        ModInfo() : devId(0), chnId(0), preLoad(0) {}
    };
    struct ModIoInfo
    {
        std::string  sectionName;
        unsigned int loopId;
        ModIoInfo() : loopId(0) {}
    };
    struct ModPortInInfo
    {
        unsigned int curLoopId;
        unsigned int curFrmRate;
        unsigned int bindType;
        unsigned int bindPara;
        ModIoInfo    stPrev;
        ModPortInInfo() : curLoopId(0), curFrmRate(0), bindType(0), bindPara(0)
        {
            stPrev.loopId = 0;
        }
    };
    struct ModPortOutInfo
    {
        unsigned int curLoopId;
        unsigned int curFrmRate;
        /* User Frc */
        bool         bUserFrc;
        /* Buffer Depth */
        unsigned int bindDepthUser;
        unsigned int bindDepthTotal;
        bool         bUseDepth;
        /* Buf Extra Config */
        unsigned int bufHAlign;
        unsigned int bufVAlign;
        unsigned int bufChromaAlign;
        unsigned int bufCompressAlign;
        unsigned int bufExtraSize;
        bool         bufClearPadding;
        bool         bUseExtConf;
        ModPortOutInfo()
            : curLoopId(0),
              curFrmRate(0),
              bindDepthUser(0),
              bindDepthTotal(0),
              bUseDepth(false),
              bufHAlign(0),
              bufVAlign(0),
              bufChromaAlign(0),
              bufCompressAlign(0),
              bufExtraSize(0),
              bufClearPadding(false),
              bUseExtConf(false)
        {
        }
    };

public:
    explicit AmigosSurfaceBase(const std::string &strInSection, bool bNeedExtInInfo, bool bNeedExtOutInfo);
    virtual ~AmigosSurfaceBase();
    void SetDbIns(AmigosDatabase *db);
    AmigosDatabase *GetDbIns() const;
    void LoadDb();
    void UnloadDb();
    const ModInfo &GetModInfo() const;
    bool GetPortInInfo(unsigned int portId, ModPortInInfo &stIn) const;
    bool GetPortOutInfo(unsigned int portId, ModPortOutInfo &stOut) const;
    unsigned int GetOutPortIdFromLoopId(unsigned int loopId) const;
    unsigned int GetInPortIdFromLoopId(unsigned int loopId) const;
    std::map<unsigned int, ModPortInInfo> &GetPortInMap();
    std::map<unsigned int, ModPortOutInfo> &GetPortOutMap();
    void SetModInfo(const ModInfo &stInfo);
    void SetPortInInfo(unsigned int portId, const ModPortInInfo &stIn);
    void SetPortOutInfo(unsigned int portId, const ModPortOutInfo &stOut);

//protected:
    //void StreamTypeTransfer(unsigned int srcFmt, unsigned int srcType, unsigned int &dstFmt, unsigned int &dstType);

protected:
    virtual void _LoadDb() = 0;
    virtual void _UnloadDb() = 0;
    std::map<unsigned int, ModPortInInfo>  mapModInputInfo;
    std::map<unsigned int, ModPortOutInfo> mapModOutputInfo;
    ModInfo                                stModInfo;

    AmigosDatabase *pDb;
private:
    bool bNeedExtInInfo;
    bool bNeedExtOutInfo;
};
#endif /* __AMIGOS_SURFACE_BASE_H__ */
