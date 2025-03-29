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
#include <stdarg.h>
#include <unistd.h>
#include "ss_log.h"

ss_log sslog;  //for global use.

const static char *lv_str[PRINT_LV_NUM] =
{
    "\033[1;32m", //Highlight + Green
    "\033[1;36m", //Highlight + Cyan
    "\033[1;33m", //Highlight + Yellow
    "\033[1;31m"  //Highlight + Red
};
static ss_log sslog_null(PRINT_LOG_NONE);
static inline void _ss_vprint(ss_log &log, const char *format, va_list args)
{
    char buffer[1024];

    vsnprintf(buffer, 1024, format, args);
    log << buffer;
}

void ss_log::write(const char *log, unsigned int len)
{
    if (store_data)
    {
        store_log().write(log, len);
        return;
    }
    std::cout.write(log, len);
}
bool ss_log::get_flag(enum log_flag flag)
{
    return store_flag == flag;
}
void ss_log::set_flag(enum log_flag flag)
{
    store_flag = flag;
    if (store_flag == PRINT_LOG_REMOTE)
    {
        if (pipe(redirect_fd) == -1)
        {
            redirect_fd[0] = -1;
            redirect_fd[1] = -1;
            perror("pipe");
            return;
        }
        // 16kb for buffering log.
        if (fcntl(redirect_fd[1], F_SETPIPE_SZ, 512 * 1024) == -1)
        {
            redirect_fd[0] = -1;
            redirect_fd[1] = -1;
            close(redirect_fd[0]);
            close(redirect_fd[1]);
            perror("fcntl");
            return;
        }
    }
}
void ss_log::set_log(bool onoff)
{
    log_onoff = onoff;
}
bool ss_log::get_log(void)
{
    return log_onoff;
}
int ss_log::pid(void)
{
    return self_pid;
}
void ss_log::set_data(ss_result *result)
{
    store_data = result;
}
void ss_log::redirect()
{
    if (store_flag != PRINT_LOG_REMOTE || !store_data)
    {
        return;
    }
    if (redirect_fd[1] == -1)
    {
        return;
    }
    stdout_fd = dup(STDOUT_FILENO);
    if (stdout_fd == -1)
    {
        perror("dup");
        return;
    }
    store_data->out_log() << "--log-tag-start-pid-" << self_pid << "--" << std::endl;
    if (dup2(redirect_fd[1], STDOUT_FILENO) == -1)
    {
        close(stdout_fd);
        perror("dup2");
        stdout_fd = -1;
        store_data->out_log().str("");
        return;
    }
}
void ss_log::restore()
{
    if (store_flag != PRINT_LOG_REMOTE || !store_data)
    {
        return;
    }
    if (redirect_fd[0] == -1)
    {
        return;
    }
    char data[128];
    int data_size = 0;
    std::string tag = "--log-tag-end--\n";
    std::cout << tag;
    fflush(stdout);
    if (dup2(stdout_fd, STDOUT_FILENO) == -1)
    {
        close(stdout_fd);
        perror("dup2");
        stdout_fd = -1;
        return;
    }
    data_size = read(redirect_fd[0], data, 128);
    while (data_size)
    {
        store_data->out_log().write(data, data_size);
        unsigned int size = store_data->out_log().str().size();
        if (size >= tag.size() && tag == (store_data->out_log().str().c_str() + size - tag.size()))
        {
            break;
        }
        data_size = read(redirect_fd[0], data, 128);
    }
    close(stdout_fd);
    stdout_fd = -1;
}
bool ss_log::is_store()
{
    return store_data ? true : false;
}
bool ss_log::is_logon()
{
    return log_onoff ? true : false;
}
std::stringstream &ss_log::store_log()
{
    return store_data->out_log();
}
std::stringstream &ss_log::store_md5()
{
    return store_data->out_md5();
}
std::stringstream &ss_log::store_ask()
{
    return store_data->out_ask();
}
std::stringstream &ss_log::store_tab()
{
    return store_data->out_tab();
}
nlohmann::json &ss_log::store_msg()
{
    return store_data->out_msg();
}
ss_log &ss_log::lv_set(enum print_lv lv)
{
    if (log_mask > lv)
    {
        return sslog_null;
    }
    *this << lv_str[lv];
    return *this;
}
ss_log &ss_log::color_set(enum print_color color, enum print_mode mode)
{
    *this << "\033[" << std::dec << mode << ';' << std::dec << color << 'm';
    return *this;
}
ss_log_time::ss_log_time()
{
    enter_time = get_time();
}
ss_log_time::~ss_log_time()
{
}
unsigned long ss_log_time::get_time()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000000) + (ts.tv_nsec / 1000);
}
ss_log_result::ss_log_result(const std::string &name)
    : log_out(sslog), result(0), expectation(0), case_name(name)
{
}
ss_log_result::ss_log_result(ss_log &out, const std::string &name)
    : log_out(out), result(0), expectation(0), case_name(name)
{
}
ss_log_result::~ss_log_result()
{
    if (result == expectation)
    {
        log_out.lv_set(PRINT_LV_DEBUG) << case_name << " : Test ok. " << PRINT_COLOR_END;
    }
    else
    {
        log_out.lv_set(PRINT_LV_ERROR) << case_name << " : Test fail. " << PRINT_COLOR_END;
    }
    log_out.color_set(PRINT_COLOR_BLUE, PRINT_MODE_HIGHTLIGHT) << "RET:0x" << std::hex << result
          << " T: " << std::dec << get_time() - enter_time << "us" << std::endl << PRINT_COLOR_END;
}
void ss_log_result::set_result(int ret_val, int exp_val)
{
    result = ret_val;
    expectation = exp_val;
}
void ss_log::print(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    _ss_vprint(*this, format, args);
    va_end(args);
}
void ss_log::print(enum print_lv lv, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    lv_set(lv);
    _ss_vprint(*this, format, args);
    va_end(args);
    *this << PRINT_COLOR_END;
}
void ss_log::print(enum print_color color, enum print_mode mode, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    color_set(color, mode);
    _ss_vprint(*this, format, args);
    va_end(args);
    *this << PRINT_COLOR_END;
}
void ss_print(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    _ss_vprint(sslog, format, args);
    va_end(args);
}
void ss_print(enum print_lv lv, const char *format, ...)
{
    va_list args;

    sslog.lv_set(lv);
    va_start(args, format);
    _ss_vprint(sslog, format, args);
    va_end(args);
    sslog << PRINT_COLOR_END;
}
void ss_print(enum print_color color, enum print_mode mode, const char *format, ...)
{
    va_list args;

    if (color == PRINT_COLOR_NORMAL)
    {
        va_start(args, format);
        _ss_vprint(sslog, format, args);
        va_end(args);
        return;
    }
    sslog.color_set(color, mode);
    va_start(args, format);
    _ss_vprint(sslog, format, args);
    va_end(args);
    sslog << PRINT_COLOR_END;
}
