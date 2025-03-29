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
#include "mi_ao_datatype.h"
#include "ssos_def.h"
#include "ptree_enum.h"
#include "ssos_list.h"
#include "ptree_log.h"
#include "ssos_mem.h"

#include "ptree_sur.h"
#include "ptree_sur_sys.h"
#include "ptree_sur_ao.h"
#include "ptree_sur_aio.h"
#include "ptree_maker.h"
#include "mi_common_datatype.h"

#define CHECK_POINTER(_ptr, _err)            \
    (                                        \
        {                                    \
            if (!(_ptr))                     \
            {                                \
                PTREE_ERR("%s NULL", #_ptr); \
                _err;                        \
            }                                \
        })

static PARENA_Tag_t *_PTREE_SUR_AO_OccupyInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena);
static int  _PTREE_SUR_AO_LoadDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_Info_t *sysInfo, PTREE_DB_Obj_t db);
static void _PTREE_SUR_AO_Free(PTREE_SUR_SYS_Obj_t *sysSur);

static const PTREE_SUR_SYS_Ops_t G_PTREE_SUR_AO_OPS = {
    .occupyInfo = _PTREE_SUR_AO_OccupyInfo,
    .loadDb     = _PTREE_SUR_AO_LoadDb,
};
static const PTREE_SUR_SYS_Hook_t G_PTREE_SUR_AO_HOOK = {
    .free = _PTREE_SUR_AO_Free,
};

static int _PTREE_SUR_AO_LoadDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_Info_t *sysInfo, PTREE_DB_Obj_t db)
{
    PTREE_SUR_AO_Info_t *aoInfo  = CONTAINER_OF(sysInfo, PTREE_SUR_AO_Info_t, base);
    const char *         str     = NULL;
    MI_AO_If_e           tmpAoIf = 0;
    (void)sysSur;
    aoInfo->u32PeriodSize   = (unsigned int)PTREE_DB_GetInt(db, "PERIOD_SIZE");
    aoInfo->s16Volume       = (signed short)PTREE_DB_GetInt(db, "VOLUME");
    aoInfo->u32SampleRate   = (unsigned int)PTREE_DB_GetInt(db, "SAMPLE_RATE");
    aoInfo->u32I2sSyncclock = (unsigned int)PTREE_DB_GetInt(db, "I2S_SYNCCLOCK");
    aoInfo->u32I2sTdmslots  = (unsigned int)PTREE_DB_GetInt(db, "I2S_TMDSLOTS");
    aoInfo->u32SyncMode     = (unsigned int)PTREE_DB_GetInt(db, "SYNC");

    aoInfo->enAoIf = 0;
    PTREE_DB_ProcessKey(db, "INTERFACE_PARAM");
    for (tmpAoIf = E_MI_AO_IF_DAC_AB; tmpAoIf < E_MI_AO_IF_MAX;)
    {
        if ((unsigned int)PTREE_DB_GetInt(db, PTREE_ENUM_TO_STR(MI_AO_If_e, tmpAoIf)))
        {
            aoInfo->enAoIf |= tmpAoIf;
        }
        tmpAoIf = tmpAoIf << 1;
    }
    PTREE_DB_ProcessBack(db);
    str = PTREE_DB_GetStr(db, "CHN_MOD");
    if (str && strlen(str) > 1)
    {
        aoInfo->eChannelMode = PTREE_ENUM_FROM_STR(MI_AO_ChannelMode_e, str);
        str                  = NULL;
    }
    str = PTREE_DB_GetStr(db, "FORMAT");
    if (str && strlen(str) > 1)
    {
        aoInfo->eAoFormat = PTREE_ENUM_FROM_STR(MI_AUDIO_Format_e, str);
        str               = NULL;
    }
    str = PTREE_DB_GetStr(db, "SND_MOD");
    if (str && strlen(str) > 1)
    {
        aoInfo->eAoSoundMode = PTREE_ENUM_FROM_STR(MI_AUDIO_SoundMode_e, str);
        str                  = NULL;
    }
    str = PTREE_DB_GetStr(db, "I2S_MOD");
    if (str && strlen(str) > 1)
    {
        aoInfo->eAoI2sMode = PTREE_ENUM_FROM_STR(MI_AUDIO_I2sMode_e, str);
        str                = NULL;
    }
    str = PTREE_DB_GetStr(db, "I2S_FORMAT");
    if (str && strlen(str) > 1)
    {
        aoInfo->eAoI2sFormat = PTREE_ENUM_FROM_STR(MI_AUDIO_I2sFormat_e, str);
        str                  = NULL;
    }
    str = PTREE_DB_GetStr(db, "I2S_MCLK");
    if (str && strlen(str) > 1)
    {
        aoInfo->eAoI2sMclkE = PTREE_ENUM_FROM_STR(MI_AUDIO_I2sMclk_e, str);
        str                 = NULL;
    }
    str = PTREE_DB_GetStr(db, "I2S_BIT_WIDTH");
    if (str && strlen(str) > 1)
    {
        aoInfo->eAoI2sBitWidth = PTREE_ENUM_FROM_STR(MI_AUDIO_I2sBitWidth_e, str);
        str                    = NULL;
    }

    return SSOS_DEF_OK;
}
static PARENA_Tag_t *_PTREE_SUR_AO_OccupyInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_AO_Info_t, base.base, PTREE_SUR_Info_t);
}

static void _PTREE_SUR_AO_Free(PTREE_SUR_SYS_Obj_t *sysSur)
{
    SSOS_MEM_Free(sysSur);
}

PTREE_SUR_Obj_t *PTREE_SUR_AO_New(void)
{
    PTREE_SUR_SYS_Obj_t *sysSur = SSOS_MEM_Alloc(sizeof(PTREE_SUR_SYS_Obj_t));
    if (!sysSur)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sysSur, 0, sizeof(PTREE_SUR_SYS_Obj_t));
    if (SSOS_DEF_OK != PTREE_SUR_SYS_Init(sysSur, &G_PTREE_SUR_AO_OPS))
    {
        SSOS_MEM_Free(sysSur);
        return NULL;
    }
    PTREE_SUR_SYS_Register(sysSur, &G_PTREE_SUR_AO_HOOK);
    return &sysSur->base;
}

PTREE_MAKER_SUR_INIT(AO, PTREE_SUR_AO_New);
