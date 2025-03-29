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

#ifndef __PTREE_MAKER_H__
#define __PTREE_MAKER_H__

#include "parena.h"
#include "ptree_cmd.h"
#include "ssos_hash.h"
#include "ssos_list.h"
#include "ssos_mem.h"
#include "ptree_sur.h"
#include "ptree_mod.h"

typedef struct PTREE_MAKER_Obj_s PTREE_MAKER_Obj_t;
typedef struct PTREE_MAKER_Ops_s PTREE_MAKER_Ops_t;

struct PTREE_MAKER_Ops_s
{
    PTREE_SUR_Obj_t *(*makeSur)(void);
    PTREE_MOD_Obj_t *(*makeMod)(PARENA_Tag_t *tag);
    PTREE_CMD_Obj_t *(*makeCmd)(void);
};
struct PTREE_MAKER_Obj_s
{
    struct SSOS_LIST_Head_s hashEntry;
    const char *            type;
    PTREE_MOD_Obj_t *(*makeMod)(PARENA_Tag_t *tag);
    PTREE_SUR_Obj_t *sur;
    PTREE_CMD_Obj_t *cmd;
};

PTREE_MAKER_Obj_t *PTREE_MAKER_Setup(const char *type, const PTREE_MAKER_Ops_t *ops);

PTREE_MAKER_Obj_t *PTREE_MAKER_Get(const char *type);

void PTREE_MAKER_Clear(void);

PTREE_SUR_Obj_t *PTREE_MAKER_MakeSur(PTREE_MAKER_Obj_t *maker);

void PTREE_MAKER_DelSur(PTREE_SUR_Obj_t *sur);

PTREE_MOD_Obj_t *PTREE_MAKER_MakeMod(PTREE_MAKER_Obj_t *maker, PARENA_Tag_t *tag);

void PTREE_MAKER_DelMod(PTREE_MOD_Obj_t *mod);

PTREE_CMD_Obj_t *PTREE_MAKER_MakeCmd(PTREE_MAKER_Obj_t *maker);

void PTREE_MAKER_Delcmd(PTREE_CMD_Obj_t *cmd);

#define PTREE_MAKER_SUR_INIT(_name, _newFunc)        \
    PTREE_SUR_Obj_t *__PTREE_MAKER_SUR_##_name(void) \
    {                                                \
        return _newFunc();                           \
    }

#define PTREE_MAKER_MOD_INIT(_name, _newFunc)                     \
    PTREE_MOD_Obj_t *__PTREE_MAKER_MOD_##_name(PARENA_Tag_t *tag) \
    {                                                             \
        return _newFunc(tag);                                     \
    }

#define PTREE_MAKER_CMD_INIT(_name, _compare, _hashVal, ...)                                                \
    PTREE_CMD_Obj_t *__PTREE_MAKER_CMD_##_name(void)                                                        \
    {                                                                                                       \
        PTREE_CMD_Ops_t __cmdOps = {                                                                        \
            .hashVal = _hashVal,                                                                            \
            .compare = _compare,                                                                            \
        };                                                                                                  \
        PTREE_CMD_Define_t __cmdDefine[] = {__VA_ARGS__};                                                   \
        PTREE_CMD_Obj_t *  __cmd         = SSOS_MEM_Alloc(sizeof(PTREE_CMD_Obj_t));                         \
        if (!__cmd)                                                                                         \
        {                                                                                                   \
            return NULL;                                                                                    \
        }                                                                                                   \
        memset(__cmd, 0, sizeof(PTREE_CMD_Obj_t));                                                          \
        if (SSOS_DEF_OK                                                                                     \
            != PTREE_CMD_Init(__cmd, &__cmdOps, __cmdDefine, sizeof(__cmdDefine) / sizeof(__cmdDefine[0]))) \
        {                                                                                                   \
            SSOS_MEM_Free(__cmd);                                                                           \
            return NULL;                                                                                    \
        }                                                                                                   \
        return __cmd;                                                                                       \
    }

#define PTREE_MAKER_CMD_ADD(_name, ...)                                                 \
    void __PTREE_MAKER_CMD_ADD_##_name(PTREE_CMD_Obj_t *_cmd)                           \
    {                                                                                   \
        PTREE_CMD_Define_t __cmdDefine[] = {__VA_ARGS__};                               \
        PTREE_CMD_Add(_cmd, __cmdDefine, sizeof(__cmdDefine) / sizeof(__cmdDefine[0])); \
    }

#define PTREE_MAKER_API_INIT(_name, _compare, _hashVal, ...)                                                \
    PTREE_CMD_Obj_t *__PTREE_MAKER_API_##_name(void)                                                        \
    {                                                                                                       \
        PTREE_CMD_Ops_t __cmdOps = {                                                                        \
            .hashVal = _hashVal,                                                                            \
            .compare = _compare,                                                                            \
        };                                                                                                  \
        PTREE_CMD_Define_t __cmdDefine[] = {__VA_ARGS__};                                                   \
        PTREE_CMD_Obj_t *  __cmd         = SSOS_MEM_Alloc(sizeof(PTREE_CMD_Obj_t));                         \
        if (!__cmd)                                                                                         \
        {                                                                                                   \
            return NULL;                                                                                    \
        }                                                                                                   \
        memset(__cmd, 0, sizeof(PTREE_CMD_Obj_t));                                                          \
        if (SSOS_DEF_OK                                                                                     \
            != PTREE_CMD_Init(__cmd, &__cmdOps, __cmdDefine, sizeof(__cmdDefine) / sizeof(__cmdDefine[0]))) \
        {                                                                                                   \
            SSOS_MEM_Free(__cmd);                                                                           \
            return NULL;                                                                                    \
        }                                                                                                   \
        return __cmd;                                                                                       \
    }

#define PTREE_MAKER_API_ADD(_name, ...)                                                 \
    void __PTREE_MAKER_API_ADD_##_name(PTREE_CMD_Obj_t *_cmd)                           \
    {                                                                                   \
        PTREE_CMD_Define_t __cmdDefine[] = {__VA_ARGS__};                               \
        PTREE_CMD_Add(_cmd, __cmdDefine, sizeof(__cmdDefine) / sizeof(__cmdDefine[0])); \
    }

#define __PTREE_MAKER_SUR_SETUP(_name)                               \
    (                                                                \
        {                                                            \
            extern PTREE_SUR_Obj_t *__PTREE_MAKER_SUR_##_name(void); \
            __PTREE_MAKER_SUR_##_name;                               \
        })

#define __PTREE_MAKER_MOD_SETUP(_name)                                         \
    (                                                                          \
        {                                                                      \
            extern PTREE_MOD_Obj_t *__PTREE_MAKER_MOD_##_name(PARENA_Tag_t *); \
            __PTREE_MAKER_MOD_##_name;                                         \
        })

#define __PTREE_MAKER_CMD_SETUP(_name)                               \
    (                                                                \
        {                                                            \
            extern PTREE_CMD_Obj_t *__PTREE_MAKER_CMD_##_name(void); \
            __PTREE_MAKER_CMD_##_name;                               \
        })

#define __PTREE_MAKER_API_SETUP(_name)                               \
    (                                                                \
        {                                                            \
            extern PTREE_CMD_Obj_t *__PTREE_MAKER_API_##_name(void); \
            __PTREE_MAKER_API_##_name;                               \
        })

#define __PTREE_MAKER_CMD_ADD(_cmd, _name)                            \
    do                                                                \
    {                                                                 \
        extern void __PTREE_MAKER_CMD_ADD_##_name(PTREE_CMD_Obj_t *); \
        __PTREE_MAKER_CMD_ADD_##_name(_cmd);                          \
    } while (0)

#define __PTREE_MAKER_API_ADD(_cmd, _name)                            \
    do                                                                \
    {                                                                 \
        extern void __PTREE_MAKER_API_ADD_##_name(PTREE_CMD_Obj_t *); \
        __PTREE_MAKER_API_ADD_##_name(_cmd);                          \
    } while (0)

#define __COMBINE(__a, __b)                                        __a##__b
#define __PTREE_MAKER_COMBINE(__a, __b)                            __COMBINE(__a, __b)
#define __PTREE_MAKER_ARGC(__0, __1, __2, __3, __4, __5, __N, ...) __N
#define __PTREE_MAKER_PARA_ID(...)                                 __PTREE_MAKER_ARGC(__VA_ARGS__, 5, 4, 3, 2, 1, 0)

#define __PTREE_MAKER_APPEND_CMD0(__cmd, ...) (void)(__cmd)

#define __PTREE_MAKER_APPEND_CMD1(__cmd, __name) __PTREE_MAKER_CMD_ADD(__cmd, __name)

#define __PTREE_MAKER_APPEND_CMD2(__cmd, __name, ...) \
    __PTREE_MAKER_CMD_ADD(__cmd, __name);             \
    __PTREE_MAKER_APPEND_CMD1(__cmd, ##__VA_ARGS__)

#define __PTREE_MAKER_APPEND_CMD3(__cmd, __name, ...) \
    __PTREE_MAKER_CMD_ADD(__cmd, __name);             \
    __PTREE_MAKER_APPEND_CMD2(__cmd, ##__VA_ARGS__)

#define __PTREE_MAKER_APPEND_CMD4(__cmd, __name, ...) \
    __PTREE_MAKER_CMD_ADD(__cmd, __name);             \
    __PTREE_MAKER_APPEND_CMD3(__cmd, ##__VA_ARGS__)

#define __PTREE_MAKER_APPEND_CMD5(__cmd, __name, ...) \
    __PTREE_MAKER_CMD_ADD(__cmd, __name);             \
    __PTREE_MAKER_APPEND_CMD4(__cmd, ##__VA_ARGS__)

#define __PTREE_MAKER_CMD_APPEND(__cmd, __name, ...)                                              \
    __PTREE_MAKER_COMBINE(__PTREE_MAKER_APPEND_CMD, __PTREE_MAKER_PARA_ID(__name, ##__VA_ARGS__)) \
    (__cmd, ##__VA_ARGS__)

#define __PTREE_MAKER_APPEND_API0(__cmd, ...) (void)(__cmd)

#define __PTREE_MAKER_APPEND_API1(__cmd, __name) __PTREE_MAKER_API_ADD(__cmd, __name)

#define __PTREE_MAKER_APPEND_API2(__cmd, __name, ...) \
    __PTREE_MAKER_API_ADD(__cmd, __name);             \
    __PTREE_MAKER_APPEND_API1(__cmd, ##__VA_ARGS__)

#define __PTREE_MAKER_APPEND_API3(__cmd, __name, ...) \
    __PTREE_MAKER_API_ADD(__cmd, __name);             \
    __PTREE_MAKER_APPEND_API2(__cmd, ##__VA_ARGS__)

#define __PTREE_MAKER_APPEND_API4(__cmd, __name, ...) \
    __PTREE_MAKER_API_ADD(__cmd, __name);             \
    __PTREE_MAKER_APPEND_API3(__cmd, ##__VA_ARGS__)

#define __PTREE_MAKER_APPEND_API5(__cmd, __name, ...) \
    __PTREE_MAKER_API_ADD(__cmd, __name);             \
    __PTREE_MAKER_APPEND_API4(__cmd, ##__VA_ARGS__)

#define __PTREE_MAKER_API_APPEND(__cmd, __name, ...)                                              \
    __PTREE_MAKER_COMBINE(__PTREE_MAKER_APPEND_API, __PTREE_MAKER_PARA_ID(__name, ##__VA_ARGS__)) \
    (__cmd, ##__VA_ARGS__)

/*
 * Setup module by '_name' and using the same name as default command.
 */
#define PTREE_MAKER_SETUP(_name, ...)                                       \
    (                                                                       \
        {                                                                   \
            PTREE_MAKER_Ops_t __ops = {                                     \
                .makeSur = __PTREE_MAKER_SUR_SETUP(_name),                  \
                .makeMod = __PTREE_MAKER_MOD_SETUP(_name),                  \
                .makeCmd = __PTREE_MAKER_CMD_SETUP(_name),                  \
            };                                                              \
            PTREE_MAKER_Obj_t *__maker = PTREE_MAKER_Setup(#_name, &__ops); \
            __PTREE_MAKER_CMD_APPEND(__maker->cmd, _name, ##__VA_ARGS__);   \
        })

/*
 * Setup module by '_name' and using the same name as default api.
 */
#define PTREE_MAKER_SETUP_API(_name, ...)                                   \
    (                                                                       \
        {                                                                   \
            PTREE_MAKER_Ops_t __ops = {                                     \
                .makeSur = __PTREE_MAKER_SUR_SETUP(_name),                  \
                .makeMod = __PTREE_MAKER_MOD_SETUP(_name),                  \
                .makeCmd = __PTREE_MAKER_API_SETUP(_name),                  \
            };                                                              \
            PTREE_MAKER_Obj_t *__maker = PTREE_MAKER_Setup(#_name, &__ops); \
            __PTREE_MAKER_API_APPEND(__maker->cmd, _name, ##__VA_ARGS__);   \
        })

/*
 * Setup module by '_name' and do not init any command or api.
 */
#define PTREE_MAKER_MOD_SETUP(_name)                       \
    (                                                      \
        {                                                  \
            PTREE_MAKER_Ops_t __ops = {                    \
                .makeSur = __PTREE_MAKER_SUR_SETUP(_name), \
                .makeMod = __PTREE_MAKER_MOD_SETUP(_name), \
                .makeCmd = NULL,                           \
            };                                             \
            PTREE_MAKER_Setup(#_name, &__ops);             \
        })

#endif /* ifndef __PTREE_MAKER_H__ */
