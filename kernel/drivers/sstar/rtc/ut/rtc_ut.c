/*
 * rtc_ut.c- Sigmastar
 *
 * Copyright (c) [2019~2020] SigmaStar Technology.
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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <linux/rtc.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <time.h>
#include <linux/input.h>

#define FLAG_SET   0x0001
#define FLAG_PWR   0x0002
#define FLAG_ALARM 0x0100

#define DEVICE_NAME "/dev/rtc0"

struct rtc_ut_t
{
    int   year;
    int   month;
    int   day;
    int   hour;
    int   minute;
    int   second;
    int   flag;
    int   offset;
    char *event;
    int   hold_time;
};

static struct rtc_time   rtc_tm;
static struct rtc_wkalrm rtc_alm;
static struct rtc_ut_t   rtc_ut;

static void print_usage(const char *prog)
{
    printf("Usage: %s [-was:o:pe:t:]\n", prog);
    puts(
        "  -w --write    set time or alarm (default read)\n"
        "  -a --alarm    read or set alarm or time (default time)\n"
        "  -s --string   specify the time via string (yyyy-mm-dd hh:mm:ss)\n"
        "  -o --offset   specify the offset (unit second)\n"
        "  -p --poweroff hold to poweroff\n"
        "  -e --event    specify the event dev\n"
        "  -t --time     specify the hold time (unit second)\n");
}

static void parse_opts(int argc, char *argv[])
{
    while (1)
    {
        static const struct option lopts[] = {
            {"write", 0, 0, 'w'},    {"alarm", 0, 0, 'a'}, {"string", 1, 0, 's'}, {"offset", 1, 0, 'o'},
            {"poweroff", 0, 0, 'p'}, {"event", 1, 0, 'e'}, {"time", 1, 0, 't'},   {NULL, 0, 0, 0},
        };
        int c;

        c = getopt_long(argc, argv, "was:o:pe:t:", lopts, NULL);

        if (c == -1)
            break;

        switch (c)
        {
            case 'w':
                rtc_ut.flag |= FLAG_SET;
                break;
            case 'a':
                rtc_ut.flag |= FLAG_ALARM;
                break;
            case 's':
                if (sscanf(optarg, "%d-%d-%d %d:%d:%d", &rtc_ut.year, &rtc_ut.month, &rtc_ut.day, &rtc_ut.hour,
                           &rtc_ut.minute, &rtc_ut.second)
                    < 0)
                    print_usage(argv[0]);
                break;
            case 'o':
                rtc_ut.offset = atoi(optarg);
                break;
            case 'p':
                rtc_ut.flag      = FLAG_PWR;
                rtc_ut.event     = "/dev/input/event0";
                rtc_ut.hold_time = 3;
                break;
            case 'e':
                rtc_ut.event = optarg;
                break;
            case 't':
                rtc_ut.hold_time = atoi(optarg);
                break;
            default:
                print_usage(argv[0]);
                break;
        }
    }
}

static void get_time(struct rtc_time *tm, int offset)
{
    time_t     utc;
    struct tm *ret;
    struct tm  now_tm;

    now_tm.tm_sec   = tm->tm_sec;
    now_tm.tm_min   = tm->tm_min;
    now_tm.tm_hour  = tm->tm_hour;
    now_tm.tm_mday  = tm->tm_mday;
    now_tm.tm_mon   = tm->tm_mon;
    now_tm.tm_year  = tm->tm_year;
    now_tm.tm_isdst = 0;

    utc = mktime(&now_tm);
    utc += offset;
    ret = gmtime(&utc);

    tm->tm_sec  = ret->tm_sec;
    tm->tm_min  = ret->tm_min;
    tm->tm_hour = ret->tm_hour;
    tm->tm_mday = ret->tm_mday;
    tm->tm_mon  = ret->tm_mon;
    tm->tm_year = ret->tm_year;
}

static void read_time(void)
{
    int fd;

    fd = open(DEVICE_NAME, O_RDWR);
    if (fd < 0)
    {
        printf(DEVICE_NAME " open fail\n");
        exit(1);
    }

    if (ioctl(fd, RTC_RD_TIME, &rtc_tm))
    {
        printf("RTC_RD_TIME ioctl fail\n");
        goto err;
    }

    printf("current RTC date/time is %d-%d-%d, %02d:%02d:%02d.\n", rtc_tm.tm_mday, rtc_tm.tm_mon + 1,
           rtc_tm.tm_year + 1900, rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);
err:
    close(fd);
}

static void read_alarm(void)
{
    int fd;

    fd = open(DEVICE_NAME, O_RDWR);
    if (fd < 0)
    {
        printf(DEVICE_NAME " open fail\n");
        exit(1);
    }

    if (ioctl(fd, RTC_ALM_READ, &rtc_alm.time))
    {
        printf("RTC_ALM_READ ioctl fail\n");
        goto err;
    }

    printf("RTC alarm date/time is %d-%d-%d, %02d:%02d:%02d.\n", rtc_alm.time.tm_mday, rtc_alm.time.tm_mon + 1,
           rtc_alm.time.tm_year + 1900, rtc_alm.time.tm_hour, rtc_alm.time.tm_min, rtc_alm.time.tm_sec);
err:
    close(fd);
}

static void set_time(void)
{
    int fd;

    fd = open(DEVICE_NAME, O_RDWR);
    if (fd < 0)
    {
        printf(DEVICE_NAME " open fail\n");
        exit(1);
    }

    if (rtc_ut.offset)
    {
        if (ioctl(fd, RTC_RD_TIME, &rtc_tm))
        {
            printf("RTC_RD_TIME ioctl fail\n");
            goto err;
        }

        get_time(&rtc_tm, rtc_ut.offset);

        printf("%d-%d-%d, %02d:%02d:%02d.\n", rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900, rtc_tm.tm_hour,
               rtc_tm.tm_min, rtc_tm.tm_sec);

        if (ioctl(fd, RTC_SET_TIME, &rtc_tm))
        {
            printf("RTC_SET_TIME ioctl fail\n");
            goto err;
        }
    }
    else
    {
        rtc_tm.tm_year = rtc_ut.year - 1900;
        rtc_tm.tm_mon  = rtc_ut.month - 1;
        rtc_tm.tm_mday = rtc_ut.day;
        rtc_tm.tm_hour = rtc_ut.hour;
        rtc_tm.tm_min  = rtc_ut.minute;
        rtc_tm.tm_sec  = rtc_ut.second;

        printf("%d-%d-%d, %02d:%02d:%02d.\n", rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900, rtc_tm.tm_hour,
               rtc_tm.tm_min, rtc_tm.tm_sec);

        if (ioctl(fd, RTC_SET_TIME, &rtc_tm))
        {
            printf("RTC_SET_TIME time is error\n");
            goto err;
        }
    }

err:
    close(fd);
}

static void alarm_signal(int sig)
{
    printf("[%s] alarm trigger !!!\n", __FUNCTION__);
}

void set_alarm(void)
{
    int       fd;
    int       flag;
    time_t    now;
    time_t    alm;
    struct tm now_tm;
    struct tm alm_tm;
    int       wait = 0;

    fd = open(DEVICE_NAME, O_RDWR);
    if (fd < 0)
    {
        printf(DEVICE_NAME " open fail\n");
        exit(1);
    }

    if (ioctl(fd, RTC_RD_TIME, &rtc_tm))
    {
        printf("RTC_RD_TIME ioctl fail\n");
        goto err;
    }

    if (rtc_ut.offset)
    {
        if (rtc_ut.offset <= 0)
        {
            printf("set time is earlier than current time %d seconds\n", abs(rtc_ut.offset));
            goto err;
        }

        wait = rtc_ut.offset;

        rtc_alm.enabled = 1;
        memcpy(&rtc_alm.time, &rtc_tm, sizeof(struct rtc_time));

        printf("date : %d-%d-%d, %02d:%02d:%02d.\n", rtc_alm.time.tm_mday, rtc_alm.time.tm_mon + 1,
               rtc_alm.time.tm_year + 1900, rtc_alm.time.tm_hour, rtc_alm.time.tm_min, rtc_alm.time.tm_sec);

        get_time(&rtc_alm.time, rtc_ut.offset);

        printf("alarm : %d-%d-%d, %02d:%02d:%02d.\n", rtc_alm.time.tm_mday, rtc_alm.time.tm_mon + 1,
               rtc_alm.time.tm_year + 1900, rtc_alm.time.tm_hour, rtc_alm.time.tm_min, rtc_alm.time.tm_sec);
    }
    else
    {
        rtc_alm.enabled      = 1;
        rtc_alm.time.tm_year = rtc_ut.year - 1900;
        rtc_alm.time.tm_mon  = rtc_ut.month - 1;
        rtc_alm.time.tm_mday = rtc_ut.day;
        rtc_alm.time.tm_hour = rtc_ut.hour;
        rtc_alm.time.tm_min  = rtc_ut.minute;
        rtc_alm.time.tm_sec  = rtc_ut.second;

        now_tm.tm_sec   = rtc_tm.tm_sec;
        now_tm.tm_min   = rtc_tm.tm_min;
        now_tm.tm_hour  = rtc_tm.tm_hour;
        now_tm.tm_mday  = rtc_tm.tm_mday;
        now_tm.tm_mon   = rtc_tm.tm_mon;
        now_tm.tm_year  = rtc_tm.tm_year;
        now_tm.tm_isdst = 0;

        alm_tm.tm_sec   = rtc_alm.time.tm_sec;
        alm_tm.tm_min   = rtc_alm.time.tm_min;
        alm_tm.tm_hour  = rtc_alm.time.tm_hour;
        alm_tm.tm_mday  = rtc_alm.time.tm_mday;
        alm_tm.tm_mon   = rtc_alm.time.tm_mon;
        alm_tm.tm_year  = rtc_alm.time.tm_year;
        alm_tm.tm_isdst = 0;

        now  = mktime(&now_tm);
        alm  = mktime(&alm_tm);
        wait = (int)difftime(alm, now);
        if (wait <= 0)
        {
            printf("set time is earlier than current time %d seconds\n", abs(wait));
            goto err;
        }

        printf("%d-%d-%d, %02d:%02d:%02d.\n", rtc_alm.time.tm_mday, rtc_alm.time.tm_mon + 1,
               rtc_alm.time.tm_year + 1900, rtc_alm.time.tm_hour, rtc_alm.time.tm_min, rtc_alm.time.tm_sec);
    }

    signal(SIGIO, alarm_signal);

    if (fcntl(fd, F_SETOWN, getpid()))
    {
        perror("F_SETOWN fcntl");
        goto err;
    }
    flag = fcntl(fd, F_GETFL);
    if (fcntl(fd, F_SETFL, flag | FASYNC))
    {
        perror("F_SETFL fcntl");
        goto err;
    }
    if (ioctl(fd, RTC_ALM_SET, &rtc_alm.time))
    {
        perror("RTC_ALM_SET ioctl");
        goto err;
    }
    if (ioctl(fd, RTC_AIE_ON, NULL))
    {
        perror("RTC_AIE_ON ioctl");
        goto err;
    }

    sleep(wait + 2);

err:
    close(fd);
}

static void poweroff_handle(union sigval sig)
{
    printf("timer %d seconds timeout\n", rtc_ut.hold_time);
    popen("poweroff", "r");
}

static void hold_poweroff(void)
{
    int                fd;
    int                ret;
    timer_t            timer;
    struct sigevent    evp;
    struct input_event event;
    struct itimerspec  ts;

    if (rtc_ut.hold_time >= 8)
    {
        printf("hold time must be less than 8 seconds\n");
        return;
    }

    fd = open(rtc_ut.event, O_RDONLY);
    if (fd < 0)
    {
        printf("%s open fail\n", rtc_ut.event);
        exit(1);
    }

    while (1)
    {
        bzero(&event, sizeof(event));
        ret = read(fd, &event, sizeof(event));
        if (ret < 0)
        {
            printf("%s read fail\n", rtc_ut.event);
            goto err;
        }

        if ((event.type == EV_KEY) && (event.code == KEY_WAKEUP) && (event.value))
        {
            printf("KEY_WAKEUP pressed\n");

            evp.sigev_notify          = SIGEV_THREAD;
            evp.sigev_notify_function = poweroff_handle;

            ret = timer_create(CLOCK_REALTIME, &evp, &timer);
            if (ret)
            {
                printf("%s create timer fail\n", rtc_ut.event);
                goto err;
            }

            ts.it_interval.tv_sec  = 0;
            ts.it_interval.tv_nsec = 0;
            ts.it_value.tv_sec     = rtc_ut.hold_time;
            ts.it_value.tv_nsec    = 0;

            ret = timer_settime(timer, 0, &ts, NULL);
            if (ret)
            {
                printf("%s set timer fail\n", rtc_ut.event);
                goto err;
            }
        }

        if ((event.type == EV_KEY) && (event.code == KEY_WAKEUP) && (!event.value))
        {
            printf("KEY_WAKEUP released\n");
            timer_delete(timer);
            break;
        }
    }

err:
    close(fd);
}

int main(int argc, char *argv[])
{
    int ret = 0;

    parse_opts(argc, argv);

    switch (rtc_ut.flag)
    {
        case 0:
            read_time();
            break;
        case FLAG_ALARM:
            read_alarm();
            break;
        case FLAG_SET:
            set_time();
            break;
        case FLAG_SET | FLAG_ALARM:
            set_alarm();
            break;
        case FLAG_PWR:
            hold_poweroff();
            break;
    }

    return ret;
}
