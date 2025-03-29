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

#ifndef __PTREE_CMD_H__
#define __PTREE_CMD_H__

#include "ssos_list.h"
#include "ptree_obj.h"
#include "ptree_mod.h"

typedef struct PTREE_CMD_Obj_s PTREE_CMD_Obj_t;
typedef struct PTREE_CMD_Ops_s PTREE_CMD_Ops_t;

typedef struct PTREE_CMD_Args_s   PTREE_CMD_Args_t;
typedef struct PTREE_CMD_Define_s PTREE_CMD_Define_t;

typedef int (*PTREE_CMD_Func_t)(PTREE_MOD_Obj_t *mod, const PTREE_CMD_Args_t *args, unsigned int argc);

struct PTREE_CMD_Args_s
{
    unsigned long cmdId;
    unsigned long cmdPara[0];
};

struct PTREE_CMD_Define_s
{
    unsigned long    cmdId;
    PTREE_CMD_Func_t cmdFunc;
    unsigned int     cmdParaCnt;
};

struct PTREE_CMD_Ops_s
{
    /* It can customize the hash value of each cmdId, set NULL is using default.*/
    unsigned int (*hashVal)(unsigned long cmdId, unsigned int hashSize);
    /* Compare the dst cmdId and the stored cmdIn if hash value is equal, set NULL is using default */
    unsigned char (*compare)(unsigned long inCmdId, unsigned long cmdId);
};

struct PTREE_CMD_Obj_s
{
    PTREE_OBJ_Obj_t          obj;
    PTREE_CMD_Ops_t          ops;
    unsigned int             hashSize;
    struct SSOS_LIST_Head_s *arrHashCmd;
};

int PTREE_CMD_Init(PTREE_CMD_Obj_t *cmd, const PTREE_CMD_Ops_t *ops, const PTREE_CMD_Define_t *cmdDefines,
                   unsigned int cmdCnt);

int PTREE_CMD_Add(PTREE_CMD_Obj_t *cmd, const PTREE_CMD_Define_t *cmdDefines, unsigned int cmdCnt);

int PTREE_CMD_AddMod(PTREE_CMD_Obj_t *cmd, PTREE_MOD_Obj_t *mod);

int PTREE_CMD_DelMod(PTREE_CMD_Obj_t *cmd, PTREE_MOD_Obj_t *mod);

void PTREE_CMD_Del(PTREE_CMD_Obj_t *cmd);

int PTREE_CMD_Run(const char *modName, unsigned int devId, unsigned int chnId, const PTREE_CMD_Args_t *args,
                  unsigned int argc);

unsigned int PTREE_CMD_StrHashVal(unsigned long cmdId, unsigned int hashSize);

unsigned char PTREE_CMD_StrCompare(unsigned long inCmdId, unsigned long cmdId);

#endif /* __PTREE_CMD_H__ */
