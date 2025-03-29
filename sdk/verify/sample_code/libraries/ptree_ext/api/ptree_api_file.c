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

#include "ssos_def.h"
#include "ssos_list.h"
#include "ptree_log.h"
#include "ssos_mem.h"
#include "ptree_mod.h"
#include "ptree_cmd.h"
#include "ptree_api_file.h"
#include "ptree_sur_file.h"
#include "ptree_mod_file.h"
#include "ptree_maker.h"

static int _PTREE_API_FILE_SetFilePathWrite(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd,
                                            unsigned int paraCnt)
{
    PTREE_MOD_InObj_t *      modIn      = NULL;
    PTREE_SUR_FILE_InInfo_t *fileInInfo = NULL;
    unsigned int             port       = 0;
    const char *             filePath   = 0;

    (void)paraCnt;
    port     = (unsigned int)pstCmd->cmdPara[0];
    filePath = (const char *)pstCmd->cmdPara[1];
    modIn    = PTREE_MOD_GetInObjByPort(pstModObj, port);
    if (!modIn)
    {
        return SSOS_DEF_FAIL;
    }
    if (PTREE_MESSAGE_Check(&modIn->message))
    {
        PTREE_ERR("Can not modify file name if input %d had linked", port);
        return SSOS_DEF_FAIL;
    }
    fileInInfo = CONTAINER_OF(modIn->info, PTREE_SUR_FILE_InInfo_t, base);
    snprintf(fileInInfo->fileName, 256, "%s", filePath);
    return SSOS_DEF_OK;
}

static int _PTREE_API_FILE_SetLeftFrameWrite(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd,
                                             unsigned int paraCnt)
{
    PTREE_MOD_InObj_t *      modIn      = NULL;
    PTREE_SUR_FILE_InInfo_t *fileInInfo = NULL;
    unsigned int             port       = 0;
    unsigned int             left       = 0;

    (void)paraCnt;
    port  = (unsigned int)pstCmd->cmdPara[0];
    left  = (int)pstCmd->cmdPara[1];
    modIn = PTREE_MOD_GetInObjByPort(pstModObj, port);
    if (!modIn)
    {
        return SSOS_DEF_FAIL;
    }
    if (left == 0)
    {
        return SSOS_DEF_FAIL;
    }
    fileInInfo = CONTAINER_OF(modIn->info, PTREE_SUR_FILE_InInfo_t, base);
    if (fileInInfo->frameCntLimit != 0)
    {
        fileInInfo->frameCntLimit = left;
    }
    if (PTREE_MESSAGE_Check(&modIn->message))
    {
        /* modify surface only in this case. */
        PTREE_ERR("Can not set left if input %d had linked", port);
        return SSOS_DEF_OK;
    }
    PTREE_MOD_FILE_SetLeftFrame(modIn, left);
    return SSOS_DEF_OK;
}

static int _PTREE_API_FILE_WaitFrameWrite(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd,
                                          unsigned int paraCnt)
{
    PTREE_MOD_InObj_t *modIn   = NULL;
    unsigned int       port    = 0;
    unsigned int       timeOut = 0;
    unsigned int       ret     = 0;

    (void)paraCnt;
    port    = (unsigned int)pstCmd->cmdPara[0];
    timeOut = (unsigned int)pstCmd->cmdPara[1];
    modIn   = PTREE_MOD_GetInObjByPort(pstModObj, port);
    if (!modIn)
    {
        return SSOS_DEF_FAIL;
    }
    if (!PTREE_MESSAGE_Check(&modIn->message))
    {
        PTREE_ERR("Can not wait if input %d had not linked", port);
        return SSOS_DEF_FAIL;
    }
    ret = PTREE_MOD_FILE_WaitFrame(modIn, timeOut);
    if (ret == SSOS_DEF_ETIMEOUT)
    {
        PTREE_ERR("Wait timeout or error!, timeOut : %d, paraCnt %d", timeOut, paraCnt);
        return SSOS_DEF_ETIMEOUT;
    }
    return SSOS_DEF_OK;
}

static int _PTREE_API_FILE_StartFrameWrite(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd,
                                           unsigned int paraCnt)
{
    PTREE_MOD_InObj_t *modIn = NULL;
    unsigned int       port  = 0;

    (void)paraCnt;
    port = (unsigned int)pstCmd->cmdPara[0];

    modIn = PTREE_MOD_GetInObjByPort(pstModObj, port);
    if (!modIn)
    {
        return SSOS_DEF_FAIL;
    }
    if (PTREE_MESSAGE_Check(&modIn->message))
    {
        PTREE_ERR("File write port %d has linked already", port);
        return SSOS_DEF_FAIL;
    }
    PTREE_MESSAGE_Access(&modIn->message);
    return SSOS_DEF_OK;
}

static int _PTREE_API_FILE_StopFrameWrite(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd,
                                          unsigned int paraCnt)
{
    PTREE_MOD_InObj_t *modIn = NULL;
    unsigned int       port  = 0;

    (void)paraCnt;
    port = (unsigned int)pstCmd->cmdPara[0];

    modIn = PTREE_MOD_GetInObjByPort(pstModObj, port);
    if (!modIn)
    {
        return SSOS_DEF_FAIL;
    }
    if (!PTREE_MESSAGE_Check(&modIn->message))
    {
        PTREE_ERR("File write port %d is not linked", port);
        return SSOS_DEF_FAIL;
    }
    PTREE_MESSAGE_Leave(&modIn->message);
    return SSOS_DEF_OK;
}
PTREE_MAKER_API_INIT(FILE, NULL, NULL,
                     {(unsigned long)E_PTREE_API_FILE_CMD_WAIT_FRAME_WRITE, _PTREE_API_FILE_WaitFrameWrite, 2},
                     {(unsigned long)E_PTREE_API_FILE_CMD_SET_FILE_PATH_WRITE, _PTREE_API_FILE_SetFilePathWrite, 2},
                     {(unsigned long)E_PTREE_API_FILE_CMD_SET_LEFT_FRAME_WRITE, _PTREE_API_FILE_SetLeftFrameWrite, 2},
                     {(unsigned long)E_PTREE_API_FILE_CMD_START_FILE_WRITE, _PTREE_API_FILE_StartFrameWrite, 1},
                     {(unsigned long)E_PTREE_API_FILE_CMD_STOP_FILE_WRITE, _PTREE_API_FILE_StopFrameWrite, 1})
