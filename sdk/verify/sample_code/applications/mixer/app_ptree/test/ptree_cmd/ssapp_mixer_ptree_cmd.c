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
#include "ptree_maker.h"
#ifdef USING_JSON
#include "ptree_db_json.h"
#endif
#ifdef USING_INI
#include "ptree_db_ini.h"
#endif
#include "arena.h"
#include "ptree.h"

static inline int _SSAPP_MIXER_PTREE_CMD_FileNameCheck(const char *pFile)
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
static int _SSAPP_MIXER_PTREE_CMD_PtreeTakeOff(int s32Argc, char **ppArgv, PTREE_Config_t *pstConfig)
{
    /* Default set as using binary. */
    int s32Ret = 2;

    if (s32Argc == 2)
    {
        s32Ret = _SSAPP_MIXER_PTREE_CMD_FileNameCheck(ppArgv[1]);
        if (s32Ret == 0)
        {
#ifdef USING_JSON
            pstConfig->pDbInstance = PTREE_DB_JSON_Init(ppArgv[1]);
#endif
        }
        else if (s32Ret == 1)
        {
#ifdef USING_INI
            pstConfig->pDbInstance = PTREE_DB_INI_Init(ppArgv[1]);
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
        return s32Ret;
    }
#include "auto_gen_cmd_test.h"
    pstConfig->pDbInstance = NULL;
    if (ARENA_Map(&pstConfig->pArenaHandle, auto_gen_cmd_test, sizeof(auto_gen_cmd_test)) == -1)
    {
        return -1;
    }
    return s32Ret;
}
static int _SSAPP_MIXER_PTREE_CMD_PtreeDropDown(int s32DbChoice, PTREE_Config_t *pstConfig)
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

int main(int argc, char **argv)
{
    int            s32Ret    = 0;
    void *         pInstance = NULL;
    PTREE_Config_t stConfig;
    PTREE_MAKER_SETUP(CMD_TEST);
    PTREE_MAKER_SETUP(STR_CMD_TEST, EMPTY0, EMPTY1, EMPTY2, EMPTY3, EMPTY4);
    s32Ret = _SSAPP_MIXER_PTREE_CMD_PtreeTakeOff(argc, argv, &stConfig);
    if (s32Ret == -1)
    {
        return s32Ret;
    }
    pInstance = PTREE_CreateInstance(&stConfig);
    if (!pInstance)
    {
        printf("Create instance fail!\n");
        return -1;
    }
    PTREE_ConstructPipeline(pInstance);
    PTREE_StartPipeline(pInstance);

    PTREE_CMD("CMD_TEST", 0, 0, 0, 333, 444, 555);
    PTREE_CMD("CMD_TEST", 0, 0, 1, 555, 666, 777);
    PTREE_CMD("CMD_TEST", 0, 0, 2);

    PTREE_CMD("CMD_TEST", 0, 1, 0, 333, 444, 555);
    PTREE_CMD("CMD_TEST", 0, 1, 1, 555, 666, 777);
    PTREE_CMD("CMD_TEST", 0, 1, 2);

    PTREE_CMD_STR("STR_CMD_TEST 0 0 cmd0 %s", "How are you!");
    PTREE_CMD_STR("STR_CMD_TEST 0 0 cmd1 %s", "SigmaStar is the best!");
    PTREE_CMD_STR("STR_CMD_TEST 0 0 cmd2");

    PTREE_CMD_STR("STR_CMD_TEST 0 1 cmd3 %s", "I am fine.");
    PTREE_CMD_STR("STR_CMD_TEST 0 1 cmd4 %s", "I also think so.");
    PTREE_CMD_STR("STR_CMD_TEST 0 1 cmd5");

    PTREE_CMD_STR("STR_CMD_TEST 0 0 cmd6 %s", "How are you!");
    PTREE_CMD_STR("STR_CMD_TEST 0 0 cmd7 %s", "SigmaStar is the best!");
    PTREE_CMD_STR("STR_CMD_TEST 0 0 cmd8");

    PTREE_CMD_STR("STR_CMD_TEST 0 1 cmd9 %s", "I am fine.");
    PTREE_CMD_STR("STR_CMD_TEST 0 1 cmd10 %s", "I also think so.");
    PTREE_CMD_STR("STR_CMD_TEST 0 1 cmd11");

    PTREE_CMD_STR("STR_CMD_TEST 0 1 cmd12 %s", "I am fine.");
    PTREE_CMD_STR("STR_CMD_TEST 0 1 cmd13 %s", "I also think so.");
    PTREE_CMD_STR("STR_CMD_TEST 0 1 cmd14");

    PTREE_CMD_STR("STR_CMD_TEST 0 1 cmd15 %s", "I am fine.");
    PTREE_CMD_STR("STR_CMD_TEST 0 1 cmd16 %s", "I also think so.");
    PTREE_CMD_STR("STR_CMD_TEST 0 1 cmd17");

    PTREE_StopPipeline(pInstance);
    PTREE_DestructPipeline(pInstance);
    PTREE_DestroyInstance(pInstance);
    _SSAPP_MIXER_PTREE_CMD_PtreeDropDown(s32Ret, &stConfig);
    PTREE_MAKER_Clear();
    return 0;
}
