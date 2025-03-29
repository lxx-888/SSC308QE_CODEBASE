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

#ifndef __AMIGOS_SURFACE_FILE_H__
#define __AMIGOS_SURFACE_FILE_H__

#include "amigos_surface_base.h"

class AmigosSurfaceFile : public AmigosSurfaceBase
{
public:
    struct FileOutInfo
    {
        std::string fileName;
        std::string strOutType;
        std::string strOutFmt;
        std::string strBayerId;
        std::string strPrecision;
        int         intFileOutWidth;
        int         intFileOutHeight;
        int         intPlaneNum;
        int         frameCntLimit;
        FileOutInfo() : intFileOutWidth(0), intFileOutHeight(0), intPlaneNum(0) ,frameCntLimit(-1) {}
    };
    struct FileInInfo
    {
        std::string fileName;
        int         frameCntLimit;
        int         bHead;
    };
public:
    explicit AmigosSurfaceFile(const std::string &strInSection);
    virtual ~AmigosSurfaceFile();

    bool GetFileInInfo(unsigned int portId, FileInInfo &stIn) const;
    void SetFileInInfo(unsigned int portId, const FileInInfo &stIn);
    bool GetFileOutInfo(unsigned int portId, FileOutInfo &stOut) const;
    void SetFileOutInfo(unsigned int portId, const FileOutInfo &stOut);

protected:
    std::map<unsigned int, FileInInfo>  mapInputWrFile;
    std::map<unsigned int, FileOutInfo> mapOutputRdFile;
private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};
#endif //__AMIGOS_SURFACE_FILE_H__
