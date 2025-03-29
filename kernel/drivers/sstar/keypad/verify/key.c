/*
 * key.c- Sigmastar
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
#include <stdlib.h>
#include <linux/input.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/string.h>

#include <sys/time.h>

#define TIME_LIMIT 30
const unsigned long Converter = 1000;

struct timeval start;
struct timeval now;

int is_repeat(char *key_list, char verify_3[4][3])
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (key_list[j] == verify_3[i][j])
            {
                if (j == 2)
                    return 1;
            }
            else
            {
                break;
            }
        }
    }
    return 0;
}

unsigned long interval_ms()
{
    gettimeofday(&now, NULL);
    return (now.tv_sec * Converter + now.tv_usec / Converter) - (start.tv_sec * Converter + start.tv_usec / Converter);
}

int check_start_time()
{
    start.tv_sec  = now.tv_sec;
    start.tv_usec = now.tv_usec;
}

int sorted(char *list, int n)
{
    for (int i = 0; i < n; i++) // sort
    {
        for (int j = 0; j < n - i - 1; ++j)
        {
            // If the current element is smaller than the next, it is swapped
            if (list[j] > list[j + 1])
            {
                int temp    = list[j];
                list[j]     = list[j + 1];
                list[j + 1] = temp;
            }
        }
    }
    return 0;
}

int test_key1(int fd, struct input_event *ev_key)
{
    int  code       = 0;
    int  count      = 0;
    int  value      = 0;
    char verify[64] = {0};
    printf("enter one key test\n");
    while (1)
    {
        read(fd, ev_key, sizeof(struct input_event));
        code = ev_key->code;
        if ((verify[code] == 0) && (code % 8 != 0))
        {
            verify[code] = 1;
            printf("code = %d,count = %d \n", code, ++count);
            if (count == 49)
            {
                printf("press one key PASS:\n");
                break;
            }
        }
    }
}

int test_key2(int fd, struct input_event *ev_key)
{
    int  code     = 0;
    int  count    = 0;
    int  value    = 0;
    int  key_2[2] = {0};
    char verify_2[64][64];
    printf("enter two key test\n");
    while (1)
    {
        read(fd, ev_key, sizeof(struct input_event));
        code  = ev_key->code;
        value = ev_key->value;
        if (code != 0 && value != 0)
        {
            gettimeofday(&now, NULL);
            // printf("ms
            // :%ld\n",(now.tv_sec*Converter+now.tv_usec/Converter)-(start.tv_sec*Converter+start.tv_usec/Converter));
            if (interval_ms() < TIME_LIMIT)
            {
                int low  = 0;
                int high = 0;
                key_2[1] = code;
                if (key_2[0] < key_2[1])
                {
                    low  = key_2[0];
                    high = key_2[1];
                }
                else if (key_2[0] > key_2[1])
                {
                    low  = key_2[1];
                    high = key_2[0];
                }
                if (!verify_2[low][high])
                {
                    verify_2[low][high] = 1;
                    printf("code1:%d,code2:%d,count = %d\n", low, high, ++count);
                    if (count == 8)
                    {
                        printf("press two key PASS:\n");
                        break;
                    }
                }
            }
            else
            {
                key_2[0] = code;
                key_2[1] = 0;
            }
            check_start_time();
        }
    }
}

int test_key3(int fd, struct input_event *ev_key)
{
    int  code     = 0;
    int  count    = 0;
    int  value    = 0;
    int  num      = 0;
    int  key_3[3] = {0};
    char verify_3[4][3];

    printf("enter three key test\n");
    while (1)
    {
        read(fd, ev_key, sizeof(struct input_event));
        gettimeofday(&now, NULL);
        code  = ev_key->code;
        value = ev_key->value;
        if (code != 0 && value != 0)
        {
            gettimeofday(&now, NULL);
            // printf("ms
            // :%ld\n",(now.tv_sec*Converter+now.tv_usec/Converter)-(start.tv_sec*Converter+start.tv_usec/Converter));
            if (num == 1 && interval_ms() < TIME_LIMIT)
            {
                key_3[num] = code;
                num        = 2;
            }
            else if (num == 2 && interval_ms() < TIME_LIMIT)
            {
                key_3[num]       = code;
                char key_list[3] = {};
                key_list[0]      = key_3[0];
                key_list[1]      = key_3[1];
                key_list[2]      = key_3[2];
                num              = 0;
                sorted(key_list, 3);

                if (!is_repeat(key_list, verify_3))
                {
                    verify_3[count][0] = key_list[0];
                    verify_3[count][1] = key_list[1];
                    verify_3[count][2] = key_list[2];
                    printf("code = %d %d %d count=%d\n", key_list[0], key_list[1], key_list[2], ++count);
                    if (count == 4)
                    {
                        printf("press three key PASS:\n");
                        break;
                    }
                }
            }
            else
            {
                num      = 0;
                key_3[0] = code;
                key_3[1] = 0;
                key_3[2] = 0;
                num      = 1;
            }
            if (num == 1)
            {
                check_start_time();
            }
        }
    }
}

int main(int argc, char *argv[])
{
    int                fd;
    struct input_event ev_key;
    if (argc != 3 && argc != 2)
    {
        printf("Usage:\n");
        printf("%s /dev/input/event0 or /dev/input/event1 + enable\n", argv[0]);
        return 0;
    }
    fd = open(argv[1], O_RDWR);
    if (fd < 0)
    {
        perror("open device buttons");
        exit(1);
    }
    if (argc == 3)
    {
        if (strcmp("enable", argv[2]) == 0)
        {
            test_key1(fd, &ev_key);
            test_key2(fd, &ev_key);
            test_key3(fd, &ev_key);
        }
        else
        {
            printf("%s /dev/input/event0 or /dev/input/event1 + enable\n", argv[0]);
            return 0;
        }
    }

    else if (argc == 2)
    {
        printf("enter normal mode\n");
        while (1)
        {
            read(fd, &ev_key, sizeof(struct input_event));
            printf("type:%d,code:%d,value:%d\n", ev_key.type, ev_key.code, ev_key.value);
        }
    }
    close(fd);
    return 0;
}
