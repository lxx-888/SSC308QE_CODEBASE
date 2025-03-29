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

#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include "amigos_module_init.h"
#include "amigos_module_rgn_metadata_define.h"
#include "amigos_surface_base.h"
#include "mi_sys.h"
#include "mi_sys_datatype.h"
#include "mi_ipu_datatype.h"
#include "mi_ipu.h"
#include "ss_enum_cast.hpp"
#include "ss_packet.h"
#include "sstar_det_api.h"
#include "mi_common_datatype.h"

#include "amigos_module_det.h"
#include "amigos_module_mi_base.h"

#define CHECK_RESULT(expectation, erraction, function, ...) \
    ({                                                      \
        if ((expectation) == (function(__VA_ARGS__)))       \
        {                                                   \
            AMIGOS_INFO("%s Success\n", #function);         \
        }                                                   \
        else                                                \
        {                                                   \
            AMIGOS_ERR("%s Failed\n", #function);           \
            erraction;                                      \
        }                                                   \
    })

#define CHECK_RESULT_NO_LOG(expectation, erraction, function, ...) \
    ({                                                             \
        if ((expectation) != (function(__VA_ARGS__)))              \
        {                                                          \
            AMIGOS_ERR("%s Failed\n", #function);                  \
            erraction;                                             \
        }                                                          \
    })

enum det_detection_mode
{
    EN_DET_DETECTION_MODE_FACE,
    EN_DET_DETECTION_MODE_PERSON,
    EN_DET_DETECTION_MODE_PERSON_FACE,
    EN_DET_DETECTION_MODE_TRAFFIC_SIGNS,
    EN_DET_DETECTION_MODE_PERSON_CAR,
    EN_DET_DETECTION_MODE_MAX,
};

SS_ENUM_CAST_STR(det_detection_mode, {
    {EN_DET_DETECTION_MODE_FACE          , "fd"  },
    {EN_DET_DETECTION_MODE_PERSON        , "pd"  },
    {EN_DET_DETECTION_MODE_PERSON_FACE   , "pfd" },
    {EN_DET_DETECTION_MODE_TRAFFIC_SIGNS , "tsd" },
    {EN_DET_DETECTION_MODE_PERSON_CAR    , "pcd" },
});

AmigosModuleDet::LinkerDetIn::LinkerDetIn(unsigned int port, AmigosModuleDet *thisModule)
    : inPortId(port), thisModule(thisModule)
{
}
AmigosModuleDet::LinkerDetIn::~LinkerDetIn()
{
}
int AmigosModuleDet::LinkerDetIn::enqueue(stream_packet_obj &packet)
{
    stream_packet_obj pa, va;
    stream_packet_info::raw_convert_vpa_packet(packet, pa, va);
    if (!pa || !va)
    {
        AMIGOS_ERR("Packet %s can not covert pa or va raw data\n", packet->get_type().c_str());
        return -1;
    }
    if (packet->raw_vid_i.plane_num != 1)
    {
        AMIGOS_ERR("Packet plane %d is not support.\n", packet->raw_vid_i.plane_num);
        return -1;
    }
    return thisModule->SendData(pa, va);
}
stream_packet_obj AmigosModuleDet::LinkerDetIn::dequeue(unsigned int ms)
{
    return nullptr;
}
AmigosModuleDet::AmigosModuleDet(const std::string &strInSection)
    : AmigosSurfaceDet(strInSection), AmigosModuleMiBase(this), detect_handle(NULL)
{

}
AmigosModuleDet::~AmigosModuleDet()
{

}

unsigned int AmigosModuleDet::GetModId() const
{
    return E_MI_MODULE_ID_IPU;
}
unsigned int AmigosModuleDet::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleDet::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
void AmigosModuleDet::_Init()
{
    CHECK_RESULT(MI_SUCCESS, return, ALGO_DET_CreateHandle, &this->detect_handle);
    DetectionInfo_t init_info;
    memset(&init_info, 0, sizeof(DetectionInfo_t));
    init_info.disp_size.width  = AMIGOS_RGN_METADATA_COORDINATE_MAX_W;
    init_info.disp_size.height = AMIGOS_RGN_METADATA_COORDINATE_MAX_H;
    init_info.threshold        = (float)this->stDetInfo.intThresHold / 1000;
    strncpy(init_info.ipu_firmware_path, this->stDetInfo.strFwPath.c_str(), MAX_DET_STRLEN - 1);
    strncpy(init_info.model, this->stDetInfo.strModelPath.c_str(), MAX_DET_STRLEN - 1);
    CHECK_RESULT(MI_SUCCESS, return, ALGO_DET_InitHandle, this->detect_handle, &init_info);
    InputAttr_t attr;
    memset(&attr, 0, sizeof(InputAttr_t));
    CHECK_RESULT(MI_SUCCESS, return, ALGO_DET_GetInputAttr, this->detect_handle, &attr);
    AMIGOS_INFO("Model %s : \ninput format : %d\ninput size : %dx%d\n", this->stDetInfo.strModelPath.c_str(),
                attr.format, attr.width, attr.height);
}
void AmigosModuleDet::_Deinit()
{
    if (this->detect_handle)
    {
        CHECK_RESULT(MI_SUCCESS, return, ALGO_DET_DeinitHandle, this->detect_handle);
        CHECK_RESULT(MI_SUCCESS, return, ALGO_DET_ReleaseHandle, this->detect_handle);
        this->detect_handle = NULL;
    }
}
void AmigosModuleDet::_StartIn(unsigned int inPortId)
{
}
void AmigosModuleDet::_StopIn(unsigned int inPortId)
{
}
void AmigosModuleDet::_StartOut(unsigned int outPortId)
{
}
void AmigosModuleDet::_StopOut(unsigned int outPortId)
{
}
int AmigosModuleDet::_Connected(unsigned int outPortId, unsigned int ref)
{
    const AmigosSurfaceBase::ModInfo &modInfo = GetSurface()->GetModInfo();
    AMIGOS_INFO("Nothing to do with _Connected for %s\n", modInfo.modName.c_str());
    return 0;
}
int AmigosModuleDet::_Disconnected(unsigned int outPortId, unsigned int ref)
{
    const AmigosSurfaceBase::ModInfo &modInfo = GetSurface()->GetModInfo();
    AMIGOS_INFO("Nothing to do with _Disconnected for %s\n", modInfo.modName.c_str());
    return 0;
}
int AmigosModuleDet::SendData(stream_packet_obj &pa, stream_packet_obj &va)
{
    if (!this->detect_handle)
    {
        return -1;
    }
    auto outPortIt = this->mapPortOut.find(0);
    if (outPortIt == this->mapPortOut.end())
    {
        return -1;
    }

    ALGO_Input_t algo_input;
    MI_S32       num_bboxes = 0;
    Box_t        bboxes[MAX_DET_OBJECT];

    memset(&algo_input, 0, sizeof(ALGO_Input_t));
    memset(&bboxes, 0, sizeof(Box_t));

    algo_input.buf_size = pa->raw_vid_pa.plane_data[0].size[0] + pa->raw_vid.plane_data[0].size[1]
                          + pa->raw_vid_pa.plane_data[0].size[2];
    algo_input.p_vir_addr = va->raw_vid.plane_data[0].data[0];
    algo_input.phy_addr   = pa->raw_vid_pa.plane_data[0].phy[0];
    algo_input.pts        = 0;

    CHECK_RESULT_NO_LOG(MI_SUCCESS, return -1, ALGO_DET_Run, this->detect_handle, &algo_input, bboxes, &num_bboxes);

    if (num_bboxes < 0) {
        return -1;
    }

    stream_packet_info packet_info;
    packet_info.en_type              = EN_USER_META_DATA;
    packet_info.meta_data_i.reserved = E_META_DATA_RECT_AREAS;
    packet_info.meta_data_i.size  = sizeof(AmigosRgnMetaDataRectAreas) + num_bboxes * sizeof(AmigosRgnMetaDataRectArea);
    stream_packet_obj send_packet = std::make_shared<stream_packet>(packet_info);
    assert(send_packet);
    AmigosRgnMetaDataRectAreas &rects = *(AmigosRgnMetaDataRectAreas *)send_packet->meta_data.data;
    memset(&rects, 0, sizeof(AmigosRgnMetaDataRectAreas) + num_bboxes * sizeof(AmigosRgnMetaDataRectArea));
    for (int i = 0; i < num_bboxes; ++i)
    {
        rects.areas[i].rect.x = bboxes[i].x;
        rects.areas[i].rect.y = bboxes[i].y;
        rects.areas[i].rect.w = bboxes[i].width;
        rects.areas[i].rect.h = bboxes[i].height;
        rects.areas[i].state  = E_META_DATA_STATUS_ON;
    }
    rects.count = num_bboxes;
    return outPortIt->second.positive.enqueue(send_packet);
}
ss_linker_base *AmigosModuleDet::_CreateInputNegativeLinker(unsigned int inPortId)
{
    return new LinkerDetIn(inPortId, this);
}
stream_packer *AmigosModuleDet::_CreateInputStreamPacker(unsigned int inPortId, bool &bFast)
{
    return new StreamPackerSysMma();
}
int AmigosModuleDet::_EnqueueOut(unsigned int outPortId, stream_packet_obj &packet)
{
    AMIGOS_ERR("Mod %s port %d not support\n", this->GetModIdStr().c_str(), outPortId);
    return -1;
}
int AmigosModuleDet::_DequeueOut(unsigned int outPortId, stream_packet_obj &packet)
{
    AMIGOS_ERR("Mod %s port %d not support\n", this->GetModIdStr().c_str(), outPortId);
    return -1;
}
stream_packet_obj AmigosModuleDet::_DequeueFromInside(unsigned int outPortId, unsigned int ms)
{
    AMIGOS_ERR("Mod %s port %d not support\n", this->GetModIdStr().c_str(), outPortId);
    return nullptr;
}
bool AmigosModuleDet::SetThreshold(unsigned int threshold)
{
    if (!this->detect_handle)
    {
        return false;
    }
    CHECK_RESULT(MI_SUCCESS, return false, ALGO_DET_SetThreshold, this->detect_handle, (MI_FLOAT)threshold / 1000);
    return true;
}
AMIGOS_MODULE_INIT("DET", AmigosModuleDet);

