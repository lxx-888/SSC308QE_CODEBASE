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
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>

#include <signal.h>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <list>

#include "json.hpp"
#include "ss_console.h"
#include "lua.hpp"

typedef enum
{
    EN_KEY_CHAR,
    EN_KEY_ENTER,
    EN_KEY_BACKSPACE,
    EN_KEY_UP,
    EN_KEY_DOWN,
    EN_KEY_LEFT,
    EN_KEY_RIGHT,
    EN_KEY_DEL,
    EN_KEY_TAB,
    EN_KEY_NOT_SUPPORT
}EN_KEY;
#define MOVELEFT(y)                  \
    do                               \
    {                                \
        if ((y) != 0)                \
        {                            \
            printf("\033[%dD", (y)); \
        }                            \
    } while (0)
#define MOVERIGHT(y)                 \
    do                               \
    {                                \
        if ((y) != 0)                \
        {                            \
            printf("\033[%dC", (y)); \
        }                            \
    } while (0)
#define PRINT_GAP(x, y)              \
    do                               \
    {                                \
        unsigned short int i = 0;    \
        for (; i < x; i++)           \
        {                            \
            printf(y);               \
        }                            \
    } while(0)
#define CLEARLINE printf("\033[K")
#define PRINT_POINT(__p) ((__p) ? (__p) : (#__p))
#define SS_CMD_EXE "sscmd_exe"
#define SS_CMD_RET "sscmd_ret"
#define SS_CONSOLE_TRANS "ss_console_trans"
#define SS_CONSOLE_TRANS_FD "ss_console_trans_fd"
#define SS_CONSOLE_DEFAULT_CASE "ss_console_default_case"

struct console_status
{
    char *line_buffer;
    unsigned int line_size;
    unsigned char data_idx;
    unsigned char cursor_idx;
};

static struct termios gstOrgOpts;
static struct console_status *p_current_console = NULL;

static inline unsigned long get_timer()
{
    struct timespec ts;
    unsigned long ms;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    ms = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
    if(ms == 0)
    {
        ms = 1;
    }
    return ms;
}

static inline long ex_atoi(const char *str)
{
    if (!str)
    {
        return -1;
    }

    if (strlen(str) >= 3
        && str[0] == '0'
        && (str[1] == 'x' || str[1] == 'X'))
    {
        long ret = 0;
        sscanf(str, "%lx", &ret);
        return ret;
    }
    return atol(str);
}

static int init_console(const char *pInitStr)
{
    if (pInitStr == NULL)
    {
        return -1;
    }
    printf("[%s]:", pInitStr);
    fflush(stdin);

    return 0;
}
static EN_KEY get_key(char *pChar)
{
    char pGetStr[4] = {0};
    EN_KEY enKey = EN_KEY_NOT_SUPPORT;
    unsigned int intOffset = 0;

    memset(pGetStr, 0, sizeof(pGetStr));
    while(1)
    {
        if (read(0, pGetStr + intOffset, 1) == 1)
        {
            if (pGetStr[0] == 27)
            {
                if (pGetStr[1] == 91 && pGetStr[2] == 65) //up
                {
                    enKey = EN_KEY_UP;
                    break;
                }
                if (pGetStr[1] == 91 && pGetStr[2] == 66) //down
                {
                    enKey = EN_KEY_DOWN;
                    break;
                }
                if (pGetStr[1] == 91 && pGetStr[2] == 68) //left
                {
                    enKey = EN_KEY_LEFT;
                    break;
                }
                if (pGetStr[1] == 91 && pGetStr[2] == 67) //right
                {
                    enKey = EN_KEY_RIGHT;
                    break;
                }
                if (pGetStr[1] == 91 && pGetStr[2] == 51 && pGetStr[3] == 126) //del
                {
                    enKey = EN_KEY_DEL;
                    break;
                } //I need fill all the keyboard keycode so that key will not lost.
                intOffset++;
                if (intOffset >= sizeof(pGetStr))
                {
                    intOffset = 0;
                    memset(pGetStr, 0, sizeof(pGetStr));
                }
                else
                {
                    continue; //Continue get key.
                }
            }
            else if (pGetStr[0] >= 32 && pGetStr[0] <= 126)
            {
                enKey = EN_KEY_CHAR;
                if (pChar)
                {
                    *pChar = pGetStr[0];
                }
                break;
            }
            else if (pGetStr[0] == '\n')
            {
                enKey = EN_KEY_ENTER;
                break;
            }
            else if (pGetStr[0] == 8 || pGetStr[0] == 127)
            {
                enKey = EN_KEY_BACKSPACE;
                break;
            }
            else if (pGetStr[0] == 9)
            {
                enKey = EN_KEY_TAB;
                break;
            }
        }
    }

    return enKey;
}
static void find_tab_strings_idx(const char* pExistStr, unsigned int size,
                                 std::vector<unsigned int> &showIdx, std::vector<std::string> &strTabStrings)
{
    unsigned int idx = 0;
    std::vector<std::string>::iterator it = strTabStrings.begin();

    for (; it != strTabStrings.end(); ++it)
    {
        if (pExistStr == NULL || size == 0)
        {
            showIdx.push_back(idx);
        }
        else if (0 == strncmp(pExistStr, it->c_str(), size))
        {
            showIdx.push_back(idx);
        }
        idx++;
    }
    return;
}
static void find_match_strings(std::vector<std::string> &strStrings, std::string &strMatch)
{
    std::vector<std::string>::iterator it = strStrings.begin();
    unsigned short i = 0, bAbort = 0, minStrSize = it->length();
    const char *pStr = NULL;
    char cChar = 0;

    if (strStrings.size() == 1)
    {
        strMatch = strStrings[0];
        return;
    }
    for (; it != strStrings.end(); ++it)
    {
        if (minStrSize > it->size())
        {
            minStrSize = it->size();
        }
    }
    for (i = 0; i < minStrSize; i++)
    {
        it = strStrings.begin();
        pStr = it->c_str();
        cChar =  pStr[i];
        ++it;
        for (; it != strStrings.end(); ++it)
        {
            pStr = it->c_str();
            if (cChar != pStr[i])
            {
                bAbort = 1;
                break;
            }
            cChar =  pStr[i];
        }
        if (bAbort == 1)
        {
            break;
        }
    }
    if (i != 0)
    {
        strMatch.assign(pStr, i);
    }
}
static std::vector<unsigned int> auto_adjust_tab(const char* pExistStr, unsigned int size,
                                                 std::string &strAdjust, std::vector<std::string> &strTabStrings)
{
    unsigned int intMatchSize = 0;
    std::vector<unsigned int> intIdx;
    std::vector<unsigned int>::iterator it = intIdx.begin();

    find_tab_strings_idx(pExistStr, size, intIdx, strTabStrings);
    intMatchSize = intIdx.size();
    if (intMatchSize == 1)
    {
        if (size < strTabStrings[intIdx[0]].size())
        {
            strAdjust.assign(strTabStrings[intIdx[0]].c_str() + size);
        }
    }
    else if (intMatchSize > 1)
    {
        for (it = intIdx.begin(); it != intIdx.end(); ++it)
        {
            if (strTabStrings[*it].size() <= size)
            {
                break;
            }
        }
        if (it == intIdx.end()) //All find strings size is larger than pExistStr.
        {
            std::vector<std::string> strTmpStrings;
            std::string strTmp;

            it = intIdx.begin();
            for (; it != intIdx.end(); ++it)
            {
                strTmp.assign(strTabStrings[*it].c_str() + size);
                strTmpStrings.push_back(strTmp);
            }
            find_match_strings(strTmpStrings, strAdjust);
        }
    }

    return intIdx;
}
static int show_tab_strings(std::vector<unsigned int> showIdx, std::vector<std::string> &strTabStrings)
{
    if (!showIdx.size())
    {
        return -1;
    }

    std::vector<unsigned int>::iterator it = showIdx.begin();

    printf("\n");
    for (;it != showIdx.end(); ++it)
    {
        printf("%s", strTabStrings[*it].c_str());
        PRINT_GAP(3, " ");
    }
    printf("\n");
    return -1;
}
static FILE *script_fp = NULL;
static int get_cmd(char *pGetBuf, struct console_status &current_console,
                   std::vector<std::string> &strTabStrings, std::vector<std::string> &strCmdStrings, unsigned int &intCmdIdx)
{
    char get_char = 0;
    struct termios new_opts;
    int res=0;
    std::string str;
    EN_KEY enKey = EN_KEY_NOT_SUPPORT;
    unsigned short bExitGetCmd = 0;
    unsigned short uTabCount = 0;
    char *script_str = NULL;
    char *script_str_end = NULL;

    current_console.cursor_idx = 0;
    current_console.data_idx = 0;
    fflush(stdout);
    if (script_fp != NULL)
    {
        script_str = current_console.line_buffer;
        if (script_str == fgets(script_str, current_console.line_size, script_fp))
        {
            if (*script_str == '\n' || *script_str == '\r' || *script_str == '#')
            {
                return 0;
            }
            script_str_end = strstr(script_str, ";");
            if (script_str_end == NULL)
            {
                printf("\033[1;31m");
                printf("Script error not parse \';\'");
                printf("\033[0m\n");
                fclose(script_fp);
                script_fp = NULL;

                return 0;
            }
            *script_str_end = 0;

            printf("\033[1;36m");
            printf("[LOAD SCRIPT]%s", script_str);
            printf("\033[0m\n");

            return script_str_end - script_str;
        }
        else
        {
            printf("close file!\n");
            fclose(script_fp);
            script_fp = NULL;

            return 0;
        }
    }
    memcpy(&new_opts, &gstOrgOpts, sizeof(new_opts));
    new_opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);
    res = tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
    if (res == -1)
    {
        assert(0);
    }
    while (bExitGetCmd == 0)
    {
        get_char = 0;
        enKey = get_key(&get_char);
        //printf("key %d\n", enKey);
        //continue;
        if (enKey == EN_KEY_TAB)
        {
            if (uTabCount < 2)
            {
                uTabCount++;
            }
        }
        else
        {
            uTabCount = 0;
        }
        switch (enKey)
        {
            case EN_KEY_TAB:
            {
                if (!strTabStrings.size())
                {
                    break;
                }
                std::vector<unsigned int> showIdx;
                std::string strAdjust;

                /*
                   sample1_cmd sample2_cmd samplen_cmd

                   rev = auto_adjust_tab(x, y, str)

                   rev :0,str: empty :
                        Cmd : empty, Input: nothing ->tab
                   rev :1,str: empty :
                        Cmd : sample1_cmd, Input: sample1_cmd ->tab
                   rev :n,str: empty :
                        Cmd : sample1_cmd/sample2_cmd/...samplen_cmd, Input: nothing ->tab
                   rev :1,str: not empty :
                        Cmd : sample1_cmd/sample2_cmd/...samplen_cmd, Input: sample1 ->tab
                        Cmd : sample1_cmd, Input: sample1 ->tab
                   rev :n,str: not empty :
                        Cmd : sample1_cmd/sample2_cmd/...samplen_cmd, Input: sample ->tab
                */
                showIdx = auto_adjust_tab(current_console.line_buffer, current_console.cursor_idx, strAdjust, strTabStrings);
                if (uTabCount == 1)//first tab
                {
                    std::string tmpStr;
                    if (showIdx.size() > 1)
                    {
                        if (current_console.data_idx + strAdjust.size() < current_console.line_size && strAdjust.size()) //protect buffer overwrite.
                        {
                            if (current_console.cursor_idx < current_console.data_idx)
                            {
                                tmpStr.assign(&current_console.line_buffer[current_console.cursor_idx], current_console.data_idx - current_console.cursor_idx);
                            }
                            strcpy(&current_console.line_buffer[current_console.cursor_idx], strAdjust.c_str());//Auto adjust
                            printf("%s", strAdjust.c_str());
                            current_console.cursor_idx += strAdjust.size();
                            current_console.data_idx += strAdjust.size();
                            if (tmpStr.size())
                            {
                                strcpy(&current_console.line_buffer[current_console.cursor_idx], tmpStr.c_str());
                                printf("%s", tmpStr.c_str());
                                MOVELEFT((int)tmpStr.size());
                            }
                            uTabCount = 0;
                        }
                    }
                    else if (showIdx.size() == 1) //if find only one
                    {
                        if (current_console.data_idx + 1 + strAdjust.size() < current_console.line_size) //protect buffer overwrite.
                        {
                            if (current_console.cursor_idx < current_console.data_idx)
                            {
                                tmpStr.assign(&current_console.line_buffer[current_console.cursor_idx], current_console.data_idx - current_console.cursor_idx);
                            }
                            if (strAdjust.size())
                            {
                                snprintf(&current_console.line_buffer[current_console.cursor_idx], current_console.line_size - current_console.cursor_idx, "%s ", strAdjust.c_str()); //Auto adjust
                                current_console.cursor_idx += strAdjust.size() + 1;
                                current_console.data_idx += strAdjust.size() + 1;
                                printf("%s ", strAdjust.c_str());
                            }
                            if (tmpStr.size())
                            {
                                snprintf(&current_console.line_buffer[current_console.cursor_idx], current_console.line_size - current_console.cursor_idx,"%s",  tmpStr.c_str());
                                printf("%s", tmpStr.c_str());
                                MOVELEFT((int)tmpStr.size());
                            }
                            uTabCount = 0;
                        }
                    }
                }
                else if (uTabCount == 2)
                {
                    if (showIdx.size())
                    {
                        show_tab_strings(showIdx, strTabStrings);//show all cmd.
                        init_console("sscmd");
                        printf("%s", current_console.line_buffer);
                        MOVELEFT(current_console.data_idx - current_console.cursor_idx);
                    }
                }
            }
            break;
            case EN_KEY_ENTER:
            {
                if (current_console.data_idx > 0) //get strings
                {
                    str.assign(current_console.line_buffer, current_console.data_idx);
                    strCmdStrings.push_back(str);
                    intCmdIdx = strCmdStrings.size();
                }
                printf("\n");
                bExitGetCmd = 1;
            }
            break;
            case EN_KEY_BACKSPACE:
            {
                if (current_console.cursor_idx > 0)
                {
                    std::string tmpStr;
                    tmpStr = &current_console.line_buffer[current_console.cursor_idx];
                    current_console.line_buffer[current_console.cursor_idx - 1] = 0;
                    snprintf(&current_console.line_buffer[current_console.cursor_idx - 1], current_console.line_size - current_console.cursor_idx + 1, "%s", tmpStr.c_str());
                    current_console.cursor_idx--;
                    current_console.data_idx--;
                    MOVELEFT(1);
                    CLEARLINE;
                    printf("%s", tmpStr.c_str());
                    MOVELEFT(current_console.data_idx - current_console.cursor_idx);
                    intCmdIdx = strCmdStrings.size();
                }
            }
            break;
            case EN_KEY_CHAR:
            {
                if (current_console.data_idx < current_console.line_size - 1) //skip current_console.line_buffer[current_console.line_size - 1] = 0
                {
                    std::string tmpStr;
                    tmpStr = get_char;
                    tmpStr += &current_console.line_buffer[current_console.cursor_idx];
                    snprintf(&current_console.line_buffer[current_console.cursor_idx], current_console.line_size - current_console.cursor_idx, "%s", tmpStr.c_str());
                    printf("%s", &current_console.line_buffer[current_console.cursor_idx]);
                    current_console.data_idx++;
                    current_console.cursor_idx++;
                    MOVELEFT(current_console.data_idx - current_console.cursor_idx);
                    intCmdIdx = strCmdStrings.size();
                }
            }
            break;
            case EN_KEY_UP:
            {
                if (strCmdStrings.size() && intCmdIdx > 0)
                {
                    memset(current_console.line_buffer, 0, sizeof(char) * current_console.line_size);
                    MOVELEFT(current_console.cursor_idx);
                    CLEARLINE;
                    strcpy(current_console.line_buffer, strCmdStrings[intCmdIdx - 1].c_str());
                    current_console.cursor_idx = current_console.data_idx = strCmdStrings[intCmdIdx - 1].length();
                    printf("%s", current_console.line_buffer);
                    intCmdIdx--;
                }
            }
            break;
            case EN_KEY_DOWN:
            {
                if (strCmdStrings.size() && intCmdIdx < strCmdStrings.size() -  1)
                {
                    memset(current_console.line_buffer, 0, sizeof(char) * current_console.line_size);
                    MOVELEFT(current_console.cursor_idx);
                    CLEARLINE;
                    strcpy(current_console.line_buffer, strCmdStrings[intCmdIdx + 1].c_str());
                    current_console.cursor_idx = current_console.data_idx = strCmdStrings[intCmdIdx + 1].length();
                    printf("%s", current_console.line_buffer);
                    intCmdIdx++;
                }
            }
            break;
            case EN_KEY_LEFT:
            {
                if (current_console.cursor_idx > 0)
                {
                    MOVELEFT(1);
                    current_console.cursor_idx--;
                }
            }
            break;
            case EN_KEY_RIGHT:
            {
                if (current_console.cursor_idx < current_console.data_idx)
                {
                    MOVERIGHT(1);
                    current_console.cursor_idx++;
                }
            }
            break;
            case EN_KEY_DEL:
            {
                if (current_console.data_idx > current_console.cursor_idx)
                {
                    std::string tmpStr;
                    tmpStr = &current_console.line_buffer[current_console.cursor_idx + 1];
                    snprintf(&current_console.line_buffer[current_console.cursor_idx], current_console.line_size - current_console.cursor_idx, "%s", tmpStr.c_str());
                    current_console.data_idx--;
                    CLEARLINE;
                    printf("%s", &current_console.line_buffer[current_console.cursor_idx]);
                    MOVELEFT(current_console.data_idx - current_console.cursor_idx);
                    intCmdIdx = strCmdStrings.size() - 1;
                }
            }
            break;
            default:
                break;
        }
        // setvbuf will not be activated after any operation of io stream in uclibc, so it should do fflush stdout in here.
        fflush(stdout);
    }
    res = tcsetattr(STDIN_FILENO, TCSANOW, &gstOrgOpts);
    if (res == -1)
    {
        assert(0);
    }
    return current_console.data_idx;
}

static int use_script(const char *file)
{
    script_fp = fopen(file, "r");
    if (script_fp == NULL)
    {
        printf("Open file error!\n");
        perror("fopen");
        return -1;
    }

    return 0;
}
static int parse_tab_strings(const char *pStr, std::vector<std::string> &strTabStrings)
{
    if (pStr == NULL)
    {
        return -1;
    }

    const char *posFront = NULL;
    const char *posBack = NULL;
    std::string tmpString;

    strTabStrings.clear();
    posFront = pStr;
    posBack = strstr(posFront, "/");

    while (posBack != NULL && posFront != posBack)
    {
        tmpString.assign(posFront, (posBack - posFront));
        strTabStrings.push_back(tmpString);
        posFront = posBack + 1;
        posBack = strstr(posFront, "/");
    }
    return 0;
}
static void do_signal(int)
{
    printf("\n");
    init_console("sscmd");
    if (p_current_console && p_current_console->line_buffer)
    {
        p_current_console->line_buffer[0] = '\0';
        p_current_console->data_idx = 0;
        p_current_console->cursor_idx = 0;
    }
}
static void do_exit(void)
{
    if (tcsetattr(STDIN_FILENO, TCSANOW, &gstOrgOpts) == -1)
    {
        perror("tcsetattr");
    }
}
static inline void parse_str(const char *str, std::vector<std::string> &in_strs)
{
    const char *str1 = NULL;
    const char *str2 = NULL;
    int size = 0;
    std::string str_tmp;

    if (str == NULL)
    {
        return;
    }
    str1 = str;
    while (1)
    {
        while (*str1 == ' ' && *str1 != 0)
        {
            str1++;
        }
        str2 = str1;
        while (*str2 != ' ' && *str2 != 0)
        {
            str2++;
        }
        size = str2 - str1;
        if (size != 0)
        {
            str_tmp.assign(str1, size);
            in_strs.push_back(str_tmp);
            str1 = str2;
        }
        else
        {
            break;
        }
    }
}
static inline void parse_cmd_para(const char *str, std::string &cmd, std::string &para)
{
    const char *str1 = NULL;
    const char *str2 = NULL;
    int size = 0;

    if (str == NULL)
    {
        return;
    }
    str1 = str;
    while (*str1 == ' ' && *str1 != 0)
    {
        str1++;
    }
    str2 = str1;
    while (*str2 != ' ' && *str2 != 0)
    {
        str2++;
    }
    size = str2 - str1;
    if (size != 0)
    {
        cmd.assign(str1, size);
        para = str2;
    }
}
static inline void setup_current_console(struct console_status &current_console, char *buf, unsigned int size)
{
    current_console.line_buffer = buf;
    current_console.line_size = size;
    current_console.cursor_idx = 0;
    current_console.data_idx = 0;
}
static inline int enter_console(long trans_fd, const struct ss_console_attr_s *attr)
{
    int get_size = 0;
    std::vector<std::string> in_strs;
    struct console_status current_console;
    std::vector<std::string> strCmdStrings;
    std::vector<std::string> strTabStrings;
    std::string tab_str;
    unsigned int intCmdIdx = 0;
    int ret = 0;
    char line_buffer[128];

    if (!attr)
    {
        return -1;
    }
    setup_current_console(current_console, line_buffer, 128);
    if (tcgetattr(STDIN_FILENO, &gstOrgOpts) == -1)
    {
        perror("tcgetattr");
        return -1;
    }
    if (signal(SIGINT, do_signal) == SIG_ERR)
    {
        perror("signal");
        return -1;
    }
    if (signal(SIGTSTP, do_signal) == SIG_ERR)
    {
        perror("signal");
        return -1;
    }
    p_current_console = &current_console;
    attr->grab_tab_list(trans_fd, tab_str);
    parse_tab_strings(tab_str.c_str(), strTabStrings);
    while (1)
    {
        init_console("sscmd");
        line_buffer[0] = 0;
        get_size = get_cmd(line_buffer, current_console, strTabStrings, strCmdStrings, intCmdIdx);
        line_buffer[get_size] = 0;
        in_strs.clear();
        parse_str(line_buffer, in_strs);
        if (in_strs.size() == 2
            && in_strs[0] == "s")
        {
            use_script(in_strs[1].c_str());
            continue;
        }
        if (in_strs.size() == 1
            && in_strs[0] == "q")
        {
            break;
        }
        if (in_strs.size())
        {
            nlohmann::json json_result;
            std::string out_log;
            ret = attr->send_cmd(trans_fd, line_buffer, get_size + 1, json_result);
            if (ret == 0)
            {
                ret = json_result["r"];
                out_log = json_result["l"];
                tab_str = json_result["t"];
                std::cout << out_log;
            }
            std::cout << "CMD[" << line_buffer << "] RET: " << ret << std::endl;
            if (tab_str.size())
            {
                //Replace current tab table after console init.
                parse_tab_strings(tab_str.c_str(), strTabStrings);
            }
        }
    }
    setup_current_console(current_console, NULL, 0);
    p_current_console = NULL;
    if (signal(SIGINT, SIG_DFL) == SIG_ERR)
    {
        perror("signal");
        return -1;
    }
    if (signal(SIGTSTP, SIG_DFL) == SIG_ERR)
    {
        perror("signal");
        return -1;
    }

    return ret;
}
int ss_console(struct ss_console_attr_s *attr, const char *key)
{
    int ret = 0;
    if (!key)
    {
        return -1;
    }
    long trans_fd = attr->trans_init(key);
    if (trans_fd == -1)
    {
        std::cout << "Transfer init err" << std::endl;
        return -1;
    }
    if (atexit(do_exit) != 0)
    {
        perror("atexit");
        return -1;
    }
    ret = enter_console(trans_fd, attr);
    attr->trans_deinit(trans_fd);
    return ret;
}
class string_lines
{
    public:
        explicit string_lines() {}
        virtual ~string_lines() {}
        bool get_line(std::string &in_str, size_t in_pos, std::string &out_str)
        {
            size_t find_pos = 0;
            size_t in_size = in_str.size();

            if (!in_size || (in_size == in_pos))
            {
                return false;
            }
            out_str.clear();
            find_pos = in_str.find_first_of('\n', in_pos);
            if (find_pos == std::string::npos)
            {
                store_str.append(in_str, in_pos, in_str.size() - in_pos);
                return false;
            }
            if (store_str.size())
            {
                out_str = store_str;
                store_str.clear();
            }
            if (find_pos - in_pos)
            {
                out_str.append(in_str, in_pos, find_pos - in_pos);
            }
            return true;
        }
        void end_line(std::string &out_str)
        {
            out_str = store_str;
        }
    private:
        std::string store_str;
};
class log_filter
{
    public:
        explicit log_filter(std::string &in): in_str(in), in_pos(0)
        {
            clear_color();
        }
        virtual ~log_filter() {}
        void pos_zero()
        {
            in_pos = 0;
        }
        bool getlines(std::string &out_str)
        {
            bool ret = false;

            ret = do_lines.get_line(in_str, in_pos, out_str);
            if (ret)
            {
                in_pos += out_str.size() + 1;
            }
            else
            {
                do_lines.end_line(out_str);
                in_pos = 0;
            }
            return ret;
        }
    private:
        void clear_color()
        {
            size_t pos = 0;
            size_t color_pos_start = 0;
            while(1)
            {
                color_pos_start = in_str.find_first_of('\u001b', color_pos_start);
                if (color_pos_start != std::string::npos)
                {
                    pos = in_str.find_first_of('m', color_pos_start);
                    if (pos != std::string::npos)
                    {
                        in_str.erase(color_pos_start, pos - color_pos_start + 1);
                        continue;
                    }
                }
                break;
            }
        }
        std::string &in_str;
        size_t in_pos;
        string_lines do_lines;
};
static void output_cmd_logs(nlohmann::json &json_out, std::string &logs)
{
    std::string log_line;
    bool ret = false;
    log_filter fliter(logs);
    do
    {
        ret = fliter.getlines(log_line);
        if (log_line.size())
            json_out.push_back(log_line);
    } while(ret);
}
static inline int get_random(int start, int end)
{
    struct timeval val;
    struct timezone zone;

    gettimeofday(&val, &zone);
    srand((int)(val.tv_usec / 1000));
    return ((rand()%(end - start + 1)) + start);
}
static inline bool do_pause(long trans_fd, const struct ss_console_attr_s *attr)
{
    while (1)
    {
        std::string reply;
        std::cout << "[y/n/c]:" << std::endl;
        std::cin >> reply;
        if (reply == "y")
        {
            getchar();
            return true;
        }
        if (reply == "n")
        {
            getchar();
            return false;
        }
        if (reply == "c")
        {
            enter_console(trans_fd, attr);
        }
    }
    return true;
}
static inline int script_out_json(nlohmann::json &json_data,
                                  nlohmann::json &json_result,
                                  const char *cmd_and_param,
                                  int log_opt)
{
    std::string cmd, para, logs;
    int cmd_ret = json_result["r"];
    nlohmann::json json_cmd_data = "{\"in\": \"\", \"logs\": null, \"md5\": null, \"ret\": null}"_json;

    logs = json_result["l"];
    std::cout << logs;
    parse_cmd_para(cmd_and_param, cmd, para);
    json_cmd_data["in"] = para;
    if ((log_opt & 0xFF) == 2 || ((log_opt & 0xFF) == 1 && cmd_ret == -1))
    {
        output_cmd_logs(json_cmd_data["logs"], logs);
    }
    json_cmd_data["md5"] = json_result["m"];
    json_cmd_data["ret"] = cmd_ret;
    if (json_data["ss_cmds"].size()
        && json_data["ss_cmds"].back().begin().key() == cmd)
    {
        json_data["ss_cmds"].back().back().push_back(json_cmd_data);
    }
    else
    {
        nlohmann::json json_cmd;
        json_cmd[cmd].push_back(json_cmd_data);
        json_data["ss_cmds"].push_back(json_cmd);
    }
    return cmd_ret;
}
static inline bool check_pipe_return()
{
    int status;
    char child_ret = 0;
    wait(&status);
    if (WIFEXITED(status))
    {
        child_ret = WEXITSTATUS(status);
    }
    else
    {
        std::cout << "CHILD EXIT!!!" << std::endl;
        return false;
    }
    return child_ret ? false : true;
}

static inline int open_pipe(int pipe_fd[2], std::vector<std::string> &in_strs)
{
    int process_pid = 0;
    int pipe_fd_read[2];
    int pipe_fd_write[2];

    if (in_strs.size() < 2)
    {
        std::cout << "Argv count error!" << std::endl;
        return -1;
    }
    if (pipe(pipe_fd_read) < 0)
    {
        perror("pipe");
        return -1;
    }
    if (pipe(pipe_fd_write) < 0)
    {
        perror("pipe");
        close(pipe_fd_read[0]);
        close(pipe_fd_read[1]);
        return -1;
    }
    const char **argv = new const char *[in_strs.size() + 1];
    if (!argv)
    {
        std::cout << "alloc error!" << std::endl;
        close(pipe_fd_write[0]);
        close(pipe_fd_write[1]);
        close(pipe_fd_read[0]);
        close(pipe_fd_read[1]);
        return -1;
    }
    /*
     *  We must use vfork to avoid deadlock issue, because the parent process may have mulitiple thread,
     *  and once one of the thread hold a thread atomic flag in libc and child process was created by the time,
     *  the atomic flag will never be clear in child, and then the child process happends deadlock if it accesses
     *  the thread lock.
     */
    process_pid = vfork();
    switch (process_pid)
    {
        case -1:
            perror("error in fork!");
            delete[] argv;
            close(pipe_fd_write[0]);
            close(pipe_fd_write[1]);
            close(pipe_fd_read[0]);
            close(pipe_fd_read[1]);
            return -1;
        case 0:
        {
            unsigned int i = 0;
            char fd_env[10];
            /*
             * ReadPipe: MainP0 <-- SubP1
             * WritePipe: MainP1 <-- SubP0
             *
             * Close:
             *  ReadPipe : SubP0
             *  WritePipe: SubP1
             */
            for (i = 0; i < in_strs.size(); i++)
            {
                argv[i] = in_strs[i].c_str();
            }
            argv[i] = NULL;
            close(pipe_fd_read[0]);
            close(pipe_fd_write[1]);
            snprintf(fd_env, 10, "%d", pipe_fd_read[1]);
            setenv(SS_CMD_EXE, fd_env, 1);
            snprintf(fd_env, 10, "%d", pipe_fd_write[0]);
            setenv(SS_CMD_RET, fd_env, 1);
            if(execvp(argv[0], (char *const *)argv) < 0)
            {
                perror("execlp error!");
                _exit(0);
            }
            _exit(0);
        }
        break;
        default:
        {
            delete[] argv;
            close(pipe_fd_read[1]);
            close(pipe_fd_write[0]);
            pipe_fd[0] = pipe_fd_read[0];
            pipe_fd[1] = pipe_fd_write[1];
            return process_pid;
        }
    }
}

static inline void adapt_cmd_of_sys_out_md5_auto(std::string &cmd_line, const char *out_path, std::string dst_md5)
{
    if (cmd_line.find("sys_out_md5_auto") >= cmd_line.length())
    {
        // Not found
        return;
    }
    int str_len = strlen(out_path) + 1;
    char* path_no_suffix = new char [str_len];
    assert(path_no_suffix);
    memset(path_no_suffix, 0, str_len);
    strcpy(path_no_suffix, out_path);
    const char* find_ptr = strstr(out_path, ".json");
    if (find_ptr)
    {
        memset(path_no_suffix, 0, str_len);
        memcpy(path_no_suffix, out_path, find_ptr - out_path);
    }
    std::string temp_line(cmd_line);
    temp_line.append(" ");
    cmd_line.clear();
    int begin = 0;
    int argc = 0;
    for (unsigned int i = 0; i < temp_line.size(); i++)
    {
        if (temp_line[i] == ' ')
        {
            argc++;
            int len = i - begin;
            std::string argv = temp_line.substr(begin, len);
            if (argc == 6)
            {
                bool is_write = (bool)atoi(argv.c_str());
                is_write = !dst_md5.size() ? is_write : 0;
                cmd_line.append(is_write ? "1" : "0");
                break;
            }
            cmd_line.append(argv);
            cmd_line.append(" ");
            begin = i + 1;
        }
    }
    cmd_line.append(" ");
    cmd_line.append(path_no_suffix);
    if (dst_md5.size())
    {
        cmd_line.append(" ");
        cmd_line.append(dst_md5);
    }
    delete[] path_no_suffix;
    //std::cout <<"adapt cmdline: " << cmd_line << std::endl;
}

static inline int do_command_string_transfer_json(long trans_fd, const struct ss_console_attr_s *attr,
                                                  std::string &out_str, nlohmann::json &json_result,
                                                  nlohmann::json &json_data, const struct ss_console_json_out_s *console_json_out)
{
    int log_opt = console_json_out->log_opt;
    int cmd_ret = 0;
    adapt_cmd_of_sys_out_md5_auto(out_str, console_json_out->out_file, "");

    std::cout << "RUN: " << out_str << std::endl;
    cmd_ret = attr->send_cmd(trans_fd, out_str.c_str(), out_str.size() + 1, json_result);
    if (cmd_ret == 0)
    {
        cmd_ret = script_out_json(json_data, json_result, out_str.c_str(), log_opt);
        std::string ask_for = json_result["a"];
        if (ask_for == "pause")
        {
            cmd_ret = do_pause(trans_fd, attr) ? cmd_ret : -1;
        }
    }
    std::cout << "CMD" << '[' << out_str << ']' << ((cmd_ret == -1) ? "\033[1;31m" : "\033[1;32m")
        << " RET: " << std::dec << cmd_ret << "\033[0m" << std::endl;

    return cmd_ret;
}

static inline int do_command_string_transfer(long trans_fd, const struct ss_console_attr_s *attr,
                                             std::string &out_str, nlohmann::json &json_result)
{
    int cmd_ret = 0;
    std::string out_log;
    std::cout << "RUN: " << out_str << std::endl;
    cmd_ret = attr->send_cmd(trans_fd, out_str.c_str(), out_str.size() + 1, json_result);
    if (cmd_ret == 0)
    {
        cmd_ret = json_result["r"];
        out_log = json_result["l"];
        std::cout << out_log;
        std::string ask_for = json_result["a"];
        if (ask_for == "pause")
        {
            cmd_ret = do_pause(trans_fd, attr) ? cmd_ret : -1;
        }
    }
    std::cout << "CMD" << '[' << out_str << ']' << ((cmd_ret == -1) ? "\033[1;31m" : "\033[1;32m")
        << " RET: " << std::dec << cmd_ret << "\033[0m" << std::endl;

    return cmd_ret;
}

static inline int result_feedback(long pipe_write_fd, nlohmann::json &json_result)
{
    if (json_result["a"] == "catch")
    {
        int ret = 0;
        std::stringstream ss;
        std::string out_log = json_result["l"];
        json_result["l"] = nullptr;
        output_cmd_logs(json_result["l"], out_log);
        nlohmann::json &json_mc = json_result;
        ss << json_mc.dump(0) << std::endl << "sscmd_end" << std::endl;
        ret = write(pipe_write_fd, ss.str().c_str(), ss.str().size());
        return ret;
    }
    return 0;
}
static inline bool do_read_pipe_command(int pipe_fd[2], long trans_fd,
                                        const struct ss_console_attr_s *attr)
{
    string_lines read_cmd;
    std::string in_str, out_str;
    char cmd_buffer[TRANS_BUFFER];
    int read_ret = -1;
    size_t in_pos = 0;
    bool total_ret = true;

    memset(cmd_buffer, 0, sizeof(cmd_buffer));
    while(1)
    {
        read_ret = read(pipe_fd[0], cmd_buffer, sizeof(cmd_buffer));
        if (read_ret < 0)
        {
            perror("read");
            break;
        }
        if (!read_ret)
        {
            nlohmann::json json_result;
            read_cmd.end_line(out_str);
            if (out_str.size())
            {
                total_ret = do_command_string_transfer(trans_fd, attr, out_str, json_result)
                            ? false : total_ret;
                result_feedback(pipe_fd[1], json_result);
            }
            break;
        }
        in_str.append(cmd_buffer, read_ret);
        while (read_cmd.get_line(in_str, in_pos, out_str))
        {
            nlohmann::json json_result;
            in_str.erase(0, out_str.size() + 1);
            in_pos = 0;
            if (out_str.size())
            {
                total_ret = do_command_string_transfer(trans_fd, attr, out_str, json_result)
                            ? false : total_ret;
                result_feedback(pipe_fd[1], json_result);
            }
        }
        in_pos = in_str.size();
    }

    return total_ret;
}

static inline void json_out_file(nlohmann::json &json_out, const ss_console_json_out_s *console_json_out,
                                 unsigned int duration_time, bool ret)
{
    int orig_stage = 0, stage_range = 0, orig_result = 0;
    std::string tmp_str;

    if (!console_json_out->out_file)
    {
        return;
    }
    tmp_str = json_out["case_stage_range"];
    stage_range = ex_atoi(tmp_str.c_str());
    tmp_str = json_out["case_stage"];
    orig_stage = ex_atoi(tmp_str.c_str()) & stage_range;
    tmp_str = json_out["case_result"];
    orig_result = ex_atoi(tmp_str.c_str());

    std::stringstream ss;
    ss << "0x" << std::hex << ((console_json_out->case_stage & 0x80000000) ? 0 : ((console_json_out->case_stage & stage_range) | orig_stage));
    json_out["case_stage"] = ss.str();

    ss.str("");
    ss << "0x" << std::hex << ((console_json_out->case_stage & 0x80000000) ? 0 : (ret ? ((console_json_out->case_stage & stage_range) | orig_result)
                               : (~(console_json_out->case_stage & stage_range) & orig_result)));
    json_out["case_result"] = ss.str();

    ss.str("");
    json_out["case_run_time"] = (console_json_out->case_stage & 0x80000000) ? 0 : duration_time;
    std::ofstream fout(console_json_out->out_file);
    fout << json_out.dump(4);
    std::cout << "OUTPUT JSON FILE: " << "\033[1;33m" << console_json_out->out_file << "\033[0m" << std::endl;
}

static inline bool do_read_pipe_command_json(int pipe_fd[2], long trans_fd,
                                             const struct ss_console_attr_s *attr,
                                             nlohmann::json &json_data,
                                             const ss_console_json_out_s *console_json_out,
                                             unsigned long start_time)
{
    string_lines read_cmd;
    std::string in_str, out_str;
    char cmd_buffer[TRANS_BUFFER];
    int read_ret = -1;
    size_t in_pos = 0;
    bool total_ret = true;
    nlohmann::json tmp_rec = json_data;

    memset(cmd_buffer, 0, sizeof(cmd_buffer));
    while(1)
    {
        read_ret = read(pipe_fd[0], cmd_buffer, sizeof(cmd_buffer));
        if (read_ret < 0)
        {
            perror("read");
            break;
        }
        if (!read_ret)
        {
            nlohmann::json json_result;
            read_cmd.end_line(out_str);
            if (out_str.size())
            {
                if (console_json_out->log_opt & 0x200)
                {
                    nlohmann::json tmp_result;
                    tmp_result["r"] = 0;
                    tmp_result["l"] = "";
                    tmp_result["m"] = "";
                    script_out_json(tmp_rec, tmp_result, out_str.c_str(), 0);
                    json_out_file(tmp_rec, console_json_out, get_timer() - start_time, total_ret);
                }
                total_ret = do_command_string_transfer_json(trans_fd, attr, out_str, json_result, json_data, console_json_out)
                            ? false : total_ret;
                result_feedback(pipe_fd[1], json_result);
            }
            break;
        }
        in_str.append(cmd_buffer, read_ret);
        while (read_cmd.get_line(in_str, in_pos, out_str))
        {
            nlohmann::json json_result;
            in_str.erase(0, out_str.size() + 1);
            in_pos = 0;
            if (out_str.size())
            {
                if (console_json_out->log_opt & 0x200)
                {
                    nlohmann::json tmp_result;
                    tmp_result["r"] = 0;
                    tmp_result["l"] = "";
                    tmp_result["m"] = "";
                    script_out_json(tmp_rec, tmp_result, out_str.c_str(), 0);
                    json_out_file(tmp_rec, console_json_out, get_timer() - start_time, total_ret);
                }
                total_ret = do_command_string_transfer_json(trans_fd, attr, out_str, json_result, json_data, console_json_out)
                            ? false : total_ret;
                result_feedback(pipe_fd[1], json_result);
            }
        }
        in_pos = in_str.size();
    }

    return total_ret;
}

static inline int close_pipe(int pipe_fd[2])
{
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    return 0;
}
static inline nlohmann::json &get_report_case_data(nlohmann::json &json_data, const char *case_path)
{
    nlohmann::json *inner_path = &json_data["root"];
    const char *str_prev = NULL, *str_post = NULL;
    std::string path_name;

    str_prev = case_path;
    do
    {
        str_post = strstr(str_prev, "/");
        if (!str_post)
        {
            path_name.assign(str_prev, (case_path + strlen(case_path) - str_prev));
            inner_path = &(*inner_path)[path_name];
            break;
        }
        path_name.assign(str_prev, (str_post - str_prev));
        inner_path = &(*inner_path)[path_name];
        str_prev = str_post + 1;
    } while (*str_prev || *str_prev == ' ');

    return (*inner_path)["cases"];
}

static std::vector<nlohmann::json *> traver_container;
static inline bool console_json_out_cases_travel(nlohmann::json &json_data, void *opt,
                                            bool (*cases_travel)(nlohmann::json &, void *))
{
    bool ret = 0;
    if (!cases_travel)
    {
        return false;
    }
    for (auto iter = json_data.begin(); iter != json_data.end(); iter++)
    {
        std::string key = iter.key();
        if (key != "cases")
        {
            ret = console_json_out_cases_travel(iter.value(), opt, cases_travel);
            if (!ret)
            {
                break;
            }
        }
        else
        {
            for (auto iter2 = iter->begin(); iter2 != iter->end(); ++iter2)
            {
                ret = cases_travel(*iter2, opt);
                if (!ret)
                {
                    break;
                }

            }
        }
    }
    return ret;
}

static inline bool grab_all_cases_in_travel(nlohmann::json &case_obj, void *opt)
{
    std::string tmp_str;
    int stage = (int)((long)opt);
    int stage_range_from = 0;

    tmp_str = case_obj["stage_range"];
    stage_range_from = ex_atoi(tmp_str.c_str());
    if (stage_range_from & stage)
    {
        traver_container.push_back(&case_obj);
    }
    return true;
}

static inline bool grab_false_cases_in_travel(nlohmann::json &case_obj, void *opt)
{
    std::string tmp_str;
    int stage = (int)((long)opt);
    int result_from = 0;
    int stage_range_from = 0;

    tmp_str = case_obj["stage_range"];
    stage_range_from = ex_atoi(tmp_str.c_str());
    tmp_str = case_obj["result"];
    result_from = ex_atoi(tmp_str.c_str());
    if ((stage_range_from & stage) && !(stage & result_from))
    {
        traver_container.push_back(&case_obj);
    }
    return true;
}

static inline void json_out_trave_dir(const char *root_path, const char *base_path, nlohmann::json &json_data)
{
    DIR *dir= NULL;
    struct dirent *dp = NULL;
    struct stat st;
    std::string tmp_str = root_path;

    tmp_str += '/';
    tmp_str += base_path;
    if(stat(tmp_str.c_str(), &st) < 0 || !S_ISDIR(st.st_mode))
    {
        printf("invalid path: %s\n", tmp_str.c_str());
        return;
    }

    if(!(dir = opendir(tmp_str.c_str())))
    {
        std::cout << "opendir[" << tmp_str.c_str() << "] error path." << std::endl;
        return;
    }

    while((dp = readdir(dir)) != NULL)
    {
        if((!strncmp(dp->d_name, ".", 1)) || (!strncmp(dp->d_name, "..", 2)))
            continue;

        std::string post_str = tmp_str;
        post_str += '/';
        post_str += dp->d_name;
        stat(post_str.c_str(), &st);
        if(S_ISDIR(st.st_mode))
        {
            std::string post_base_str = base_path;
            post_base_str += '/';
            post_base_str += dp->d_name;
            json_out_trave_dir(root_path, post_base_str.c_str(), json_data);
        }
        else if (S_ISREG(st.st_mode) && !strcmp(dp->d_name + strlen(dp->d_name) - 5, ".json"))
        {
            nlohmann::json json_out_data, json_report_data;
            std::string str_val;
            size_t skip_idx = 0;
            const char *base_path_skip;

            while (base_path[skip_idx] == '/') skip_idx++;
            base_path_skip = base_path + skip_idx;
            std::ifstream fin(post_str.c_str());
            if (!fin.is_open())
            {
                std::cout << ">>>>>SS_REPORT_GEN[CAN NOT OPEN: "<< post_str <<" ] <<<<<" << std::endl;
                break;
            }
            fin >> json_out_data;
            nlohmann::json &inner_json = get_report_case_data(json_data, base_path_skip);
            json_report_data["case_name"] = json_out_data["case_name"];
            json_report_data["stage"] = json_out_data["case_stage"];
            json_report_data["stage_range"] = json_out_data["case_stage_range"];
            json_report_data["run_time"] = json_out_data["case_run_time"];
            json_report_data["path"] = base_path_skip;
            json_report_data["result"] = json_out_data["case_result"];
            inner_json.push_back(json_report_data);
            std::cout << "GEN REPORT: " << base_path_skip << "/" << dp->d_name << std::endl;
        }
    }
    closedir(dir);

    return;
}
static inline int ss_cmd(const struct ss_console_attr_s *attr, const char *key, const char *cmd)
{
    long trans_fd = 0, cmd_ret = 0;
    nlohmann::json json_result;
    assert(attr);
    assert(cmd);
    trans_fd = attr->trans_init(key);
    cmd_ret = attr->send_cmd(trans_fd, cmd, strlen(cmd) + 1, json_result);
    if (cmd_ret == 0)
    {
        cmd_ret = json_result["r"];
        std::string ret_log = json_result["l"];
        std::cout << ret_log;
    }
    attr->trans_deinit(trans_fd);
    return cmd_ret;
}

static inline int ss_exec(int pipe_fd[2], const struct ss_console_attr_s *attr, const char *key,
                          const struct ss_console_json_out_s *console_json_out)
{
    bool ret = 0;
    long trans_fd = 0;
    unsigned long duration_time = 0;
    nlohmann::json json_data;

    assert(attr);
    assert(console_json_out);
    if (console_json_out->out_file
        && (!console_json_out->case_name || !console_json_out->case_stage_range || !console_json_out->case_stage))
    {
        std::cout << ">>>>>SS_CMD_JSON [NO CASE NAME/STAGE/STAGE_RANGE] <<<<<" << std::endl;
        return -1;
    }
    trans_fd = attr->trans_init(key);
    if (trans_fd == -1)
    {
        return -1;
    }
    if (console_json_out->out_file)
    {
        std::ifstream fin(console_json_out->out_file);
        std::streampos file_size = 0;
        if (fin.is_open())
        {
            fin.seekg(0, std::ios_base::end);
            file_size = fin.tellg();
            fin.seekg(0, std::ios_base::beg);
        }
        if (file_size)
        {
            fin >> json_data;
            json_data["case_name"] = console_json_out->case_name;
            if (!console_json_out->do_append_cmds)
            {
                json_data["ss_cmds"].clear();
            }
        }
        else
        {
            std::stringstream ss;
            json_data = "{\"case_name\": null, \"case_result\": null, \"case_stage\": null, \"case_stage_range\": null, \"case_run_time\": null, \"ss_cmds\": null}"_json;
            json_data["case_name"] = console_json_out->case_name;
            ss << "0x" << std::hex << console_json_out->case_stage_range;
            json_data["case_stage_range"] = ss.str();
            ss.str("");
            ss << "0x" << std::hex << console_json_out->case_stage;
            json_data["case_stage"] = ss.str();
            json_data["case_result"] = "0x0";
        }
        duration_time = get_timer();
        ret = do_read_pipe_command_json(pipe_fd, trans_fd, attr, json_data, console_json_out, duration_time);
        duration_time = get_timer() - duration_time;
    }
    else
    {
        duration_time = get_timer();
        ret = do_read_pipe_command(pipe_fd, trans_fd, attr);
        duration_time = get_timer() - duration_time;
    }
    if (!check_pipe_return())
    {
        std::cout << "SCRIPT EXIT ABNORMALLY!!!" << std::endl;
        ret = false;
    }
    else if (console_json_out->log_opt & 0x100)
    {
        ret = do_pause(trans_fd, attr) ? ret : false;
    }
    if (console_json_out->case_name)
    {
        std::cout << ">>>>>SS_CMD_JSON [CASE: " << "\033[1;35m"<< console_json_out->case_name << "\033[0m" << "] "
            << (!ret ? "\033[1;31m FAIL!!" : "\033[1;32m PASS!!") << "\033[0m" << " T: " << duration_time << "ms <<<<<" << std::endl;
    }
    else
    {
        std::cout << ">>>>>SS_CMD_JSON [CASE: " << "\033[1;35m NO ASSIGN \033[0m" << "] "
            << (!ret ? "\033[1;31m FAIL!!" : "\033[1;32m PASS!!") << "\033[0m" << " T: " << duration_time << "ms <<<<<" << std::endl;
    }
    attr->trans_deinit(trans_fd);
    json_out_file(json_data, console_json_out, duration_time, ret);
    return ret ? 0 : -1;
}
class SsLuaCase
{
public:
    SsLuaCase(struct ss_console_attr_s *attr, long transFd, const char *case_name, const char *json_out_file_path, int stage, int stage_range, int log_opt);
    SsLuaCase(struct ss_console_attr_s *attr, long transFd, const ss_console_json_out_s &console_json_out);
    ~SsLuaCase();
    bool run_cmd(const char *cmd, std::string &log, bool record_result);
private:
    void _init();
    struct ss_console_attr_s *attr;
    int transFd;
    ss_console_json_out_s console_json_out;
    std::string case_name;
    std::string json_out_file_path;

    int result; /* Record the result of all command by `|` */
    unsigned int duration_time; /* Record total time of this case */
    nlohmann::json json_data; /* Record all cmds and case info */
};
SsLuaCase::SsLuaCase(struct ss_console_attr_s *attr, long transFd, const ss_console_json_out_s &console_json_out)
    : attr(attr), transFd(transFd), console_json_out(console_json_out)
{
    assert(attr);
    this->_init();
}
SsLuaCase::SsLuaCase(struct ss_console_attr_s *attr, long transFd, const char *case_name, const char *json_out_file_path, int stage, int stage_range, int log_opt)
    : attr(attr), transFd(transFd), case_name(case_name)
{
    assert(attr);
    assert(case_name);
    memset(&this->console_json_out, 0, sizeof(ss_console_json_out_s));
    this->console_json_out.case_name = this->case_name.c_str();
    if (json_out_file_path)
    {
        this->json_out_file_path = json_out_file_path;
        this->console_json_out.out_file = this->json_out_file_path.c_str();
    }
    this->console_json_out.case_stage       = stage;
    this->console_json_out.case_stage_range = stage_range;
    this->console_json_out.log_opt          = log_opt;
    this->_init();
}
SsLuaCase::~SsLuaCase()
{
    bool whole_ret = !this->result;
    duration_time = get_timer() - duration_time;
    if (this->console_json_out.log_opt & 0x100)
    {
        whole_ret = do_pause(this->transFd, this->attr) ? !this->result : false;
    }
    std::cout << ">>>>>SS_CMD_LUA [CASE: "
        << "\033[1;35m"<< this->console_json_out.case_name << "\033[0m" << "] "
        << (!whole_ret? "\033[1;31m FAIL!!" : "\033[1;32m PASS!!")
        << "\033[0m" << " T: " << duration_time << "ms <<<<<" << std::endl;
    json_out_file(this->json_data, &this->console_json_out, this->duration_time, whole_ret);
}
void SsLuaCase::_init()
{
    std::stringstream ss;
    this->json_data = "{\"case_name\": null, \"case_result\": null, \"case_stage\": null, \"case_stage_range\": null, \"case_run_time\": null, \"ss_cmds\": null}"_json;

    this->json_data["case_name"] = this->console_json_out.case_name;

    ss << "0x" << std::hex << this->console_json_out.case_stage_range;
    this->json_data["case_stage_range"] = ss.str();

    ss.str("");
    ss << "0x" << std::hex << this->console_json_out.case_stage;
    this->json_data["case_stage"]  = ss.str();

    this->json_data["case_result"] = "0x0";

    this->result = 0;
    this->duration_time = get_timer();
}
bool SsLuaCase::run_cmd(const char *cmd, std::string &log, bool record_result)
{
    nlohmann::json json_result;
    std::string command = cmd;
    int ret;
    if (!this->console_json_out.out_file)
    {
        ret = do_command_string_transfer(this->transFd, this->attr, command, json_result);
        this->result |= ret;
        return !ret;
    }
    ret = do_command_string_transfer_json(this->transFd, this->attr, command, json_result, this->json_data, &this->console_json_out);
    log = json_result["l"];
    if (record_result)
    {
        this->result |= ret;
    }
    return !ret;
}
static SsLuaCase *g_currLuaCase = NULL;
static int ss_lua_create_case(lua_State *L)
{
    int top = lua_gettop(L);

    if (top < 1 || g_currLuaCase)
    {
        lua_pushboolean(L, 0);
        return 1;
    }

    const char *case_name     = luaL_checkstring(L, 1);
    const char *json_out_file = top < 2 ? NULL : lua_isnil(L, 2) ? NULL : luaL_checkstring(L, 2);
    int stage                 = top < 3 ? 0 : luaL_checkint(L, 3);
    int stage_range           = top < 4 ? 0 : luaL_checkint(L, 4);
    int log_opt               = top < 5 ? 0 : luaL_checkint(L, 5);

    lua_getfield(L, LUA_REGISTRYINDEX, SS_CONSOLE_TRANS);
    struct ss_console_attr_s *attr = static_cast<struct ss_console_attr_s *>(lua_touserdata(L, -1));
    lua_pop(L, -1);

    lua_getfield(L, LUA_REGISTRYINDEX, SS_CONSOLE_TRANS_FD);
    long transFd = static_cast<long>(lua_tointeger(L, -1));
    lua_pop(L, -1);

    g_currLuaCase = new SsLuaCase(attr, transFd, case_name, json_out_file, stage, stage_range, log_opt);
    lua_pushboolean(L, 1);
    return 1;
}
static int ss_lua_run_cmd(lua_State *L)
{
    int top = lua_gettop(L);

    if (top < 1)
    {
        lua_pushboolean(L, 0);
        return 1;
    }

    const char *cmd = luaL_checkstring(L, 1);
    if (top >= 2)
    {
        luaL_checktype(L, 2, LUA_TBOOLEAN);
    }
    bool record_result = top < 2 ? true : lua_toboolean(L, 2);
    SsLuaCase *luaCase = NULL;

    if (!g_currLuaCase)
    {
        /* If handle has not been transfer by lua, use global luaCase */
        lua_getfield(L, LUA_REGISTRYINDEX, SS_CONSOLE_DEFAULT_CASE);
        luaCase = static_cast<SsLuaCase *>(lua_touserdata(L, -1));
        lua_pop(L, -1);
    }
    else
    {
        /* If handle has been transfer by lua, use it */
        luaCase = g_currLuaCase;
    }

    std::string log;
    lua_pushboolean(L, luaCase->run_cmd(cmd, log, record_result));
    lua_pushstring(L, log.c_str());
    return 2;
}
static int ss_lua_destroy_case(lua_State *L)
{
    if (!g_currLuaCase)
    {
        return 0;
    }
    delete g_currLuaCase;
    g_currLuaCase = NULL;
    return 0;
}
static int ss_lua_error_handle(lua_State *L)
{
    const char *msg = lua_tostring(L, 1);
    if (msg) {
        luaL_traceback(L, L, msg, 1);
    } else {
        lua_pushliteral(L, "(no error message)");
    }
    return 1;
}
static std::string ss_lua_create_arg_table(lua_State *L, const char *command)
{
    std::string            cmdLine  = command;
    unsigned int           posStart = 0, posEnd = 0;
    std::list<std::string> cmdList;

    lua_newtable(L);

    while (posEnd < cmdLine.length())
    {
        posStart = cmdLine.find_first_not_of(' ', posEnd);
        if (posStart >= cmdLine.length())
        {
            break;
        }
        posEnd = cmdLine.find(' ', posStart);
        cmdList.push_back(cmdLine.substr(posStart, posEnd - posStart));
    }

    lua_createtable(L, cmdList.size() - 1, cmdList.size());
    int idx = 0;
    for (auto it = cmdList.begin(); it != cmdList.end(); ++it)
    {
        lua_pushstring(L, it->c_str());
        lua_rawseti(L, -2, idx);
        ++idx;
    }
    lua_setglobal(L, "arg");

    return cmdList.front();
}
static const struct luaL_Reg g_ss_lua_lib[] = {
    { "create_case" , ss_lua_create_case  },
    { "run_cmd"     , ss_lua_run_cmd      },
    { "destroy_case", ss_lua_destroy_case },
    { NULL, NULL },
};
int ss_command(struct ss_console_attr_s *attr, const char *key, const char *command)
{
    return ss_cmd(attr, key, command);
}
int ss_python(struct ss_console_attr_s *attr, const char *key, const char *script_file, const struct ss_console_json_out_s *console_json_out)
{
    int ret = 0;
    int pipe_fd[2];
    std::vector<std::string> in_strs;

    in_strs.push_back("python");
    in_strs.push_back(script_file);
    ret = open_pipe(pipe_fd, in_strs);
    if (ret == -1)
    {
        return -1;
    }
    ret = ss_exec(pipe_fd, attr, key, console_json_out);
    close_pipe(pipe_fd);
    return ret;
}
int ss_shell(struct ss_console_attr_s *attr, const char *key, const char *script_file, const struct ss_console_json_out_s *console_json_out)
{
    int ret = 0;
    int pipe_fd[2];
    std::vector<std::string> in_strs;

    in_strs.push_back("sh");
    in_strs.push_back("-c");
    in_strs.push_back(script_file);
    ret = open_pipe(pipe_fd, in_strs);
    if (ret == -1)
    {
        return -1;
    }
    ret = ss_exec(pipe_fd, attr, key, console_json_out);
    close_pipe(pipe_fd);
    return ret;
}
int ss_lua(struct ss_console_attr_s *attr, const char *key, const char *command, const struct ss_console_json_out_s *console_json_out)
{
    assert(attr);
    assert(command);
    assert(console_json_out);

    int ret;
    lua_State *L;
    long transFd;
    SsLuaCase *luaCase;

    transFd = attr->trans_init(key);
    if (-1 == transFd)
    {
        std::cout << "trans_init fail" << std::endl;
        ret = -1;
        goto ERR_TRANS_INIT;
    }

    L = luaL_newstate();
    if (!L)
    {
        std::cout << "luaL_newstate fail" << std::endl;
        ret = -1;
        goto ERR_NEW_LUA_STATE;
    }
    luaL_openlibs(L);
    luaL_register(L, "ss_console", g_ss_lua_lib);

    if (luaL_loadfile(L, ss_lua_create_arg_table(L, command).c_str()))
    {
        std::cout << "Couldn't load file " << lua_tostring(L, -1) << std::endl;
        ret = -1;
        goto ERR_LOAD_LUA_FILE;
    }

    lua_pushlightuserdata(L, attr);
    lua_setfield(L, LUA_REGISTRYINDEX, SS_CONSOLE_TRANS);

    lua_pushinteger(L, transFd);
    lua_setfield(L, LUA_REGISTRYINDEX, SS_CONSOLE_TRANS_FD);

    luaCase = new SsLuaCase(attr, transFd, *console_json_out);
    lua_pushlightuserdata(L, luaCase);
    lua_setfield(L, LUA_REGISTRYINDEX, SS_CONSOLE_DEFAULT_CASE);

    lua_pushcfunction(L, ss_lua_error_handle);
    lua_insert(L, 1);

    if (lua_pcall(L, 0, LUA_MULTRET, 1))
    {
        std::cout << "Error: " << lua_tostring(L, -1) << std::endl;
        ret = -1;
        goto ERR_RUN_LOA_FILE;
    }

    ret = 0;

ERR_RUN_LOA_FILE:
    delete luaCase;
ERR_LOAD_LUA_FILE:
    lua_close(L);
ERR_NEW_LUA_STATE:
    attr->trans_deinit(transFd);
ERR_TRANS_INIT:
    return ret;
}
int ss_json(struct ss_console_attr_s *attr, const char *key, const char *in_file, const struct ss_console_json_out_s *console_json_out)
{
    std::string cmd, cmd_name;
    bool ret = true;
    bool whole_ret = true;
    long trans_fd = 0;
    nlohmann::json json_data;
    assert(attr);
    assert(in_file);
    assert(console_json_out);
    if (console_json_out->out_file && !console_json_out->case_stage)
    {
        std::cout << ">>>>>SS_CMD_JSON [NO CASE STAGE] <<<<<" << std::endl;
        return -1;
    }
    std::ifstream fin(in_file);
    if (!fin.is_open())
    {
        std::cout << ">>>>>SS_CMD_JSON OPEN: "<< "\033[1;33m" << in_file << "\033[1;31m" << " FAIL!! \033[0m" << std::endl;
        return -1;
    }

    fin >> json_data;
    trans_fd = attr->trans_init(key);
    if (trans_fd == -1)
    {
        return -1;
    }
    std::cout << ">>>>>SS_CMD_JSON [FILE: " << "\033[1;33m" << in_file << "\033[0m" << "] START!!! <<<<<" << std::endl;
    unsigned int duration_time = get_timer();
    for (auto it = json_data["ss_cmds"].begin(); it != json_data["ss_cmds"].end(); ++it)
    {
        cmd_name = it->begin().key();
        ret = true;
        for (auto it_cmd = it->begin()->begin(); it_cmd != it->begin()->end(); ++it_cmd)
        {
            int cmd_ret = 0;
            bool is_md5_fail = false;
            nlohmann::json json_result;
            std::string out_log, md5_left, md5_right;
            md5_left = it_cmd->at("md5");
            cmd = cmd_name;
            cmd += it_cmd->at("in");
            adapt_cmd_of_sys_out_md5_auto(cmd, in_file, md5_left);
            cmd_ret = attr->send_cmd(trans_fd, cmd.c_str(), cmd.size() + 1, json_result);
            if (cmd_ret == 0)
            {
                cmd_ret = json_result["r"];
                out_log = json_result["l"];
                std::cout << out_log;
                md5_right = json_result["m"];
                if (md5_left.size() || md5_right.size())
                {
                    is_md5_fail = (md5_left != md5_right);
                    std::cout << "CMD" << '[' << cmd  << ']' << ((cmd_ret == -1) ? "\033[1;31m" : "\033[1;32m") << " RET: " << std::dec << cmd_ret << "\033[0m"
                        << ", MD5 CHECK: " << ((is_md5_fail) ? "\033[1;31m DIFFERENTLY!!" : "\033[1;32m SAME!!") << "\033[0m" << std::endl;
                }
                ret = (cmd_ret || is_md5_fail) ? false : ret;
                std::string ask_for = json_result["a"];
                if (ask_for == "pause")
                {
                    ret = do_pause(trans_fd, attr);
                }
            }
            else
            {
                ret = false;
            }
            std::cout << "CMD" << '[' << cmd_name << ']' << (!ret ? "\033[1;31m" : "\033[1;32m")
                << " RET: " << std::dec << cmd_ret << "\033[0m" << std::endl;
            if (console_json_out->out_file)
            {
                //output json data.
                if (!md5_left.size())
                {
                    it_cmd->at("md5") = md5_right;
                }
                it_cmd->at("ret") = cmd_ret;
                it_cmd->at("logs") = nullptr;
                if ((console_json_out->log_opt & 0xFF) == 2
                    || ((console_json_out->log_opt & 0xFF) == 1 && cmd_ret == -1))
                {
                    output_cmd_logs(it_cmd->at("logs"), out_log);
                }
            }
        }
        whole_ret = (ret == false) ? false : whole_ret;
    }
    duration_time = get_timer() - duration_time;
    if (console_json_out->log_opt & 0x100)
    {
        whole_ret = do_pause(trans_fd, attr) ? whole_ret : false;
    }
    std::cout << ">>>>>SS_CMD_JSON [CASE: " << "\033[1;35m"<< json_data["case_name"] << "\033[0m" << "] "
              << (!whole_ret ? "\033[1;31m FAIL!!" : "\033[1;32m PASS!!") << "\033[0m" << " T: " << duration_time << "ms <<<<<" << std::endl;
    attr->trans_deinit(trans_fd);
    json_out_file(json_data, console_json_out, duration_time, whole_ret);
    return whole_ret ? 0 : -1;
}
int ss_run_json_case(struct ss_console_attr_s *attr, const char *key, const char *report_file,
                     const char *case_path_name, const char *script_path, int stage, int log_opt)
{
    nlohmann::json json_data;
    std::string script_file, str_case_name, str_case_path, str_case_stage_range;
    struct ss_console_json_out_s console_json_out;
    std::ifstream fin(report_file);
    assert(attr);
    assert(report_file);
    if (!fin.is_open())
    {
        std::cout << "Open file: " << report_file << " error!" << std::endl;
        return -1;
    }
    fin >> json_data;
    memset(&console_json_out, 0, sizeof(struct ss_console_json_out_s));
    str_case_path = case_path_name;
    size_t find_pos = str_case_path.find_last_of("/");
    if (find_pos != std::string::npos)
    {
        str_case_name = str_case_path.c_str() + find_pos + 1;
        str_case_path.erase(find_pos, str_case_path.size() - find_pos);
    }
    nlohmann::json &inner_json = get_report_case_data(json_data, str_case_path.c_str());
    if (!inner_json.size())
    {
        std::cout << "#### NOT FOUND PATH: " << "\033[1;32m" << script_path << '/' << str_case_path << '/' << str_case_name << "\033[0m" << " #####" << std::endl;
        return -1;
    }
    for (auto iter = inner_json.begin(); iter != inner_json.end(); iter++)
    {
        std::string found_case_name = iter->at("case_name");
        if (found_case_name == str_case_name)
        {
            //Must be found only one.
            console_json_out.case_name = str_case_name.c_str();
            script_file = script_path;
            script_file += script_file.size() ? "/" : "./";
            script_file += str_case_path + '/' + str_case_name + ".json";
            console_json_out.out_file = script_file.c_str();
            console_json_out.case_stage = stage;
            console_json_out.case_stage_range = ex_atoi(str_case_stage_range.c_str());
            console_json_out.log_opt = log_opt;
            std::cout << "#### RUN CASE : " << "\033[1;33m" <<  str_case_path << '/' << "\033[1;35m" << str_case_name << "\033[0m #####" << std::endl;
            return ss_json(attr, key, console_json_out.out_file, &console_json_out);
        }
    }
    std::cout << "#### NOT FOUND CASE: " << "\033[1;32m" << script_path << '/' << str_case_path << '/' << str_case_name << "\033[0m" << " #####" << std::endl;
    return -1;
}
int ss_run_json_case_false(struct ss_console_attr_s *attr, const char *key, const char *report_file,
                           const char *script_path, int stage, int log_opt)
{
    nlohmann::json json_data;
    std::string script_file, str_case_name, str_case_path, str_case_stage_range;
    struct ss_console_json_out_s console_json_out;
    std::ifstream fin(report_file);
    int ret = 0;
    assert(attr);
    assert(report_file);
    if (!fin.is_open())
    {
        std::cout << "Open file: " << report_file << " error!" << std::endl;
        return -1;
    }
    fin >> json_data;
    memset(&console_json_out, 0, sizeof(struct ss_console_json_out_s));
    console_json_out_cases_travel(json_data, (void *)(long)stage, grab_false_cases_in_travel);
    for (auto it = traver_container.begin(); it != traver_container.end(); ++it)
    {
        nlohmann::json &json_find = **it;
        str_case_name = json_find["case_name"];
        console_json_out.case_name = str_case_name.c_str();
        str_case_path = json_find["path"];
        script_file = script_path;
        script_file += script_file.size() ? "/" : "./";
        script_file += str_case_path + '/' + str_case_name + ".json";
        console_json_out.out_file = script_file.c_str();
        console_json_out.case_stage = stage;
        str_case_stage_range = json_find["stage_range"];
        console_json_out.case_stage_range = ex_atoi(str_case_stage_range.c_str());
        console_json_out.log_opt = log_opt;
        std::cout << "#### RUN CASE : " << "\033[1;33m" <<  str_case_path << '/' << "\033[1;35m" << str_case_name << "\033[0m #####" << std::endl;
        ret |= ss_json(attr, key, console_json_out.out_file, &console_json_out);
    }
    traver_container.clear();
    std::cout << "======================================================================" << std::endl;
    std::cout << "---------RUN ALL FAIL CASE: " << "\033[1;36m" << report_file << "\033[0m" << ((ret < 0) ? "\033[5;31m FAIL!!" : "\033[5;32m PASS!!") << "\033[0m---------" << std::endl;
    std::cout << "======================================================================" << std::endl;

    return ret;
}
int ss_run_json_case_all(struct ss_console_attr_s *attr, const char *key, const char *report_file,
                         const char *script_path, int stage, int log_opt)
{
    nlohmann::json json_data;
    std::string script_file, str_case_name, str_case_path, str_case_stage_range;
    struct ss_console_json_out_s console_json_out;
    std::ifstream fin(report_file);
    int ret = 0;
    assert(attr);
    assert(report_file);
    if (!fin.is_open())
    {
        std::cout << "Open file: " << report_file << " error!" << std::endl;
        return -1;
    }
    fin >> json_data;
    memset(&console_json_out, 0, sizeof(struct ss_console_json_out_s));
    console_json_out_cases_travel(json_data, (void *)(long)stage, grab_all_cases_in_travel);
    for (auto it = traver_container.begin(); it != traver_container.end(); ++it)
    {
        nlohmann::json &json_find = **it;
        str_case_name = json_find["case_name"];
        console_json_out.case_name = str_case_name.c_str();
        str_case_path = json_find["path"];
        script_file = script_path;
        script_file += script_file.size() ? "/" : "./";
        script_file += str_case_path + '/' + str_case_name + ".json";
        console_json_out.out_file = script_file.c_str();
        console_json_out.case_stage = stage;
        str_case_stage_range = json_find["stage_range"];
        console_json_out.case_stage_range = ex_atoi(str_case_stage_range.c_str());
        console_json_out.log_opt = log_opt;
        std::cout << "#### RUN CASE : " << "\033[1;33m" <<  str_case_path << '/' << "\033[1;35m" << str_case_name << "\033[0m #####" << std::endl;
        ret |= ss_json(attr, key, console_json_out.out_file, &console_json_out);
    }
    traver_container.clear();
    std::cout << "======================================================================" << std::endl;
    std::cout << "---------------RUN ALL: " << "\033[1;36m" << report_file << "\033[0m" << ((ret < 0) ? "\033[5;31m FAIL!!" : "\033[5;32m PASS!!") << "\033[0m---------------" << std::endl;
    std::cout << "======================================================================" << std::endl;

    return ret;
}
int ss_run_json_case_random(struct ss_console_attr_s *attr, const char *key, const char *report_file, const char *script_path,
                            int stage, int log_opt)
{
    unsigned int order = 0;
    nlohmann::json json_data;
    std::string script_file, str_case_name, str_case_path, str_case_stage_range;
    struct ss_console_json_out_s console_json_out;
    std::ifstream fin(report_file);
    assert(attr);
    assert(report_file);
    if (!fin.is_open())
    {
        std::cout << "Open file: " << report_file << " error!" << std::endl;
        return -1;
    }
    fin >> json_data;
    memset(&console_json_out, 0, sizeof(struct ss_console_json_out_s));
    console_json_out_cases_travel(json_data, (void *)(long)stage, grab_all_cases_in_travel);
    if (!traver_container.size())
    {
        std::cout << "#### NO CASE IN STAGE: " << "\033[1;32m" << stage << "\033[0m" << " #####" << std::endl;
        return -1;
    }
    order = get_random(0, traver_container.size() - 1);
    nlohmann::json &json_find = *traver_container[order];
    traver_container.clear();
    str_case_name = json_find["case_name"];
    console_json_out.case_name = str_case_name.c_str();
    str_case_path = json_find["path"];
    script_file = script_path;
    script_file += script_file.size() ? "/" : "./";
    script_file += str_case_path + '/' + str_case_name + ".json";
    console_json_out.out_file = script_file.c_str();
    console_json_out.case_stage = stage;
    str_case_stage_range = json_find["stage_range"];
    console_json_out.case_stage_range = ex_atoi(str_case_stage_range.c_str());
    console_json_out.log_opt = log_opt;
    std::cout << "#### RUN RAMDON CASE : " << "\033[1;33m" <<  str_case_path << '/' << "\033[1;35m" << str_case_name << "\033[0m #####" << std::endl;
    return ss_json(attr, key, console_json_out.out_file, &console_json_out);
}

int ss_generate_report_file(const char *script_path, const char *report_file)
{
    std::ofstream fout(report_file);
    nlohmann::json json_data;

    json_out_trave_dir(script_path, "/", json_data);
    fout << json_data.dump(4);
    return 0;
}
