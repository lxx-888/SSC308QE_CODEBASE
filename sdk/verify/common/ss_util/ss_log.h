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
#ifndef __SS_LOG_H__
#define __SS_LOG_H__
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include "json.hpp"

#define PRINT_COLOR_END "\033[0m"
#define CODE_TRACE '[' << basename(__FILE__) << ']' << '[' << __FUNCTION__ << ']' << '[' << __LINE__ << ']'
#define COLOR_ENDL PRINT_COLOR_END << std::endl
#define EXPECT_OK(__func, __ret, __expval)  do { \
        ss_log_result result(#__func);           \
        int __ret_val = __func;                  \
        __ret |= __ret_val;                      \
        result.set_result(__ret_val, __expval);  \
    } while (0)

enum print_lv
{
    PRINT_LV_TRACE,
    PRINT_LV_DEBUG,
    PRINT_LV_WARN,
    PRINT_LV_ERROR,
    PRINT_LV_NUM
};

enum print_color
{
    PRINT_COLOR_NORMAL = 0,
    PRINT_COLOR_BLACK = 30,
    PRINT_COLOR_RED ,
    PRINT_COLOR_GREEN,
    PRINT_COLOR_YELLOW,
    PRINT_COLOR_BLUE,
    PRINT_COLOR_FUNCHSIN,
    PRINT_COLOR_CYAN,
    PRINT_COLOR_WHITE
};

enum print_mode
{
    PRINT_MODE_NORMAL = 0,
    PRINT_MODE_HIGHTLIGHT = 1,
    PRINT_MODE_UNDERLINE = 4,
    PRINT_MODE_FLICK = 5,
    PRINT_MODE_INVERT = 7,
};

enum log_flag
{
    PRINT_LOG_NONE = 0,
    PRINT_LOG_LOCAL,
    PRINT_LOG_REMOTE,
    PRINT_STORED_LOG_REMOTE
};

class ss_result
{
    public:
        explicit ss_result(nlohmann::json &json_result) : res(json_result) {}
        virtual ~ss_result() {}
        std::stringstream &out_log() {return log;}
        std::stringstream &out_md5() {return md5;}
        std::stringstream &out_ask() {return ask;}
        std::stringstream &out_tab() {return tab;}
        nlohmann::json &out_msg() {return res["c"];}
        void to_json(int ret)
        {
            sync_data();
            res["r"] = ret;
        }
        void to_json_str(int ret, std::stringstream &out_str)
        {
            sync_data();
            res["r"] = ret;
            out_str.str("");
            out_str << res.dump(0);
        }
    private:
        void sync_data(void)
        {
            res["m"] = md5.str();
            res["l"] = log.str();
            res["a"] = ask.str();
            res["t"] = tab.str();
        }
        std::stringstream log;
        std::stringstream md5;
        std::stringstream ask;
        std::stringstream tab;
        nlohmann::json &res;
};
class ss_log
{
    public:
        ss_log(enum log_flag flag = PRINT_STORED_LOG_REMOTE)
            : store_data(nullptr), log_mask(PRINT_LV_TRACE), log_onoff(true)
        {
            redirect_fd[0] = -1;
            redirect_fd[1] = -1;
            stdout_fd      = -1;
            self_pid       = getpid();
            set_flag(flag);
        }
        virtual ~ss_log()
        {
            if (redirect_fd[0] != -1)
                close(redirect_fd[0]);
            if (redirect_fd[1] != -1)
                close(redirect_fd[1]);
        }
        void write(const char *log, unsigned int len);
        void redirect();
        void restore();
        int pid(void);
        void set_flag(enum log_flag flag);
        bool get_flag(enum log_flag flag);
        void set_log(bool onoff);
        bool get_log(void);
        void set_data(ss_result *data);
        bool is_store();
        bool is_logon();
        std::stringstream &store_log();
        std::stringstream &store_md5();
        std::stringstream &store_ask(); //Ask client to do sth, such as 'pause', 'catch'.
        std::stringstream &store_tab();
        nlohmann::json &store_msg();
        ss_log &color_set(enum print_color color, enum print_mode mode);
        ss_log &lv_set(enum print_lv lv);
        void lv_mask(enum print_lv mask) {log_mask = mask;}
        void print(const char *format, ...);
        void print(enum print_lv lv, const char *format, ...);
        void print(enum print_color color, enum print_mode mode, const char *format, ...);
    private:
        ss_result *store_data;
        enum print_lv log_mask;
        bool log_onoff;
        enum log_flag store_flag;
        int redirect_fd[2];
        int stdout_fd;
        int self_pid;
};
extern ss_log sslog;
class ss_log_time
{
    public:
        ss_log_time();
        virtual ~ss_log_time();
        unsigned long get_time();
        unsigned long enter_time;
};
class ss_log_result : ss_log_time
{
    public:
        ss_log_result(const std::string &);
        ss_log_result(ss_log &, const std::string &);
        virtual ~ss_log_result();
        void set_result(int ret_val, int exp_val);
    private:
        ss_log &log_out;
        int result;
        int expectation;
        std::string case_name;
};
template <typename T>
static inline ss_log &operator<<(ss_log &log, T str)
{
    if (log.get_flag(PRINT_LOG_NONE) || !log.get_log())
        return log;
    if (log.get_flag(PRINT_STORED_LOG_REMOTE) && log.is_store())
        log.store_log() << str;
    else
        std::cout << str;

    return log;
}
static inline ss_log &operator<<(ss_log &log, std::ostream& (*os)(std::ostream&))
{
    if (log.get_flag(PRINT_LOG_NONE) || !log.get_log())
        return log;
    if (log.get_flag(PRINT_STORED_LOG_REMOTE) && log.is_store())
        log.store_log() << os;
    else
        std::cout << os;

    return log;
}
void ss_print(const char *format, ...);
void ss_print(enum print_lv lv, const char *format, ...);
void ss_print(enum print_color color, enum print_mode mode, const char *format, ...);
#endif //__SS_LOG_H__
