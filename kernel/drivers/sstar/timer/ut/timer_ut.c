/*
 * timer_ut.c - Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */

#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <drv_timer.h>

#define TIMER_TEST_EXPIRE     200
#define HANDLE_CALLBACK_GRACE 100
#define RUNLOOP_TEST_TIMES    4

struct timer_test_record
{
    char         case_name[32];
    unsigned int timer_id;
    ktime_t      start_timestamp;
    ktime_t      expire_timestamps[8];
    unsigned int expire_count;
    char         notice[32];
};

static struct timer_test_record records[512];

void timer_ut_callback(void *pdata)
{
    struct timer_test_record *record = pdata;

    if (record->expire_count < 8)
        record->expire_timestamps[record->expire_count++] = ktime_get();

    return;
}

static int __init sstar_timer_ut_init(void)
{
    unsigned int              timer_id;
    unsigned int              timer_id_2nd;
    sstar_timer_handle        handle;
    sstar_timer_handle        handle_2nd;
    struct timer_test_record *record;
    struct timer_test_record *record_2nd;
    unsigned long long *      timestamp;

    memset((void *)records, 0, sizeof(records));
    record = &records[0];

    pr_err(">>>>>>>>>>>>>>TIMER ONESHOT TEST<<<<<<<<<<<<<<<<<< \n");
    timer_id = 0;
    while ((handle = sstar_timer_register(timer_id, SSTAR_TIMER_MODE_ONESHOT, timer_ut_callback, record)))
    {
        pr_err("timer[%d] ...\n", timer_id);

        strcpy(record->case_name, "ONESHOT_TEST");
        record->timer_id        = timer_id;
        record->start_timestamp = ktime_get();

        sstar_timer_start(handle, TIMER_TEST_EXPIRE);
        msleep(TIMER_TEST_EXPIRE + HANDLE_CALLBACK_GRACE);
        sstar_timer_unregister(handle);

        if (record->expire_count != 1)
            strcpy(record->notice, "FAIL");
        else
            strcpy(record->notice, "SUCCESS");

        record++;
        timer_id++;
    }

    pr_err(">>>>>>>>>>>>>>TIMER RUNLOOP TEST<<<<<<<<<<<<<<<<<< \n");
    timer_id = 0;
    while ((handle = sstar_timer_register(timer_id, SSTAR_TIMER_MODE_RUNLOOP, timer_ut_callback, record)))
    {
        pr_err("timer[%d] ...\n", timer_id);

        strcpy(record->case_name, "RUNLOOP_TEST");
        record->timer_id        = timer_id;
        record->start_timestamp = ktime_get();

        sstar_timer_start(handle, TIMER_TEST_EXPIRE);
        msleep(TIMER_TEST_EXPIRE * RUNLOOP_TEST_TIMES + HANDLE_CALLBACK_GRACE);
        sstar_timer_unregister(handle);

        if (record->expire_count < 4)
            strcpy(record->notice, "FAIL");
        else
            strcpy(record->notice, "SUCCESS");

        record++;
        timer_id++;
    }

    pr_err(">>>>>>>>>>>>>>TIMER MULTIPLE TEST<<<<<<<<<<<<<<<<<<\n");
    timer_id = 0;
    while (timer_id < sstar_timer_device_count())
    {
        timer_id_2nd = 0;
        while (timer_id_2nd < sstar_timer_device_count())
        {
            if ((timer_id_2nd == timer_id) && ++timer_id_2nd >= sstar_timer_device_count())
                break;

            sprintf(record->case_name, "MULT_TEST[%d+%d]", timer_id, timer_id_2nd);
            record->timer_id = timer_id;
            if (!(handle = sstar_timer_register(timer_id, SSTAR_TIMER_MODE_ONESHOT, timer_ut_callback, record)))
                break;

            record_2nd = record + 1;
            sprintf(record_2nd->case_name, "MULT_TEST[%d+%d]", timer_id, timer_id_2nd);
            record_2nd->timer_id = timer_id_2nd;
            if (!(handle_2nd =
                      sstar_timer_register(timer_id_2nd, SSTAR_TIMER_MODE_ONESHOT, timer_ut_callback, record_2nd)))
                goto next_2nd_timer;

            pr_err("timer[%d+%d] ...\n", timer_id, timer_id_2nd);

            record->start_timestamp = ktime_get();
            sstar_timer_start(handle, TIMER_TEST_EXPIRE);

            record_2nd->start_timestamp = ktime_get();
            sstar_timer_start(handle_2nd, TIMER_TEST_EXPIRE);

            msleep(TIMER_TEST_EXPIRE + HANDLE_CALLBACK_GRACE);

            if ((record->expire_count != 1) || (record_2nd->expire_count != 1))
            {
                strcpy(record->notice, "FAIL");
                strcpy(record_2nd->notice, "FAIL");
            }
            else
            {
                strcpy(record->notice, "SUCCESS");
                strcpy(record_2nd->notice, "SUCCESS");
            }

            record += 2;
        next_2nd_timer:
            sstar_timer_unregister(handle);
            sstar_timer_unregister(handle_2nd);
            timer_id_2nd++;
        }
        timer_id++;
    }

    pr_err(">>>>>>>>>>>>>>>>> TEST REPORT <<<<<<<<<<<<<<<<<<<<<\n");
    pr_err("* TIMER_TEST_EXPIRE:\t%d\n", TIMER_TEST_EXPIRE);
    pr_err("* HANDLE_CALLBACK_GRACE:\t%d\n", HANDLE_CALLBACK_GRACE);
    pr_err("* RUNLOOP_TEST_TIMES: \t%d\n\n", RUNLOOP_TEST_TIMES);

    for (record = &records[0]; record->case_name[0] != '\0'; record++)
    {
        pr_err("* case_name:\t%s\n", record->case_name);
        pr_err("  notice:\t%s\n", record->notice);
        pr_err("  timer_id:\t%u\n", record->timer_id);
        pr_err("  start (us):\t%llu\n", ktime_to_us(record->start_timestamp));
        pr_err("  expires (us):\n");
        if (record->expire_count != 0)
        {
            for (timestamp = &record->expire_timestamps[0];
                 ((void *)timestamp <= (void *)&record->expire_timestamps[7]) && (*timestamp != 0); timestamp++)
            {
                pr_err("\t\t%llu diff:%llu\n", ktime_to_us(*timestamp),
                       ktime_to_us(ktime_sub(*timestamp, record->start_timestamp)));
            }
        }
        else
        {
            pr_err("\t\tN/A\n");
        }
        pr_err("\n");
    }

    return 0;
}

static void __exit sstar_timer_ut_exit(void) {}

module_init(sstar_timer_ut_init);
module_exit(sstar_timer_ut_exit);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("Sigmastar Timer Driver UT");
MODULE_LICENSE("GPL");
MODULE_ALIAS("sstar-timer-ut");
