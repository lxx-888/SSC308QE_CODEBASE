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

#include <memory>
#include "amigos_module_init.h"
#include "amigos_module_rgn_metadata_define.h"
#include "mi_common_datatype.h"
#include "mi_vdf.h"
#include "mi_md.h"
#include "mi_od.h"
#include "mi_vg.h"
#include "amigos_module_vdf.h"
#include "ss_enum_cast.hpp"
#include "ss_packet.h"
#include "ss_thread.h"

SS_ENUM_CAST_STR(MI_VDF_WorkMode_e,
{
    { E_MI_VDF_WORK_MODE_MD, "md" },
    { E_MI_VDF_WORK_MODE_OD, "od" },
    { E_MI_VDF_WORK_MODE_VG, "vg" },
});

SS_ENUM_CAST_STR(MDMB_MODE_e,
{
    { MDMB_MODE_MB_4x4, "4x4" },
    { MDMB_MODE_MB_8x8, "8x8" },
    { MDMB_MODE_MB_16x16, "16x16" },
});
SS_ENUM_CAST_STR(MDALG_MODE_e,
{
    { MDALG_MODE_FG, "fg" },
    { MDALG_MODE_SAD, "sad" },
    { MDALG_MODE_FRAMEDIFF, "framediff" },
});
SS_ENUM_CAST_STR(ODWindow_e,
{
    { OD_WINDOW_1X1, "1x1" },
    { OD_WINDOW_2X2, "2x2" },
    { OD_WINDOW_3X3, "3x3" },
});
SS_ENUM_CAST_STR(MDSAD_OUT_CTRL_e,
{
    { MDSAD_OUT_CTRL_16BIT_SAD, "16bit" },
    { MDSAD_OUT_CTRL_8BIT_SAD, "8bit" },
    { MDSAD_OUT_CTRL_BUTT, "bult" },
});

SS_ENUM_CAST_STR(VgSize_Sensitively,
{
    { VG_SENSITIVELY_MIN, "min" },
    { VG_SENSITIVELY_LOW, "low" },
    { VG_SENSITIVELY_MIDDLE, "middle" },
    { VG_SENSITIVELY_HIGH, "high" },
    { VG_SENSITIVELY_MAX, "max" },
});
SS_ENUM_CAST_STR(VgFunction,
{
    { VG_REGION_INVASION, "inva" },
    { VG_VIRTUAL_GATE, "gate" },
});
SS_ENUM_CAST_STR(VgRegion_Dir,
{
    { VG_REGION_ENTER, "enter" },
    { VG_REGION_LEAVING, "leave" },
    { VG_REGION_CROSS, "cross" },
});

typedef enum _Od_MotionSensitively
{
    OD_MOTIONSENSITIVELY_MIN       = 0,
    OD_MOTIONSENSITIVELY_LOW       = 1,
    OD_MOTIONSENSITIVELY_MIDDLE    = 2,
    OD_MOTIONSENSITIVELY_HIGH      = 3,
    OD_MOTIONSENSITIVELY_MAX       = 4
} Od_MotionSensitively_t;

SS_ENUM_CAST_STR(Od_MotionSensitively_t,
{
    { OD_MOTIONSENSITIVELY_MIN, "min" },
    { OD_MOTIONSENSITIVELY_LOW, "low" },
    { OD_MOTIONSENSITIVELY_MIDDLE, "middle" },
    { OD_MOTIONSENSITIVELY_HIGH, "high" },
    { OD_MOTIONSENSITIVELY_MAX, "max" },
});

#define VDF_ALIGN_xUP(x, align)    ((((x) + (align - 1)) / align) * align)
#define VDF_ALIGN_xDOWN(x, align)  ((x) / align * align)

const static int VDF_MOD_PARAM_W = 720;
const static int VDF_MOD_PARAM_H = 576;

template<typename T>  //T is data as int/char/short etc
T CheckDataAlign(T _min, T _Max, T _checkData, const char* pName)
{
    if (_checkData < _min || _checkData > _Max)
    {
        AMIGOS_WRN("check data err, please check %s: %d, must in: [%d, %d]\n", pName, _checkData, _min, _Max);
    }
    if (_checkData < _min)
        _checkData = _min;
    if (_checkData > _Max)
        _checkData = _Max;
    return _checkData;
}

static unsigned char GetMapSizeAlign(MDMB_MODE_e mb_size)
{
    unsigned char mapSize = 4;
    switch (mb_size)
    {
    case MDMB_MODE_MB_4x4:
        mapSize = 4;
        break;
    case MDMB_MODE_MB_8x8:
        mapSize = 8;
        break;
    case MDMB_MODE_MB_16x16:
        mapSize = 16;
        break;
    default:
        AMIGOS_WRN("set mbsize(%d) error\n", mb_size);
        break;
    }
    return mapSize;
}
static void SetMdVdfAttr(MI_VDF_ChnAttr_t &stMdChnAttr, const AmigosSurfaceVdf::VdfInfo &stVdfInfo)
{
    unsigned char mapSize = 0;
    stMdChnAttr.enWorkMode = E_MI_VDF_WORK_MODE_MD;
    stMdChnAttr.stMdAttr.u8Enable    = 1;
    stMdChnAttr.stMdAttr.u8MdBufCnt  = 4;
    stMdChnAttr.stMdAttr.u8VDFIntvl  = 0;
    stMdChnAttr.stMdAttr.ccl_ctrl.u16InitAreaThr = 8;
    stMdChnAttr.stMdAttr.ccl_ctrl.u16Step = 2;
    stMdChnAttr.stMdAttr.stMdDynamicParamsIn.sensitivity = CheckDataAlign<uint8_t>(10, 100, stVdfInfo.stMdAttr.u32Sensitivity, "sensitivity");
    stMdChnAttr.stMdAttr.stMdDynamicParamsIn.obj_num_max = CheckDataAlign<uint32_t>(0, 90, stVdfInfo.stMdAttr.u32ObjNumMax, "obj_num_max");
    stMdChnAttr.stMdAttr.stMdStaticParamsIn.width = VDF_ALIGN_xUP(stMdChnAttr.stMdAttr.stMdStaticParamsIn.width, 16);//must < divp width & 16 align
    stMdChnAttr.stMdAttr.stMdStaticParamsIn.stride  = stMdChnAttr.stMdAttr.stMdStaticParamsIn.width;
    stMdChnAttr.stMdAttr.stMdStaticParamsIn.color     = 1;
    stMdChnAttr.stMdAttr.stMdStaticParamsIn.mb_size = ss_enum_cast<MDMB_MODE_e>::from_str(stVdfInfo.strMdMbMode);
    stMdChnAttr.stMdAttr.stMdStaticParamsIn.md_alg_mode = ss_enum_cast<MDALG_MODE_e>::from_str(stVdfInfo.strAlgMode);
    stMdChnAttr.stMdAttr.stMdStaticParamsIn.sad_out_ctrl = ss_enum_cast<MDSAD_OUT_CTRL_e>::from_str(stVdfInfo.strMdSadOutMode);
    if (MDALG_MODE_FG == stMdChnAttr.stMdAttr.stMdStaticParamsIn.md_alg_mode)
    {
        stMdChnAttr.stMdAttr.stMdDynamicParamsIn.learn_rate = CheckDataAlign<uint16_t>(1000, 30000, stVdfInfo.stMdAttr.u32LearnRate, "learn_rate");
        stMdChnAttr.stMdAttr.stMdDynamicParamsIn.md_thr = CheckDataAlign<uint32_t>(0, 99, stVdfInfo.stMdAttr.u32Thr, "md_thr");
    }
    else
    {
        stMdChnAttr.stMdAttr.stMdDynamicParamsIn.learn_rate = CheckDataAlign<uint16_t>(1, 255, stVdfInfo.stMdAttr.u32LearnRate, "learn_rate");
        stMdChnAttr.stMdAttr.stMdDynamicParamsIn.md_thr = CheckDataAlign<uint32_t>(0, 255, stVdfInfo.stMdAttr.u32Thr, "md_thr");
    }
    mapSize = GetMapSizeAlign(stMdChnAttr.stMdAttr.stMdStaticParamsIn.mb_size);
    stMdChnAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[0].x = CheckDataAlign<uint16_t>(0, stMdChnAttr.stMdAttr.stMdStaticParamsIn.width - 1, VDF_ALIGN_xDOWN(stVdfInfo.stMdAttr.stPnt[0].u32x, mapSize), "MD point0 x");
    stMdChnAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[0].y = CheckDataAlign<uint16_t>(0, stMdChnAttr.stMdAttr.stMdStaticParamsIn.height - 1, VDF_ALIGN_xDOWN(stVdfInfo.stMdAttr.stPnt[0].u32y, mapSize), "MD point0 y");
    stMdChnAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[2].x = CheckDataAlign<uint16_t>(0, stMdChnAttr.stMdAttr.stMdStaticParamsIn.width - 1, VDF_ALIGN_xUP(stVdfInfo.stMdAttr.stPnt[2].u32x, mapSize) - 1, "MD point2 x");
    stMdChnAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[2].y = CheckDataAlign<uint16_t>(0, stMdChnAttr.stMdAttr.stMdStaticParamsIn.height - 1, VDF_ALIGN_xUP(stVdfInfo.stMdAttr.stPnt[2].u32y, mapSize) - 1, "MD point2 y");
    stMdChnAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[1].x = stMdChnAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[2].x;
    stMdChnAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[1].y = stMdChnAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[0].y;
    stMdChnAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[3].x = stMdChnAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[0].x;
    stMdChnAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[3].y = stMdChnAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[2].y;
    stMdChnAttr.stMdAttr.stMdStaticParamsIn.roi_md.num = stVdfInfo.stMdAttr.u32PointNum;
    AMIGOS_INFO("MD width:%d,height:%d, 0(%dx%d), 1(%dx%d), 2(%dx%d), 3(%dx%d), point num: %d\n",
                stMdChnAttr.stMdAttr.stMdStaticParamsIn.width,
                stMdChnAttr.stMdAttr.stMdStaticParamsIn.height,
                stMdChnAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[0].x,
                stMdChnAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[0].y,
                stMdChnAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[1].x,
                stMdChnAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[1].y,
                stMdChnAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[2].x,
                stMdChnAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[2].y,
                stMdChnAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[3].x,
                stMdChnAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[3].y,
                stMdChnAttr.stMdAttr.stMdStaticParamsIn.roi_md.num);
    AMIGOS_INFO("md model is %d\n", stMdChnAttr.stMdAttr.stMdStaticParamsIn.md_alg_mode);
    return;
}

static void SetOdVdfAttr(MI_VDF_ChnAttr_t &stOdChnAttr, const AmigosSurfaceVdf::VdfInfo &stVdfInfo)
{
    stOdChnAttr.enWorkMode = E_MI_VDF_WORK_MODE_OD;
    stOdChnAttr.stOdAttr.u8OdBufCnt  = 4;
    stOdChnAttr.stOdAttr.u8VDFIntvl  = 0;
    stOdChnAttr.stOdAttr.stOdDynamicParamsIn.thd_tamper     = 3;
    stOdChnAttr.stOdAttr.stOdStaticParamsIn.inImgW = VDF_ALIGN_xUP(stOdChnAttr.stOdAttr.stOdStaticParamsIn.inImgW, 16);
    stOdChnAttr.stOdAttr.stOdStaticParamsIn.inImgH = VDF_ALIGN_xUP(stOdChnAttr.stOdAttr.stOdStaticParamsIn.inImgH, 2);
    stOdChnAttr.stOdAttr.stOdStaticParamsIn.inImgStride = stOdChnAttr.stOdAttr.stOdStaticParamsIn.inImgW;
    stOdChnAttr.stOdAttr.stOdStaticParamsIn.nClrType = OD_Y;
    stOdChnAttr.stOdAttr.stOdStaticParamsIn.div = ss_enum_cast<ODWindow_e>::from_str(stVdfInfo.strOdWindows);
    stOdChnAttr.stOdAttr.stOdStaticParamsIn.alpha = 2;
    stOdChnAttr.stOdAttr.stOdStaticParamsIn.M = 120;
    stOdChnAttr.stOdAttr.stOdStaticParamsIn.MotionSensitivity = ss_enum_cast<Od_MotionSensitively_t>::from_str(stVdfInfo.strMotionSensitivity);
    stOdChnAttr.stOdAttr.stOdDynamicParamsIn.tamper_blk_thd = 1;
    stOdChnAttr.stOdAttr.stOdDynamicParamsIn.min_duration   = 15;
    if (stVdfInfo.strSensitivity == "low")
    {
        stOdChnAttr.stOdAttr.stOdDynamicParamsIn.tamper_blk_thd = 8;
        stOdChnAttr.stOdAttr.stOdDynamicParamsIn.min_duration   = 30;
    }
    if (stVdfInfo.strSensitivity == "middle")
    {
        stOdChnAttr.stOdAttr.stOdDynamicParamsIn.tamper_blk_thd = 4;
        stOdChnAttr.stOdAttr.stOdDynamicParamsIn.min_duration   = 15;
    }
    if (stVdfInfo.strSensitivity == "high")
    {
        stOdChnAttr.stOdAttr.stOdDynamicParamsIn.tamper_blk_thd = 2;  //if 1x1 subwindow, this value should be 1
        stOdChnAttr.stOdAttr.stOdDynamicParamsIn.min_duration   = 5;
    }
    stOdChnAttr.stOdAttr.stOdStaticParamsIn.roi_od.num = stVdfInfo.stOdAttr.u32PointNum;
    for (unsigned int i = 0; i < stVdfInfo.stOdAttr.u32PointNum; i++)
    {
        stOdChnAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[i].x = CheckDataAlign<uint16_t>(0, stOdChnAttr.stOdAttr.stOdStaticParamsIn.inImgW - 1, stVdfInfo.stOdAttr.stPnt[i].u32x, "OD pointNum x");
        stOdChnAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[i].y = CheckDataAlign<uint16_t>(0, stOdChnAttr.stOdAttr.stOdStaticParamsIn.inImgH - 1, stVdfInfo.stOdAttr.stPnt[i].u32y, "OD pointNum y");
    }
    AMIGOS_INFO("OD width:%d,height:%d, 0(%dx%d), 1(%dx%d), 2(%dx%d), 3(%dx%d)\n",
                stOdChnAttr.stOdAttr.stOdStaticParamsIn.inImgW,
                stOdChnAttr.stOdAttr.stOdStaticParamsIn.inImgH,
                stOdChnAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[0].x,
                stOdChnAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[0].y,
                stOdChnAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[1].x,
                stOdChnAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[1].y,
                stOdChnAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[2].x,
                stOdChnAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[2].y,
                stOdChnAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[3].x,
                stOdChnAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[3].y);
    return;
}

static void SetVgVdfAttr(MI_VDF_ChnAttr_t &stVgChnAttr, const AmigosSurfaceVdf::VdfInfo &stVdfInfo)
{
    stVgChnAttr.enWorkMode = E_MI_VDF_WORK_MODE_VG;
    stVgChnAttr.stVgAttr.u8VgBufCnt  = 4;
    stVgChnAttr.stVgAttr.u8VDFIntvl  = 0;
    stVgChnAttr.stVgAttr.height = VDF_ALIGN_xUP(stVgChnAttr.stVgAttr.height, 2);
    stVgChnAttr.stVgAttr.width = VDF_ALIGN_xUP(stVgChnAttr.stVgAttr.width, 16);
    stVgChnAttr.stVgAttr.stride = stVgChnAttr.stVgAttr.width;
    stVgChnAttr.stVgAttr.object_size_thd = ss_enum_cast<VgSize_Sensitively>::from_str(stVdfInfo.strSensitivity);
    stVgChnAttr.stVgAttr.indoor = 1;
    stVgChnAttr.stVgAttr.function_state = ss_enum_cast<VgFunction>::from_str(stVdfInfo.strAlgMode);
    stVgChnAttr.stVgAttr.line_number = stVdfInfo.stVgAttr.u32LineNum;
    if (VG_VIRTUAL_GATE == stVgChnAttr.stVgAttr.function_state)
    {
        for (unsigned int i = 0; i < stVdfInfo.stVgAttr.u32LineNum; i++)
        {
            stVgChnAttr.stVgAttr.line[i].px.x = CheckDataAlign<unsigned int>(30, stVgChnAttr.stVgAttr.width - 30, stVdfInfo.stVgAttr.stLine[i].px.u32x, "VG line pointNum x");
            stVgChnAttr.stVgAttr.line[i].px.y = CheckDataAlign<unsigned int>(30, stVgChnAttr.stVgAttr.height - 30, stVdfInfo.stVgAttr.stLine[i].px.u32y, "VG line pointNum y");
            stVgChnAttr.stVgAttr.line[i].py.x = CheckDataAlign<unsigned int>(30, stVgChnAttr.stVgAttr.width - 30, stVdfInfo.stVgAttr.stLine[i].py.u32x, "VG line pointNum x");
            stVgChnAttr.stVgAttr.line[i].py.y = CheckDataAlign<unsigned int>(30, stVgChnAttr.stVgAttr.height - 30, stVdfInfo.stVgAttr.stLine[i].py.u32y, "VG line pointNum y");
            stVgChnAttr.stVgAttr.line[i].pdx.x = CheckDataAlign<unsigned int>(30, stVgChnAttr.stVgAttr.width - 30, stVdfInfo.stVgAttr.stLine[i].pdx.u32x, "VG line pointNum x");
            stVgChnAttr.stVgAttr.line[i].pdx.y = CheckDataAlign<unsigned int>(30, stVgChnAttr.stVgAttr.height - 30, stVdfInfo.stVgAttr.stLine[i].pdx.u32y, "VG line pointNum y");
            stVgChnAttr.stVgAttr.line[i].pdy.x = CheckDataAlign<unsigned int>(30, stVgChnAttr.stVgAttr.width - 30, stVdfInfo.stVgAttr.stLine[i].pdy.u32x, "VG line pointNum x");
            stVgChnAttr.stVgAttr.line[i].pdy.y = CheckDataAlign<unsigned int>(30, stVgChnAttr.stVgAttr.height - 30, stVdfInfo.stVgAttr.stLine[i].pdy.u32y, "VG line pointNum y");
        }
        AMIGOS_INFO("VG gate type, line_number=%d, function_state=%d\n", stVgChnAttr.stVgAttr.line_number, stVgChnAttr.stVgAttr.function_state);
        return;
    }
    stVgChnAttr.stVgAttr.vg_region.p_one.x = CheckDataAlign<unsigned int>(30, stVgChnAttr.stVgAttr.width - 30, stVdfInfo.stVgAttr.stPt[0].u32x, "VG region pointNum x");
    stVgChnAttr.stVgAttr.vg_region.p_one.y = CheckDataAlign<unsigned int>(30, stVgChnAttr.stVgAttr.height - 30, stVdfInfo.stVgAttr.stPt[0].u32y, "VG region pointNum y");
    stVgChnAttr.stVgAttr.vg_region.p_two.x = CheckDataAlign<unsigned int>(30, stVgChnAttr.stVgAttr.width - 30, stVdfInfo.stVgAttr.stPt[1].u32x, "VG region pointNum x");
    stVgChnAttr.stVgAttr.vg_region.p_two.y = CheckDataAlign<unsigned int>(30, stVgChnAttr.stVgAttr.height - 30, stVdfInfo.stVgAttr.stPt[1].u32y, "VG region pointNum y");
    stVgChnAttr.stVgAttr.vg_region.p_three.x = CheckDataAlign<unsigned int>(30, stVgChnAttr.stVgAttr.width - 30, stVdfInfo.stVgAttr.stPt[2].u32x, "VG region pointNum x");
    stVgChnAttr.stVgAttr.vg_region.p_three.y = CheckDataAlign<unsigned int>(30, stVgChnAttr.stVgAttr.height - 30, stVdfInfo.stVgAttr.stPt[2].u32y, "VG region pointNum y");
    stVgChnAttr.stVgAttr.vg_region.p_four.x = CheckDataAlign<unsigned int>(30, stVgChnAttr.stVgAttr.width - 30, stVdfInfo.stVgAttr.stPt[3].u32x, "VG region pointNum x");
    stVgChnAttr.stVgAttr.vg_region.p_four.y = CheckDataAlign<unsigned int>(30, stVgChnAttr.stVgAttr.height - 30, stVdfInfo.stVgAttr.stPt[3].u32y, "VG region pointNum y");
    //Set region direction
    stVgChnAttr.stVgAttr.vg_region.region_dir = ss_enum_cast<VgRegion_Dir>::from_str(stVdfInfo.strVgRegionDir);
    if (VG_REGION_CROSS == stVgChnAttr.stVgAttr.vg_region.region_dir)
    {
        stVgChnAttr.stVgAttr.vg_region.spec_dir_state = ss_enum_cast<VgRegion_Dir>::from_str(stVdfInfo.strVgSpecDirState);
        if (VG_REGION_ENTER != stVgChnAttr.stVgAttr.vg_region.spec_dir_state && VG_REGION_LEAVING != stVgChnAttr.stVgAttr.vg_region.spec_dir_state)
        {
            stVgChnAttr.stVgAttr.vg_region.spec_dir_state = VG_REGION_ENTER;
        }
    }
    AMIGOS_INFO("VG line_number=%d, function_state=%d\n", stVgChnAttr.stVgAttr.line_number, stVgChnAttr.stVgAttr.function_state);
    return;
}

//set vg result before
static int CalcVgNum(const AmigosSurfaceVdf::VdfInfo &stVdfInfo, stream_packet_obj &packet, const unsigned int& inportWith, const unsigned int& inportHeight)
{
    MI_U8 i = 0x0;// offset = 0x0;
    VgFunction algMode = ss_enum_cast<VgFunction>::from_str(stVdfInfo.strAlgMode);
    stream_packet_info packet_info;
    packet_info.en_type              = EN_USER_META_DATA;
    packet_info.meta_data_i.reserved = E_META_DATA_LINES;
    if (VG_VIRTUAL_GATE == algMode)
    {
        packet_info.meta_data_i.size     = sizeof(AmigosRgnMetaDataLines) + stVdfInfo.stVgAttr.u32LineNum * sizeof(AmigosRgnMetaDataLine);
        packet = stream_packet_base::make<stream_packet>(packet_info);
        assert(packet);
        struct AmigosRgnMetaDataLines *pstRgnData = (struct AmigosRgnMetaDataLines*)packet->meta_data.data;
        for (i = 0; i < stVdfInfo.stVgAttr.u32LineNum; i++)
        {
            pstRgnData->lines[i].pt0.x = (AMIGOS_RGN_METADATA_COORDINATE_MAX_W * stVdfInfo.stVgAttr.stLine[i].px.u32x / inportWith) & 0xFFFE;
            pstRgnData->lines[i].pt0.y = (AMIGOS_RGN_METADATA_COORDINATE_MAX_H * stVdfInfo.stVgAttr.stLine[i].px.u32y / inportHeight) & 0xFFFE;
            pstRgnData->lines[i].pt1.x = (AMIGOS_RGN_METADATA_COORDINATE_MAX_W * stVdfInfo.stVgAttr.stLine[i].py.u32x / inportWith) & 0xFFFE;
            pstRgnData->lines[i].pt1.y = (AMIGOS_RGN_METADATA_COORDINATE_MAX_H * stVdfInfo.stVgAttr.stLine[i].py.u32y / inportHeight) & 0xFFFE;
            pstRgnData->lines[i].state = E_META_DATA_STATUS_OFF;
        }
        pstRgnData->count = stVdfInfo.stVgAttr.u32LineNum;
        return 0;
    }
    packet_info.meta_data_i.size     = sizeof(AmigosRgnMetaDataLines) + 4 * sizeof(AmigosRgnMetaDataLine);
    packet = stream_packet_base::make<stream_packet>(packet_info);
    assert(packet);
    struct AmigosRgnMetaDataLines *pstRgnData = (struct AmigosRgnMetaDataLines*)packet->meta_data.data;
    for (i = 0; i < 3; i++)
    {
        pstRgnData->lines[i].pt0.x = (AMIGOS_RGN_METADATA_COORDINATE_MAX_W * stVdfInfo.stVgAttr.stPt[i].u32x / inportWith) & 0xFFFE;
        pstRgnData->lines[i].pt0.y = (AMIGOS_RGN_METADATA_COORDINATE_MAX_H * stVdfInfo.stVgAttr.stPt[i].u32y / inportHeight) & 0xFFFE;
        pstRgnData->lines[i].pt1.x = (AMIGOS_RGN_METADATA_COORDINATE_MAX_W * stVdfInfo.stVgAttr.stPt[i + 1].u32x / inportWith) & 0xFFFE;
        pstRgnData->lines[i].pt1.y = (AMIGOS_RGN_METADATA_COORDINATE_MAX_H * stVdfInfo.stVgAttr.stPt[i + 1].u32y / inportHeight) & 0xFFFE;
        pstRgnData->lines[i].state = E_META_DATA_STATUS_OFF;
    }
    pstRgnData->lines[i].pt0.x = (AMIGOS_RGN_METADATA_COORDINATE_MAX_W * stVdfInfo.stVgAttr.stPt[i].u32x / inportWith) & 0xFFFE;
    pstRgnData->lines[i].pt0.y = (AMIGOS_RGN_METADATA_COORDINATE_MAX_H * stVdfInfo.stVgAttr.stPt[i].u32y / inportHeight) & 0xFFFE;
    pstRgnData->lines[i].pt1.x = (AMIGOS_RGN_METADATA_COORDINATE_MAX_W * stVdfInfo.stVgAttr.stPt[0].u32x / inportWith) & 0xFFFE;
    pstRgnData->lines[i].pt1.y = (AMIGOS_RGN_METADATA_COORDINATE_MAX_H * stVdfInfo.stVgAttr.stPt[0].u32y / inportHeight) & 0xFFFE;
    pstRgnData->lines[i].state = E_META_DATA_STATUS_OFF;
    pstRgnData->count = 4;
    return 0;
}

//Get vg result
static int GetVgResult(const AmigosSurfaceVdf::VdfInfo & stVdfInfo, const MI_VG_Result_t& tVgRes, const struct AmigosRgnMetaDataLines &srcData, stream_packet_obj &resData)
{
    MI_U8 i = 0x0;
    VgFunction algMode = ss_enum_cast<VgFunction>::from_str(stVdfInfo.strAlgMode);
    struct AmigosRgnMetaDataLines *pstRgnData = (struct AmigosRgnMetaDataLines*)resData->meta_data.data;
    memcpy(pstRgnData, &srcData, sizeof(struct AmigosRgnMetaDataLines) + srcData.count * sizeof(AmigosRgnMetaDataLine));
    if (0 == tVgRes.alarm_cnt)
    {
        return 0;
    }
    if (VG_VIRTUAL_GATE == algMode)
    {
        for (i = 0; i < srcData.count; i++)
        {
            if (1 == tVgRes.alarm[i])
            {
                pstRgnData->lines[i].state = E_META_DATA_STATUS_ON; //detect line alarm, other not alarm.
            }
        }
        return 0;
    }
    for (i = 0; i < srcData.count; i++)
    {
        pstRgnData->lines[i].state = E_META_DATA_STATUS_ON;   //all alarm
    }
    return 0;
}

std::map<unsigned int, unsigned int> AmigosModuleVdf::mapVdfMode = {
    { E_MI_VDF_WORK_MODE_MD, 0 },
    { E_MI_VDF_WORK_MODE_OD, 0 },
    { E_MI_VDF_WORK_MODE_VG, 0 },
};
unsigned int AmigosModuleVdf::vdfInitCount = 0;

AmigosModuleVdf::AmigosModuleVdf(const std::string &strSection)
    : AmigosSurfaceVdf(strSection)
    , AmigosModuleMiBase(this)
    , bLastSend(false)
    , bVgFirstSend(false)
    , inportWith(VDF_MOD_PARAM_W)
    , inportHeight(VDF_MOD_PARAM_H)
    , streamMapWidth(0)
    , streamMapHeight(0)
    , vdfMapStartX(0)
    , vdfMapStartY(0)
    , vdfMapEndX(0)
    , vdfMapEndY(0)
    , vdfPacketInfoObj(nullptr)
    , threadHandle(nullptr)
{
}

AmigosModuleVdf::~AmigosModuleVdf()
{
}
void AmigosModuleVdf::_Init()
{
    auto mode  = ss_enum_cast<MI_VDF_WorkMode_e>::from_str(this->stVdfInfo.strVdfMode);
    if (!vdfInitCount)
    {
        MI_VDF_Init();
        AMIGOS_INFO("MI_VDF_Init\n");
    }
    ++vdfInitCount;
    AMIGOS_INFO("vdf init ok! channnel: %d, mode: %d\n", this->stModInfo.chnId, mode);
}

void AmigosModuleVdf::_Deinit()
{
    auto mode = ss_enum_cast<MI_VDF_WorkMode_e>::from_str(this->stVdfInfo.strVdfMode);
    --vdfInitCount;
    if (!vdfInitCount)
    {
        MI_VDF_Uninit();
        AMIGOS_INFO("MI_VDF_Uninit\n");
    }
    AMIGOS_INFO("vdf deinit ok! channnel: %d, mode: %d\n", this->stModInfo.chnId, mode);
}
unsigned int AmigosModuleVdf::GetModId() const
{
    return E_MI_MODULE_ID_VDF;
}
unsigned int AmigosModuleVdf::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_KERNEL | E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleVdf::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
void AmigosModuleVdf::_Prepare()
{
    stream_packet_info	info;
    auto it = mapPortIn.begin();
    if (mapPortIn.end() == it)
    {
        AMIGOS_ERR("vdf chn: has no input port!, start failed!\n");
        return;
    }
    info = it->second.get_packet_info();
    switch (info.en_type)
    {
    case EN_RAW_FRAME_DATA:
        if (1 != info.raw_vid_i.plane_num)
        {
            AMIGOS_ERR("vdf chn: mybe input data resulution  error, plane num : %d!!!\n", info.raw_vid_i.plane_num);
        }
        inportWith = info.raw_vid_i.plane_info[0].width;
        inportHeight = info.raw_vid_i.plane_info[0].height;
        break;
    case EN_VIDEO_CODEC_DATA:
        inportWith = info.es_vid_i.width;
        inportHeight = info.es_vid_i.height;
        AMIGOS_ERR("vdf chn: mybe input data format error, not support es data!!!\n");
        break;
    default:
        inportWith = VDF_MOD_PARAM_W;
        inportHeight = VDF_MOD_PARAM_H;
        AMIGOS_ERR("vdf chn: mybe input data format error, input data type: %d!!!\n", info.en_type);
        break;
    }
    MI_VDF_ChnAttr_t stVdfChnAttr;
    MI_VDF_WorkMode_e enModeType = ss_enum_cast<MI_VDF_WorkMode_e>::from_str(this->stVdfInfo.strVdfMode);
    memset(&stVdfChnAttr, 0, sizeof(MI_VDF_ChnAttr_t));
    switch (enModeType)
    {
    case E_MI_VDF_WORK_MODE_MD:
        stVdfChnAttr.stMdAttr.stMdStaticParamsIn.width = inportWith;
        stVdfChnAttr.stMdAttr.stMdStaticParamsIn.height = inportHeight;
        SetMdVdfAttr(stVdfChnAttr, stVdfInfo);
        this->_CalMdMap(stVdfChnAttr.stMdAttr);
        break;
    case E_MI_VDF_WORK_MODE_OD:
        stVdfChnAttr.stOdAttr.stOdStaticParamsIn.inImgW = inportWith;
        stVdfChnAttr.stOdAttr.stOdStaticParamsIn.inImgH = inportHeight;
        SetOdVdfAttr(stVdfChnAttr, stVdfInfo);
        this->_CalOdMap(stVdfChnAttr.stOdAttr);
        break;
    case E_MI_VDF_WORK_MODE_VG:
        stVdfChnAttr.stVgAttr.width = inportWith;
        stVdfChnAttr.stVgAttr.height = inportHeight;
        SetVgVdfAttr(stVdfChnAttr, stVdfInfo);
        CalcVgNum(stVdfInfo, vdfPacketInfoObj, inportWith, inportHeight);
        break;
    default:
        AMIGOS_ERR("vdf init failed! mode type: %d\n", enModeType);
        return;
    }
    if (MI_SUCCESS != MI_VDF_CreateChn(stModInfo.chnId, &stVdfChnAttr))
    {
        AMIGOS_ERR("vdf init failed! mode type: %d\n", enModeType);
        return;
    }
    auto itMode = mapVdfMode.find(enModeType);
    assert(itMode != mapVdfMode.end());
    if (!itMode->second)
    {
        AMIGOS_INFO("Vdf run model: %s\n", this->stVdfInfo.strVdfMode.c_str());
        MI_VDF_Run(enModeType);
    }
    ++itMode->second;
    MI_VDF_EnableSubWindow(this->stModInfo.chnId, 0, 0, 1);
}
void AmigosModuleVdf::_Unprepare()
{
    auto it = mapPortIn.begin();
    if (mapPortIn.end() == it)
    {
        AMIGOS_ERR("vdf chn: has no input port! stop failed!\n");
        return;
    }
    auto enModeType = ss_enum_cast<MI_VDF_WorkMode_e>::from_str(this->stVdfInfo.strVdfMode);
    AMIGOS_INFO("Vdf reader %s stop\n", this->GetModIdStr().c_str());
    MI_VDF_EnableSubWindow(this->stModInfo.chnId, 0, 0, 0);
    MI_VDF_DestroyChn(this->stModInfo.chnId);
    auto itMode = mapVdfMode.find(enModeType);
    assert(itMode != mapVdfMode.end());
    --itMode->second;
    if (0 == itMode->second)
    {
        AMIGOS_INFO("Vdf stop mode: %s\n", this->stVdfInfo.strVdfMode.c_str());
        MI_VDF_Stop(enModeType);
    }

}
int AmigosModuleVdf::_Connected(unsigned int outPortId, unsigned int ref)
{
    if (ref > 0)
    {
        return 0;
    }
    this->bVgFirstSend = false;
    ss_thread_attr ss_attr;
    memset(&ss_attr, 0, sizeof(ss_thread_attr));
    ss_attr.do_signal          = nullptr;
    ss_attr.do_monitor         = VdfReader;
    ss_attr.monitor_cycle_sec  = 0;
    ss_attr.monitor_cycle_nsec = 20000000;
    ss_attr.is_reset_timer     = 0;
    ss_attr.in_buf.buf         = this;
    ss_attr.in_buf.size        = 0;
    snprintf(ss_attr.thread_name, 128, "%s", this->GetModIdStr().c_str());
    threadHandle = ss_thread_open(&ss_attr);
    if (!threadHandle)
    {
        AMIGOS_ERR("Monitor return error!\n");
    }
    ss_thread_start_monitor(threadHandle);
    AMIGOS_INFO("vdf reader %s connected\n", this->GetModIdStr().c_str());
    return 0;
}
int AmigosModuleVdf::_Disconnected(unsigned int outPortId, unsigned int ref)
{
    if (ref > 0)
    {
        return 0;
    }
    ss_thread_close(threadHandle);
    threadHandle = nullptr;
    AMIGOS_INFO("vdf reader %s disconnected\n", this->GetModIdStr().c_str());
    return 0;
}

void AmigosModuleVdf::SendDetectResult(stream_packet_obj& stPacket, bool bSendforce)
{
    bool bDetected = false;
    if (E_META_DATA_MAP == stPacket->meta_data_i.reserved)
    {
        bDetected = true;
    }
    else
    {
        struct AmigosRgnMetaDataLines* pRgnData = (struct AmigosRgnMetaDataLines*)stPacket->meta_data.data;;
        for (unsigned int i = 0; i < pRgnData->count; i++)
        {
            if (pRgnData->lines[i].state == E_META_DATA_STATUS_ON)
            {
                bDetected = true;
                break;
            }
        }
    }
    if (bSendforce || bDetected || bLastSend)
    {
        for (auto& it : mapPortOut)
        {
            it.second.positive.enqueue(stPacket);
        }
        bLastSend = bDetected;
    }
}

bool AmigosModuleVdf::GetOutPortApproch()
{
    for (auto& it : mapPortOut)
    {
        if (it.second.connection_check())
        {
            return true;
        }
    }
    return false;
}

void AmigosModuleVdf::_CalMdMap(MI_VDF_MdAttr_t &mdAttr)
{
    unsigned char mapSize = GetMapSizeAlign(mdAttr.stMdStaticParamsIn.mb_size);
    this->streamMapWidth  = this->inportWith / mapSize;
    this->streamMapHeight = this->inportHeight / mapSize;
    this->vdfMapStartX    = mdAttr.stMdStaticParamsIn.roi_md.pnt[0].x / mapSize;
    this->vdfMapStartY    = mdAttr.stMdStaticParamsIn.roi_md.pnt[0].y / mapSize;
    this->vdfMapEndX      = mdAttr.stMdStaticParamsIn.roi_md.pnt[2].x / mapSize;
    this->vdfMapEndY      = mdAttr.stMdStaticParamsIn.roi_md.pnt[2].y / mapSize;
    if (this->vdfMapEndX >= this->streamMapWidth)
    {
        this->vdfMapEndX = this->streamMapWidth - 1;
    }
    if (this->vdfMapEndY >= this->streamMapHeight)
    {
        this->vdfMapEndY = this->streamMapHeight - 1;
    }
    AMIGOS_DBG("Set md area [(%d, %d), (%d, %d)] in map [%dx%d]", this->vdfMapStartX, this->vdfMapStartY,
               this->vdfMapEndX, this->vdfMapEndY, this->streamMapWidth, this->streamMapHeight);
}

void AmigosModuleVdf::_CalOdMap(MI_VDF_OdAttr_t &odAttr)
{
    unsigned char mapDiv = 1;
    unsigned int  subWindowW, subWindowH;
    switch (odAttr.stOdStaticParamsIn.div)
    {
    case OD_WINDOW_1X1:
        mapDiv = 1;
        break;
    case OD_WINDOW_2X2:
        mapDiv = 2;
        break;
    case OD_WINDOW_3X3:
        mapDiv = 3;
        break;
    default:
        AMIGOS_ERR("param (%d) error! please check config", odAttr.stOdStaticParamsIn.div);
        return;
    }
    subWindowW = (odAttr.stOdStaticParamsIn.roi_od.pnt[2].x - odAttr.stOdStaticParamsIn.roi_od.pnt[0].x) / mapDiv;
    subWindowH = (odAttr.stOdStaticParamsIn.roi_od.pnt[2].y - odAttr.stOdStaticParamsIn.roi_od.pnt[0].y) / mapDiv;
    this->streamMapWidth  = this->inportWith / subWindowW;
    this->streamMapHeight = this->inportHeight / subWindowH;
    this->vdfMapStartX    = odAttr.stOdStaticParamsIn.roi_od.pnt[0].x / subWindowW;
    this->vdfMapStartY    = odAttr.stOdStaticParamsIn.roi_od.pnt[0].y / subWindowH;
    this->vdfMapEndX      = odAttr.stOdStaticParamsIn.roi_od.pnt[2].x / subWindowW;
    this->vdfMapEndY      = odAttr.stOdStaticParamsIn.roi_od.pnt[2].y / subWindowH;
    if (this->vdfMapEndX >= this->streamMapWidth)
    {
        this->vdfMapEndX = this->streamMapWidth - 1;
    }
    if (this->vdfMapEndY >= this->streamMapHeight)
    {
        this->vdfMapEndY = this->streamMapHeight - 1;
    }
    AMIGOS_DBG("Set md area [(%d, %d), (%d, %d)] in map [%dx%d]", this->vdfMapStartX, this->vdfMapStartY,
               this->vdfMapEndX, this->vdfMapEndY, this->streamMapWidth, this->streamMapHeight);
}

void AmigosModuleVdf::_GetMdResult(MI_MD_Result_t &mdResult, AmigosRgnMetaDataMap *map)
{
    unsigned int          row, col;
    unsigned int          offset;
    map->w = this->streamMapWidth;
    map->h = this->streamMapHeight;
    offset = 0;
    memset(map->data, 0, map->w * map->h);
    for (row = this->vdfMapStartY; row <= this->vdfMapEndY; ++row)
    {
        for (col = this->vdfMapStartX; col <= this->vdfMapEndX; ++col)
        {
            if (offset >= mdResult.stSubResultSize.u32RstStatusLen)
            {
                return;
            }
            map->data[row * this->streamMapWidth + col] = mdResult.pstMdResultStatus->paddr[offset];
            ++offset;
        }
    }
}

void AmigosModuleVdf::_GetOdResult(MI_OD_Result_t &odResult, AmigosRgnMetaDataMap *map)
{
    unsigned int            row, col;
    map->w = this->streamMapWidth;
    map->h = this->streamMapHeight;
    memset(map->data, 0, map->w * map->h);
    for (row = this->vdfMapStartY; row <= this->vdfMapEndY; ++row)
    {
        for (col = this->vdfMapStartX; col <= this->vdfMapEndX; ++col)
        {
            map->data[row * this->streamMapWidth + col] =
                odResult.u8RgnAlarm[row - this->vdfMapStartY][col - this->vdfMapStartX];
        }
    }
}

void *AmigosModuleVdf::VdfReader(struct ss_thread_buffer *thread_buf)
{
    AmigosModuleVdf* pReader = (AmigosModuleVdf*)thread_buf->buf;
    if (nullptr == pReader || !pReader->GetOutPortApproch())
    {
        return nullptr;
    }
    MI_VDF_Result_t stVdfResult = { (MI_VDF_WorkMode_e)0 };
    stream_packet_obj stStreamPacket = nullptr;
    memset(&stVdfResult, 0x00, sizeof(MI_VDF_Result_t));
    stVdfResult.enWorkMode = ss_enum_cast<MI_VDF_WorkMode_e>::from_str(pReader->GetAttrInfo().strVdfMode);
    if (E_MI_VDF_WORK_MODE_VG == stVdfResult.enWorkMode)
    {
        struct AmigosRgnMetaDataLines *pstRgnData = (struct AmigosRgnMetaDataLines*)pReader->vdfPacketInfoObj->meta_data.data;
        stream_packet_info packet_info;
        packet_info.en_type              = EN_USER_META_DATA;
        packet_info.meta_data_i.reserved = E_META_DATA_LINES;
        packet_info.meta_data_i.size     = sizeof(AmigosRgnMetaDataLines) +  pstRgnData->count * sizeof(AmigosRgnMetaDataLine);
        stStreamPacket = stream_packet_base::make<stream_packet>(packet_info);
        assert(stStreamPacket);
        if (!pReader->bVgFirstSend)
        {
            GetVgResult(pReader->GetAttrInfo(), stVdfResult.stVgResult, *pstRgnData, stStreamPacket);
            pReader->SendDetectResult(stStreamPacket, true);
            pReader->bVgFirstSend = true;
        }
    }
    MI_S32 ret = MI_VDF_GetResult(pReader->stModInfo.chnId, &stVdfResult, 0);
    if (MI_SUCCESS == ret)
    {
        switch (stVdfResult.enWorkMode)
        {
        case E_MI_VDF_WORK_MODE_MD:
        {
            if (1 == stVdfResult.stMdResult.u8Enable)
            {
                stream_packet_info packet_info;
                packet_info.en_type              = EN_USER_META_DATA;
                packet_info.meta_data_i.reserved = E_META_DATA_MAP;
                packet_info.meta_data_i.size     = sizeof(AmigosRgnMetaDataMap) + pReader->streamMapWidth * pReader->streamMapHeight * sizeof(char);
                stStreamPacket                   = stream_packet_base::make<stream_packet>(packet_info);
                assert(stStreamPacket);
                pReader->_GetMdResult(stVdfResult.stMdResult, (AmigosRgnMetaDataMap*)stStreamPacket->meta_data.data);
            }
        }
        break;
        case E_MI_VDF_WORK_MODE_OD:
        {
            if (1 == stVdfResult.stOdResult.u8Enable)
            {
                stream_packet_info packet_info;
                packet_info.en_type              = EN_USER_META_DATA;
                packet_info.meta_data_i.reserved = E_META_DATA_MAP;
                packet_info.meta_data_i.size     = sizeof(AmigosRgnMetaDataMap) + pReader->streamMapWidth * pReader->streamMapHeight * sizeof(char);
                stStreamPacket                   = stream_packet_base::make<stream_packet>(packet_info);
                assert(stStreamPacket);
                pReader->_GetOdResult(stVdfResult.stOdResult, (AmigosRgnMetaDataMap*)stStreamPacket->meta_data.data);
            }
        }
        break;
        case E_MI_VDF_WORK_MODE_VG:
        {
            struct AmigosRgnMetaDataLines *pstRgnData = (struct AmigosRgnMetaDataLines*)pReader->vdfPacketInfoObj->meta_data.data;
            stream_packet_info packet_info;
            packet_info.en_type              = EN_USER_META_DATA;
            packet_info.meta_data_i.reserved = E_META_DATA_LINES;
            packet_info.meta_data_i.size     = sizeof(AmigosRgnMetaDataLines) + ((struct AmigosRgnMetaDataLines*)pReader->vdfPacketInfoObj->meta_data.data)->count * sizeof(AmigosRgnMetaDataLine);
            stStreamPacket                   = stream_packet_base::make<stream_packet>(packet_info);
            assert(stStreamPacket);
            GetVgResult(pReader->GetAttrInfo(), stVdfResult.stVgResult, *pstRgnData, stStreamPacket);
        }
        break;
        default:
            AMIGOS_ERR("vdf init failed! mode type: %d\n", stVdfResult.enWorkMode);
            return nullptr;
        }
        MI_VDF_PutResult(pReader->stModInfo.chnId, &stVdfResult);
        if (nullptr == stStreamPacket)
        {
            return nullptr;
        }
        pReader->SendDetectResult(stStreamPacket);
    }
    return nullptr;
}

AMIGOS_MODULE_INIT("VDF", AmigosModuleVdf);

