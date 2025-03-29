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

#include <time.h>
#if defined(__KERNEL__) || defined(CAM_OS_RTK)
#include "ms_platform.h"
#endif
#include "cam_os_wrapper.h"
#include "cam_fs_wrapper.h"
#include "sys_sys_time.h"
#include "sys_sys_isw_cli.h"
#include "sys_memmap.h"
#include "initcall.h"
#include "drv_dualos.h"
#include "mi_device.h"
#include "mi_sys_internal.h"
#include "mi_common_internal.h"
#include "sys_sys_boot_timestamp.h"
#include "application_selector.h"

#ifndef USING_BINARY
#ifdef USING_JSON
#include "ptree_db_json.h"
#endif
#ifdef USING_INI
#include "ptree_db_ini.h"
#endif
#else
#include "ptree_bin.h"
#endif
#include "arena.h"
#include "ptree.h"
#include "ptree_log.h"
#include "ptree_preload.h"


#ifdef USING_JSON
#define CONFIG_FILE_PATH "/misc/ptree.json"
#elif USING_INI
#define CONFIG_FILE_PATH "/misc/ptree.ini"
#endif

static void *         g_pIns;
static PTREE_Config_t g_stConfig;


#define MI_CLICMD_PTREE_CMD    66

static int _PRELOAD_RTOS_ParseCmd(void *data);

static int _PtreeTakeOff(PTREE_Config_t *pstConfig, const char *binName)
{
#ifndef USING_BINARY
#ifdef USING_JSON
    pstConfig->pDbInstance = PTREE_DB_JSON_Init(CONFIG_FILE_PATH);
    if(pstConfig->pDbInstance == NULL)
    {
        PTREE_ERR("DB_JSON Init fail!");
        return -1;
    }
#elif USING_INI
    pstConfig->pDbInstance = PTREE_DB_INI_Init(CONFIG_FILE_PATH);
    if(pstConfig->pDbInstance == NULL)
    {
        PTREE_ERR("DB_INI Init fail!");
        return -1;
    }
#else
    else
    {
        return -1;
    }
#endif
    if (ARENA_Create(&pstConfig->pArenaHandle, 0x10000) == -1)
    {
        PTREE_ERR("Arena create fail!");
        return -1;
    }
    return 0;
#else
    const PTREE_BIN_Info_t *info = NULL;
    if (!binName)
    {
        PTREE_ERR("Binary name is NULL\n");
        return -1;
    }
    info = _PTREE_BIN_GetBinInfo(binName);
    if (!info)
    {
        PTREE_ERR("Can not get binary name %s\n", binName);
        return -1;
    }
    PTREE_DBG("BINARY-PIPELINE: %s\n", info->name);
    return ARENA_Map(&pstConfig->pArenaHandle, info->data, info->size);
#endif
}

static int _PtreeDropDown(const PTREE_Config_t *pstConfig)
{
#ifndef USING_BINARY
    ARENA_Destroy(pstConfig->pArenaHandle);
#ifdef USING_JSON
    PTREE_DB_JSON_Deinit(pstConfig->pDbInstance);
#elif USING_INI
    PTREE_DB_INI_Deinit(pstConfig->pDbInstance);
#else
    return -1;
#endif
#else
    ARENA_Unmap(pstConfig->pArenaHandle);
#endif
    return 0;
}

static void _PRELOAD_RTOS_Deint(void)
{

    if (!g_pIns)
    {
        PTREE_ERR("ins is null, not need deinit");
        return;
    }
    PTREE_StopPipeline(g_pIns);
    PTREE_DestructPipeline(g_pIns);
    PTREE_DestroyInstance(g_pIns);
    g_pIns = NULL;
    _PtreeDropDown(&g_stConfig);

    PTREE_PRELOAD_CLear();

    MI_DEVICE_RegMiCliGetDataL2R(MI_CLICMD_PTREE_CMD, NULL);
    MI_DEVICE_RegMiSysExitCall(NULL);
}

static int _PRELOAD_RTOS_Init(int argc, char **argv)
{
    int s32Ret          = 0;
    const char *binName = NULL;

    if (argc == 2)
    {
        binName = argv[1];
    }
    PTREE_DBG("-------------->ready to _PRELOAD_RTOS_Init");
    BootTimestampRecord(__LINE__, "_PRELOAD_RTOS_Init");
    PTREE_PRELOAD_Setup();
    s32Ret = _PtreeTakeOff(&g_stConfig, binName);
    if (s32Ret == -1)
    {
        application_selector_retreat();
        return -1;
    }
    MI_DEVICE_RegMiSysExitCall(_PRELOAD_RTOS_Deint);
    MI_DEVICE_RegMiCliGetDataL2R(MI_CLICMD_PTREE_CMD, _PRELOAD_RTOS_ParseCmd);
    g_pIns = PTREE_CreateInstance(&g_stConfig);
    BootTimestampRecord(__LINE__, "PTREE_Construct");
    PTREE_ConstructPipeline(g_pIns);
    BootTimestampRecord(__LINE__, "PTREE_Start");
    PTREE_StartPipeline(g_pIns);
    BootTimestampRecord(__LINE__, "PTREE_End");
    application_selector_retreat();
    PTREE_DBG("-------------->end to _PRELOAD_RTOS_Init");
    return 0;
}


static int _PRELOAD_RTOS_ParseCmd(void *data)
{
    char * buf = (char *)data;
    char * cmd = NULL;

    if (!strlen(buf))
    {
        CamOsPrintf("receive from linux data is NULL!");
        return -1;
    }

    if (!strncmp(buf, "ptree_deinit", 12))
    {
        _PRELOAD_RTOS_Deint();
        return 0;
    }
    if (!strncmp(buf, "ptree_mod", 9))
    {
        cmd = strstr(buf, " ");
        return PTREE_RunCmdStr(cmd+1);
    }
    if (!strncmp(buf, "ptree_pipe", 10))
    {
        cmd = strstr(buf, " ");
        return PTREE_IoCtlStr(cmd+1);
    }

    CamOsPrintf("ERR: Not found ptree cmd %s\n", buf);
    return -1;
}

//for not run amigos, echo cli exit ptree
static int _PRELOAD_RTOS_DeinitCmd(CLI_t *pCli, char *p)
{
    int cnt = 0;

    cnt = CliTokenCount(pCli);

    if (cnt == 0)
    {
        _PRELOAD_RTOS_Deint();
    }
    else
    {
        PTREE_DBG("cnt %d, not need input args", cnt);
    }

    return 0;
}

SS_RTOS_CLI_CMD(ptree_deinit, "get mixer4.0 To ptree deinit pipeline cmd",
                "Usage: ptree_deinit_cmd [will deinit all]\n", _PRELOAD_RTOS_DeinitCmd);

rtos_application_selector_initcall(ptree, _PRELOAD_RTOS_Init);
