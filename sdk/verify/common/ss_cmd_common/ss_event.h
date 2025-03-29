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
#ifndef __SS_EVENT__
#define __SS_EVENT__
#include <assert.h>
#include "ss_thread.h"
#include "ss_handle.h"
#include "ss_cmd_base.h"
class ss_event : public ss_handle
{
    public:
        explicit ss_event(const string &handle, unsigned int cycle_time_ms)
        {
            struct ss_thread_attr ss_attr;
            event_choice = NULL;
            current_event = 0;
            memset(&ss_attr, 0, sizeof(struct ss_thread_attr));
            ss_attr.do_signal = NULL;
            ss_attr.do_monitor = do_monitor;
            ss_attr.in_buf.buf = (void *)this;
            ss_attr.monitor_cycle_sec = cycle_time_ms / 1000;
            ss_attr.monitor_cycle_nsec = (cycle_time_ms % 1000) * 1000 * 1000;
            snprintf(ss_attr.thread_name, 128, "ss_event_%s", handle.c_str());
            thread_handle = ss_thread_open(&ss_attr);
        }
        virtual ~ss_event()
        {
            ss_thread_close(thread_handle);
            store_cmds.clear();
            event_params.clear();
        }
        void start_event()
        {
            ss_thread_start_monitor(thread_handle);
        }
        void stop_event()
        {
            ss_thread_stop(thread_handle);
        }
        void set_choice(int (*choice)(const vector<string> &, int, void (*p)(int, class ss_event *), class ss_event *),
                        vector<string> &params, int default_event)
        {
            event_choice = choice;
            event_params = params;
            current_event = default_event;
        }
        void add_cmds(int choice_id, const string &cmd_str)
        {
            store_cmds[choice_id].push_back(cmd_str);
        }
        static ss_event *create(const string &handle, unsigned int cycle_time_ms)
        {
            ss_event *event = new ss_event(handle, cycle_time_ms);
            assert(event);
            if (!ss_handle::install(handle, event))
            {
                delete event;
                return NULL;
            }
            return event;
        }
        static void *do_monitor(struct ss_thread_buffer *thread_buf)
        {
            ss_event *this_class = (ss_event *)thread_buf->buf;
            assert(this_class);
            if (this_class->event_choice)
            {
                this_class->current_event = this_class->event_choice(this_class->event_params,
                                                                     this_class->current_event,
                                                                     do_event, this_class);

            }
            return NULL;
        }
    private:
        static void do_event(int event, class ss_event *this_class)
        {
            auto it = this_class->store_cmds.find(event);
            if (it != this_class->store_cmds.end())
            {
                for (int i = 0; i < (int)it->second.size(); i++)
                {
                    run_cmd(it->second[i].c_str());
                }
            }
        }
        map<int, vector<string>> store_cmds;
        int (*event_choice)(const vector<string> &, int, void (*p)(int, class ss_event *), class ss_event *);
        vector<string> event_params;
        int current_event;
        void *thread_handle;
};
#endif
