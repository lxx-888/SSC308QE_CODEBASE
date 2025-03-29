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
#ifndef __SS_CMD_BASE__
#define __SS_CMD_BASE__

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <map>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>

#include "ss_handle.h"
#include "ss_log.h"

using namespace std;
#define PCIE_EP_CHIP_ROUTE_MASK 0x10000

#define INIT_CMD_BASE(__prog)                                                  \
        std::map<std::string, struct base_command_data> ss_cmd_base::cmd_data; \
        std::map<std::string, mod_init_func> ss_cmd_base::mod_func;            \
        unsigned int ss_cmd_base::selected_chip_id = 0;                        \
        std::string ss_cmd_base::prog_name = #__prog;                          \
        std::string ss_cmd_base::prog_path;                                    \
        ext_init_data ss_cmd_base::ext_data;                                   \

//Module setup outside function.
#define MOD_SETUP(__name)                                                                                     \
        extern void __sscmd_early_init_func_##__name(std::map<std::string, struct base_command_data> &data);  \
        static ss_cmd_preload __sscmd_early_init_preload_##__name(__sscmd_early_init_func_##__name, #__name); \

//Module setup inside function.
#define MOD_SETUP_IN(__name)                                                                                 \
        extern void __sscmd_early_init_func_##__name(std::map<std::string, struct base_command_data> &data); \
        ss_cmd_base::regist_module(__sscmd_early_init_func_##__name, #__name);

#define MOD_SETUP_IN_EXT(__name, __ext_cmd)                                                                  \
        extern void __sscmd_early_init_func_##__name(std::map<std::string, struct base_command_data> &data); \
        __sscmd_early_init_func_##__name(__ext_cmd);

#define MOD_CMDS(__name)                                                                               \
        void __sscmd_early_init_func_##__name(std::map<std::string, struct base_command_data> &__data)

#define ADD_CMD(__str, __fp, __para_count) do {  \
        __data[__str].argc_count = __para_count; \
        __data[__str].is_variable_arg = false;   \
        __data[__str].is_running_nolock = false; \
        __data[__str].do_act = __fp;             \
    } while (0)

#define ADD_CMD_VAR_ARG(__str, __fp, __less_para_count) do { \
        __data[__str].argc_count = __less_para_count;        \
        __data[__str].is_variable_arg = true;                \
        __data[__str].is_running_nolock = false;             \
        __data[__str].do_act = __fp;                         \
    } while (0)

#define ADD_CMD_NO_LOCK(__str, __fp, __para_count) do { \
        __data[__str].argc_count = __para_count;        \
        __data[__str].is_variable_arg = false;          \
        __data[__str].is_running_nolock = true;         \
        __data[__str].do_act = __fp;                    \
    } while (0)

#define ADD_CMD_VAR_ARG_NO_LOCK(__str, __fp, __less_para_count) do { \
        __data[__str].argc_count = __less_para_count;                \
        __data[__str].is_variable_arg = true;                        \
        __data[__str].is_running_nolock = true;                      \
        __data[__str].do_act = __fp;                                 \
    } while (0)

#define ADD_CMD_HELP(__str, __arg, __exp, ...) do {            \
        ss_cmd_help __helps(__str, __arg, __data[__str].help); \
        __helps.input_usage(__exp, ##__VA_ARGS__);             \
    } while (0)

#define FOR_EACH_STRINGS_DIV_BY_KEY(__in, __key, __dst)                                                                       \
    for(const char *__cur = (__in), *__next = strstr(__in, (__key));                                                          \
        (__cur != NULL && *__cur != '\0')? ((__next? __dst.assign(__cur, __next - __cur): __dst.assign(__cur)), true): false; \
        __next = __next? (__cur = __next + 1, strstr(__next + 1, __key)): __cur = NULL)

#define GET_PRIVATE_CLASS_OBJ(__class, __base)                        \
        dynamic_cast<__class *>((__base *)ss_cmd_base::get_private());

typedef int (*do_action_fp)(std::vector<std::string> &);
typedef void (*mod_init_func)(std::map<std::string, struct base_command_data> &);
struct ext_init_data
{
    std::map<std::string, struct base_command_data> *cmd_data_ext;
    void                                  *private_data;
    ext_init_data()
        : cmd_data_ext(NULL), private_data(NULL)
    {
    }
};
struct base_command_data
{
    unsigned int argc_count;
    bool is_variable_arg;
    bool is_running_nolock;
    do_action_fp do_act;
    std::string help;
    base_command_data ()
        : argc_count(0), is_variable_arg(false), is_running_nolock(false),
          do_act(NULL)
    {
    }
};

class ss_cmd_help
{
    public:
        explicit ss_cmd_help(const std::string &name, const std::string &para, std::string &store)
            : store_str(store)
        {
            std::stringstream ss;
            ss << std::dec << "\033[" << PRINT_MODE_HIGHTLIGHT << ';' << PRINT_COLOR_BLUE << 'm'
               << "SYNOPSIS" << std::endl;
            ss << std::dec << "    \033[" << PRINT_MODE_UNDERLINE << ';' << PRINT_COLOR_YELLOW << 'm'
               << name << "\033[0m";
            ss << std::dec << "\033[" << PRINT_MODE_HIGHTLIGHT << ';' << PRINT_COLOR_FUNCHSIN << 'm'
               <<  " " << para << "\033[0m" << std::endl;
            store_str = ss.str();
        }
        ~ss_cmd_help(){}
        template<typename... T>
        void input_usage(const std::string &exp, T &&... args)
        {
            std::stringstream ss;
            ss << std::dec << "\033[" << PRINT_MODE_HIGHTLIGHT << ';' << PRINT_COLOR_BLUE << 'm'
               << "DESCRIPTION" << std::endl;
            ss << std::dec << "\033[" << PRINT_MODE_HIGHTLIGHT << ';' << PRINT_COLOR_CYAN << 'm'
               << "    " << exp << "\033[0m\n";
            store_str.append(ss.str());
            travel_arg(std::forward<T>(args)...);
        }
        void process_arg(const std::string &&arg)
        {
            std::stringstream ss;
            ss << std::dec << "\033[" << PRINT_MODE_HIGHTLIGHT << ';' << std::dec << PRINT_COLOR_GREEN << 'm'
               << "    "<< arg<< "\033[0m\n";
            store_str.append(ss.str());
        }
        template<typename Arg, typename... T>
        void travel_arg(Arg &&arg, T &&... args)
        {
            process_arg(std::forward<Arg>(arg));
            travel_arg(std::forward<T>(args)...);
        }
        void travel_arg()
        {
            store_str.append("\n");
        }
    private:
        std::string &store_str;
};

class ss_cmd_base
{
   public:
        ss_cmd_base() {}
        virtual ~ss_cmd_base() {}
        static void regist_module(mod_init_func init_func, const char *mod_name)
        {
            mod_func[mod_name] =  init_func;
            init_func(cmd_data);
        }
        static mod_init_func get_module_func(const std::string &name)
        {
            auto it = mod_func.find(name);
            if (it == mod_func.end())
            {
                return NULL;
            }
            return it->second;
        }
        static const std::string &get_name() {return prog_name;}
        static const std::string &get_path() {return prog_path;}
        static const void set_path(const std::string &path) {prog_path = path;}
        static void set_ext_data(std::map<std::string, struct base_command_data> *ext, void *user)
        {
            ext_data.cmd_data_ext = ext;
            ext_data.private_data = user;
        }
        static void *get_private()
        {
            return ext_data.private_data;
        }
        static std::map<std::string, struct base_command_data> &get_data()
        {
            if (ext_data.cmd_data_ext)
            {
                return *ext_data.cmd_data_ext;
            }
            return cmd_data;
        }
        static void set_chip(unsigned int chip_id) {selected_chip_id = chip_id;}
        static unsigned int get_chip() {return selected_chip_id;};
    private:
        static std::map<std::string, struct base_command_data> cmd_data;
        static std::map<std::string, mod_init_func> mod_func;
        static unsigned int selected_chip_id;
        static std::string prog_name;
        static ext_init_data ext_data;
        static std::string prog_path;
};
class ss_cmd_preload
{
    public:
        explicit ss_cmd_preload(mod_init_func init_func, const char *mod_name)
        {
            ss_cmd_base::regist_module(init_func, mod_name);
        }
};
int run_cmd_trans_log(ss_result &result, const char *fmt, ...);
int run_cmd_trans_log(unsigned int chip_id, ss_result &result, const char *fmt, ...);
int run_cmd(unsigned int chip_id, const char *fmt, ...);
int run_cmd(const char *fmt, ...);
int run_cmd_trans_log_nolock(ss_result &result, const char *fmt, ...);
int run_cmd_trans_log_nolock(unsigned int chip_id, ss_result &result, const char *fmt, ...);
int run_cmd_nolock(unsigned int chip_id, const char *fmt, ...);
int run_cmd_nolock(const char *fmt, ...);

int setup_ui(int argc, char **argv);
long ss_cmd_atoi(const char *str);

#endif
