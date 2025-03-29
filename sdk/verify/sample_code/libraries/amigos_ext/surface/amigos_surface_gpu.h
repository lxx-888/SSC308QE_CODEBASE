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

#ifndef __AMIGOS_SURFACE_GPU_H__
#define __AMIGOS_SURFACE_GPU_H__

#include "amigos_base.h"

typedef struct Gpu_Input_Res_s
{
    unsigned int u32ResWidth;
    unsigned int u32ResHeight;
} Gpu_Input_Res_t;

typedef struct Gpu_Output_Res_s
{
    unsigned int u32ResWidth;
    unsigned int u32ResHeight;
} Gpu_Output_Res_t;

typedef struct Gpu_BinInfo_s
{
    unsigned int u32MapGridX;
    unsigned int u32MapGridY;
    unsigned int u32GridSize;
    char         Map_X_Path[260];
    char         Map_Y_Path[260];
    void         *Module;
} Gpu_BinInfo_t;

template <class T>
class AmigosSurfaceGpu : public T
{
   public:
        explicit AmigosSurfaceGpu(const std::string &strInSection) : T(strInSection, E_SYS_MOD_GPU)
        {
            for (auto itMapGpuIn = T::mapModInputInfo.begin(); itMapGpuIn != T::mapModInputInfo.end(); itMapGpuIn++)
            {
                stInRes.u32ResWidth = T::pDb->GetInUint(itMapGpuIn->second.curLoopId, "VID_W", 1920);
                stInRes.u32ResHeight = T::pDb->GetInUint(itMapGpuIn->second.curLoopId, "VID_H", 1088);
            }

            for (auto itMapGpuOut = T::mapModOutputInfo.begin(); itMapGpuOut != T::mapModOutputInfo.end(); itMapGpuOut++)
            {
                stOutRes.u32ResWidth = T::pDb->GetOutUint(itMapGpuOut->second.curLoopId, "VID_W", 1920);
                stOutRes.u32ResHeight = T::pDb->GetOutUint(itMapGpuOut->second.curLoopId, "VID_H", 1088);
            }
            strcpy(stBinInfo.Map_X_Path, (T::pDb->GetModStr("MAP_X_PATH")).c_str());
            strcpy(stBinInfo.Map_Y_Path, (T::pDb->GetModStr("MAP_Y_PATH")).c_str());
            stBinInfo.u32MapGridX = T::pDb->GetModUint("MAP_GRID_X");
            stBinInfo.u32MapGridY = T::pDb->GetModUint("MAP_GRID_Y");
            stBinInfo.u32GridSize = T::pDb->GetModUint("GRID_SIZE");
        }
    virtual ~AmigosSurfaceGpu() {}

    protected:
        Gpu_Input_Res_t  stInRes;
        Gpu_Output_Res_t stOutRes;
        Gpu_BinInfo_t    stBinInfo;
};
#endif //__AMIGOS_SURFACE_GPU_H__
