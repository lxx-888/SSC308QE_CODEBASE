/*
 * i2c_read_write.c- Sigmastar
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
#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>

#define MAX_READ_LINE (0x1000)

typedef enum
{
    I2C_FMT_A8D8,   /**< 8 bits Address, 8 bits Data */
    I2C_FMT_A16D8,  /**< 16 bits Address 8 bits Data */
    I2C_FMT_A8D16,  /**< 8 bits Address 16 bits Data */
    I2C_FMT_A16D16, /**< 16 bits Address 16 bits Data */
    I2C_FMT_END     /**< Reserved */
} ISP_I2C_FMT;

// I2C Linux device handle
int g_i2cFile;

// option flags
static int bus_flag;
static int slaveAddr_flag;
static int i2cFMT_flag;
static int read_flag;
static int write_flag;
static int dump_flag;
static int script_flag;

char *i2cBus     = NULL;
char *slaveAddr  = NULL;
char *i2cFMT     = NULL;
char *readReg    = NULL;
char *writeReg   = NULL;
char *dumpReg    = NULL;
char *scriptFile = NULL;

// global variable
int            devSlaveAddr;
ISP_I2C_FMT    fmt;
unsigned short devRegAddr;
unsigned short devRegValue;

// free memory
void freeMem()
{
    if (i2cBus != NULL)
        free(i2cBus);
    if (slaveAddr != NULL)
        free(slaveAddr);
    if (i2cFMT != NULL)
        free(i2cFMT);
    if (readReg != NULL)
        free(readReg);
    if (writeReg != NULL)
        free(writeReg);
    if (dumpReg != NULL)
        free(dumpReg);
    if (scriptFile != NULL)
        free(scriptFile);
}

// open the Linux device
void i2cOpen()
{
    char devPath[64];
    sprintf(devPath, "/dev/i2c-%s", i2cBus);
    g_i2cFile = open(devPath, O_RDWR);
    if (g_i2cFile < 0)
    {
        perror("i2cOpen");
        freeMem();
        exit(1);
    }
}

// close the Linux device
void i2cClose()
{
    close(g_i2cFile);
}

// set the I2C slave address for all subsequent I2C device transfers
void i2cSetAddress(int address)
{
    if (ioctl(g_i2cFile, I2C_SLAVE_FORCE, address) < 0)
    {
        perror("i2cSetAddress");
        freeMem();
        exit(1);
    }
}

// salveAddr : 8 bit savle address
int WriteRegisterPair(int slaveAddr, short reg, unsigned short value, ISP_I2C_FMT fmt)
{
    unsigned char data[4];

    i2cSetAddress(slaveAddr);

    memset(data, 0, sizeof(data));
    switch (fmt)
    {
        default:
        case I2C_FMT_A8D8:
            data[0] = reg & 0xff;
            data[1] = value & 0xff;
            if (write(g_i2cFile, data, 2) != 2)
            {
                perror("Write Register");
                return -1;
            }
            break;
        case I2C_FMT_A16D8:
            data[0] = (reg >> 8) & 0xff;
            data[1] = reg & 0xff;
            data[2] = value & 0xff;
            if (write(g_i2cFile, data, 3) != 3)
            {
                perror("Write Register");
                return -1;
            }
            break;
        case I2C_FMT_A8D16:
            data[0] = reg & 0xff;
            data[1] = (value >> 8) & 0xff;
            data[2] = (value)&0xff;
            if (write(g_i2cFile, data, 3) != 3)
            {
                perror("Write Register");
                return -1;
            }
            break;
        case I2C_FMT_A16D16:
            data[0] = (reg >> 8) & 0xff;
            data[1] = (reg)&0xff;
            data[2] = (value >> 8) & 0xff;
            data[3] = (value)&0xff;
            if (write(g_i2cFile, data, 4) != 4)
            {
                perror("SetRegisterPair");
            }

            break;
    }

    return 0;
}

int ReadRegisterPair(int slaveAddr, unsigned int reg, unsigned short *val, ISP_I2C_FMT fmt)
{
    unsigned char reg_addr[2];

    i2cSetAddress(slaveAddr);
    memset(reg_addr, 0, sizeof(unsigned char));

    switch (fmt)
    {
        case I2C_FMT_A8D8:
            reg_addr[0] = reg & 0xff;
            if (write(g_i2cFile, reg_addr, 1) != 1)
            {
                perror("Read RegisterPair set register");
                return -1;
            }
            if (read(g_i2cFile, val, 1) != 1)
            {
                perror("Read RegisterPair read value");
                return -1;
            }

            break;
        case I2C_FMT_A16D8:
            reg_addr[0] = (reg >> 8) & 0xff;
            reg_addr[1] = reg & 0xff;
            if (write(g_i2cFile, reg_addr, 2) != 2)
            {
                perror("Read RegisterPair set register");
                return -1;
            }
            if (read(g_i2cFile, val, 1) != 1)
            {
                perror("Read RegisterPair read value");
                return -1;
            }
            break;
        case I2C_FMT_A8D16:
            reg_addr[0] = reg & 0xff;
            if (write(g_i2cFile, reg_addr, 1) != 1)
            {
                perror("Read RegisterPair set register");
                return -1;
            }
            if (read(g_i2cFile, val, 2) != 2)
            {
                perror("Read RegisterPair read value");
                return -1;
            }

            break;
        case I2C_FMT_A16D16:
            reg_addr[0] = (reg >> 8) & 0xff;
            reg_addr[1] = reg & 0xff;
            if (write(g_i2cFile, reg_addr, 2) != 2)
            {
                perror("Read RegisterPair set register");
                return -1;
            }
            if (read(g_i2cFile, val, 2) != 2)
            {
                perror("Read RegisterPair read value");
                return -1;
            }

            break;
        default:
            break;
    }

    return 0;
}

int getRegAddr(char *Addr)
{
    if ((fmt == I2C_FMT_A8D8) || (fmt == I2C_FMT_A8D16))
    {
        if (strlen(Addr) != 4)
        {
            printf("I2C addr FMT error: %d\n", fmt);
            freeMem();
            exit(-1);
        }
    }

    if ((fmt == I2C_FMT_A16D8) || (fmt == I2C_FMT_A16D16))
    {
        if (strlen(Addr) != 6)
        {
            printf("I2C addr FMT error: %d \n", fmt);
            freeMem();
            exit(-1);
        }
    }

    return strtol(Addr, NULL, 16);
}

int getRegValue(char *value)
{
    if ((fmt == I2C_FMT_A8D8) || (fmt == I2C_FMT_A16D8))
    {
        if (strlen(value) != 4)
        {
            printf("I2C value FMT error\n");
            freeMem();
            exit(-1);
        }
    }

    if ((fmt == I2C_FMT_A8D16) || (fmt == I2C_FMT_A16D16))
    {
        if (strlen(value) != 6)
        {
            printf("I2C value FMT error\n");
            freeMem();
            exit(-1);
        }
    }

    return strtol(value, NULL, 16);
}

int getSlaveAddr(char *Addr)
{
    return strtol(Addr, NULL, 16);
}

ISP_I2C_FMT getI2CFMT(char *fmt)
{
    if (strcmp(fmt, "A8D8") == 0)
        return I2C_FMT_A8D8;
    if (strcmp(fmt, "A16D8") == 0)
        return I2C_FMT_A16D8;
    if (strcmp(fmt, "A8D16") == 0)
        return I2C_FMT_A8D16;
    if (strcmp(fmt, "A16D16") == 0)
        return I2C_FMT_A16D16;

    return I2C_FMT_END;
}

int doCMD(char *cmd, char *op, char *addr, char *value)
{
    unsigned short regVal;

    // printf("[%s] %s %s %s %s\n", __func__, cmd, op, addr, value);
    if (strncmp(cmd, "w", 1) == 0)
    {
        devRegAddr  = getRegAddr(addr);
        devRegValue = getRegValue(value);
        WriteRegisterPair(devSlaveAddr, devRegAddr, devRegValue, fmt);
        printf("write: %#x %#x \n", devRegAddr, devRegValue);
    }
    else if (strncmp(cmd, "r", 1) == 0)
    {
        devRegAddr = getRegAddr(addr);
        ReadRegisterPair(devSlaveAddr, devRegAddr, &regVal, fmt);
        printf("read: %#x \n", regVal);
    }
    else if (strncmp(cmd, "delay", 4) == 0)
    {
        printf("delay %ld\n", strtol(value, NULL, 10));
        usleep(strtol(value, NULL, 10) * 1000);
    }
    return 0;
}

int doFileParser()
{
    unsigned long fileSize;
    char *        buffer;
    char          cmd[16];
    char          i2cfmt[16];
    char          SlaveAddr[16];
    char          addr[16];
    char          value[16];
    FILE *        writeData;

    writeData = fopen(scriptFile, "r");
    if (writeData == NULL)
    {
        printf("open file fail\n");
        freeMem();
        exit(-1);
    }
    fseek(writeData, 0, SEEK_END);
    fileSize = ftell(writeData);
    rewind(writeData);
    buffer = (char *)malloc(sizeof(char) * MAX_READ_LINE);
    if (buffer == NULL)
    {
        fputs("Memory error", stderr);
        freeMem();
        exit(2);
    }

    // start parser i2c script cmd
    do
    {
        memset(buffer, 0, (sizeof(char) * MAX_READ_LINE));
        fgets(buffer, MAX_READ_LINE, writeData);
        if (buffer[0] == '\n')
            continue;
        if (strncmp(buffer, "//", 2) == 0)
        {
            continue;
        }
        else if (strncmp(buffer, "fmt", 3) == 0)
        {
            sscanf(buffer, "%s %s\n", cmd, i2cfmt);
            fmt = getI2CFMT(i2cfmt);
            continue;
        }
        else if (strncmp(buffer, "slaveAddr", 9) == 0)
        {
            sscanf(buffer, "%s %s\n", cmd, SlaveAddr);
            devSlaveAddr = getI2CFMT(SlaveAddr);
            devSlaveAddr = getSlaveAddr(SlaveAddr);
            continue;
        }
        else if (strncmp(buffer, "w", 1) == 0)
        {
            sscanf(buffer, "%s %s %s\n", cmd, addr, value);
        }
        else if (strncmp(buffer, "r", 1) == 0)
        {
            sscanf(buffer, "%s %s\n", cmd, addr);
            memset(value, 0, sizeof(value));
        }
        else
        {
            if (strncmp(buffer, "delay", 4) == 0)
            {
                sscanf(buffer, "%s %s\n", cmd, value);
            }
        }
        if (!feof(writeData))
            doCMD(cmd, i2cfmt, addr, value);
    } while (!feof(writeData));

    printf("do file: %s done, size: %#lx\n", scriptFile, fileSize);
    fclose(writeData);
    if (scriptFile != NULL)
    {
        free(scriptFile);
        scriptFile = NULL;
    }
    free(buffer);

    return 0;
}

int option(int argc, char **argv)
{
    int c;

    while (1)
    {
        static struct option long_options[] = {
            {"bus", required_argument, &bus_flag, 1},       {"slaveAddr", required_argument, &slaveAddr_flag, 1},
            {"fmt", required_argument, &i2cFMT_flag, 1},    {"read", required_argument, &read_flag, 1},
            {"write", required_argument, &write_flag, 1},   {"dump", required_argument, &dump_flag, 1},
            {"script", required_argument, &script_flag, 1}, {0, 0, 0, 0}};
        int option_index = 0;

        c = getopt_long(argc, argv, "b:a:f:r:w:d:s:", long_options, &option_index);

        if (c == -1)
            break;

        switch (c)
        {
            // debug -- option , -r long option is --register
            case 0:
                printf("option %s", long_options[option_index].name);
                if (optarg)
                    printf(" with arg %s", optarg);
                printf("\n");
                break;

            case 'b':
                i2cBus = (char *)malloc(strlen(optarg) + 1);
                strncpy(i2cBus, optarg, strlen(optarg) + 1);
                bus_flag = 1;
                break;

            case 'a':
                slaveAddr = (char *)malloc(strlen(optarg) + 1);
                strncpy(slaveAddr, optarg, strlen(optarg) + 1);
                slaveAddr_flag = 1;
                devSlaveAddr   = getSlaveAddr(slaveAddr);
                break;

            case 'f':
                i2cFMT = (char *)malloc(strlen(optarg) + 1);
                strncpy(i2cFMT, optarg, strlen(optarg) + 1);
                i2cFMT_flag = 1;
                fmt         = getI2CFMT(i2cFMT);
                break;

            case 'r':
                readReg = (char *)malloc(strlen(optarg) + 1);
                strncpy(readReg, optarg, strlen(optarg) + 1);
                read_flag  = 1;
                devRegAddr = getRegAddr(readReg);
                break;

            case 'w':
                writeReg = (char *)malloc(strlen(optarg) + 1);
                strncpy(writeReg, optarg, strlen(optarg) + 1);
                write_flag = 1;
                char *pch;
                int   writeRegIndex = 0;
                pch                 = strtok(writeReg, " ");
                while (pch != NULL)
                {
                    switch (writeRegIndex)
                    {
                        case 0:
                            devRegAddr = getRegAddr(pch);
                            break;
                        case 1:
                            devRegValue = getRegValue(pch);
                            break;
                    }
                    writeRegIndex++;
                    pch = strtok(NULL, " ");
                }
                break;

            case 'd':
                dumpReg = (char *)malloc(strlen(optarg) + 1);
                strncpy(dumpReg, optarg, strlen(optarg) + 1);
                printf("dumpReg: %s\n", dumpReg);
                dump_flag = 1;
                break;

            case 's':
                scriptFile = (char *)malloc(strlen(optarg) + 1);
                strncpy(scriptFile, optarg, strlen(optarg) + 1);
                printf("scriptFile: %s\n", scriptFile);
                script_flag = 1;
                break;

            case '?':
                /* getopt_long already printed an error message. */
                break;

            default:
                abort();
        }
    }

    /* show usage  */
    if ((argc < 2) || (bus_flag != 1) || (slaveAddr_flag != 1) || (i2cFMT_flag != 1))
    {
        if (script_flag == 1)
            return 0;
        printf("Usage:\n");
        printf("-b : 0 -> /dev/i2c-0, 1 -> /dev/i2c-1 \n");
        printf("-a : 8 bit i2c slave address \n");
        printf("-f : i2c format by A8D8, A16D8, A8D16, A16D16 \n");
        printf("-r : read_reg \n");
        printf("-w : write_reg write_val \n");
        printf("-d : dump reg addr by range \n");
        printf("-s : do i2c script file \n");
        printf("%s -b 0 -a slve_addr -f A16D16 -r reg \n", argv[0]);
        printf("%s -b 0 -a slve_addr -f A16D16 -w \"reg + value\" \n", argv[0]);
        printf("%s -b 0 -a slve_addr -f A16D16 -d \"start_reg [stop_reg]\" \n", argv[0]);
        printf("%s -b 0 -s i2c_script_file \n", argv[0]);
        freeMem();
        exit(1);
    }

    return 0;
}

/* bus: 0, 1 // i2c-0 or i2c-1
 * slave addr : slave device ID
 * r/w : read or write
 * reg : i2c reg Addr
 * val : write value
 * dump : start reg ~ stop reg
 */
int main(int argc, char **argv)
{
    unsigned short regVal;

    option(argc, argv);

    // open Linux I2C device
    i2cOpen();

    if (script_flag == 1)
    {
        read_flag  = 0;
        write_flag = 0;
        doFileParser();
    }

    // do read
    if (read_flag == 1)
    {
        ReadRegisterPair(devSlaveAddr, devRegAddr, &regVal, fmt);
        printf("Reg value: %#x \n", regVal);
    }

    // do write
    if (write_flag == 1)
    {
        WriteRegisterPair(devSlaveAddr, devRegAddr, devRegValue, fmt);
    }

    // close Linux I2C device
    i2cClose();

    // free all malloc memory
    freeMem();

    return 0;
}
