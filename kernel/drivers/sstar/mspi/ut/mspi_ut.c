/*
 * mspi_ut.c- Sigmastar
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
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define UDELAY(a)     usleep(a)

static const char *device = "/dev/spidev0.0";
static uint32_t    mode;
static uint32_t    lsb_mode;
static uint32_t    three_wire_mode;
static uint32_t    status;
static uint8_t     bits = 8;
static char *      input_file;
static char *      output_file;
static uint32_t    speed = 500000;
static uint16_t    delay;
static int         verbose;

uint8_t default_tx[] = {
    0x02, 0x00, 0x00, 0x00, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x25, 0x36, 0x26, 0x75, 0xa4, 0xF0, 0x38, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0C, 0x0D, 0x0E, 0x0F, 0x25, 0x04, 0x25, 0x36, 0x26, 0x75, 0xa4, 0xF0, 0x38, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x25, 0x36, 0x26, 0x75, 0xa4, 0xF0, 0x38, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x25, 0x0C, 0x0D, 0x0E, 0x0F, 0x25, 0x04, 0x0C, 0x0D, 0x0E, 0x0F, 0x25,
    0x04, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x25, 0x36, 0x26, 0x0D, 0x0E, 0x0F, 0x25, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,
    0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x25, 0x36, 0x26, 0x75, 0xa4, 0xF0, 0x38, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x25, 0x36, 0x26, 0x0D, 0x0E, 0x0F, 0x25, 0x0C, 0x0D, 0x0E, 0x0F, 0x25, 0x04, 0x0B, 0x0C,
    0x0D, 0x04, 0x03, 0x02, 0x01, 0x00, 0x01, 0xa4, 0xF0, 0x38, 0x05, 0x06, 0x04, 0x03, 0x02, 0x01, 0x00, 0x0C, 0x0D,
    0x0E, 0x0F, 0x25, 0x04, 0x03, 0x02, 0x01, 0x00, 0x01, 0x05, 0x06, 0x04, 0x04, 0x03, 0x02, 0x01, 0x00, 0x01, 0x03,
    0x02, 0x01, 0x00, 0x0C, 0x0D, 0x0E, 0x0F, 0x25, 0x06, 0x05, 0x02, 0x01, 0x00, 0x01,
};

uint8_t default_rx[ARRAY_SIZE(default_tx)] = {
    0,
};
char *input_tx;

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

/*
 *  Unescape - process hexadecimal escape character
 *      converts shell input "\x23" -> 0x23
 */
static int unescape(char *_dst, char *_src, size_t len)
{
    int          ret = 0;
    int          match;
    char *       src = _src;
    char *       dst = _dst;
    unsigned int ch;

    while (*src)
    {
        if (*src == '\\' && *(src + 1) == 'x')
        {
            match = sscanf(src + 2, "%2x", &ch);
            if (!match)
                printf("malformed input string");

            src += 4;
            *dst++ = (unsigned char)ch;
        }
        else
        {
            *dst++ = *src++;
        }
        ret++;
    }
    return ret;
}
#define FLASH_IS_BUSY (0x01)
#define FLASH_WR_ENAB (0x02)
static int bit_per_word_test(int fd)
{
    int ret = 0;
    // const char tx[2]   = {0xaa, 0x55};
    const char              tx[2]   = {0x9a, 0x71};
    const char              rx[2]   = {0x00, 0x00};
    struct spi_ioc_transfer bit_per = {
        .tx_buf        = (unsigned long)tx,
        .rx_buf        = (unsigned long)rx,
        .len           = 2,
        .delay_usecs   = delay,
        .speed_hz      = speed,
        .bits_per_word = bits,
    };

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &bit_per);
    if (ret < 1)
    {
        printf("can't send %d bit word\n", bits);
        return -1;
    }

    return 0;
}

static char lsb_to_msb_word(char lsb)
{
    char tmp = 0;
    int  i   = 0;
    for (i = 0; i < 8; i++)
        tmp |= ((lsb & (1 << i)) ? 1 : 0) << (7 - i);

    return tmp;
}

static int compare_id(char *rx_lsb, char *rx_msb, int len)
{
    int i = 0;
    for (i = 0; i < len; i++)
    {
        rx_lsb[i] = lsb_to_msb_word(rx_lsb[i]);
        if (rx_lsb[i] != rx_msb[i])
        {
            printf("The data for lsb and msb are different\n");
            return -1;
        }
    }
    return 0;
}

static void transfer(int fd, uint8_t const *tx, uint8_t const *rx, size_t len)
{
    int ret;
    // int out_fd;
    const char tx_wrenable     = 0x06;
    char       tx_msb[3]       = {0x9f, 0x9f, 0x9f};
    char       rx_msb[3]       = {0x00, 0x00, 0x00};
    char       tx_lsb[3]       = {0xf9, 0xf9, 0xf9};
    char       rx_lsb[3]       = {0x00, 0x00, 0x00};
    char       tx_rdstu[3]     = {0x05, 0x00, 0x00};
    char       rx_rdstu[3]     = {0x00, 0x00, 0x00};
    const char tx_erase[4]     = {0x20, 0x00, 0x00, 0x00};
    const char tx_fastread[48] = {0x0b, 0x00, 0x00, 0x00};
    const char tx_3wire_en[4]  = {0x3B, 0x00, 0x00, 0x00};
    const char rx_3wire_da[4]  = {0x00, 0x00, 0x00, 0x00};
    const char tx_3wire_da[8]  = {0x02, 0x00, 0x00, 0x00, 0x0A, 0x0B, 0x0C, 0x0D};

    struct spi_ioc_transfer tr_wren = {
        .tx_buf        = (unsigned long)&tx_wrenable,
        .rx_buf        = 0, //(unsigned long)rx,
        .len           = 1,
        .delay_usecs   = delay,
        .speed_hz      = speed,
        .bits_per_word = bits,
    };

    if (mode & SPI_TX_QUAD)
        tr_wren.tx_nbits = 4;
    else if (mode & SPI_TX_DUAL)
        tr_wren.tx_nbits = 2;
    if (mode & SPI_RX_QUAD)
        tr_wren.rx_nbits = 4;
    else if (mode & SPI_RX_DUAL)
        tr_wren.rx_nbits = 2;
    if (!(mode & SPI_LOOP))
    {
        if (mode & (SPI_TX_QUAD | SPI_TX_DUAL))
            tr_wren.rx_buf = 0;
        else if (mode & (SPI_RX_QUAD | SPI_RX_DUAL))
            tr_wren.tx_buf = 0;
    }

    struct spi_ioc_transfer tr_rdstu = {
        .tx_buf        = (unsigned long)tx_rdstu,
        .rx_buf        = (unsigned long)rx_rdstu,
        .len           = 3,
        .delay_usecs   = delay,
        .speed_hz      = speed,
        .bits_per_word = bits,
    };

    if (mode & SPI_TX_QUAD)
        tr_rdstu.tx_nbits = 4;
    else if (mode & SPI_TX_DUAL)
        tr_rdstu.tx_nbits = 2;
    if (mode & SPI_RX_QUAD)
        tr_rdstu.rx_nbits = 4;
    else if (mode & SPI_RX_DUAL)
        tr_rdstu.rx_nbits = 2;
    if (!(mode & SPI_LOOP))
    {
        if (mode & (SPI_TX_QUAD | SPI_TX_DUAL))
            tr_rdstu.rx_buf = 0;
        else if (mode & (SPI_RX_QUAD | SPI_RX_DUAL))
            tr_rdstu.tx_buf = 0;
    }

    struct spi_ioc_transfer tr_erase = {
        .tx_buf        = (unsigned long)tx_erase,
        .rx_buf        = 0, //(unsigned long)rx_rdstu,
        .len           = 4,
        .delay_usecs   = delay,
        .speed_hz      = speed,
        .bits_per_word = bits,
    };

    if (mode & SPI_TX_QUAD)
        tr_erase.tx_nbits = 4;
    else if (mode & SPI_TX_DUAL)
        tr_erase.tx_nbits = 2;
    if (mode & SPI_RX_QUAD)
        tr_erase.rx_nbits = 4;
    else if (mode & SPI_RX_DUAL)
        tr_erase.rx_nbits = 2;
    if (!(mode & SPI_LOOP))
    {
        if (mode & (SPI_TX_QUAD | SPI_TX_DUAL))
            tr_erase.rx_buf = 0;
        else if (mode & (SPI_RX_QUAD | SPI_RX_DUAL))
            tr_erase.tx_buf = 0;
    }
    struct spi_ioc_transfer tr_tx = {
        .tx_buf        = (unsigned long)tx,
        .rx_buf        = 0, //(unsigned long)rx,
        .len           = len,
        .delay_usecs   = delay,
        .speed_hz      = speed,
        .bits_per_word = bits,
    };

    if (mode & SPI_TX_QUAD)
        tr_tx.tx_nbits = 4;
    else if (mode & SPI_TX_DUAL)
        tr_tx.tx_nbits = 2;
    if (mode & SPI_RX_QUAD)
        tr_tx.rx_nbits = 4;
    else if (mode & SPI_RX_DUAL)
        tr_tx.rx_nbits = 2;
    if (!(mode & SPI_LOOP))
    {
        if (mode & (SPI_TX_QUAD | SPI_TX_DUAL))
            tr_tx.rx_buf = 0;
        else if (mode & (SPI_RX_QUAD | SPI_RX_DUAL))
            tr_tx.tx_buf = 0;
    }
    struct spi_ioc_transfer tr_fstrd = {
        .tx_buf        = (unsigned long)tx_fastread,
        .rx_buf        = (unsigned long)rx,
        .len           = len,
        .delay_usecs   = delay,
        .speed_hz      = speed,
        .bits_per_word = bits,
    };

    if (mode & SPI_TX_QUAD)
        tr_fstrd.tx_nbits = 4;
    else if (mode & SPI_TX_DUAL)
        tr_fstrd.tx_nbits = 2;
    if (mode & SPI_RX_QUAD)
        tr_fstrd.rx_nbits = 4;
    else if (mode & SPI_RX_DUAL)
        tr_fstrd.rx_nbits = 2;
    if (!(mode & SPI_LOOP))
    {
        if (mode & (SPI_TX_QUAD | SPI_TX_DUAL))
            tr_fstrd.rx_buf = 0;
        else if (mode & (SPI_RX_QUAD | SPI_RX_DUAL))
            tr_fstrd.tx_buf = 0;
    }

    struct spi_ioc_transfer tr_msb = {
        .tx_buf        = (unsigned long)tx_msb,
        .rx_buf        = (unsigned long)rx_msb,
        .len           = 3,
        .delay_usecs   = delay,
        .speed_hz      = speed,
        .bits_per_word = bits,
    };

    struct spi_ioc_transfer tr_lsb = {
        .tx_buf        = (unsigned long)tx_lsb,
        .rx_buf        = (unsigned long)rx_lsb,
        .len           = 3,
        .delay_usecs   = delay,
        .speed_hz      = speed,
        .bits_per_word = bits,
    };

    struct spi_ioc_transfer tw_3_wire = {
        .tx_buf        = (unsigned long)tx_3wire_da,
        .rx_buf        = 0,
        .len           = 8,
        .delay_usecs   = delay,
        .speed_hz      = speed,
        .bits_per_word = bits,
    };

    struct spi_ioc_transfer tw_3_wire_en = {
        .tx_buf        = (unsigned long)tx_3wire_en,
        .rx_buf        = 0, //(unsigned long)rx_3wire_da,
        .len           = 4,
        .delay_usecs   = delay,
        .speed_hz      = speed,
        .bits_per_word = bits,
    };

    struct spi_ioc_transfer tr_3_wire = {
        .tx_buf        = 0, //(unsigned long)tx_3wire_en,
        .rx_buf        = (unsigned long)rx_3wire_da,
        .len           = 4,
        .delay_usecs   = delay,
        .speed_hz      = speed,
        .bits_per_word = bits,
    };

    /* LSB test */
    if ((lsb_mode & SPI_LSB_FIRST) && (bits == 8))
    {
        printf("--->msb first test\n");
        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr_msb);
        if (ret < 1)
        {
            printf("can't msb write first\n");
            goto err;
        }

        printf("--->lsb first test\n");
        ret = ioctl(fd, SPI_IOC_WR_LSB_FIRST, &lsb_mode);
        if (ret == -1)
            printf("can't set spi lsb mode\n");
        ret = ioctl(fd, SPI_IOC_RD_LSB_FIRST, &status);
        if (ret == -1)
            printf("can't get spi lsb mode\n");
        ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
        if (ret == -1)
            printf("can't set max speed hz");

        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr_lsb);
        if (ret < 1)
        {
            printf("can't lsb write first\n");
            goto err;
        }

        if (compare_id(rx_lsb, rx_msb, 3))
            goto err;

        printf("--->lsb first test done!\n");
        return;
    }

    /******************** Not an 8-bit data to send *********/
    if (bits != 8)
    {
        printf("The data sent by NOR requires an 8-bit word width\n");
        ret = bit_per_word_test(fd);
        if (ret)
            goto err;

        printf("--->bits word test done!\n");
        return;
    }

    /******************************************* write enable****/
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr_wren);
    if (ret < 1)
    {
        printf("can't send write enable\n");
        goto err;
    }
    /******************************************* read status write enable****/
    ret = 1;
    do
    {
        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr_rdstu);
        if (ret < 1)
        {
            printf("can't read status write enable111\n");
            goto err;
        }
        if (rx_rdstu[2] & FLASH_WR_ENAB)
        {
            ret = 0;
        }
        if ((mode & SPI_CPHA) != ((mode & SPI_CPOL) >> 1) || (mode & SPI_CS_HIGH))
            return;
        UDELAY(5000);
    } while (ret);

    /******************************************* erase page****/
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr_erase);
    if (ret < 1)
    {
        printf("can't send erase page\n");
        goto err;
    }
    /*******************************************read status is busy****/
    ret = 1;
    do
    {
        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr_rdstu);
        if (ret < 1)
        {
            printf("can't read status is busy\n");
            goto err;
        }
        if (!(rx_rdstu[2] & FLASH_IS_BUSY))
        {
            ret = 0;
            break;
        }
        UDELAY(5000);
    } while (ret);

    /******************************************* write enable****/
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr_wren);
    if (ret < 1)
    {
        printf("can't write enable\n");
        goto err;
    }
    /******************************************* read status write enable****/
    ret = 1;
    do
    {
        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr_rdstu);
        if (ret < 1)
        {
            printf("can't read status write enable222\n");
            goto err;
        }
        if (rx_rdstu[2] & FLASH_WR_ENAB)
        {
            ret = 0;
            break;
        }
        UDELAY(5000);
    } while (ret);

    /* 3 wire test */
    if (three_wire_mode & SPI_3WIRE)
    {
        printf("--->3_wire mode test\n");

        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr_wren);
        if (ret < 1)
        {
            printf("can't send write enable\n");
            goto err;
        }
        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tw_3_wire);
        if (ret < 1)
        {
            printf("can't 3-wire write\n");
            goto err;
        }

        ret = ioctl(fd, SPI_IOC_WR_MODE32, &three_wire_mode);
        if (ret == -1)
            printf("can't set spi 3-wire mode\n");

        ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
        if (ret == -1)
            printf("can't set max speed hz");

        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tw_3_wire_en);
        if (ret < 1)
        {
            printf("can't 3-wire write en\n");
            goto err;
        }
        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr_3_wire);
        if (ret < 1)
        {
            printf("can't 3-wire read\n");
            goto err;
        }

        printf("--->3-wire test done!\n");
        return;
    }

    /*******************************************page program ****/
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr_tx);
    if (ret < 1)
    {
        printf("can't send page program\n");
        goto err;
    }
    ret = 1;
    do
    {
        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr_rdstu);
        if (ret < 1)
        {
            printf("can't read status write enable222\n");
            goto err;
        }
        if (!(rx_rdstu[2] & FLASH_IS_BUSY))
        {
            ret = 0;
            break;
        }
        UDELAY(5000);
    } while (ret);

    /*******************************************fast read *****/
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr_fstrd);
    if (ret < 1)
    {
        printf("can't send fast read\n");
        goto err;
    }

    ret = compare_buf(tx + 4, rx + 5, len - 5);
    if (ret)
    {
        printf("--->test failed!\n");
    }
    else
    {
        printf("--->fullduplex success\n");
        printf("--->half transfer success\n");
        printf("--->checkout data success\n");
    }

    printf("--->done\n");
    printf("------------");
err:
    ret = ret;
}

static void print_usage(const char *prog)
{
    printf("Usage: %s [-DsbdlHOLC3]\n", prog);
    puts(
        "  -D --device   device to use (default /dev/spidev1.1)\n"
        "  -s --speed    max speed (Hz)\n"
        "  -d --delay    delay (usec)\n"
        "  -b --bpw      bits per word\n"
        "  -i --input    input data from a file (e.g. \"test.bin\")\n"
        "  -o --output   output data to a file (e.g. \"results.bin\")\n"
        "  -l --loop     loopback\n"
        "  -H --cpha     clock phase\n"
        "  -O --cpol     clock polarity\n"
        "  -L --lsb      least significant bit first\n"
        "  -C --cs-high  chip select active high\n"
        "  -3 --3wire    SI/SO signals shared\n"
        "  -v --verbose  Verbose (show tx buffer)\n"
        "  -p            Send data (e.g. \"1234\\xde\\xad\")\n"
        "  -N --no-cs    no chip select\n"
        "  -R --ready    slave pulls low to pause\n"
        "  -2 --dual     dual transfer\n"
        "  -4 --quad     quad transfer\n");
    exit(1);
}

static void parse_opts(int argc, char *argv[])
{
    while (1)
    {
        static const struct option lopts[] = {
            {"device", 1, 0, 'D'}, {"speed", 1, 0, 's'},  {"delay", 1, 0, 'd'},   {"bpw", 1, 0, 'b'},
            {"input", 1, 0, 'i'},  {"output", 1, 0, 'o'}, {"loop", 0, 0, 'l'},    {"cpha", 0, 0, 'H'},
            {"cpol", 0, 0, 'O'},   {"lsb", 0, 0, 'L'},    {"cs-high", 0, 0, 'C'}, {"3wire", 0, 0, '3'},
            {"no-cs", 0, 0, 'N'},  {"ready", 0, 0, 'R'},  {"dual", 0, 0, '2'},    {"verbose", 0, 0, 'v'},
            {"quad", 0, 0, '4'},   {NULL, 0, 0, 0},
        };
        int c;

        c = getopt_long(argc, argv, "D:s:d:b:i:o:lHOLC3NR24p:v", lopts, NULL);

        if (c == -1)
            break;

        switch (c)
        {
            case 'D':
                device = optarg;
                break;
            case 's':
                speed = atoi(optarg);
                break;
            case 'd':
                delay = atoi(optarg);
                break;
            case 'b':
                bits = atoi(optarg);
                break;
            case 'i':
                input_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            case 'l':
                mode |= SPI_LOOP;
                break;
            case 'H':
                mode |= SPI_CPHA;
                break;
            case 'O':
                mode |= SPI_CPOL;
                break;
            case 'L':
                lsb_mode |= SPI_LSB_FIRST;
                break;
            case 'C':
                mode |= SPI_CS_HIGH;
                break;
            case '3':
                three_wire_mode |= SPI_3WIRE;
                break;
            case 'N':
                mode |= SPI_NO_CS;
                break;
            case 'v':
                verbose = 1;
                break;
            case 'R':
                mode |= SPI_READY;
                break;
            case 'p':
                input_tx = optarg;
                break;
            case '2':
                mode |= SPI_TX_DUAL;
                break;
            case '4':
                mode |= SPI_TX_QUAD;
                break;
            default:
                print_usage(argv[0]);
                break;
        }
    }
    if (mode & SPI_LOOP)
    {
        if (mode & SPI_TX_DUAL)
            mode |= SPI_RX_DUAL;
        if (mode & SPI_TX_QUAD)
            mode |= SPI_RX_QUAD;
    }
}

static void transfer_escaped_string(int fd, char *str)
{
    size_t   size = strlen(str);
    uint8_t *tx;
    uint8_t *rx;

    tx = malloc(size);
    if (!tx)
        printf("can't allocate tx buffer");

    rx = malloc(size);
    if (!rx)
        printf("can't allocate rx buffer");

    size = unescape((char *)tx, str, size);
    transfer(fd, tx, rx, size);
    free(rx);
    free(tx);
}

static void transfer_file(int fd, char *filename)
{
    ssize_t     bytes;
    struct stat sb;
    int         tx_fd;
    uint8_t *   tx;
    uint8_t *   rx;

    if (stat(filename, &sb) == -1)
        printf("can't stat input file");

    tx_fd = open(filename, O_RDONLY);
    if (fd < 0)
        printf("can't open input file");

    tx = malloc(sb.st_size);
    if (!tx)
        printf("can't allocate tx buffer");

    rx = malloc(sb.st_size);
    if (!rx)
        printf("can't allocate rx buffer");

    bytes = read(tx_fd, tx, sb.st_size);
    if (bytes != sb.st_size)
        printf("failed to read input file");

    transfer(fd, tx, rx, sb.st_size);
    free(rx);
    free(tx);
    close(tx_fd);
}
#define MSPI_MUCH_LENGTH (2048)
int main(int argc, char *argv[])
{
    int            ret = 0;
    int            fd;
    unsigned char *tx_much = NULL;
    unsigned char *rx_much = NULL;
    unsigned int   i       = 0;
    unsigned char  j       = 0;
    parse_opts(argc, argv);

    fd = open(device, O_RDWR);
    if (fd < 0)
        printf("can't open device\n");

    tx_much = malloc(MSPI_MUCH_LENGTH);
    rx_much = malloc(MSPI_MUCH_LENGTH);
    memset(tx_much, 0, MSPI_MUCH_LENGTH);
    memset(rx_much, 0, MSPI_MUCH_LENGTH);
    tx_much[0] = 0x02;
    tx_much[1] = 0x00;
    tx_much[2] = 0x00;
    tx_much[3] = 0x00;
    for (i = 4; i < MSPI_MUCH_LENGTH; i++)
    {
        tx_much[i] = j++;
    }

    /*
     * spi mode
     */
    ret = ioctl(fd, SPI_IOC_WR_MODE32, &mode);
    if (ret == -1)
        printf("can't set spi mode");

    ret = ioctl(fd, SPI_IOC_RD_MODE32, &status);
    if (ret == -1)
        printf("can't get spi mode");

    /*
     * bits per word
     */
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1)
        printf("can't set bits per word");

    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &status);
    if (ret == -1)
        printf("can't get bits per word");

    /*
     * max speed hz
     */
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        printf("can't set max speed hz");

    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &status);
    if (ret == -1)
        printf("can't get max speed hz");

    printf("spi mode: 0x%x\n", mode);
    printf("bits per word: %d\n", bits);
    printf("max speed: %d Hz (%d KHz)\n", speed, speed / 1000);

    if (input_tx && input_file)
        printf("only one of -p and --input may be selected");

    printf("------------\n");
    printf("--->start transfer\n");

    if (input_tx)
        transfer_escaped_string(fd, input_tx);
    else if (input_file)
        transfer_file(fd, input_file);
    else
    {
        if (0)
        {
            printf("--->much length test\n");
            transfer(fd, tx_much, rx_much, MSPI_MUCH_LENGTH);
        }
        else
        {
            transfer(fd, default_tx, default_rx, sizeof(default_rx));
        }
    }
    close(fd);

    return ret;
}
