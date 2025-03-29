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
#include "ss_thread.h"
#include "ss_connector.h"
#include "ss_event.h"
#include "ss_mload.h"
#include "ss_cmd_base.h"

static std::map<std::string, void *>threads;
static void *do_work(struct ss_thread_buffer *thread_buf, struct ss_thread_user_data *work_data)
{
    const char *cmd_str = NULL;
    assert(work_data);
    if (!work_data->data)
    {
        cout << "Work data null!" << endl;
        return NULL;
    }
    if (!work_data->size)
    {
        cout << "Work data size zero!" << endl;
        return NULL;
    }
    cmd_str = (const char *)work_data->data;
    return (void *)(long)run_cmd(cmd_str);
}
static int destroy_workqueue(std::vector<std::string> &in_strs)
{
    auto iter = threads.find(in_strs[1]);

    if (iter == threads.end())
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Hanle not found." << endl << PRINT_COLOR_END;
        return -1;
    }
    ss_thread_close(iter->second);
    threads.erase(iter);
    return 0;
}
static int create_workqueue(std::vector<std::string> &in_strs)
{
    struct ss_thread_attr ss_attr;
    void *handle = NULL;

    memset(&ss_attr, 0, sizeof(struct ss_thread_attr));
    ss_attr.do_signal = do_work;
    ss_attr.do_monitor = NULL;
    snprintf(ss_attr.thread_name, 128, "%s", in_strs[1].c_str());
    handle = ss_thread_open(&ss_attr);
    if (!handle)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Hanle not found." << endl << PRINT_COLOR_END;
        return -1;
    }
    threads[in_strs[1]] = handle;
    return 0;
}
static int schedule_work(std::vector<std::string> &in_strs)
{
    struct ss_thread_user_data work_data;
    auto iter = threads.find(in_strs[1]);

    if (iter == threads.end())
    {
        sslog.lv_set(PRINT_LV_ERROR) << "Hanle not found." << endl << PRINT_COLOR_END;
        return -1;
    }
    memset(&work_data, 0, sizeof(struct ss_thread_user_data));
    std::string str;
    for (unsigned int i = 2; i < in_strs.size(); i++)
    {
        str += (' ' + in_strs[i]);
    }
    work_data.data = (void *)str.c_str();
    work_data.size = str.size() + 1;
    return ss_thread_send(iter->second, &work_data);
}

static int create_connector(vector<string> &in_strs)
{
    const string &handle = in_strs[1];
    return ss_connector::create(handle) ? 0 : -1;
}
static int create_event(std::vector<std::string> &in_strs)
{
    const string &handle = in_strs[1];
    unsigned int cycle_time_ms = (unsigned int)ss_cmd_atoi(in_strs[2].c_str());
    return ss_event::create(handle, cycle_time_ms) ? 0 : -1;
}
static int add_event_cmds(std::vector<std::string> &in_strs)
{
    const string &handle = in_strs[1];

    ss_event *event = dynamic_cast<ss_event *>(ss_handle::get(handle));
    if (!event)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "ss_event not found." << endl << PRINT_COLOR_END;
        return -1;
    }
    std::string str;
    for (unsigned int i = 3; i < in_strs.size(); i++)
    {
        str += (' ' + in_strs[i]);
    }
    event->add_cmds((int)ss_cmd_atoi(in_strs[2].c_str()), str);
    return 0;
}
static int cmd_choice(const std::vector<std::string> &in_strs, int last_event,
                      void (*do_event)(int, class ss_event *), class ss_event *this_class)
{
    int ret = 0;
    if (in_strs.size() != 1)
    {
        return -1;
    }
    sslog << "Last event: " << last_event << "." << endl << PRINT_COLOR_END;
    ret = run_cmd(in_strs[0].c_str());
    do_event(ret, this_class);
    return ret;
}
static int set_cmd_return_choice(std::vector<std::string> &in_strs)
{
    const string &handle = in_strs[1];
    ss_event *event = dynamic_cast<ss_event *>(ss_handle::get(handle));
    if (!event)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "ss_event not found." << endl << PRINT_COLOR_END;
        return -1;
    }
    std::vector<std::string> set_strs;
    std::string str;
    for (unsigned int i = 2; i < in_strs.size(); i++)
    {
        str += (' ' + in_strs[i]);
    }
    set_strs.push_back(str);
    event->set_choice(cmd_choice, set_strs, 0);
    return 0;
}
static int start_event(std::vector<std::string> &in_strs)
{
    const string &handle = in_strs[1];
    ss_event *event = dynamic_cast<ss_event *>(ss_handle::get(handle));
    if (!event)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "ss_event not found." << endl << PRINT_COLOR_END;
        return -1;
    }
    event->start_event();
    return 0;
}
static int stop_event(std::vector<std::string> &in_strs)
{
    const string &handle = in_strs[1];
    ss_event *event = dynamic_cast<ss_event *>(ss_handle::get(handle));
    if (!event)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "ss_event not found." << endl << PRINT_COLOR_END;
        return -1;
    }
    event->stop_event();
    return 0;
}
static int create_mload(std::vector<std::string> &in_strs)
{
    const string &handle = in_strs[1];
    return ss_mload::create(handle) ? 0 : -1;
}
static int add_mload_cmds(std::vector<std::string> &in_strs)
{
    const string &handle = in_strs[1];

    ss_mload *mload = dynamic_cast<ss_mload *>(ss_handle::get(handle));
    if (!mload)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "ss_mload not found." << endl << PRINT_COLOR_END;
        return -1;
    }
    std::string str;
    for (unsigned int i = 2; i < in_strs.size(); i++)
    {
        str += (' ' + in_strs[i]);
    }
    mload->add_cmds(str);
    return 0;
}
static int run_mload(std::vector<std::string> &in_strs)
{
    const string &handle = in_strs[1];
    ss_mload *mload = dynamic_cast<ss_mload *>(ss_handle::get(handle));
    if (!mload)
    {
        sslog.lv_set(PRINT_LV_ERROR) << "ss_event not found." << endl << PRINT_COLOR_END;
        return -1;
    }
    mload->run();
    return 0;
}
static int destroy_hanle(std::vector<std::string> &in_strs)
{
    const string &handle = in_strs[1];
    return ss_handle::destroy(handle);
}

MOD_CMDS(common) {
    ADD_CMD("create_workqueue", create_workqueue, 1);
    ADD_CMD_HELP("create_workqueue", "[workqueue_name]", "Creata a work queue for runing commads.");
    ADD_CMD_NO_LOCK("destroy_workqueue", destroy_workqueue, 1);
    ADD_CMD_HELP("destroy_workqueue", "[workqueue_name]", "Destroy specified work queue.");
    ADD_CMD_VAR_ARG("schedule_work", schedule_work, 2);
    ADD_CMD_HELP("schedule_work", "[workqueue_name] [cmds and args] ...", "Schedule thework to run a command.");
    ADD_CMD("create_connector", create_connector, 1);
    ADD_CMD_HELP("create_connector", "[handle]", "Create a regular connector by handle.");
    ADD_CMD_NO_LOCK("destroy_connector", destroy_hanle, 1);
    ADD_CMD_HELP("destroy_connector", "[handle]", "Destroy a regular connector by handle.");
    ADD_CMD("create_event", create_event, 2);
    ADD_CMD_HELP("create_event", "[handle] [cycle_time_ms]", "Create a event monitor by handle.");
    ADD_CMD_NO_LOCK("destroy_event", destroy_hanle, 1);
    ADD_CMD_HELP("destroy_event", "[handle]", "Destroy a event monitor by handle.");
    ADD_CMD_VAR_ARG("add_event_cmds", add_event_cmds, 3);
    ADD_CMD_HELP("add_event_cmds", "[handle] [choice id] [cmd and args] ...", "Add a event command to ss_event.");
    ADD_CMD_VAR_ARG("set_cmd_return_choice", set_cmd_return_choice, 2);
    ADD_CMD_HELP("set_cmd_return_choice", "[handle] [cmd and args] ...", "set a command and monitor its return val.");
    ADD_CMD("start_event", start_event, 1);
    ADD_CMD_HELP("start_event", "[handle]", "Start ss_event monitor.");
    ADD_CMD("stop_event", stop_event, 1);
    ADD_CMD_HELP("stop_event", "[handle]", "Stop ss_event monitor.");
    ADD_CMD("create_mload", create_mload, 1);
    ADD_CMD_HELP("create_mload", "[handle]", "Create a menu load by handle.");
    ADD_CMD_VAR_ARG("add_mload_cmds", add_mload_cmds, 2);
    ADD_CMD_HELP("add_mload_cmds", "[handle] [cmds] ...", "Add a event command to ss_mload.");
    ADD_CMD("run_mload", run_mload, 1);
    ADD_CMD_HELP("run_mload", "[handle]", "Start ss_mload.");
    ADD_CMD("destroy_mload", destroy_hanle, 1);
    ADD_CMD_HELP("destroy_mload", "[handle]", "Destroy a menu load by handle.");
}
