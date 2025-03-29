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

#ifndef __PTREE_OBJ_H__
#define __PTREE_OBJ_H__

#include "ssos_thread.h"

typedef struct PTREE_OBJ_Obj_s  PTREE_OBJ_Obj_t;
typedef struct PTREE_OBJ_Hook_s PTREE_OBJ_Hook_t;

struct PTREE_OBJ_Obj_s
{
    /*< public >*/
    const PTREE_OBJ_Hook_t *hook;
    PTREE_OBJ_Obj_t *       parent;
    char *                  name;
    /*< private >*/
    SSOS_THREAD_Mutex_t mutex;
    unsigned int        ref;
    unsigned int        stateInit : 1;
};

struct PTREE_OBJ_Hook_s
{
    void (*destruct)(PTREE_OBJ_Obj_t *obj);
    void (*free)(PTREE_OBJ_Obj_t *obj);
};

int PTREE_OBJ_Init(PTREE_OBJ_Obj_t *obj);

void PTREE_OBJ_Register(PTREE_OBJ_Obj_t *obj, const PTREE_OBJ_Hook_t *hook);

int PTREE_OBJ_Add(PTREE_OBJ_Obj_t *obj, PTREE_OBJ_Obj_t *parent, const char *name);

PTREE_OBJ_Obj_t *PTREE_OBJ_Dup(PTREE_OBJ_Obj_t *obj);

void PTREE_OBJ_Del(PTREE_OBJ_Obj_t *obj);

const char *PTREE_OBJ_Name(PTREE_OBJ_Obj_t *obj);

PTREE_OBJ_Obj_t *PTREE_OBJ_Parent(PTREE_OBJ_Obj_t *obj);

#endif /* ifndef __PTREE_OBJ_H__ */
