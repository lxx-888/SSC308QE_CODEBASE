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
#include <assert.h>
#include <getopt.h>
#include <pthread.h>
#include <poll.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstddef>
#include <stdexcept>
#include "ss_handle.h"
#include "ss_log.h"
#ifdef PCIE_RPMSG_ROUTE
#include <list>
#include "sstar_rpmsg.h"
#endif
#include "ss_cmd_base.h"
#include "ss_console.h"
#include "ss_auto_lock.h"


#define SEL_WAIT_TIMEOUT_MS 200
enum UI_FUNCTION
{
    START_SOCKET = 0x1,
    START_SHM = 0x2,
    START_RPMSG = 0x4,
    RUN_CMD = 0x8,
    SET_CHIP = 0x10,
    SET_PCIE_MODE = 0x20,
    RUN_PYTHON = 0x40,
    RUN_SHELL = 0x80,
    RUN_JSON = 0x100,
    RUN_JSON_CASE = 0x200,
    RUN_JSON_CASE_ALL = 0x400,
    RUN_JSON_CASE_FALSE = 0x800,
    RUN_JSON_CASE_RAMDOM = 0x1000,
    OUT_JSON_FILE = 0x2000,
    OUT_JSON_REPORT_FILE= 0x4000,
    LOG_OPTION = 0x8000,
    OUT_JSON_CASE_NAME = 0x10000,
    OUT_JSON_CASE_STAGE = 0x20000,
    OUT_JSON_CASE_STAGE_RANGE = 0x40000,
    OUT_JSON_PATH = 0x80000,
    CONVERT_TO_JSON = 0x100000,
    GENERATE_REPORT_FILE = 0x200000,
    APPEND_CMDS_IN_JSON = 0x4000000,
    LOG_TRANSFER = 0x8000000,
    RUN_LUA = 0x10000000,
};

static pthread_mutex_t cmd_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
#ifdef PCIE_RPMSG_ROUTE
static pthread_mutex_t cmd_rpmsg_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
#endif
struct shm_catcher
{
    pthread_t server_thread;
    unsigned char exit_flag;
    int key;
};
struct shm_st
{
    unsigned int magic;
    unsigned int ref;
    unsigned char bwrite;
    unsigned int size;
    char buf[TRANS_BUFFER];
};
static struct shm_catcher shm_server_catcher;
static const string &get_prog_name(void);
static const string &get_prog_path(void);
class cmd_env
{
    public:
        explicit cmd_env(ss_result *result, pthread_mutex_t *mutex)
            : auto_mutex(mutex), result_data(result)
        {
            if (auto_mutex)
            {
                pthread_mutex_lock(auto_mutex);
            }
            if (result_data)
            {
                sslog.set_data(result_data);
                sslog.redirect();
            }
        }
        virtual ~cmd_env()
        {
            if (result_data)
            {
                sslog.restore();
                sslog.set_data(NULL);
            }
            if (auto_mutex)
            {
                pthread_mutex_unlock(auto_mutex);
            }
        }
        pthread_mutex_t *auto_mutex;
        ss_result *result_data;
};
class child_shm
{
public:
    child_shm(const string &key) : p_shm_config(nullptr), child_pid(-1), child_killed(false)
    {
        client_shmid = shmget((key_t)ss_cmd_atoi(key.c_str()), sizeof(struct shm_st), 0666|IPC_CREAT);
        if(client_shmid == -1)
        {
            throw std::invalid_argument("shmget failed.");
        }
        p_shm_config = (struct shm_st *)shmat(client_shmid, (void *)0, 0);
        if(p_shm_config == (void *)-1 || p_shm_config == NULL)
        {
            throw std::invalid_argument("shmget failed.");
        }
        memset(p_shm_config, 0, sizeof(struct shm_st));
        p_shm_config->magic = 88860611;
        p_shm_config->ref++;
        if (child_pid != -1)
        {
            sslog.lv_set(PRINT_LV_ERROR) << "child had beed creaded!" << "" << COLOR_ENDL;
        }
        child_pid = vfork();
        if (child_pid == -1)
        {
            throw std::invalid_argument("create child failed.");
        }
        if (child_pid == 0)
        {
            /* child */
            if (execlp(get_prog_path().c_str(), get_prog_path().c_str(),
                       "--shm", key.c_str(), sslog.get_flag(PRINT_LOG_REMOTE) ?
                       "--log-transfer=all" : "--log-transfer=keywords", NULL) != 0)
            {
                perror("execlp error!");
            }
            _exit(0);
        }
        sslog.lv_set(PRINT_LV_DEBUG) << "Create child pid: " << child_pid << COLOR_ENDL;
    }
    void use_current()
    {
        current = this;
    }
    void kill_child(int sig, bool b_child_exit)
    {
        kill(child_pid, sig);
        child_killed = b_child_exit;
    }
    static void unuse_current()
    {
        current = nullptr;
    }
    static bool try_send_cmd(const char *cmd)
    {
        if (!current)
        {
            return false;
        }
        const char *ptr = cmd;
        while (*ptr == ' ')
        {
            ptr++;
        }
        if (!strncmp(ptr, "leave_child_process", strlen("leave_child_process"))
            || !strncmp(ptr, "access_child_process", strlen("access_child_process")))
        {
            return false;
        }
        return true;
    }
    static int send_cmd(const char *cmd)
    {
        return current->send_cmd_internal(cmd, strlen(cmd) + 1);
    }
    virtual ~child_shm()
    {
        if (current == this)
        {
            current = nullptr;
        }
        if (child_pid == -1)
        {
            return;
        }
        int status;
        if (!child_killed && waitpid(child_pid, &status, WNOHANG) == 0)
        {
            send_cmd_internal("stop_shm", strlen("stop_shm") + 1);
        }
        waitpid(child_pid, &status, 0);
        if (WIFEXITED(status))
        {
            sslog.lv_set(PRINT_LV_DEBUG) << "Wait child pid " << child_pid << " done, exit: " << status << COLOR_ENDL;
        }
        memset(p_shm_config, 0, sizeof(struct shm_st ));
        shmdt(p_shm_config);
        shmctl(client_shmid, IPC_RMID, NULL);
    }
private:
    int send_cmd_internal(const char *cmd, unsigned int size)
    {
        unsigned int write_size = 0;
        unsigned int write_total = 0;
        stringstream out_ss;
        while (1)
        {
            write_size = (size - write_total > TRANS_BUFFER ? TRANS_BUFFER : size - write_total);
            memcpy(p_shm_config->buf, cmd + write_total, write_size);
            write_total += write_size;
            p_shm_config->size = write_size;
            if (write_total == size)
            {
                break;
            }
            write_sync();
        }
        while (1)
        {
            write_sync();
            out_ss.write(p_shm_config->buf, p_shm_config->size);
            if (p_shm_config->size < TRANS_BUFFER)
            {
                break;
            }
        }
        nlohmann::json json_target;
        out_ss >> json_target;
        string ss_log = json_target["l"].get<string>();
        sslog << ss_log;
        int ret = json_target["r"];
        return ret;
    }
    void write_sync()
    {
        p_shm_config->bwrite = 1;
        while (p_shm_config->bwrite == 1)
        {
            usleep(1000);
        }
    }
    struct shm_st *p_shm_config;
    int child_pid;
    bool child_killed;
    key_t client_shmid;
    static child_shm *current;
};
class shm_child_resource
{
public:
    shm_child_resource()
    {
        signal(SIGUSR1, shm_process_exit);
    }
    ~shm_child_resource()
    {
        // Clear the handled objects.
        ss_handle_template<child_shm>::clear();
    }
private:
    static void shm_process_exit(int sig)
    {
        // mark the exit flag.
        ss_handle_template<child_shm>::clear();
        if (shm_server_catcher.server_thread)
        {
            shm_server_catcher.exit_flag = 1;
            pthread_join(shm_server_catcher.server_thread, NULL);
        }
        std::cout << "Exit by user kill -10" << std::endl;
        _exit(0);
    }
};
child_shm * child_shm::current = nullptr;

template <class T>
std::map<std::string, T *> ss_handle_template<T>::handle_map;

#define CMD_AUTO_LOCK(__result, __b_lock) cmd_env lock(__result, __b_lock ? &cmd_mutex : NULL)
#define CMD_RPMSG_AUTO_LOCK(__result) cmd_env lock(__result, &cmd_rpmsg_mutex)
#define VTRANSFER_BUF(__buf, __fmt) do {                                                \
                                        va_list args;                                   \
                                        va_start(args, __fmt);                          \
                                        vsnprintf(__buf, TRANS_BUFFER, __fmt, args);    \
                                        va_end(args);                                   \
                                    }while(0)
static int relative_call(ss_result *result, bool b_use_lock, const char *buffer);
static int absolute_call(unsigned int chip_id, ss_result *result, bool b_use_lock, const char *buffer);
static int find_and_check_cmd(vector<string> &in_strs, struct base_command_data &cmd_data)
{
    map<string, struct base_command_data>::iterator it;

    if (in_strs.size() < 1)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "CMD ERROR:" << endl << PRINT_COLOR_END;
        return -1;
    }
    it = ss_cmd_base::get_data().find(in_strs[0]);
    if (it == ss_cmd_base::get_data().end())
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Not fount CMD:[" << in_strs[0]  << ']' << endl << PRINT_COLOR_END;
        return -1;
    }
    if ((!it->second.is_variable_arg && it->second.argc_count != in_strs.size() - 1)
        || (it->second.is_variable_arg && it->second.argc_count > in_strs.size() - 1))
    {
        sslog.lv_set(PRINT_LV_ERROR) << "CMD:[" << in_strs[0] << ']' << ",PARA_ERR!" << endl;
        sslog << "ERR PARA: ";
        for (unsigned int i = 0; i < in_strs.size(); i++)
        {
            sslog << in_strs[i] << ' ';
        }
        sslog << endl;
        sslog << PRINT_COLOR_END;
        sslog << it->second.help;
        return -1;
    }
    cmd_data = it->second;
    return 0;
}
static const string &get_prog_name(void)
{
    return ss_cmd_base::get_name();
}
static const string &get_prog_path(void)
{
    return ss_cmd_base::get_path();
}
static void sel_chip(unsigned int id)
{
    ss_cmd_base::set_chip(id);
}
static unsigned int get_chip(void)
{
    return ss_cmd_base::get_chip();
}
static void dump_cmd_for_help(const char *cmd)
{
    map<string, struct base_command_data>::iterator it;

    if (!cmd)
    {
        //DUMP ALL
        for (it = ss_cmd_base::get_data().begin(); it != ss_cmd_base::get_data().end(); ++it)
        {
            if (it->second.help.size())
            {
                sslog << it->second.help;
            }
        }
        return;
    }
    it = ss_cmd_base::get_data().find(cmd);
    if (it != ss_cmd_base::get_data().end())
    {
        if (it->second.help.size())
        {
            sslog << it->second.help;
        }
    }
}
static inline void parse_str(const char *str, vector<string> &in_strs)
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
        while (*str1 != '\0' && *(str1) == ' ')
        {
            str1++;
        }
        str2 = str1;
        if (*str2 == '\"')
        {
            while (*str2 != '\0' && *(++str2) != '\"');
            if (*str2 != '\"')
            {
                break;
            }
            else
            {
                size = (str2++) - (str1++) - 1;
            }
        }
        else if (*str2 == '\'')
        {
            while (*str2 != '\0' && *(++str2) != '\'');
            if (*str2 != '\'')
            {
                break;
            }
            else
            {
                size = (str2++) - (str1++) - 1;
            }
        }
        else
        {
            while (*str2 != '\0' && *(++str2) != ' ');
            size = str2 - str1;
        }
        if (size != 0 || *str2 != '\0')
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

static int none_send_cmd_json(long fd, const char *cmd, unsigned int size, nlohmann::json &json_out)
{
    json_out["m"] = "";
    json_out["l"] = "";
    json_out["a"] = "";
    json_out["t"] = "";
    json_out["c"] = "";
    json_out["r"] = -1;
    return 0;
}

static int none_grab_tab_list(long fd, string &str)
{
    return 0;
}


static long console_transfer_init(const char *key)
{
    return 0;
}

static int console_transfer_deinit(long fd)
{
    return 0;
}

static int console_send_cmd_json(long fd, const char *cmd, unsigned int size, nlohmann::json &json_out)
{
    ss_result result(json_out);
    int ret = relative_call(&result, true, cmd);
    result.to_json(ret);
    return 0;
}

static int console_grab_tab_list(long fd, string &str)
{
    if (!ss_cmd_base::get_chip())
    {
        map<string, struct base_command_data>::iterator it;

        for (it = ss_cmd_base::get_data().begin(); it != ss_cmd_base::get_data().end(); ++it)
        {
            str += it->first;
            str += "/";
        }
    }

    return 0;
}

static void *shm_catcher_thread(void *args)
{
    struct shm_st *p_shm_config = NULL;
    struct shm_catcher *catcher = (struct shm_catcher *)args;
    key_t shmid = 0;
    unsigned int logs_trans_left_len = 0, log_trans_size = 0;
    stringstream str_trans;
    string cmd_str;

    prctl(PR_SET_NAME, (unsigned long)"cmd_base_shm");
    shmid = shmget((key_t)catcher->key, sizeof(struct shm_st), 0666|IPC_CREAT);
    if(shmid == -1)
    {
        perror("shmget");
        return NULL;
    }
    p_shm_config = (struct shm_st *)shmat(shmid, (void *)0, 0);
    if(p_shm_config == (void *)-1 || p_shm_config == NULL)
    {
        perror("shmat");
        return NULL;
    }
    if (p_shm_config->magic != 88860611)
    {
        cout << "New shm, Key: " << catcher->key << endl;
        memset(p_shm_config, 0, sizeof(struct shm_st));
        p_shm_config->magic = 88860611;
    }
    p_shm_config->ref++;
    while (!catcher->exit_flag)
    {
        if (p_shm_config->bwrite == 1)
        {
            if (!logs_trans_left_len)
            {
                cmd_str.append(p_shm_config->buf, p_shm_config->size);
                if (p_shm_config->size == TRANS_BUFFER)
                {
                    p_shm_config->bwrite = 0;
                    continue;
                }
                nlohmann::json json_target;
                ss_result result(json_target);
                int ret = relative_call(&result, true, cmd_str.c_str());
                cmd_str.clear();
                result.to_json_str(ret, str_trans);
                logs_trans_left_len = str_trans.str().size();
            }
            log_trans_size = logs_trans_left_len >= TRANS_BUFFER ? TRANS_BUFFER : logs_trans_left_len;
            memcpy(p_shm_config->buf, str_trans.str().c_str() + str_trans.str().size() - logs_trans_left_len,
                   log_trans_size);
            p_shm_config->size = log_trans_size;
            p_shm_config->bwrite = 0;
            logs_trans_left_len -= log_trans_size;
        }
        usleep(1000);
    }
    p_shm_config->ref--;
    if (!p_shm_config->ref)
    {
        memset(p_shm_config, 0, sizeof(struct shm_st));
        if (shmdt(p_shm_config) == -1)
        {
            perror("shmdt");
            return NULL;
        }
        if (shmctl(shmid, IPC_RMID, NULL) == -1)
        {
            cout << "Destroy share memory failed!" << endl;
            return NULL;
        }
        cout << "Del shm, Key: " << catcher->key << endl;
        cout << "Stop shm server!" << endl;
        return NULL;
    }
    if (shmdt(p_shm_config) == -1)
    {
        cout << "shmdt failed." << endl;
        return NULL;
    }
    cout << "Stop shm server!" << endl;
    return NULL;
}
static int file_desc_do_read(int fd, char *p_trans_data, unsigned int size, bool &b_end)
{
    int ret = 0;
    unsigned data_offset = 0;

    b_end = false;
    while(1)
    {
        ret = read(fd, p_trans_data + data_offset, size - data_offset);
        if (ret == 0)
        {
            continue;
        }
        if (ret < 0)
        {
            perror("read");
            return ret;
        }
        data_offset += ret;
        if (p_trans_data[data_offset - 1] == '\0')
        {
            b_end = true;
            //C++ string append not contain '\0'
            data_offset--;
            break;
        }
        if (size == data_offset)
        {
            break;
        }
    }
    return data_offset;
}
static int file_desc_do_write(int fd, const char *p_trans_data, unsigned int size)
{
    int ret = 0;
    unsigned int data_offset = 0;
    unsigned trans_size;

    while(1)
    {
        trans_size = size - data_offset;
        ret = write(fd, p_trans_data + data_offset, (trans_size > TRANS_BUFFER) ? TRANS_BUFFER : trans_size);
        if (ret == 0)
        {
            continue;
        }
        if (ret < 0)
        {
            cout << "write ret " << ret << endl;
            return ret;
        }
        data_offset += ret;
        if ((size == data_offset) || p_trans_data[data_offset - 1] == '\0')
        {
            break;
        }
    }

    return data_offset;
}
static int file_desc_server(int fd, unsigned char &is_exit, int (*run_cmd_cb)(ss_result &result, const char *cmd_buffer))
{
    struct timeval tv;
    fd_set fdsr;
    int ret = 0;
    char transfer_buffer[TRANS_BUFFER];
    int fd_flags = 0;
    fd_flags = fcntl(fd, F_GETFL, 0);
    if (fd_flags < 0)
    {
        perror("fcntl");
        return ret;
    }
    ret = fcntl(fd, F_SETFL, fd_flags | O_NONBLOCK);
    if (ret < 0)
    {
        perror("fcntl");
        return ret;
    }
    while (!is_exit)
    {
        tv.tv_sec = 0;
        tv.tv_usec = 1000 * SEL_WAIT_TIMEOUT_MS;
        FD_ZERO(&fdsr);
        FD_SET(fd, &fdsr);
        ret = select(fd + 1, &fdsr, NULL, NULL, &tv);
        switch (ret)
        {
            case -1:
                perror("select");
                return -1;
            case 0:
                break;
            case 1:
            {
                bool b_end = false;
                stringstream str_trans;
                nlohmann::json json_target;
                ss_result result(json_target);
                ret = file_desc_do_read(fd, transfer_buffer, sizeof(transfer_buffer), b_end);
                if (ret <= 0)
                {
                    cout << "file_desc_do_read return: " << dec << ret << endl;;
                    return ret;
                }
                if (b_end)
                {
                    //cout << "RECV: " << transfer_buffer << endl;
                    ret = run_cmd_cb(result, transfer_buffer);
                }
                else
                {
                    string cmd_str;
                    cmd_str = cmd_str.assign(transfer_buffer, ret);
                    do
                    {
                        ret = file_desc_do_read(fd, transfer_buffer, sizeof(transfer_buffer), b_end);
                        if (ret <= 0)
                        {
                            cout << "file_desc_do_read return: " << dec << ret << endl;
                            return ret;
                        }
                        cmd_str.append(transfer_buffer, ret);
                    } while (!b_end);
                    //cout << "RECV: " << cmd_str.c_str() << endl;
                    ret = run_cmd_cb(result, cmd_str.c_str());
                }
                result.to_json_str(ret, str_trans);
                ret = file_desc_do_write(fd, str_trans.str().c_str(), str_trans.str().size() + 1);
                if (ret < 0)
                {
                    perror("write");
                    return ret;
                }
            }
            break;
        }
    }
    return ret;
}

struct socket_catcher
{
    pthread_t server_thread;
    unsigned char exit_flag;
    unsigned char reset_flag;
    int server_sockfd;
    struct sockaddr_in server_address;
};
static struct socket_catcher socket_server_catcher;
static int socket_accept_nonblock(const int &server_fd, struct sockaddr *server_address, int &client_fd)
{
    fd_set fdsr;
    int ret = 0;
    struct timeval tv;

    tv.tv_sec = 0;
    tv.tv_usec = 1000 * SEL_WAIT_TIMEOUT_MS;
    FD_ZERO(&fdsr);
    FD_SET(server_fd, &fdsr);
    ret = select(server_fd + 1, &fdsr, NULL, NULL, &tv);
    switch (ret)
    {
        case -1:
            perror("select");
            return -1;
        case 0:
            client_fd = -1;
            return 0;
        default:
            if (FD_ISSET(server_fd, &fdsr))
            {
                int client_len = sizeof (struct sockaddr_in);
                ret = accept(server_fd, server_address, (socklen_t *)&client_len);
                if (ret == -1)
                {
                    if (errno == ECONNABORTED)
                    {
                        client_fd = -1;
                        return 0;
                    }
                    perror("accept");
                    return ret;
                }
                client_fd = ret;
            }
            break;
    }
    return ret;
}
static int file_desc_catcher_run_cmd(ss_result &result, const char *cmd_buffer)
{
    return relative_call(&result, true, cmd_buffer);
}
static void *socket_catcher_thread(void *args)
{
    int client_sockfd;
    struct socket_catcher *catcher = (struct socket_catcher *)args;
    int fd_flags = 0, ret = 0;

    prctl(PR_SET_NAME, (unsigned long)"cmd_base_socket");

    fd_flags = fcntl(catcher->server_sockfd, F_GETFL, 0);
    if (fd_flags < 0)
    {
        perror("fcntl");
        return NULL;
    }
    ret = fcntl(catcher->server_sockfd, F_SETFL, fd_flags | O_NONBLOCK);
    if (ret < 0)
    {
        perror("fcntl");
        return NULL;
    }
    cout << "Server is waiting for client connect...\n";
    while (!catcher->exit_flag)
    {
        ret = socket_accept_nonblock(catcher->server_sockfd, (struct sockaddr *)&catcher->server_address, client_sockfd);
        if (ret == 0 && client_sockfd == -1)
        {
            continue;
        }
        if (ret == -1)
        {
            cout << "socket accept error!" << endl;
            return NULL;
        }
        cout << "Connect client fd " << client_sockfd << endl;
        file_desc_server(client_sockfd, catcher->reset_flag, file_desc_catcher_run_cmd);
        cout << "Disconnect client fd " << client_sockfd << endl;
        close(client_sockfd);
        catcher->reset_flag = 0;
    }
    close(catcher->server_sockfd);
    unlink (SOCKET_ADDR);
    cout << "Stop socket server!" << endl;

    return NULL;
}
#ifdef PCIE_RPMSG_ROUTE
#define USER_EP_ADDR 66
#define USER_RC_ADDR 88
#define USER_SERVER_MASK 0x100
struct rpmsg_catcher
{
    pthread_t server_thread;
    unsigned char exit_flag;
    int server_fd;
};
static list<struct rpmsg_catcher> rpmsg_server_catcher;
map<unsigned int, int> map_client_fd;
static int pcie_rpmsg_dev_open(unsigned int remote_chip_id, bool is_src_server)
{
    int fd = open("/dev/rpmsg_ctrl0", O_RDWR);
    struct ss_rpmsg_endpoint_info info;
    stringstream ss_path;
    int open_retry_cout = 10;

    //Low 16 bit of remote_chip_id is target_id for mi_pcie/rpmsg driver.
    //App use high 16bit with mask PCIE_EP_CHIP_ROUTE_MASK to adjust whether EP/RC.

    if (fd < 0)
    {
        ss_print(PRINT_LV_ERROR, "RPMSG device open error.\n");
        return -1;
    }
    memset(&info, 0, sizeof(struct ss_rpmsg_endpoint_info));
    snprintf(info.name, sizeof(info.name), "sstar");
    if (PCIE_EP_CHIP_ROUTE_MASK & remote_chip_id)
    {
        //If local is ep, so remote must be rc, otherwise is the same.
        info.src = (USER_EP_ADDR & 0x3FFFFFFF) | 0x40000000 | (is_src_server ? USER_SERVER_MASK : 0); //bit[31]=0, bit[30]=1
        info.dst = (USER_RC_ADDR & 0x3FFFFFFF) | 0x40000000 | (is_src_server ? 0 : USER_SERVER_MASK);
        info.mode = RPMSG_MODE_EP_PORT;
    }
    else
    {
        info.src = (USER_RC_ADDR & 0x3FFFFFFF) | 0x40000000 | (is_src_server ? USER_SERVER_MASK : 0); //bit[31]=0, bit[30]=1
        info.dst = (USER_EP_ADDR & 0x3FFFFFFF) | 0x40000000 | (is_src_server ? 0 : USER_SERVER_MASK);
        info.mode  = RPMSG_MODE_RC_PORT;
    }
    info.target_id = remote_chip_id & 0xFF;
    cout << "Mode " << (info.mode == RPMSG_MODE_EP_PORT ? "EP" : "RC") << " Tgt_Id: " << info.target_id << endl;
    if (ioctl(fd, SS_RPMSG_CREATE_EPT_IOCTL, &info) < 0)
    {
        ss_print(PRINT_LV_ERROR, "Failed to open /dev/rpmsg_ctrl0!\n");
        close(fd);
        return -1;
    }
    close(fd);
    do
    {
        cout << "sleep 0.5s" << endl;
        usleep(500 * 1000);
        ss_path.str("");
        ss_path << "/dev/rpmsg" << (int)info.id;
        cout << "Open: " << ss_path.str() << endl;
        fd = open(ss_path.str().c_str(), O_RDWR);
        if (fd < 0)
        {
            if (open_retry_cout)
            {
                cout << "RETRY:" << dec << open_retry_cout << endl;
                open_retry_cout--;
                continue;
            }
            perror("open");
            ss_print(PRINT_LV_ERROR, "%s open error.\n", ss_path.str().c_str());
            return -1;
        }
        break;
    }while (1);

    return fd;
}
static int pcie_rpmsg_get_client(unsigned int remote_chip_id)
{
    int fd = 0;

    if (map_client_fd.find(remote_chip_id) == map_client_fd.end())
    {
        fd = pcie_rpmsg_dev_open(remote_chip_id, false);
        if (fd < 0)
        {
            return -1;
        }
        map_client_fd[remote_chip_id] = fd;
    }
    else
    {
        fd = map_client_fd[remote_chip_id];
    }
    return fd;
}
static inline int pcie_rpmsg_transfer(unsigned int remote_chip_id, int fd, const char *buffer, const char *cmd_tittle, bool cmd_b_lock)
{
    stringstream trans_msg;
    stringstream out_ss;
    int ret = 0;
    char ret_log_buf[TRANS_BUFFER];
    bool b_end = false;

    trans_msg << cmd_tittle << ' ' << (int)((remote_chip_id >> 8) & 0xFF) << ' ' << (int)(cmd_b_lock ? 1 : 0) << " \'" << buffer << '\'';
    if (write(fd, trans_msg.str().c_str(), trans_msg.str().size() + 1) < 0)
    {
        perror("write");
        return -1;
    }
    while (1)
    {
        ret = file_desc_do_read(fd, ret_log_buf, TRANS_BUFFER, b_end);
        if (ret <= 0)
        {
            return -1;
        }
        //Store string and find the last return logs.
        out_ss.write(ret_log_buf, ret);
        if (b_end)
        {
            nlohmann::json json_target;
            out_ss >> json_target;
            string ss_log = json_target["l"].get<string>();
            sslog << ss_log;
            //sslog.store_md5() << json_target["m"].dump(0);
            //sslog.client_pause() = json_target["a"];
            ret = json_target["r"];
            break;
        }
    }
    return ret;
}
static int rpmsg_catcher_run_cmd(ss_result &result, const char *cmd_buffer)
{
    vector<string> in_str_cmd_paras;
    unsigned int chip_id = 0;

    parse_str(cmd_buffer, in_str_cmd_paras);
    if (in_str_cmd_paras.size() != 4)
    {
        result.out_log() << "PARA ERROR!" << endl;
        return -1;
    }
    chip_id = ss_cmd_atoi(in_str_cmd_paras[1].c_str());
    if (in_str_cmd_paras[0] == "absolute_call")
    {
        return absolute_call(chip_id, &result, (bool)ss_cmd_atoi(in_str_cmd_paras[2].c_str()), in_str_cmd_paras[3].c_str());
    }
    if (in_str_cmd_paras[0] == "relative_call")
    {
        if (!chip_id)
        {
            return relative_call(&result, (bool)ss_cmd_atoi(in_str_cmd_paras[2].c_str()), in_str_cmd_paras[3].c_str());
        }
        CMD_RPMSG_AUTO_LOCK(&result);
        //Rpmsg transfer command:
        int fd = pcie_rpmsg_get_client(chip_id);
        if (fd < 0)
        {
            return -1;
        }
        return pcie_rpmsg_transfer(chip_id, fd, in_str_cmd_paras[3].c_str(), "relative_call", (bool)ss_cmd_atoi(in_str_cmd_paras[2].c_str()));
    }
    result.out_log() << "PARA TITTLE ERROR!" << endl;
    return -1;

}
static void *rpmsg_catcher_thread(void *args)
{
    struct rpmsg_catcher *catcher = (struct rpmsg_catcher *)args;

    prctl(PR_SET_NAME, (unsigned long)"cmd_base_rpmsg");
    file_desc_server(catcher->server_fd, catcher->exit_flag, rpmsg_catcher_run_cmd);
    ioctl(catcher->server_fd, SS_RPMSG_DESTROY_EPT_IOCTL);
    close(catcher->server_fd);
    cout << "Stop rpmsg server!" << endl;

    return NULL;
}
static int pcie_rpmsg_get_info(vector<unsigned int> &remote_chip_ids, int mode, int cnt)
{
    struct ss_rpmsg_devices_info info;
    unsigned int *tgt_id = NULL;
    int fd = 0;
    int timeout_cnt = 100;


    if (mode != RPMSG_MODE_RC_PORT && mode != RPMSG_MODE_EP_PORT)
    {
        ss_print(PRINT_LV_ERROR, "type error!\n");
        return -1;
    }
    fd = open("/dev/rpmsg_ctrl0", O_RDWR);
    if (fd < 0)
      return -1;

    memset(&info, 0, sizeof(struct ss_rpmsg_devices_info));
    tgt_id = (unsigned int *)malloc(info.count * sizeof(unsigned int));
    assert(tgt_id);
    info.mode = mode;
    info.count = 256;
    info.buffer = (unsigned long long)tgt_id;
    do
    {
        if (ioctl(fd, SS_RPMSG_DEVICES_INFO_IOCTL, &info) < 0)
        {
            cout << "RPMSG Get device Info fail!\n" << endl;
            close(fd);
            free(tgt_id);
            return -1;
        }
        usleep(200 * 1000);
        timeout_cnt--;
        if (!timeout_cnt)
        {
            cout << "Connect time out!" << endl;
            close(fd);
            free(tgt_id);
            return -1;
        }
    } while (info.count < (unsigned int)cnt);
    cout << "Success to connected slaves of mode " << mode  << endl;
    for (unsigned int i = 0; i < info.count; i++)
    {
        cout << "push back tgt id: " << tgt_id[i] << endl;
        remote_chip_ids.push_back(tgt_id[i]);
    }
    close(fd);
    free(tgt_id);
    cout << "sleep 0.5s" << endl;
    usleep(500 * 1000);

    return remote_chip_ids.size();
}
#endif
static int relative_call(ss_result *result, bool b_use_lock, const char *buffer)
{
    unsigned int chip_id = 0;

    if (result)
        result->out_log().str("");
    chip_id = get_chip();
    if (!chip_id)
    {
        vector<string> in_str_cmd_paras;
        struct base_command_data cmd_data;

        CMD_AUTO_LOCK(result, b_use_lock && (!cmd_data.is_running_nolock));
        if (child_shm::try_send_cmd(buffer))
        {
            return child_shm::send_cmd(buffer);
        }
        parse_str(buffer, in_str_cmd_paras);
        if (find_and_check_cmd(in_str_cmd_paras, cmd_data) == -1)
        {
            return -1;
        }
        return cmd_data.do_act(in_str_cmd_paras);
    }
    if (strstr(buffer, "route_call"))
    {
        vector<string> in_str_cmd_paras;
        struct base_command_data cmd_data;
        int ret = 0;

        parse_str(buffer, in_str_cmd_paras);
        CMD_AUTO_LOCK(result, b_use_lock && (!cmd_data.is_running_nolock));
        if (find_and_check_cmd(in_str_cmd_paras, cmd_data) == -1)
        {
            return -1;
        }
        ret = cmd_data.do_act(in_str_cmd_paras);
        if (ret == 99)
        {
            return 0;
        }
        if (ret ==-1)
        {
            return -1;
        }
    }
#ifdef PCIE_RPMSG_ROUTE
    CMD_RPMSG_AUTO_LOCK(result);
    //Rpmsg transfer command:
    int fd = pcie_rpmsg_get_client(chip_id);
    if (fd < 0)
    {
        return -1;
    }
    return pcie_rpmsg_transfer(chip_id, fd, buffer, "relative_call", b_use_lock);
#else
    return -1;
#endif
}

static int absolute_call(unsigned int chip_id, ss_result *result, bool b_use_lock, const char *buffer)
{
    if (result)
        result->out_log().str("");
    if (!chip_id)
    {
        vector<string> in_str_cmd_paras;
        struct base_command_data cmd_data;

        CMD_AUTO_LOCK(result, b_use_lock && (!cmd_data.is_running_nolock));
        if (child_shm::try_send_cmd(buffer))
        {
            return child_shm::send_cmd(buffer);
        }
        parse_str(buffer, in_str_cmd_paras);
        if (find_and_check_cmd(in_str_cmd_paras, cmd_data) == -1)
        {
            return -1;
        }
        return cmd_data.do_act(in_str_cmd_paras);
    }
#ifdef PCIE_RPMSG_ROUTE
    CMD_RPMSG_AUTO_LOCK(result);
    //Rpmsg transfer command:
    int fd = pcie_rpmsg_get_client(chip_id);
    if (fd < 0)
    {
        return -1;
    }
    return pcie_rpmsg_transfer(chip_id, fd, buffer, "absolute_call", b_use_lock);
#else
    return -1;
#endif
}
static int start_shm_server(std::vector<std::string> &in_strs)
{
    int ret = 0;

    if (shm_server_catcher.server_thread)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Shm had alread started." << COLOR_ENDL;
        return -1;
    }
    cout << "Start shm server!" << endl;
    memset(&shm_server_catcher, 0, sizeof(struct shm_catcher));
    shm_server_catcher.key = ss_cmd_atoi(in_strs[1].c_str());
    ret = pthread_create(&shm_server_catcher.server_thread, NULL,
                         shm_catcher_thread, &shm_server_catcher);
    if (ret < 0)
    {
        return ret;
    }
    return 0;
}
static int stop_shm_server(std::vector<std::string> &in_strs)
{
    shm_server_catcher.exit_flag = 1;
    pthread_join(shm_server_catcher.server_thread, NULL);
    shm_server_catcher.server_thread = 0;
    // If command running in the thread and join will return instantly.
    return 0;
}
static int start_socket_server(std::vector<std::string> &in_strs)
{
    int ret = 0;
    int on = 1;

    if (socket_server_catcher.server_thread)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Socket had alread started." << COLOR_ENDL;
        return -1;
    }
    memset(&socket_server_catcher, 0, sizeof(struct socket_catcher));
    unlink (SOCKET_ADDR);
    socket_server_catcher.server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_server_catcher.server_sockfd == -1)
    {
        perror("socket");
        return -1;
    }
    if (-1 == setsockopt(socket_server_catcher.server_sockfd , SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) ))
    {
        perror("setsockopt");
        close(socket_server_catcher.server_sockfd);

        return -1;
    }
    memset(&socket_server_catcher.server_address, 0, sizeof(sockaddr_in));
    socket_server_catcher.server_address.sin_family = AF_INET;
    socket_server_catcher.server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    socket_server_catcher.server_address.sin_port = htons(8000);
#if defined(__BIONIC__)
    bind(socket_server_catcher.server_sockfd, (struct sockaddr *)&socket_server_catcher.server_address, sizeof(struct sockaddr_in));
#else
    if (-1 == bind(socket_server_catcher.server_sockfd, (struct sockaddr *)&socket_server_catcher.server_address, sizeof(struct sockaddr_in)))
    {
        perror("bind");
        close(socket_server_catcher.server_sockfd);

        return -1;
    }
#endif
    if (-1 == listen(socket_server_catcher.server_sockfd, 5))
    {
        perror("listen");
        close(socket_server_catcher.server_sockfd);

        return -1;
    }
    cout << "Start socket server!" << endl;
    ret = pthread_create(&socket_server_catcher.server_thread, NULL, socket_catcher_thread, &socket_server_catcher);
    if (ret < 0)
    {
        perror("pthread_create");
        close(socket_server_catcher.server_sockfd);

        return ret;
    }

    return 0;
}
static int stop_socket_server(std::vector<std::string> &in_strs)
{
    socket_server_catcher.reset_flag = 1;
    socket_server_catcher.exit_flag = 1;
    pthread_join(socket_server_catcher.server_thread, NULL);
    // If command running in the thread and join will return instantly.
    return 0;
}
static int reset_socket_server(std::vector<std::string> &in_strs)
{
    socket_server_catcher.reset_flag = 1;

    return 0;
}

static int start_rpmsg_server(std::vector<std::string> &in_strs)
{
#ifdef PCIE_RPMSG_ROUTE
    vector<unsigned int> chip_ids;
    struct rpmsg_catcher catcher;
    int ret = 0;
    int mode = ss_cmd_atoi(in_strs[1].c_str());
    int target_cnt = ss_cmd_atoi(in_strs[2].c_str());

    if (!target_cnt)
    {
        ss_print(PRINT_LV_ERROR, "Target count is 0\n");

        return -1;
    }
    ret = pcie_rpmsg_get_info(chip_ids, mode, target_cnt);
    if (ret < 0 || !chip_ids.size())
    {
        ss_print(PRINT_LV_ERROR, "Not enum slave chips.\n");

        return -1;
    }
    rpmsg_server_catcher.clear();
    for (unsigned int i = 0; i < chip_ids.size(); i++)
    {
        auto fd = pcie_rpmsg_dev_open(chip_ids[i] | (mode == RPMSG_MODE_EP_PORT ? PCIE_EP_CHIP_ROUTE_MASK: 0), true);
        if (fd < 0)
        {
            return -1;
        }
        memset(&catcher, 0, sizeof(struct rpmsg_catcher));
        catcher.server_fd = fd;
        rpmsg_server_catcher.push_back(catcher);
        ret = pthread_create(&rpmsg_server_catcher.back().server_thread, NULL, rpmsg_catcher_thread, &rpmsg_server_catcher.back());
        if (ret < 0)
        {
            perror("pthread_create");
            close(rpmsg_server_catcher.back().server_fd);
            rpmsg_server_catcher.pop_back();

            return ret;
        }
    }
    sslog << "Start rpmsg server! mode: " << mode << endl;
#endif

    return 0;
}
static int stop_rpmsg_server(std::vector<std::string> &in_strs)
{
#ifdef PCIE_RPMSG_ROUTE
    if (rpmsg_server_catcher.size())
    {
        for (auto &it : rpmsg_server_catcher)
        {
            it.exit_flag = 1;
        }
        for (auto &it : rpmsg_server_catcher)
        {
            pthread_join(it.server_thread, NULL);
        }
    }
#endif
    sslog << "Stop rpmsg server!" << endl;

    return 0;
}
static int select_chip_id(std::vector<std::string> &in_strs)
{
    unsigned int id = 0;

    id = ss_cmd_atoi(in_strs[1].c_str());
    sel_chip(id);
    sslog.lv_set(PRINT_LV_DEBUG) << "Select chip id: " << "0x" << hex << id << " in console." << "\033[0m" << endl;

    return 0;
}
static int dump_help_module(std::vector<std::string> &in_strs)
{
    auto init_func = ss_cmd_base::get_module_func(in_strs[1]);

    if (!init_func)
    {
        sslog.color_set(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT) << "Did not find module name: " << in_strs[1] << PRINT_COLOR_END << std::endl;
        return -1;
    }
    map<string, struct base_command_data> cmd_data;
    init_func(cmd_data);
    for (auto it_cmd = cmd_data.begin(); it_cmd != cmd_data.end(); ++it_cmd)
    {
        sslog << it_cmd->second.help;
    }
    return 0;
}
static int dump_help(std::vector<std::string> &in_strs)
{
    if (in_strs.size() == 2)
        dump_cmd_for_help(in_strs[1].c_str());
    else if (in_strs.size() == 1)
        dump_cmd_for_help(NULL);

    return 0;
}
static int get_tabs(std::vector<std::string> &in_strs)
{
    map<string, struct base_command_data>::iterator it;

    for (it = ss_cmd_base::get_data().begin(); it != ss_cmd_base::get_data().end(); ++it)
    {
        sslog.store_tab() << it->first;
        sslog.store_tab() << "/";
    }
    return 0;
}

static int wish_call(std::vector<std::string> &in_strs)
{
    int ret = 0;
    int wish_val = ss_cmd_atoi(in_strs[1].c_str());
    struct base_command_data cmd_data;

    in_strs.erase(in_strs.begin());
    in_strs.erase(in_strs.begin());
    if (find_and_check_cmd(in_strs, cmd_data) == -1)
    {
        return -1;
    }
    ret = cmd_data.do_act(in_strs);
    return (ret == wish_val) ? 0 : -1;
}

static int route_call(std::vector<std::string> &in_strs)
{
    //is_route flag determine whether call run_cmd on next chip, if not then return 99;
    unsigned char is_route = 0;
    int ret = 0;
    struct base_command_data cmd_data;

    is_route = ss_cmd_atoi(in_strs[1].c_str());
    in_strs.erase(in_strs.begin());
    in_strs.erase(in_strs.begin());
    if (find_and_check_cmd(in_strs, cmd_data) == -1)
    {
        return -1;
    }
    ret = cmd_data.do_act(in_strs);
    return (is_route) ? ret: 99;
}

static int sync_call(std::vector<std::string> &in_strs)
{
    int ret = 0;
    struct base_command_data cmd_data;

    in_strs.erase(in_strs.begin());
    if (find_and_check_cmd(in_strs, cmd_data) == -1)
    {
        return -1;
    }
    ret = cmd_data.do_act(in_strs);
    sslog.store_msg()["sync_call"] = "ok";
    sslog.store_ask().str("");
    sslog.store_ask() << "catch";
    return ret;
}

static int run_chip_id(std::vector<std::string> &in_strs)
{
    unsigned int chip_id = 0;

    chip_id = ss_cmd_atoi(in_strs[1].c_str());
    return absolute_call(chip_id, NULL, true, in_strs[2].c_str());
}
static int clear_rpmsg_client(std::vector<std::string> &in_strs)
{
#ifdef PCIE_RPMSG_ROUTE
    for (auto iter = map_client_fd.begin(); iter != map_client_fd.end(); ++iter)
    {
        ioctl(iter->second, SS_RPMSG_DESTROY_EPT_IOCTL);
        close(iter->second);
    }
    map_client_fd.clear();
#endif
    return 0;
}
static int wait_server(void)
{
    if (shm_server_catcher.server_thread)
    {
        pthread_join(shm_server_catcher.server_thread, NULL);
        memset(&shm_server_catcher, 0, sizeof(struct shm_catcher));
    }
    if (socket_server_catcher.server_thread)
    {
        pthread_join(socket_server_catcher.server_thread, NULL);
        memset(&socket_server_catcher, 0, sizeof(struct socket_catcher));
    }
#ifdef PCIE_RPMSG_ROUTE
    if (rpmsg_server_catcher.size())
    {
        for (auto &it : rpmsg_server_catcher)
        {
            pthread_join(it.server_thread, NULL);
        }
        rpmsg_server_catcher.clear();
    }
#endif
    return 0;
}
static int run_python(const string &script_file, const struct ss_console_json_out_s &console_json_out, bool conv_json_only)
{
    struct ss_console_attr_s console_attr;

    console_attr.trans_init = console_transfer_init;
    console_attr.trans_deinit = console_transfer_deinit;
    console_attr.send_cmd = conv_json_only ? none_send_cmd_json : console_send_cmd_json;
    console_attr.grab_tab_list = conv_json_only ? none_grab_tab_list : console_grab_tab_list;

    return ss_python(&console_attr, "123", script_file.c_str(), &console_json_out);
}
static int run_shell(const string &script_file, const struct ss_console_json_out_s &console_json_out, bool conv_json_only)
{
    struct ss_console_attr_s console_attr;

    console_attr.trans_init = console_transfer_init;
    console_attr.trans_deinit = console_transfer_deinit;
    console_attr.send_cmd = conv_json_only ? none_send_cmd_json : console_send_cmd_json;
    console_attr.grab_tab_list = conv_json_only ? none_grab_tab_list : console_grab_tab_list;

    return ss_shell(&console_attr, "123", script_file.c_str(), &console_json_out);
}
static int run_lua(const string &script_file, const struct ss_console_json_out_s &console_json_out, bool conv_json_only)
{
    struct ss_console_attr_s console_attr;

    console_attr.trans_init = console_transfer_init;
    console_attr.trans_deinit = console_transfer_deinit;
    console_attr.send_cmd = conv_json_only ? none_send_cmd_json : console_send_cmd_json;
    console_attr.grab_tab_list = conv_json_only ? none_grab_tab_list : console_grab_tab_list;

    return ss_lua(&console_attr, "123", script_file.c_str(), &console_json_out);
}
static int run_json(const string &script_file, const struct ss_console_json_out_s &console_json_out, bool conv_json_only)
{
    struct ss_console_attr_s console_attr;

    console_attr.trans_init = console_transfer_init;
    console_attr.trans_deinit = console_transfer_deinit;
    console_attr.send_cmd = conv_json_only ? none_send_cmd_json : console_send_cmd_json;
    console_attr.grab_tab_list = conv_json_only ? none_grab_tab_list : console_grab_tab_list;

    return ss_json(&console_attr, "123", script_file.c_str(), &console_json_out);
}
static int run_case(const char *report_file, const char *case_path_name, const char *script_path, int stage, int log_opt)
{
    struct ss_console_attr_s console_attr;

    console_attr.trans_init = console_transfer_init;
    console_attr.trans_deinit = console_transfer_deinit;
    console_attr.send_cmd = console_send_cmd_json;
    console_attr.grab_tab_list = console_grab_tab_list;

    return ss_run_json_case(&console_attr, "123", report_file, case_path_name, script_path, stage, log_opt);
}
static int run_case_all(const char *report_file, const char *script_path, int stage, int log_opt)
{
    struct ss_console_attr_s console_attr;

    console_attr.trans_init = console_transfer_init;
    console_attr.trans_deinit = console_transfer_deinit;
    console_attr.send_cmd = console_send_cmd_json;
    console_attr.grab_tab_list = console_grab_tab_list;

    return ss_run_json_case_all(&console_attr, "123", report_file, script_path, stage, log_opt);
}
static int run_case_false(const char *report_file, const char *script_path, int stage, int log_opt)
{
    struct ss_console_attr_s console_attr;

    console_attr.trans_init = console_transfer_init;
    console_attr.trans_deinit = console_transfer_deinit;
    console_attr.send_cmd = console_send_cmd_json;
    console_attr.grab_tab_list = console_grab_tab_list;

    return ss_run_json_case_false(&console_attr, "123", report_file, script_path, stage, log_opt);
}
static int run_case_random(const char *report_file, const char *script_path, int stage, int log_opt)
{
    struct ss_console_attr_s console_attr;

    console_attr.trans_init = console_transfer_init;
    console_attr.trans_deinit = console_transfer_deinit;
    console_attr.send_cmd = console_send_cmd_json;
    console_attr.grab_tab_list = console_grab_tab_list;

    return ss_run_json_case_random(&console_attr, "123", report_file, script_path, stage, log_opt);
}
static int run_python_out(std::vector<std::string> &in_strs)
{
    struct ss_console_json_out_s console_json_out;
    std::string python_str;
    unsigned int i;

    for (i = 1; i < in_strs.size() - 6; i++)
    {
        python_str += (' ' + in_strs[i]);
    }
    memset(&console_json_out, 0, sizeof(struct ss_console_json_out_s));
    console_json_out.out_file = in_strs[i++].c_str();
    console_json_out.case_name = in_strs[i++].c_str();
    console_json_out.case_stage = ss_cmd_atoi(in_strs[i++].c_str());
    console_json_out.case_stage_range = ss_cmd_atoi(in_strs[i++].c_str());
    console_json_out.log_opt = ss_cmd_atoi(in_strs[i++].c_str());
    console_json_out.do_append_cmds = ss_cmd_atoi(in_strs[i].c_str());
    return run_python(python_str, console_json_out, false);
}
static int run_python(std::vector<std::string> &in_strs)
{
    struct ss_console_json_out_s console_json_out;
    std::string python_str;

    for (unsigned int i = 1; i < in_strs.size(); i++)
    {
        python_str += (' ' + in_strs[i]);
    }
    memset(&console_json_out, 0, sizeof(struct ss_console_json_out_s));
    return run_python(python_str, console_json_out, false);
}
static int run_shell_out(std::vector<std::string> &in_strs)
{
    struct ss_console_json_out_s console_json_out;
    std::string shell_str;
    unsigned int i;

    for (i = 1; i < in_strs.size() - 6; i++)
    {
        shell_str += (' ' + in_strs[i]);
    }
    memset(&console_json_out, 0, sizeof(struct ss_console_json_out_s));
    console_json_out.out_file = in_strs[i++].c_str();
    console_json_out.case_name = in_strs[i++].c_str();
    console_json_out.case_stage = ss_cmd_atoi(in_strs[i++].c_str());
    console_json_out.case_stage_range = ss_cmd_atoi(in_strs[i++].c_str());
    console_json_out.log_opt = ss_cmd_atoi(in_strs[i++].c_str());
    console_json_out.do_append_cmds = ss_cmd_atoi(in_strs[i].c_str());
    return run_shell(shell_str, console_json_out, false);
}
static int run_shell(std::vector<std::string> &in_strs)
{
    struct ss_console_json_out_s console_json_out;
    std::string shell_str;

    for (unsigned int i = 1; i < in_strs.size(); i++)
    {
        shell_str += (' ' + in_strs[i]);
    }
    memset(&console_json_out, 0, sizeof(struct ss_console_json_out_s));
    return run_shell(shell_str, console_json_out, false);
}
static int run_json(std::vector<std::string> &in_strs)
{
    struct ss_console_json_out_s console_json_out;

    memset(&console_json_out, 0, sizeof(struct ss_console_json_out_s));
    if (in_strs.size() == 8)
    {
        console_json_out.out_file = in_strs[2].c_str();
        console_json_out.case_name = in_strs[3].c_str();
        console_json_out.case_stage = ss_cmd_atoi(in_strs[4].c_str());
        console_json_out.case_stage_range = ss_cmd_atoi(in_strs[5].c_str());
        console_json_out.log_opt = ss_cmd_atoi(in_strs[6].c_str());
    }
    return run_json(in_strs[1], console_json_out, false);
}
static int run_system(std::vector<std::string> &in_strs)
{
    int ret = system(in_strs[1].c_str());
    if (WIFEXITED(ret))
    {
        return WEXITSTATUS(ret);
    }
    return -1;
}
static int proc_set(std::vector<std::string> &in_strs)
{
    int fd = open(in_strs[1].c_str(), O_WRONLY);
    int ret = 0;

    if (fd < 0)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Open: " << in_strs[1] << " error!" <<endl << PRINT_COLOR_END;
        return -1;
    }
    ret = file_desc_do_write(fd, in_strs[2].c_str(), in_strs[2].size() + 1);
    if (ret < 0)
    {
        perror("write");
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}
static int proc_cat(std::vector<std::string> &in_strs)
{
    int fd = open(in_strs[1].c_str(), O_RDONLY);
    int ret = 0;
    bool b_end = false;
    int fd_flags = 0;
    char read_buf[512];
    int max_buf = ss_cmd_atoi(in_strs[3].c_str());
    struct timeval tv;

    if (fd < 0)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Open: " << in_strs[1] << " error!" <<endl << PRINT_COLOR_END;
        return -1;
    }
    if (max_buf < 512)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Max Buffer Size must be larger than 512byte!" <<endl << PRINT_COLOR_END;
        close(fd);
        return -1;
    }
    fd_flags = fcntl(fd, F_GETFL, 0);
    if (fd_flags < 0)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "FCNTL Error" << PRINT_COLOR_END;
        close(fd);
        return -1;
    }
    ret = fcntl(fd, F_SETFL, fd_flags | O_NONBLOCK);
    if (ret < 0)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "FCNTL Non block error" << PRINT_COLOR_END;
        close(fd);
        return -1;
    }
    while (!b_end)
    {
        fd_set fdsr;
        tv.tv_sec = 0;
        tv.tv_usec = 1000 * ss_cmd_atoi(in_strs[2].c_str());
        FD_ZERO(&fdsr);
        FD_SET(fd, &fdsr);
        ret = select(fd + 1, &fdsr, NULL, NULL, &tv);
        switch (ret)
        {
            case -1:
                perror("select");
                sslog.lv_set(PRINT_LV_ERROR) << "Select error" << PRINT_COLOR_END;
                close(fd);
                return -1;
            case 0:
                std::cout << "Wait client's action timeout." << "Time out count for "
                    << ss_cmd_atoi(in_strs[2].c_str()) << "ms."<< std::endl;
                b_end = true;
                break;
            case 1:
            {
                if (FD_ISSET(fd, &fdsr))
                {
                    ret = read(fd, read_buf, 512);
                    if (errno == EAGAIN)
                    {
                        break;
                    }
                    if (ret < 0)
                    {
                        sslog.lv_set(PRINT_LV_ERROR) << "Read error:" << strerror(errno) <<endl << PRINT_COLOR_END;
                        close(fd);
                        return -1;
                    }
                    if (ret == 0)
                    {
                        std::cout << "Client closes the connection." << std::endl;
                        b_end = true;
                        break;
                    }
                    sslog.write(read_buf, ret);
                    max_buf -= ret;
                    if (max_buf - 512 < 0)
                    {
                        std::cout << "Buffer size exceed." << std::endl;
                        sslog.write(read_buf, ret);
                        b_end = true;
                        break;
                    }
                }
            }
            break;
        }
    }
    sslog << endl;
    close(fd);
    return 0;
}
static int delay_ms(std::vector<std::string> &in_strs)
{
    unsigned int ms = ss_cmd_atoi(in_strs[1].c_str());
    usleep(ms * 1000);
    return 0;
}
static inline int str_2_log_color(const std::string &str)
{
    if (str == "normal")
        return PRINT_COLOR_NORMAL;
    if (str == "black")
        return PRINT_COLOR_BLACK;
    if (str == "red")
        return PRINT_COLOR_RED;
    if (str == "green")
        return PRINT_COLOR_GREEN;
    if (str == "yellow")
        return PRINT_COLOR_YELLOW;
    if (str == "blue")
        return PRINT_COLOR_BLUE;
    if (str == "funchsin")
        return PRINT_COLOR_FUNCHSIN;
    if (str == "cyan")
        return PRINT_COLOR_CYAN;
    if (str == "white")
        return PRINT_COLOR_WHITE;
    return PRINT_COLOR_NORMAL;
}
static inline int str_2_log_mode(const std::string &str)
{
    if (str == "normal")
        return PRINT_MODE_NORMAL;
    if (str == "highlight")
        return PRINT_MODE_HIGHTLIGHT;
    if (str == "underline")
        return PRINT_MODE_UNDERLINE;
    if (str == "flick")
        return PRINT_MODE_FLICK;
    if (str == "invert")
        return PRINT_MODE_INVERT;
    return PRINT_MODE_NORMAL;
}
static int print_log(std::vector<std::string> &in_strs)
{
    if (in_strs[0] == "print_log_color")
    {
        enum print_color color = (enum print_color)str_2_log_color(in_strs[1]);
        enum print_mode mode = (enum print_mode)str_2_log_mode(in_strs[2]);
        sslog.color_set(color, mode) << in_strs[3] << endl << PRINT_COLOR_END;
        if (ss_cmd_atoi(in_strs[4].c_str()))
        {
            sslog.store_ask() << "pause";
        }
        return 0;
    }
    sslog << in_strs[1] << endl;
    if (ss_cmd_atoi(in_strs[2].c_str()))
    {
        sslog.store_ask() << "pause";
    }
    return 0;
}
static int ping(std::vector<std::string> &in_strs)
{
    sslog.store_msg()["ping"] = "ok";
    sslog.store_ask() << "catch";
    return 0;
}
static int set_log_lv(std::vector<std::string> &in_strs)
{
    if (in_strs[1] == "trace")
    {
        sslog.lv_mask(PRINT_LV_TRACE);
        return 0;
    }
    if (in_strs[1] == "debug")
    {
        sslog.lv_mask(PRINT_LV_DEBUG);
        return 0;
    }
    if (in_strs[1] == "warn")
    {
        sslog.lv_mask(PRINT_LV_TRACE);
        return 0;
    }
    if (in_strs[1] == "error")
    {
        sslog.lv_mask(PRINT_LV_TRACE);
        return 0;
    }
    return 0;
}
static int create_child_process(std::vector<std::string> &in_strs)
{
    const string &handle = in_strs[1];

    child_shm *ins = dynamic_cast<child_shm *>(ss_handle_template<child_shm>::get(handle));
    if (ins)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Handle " << handle << " had beed created." << COLOR_ENDL;
        return -1;
    }
    if (!ss_cmd_atoi(in_strs[1].c_str()))
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Handle " << handle << " shm key is 0." << COLOR_ENDL;
        return -1;
    }
    try
    {
        child_shm *ins = new child_shm(in_strs[1]);
        ss_handle_template<child_shm>::install(handle, ins);
    }
    catch (const std::invalid_argument& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
static int access_child_process(std::vector<std::string> &in_strs)
{
    const string &handle = in_strs[1];
    child_shm *ins = dynamic_cast<child_shm *>(ss_handle_template<child_shm>::get(handle));
    if (!ins)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Can not find handle: " << handle << COLOR_ENDL;
        return -1;
    }
    ins->use_current();
    return 0;
}
static int leave_child_process(std::vector<std::string> &in_strs)
{
    child_shm::unuse_current();
    return 0;
}
static int kill_child_process(std::vector<std::string> &in_strs)
{
    const string &handle = in_strs[1];
    child_shm *ins = dynamic_cast<child_shm *>(ss_handle_template<child_shm>::get(handle));
    if (!ins)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Can not find handle: " << handle << COLOR_ENDL;
        return -1;
    }
    bool b_child_exit = ss_cmd_atoi(in_strs[3].c_str());
    ins->kill_child(ss_cmd_atoi(in_strs[2].c_str()), b_child_exit);
    if (b_child_exit)
    {
        return ss_handle_template<child_shm>::destroy(handle);
    }
    return 0;
}
static int destroy_child_process(std::vector<std::string> &in_strs)
{
    const string &handle = in_strs[1];
    return ss_handle_template<child_shm>::destroy(handle);
}
static int ignore_signal(std::vector<std::string> &in_strs)
{
    int sig = (int)ss_cmd_atoi(in_strs[1].c_str());
    signal(sig, SIG_IGN);
    return 0;
}
static inline void print_help(void)
{
    cout << "Program: <" << get_prog_name() << "> is for Sigmastar sdk ut test." << endl;
    cout << "Author: pedro.peng <pedro.peng@sigmastar.com.cn>." << endl;
    cout << "Build info : Commit " << GIT_COMMIT << ", Build by " << BUILD_OWNER << ", Date " << BUILD_DATE << '.' << endl;
    cout << "Run socket server on target chip(default is 0):" << endl;
    cout << get_prog_name() << " --socket  --chip-id=<id>" << endl;
    cout << "Run shm server on target chip(default is 0):" << endl;
    cout << get_prog_name() << " --shm=<key>  --chip-id=<id>" << endl;
    cout << "Run rpmsg server on target chip(default is 0):" << endl;
    cout << get_prog_name() << " --rpmsg --chip-id=<id> --mode=<epx/rcx>" << endl;
    cout << "Run console options:" << endl;
    cout << get_prog_name() << " --console" << endl;
    cout << "Run a signal command options:" << endl;
    cout << get_prog_name() << "  --run-cmd=<commands> --chip-id=<id>" << endl;
    cout << "Run python script options:" << endl;
    cout << get_prog_name() << " --python=<py file> --chip-id=<id> -o <json out file> --log=<option> --case-name=<name> --stage=<id> --stage-range=<id>" << endl;
    cout << "Run shell script options:" << endl;
    cout << get_prog_name() << " --shell=<sh file> --chip-id=<id> -o <json out file> --log=<option> --case-name=<name> --stage=<id> --stage-range=<id>" << endl;
    cout << "Run json script options:" << endl;
    cout << get_prog_name() << " --json=<json file> --chip-id=<id> -o <json out file> --log=<option> --case-name=<name> --stage=<id>" << endl;
    cout << "Run a specified test case in json report file:" << endl;
    cout << get_prog_name() << " --run=<case_path_and_name> -t <report json file> --stage=<id> --script-path=<path>" << endl;
    cout << "Run all test case in json report file:" << endl;
    cout << get_prog_name() << " --run-all -t <report json file> --stage=<id> --script-path=<path>" << endl;
    cout << "Run all fail test case in json report file:" << endl;
    cout << get_prog_name() << " --run-false -t <report json file> --stage=<id> --script-path=<path>" << endl;
    cout << "Run a test case in json report file by random:" << endl;
    cout << get_prog_name() << " --run-random -t <report json file> --stage=<id> --script-path=<path>" << endl;
    cout << "Generate report file by searching json file in the specified path:" << endl;
    cout << get_prog_name() << " --gen-report -t <report json file> --script-path=<path>" << endl;
    cout << "Run help options:" << endl;
    cout << get_prog_name() << " --help" << endl;
    cout << "--socket:        Create a socket server for remote console app connect." << endl;
    cout << "--shm:           Create a share memory server for local console app connect." << endl;
    cout << "--rpmsg:         Create a rpmsg server over pcie bus for remote chip to run command on current chip." << endl;
    cout << "--console:       Enter a local console ui without any IPC server." << endl;
    cout << "--run            Run json case by case id from report json file." << endl;
    cout << "--run-all        Run all json case from report json file." << endl;
    cout << "--run-false      Run fail json cases from report json file." << endl;
    cout << "--run-random     Run a json case from report json file by random." << endl;
    cout << "--conv2json      Run a json or shell case and only output json file without any real command's execution." << endl;
    cout << "-c --chip-id:    Enter a chip id for option --run-cmd." << endl;
    cout << "-r --run-cmd:    To run a command with 'argv'." << endl;
    cout << "-s --shell:      To run a serial of commands in shell script." << endl;
    cout << "-j --json:       To run a serial of commands in json script." << endl;
    cout << "-u --lua:        To run a serial of commands in lua script." << endl;
    cout << "-l --log:        Config json script output log option, option, 0: not output log, 1: output log if command return fail. 2: output all log." << endl
         << "                 Log output Mask: 0x100: Enter the judgement ui at the end of command, for example: -l 0x102, do judgement after it had outputed all logs."<< endl
         << "                 Log output Mask: 0x200: Output json file at the end of every command's execution, it is for debug use."<< endl;
    cout << "-o:              Config json script output file." << endl;
    cout << "-t:              Input/Output a json report file for test case." << endl;
    cout << "-n --case-name:  Config json script output case name." << endl;
    cout << " --stage:        Set current case's 'stage' value which is the mask value for filtering cases." << endl;
    cout << " --stage-range:  Set current case's 'stage range' which is the amount of all 'stage' values that cases support." << endl;
    cout << " --script-path:  Report file used only in input, to set the base path to find the json file that is built like 'script_path/case_path/case_name.json'." << endl;
    cout << " --append-cmds:  To append commands in the output json file existed before." << endl;
    cout << " --log-transfer: To transfer the logs from standrad output to the console or just print on local only. " << endl;
    cout << "                 The parameter is choosed with local/keywords/all, which is to print the log in local, with keywords only on remote, all stdout logs on remote." << endl;
    cout << "-h --help:       To show this HELP infomation." << endl;
}
int setup_ui(int argc, char **argv)
{
    int result = 0;
    int option_index = 0;
    int stage = 0;
    int case_stage = 1;
    int mode = 0;
    int count = 0;
    bool conv_json_only = false;
    unsigned short chip_id = 0;
    struct ss_console_json_out_s console_json_out;
    string script_file, out_file, out_report_file;
    string in_para_strs, case_name = "defalut_case", script_path;
    string case_path, shm_key;
    struct option long_options[] = {
        {"socket",        no_argument, 0, 0},
        {"shm",           required_argument, 0, 0},
        {"rpmsg",         no_argument, 0, 0},
        {"console",       no_argument, 0, 0},
        {"run",           required_argument, 0, 0},
        {"run-all",       no_argument, 0, 0},
        {"run-false",     no_argument, 0, 0},
        {"run-random",    no_argument, 0, 0},
        {"gen-report",    no_argument, 0, 0},
        {"stage",        required_argument, 0, 0},
        {"stage-range",  required_argument, 0, 0},
        {"conv2json",    no_argument, 0, 0},
        {"script-path",  required_argument, 0, 0},
        {"append-cmds",  no_argument, 0, 0},
        {"log-transfer", required_argument, 0, 0},
        {"help",         no_argument, 0, 'h'},
        {"run-cmd",      required_argument, 0, 'r'},
        {"python",       required_argument, 0, 'p'},
        {"shell",        required_argument, 0, 's'},
        {"lua",           required_argument, 0, 'u'},
        {"chip-id",      required_argument, 0, 'c'},
        {"mode",         required_argument, 0, 'm'},
        {"json",         required_argument, 0, 'j'},
        {"log",          required_argument, 0, 'l'},
        {"case-name",    required_argument, 0, 'n'},
        { 0, 0, 0, 0 }
    };
    memset(&console_json_out, 0, sizeof(struct ss_console_json_out_s));
    console_json_out.log_opt = 1; //defalut is 1, show only error
    console_json_out.case_stage = console_json_out.case_stage_range = 1; //defalut is 1
    console_json_out.case_name = case_name.c_str();
    ss_cmd_base::set_path(argv[0]);
    shm_child_resource res;
    while ((result = getopt_long(argc, argv, "hr:c:s:m:j:o:l:n:p:t:u:", long_options, &option_index)) != -1)
    {
        switch (result)
        {
            case 0:
            {
                if (!strcmp(long_options[option_index].name, "socket"))
                {
                    if (stage & (RUN_CMD | START_SOCKET)) //1001
                        break;

                    stage |= START_SOCKET;
                }
                else if (!strcmp(long_options[option_index].name, "shm"))
                {
                    if (stage & (RUN_CMD | START_SHM)) //1010
                        break;
                    if (optarg[0] == '=')
                        optarg++;
                    shm_key = optarg;
                    stage |= START_SHM;
                }
                else if (!strcmp(long_options[option_index].name, "rpmsg"))
                {
                    if (stage & (RUN_CMD | START_RPMSG)) //1100
                        break;

                    stage |= START_RPMSG;
                }
                else if (!strcmp(long_options[option_index].name, "run"))
                {
                    if (stage & (SET_PCIE_MODE | RUN_CMD | START_RPMSG | START_SHM | START_SOCKET
                                | RUN_PYTHON | RUN_SHELL | RUN_JSON | RUN_LUA | RUN_JSON_CASE))
                        break;
                    if (optarg[0] == '=')
                        optarg++;
                    case_name = optarg;
                    stage |= RUN_JSON_CASE;
                }
                else if (!strcmp(long_options[option_index].name, "run-all"))
                {
                    if (stage & (SET_PCIE_MODE | RUN_CMD | START_RPMSG | START_SHM | START_SOCKET
                                | RUN_PYTHON | RUN_SHELL | RUN_JSON | RUN_LUA | RUN_JSON_CASE_ALL))
                        break;
                    stage |= RUN_JSON_CASE_ALL;
                }
                else if (!strcmp(long_options[option_index].name, "run-false"))
                {
                    if (stage & (SET_PCIE_MODE | RUN_CMD | START_RPMSG | START_SHM | START_SOCKET
                                | RUN_PYTHON | RUN_SHELL | RUN_JSON | RUN_LUA | RUN_JSON_CASE_FALSE))
                        break;
                    stage |= RUN_JSON_CASE_FALSE;
                }
                else if (!strcmp(long_options[option_index].name, "run-random"))
                {
                    if (stage & (SET_PCIE_MODE | RUN_CMD | START_RPMSG | START_SHM | START_SOCKET
                                | RUN_PYTHON | RUN_SHELL | RUN_JSON | RUN_LUA | RUN_JSON_CASE_RAMDOM))
                        break;
                    stage |= RUN_JSON_CASE_RAMDOM;
                }
                else if (!strcmp(long_options[option_index].name, "gen-report"))
                {
                    if (stage & (SET_PCIE_MODE | RUN_CMD | START_RPMSG | START_SHM | START_SOCKET
                                | RUN_PYTHON | RUN_SHELL | RUN_JSON | RUN_LUA | GENERATE_REPORT_FILE))
                        break;
                    stage |= GENERATE_REPORT_FILE;
                }
                else if (!strcmp(long_options[option_index].name, "stage"))
                {
                    if (stage & (SET_PCIE_MODE | RUN_CMD | START_RPMSG | START_SHM | START_SOCKET
                                | OUT_JSON_CASE_STAGE | CONVERT_TO_JSON))
                        break;
                    if (optarg[0] == '=')
                        optarg++;
                    console_json_out.case_stage = case_stage = ss_cmd_atoi(optarg);
                    stage |= OUT_JSON_CASE_STAGE;
                }
                else if (!strcmp(long_options[option_index].name, "stage-range"))
                {
                    if (stage & (SET_PCIE_MODE | RUN_CMD | START_RPMSG | START_SHM | START_SOCKET
                                | OUT_JSON_CASE_STAGE_RANGE))
                        break;
                    if (optarg[0] == '=')
                        optarg++;
                    console_json_out.case_stage_range = ss_cmd_atoi(optarg);
                    stage |= OUT_JSON_CASE_STAGE_RANGE;
                }
                else if (!strcmp(long_options[option_index].name, "conv2json"))
                {
                    if (stage & (SET_PCIE_MODE | RUN_CMD | START_RPMSG | START_SHM | START_SOCKET
                                | CONVERT_TO_JSON))
                        break;
                    conv_json_only = true;
                    console_json_out.case_stage = -1;
                    stage |= CONVERT_TO_JSON;
                }
                else if (!strcmp(long_options[option_index].name, "script-path"))
                {
                    if (stage & (SET_PCIE_MODE | RUN_CMD | START_RPMSG | START_SHM | START_SOCKET
                                | RUN_PYTHON | RUN_SHELL | RUN_JSON | RUN_LUA | OUT_JSON_PATH))
                        break;
                    if (optarg[0] == '=')
                        optarg++;
                    script_path  = optarg;
                    stage |= OUT_JSON_PATH;
                }
                else if (!strcmp(long_options[option_index].name, "console"))
                {
                    struct ss_console_attr_s console_attr;

                    if (stage & (SET_PCIE_MODE | SET_CHIP | RUN_CMD | START_RPMSG | START_SHM | START_SOCKET))
                        break;

                    console_attr.trans_init = console_transfer_init;
                    console_attr.trans_deinit = console_transfer_deinit;
                    console_attr.send_cmd = console_send_cmd_json;
                    console_attr.grab_tab_list = console_grab_tab_list;
                    return ss_console(&console_attr, "123");
                }
                else if (!strcmp(long_options[option_index].name, "append-cmds"))
                {
                    if (stage & (RUN_CMD | APPEND_CMDS_IN_JSON))
                        break;
                    console_json_out.do_append_cmds = 1;
                    stage |= APPEND_CMDS_IN_JSON;
                }
                else if (!strcmp(long_options[option_index].name, "log-transfer"))
                {
                    if (stage & LOG_TRANSFER)
                        break;
                    if (optarg[0] == '=')
                        optarg++;
                    if (!strcmp("local", optarg))
                    {
                        sslog.set_flag(PRINT_LOG_LOCAL);
                        break;
                    }
                    if (!strcmp("keywords", optarg))
                    {
                        sslog.set_flag(PRINT_STORED_LOG_REMOTE);
                        break;
                    }
                    if (!strcmp("all", optarg))
                    {
                        sslog.set_flag(PRINT_LOG_REMOTE);
                        break;
                    }
                    stage |= LOG_TRANSFER;
                }
            }
            break;
            case 'r':
            {
                int i = 0;

                if (stage & (SET_PCIE_MODE | RUN_CMD | START_RPMSG | START_SHM | START_SOCKET))
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
                    in_para_strs += '"';
                    in_para_strs += argv[optind + i];
                    in_para_strs += '"';
                }
                stage |= RUN_CMD;
            }
            break;
            case 'c':
            {
                if (stage & SET_CHIP)
                    break;
                if (optarg[0] == '=')
                    optarg++;
                chip_id = (unsigned short)ss_cmd_atoi(optarg);
                cout << "chip id is: " << chip_id << endl;
                stage |= SET_CHIP;
                break;
            }
            break;
            case 'm':
            {
                if (stage & (SET_PCIE_MODE | RUN_CMD))
                    break;
                if (optarg[0] == '=')
                    optarg++;
                cout << "mode: " << optarg << endl;
#ifdef PCIE_RPMSG_ROUTE
                if (!strncmp("rc", optarg, 2))
                    mode = RPMSG_MODE_RC_PORT;
                else if (!strncmp("ep", optarg, 2))
                    mode = RPMSG_MODE_EP_PORT;
                count = ss_cmd_atoi(optarg + 2);
#endif
                stage |= SET_PCIE_MODE;
                break;
            }
            break;
            case 'p':
            {
                if (stage & (SET_PCIE_MODE | RUN_CMD | START_RPMSG | START_SHM | START_SOCKET | RUN_PYTHON | RUN_SHELL | RUN_JSON | RUN_LUA | RUN_JSON_CASE_RAMDOM))
                    break;
                if (optarg[0] == '=')
                    optarg++;
                script_file = optarg;
                stage |= RUN_PYTHON;
            }
            break;
            case 's':
            {
                if (stage & (SET_PCIE_MODE | RUN_CMD | START_RPMSG | START_SHM | START_SOCKET |  RUN_PYTHON | RUN_SHELL | RUN_JSON | RUN_LUA | RUN_JSON_CASE_RAMDOM))
                    break;
                if (optarg[0] == '=')
                    optarg++;
                script_file = optarg;
                stage |= RUN_SHELL;
            }
            break;
            case 'u':
            {
                if (stage & (SET_PCIE_MODE | RUN_CMD | START_RPMSG | START_SHM | START_SOCKET |  RUN_PYTHON | RUN_SHELL | RUN_JSON | RUN_LUA | RUN_JSON_CASE_RAMDOM))
                    break;
                if (optarg[0] == '=')
                    optarg++;
                script_file = optarg;
                stage |= RUN_LUA;
            }
            break;
            case 'j':
            {
                if (stage & (SET_PCIE_MODE | RUN_CMD | START_RPMSG | START_SHM | START_SOCKET |  RUN_PYTHON | RUN_SHELL | RUN_JSON | RUN_LUA | RUN_JSON_CASE_RAMDOM))
                    break;
                if (optarg[0] == '=')
                    optarg++;
                script_file = optarg;
                stage |= RUN_JSON;
            }
            break;
            case 'o':
            {
                if (stage & (RUN_CMD | OUT_JSON_FILE))
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
                if (stage & (RUN_CMD | OUT_JSON_REPORT_FILE))
                    break;
                if (optarg[0] == '=')
                    optarg++;
                out_report_file = optarg;
                stage |= OUT_JSON_REPORT_FILE;
            }
            break;
            case 'l':
            {
                if (stage & (RUN_CMD | LOG_OPTION))
                    break;
                if (optarg[0] == '=')
                    optarg++;
                console_json_out.log_opt = ss_cmd_atoi(optarg);
                stage |= LOG_OPTION;
            }
            break;
            case 'n':
            {
                if (stage & (RUN_CMD | OUT_JSON_CASE_NAME))
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
    if (!stage)
    {
        print_help();
        return 0;
    }
    if (stage & SET_CHIP)
        run_cmd(0, "select_chip %d", chip_id);
    if (in_para_strs.size())
        return run_cmd(chip_id, in_para_strs.c_str());

    //Script command.
    if (stage & RUN_JSON)
        return run_json(script_file, console_json_out, conv_json_only);
    if (stage & RUN_SHELL)
        return run_shell(script_file, console_json_out, conv_json_only);
    if (stage & RUN_LUA)
        return run_lua(script_file, console_json_out, conv_json_only);
    if (stage & RUN_PYTHON)
        return run_python(script_file, console_json_out, conv_json_only);
    if ((stage & RUN_JSON_CASE) && (stage & OUT_JSON_REPORT_FILE) && (stage & OUT_JSON_CASE_STAGE))
        return run_case(out_report_file.c_str(), case_name.c_str(), script_path.c_str(), case_stage, console_json_out.log_opt);
    if ((stage & RUN_JSON_CASE_ALL) && (stage & OUT_JSON_REPORT_FILE) && (stage & OUT_JSON_CASE_STAGE)
        && !out_report_file.empty())
        return run_case_all(out_report_file.c_str(), script_path.c_str(), case_stage, console_json_out.log_opt);
    if ((stage & RUN_JSON_CASE_FALSE) && (stage & OUT_JSON_REPORT_FILE) && (stage & OUT_JSON_CASE_STAGE)
        && !out_report_file.empty())
        return run_case_false(out_report_file.c_str(), script_path.c_str(), case_stage, console_json_out.log_opt);
    if ((stage & RUN_JSON_CASE_RAMDOM) && (stage & OUT_JSON_REPORT_FILE) && (stage & OUT_JSON_CASE_STAGE)
        && !out_report_file.empty())
        return run_case_random(out_report_file.c_str(), script_path.c_str(), case_stage, console_json_out.log_opt);
    if ((stage & GENERATE_REPORT_FILE) && (stage & OUT_JSON_REPORT_FILE) && !out_report_file.empty())
        return ss_generate_report_file(script_path.c_str(), out_report_file.c_str());

    //Server commands...
    if (stage & START_SOCKET)
        run_cmd(0, "start_socket");
    if (stage & START_SHM)
        run_cmd(0, "start_shm %s", shm_key.c_str());
    if (stage & START_RPMSG)
        run_cmd(0, "start_rpmsg %d %d", mode, count);
    return wait_server();
}
int run_cmd(const char *fmt, ...)
{
    char buffer[TRANS_BUFFER];
    VTRANSFER_BUF(buffer, fmt);
    return relative_call(NULL, true, buffer);
}
int run_cmd_nolock(const char *fmt, ...)
{
    char buffer[TRANS_BUFFER];
    VTRANSFER_BUF(buffer, fmt);
    return relative_call(NULL, false, buffer);
}
int run_cmd(unsigned int chip_id, const char *fmt, ...)
{
    char buffer[TRANS_BUFFER];
    VTRANSFER_BUF(buffer, fmt);
    return absolute_call(chip_id, NULL, true, buffer);
}
int run_cmd_nolock(unsigned int chip_id, const char *fmt, ...)
{
    char buffer[TRANS_BUFFER];
    VTRANSFER_BUF(buffer, fmt);
    return absolute_call(chip_id, NULL, false, buffer);
}

int run_cmd_trans_log(ss_result &result, const char *fmt, ...)
{
    char buffer[TRANS_BUFFER];
    VTRANSFER_BUF(buffer, fmt);
    return relative_call(&result, true, buffer);
}
int run_cmd_trans_log_nolock(ss_result &result, const char *fmt, ...)
{
    char buffer[TRANS_BUFFER];
    VTRANSFER_BUF(buffer, fmt);
    return relative_call(&result, false, buffer);
}
int run_cmd_trans_log(unsigned int chip_id, ss_result &result, const char *fmt, ...)
{
    char buffer[TRANS_BUFFER];
    VTRANSFER_BUF(buffer, fmt);
    return absolute_call(chip_id, &result, true, buffer);
}
int run_cmd_trans_log_nolock(unsigned int chip_id, ss_result &result, const char *fmt, ...)
{
    char buffer[TRANS_BUFFER];
    VTRANSFER_BUF(buffer, fmt);
    return absolute_call(chip_id, &result, false, buffer);
}
long ss_cmd_atoi(const char *str)
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

MOD_CMDS(base) {
    ADD_CMD("run_chip_id", run_chip_id, 2);
    ADD_CMD_VAR_ARG("wish_call", wish_call, 2);
    ADD_CMD_HELP("wish_call", "[wish_val] [cmds_str] ...", "Call cmds and return 0 if wish value equal to sub cmd return val.");
    ADD_CMD_VAR_ARG("route_call", route_call, 2);
    ADD_CMD_HELP("route_call", "[b_transfer] [cmds_str] ...", "Call cmds on local and determine the cmds wether transfer to next chip.");
    ADD_CMD_VAR_ARG("sync_call", sync_call, 1);
    ADD_CMD_HELP("sync_call", "[cmds_str] ...", "Run a commad and let shell to get result synchronously.");
    ADD_CMD("dump_help_all", dump_help, 0);
    ADD_CMD_HELP("dump_help_all", "No argumemt", "Dump all command's help infomation.");
    ADD_CMD("dump_help", dump_help, 1);
    ADD_CMD_HELP("dump_help", "[cmd]", "Dump a specified commands's help infomation.");
    ADD_CMD("dump_help_module", dump_help_module, 1);
    ADD_CMD_HELP("dump_help_module", "[module name]", "Dump all help infomation from the specified module name.");
    ADD_CMD("start_shm", start_shm_server, 1);
    ADD_CMD_HELP("start_shm", "[shm_key]", "Start a share memory server on dst chip.");
    ADD_CMD("stop_shm", stop_shm_server, 0);
    ADD_CMD_HELP("stop_shm", "No argument", "Stop share memory server on dst chip.");
    ADD_CMD("start_socket", start_socket_server, 0);
    ADD_CMD_HELP("start_socket", "No argument", "Stop socket server on dst chip.");
    ADD_CMD("stop_socket", stop_socket_server, 0);
    ADD_CMD_HELP("stop_socket", "No argument", "Stop socket server on dst chip.");
    ADD_CMD("reset_socket", reset_socket_server, 0);
    ADD_CMD_HELP("reset_socket", "No argument", "Reset socket server on dst chip.");
    ADD_CMD("start_rpmsg", start_rpmsg_server, 2);
    ADD_CMD_HELP("start_rpmsg", "[mode] [target count]", "Start a rpmsg server on dst chip, mode:0 rc, mode:1 ep. Target count expected target chips count.");
    ADD_CMD("stop_rpmsg", stop_rpmsg_server, 0);
    ADD_CMD_HELP("stop_rpmsg", "No argument", "Stop rpmsg server on dst chip.");
    ADD_CMD("select_chip", select_chip_id, 1);
    ADD_CMD_HELP("select_chip", "[id]", "Config the environment of chip id on target chip.");
    ADD_CMD("clear_rpmsg_client", clear_rpmsg_client, 0);
    ADD_CMD_HELP("clear_rpmsg_client", "No argument", "Clear rpmsg client file descriptor.");
    ADD_CMD_VAR_ARG_NO_LOCK("run_python_out", run_python_out, 7);
    ADD_CMD_HELP("run_python_out", "[script and args] ... [output file] [case name] [case stage] [case stage range] [out log level] [do append cmds]",
                 "Run python script, and output json & report.");
    ADD_CMD_VAR_ARG_NO_LOCK("run_shell_out", run_shell_out, 7);
    ADD_CMD_HELP("run_shell_out", "[script and args] ... [output file] [case name] [case stage] [case stage range] [out log level] [do append cmds]",
                 "Run shell script, and output json & report.");
    ADD_CMD_NO_LOCK("run_json_out", run_json, 6);
    ADD_CMD_HELP("run_json_out", "[script file] [output file] [case name] [case stage] [out log level]",
                 "Run json script, and output json & report.");
    ADD_CMD_VAR_ARG_NO_LOCK("run_python", run_python, 1);
    ADD_CMD_HELP("run_python", "[script and args] ...", "Run python script.");
    ADD_CMD_VAR_ARG_NO_LOCK("run_shell", run_shell, 1);
    ADD_CMD_HELP("run_shell", "[script and args] ...", "Run shell script.");
    ADD_CMD_NO_LOCK("run_json", run_json, 1);
    ADD_CMD_HELP("run_json", "[script file]", "Run json script.");
    ADD_CMD_NO_LOCK("run_system", run_system, 1);
    ADD_CMD_HELP("run_system", "[commands]", "Execute a system command.");
    ADD_CMD("proc_set", proc_set, 2);
    ADD_CMD_HELP("proc_set", "[file path] [proc commands]" , "Set string commands to file path.");
    ADD_CMD("proc_cat", proc_cat, 3);
    ADD_CMD_HELP("proc_cat", "[file_path] [delay_ms] [max_buf]" , "Read data from file path, like procfs.",
                 "[file_path]: The file path to be read.",
                 "[delay_ms] : The reading delay in millisecond if no data from proc.",
                 "[max_buf]  : The max buffer size for transfered data from procfs.");
    ADD_CMD_NO_LOCK("delay_ms",delay_ms, 1 );
    ADD_CMD_HELP("delay_ms", "[time_ms]" , "Sleep ms.");
    ADD_CMD("print_log", print_log, 2);
    ADD_CMD_HELP("print_log", "[logs] [do pause]" , "Print log on dst UI console.");
    ADD_CMD("print_log_color", print_log, 4);
    ADD_CMD_HELP("print_log_color", "[color] [mode] [logs] [do_pause]" ,
                 "Print log on dst UI console.",
                 "[color]: 'normal', 'black', 'red', 'green', 'yellow', 'blue', 'funchsin', 'cyan', 'white'",
                 "[mode]: 'normal', 'highlight', 'underline', 'flick', 'invert'",
                 "[do pause]: Pause the client's process and ask client if is case pass or not.");
    ADD_CMD("ping", ping, 0);
    ADD_CMD_HELP("ping", "No argument" , "To test message center or check server is alive or not.");
    ADD_CMD("set_log_lv", set_log_lv, 1);
    ADD_CMD_HELP("set_log_lv", "[level]", "To mask log in sslog.",
                 "[level]: 'trace', 'debug', 'warn', 'error'");
    ADD_CMD("create_child_process", create_child_process, 1);
    ADD_CMD_HELP("create_child_process", "[key]", "Create a child process by specified handle.");
    ADD_CMD("destroy_child_process", destroy_child_process, 1);
    ADD_CMD_HELP("destroy_child_process", "[key]", "Destroy :wa child process by specified handle.");
    ADD_CMD("access_child_process", access_child_process, 1);
    ADD_CMD_HELP("access_child_process", "[key]", "Set the command setting path to child process by specified handle.");
    ADD_CMD("leave_child_process", leave_child_process, 0);
    ADD_CMD_HELP("leave_child_process", "No argumemt", "Set the command setting path to father process.");
    ADD_CMD("kill_child_process", kill_child_process, 3);
    ADD_CMD_HELP("kill_child_process", "[key] [signal] [b_child_exit]", "Trigger a signal event to the child process.");
    ADD_CMD("ignore_signal", ignore_signal, 1);
    ADD_CMD_HELP("ignore_signal", "[signal num]", "Let ss_cmd process ignore signal, make signal only received on script.");
    //*****************************INTERNAL USE COMMANDS********************************
    //Althrough user can execute it by console/shell, but this is for internal use.
    ADD_CMD("internal_get_tabs", get_tabs, 0);
    //*********************************************************************************
}
