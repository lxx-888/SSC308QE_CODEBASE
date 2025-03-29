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

#include "mi_common_datatype.h"
#include "mi_rgn_datatype.h"
#include "mi_rgn.h"
#include "ssos_def.h"
#include "ptree_log.h"
#include "ptree_maker.h"
#include "ptree_packer.h"
#include "ptree_packet.h"
#include "ptree_linker.h"
#include "ssos_mem.h"
#include "ptree_mod_sys.h"
#include "ptree_mod_rgn.h"
#include "ptree_sur_rgn.h"
#include "ptree_rgn_packet.h"
#include "ptree_gl.h"

#define CHECK_POINTER(_ptr, _err)            \
    (                                        \
        {                                    \
            if (!(_ptr))                     \
            {                                \
                PTREE_ERR("%s NULL", #_ptr); \
                _err;                        \
            }                                \
        })

#define PTREE_MOD_RGN_CAL_ABS_COORD_XY(xyAbs, xy, coordAbs, coord) xyAbs = (xy * coordAbs + coord / 2) / coord
#define PTREE_MOD_RGN_CAL_ABS_COORD_WH(whAbs, wh, xyAbs, xy, coordAbs, coord) \
    whAbs = ((xy + wh) * coordAbs + coord / 2) / coord - xyAbs

typedef struct PTREE_MOD_RGN_OsdCanvasData_s PTREE_MOD_RGN_OsdCanvasData_t;

typedef struct PTREE_MOD_RGN_ControlOps_s          PTREE_MOD_RGN_ControlOps_t;
typedef struct PTREE_MOD_RGN_Control_s             PTREE_MOD_RGN_Control_t;
typedef struct PTREE_MOD_RGN_FrameControl_s        PTREE_MOD_RGN_FrameControl_t;
typedef struct PTREE_MOD_RGN_OsdFrameControl_s     PTREE_MOD_RGN_OsdFrameControl_t;
typedef struct PTREE_MOD_RGN_OsdDotMatrixControl_s PTREE_MOD_RGN_OsdDotMatrixControl_t;
typedef struct PTREE_MOD_RGN_ChnPortData_s         PTREE_MOD_RGN_ChnPortData_t;

typedef struct PTREE_MOD_RGN_Obj_s   PTREE_MOD_RGN_Obj_t;
typedef struct PTREE_MOD_RGN_InObj_s PTREE_MOD_RGN_InObj_t;

struct PTREE_MOD_RGN_ControlOps_s
{
    PTREE_MOD_RGN_Control_t *(*fpConstruct)(PTREE_SUR_RGN_TypeInfo_t *pstTypeInfo);
    void (*fpAttach)(PTREE_MOD_RGN_Control_t *control, const PTREE_SUR_RGN_AttachInfo_t *pstAttachInfo);
    void (*fpDetach)(PTREE_MOD_RGN_Control_t *control, const PTREE_SUR_RGN_AttachInfo_t *pstAttachInfo);
    void (*fpSetParam)(PTREE_MOD_RGN_Control_t *control, const PTREE_SUR_RGN_AttachInfo_t *pstAttachInfo);
    int (*fpCheck)(PTREE_MOD_RGN_Control_t *control, PTREE_PACKET_Obj_t *packet);
    void (*fpFinish)(PTREE_MOD_RGN_Control_t *control);
    void (*fpDestruct)(PTREE_MOD_RGN_Control_t *control);
};

struct PTREE_MOD_RGN_OsdCanvasData_s
{
    struct SSOS_LIST_Head_s canvasLst;
    MI_RGN_HANDLE           handle;
    unsigned int            width;
    unsigned int            height;
    unsigned int            usingCnt;
};

struct PTREE_MOD_RGN_Control_s
{
    const PTREE_MOD_RGN_ControlOps_t *ops;
    SSOS_THREAD_Mutex_t               mutex;
    unsigned int                      devId;
};

#define PTREE_MOD_RGN_FRAME_NUM_MAX (32)
struct PTREE_MOD_RGN_FrameControl_s
{
    PTREE_MOD_RGN_Control_t   base;
    PTREE_RGN_PACKET_Rects_t *currPacket;
    PTREE_SUR_RGN_FrameInfo_t frameInfo;
    MI_RGN_HANDLE             handles[PTREE_MOD_RGN_FRAME_NUM_MAX];
};

struct PTREE_MOD_RGN_OsdFrameControl_s
{
    PTREE_MOD_RGN_Control_t      base;
    struct SSOS_LIST_Head_s      canvasLstHead;
    PTREE_RGN_PACKET_Rects_t *   currPacket;
    PTREE_RGN_PACKET_Rects_t *   lastPacket;
    PTREE_SUR_RGN_OsdFrameInfo_t osdFrameInfo;
    unsigned char                bpp;
};
struct PTREE_MOD_RGN_OsdDotMatrixControl_s
{
    PTREE_MOD_RGN_Control_t          base;
    struct SSOS_LIST_Head_s          canvasLstHead;
    PTREE_RGN_PACKET_Map_t *         currPacket;
    PTREE_RGN_PACKET_Map_t *         lastPacket;
    PTREE_SUR_RGN_OsdDotMatrixInfo_t osdDotMatrixInfo;
    unsigned char                    bpp;
};

struct PTREE_MOD_RGN_ChnPortData_s
{
    struct SSOS_LIST_Head_s    stChnPortLst;
    PTREE_SUR_RGN_AttachInfo_t stAttachInfo;
};

struct PTREE_MOD_RGN_Obj_s
{
    PTREE_MOD_SYS_Obj_t base;
};

struct PTREE_MOD_RGN_InObj_s
{
    PTREE_MOD_InObj_t        base;
    PTREE_MOD_RGN_Control_t *pControl;
    PTREE_LINKER_Obj_t       linker;
    PTREE_PACKER_Obj_t       packer;
};

static PTREE_MOD_RGN_Control_t *_PTREE_MOD_RGN_FrameControlConstruct(PTREE_SUR_RGN_TypeInfo_t *pstTypeInfo);
static void                     _PTREE_MOD_RGN_FrameControlDestruct(struct PTREE_MOD_RGN_Control_s *control);

static void _PTREE_MOD_RGN_FrameControlAttach(struct PTREE_MOD_RGN_Control_s *  control,
                                              const PTREE_SUR_RGN_AttachInfo_t *pstAttachInfo);
static void _PTREE_MOD_RGN_FrameControlDetach(struct PTREE_MOD_RGN_Control_s *  control,
                                              const PTREE_SUR_RGN_AttachInfo_t *pstAttachInfo);
static void _PTREE_MOD_RGN_FrameControlSetParam(struct PTREE_MOD_RGN_Control_s *  control,
                                                const PTREE_SUR_RGN_AttachInfo_t *pstAttachInfo);

static int  _PTREE_MOD_RGN_FrameControlCheck(struct PTREE_MOD_RGN_Control_s *control, PTREE_PACKET_Obj_t *packet);
static void _PTREE_MOD_RGN_FrameControlFinish(struct PTREE_MOD_RGN_Control_s *control);

static PTREE_MOD_RGN_Control_t *_PTREE_MOD_RGN_OsdFrameControlConstruct(PTREE_SUR_RGN_TypeInfo_t *pstTypeInfo);
static void                     _PTREE_MOD_RGN_OsdFrameControlDestruct(struct PTREE_MOD_RGN_Control_s *control);

static void _PTREE_MOD_RGN_OsdFrameControlAttach(struct PTREE_MOD_RGN_Control_s *  control,
                                                 const PTREE_SUR_RGN_AttachInfo_t *pstAttachInfo);
static void _PTREE_MOD_RGN_OsdFrameControlDetach(struct PTREE_MOD_RGN_Control_s *  control,
                                                 const PTREE_SUR_RGN_AttachInfo_t *pstAttachInfo);
static void _PTREE_MOD_RGN_OsdFrameControlSetParam(struct PTREE_MOD_RGN_Control_s *  control,
                                                   const PTREE_SUR_RGN_AttachInfo_t *pstAttachInfo);

static int  _PTREE_MOD_RGN_OsdFrameControlCheck(struct PTREE_MOD_RGN_Control_s *control, PTREE_PACKET_Obj_t *packet);
static void _PTREE_MOD_RGN_OsdFrameControlFinish(struct PTREE_MOD_RGN_Control_s *control);

static PTREE_MOD_RGN_Control_t *_PTREE_MOD_RGN_OsdDotMatrixControlConstruct(PTREE_SUR_RGN_TypeInfo_t *pstTypeInfo);
static void                     _PTREE_MOD_RGN_OsdDotMatrixControlDestruct(struct PTREE_MOD_RGN_Control_s *control);

static void _PTREE_MOD_RGN_OsdDotMatrixControlAttach(struct PTREE_MOD_RGN_Control_s *  control,
                                                     const PTREE_SUR_RGN_AttachInfo_t *pstAttachInfo);
static void _PTREE_MOD_RGN_OsdDotMatrixControlDetach(struct PTREE_MOD_RGN_Control_s *  control,
                                                     const PTREE_SUR_RGN_AttachInfo_t *pstAttachInfo);
static void _PTREE_MOD_RGN_OsdDotMatrixControlSetParam(struct PTREE_MOD_RGN_Control_s *  control,
                                                       const PTREE_SUR_RGN_AttachInfo_t *pstAttachInfo);

static int _PTREE_MOD_RGN_OsdDotMatrixControlCheck(struct PTREE_MOD_RGN_Control_s *control, PTREE_PACKET_Obj_t *packet);
static void _PTREE_MOD_RGN_OsdDotMatrixControlFinish(struct PTREE_MOD_RGN_Control_s *control);

static PTREE_PACKET_Obj_t *_PTREE_MOD_RGN_PackerMake(PTREE_PACKER_Obj_t *packer, const PTREE_PACKET_Info_t *info);

static int                 _PTREE_MOD_RGN_InLinkerEnqueue(PTREE_LINKER_Obj_t *linker, PTREE_PACKET_Obj_t *packet);
static PTREE_PACKET_Obj_t *_PTREE_MOD_RGN_InLinkerDequeue(PTREE_LINKER_Obj_t *linker, int ms);

static int                 _PTREE_MOD_RGN_InStart(PTREE_MOD_InObj_t *modIn);
static int                 _PTREE_MOD_RGN_InStop(PTREE_MOD_InObj_t *modIn);
static int                 _PTREE_MOD_RGN_InGetType(PTREE_MOD_InObj_t *modIn);
static PTREE_LINKER_Obj_t *_PTREE_MOD_RGN_InCreateNLinker(PTREE_MOD_InObj_t *modIn);
static PTREE_PACKER_Obj_t *_PTREE_MOD_RGN_InCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast);
static void                _PTREE_MOD_RGN_InDestruct(PTREE_MOD_InObj_t *modIn);
static void                _PTREE_MOD_RGN_InFree(PTREE_MOD_InObj_t *modIn);
static PTREE_MOD_InObj_t * _PTREE_MOD_RGN_InNew(PTREE_MOD_Obj_t *mod, unsigned int loopId);

static int                 _PTREE_MOD_RGN_Init(PTREE_MOD_SYS_Obj_t *sysMod);
static int                 _PTREE_MOD_RGN_Deinit(PTREE_MOD_SYS_Obj_t *sysMod);
static PTREE_MOD_InObj_t * _PTREE_MOD_RGN_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static PTREE_MOD_OutObj_t *_PTREE_MOD_RGN_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static void                _PTREE_MOD_RGN_Destruct(PTREE_MOD_SYS_Obj_t *sysMod);
static void                _PTREE_MOD_RGN_Free(PTREE_MOD_SYS_Obj_t *sysMod);

static const PTREE_MOD_RGN_ControlOps_t G_FRAME_CONTROL_OPS = {
    .fpConstruct = _PTREE_MOD_RGN_FrameControlConstruct,
    .fpDestruct  = _PTREE_MOD_RGN_FrameControlDestruct,
    .fpAttach    = _PTREE_MOD_RGN_FrameControlAttach,
    .fpDetach    = _PTREE_MOD_RGN_FrameControlDetach,
    .fpSetParam  = _PTREE_MOD_RGN_FrameControlSetParam,
    .fpCheck     = _PTREE_MOD_RGN_FrameControlCheck,
    .fpFinish    = _PTREE_MOD_RGN_FrameControlFinish,
};

static const PTREE_MOD_RGN_ControlOps_t G_OSD_FRAME_CONTROL_OPS = {
    .fpConstruct = _PTREE_MOD_RGN_OsdFrameControlConstruct,
    .fpDestruct  = _PTREE_MOD_RGN_OsdFrameControlDestruct,
    .fpAttach    = _PTREE_MOD_RGN_OsdFrameControlAttach,
    .fpDetach    = _PTREE_MOD_RGN_OsdFrameControlDetach,
    .fpSetParam  = _PTREE_MOD_RGN_OsdFrameControlSetParam,
    .fpCheck     = _PTREE_MOD_RGN_OsdFrameControlCheck,
    .fpFinish    = _PTREE_MOD_RGN_OsdFrameControlFinish,
};

static const PTREE_MOD_RGN_ControlOps_t G_OSD_DOT_MATRIX_CONTROL_OPS = {
    .fpConstruct = _PTREE_MOD_RGN_OsdDotMatrixControlConstruct,
    .fpDestruct  = _PTREE_MOD_RGN_OsdDotMatrixControlDestruct,
    .fpAttach    = _PTREE_MOD_RGN_OsdDotMatrixControlAttach,
    .fpDetach    = _PTREE_MOD_RGN_OsdDotMatrixControlDetach,
    .fpSetParam  = _PTREE_MOD_RGN_OsdDotMatrixControlSetParam,
    .fpCheck     = _PTREE_MOD_RGN_OsdDotMatrixControlCheck,
    .fpFinish    = _PTREE_MOD_RGN_OsdDotMatrixControlFinish,
};

static const PTREE_PACKER_Ops_t G_PTREE_MOD_RGN_PACKER_OPS = {
    .make = _PTREE_MOD_RGN_PackerMake,
};
static const PTREE_PACKER_Hook_t G_PTREE_MOD_RGN_PACKER_HOOK = {};

static const PTREE_LINKER_Ops_t G_PTREE_MOD_RGN_IN_LINKER_OPS = {
    .enqueue = _PTREE_MOD_RGN_InLinkerEnqueue,
    .dequeue = _PTREE_MOD_RGN_InLinkerDequeue,
};
static const PTREE_LINKER_Hook_t G_PTREE_MOD_RGN_IN_LINKER_HOOK = {};

static const PTREE_MOD_InOps_t G_PTREE_MOD_RGN_IN_OPS = {
    .start         = _PTREE_MOD_RGN_InStart,
    .stop          = _PTREE_MOD_RGN_InStop,
    .getType       = _PTREE_MOD_RGN_InGetType,
    .createNLinker = _PTREE_MOD_RGN_InCreateNLinker,
    .createPacker  = _PTREE_MOD_RGN_InCreatePacker,
};
static const PTREE_MOD_InHook_t G_PTREE_MOD_RGN_IN_HOOK = {
    .destruct = _PTREE_MOD_RGN_InDestruct,
    .free     = _PTREE_MOD_RGN_InFree,
};

static const PTREE_MOD_SYS_Ops_t G_PTREE_MOD_RGN_SYS_OPS = {
    .init         = _PTREE_MOD_RGN_Init,
    .deinit       = _PTREE_MOD_RGN_Deinit,
    .createModIn  = _PTREE_MOD_RGN_CreateModIn,
    .createModOut = _PTREE_MOD_RGN_CreateModOut,
};
static const PTREE_MOD_SYS_Hook_t G_PTREE_MOD_RGN_SYS_HOOK = {
    .destruct = _PTREE_MOD_RGN_Destruct,
    .free     = _PTREE_MOD_RGN_Free,
};

/*
 * | group id | handle range |
 * |----------|--------------|
 * |     0    | [0, 128)     |
 * |     1    | [128, 256)   |
 * |     2    | [256, 384)   |
 * |     3    | [384, 512)   |
 * |     4    | [512, 640)   |
 * |     5    | [640, 768)   |
 * |     6    | [768, 896)   |
 * |     7    | [896, 1024)  |
 */
#define PTREE_MOD_RGN_HANDLE_GROUP_NUM            (8)
#define PTREE_MOD_RGN_HANDLE_GROUP_MAX_HANDLE_NUM (MI_RGN_MAX_HANDLE / PTREE_MOD_RGN_HANDLE_GROUP_NUM)
static MI_RGN_HANDLE g_handlePoolTable[PTREE_MOD_RGN_HANDLE_GROUP_NUM][PTREE_MOD_RGN_HANDLE_GROUP_MAX_HANDLE_NUM] = {0};
static unsigned int  g_handlePoolSizeTable[PTREE_MOD_RGN_HANDLE_GROUP_NUM]                                        = {0};
static SSOS_THREAD_Mutex_t gp_handlePoolMutex;
static SSOS_THREAD_Mutex_t gp_rgnCanvasMutex;
static unsigned int        g_handlePoolInitCnt = 0;

static MI_RGN_PaletteTable_t g_paletteTable = {
    .astElement = {
        {0xff, 0x00, 0x00, 0x00}, {0xff, 0x80, 0x00, 0x00}, {0xff, 0x00, 0x80, 0x00}, {0xff, 0x80, 0x80, 0x00},
        {0xff, 0x00, 0x00, 0x80}, {0xff, 0x80, 0x00, 0x80}, {0xff, 0x00, 0x80, 0x80}, {0xff, 0xc0, 0xc0, 0xc0},
        {0xff, 0x80, 0x80, 0x80}, {0xff, 0xff, 0x00, 0x00}, {0xff, 0x00, 0xff, 0x00}, {0xff, 0xff, 0xff, 0x00},
        {0xff, 0x00, 0x00, 0xff}, {0xff, 0xff, 0x00, 0xff}, {0xff, 0x00, 0xff, 0xff}, {0xff, 0xff, 0xff, 0xff},
        {0xff, 0x00, 0x00, 0x00}, {0xff, 0x00, 0x00, 0x5f}, {0xff, 0x00, 0x00, 0x87}, {0xff, 0x00, 0x00, 0xaf},
        {0xff, 0x00, 0x00, 0xd7}, {0xff, 0x00, 0x00, 0xff}, {0xff, 0x00, 0x5f, 0x00}, {0xff, 0x00, 0x5f, 0x5f},
        {0xff, 0x00, 0x5f, 0x87}, {0xff, 0x00, 0x5f, 0xaf}, {0xff, 0x00, 0x5f, 0xd7}, {0xff, 0x00, 0x5f, 0xff},
        {0xff, 0x00, 0x87, 0x00}, {0xff, 0x00, 0x87, 0x5f}, {0xff, 0x00, 0x87, 0x87}, {0xff, 0x00, 0x87, 0xaf},
        {0xff, 0x00, 0x87, 0xd7}, {0xff, 0x00, 0x87, 0xff}, {0xff, 0x00, 0xaf, 0x00}, {0xff, 0x00, 0xaf, 0x5f},
        {0xff, 0x00, 0xaf, 0x87}, {0xff, 0x00, 0xaf, 0xaf}, {0xff, 0x00, 0xaf, 0xd7}, {0xff, 0x00, 0xaf, 0xff},
        {0xff, 0x00, 0xd7, 0x00}, {0xff, 0x00, 0xd7, 0x5f}, {0xff, 0x00, 0xd7, 0x87}, {0xff, 0x00, 0xd7, 0xaf},
        {0xff, 0x00, 0xd7, 0xd7}, {0xff, 0x00, 0xd7, 0xff}, {0xff, 0x00, 0xff, 0x00}, {0xff, 0x00, 0xff, 0x5f},
        {0xff, 0x00, 0xff, 0x87}, {0xff, 0x00, 0xff, 0xaf}, {0xff, 0x00, 0xff, 0xd7}, {0xff, 0x00, 0xff, 0xff},
        {0xff, 0x5f, 0x00, 0x00}, {0xff, 0x5f, 0x00, 0x5f}, {0xff, 0x5f, 0x00, 0x87}, {0xff, 0x5f, 0x00, 0xaf},
        {0xff, 0x5f, 0x00, 0xd7}, {0xff, 0x5f, 0x00, 0xff}, {0xff, 0x5f, 0x5f, 0x00}, {0xff, 0x5f, 0x5f, 0x5f},
        {0xff, 0x5f, 0x5f, 0x87}, {0xff, 0x5f, 0x5f, 0xaf}, {0xff, 0x5f, 0x5f, 0xd7}, {0xff, 0x5f, 0x5f, 0xff},
        {0xff, 0x5f, 0x87, 0x00}, {0xff, 0x5f, 0x87, 0x5f}, {0xff, 0x5f, 0x87, 0x87}, {0xff, 0x5f, 0x87, 0xaf},
        {0xff, 0x5f, 0x87, 0xd7}, {0xff, 0x5f, 0x87, 0xff}, {0xff, 0x5f, 0xaf, 0x00}, {0xff, 0x5f, 0xaf, 0x5f},
        {0xff, 0x5f, 0xaf, 0x87}, {0xff, 0x5f, 0xaf, 0xaf}, {0xff, 0x5f, 0xaf, 0xd7}, {0xff, 0x5f, 0xaf, 0xff},
        {0xff, 0x5f, 0xd7, 0x00}, {0xff, 0x5f, 0xd7, 0x5f}, {0xff, 0x5f, 0xd7, 0x87}, {0xff, 0x5f, 0xd7, 0xaf},
        {0xff, 0x5f, 0xd7, 0xd7}, {0xff, 0x5f, 0xd7, 0xff}, {0xff, 0x5f, 0xff, 0x00}, {0xff, 0x5f, 0xff, 0x5f},
        {0xff, 0x5f, 0xff, 0x87}, {0xff, 0x5f, 0xff, 0xaf}, {0xff, 0x5f, 0xff, 0xd7}, {0xff, 0x5f, 0xff, 0xff},
        {0xff, 0x87, 0x00, 0x00}, {0xff, 0x87, 0x00, 0x5f}, {0xff, 0x87, 0x00, 0x87}, {0xff, 0x87, 0x00, 0xaf},
        {0xff, 0x87, 0x00, 0xd7}, {0xff, 0x87, 0x00, 0xff}, {0xff, 0x87, 0x5f, 0x00}, {0xff, 0x87, 0x5f, 0x5f},
        {0xff, 0x87, 0x5f, 0x87}, {0xff, 0x87, 0x5f, 0xaf}, {0xff, 0x87, 0x5f, 0xd7}, {0xff, 0x87, 0x5f, 0xff},
        {0xff, 0x87, 0x87, 0x00}, {0xff, 0x87, 0x87, 0x5f}, {0xff, 0x87, 0x87, 0x87}, {0xff, 0x87, 0x87, 0xaf},
        {0xff, 0x87, 0x87, 0xd7}, {0xff, 0x87, 0x87, 0xff}, {0xff, 0x87, 0xaf, 0x00}, {0xff, 0x87, 0xaf, 0x5f},
        {0xff, 0x87, 0xaf, 0x87}, {0xff, 0x87, 0xaf, 0xaf}, {0xff, 0x87, 0xaf, 0xd7}, {0xff, 0x87, 0xaf, 0xff},
        {0xff, 0x87, 0xd7, 0x00}, {0xff, 0x87, 0xd7, 0x5f}, {0xff, 0x87, 0xd7, 0x87}, {0xff, 0x87, 0xd7, 0xaf},
        {0xff, 0x87, 0xd7, 0xd7}, {0xff, 0x87, 0xd7, 0xff}, {0xff, 0x87, 0xff, 0x00}, {0xff, 0x87, 0xff, 0x5f},
        {0xff, 0x87, 0xff, 0x87}, {0xff, 0x87, 0xff, 0xaf}, {0xff, 0x87, 0xff, 0xd7}, {0xff, 0x87, 0xff, 0xff},
        {0xff, 0xaf, 0x00, 0x00}, {0xff, 0xaf, 0x00, 0x5f}, {0xff, 0xaf, 0x00, 0x87}, {0xff, 0xaf, 0x00, 0xaf},
        {0xff, 0xaf, 0x00, 0xd7}, {0xff, 0xaf, 0x00, 0xff}, {0xff, 0xaf, 0x5f, 0x00}, {0xff, 0xaf, 0x5f, 0x5f},
        {0xff, 0xaf, 0x5f, 0x87}, {0xff, 0xaf, 0x5f, 0xaf}, {0xff, 0xaf, 0x5f, 0xd7}, {0xff, 0xaf, 0x5f, 0xff},
        {0xff, 0xaf, 0x87, 0x00}, {0xff, 0xaf, 0x87, 0x5f}, {0xff, 0xaf, 0x87, 0x87}, {0xff, 0xaf, 0x87, 0xaf},
        {0xff, 0xaf, 0x87, 0xd7}, {0xff, 0xaf, 0x87, 0xff}, {0xff, 0xaf, 0xaf, 0x00}, {0xff, 0xaf, 0xaf, 0x5f},
        {0xff, 0xaf, 0xaf, 0x87}, {0xff, 0xaf, 0xaf, 0xaf}, {0xff, 0xaf, 0xaf, 0xd7}, {0xff, 0xaf, 0xaf, 0xff},
        {0xff, 0xaf, 0xd7, 0x00}, {0xff, 0xaf, 0xd7, 0x5f}, {0xff, 0xaf, 0xd7, 0x87}, {0xff, 0xaf, 0xd7, 0xaf},
        {0xff, 0xaf, 0xd7, 0xd7}, {0xff, 0xaf, 0xd7, 0xff}, {0xff, 0xaf, 0xff, 0x00}, {0xff, 0xaf, 0xff, 0x5f},
        {0xff, 0xaf, 0xff, 0x87}, {0xff, 0xaf, 0xff, 0xaf}, {0xff, 0xaf, 0xff, 0xd7}, {0xff, 0xaf, 0xff, 0xff},
        {0xff, 0xd7, 0x00, 0x00}, {0xff, 0xd7, 0x00, 0x5f}, {0xff, 0xd7, 0x00, 0x87}, {0xff, 0xd7, 0x00, 0xaf},
        {0xff, 0xd7, 0x00, 0xd7}, {0xff, 0xd7, 0x00, 0xff}, {0xff, 0xd7, 0x5f, 0x00}, {0xff, 0xd7, 0x5f, 0x5f},
        {0xff, 0xd7, 0x5f, 0x87}, {0xff, 0xd7, 0x5f, 0xaf}, {0xff, 0xd7, 0x5f, 0xd7}, {0xff, 0xd7, 0x5f, 0xff},
        {0xff, 0xd7, 0x87, 0x00}, {0xff, 0xd7, 0x87, 0x5f}, {0xff, 0xd7, 0x87, 0x87}, {0xff, 0xd7, 0x87, 0xaf},
        {0xff, 0xd7, 0x87, 0xd7}, {0xff, 0xd7, 0x87, 0xff}, {0xff, 0xd7, 0xaf, 0x00}, {0xff, 0xd7, 0xaf, 0x5f},
        {0xff, 0xd7, 0xaf, 0x87}, {0xff, 0xd7, 0xaf, 0xaf}, {0xff, 0xd7, 0xaf, 0xd7}, {0xff, 0xd7, 0xaf, 0xff},
        {0xff, 0xd7, 0xd7, 0x00}, {0xff, 0xd7, 0xd7, 0x5f}, {0xff, 0xd7, 0xd7, 0x87}, {0xff, 0xd7, 0xd7, 0xaf},
        {0xff, 0xd7, 0xd7, 0xd7}, {0xff, 0xd7, 0xd7, 0xff}, {0xff, 0xd7, 0xff, 0x00}, {0xff, 0xd7, 0xff, 0x5f},
        {0xff, 0xd7, 0xff, 0x87}, {0xff, 0xd7, 0xff, 0xaf}, {0xff, 0xd7, 0xff, 0xd7}, {0xff, 0xd7, 0xff, 0xff},
        {0xff, 0xff, 0x00, 0x00}, {0xff, 0xff, 0x00, 0x5f}, {0xff, 0xff, 0x00, 0x87}, {0xff, 0xff, 0x00, 0xaf},
        {0xff, 0xff, 0x00, 0xd7}, {0xff, 0xff, 0x00, 0xff}, {0xff, 0xff, 0x5f, 0x00}, {0xff, 0xff, 0x5f, 0x5f},
        {0xff, 0xff, 0x5f, 0x87}, {0xff, 0xff, 0x5f, 0xaf}, {0xff, 0xff, 0x5f, 0xd7}, {0xff, 0xff, 0x5f, 0xff},
        {0xff, 0xff, 0x87, 0x00}, {0xff, 0xff, 0x87, 0x5f}, {0xff, 0xff, 0x87, 0x87}, {0xff, 0xff, 0x87, 0xaf},
        {0xff, 0xff, 0x87, 0xd7}, {0xff, 0xff, 0x87, 0xff}, {0xff, 0xff, 0xaf, 0x00}, {0xff, 0xff, 0xaf, 0x5f},
        {0xff, 0xff, 0xaf, 0x87}, {0xff, 0xff, 0xaf, 0xaf}, {0xff, 0xff, 0xaf, 0xd7}, {0xff, 0xff, 0xaf, 0xff},
        {0xff, 0xff, 0xd7, 0x00}, {0xff, 0xff, 0xd7, 0x5f}, {0xff, 0xff, 0xd7, 0x87}, {0xff, 0xff, 0xd7, 0xaf},
        {0xff, 0xff, 0xd7, 0xd7}, {0xff, 0xff, 0xd7, 0xff}, {0xff, 0xff, 0xff, 0x00}, {0xff, 0xff, 0xff, 0x5f},
        {0xff, 0xff, 0xff, 0x87}, {0xff, 0xff, 0xff, 0xaf}, {0xff, 0xff, 0xff, 0xd7}, {0xff, 0xff, 0xff, 0xff},
        {0xff, 0x80, 0x80, 0x80}, {0xff, 0x12, 0x12, 0x12}, {0xff, 0x1c, 0x1c, 0x1c}, {0xff, 0x26, 0x26, 0x26},
        {0xff, 0x30, 0x30, 0x30}, {0xff, 0x3a, 0x3a, 0x3a}, {0xff, 0x44, 0x44, 0x44}, {0xff, 0x4e, 0x4e, 0x4e},
        {0xff, 0x58, 0x58, 0x58}, {0xff, 0x62, 0x62, 0x62}, {0xff, 0x6c, 0x6c, 0x6c}, {0xff, 0x76, 0x76, 0x76},
        {0xff, 0x80, 0x80, 0x80}, {0xff, 0x8a, 0x8a, 0x8a}, {0xff, 0x94, 0x94, 0x94}, {0xff, 0x9e, 0x9e, 0x9e},
        {0xff, 0xa8, 0xa8, 0xa8}, {0xff, 0xb2, 0xb2, 0xb2}, {0xff, 0xbc, 0xbc, 0xbc}, {0xff, 0xc6, 0xc6, 0xc6},
        {0xff, 0xd0, 0xd0, 0xd0}, {0xff, 0xda, 0xda, 0xda}, {0xff, 0xe4, 0xe4, 0xe4}, {0xff, 0xee, 0xee, 0xee},
    }};

static unsigned int _PTREE_MOD_RGN_ColorConvert(MI_RGN_PixelFormat_e ePixelFormat, unsigned int c)
{
    unsigned int paletteTableSize = 0;
    switch (ePixelFormat)
    {
        case E_MI_RGN_PIXEL_FORMAT_I8:
            paletteTableSize = 256;
            break;
        case E_MI_RGN_PIXEL_FORMAT_I4:
            paletteTableSize = 16;
            break;
        case E_MI_RGN_PIXEL_FORMAT_I2:
            paletteTableSize = 4;
            break;
        case E_MI_RGN_PIXEL_FORMAT_ARGB1555:
            return (((c & 0xff000000) >> 16) & 0x8000) | (((c & 0x00ff0000) >> 9) & 0x7c00)
                   | (((c & 0x0000ff00) >> 6) & 0x03e0) | (((c & 0x000000ff) >> 3) & 0x001f);
        case E_MI_RGN_PIXEL_FORMAT_ARGB4444:
            return (((c & 0xff000000) >> 16) & 0xf000) | (((c & 0x00ff0000) >> 12) & 0x0f00)
                   | (((c & 0x0000ff00) >> 8) & 0x00f0) | (((c & 0x000000ff) >> 4) & 0x000f);
        case E_MI_RGN_PIXEL_FORMAT_RGB565:
            return (((c & 0x00ff0000) >> 8) & 0xf800) | (((c & 0x0000ff00) >> 5) & 0x07e0)
                   | (((c & 0x000000ff) >> 3) & 0x001f);
        default:
            return c;
    }
    {
        unsigned int  minDiff  = 0xffffffff;
        unsigned int  currDiff = 0;
        unsigned char r        = (c & 0xff0000) >> 16;
        unsigned char g        = (c & 0x00ff00) >> 8;
        unsigned char b        = (c & 0x0000ff);
        unsigned int  index    = 0;
        unsigned int  i;
        for (i = 0; i < paletteTableSize; ++i)
        {
            currDiff = g_paletteTable.astElement[i].u8Red > r ? g_paletteTable.astElement[i].u8Red - r
                                                              : r - g_paletteTable.astElement[i].u8Red;
            currDiff += g_paletteTable.astElement[i].u8Green > g ? g_paletteTable.astElement[i].u8Green - g
                                                                 : g - g_paletteTable.astElement[i].u8Green;
            currDiff += g_paletteTable.astElement[i].u8Blue > b ? g_paletteTable.astElement[i].u8Blue - b
                                                                : b - g_paletteTable.astElement[i].u8Blue;
            if (currDiff < minDiff)
            {
                minDiff = currDiff;
                index   = i;
            }
        }
        return index;
    }
}

static unsigned int _PTREE_MOD_RGN_GetBpp(MI_RGN_PixelFormat_e ePixelFormat)
{
    switch (ePixelFormat)
    {
        case E_MI_RGN_PIXEL_FORMAT_I8:
            return 8;
        case E_MI_RGN_PIXEL_FORMAT_I4:
            return 4;
        case E_MI_RGN_PIXEL_FORMAT_I2:
            return 2;
        case E_MI_RGN_PIXEL_FORMAT_ARGB1555:
        case E_MI_RGN_PIXEL_FORMAT_ARGB4444:
        case E_MI_RGN_PIXEL_FORMAT_RGB565:
            return 16;
        case E_MI_RGN_PIXEL_FORMAT_ARGB8888:
            return 32;
        default:
            return 0;
    }
}
static unsigned int _PTREE_MOD_RGN_CalTinckness(unsigned int width, unsigned int height,
                                                enum PTREE_SUR_RGN_Thickness_e eThickness)
{
    unsigned int mapDiv[]  = {[E_PTREE_SUR_RGN_THICKNESS_THIN]   = 1024,
                             [E_PTREE_SUR_RGN_THICKNESS_NORMAL] = 512,
                             [E_PTREE_SUR_RGN_THICKNESS_THICK]  = 256};
    unsigned int size      = 0;
    unsigned int thickness = 0;

    size      = width > height ? width : height;
    size      = size < 2048 ? 2048 : size;
    thickness = size / mapDiv[eThickness];
    return thickness == 0 ? 2 : thickness;
}
static unsigned int _PTREE_MOD_RGN_CalDotMatrixSize(unsigned int width, unsigned int height, unsigned int mapWidth,
                                                    unsigned int mapHeight, enum PTREE_SUR_RGN_Size_e eSize)
{
    unsigned int mapDiv[] = {
        [E_PTREE_SUR_RGN_SIZE_TINY] = 16, [E_PTREE_SUR_RGN_SIZE_SMALL] = 8, [E_PTREE_SUR_RGN_SIZE_NORMAL] = 4,
        [E_PTREE_SUR_RGN_SIZE_LARGE] = 2, [E_PTREE_SUR_RGN_SIZE_HUGE] = 1,
    };
    unsigned int size = 0;

    size = width > height ? width / mapWidth : height / mapHeight;
    size = size / mapDiv[eSize];
    return size == 0 ? 2 : size;
}

static MI_RGN_HANDLE _PTREE_MOD_RGN_AdapterCreate(unsigned int handleGroupId, MI_RGN_Attr_t *pstRgnAttr)
{
    MI_RGN_HANDLE  handle         = MI_RGN_HANDLE_NULL;
    MI_RGN_HANDLE *handlePool     = g_handlePoolTable[handleGroupId % PTREE_MOD_RGN_HANDLE_GROUP_NUM];
    unsigned int * handlePoolSize = &g_handlePoolSizeTable[handleGroupId % PTREE_MOD_RGN_HANDLE_GROUP_NUM];
    SSOS_THREAD_MutexLock(&gp_handlePoolMutex);
    if (MI_RGN_HANDLE_NULL == handlePool[*handlePoolSize - 1])
    {
        handle = handleGroupId % PTREE_MOD_RGN_HANDLE_GROUP_NUM * PTREE_MOD_RGN_HANDLE_GROUP_MAX_HANDLE_NUM
                 + *handlePoolSize - 1;
    }
    else
    {
        handle = handlePool[*handlePoolSize - 1];
    }
    --(*handlePoolSize);
    SSOS_THREAD_MutexUnlock(&gp_handlePoolMutex);
    if (MI_SUCCESS != MI_RGN_Create(0, handle, pstRgnAttr))
    {
        SSOS_THREAD_MutexLock(&gp_handlePoolMutex);
        handlePool[*handlePoolSize] = handle;
        ++(*handlePoolSize);
        SSOS_THREAD_MutexUnlock(&gp_handlePoolMutex);
        return MI_RGN_HANDLE_NULL;
    }
    return handle;
}
static void _PTREE_MOD_RGN_AdapterDestroy(MI_RGN_HANDLE handle)
{
    unsigned int   handleGroupId  = handle / PTREE_MOD_RGN_HANDLE_GROUP_MAX_HANDLE_NUM;
    MI_RGN_HANDLE *handlePool     = g_handlePoolTable[handleGroupId];
    unsigned int * handlePoolSize = &g_handlePoolSizeTable[handleGroupId];
    MI_RGN_Destroy(0, handle);
    SSOS_THREAD_MutexLock(&gp_handlePoolMutex);
    handlePool[*handlePoolSize] = handle;
    ++(*handlePoolSize);
    SSOS_THREAD_MutexUnlock(&gp_handlePoolMutex);
}
static int _PTREE_MOD_RGN_AdapterGetCanvas(MI_RGN_HANDLE handle, MI_RGN_CanvasInfo_t *pstCanvasInfo)
{
    int ret = MI_SUCCESS;
    SSOS_THREAD_MutexLock(&gp_rgnCanvasMutex);
    ret = MI_RGN_GetCanvasInfo(0, handle, pstCanvasInfo);
    SSOS_THREAD_MutexUnlock(&gp_rgnCanvasMutex);
    return ret;
}
static int _PTREE_MOD_RGN_AdapterUpdateCanvas(MI_RGN_HANDLE handle)
{
    int ret = MI_SUCCESS;
    SSOS_THREAD_MutexLock(&gp_rgnCanvasMutex);
    ret = MI_RGN_UpdateCanvas(0, handle);
    SSOS_THREAD_MutexUnlock(&gp_rgnCanvasMutex);
    return ret;
}

static PTREE_MOD_RGN_Control_t *_PTREE_MOD_RGN_ControlInit(const PTREE_MOD_RGN_ControlOps_t *ops, unsigned int devId,
                                                           PTREE_SUR_RGN_TypeInfo_t *pstTypeInfo)
{
    PTREE_MOD_RGN_Control_t *control = NULL;
    CHECK_POINTER(ops, return NULL);
    CHECK_POINTER(ops->fpConstruct, return NULL);
    CHECK_POINTER(ops->fpDestruct, return NULL);
    control        = ops->fpConstruct(pstTypeInfo);
    control->ops   = ops;
    control->devId = devId;
    SSOS_THREAD_MutexInit(&control->mutex);
    return control;
}

static void _PTREE_MOD_RGN_ControlDeinit(PTREE_MOD_RGN_Control_t *control)
{
    CHECK_POINTER(control, return );
    CHECK_POINTER(control->ops, return );
    SSOS_THREAD_MutexDeinit(&control->mutex);
    control->ops->fpDestruct(control);
}

static int _PTREE_MOD_RGN_ControlAttach(PTREE_MOD_RGN_Control_t *         control,
                                        const PTREE_SUR_RGN_AttachInfo_t *pstAttachInfo)
{
    CHECK_POINTER(control, return SSOS_DEF_FAIL);
    CHECK_POINTER(control->ops, return SSOS_DEF_FAIL);
    SSOS_THREAD_MutexLock(&control->mutex);
    if (control->ops->fpAttach)
    {
        control->ops->fpAttach(control, pstAttachInfo);
    }
    SSOS_THREAD_MutexUnlock(&control->mutex);
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_RGN_ControlDetach(PTREE_MOD_RGN_Control_t *         control,
                                        const PTREE_SUR_RGN_AttachInfo_t *pstAttachInfo)
{
    CHECK_POINTER(control, return SSOS_DEF_FAIL);
    CHECK_POINTER(control->ops, return SSOS_DEF_FAIL);
    SSOS_THREAD_MutexLock(&control->mutex);
    if (control->ops->fpDetach)
    {
        control->ops->fpDetach(control, pstAttachInfo);
    }
    SSOS_THREAD_MutexUnlock(&control->mutex);
    return SSOS_DEF_OK;
}

static void _PTREE_MOD_RGN_ControlProcess(struct PTREE_MOD_RGN_Control_s *control, const PTREE_SUR_RGN_Info_t *pstInfo,
                                          PTREE_PACKET_Obj_t *packet)
{
    int i = 0;
    CHECK_POINTER(control, return );
    CHECK_POINTER(control->ops, return );
    SSOS_THREAD_MutexLock(&control->mutex);
    if (!control->ops->fpCheck || control->ops->fpCheck(control, packet))
    {
        if (control->ops->fpSetParam)
        {
            for (i = 0; i < pstInfo->attachCnt; ++i)
            {
                control->ops->fpSetParam(control, &pstInfo->astAttachInfo[i]);
            }
        }
    }
    if (control->ops->fpFinish)
    {
        control->ops->fpFinish(control);
    }
    SSOS_THREAD_MutexUnlock(&control->mutex);
}
static PTREE_MOD_RGN_Control_t *_PTREE_MOD_RGN_FrameControlConstruct(PTREE_SUR_RGN_TypeInfo_t *pstTypeInfo)
{
    PTREE_MOD_RGN_FrameControl_t *frameControl = SSOS_MEM_Alloc(sizeof(PTREE_MOD_RGN_FrameControl_t));
    if (!frameControl)
    {
        return NULL;
    }
    memset(frameControl, 0, sizeof(PTREE_MOD_RGN_FrameControl_t));
    frameControl->currPacket = NULL;
    frameControl->frameInfo  = pstTypeInfo->stFrameInfo;
    return &frameControl->base;
}
static void _PTREE_MOD_RGN_FrameControlDestruct(struct PTREE_MOD_RGN_Control_s *control)
{
    PTREE_MOD_RGN_FrameControl_t *frameControl = CONTAINER_OF(control, PTREE_MOD_RGN_FrameControl_t, base);
    if (frameControl->currPacket)
    {
        PTREE_PACKET_Del(&frameControl->currPacket->base);
        frameControl->currPacket = NULL;
    }
    SSOS_MEM_Free(frameControl);
}
static void _PTREE_MOD_RGN_FrameControlAttach(struct PTREE_MOD_RGN_Control_s *  control,
                                              const PTREE_SUR_RGN_AttachInfo_t *pstAttachInfo)
{
    PTREE_MOD_RGN_FrameControl_t *frameControl = CONTAINER_OF(control, PTREE_MOD_RGN_FrameControl_t, base);
    MI_RGN_Attr_t                 stRgnAttr;
    MI_RGN_ChnPortParam_t         stChnPortParam;
    MI_RGN_ChnPort_t              stChnPort;
    int                           i = 0;
    memset(&stRgnAttr, 0, sizeof(MI_RGN_Attr_t));
    stRgnAttr.eType = E_MI_RGN_TYPE_FRAME;
    memset(&stChnPort, 0, sizeof(MI_RGN_ChnPort_t));
    stChnPort = pstAttachInfo->stChnPort;
    memset(&stChnPortParam, 0, sizeof(MI_RGN_ChnPortParam_t));
    stChnPortParam.stFrameChnPort.u32Color = frameControl->frameInfo.color;
    stChnPortParam.stFrameChnPort.u8Thickness =
        _PTREE_MOD_RGN_CalTinckness(pstAttachInfo->timingW, pstAttachInfo->timingH, frameControl->frameInfo.thickness);
    stChnPortParam.stFrameChnPort.stRect.s32X      = 0;
    stChnPortParam.stFrameChnPort.stRect.s32Y      = 0;
    stChnPortParam.stFrameChnPort.stRect.u32Width  = 1;
    stChnPortParam.stFrameChnPort.stRect.u32Height = 1;
    for (i = 0; i < PTREE_MOD_RGN_FRAME_NUM_MAX; ++i)
    {
        frameControl->handles[i] = _PTREE_MOD_RGN_AdapterCreate(control->devId, &stRgnAttr);
        SSOS_ASSERT(frameControl->handles[i] != MI_RGN_HANDLE_NULL);
        MI_RGN_AttachToChn(0, frameControl->handles[i], &stChnPort, &stChnPortParam);
    }
}
static void _PTREE_MOD_RGN_FrameControlDetach(struct PTREE_MOD_RGN_Control_s *  control,
                                              const PTREE_SUR_RGN_AttachInfo_t *pstAttachInfo)
{
    PTREE_MOD_RGN_FrameControl_t *frameControl = CONTAINER_OF(control, PTREE_MOD_RGN_FrameControl_t, base);
    MI_RGN_ChnPort_t              stChnPort;
    int                           i = 0;
    memset(&stChnPort, 0, sizeof(MI_RGN_ChnPort_t));
    stChnPort = pstAttachInfo->stChnPort;
    for (i = 0; i < PTREE_MOD_RGN_FRAME_NUM_MAX; ++i)
    {
        MI_RGN_DetachFromChn(0, frameControl->handles[i], &stChnPort);
        _PTREE_MOD_RGN_AdapterDestroy(frameControl->handles[i]);
    }
}
static void _PTREE_MOD_RGN_FrameControlSetParam(struct PTREE_MOD_RGN_Control_s *  control,
                                                const PTREE_SUR_RGN_AttachInfo_t *pstAttachInfo)
{
    PTREE_MOD_RGN_FrameControl_t *frameControl = CONTAINER_OF(control, PTREE_MOD_RGN_FrameControl_t, base);
    unsigned int                  framesCount  = 0;
    MI_RGN_Attr_t                 stRgnAttr;
    MI_RGN_ChnPortParam_t         stChnPortParam;
    MI_RGN_ChnPort_t              stChnPort;
    int                           i = 0;
    memset(&stRgnAttr, 0, sizeof(MI_RGN_Attr_t));
    stRgnAttr.eType = E_MI_RGN_TYPE_FRAME;
    memset(&stChnPort, 0, sizeof(MI_RGN_ChnPort_t));
    stChnPort = pstAttachInfo->stChnPort;
    memset(&stChnPortParam, 0, sizeof(MI_RGN_ChnPortParam_t));

    stChnPortParam.stFrameChnPort.u32Color = frameControl->frameInfo.color;
    stChnPortParam.stFrameChnPort.u8Thickness =
        _PTREE_MOD_RGN_CalTinckness(pstAttachInfo->timingW, pstAttachInfo->timingH, frameControl->frameInfo.thickness);

    framesCount = frameControl->currPacket->info.count > PTREE_MOD_RGN_FRAME_NUM_MAX
                      ? PTREE_MOD_RGN_FRAME_NUM_MAX
                      : frameControl->currPacket->info.count;

    for (i = 0; i < framesCount; ++i)
    {
        stChnPortParam.bShow                           = SSOS_DEF_TRUE;
        stChnPortParam.stFrameChnPort.stRect.s32X      = frameControl->currPacket->rects[i].x;
        stChnPortParam.stFrameChnPort.stRect.s32Y      = frameControl->currPacket->rects[i].y;
        stChnPortParam.stFrameChnPort.stRect.u32Width  = frameControl->currPacket->rects[i].w;
        stChnPortParam.stFrameChnPort.stRect.u32Height = frameControl->currPacket->rects[i].h;
        if (MI_SUCCESS != MI_RGN_SetDisplayAttr(0, frameControl->handles[i], &stChnPort, &stChnPortParam))
        {
            PTREE_ERR("MI_RGN_SetDisplayAttr Failed");
        }
    }
    stChnPortParam.bShow                           = SSOS_DEF_FALSE;
    stChnPortParam.stFrameChnPort.stRect.s32X      = 0;
    stChnPortParam.stFrameChnPort.stRect.s32Y      = 0;
    stChnPortParam.stFrameChnPort.stRect.u32Width  = 1;
    stChnPortParam.stFrameChnPort.stRect.u32Height = 1;
    for (; i < PTREE_MOD_RGN_FRAME_NUM_MAX; ++i)
    {
        if (MI_SUCCESS != MI_RGN_SetDisplayAttr(0, frameControl->handles[i], &stChnPort, &stChnPortParam))
        {
            PTREE_ERR("MI_RGN_SetDisplayAttr Failed");
        }
    }
}
static int _PTREE_MOD_RGN_FrameControlCheck(struct PTREE_MOD_RGN_Control_s *control, PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_RGN_FrameControl_t *frameControl = CONTAINER_OF(control, PTREE_MOD_RGN_FrameControl_t, base);
    if (!PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(rects)))
    {
        return SSOS_DEF_FALSE;
    }
    frameControl->currPacket = CONTAINER_OF(PTREE_PACKET_Dup(packet), PTREE_RGN_PACKET_Rects_t, base);
    return SSOS_DEF_TRUE;
}
static void _PTREE_MOD_RGN_FrameControlFinish(struct PTREE_MOD_RGN_Control_s *control)
{
    PTREE_MOD_RGN_FrameControl_t *frameControl = CONTAINER_OF(control, PTREE_MOD_RGN_FrameControl_t, base);
    if (frameControl->currPacket)
    {
        PTREE_PACKET_Del(&frameControl->currPacket->base);
        frameControl->currPacket = NULL;
    }
}

static PTREE_MOD_RGN_Control_t *_PTREE_MOD_RGN_OsdFrameControlConstruct(PTREE_SUR_RGN_TypeInfo_t *pstTypeInfo)
{
    PTREE_MOD_RGN_OsdFrameControl_t *osdFrameControl = SSOS_MEM_Alloc(sizeof(PTREE_MOD_RGN_OsdFrameControl_t));
    if (!osdFrameControl)
    {
        return NULL;
    }
    memset(osdFrameControl, 0, sizeof(PTREE_MOD_RGN_OsdFrameControl_t));
    osdFrameControl->osdFrameInfo = pstTypeInfo->stOsdFrameInfo;
    osdFrameControl->currPacket   = NULL;
    osdFrameControl->lastPacket   = NULL;
    osdFrameControl->bpp          = _PTREE_MOD_RGN_GetBpp(osdFrameControl->osdFrameInfo.ePixelFormat);
    SSOS_LIST_InitHead(&osdFrameControl->canvasLstHead);
    return &osdFrameControl->base;
}
static void _PTREE_MOD_RGN_OsdFrameControlDestruct(struct PTREE_MOD_RGN_Control_s *control)
{
    PTREE_MOD_RGN_OsdFrameControl_t *osdFrameControl = CONTAINER_OF(control, PTREE_MOD_RGN_OsdFrameControl_t, base);
    PTREE_MOD_RGN_OsdCanvasData_t *  pos = NULL, *posN = NULL;
    SSOS_LIST_FOR_EACH_ENTRY_SAFE(pos, posN, &osdFrameControl->canvasLstHead, canvasLst)
    {
        SSOS_LIST_Del(&pos->canvasLst);
        SSOS_MEM_Free(pos);
    }
    if (osdFrameControl->currPacket)
    {
        PTREE_PACKET_Del(&osdFrameControl->currPacket->base);
        osdFrameControl->currPacket = NULL;
    }
    if (osdFrameControl->lastPacket)
    {
        PTREE_PACKET_Del(&osdFrameControl->lastPacket->base);
        osdFrameControl->lastPacket = NULL;
    }
    SSOS_MEM_Free(osdFrameControl);
}

static void _PTREE_MOD_RGN_OsdFrameControlAttach(struct PTREE_MOD_RGN_Control_s *  control,
                                                 const PTREE_SUR_RGN_AttachInfo_t *pstAttachInfo)
{
    PTREE_MOD_RGN_OsdFrameControl_t *osdFrameControl = CONTAINER_OF(control, PTREE_MOD_RGN_OsdFrameControl_t, base);
    PTREE_MOD_RGN_OsdCanvasData_t *  pos             = NULL;
    MI_RGN_Attr_t                    stRgnAttr;
    MI_RGN_ChnPortParam_t            stChnPortParam;
    MI_RGN_ChnPort_t                 stChnPort;
    SSOS_LIST_FOR_EACH_ENTRY(pos, &osdFrameControl->canvasLstHead, canvasLst)
    {
        if (pstAttachInfo->timingW == pos->width && pstAttachInfo->timingH == pos->height)
        {
            ++pos->usingCnt;
            return;
        }
    }

    pos = SSOS_MEM_Alloc(sizeof(PTREE_MOD_RGN_OsdCanvasData_t));
    if (!pos)
    {
        PTREE_ERR("Alloc err");
        return;
    }

    memset(pos, 0, sizeof(PTREE_MOD_RGN_OsdCanvasData_t));

    memset(&stRgnAttr, 0, sizeof(MI_RGN_Attr_t));
    stRgnAttr.eType                           = E_MI_RGN_TYPE_OSD;
    stRgnAttr.stOsdInitParam.ePixelFmt        = osdFrameControl->osdFrameInfo.ePixelFormat;
    stRgnAttr.stOsdInitParam.stSize.u32Width  = pstAttachInfo->timingW;
    stRgnAttr.stOsdInitParam.stSize.u32Height = pstAttachInfo->timingH;

    memset(&stChnPort, 0, sizeof(MI_RGN_ChnPort_t));
    stChnPort = pstAttachInfo->stChnPort;

    memset(&stChnPortParam, 0, sizeof(MI_RGN_ChnPortParam_t));
    stChnPortParam.bShow                                                   = SSOS_DEF_TRUE;
    stChnPortParam.u32Layer                                                = 0;
    stChnPortParam.stOsdChnPort.stPoint.u32X                               = 0;
    stChnPortParam.stOsdChnPort.stPoint.u32Y                               = 0;
    stChnPortParam.stOsdChnPort.u8PaletteIdx                               = 0;
    stChnPortParam.stOsdChnPort.stOsdAlphaAttr.eAlphaMode                  = E_MI_RGN_CONSTANT_ALPHA;
    stChnPortParam.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.u8ConstantAlpha = 0x40;

    pos->handle = _PTREE_MOD_RGN_AdapterCreate(control->devId, &stRgnAttr);
    if (MI_RGN_HANDLE_NULL == pos->handle)
    {
        SSOS_MEM_Free(pos);
        PTREE_ERR("handle err");
        return;
    }
    if (MI_SUCCESS != MI_RGN_AttachToChn(0, pos->handle, &stChnPort, &stChnPortParam))
    {
        _PTREE_MOD_RGN_AdapterDestroy(pos->handle);
        SSOS_MEM_Free(pos);
        PTREE_ERR("Attach err");
        return;
    }

    SSOS_LIST_InitHead(&pos->canvasLst);
    pos->width    = pstAttachInfo->timingW;
    pos->height   = pstAttachInfo->timingH;
    pos->usingCnt = 1;

    SSOS_LIST_AddTail(&pos->canvasLst, &osdFrameControl->canvasLstHead);
}
static void _PTREE_MOD_RGN_OsdFrameControlDetach(struct PTREE_MOD_RGN_Control_s *  control,
                                                 const PTREE_SUR_RGN_AttachInfo_t *pstAttachInfo)
{
    PTREE_MOD_RGN_OsdFrameControl_t *osdFrameControl = CONTAINER_OF(control, PTREE_MOD_RGN_OsdFrameControl_t, base);
    PTREE_MOD_RGN_OsdCanvasData_t *  pos = NULL, *posN = NULL;
    MI_RGN_ChnPort_t                 stChnPort;
    memset(&stChnPort, 0, sizeof(MI_RGN_ChnPort_t));
    stChnPort = pstAttachInfo->stChnPort;
    SSOS_LIST_FOR_EACH_ENTRY_SAFE(pos, posN, &osdFrameControl->canvasLstHead, canvasLst)
    {
        if (pstAttachInfo->timingW != pos->width || pstAttachInfo->timingH != pos->height)
        {
            continue;
        }
        --pos->usingCnt;
        if (pos->usingCnt)
        {
            break;
        }
        SSOS_LIST_Del(&pos->canvasLst);
        MI_RGN_DetachFromChn(0, pos->handle, &stChnPort);
        _PTREE_MOD_RGN_AdapterDestroy(pos->handle);
        SSOS_MEM_Free(pos);
        break;
    }
}
static void _PTREE_MOD_RGN_OsdFrameControlSetParam(struct PTREE_MOD_RGN_Control_s *  control,
                                                   const PTREE_SUR_RGN_AttachInfo_t *pstAttachInfo)
{
    PTREE_MOD_RGN_OsdFrameControl_t *osdFrameControl = CONTAINER_OF(control, PTREE_MOD_RGN_OsdFrameControl_t, base);
    PTREE_MOD_RGN_OsdCanvasData_t *  pos             = NULL;
    MI_RGN_CanvasInfo_t              stCanvasInfo;
    int                              i = 0;
    PTREE_GL_Color_t                 color;
    (void)pstAttachInfo;
    color =
        _PTREE_MOD_RGN_ColorConvert(osdFrameControl->osdFrameInfo.ePixelFormat, osdFrameControl->osdFrameInfo.color);
    memset(&stCanvasInfo, 0, sizeof(MI_RGN_CanvasInfo_t));
    SSOS_LIST_FOR_EACH_ENTRY(pos, &osdFrameControl->canvasLstHead, canvasLst)
    {
        PTREE_GL_Obj_t    gl;
        PTREE_GL_Canvas_t glCanvas;
        PTREE_GL_Coord_t  thickness;
        PTREE_GL_Coord_t  x, y, w, h;
        memset(&gl, 0, sizeof(PTREE_GL_Obj_t));
        memset(&glCanvas, 0, sizeof(PTREE_GL_Canvas_t));
        if (MI_SUCCESS != _PTREE_MOD_RGN_AdapterGetCanvas(pos->handle, &stCanvasInfo))
        {
            continue;
        }
        glCanvas.data   = (char *)stCanvasInfo.virtAddr;
        glCanvas.width  = stCanvasInfo.stSize.u32Width;
        glCanvas.height = stCanvasInfo.stSize.u32Height;
        glCanvas.stride = stCanvasInfo.u32Stride;
        glCanvas.bpp    = osdFrameControl->bpp;
        if (SSOS_DEF_OK != PTREE_GL_Init(&gl, &glCanvas))
        {
            continue;
        }
        thickness = _PTREE_MOD_RGN_CalTinckness(stCanvasInfo.stSize.u32Width, stCanvasInfo.stSize.u32Height,
                                                osdFrameControl->osdFrameInfo.thickness);
        if (osdFrameControl->lastPacket)
        {
            for (i = 0; i < osdFrameControl->lastPacket->info.count; ++i)
            {
                PTREE_MOD_RGN_CAL_ABS_COORD_XY(x, osdFrameControl->lastPacket->rects[i].x, stCanvasInfo.stSize.u32Width,
                                               PTREE_RGN_PACKET_COORDINATE_MAX_W);
                PTREE_MOD_RGN_CAL_ABS_COORD_XY(y, osdFrameControl->lastPacket->rects[i].y,
                                               stCanvasInfo.stSize.u32Height, PTREE_RGN_PACKET_COORDINATE_MAX_H);
                PTREE_MOD_RGN_CAL_ABS_COORD_WH(w, osdFrameControl->lastPacket->rects[i].w, x,
                                               osdFrameControl->lastPacket->rects[i].x, stCanvasInfo.stSize.u32Width,
                                               PTREE_RGN_PACKET_COORDINATE_MAX_W);
                PTREE_MOD_RGN_CAL_ABS_COORD_WH(h, osdFrameControl->lastPacket->rects[i].h, y,
                                               osdFrameControl->lastPacket->rects[i].y, stCanvasInfo.stSize.u32Height,
                                               PTREE_RGN_PACKET_COORDINATE_MAX_H);
                PTREE_GL_DrawFrame(&gl, 0, x, y, w, h, thickness);
            }
        }
        if (osdFrameControl->currPacket)
        {
            for (i = 0; i < osdFrameControl->currPacket->info.count; ++i)
            {
                PTREE_MOD_RGN_CAL_ABS_COORD_XY(x, osdFrameControl->currPacket->rects[i].x, stCanvasInfo.stSize.u32Width,
                                               PTREE_RGN_PACKET_COORDINATE_MAX_W);
                PTREE_MOD_RGN_CAL_ABS_COORD_XY(y, osdFrameControl->currPacket->rects[i].y,
                                               stCanvasInfo.stSize.u32Height, PTREE_RGN_PACKET_COORDINATE_MAX_H);
                PTREE_MOD_RGN_CAL_ABS_COORD_WH(w, osdFrameControl->currPacket->rects[i].w, x,
                                               osdFrameControl->currPacket->rects[i].x, stCanvasInfo.stSize.u32Width,
                                               PTREE_RGN_PACKET_COORDINATE_MAX_W);
                PTREE_MOD_RGN_CAL_ABS_COORD_WH(h, osdFrameControl->currPacket->rects[i].h, y,
                                               osdFrameControl->currPacket->rects[i].y, stCanvasInfo.stSize.u32Height,
                                               PTREE_RGN_PACKET_COORDINATE_MAX_H);
                PTREE_GL_DrawFrame(&gl, color, x, y, w, h, thickness);
            }
        }
        _PTREE_MOD_RGN_AdapterUpdateCanvas(pos->handle);
    }
}

static int _PTREE_MOD_RGN_OsdFrameControlCheck(struct PTREE_MOD_RGN_Control_s *control, PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_RGN_OsdFrameControl_t *osdFrameControl = CONTAINER_OF(control, PTREE_MOD_RGN_OsdFrameControl_t, base);
    PTREE_RGN_PACKET_Rects_t *       rects           = NULL;
    if (!PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(rects)))
    {
        return SSOS_DEF_FALSE;
    }
    if (!osdFrameControl->currPacket)
    {
        osdFrameControl->currPacket = CONTAINER_OF(PTREE_PACKET_Dup(packet), PTREE_RGN_PACKET_Rects_t, base);
        return SSOS_DEF_TRUE;
    }
    rects = CONTAINER_OF(packet, PTREE_RGN_PACKET_Rects_t, base);
    if (rects->info.count == osdFrameControl->currPacket->info.count)
    {
        if (0
            == memcmp(rects->rects, osdFrameControl->currPacket->rects,
                      sizeof(PTREE_RGN_PACKET_Rect_t) * rects->info.count))
        {
            return SSOS_DEF_TRUE;
        }
    }

    PTREE_PACKET_Del(&osdFrameControl->currPacket->base);
    osdFrameControl->currPacket = CONTAINER_OF(PTREE_PACKET_Dup(packet), PTREE_RGN_PACKET_Rects_t, base);

    return SSOS_DEF_TRUE;
}
static void _PTREE_MOD_RGN_OsdFrameControlFinish(struct PTREE_MOD_RGN_Control_s *control)
{
    PTREE_MOD_RGN_OsdFrameControl_t *osdFrameControl = CONTAINER_OF(control, PTREE_MOD_RGN_OsdFrameControl_t, base);
    if (osdFrameControl->lastPacket)
    {
        PTREE_PACKET_Del(&osdFrameControl->lastPacket->base);
    }
    osdFrameControl->lastPacket =
        CONTAINER_OF(PTREE_PACKET_Dup(&osdFrameControl->currPacket->base), PTREE_RGN_PACKET_Rects_t, base);
}

static PTREE_MOD_RGN_Control_t *_PTREE_MOD_RGN_OsdDotMatrixControlConstruct(PTREE_SUR_RGN_TypeInfo_t *pstTypeInfo)
{
    PTREE_MOD_RGN_OsdDotMatrixControl_t *osdDotMatrixControl =
        SSOS_MEM_Alloc(sizeof(PTREE_MOD_RGN_OsdDotMatrixControl_t));
    if (!osdDotMatrixControl)
    {
        return NULL;
    }
    memset(osdDotMatrixControl, 0, sizeof(PTREE_MOD_RGN_OsdDotMatrixControl_t));
    osdDotMatrixControl->osdDotMatrixInfo = pstTypeInfo->stOsdDotMatrixInfo;
    osdDotMatrixControl->currPacket       = NULL;
    osdDotMatrixControl->lastPacket       = NULL;
    osdDotMatrixControl->bpp              = _PTREE_MOD_RGN_GetBpp(osdDotMatrixControl->osdDotMatrixInfo.ePixelFormat);
    SSOS_LIST_InitHead(&osdDotMatrixControl->canvasLstHead);
    return &osdDotMatrixControl->base;
}
static void _PTREE_MOD_RGN_OsdDotMatrixControlDestruct(struct PTREE_MOD_RGN_Control_s *control)
{
    PTREE_MOD_RGN_OsdDotMatrixControl_t *osdDotMatrixControl =
        CONTAINER_OF(control, PTREE_MOD_RGN_OsdDotMatrixControl_t, base);
    PTREE_MOD_RGN_OsdCanvasData_t *pos = NULL, *posN = NULL;
    SSOS_LIST_FOR_EACH_ENTRY_SAFE(pos, posN, &osdDotMatrixControl->canvasLstHead, canvasLst)
    {
        SSOS_LIST_Del(&pos->canvasLst);
        SSOS_MEM_Free(pos);
    }
    if (osdDotMatrixControl->currPacket)
    {
        PTREE_PACKET_Del(&osdDotMatrixControl->currPacket->base);
        osdDotMatrixControl->currPacket = NULL;
    }
    if (osdDotMatrixControl->lastPacket)
    {
        PTREE_PACKET_Del(&osdDotMatrixControl->lastPacket->base);
        osdDotMatrixControl->lastPacket = NULL;
    }
    SSOS_MEM_Free(osdDotMatrixControl);
}

static void _PTREE_MOD_RGN_OsdDotMatrixControlAttach(struct PTREE_MOD_RGN_Control_s *  control,
                                                     const PTREE_SUR_RGN_AttachInfo_t *pstAttachInfo)
{
    PTREE_MOD_RGN_OsdDotMatrixControl_t *osdDotMatrixControl =
        CONTAINER_OF(control, PTREE_MOD_RGN_OsdDotMatrixControl_t, base);
    PTREE_MOD_RGN_OsdCanvasData_t *pos = NULL;
    MI_RGN_Attr_t                  stRgnAttr;
    MI_RGN_ChnPortParam_t          stChnPortParam;
    MI_RGN_ChnPort_t               stChnPort;
    SSOS_LIST_FOR_EACH_ENTRY(pos, &osdDotMatrixControl->canvasLstHead, canvasLst)
    {
        if (pstAttachInfo->timingW == pos->width && pstAttachInfo->timingH == pos->height)
        {
            ++pos->usingCnt;
            return;
        }
    }

    pos = SSOS_MEM_Alloc(sizeof(PTREE_MOD_RGN_OsdCanvasData_t));
    if (!pos)
    {
        PTREE_ERR("Alloc err");
        return;
    }

    memset(pos, 0, sizeof(PTREE_MOD_RGN_OsdCanvasData_t));

    memset(&stRgnAttr, 0, sizeof(MI_RGN_Attr_t));
    stRgnAttr.eType                           = E_MI_RGN_TYPE_OSD;
    stRgnAttr.stOsdInitParam.ePixelFmt        = osdDotMatrixControl->osdDotMatrixInfo.ePixelFormat;
    stRgnAttr.stOsdInitParam.stSize.u32Width  = pstAttachInfo->timingW;
    stRgnAttr.stOsdInitParam.stSize.u32Height = pstAttachInfo->timingH;

    memset(&stChnPort, 0, sizeof(MI_RGN_ChnPort_t));
    stChnPort = pstAttachInfo->stChnPort;

    memset(&stChnPortParam, 0, sizeof(MI_RGN_ChnPortParam_t));
    stChnPortParam.bShow                                                   = SSOS_DEF_TRUE;
    stChnPortParam.u32Layer                                                = 0;
    stChnPortParam.stOsdChnPort.stPoint.u32X                               = 0;
    stChnPortParam.stOsdChnPort.stPoint.u32Y                               = 0;
    stChnPortParam.stOsdChnPort.u8PaletteIdx                               = 0;
    stChnPortParam.stOsdChnPort.stOsdAlphaAttr.eAlphaMode                  = E_MI_RGN_CONSTANT_ALPHA;
    stChnPortParam.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.u8ConstantAlpha = 0x40;

    pos->handle = _PTREE_MOD_RGN_AdapterCreate(control->devId, &stRgnAttr);
    if (MI_RGN_HANDLE_NULL == pos->handle)
    {
        SSOS_MEM_Free(pos);
        PTREE_ERR("handle err");
        return;
    }
    if (MI_SUCCESS != MI_RGN_AttachToChn(0, pos->handle, &stChnPort, &stChnPortParam))
    {
        _PTREE_MOD_RGN_AdapterDestroy(pos->handle);
        SSOS_MEM_Free(pos);
        PTREE_ERR("Attach err");
        return;
    }

    SSOS_LIST_InitHead(&pos->canvasLst);
    pos->width    = pstAttachInfo->timingW;
    pos->height   = pstAttachInfo->timingH;
    pos->usingCnt = 1;

    SSOS_LIST_AddTail(&pos->canvasLst, &osdDotMatrixControl->canvasLstHead);
}
static void _PTREE_MOD_RGN_OsdDotMatrixControlDetach(struct PTREE_MOD_RGN_Control_s *  control,
                                                     const PTREE_SUR_RGN_AttachInfo_t *pstAttachInfo)
{
    PTREE_MOD_RGN_OsdDotMatrixControl_t *osdDotMatrixControl =
        CONTAINER_OF(control, PTREE_MOD_RGN_OsdDotMatrixControl_t, base);
    PTREE_MOD_RGN_OsdCanvasData_t *pos = NULL, *posN = NULL;
    MI_RGN_ChnPort_t               stChnPort;
    memset(&stChnPort, 0, sizeof(MI_RGN_ChnPort_t));
    stChnPort = pstAttachInfo->stChnPort;
    SSOS_LIST_FOR_EACH_ENTRY_SAFE(pos, posN, &osdDotMatrixControl->canvasLstHead, canvasLst)
    {
        if (pstAttachInfo->timingW != pos->width || pstAttachInfo->timingH != pos->height)
        {
            continue;
        }
        --pos->usingCnt;
        if (pos->usingCnt)
        {
            break;
        }
        SSOS_LIST_Del(&pos->canvasLst);
        MI_RGN_DetachFromChn(0, pos->handle, &stChnPort);
        _PTREE_MOD_RGN_AdapterDestroy(pos->handle);
        SSOS_MEM_Free(pos);
        break;
    }
}
static void _PTREE_MOD_RGN_OsdDotMatrixControlSetParam(struct PTREE_MOD_RGN_Control_s *  control,
                                                       const PTREE_SUR_RGN_AttachInfo_t *pstAttachInfo)
{
    PTREE_MOD_RGN_OsdDotMatrixControl_t *osdDotMatrixControl =
        CONTAINER_OF(control, PTREE_MOD_RGN_OsdDotMatrixControl_t, base);
    PTREE_MOD_RGN_OsdCanvasData_t *pos = NULL;
    MI_RGN_CanvasInfo_t            stCanvasInfo;
    PTREE_GL_Color_t               color;
    (void)pstAttachInfo;
    color = _PTREE_MOD_RGN_ColorConvert(osdDotMatrixControl->osdDotMatrixInfo.ePixelFormat,
                                        osdDotMatrixControl->osdDotMatrixInfo.color);
    memset(&stCanvasInfo, 0, sizeof(MI_RGN_CanvasInfo_t));
    SSOS_LIST_FOR_EACH_ENTRY(pos, &osdDotMatrixControl->canvasLstHead, canvasLst)
    {
        PTREE_GL_Obj_t    gl;
        PTREE_GL_Canvas_t glCanvas;
        PTREE_GL_Coord_t  size;
        memset(&gl, 0, sizeof(PTREE_GL_Obj_t));
        memset(&glCanvas, 0, sizeof(PTREE_GL_Canvas_t));
        if (MI_SUCCESS != _PTREE_MOD_RGN_AdapterGetCanvas(pos->handle, &stCanvasInfo))
        {
            continue;
        }
        glCanvas.data   = (char *)stCanvasInfo.virtAddr;
        glCanvas.width  = stCanvasInfo.stSize.u32Width;
        glCanvas.height = stCanvasInfo.stSize.u32Height;
        glCanvas.stride = stCanvasInfo.u32Stride;
        glCanvas.bpp    = osdDotMatrixControl->bpp;
        if (SSOS_DEF_OK != PTREE_GL_Init(&gl, &glCanvas))
        {
            continue;
        }
        if (osdDotMatrixControl->lastPacket)
        {
            size = _PTREE_MOD_RGN_CalDotMatrixSize(
                glCanvas.width, glCanvas.height, osdDotMatrixControl->lastPacket->info.width,
                osdDotMatrixControl->lastPacket->info.height, osdDotMatrixControl->osdDotMatrixInfo.size);
            PTREE_GL_DrawDotMatrix(&gl, 0, osdDotMatrixControl->lastPacket->data,
                                   osdDotMatrixControl->lastPacket->info.width,
                                   osdDotMatrixControl->lastPacket->info.height, size);
        }
        if (osdDotMatrixControl->currPacket)
        {
            size = _PTREE_MOD_RGN_CalDotMatrixSize(
                glCanvas.width, glCanvas.height, osdDotMatrixControl->currPacket->info.width,
                osdDotMatrixControl->currPacket->info.height, osdDotMatrixControl->osdDotMatrixInfo.size);
            PTREE_GL_DrawDotMatrix(&gl, color, osdDotMatrixControl->currPacket->data,
                                   osdDotMatrixControl->currPacket->info.width,
                                   osdDotMatrixControl->currPacket->info.height, size);
        }
        _PTREE_MOD_RGN_AdapterUpdateCanvas(pos->handle);
    }
}

static int _PTREE_MOD_RGN_OsdDotMatrixControlCheck(struct PTREE_MOD_RGN_Control_s *control, PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_RGN_OsdDotMatrixControl_t *osdDotMatrixControl =
        CONTAINER_OF(control, PTREE_MOD_RGN_OsdDotMatrixControl_t, base);
    PTREE_RGN_PACKET_Map_t *map = NULL;
    if (!PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(map)))
    {
        return SSOS_DEF_FALSE;
    }
    if (!osdDotMatrixControl->currPacket)
    {
        osdDotMatrixControl->currPacket = CONTAINER_OF(PTREE_PACKET_Dup(packet), PTREE_RGN_PACKET_Map_t, base);
        return SSOS_DEF_TRUE;
    }
    if (PTREE_PACKET_InfoEqual(packet->info, &osdDotMatrixControl->currPacket->info.base))
    {
        map = CONTAINER_OF(packet, PTREE_RGN_PACKET_Map_t, base);
        if (0
            == memcmp(map->data, osdDotMatrixControl->currPacket->data,
                      sizeof(PTREE_RGN_PACKET_Rect_t) * map->info.width * map->info.height))
        {
            return SSOS_DEF_TRUE;
        }
    }

    PTREE_PACKET_Del(&osdDotMatrixControl->currPacket->base);
    osdDotMatrixControl->currPacket = CONTAINER_OF(PTREE_PACKET_Dup(packet), PTREE_RGN_PACKET_Map_t, base);

    return SSOS_DEF_TRUE;
}
static void _PTREE_MOD_RGN_OsdDotMatrixControlFinish(struct PTREE_MOD_RGN_Control_s *control)
{
    PTREE_MOD_RGN_OsdDotMatrixControl_t *osdDotMatrixControl =
        CONTAINER_OF(control, PTREE_MOD_RGN_OsdDotMatrixControl_t, base);
    if (osdDotMatrixControl->lastPacket)
    {
        PTREE_PACKET_Del(&osdDotMatrixControl->lastPacket->base);
    }
    osdDotMatrixControl->lastPacket =
        CONTAINER_OF(PTREE_PACKET_Dup(&osdDotMatrixControl->currPacket->base), PTREE_RGN_PACKET_Map_t, base);
}

static PTREE_PACKET_Obj_t *_PTREE_MOD_RGN_PackerMake(PTREE_PACKER_Obj_t *packer, const PTREE_PACKET_Info_t *info)
{
    PTREE_RGN_PACKET_RectsInfo_t *rectsInfo = NULL;
    (void)packer;
    if (!PTREE_PACKET_InfoLikely(info, PTREE_PACKET_INFO_TYPE(rects)))
    {
        PTREE_ERR("packet info type %s is not support", info->type);
        return NULL;
    }
    rectsInfo = CONTAINER_OF(info, PTREE_RGN_PACKET_RectsInfo_t, base);
    return PTREE_RGN_PACKET_RectsNew(rectsInfo->count);
}
static int _PTREE_MOD_RGN_InLinkerEnqueue(PTREE_LINKER_Obj_t *linker, PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_RGN_InObj_t *rgnInMod = CONTAINER_OF(linker, PTREE_MOD_RGN_InObj_t, linker);
    PTREE_SUR_RGN_Info_t * rgnInfo  = CONTAINER_OF(rgnInMod->base.thisMod->info, PTREE_SUR_RGN_Info_t, base.base);
    if (rgnInMod->pControl)
    {
        _PTREE_MOD_RGN_ControlProcess(rgnInMod->pControl, rgnInfo, packet);
    }
    return SSOS_DEF_OK;
}
static PTREE_PACKET_Obj_t *_PTREE_MOD_RGN_InLinkerDequeue(PTREE_LINKER_Obj_t *linker, int ms)
{
    (void)linker;
    (void)ms;
    return NULL;
}

static int _PTREE_MOD_RGN_InStart(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_RGN_InObj_t * rgnModIn  = CONTAINER_OF(modIn, PTREE_MOD_RGN_InObj_t, base);
    PTREE_SUR_RGN_InInfo_t *rgnInInfo = CONTAINER_OF(modIn->info, PTREE_SUR_RGN_InInfo_t, base.base);
    PTREE_SUR_RGN_Info_t *  rgnInfo   = CONTAINER_OF(modIn->thisMod->info, PTREE_SUR_RGN_Info_t, base.base);
    int                     i         = 0;
    if (rgnInInfo->mode == E_PTREE_SUR_RGN_IN_MODE_FRAME)
    {
        rgnModIn->pControl =
            _PTREE_MOD_RGN_ControlInit(&G_FRAME_CONTROL_OPS, rgnInfo->base.base.devId, &rgnInInfo->info);
    }
    else if (rgnInInfo->mode == E_PTREE_SUR_RGN_IN_MODE_OSD_FRAME)
    {
        rgnModIn->pControl =
            _PTREE_MOD_RGN_ControlInit(&G_OSD_FRAME_CONTROL_OPS, rgnInfo->base.base.devId, &rgnInInfo->info);
    }
    else if (rgnInInfo->mode == E_PTREE_SUR_RGN_IN_MODE_OSD_DOT_MATRIX)
    {
        rgnModIn->pControl =
            _PTREE_MOD_RGN_ControlInit(&G_OSD_DOT_MATRIX_CONTROL_OPS, rgnInfo->base.base.devId, &rgnInInfo->info);
    }
    if (!rgnModIn->pControl)
    {
        return SSOS_DEF_FAIL;
    }
    for (i = 0; i < rgnInfo->attachCnt; ++i)
    {
        _PTREE_MOD_RGN_ControlAttach(rgnModIn->pControl, &rgnInfo->astAttachInfo[i]);
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_RGN_InStop(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_RGN_InObj_t *rgnModIn = CONTAINER_OF(modIn, PTREE_MOD_RGN_InObj_t, base);
    PTREE_SUR_RGN_Info_t * rgnInfo  = CONTAINER_OF(modIn->thisMod->info, PTREE_SUR_RGN_Info_t, base.base);
    int                    i        = 0;
    if (!rgnModIn->pControl)
    {
        return SSOS_DEF_FAIL;
    }
    for (i = 0; i < rgnInfo->attachCnt; ++i)
    {
        _PTREE_MOD_RGN_ControlDetach(rgnModIn->pControl, &rgnInfo->astAttachInfo[i]);
    }
    _PTREE_MOD_RGN_ControlDeinit(rgnModIn->pControl);
    rgnModIn->pControl = NULL;
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_RGN_InGetType(PTREE_MOD_InObj_t *modIn)
{
    (void)modIn;
    return E_PTREE_MOD_BIND_TYPE_USER;
}
static PTREE_LINKER_Obj_t *_PTREE_MOD_RGN_InCreateNLinker(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_RGN_InObj_t *rgnModIn = CONTAINER_OF(modIn, PTREE_MOD_RGN_InObj_t, base);
    return PTREE_LINKER_Dup(&rgnModIn->linker);
}
static PTREE_PACKER_Obj_t *_PTREE_MOD_RGN_InCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast)
{
    PTREE_MOD_RGN_InObj_t *rgnModIn = CONTAINER_OF(modIn, PTREE_MOD_RGN_InObj_t, base);
    *isFast                         = SSOS_DEF_FALSE;
    return PTREE_PACKER_Dup(&rgnModIn->packer);
}
static void _PTREE_MOD_RGN_InDestruct(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_RGN_InObj_t *rgnModIn = CONTAINER_OF(modIn, PTREE_MOD_RGN_InObj_t, base);

    rgnModIn->pControl = NULL;
    PTREE_PACKER_Del(&rgnModIn->packer);
    PTREE_LINKER_Del(&rgnModIn->linker);
}
static void _PTREE_MOD_RGN_InFree(PTREE_MOD_InObj_t *modIn)
{
    SSOS_MEM_Free(CONTAINER_OF(modIn, PTREE_MOD_RGN_InObj_t, base));
}
static PTREE_MOD_InObj_t *_PTREE_MOD_RGN_InNew(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_RGN_InObj_t *rgnModIn = NULL;

    rgnModIn = SSOS_MEM_Alloc(sizeof(PTREE_MOD_RGN_InObj_t));
    if (!rgnModIn)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(rgnModIn, 0, sizeof(PTREE_MOD_RGN_InObj_t));

    if (SSOS_DEF_OK != PTREE_MOD_InObjInit(&rgnModIn->base, &G_PTREE_MOD_RGN_IN_OPS, mod, loopId))
    {
        goto ERR_MOD_IN_INIT;
    }

    if (SSOS_DEF_OK != PTREE_LINKER_Init(&rgnModIn->linker, &G_PTREE_MOD_RGN_IN_LINKER_OPS))
    {
        goto ERR_LINKER_INIT;
    }

    if (SSOS_DEF_OK != PTREE_PACKER_Init(&rgnModIn->packer, &G_PTREE_MOD_RGN_PACKER_OPS))
    {
        goto ERR_PACKER_INIT;
    }

    rgnModIn->pControl = NULL;

    PTREE_PACKER_Register(&rgnModIn->packer, &G_PTREE_MOD_RGN_PACKER_HOOK);
    PTREE_LINKER_Register(&rgnModIn->linker, &G_PTREE_MOD_RGN_IN_LINKER_HOOK);
    PTREE_MOD_InObjRegister(&rgnModIn->base, &G_PTREE_MOD_RGN_IN_HOOK);
    return &rgnModIn->base;

ERR_PACKER_INIT:
    PTREE_LINKER_Del(&rgnModIn->linker);
ERR_LINKER_INIT:
    PTREE_MOD_InObjDel(&rgnModIn->base);
ERR_MOD_IN_INIT:
    SSOS_MEM_Free(rgnModIn);
ERR_MEM_ALLOC:
    return NULL;
}

static int _PTREE_MOD_RGN_Init(PTREE_MOD_SYS_Obj_t *sysMod)
{
    (void)sysMod;
    MI_RGN_Init(0, &g_paletteTable);
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_RGN_Deinit(PTREE_MOD_SYS_Obj_t *sysMod)
{
    (void)sysMod;
    MI_RGN_DeInit(0);
    return SSOS_DEF_OK;
}
static PTREE_MOD_InObj_t *_PTREE_MOD_RGN_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    return _PTREE_MOD_RGN_InNew(mod, loopId);
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_RGN_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    (void)mod;
    (void)loopId;
    return NULL;
}
static void _PTREE_MOD_RGN_Destruct(PTREE_MOD_SYS_Obj_t *sysMod)
{
    (void)sysMod;
    if (1 == g_handlePoolInitCnt)
    {
        int groupIdx;
        SSOS_THREAD_MutexDeinit(&gp_rgnCanvasMutex);
        SSOS_THREAD_MutexDeinit(&gp_handlePoolMutex);
        for (groupIdx = 0; groupIdx < PTREE_MOD_RGN_HANDLE_GROUP_NUM; ++groupIdx)
        {
            int handleIdx;
            g_handlePoolSizeTable[groupIdx] = PTREE_MOD_RGN_HANDLE_GROUP_MAX_HANDLE_NUM;
            for (handleIdx = 0; handleIdx < PTREE_MOD_RGN_HANDLE_GROUP_MAX_HANDLE_NUM; ++handleIdx)
            {
                g_handlePoolTable[groupIdx][handleIdx] = MI_RGN_HANDLE_NULL;
            }
        }
    }
    --g_handlePoolInitCnt;
}
static void _PTREE_MOD_RGN_Free(PTREE_MOD_SYS_Obj_t *sysMod)
{
    SSOS_MEM_Free(CONTAINER_OF(sysMod, PTREE_MOD_RGN_Obj_t, base));
}

PTREE_MOD_Obj_t *PTREE_MOD_RGN_New(PARENA_Tag_t *tag)
{
    PTREE_MOD_RGN_Obj_t *rgnMod = NULL;

    rgnMod = SSOS_MEM_Alloc(sizeof(PTREE_MOD_RGN_Obj_t));
    if (!rgnMod)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(rgnMod, 0, sizeof(PTREE_MOD_RGN_Obj_t));

    if (SSOS_DEF_OK != PTREE_MOD_SYS_ObjInit(&rgnMod->base, &G_PTREE_MOD_RGN_SYS_OPS, tag, E_MI_MODULE_ID_RGN))
    {
        goto ERR_MOD_SYS_INIT;
    }

    if (!g_handlePoolInitCnt)
    {
        int groupIdx;
        for (groupIdx = 0; groupIdx < PTREE_MOD_RGN_HANDLE_GROUP_NUM; ++groupIdx)
        {
            int handleIdx;
            g_handlePoolSizeTable[groupIdx] = PTREE_MOD_RGN_HANDLE_GROUP_MAX_HANDLE_NUM;
            for (handleIdx = 0; handleIdx < PTREE_MOD_RGN_HANDLE_GROUP_MAX_HANDLE_NUM; ++handleIdx)
            {
                g_handlePoolTable[groupIdx][handleIdx] = MI_RGN_HANDLE_NULL;
            }
        }
        SSOS_THREAD_MutexInit(&gp_handlePoolMutex);
        SSOS_THREAD_MutexInit(&gp_rgnCanvasMutex);
    }
    ++g_handlePoolInitCnt;

    PTREE_MOD_SYS_ObjRegister(&rgnMod->base, &G_PTREE_MOD_RGN_SYS_HOOK);
    return &rgnMod->base.base;

ERR_MOD_SYS_INIT:
    SSOS_MEM_Free(rgnMod);
ERR_MEM_ALLOC:
    return NULL;
}

PTREE_MAKER_MOD_INIT(RGN, PTREE_MOD_RGN_New);
