/*
 * gyro_demo.h- Sigmastar
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

#ifndef _GYRO_DEMO_H
#define _GYRO_DEMO_H

#include "gyro_ioctl.h"

#define GYRO_IOC_DIR "/dev/" GYRO_IOC_DEVICNAME

/* Log color related */
#define ASCII_COLOR_RED    "\033[1;31m"
#define ASCII_COLOR_WHITE  "\033[1;37m"
#define ASCII_COLOR_YELLOW "\033[1;33m"
#define ASCII_COLOR_BLUE   "\033[1;36m"
#define ASCII_COLOR_GREEN  "\033[1;32m"
#define ASCII_COLOR_END    "\033[0m"

#define GYRO_LOG_PRINT(fmt, args...) printf("[GYRO IOC]" fmt, ##args)

#define GYRO_LOG_ERR(fmt, args...)                                                                                \
    (                                                                                                             \
        {                                                                                                         \
            do                                                                                                    \
            {                                                                                                     \
                GYRO_LOG_PRINT(ASCII_COLOR_RED "[LOG_ERR]: %s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__, __LINE__, \
                               ##args);                                                                           \
            } while (0);                                                                                          \
        })
#define GYRO_LOG_WRN(fmt, args...)                                                                                   \
    (                                                                                                                \
        {                                                                                                            \
            do                                                                                                       \
            {                                                                                                        \
                GYRO_LOG_PRINT(ASCII_COLOR_YELLOW "[LOG_WRN]: %s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__, __LINE__, \
                               ##args);                                                                              \
            } while (0);                                                                                             \
        })
#define GYRO_LOG_DBG(fmt, args...)                                                                                  \
    (                                                                                                               \
        {                                                                                                           \
            do                                                                                                      \
            {                                                                                                       \
                GYRO_LOG_PRINT(ASCII_COLOR_GREEN "[LOG_DBG]: %s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__, __LINE__, \
                               ##args);                                                                             \
            } while (0);                                                                                            \
        })
#define GYRO_LOG_INFO(fmt, args...)                                                                                 \
    (                                                                                                               \
        {                                                                                                           \
            do                                                                                                      \
            {                                                                                                       \
                GYRO_LOG_PRINT(ASCII_COLOR_GREEN "[LOG_INFO]:%s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__, __LINE__, \
                               ##args);                                                                             \
            } while (0);                                                                                            \
        })
#define GYRO_LOG_ENTER(fmt, args...)                                                                                  \
    (                                                                                                                 \
        {                                                                                                             \
            do                                                                                                        \
            {                                                                                                         \
                GYRO_LOG_PRINT(ASCII_COLOR_BLUE "[LOG_ENTER]:%s[%d]: \n" fmt ASCII_COLOR_END, __FUNCTION__, __LINE__, \
                               ##args);                                                                               \
            } while (0);                                                                                              \
        })
#define GYRO_LOG_EXIT_OK(fmt, args...)                                                                        \
    (                                                                                                         \
        {                                                                                                     \
            do                                                                                                \
            {                                                                                                 \
                GYRO_LOG_PRINT(ASCII_COLOR_BLUE "[LOG_EXIT_OK]:%s[%d]: \n" fmt ASCII_COLOR_END, __FUNCTION__, \
                               __LINE__, ##args);                                                             \
            } while (0);                                                                                      \
        })
#define GYRO_LOG_EMERG(fmt, args...)                                                                                 \
    (                                                                                                                \
        {                                                                                                            \
            do                                                                                                       \
            {                                                                                                        \
                GYRO_LOG_PRINT(ASCII_COLOR_GREEN "[LOG_EMERG]:%s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__, __LINE__, \
                               ##args);                                                                              \
            } while (0);                                                                                             \
        })
#define GYRO_LOG_EXIT_ERR(fmt, args...)                                                                           \
    (                                                                                                             \
        {                                                                                                         \
            do                                                                                                    \
            {                                                                                                     \
                GYRO_LOG_PRINT(ASCII_COLOR_RED "<<<%s[%d] " fmt ASCII_COLOR_END, __FUNCTION__, __LINE__, ##args); \
            } while (0);                                                                                          \
        })

#define PARAM_SAFE(attr, ill_value) CHECK_SUCCESS((attr != ill_value), "parameter is illega\n")
#define CHECK_SUCCESS(condition, err_log, ...)    \
    do                                            \
    {                                             \
        if (!(condition))                         \
        {                                         \
            GYRO_LOG_ERR(err_log, ##__VA_ARGS__); \
            return -1;                            \
        }                                         \
    } while (0);

#define CHECK_SUCCESS_OPS(condition, excp_ops, err_log, ...) \
    do                                                       \
    {                                                        \
        if (!(condition))                                    \
        {                                                    \
            GYRO_LOG_ERR(err_log, ##__VA_ARGS__);            \
            excp_ops;                                        \
        }                                                    \
    } while (0)

#define GYRO_CALL_IOCTL(result, fd, desc, agrs...)             \
    do                                                         \
    {                                                          \
        int ret = 0;                                           \
        ret     = ioctl(fd, desc.ioc_num, ##agrs);             \
        if (ret != 0)                                          \
        {                                                      \
            GYRO_LOG_ERR("%s fail, ret:%d\n", desc.name, ret); \
            result = !result;                                  \
            break;                                             \
        }                                                      \
        GYRO_LOG_INFO("%s success!\n", desc.name);             \
    } while (0)

#define GYRO_IOC_NAME_LEN 128

typedef struct gyro_ioc_desc_s
{
    unsigned int ioc_num;
    char         name[GYRO_IOC_NAME_LEN];
} gyro_ioc_desc_t;

typedef struct gyro_ioc_handle_s
{
    int              ioc_fd;
    gyro_ioc_desc_t* p_ioc_desc;
} gyro_ioc_handle_t;

#endif /* _GYRO_DEMO_H */