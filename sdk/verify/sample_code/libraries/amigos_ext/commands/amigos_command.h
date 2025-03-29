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
#ifndef __AMIGOS_COMMAND__
#define __AMIGOS_COMMAND__

#include <string>
#include <set>
#include <utility>
#include "ss_cmd_base.h"

class AmigosCommand
{
    public:
        template<typename... Args>
        AmigosCommand(const std::string &modName, Args&&... args)
            : cmdModName(modName)
        {
            Install();
            TraverseArg(std::forward<Args>(args)...);
        }
        virtual ~AmigosCommand() {}
        const std::set<mod_init_func> &GetModFunc()
        {
            return setCmdFunc;
        }
        map<string, struct base_command_data> &GetCmdData() {return mapCmdData;}
        static AmigosCommand *GetObj(const std::string &modName);
    private:
        void Install();
        void ProcessArg(mod_init_func && fp)
        {
            fp(mapCmdData);
            setCmdFunc.insert(fp);
        }
        template <typename T, typename... Args>
        void TraverseArg(T&& arg, Args&&... args)
        {
            ProcessArg(std::forward<T>(arg));
            TraverseArg(std::forward<Args>(args)...);
        }
        void TraverseArg() {}
        map<string, struct base_command_data> mapCmdData;
        std::string cmdModName;
        std::set<mod_init_func > setCmdFunc;
};
#define COMBINE(__a, __b) _COMBINE(__a, __b)
#define _COMBINE(__a, __b) __a##__b

#define AMIGOS_COMMAND_ARG1(__arg1)                                                                      \
        extern void __sscmd_early_init_func_AmiCmd##__arg1(map<string, struct base_command_data> &data);

#define AMIGOS_COMMAND_ARG2(__arg1, __arg2)                                                              \
        extern void __sscmd_early_init_func_AmiCmd##__arg2(map<string, struct base_command_data> &data); \
        AMIGOS_COMMAND_ARG1(__arg1)

#define AMIGOS_COMMAND_ARG3(__arg1, __arg2, __arg3)                                                      \
        extern void __sscmd_early_init_func_AmiCmd##__arg3(map<string, struct base_command_data> &data); \
        AMIGOS_COMMAND_ARG2(__arg1, __arg2)

#define AMIGOS_COMMAND_ARG4(__arg1, __arg2, __arg3, __arg4)                                              \
        extern void __sscmd_early_init_func_AmiCmd##__arg4(map<string, struct base_command_data> &data); \
        AMIGOS_COMMAND_ARG3(__arg1, __arg2, __arg3)

#define AMIGOS_COMMAND_ARG5(__arg1, __arg2, __arg3, __arg4, __arg5)                                      \
        extern void __sscmd_early_init_func_AmiCmd##__arg5(map<string, struct base_command_data> &data); \
        AMIGOS_COMMAND_ARG4(__arg1, __arg2, __arg3, __arg4)

#define AMIGOS_COMMAND_OBJ1(__name, __arg1)                                                                          \
        static AmigosCommand __amigos_command_object_AmiCmd##__arg1(__name,                                          \
                                                                    COMBINE(__sscmd_early_init_func_AmiCmd, __arg1))
#define AMIGOS_COMMAND_OBJ2(__name, __arg1, __arg2) \
        static AmigosCommand __amigos_command_object_AmiCmd##__arg1(__name,                                          \
                                                                    COMBINE(__sscmd_early_init_func_AmiCmd, __arg1), \
                                                                    COMBINE(__sscmd_early_init_func_AmiCmd, __arg2))
#define AMIGOS_COMMAND_OBJ3(__name, __arg1, __arg2, __arg3)                                                           \
        static AmigosCommand __amigos_command_object_AmiCmd##__arg1(__name,                                           \
                                                                     COMBINE(__sscmd_early_init_func_AmiCmd, __arg1), \
                                                                     COMBINE(__sscmd_early_init_func_AmiCmd, __arg2), \
                                                                     COMBINE(__sscmd_early_init_func_AmiCmd, __arg3))
#define AMIGOS_COMMAND_OBJ4(__name, __arg1, __arg2, __arg3, __arg4)                                                   \
        static AmigosCommand __amigos_command_object_AmiCmd##__arg1(__name,                                           \
                                                                     COMBINE(__sscmd_early_init_func_AmiCmd, __arg1), \
                                                                     COMBINE(__sscmd_early_init_func_AmiCmd, __arg2), \
                                                                     COMBINE(__sscmd_early_init_func_AmiCmd, __arg3), \
                                                                     COMBINE(__sscmd_early_init_func_AmiCmd, __arg4))
#define AMIGOS_COMMAND_OBJ5(__name, __arg1, __arg2, __arg3, __arg4, __arg5)                                           \
        static AmigosCommand __amigos_command_object_AmiCmd##__arg1(__name,                                           \
                                                                     COMBINE(__sscmd_early_init_func_AmiCmd, __arg1), \
                                                                     COMBINE(__sscmd_early_init_func_AmiCmd, __arg2), \
                                                                     COMBINE(__sscmd_early_init_func_AmiCmd, __arg3), \
                                                                     COMBINE(__sscmd_early_init_func_AmiCmd, __arg4), \
                                                                     COMBINE(__sscmd_early_init_func_AmiCmd, __arg5))
#define AMIGOS_COMMAND_ARGC(...)               \
        _AMIGOS_COMMAND_ARGC(__VA_ARGS__, 5, 4, 3, 2, 1)
#define _AMIGOS_COMMAND_ARGC(__1, __2, __3, __4, __5, N, ...) N

#define AMIGOS_COMMAND_SETUP(__name, ...)                                                  \
        COMBINE(AMIGOS_COMMAND_ARG, AMIGOS_COMMAND_ARGC(__VA_ARGS__))(__VA_ARGS__);        \
        COMBINE(AMIGOS_COMMAND_OBJ, AMIGOS_COMMAND_ARGC(__VA_ARGS__))(__name, __VA_ARGS__);

#define AMIGOS_MODULE_SETUP(__mod)\
        extern void __amigos_early_init_preload_AmigosModule##__mod(void); \
        __amigos_early_init_preload_AmigosModule##__mod();

#define AMIGOS_SETUP(__name, __mod, ...)                   \
        AMIGOS_MODULE_SETUP(__mod);                        \
        AMIGOS_COMMAND_SETUP(__name, __mod, ##__VA_ARGS__, Base)
#endif
