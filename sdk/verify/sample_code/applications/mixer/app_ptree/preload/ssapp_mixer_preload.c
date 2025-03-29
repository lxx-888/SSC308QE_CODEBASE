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

#include <assert.h>
#include <stdio.h>
#include <time.h>
#include "arena.h"
#include "ptree_bin.h"
#include "ptree.h"
#ifdef USING_JSON
#include "ptree_db_json.h"
#endif
#ifdef USING_INI
#include "ptree_db_ini.h"
#endif

static inline int _SSAPP_MIXER_PRELOAD_FileNameCheck(const char *pFile)
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
    return -1;
}

static inline void _SSAPP_MIXER_PRELOAD_PipelineControlHelp(void)
{
    printf("Pipeline control command usage:\n");
    printf("    [mod name] [dev] [chn] init\n");
    printf("    [mod name] [dev] [chn] deinit\n");
    printf("    [mod name] [dev] [chn] prepare\n");
    printf("    [mod name] [dev] [chn] unprepare\n");
    printf("    [mod name] [dev] [chn] start\n");
    printf("    [mod name] [dev] [chn] stop\n");
    printf("    [mod name] [dev] [chn] bind_all\n");
    printf("    [mod name] [dev] [chn] unbind_all\n");
    printf("    [mod name] [dev] [chn] start_in_all\n");
    printf("    [mod name] [dev] [chn] stop_in_all\n");
    printf("    [mod name] [dev] [chn] start_out_all\n");
    printf("    [mod name] [dev] [chn] stop_out_all\n");
    printf("    [mod name] [dev] [chn] bind          [input port]\n");
    printf("    [mod name] [dev] [chn] unbind        [input port]\n");
    printf("    [mod name] [dev] [chn] start_in      [input port]\n");
    printf("    [mod name] [dev] [chn] stop_in       [input port]\n");
    printf("    [mod name] [dev] [chn] start_out     [output port]\n");
    printf("    [mod name] [dev] [chn] stop_out      [output port]\n");
    printf("    [mod name] [dev] [chn] create_delay  [input port]\n");
    printf("    [mod name] [dev] [chn] destroy_delay [input port]\n");
    printf("    [mod name] [dev] [chn] init_delay    [input port]\n");
    printf("    [mod name] [dev] [chn] deinit_delay  [input port]\n");
    printf("    [mod name] [dev] [chn] bind_delay    [input port]\n");
    printf("    [mod name] [dev] [chn] unbind_delay  [input port]\n");
    printf("    [mod name] [dev] [chn] start_delay   [input port]\n");
    printf("    [mod name] [dev] [chn] stop_delay    [input port]\n");
}

unsigned long SSAPP_MIXER_PRELOAD_GetTimer(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000000) + (ts.tv_nsec / 1000);
}

int SSAPP_MIXER_PRELOAD_PtreeTakeOff(PTREE_Config_t *pstConfig, int s32Argc, char **ppArgv)
{
    /* Default set as using binary. */
    int                     s32Ret  = 2;
    const PTREE_BIN_Info_t *binInfo = NULL;

    if (s32Argc != 2)
    {
        printf("Usage: %s [binary name] | [json/ini file]\n", ppArgv[0]);
        return -1;
    }
    s32Ret = _SSAPP_MIXER_PRELOAD_FileNameCheck(ppArgv[1]);
    if (s32Ret == 0)
    {
#ifdef USING_JSON
        pstConfig->pDbInstance = PTREE_DB_JSON_Init(ppArgv[1]);
#endif
        return (pstConfig->pDbInstance && ARENA_Create(&pstConfig->pArenaHandle, 0x10000) == 0) ? s32Ret : -1;
    }
    if (s32Ret == 1)
    {
#ifdef USING_INI
        pstConfig->pDbInstance = PTREE_DB_INI_Init(ppArgv[1]);
#endif
        return (pstConfig->pDbInstance && ARENA_Create(&pstConfig->pArenaHandle, 0x10000) == 0) ? s32Ret : -1;
    }
    /* Binary mapping. */
    pstConfig->pDbInstance = NULL;
    binInfo                = _PTREE_BIN_GetBinInfo(ppArgv[1]);
    if (binInfo)
    {
        return (ARENA_Map(&pstConfig->pArenaHandle, binInfo->data, binInfo->size) == 0) ? 2 : -1;
    }
    printf("Can not find the pipeline name: %s\n", ppArgv[1]);
    return -1;
}
int SSAPP_MIXER_PRELOAD_PtreeDropDown(int s32DbChoice, PTREE_Config_t *pstConfig)
{
    if (s32DbChoice == 2)
    {
        ARENA_Unmap(pstConfig->pArenaHandle);
        return 0;
    }
    ARENA_Destroy(pstConfig->pArenaHandle);
    if (s32DbChoice == 0)
    {
#ifdef USING_JSON
        PTREE_DB_JSON_Deinit(pstConfig->pDbInstance);
#endif
        return 0;
    }
    if (s32DbChoice == 1)
    {
#ifdef USING_INI
        PTREE_DB_INI_Deinit(pstConfig->pDbInstance);
#endif
        return 0;
    }
    printf("Error in choice %d\n", s32DbChoice);
    return -1;
}

void SSAPP_MIXER_PRELOAD_PipelineModuleCommandConsole(void)
{
    char          buf[1024] = {0};
    int           cmdRet    = 0;
    unsigned long ulTimer   = 0;

    printf("Type 'commands' to change the setting of module!\n");
    printf("Type 'q' to get back to the previous menu!\n");
    while (1)
    {
        fgets(buf, sizeof(buf), stdin);
        if (buf[0] == '\n' && buf[1] == '\0')
        {
            continue;
        }
        if (buf[0] == 'q' && buf[1] == '\n')
        {
            return;
        }
        ulTimer = SSAPP_MIXER_PRELOAD_GetTimer();
        cmdRet  = PTREE_RunCmdStr(buf);
        printf("CMD: %sreturn: %d duration %luus\n", buf, cmdRet, SSAPP_MIXER_PRELOAD_GetTimer() - ulTimer);
    }
}

void SSAPP_MIXER_PRELOAD_PipelineControlConsole(void *pIns)
{
    int           ret       = 0;
    char          buf[1024] = {0};
    unsigned long ulTimer   = 0;

    printf("Type 'commands' to contorl pipeline!\n");
    _SSAPP_MIXER_PRELOAD_PipelineControlHelp();
    while (1)
    {
        printf("Type 'q' to get back to the previous menu!\n");
        printf("Type 's' to start all pipeline modules!\n");
        printf("Type 't' to stop all pipeline modules!\n");
        printf("Type 'c' to construct all pipeline modules!\n");
        printf("Type 'd' to destruct all pipeline modules!\n");
        printf("Type 'h' to get help of 'commands'!\n");
        fgets(buf, sizeof(buf), stdin);
        if (buf[0] == '\n' && buf[1] == '\0')
        {
            continue;
        }
        if (buf[0] == 'h' && buf[1] == '\n')
        {
            _SSAPP_MIXER_PRELOAD_PipelineControlHelp();
            continue;
        }
        if (buf[0] == 's' && buf[1] == '\n')
        {
            PTREE_StartPipeline(pIns);
            continue;
        }
        if (buf[0] == 't' && buf[1] == '\n')
        {
            PTREE_StopPipeline(pIns);
            continue;
        }
        if (buf[0] == 'c' && buf[1] == '\n')
        {
            PTREE_ConstructPipeline(pIns);
            continue;
        }
        if (buf[0] == 'd' && buf[1] == '\n')
        {
            PTREE_DestructPipeline(pIns);
            continue;
        }
        if (buf[0] == 'q' && buf[1] == '\n')
        {
            return;
        }
        ulTimer = SSAPP_MIXER_PRELOAD_GetTimer();
        ret     = PTREE_IoCtlStr(buf);
        printf("CMD: %sreturn %d duration %luus!\n", buf, ret, SSAPP_MIXER_PRELOAD_GetTimer() - ulTimer);
    }
}
