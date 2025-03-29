/*
 * ut_i2cdev.c- Sigmastar
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
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <time.h>

#define EEPROM_RAM_SIZE        (64)
#define BUF_DATA_LENGTH        (EEPROM_RAM_SIZE * 2)
#define DOUBLE_BUF_DATA_LENGTH (BUF_DATA_LENGTH + 2)

#define UT_I2C_WRITE    1
#define UT_I2C_READ     1
#define UT_I2C_DATA_CHK 1

#if UT_I2C_WRITE
static int set_i2c_register(int file, unsigned int addr, unsigned int reg, unsigned char *buf, unsigned int len)
{
    struct i2c_rdwr_ioctl_data ioctl_packets;
    struct i2c_msg             i2c_message[1];
    unsigned char *            write_buf;
    int                        i;

    write_buf = (unsigned char *)malloc(DOUBLE_BUF_DATA_LENGTH);

    i2c_message[0].addr  = addr;
    i2c_message[0].flags = 0;
    i2c_message[0].len   = len + 2;
    i2c_message[0].buf   = write_buf;

    write_buf[0] = (reg >> 8) & 0x00ff;
    write_buf[1] = reg & 0x00ff;

    /* The first byte indicates which register we‘ll write */
    /*
     * The second byte indicates the value to write.  Note that for many
     * devices, we can write multiple, sequential registers at once by
     * simply making outbuf bigger.
     */
    for (i = 0; i < len; i++)
    {
        write_buf[i + 2] = buf[i];
    }

    /* Transfer the i2c packets to the kernel and verify it worked */
    ioctl_packets.msgs  = i2c_message;
    ioctl_packets.nmsgs = 1;
    do
    {
        if (ioctl(file, I2C_RDWR, &ioctl_packets) < 0)
        {
            free(write_buf);
            return 1;
        }
    } while (0);

    free(write_buf);
    return 0;
}
#endif

#if UT_I2C_READ
static int get_i2c_register(int file, unsigned int addr, unsigned int reg, unsigned char *buf, unsigned int len)
{
    unsigned char              reg_buf[2];
    struct i2c_rdwr_ioctl_data ioctl_packets;
    struct i2c_msg             i2c_message[2];

    /*
     * In order to read a register, we first do a "dummy write" by writing
     * 0 bytes to the register we want to read from.  This is similar to
     * the packet in set_i2c_register, except it‘s 1 byte rather than 2.
     */
    reg_buf[0]           = (reg >> 8) & 0x00ff;
    reg_buf[1]           = reg & 0x00ff;
    i2c_message[0].addr  = addr;
    i2c_message[0].flags = 0;
    i2c_message[0].len   = sizeof(reg_buf);
    i2c_message[0].buf   = reg_buf;

    /* The data will get returned in this structure */
    i2c_message[1].addr  = addr;
    i2c_message[1].flags = I2C_M_RD /* | I2C_M_NOSTART*/;
    i2c_message[1].len   = len;
    i2c_message[1].buf   = buf;

    /* Send the request to the kernel and get the result back */
    ioctl_packets.msgs  = i2c_message;
    ioctl_packets.nmsgs = 2;
    do
    {
        if (ioctl(file, I2C_RDWR, &ioctl_packets) < 0)
        {
            return 1;
        }
    } while (0);

    return 0;
}
#endif

#if UT_I2C_DATA_CHK
int compare_buf(const unsigned char *buf1, const unsigned char *buf2, const unsigned int len)
{
    int i;

    for (i = 0; i < len; i++)
    {
        if (buf1[i] - buf2[i])
        {
            printf("checkout index %d error\n", i);
            return 1;
        }
    }
    return 0;
}
#endif

int main(int argc, char **argv)
{
    unsigned int   i;
    unsigned int   count;
    unsigned int   remainder;
    unsigned char *in_buf;
    unsigned char *out_buf;
    char *         speed;
    char           i2c_name[32];
    char           speed_path[64];
    int            i2c_file;
    int            speed_file;
    int            ret, len, addr, reg, bus;

    if (argc < 5)
    {
        printf("please input cmd like: './ut_i2cdev [bus] [slv addr] [reg addr] [len] [speed]'\n");
        exit(1);
    }

    bus  = strtol(argv[1], NULL, 0);
    addr = strtol(argv[2], NULL, 0);
    reg  = strtol(argv[3], NULL, 0);
    len  = strtol(argv[4], NULL, 0);

    // set i2c speed
    if (argc == 6)
    {
        speed = argv[5];
        if (!snprintf(speed_path, sizeof(speed_path), "/sys/class/sstar/i2c%d/speed", bus))
        {
            printf("i2c speed path does not exist\n");
            exit(1);
        }

        if ((speed_file = open(speed_path, O_RDWR)) < 0)
        {
            printf("unable to open i2c%d speed file\n", bus);
            exit(1);
        }

        if ((write(speed_file, speed, strlen(speed)) < 0))
        {
            printf("i2c%d speed change fail\n", bus);
            exit(1);
        }
    }

    // open i2c dev node
    if (!snprintf(i2c_name, sizeof(i2c_name), "/dev/i2c-%d", bus))
    {
        printf("set i2c name error\n");
        exit(1);
    }
    if ((i2c_file = open(i2c_name, O_RDWR)) < 0)
    {
        printf("unable to open i2c control file");
        exit(1);
    }

    in_buf  = (unsigned char *)malloc(BUF_DATA_LENGTH);
    out_buf = (unsigned char *)malloc(BUF_DATA_LENGTH);

    // set write buf data
    for (i = 0; i < EEPROM_RAM_SIZE; i++)
    {
        out_buf[i] = i;
    }

    count     = (len / EEPROM_RAM_SIZE);
    remainder = len % EEPROM_RAM_SIZE;

    for (i = 0; i < count + 1; i++)
    {
        if (i == count)
        {
            len = remainder;
            if (len == 0)
                break;
        }
        else
            len = EEPROM_RAM_SIZE;

#if UT_I2C_WRITE
        ret = set_i2c_register(i2c_file, addr, reg, out_buf, len);
        if (ret)
        {
            printf("i2c-%d write failed\n", bus);
            free(in_buf);
            free(out_buf);
            exit(1);
        }
#endif
        usleep(10000);

#if UT_I2C_READ
        ret = get_i2c_register(i2c_file, addr, reg, in_buf, len);
        if (ret)
        {
            printf("i2c-%d read failed\n", bus);
            free(in_buf);
            free(out_buf);
            exit(1);
        }
#endif

#if UT_I2C_DATA_CHK
        ret = compare_buf(out_buf, in_buf, len);
        if (ret)
        {
            printf("i2c-%d transfer data failed\n", bus);
            free(in_buf);
            free(out_buf);
            exit(1);
        }
#endif
    }

    free(in_buf);
    free(out_buf);
    return 0;
}
