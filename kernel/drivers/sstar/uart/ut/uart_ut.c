/*
 * uart_ut.c- Sigmastar
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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <linux/serial.h>
#include <sys/ioctl.h>

static int uart_open(const char *pathname, int flags, int timeout_ms)
{
    int            fd = 0;
    struct termios options;

    if (!(fd = open(pathname, flags)))
    {
        printf("open %s fail !!!\n", pathname);
        return 0;
    }

    fcntl(fd, F_SETFL, 0); // block

    tcgetattr(fd, &options);

    if (!timeout_ms)
    {
        options.c_cc[VMIN]  = 1;
        options.c_cc[VTIME] = 0;
    }
    else
    {
        options.c_cc[VMIN]  = 0;
        options.c_cc[VTIME] = timeout_ms / 100;
    }

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &options);

    return fd;
}

static int uart_close(int fd)
{
    return close(fd);
}

static size_t uart_write(int fd, void *buf, size_t count)
{
    size_t  left_count = count;
    ssize_t real_count = 0;

    do
    {
        real_count = write(fd, buf, left_count);

        if (0 == real_count)
        {
            printf("write timeout !!!\n");
            break;
        }

        if (0 > real_count)
        {
            printf("write fail !!!\n");
            break;
        }

        buf += real_count;
        left_count -= real_count;
    } while (left_count);

    count -= left_count;

    return count;
}

static size_t uart_read(int fd, void *buf, size_t count)
{
    size_t  left_count = count;
    ssize_t real_count = 0;

    do
    {
        real_count = read(fd, buf, left_count);

        if (0 == real_count)
        {
            break;
        }

        if (0 > real_count)
        {
            printf("read fail !!!\n");
            break;
        }

        buf += real_count;
        left_count -= real_count;
    } while (left_count);

    count -= left_count;

    return count;
}

int main(int argc, char **argv)
{
    int         opt;
    int         tty_fd       = 0;
    long        filesize     = 0;
    char *      tty_device   = NULL;
    char *      put_file     = NULL;
    char *      get_file     = NULL;
    char *      write_buffer = NULL;
    char *      read_buffer  = NULL;
    FILE *      stream       = NULL;
    struct stat put_statbuf;
    char *      ack_buf = "uart";

    while ((opt = getopt(argc, argv, "d:p:g:")) != -1)
    {
        switch (opt)
        {
            case 'd':
                tty_device = optarg;
                break;
            case 'p':
                put_file = optarg;
                break;
            case 'g':
                get_file = optarg;
                break;
            case '?':
                printf(
                    "\n -d    tty device \
                    \n -p    put file \
                    \n -g    get file \
                    \n -h    help \n");
                return 0;
                break;
            default:
                printf("Invalid parameter\n");
                break;
        }
    }

    if (!tty_device)
    {
        printf("parameter error\n");
        return -1;
    }

    if (!(tty_fd = uart_open(tty_device, O_RDWR | O_NOCTTY | O_SYNC, 3000)))
    {
        goto out;
    }

    if (put_file)
    {
        stat(put_file, &put_statbuf);

        filesize = put_statbuf.st_size;

        // printf("put file size : %ld\n", filesize);

        if (!(write_buffer = malloc(filesize)))
        {
            printf("malloc write_buffer fail !!!\n");
            goto out;
        }

        if (!(stream = fopen(put_file, "r")))
        {
            printf("fopen %s fail !!!\n", put_file);
            goto out;
        }

        fread(write_buffer, 1, filesize, stream);

        if (filesize != uart_write(tty_fd, write_buffer, filesize))
        {
            printf("write %s fail !!!\n", put_file);
            goto out;
        }

        uart_read(tty_fd, write_buffer, strlen(ack_buf));

        if (!strcmp(write_buffer, ack_buf))
            printf("ack\n");

        goto out;
    }

    if (get_file)
    {
        if (!(read_buffer = malloc(2048)))
        {
            printf("malloc read_buffer fail !!!\n");
            goto out;
        }

        if (!(stream = fopen(get_file, "w")))
        {
            printf("fopen %s fail !!!\n", get_file);
            goto out;
        }

        while (0 != (filesize = uart_read(tty_fd, read_buffer, 2048)))
        {
            fwrite(read_buffer, 1, filesize, stream);
        }

        uart_write(tty_fd, ack_buf, strlen(ack_buf));

        goto out;
    }

out:
    if (tty_fd)
        uart_close(tty_fd);

    if (write_buffer)
        free(write_buffer);

    if (read_buffer)
        free(read_buffer);

    if (stream)
        fclose(stream);

    return 0;
}
