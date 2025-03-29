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
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/prctl.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#include "list.h"
#include "ss_thread.h"

#define add_time(_ts, _sec, _nsec)      \
    do                                  \
    {                                   \
        _ts->tv_sec += _sec;            \
        _ts->tv_nsec += _nsec;          \
        if (_ts->tv_nsec > 1000000000)  \
        {                               \
            _ts->tv_nsec %= 1000000000; \
            _ts->tv_sec++;              \
        }                               \
    } while(0);

#define DEBUG(fmt, args...) //printf("[ss_thread][%s][%d]", __FUNCTION__, __LINE__);printf(fmt, ##args);
#define TRACE(fmt, args...) //printf("[ss_thread][%s][%d]", __FUNCTION__, __LINE__);printf(fmt, ##args);
#define INFO(fmt, args...) //printf("[ss_thread][%s][%d]", __FUNCTION__, __LINE__);printf(fmt, ##args);
#define WARN(fmt, args...) //printf("[ss_thread][%s][%d]", __FUNCTION__, __LINE__);printf(fmt, ##args);
#define ERROR(fmt, args...) printf("[ss_thread][%s][%d]", __FUNCTION__, __LINE__);printf(fmt, ##args);


#define SS_THREAD_HASH_SIZE 128

enum ss_thread_event
{
    E_SS_THREAD_IDLE,
    E_SS_THREAD_DO_USER_DATA,
    E_SS_THREAD_DROP_USER_DATA,
    E_SS_THREAD_DROP_USER_DATA_END,
    E_SS_THREAD_EXIT,
    E_SS_THREAD_START_MONITOR,
    E_SS_THREAD_ONE_SHOT,
    E_SS_THREAD_STOP,
    E_SS_THREAD_CONFIG_TIMER,
};

struct ss_thread_data_node
{
    enum ss_thread_event thread_event;    /* Thread status. */
    struct list_head thread_data_list;    /* Data list node. */
    struct ss_thread_user_data user_data; /* User data. */
};

struct ss_thread_info
{
    pthread_attr_t thread_attr;
    pthread_mutex_t mutex;
    pthread_mutex_t node_mutex;
    pthread_mutex_t data_mutex;
    pthread_cond_t cond;
    pthread_t thread;
    pthread_condattr_t cond_attr;
};

struct ss_thread_node
{
    struct list_head      thread_data_list; /* 'ss_thread' data event list head of struct ss_thread_data_node.i */
    struct ss_thread_attr attr;             /* User setting. */
    struct ss_thread_info info;             /* Info of thread. */
    unsigned int          data_size;        /* List data size count. */
    unsigned int          event_count;      /* List count. */
};

static int _ss_thread_push_event(struct ss_thread_node *node,
                                 struct ss_thread_user_data *data,
                                 enum ss_thread_event status)
{
    struct ss_thread_data_node *data_node = NULL;

    ASSERT(node);
    data_node = (struct ss_thread_data_node *)malloc(sizeof(struct ss_thread_data_node));
    ASSERT(data_node);
    INFO("Malloc Node 0x%lx\n", (unsigned long)data_node);
    data_node->user_data.size= 0;
    data_node->user_data.real_size = 0;
    if (data != NULL)
    {
        memcpy(&data_node->user_data, data, sizeof(struct ss_thread_user_data));
    }
    data_node->thread_event = status;
    pthread_mutex_lock(&node->info.data_mutex);
    list_add_tail(&data_node->thread_data_list, &node->thread_data_list);
    node->event_count++;
    node->data_size += data_node->user_data.real_size;
    INFO("[name:%s]: Real Size: %d Node: %d, Event %d\n", node->attr.thread_name, data_node->user_data.real_size, node->data_size, node->event_count);
    if (status == E_SS_THREAD_DO_USER_DATA)
    {
        struct ss_thread_data_node *data_node_drop = NULL;
        struct ss_thread_data_node *data_node_drop_end = NULL;

        if (node->data_size > node->attr.max_data && node->attr.is_drop_data)
        {
            ERROR("[thread %s]: Event data is over range %d[max: %d]! Drop data event!\n", node->attr.thread_name, node->data_size, node->attr.max_data);
            data_node_drop = (struct ss_thread_data_node *)malloc(sizeof(struct ss_thread_data_node));
            ASSERT(data_node_drop);
            data_node_drop->user_data.data = NULL;
            data_node_drop->user_data.size = 0;
            data_node_drop->user_data.real_size = 0;
            data_node_drop->thread_event = E_SS_THREAD_DROP_USER_DATA;
            data_node_drop_end = (struct ss_thread_data_node *)malloc(sizeof(struct ss_thread_data_node));
            ASSERT(data_node_drop_end);
            data_node_drop_end->user_data.size = 0;
            data_node_drop_end->user_data.real_size = 0;
            data_node_drop_end->user_data.data = NULL;
            data_node_drop_end->thread_event = E_SS_THREAD_DROP_USER_DATA_END;
            list_add(&data_node_drop->thread_data_list, &node->thread_data_list);
            list_add_tail(&data_node_drop_end->thread_data_list, &node->thread_data_list);
            node->event_count += 2;
            pthread_mutex_unlock(&node->info.data_mutex);
            return -1;
        }
        if (node->event_count > node->attr.max_event && node->attr.is_drop_event)
        {
            ERROR("[thread %s]: Event list is over range %d[max: %d]! Drop data event!\n", node->attr.thread_name, node->event_count, node->attr.max_event);
            data_node_drop = (struct ss_thread_data_node *)malloc(sizeof(struct ss_thread_data_node));
            ASSERT(data_node_drop);
            data_node_drop->user_data.size= 0;
            data_node_drop->user_data.real_size = 0;
            data_node_drop->user_data.data = NULL;
            data_node_drop->thread_event = E_SS_THREAD_DROP_USER_DATA;
            data_node_drop_end = (struct ss_thread_data_node *)malloc(sizeof(struct ss_thread_data_node));
            ASSERT(data_node_drop_end);
            data_node_drop_end->user_data.size = 0;
            data_node_drop_end->user_data.real_size = 0;
            data_node_drop_end->user_data.data = NULL;
            data_node_drop_end->thread_event = E_SS_THREAD_DROP_USER_DATA_END;
            list_add(&data_node_drop->thread_data_list, &node->thread_data_list);
            list_add_tail(&data_node_drop_end->thread_data_list, &node->thread_data_list);
            node->event_count += 2;
            pthread_mutex_unlock(&node->info.data_mutex);
            return -1;
        }
    }
    pthread_mutex_unlock(&node->info.data_mutex);
    return 0;
}
static enum ss_thread_event _ss_thread_pop_event(struct ss_thread_node *node,
                                                 struct ss_thread_user_data *user_data)
{
    struct ss_thread_data_node *data_node = NULL;
    enum ss_thread_event event;

    ASSERT(node);
    ASSERT(user_data);
    pthread_mutex_lock(&node->info.data_mutex);
    if (list_empty(&node->thread_data_list))
    {
        pthread_mutex_unlock(&node->info.data_mutex);
        return E_SS_THREAD_IDLE;
    }
    data_node = list_first_entry(&node->thread_data_list, struct ss_thread_data_node, thread_data_list);
    list_del(&data_node->thread_data_list);
    node->event_count--;
    node->data_size -= data_node->user_data.real_size;
    pthread_mutex_unlock(&node->info.data_mutex);
    *user_data = data_node->user_data;
    event = data_node->thread_event;
    INFO("Free Node 0x%lx\n", (unsigned long)data_node);
    free(data_node);
    return event;
}
static void _ss_thread_event_monitor(struct ss_thread_node *node, struct timespec *start_time, struct timespec *step_time,
                                     unsigned char *is_run_one_shot, unsigned char *is_run)
{
    unsigned short drop_event_count= 0;
    struct ss_thread_user_data user_data;
    enum ss_thread_event event;

    while (1)
    {
        event = _ss_thread_pop_event(node, &user_data);
        if (E_SS_THREAD_IDLE == event)
        {
            break;
        }
        switch (event)
        {
            case E_SS_THREAD_EXIT:
                {
                    INFO("Exit thread id:[%x], name[%s]\n", (int)node->info.thread, node->attr.thread_name);
                    *is_run = 0;
                    step_time->tv_sec = 0;
                    step_time->tv_nsec = 0;
                }
                break;
            case E_SS_THREAD_STOP:
                {
                    INFO("Stop thread id:[%x], name[%s]\n", (int)node->info.thread, node->attr.thread_name);
                    step_time->tv_sec = -1;
                    step_time->tv_nsec = -1;
                }
                break;
            case E_SS_THREAD_START_MONITOR:
                {
                    step_time->tv_sec = node->attr.monitor_cycle_sec;
                    step_time->tv_nsec = node->attr.monitor_cycle_nsec;
                    clock_gettime(CLOCK_MONOTONIC, start_time);
                    INFO("Start thread monitor id:[%x], name[%s], time[%ld,%ld]\n",
                         (int)node->info.thread, node->attr.thread_name, step_time->tv_sec, step_time->tv_nsec);
                }
                break;
            case E_SS_THREAD_ONE_SHOT:
                {
                    if (node->attr.do_monitor && !step_time->tv_nsec && !step_time->tv_sec)
                    {
                        node->attr.do_monitor(&node->attr.in_buf);
                    }
                    else
                    {
                        *is_run_one_shot = 1;
                        clock_gettime(CLOCK_MONOTONIC, start_time);
                        step_time->tv_sec = node->attr.monitor_cycle_sec;
                        step_time->tv_nsec = node->attr.monitor_cycle_nsec;
                    }
                    INFO("One shot thread id::[%x], name[%s], time[%ld,%ld]\n",
                         (int)node->info.thread, node->attr.thread_name, step_time->tv_sec, step_time->tv_nsec);
                }
                break;
            case E_SS_THREAD_CONFIG_TIMER:
                {
                    step_time->tv_sec = node->attr.monitor_cycle_sec;
                    step_time->tv_nsec = node->attr.monitor_cycle_nsec;
                    clock_gettime(CLOCK_MONOTONIC, start_time);
                    INFO("Config  thread id::[%x], name[%s], time[%ld,%ld]\n",
                         (int)node->info.thread, node->attr.thread_name, step_time->tv_sec, step_time->tv_nsec);
                }
                break;
            case E_SS_THREAD_DO_USER_DATA:
                {
                    if (drop_event_count)
                    {
                        //ERROR("Do user data thread id:[%x], name[%s] Drop event!\n", (int)node->info.thread, node->attr.thread_name);
                        if (node->attr.do_signal_drop)
                        {
                            node->attr.do_signal_drop(&node->attr.in_buf, &user_data);
                        }
                    }
                    else
                    {
                        INFO("Do user data thread id:[%x], name[%s]\n", (int)node->info.thread, node->attr.thread_name);
                        if (node->attr.do_signal)
                        {
                            node->attr.do_signal(&node->attr.in_buf, &user_data);
                        }
                    }
                    if (user_data.size != 0)
                    {
                        INFO("Free Node Data 0x%lx\n", (unsigned long)user_data.data);
                        free(user_data.data);
                        user_data.data = NULL;
                    }
                }
                break;
            case E_SS_THREAD_DROP_USER_DATA:
                {
                    drop_event_count++;
                    ERROR("T[%s][%x] Drop event! cnt: %d\n", node->attr.thread_name, (int)node->info.thread, drop_event_count);
                }
                break;
            case E_SS_THREAD_DROP_USER_DATA_END:
                {
                    drop_event_count--;
                    ERROR("T[%s][%x] Drop event! cnt: %d\n", node->attr.thread_name, (int)node->info.thread, drop_event_count);
                }
                break;
            default:
                {
                    ERROR("T[%s][%x] Error ss_thread thread event.id:[%d],", node->attr.thread_name, (int)node->info.thread, event);
                }
                break;
        }
    }
    return;
}
static int _ss_thread_event_wait(struct ss_thread_node *node, struct timespec *time_start,
                                 const struct timespec *time_step, unsigned char is_reset_timer)
{
    int cond_wait_ret = 0;

    if (!time_step->tv_sec && !time_step->tv_nsec)
    {
        return ETIMEDOUT;
    }
    pthread_mutex_lock(&node->info.mutex);
    pthread_mutex_lock(&node->info.data_mutex);
    if (!list_empty(&node->thread_data_list))
    {
        pthread_mutex_unlock(&node->info.data_mutex);
        PTH_RET_CHK(pthread_mutex_unlock(&node->info.mutex));
        return 0;
    }
    pthread_mutex_unlock(&node->info.data_mutex);
    switch (time_step->tv_sec)
    {
        case -1:
        {
            cond_wait_ret = pthread_cond_wait(&node->info.cond, &node->info.mutex);
        }
        break;
        default:
        {
            struct timespec cur_time;
            if (is_reset_timer)
            {
                clock_gettime(CLOCK_MONOTONIC, time_start);
            }
            add_time(time_start, time_step->tv_sec, time_step->tv_nsec);
            clock_gettime(CLOCK_MONOTONIC, &cur_time);
            if (time_start->tv_sec < cur_time.tv_sec
                || (time_start->tv_sec == cur_time.tv_sec ? (time_start->tv_nsec < cur_time.tv_nsec) : 0))
            {
                pthread_mutex_unlock(&node->info.mutex);
                /*Stop a while and then Stat Monitor.*/
                return ETIMEDOUT;
            }
            cond_wait_ret = pthread_cond_timedwait(&node->info.cond, &node->info.mutex, time_start);
        }
        break;
    }
    pthread_mutex_unlock(&node->info.mutex);
    return cond_wait_ret;
}
static void *_ss_thread_main(void * arg)
{
    int cond_wait_ret = 0;
    unsigned char is_run = 1;
    unsigned char is_run_one_shot = 0;
    struct timespec start_time = {-1, -1};
    struct timespec step_time = {-1, -1};
    struct ss_thread_node *node = (struct ss_thread_node *)arg;

    ASSERT(node);
    if (strlen(node->attr.thread_name))
    {
        prctl(PR_SET_NAME, (unsigned long)node->attr.thread_name);
    }
    while (1)
    {
        _ss_thread_event_monitor(node, &start_time, &step_time, &is_run_one_shot, &is_run);
        if (!is_run)
        {
            INFO("SS_THREAD name [%s]: Bye bye~\n", node->attr.thread_name);
            break;
        }
        if (node->attr.do_monitor && !step_time.tv_nsec && !step_time.tv_sec)
        {
            node->attr.do_monitor(&node->attr.in_buf);
            continue;
        }
        PTH_RET_CHK(pthread_mutex_lock(&node->info.node_mutex));
        if (node->attr.do_monitor && cond_wait_ret == ETIMEDOUT)
        {
            node->attr.do_monitor(&node->attr.in_buf);
            if (is_run_one_shot)
            {
                step_time.tv_sec = -1;
                step_time.tv_nsec = -1;
                is_run_one_shot = 0;
            }
        }
        PTH_RET_CHK(pthread_mutex_unlock(&node->info.node_mutex));
        cond_wait_ret = _ss_thread_event_wait(node, &start_time, &step_time, node->attr.is_reset_timer);
    }
    return NULL;
}
static void _ss_thread_cond_signal(struct ss_thread_node *node)
{
    PTH_RET_CHK(pthread_mutex_lock(&node->info.mutex));
    PTH_RET_CHK(pthread_cond_signal(&node->info.cond));
    PTH_RET_CHK(pthread_mutex_unlock(&node->info.mutex));
}
void *ss_thread_open(struct ss_thread_attr *attr)
{
    struct ss_thread_node *node = NULL;

    node = (struct ss_thread_node *)malloc(sizeof(struct ss_thread_node));
    ASSERT(node);
    memset(node, 0, sizeof(struct ss_thread_node));

    /*Init data event list*/
    INIT_LIST_HEAD(&node->thread_data_list);

    /*Copy Attr*/
    memcpy(&node->attr, attr, sizeof(struct ss_thread_attr));

    /*Malloc 'ss_thread' internal buffer size*/
    if (attr->in_buf.size != 0 && attr->in_buf.buf != NULL)
    {
        node->attr.in_buf.buf = (void *)malloc(attr->in_buf.size);
        ASSERT(node->attr.in_buf.buf);
        memcpy(node->attr.in_buf.buf, attr->in_buf.buf, attr->in_buf.size);
    }

    /*Malloc ss_thread info*/
    PTH_RET_CHK(pthread_mutex_init(&node->info.mutex, NULL));
    PTH_RET_CHK(pthread_mutex_init(&node->info.node_mutex, NULL));
    PTH_RET_CHK(pthread_mutex_init(&node->info.data_mutex, NULL));
    PTH_RET_CHK(pthread_attr_init(&node->info.thread_attr));
    if (node->attr.sched_priority)
    {
        struct sched_param param;
        memset(&param, 0, sizeof(struct sched_param));
        PTH_RET_CHK(pthread_attr_setschedpolicy(&node->info.thread_attr, SCHED_FIFO));
        param.__sched_priority = node->attr.sched_priority;
        PTH_RET_CHK(pthread_attr_setschedparam(&node->info.thread_attr, &param));
    }
    PTH_RET_CHK(pthread_condattr_init(&(node->info.cond_attr)));
    PTH_RET_CHK(pthread_condattr_setclock(&(node->info.cond_attr), CLOCK_MONOTONIC));
    PTH_RET_CHK(pthread_cond_init(&(node->info.cond), &(node->info.cond_attr)));
    PTH_RET_CHK(pthread_create(&(node->info.thread), &(node->info.thread_attr), _ss_thread_main, (void *)node));

    return (void *)node;
}
int ss_thread_close(void* handle)
{
    struct ss_thread_node *node = NULL;
    void *ret_val = NULL;

    node = (struct ss_thread_node *)handle;
    if (node == NULL)
    {
        ERROR("handle is NULL!!!\n");
        return -1;
    }
    ASSERT(node);
    _ss_thread_push_event(node, NULL, E_SS_THREAD_EXIT);
    PTH_RET_CHK(pthread_mutex_lock(&node->info.mutex));
    PTH_RET_CHK(pthread_cond_signal(&node->info.cond));
    PTH_RET_CHK(pthread_mutex_unlock(&node->info.mutex));
    PTH_RET_CHK(pthread_join(node->info.thread, &ret_val));
    ASSERT(list_empty(&node->thread_data_list));
    PTH_RET_CHK(pthread_condattr_destroy(&node->info.cond_attr));
    PTH_RET_CHK(pthread_cond_destroy(&node->info.cond));
    PTH_RET_CHK(pthread_attr_destroy(&node->info.thread_attr));
    PTH_RET_CHK(pthread_mutex_destroy(&node->info.data_mutex));
    PTH_RET_CHK(pthread_mutex_destroy(&node->info.node_mutex));
    PTH_RET_CHK(pthread_mutex_destroy(&node->info.mutex));
    DEBUG("Left data node size %d event count %d\n", node->data_size, node->event_count);
    if (node->attr.in_buf.buf
        && node->attr.in_buf.size != 0)
    {
        free(node->attr.in_buf.buf);
    }
    node->attr.in_buf.buf = NULL;
    free(node);
    return 0;
}
int ss_thread_start_monitor(void *handle)
{
    struct ss_thread_node *node = NULL;

    node = (struct ss_thread_node *)handle;
    if (node == NULL)
    {
        ERROR("handle is NULL!!!\n");
        return -1;
    }
    _ss_thread_push_event(node, NULL, E_SS_THREAD_START_MONITOR);
    _ss_thread_cond_signal(node);

    return 0;
}
int ss_thread_one_shot(void *handle)
{
    struct ss_thread_node *node = NULL;

    node = (struct ss_thread_node *)handle;
    if (node == NULL)
    {
        ERROR("handle is NULL!!!\n");
        return -1;
    }
    _ss_thread_push_event(node, NULL, E_SS_THREAD_ONE_SHOT);
    _ss_thread_cond_signal(node);

    return 0;
}
int ss_thread_config_timer(void *handle, long time_out_sec, long time_out_nsec, unsigned char is_reset_timer)
{
    struct ss_thread_node *node = NULL;

    node = (struct ss_thread_node *)handle;
    if (node == NULL)
    {
        ERROR("handle is NULL!!!\n");
        return -1;
    }
    if (node->info.thread == getpid())
    {
        if (time_out_sec != 0)
        {
            node->attr.monitor_cycle_sec = time_out_sec;
        }
        if (time_out_nsec != 0)
        {
            node->attr.monitor_cycle_nsec = time_out_nsec;
        }
        node->attr.is_reset_timer = is_reset_timer;
    }
    else
    {
        PTH_RET_CHK(pthread_mutex_lock(&node->info.node_mutex));
        if (time_out_sec != 0)
        {
            node->attr.monitor_cycle_sec = time_out_sec;
        }
        if (time_out_nsec != 0)
        {
            node->attr.monitor_cycle_nsec = time_out_nsec;
        }
        node->attr.is_reset_timer = is_reset_timer;
        PTH_RET_CHK(pthread_mutex_unlock(&node->info.node_mutex));
        _ss_thread_push_event(node, NULL, E_SS_THREAD_CONFIG_TIMER);
        _ss_thread_cond_signal(node);
    }
    return 0;
}
int ss_thread_stop(void *handle)
{
    struct ss_thread_node *node = NULL;

    node = (struct ss_thread_node *)handle;
    if (node == NULL)
    {
        ERROR("handle is NULL!!!\n");
        return -1;
    }
    _ss_thread_push_event(node, NULL, E_SS_THREAD_STOP);
    _ss_thread_cond_signal(node);
    return 0;
}
int ss_thread_send(void *handle, struct ss_thread_user_data *user_data)
{
    struct ss_thread_node *node = NULL;
    struct ss_thread_user_data int_user_data;

    node = (struct ss_thread_node *)handle;
    if (node == NULL)
    {
        ERROR("handle is NULL!!!\n");
        return -1;
    }
    if (user_data && user_data->size != 0)
    {
        int_user_data = *user_data;
        int_user_data.data = (void *)malloc(user_data->size);
        ASSERT(int_user_data.data);
        INFO("Malloc Node Data 0x%lx\n", (unsigned long)int_user_data.data);
        memcpy(int_user_data.data, user_data->data, user_data->size);
        user_data = &int_user_data;
    }
    _ss_thread_push_event(node, user_data, E_SS_THREAD_DO_USER_DATA);
    _ss_thread_cond_signal(node);

    return 0;
}
int ss_thread_set_buffer(void *handle, void *data)
{
    struct ss_thread_node *node = NULL;

    if(data == NULL)
    {
        ERROR("p or data is NULL!!!\n");
        return -1;
    }
    node = (struct ss_thread_node *)handle;
    if (node == NULL)
    {
        return -1;
    }
    if (node->info.thread == getpid())
    {
        memcpy(node->attr.in_buf.buf, data, node->attr.in_buf.size);
    }
    else
    {
        PTH_RET_CHK(pthread_mutex_lock(&node->info.node_mutex));
        memcpy(node->attr.in_buf.buf, data, node->attr.in_buf.size);
        PTH_RET_CHK(pthread_mutex_unlock(&node->info.node_mutex));
        _ss_thread_cond_signal(node);
    }

    return 0;
}
int ss_thread_get_buffer(void *handle, void *data)
{
    struct ss_thread_node *node = NULL;

    if(data == NULL)
    {
        ERROR("p or data is NULL!!!\n");
        return -1;
    }
    node = (struct ss_thread_node *)handle;
    if (node == NULL)
    {
        return -1;
    }
    if (node->info.thread == getpid())
    {
        memcpy(data, node->attr.in_buf.buf, node->attr.in_buf.size);
    }
    else
    {
        PTH_RET_CHK(pthread_mutex_lock(&node->info.node_mutex));
        memcpy(data, node->attr.in_buf.buf, node->attr.in_buf.size);
        PTH_RET_CHK(pthread_mutex_unlock(&node->info.node_mutex));
        _ss_thread_cond_signal(node);
    }

    return 0;
}

int ss_thread_set_part_buf_data(void *handle, void *head, void *part, unsigned int size, void *in_buf)
{
    struct ss_thread_node *node = NULL;
    unsigned int offset = 0;

    if(head == NULL || part == NULL || size == 0)
    {
        ERROR("p or head or part or size is NULL!!!\n");
        return -1;
    }
    node = (struct ss_thread_node *)handle;
    if (node == NULL)
    {
        return -1;
    }
    if (node->info.thread == getpid())
    {
        offset = part - head;
        if (node->attr.in_buf.size != 0 && offset < node->attr.in_buf.size)
        {
            memcpy(node->attr.in_buf.buf + offset, in_buf, size);
        }
        else
        {
            ERROR("######your part of ss_thread buf data addr is error!!!\n");
        }
    }
    else
    {
        PTH_RET_CHK(pthread_mutex_lock(&node->info.node_mutex));
        offset = part - head;
        if (node->attr.in_buf.size != 0 && offset < node->attr.in_buf.size)
        {
            memcpy(node->attr.in_buf.buf + offset, in_buf, size);
        }
        else
        {
            ERROR("######your part of ss_thread buf data addr is error!!!\n");
        }
        PTH_RET_CHK(pthread_mutex_unlock(&node->info.node_mutex));
        _ss_thread_cond_signal(node);
    }
    return 0;
}
int ss_thread_get_part_buf_data(void *handle, void *head, void *part, unsigned int size, void *out_buf)
{
    struct ss_thread_node *node = NULL;
    unsigned int offset= 0;

    if(head == NULL || part == NULL || size == 0)
    {
        ERROR("p or head or part or size is NULL!!!\n");
        return -1;
    }
    node = (struct ss_thread_node *)handle;
    if (node == NULL)
    {
        return -1;
    }
    if (node->info.thread == getpid())
    {
        offset = part - head;
        if (node->attr.in_buf.size != 0 && offset < node->attr.in_buf.size)
        {
            memcpy(out_buf, node->attr.in_buf.buf + offset, size);
        }
        else
        {
            ERROR("######Your part of ss_thread buf data addr is error!!!\n");
        }
    }
    else
    {
        PTH_RET_CHK(pthread_mutex_lock(&node->info.node_mutex));
        offset = part - head;
        if (node->attr.in_buf.size != 0 && offset < node->attr.in_buf.size)
        {
            memcpy(out_buf, node->attr.in_buf.buf + offset, size);
        }
        else
        {
            ERROR("######Your part of ss_thread buf data addr is error!!!\n");
        }
        PTH_RET_CHK(pthread_mutex_unlock(&node->info.node_mutex));
        _ss_thread_cond_signal(node);
    }
    return 0;
}
