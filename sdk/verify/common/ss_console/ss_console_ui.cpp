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
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <sstream>

#include "json.hpp"
#include "ss_console.h"

using namespace std;

struct shm_st
{
    unsigned int magic;
    unsigned int ref;
    unsigned char bwrite;
    unsigned int size;
    char buf[TRANS_BUFFER];
};

struct shm_client
{
    key_t client_shm_id;
    struct shm_st *shm_data;
};

static inline long ex_atoi(const char *str)
{
    if (!str)
        return -1;

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

static long shm_transfer_init(const char *key)
{
    struct shm_client *p_shm_client = NULL;

    if (!key)
    {
        return -1;
    }
    p_shm_client = new struct shm_client;
    if (!p_shm_client)
    {
        cout << "alloc fall!" << endl;
        return -1;
    }
    p_shm_client->client_shm_id = shmget((key_t)atoi(key), sizeof(struct shm_st),
                                         0666|IPC_CREAT);
    if(p_shm_client->client_shm_id == -1)
    {
        delete p_shm_client;
        cout << "shmget failed." << endl;
        return -1;
    }
    p_shm_client->shm_data = (struct shm_st *)shmat(p_shm_client->client_shm_id,
                                                    (void *)0, 0);
    if(p_shm_client->shm_data == (void *)-1 || p_shm_client->shm_data == NULL)
    {
        shmctl(p_shm_client->client_shm_id, IPC_RMID, NULL);
        delete p_shm_client;
        cout << "shmat failed" << endl;
        return -1;
    }
    if (p_shm_client->shm_data->magic != 88860611)
    {
        memset(p_shm_client->shm_data, 0, sizeof(struct shm_st));
        p_shm_client->shm_data->magic = 88860611;
        cout << "New shm, Key: " << key << endl;
    }
    p_shm_client->shm_data->ref++;
    return (long)p_shm_client;
}
static int shm_transfer_deinit(long fd)
{
    struct shm_client *p_shm_client = (struct shm_client *)fd;
    if (!p_shm_client)
    {
        cout << "Fd is null." << endl;
        return -1;
    }
    p_shm_client->shm_data->ref--;
    if (!p_shm_client->shm_data->ref)
    {
        memset(p_shm_client->shm_data, 0, sizeof(struct shm_st));
        if (shmdt(p_shm_client->shm_data) == -1)
        {
            cout << "shmdt failed." << endl;
            return -1;
        }
        if (shmctl(p_shm_client->client_shm_id, IPC_RMID, NULL) == -1)
        {
            cout << "Destroy share memory failed!" << endl;
            return -1;
        }
        delete p_shm_client;
        cout << "Del shm" << endl;
        return 0;
    }
    if (shmdt(p_shm_client->shm_data) == -1)
    {
        cout << "shmdt failed." << endl;
        return -1;
    }
    delete p_shm_client;
    return 0;
}
static int shm_send_cmd_json_out(long fd, const char *cmd, unsigned int size, nlohmann::json &json_out)
{
    stringstream out_ss;
    struct shm_client *p_shm_client = (struct shm_client *)fd;
    unsigned int write_size = 0;
    unsigned int write_total = 0;

    if (!p_shm_client || !p_shm_client->shm_data)
    {
        return -1;
    }
    while (1)
    {
        write_size = (size - write_total > TRANS_BUFFER ? TRANS_BUFFER : size - write_total);
        memcpy(p_shm_client->shm_data->buf, cmd + write_total, write_size);
        write_total += write_size;
        p_shm_client->shm_data->size = write_size;
        if (write_total == size)
        {
            break;
        }
        p_shm_client->shm_data->bwrite = 1;
        while (p_shm_client->shm_data->bwrite == 1)
        {
            usleep(1000);
        }
    }
    while (1)
    {
        p_shm_client->shm_data->bwrite = 1;
        while (p_shm_client->shm_data->bwrite == 1)
        {
            usleep(1000);
        }
        out_ss.write(p_shm_client->shm_data->buf, p_shm_client->shm_data->size);
        if (p_shm_client->shm_data->size < TRANS_BUFFER)
        {
            break;
        }
    }
    out_ss >> json_out;
    return 0;
}
static int shm_grab_tab_list(long fd, string &str)
{
    long ret = 0;
    nlohmann::json json_out;

    shm_send_cmd_json_out(fd, "internal_get_tabs", sizeof("internal_get_tabs"), json_out);
    str = json_out["t"];
    ret = json_out["r"];
    return ret;
}
static int socket_do_recv(long fd, char *p_trans_data, unsigned int size)
{
    int ret = 0;
    unsigned int data_offset = 0;

    while(1)
    {
        ret = recv(fd, p_trans_data + data_offset, size - data_offset, 0);
        if (ret <= 0)
        {
            return ret;
        }
        data_offset += ret;
        if (p_trans_data[data_offset - 1] == '\0')
        {
            //for c++ string append, return size is not include c string tail '\0'.
            return data_offset - 1;
        }
        if (size == data_offset)
        {
            break;
        }
    }

    return data_offset;
}

static long none_transfer_init(const char *key)
{
    return 0;
}

static int none_transfer_deinit(long fd)
{
    return 0;
}

static int none_send_cmd_json_out(long fd, const char *cmd, unsigned int size, nlohmann::json &json_out)
{
    json_out["m"] = "";
    json_out["l"] = "";
    json_out["a"] = "";
    json_out["c"] = "";
    json_out["r"] = -1;
    return 0;
}

static int none_grab_tab_list(long fd, string &str)
{
    return 0;
}

static long socket_transfer_init(const char *key)
{
    struct sockaddr_in address;
    long socket_fd;
    int len;
    int ret;

    if (key == NULL)
    {
        return -1;
    }
    if ((socket_fd = (long)socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("socket");
        return -1;
    }
    memset(&address, 0, sizeof(struct sockaddr_in));
    address.sin_family = AF_INET;
    address.sin_port = htons(8000);
    if(inet_pton(AF_INET, key, &address.sin_addr) <= 0)
    {
        printf("inet_pton error for %s\n", key);
        close(socket_fd);
        return -1;
    }
    len = sizeof(struct sockaddr_in);
    ret = connect(socket_fd, (struct sockaddr *)&address, len);
    if (ret < 0)
    {
        printf ("ensure the server is up\n");
        perror ("connect");
        close(socket_fd);
        return -1;
    }

    return socket_fd;
}
static int socket_send_cmd_json_out(long fd, const char *cmd, unsigned int size, nlohmann::json &json_out)
{
    int ret = -1;
    char ret_log_buf[TRANS_BUFFER];
    stringstream out_ss;

    if (send(fd, cmd, size, 0) < 0)
    {
        perror("send");
        return -1;
    }
    while (1)
    {
        ret = socket_do_recv(fd, ret_log_buf, TRANS_BUFFER);
        if (ret < 0)
        {
            cout << "recv ret" << ret << endl;
            return -1;
        }
        if (ret == 0)
        {
            std::cout << "Server Broke Down!!!" << std::endl;
            return -1;
        }
        out_ss.write(ret_log_buf, ret);
        if (ret < TRANS_BUFFER)
        {
            break;
        }
    }
    out_ss >> json_out;
    return 0;
}

static int socket_transfer_deinit(long fd)
{
    //Force to set local command if current chip_id is not 0.
    nlohmann::json json_out;
    long cmd_ret = 0;
    cmd_ret = socket_send_cmd_json_out(fd, "route_call 0 reset_socket", sizeof("route_call 0 reset_socket"), json_out);
    if (cmd_ret == -1)
    {
        return -1;
    }
    close (fd);

    return 0;
}
static int socket_grab_tab_list(long fd, string &str)
{
    int ret = 0;
    nlohmann::json json_out;
    ret = socket_send_cmd_json_out(fd, "internal_get_tabs", sizeof("internal_get_tabs"), json_out);
    if (ret == -1)
    {
        return -1;
    }
    str = json_out["t"];
    ret = json_out["r"];
    return ret;
}

static inline void print_help(const char *prog_name)
{
    cout << "Program    : <" << prog_name << "> is for Sigmastar sdk ut client console." << endl;
    cout << "Author     : pedro.peng <pedro.peng@sigmastar.com.cn>." << endl;
    cout << "Build info : Commit " << GIT_COMMIT << ", Build by " << BUILD_OWNER << ", Date " << BUILD_DATE << '.' << endl;
    cout << "Connect socket server or shm and run on console:" << endl;
    cout << prog_name << " [--shm] --console" << endl;
    cout << "Connect socket server or shm and run by python file and output json file:" << endl;
    cout << prog_name << " --shm|--socket=<ip address> --python <py file> -o <json out file> --log=<option> --case-name=<name> --stage=<id> --stage-range=<id>" << endl;
    cout << "Connect socket server or shm and run by shell file and output json file:" << endl;
    cout << prog_name << " --shm|--socket=<ip address> --shell <sh file> -o <json out file> --log=<option> --case-name=<name> --stage=<id> --stage-range=<id>" << endl;
    cout << "Connect socket server or shm and run by json file and output json file:" << endl;
    cout << prog_name << " --shm|--socket=<ip address> --json <json file> -o <json out file> --log=<option> --stage=<id> --stage-range=<id>" << endl;
    cout << "Run a specified test case in json report file:" << endl;
    cout << prog_name << " --shm|--socket=<ip address> --run=<case_path_and_name> -t <report json file> --stage=<id> --script-path=<path>" << endl;
    cout << "Run all test case in json report file:" << endl;
    cout << prog_name << " --shm|--socket=<ip address> --run-all -t <report json file> --stage=<id> --script-path=<path>" << endl;
    cout << "Run all fail test case in json report file:" << endl;
    cout << prog_name << " --shm|--socket=<ip address> --run-false -t <report json file> --stage=<id> --script-path=<path>" << endl;
    cout << "Run a test case in json report file by random:" << endl;
    cout << prog_name << " --shm|--socket=<ip address> --run-random --stage=<id> --script-path=<path>" << endl;
    cout << "Generate report file by searching json file in the specified path:" << endl;
    cout << prog_name << " --gen-report -t <report json file> --script-path=<path>" << endl;
    cout << "--socket=<ip address>: Client use socket to connect server." << endl;
    cout << "--shm:                 Client use share memory(shm) to connect server." << endl;
    cout << "--console:             Enter a console ui over IPC." << endl;
    cout << "--run                  Run json case by case id from report json file." << endl;
    cout << "--run-all              Run all json case from report json file." << endl;
    cout << "--run-false            Run fail json cases from report json file." << endl;
    cout << "--run-random           Run a json case from report json file by random." << endl;
    cout << "--conv2json            Run a json or shell case and only output json file without any real command's execution." << endl;
    cout << "-r --run-cmd:          To run a command with 'argv'." << endl;
    cout << "-s --shell:            To run a serial of commands in shell script, commands transform by socket or shm." << endl;
    cout << "-j --json:             To run a serial of commands in json script, commands transform by socket or shm." << endl;
    cout << "-u --lua:              To run a serial of commands in lua script, commands transform by socket or shm." << endl;
    cout << "-l --log:              Config json script output log option, option, 0: Not output log, 1: Output log only if command return error. 2: Output all log." << endl
         << "                       Log output Mask: 0x100: Enter the judgement ui at the end of command, for example: -l 0x102, do judgement after it had outputed all logs."<< endl
         << "                       Log output Mask: 0x200: Output json file at the end of every command's execution, it is for debug use."<< endl;
    cout << "-o:                    Config json script output file." << endl;
    cout << "-t:                    Input/Output a json report file for test case." << endl;
    cout << "-n --case-name:        Config json script output case name." << endl;
    cout << " --stage:              Set current case's 'stage' value which is the mask value for filtering cases." << endl;
    cout << " --stage-range:        Set current case's 'stage range' which is the amount of all 'stage' values that cases support." << endl;
    cout << " --script-path:        Report file used only in input, to set the base path to find the json file that is built like 'script_path/case_path/case_name.json'." << endl;
    cout << " --append-cmds         To append commands in the output json file existed before.";
    cout << "-h --help:             To show this HELP infomation." << endl;
}
enum CONSOLE_UI_FUNCTION
{
    CONNECT_SOCKET = 0x1,
    CONNECT_SHM = 0x2,
    RUN_CMD = 0x4,
    RUN_CONSOLE = 0x8,
    RUN_PYTHON = 0x10,
    RUN_SHELL = 0x20,
    RUN_JSON = 0x40,
    RUN_JSON_CASE = 0x80,
    RUN_JSON_CASE_FALSE = 0x100,
    RUN_JSON_CASE_ALL = 0x200,
    RUN_JSON_CASE_RAMDOM = 0x400,
    OUT_JSON_FILE = 0x800,
    OUT_JSON_REPORT_FILE = 0x1000,
    LOG_OPTION = 0x2000,
    OUT_JSON_CASE_NAME = 0x4000,
    OUT_JSON_CASE_STAGE = 0x8000,
    OUT_JSON_CASE_STAGE_RANGE = 0x10000,
    OUT_JSON_PATH = 0x20000,
    CONVERT_TO_JSON = 0x40000,
    GENERATE_REPORT_FILE = 0x80000,
    APPEND_CMDS_IN_JSON = 0x100000,
    RUN_LUA = 0x200000,
};

int setup_console_ui(int argc, char **argv)
{
    int result = 0;
    int option_index = 0;
    int stage = 0;
    int case_stage = 1;
    string in_para_strs, case_name = "defalut_case", case_path, script_path;
    string console_key, script_file, out_file, out_report_file;
    struct ss_console_attr_s console_attr;
    struct ss_console_json_out_s console_json_out;
    struct option long_options[] = {
        {"socket",        required_argument, 0, 0},
        {"shm",           required_argument, 0, 0},
        {"console",       no_argument, 0, 0},
        {"run",           required_argument, 0, 0},
        {"run-all",       no_argument, 0, 0},
        {"run-false",     no_argument, 0, 0},
        {"run-random",    no_argument, 0, 0},
        {"gen-report",    no_argument, 0, 0},
        {"stage",         required_argument, 0, 0},
        {"stage-range",   required_argument, 0, 0},
        {"conv2json",     no_argument, 0, 0},
        {"script-path",   required_argument, 0, 0},
        {"append-cmds",   no_argument, 0, 0},
        {"help",          no_argument, 0, 'h'},
        {"run-cmd",       required_argument, 0, 'r'},
        {"python",        required_argument, 0, 'p'},
        {"shell",         required_argument, 0, 's'},
        {"json",          required_argument, 0, 'j'},
        {"lua",           required_argument, 0, 'u'},
        {"log",           required_argument, 0, 'l'},
        {"case-name",     required_argument, 0, 'n'},
        { 0, 0, 0, 0 }
    };

    memset(&console_json_out, 0, sizeof(struct ss_console_json_out_s));
    console_json_out.log_opt = 1; //defalut is 1, show only error
    console_json_out.case_stage = console_json_out.case_stage_range = 1; //defalut is 1
    console_json_out.case_name = case_name.c_str();
    while ((result = getopt_long(argc, argv, "hr:s:j:o:l:n:t:u:", long_options, &option_index)) != -1)
    {
        switch (result)
        {
            case 0:
            {
                if (!strcmp(long_options[option_index].name, "socket"))
                {
                    if (stage & (CONNECT_SOCKET | CONNECT_SHM | CONVERT_TO_JSON | CONVERT_TO_JSON))
                        break;
                    console_attr.trans_init = socket_transfer_init;
                    console_attr.trans_deinit = socket_transfer_deinit;
                    console_attr.send_cmd = socket_send_cmd_json_out;
                    console_attr.grab_tab_list = socket_grab_tab_list;
                    if (optarg[0] == '=')
                        optarg++;
                    console_key = optarg;
                    stage |= CONNECT_SOCKET;
                }
                else if (!strcmp(long_options[option_index].name, "shm"))
                {
                    if (stage & (CONNECT_SOCKET | CONNECT_SHM | CONVERT_TO_JSON | CONVERT_TO_JSON))
                        break;
                    console_attr.trans_init = shm_transfer_init;
                    console_attr.trans_deinit = shm_transfer_deinit;
                    console_attr.send_cmd = shm_send_cmd_json_out;
                    console_attr.grab_tab_list = shm_grab_tab_list;
                    if (optarg[0] == '=')
                        optarg++;
                    console_key = optarg;
                    stage |= CONNECT_SHM;
                }
                else if (!strcmp(long_options[option_index].name, "console"))
                {
                    if (stage & (RUN_CMD | RUN_CONSOLE | RUN_SHELL | RUN_JSON | RUN_LUA | RUN_PYTHON | OUT_JSON_FILE | RUN_JSON_CASE_RAMDOM))
                        break;
                    stage |= RUN_CONSOLE;
                    //bit 2
                }
                else if (!strcmp(long_options[option_index].name, "run"))
                {
                    if (stage & (RUN_CMD | RUN_CONSOLE | RUN_SHELL | RUN_JSON | RUN_LUA | RUN_PYTHON | OUT_JSON_FILE | RUN_JSON_CASE))
                        break;
                    if (optarg[0] == '=')
                        optarg++;
                    case_name = optarg;
                    stage |= RUN_JSON_CASE;
                }
                else if (!strcmp(long_options[option_index].name, "run-all"))
                {
                    if (stage & (RUN_CMD | RUN_CONSOLE | RUN_SHELL | RUN_JSON | RUN_LUA | RUN_PYTHON | OUT_JSON_FILE | RUN_JSON_CASE_ALL))
                        break;
                    stage |= RUN_JSON_CASE_ALL;
                }
                else if (!strcmp(long_options[option_index].name, "run-false"))
                {
                    if (stage & (RUN_CMD | RUN_CONSOLE | RUN_SHELL | RUN_JSON | RUN_LUA | RUN_PYTHON | OUT_JSON_FILE | RUN_JSON_CASE_FALSE))
                        break;
                    stage |= RUN_JSON_CASE_FALSE;
                }
                else if (!strcmp(long_options[option_index].name, "run-random"))
                {
                    if (stage & (RUN_CMD | RUN_CONSOLE | RUN_SHELL | RUN_JSON | RUN_LUA | RUN_PYTHON | OUT_JSON_FILE | RUN_JSON_CASE_RAMDOM))
                        break;
                    stage |= RUN_JSON_CASE_RAMDOM;
                }
                else if (!strcmp(long_options[option_index].name, "gen-report"))
                {
                    if (stage & (RUN_CMD | RUN_CONSOLE | RUN_SHELL | RUN_JSON | RUN_LUA | RUN_PYTHON | OUT_JSON_FILE | GENERATE_REPORT_FILE))
                        break;
                    stage |= GENERATE_REPORT_FILE;
                }
                else if (!strcmp(long_options[option_index].name, "stage"))
                {
                    if (stage & (RUN_CMD | RUN_CONSOLE | OUT_JSON_CASE_STAGE | CONVERT_TO_JSON))
                        break;
                    if (optarg[0] == '=')
                        optarg++;
                    console_json_out.case_stage = case_stage = ex_atoi(optarg);
                    stage |= OUT_JSON_CASE_STAGE;
                }
                else if (!strcmp(long_options[option_index].name, "stage-range"))
                {
                    if (stage & (RUN_CMD | RUN_CONSOLE | OUT_JSON_CASE_STAGE_RANGE))
                        break;
                    if (optarg[0] == '=')
                        optarg++;
                    console_json_out.case_stage_range = ex_atoi(optarg);
                    stage |= OUT_JSON_CASE_STAGE_RANGE;
                }
                else if (!strcmp(long_options[option_index].name, "conv2json"))
                {
                    if (stage & (RUN_CMD | RUN_CONSOLE | CONVERT_TO_JSON))
                        break;
                    console_attr.trans_init = none_transfer_init;
                    console_attr.trans_deinit = none_transfer_deinit;
                    console_attr.send_cmd = none_send_cmd_json_out;
                    console_attr.grab_tab_list = none_grab_tab_list;
                    console_json_out.case_stage = -1;
                    stage |= CONVERT_TO_JSON;
                }
                else if (!strcmp(long_options[option_index].name, "script-path"))
                {
                    if (stage & (RUN_CMD | RUN_CONSOLE | RUN_SHELL | RUN_JSON | RUN_LUA | RUN_PYTHON | OUT_JSON_FILE | OUT_JSON_PATH))
                        break;
                    if (optarg[0] == '=')
                        optarg++;
                    script_path  = optarg;
                    stage |= OUT_JSON_PATH;
                }
                else if (!strcmp(long_options[option_index].name, "append-cmds"))
                {
                    if (stage & (RUN_CMD | RUN_CONSOLE | APPEND_CMDS_IN_JSON))
                        break;
                    console_json_out.do_append_cmds = 1;
                    stage |= APPEND_CMDS_IN_JSON;
                }
            }
            break;
            case 'r':
            {
                int i = 0;

                if (stage & (RUN_CMD | RUN_CONSOLE | RUN_SHELL | RUN_JSON | RUN_LUA | RUN_JSON_CASE | RUN_JSON_CASE_ALL | RUN_JSON_CASE_FALSE | RUN_JSON_CASE_RAMDOM | GENERATE_REPORT_FILE))
                    break;
                if (optarg[0] == '-')
                    break;

                in_para_strs += optarg;
                for (i = 0; i < argc - optind; i++)
                {
                    if (argv[optind + i][0] == '-')
                    {
                        break;
                    }
                    in_para_strs += " ";
                    in_para_strs += argv[optind + i];
                }
                stage |= RUN_CMD;
            }
            break;
            case 'p':
            {
                if (stage & (RUN_CMD | RUN_CONSOLE | RUN_SHELL | RUN_JSON | RUN_LUA | RUN_PYTHON))
                    break;
                if (optarg[0] == '=')
                    optarg++;
                script_file = optarg;
                stage |= RUN_PYTHON;
            }
            break;
            case 's':
            {
                if (stage & (RUN_CMD | RUN_CONSOLE | RUN_SHELL | RUN_JSON | RUN_LUA | RUN_PYTHON))
                    break;
                if (optarg[0] == '=')
                    optarg++;
                script_file = optarg;
                stage |= RUN_SHELL;
            }
            break;
            case 'u':
            {
                if (stage & (RUN_CMD | RUN_CONSOLE | RUN_SHELL | RUN_JSON | RUN_LUA | RUN_PYTHON))
                    break;
                if (optarg[0] == '=')
                    optarg++;
                script_file = optarg;
                stage |= RUN_LUA;
            }
            break;
            case 'j':
            {
                if (stage & (RUN_CMD | RUN_CONSOLE | RUN_SHELL | RUN_JSON | RUN_LUA | RUN_PYTHON))
                    break;
                if (optarg[0] == '=')
                    optarg++;
                script_file = optarg;
                stage |= RUN_JSON;
            }
            break;
            case 'o':
            {
                if (stage & (RUN_CONSOLE | OUT_JSON_FILE))
                    break;
                if (optarg[0] == '=')
                    optarg++;
                out_file = optarg;
                console_json_out.out_file = out_file.c_str();
                stage |= OUT_JSON_FILE;
            }
            break;
            case 't':
            {
                if (stage & (RUN_CONSOLE | OUT_JSON_REPORT_FILE))
                    break;
                if (optarg[0] == '=')
                    optarg++;
                out_report_file = optarg;
                stage |= OUT_JSON_REPORT_FILE;
            }
            break;
            case 'l':
            {
                if (stage & (RUN_CONSOLE | LOG_OPTION))
                    break;
                if (optarg[0] == '=')
                    optarg++;
                console_json_out.log_opt = ex_atoi(optarg);
                stage |= LOG_OPTION;
            }
            break;
            case 'n':
            {
                if (stage & (RUN_CONSOLE | OUT_JSON_CASE_NAME))
                    break;
                if (optarg[0] == '=')
                    optarg++;
                case_name = optarg;
                console_json_out.case_name = case_name.c_str();
                stage |= OUT_JSON_CASE_NAME;
            }
            break;
            default:
                break;
        }
    }
    if ((stage & (CONNECT_SOCKET | CONNECT_SHM)) && in_para_strs.size())
        return ss_command(&console_attr, console_key.c_str(), in_para_strs.c_str());
    if ((stage & (CONNECT_SOCKET | CONNECT_SHM)) && (stage & RUN_CONSOLE))
        return ss_console(&console_attr, console_key.c_str());
    if ((stage & (CONNECT_SOCKET | CONNECT_SHM | CONVERT_TO_JSON)) && (RUN_PYTHON & stage))
        return ss_python(&console_attr, console_key.c_str(), script_file.c_str(), &console_json_out);
    if ((stage & (CONNECT_SOCKET | CONNECT_SHM | CONVERT_TO_JSON)) && (RUN_SHELL & stage))
        return ss_shell(&console_attr, console_key.c_str(), script_file.c_str(), &console_json_out);
    if ((stage & (CONNECT_SOCKET | CONNECT_SHM | CONVERT_TO_JSON)) && (RUN_LUA & stage))
        return ss_lua(&console_attr, console_key.c_str(), script_file.c_str(), &console_json_out);
    if ((stage & (CONNECT_SOCKET | CONNECT_SHM | CONVERT_TO_JSON)) && (RUN_JSON & stage))
        return ss_json(&console_attr, console_key.c_str(), script_file.c_str(), &console_json_out);
    if ((stage & (CONNECT_SOCKET | CONNECT_SHM)) && (stage & RUN_JSON_CASE) && (stage & OUT_JSON_CASE_STAGE)
        && !out_report_file.empty())
        return ss_run_json_case(&console_attr, console_key.c_str(), out_report_file.c_str(), case_name.c_str(), script_path.c_str(), case_stage, console_json_out.log_opt);
    if ((stage & (CONNECT_SOCKET | CONNECT_SHM)) && (stage & RUN_JSON_CASE_FALSE) && (stage & OUT_JSON_CASE_STAGE)
        && !out_report_file.empty())
        return ss_run_json_case_false(&console_attr, console_key.c_str(), out_report_file.c_str(), script_path.c_str(), case_stage, console_json_out.log_opt);
    if ((stage & (CONNECT_SOCKET | CONNECT_SHM)) && (stage & RUN_JSON_CASE_ALL) && (stage & OUT_JSON_CASE_STAGE)
        && !out_report_file.empty())
        return ss_run_json_case_all(&console_attr, console_key.c_str(), out_report_file.c_str(), script_path.c_str(), case_stage, console_json_out.log_opt);
    if ((stage & (CONNECT_SOCKET | CONNECT_SHM)) && (stage & RUN_JSON_CASE_RAMDOM) && (stage & OUT_JSON_CASE_STAGE)
        && !out_report_file.empty())
        return ss_run_json_case_random(&console_attr, console_key.c_str(), out_report_file.c_str(), script_path.c_str(), case_stage, console_json_out.log_opt);
    if ((stage & GENERATE_REPORT_FILE) && !out_report_file.empty())
        return ss_generate_report_file(script_path.c_str(), out_report_file.c_str());

    print_help((const char *)argv[0]);

    return 0;
}
