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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>
#include "ssos_mem.h"
#include "parena.h"
#include "ptree_mod.h"
#include "ptree_maker.h"
#include "ptree_mod.h"
#include "ptree_db.h"
#ifdef USING_JSON
#include "ptree_db_json.h"
#endif
#ifdef USING_INI
#include "ptree_db_ini.h"
#endif
#include "arena.h"
#include "ptree.h"
#include "ptree_sur_snr.h"
#include "ptree_sur_vif.h"
#include "ptree_sur_isp.h"
#include "ptree_sur_scl.h"
#include "ptree_sur_scl_stretch.h"
#include "ptree_sur_venc.h"
#include "ptree_sur_pool.h"
#include "ptree_sur_ldc.h"
#include "ptree_sur_vdf.h"
#include "ptree_sur_file.h"
#include "ptree_sur_iq.h"
#include "ptree_sur_tick.h"
#include "ptree_sur_ai.h"
#include "ptree_sur_ao.h"
#include "ptree_sur_rgn.h"
#include "ptree_sur_det.h"
#include "ptree_sur_sync.h"
#include "ptree_sur_hseg.h"
#include "ptree_sur_stdio.h"
#include "ptree_sur_disp.h"
#include "ptree_sur_pass.h"
#include "ptree_sur_aestable.h"
#include "ptree_sur_uvc.h"

#ifndef UNUSED
#define UNUSED(x) (void)x
#endif

static void _SSAPP_MIXER_GENBIN_PtreeModDummyInFree(PTREE_MOD_InObj_t *modIn);

static void _SSAPP_MIXER_GENBIN_PtreeModDummyOutFree(PTREE_MOD_OutObj_t *modOut);

static PTREE_MOD_InObj_t * _SSAPP_MIXER_GENBIN_PtreeModDummyCreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static PTREE_MOD_OutObj_t *_SSAPP_MIXER_GENBIN_PtreeModDummyCreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static void                _SSAPP_MIXER_GENBIN_PtreeModDummyFree(PTREE_MOD_Obj_t *mod);

static const PTREE_MOD_Ops_t G_PTREE_MOD_DUMMY_OPS = {
    .createModIn  = _SSAPP_MIXER_GENBIN_PtreeModDummyCreateModIn,
    .createModOut = _SSAPP_MIXER_GENBIN_PtreeModDummyCreateModOut,
};
static const PTREE_MOD_Hook_t G_PTREE_MOD_DUMMY_HOOK = {
    .free = _SSAPP_MIXER_GENBIN_PtreeModDummyFree,
};

static const PTREE_MOD_InOps_t  G_PTREE_MOD_DUMMY_IN_OPS  = {};
static const PTREE_MOD_InHook_t G_PTREE_MOD_DUMMY_IN_HOOK = {
    .free = _SSAPP_MIXER_GENBIN_PtreeModDummyInFree,
};

static const PTREE_MOD_OutOps_t  G_PTREE_MOD_DUMMY_OUT_OPS  = {};
static const PTREE_MOD_OutHook_t G_PTREE_MOD_DUMMY_OUT_HOOK = {
    .free = _SSAPP_MIXER_GENBIN_PtreeModDummyOutFree,
};

static void _SSAPP_MIXER_GENBIN_PtreeModDummyInFree(PTREE_MOD_InObj_t *modIn)
{
    SSOS_MEM_Free(modIn);
}

static void _SSAPP_MIXER_GENBIN_PtreeModDummyOutFree(PTREE_MOD_OutObj_t *modOut)
{
    SSOS_MEM_Free(modOut);
}

static PTREE_MOD_InObj_t *_SSAPP_MIXER_GENBIN_PtreeModDummyCreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_InObj_t *modIn = SSOS_MEM_Alloc(sizeof(PTREE_MOD_InObj_t));
    if (!modIn)
    {
        return NULL;
    }
    memset(modIn, 0, sizeof(PTREE_MOD_InObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_InObjInit(modIn, &G_PTREE_MOD_DUMMY_IN_OPS, mod, loopId))
    {
        SSOS_MEM_Free(modIn);
        return NULL;
    }
    PTREE_MOD_InObjRegister(modIn, &G_PTREE_MOD_DUMMY_IN_HOOK);
    return modIn;
}
static PTREE_MOD_OutObj_t *_SSAPP_MIXER_GENBIN_PtreeModDummyCreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_OutObj_t *modOut = SSOS_MEM_Alloc(sizeof(PTREE_MOD_OutObj_t));
    if (!modOut)
    {
        return NULL;
    }
    memset(modOut, 0, sizeof(PTREE_MOD_OutObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_OutObjInit(modOut, &G_PTREE_MOD_DUMMY_OUT_OPS, mod, loopId))
    {
        SSOS_MEM_Free(modOut);
        return NULL;
    }
    PTREE_MOD_OutObjRegister(modOut, &G_PTREE_MOD_DUMMY_OUT_HOOK);
    return modOut;
}
static void _SSAPP_MIXER_GENBIN_PtreeModDummyFree(PTREE_MOD_Obj_t *mod)
{
    SSOS_MEM_Free(mod);
}

static PTREE_MOD_Obj_t *_SSAPP_MIXER_GENBIN_PtreeModDummyNew(PARENA_Tag_t *tag)
{
    PTREE_MOD_Obj_t *mod = NULL;

    mod = SSOS_MEM_Alloc(sizeof(PTREE_MOD_Obj_t));
    if (!mod)
    {
        return NULL;
    }
    memset(mod, 0, sizeof(PTREE_MOD_Obj_t));
    if (SSOS_DEF_OK != PTREE_MOD_ObjInit(mod, &G_PTREE_MOD_DUMMY_OPS, tag))
    {
        SSOS_MEM_Free(mod);
        return NULL;
    }
    PTREE_MOD_ObjRegister(mod, &G_PTREE_MOD_DUMMY_HOOK);
    return mod;
}

PTREE_MAKER_MOD_INIT(SNR, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);
PTREE_MAKER_MOD_INIT(VIF, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);
PTREE_MAKER_MOD_INIT(ISP, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);
PTREE_MAKER_MOD_INIT(SCL, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);
PTREE_MAKER_MOD_INIT(SCL_STRETCH, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);
PTREE_MAKER_MOD_INIT(VENC, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);
PTREE_MAKER_MOD_INIT(POOL, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);
PTREE_MAKER_MOD_INIT(LDC, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);
PTREE_MAKER_MOD_INIT(VDF, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);
PTREE_MAKER_MOD_INIT(IQ, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);
PTREE_MAKER_MOD_INIT(FILE, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);
PTREE_MAKER_MOD_INIT(EMPTY, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);
PTREE_MAKER_MOD_INIT(CMD_TEST, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);
PTREE_MAKER_MOD_INIT(STR_CMD_TEST, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);
PTREE_MAKER_MOD_INIT(TICK, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);
PTREE_MAKER_MOD_INIT(AI, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);
PTREE_MAKER_MOD_INIT(AO, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);
PTREE_MAKER_MOD_INIT(RGN, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);
PTREE_MAKER_MOD_INIT(DET, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);
PTREE_MAKER_MOD_INIT(SYNC, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);
PTREE_MAKER_MOD_INIT(HSEG, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);
PTREE_MAKER_MOD_INIT(STDIO, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);
PTREE_MAKER_MOD_INIT(DISP, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);
PTREE_MAKER_MOD_INIT(PASS, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);
PTREE_MAKER_MOD_INIT(AESTABLE, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);
PTREE_MAKER_MOD_INIT(UVC, _SSAPP_MIXER_GENBIN_PtreeModDummyNew);

static void _SSAPP_MIXER_GENBIN_PrintHelp(const char *bin)
{
    printf("Build info : Commit %s, Build by %s, Date %s.\n", GIT_COMMIT, BUILD_OWNER, BUILD_DATE);
    printf("This tool converts ini/json file of 'PTREE' into binary.\n");
    printf("Parameters:\n");
    printf("-i   Input ini or json file.\n");
    printf("-o   Output head file.\n");
    printf("-n   Output value name in head file.\n");
    printf("Example:\n");
    printf("%s -i ./pipeline.ini -o pipeline.bin -n pipeline\n", bin);
}
static int _SSAPP_MIXER_GENBIN_FileNameCheck(const char *pFile)
{
    int s32SurfixOff = 0;

    s32SurfixOff = strlen(pFile);
    if (s32SurfixOff > 6 && !strcmp(pFile + s32SurfixOff - 5, ".json"))
    {
        printf("[PTREE_DB_JSON]:File name (%s) check pass.\n", pFile);
        return 0;
    }
    if (s32SurfixOff > 5 && !strcmp(pFile + s32SurfixOff - 4, ".ini"))
    {
        printf("[PTREE_DB_INI]:File name (%s) check pass.\n", pFile);
        return 1;
    }
    printf("[PTREE_DB_JSON]: File name (%s) check error.\n", pFile);
    return -1;
}

static void _SSAPP_MIXER_GENBIN_PreloadModule(void) // NOLINT
{
    PTREE_MAKER_MOD_SETUP(SNR);
    PTREE_MAKER_MOD_SETUP(VIF);
    PTREE_MAKER_MOD_SETUP(ISP);
    PTREE_MAKER_MOD_SETUP(SCL);
    PTREE_MAKER_MOD_SETUP(SCL_STRETCH);
    PTREE_MAKER_MOD_SETUP(VENC);
    PTREE_MAKER_MOD_SETUP(POOL);
    PTREE_MAKER_MOD_SETUP(LDC);
    PTREE_MAKER_MOD_SETUP(VDF);
    PTREE_MAKER_MOD_SETUP(IQ);
    PTREE_MAKER_MOD_SETUP(FILE);
    PTREE_MAKER_MOD_SETUP(EMPTY);
    PTREE_MAKER_MOD_SETUP(CMD_TEST);
    PTREE_MAKER_MOD_SETUP(STR_CMD_TEST);
    PTREE_MAKER_MOD_SETUP(TICK);
    PTREE_MAKER_MOD_SETUP(AI);
    PTREE_MAKER_MOD_SETUP(AO);
    PTREE_MAKER_MOD_SETUP(RGN);
    PTREE_MAKER_MOD_SETUP(DET);
    PTREE_MAKER_MOD_SETUP(SYNC);
    PTREE_MAKER_MOD_SETUP(HSEG);
    PTREE_MAKER_MOD_SETUP(STDIO);
    PTREE_MAKER_MOD_SETUP(DISP);
    PTREE_MAKER_MOD_SETUP(PASS);
    PTREE_MAKER_MOD_SETUP(AESTABLE);
    PTREE_MAKER_MOD_SETUP(UVC);
}

static void _SSAPP_MIXER_GENBIN_OutputFile(void *pArenaHandle, const char *pFilePath, const char *valName)
{
    FILE *         pFile    = NULL;
    unsigned char *pBase    = NULL;
    unsigned int   u32Size  = 0;
    unsigned int   u32Count = 0;

    pBase   = ARENA_Addr(pArenaHandle, 0);
    u32Size = ARENA_Size(pArenaHandle);
    if (!u32Size || !pBase)
    {
        printf("Get arena base addr error.\n");
        return;
    }
    pFile = fopen(pFilePath, "w+");
    if (!pFile)
    {
        printf("File: %s open error.\n", pFilePath);
        return;
    }
    fprintf(pFile, "/* SigmaStar trade secret */\n");
    fprintf(pFile, "/* Copyright (c) [2019~2020] SigmaStar Technology.\n");
    fprintf(pFile, "All rights reserved.\n");
    fprintf(pFile, "\n");
    fprintf(pFile, "Unless otherwise stipulated in writing, any and all information contained\n");
    fprintf(pFile, "herein regardless in any format shall remain the sole proprietary of\n");
    fprintf(pFile, "SigmaStar and be kept in strict confidence\n");
    fprintf(pFile, "(SigmaStar Confidential Information) by the recipient.\n");
    fprintf(pFile, "Any unauthorized act including without limitation unauthorized disclosure,\n");
    fprintf(pFile, "copying, use, reproduction, sale, distribution, modification, disassembling,\n");
    fprintf(pFile, "reverse engineering and compiling of the contents of SigmaStar Confidential\n");
    fprintf(pFile, "Information is unlawful and strictly prohibited. SigmaStar hereby reserves the\n");
    fprintf(pFile, "rights to any and all damages, losses, costs and expenses resulting therefrom.\n");
    fprintf(pFile, "*/\n");
    fprintf(pFile, "#ifndef __%s__\n", valName);
    fprintf(pFile, "#define __%s__\n", valName);
    fprintf(pFile, "static char auto_gen_%s[] = { // NOLINT\n", valName);
    fprintf(pFile, "    ");
    while (u32Count < u32Size)
    {
        fprintf(pFile, "0x%02x", pBase[u32Count]);
        u32Count++;
        if (u32Count == u32Size)
        {
            break;
        }
        if (!(u32Count % 19))
        {
            fprintf(pFile, ",\n    ");
        }
        else
        {
            fprintf(pFile, ", ");
        }
    }
    fprintf(pFile, "};\n");
    fprintf(pFile, "#endif");
    fclose(pFile);
    printf("Write %s done!!!!!!\n", pFilePath);
}

static int _SSAPP_MIXER_GENBIN_PtreeTakeOff(const char *pFile, PTREE_Config_t *pstConfig)
{
    int ret = 0;
    ret     = _SSAPP_MIXER_GENBIN_FileNameCheck(pFile);
    if (ret == 0)
    {
#ifdef USING_JSON
        pstConfig->pDbInstance = PTREE_DB_JSON_Init(pFile);
        if (!pstConfig->pDbInstance)
        {
            return -1;
        }
#endif
    }
    else if (ret == 1)
    {
#ifdef USING_INI
        pstConfig->pDbInstance = PTREE_DB_INI_Init(pFile);
        if (!pstConfig->pDbInstance)
        {
            return -1;
        }
#endif
    }
    else
    {
        return -1;
    }
    if (ARENA_Create(&pstConfig->pArenaHandle, 0x10000) == -1)
    {
        printf("Arena create fail!\n");
        return -1;
    }
    return ret;
}

static int _SSAPP_MIXER_GENBIN_PtreeDropDown(int s32DbChoice, PTREE_Config_t *pstConfig)
{
    ARENA_Destroy(pstConfig->pArenaHandle);
    if (s32DbChoice == 0)
    {
#ifdef USING_JSON
        PTREE_DB_JSON_Deinit(pstConfig->pDbInstance);
#endif
    }
    else if (s32DbChoice == 1)
    {
#ifdef USING_INI
        PTREE_DB_INI_Deinit(pstConfig->pDbInstance);
#endif
    }
    return 0;
}

int main(int argc, char **argv)
{
    int            s32Res  = 0;
    int            strSize = 0;
    void *         pIns    = NULL;
    char *         pInStr  = NULL;
    char *         pOutStr = NULL;
    char *         pValStr = NULL;
    PTREE_Config_t stConfig;

    memset(&stConfig, 0, sizeof(PTREE_Config_t));
    while ((s32Res = getopt(argc, argv, "i:o:n:")) != -1)
    {
        switch (s32Res)
        {
            case 'i':
            {
                assert(!pInStr);
                strSize = strlen(optarg) + 1;
                pInStr  = malloc(strSize);
                assert(pInStr);
                snprintf(pInStr, strSize, "%s", optarg);
            }
            break;
            case 'o':
            {
                assert(!pOutStr);
                strSize = strlen(optarg) + 1;
                pOutStr = malloc(strSize);
                assert(pOutStr);
                snprintf(pOutStr, strSize, "%s", optarg);
            }
            break;
            case 'n':
            {
                assert(!pValStr);
                strSize = strlen(optarg) + 1;
                pValStr = malloc(strSize);
                assert(pValStr);
                snprintf(pValStr, strSize, "%s", optarg);
            }
            break;
            default:
            {
                if (pInStr)
                {
                    free(pInStr);
                }
                if (pOutStr)
                {
                    free(pOutStr);
                }
                _SSAPP_MIXER_GENBIN_PrintHelp(argv[0]);
                return 0;
            }
            break;
        }
    }
    if (!pOutStr || !pInStr || !pValStr)
    {
        _SSAPP_MIXER_GENBIN_PrintHelp(argv[0]);
        goto EXIT0;
    }
    _SSAPP_MIXER_GENBIN_PreloadModule();
    s32Res = _SSAPP_MIXER_GENBIN_PtreeTakeOff(pInStr, &stConfig);
    if (s32Res == -1)
    {
        goto EXIT1;
    }
    pIns = PTREE_CreateInstance(&stConfig);
    if (!pIns)
    {
        s32Res = -1;
        printf("Create instance fail!\n");
        goto EXIT2;
    }
    s32Res = PTREE_ConstructPipeline(pIns);
    if (s32Res != 0)
    {
        printf("Construct instance fail!\n");
        goto EXIT3;
    }
    _SSAPP_MIXER_GENBIN_OutputFile(stConfig.pArenaHandle, pOutStr, pValStr);
    PTREE_DestructPipeline(pIns);
    s32Res = 0;
EXIT3:
    PTREE_DestroyInstance(pIns);
EXIT2:
    _SSAPP_MIXER_GENBIN_PtreeDropDown(s32Res, &stConfig);
EXIT1:
    PTREE_MAKER_Clear();
EXIT0:
    if (pInStr)
    {
        free(pInStr);
    }
    if (pOutStr)
    {
        free(pOutStr);
    }
    if (pValStr)
    {
        free(pValStr);
    }
    return s32Res;
}
