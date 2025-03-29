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

#ifdef __KERNEL__

#else /* __KERNEL__ */

#include "ptree_log.h"
#include "ptree_enum.h"
#include "ssos_io.h"
#include "mi_vdf_datatype.h"
#include "ptree_sur_vdf.h"
#include "ssos_list.h"
#include "ssos_mem.h"
#include "ptree_maker.h"

#define CHECK_POINTER(_ptr, _err)            \
    (                                        \
        {                                    \
            if (!(_ptr))                     \
            {                                \
                PTREE_ERR("%s NULL", #_ptr); \
                _err;                        \
            }                                \
        })

PTREE_ENUM_DEFINE(MI_VDF_WorkMode_e, {E_MI_VDF_WORK_MODE_MD, "md"}, {E_MI_VDF_WORK_MODE_OD, "od"}, );

PTREE_ENUM_DEFINE(MDMB_MODE_e, {MDMB_MODE_MB_4x4, "4x4"}, {MDMB_MODE_MB_8x8, "8x8"}, {MDMB_MODE_MB_16x16, "16x16"}, );

PTREE_ENUM_DEFINE(MDALG_MODE_e, {MDALG_MODE_FG, "fg"}, {MDALG_MODE_SAD, "sad"}, {MDALG_MODE_FRAMEDIFF, "framediff"}, );

PTREE_ENUM_DEFINE(ODWindow_e, {OD_WINDOW_1X1, "1x1"}, {OD_WINDOW_2X2, "2x2"}, {OD_WINDOW_3X3, "3x3"}, );

PTREE_ENUM_DEFINE(MDSAD_OUT_CTRL_e, {MDSAD_OUT_CTRL_16BIT_SAD, "16bit"}, {MDSAD_OUT_CTRL_8BIT_SAD, "8bit"},
                  {MDSAD_OUT_CTRL_BUTT, "bult"}, );

PTREE_ENUM_DEFINE(PTREE_SUR_VDF_OdMotionSensitivelyE_t, {E_PTREE_SUR_VDF_OD_MOTIONSENSITIVELY_MIN, "min"},
                  {E_PTREE_SUR_VDF_OD_MOTIONSENSITIVELY_LOW, "low"},
                  {E_PTREE_SUR_VDF_OD_MOTIONSENSITIVELY_MIDDLE, "middle"},
                  {E_PTREE_SUR_VDF_OD_MOTIONSENSITIVELY_HIGH, "high"},
                  {E_PTREE_SUR_VDF_OD_MOTIONSENSITIVELY_MAX, "max"}, );

static PARENA_Tag_t *_PTREE_SUR_VDF_OccupyInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena);
static int  _PTREE_SUR_VDF_LoadDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_Info_t *sysInfo, PTREE_DB_Obj_t db);
static void _PTREE_SUR_VDF_Free(PTREE_SUR_SYS_Obj_t *sysSur);

static const PTREE_SUR_SYS_Ops_t G_PTREE_SUR_VDF_OPS = {
    .occupyInfo = _PTREE_SUR_VDF_OccupyInfo,
    .loadDb     = _PTREE_SUR_VDF_LoadDb,
};
static const PTREE_SUR_SYS_Hook_t G_PTREE_SUR_VDF_HOOK = {
    .free = _PTREE_SUR_VDF_Free,
};

static PARENA_Tag_t *_PTREE_SUR_VDF_OccupyInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_VDF_Info_t, base.base, PTREE_SUR_Info_t);
}
static int _PTREE_SUR_VDF_LoadDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_Info_t *sysInfo, PTREE_DB_Obj_t db)
{
    PTREE_SUR_VDF_Info_t *vdfInfo = CONTAINER_OF(sysInfo, PTREE_SUR_VDF_Info_t, base);

    unsigned int u32Num = 0;
    const char * str    = NULL;
    char         strPointName[16];

    (void)sysSur;

    str = PTREE_DB_GetStr(db, "MOD_TYPE");
    if (NULL == str || strlen(str) < 1)
    {
        PTREE_ERR("get vdf type failed please check db!");
        return -1;
    }
    vdfInfo->enVdfMode = PTREE_ENUM_FROM_STR(MI_VDF_WorkMode_e, str);
    PTREE_DB_ProcessKey(db, "MODE_TYPE_PARAM");
    str = NULL;
    if (vdfInfo->enVdfMode == E_MI_VDF_WORK_MODE_MD)
    {
        str = PTREE_DB_GetStr(db, "ALG_MODE");
        if (str && strlen(str) > 1)
        {
            vdfInfo->enAlgMode = PTREE_ENUM_FROM_STR(MDALG_MODE_e, str);
            str                = NULL;
        }

        str = PTREE_DB_GetStr(db, "MB_MODE");
        if (str && strlen(str) > 1)
        {
            vdfInfo->enMdMbMode = PTREE_ENUM_FROM_STR(MDMB_MODE_e, str);
            str                 = NULL;
        }

        str = PTREE_DB_GetStr(db, "SAD_MODE");
        if (str && strlen(str) > 1)
        {
            vdfInfo->enMdSadOutMode = PTREE_ENUM_FROM_STR(MDSAD_OUT_CTRL_e, str);
            str                     = NULL;
        }

        vdfInfo->stMdAttr.u32Sensitivity = PTREE_DB_GetInt(db, "ALG_SENS");
        vdfInfo->stMdAttr.u32ObjNumMax   = PTREE_DB_GetInt(db, "OBJ_MAXNUM");
        if (0 == PTREE_DB_ProcessKey(db, "VDF_ALGO_MODE"))
        {
            vdfInfo->stMdAttr.u32Thr       = PTREE_DB_GetInt(db, "MD_THR");
            vdfInfo->stMdAttr.u32LearnRate = PTREE_DB_GetInt(db, "LEARN_RATE");
            PTREE_DB_ProcessBack(db);
        }
        for (u32Num = 0; u32Num < 4; u32Num++)
        {
            snprintf(strPointName, 16, "POINT_%u", u32Num);
            if (0 == PTREE_DB_ProcessKey(db, strPointName))
            {
                vdfInfo->stMdAttr.stPnt[u32Num].u32x = PTREE_DB_GetInt(db, "PX");
                vdfInfo->stMdAttr.stPnt[u32Num].u32y = PTREE_DB_GetInt(db, "PY");
                PTREE_DB_ProcessBack(db);
            }
        }
        vdfInfo->stMdAttr.u32PointNum = 4;
    }
    else if (vdfInfo->enVdfMode == E_MI_VDF_WORK_MODE_OD)
    {
        str = PTREE_DB_GetStr(db, "MOT_SENS");
        if (str && strlen(str) > 1)
        {
            vdfInfo->enMotionSensitivity = PTREE_ENUM_FROM_STR(PTREE_SUR_VDF_OdMotionSensitivelyE_t, str);
            str                          = NULL;
        }

        str = PTREE_DB_GetStr(db, "ALG_SENS");
        snprintf(vdfInfo->strSensitivity, 16, "%s", str);

        str = PTREE_DB_GetStr(db, "OD_WINDOW");
        if (str && strlen(str) > 1)
        {
            vdfInfo->enOdWindows = PTREE_ENUM_FROM_STR(ODWindow_e, str);
            str                  = NULL;
        }

        for (u32Num = 0; u32Num < 4; u32Num++)
        {
            snprintf(strPointName, 16, "POINT_%u", u32Num);
            if (0 == PTREE_DB_ProcessKey(db, strPointName))
            {
                vdfInfo->stOdAttr.stPnt[u32Num].u32x = PTREE_DB_GetInt(db, "PX");
                vdfInfo->stOdAttr.stPnt[u32Num].u32y = PTREE_DB_GetInt(db, "PY");
                PTREE_DB_ProcessBack(db);
            }
        }
        vdfInfo->stOdAttr.u32PointNum = 4;
    }
    else
    {
        PTREE_ERR("Mode error!");
        PTREE_DB_ProcessBack(db);
        return -1;
    }
    PTREE_DB_ProcessBack(db);
    return SSOS_DEF_OK;
}

static void _PTREE_SUR_VDF_Free(PTREE_SUR_SYS_Obj_t *sysSur)
{
    SSOS_MEM_Free(sysSur);
}

PTREE_SUR_Obj_t *PTREE_SUR_VDF_New(void)
{
    PTREE_SUR_SYS_Obj_t *sysSur = SSOS_MEM_Alloc(sizeof(PTREE_SUR_SYS_Obj_t));
    if (!sysSur)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sysSur, 0, sizeof(PTREE_SUR_SYS_Obj_t));
    if (SSOS_DEF_OK != PTREE_SUR_SYS_Init(sysSur, &G_PTREE_SUR_VDF_OPS))
    {
        SSOS_MEM_Free(sysSur);
        return NULL;
    }
    PTREE_SUR_SYS_Register(sysSur, &G_PTREE_SUR_VDF_HOOK);
    return &sysSur->base;
}

PTREE_MAKER_SUR_INIT(VDF, PTREE_SUR_VDF_New);
#endif /* __KERNEL__ */
