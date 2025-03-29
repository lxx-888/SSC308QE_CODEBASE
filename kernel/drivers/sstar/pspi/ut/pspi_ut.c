/*
 * pspi_ut.c- Sigmastar
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
#define MAX_BUF_SIZE  (128)

static const char *spidev = "/dev/spidev0.0";
static uint32_t    spimode;
static uint32_t    status;
static uint8_t     bits = 8;
static char *      input_file;
static char *      output_file;
static uint32_t    speed  = 500000;
static int         r_flag = 0;
static int         w_flag = 0;

uint8_t default_tx[] = {0x1};

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

static void transfer(int fd, uint8_t const *tx, uint8_t const *rx, size_t len)
{
    int ret = 0;
    int i   = 0;
    // int out_fd;
    unsigned char           pspi_wr_en[3]             = {0};
    unsigned char           pspi_wr_buf[MAX_BUF_SIZE] = {0};
    unsigned char           pspi_rd_buf[MAX_BUF_SIZE] = {0};
    struct spi_ioc_transfer tfr[2]                    = {0};

    struct spi_ioc_transfer input_transfer = {
        .tx_buf        = (unsigned long)tx,
        .rx_buf        = (unsigned long)rx,
        .len           = len,
        .speed_hz      = speed,
        .bits_per_word = bits,
    };

    // input tx
    if (input_tx && (r_flag || w_flag))
    {
        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &input_transfer);
        if (ret < 1)
        {
            printf("can't input transfer fail\n");
            goto err;
        }
        return;
    }

    // write enable
    pspi_wr_buf[0] = 0x06;

    tfr[0].rx_buf = 0;
    tfr[0].tx_buf = (unsigned long)pspi_wr_buf;
    tfr[0].len    = 0x1;

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tfr);
    if (ret < 1)
    {
        printf("can't write enable111\n");
        goto err;
    }

    // unlock
    pspi_wr_buf[0] = 0x1F;
    pspi_wr_buf[1] = 0xA0;
    pspi_wr_buf[2] = 0x00;

    tfr[0].rx_buf = 0;
    tfr[0].tx_buf = (unsigned long)pspi_wr_buf;
    tfr[0].len    = 0x3;

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tfr);
    if (ret < 1)
    {
        printf("can't unlock1\n");
        goto err;
    }

    pspi_wr_buf[0] = 0x0F;
    pspi_wr_buf[1] = 0xA0;

    tfr[0].rx_buf = 0;
    tfr[0].tx_buf = (unsigned long)pspi_wr_buf;
    tfr[0].len    = 0x2;
    tfr[1].rx_buf = (unsigned long)pspi_rd_buf;
    tfr[1].tx_buf = 0;
    tfr[1].len    = 0x1;

    ret = ioctl(fd, SPI_IOC_MESSAGE(2), &tfr);
    if (ret < 1)
    {
        printf("can't unlock2\n");
        goto err;
    }

    // write enable
    pspi_wr_buf[0] = 0x06;

    tfr[0].rx_buf = 0;
    tfr[0].tx_buf = (unsigned long)pspi_wr_buf;
    tfr[0].len    = 0x1;

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tfr);
    if (ret < 1)
    {
        printf("can't write enable222\n");
        goto err;
    }
    usleep(10000);

    // erase block 0
    pspi_wr_buf[0] = 0xd8;
    pspi_wr_buf[1] = 0x00;
    pspi_wr_buf[2] = 0x00;
    pspi_wr_buf[3] = 0x00;

    tfr[0].rx_buf = 0;
    tfr[0].tx_buf = (unsigned long)pspi_wr_buf;
    tfr[0].len    = 0x4;

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tfr);
    if (ret < 1)
    {
        printf("can't erase block\n");
        goto err;
    }

    usleep(20000);

    // read page to cache
    pspi_wr_buf[0] = 0x13;
    pspi_wr_buf[1] = 0x00;
    pspi_wr_buf[2] = 0x00;
    pspi_wr_buf[3] = 0x00;

    tfr[0].rx_buf = 0;
    tfr[0].tx_buf = (unsigned long)pspi_wr_buf;
    tfr[0].len    = 0x4;

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tfr);
    if (ret < 1)
    {
        printf("can't read page to cache\n");
        goto err;
    }

    // read data from cache
    pspi_wr_buf[0] = 0x03;
    pspi_wr_buf[1] = 0x00;
    pspi_wr_buf[2] = 0x00;
    pspi_wr_buf[3] = 0x00;

    tfr[0].rx_buf = 0;
    tfr[0].tx_buf = (unsigned long)pspi_wr_buf;
    tfr[0].len    = 0x4;
    tfr[1].rx_buf = (unsigned long)pspi_rd_buf;
    tfr[1].tx_buf = 0;
    tfr[1].len    = MAX_BUF_SIZE;

    ret = ioctl(fd, SPI_IOC_MESSAGE(2), &tfr);
    if (ret < 1)
    {
        printf("can't read data from cache\n");
        goto err;
    }

    for (i = 0; i < MAX_BUF_SIZE; i++)
    {
        if (pspi_rd_buf[i] != 0xFF)
        {
            printf("erase fail\n");
            goto err;
        }
    }
    printf("--->erase block done!\n");

    // write enable
    pspi_wr_buf[0] = 0x06;
    tfr[0].rx_buf  = 0;
    tfr[0].tx_buf  = (unsigned long)pspi_wr_buf;
    tfr[0].len     = 0x1;

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tfr);
    if (ret < 1)
    {
        printf("can't write enable333\n");
        goto err;
    }

    // write data to cache
    pspi_wr_en[0] = 0x02;
    pspi_wr_en[1] = 0x00;
    pspi_wr_en[2] = 0x00;

    for (i = 0; i < MAX_BUF_SIZE; i++)
    {
        pspi_wr_buf[i] = i;
    }

    tfr[0].rx_buf = 0;
    tfr[0].tx_buf = (unsigned long)pspi_wr_en;
    tfr[0].len    = 0x3;
    tfr[1].rx_buf = 0;
    tfr[1].tx_buf = (unsigned long)pspi_wr_buf;
    tfr[1].len    = MAX_BUF_SIZE;

    ret = ioctl(fd, SPI_IOC_MESSAGE(2), &tfr);
    if (ret < 1)
    {
        printf("can't write data to cache\n");
        goto err;
    }
    printf("--->write data done!\n");

    // program execute
    pspi_wr_buf[0] = 0x10;
    pspi_wr_buf[1] = 0x00;
    pspi_wr_buf[2] = 0x00;
    pspi_wr_buf[3] = 0x00;
    // tfr[0].cs_change = 1;
    tfr[0].rx_buf = 0;
    tfr[0].tx_buf = (unsigned long)pspi_wr_buf;
    tfr[0].len    = 0x4;

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tfr);
    if (ret < 1)
    {
        printf("can't program execute\n");
        goto err;
    }

    usleep(20000);

    // read page to cache
    pspi_wr_buf[0] = 0x13;
    pspi_wr_buf[1] = 0x00;
    pspi_wr_buf[2] = 0x00;
    pspi_wr_buf[3] = 0x00;

    tfr[0].rx_buf = 0;
    tfr[0].tx_buf = (unsigned long)pspi_wr_buf;
    tfr[0].len    = 0x4;

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tfr);
    if (ret < 1)
    {
        printf("can't read page to cache\n");
        goto err;
    }

    usleep(20000);

    // read data from cache
    pspi_wr_buf[0] = 0x03;
    pspi_wr_buf[1] = 0x00;
    pspi_wr_buf[2] = 0x00;
    pspi_wr_buf[3] = 0x00;

    tfr[0].rx_buf = 0;
    tfr[0].tx_buf = (unsigned long)pspi_wr_buf;
    tfr[0].len    = 0x4;
    tfr[1].rx_buf = (unsigned long)pspi_rd_buf;
    tfr[1].tx_buf = 0;
    tfr[1].len    = MAX_BUF_SIZE;

    ret = ioctl(fd, SPI_IOC_MESSAGE(2), &tfr);
    if (ret < 1)
    {
        printf("can't read data from cache333\n");
        goto err;
    }
    printf("--->read data done!\n");

    for (i = 0; i < MAX_BUF_SIZE; i++)
    {
        if (pspi_rd_buf[i] != i)
        {
            printf("data check fail\n");
            goto err;
        }
    }

    printf("--->checkout data success\n");
    printf("--->done\n");
    return;

err:
    printf("pspi transfer err!!!\n");
    return;
}

static void sstar_usage(const char *paramet)
{
    printf("Usage: %s [-DCHOsbL3prwio]\n", paramet);
    puts(
        "  -D --device  Refers to SPI devices\n"
        "  -C --cs-high CS is highly effective\n"
        "  -H --pha     Refers to SPI clock phase\n"
        "  -O --pol     Refers to SPI clock polarity\n"
        "  -s --speed   Refers to SPI devices max speed (Hz)\n"
        "  -L --lsb     Low bit first mover\n"
        "  -b --bpw     SPI devices transferred bits per word\n"
        "  -3 --3wire   Both input and output use DI\n"
        "  -p           Send specified parameters\n"
        "  -r --read    read only and depend on -p\n"
        "  -w --write   write only and depend on -p\n"
        "  -i --input   Transfer file data\n"
        "  -o --output  Receive data and save it to a file\n");
    exit(1);
}

static void parse_opts(int argc, char *argv[])
{
    while (1)
    {
        static const struct option sstar_lopts[] = {
            {"device", 1, 0, 'D'}, {"cs-high", 0, 0, 'C'}, {"cpha", 0, 0, 'H'},   {"cpol", 0, 0, 'O'},
            {"speed", 1, 0, 's'},  {"lsb", 0, 0, 'L'},     {"bpw", 1, 0, 'b'},    {"3wire", 0, 0, '3'},
            {"read", 0, 0, 'r'},   {"write", 0, 0, 'w'},   {"output", 1, 0, 'o'}, {"input", 1, 0, 'i'},
            {NULL, 0, 0, 0},
        };
        int val;

        val = getopt_long(argc, argv, "D:Cs:HOi:b:o:L3p:rw", sstar_lopts, NULL);

        if (val == -1)
            break;

        switch (val)
        {
            case 'D':
                spidev = optarg;
                break;
            case 'C':
                spimode |= SPI_CS_HIGH;
                break;
            case 's':
                speed = atoi(optarg);
                break;
            case 'H':
                spimode |= SPI_CPHA;
                break;
            case 'O':
                spimode |= SPI_CPOL;
                break;
            case 'i':
                input_file = optarg;
                break;
            case 'b':
                bits = atoi(optarg);
                break;
            case 'o':
                output_file = optarg;
                break;
            case 'L':
                spimode |= SPI_LSB_FIRST;
                break;
            case '3':
                spimode |= SPI_3WIRE;
                break;
            case 'p':
                input_tx = optarg;
                break;
            case 'r':
                r_flag = 1;
                break;
            case 'w':
                w_flag = 1;
                break;
            default:
                sstar_usage(argv[0]);
                break;
        }
    }
}

static void transfer_escaped_string(int fd, char *str)
{
    size_t   size = strlen(str);
    uint8_t *tx;
    uint8_t *rx;
    int      i;

    tx = malloc(size);
    if (!tx)
        printf("can't allocate tx buffer");

    rx = malloc(size);
    if (!rx)
        printf("can't allocate rx buffer");

    size = unescape((char *)tx, str, size);

    if (input_tx && w_flag)
    {
        transfer(fd, tx, NULL, size);
    }
    else if (input_tx && r_flag)
    {
        transfer(fd, NULL, rx, size);
        for (i = 1; i <= size; i++)
        {
            printf("0x%x ", rx[i - 1]);
            if (i % 8 == 0)
                printf("\n");
        }
        printf("\n");
    }
    else
    {
        transfer(fd, tx, rx, size);
    }

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

    fd = open(spidev, O_RDWR);
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
    ret = ioctl(fd, SPI_IOC_WR_MODE32, &spimode);
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

    printf("spi mode: 0x%x\n", spimode);
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
