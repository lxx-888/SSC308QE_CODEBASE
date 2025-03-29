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
#include <cstring>
#include <cwchar>
#include "amigos_module_rgn_metadata_define.h"
#include "amigos_surface_rgn.h"
#include "ss_cmd_base.h"
#include "amigos_module_rgn.h"
#include "ss_log.h"
#include "ss_packet.h"

static int attach(vector<string> &in_strs)
{
    AmigosModuleRgn *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleRgn, AmigosModuleBase);
    AmigosSurfaceRgn::RgnModAttachInfo attach_info;
    bool bUserTiming = false;
    attach_info.strMod      = in_strs[1];
    attach_info.intPort     = ss_cmd_atoi(in_strs[2].c_str());
    attach_info.intIsInPort = ss_cmd_atoi(in_strs[3].c_str());
    if (in_strs.size() >= 6)
    {
        attach_info.intTimingW  = ss_cmd_atoi(in_strs[4].c_str());
        attach_info.intTimingH  = ss_cmd_atoi(in_strs[5].c_str());
        bUserTiming = true;
    }
    if (!pMyClass->Attach(attach_info, bUserTiming))
    {
        ss_print(PRINT_LV_ERROR, "attach %s-%s_%d failed\n", attach_info.strMod.c_str(),
                 attach_info.intIsInPort ? "IN" : "OUT", attach_info.intPort);
        return -1;
    }
    ss_print(PRINT_LV_TRACE, "attach %s-%s_%d success\n", attach_info.strMod.c_str(),
             attach_info.intIsInPort ? "IN" : "OUT", attach_info.intPort);
    return 0;
}
static int detach(vector<string> &in_strs)
{
    AmigosModuleRgn *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleRgn, AmigosModuleBase);
    AmigosSurfaceRgn::RgnModAttachInfo attach_info;
    attach_info.strMod      = in_strs[1];
    attach_info.intPort     = ss_cmd_atoi(in_strs[2].c_str());
    attach_info.intIsInPort = ss_cmd_atoi(in_strs[3].c_str());
    if (!pMyClass->Detach(attach_info))
    {
        ss_print(PRINT_LV_ERROR, "detach %s-%s_%d failed\n", attach_info.strMod.c_str(),
                 attach_info.intIsInPort ? "IN" : "OUT", attach_info.intPort);
        return -1;
    }
    ss_print(PRINT_LV_TRACE, "detach %s-%s_%d success\n", attach_info.strMod.c_str(),
             attach_info.intIsInPort ? "IN" : "OUT", attach_info.intPort);
    return 0;
}
static int set_text_attr(vector<string> &in_strs)
{
    AmigosModuleRgn *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleRgn, AmigosModuleBase);
    AmigosSurfaceRgn *pMySurface = dynamic_cast<AmigosSurfaceRgn*>(pMyClass->GetSurface());

    unsigned int inPortId = ss_cmd_atoi(in_strs[1].c_str());
    AmigosSurfaceRgn::RgnInputInfo info;

    if (!pMySurface->GetRgnInInfo(inPortId, info))
    {
        ss_print(PRINT_LV_ERROR, "inPortId %d is not found\n", inPortId);
        return -1;
    }

    if (info.strMode != "text")
    {
        ss_print(PRINT_LV_ERROR, "inPortId %d is not text mode but %s\n", inPortId, info.strMode.c_str());
        return -1;
    }

    info.stTextInfo.intPosX  = ss_cmd_atoi(in_strs[2].c_str());
    info.stTextInfo.intPosY  = ss_cmd_atoi(in_strs[3].c_str());
    info.stTextInfo.intColor = ss_cmd_atoi(in_strs[4].c_str());

    if (!pMyClass->SetAttr(inPortId, info))
    {
        ss_print(PRINT_LV_ERROR, "set_text_attr [%d]-(%d,%d)-0x%x failed\n", inPortId, info.stTextInfo.intPosX,
                 info.stTextInfo.intPosY, info.stTextInfo.intColor);
    }
    pMySurface->SetRgnInInfo(inPortId, info);
    return 0;
}
static int set_line_attr(vector<string> &in_strs)
{
    AmigosModuleRgn *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleRgn, AmigosModuleBase);
    AmigosSurfaceRgn *pMySurface = dynamic_cast<AmigosSurfaceRgn*>(pMyClass->GetSurface());

    unsigned int inPortId = ss_cmd_atoi(in_strs[1].c_str());
    AmigosSurfaceRgn::RgnInputInfo info;

    if (!pMySurface->GetRgnInInfo(inPortId, info))
    {
        ss_print(PRINT_LV_ERROR, "inPortId %d is not found\n", inPortId);
        return -1;
    }

    if (info.strMode != "osd_frame" && info.strMode != "frame" && info.strMode != "line")
    {
        ss_print(PRINT_LV_ERROR, "inPortId %d is not line, osd_frame or frame mode but %s\n", inPortId,
                 info.strMode.c_str());
        return -1;
    }

    info.stLineInfo.intColor = info.stFrameInfo.intColor = ss_cmd_atoi(in_strs[2].c_str());
    info.stLineInfo.strThickness = info.stFrameInfo.strThickness = in_strs[3];

    if (!pMyClass->SetAttr(inPortId, info))
    {
        ss_print(PRINT_LV_ERROR, "set_line_attr [%d]-0x%x-%s failed\n", inPortId, info.stLineInfo.intColor,
                 info.stLineInfo.strThickness.c_str());
    }
    pMySurface->SetRgnInInfo(inPortId, info);
    return 0;
}
static int set_cover_attr(vector<string> &in_strs)
{
    AmigosModuleRgn *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleRgn, AmigosModuleBase);
    AmigosSurfaceRgn *pMySurface = dynamic_cast<AmigosSurfaceRgn*>(pMyClass->GetSurface());

    unsigned int inPortId = ss_cmd_atoi(in_strs[1].c_str());
    AmigosSurfaceRgn::RgnInputInfo info;

    if (!pMySurface->GetRgnInInfo(inPortId, info))
    {
        ss_print(PRINT_LV_ERROR, "inPortId %d is not found\n", inPortId);
        return -1;
    }

    if (info.strMode != "cover" && info.strMode != "poly")
    {
        ss_print(PRINT_LV_ERROR, "inPortId %d is not cover or poly mode but %s\n", inPortId,
                 info.strMode.c_str());
        return -1;
    }

    info.stCoverInfo.strType      = in_strs[2];
    info.stCoverInfo.intColor     = ss_cmd_atoi(in_strs[3].c_str());
    info.stCoverInfo.strBlockSize = in_strs[4];

    if (!pMyClass->SetAttr(inPortId, info))
    {
        ss_print(PRINT_LV_ERROR, "set_cover_attr [%d]-%s-0x%x-%s failed\n", inPortId, info.stCoverInfo.strType.c_str(),
                 info.stCoverInfo.intColor, info.stCoverInfo.strBlockSize);
        return -1;
    }
    pMySurface->SetRgnInInfo(inPortId, info);
    return 0;
}
static int set_color_invert(vector<string> &in_strs)
{
    AmigosModuleRgn *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleRgn, AmigosModuleBase);
    AmigosSurfaceRgn *pMySurface = dynamic_cast<AmigosSurfaceRgn*>(pMyClass->GetSurface());

    AmigosSurfaceRgn::RgnInfo info;

    pMySurface->GetRgnInfo(info);

    info.intColorInvertEn         = ss_cmd_atoi(in_strs[1].c_str());
    info.strColorInvertMode       = in_strs[2];
    info.intColorInvertThresholdL = ss_cmd_atoi(in_strs[3].c_str());
    info.intColorInvertThresholdH = ss_cmd_atoi(in_strs[4].c_str());

    if (!pMyClass->SetColorInvert(info, false))
    {
        ss_print(PRINT_LV_ERROR, "set_color_invert %d-%s-(%d~%d) failed\n", info.intColorInvertEn,
                 info.strColorInvertMode.c_str(), info.intColorInvertThresholdL, info.intColorInvertThresholdH);
        return -1;
    }
    pMySurface->SetRgnInfo(info);
    return 0;
}

static int push_texts(vector<string> &in_strs)
{
    AmigosModuleRgn *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleRgn, AmigosModuleBase);
    unsigned int inPortId = ss_cmd_atoi(in_strs[1].c_str());
    int cmd_count = ss_cmd_atoi(in_strs[2].c_str());
    if(cmd_count < 0)
    {
        ss_print(PRINT_LV_ERROR, "Invalid count %d\n", cmd_count);
        return -1;
    }
    unsigned int count = (unsigned int)cmd_count;


    if (in_strs.size() - 3 < count * 3)
    {
        ss_print(PRINT_LV_ERROR, "Extern parament count is not enough %d/%d\n", in_strs.size() - 3, count * 3);
        return -1;
    }

    stream_packet_info packet_info;
    packet_info.en_type              = EN_USER_META_DATA;
    packet_info.meta_data_i.reserved = E_META_DATA_TEXTS;
    packet_info.meta_data_i.size     = sizeof(AmigosRgnMetaDataTexts) + sizeof(AmigosRgnMetaDataText) * count;
    stream_packet_obj packet         = stream_packet_base::make<stream_packet>(packet_info);
    if (!packet)
    {
        ss_print(PRINT_LV_ERROR, "Packet make failed\n");
        return -1;
    }

    AmigosRgnMetaDataTexts &texts = *(AmigosRgnMetaDataTexts*)packet->meta_data.data;
    memset(&texts, 0, packet_info.meta_data_i.size);
    texts.count = count;
    for (unsigned int i = 0, j = 3; i < count; ++i, j += 3)
    {
        swprintf(texts.texts[i].str, AMIGOS_RGN_METADATA_TEXT_MAX_SIZE, L"%s", in_strs[j].c_str());
        texts.texts[i].pt.x = ss_cmd_atoi(in_strs[j + 1].c_str());
        texts.texts[i].pt.y = ss_cmd_atoi(in_strs[j + 2].c_str());
    }

    if (!pMyClass->Process(inPortId, packet))
    {
        ss_print(PRINT_LV_ERROR, "push_texts [%d]-%d ... failed\n", inPortId, count);
        return -1;
    }
    return 0;
}
static int push_lines(vector<string> &in_strs)
{
    AmigosModuleRgn *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleRgn, AmigosModuleBase);
    unsigned int inPortId = ss_cmd_atoi(in_strs[1].c_str());
    int cmd_count = ss_cmd_atoi(in_strs[2].c_str());

    if(cmd_count < 0)
    {
        ss_print(PRINT_LV_ERROR, "Invalid count %d\n", cmd_count);
        return -1;
    }
    unsigned int count = (unsigned int)cmd_count;


    if (in_strs.size() - 3 < count * 4)
    {
        ss_print(PRINT_LV_ERROR, "Extern parament count is not enough %d/%d\n", in_strs.size() - 3, count * 4);
        return -1;
    }

    stream_packet_info packet_info;
    packet_info.en_type              = EN_USER_META_DATA;
    packet_info.meta_data_i.reserved = E_META_DATA_LINES;
    packet_info.meta_data_i.size     = sizeof(AmigosRgnMetaDataLines) + sizeof(AmigosRgnMetaDataLine) * count;
    stream_packet_obj packet         = stream_packet_base::make<stream_packet>(packet_info);
    if (!packet)
    {
        ss_print(PRINT_LV_ERROR, "Packet make failed\n");
        return -1;
    }

    AmigosRgnMetaDataLines &lines = *(AmigosRgnMetaDataLines*)packet->meta_data.data;
    memset(&lines, 0, packet_info.meta_data_i.size);

    lines.count = count;
    for (unsigned int i = 0, j = 3; i < count; ++i, j += 4)
    {
        lines.lines[i].pt0.x = ss_cmd_atoi(in_strs[j].c_str());
        lines.lines[i].pt0.y = ss_cmd_atoi(in_strs[j + 1].c_str());
        lines.lines[i].pt1.x = ss_cmd_atoi(in_strs[j + 2].c_str());
        lines.lines[i].pt1.y = ss_cmd_atoi(in_strs[j + 3].c_str());
        lines.lines[i].state = E_META_DATA_STATUS_ON;
    }

    if (!pMyClass->Process(inPortId, packet))
    {
        ss_print(PRINT_LV_ERROR, "push_lines [%d]-%d ... failed\n", inPortId, count);
        return -1;
    }
    return 0;
}
static int push_rects(vector<string> &in_strs)
{
    AmigosModuleRgn *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleRgn, AmigosModuleBase);
    unsigned int inPortId = ss_cmd_atoi(in_strs[1].c_str());
    int cmd_count = ss_cmd_atoi(in_strs[2].c_str());

    if(cmd_count < 0)
    {
        ss_print(PRINT_LV_ERROR, "Invalid count %d\n", cmd_count);
        return -1;
    }
    unsigned int count = (unsigned int)cmd_count;


    if (in_strs.size() - 3 < count * 4)
    {
        ss_print(PRINT_LV_ERROR, "Extern parament count is not enough %d/%d\n", in_strs.size() - 3, count * 4);
        return -1;
    }

    stream_packet_info packet_info;
    packet_info.en_type              = EN_USER_META_DATA;
    packet_info.meta_data_i.reserved = E_META_DATA_RECT_AREAS;
    packet_info.meta_data_i.size     = sizeof(AmigosRgnMetaDataRectAreas) + sizeof(AmigosRgnMetaDataRectArea) * count;
    stream_packet_obj packet         = stream_packet_base::make<stream_packet>(packet_info);
    if (!packet)
    {
        ss_print(PRINT_LV_ERROR, "Packet make failed\n");
        return -1;
    }

    AmigosRgnMetaDataRectAreas &rects = *(AmigosRgnMetaDataRectAreas*)packet->meta_data.data;;
    memset(&rects, 0, packet_info.meta_data_i.size);
    rects.count = count;
    for (unsigned int i = 0, j = 3; i < count; ++i, j += 4)
    {
        rects.areas[i].rect.x = ss_cmd_atoi(in_strs[j].c_str());
        rects.areas[i].rect.y = ss_cmd_atoi(in_strs[j + 1].c_str());
        rects.areas[i].rect.w = ss_cmd_atoi(in_strs[j + 2].c_str());
        rects.areas[i].rect.h = ss_cmd_atoi(in_strs[j + 3].c_str());
        rects.areas[i].state  = E_META_DATA_STATUS_ON;
    }

    if (!pMyClass->Process(inPortId, packet))
    {
        ss_print(PRINT_LV_ERROR, "push_rects[%d]-%d ... failed\n", inPortId, count);
        return -1;
    }
    return 0;
}
static int push_polys(vector<string> &in_strs)
{
    AmigosModuleRgn *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleRgn, AmigosModuleBase);
    unsigned int inPortId = ss_cmd_atoi(in_strs[1].c_str());
    int cmd_count = ss_cmd_atoi(in_strs[2].c_str());

    if(cmd_count < 0)
    {
        ss_print(PRINT_LV_ERROR, "Invalid count %d\n", cmd_count);
        return -1;
    }
    unsigned int count = (unsigned int)cmd_count;


    stream_packet_info packet_info;
    packet_info.en_type              = EN_USER_META_DATA;
    packet_info.meta_data_i.reserved = E_META_DATA_POLYS;
    packet_info.meta_data_i.size     = sizeof(AmigosRgnMetaDataPolys) + sizeof(AmigosRgnMetaDataPoly) * count;
    stream_packet_obj packet         = stream_packet_base::make<stream_packet>(packet_info);
    if (!packet)
    {
        ss_print(PRINT_LV_ERROR, "Packet make failed\n");
        return -1;
    }

    AmigosRgnMetaDataPolys &polys = *(AmigosRgnMetaDataPolys *)packet->meta_data.data;
    memset(&polys, 0, sizeof(AmigosRgnMetaDataPolys));
    polys.count = count;
    for (unsigned int i = 0, j = 3; i < count; ++i, ++j)
    {
        if (j >= in_strs.size())
        {
            ss_print(PRINT_LV_ERROR, "Extern parament is not enough\n");
            return -1;
        }
        unsigned int num = ss_cmd_atoi(in_strs[j].c_str());
        if (num < 3 || num > AMIGOS_RGN_METADATA_POLY_VECTEX_MAX)
        {
            ss_print(PRINT_LV_ERROR, "vectex number %d error, need in 3 ~ %d\n", num,
                     AMIGOS_RGN_METADATA_POLY_VECTEX_MAX);
            return -1;
        }
        if (j + num * 2 >= in_strs.size())
        {
            ss_print(PRINT_LV_ERROR, "Extern parament is not enough\n");
            return -1;
        }
        polys.polys[i].vectexNum = num;
        for (unsigned int k = 0; k < num; ++k)
        {
            polys.polys[i].vectexArr[k].x = ss_cmd_atoi(in_strs[++j].c_str());
            polys.polys[i].vectexArr[k].y = ss_cmd_atoi(in_strs[++j].c_str());
        }
    }

    if (!pMyClass->Process(inPortId, packet))
    {
        ss_print(PRINT_LV_ERROR, "push_polys [%d]-%d ... failed\n", inPortId, count);
        return -1;
    }
    return 0;
}
MOD_CMDS(AmiCmdRgn)
{
    ADD_CMD_VAR_ARG("attach", attach, 3);
    ADD_CMD_HELP("attach", "[mod] [port] [isinport] { [w] [h] }",
            "Attach rgn to target module with optional timing",
            "[mod]      : module block name, such as VENC_CH0_DEV0",
            "[port]     : port id",
            "[isinport] : 0)output port, 1)input port",
            "[w]        : The timing width of target module",
            "[h]        : The timing height of target module");
    ADD_CMD("detach", detach, 3);
    ADD_CMD_HELP("detach", "[mod] [port] [isinport]",
            "Detach rgn from target module",
            "[mod]      : module block name, such as VENC_CH0_DEV0",
            "[port]     : port id",
            "[isinport] : 0)output port, 1)input port");
    ADD_CMD("set_text_attr", set_text_attr, 4);
    ADD_CMD_HELP("set_text_attr", "[port] [x] [y] [color]",
            "Set port attribute work in text mode",
            "[port]  : input port ID",
            "[x]     : x pos (in text coordinate)",
            "[y]     : y pos (in text coordinate)",
            "[color] : text color");
    ADD_CMD("set_line_attr", set_line_attr, 3);
    ADD_CMD_HELP("set_line_attr", "[port] [color] [thickness]",
            "Set port attribute work in line, frame or osd_frame mode",
            "[port]      : input port ID",
            "[color]     : line color",
            "[thickness] : line thickness, (thin, normal, thick)");
    ADD_CMD("set_cover_attr", set_cover_attr, 4);
    ADD_CMD_HELP("set_cover_attr", "[port] [type] [color] [block_size]",
            "Set port attribute work in cover or poly mode",
            "[port]       : input port ID",
            "[type]       : cover type, mosaic or color",
            "[color]      : cover color with type == color",
            "[block_size] : cover block size with type == mosaic");
    ADD_CMD("set_color_invert", set_color_invert, 4);
    ADD_CMD_HELP("set_color_invert", "[en] [mode] [low] [high]",
            "Set color invert attribute",
            "[en]   : enable color invert (0 / 1)",
            "[mode] : color invert work mode (auto or manual)",
            "[low]  : threshold low (0 ~ 255)",
            "[high] : threshold high (0 ~ 255)");
    ADD_CMD_VAR_ARG("push_texts", push_texts, 2);
    ADD_CMD_HELP("push_texts", "[port] [count] { [str] [x] [y], ... }",
            "[port]  : input port ID",
            "[count] : texts count",
            "[str]   : text string",
            "[x]     : x pos (0 ~ 8191), use default_x when x >= 8192",
            "[y]     : y pos (0 ~ 8191), use default_y when y >= 8192");
    ADD_CMD_VAR_ARG("push_lines", push_lines, 2);
    ADD_CMD_HELP("push_lines", "[port] [count] { [x0] [y0] [x1] [y1], ... }",
            "[port]  : input port ID",
            "[count] : lines count",
            "[x0]    : x0 pos (0 ~ 8191)",
            "[y0]    : y0 pos (0 ~ 8191)",
            "[x1]    : x1 pos (0 ~ 8191)",
            "[y1]    : y1 pos (0 ~ 8191)");
    ADD_CMD_VAR_ARG("push_rects", push_rects, 2);
    ADD_CMD_HELP("push_rects", "[port] [count] { [x] [y] [w] [h], ... }",
            "[port]  : input port ID",
            "[count] : rects count",
            "[x]     : x pos (0 ~ 8191)",
            "[y]     : y pos (0 ~ 8191)",
            "[w]     : width (0 ~ 8191)",
            "[h]     : height (0 ~ 8191)");
    ADD_CMD_VAR_ARG("push_polys", push_polys, 2);
    ADD_CMD_HELP("push_polys", "[port] [count] {[num] {[x] [y], ...}, ...}",
            "[port]  : input port ID",
            "[count] : polys count",
            "[num]   : poly vectex number",
            "[x]     : x pos (0 ~ 8191)",
            "[y]     : y pos (0 ~ 8191)");
}
