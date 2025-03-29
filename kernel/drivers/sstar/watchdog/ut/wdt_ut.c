/*
 * wdt_ut.c - Sigmastar
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <time.h>
#include <sys/time.h>

#define WATCHDOG_IOCTL_BASE 'W'

#define WDIOC_SETOPTIONS    _IOR(WATCHDOG_IOCTL_BASE, 4, int)
#define WDIOC_KEEPALIVE     _IOR(WATCHDOG_IOCTL_BASE, 5, int)
#define WDIOC_SETTIMEOUT    _IOWR(WATCHDOG_IOCTL_BASE, 6, int)
#define WDIOC_GETTIMEOUT    _IOR(WATCHDOG_IOCTL_BASE, 7, int)
#define WDIOC_SETPRETIMEOUT _IOWR(WATCHDOG_IOCTL_BASE, 8, int)
#define WDIOC_GETPRETIMEOUT _IOR(WATCHDOG_IOCTL_BASE, 9, int)
#define WDIOS_DISABLECARD   0x0001 /* Turn off the watchdog timer */
#define WDIOS_ENABLECARD    0x0002 /* Turn on the watchdog timer */

static int wdt_fd          = -1;
static int watchdog_fd     = -1;
static int keep_alive_flag = 1;

static int stop_watchdog(void)
{
    if (watchdog_fd == -1)
    {
        printf("[%s] failed to open watchdog device\n", __FUNCTION__);
        return -1;
    }

    int option = WDIOS_DISABLECARD;
    ioctl(watchdog_fd, WDIOC_SETOPTIONS, &option);

    if (watchdog_fd != -1)
    {
        close(watchdog_fd);
        watchdog_fd = -1;
    }

    if (wdt_fd != -1)
    {
        close(wdt_fd);
        wdt_fd = -1;
    }

    return 0;
}

static int open_watchdog(void)
{
    watchdog_fd = open("/dev/watchdog", O_WRONLY);
    if (watchdog_fd == -1)
    {
        printf("[%s] failed to open watchdog device (%d)\n", __FUNCTION__, watchdog_fd);
        return -1;
    }

    wdt_fd = open("/dev/wdt", O_RDONLY);
    if (wdt_fd == -1)
    {
        printf("[%s] failed to open wdt device, skip pretimeout case.\n", __FUNCTION__);
    }

    return 0;
}

static int set_watchdog_timeout(int time)
{
    if (watchdog_fd == -1)
    {
        printf("[%s] failed to open watchdog device\n", __FUNCTION__);
        return -1;
    }

    ioctl(watchdog_fd, WDIOC_SETTIMEOUT, &time);

    return 0;
}

static int get_watchdog_timeout(void)
{
    int time;

    if (watchdog_fd == -1)
    {
        printf("[%s] failed to open watchdog device\n", __FUNCTION__);
        return -1;
    }

    ioctl(watchdog_fd, WDIOC_GETTIMEOUT, &time);

    return time;
}

static int set_watchdog_pretimeout(int time)
{
    if (watchdog_fd == -1)
    {
        printf("[%s] failed to open watchdog device\n", __FUNCTION__);
        return -1;
    }

    ioctl(watchdog_fd, WDIOC_SETPRETIMEOUT, &time);

    return 0;
}

static int get_watchdog_pretimeout(void)
{
    int time;

    if (watchdog_fd == -1)
    {
        printf("[%s] failed to open watchdog device\n", __FUNCTION__);
        return -1;
    }

    ioctl(watchdog_fd, WDIOC_GETPRETIMEOUT, &time);

    return time;
}

static int set_watchdog_keep_alive(void)
{
    if (watchdog_fd == -1)
    {
        printf("[%s] failed to open watchdog device\n", __FUNCTION__);
        return -1;
    }

    ioctl(watchdog_fd, WDIOC_KEEPALIVE, 0);

    return 0;
}

static void exit_signal(int sig)
{
    printf("exit watchdog\n");
    stop_watchdog();
    exit(0);
}

static void pretimeout_signal(int sig)
{
    printf("[%s] pretimeout is triggered !!!\n", __FUNCTION__);
}

static int set_watchdog_signal(void)
{
    int flag;

    printf("start recive SIGIO\n");

    signal(SIGIO, pretimeout_signal);

    if (fcntl(wdt_fd, F_SETOWN, getpid()))
    {
        perror("F_SETOWN fcntl");
        return -1;
    }

    flag = fcntl(wdt_fd, F_GETFL);
    if (fcntl(wdt_fd, F_SETFL, flag | FASYNC))
    {
        perror("F_SETFL fcntl");
        return -1;
    }

    return 0;
}

static void timer_signal(int sig)
{
    set_watchdog_keep_alive();
}

static int set_watchdog_timer(int time_ms)
{
    int              ret;
    struct itimerval itv;

    itv.it_interval.tv_sec  = time_ms / 1000;
    itv.it_interval.tv_usec = (time_ms % 1000) * 1000;
    itv.it_value.tv_sec     = time_ms / 1000;
    itv.it_value.tv_usec    = (time_ms % 1000) * 1000;

    ret = setitimer(ITIMER_REAL, &itv, NULL);
    if (ret)
    {
        printf("watchdog set timer error\n");
        return ret;
    }

    signal(SIGALRM, timer_signal);

    return 0;
}

int main(int argc, char* argv[])
{
    int       time         = 5;
    long long itime        = 0;
    char      ctrltype[32] = {0};

    if (argc > 1)
    {
        strcpy(ctrltype, argv[1]);
        printf("ctrl:%s\n", ctrltype);
    }
    else
    {
        printf("===================\n");
        printf("watchdog [start/reset] [time]\n");
        printf("===================\n");
        return -1;
    }

    // set a signal, if press "ctrl+c", it will exit watchdog
    signal(SIGINT, exit_signal);

    if (strcmp("start", ctrltype) == 0)
    {
        if (argc == 3)
        {
            char times[32] = {0};
            strcpy(times, argv[2]);
            time = atoi(times);
        }
        printf("timeout:%ds\n", time);
    }
    else if (strcmp("reset", ctrltype) == 0)
    {
        if (argc == 3)
        {
            char times[32] = {0};
            strcpy(times, argv[2]);
            time = atoi(times);
        }
        printf("timeout:%ds\n", time);

        keep_alive_flag = 0;
        printf("just start the watchdog and don't ping it\n");
    }
    else
    {
        printf("===================\n");
        printf("watchdog [start/reset] [time]\n");
        printf("===================\n");
        return -1;
    }

    if (open_watchdog())
    {
        printf("watchdog open fail\n");
        return -1;
    }

    if ((wdt_fd >= 0) && set_watchdog_signal())
    {
        printf("watchdog signal set fail\n");
        return -1;
    }

    if (time > 0)
    {
        set_watchdog_timeout(time);
        if (get_watchdog_timeout() != time)
        {
            printf("watchdog timeout set fail\n");
            return -1;
        }

        set_watchdog_pretimeout(time / 4);
        if (get_watchdog_pretimeout() != (time / 4))
        {
            printf("watchdog pretimeout set fail\n");
            return -1;
        }
    }
    else
    {
        printf("set_watchdog_timeout err\n");
        return -1;
    }

    if (keep_alive_flag)
    {
        printf("now it will keep watchdog alive...\n");
        itime = (time * 1000) / 2;
        if (set_watchdog_timer(itime))
        {
            printf("set_watchdog_timer err\n");
            return -1;
        }
    }

    while (1)
    {
        sleep(time);
    }

    return 0;
}
