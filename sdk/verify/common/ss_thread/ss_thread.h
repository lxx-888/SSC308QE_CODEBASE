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
#ifndef _SS_THREAD_
#define _SS_THREAD_

#ifdef __cplusplus
extern "C" {
#endif
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <sys/prctl.h>

/*
 * Thread event manager from SigmaStar.
 */
#define MUTEXCHECK(x)                                              \
    do{                                                            \
        if (x != 0)                                                \
        {                                                          \
            fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
        }                                                          \
    } while(0);
#ifndef ASSERT
#define ASSERT(_x_)                                                                         \
    do  {                                                                                   \
        if ( ! ( _x_ ) )                                                                    \
        {                                                                                   \
            printf("ASSERT FAIL: %s %s %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);     \
            abort();                                                                        \
        }                                                                                   \
    } while (0)
#endif
#ifndef PTH_RET_CHK
// Just Temp Solution
#define PTH_RET_CHK(_pf_)                                                \
    ({                                                                   \
        int r = _pf_;                                                    \
        if ((r != 0) && (r != ETIMEDOUT))                                \
            printf("[PTHREAD] %s: %d: %s\n", __FILE__, __LINE__, #_pf_); \
        r;                                                               \
    })
#endif
#define SS_THREAD_GET_PART_BUFFER(name, type, member, value)                                \
    do{                                                                                        \
        type tmp;                                                                           \
        ss_thread_get_part_buf_data(name, &tmp, &(tmp.member), sizeof(tmp.member), &value); \
    }while(0);
#define SS_THREAD_SET_PART_BUFFER(name, type, member, value)                                \
    do{                                                                                     \
        type tmp;                                                                           \
        ss_thread_set_part_buf_data(name, &tmp, &(tmp.member), sizeof(tmp.member), &value); \
    }while(0);


struct ss_thread_user_data
{
    /*
     * It is because that 'user_data' and 'size' only describe for the data struct,
     * and user wish count the size that do not related to "user_data" which point to.
     * So the 'read_size' is for user to count the real memory size of system memory in some usecase.
     */
    void *data;
    unsigned int size;
    unsigned int real_size;
};
struct ss_thread_buffer
{
    /*
     * Declare for 'ss_thread' internal bufffer.
     */
    void *buf;
    unsigned int size;
};

typedef void *(*ss_thread_signal)(struct ss_thread_buffer *, struct ss_thread_user_data *);
typedef void *(*ss_thread_monitor)(struct ss_thread_buffer *);

struct ss_thread_attr
{
    long monitor_cycle_sec;          /* Monitor wait timeout sec. */
    long monitor_cycle_nsec;         /* Monitor wait timeout nsec. */
    ss_thread_monitor do_monitor;    /* Callback monitor main function. */
    ss_thread_signal do_signal;      /* Do event. */
    ss_thread_signal do_signal_drop; /* Do drop event if set 'is_drop_data' or 'is_drop_event'. */
    char thread_name[128];

    struct ss_thread_buffer in_buf;  /* Use for internal buffer. */

    unsigned char is_drop_data;
    unsigned char is_drop_event;
    unsigned int max_data;
    unsigned int max_event;
    unsigned int sched_priority;     /* Task scheduled priority in fifo policy, range: 1~99, set 0 for scheduled none */

    unsigned char is_reset_timer;    /* Reset timer after get signal. */
};

void *ss_thread_open(struct ss_thread_attr *attr);
int ss_thread_close(void *handle);
int ss_thread_start_monitor(void *handle);
int ss_thread_one_shot(void *handle);
int ss_thread_config_timer(void *handle, long time_out_sec, long time_out_nsec, unsigned char is_reset_timer); //if time_out is 0, use default setting from ss_thread_open
int ss_thread_stop(void *handle);
int ss_thread_send(void *handle, struct ss_thread_user_data *user_data);
int ss_thread_set_buffer(void *handle, void *buf);
int ss_thread_get_buffer(void *handle, void *buf);
int ss_thread_set_part_buf_data(void *handle, void *head, void *part, unsigned int size, void *out_buf);
int ss_thread_get_part_buf_data(void *handle, void *head, void *part, unsigned int size, void *out_buf);

#ifdef __cplusplus
}
#endif

#endif
