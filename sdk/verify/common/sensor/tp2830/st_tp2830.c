/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdio.h>
#include <linux/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "st_tp28xx_def.h"
#include "st_tp2830.h"


#define FILE_NAME "/dev/i2c-1"

#ifdef __cplusplus
}
#endif


#define DEBUG            0  //printf debug information on/off
#define TP28XX_EVK       0  //for EVK delay fine tune

//TP28xx audio
//both record and playback are master, 20ch,I2S mode, backend can use 16ch mode to capture.
#define I2S  0
#define DSP  1
#define AUDIO_FORMAT   I2S

#define SAMPLE_8K    0
#define SAMPLE_16K   1
#define SAMPLE_RATE  SAMPLE_8K

#define DATA_16BIT  0
#define DATA_8BIT   1
#define DATA_BIT    DATA_16BIT

#define AUDIO_CHN   20

//TP2802D EQ for short cable option
#define TP2802D_EQ_SHORT 0x0d
#define TP2802D_CGAIN_SHORT 0x74

//#define TP2802D_EQ_SHORT 0x01
//#define TP2802D_CGAIN_SHORT 0x70

#define BT1120_HEADER_8BIT   0x00 //reg0x02 bit3 0=BT1120,
#define BT656_HEADER_8BIT   0x08 //reg0x02 bit3 1=656,
#define SAV_HEADER_1MUX     BT656_HEADER_8BIT //BT656_HEADER_8BIT
#define HALF_FLAG_ENABLE    0

#define DEFAULT_FORMAT      TP2802_720P30V2 //TP2802_720P30V2  //TP2802_1080P30  //TP2802_720P30 //TP2802_QHD25
static int audio_format = AUDIO_FORMAT;
static int audio_channels = AUDIO_CHN;
static int HDC_enable = 1;
static int mode = DEFAULT_FORMAT;
static int chips = 2;
static int output[] = { DDR_4CH,
                        DDR_4CH,
                        DDR_1CH,
                        DDR_1CH
                      };

static unsigned int id[MAX_CHIPS];

#define TP2802A_I2C_ADDR    0x88
#define TP2802B_I2C_ADDR    0x8A
#define TP2802C_I2C_ADDR    0x8C
#define TP2802D_I2C_ADDR    0x8E

unsigned char tp2802_i2c_addr[] = { TP2802A_I2C_ADDR,
                                    TP2802B_I2C_ADDR,
                                    TP2802C_I2C_ADDR,
                                    TP2802D_I2C_ADDR
                                  };


#define TP2802_I2C_ADDR(chip_id)    (tp2802_i2c_addr[chip_id])


typedef struct
{
    unsigned int            count[CHANNELS_PER_CHIP];
    unsigned int            mode[CHANNELS_PER_CHIP];
    unsigned int            scan[CHANNELS_PER_CHIP];
    unsigned int            gain[CHANNELS_PER_CHIP][4];
    unsigned int            std[CHANNELS_PER_CHIP];
    unsigned int            state[CHANNELS_PER_CHIP];
    unsigned int            force[CHANNELS_PER_CHIP];
    unsigned char addr;
} tp2802wd_info;

static const unsigned char SYS_MODE[5]= {0x01,0x02,0x04,0x08,0x0f}; //{ch1,ch2,ch3,ch4,ch_all}
static const unsigned char SYS_AND[5] = {0xfe,0xfd,0xfb,0xf7,0xf0};

static const unsigned char DDR_2CH_MUX[5] = {0x01,0x02,0x40,0x80,0xc3}; //default Vin1&2@port1 Vin3&4@port2
static const unsigned char TP2826_DDR2CH_MUX[5]= {0x01,0x02,0x04,0x08,0x0f}; //{ch1,ch2,ch3,ch4,ch_all}

static const unsigned char CLK_MODE[4]= {0x01,0x10,0x01,0x10}; //for SDR_1CH/DDR_1CH output only
static const unsigned char CLK_ADDR[4]= {0xfa,0xfa,0xfb,0xfb};
static const unsigned char CLK_AND[4] = {0xf8,0x8f,0xf8,0x8f};
static const unsigned char SDR1_SEL[4]= {0x00,0x11,0x22,0x33}; //
static const unsigned char DDR1_SEL[4]= {0x40,0x51,0x62,0x73}; //
static const unsigned char TP2827C_DDR1_SEL[4]= {0x04,0x15,0x26,0x37}; //
static const unsigned char DAT_ADDR[4]= {0xf6,0xf7,0xf8,0xf9};

static tp2802wd_info watchdog_info[MAX_CHIPS];
volatile static unsigned int watchdog_state = 0;

pthread_mutex_t lock;
int i2cFd = 0;
pthread_t ptWatchdog = NULL;

#define msleep(x) usleep((x)*1000)

#define WATCHDOG_EXIT    0
#define WATCHDOG_RUNNING 1
#define WDT              1

int  TP2802_watchdog_init(void);
void TP2802_watchdog_exit(void);
static void TP2822_PTZ_mode(unsigned char, unsigned char, unsigned char);
static void TP2826_PTZ_mode(unsigned char, unsigned char, unsigned char);
static void TP2826_PTZ_new(unsigned char, unsigned char, unsigned char); //for test

unsigned char ReverseByte(unsigned char dat)
{

    static const unsigned char BitReverseTable256[] =
    {
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
    0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
    0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
    0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
    0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
    0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
    0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
    0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
    0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
    0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
    0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
    };
    return BitReverseTable256[dat];
}
/////////////////////////////////////////////////////////////////////
unsigned char PELCO_Decode(unsigned char *buf)
{
    unsigned char i, j;
    unsigned char cnt_high = 0;
    unsigned char cnt_low = 0;
    unsigned char dat = 0;
    unsigned char cnt_bit = 0;

    for(j = 0; j < 6; j++ )
    {
                for(i = 0; i <8; i++ )
                {
                    if(0x80&buf[j])
                    {
                        cnt_high++;
                        cnt_low = 0;
                    }
                    else
                    {
                        if(0 == cnt_low)
                        {
                            if(cnt_bit < 8)
                            {
                                dat <<= 1;
                                if(cnt_high > 2) dat |= 0x01;
                                cnt_high = 0;
                                cnt_bit++;
                            }
                            else break;
                        }
                        cnt_low++;
                    }
                    buf[j] <<= 1;

                }
        }
        if( (0 == cnt_low) && (cnt_high > 2) && (cnt_bit < 8))
        {
                dat <<= 1;
                dat |= 0x01;
        }
        return dat;
}
///////////////////////////////////////////////////////
unsigned int ConvertACPV1Data(unsigned char dat)
{
    unsigned int i, tmp=0;
    for(i = 0; i < 8; i++)
    {
        tmp <<= 3;

        if(0x01 & dat) tmp |= 0x06;
        else tmp |= 0x04;

        dat >>= 1;
    }
    return tmp;
}

static int i2c_write(int fd,unsigned char slave_addr, unsigned char reg_addr, unsigned char value)
{
    unsigned char outbuf[2];
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[1];

    //printf("[%s]fd %d, slave addr %x, regaddr %x value %x \n", __FUNCTION__, fd, slave_addr, reg_addr, value);

    messages[0].addr  = slave_addr>>1;
    messages[0].flags = 0;
    messages[0].len   = sizeof(outbuf);
    messages[0].buf   = outbuf;

    /* The first byte indicates which register we‘ll write */
    outbuf[0] = reg_addr & 0xff;

    /*
     * The second byte indicates the value to write.  Note that for many
     * devices, we can write multiple, sequential registers at once by
     * simply making outbuf bigger.
     */
    outbuf[1] = value &0xff;

    /* Transfer the i2c packets to the kernel and verify it worked */
    packets.msgs  = messages;
    packets.nmsgs = 1;
    if(ioctl(fd, I2C_RDWR, &packets) < 0)
    {
        perror("Unable to send data");
        return 1;
    }

    return 0;
}

static int i2c_read(int fd, unsigned char slave_addr, unsigned char reg_addr, unsigned char *value)
{
    unsigned char inbuf, outbuf[1];
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

    /*
     * In order to read a register, we first do a "dummy write" by writing
     * 0 bytes to the register we want to read from.  This is similar to
     * the packet in set_i2c_register, except it‘s 1 byte rather than 2.
     */
    //outbuf[0] = (reg_addr >> 8) & 0xff;
    outbuf[0] = reg_addr & 0xff;

    messages[0].addr  = slave_addr>>1;
    messages[0].flags = 0;
    messages[0].len   = sizeof(outbuf);
    messages[0].buf   = outbuf;

    /* The data will get returned in this structure */
    messages[1].addr  = slave_addr>>1;
    messages[1].flags = I2C_M_RD/* | I2C_M_NOSTART*/;
    messages[1].len   = sizeof(inbuf);
    messages[1].buf   = &inbuf;

    /* Send the request to the kernel and get the result back */
    packets.msgs      = messages;
    packets.nmsgs     = 2;
    if(ioctl(fd, I2C_RDWR, &packets) < 0)
    {
        perror("Unable to send data");
        return 1;
    }
    *value = inbuf;

    //printf("[%s]fd %d, slave addr %x, regaddr %x value %x \n",__FUNCTION__, fd, slave_addr, reg_addr, *value);

    return 0;
}

void tp28xx_byte_write(unsigned char chip, unsigned char reg_addr, unsigned char value)
{
    int ret;

    ret = i2c_write(i2cFd, tp2802_i2c_addr[chip], reg_addr, value);
    if(0 != ret)
    {
        printf("[%s]: i2c_write fail !!!!!!!!!!! \n",__FUNCTION__);
    }
    return ret;
}

unsigned char tp28xx_byte_read(unsigned char chip, unsigned char reg_addr)
{
    unsigned char ret_data = 0xFF;
    int ret;

    ret = i2c_read(i2cFd, tp2802_i2c_addr[chip], reg_addr, &ret_data);
    if(0 != ret)
    {
        printf("[%s]: i2c_read fail !!!!!!!!!!! \n",__FUNCTION__);
    }
    return ret_data;
}

void dump_all_reg(unsigned char chip)
{
    int i = 0,j=0;
    printf("********************IIC ADDR 0x%02x ********************\r\n", tp2802_i2c_addr[chip]);
    //printf("      0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\r\n",
    //                i, i+1, i+2, i+3, i+4, i+5, i+6, i+7, i+8, i+9, i+10, i+11, i+12, i+13, i+14, i+15);
    for(j=0;j<4;j++)
    {
        tp28xx_byte_write(chip, 0x40, j);//choose channel,total 4 channels in each chip
        printf("      0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\r\n",
                i, i+1, i+2, i+3, i+4, i+5, i+6, i+7, i+8, i+9, i+10, i+11, i+12, i+13, i+14, i+15);
        for(i = 0; i <= 0xF; i++)
        {
            printf("0x%02x: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\r\n",
                        i << 4, tp28xx_byte_read(chip, 16*i),
                        tp28xx_byte_read(chip, 16*i+1),
                        tp28xx_byte_read(chip, 16*i+2),
                        tp28xx_byte_read(chip, 16*i+3),
                        tp28xx_byte_read(chip, 16*i+4),
                        tp28xx_byte_read(chip, 16*i+5),
                        tp28xx_byte_read(chip, 16*i+6),
                        tp28xx_byte_read(chip, 16*i+7),
                        tp28xx_byte_read(chip, 16*i+8),
                        tp28xx_byte_read(chip, 16*i+9),
                        tp28xx_byte_read(chip, 16*i+10),
                        tp28xx_byte_read(chip, 16*i+11),
                        tp28xx_byte_read(chip, 16*i+12),
                        tp28xx_byte_read(chip, 16*i+13),
                        tp28xx_byte_read(chip, 16*i+14),
                        tp28xx_byte_read(chip, 16*i+15));
        }
        printf("\n");
    }
    printf("********************Dump end ********************\r\n");
}

static void tp2802_write_table(unsigned char chip,
                               unsigned char addr, unsigned char *tbl_ptr, unsigned char tbl_cnt)
{
    unsigned char i = 0;
    for(i = 0; i < tbl_cnt; i ++)
    {
        tp28xx_byte_write(chip, (addr + i), *(tbl_ptr + i));
    }
}

void TP2826_StartTX(unsigned char chip, unsigned char ch)
{
    unsigned char tmp;
    int i;

    tp28xx_byte_write(chip, 0xB6, (0x01<<(ch))); //clear flag

    tmp = tp28xx_byte_read(chip, 0x6f);
    tmp |= 0x01;
    tp28xx_byte_write(chip, 0x6f, tmp); //TX enable
    tmp &= 0xfe;
    tp28xx_byte_write(chip, 0x6f, tmp); //TX disable

    i = 100;
    while(i--)
    {
        if( (0x01<<(ch)) & tp28xx_byte_read(chip, 0xb6)) break;
        //udelay(1000);
        msleep(2);
    }

}
void TP2822_StartTX(unsigned char chip, unsigned char ch)
{
    int i;

    tp28xx_byte_write(chip, 0xB6, (0x01<<(ch))); //clear flag

    tp28xx_byte_write(chip, 0xBB, (0x01<<(ch)));

    i = 100;
    while(i--)
    {
        if( (0x01<<(ch)) & tp28xx_byte_read(chip, 0xb6)) break;
        //udelay(1000);
        msleep(2);
    }

    tp28xx_byte_write(chip, 0xBB, tp28xx_byte_read(chip,0xBB) & ~(0x11<<(ch))); //TX disable
}
void HDC_QHD_SetData(unsigned char chip, unsigned char reg, unsigned int dat)
{

    unsigned int i;
    unsigned char ret=0;
    unsigned char crc=0;
    if( dat > 0xff)
    {
        tp28xx_byte_write(chip, reg + 0 , 0x00);
        tp28xx_byte_write(chip, reg + 1 , 0x07);
        tp28xx_byte_write(chip, reg + 2 , 0xff);
        tp28xx_byte_write(chip, reg + 3 , 0xff);
        tp28xx_byte_write(chip, reg + 4 , 0xfc);
    }
    else
    {
        for(i = 0; i < 8; i++ )
        {
            ret >>= 1;
            if(0x80 & dat) {ret |= 0x80; crc +=0x80;}
            dat <<= 1;
        }

        tp28xx_byte_write(chip, reg + 0 , 0x00);
        tp28xx_byte_write(chip, reg + 1 , 0x06);
        tp28xx_byte_write(chip, reg + 2 , ret);
        tp28xx_byte_write(chip, reg + 3 , 0x7f|crc);
        tp28xx_byte_write(chip, reg + 4 , 0xfc);
    }

}
void HDC_SetData(unsigned char chip, unsigned char reg, unsigned int dat)
{

    unsigned int i;
    unsigned char ret=0;
    unsigned char crc=0;
    if( dat > 0xff)
    {
        tp28xx_byte_write(chip, reg + 0 , 0x07);
        tp28xx_byte_write(chip, reg + 1 , 0xff);
        tp28xx_byte_write(chip, reg + 2 , 0xff);
        tp28xx_byte_write(chip, reg + 3 , 0xff);
        tp28xx_byte_write(chip, reg + 4 , 0xfc);
    }
    else
    {
        for(i = 0; i < 8; i++ )
        {
            ret >>= 1;
            if(0x80 & dat) {ret |= 0x80; crc +=0x80;}
            dat <<= 1;
        }

        tp28xx_byte_write(chip, reg + 0 , 0x06);
        tp28xx_byte_write(chip, reg + 1 , ret);
        tp28xx_byte_write(chip, reg + 2 , 0x7f|crc);
        tp28xx_byte_write(chip, reg + 3 , 0xff);
        tp28xx_byte_write(chip, reg + 4 , 0xfc);
    }

}
void HDA_SetACPV2Data(unsigned char chip, unsigned char reg,unsigned char dat)
{
    unsigned int i;
    unsigned int PTZ_pelco=0;

    for(i = 0; i < 8; i++)
    {
        PTZ_pelco <<= 3;

        if(0x80 & dat) PTZ_pelco |= 0x06;
        else PTZ_pelco |= 0x04;

        dat <<= 1;
    }

    tp28xx_byte_write(chip, reg + 0 , (PTZ_pelco>>16)&0xff);
    tp28xx_byte_write(chip, reg + 1 , (PTZ_pelco>>8)&0xff);
    tp28xx_byte_write(chip, reg + 2 , (PTZ_pelco)&0xff);
}
void HDA_SetACPV1Data(unsigned char chip, unsigned char reg,unsigned char dat)
{

    unsigned int i;
    unsigned int PTZ_pelco=0;

    for(i = 0; i < 8; i++)
    {
        PTZ_pelco <<= 3;

        if(0x01 & dat) PTZ_pelco |= 0x06;
        else PTZ_pelco |= 0x04;

        dat >>= 1;
    }

    tp28xx_byte_write(chip, reg + 0 , (PTZ_pelco>>16)&0xff);
    tp28xx_byte_write(chip, reg + 1 , (PTZ_pelco>>8)&0xff);
    tp28xx_byte_write(chip, reg + 2 , (PTZ_pelco)&0xff);

}

int tp2802_open(struct inode * inode, struct file * file)
{
    return SUCCESS;
}

int tp2802_close(struct inode * inode, struct file * file)
{
    return SUCCESS;
}

static void tp2802_set_work_mode_1080p25(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_1080p25_raster, 9);
}

static void tp2802_set_work_mode_1080p30(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_1080p30_raster, 9);
}

static void tp2802_set_work_mode_720p25(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_720p25_raster, 9);
}

static void tp2802_set_work_mode_720p30(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_720p30_raster, 9);
}

static void tp2802_set_work_mode_720p50(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_720p50_raster, 9);
}

static void tp2802_set_work_mode_720p60(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_720p60_raster, 9);
}

static void tp2802_set_work_mode_PAL(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_PAL_raster, 9);
}

static void tp2802_set_work_mode_NTSC(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_NTSC_raster, 9);
}

static void tp2802_set_work_mode_3M(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_3M_raster, 9);
}

static void tp2802_set_work_mode_5M(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_5M_raster, 9);
}
static void tp2802_set_work_mode_4M(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_4M_raster, 9);
}
static void tp2802_set_work_mode_3M20(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_3M20_raster, 9);
}
static void tp2802_set_work_mode_4M12(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_4M12_raster, 9);
}
static void tp2802_set_work_mode_6M10(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_6M10_raster, 9);
}
static void tp2802_set_work_mode_QHDH30(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_QHDH30_raster, 9);
}
static void tp2802_set_work_mode_QHDH25(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_QHDH25_raster, 9);
}
static void tp2802_set_work_mode_QHD15(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_QHD15_raster, 9);
}
static void tp2802_set_work_mode_QXGAH30(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_QXGAH30_raster, 9);
}
static void tp2802_set_work_mode_QXGAH25(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_QXGAH25_raster, 9);
}
static void tp2802_set_work_mode_QHD30(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_QHD30_raster, 9);
}
static void tp2802_set_work_mode_QHD25(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_QHD25_raster, 9);
}
static void tp2802_set_work_mode_QXGA30(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_QXGA30_raster, 9);
}
static void tp2802_set_work_mode_QXGA25(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_QXGA25_raster, 9);
}
/*
static void tp2802_set_work_mode_4M30(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_4M30_raster, 9);
}
static void tp2802_set_work_mode_4M25(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_4M25_raster, 9);
}
*/
static void tp2802_set_work_mode_5M20(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_5M20_raster, 9);
}
static void tp2802_set_work_mode_5MH20(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_5MH20_raster, 9);
}
/*
static void tp2802_set_work_mode_4MH30(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_4MH30_raster, 9);
}
static void tp2802_set_work_mode_4MH25(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_4MH25_raster, 9);
}
*/
static void tp2802_set_work_mode_8M15(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_8M15_raster, 9);
}
static void tp2802_set_work_mode_8MH15(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_8MH15_raster, 9);
}
static void tp2802_set_work_mode_8M12(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_8M12_raster, 9);
}
static void tp2802_set_work_mode_8MH12(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_8MH12_raster, 9);
}
static void tp2802_set_work_mode_720p30HDR(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_720p30HDR_raster, 9);
}
static void tp2802_set_work_mode_6M20(unsigned chip)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip, 0x15, tbl_tp2802_6M20_raster, 9);
}
//#define AMEND

static int tp2833_audio_config_rmpos(unsigned chip, unsigned format, unsigned chn_num)
{
    int i = 0;

    //clear first
    for (i=0; i<20; i++)
    {
        tp28xx_byte_write(chip, i, 0);
    }

    switch(chn_num)
    {
    case 2:
        if (format)
        {
            tp28xx_byte_write(chip, 0x0, 1);
            tp28xx_byte_write(chip, 0x1, 2);
        }
        else
        {
            tp28xx_byte_write(chip, 0x0, 1);
            tp28xx_byte_write(chip, 0x8, 2);
        }

        break;
    case 4:
        if (format)
        {
            tp28xx_byte_write(chip, 0x0, 1);
            tp28xx_byte_write(chip, 0x1, 2);
            tp28xx_byte_write(chip, 0x2, 3);
            tp28xx_byte_write(chip, 0x3, 4);/**/
        }
        else
        {
            tp28xx_byte_write(chip, 0x0, 1);
            tp28xx_byte_write(chip, 0x1, 2);
            tp28xx_byte_write(chip, 0x8, 3);
            tp28xx_byte_write(chip, 0x9, 4);/**/
        }

        break;

    case 8:
        if (0 == chip%4)
        {
            if (format)
            {
                tp28xx_byte_write(chip, 0x0, 1);
                tp28xx_byte_write(chip, 0x1, 2);
                tp28xx_byte_write(chip, 0x2, 3);
                tp28xx_byte_write(chip, 0x3, 4);/**/
                tp28xx_byte_write(chip, 0x4, 5);
                tp28xx_byte_write(chip, 0x5, 6);
                tp28xx_byte_write(chip, 0x6, 7);
                tp28xx_byte_write(chip, 0x7, 8);/**/
            }
            else
            {
                tp28xx_byte_write(chip, 0x0, 1);
                tp28xx_byte_write(chip, 0x1, 2);
                tp28xx_byte_write(chip, 0x2, 3);
                tp28xx_byte_write(chip, 0x3, 4);/**/
                tp28xx_byte_write(chip, 0x8, 5);
                tp28xx_byte_write(chip, 0x9, 6);
                tp28xx_byte_write(chip, 0xa, 7);
                tp28xx_byte_write(chip, 0xb, 8);/**/
            }
        }
        else if (1 == chip%4)
        {
            if (format)
            {
                tp28xx_byte_write(chip, 0x0, 0);
                tp28xx_byte_write(chip, 0x1, 0);
                tp28xx_byte_write(chip, 0x2, 0);
                tp28xx_byte_write(chip, 0x3, 0);
                tp28xx_byte_write(chip, 0x4, 1);
                tp28xx_byte_write(chip, 0x5, 2);
                tp28xx_byte_write(chip, 0x6, 3);
                tp28xx_byte_write(chip, 0x7, 4);/**/
            }
            else
            {
                tp28xx_byte_write(chip, 0x0, 0);
                tp28xx_byte_write(chip, 0x1, 0);
                tp28xx_byte_write(chip, 0x2, 1);
                tp28xx_byte_write(chip, 0x3, 2);
                tp28xx_byte_write(chip, 0x8, 0);
                tp28xx_byte_write(chip, 0x9, 0);
                tp28xx_byte_write(chip, 0xa, 3);
                tp28xx_byte_write(chip, 0xb, 4);/**/
            }
        }


        break;

    case 16:
        if (0 == chip%4)
        {
            for (i=0; i<16; i++)
            {
                tp28xx_byte_write(chip, i, i+1);
            }
        }
        else if (1 == chip%4)
        {
            for (i=4; i<16; i++)
            {
                tp28xx_byte_write(chip, i, i+1 -4);
            }
        }
        else if (2 == chip%4)
        {
            for (i=8; i<16; i++)
            {

                tp28xx_byte_write(chip, i, i+1 - 8);

            }
        }
        else
        {
            for (i=12; i<16; i++)
            {
                tp28xx_byte_write(chip, i, i+1 - 12);

            }
        }
        break;

    case 20:
        for (i=0; i<20; i++)
        {
            tp28xx_byte_write(chip, i, i+1);
        }
        break;

    default:
        for (i=0; i<20; i++)
        {
            tp28xx_byte_write(chip, i, i+1);
        }
        break;
    }

    msleep(10);
    return 0;
}

static int tp2833_set_audio_rm_format(unsigned chip, tp2833_audio_format *pstFormat)
{
    unsigned char temp = 0;

    if (pstFormat->mode > 1)
    {
        return FAILURE;
    }

    if (pstFormat->format> 1)
    {
        return FAILURE;
    }

    if (pstFormat->bitrate> 1)
    {
        return FAILURE;
    }

    if (pstFormat->clkdir> 1)
    {
        return FAILURE;
    }

    if (pstFormat->precision > 1)
    {
        return FAILURE;
    }

    temp = tp28xx_byte_read(chip, 0x17);
    temp &= 0xf2;
    temp |= pstFormat->format;
    temp |= pstFormat->precision << 2;
    /*dsp*/
    //if (pstFormat->format)
    {
        temp |= 1 << 3;
    }
    tp28xx_byte_write(chip, 0x17, temp);

    temp = tp28xx_byte_read(chip, 0x18);
    temp &= 0xef;
    temp |= pstFormat->bitrate << 4;
    tp28xx_byte_write(chip, 0x18, temp);

    temp = tp28xx_byte_read(chip, 0x19);
    temp &= 0xdc;
    if (pstFormat->mode == 0)
    {
        /*slave mode*/
        temp |= 1 << 5;
    }
    else
    {
        /*master mode*/
        temp |= 0x3;
    }
    /*Notice:*/
    temp |= pstFormat->bitrate << 4;
    tp28xx_byte_write(chip, 0x19, temp);

    tp2833_audio_config_rmpos(chip, pstFormat->format, pstFormat->chn);

    return SUCCESS;
}

static int tp2833_set_audio_pb_format(unsigned chip, tp2833_audio_format *pstFormat)
{
    unsigned char temp = 0;

    if (pstFormat->mode > 1)
    {
        return FAILURE;
    }

    if (pstFormat->format> 1)
    {
        return FAILURE;
    }

    if (pstFormat->bitrate> 1)
    {
        return FAILURE;
    }

    if (pstFormat->clkdir> 1)
    {
        return FAILURE;
    }

    if (pstFormat->precision > 1)
    {
        return FAILURE;
    }

    temp = tp28xx_byte_read(chip, 0x1b);
    temp &= 0xb2;
    temp |= pstFormat->mode;
    temp |= pstFormat->format << 2;
    /*dsp*/
    if (pstFormat->format)
    {
        temp |= 1 << 3;
    }
    temp |= pstFormat->precision << 6;

    tp28xx_byte_write(chip, 0x1b, temp);

    return SUCCESS;
}
#if 0
long tp2802_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    unsigned int __user *argp = (unsigned int __user *)arg;
    unsigned int i, j, chip, tmp, ret = 0;
   // unsigned long flags;

    tp2802_register        dev_register;
    tp2802_image_adjust    image_adjust;
    tp2802_work_mode       work_mode;
    tp2802_video_mode      video_mode;
    tp2802_video_loss      video_loss;
    tp2802_PTZ_data        PTZ_data;
    tp2802_audio_playback  audio_playback ;
    tp2802_audio_da_volume audio_da_volume;

    switch (_IOC_NR(cmd))
    {

    case _IOC_NR(TP2802_READ_REG):
    {
        if (copy_from_user(&dev_register, argp, sizeof(tp2802_register)))
            return FAILURE;

        pthread_mutex_lock(&lock);

        chip = dev_register.chip;

        tp2802_set_reg_page(chip, dev_register.ch);

        dev_register.value = tp28xx_byte_read(chip, dev_register.reg_addr);

        pthread_mutex_unlock(&lock);

        if (copy_to_user(argp, &dev_register, sizeof(tp2802_register)))
            return FAILURE;

        break;
    }

    case _IOC_NR(TP2802_WRITE_REG):
    {
        if (copy_from_user(&dev_register, argp, sizeof(tp2802_register)))
            return FAILURE;

        pthread_mutex_lock(&lock);

        chip = dev_register.chip;

        tp2802_set_reg_page(chip, dev_register.ch);

        tp28xx_byte_write(chip, dev_register.reg_addr, dev_register.value);

        pthread_mutex_unlock(&lock);
        break;
    }

    case _IOC_NR(TP2802_SET_VIDEO_MODE):
    {
        if (copy_from_user(&video_mode, argp, sizeof(tp2802_video_mode)))
            return FAILURE;

        if(video_mode.ch >= CHANNELS_PER_CHIP)  return FAILURE;

        pthread_mutex_lock(&lock);

        ret = tp2802_set_video_mode(video_mode.chip, video_mode.mode, video_mode.ch, video_mode.std);

        pthread_mutex_unlock(&lock);

        if (!(ret))
        {

            watchdog_info[video_mode.chip].mode[video_mode.ch] = video_mode.mode;
            watchdog_info[video_mode.chip].std[video_mode.ch] = video_mode.std;
            return SUCCESS;
        }
        else
        {
            printf("Invalid mode:%d\n", video_mode.mode);
            return FAILURE;
        }

        break;
    }

    case _IOC_NR(TP2802_GET_VIDEO_MODE):
    {
        if (copy_from_user(&video_mode, argp, sizeof(tp2802_video_mode)))
            return FAILURE;

        if(video_mode.ch >= CHANNELS_PER_CHIP)  return FAILURE;
#if (WDT)
        video_mode.mode = watchdog_info[video_mode.chip].mode[video_mode.ch];
        video_mode.std = watchdog_info[video_mode.chip].std[video_mode.ch];
#else
        pthread_mutex_lock(&lock);

        chip = video_mode.chip;

        tp2802_set_reg_page(chip, video_mode.ch);

        tmp = tp28xx_byte_read(chip, 0x03);
        tmp &= 0x7; /* [2:0] - CVSTD */
        video_mode.mode = tmp;

        pthread_mutex_unlock(&lock);
#endif
        if (copy_to_user(argp, &video_mode, sizeof(tp2802_video_mode)))
            return FAILURE;
        break;
    }

    case _IOC_NR(TP2802_GET_VIDEO_LOSS):/* get video loss state */
    {
        if (copy_from_user(&video_loss, argp, sizeof(tp2802_video_loss)))
            return FAILURE;

        if(video_loss.ch >= CHANNELS_PER_CHIP)  return FAILURE;
#if (WDT)
        video_loss.is_lost = ( VIDEO_LOCKED == watchdog_info[video_loss.chip].state[video_loss.ch] ) ? 0:1;
        if(video_loss.is_lost) video_loss.is_lost = ( VIDEO_UNLOCK == watchdog_info[video_loss.chip].state[video_loss.ch] ) ? 0:1;
#else
        pthread_mutex_lock(&lock);

        chip = video_loss.chip;

        tp2802_set_reg_page(chip, video_loss.ch);

        tmp = tp28xx_byte_read(chip, 0x01);
        tmp = (tmp & 0x80) >> 7;
        if(!tmp)
        {
            if(0x08 == tp28xx_byte_read(chip, 0x2f))
            {
                tmp = tp28xx_byte_read(chip, 0x04);
                if(tmp < 0x30) tmp = 0;
                else tmp = 1;
            }

        }
        video_loss.is_lost = tmp;   /* [7] - VDLOSS */

        pthread_mutex_unlock(&lock);
#endif
        if (copy_to_user(argp, &video_loss, sizeof(tp2802_video_loss))) // Changed from "video_loss" to "tp2802_video_loss" - 10/17/2018
            return FAILURE;

        break;
    }

    case _IOC_NR(TP2802_SET_IMAGE_ADJUST):
    {
        if (copy_from_user(&image_adjust, argp, sizeof(tp2802_image_adjust)))
        {
            return FAILURE;
        }

        if(image_adjust.ch >= CHANNELS_PER_CHIP)  return FAILURE;

        pthread_mutex_lock(&lock);

        chip = image_adjust.chip;

        tp2802_set_reg_page(chip, image_adjust.ch);

        // Set Brightness
        tp28xx_byte_write(chip, BRIGHTNESS, image_adjust.brightness);

        // Set Contrast
        tp28xx_byte_write(chip, CONTRAST, image_adjust.contrast);

        // Set Saturation
        tp28xx_byte_write(chip, SATURATION, image_adjust.saturation);

        // Set Hue
        tp28xx_byte_write(chip, HUE, image_adjust.hue);

        // Set Sharpness
        tmp = tp28xx_byte_read(chip, SHARPNESS);
        tmp &= 0xe0;
        tmp |= (image_adjust.sharpness & 0x1F);
        tp28xx_byte_write(chip, SHARPNESS, tmp);

        pthread_mutex_unlock(&lock);
        break;
    }

    case _IOC_NR(TP2802_GET_IMAGE_ADJUST):
    {
        if (copy_from_user(&image_adjust, argp, sizeof(tp2802_image_adjust)))
            return FAILURE;

        if(image_adjust.ch >= CHANNELS_PER_CHIP)  return FAILURE;

        pthread_mutex_lock(&lock);

        chip = image_adjust.chip;

        tp2802_set_reg_page(chip, image_adjust.ch);

        // Get Brightness
        image_adjust.brightness = tp28xx_byte_read(chip, BRIGHTNESS);

        // Get Contrast
        image_adjust.contrast = tp28xx_byte_read(chip, CONTRAST);

        // Get Saturation
        image_adjust.saturation = tp28xx_byte_read(chip, SATURATION);

        // Get Hue
        image_adjust.hue = tp28xx_byte_read(chip, HUE);

        // Get Sharpness
        image_adjust.sharpness = 0x1F & tp28xx_byte_read(chip, SHARPNESS);

        pthread_mutex_unlock(&lock);

        if (copy_to_user(argp, &image_adjust, sizeof(tp2802_image_adjust)))
            return FAILURE;

        break;
    }
    case _IOC_NR(TP2802_SET_PTZ_DATA):
    case _IOC_NR(TP2802_SET_PTZ_NEW):
    {
        if (copy_from_user(&PTZ_data, argp, sizeof(tp2802_PTZ_data)))
        {
            return FAILURE;
        }

        if(PTZ_data.ch >= CHANNELS_PER_CHIP)  return FAILURE;

        pthread_mutex_lock(&lock);

        chip = PTZ_data.chip;

        if( TP2830 == id[chip] || TP2831 == id[chip]) // Added TP2830/31 ID - 10/15/2018
        {
            TP2831_PTZ_mode(chip, PTZ_data.ch, PTZ_data.mode );

            for(i = 0; i < 24; i++)
            {
                tp28xx_byte_write(chip, 0x55+i, 0x00);
            }

            if(PTZ_HDC == PTZ_data.mode || PTZ_HDC_QHD == PTZ_data.mode || PTZ_HDC_8M12 == PTZ_data.mode || PTZ_HDC_8M15 == PTZ_data.mode || PTZ_HDC_6M20 == PTZ_data.mode) //HDC
            {

                           for(i = 0; i < 7; i++)
                            {
                                tp28xx_byte_write(chip, 0x6e, PTZ_data.data[i]);
                            }

                            TP2826_StartTX(chip, PTZ_data.ch);

            }
            else if(PTZ_HDA_4M25 == PTZ_data.mode || PTZ_HDA_4M15 == PTZ_data.mode) //HDA QHD
            {
                           for(i = 0; i < 8; i++)
                            {
                                tp28xx_byte_write(chip, 0x6e, 0x00);
                            }
                            TP2826_StartTX(chip, PTZ_data.ch);

                            for(i = 0; i < 8; i++)
                            {
                                tp28xx_byte_write(chip, 0x6e, ReverseByte(PTZ_data.data[i]));
                            }
                            TP2826_StartTX(chip, PTZ_data.ch);

            }
            else if(PTZ_HDA_1080P == PTZ_data.mode || PTZ_HDA_3M18 == PTZ_data.mode || PTZ_HDA_3M25 == PTZ_data.mode) //HDA 1080p
            {
                                HDA_SetACPV2Data(chip, 0x58, 0x00);
                                HDA_SetACPV2Data(chip, 0x5e, 0x00);
                                HDA_SetACPV2Data(chip, 0x64, 0x00);
                                HDA_SetACPV2Data(chip, 0x6a, 0x00);
                                TP2826_StartTX(chip, PTZ_data.ch);
                                HDA_SetACPV2Data(chip, 0x58, PTZ_data.data[0]);
                                HDA_SetACPV2Data(chip, 0x5e, PTZ_data.data[1]);
                                HDA_SetACPV2Data(chip, 0x64, PTZ_data.data[2]);
                                HDA_SetACPV2Data(chip, 0x6a, PTZ_data.data[3]);
                                TP2826_StartTX(chip, PTZ_data.ch);

            }
            else if( PTZ_HDA_720P == PTZ_data.mode ) //HDA 720p
            {
                                HDA_SetACPV1Data(chip, 0x55, 0x00);
                                HDA_SetACPV1Data(chip, 0x58, 0x00);
                                HDA_SetACPV1Data(chip, 0x5b, 0x00);
                                HDA_SetACPV1Data(chip, 0x5e, 0x00);
                                TP2826_StartTX(chip, PTZ_data.ch);
                                HDA_SetACPV1Data(chip, 0x55, PTZ_data.data[0]);
                                HDA_SetACPV1Data(chip, 0x58, PTZ_data.data[1]);
                                HDA_SetACPV1Data(chip, 0x5b, PTZ_data.data[2]);
                                HDA_SetACPV1Data(chip, 0x5e, PTZ_data.data[3]);
                                TP2826_StartTX(chip, PTZ_data.ch);
            }
            else if( PTZ_HDA_CVBS == PTZ_data.mode) //HDA 960H
            {
                                HDA_SetACPV1Data(chip, 0x55, 0x00);
                                HDA_SetACPV1Data(chip, 0x58, 0x00);
                                HDA_SetACPV1Data(chip, 0x5b, 0x00);
                                HDA_SetACPV1Data(chip, 0x5e, 0x00);
                                TP2826_StartTX(chip, PTZ_data.ch);
                                HDA_SetACPV1Data(chip, 0x55, PTZ_data.data[0]);
                                HDA_SetACPV1Data(chip, 0x58, PTZ_data.data[1]);
                                HDA_SetACPV1Data(chip, 0x5b, PTZ_data.data[2]);
                                HDA_SetACPV1Data(chip, 0x5e, PTZ_data.data[3]);
                                TP2826_StartTX(chip, PTZ_data.ch);
            }
            else //TVI
            {
                            //line1
                                tp28xx_byte_write(chip, 0x56 , 0x02);
                                tp28xx_byte_write(chip, 0x57 , PTZ_data.data[0]);
                                tp28xx_byte_write(chip, 0x58 , PTZ_data.data[1]);
                                tp28xx_byte_write(chip, 0x59 , PTZ_data.data[2]);
                                tp28xx_byte_write(chip, 0x5A , PTZ_data.data[3]);
                            //line2
                                tp28xx_byte_write(chip, 0x5C , 0x02);
                                tp28xx_byte_write(chip, 0x5D , PTZ_data.data[4]);
                                tp28xx_byte_write(chip, 0x5E , PTZ_data.data[5]);
                                tp28xx_byte_write(chip, 0x5F , PTZ_data.data[6]);
                                tp28xx_byte_write(chip, 0x60 , PTZ_data.data[7]);
                            //line3
                                tp28xx_byte_write(chip, 0x62 , 0x02);
                                tp28xx_byte_write(chip, 0x63 , PTZ_data.data[0]);
                                tp28xx_byte_write(chip, 0x64 , PTZ_data.data[1]);
                                tp28xx_byte_write(chip, 0x65 , PTZ_data.data[2]);
                                tp28xx_byte_write(chip, 0x66 , PTZ_data.data[3]);
                            //line4
                                tp28xx_byte_write(chip, 0x68 , 0x02);
                                tp28xx_byte_write(chip, 0x69 , PTZ_data.data[4]);
                                tp28xx_byte_write(chip, 0x6A , PTZ_data.data[5]);
                                tp28xx_byte_write(chip, 0x6B , PTZ_data.data[6]);
                                tp28xx_byte_write(chip, 0x6C , PTZ_data.data[7]);
                                TP2826_StartTX(chip, PTZ_data.ch);
            }


        }
        else if( TP2828 == id[chip] || TP2829 == id[chip])
        {
            TP2829_PTZ_mode(chip, PTZ_data.ch, PTZ_data.mode );

            for(i = 0; i < 24; i++)
            {
                tp28xx_byte_write(chip, 0x55+i, 0x00);
            }

            if(PTZ_HDC == PTZ_data.mode || PTZ_HDC_QHD == PTZ_data.mode || PTZ_HDC_8M12 == PTZ_data.mode || PTZ_HDC_8M15 == PTZ_data.mode) //HDC
            {

                           for(i = 0; i < 7; i++)
                            {
                                tp28xx_byte_write(chip, 0x6e, PTZ_data.data[i]);
                            }

                            TP2826_StartTX(chip, PTZ_data.ch);

            }
            else if(PTZ_HDA_4M25 == PTZ_data.mode || PTZ_HDA_4M15 == PTZ_data.mode) //HDA QHD
            {
                           for(i = 0; i < 8; i++)
                            {
                                tp28xx_byte_write(chip, 0x6e, 0x00);
                            }
                            TP2826_StartTX(chip, PTZ_data.ch);

                            for(i = 0; i < 8; i++)
                            {
                                tp28xx_byte_write(chip, 0x6e, ReverseByte(PTZ_data.data[i]));
                            }
                            TP2826_StartTX(chip, PTZ_data.ch);

            }
            else if(PTZ_HDA_1080P == PTZ_data.mode || PTZ_HDA_3M18 == PTZ_data.mode || PTZ_HDA_3M25 == PTZ_data.mode) //HDA 1080p
            {
                                HDA_SetACPV2Data(chip, 0x58, 0x00);
                                HDA_SetACPV2Data(chip, 0x5e, 0x00);
                                HDA_SetACPV2Data(chip, 0x64, 0x00);
                                HDA_SetACPV2Data(chip, 0x6a, 0x00);
                                TP2826_StartTX(chip, PTZ_data.ch);
                                HDA_SetACPV2Data(chip, 0x58, PTZ_data.data[0]);
                                HDA_SetACPV2Data(chip, 0x5e, PTZ_data.data[1]);
                                HDA_SetACPV2Data(chip, 0x64, PTZ_data.data[2]);
                                HDA_SetACPV2Data(chip, 0x6a, PTZ_data.data[3]);
                                TP2826_StartTX(chip, PTZ_data.ch);

            }
            else if( PTZ_HDA_720P == PTZ_data.mode ) //HDA 720p
            {
                                HDA_SetACPV1Data(chip, 0x55, 0x00);
                                HDA_SetACPV1Data(chip, 0x58, 0x00);
                                HDA_SetACPV1Data(chip, 0x5b, 0x00);
                                HDA_SetACPV1Data(chip, 0x5e, 0x00);
                                TP2826_StartTX(chip, PTZ_data.ch);
                                HDA_SetACPV1Data(chip, 0x55, PTZ_data.data[0]);
                                HDA_SetACPV1Data(chip, 0x58, PTZ_data.data[1]);
                                HDA_SetACPV1Data(chip, 0x5b, PTZ_data.data[2]);
                                HDA_SetACPV1Data(chip, 0x5e, PTZ_data.data[3]);
                                TP2826_StartTX(chip, PTZ_data.ch);
            }
            else if( PTZ_HDA_CVBS == PTZ_data.mode) //HDA 960H
            {
                                HDA_SetACPV1Data(chip, 0x55, 0x00);
                                HDA_SetACPV1Data(chip, 0x58, 0x00);
                                HDA_SetACPV1Data(chip, 0x5b, 0x00);
                                HDA_SetACPV1Data(chip, 0x5e, 0x00);
                                TP2826_StartTX(chip, PTZ_data.ch);
                                HDA_SetACPV1Data(chip, 0x55, PTZ_data.data[0]);
                                HDA_SetACPV1Data(chip, 0x58, PTZ_data.data[1]);
                                HDA_SetACPV1Data(chip, 0x5b, PTZ_data.data[2]);
                                HDA_SetACPV1Data(chip, 0x5e, PTZ_data.data[3]);
                                TP2826_StartTX(chip, PTZ_data.ch);
            }
            else //TVI
            {
                            //line1
                                tp28xx_byte_write(chip, 0x56 , 0x02);
                                tp28xx_byte_write(chip, 0x57 , PTZ_data.data[0]);
                                tp28xx_byte_write(chip, 0x58 , PTZ_data.data[1]);
                                tp28xx_byte_write(chip, 0x59 , PTZ_data.data[2]);
                                tp28xx_byte_write(chip, 0x5A , PTZ_data.data[3]);
                            //line2
                                tp28xx_byte_write(chip, 0x5C , 0x02);
                                tp28xx_byte_write(chip, 0x5D , PTZ_data.data[4]);
                                tp28xx_byte_write(chip, 0x5E , PTZ_data.data[5]);
                                tp28xx_byte_write(chip, 0x5F , PTZ_data.data[6]);
                                tp28xx_byte_write(chip, 0x60 , PTZ_data.data[7]);
                            //line3
                                tp28xx_byte_write(chip, 0x62 , 0x02);
                                tp28xx_byte_write(chip, 0x63 , PTZ_data.data[0]);
                                tp28xx_byte_write(chip, 0x64 , PTZ_data.data[1]);
                                tp28xx_byte_write(chip, 0x65 , PTZ_data.data[2]);
                                tp28xx_byte_write(chip, 0x66 , PTZ_data.data[3]);
                            //line4
                                tp28xx_byte_write(chip, 0x68 , 0x02);
                                tp28xx_byte_write(chip, 0x69 , PTZ_data.data[4]);
                                tp28xx_byte_write(chip, 0x6A , PTZ_data.data[5]);
                                tp28xx_byte_write(chip, 0x6B , PTZ_data.data[6]);
                                tp28xx_byte_write(chip, 0x6C , PTZ_data.data[7]);
                                TP2826_StartTX(chip, PTZ_data.ch);
            }


        }
        else if( TP2827C == id[chip] || TP2826C == id[chip])
        {
            TP2827C_PTZ_mode(chip, PTZ_data.ch, PTZ_data.mode );

            for(i = 0; i < 24; i++)
            {
                tp28xx_byte_write(chip, 0x55+i, 0x00);
            }

            if(PTZ_HDC == PTZ_data.mode || PTZ_HDC_8M12 == PTZ_data.mode) //HDC 1080p
            {

                                HDC_SetData(chip, 0x56, PTZ_data.data[0]);
                                HDC_SetData(chip, 0x5c, PTZ_data.data[1]);
                                HDC_SetData(chip, 0x62, PTZ_data.data[2]);
                                HDC_SetData(chip, 0x68, PTZ_data.data[3]);
                                TP2826_StartTX(chip, PTZ_data.ch);
                                HDC_SetData(chip, 0x56, PTZ_data.data[4]);
                                HDC_SetData(chip, 0x5c, PTZ_data.data[5]);
                                HDC_SetData(chip, 0x62, PTZ_data.data[6]);
                                HDC_SetData(chip, 0x68, 0xffff);
                                TP2826_StartTX(chip, PTZ_data.ch);

            }
            else if(PTZ_HDC_FIFO == PTZ_data.mode ) //HDC 1080p FIFO mode
            {

                           for(i = 0; i < 7; i++)
                            {
                                tp28xx_byte_write(chip, 0x6e, PTZ_data.data[i]);
                            }

                            TP2826_StartTX(chip, PTZ_data.ch);

            }
            else if(PTZ_HDC_QHD == PTZ_data.mode || PTZ_HDC_8M15 == PTZ_data.mode ) //HDC QHD
            {

                                HDC_QHD_SetData(chip, 0x56, PTZ_data.data[0]);
                                HDC_QHD_SetData(chip, 0x5c, PTZ_data.data[1]);
                                HDC_QHD_SetData(chip, 0x62, PTZ_data.data[2]);
                                HDC_QHD_SetData(chip, 0x68, PTZ_data.data[3]);
                                TP2826_StartTX(chip, PTZ_data.ch);
                                HDC_QHD_SetData(chip, 0x56, PTZ_data.data[4]);
                                HDC_QHD_SetData(chip, 0x5c, PTZ_data.data[5]);
                                HDC_QHD_SetData(chip, 0x62, PTZ_data.data[6]);
                                HDC_QHD_SetData(chip, 0x68, 0xffff);
                                TP2826_StartTX(chip, PTZ_data.ch);

            }
            else if(PTZ_HDA_4M25 == PTZ_data.mode || PTZ_HDA_4M15 == PTZ_data.mode) //HDA QHD
            {
                           for(i = 0; i < 8; i++)
                            {
                                tp28xx_byte_write(chip, 0x6e, 0x00);
                            }
                            TP2826_StartTX(chip, PTZ_data.ch);

                            for(i = 0; i < 8; i++)
                            {
                                tp28xx_byte_write(chip, 0x6e, ReverseByte(PTZ_data.data[i]));
                            }
                            TP2826_StartTX(chip, PTZ_data.ch);

            }
            else if(PTZ_HDA_1080P == PTZ_data.mode || PTZ_HDA_3M18 == PTZ_data.mode || PTZ_HDA_3M25 == PTZ_data.mode) //HDA 1080p
            {
                                HDA_SetACPV2Data(chip, 0x58, 0x00);
                                HDA_SetACPV2Data(chip, 0x5e, 0x00);
                                HDA_SetACPV2Data(chip, 0x64, 0x00);
                                HDA_SetACPV2Data(chip, 0x6a, 0x00);
                                TP2826_StartTX(chip, PTZ_data.ch);
                                HDA_SetACPV2Data(chip, 0x58, PTZ_data.data[0]);
                                HDA_SetACPV2Data(chip, 0x5e, PTZ_data.data[1]);
                                HDA_SetACPV2Data(chip, 0x64, PTZ_data.data[2]);
                                HDA_SetACPV2Data(chip, 0x6a, PTZ_data.data[3]);
                                TP2826_StartTX(chip, PTZ_data.ch);

            }
            else if( PTZ_HDA_720P == PTZ_data.mode ) //HDA 720p
            {
                                HDA_SetACPV1Data(chip, 0x55, 0x00);
                                HDA_SetACPV1Data(chip, 0x58, 0x00);
                                HDA_SetACPV1Data(chip, 0x5b, 0x00);
                                HDA_SetACPV1Data(chip, 0x5e, 0x00);
                                TP2826_StartTX(chip, PTZ_data.ch);
                                HDA_SetACPV1Data(chip, 0x55, PTZ_data.data[0]);
                                HDA_SetACPV1Data(chip, 0x58, PTZ_data.data[1]);
                                HDA_SetACPV1Data(chip, 0x5b, PTZ_data.data[2]);
                                HDA_SetACPV1Data(chip, 0x5e, PTZ_data.data[3]);
                                TP2826_StartTX(chip, PTZ_data.ch);
            }
            else if( PTZ_HDA_CVBS == PTZ_data.mode) //HDA 960H
            {
                                HDA_SetACPV1Data(chip, 0x55, 0x00);
                                HDA_SetACPV1Data(chip, 0x58, 0x00);
                                HDA_SetACPV1Data(chip, 0x5b, 0x00);
                                HDA_SetACPV1Data(chip, 0x5e, 0x00);
                                //TP2826_StartTX(chip, PTZ_data.ch);
                                TP2826_StartTX(chip, PTZ_data.ch);
                                HDA_SetACPV1Data(chip, 0x55, PTZ_data.data[0]);
                                HDA_SetACPV1Data(chip, 0x58, PTZ_data.data[1]);
                                HDA_SetACPV1Data(chip, 0x5b, PTZ_data.data[2]);
                                HDA_SetACPV1Data(chip, 0x5e, PTZ_data.data[3]);
                                //TP2826_StartTX(chip, PTZ_data.ch);
                                TP2826_StartTX(chip, PTZ_data.ch);
            }
            else //TVI
            {
                            //line1
                                tp28xx_byte_write(chip, 0x56 , 0x02);
                                tp28xx_byte_write(chip, 0x57 , PTZ_data.data[0]);
                                tp28xx_byte_write(chip, 0x58 , PTZ_data.data[1]);
                                tp28xx_byte_write(chip, 0x59 , PTZ_data.data[2]);
                                tp28xx_byte_write(chip, 0x5A , PTZ_data.data[3]);
                            //line2
                                tp28xx_byte_write(chip, 0x5C , 0x02);
                                tp28xx_byte_write(chip, 0x5D , PTZ_data.data[4]);
                                tp28xx_byte_write(chip, 0x5E , PTZ_data.data[5]);
                                tp28xx_byte_write(chip, 0x5F , PTZ_data.data[6]);
                                tp28xx_byte_write(chip, 0x60 , PTZ_data.data[7]);

                            //line3
                                tp28xx_byte_write(chip, 0x62 , 0x02);
                                tp28xx_byte_write(chip, 0x63 , PTZ_data.data[0]);
                                tp28xx_byte_write(chip, 0x64 , PTZ_data.data[1]);
                                tp28xx_byte_write(chip, 0x65 , PTZ_data.data[2]);
                                tp28xx_byte_write(chip, 0x66 , PTZ_data.data[3]);
                            //line4
                                tp28xx_byte_write(chip, 0x68 , 0x02);
                                tp28xx_byte_write(chip, 0x69 , PTZ_data.data[4]);
                                tp28xx_byte_write(chip, 0x6A , PTZ_data.data[5]);
                                tp28xx_byte_write(chip, 0x6B , PTZ_data.data[6]);
                                tp28xx_byte_write(chip, 0x6C , PTZ_data.data[7]);

                                TP2826_StartTX(chip, PTZ_data.ch);
            }


        }
        else if( TP2827 == id[chip] || TP2826 == id[chip] || TP2816 == id[chip])
        {
            TP2826_PTZ_mode(chip, PTZ_data.ch, PTZ_data.mode );

            for(i = 0; i < 24; i++)
            {
                tp28xx_byte_write(chip, 0x55+i, 0x00);
            }
            if(PTZ_HDC == PTZ_data.mode || PTZ_HDC_8M12 == PTZ_data.mode) //HDC 1080p
            {

                                HDC_SetData(chip, 0x56, PTZ_data.data[0]);
                                HDC_SetData(chip, 0x5c, PTZ_data.data[1]);
                                HDC_SetData(chip, 0x62, PTZ_data.data[2]);
                                HDC_SetData(chip, 0x68, PTZ_data.data[3]);
                                TP2826_StartTX(chip, PTZ_data.ch);
                                HDC_SetData(chip, 0x56, PTZ_data.data[4]);
                                HDC_SetData(chip, 0x5c, PTZ_data.data[5]);
                                HDC_SetData(chip, 0x62, PTZ_data.data[6]);
                                HDC_SetData(chip, 0x68, 0xffff);
                                TP2826_StartTX(chip, PTZ_data.ch);

            }
            else if(PTZ_HDC_QHD == PTZ_data.mode || PTZ_HDC_8M15 == PTZ_data.mode ) //HDC QHD
            {
                                HDC_QHD_SetData(chip, 0x56, PTZ_data.data[0]);
                                HDC_QHD_SetData(chip, 0x5c, PTZ_data.data[1]);
                                HDC_QHD_SetData(chip, 0x62, PTZ_data.data[2]);
                                HDC_QHD_SetData(chip, 0x68, PTZ_data.data[3]);
                                TP2826_StartTX(chip, PTZ_data.ch);
                                HDC_QHD_SetData(chip, 0x56, PTZ_data.data[4]);
                                HDC_QHD_SetData(chip, 0x5c, PTZ_data.data[5]);
                                HDC_QHD_SetData(chip, 0x62, PTZ_data.data[6]);
                                HDC_QHD_SetData(chip, 0x68, 0xffff);
                                TP2826_StartTX(chip, PTZ_data.ch);
            }
            else if(PTZ_HDA_1080P == PTZ_data.mode || PTZ_HDA_3M18 == PTZ_data.mode || PTZ_HDA_3M25 == PTZ_data.mode || PTZ_HDA_4M25 == PTZ_data.mode || PTZ_HDA_4M15 == PTZ_data.mode) //HDA 1080p
            {
                                HDA_SetACPV2Data(chip, 0x58, 0x00);
                                HDA_SetACPV2Data(chip, 0x5e, 0x00);
                                HDA_SetACPV2Data(chip, 0x64, 0x00);
                                HDA_SetACPV2Data(chip, 0x6a, 0x00);
                                TP2826_StartTX(chip, PTZ_data.ch);
                                HDA_SetACPV2Data(chip, 0x58, PTZ_data.data[0]);
                                HDA_SetACPV2Data(chip, 0x5e, PTZ_data.data[1]);
                                HDA_SetACPV2Data(chip, 0x64, PTZ_data.data[2]);
                                HDA_SetACPV2Data(chip, 0x6a, PTZ_data.data[3]);
                                TP2826_StartTX(chip, PTZ_data.ch);
            }
            else if( PTZ_HDA_720P == PTZ_data.mode ) //HDA 720p
            {
                                HDA_SetACPV1Data(chip, 0x55, 0x00);
                                HDA_SetACPV1Data(chip, 0x58, 0x00);
                                HDA_SetACPV1Data(chip, 0x5b, 0x00);
                                HDA_SetACPV1Data(chip, 0x5e, 0x00);
                                TP2826_StartTX(chip, PTZ_data.ch);
                                HDA_SetACPV1Data(chip, 0x55, PTZ_data.data[0]);
                                HDA_SetACPV1Data(chip, 0x58, PTZ_data.data[1]);
                                HDA_SetACPV1Data(chip, 0x5b, PTZ_data.data[2]);
                                HDA_SetACPV1Data(chip, 0x5e, PTZ_data.data[3]);
                                TP2826_StartTX(chip, PTZ_data.ch);
            }
            else if( PTZ_HDA_CVBS == PTZ_data.mode) //HDA 960H
            {
                                HDA_SetACPV1Data(chip, 0x55, 0x00);
                                HDA_SetACPV1Data(chip, 0x58, 0x00);
                                HDA_SetACPV1Data(chip, 0x5b, 0x00);
                                HDA_SetACPV1Data(chip, 0x5e, 0x00);
                                TP2826_StartTX(chip, PTZ_data.ch);
                                //TP2826_StartTX(chip, PTZ_data.ch);
                                HDA_SetACPV1Data(chip, 0x55, PTZ_data.data[0]);
                                HDA_SetACPV1Data(chip, 0x58, PTZ_data.data[1]);
                                HDA_SetACPV1Data(chip, 0x5b, PTZ_data.data[2]);
                                HDA_SetACPV1Data(chip, 0x5e, PTZ_data.data[3]);
                                TP2826_StartTX(chip, PTZ_data.ch);
                                //TP2826_StartTX(chip, PTZ_data.ch);
            }
            else //TVI
            {
                            //line1
                                tp28xx_byte_write(chip, 0x56 , 0x02);
                                tp28xx_byte_write(chip, 0x57 , PTZ_data.data[0]);
                                tp28xx_byte_write(chip, 0x58 , PTZ_data.data[1]);
                                tp28xx_byte_write(chip, 0x59 , PTZ_data.data[2]);
                                tp28xx_byte_write(chip, 0x5A , PTZ_data.data[3]);
                            //line2
                                tp28xx_byte_write(chip, 0x5C , 0x02);
                                tp28xx_byte_write(chip, 0x5D , PTZ_data.data[4]);
                                tp28xx_byte_write(chip, 0x5E , PTZ_data.data[5]);
                                tp28xx_byte_write(chip, 0x5F , PTZ_data.data[6]);
                                tp28xx_byte_write(chip, 0x60 , PTZ_data.data[7]);
                            //line3
                                tp28xx_byte_write(chip, 0x62 , 0x02);
                                tp28xx_byte_write(chip, 0x63 , PTZ_data.data[0]);
                                tp28xx_byte_write(chip, 0x64 , PTZ_data.data[1]);
                                tp28xx_byte_write(chip, 0x65 , PTZ_data.data[2]);
                                tp28xx_byte_write(chip, 0x66 , PTZ_data.data[3]);
                            //line4
                                tp28xx_byte_write(chip, 0x68 , 0x02);
                                tp28xx_byte_write(chip, 0x69 , PTZ_data.data[4]);
                                tp28xx_byte_write(chip, 0x6A , PTZ_data.data[5]);
                                tp28xx_byte_write(chip, 0x6B , PTZ_data.data[6]);
                                tp28xx_byte_write(chip, 0x6C , PTZ_data.data[7]);
                                TP2826_StartTX(chip, PTZ_data.ch);
            }


        }
        else if( id[chip] >= TP2822 )
        {

            TP2822_PTZ_mode(chip, PTZ_data.ch, PTZ_data.mode );

            tp28xx_byte_write(chip, 0x40, 0x00);//data buffer bank0 switch for 2822
            if(id[chip] >= TP2853C) //clear extended MSB byte in TP28x3C
            {
                tp28xx_byte_write(chip, 0x7f + PTZ_data.ch*2, 0x00);
                tp28xx_byte_write(chip, 0x80 + PTZ_data.ch*2, 0x00);
            }
            tp28xx_byte_write(chip, 0x56 + PTZ_data.ch*10 , 0x00);
            tp28xx_byte_write(chip, 0x57 + PTZ_data.ch*10 , 0x00);
            tp28xx_byte_write(chip, 0x5B + PTZ_data.ch*10 , 0x00);
            tp28xx_byte_write(chip, 0x5C + PTZ_data.ch*10 , 0x00);

            tp28xx_byte_write(chip, 0x40, 0x10); //data buffer bank1 switch for 2822
            tp28xx_byte_write(chip, 0x56 + PTZ_data.ch*10 , 0x00);
            tp28xx_byte_write(chip, 0x57 + PTZ_data.ch*10 , 0x00);
            tp28xx_byte_write(chip, 0x5B + PTZ_data.ch*10 , 0x00);
            tp28xx_byte_write(chip, 0x5C + PTZ_data.ch*10 , 0x00);

            if( PTZ_TVI == PTZ_data.mode)
            {

                tp28xx_byte_write(chip, 0x40, 0x00);
                //line1
                tp28xx_byte_write(chip, 0x56 + PTZ_data.ch*10 , 0x02);
                tp28xx_byte_write(chip, 0x57 + PTZ_data.ch*10 , PTZ_data.data[0]);
                tp28xx_byte_write(chip, 0x58 + PTZ_data.ch*10 , PTZ_data.data[1]);
                tp28xx_byte_write(chip, 0x59 + PTZ_data.ch*10 , PTZ_data.data[2]);
                tp28xx_byte_write(chip, 0x5A + PTZ_data.ch*10 , PTZ_data.data[3]);
                //line2
                tp28xx_byte_write(chip, 0x5B + PTZ_data.ch*10 , 0x02);
                tp28xx_byte_write(chip, 0x5C + PTZ_data.ch*10 , PTZ_data.data[4]);
                tp28xx_byte_write(chip, 0x5D + PTZ_data.ch*10 , PTZ_data.data[5]);
                tp28xx_byte_write(chip, 0x5E + PTZ_data.ch*10 , PTZ_data.data[6]);
                tp28xx_byte_write(chip, 0x5F + PTZ_data.ch*10 , PTZ_data.data[7]);

                tp28xx_byte_write(chip, 0x40, 0x10);
                //line3
                tp28xx_byte_write(chip, 0x56 + PTZ_data.ch*10 , 0x02);
                tp28xx_byte_write(chip, 0x57 + PTZ_data.ch*10 , PTZ_data.data[0]);
                tp28xx_byte_write(chip, 0x58 + PTZ_data.ch*10 , PTZ_data.data[1]);
                tp28xx_byte_write(chip, 0x59 + PTZ_data.ch*10 , PTZ_data.data[2]);
                tp28xx_byte_write(chip, 0x5A + PTZ_data.ch*10 , PTZ_data.data[3]);
                //line4
                tp28xx_byte_write(chip, 0x5B + PTZ_data.ch*10 , 0x02);
                tp28xx_byte_write(chip, 0x5C + PTZ_data.ch*10 , PTZ_data.data[4]);
                tp28xx_byte_write(chip, 0x5D + PTZ_data.ch*10 , PTZ_data.data[5]);
                tp28xx_byte_write(chip, 0x5E + PTZ_data.ch*10 , PTZ_data.data[6]);
                tp28xx_byte_write(chip, 0x5F + PTZ_data.ch*10 , PTZ_data.data[7]);

                TP2822_StartTX(chip, PTZ_data.ch);

            }
           else if(PTZ_HDA_1080P == PTZ_data.mode || PTZ_HDA_3M18 == PTZ_data.mode || PTZ_HDA_3M25 == PTZ_data.mode || PTZ_HDA_4M25 == PTZ_data.mode || PTZ_HDA_4M15 == PTZ_data.mode) //HDA 1080p
            {

                tp28xx_byte_write(chip, 0x40, 0x00); //data buffer bank0 switch for 2822
                HDA_SetACPV2Data(chip, 0x58 + PTZ_data.ch*10, 0x00);
                HDA_SetACPV2Data(chip, 0x5D + PTZ_data.ch*10, 0x00);

                tp28xx_byte_write(chip, 0x40, 0x10); //data buffer bank0 switch for 2822
                HDA_SetACPV2Data(chip, 0x58 + PTZ_data.ch*10, 0x00);
                HDA_SetACPV2Data(chip, 0x5D + PTZ_data.ch*10, 0x00);

                // TX enable
                TP2822_StartTX(chip, PTZ_data.ch);

                tp28xx_byte_write(chip, 0x40, 0x00); //data buffer bank0 switch for 2822
                HDA_SetACPV2Data(chip, 0x58 + PTZ_data.ch*10, PTZ_data.data[0]);
                HDA_SetACPV2Data(chip, 0x5D + PTZ_data.ch*10, PTZ_data.data[1]);

                tp28xx_byte_write(chip, 0x40, 0x10); //data buffer bank0 switch for 2822
                HDA_SetACPV2Data(chip, 0x58 + PTZ_data.ch*10, PTZ_data.data[2]);
                HDA_SetACPV2Data(chip, 0x5D + PTZ_data.ch*10, PTZ_data.data[3]);

                TP2822_StartTX(chip, PTZ_data.ch);

            }
            else if( PTZ_HDA_720P == PTZ_data.mode || PTZ_HDA_CVBS == PTZ_data.mode)
            {
                tp28xx_byte_write(chip, 0x40, 0x00); //data buffer bank0 switch for 28x3C

                //line1
                tmp = ConvertACPV1Data(0x00);
                tp28xx_byte_write(chip, 0x7f + PTZ_data.ch*2 , (tmp>>16)&0xff);
                tp28xx_byte_write(chip, 0x56 + PTZ_data.ch*10 , (tmp>>8)&0xff);
                tp28xx_byte_write(chip, 0x57 + PTZ_data.ch*10 , (tmp)&0xff);

                tp28xx_byte_write(chip, 0x58 + PTZ_data.ch*10 , (tmp>>16)&0xff);
                tp28xx_byte_write(chip, 0x59 + PTZ_data.ch*10 , (tmp>>8)&0xff);
                tp28xx_byte_write(chip, 0x5A + PTZ_data.ch*10 , (tmp)&0xff);

                //line2
                tp28xx_byte_write(chip, 0x80 + PTZ_data.ch*2 , (tmp>>16)&0xff);
                tp28xx_byte_write(chip, 0x5B + PTZ_data.ch*10 , (tmp>>8)&0xff);
                tp28xx_byte_write(chip, 0x5C + PTZ_data.ch*10 , (tmp)&0xff);

                tp28xx_byte_write(chip, 0x5D + PTZ_data.ch*10, (tmp>>16)&0xff);
                tp28xx_byte_write(chip, 0x5E + PTZ_data.ch*10 , (tmp>>8)&0xff);
                tp28xx_byte_write(chip, 0x5F + PTZ_data.ch*10 , (tmp)&0xff);

                // TX enable
                TP2822_StartTX(chip, PTZ_data.ch);

                //line1
                tmp = ConvertACPV1Data(PTZ_data.data[0]);
                tp28xx_byte_write(chip, 0x7f + PTZ_data.ch*2 , (tmp>>16)&0xff);
                tp28xx_byte_write(chip, 0x56 + PTZ_data.ch*10 , (tmp>>8)&0xff);
                tp28xx_byte_write(chip, 0x57 + PTZ_data.ch*10 , (tmp)&0xff);

                tmp = ConvertACPV1Data(PTZ_data.data[1]);
                tp28xx_byte_write(chip, 0x58 + PTZ_data.ch*10 , (tmp>>16)&0xff);
                tp28xx_byte_write(chip, 0x59 + PTZ_data.ch*10 , (tmp>>8)&0xff);
                tp28xx_byte_write(chip, 0x5A + PTZ_data.ch*10 , (tmp)&0xff);

                //line2
                tmp = ConvertACPV1Data(PTZ_data.data[2]);
                tp28xx_byte_write(chip, 0x80 + PTZ_data.ch*2 , (tmp>>16)&0xff);
                tp28xx_byte_write(chip, 0x5B + PTZ_data.ch*10 , (tmp>>8)&0xff);
                tp28xx_byte_write(chip, 0x5C + PTZ_data.ch*10 , (tmp)&0xff);

                tmp = ConvertACPV1Data(PTZ_data.data[3]);
                tp28xx_byte_write(chip, 0x5D + PTZ_data.ch*10, (tmp>>16)&0xff);
                tp28xx_byte_write(chip, 0x5E + PTZ_data.ch*10 , (tmp>>8)&0xff);
                tp28xx_byte_write(chip, 0x5F + PTZ_data.ch*10 , (tmp)&0xff);

                TP2822_StartTX(chip, PTZ_data.ch);

            }
            else if( PTZ_HDC == PTZ_data.mode ) //
            {

                tp28xx_byte_write(chip, 0x40, 0x00); //data buffer bank0 switch for 2822

                HDC_SetData(chip, 0x56 + PTZ_data.ch*10, PTZ_data.data[0]);
                HDC_SetData(chip, 0x5b + PTZ_data.ch*10, PTZ_data.data[1]);

                tp28xx_byte_write(chip, 0x40, 0x10); //data buffer bank0 switch for 2822
                HDC_SetData(chip, 0x56 + PTZ_data.ch*10, PTZ_data.data[2]);
                HDC_SetData(chip, 0x5b + PTZ_data.ch*10, PTZ_data.data[3]);

                // TX enable
                TP2822_StartTX(chip, PTZ_data.ch);

                tp28xx_byte_write(chip, 0x40, 0x00); //data buffer bank0 switch for 2822
                HDC_SetData(chip, 0x56 + PTZ_data.ch*10, PTZ_data.data[4]);
                HDC_SetData(chip, 0x5b + PTZ_data.ch*10, PTZ_data.data[5]);

                tp28xx_byte_write(chip, 0x40, 0x10); //data buffer bank0 switch for 2822
                HDC_SetData(chip, 0x56 + PTZ_data.ch*10, PTZ_data.data[6]);
                HDC_SetData(chip, 0x5b + PTZ_data.ch*10, 0xffff);

                TP2822_StartTX(chip, PTZ_data.ch);

            }
            else if( PTZ_HDC_QHD == PTZ_data.mode ) //
            {

                tp28xx_byte_write(chip, 0x40, 0x00); //data buffer bank0 switch for 2822

                HDC_QHD_SetData(chip, 0x56 + PTZ_data.ch*10, PTZ_data.data[0]);
                HDC_QHD_SetData(chip, 0x5b + PTZ_data.ch*10, PTZ_data.data[1]);

                tp28xx_byte_write(chip, 0x40, 0x10); //data buffer bank0 switch for 2822
                HDC_QHD_SetData(chip, 0x56 + PTZ_data.ch*10, PTZ_data.data[2]);
                HDC_QHD_SetData(chip, 0x5b + PTZ_data.ch*10, PTZ_data.data[3]);

                // TX enable
                TP2822_StartTX(chip, PTZ_data.ch);

                tp28xx_byte_write(chip, 0x40, 0x00); //data buffer bank0 switch for 2822
                HDC_QHD_SetData(chip, 0x56 + PTZ_data.ch*10, PTZ_data.data[4]);
                HDC_QHD_SetData(chip, 0x5b + PTZ_data.ch*10, PTZ_data.data[5]);

                tp28xx_byte_write(chip, 0x40, 0x10); //data buffer bank0 switch for 2822
                HDC_QHD_SetData(chip, 0x56 + PTZ_data.ch*10, PTZ_data.data[6]);
                HDC_QHD_SetData(chip, 0x5b + PTZ_data.ch*10, 0xffff);

                TP2822_StartTX(chip, PTZ_data.ch);

            }
        }
        else if(TP2802D == id[PTZ_data.chip] )
        {
            tp28xx_byte_write(chip, 0x40, 0x00); //data buffer bank0 switch for 2802D
            tp28xx_byte_write(chip, 0xBB, tp28xx_byte_read(chip,0xBB) & ~(0x11<<(PTZ_data.ch))); // TX disable



            tmp = tp28xx_byte_read(chip, 0xf5); //check TVI 1 or 2
            if( (tmp >>PTZ_data.ch) & 0x01)
            {
                tp28xx_byte_write(chip, 0x53, 0x33);
                tp28xx_byte_write(chip, 0x54, 0xf0);

            }
            else
            {
                tp28xx_byte_write(chip, 0x53, 0x19);
                tp28xx_byte_write(chip, 0x54, 0x78);
            }

            // TX disable
            tp28xx_byte_write(chip, 0xBB, tp28xx_byte_read(chip,0xBB) & ~(0x01<<(PTZ_data.ch)));

            //line1
            tp28xx_byte_write(chip, 0x56 + PTZ_data.ch*10 , 0x02);
            tp28xx_byte_write(chip, 0x57 + PTZ_data.ch*10 , PTZ_data.data[0]);
            tp28xx_byte_write(chip, 0x58 + PTZ_data.ch*10 , PTZ_data.data[1]);
            tp28xx_byte_write(chip, 0x59 + PTZ_data.ch*10 , PTZ_data.data[2]);
            tp28xx_byte_write(chip, 0x5A + PTZ_data.ch*10 , PTZ_data.data[3]);
            //line2
            tp28xx_byte_write(chip, 0x5B + PTZ_data.ch*10 , 0x02);
            tp28xx_byte_write(chip, 0x5C + PTZ_data.ch*10 , PTZ_data.data[4]);
            tp28xx_byte_write(chip, 0x5D + PTZ_data.ch*10 , PTZ_data.data[5]);
            tp28xx_byte_write(chip, 0x5E + PTZ_data.ch*10 , PTZ_data.data[6]);
            tp28xx_byte_write(chip, 0x5F + PTZ_data.ch*10 , PTZ_data.data[7]);

            // TX enable
            tp28xx_byte_write(chip, 0xBB, (0x01<<(PTZ_data.ch)));

        }

        pthread_mutex_unlock(&lock);
        break;
    }
    case _IOC_NR(TP2802_GET_PTZ_DATA):
    {
        if (copy_from_user(&PTZ_data, argp, sizeof(tp2802_PTZ_data)))
        {
            return FAILURE;
        }

        if(PTZ_data.ch >= CHANNELS_PER_CHIP)  return FAILURE;

        pthread_mutex_lock(&lock);

        chip = PTZ_data.chip;

        if( TP2827 == id[chip] || TP2826 == id[chip] || TP2816 == id[chip] || TP2826C == id[chip] || TP2827C == id[chip] || TP2828 == id[chip] || TP2829 == id[chip] || TP2830 == id[chip] || TP2831 == id[chip]) // Added TP2830/31 ID - 10/15/2018
        {
            tp28xx_byte_write(chip, 0x40,  PTZ_data.ch); //bank switch
            if(PTZ_RX_TVI_CMD == PTZ_data.mode || PTZ_RX_TVI_BURST == PTZ_data.mode)
            {
            // line1
                PTZ_data.data[0]=tp28xx_byte_read(chip, 0x8C );
                PTZ_data.data[1]=tp28xx_byte_read(chip, 0x8D );
                PTZ_data.data[2]=tp28xx_byte_read(chip, 0x8E );
                PTZ_data.data[3]=tp28xx_byte_read(chip, 0x8F );
            //line2
                PTZ_data.data[4]=tp28xx_byte_read(chip, 0x92 );
                PTZ_data.data[5]=tp28xx_byte_read(chip, 0x93 );
                PTZ_data.data[6]=tp28xx_byte_read(chip, 0x94 );
                PTZ_data.data[7]=tp28xx_byte_read(chip, 0x95 );
            // line3
                PTZ_data.data[8]=tp28xx_byte_read(chip, 0x98 );
                PTZ_data.data[9]=tp28xx_byte_read(chip, 0x99 );
                PTZ_data.data[10]=tp28xx_byte_read(chip, 0x9a );
                PTZ_data.data[11]=tp28xx_byte_read(chip, 0x9b );
            //line4
                PTZ_data.data[12]=tp28xx_byte_read(chip, 0x9e );
                PTZ_data.data[13]=tp28xx_byte_read(chip, 0x9f );
                PTZ_data.data[14]=tp28xx_byte_read(chip, 0xa0 );
                PTZ_data.data[15]=tp28xx_byte_read(chip, 0xa1 );
            }
            else if(PTZ_RX_ACP1 == PTZ_data.mode || PTZ_RX_ACP2 == PTZ_data.mode || PTZ_RX_ACP3 == PTZ_data.mode)
            {

                for(i =0; i < 6; i++)
                    PTZ_data.data[i]=tp28xx_byte_read(chip, 0x8a+i );
                PTZ_data.data[8]=PELCO_Decode(PTZ_data.data);

                for(i =0; i < 6; i++)
                    PTZ_data.data[i]=tp28xx_byte_read(chip, 0x90+i );
                PTZ_data.data[9]=PELCO_Decode(PTZ_data.data);

                for(i =0; i < 6; i++)
                    PTZ_data.data[i]=tp28xx_byte_read(chip, 0x96+i );
                PTZ_data.data[10]=PELCO_Decode(PTZ_data.data);

                for(i =0; i < 6; i++)
                    PTZ_data.data[i]=tp28xx_byte_read(chip, 0x9c+i );
                PTZ_data.data[11]=PELCO_Decode(PTZ_data.data);

                for(i =0; i < 4; i++)
                    PTZ_data.data[i]=PTZ_data.data[8+i];
/*

                PTZ_data.data[0]=ReverseByte(tp28xx_byte_read(chip, 0x8f ));
                PTZ_data.data[1]=ReverseByte(tp28xx_byte_read(chip, 0x8e ));
                PTZ_data.data[2]=ReverseByte(tp28xx_byte_read(chip, 0x8d ));
                PTZ_data.data[3]=ReverseByte(tp28xx_byte_read(chip, 0x8c ));
                PTZ_data.data[4]=ReverseByte(tp28xx_byte_read(chip, 0x95 ));
                PTZ_data.data[5]=ReverseByte(tp28xx_byte_read(chip, 0x94 ));
                PTZ_data.data[6]=ReverseByte(tp28xx_byte_read(chip, 0x93 ));
                PTZ_data.data[7]=ReverseByte(tp28xx_byte_read(chip, 0x92 ));
*/
            }
            else
            {
                PTZ_data.data[0]=tp28xx_byte_read(chip, 0xa3 );
                PTZ_data.data[1]=tp28xx_byte_read(chip, 0xa3 );
                PTZ_data.data[2]=tp28xx_byte_read(chip, 0xa3 );
                PTZ_data.data[3]=tp28xx_byte_read(chip, 0xa3 );
                PTZ_data.data[4]=tp28xx_byte_read(chip, 0xa3 );
                PTZ_data.data[5]=tp28xx_byte_read(chip, 0xa3 );
                PTZ_data.data[6]=tp28xx_byte_read(chip, 0xa3 );

            }

        }
        else
        {
            tp28xx_byte_write(chip, 0x40, 0x00); //bank switch for 0
            // line1
            PTZ_data.data[0]=tp28xx_byte_read(chip, 0x8C + PTZ_data.ch*10 );
            PTZ_data.data[1]=tp28xx_byte_read(chip, 0x8D + PTZ_data.ch*10 );
            PTZ_data.data[2]=tp28xx_byte_read(chip, 0x8E + PTZ_data.ch*10 );
            PTZ_data.data[3]=tp28xx_byte_read(chip, 0x8F + PTZ_data.ch*10 );

            //line2
            PTZ_data.data[4]=tp28xx_byte_read(chip, 0x91 + PTZ_data.ch*10 );
            PTZ_data.data[5]=tp28xx_byte_read(chip, 0x92 + PTZ_data.ch*10 );
            PTZ_data.data[6]=tp28xx_byte_read(chip, 0x93 + PTZ_data.ch*10 );
            PTZ_data.data[7]=tp28xx_byte_read(chip, 0x94 + PTZ_data.ch*10 );

        }

        pthread_mutex_unlock(&lock);

        if (copy_to_user(argp, &PTZ_data, sizeof(tp2802_PTZ_data)))
        {
            return FAILURE;
        }

        break;
    }
    case _IOC_NR(TP2802_SET_SCAN_MODE):
    {
        if (copy_from_user(&work_mode, argp, sizeof(tp2802_work_mode)))
            return FAILURE;

        pthread_mutex_lock(&lock);

        if(work_mode.ch >= CHANNELS_PER_CHIP)
        {
            for(i = 0; i < CHANNELS_PER_CHIP; i++)
                watchdog_info[work_mode.chip].scan[i] = work_mode.mode;
        }
        else
        {
            watchdog_info[work_mode.chip].scan[work_mode.ch] = work_mode.mode;

        }

        pthread_mutex_unlock(&lock);


        break;
    }
    case _IOC_NR(TP2802_DUMP_REG):
    {
        if (copy_from_user(&dev_register, argp, sizeof(tp2802_register)))
            return FAILURE;

        pthread_mutex_lock(&lock);

        for(i = 0; i < CHANNELS_PER_CHIP ; i++)
        {

            tp2802_set_reg_page(dev_register.chip, i);

            for(j = 0; j < 0x100; j++)
            {
                dev_register.value = tp28xx_byte_read(dev_register.chip, j);
                printf("%02x:%02x\n", j, dev_register.value );
            }
        }
        /*
        for(j = 0x40; j < 0x100; j++)
        {
            dev_register.value = tp28xx_byte_read(dev_register.chip, j);
            printf("%02x:%02x\n", j, dev_register.value );
        }*/
        tp2802_set_reg_page(dev_register.chip, AUDIO_PAGE);
        for(j = 0x0; j < 0x80; j++)
        {
            dev_register.value = tp28xx_byte_read(dev_register.chip, j);
            printf("%02x:%02x\n", j, dev_register.value );
        }
        pthread_mutex_unlock(&lock);

        if (copy_to_user(argp, &dev_register, sizeof(tp2802_register)))
            return FAILURE;

        break;
    }
    case _IOC_NR(TP2802_FORCE_DETECT):
    {
        if (copy_from_user(&work_mode, argp, sizeof(tp2802_work_mode)))
            return FAILURE;

        pthread_mutex_lock(&lock);

        if(work_mode.ch >= CHANNELS_PER_CHIP)
        {
            for(i = 0; i < CHANNELS_PER_CHIP; i++)
                watchdog_info[work_mode.chip].force[i] = 1;
        }
        else
        {
            watchdog_info[work_mode.chip].force[work_mode.ch] = 1;

        }

        pthread_mutex_unlock(&lock);


        break;
    }
    case _IOC_NR(TP2802_SET_SAMPLE_RATE):
    {
        tp2802_audio_samplerate samplerate;

        if (copy_from_user(&samplerate, argp, sizeof(samplerate)))
            return FAILURE;

        pthread_mutex_lock(&lock);

        for (i = 0; i < chips; i ++)
        {
            tp2802_set_reg_page(i, AUDIO_PAGE); //audio page
            tmp = tp28xx_byte_read(i, 0x18);
            tmp &= 0xf8;

            if(SAMPLE_RATE_16000 == samplerate)   tmp |= 0x01;

            tp28xx_byte_write(i, 0x18, tmp);
            tp28xx_byte_write(i, 0x3d, 0x01); //audio reset
        }

        pthread_mutex_unlock(&lock);
        break;
    }

    case _IOC_NR(TP2802_SET_AUDIO_PLAYBACK):
    {

        if (copy_from_user(&audio_playback, argp, sizeof(tp2802_audio_playback)))
            return FAILURE;
        if(audio_playback.chn > 25 || audio_playback.chn < 1)
            return FAILURE;;

        pthread_mutex_lock(&lock);

        tp2802_set_reg_page(audio_playback.chip, AUDIO_PAGE); //audio page

        tp28xx_byte_write(audio_playback.chip, 0x1a, audio_playback.chn);

        pthread_mutex_unlock(&lock);

        break;
    }
    case _IOC_NR(TP2802_SET_AUDIO_DA_VOLUME):
    {
        if (copy_from_user(&audio_da_volume, argp, sizeof(tp2802_audio_da_volume)))
            return FAILURE;
        if(audio_da_volume.volume > 15 || audio_da_volume.volume < 0)
            return FAILURE;;

        pthread_mutex_lock(&lock);

        tp2802_set_reg_page(audio_da_volume.chip, AUDIO_PAGE); //audio page
        tp28xx_byte_write(audio_da_volume.chip, 0x1f, audio_da_volume.volume);

        pthread_mutex_unlock(&lock);

        break;
    }
    case _IOC_NR(TP2802_SET_AUDIO_DA_MUTE):
    {
        tp2802_audio_da_mute audio_da_mute;

        if (copy_from_user(&audio_da_mute, argp, sizeof(tp2802_audio_da_mute)))
            return FAILURE;
        if(audio_da_mute.chip > chips || audio_da_mute.chip < 0)
            return FAILURE;;

        pthread_mutex_lock(&lock);

        tp2802_set_reg_page(audio_da_mute.chip, AUDIO_PAGE); //audio page
        tmp = tp28xx_byte_read(audio_da_mute.chip, 0x38);
        tmp &=0xf0;
        if(audio_da_mute.flag)
        {
            tp28xx_byte_write(audio_da_mute.chip, 0x38, tmp);
        }
        else
        {
            tmp |=0x08;
            tp28xx_byte_write(audio_da_mute.chip, 0x38, tmp);
        }

        pthread_mutex_unlock(&lock);

        break;
    }
    case _IOC_NR(TP2802_SET_BURST_DATA):
    case _IOC_NR(TP2802_SET_BURST_NEW):
    {
        if (copy_from_user(&PTZ_data, argp, sizeof(tp2802_PTZ_data)))
        {
            return FAILURE;
        }

        if(PTZ_data.ch >= CHANNELS_PER_CHIP)  return FAILURE;

        pthread_mutex_lock(&lock);

        chip = PTZ_data.chip;

        if( TP2827 == id[chip] || TP2826 == id[chip] || TP2816 == id[chip] || TP2826C == id[chip] || TP2827C == id[chip] || TP2828 == id[chip] || TP2829 == id[chip] || TP2830 == id[chip] || TP2831 == id[chip]) // Added TP2830/31 ID - 10/15/2018
        {
            TP2827C_PTZ_mode(chip, PTZ_data.ch, PTZ_TVI);

                tp28xx_byte_write(chip, 0x55, 0x00);
                tp28xx_byte_write(chip, 0x5b, 0x00);

                            //line1
                                tp28xx_byte_write(chip, 0x56 , 0x03);
                                tp28xx_byte_write(chip, 0x57 , PTZ_data.data[0]);
                                tp28xx_byte_write(chip, 0x58 , PTZ_data.data[1]);
                                tp28xx_byte_write(chip, 0x59 , PTZ_data.data[2]);
                                tp28xx_byte_write(chip, 0x5A , PTZ_data.data[3]);
                            //line2
                                tp28xx_byte_write(chip, 0x5C , 0x03);
                                tp28xx_byte_write(chip, 0x5D , PTZ_data.data[4]);
                                tp28xx_byte_write(chip, 0x5E , PTZ_data.data[5]);
                                tp28xx_byte_write(chip, 0x5F , PTZ_data.data[6]);
                                tp28xx_byte_write(chip, 0x60 , PTZ_data.data[7]);
                            //line3
                                tp28xx_byte_write(chip, 0x62 , 0x03);
                                tp28xx_byte_write(chip, 0x63 , PTZ_data.data[8]);
                                tp28xx_byte_write(chip, 0x64 , PTZ_data.data[9]);
                                tp28xx_byte_write(chip, 0x65 , PTZ_data.data[10]);
                                tp28xx_byte_write(chip, 0x66 , PTZ_data.data[11]);
                            //line4
                                tp28xx_byte_write(chip, 0x68 , 0x03);
                                tp28xx_byte_write(chip, 0x69 , PTZ_data.data[12]);
                                tp28xx_byte_write(chip, 0x6A , PTZ_data.data[13]);
                                tp28xx_byte_write(chip, 0x6B , PTZ_data.data[14]);
                                tp28xx_byte_write(chip, 0x6C , PTZ_data.data[15]);

            TP2826_StartTX(chip, PTZ_data.ch);

        }
        else if( id[chip] >= TP2822 )
        {

            TP2822_PTZ_mode(chip, PTZ_data.ch, PTZ_TVI);


                tp28xx_byte_write(chip, 0xBB, tp28xx_byte_read(chip,0xBB) & ~(0x11<<(PTZ_data.ch))); // TX disable

                tp28xx_byte_write(chip, 0x40, 0x00); //data buffer bank0 switch for 2822

                if(id[chip] >= TP2853C) //clear extended MSB byte in TP28x3C
                {
                    tp28xx_byte_write(chip, 0x7f + PTZ_data.ch*2, 0x00);
                    tp28xx_byte_write(chip, 0x80 + PTZ_data.ch*2, 0x00);
                }

                //line1
                tp28xx_byte_write(chip, 0x56 + PTZ_data.ch*10 , 0x03);
                tp28xx_byte_write(chip, 0x57 + PTZ_data.ch*10 , PTZ_data.data[0]);
                tp28xx_byte_write(chip, 0x58 + PTZ_data.ch*10 , PTZ_data.data[1]);
                tp28xx_byte_write(chip, 0x59 + PTZ_data.ch*10 , PTZ_data.data[2]);
                tp28xx_byte_write(chip, 0x5A + PTZ_data.ch*10 , PTZ_data.data[3]);
                //line2
                tp28xx_byte_write(chip, 0x5B + PTZ_data.ch*10 , 0x03);
                tp28xx_byte_write(chip, 0x5C + PTZ_data.ch*10 , PTZ_data.data[4]);
                tp28xx_byte_write(chip, 0x5D + PTZ_data.ch*10 , PTZ_data.data[5]);
                tp28xx_byte_write(chip, 0x5E + PTZ_data.ch*10 , PTZ_data.data[6]);
                tp28xx_byte_write(chip, 0x5F + PTZ_data.ch*10 , PTZ_data.data[7]);

                tp28xx_byte_write(chip, 0x40, 0x10); //data buffer bank0 switch for 2822

                //line1
                tp28xx_byte_write(chip, 0x56 + PTZ_data.ch*10 , 0x03);
                tp28xx_byte_write(chip, 0x57 + PTZ_data.ch*10 , PTZ_data.data[8]);
                tp28xx_byte_write(chip, 0x58 + PTZ_data.ch*10 , PTZ_data.data[9]);
                tp28xx_byte_write(chip, 0x59 + PTZ_data.ch*10 , PTZ_data.data[10]);
                tp28xx_byte_write(chip, 0x5A + PTZ_data.ch*10 , PTZ_data.data[11]);
                //line2
                tp28xx_byte_write(chip, 0x5B + PTZ_data.ch*10 , 0x03);
                tp28xx_byte_write(chip, 0x5C + PTZ_data.ch*10 , PTZ_data.data[12]);
                tp28xx_byte_write(chip, 0x5D + PTZ_data.ch*10 , PTZ_data.data[13]);
                tp28xx_byte_write(chip, 0x5E + PTZ_data.ch*10 , PTZ_data.data[14]);
                tp28xx_byte_write(chip, 0x5F + PTZ_data.ch*10 , PTZ_data.data[15]);

                tp28xx_byte_write(chip, 0xBB, (0x01<<(PTZ_data.ch))); //TX enable

        }

        pthread_mutex_unlock(&lock);
        break;
    }

    case _IOC_NR(TP2802_SET_HDA_MODE): //
    {
        if (copy_from_user(&PTZ_data, argp, sizeof(tp2802_PTZ_data)))
        {
            return FAILURE;
        }

        if(PTZ_data.ch >= CHANNELS_PER_CHIP)  return FAILURE;

        pthread_mutex_lock(&lock);

        chip = PTZ_data.chip;

        if( TP2827C == id[chip] || TP2826C == id[chip] || TP2828 == id[chip] || TP2829 == id[chip] || TP2830 == id[chip] || TP2831 == id[chip] ) // Added TP2830/31 ID - 10/15/2018
        {
            TP2827C_PTZ_mode(chip, PTZ_data.ch, PTZ_data.mode );

            for(i = 0; i < 24; i++)
            {
                tp28xx_byte_write(chip, 0x55+i, 0x00);
            }


            if(PTZ_HDA_4M25 == PTZ_data.mode || PTZ_HDA_4M15 == PTZ_data.mode) //HDA QHD
            {

                           for(i = 0; i < 8; i++)
                            {
                                tp28xx_byte_write(chip, 0x6e, 0x00);
                            }
                            TP2826_StartTX(chip, PTZ_data.ch);

                            for(i = 0; i < 8; i++)
                            {
                                tp28xx_byte_write(chip, 0x6e, ReverseByte(PTZ_data.data[i]));
                            }
                            TP2826_StartTX(chip, PTZ_data.ch);
                            for(i = 0; i < 8; i++)
                            {
                                tp28xx_byte_write(chip, 0x6e, ReverseByte(PTZ_data.data[i]));
                            }
                            TP2826_StartTX(chip, PTZ_data.ch);
                            for(i = 0; i < 8; i++)
                            {
                                tp28xx_byte_write(chip, 0x6e, ReverseByte(PTZ_data.data[i]));
                            }
                            TP2826_StartTX(chip, PTZ_data.ch);
                            for(i = 0; i < 8; i++)
                            {
                                tp28xx_byte_write(chip, 0x6e, ReverseByte(PTZ_data.data[i]));
                            }
                            TP2826_StartTX(chip, PTZ_data.ch);
            }

        }
        else if( TP2827 == id[chip] || TP2826 == id[chip] || TP2816 == id[chip])
        {
            TP2826_PTZ_mode(chip, PTZ_data.ch, PTZ_data.mode );

            for(i = 0; i < 24; i++)
            {
                tp28xx_byte_write(chip, 0x55+i, 0x00);
            }

            if(PTZ_HDA_1080P == PTZ_data.mode || PTZ_HDA_3M18 == PTZ_data.mode || PTZ_HDA_3M25 == PTZ_data.mode || PTZ_HDA_4M25 == PTZ_data.mode || PTZ_HDA_4M15 == PTZ_data.mode) //HDA 1080p
            {
                                HDA_SetACPV2Data(chip, 0x58, 0x00);
                                HDA_SetACPV2Data(chip, 0x5e, 0x00);
                                HDA_SetACPV2Data(chip, 0x64, 0x00);
                                HDA_SetACPV2Data(chip, 0x6a, 0x00);
                                TP2826_StartTX(chip, PTZ_data.ch);
                                HDA_SetACPV2Data(chip, 0x58, PTZ_data.data[0]);
                                HDA_SetACPV2Data(chip, 0x5e, PTZ_data.data[1]);
                                HDA_SetACPV2Data(chip, 0x64, PTZ_data.data[2]);
                                HDA_SetACPV2Data(chip, 0x6a, PTZ_data.data[3]);
                                TP2826_StartTX(chip, PTZ_data.ch);
                                TP2826_StartTX(chip, PTZ_data.ch);
                                TP2826_StartTX(chip, PTZ_data.ch);
                                TP2826_StartTX(chip, PTZ_data.ch);

            }


        }


        pthread_mutex_unlock(&lock);
        break;
    }

    case _IOC_NR(TP2802_SET_RX_MODE):
    {
        if (copy_from_user(&PTZ_data, argp, sizeof(tp2802_PTZ_data)))
        {
            return FAILURE;
        }

        if(PTZ_data.ch >= CHANNELS_PER_CHIP)  return FAILURE;

        pthread_mutex_lock(&lock);

        chip = PTZ_data.chip;

        if( TP2830 == id[chip] || TP2831 == id[chip] ) // Added TP2830/31 ID - 10/15/2018
        {
            tp28xx_byte_write(chip, 0x40,  PTZ_data.ch); //bank switch

            TP2831_RX_init(chip, PTZ_data.mode);

        }
        else if( TP2827C == id[chip] || TP2826C == id[chip] || TP2828 == id[chip] || TP2829 == id[chip] )
        {
            tp28xx_byte_write(chip, 0x40,  PTZ_data.ch); //bank switch

            TP2829_RX_init(chip, PTZ_data.mode);

        }
        else if( TP2827 == id[chip] || TP2826 == id[chip] || TP2816 == id[chip])
        {
            tp28xx_byte_write(chip, 0x40,  PTZ_data.ch); //bank switch

            TP2827_RX_init(chip, PTZ_data.mode );

        }


        pthread_mutex_unlock(&lock);
        break;
    }

    case _IOC_NR(TP2830_SET_AUDIO_COAX_CHANNEL): // Added TP2830_SET_AUDIO_COAX_CHANNEL - 10/16/2018
    {
        tp2830_audio_coax_channel audio_coax_channel;

        if (copy_from_user(&audio_coax_channel, argp, sizeof(tp2830_audio_coax_channel)))
            return FAILURE;

        chip = audio_coax_channel.chip;

        if( TP2830 != id[chip] && TP2831 != id[chip] )
        {
            printf("\nchip id [ %04x ] doesn't support the audio coax function\n", id[chip]);
            return FAILURE; // This function is only supported in TP2830/31 chip.
        }

        if ( ( audio_coax_channel.chn < 0 ) && ( audio_coax_channel.chn > 4 ) ) return FAILURE; // audio channel range is from 0 to 4 (0 : ch1, 1 : ch2, 2 : ch3, 3 : ch4, 4 : ch_all).

        pthread_mutex_lock(&lock);

        tp2802_set_reg_page(chip, AUDIO_PAGE); //audio page

        if(0 == chip ) //chip that I2S interface with Hi35xx
        {
            if (audio_coax_channel.input_type ) // 1 : coaxial audio, 0 : AIN base-band audio
            {
                if(audio_channels > 4 || audio_format == DSP)
                {
                    if(audio_coax_channel.chn < 4)
                    {
                        tp28xx_byte_write(chip, audio_coax_channel.chn, 0x1a+audio_coax_channel.chn);
                    }
                    else
                    {
                        tp28xx_byte_write(chip, 0, 0x1a);
                        tp28xx_byte_write(chip, 1, 0x1b);
                        tp28xx_byte_write(chip, 2, 0x1c);
                        tp28xx_byte_write(chip, 3, 0x1d);
                    }
                }
                else
                {
                    if(audio_coax_channel.chn < 2)
                    {
                        tp28xx_byte_write(chip, audio_coax_channel.chn, 0x1a+audio_coax_channel.chn);
                    }
                    if(audio_coax_channel.chn < 4)
                    {
                        tp28xx_byte_write(chip, audio_coax_channel.chn+0x08, 0x1a+audio_coax_channel.chn);
                    }
                    else
                    {
                        tp28xx_byte_write(chip, 0, 0x1a);
                        tp28xx_byte_write(chip, 1, 0x1b);
                        tp28xx_byte_write(chip, 8, 0x1c);
                        tp28xx_byte_write(chip, 9, 0x1d);
                    }
                }
            }
            else //AIN analog audio
            {
                if(audio_channels > 4 || audio_format == DSP)
                {
                    if(audio_coax_channel.chn < 4)
                    {
                        tp28xx_byte_write(chip, audio_coax_channel.chn, 0x01+audio_coax_channel.chn);
                    }
                    else
                    {
                        tp28xx_byte_write(chip, 0, 0x01);
                        tp28xx_byte_write(chip, 1, 0x02);
                        tp28xx_byte_write(chip, 2, 0x03);
                        tp28xx_byte_write(chip, 3, 0x04);
                    }
                }
                else
                {
                    if(audio_coax_channel.chn < 2)
                    {
                        tp28xx_byte_write(chip, audio_coax_channel.chn, 0x01+audio_coax_channel.chn);
                    }
                    if(audio_coax_channel.chn < 4)
                    {
                        tp28xx_byte_write(chip, audio_coax_channel.chn+0x08, 0x01+audio_coax_channel.chn);
                    }
                    else
                    {
                        tp28xx_byte_write(chip, 0, 0x01);
                        tp28xx_byte_write(chip, 1, 0x02);
                        tp28xx_byte_write(chip, 8, 0x03);
                        tp28xx_byte_write(chip, 9, 0x04);
                    }
                }
            }

        }
        else
        {
            tmp = tp28xx_byte_read(chip, CXSEL_REG7B); // get Coaxial Audio Selection Register 0x7B.
            if (audio_coax_channel.input_type ) // 1 : coaxial audio, 0 : AIN base-band audio
            {
                tmp |= SYS_MODE[audio_coax_channel.chn]; // set to high on the specific channel.
            }
            else
            {
                tmp &= SYS_AND[audio_coax_channel.chn]; // set to low on the specific channel.
            }
            tp28xx_byte_write(chip, CXSEL_REG7B, tmp);
        }

        pthread_mutex_unlock(&lock);
        break;
    }

    default:
    {
        printf("Invalid tp2802 ioctl cmd!\n");
        ret = -1;
        break;
    }
    }

    return ret;
}
#endif

static void TP2823_NTSC_DataSet(unsigned char chip);
static void TP2823_PAL_DataSet(unsigned char chip);
static void TP2823_V2_DataSet(unsigned char chip);
static void TP2823_V1_DataSet(unsigned char chip);
static void TP2834_NTSC_DataSet(unsigned char chip);
static void TP2834_PAL_DataSet(unsigned char chip);
static void TP2834_V2_DataSet(unsigned char chip);
static void TP2834_V1_DataSet(unsigned char chip);
static void TP2833_NTSC_DataSet(unsigned char chip);
static void TP2833_PAL_DataSet(unsigned char chip);
static void TP2833_V2_DataSet(unsigned char chip);
static void TP2833_V1_DataSet(unsigned char chip);
static void TP2833_A1080N_DataSet(unsigned char chip);
static void TP2833_A1080P_DataSet(unsigned char chip);
static void TP2833_A720N_DataSet(unsigned char chip);
static void TP2833_A720P_DataSet(unsigned char chip);
static void TP2853C_NTSC_DataSet(unsigned char chip);
static void TP2853C_PAL_DataSet(unsigned char chip);
static void TP2853C_V2_DataSet(unsigned char chip);
static void TP2853C_V1_DataSet(unsigned char chip);
static void TP2853C_A1080P30_DataSet(unsigned char chip);
static void TP2853C_A1080P25_DataSet(unsigned char chip);
static void TP2853C_A720P30_DataSet(unsigned char chip);
static void TP2853C_A720P25_DataSet(unsigned char chip);
static void TP2853C_C1080P30_DataSet(unsigned char chip);
static void TP2853C_C1080P25_DataSet(unsigned char chip);
static void TP2853C_C720P30_DataSet(unsigned char chip);
static void TP2853C_C720P25_DataSet(unsigned char chip);
static void TP2853C_C720P60_DataSet(unsigned char chip);
static void TP2853C_C720P50_DataSet(unsigned char chip);
static void tp282x_SYSCLK_V1(unsigned char chip, unsigned char ch);
static void tp282x_SYSCLK_V2(unsigned char chip, unsigned char ch);
static void tp282x_SYSCLK_V3(unsigned char chip, unsigned char ch);

static void TP2834_C1080P30_DataSet(unsigned char chip);
static void TP2834_C1080P25_DataSet(unsigned char chip);
static void TP2834_C720P30_DataSet(unsigned char chip);
static void TP2834_C720P25_DataSet(unsigned char chip);
static void TP2834_C720P60_DataSet(unsigned char chip);
static void TP2834_C720P50_DataSet(unsigned char chip);

static void TP2826_NTSC_DataSet(unsigned char chip);
static void TP2826_PAL_DataSet(unsigned char chip);
static void TP2826_V2_DataSet(unsigned char chip);
static void TP2826_V1_DataSet(unsigned char chip);
static void TP2826_A1080P30_DataSet(unsigned char chip);
static void TP2826_A1080P25_DataSet(unsigned char chip);
static void TP2826_A720P30_DataSet(unsigned char chip);
static void TP2826_A720P25_DataSet(unsigned char chip);
static void TP2826_C1080P30_DataSet(unsigned char chip);
static void TP2826_C1080P25_DataSet(unsigned char chip);
static void TP2826_C720P30_DataSet(unsigned char chip);
static void TP2826_C720P25_DataSet(unsigned char chip);
static void TP2826_C720P60_DataSet(unsigned char chip);
static void TP2826_C720P50_DataSet(unsigned char chip);
static void TP2826_A720P60_DataSet(unsigned char chip);
static void TP2853C_A720P60_DataSet(unsigned char chip);
static void TP2833_A720P60_DataSet(unsigned char chip);
static void TP28xx_reset_default(int chip, unsigned char ch);

static void TP2827C_C5MP20_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x13, 0x40);

    tp28xx_byte_write(chip, 0x15, 0x33);
    tp28xx_byte_write(chip, 0x16, 0x9c);

    tp28xx_byte_write(chip, 0x20, 0x50);
    tp28xx_byte_write(chip, 0x21, 0x86);
    tp28xx_byte_write(chip, 0x22, 0x38);

    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xda);

    tp28xx_byte_write(chip, 0x2d, 0x74);
    tp28xx_byte_write(chip, 0x2e, 0x50);

    tp28xx_byte_write(chip, 0x30, 0x75);
    tp28xx_byte_write(chip, 0x31, 0x39);
    tp28xx_byte_write(chip, 0x32, 0xc0);
    tp28xx_byte_write(chip, 0x33, 0x30);

    tp28xx_byte_write(chip, 0x39, 0x48);
}

static void TP2827C_C6MP20_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x13, 0x40);

    tp28xx_byte_write(chip, 0x20, 0x50);
    tp28xx_byte_write(chip, 0x21, 0x86);
    tp28xx_byte_write(chip, 0x22, 0x38);

    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xda);

    tp28xx_byte_write(chip, 0x2d, 0x74);
    tp28xx_byte_write(chip, 0x2e, 0x50);

    tp28xx_byte_write(chip, 0x30, 0x75);
    tp28xx_byte_write(chip, 0x31, 0x39);
    tp28xx_byte_write(chip, 0x32, 0xc0);
    tp28xx_byte_write(chip, 0x33, 0x30);

    tp28xx_byte_write(chip, 0x39, 0x48);
}

void TP2829_PLL_Reset( int chip)
{

    tp28xx_byte_write(chip, 0x40, 0x04);
    tp28xx_byte_write(chip, 0x3b, 0x20);
    tp28xx_byte_write(chip, 0x3d, 0xe0);
    tp28xx_byte_write(chip, 0x3d, 0x60);
    tp28xx_byte_write(chip, 0x3b, 0x25);
    tp28xx_byte_write(chip, 0x40, 0x40);
    tp28xx_byte_write(chip, 0x7a, 0x20);
    tp28xx_byte_write(chip, 0x3c, 0x20);
    tp28xx_byte_write(chip, 0x3c, 0x00);
    tp28xx_byte_write(chip, 0x7a, 0x25);
    tp28xx_byte_write(chip, 0x40, 0x00);

    if(DDR_2CH == output[chip] || DDR_4CH == output[chip] || DDR_1CH == output[chip])
        tp28xx_byte_write(chip, 0x44, 0x07);
    else
        tp28xx_byte_write(chip, 0x44, 0x17);

    tp28xx_byte_write(chip, 0x43, 0x12);
    tp28xx_byte_write(chip, 0x45, 0x09);
}

static void TP2829_Audio_DataSet(unsigned char chip)
{

    unsigned int bank;

    bank = tp28xx_byte_read(chip, 0x40);
    tp28xx_byte_write(chip, 0x40, 0x40);

    tp2833_audio_config_rmpos(chip, AUDIO_FORMAT , AUDIO_CHN);

    tp28xx_byte_write(chip, 0x17, 0x00|(DATA_BIT<<2));
    tp28xx_byte_write(chip, 0x1B, 0x01|(DATA_BIT<<6));

#if(AUDIO_CHN == 20)
    tp28xx_byte_write(chip, 0x18, 0x90|(SAMPLE_RATE));
#else
    tp28xx_byte_write(chip, 0x18, 0x80|(SAMPLE_RATE));
#endif

#if(AUDIO_CHN >= 8)
    tp28xx_byte_write(chip, 0x19, 0x1F);
#else
    tp28xx_byte_write(chip, 0x19, 0x0F);
#endif

    tp28xx_byte_write(chip, 0x1A, 0x15);

    tp28xx_byte_write(chip, 0x37, 0x20);
    tp28xx_byte_write(chip, 0x38, 0x38);
    tp28xx_byte_write(chip, 0x3E, 0x00);

    //tp28xx_byte_write(chip, 0x1D, 0x08);
    //mdelay(100);
    //tp28xx_byte_write(chip, 0x1D, 0x00);

    //tp28xx_byte_write(chip, 0x3C, 0x3f);
    //mdelay(200);
    //tp28xx_byte_write(chip, 0x3C, 0x00);

    tp28xx_byte_write(chip, 0x3d, 0x01);//audio reset

    tp28xx_byte_write(chip, 0x40, bank);

}
/*
void TP2829_StartTX(unsigned char chip, unsigned char ch)
{
	unsigned char tmp;
	int i;

	//tp28xx_byte_write(chip, 0xB6, (0x01<<(ch))); //clear flag

	tmp = tp28xx_byte_read(chip, 0x6f);
    tmp |= 0x01;
    tp28xx_byte_write(chip, 0x6f, tmp); //TX enable

    i = 100;
    while(i--)
    {
        if( 0x00 == tp28xx_byte_read(chip, 0x6d)) break;
        //udelay(1000);
        msleep(2);
    }

    tmp &= 0xfe;
    tp28xx_byte_write(chip, 0x6f, tmp); //TX disable
}
*/
static void TP2829_RX_init(unsigned char chip, unsigned char mod)
{

    int i, index=0;
    unsigned char regA7=0x00;
    unsigned char regA8=0x00;

    //regC9~D7
    static const unsigned char PTZ_RX_dat[][15]=
    {
        {0x00,0x00,0x07,0x08,0x00,0x00,0x04,0x00,0x00,0x60,0x10,0x06,0xbe,0x39,0x27}, //TVI command
        {0x00,0x00,0x07,0x08,0x09,0x0a,0x04,0x00,0x00,0x60,0x10,0x06,0xbe,0x39,0x27}, //TVI burst
        {0x00,0x00,0x06,0x07,0x08,0x09,0x05,0xbf,0x11,0x60,0x0b,0x04,0xf0,0xd8,0x2f}, //ACP1 0.525
        {0x00,0x00,0x06,0x07,0x08,0x09,0x02,0xdf,0x88,0x60,0x10,0x04,0xf0,0xd8,0x17}, //ACP2 0.6
        //{0x00,0x00,0x06,0x07,0x08,0x09,0x04,0xec,0xe9,0x60,0x10,0x04,0xf0,0xd8,0x17}, //ACP3 0.35
        {0x00,0x00,0x07,0x08,0x09,0x0a,0x09,0xd9,0xd3,0x60,0x08,0x04,0xf0,0xd8,0x2f}, //ACP3 0.35
        {0x00,0x00,0x06,0x07,0x08,0x09,0x03,0x52,0x53,0x60,0x10,0x04,0xf0,0xd8,0x17}  //ACP1 0.525
    };

        if(PTZ_RX_TVI_CMD == mod)
        {
            index = 0;
            regA7 = 0x03;
            regA8 = 0x00;
        }
        else if(PTZ_RX_TVI_BURST == mod)
        {
            index = 1;
            regA7 = 0x03;
            regA8 = 0x00;
        }
        else if(PTZ_RX_ACP1 == mod)
        {
            index = 2;
            regA7 = 0x03;
            regA8 = 0x00;
        }
        else if(PTZ_RX_ACP2 == mod)
        {
            index = 3;
            regA7 = 0x27;
            regA8 = 0x0f;
        }
        else if(PTZ_RX_ACP3 == mod)
        {
            index = 4;
            regA7 = 0x03;
            regA8 = 0x00;
        }
        else if(PTZ_RX_TEST == mod)
        {
            index = 5;
            regA7 = 0x03;
            regA8 = 0x00;
        }

        for(i = 0; i < 15; i++)
        {
            tp28xx_byte_write(chip, 0xc9+i, PTZ_RX_dat[index][i]);
            tp28xx_byte_write(chip, 0xa8, regA8);
            tp28xx_byte_write(chip, 0xa7, regA7);
        }

}
static void TP2829_PTZ_mode(unsigned char chip, unsigned char ch, unsigned char mod)
{
    unsigned int tmp, i, index=0;

    static const unsigned char PTZ_reg[13]=
    {
        0x6f,0x70,0x71,0x72,0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8
    };
    static const unsigned char PTZ_dat[][13]=
    {
        {0x42,0x40,0x20,0x00,0x00,0x00,0x0b,0x0c,0x0d,0x0e,0x19,0x78,0x21}, //TVI1.0
        {0x42,0x40,0x20,0x00,0x00,0x00,0x0b,0x0c,0x0d,0x0e,0x33,0xf0,0x21}, //TVI2.0
        {0x42,0x40,0x20,0x00,0x00,0x00,0x0e,0x0f,0x10,0x11,0x26,0xf0,0x57}, //A1080p for 2829 0.525
        {0x42,0x40,0x20,0x00,0x00,0x00,0x0e,0x0f,0x00,0x00,0x26,0xe0,0xef}, //A720p for 2829 0.525
        {0x42,0x40,0x20,0x00,0x00,0x00,0x0f,0x10,0x00,0x00,0x4a,0xf0,0x6f}, //960H for 2829
        {0x4a,0x5d,0xa0,0x00,0x00,0x00,0x10,0x11,0x12,0x13,0x15,0xb8,0x9f}, //HDC for 2829
        {0x42,0x40,0x20,0x00,0x00,0x00,0x0f,0x10,0x11,0x12,0x2c,0xf0,0x57}, //ACP 3M18 for 2829 8+0.6
        {0x42,0x40,0x20,0x00,0x00,0x00,0x0f,0x10,0x11,0x12,0x19,0xd0,0x17}, //ACP 3M2530 for 2829 4+0.35
        {0x46,0x5f,0x20,0x00,0x00,0x00,0x0f,0x10,0x12,0x16,0x16,0xd0,0x57}, //ACP 4M2530_5M20 for 2829 7+0.3
        {0x46,0x5f,0x20,0x00,0x00,0x00,0x0f,0x10,0x12,0x16,0x2c,0xf0,0x97}, //ACP 4M15 5M12.5 for 2829 8+0.6
        {0x4a,0x5d,0xa0,0x01,0x00,0x00,0x16,0x17,0x18,0x19,0x2a,0xf0,0x17}, //HDC QHD25/30 for 2829
        {0x4a,0x5f,0xa0,0x00,0x00,0x00,0x1a,0x1b,0x1c,0x1d,0x36,0x50,0x5f}, //HDC 4K12.5 for 2829
        {0x4a,0x5f,0xa0,0x01,0x00,0x00,0x1a,0x1b,0x1c,0x1d,0x36,0x50,0x57}, //HDC 4K15 for 2829
        {0x4a,0x5f,0xa0,0x01,0x00,0x00,0x1a,0x1b,0x1c,0x1d,0x36,0x50,0x57}  //HDC 6M20 for 2829
    };

    tp28xx_byte_write(chip, 0x40, ch); //reg bank1 switch for 2826

        if(PTZ_TVI == mod)
        {
            tmp = tp28xx_byte_read(chip, 0xf5); //check TVI 1 or 2
            if( (tmp >>ch) & 0x01)
            {
                index = 1;
            }
            else
            {
                index = 0;
            }
        }
        else if(PTZ_HDA_1080P == mod) //HDA 1080p
        {
                index = 2;
        }
        else if(PTZ_HDA_720P == mod) //HDA 720p
        {
                index = 3;
        }
        else if(PTZ_HDA_CVBS == mod) //HDA CVBS
        {
                index = 4;
        }
        else if(PTZ_HDC == mod) // test
        {
                index = 5;
        }
        else if(PTZ_HDA_3M18 == mod) // test
        {
                index = 6;
        }
        else if(PTZ_HDA_3M25 == mod) // test
        {
                index = 7;
        }
        else if(PTZ_HDA_4M25 == mod) // test
        {
                index = 8;
        }
        else if(PTZ_HDA_4M15 == mod) // test
        {
                index = 9;
        }
        else if(PTZ_HDC_QHD == mod) // test
        {
                index = 10;
        }
        else if(PTZ_HDC_8M12 == mod) // test
        {
                index = 11;
        }
        else if(PTZ_HDC_8M15 == mod) // test
        {
                index = 12;
        }
        else if(PTZ_HDC_6M20 == mod) // test
        {
                index = 13;
        }

     for(i = 0; i < 13; i++)
     {
         tp28xx_byte_write(chip, PTZ_reg[i], PTZ_dat[index][i]);
     }

    tmp = tp28xx_byte_read(chip, 0x71);
    tmp |= 0x20;
    tp28xx_byte_write(chip, 0x71, tmp);
}
static void TP2829_PTZ_mode_new(unsigned char chip, unsigned char ch, unsigned char mod)
{
    unsigned int tmp;

    TP2829_PTZ_mode(chip, ch, mod);

    //tmp = tp28xx_byte_read(chip, 0x6f);
    //tmp &= 0xbf;
    //tp28xx_byte_write(chip, 0x6f, tmp);
    tmp = tp28xx_byte_read(chip, 0x71);
    tmp |= 0x20;
    tp28xx_byte_write(chip, 0x71, tmp);

}
static void TP2829_output(unsigned char chip)
{
    unsigned int tmp;

    tp28xx_byte_write(chip, 0xF5, 0xf0);
    tp28xx_byte_write(chip, 0xF1, 0x00);
    tp28xx_byte_write(chip, 0x4D, 0x0f);
    tp28xx_byte_write(chip, 0x4E, 0x0f);
    tp28xx_byte_write(chip, 0x4f, 0x03);

    if( SDR_1CH == output[chip] )
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x88);
        tp28xx_byte_write(chip, 0xF4, 0x80); //output clock 148.5M
        tp28xx_byte_write(chip, 0xF6, 0x00);
        tp28xx_byte_write(chip, 0xF7, 0x11);
        tp28xx_byte_write(chip, 0xF8, 0x22);
        tp28xx_byte_write(chip, 0xF9, 0x33);
        tp28xx_byte_write(chip, 0x50, 0x00); //
        tp28xx_byte_write(chip, 0x51, 0x00); //
        tp28xx_byte_write(chip, 0x52, 0x00); //
        tp28xx_byte_write(chip, 0x53, 0x00); //
        tp28xx_byte_write(chip, 0xF3, 0x77);
        tp28xx_byte_write(chip, 0xF2, 0x77);
        if(TP2802_720P25V2 == mode || TP2802_720P30V2 == mode || TP2802_PAL == mode || TP2802_NTSC == mode )
        {
            tmp = tp28xx_byte_read(chip, 0xFA);
            tmp &= 0x88;
            tmp |= 0x11;
            tp28xx_byte_write(chip, 0xFA, tmp);
            tmp = tp28xx_byte_read(chip, 0xFB);
            tmp &= 0x88;
            tmp |= 0x11;
            tp28xx_byte_write(chip, 0xFB, tmp);
        }
        else if(FLAG_HALF_MODE == (mode & FLAG_HALF_MODE) )
        {
            tmp = tp28xx_byte_read(chip, 0xFA);
            tmp &= 0x88;
            tmp |= 0x43;
            tp28xx_byte_write(chip, 0xFA, tmp);
            tmp = tp28xx_byte_read(chip, 0xFB);
            tmp &= 0x88;
            tmp |= 0x65;
            tp28xx_byte_write(chip, 0xFB, tmp);
        }
    }
    else if(SDR_2CH == output[chip])
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x88);
        tp28xx_byte_write(chip, 0xF4, 0x80); //output clock 148.5M
        tp28xx_byte_write(chip, 0xF6, 0x10);
        tp28xx_byte_write(chip, 0xF7, 0x23);
        tp28xx_byte_write(chip, 0xF8, 0x10);
        tp28xx_byte_write(chip, 0xF9, 0x23);
        tp28xx_byte_write(chip, 0x50, 0x00); //
        tp28xx_byte_write(chip, 0x51, 0x00); //
        tp28xx_byte_write(chip, 0x52, 0x00); //
        tp28xx_byte_write(chip, 0x53, 0x00); //
        tp28xx_byte_write(chip, 0xF3, 0x77);
        tp28xx_byte_write(chip, 0xF2, 0x77);
    }
    else if(DDR_2CH == output[chip])
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x88);
        //tp28xx_byte_write(chip, 0x45, 0x54); //PLL 297M
        tp28xx_byte_write(chip, 0xF4, 0xa0); //output clock 148.5M
        tp28xx_byte_write(chip, 0xF6, 0x10); //
        tp28xx_byte_write(chip, 0xF7, 0x23); //
        tp28xx_byte_write(chip, 0xF8, 0x10); //
        tp28xx_byte_write(chip, 0xF9, 0x23); //
        tp28xx_byte_write(chip, 0x50, 0x00); //
        tp28xx_byte_write(chip, 0x51, 0x00); //
        tp28xx_byte_write(chip, 0x52, 0x00); //
        tp28xx_byte_write(chip, 0x53, 0x00); //
        tp28xx_byte_write(chip, 0xF3, 0x77);
        tp28xx_byte_write(chip, 0xF2, 0x77);

    }
    else if(DDR_4CH == output[chip])
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x88);
        //tp28xx_byte_write(chip, 0x45, 0x54); //PLL 297M
        tp28xx_byte_write(chip, 0xF4, 0xa0); //output clock 148.5M
        tp28xx_byte_write(chip, 0xF6, 0x10); //
        tp28xx_byte_write(chip, 0xF7, 0x10); //
        tp28xx_byte_write(chip, 0xF8, 0x10); //
        tp28xx_byte_write(chip, 0xF9, 0x10); //
        tp28xx_byte_write(chip, 0x50, 0xB2); //
        tp28xx_byte_write(chip, 0x51, 0xB2); //
        tp28xx_byte_write(chip, 0x52, 0xB2); //
        tp28xx_byte_write(chip, 0x53, 0xB2); //
        tp28xx_byte_write(chip, 0xF3, 0x77);
        tp28xx_byte_write(chip, 0xF2, 0x77);
    }
    else if( DDR_1CH == output[chip] )
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x88);
        //tp28xx_byte_write(chip, 0x45, 0x54); //PLL 297M
        tp28xx_byte_write(chip, 0xF4, 0xa0); //output clock 148.5M
        tp28xx_byte_write(chip, 0xF6, 0x04);
        tp28xx_byte_write(chip, 0xF7, 0x15);
        tp28xx_byte_write(chip, 0xF8, 0x26);
        tp28xx_byte_write(chip, 0xF9, 0x37);
        tp28xx_byte_write(chip, 0x50, 0x00); //
        tp28xx_byte_write(chip, 0x51, 0x00); //
        tp28xx_byte_write(chip, 0x52, 0x00); //
        tp28xx_byte_write(chip, 0x53, 0x00); //
        tp28xx_byte_write(chip, 0xF3, 0x77);
        tp28xx_byte_write(chip, 0xF2, 0x77);
    }
}
static void TP2828_output(unsigned char chip)
{
    unsigned int tmp;
    tp28xx_byte_write(chip, 0xF5, 0xf0);
    tp28xx_byte_write(chip, 0xF1, 0x14);
    tp28xx_byte_write(chip, 0x4D, 0x07);
    tp28xx_byte_write(chip, 0x4E, 0x05);
    tp28xx_byte_write(chip, 0x4f, 0x03);

    if( SDR_1CH == output[chip] )
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x88);
        //tp28xx_byte_write(chip, 0x45, 0x09); //PLL 148.5M
        tp28xx_byte_write(chip, 0xF4, 0x80); //output clock 148.5M
        tp28xx_byte_write(chip, 0xF6, 0x00);
        tp28xx_byte_write(chip, 0xF7, 0x11);
        tp28xx_byte_write(chip, 0xF8, 0x22);
        tp28xx_byte_write(chip, 0xF9, 0x33);
        tp28xx_byte_write(chip, 0x50, 0x00); //
        tp28xx_byte_write(chip, 0x51, 0x00); //
        tp28xx_byte_write(chip, 0x52, 0x00); //
        tp28xx_byte_write(chip, 0x53, 0x00); //
        tp28xx_byte_write(chip, 0xF3, 0x04); //0
        tp28xx_byte_write(chip, 0xF2, 0x04); //0
        if(TP2802_720P25V2 == mode || TP2802_720P30V2 == mode || TP2802_PAL == mode || TP2802_NTSC == mode )
        {
            tmp = tp28xx_byte_read(chip, 0xFA);
            tmp &= 0x88;
            tmp |= 0x11;
            tp28xx_byte_write(chip, 0xFA, tmp);
            tmp = tp28xx_byte_read(chip, 0xFB);
            tmp &= 0x88;
            tmp |= 0x11;
            tp28xx_byte_write(chip, 0xFB, tmp);
        }
        else if(FLAG_HALF_MODE == (mode & FLAG_HALF_MODE) )
        {
            tmp = tp28xx_byte_read(chip, 0xFA);
            tmp &= 0x88;
            tmp |= 0x43;
            tp28xx_byte_write(chip, 0xFA, tmp);
            tmp = tp28xx_byte_read(chip, 0xFB);
            tmp &= 0x88;
            tmp |= 0x65;
            tp28xx_byte_write(chip, 0xFB, tmp);
        }
    }
    else if(SDR_2CH == output[chip])
    {
        tp28xx_byte_write(chip, 0xFA, 0x88); //0x88 148.5M single, 0x99 74.25M double
        tp28xx_byte_write(chip, 0xFB, 0x88); //0x88 148.5M single, 0x99 74.25M double
        //tp28xx_byte_write(chip, 0x45, 0x09); //PLL 148.5M
        tp28xx_byte_write(chip, 0xF4, 0x80); //output clock 148.5M
        tp28xx_byte_write(chip, 0xF6, 0x10);
        tp28xx_byte_write(chip, 0xF7, 0x10);
        tp28xx_byte_write(chip, 0xF8, 0x23);
        tp28xx_byte_write(chip, 0xF9, 0x23);
        tp28xx_byte_write(chip, 0x50, 0x00); //
        tp28xx_byte_write(chip, 0x51, 0x00); //
        tp28xx_byte_write(chip, 0x52, 0x00); //
        tp28xx_byte_write(chip, 0x53, 0x00); //
        tp28xx_byte_write(chip, 0xF3, 0x00);
        tp28xx_byte_write(chip, 0xF2, 0x00);
    }
    else if(DDR_2CH == output[chip])
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x88);
        tp28xx_byte_write(chip, 0x45, 0x09); //PLL 297M
        tp28xx_byte_write(chip, 0xF4, 0xa0); //output clock 148.5M
        tp28xx_byte_write(chip, 0xF6, 0x10); //
        tp28xx_byte_write(chip, 0xF7, 0x10); //
        tp28xx_byte_write(chip, 0xF8, 0x23); //
        tp28xx_byte_write(chip, 0xF9, 0x23); //
        tp28xx_byte_write(chip, 0x50, 0x00); //
        tp28xx_byte_write(chip, 0x51, 0x00); //
        tp28xx_byte_write(chip, 0x52, 0x00); //
        tp28xx_byte_write(chip, 0x53, 0x00); //
        tp28xx_byte_write(chip, 0xF3, 0x00);
        tp28xx_byte_write(chip, 0xF2, 0x00);

    }
    else if(DDR_4CH == output[chip])
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x88);
        tp28xx_byte_write(chip, 0x45, 0x09); //PLL 297M
        tp28xx_byte_write(chip, 0xF4, 0xa0); //output clock 148.5M
        tp28xx_byte_write(chip, 0xF6, 0x10); //
        tp28xx_byte_write(chip, 0xF7, 0x10); //
        tp28xx_byte_write(chip, 0xF8, 0x10); //
        tp28xx_byte_write(chip, 0xF9, 0x10); //
        tp28xx_byte_write(chip, 0x50, 0xB2); //
        tp28xx_byte_write(chip, 0x51, 0xB2); //
        tp28xx_byte_write(chip, 0x52, 0xB2); //
        tp28xx_byte_write(chip, 0x53, 0xB2); //
        tp28xx_byte_write(chip, 0xF3, 0x04); //00
        tp28xx_byte_write(chip, 0xF2, 0x04); //00
    }
    else if( DDR_1CH == output[chip] )
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x88);
        tp28xx_byte_write(chip, 0x45, 0x09); //PLL 297M
        tp28xx_byte_write(chip, 0xF4, 0xa0); //output clock 148.5M
        tp28xx_byte_write(chip, 0xF6, 0x04);
        tp28xx_byte_write(chip, 0xF7, 0x15);
        tp28xx_byte_write(chip, 0xF8, 0x26);
        tp28xx_byte_write(chip, 0xF9, 0x37);
        tp28xx_byte_write(chip, 0x50, 0x00); //
        tp28xx_byte_write(chip, 0x51, 0x00); //
        tp28xx_byte_write(chip, 0x52, 0x00); //
        tp28xx_byte_write(chip, 0x53, 0x00); //
        tp28xx_byte_write(chip, 0xF3, 0x00);
        tp28xx_byte_write(chip, 0xF2, 0x00);
    }
}
static void TP2829_reset_default(unsigned char chip, unsigned char ch)
{
    unsigned int tmp;
    //tp28xx_byte_write(chip, 0x26, 0x04);
    tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x38, 0x00);
    tp28xx_byte_write(chip, 0x39, 0x1C);
    tp28xx_byte_write(chip, 0x3a, 0x32);
    tp28xx_byte_write(chip, 0x3B, 0x25);

    tmp = tp28xx_byte_read(chip, 0x26);
    tmp &= 0xfe;
    tp28xx_byte_write(chip, 0x26, tmp);

    tmp = tp28xx_byte_read(chip, 0x06);
    tmp &= 0xfb;
    tp28xx_byte_write(chip, 0x06, tmp);
}
//////////////////////////////////////////////////////////////
static void TP2829_NTSC_DataSet(unsigned char chip)
{
    unsigned char tmp;
    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x13);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x20, 0x40);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x70);
    tp28xx_byte_write(chip, 0x2c, 0x2a);
    tp28xx_byte_write(chip, 0x2d, 0x68);
    tp28xx_byte_write(chip, 0x2e, 0x57);

    tp28xx_byte_write(chip, 0x30, 0x62);
    tp28xx_byte_write(chip, 0x31, 0xbb);
    tp28xx_byte_write(chip, 0x32, 0x96);
    tp28xx_byte_write(chip, 0x33, 0xc0);
    //tp28xx_byte_write(chip, 0x35, 0x25);
    tp28xx_byte_write(chip, 0x38, 0x00);
    tp28xx_byte_write(chip, 0x39, 0x04);
    tp28xx_byte_write(chip, 0x3a, 0x32);
    tp28xx_byte_write(chip, 0x3B, 0x25);

    tp28xx_byte_write(chip, 0x18, 0x12);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
}
static void TP2829_PAL_DataSet(unsigned char chip)
{
    unsigned char tmp;
    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x13);
    tp28xx_byte_write(chip, 0x0d, 0x51);

    tp28xx_byte_write(chip, 0x20, 0x48);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x22, 0x37);
    tp28xx_byte_write(chip, 0x23, 0x3f);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x70);
    tp28xx_byte_write(chip, 0x2c, 0x2a);
    tp28xx_byte_write(chip, 0x2d, 0x64);
    tp28xx_byte_write(chip, 0x2e, 0x56);

    tp28xx_byte_write(chip, 0x30, 0x7a);
    tp28xx_byte_write(chip, 0x31, 0x4a);
    tp28xx_byte_write(chip, 0x32, 0x4d);
    tp28xx_byte_write(chip, 0x33, 0xf0);
    //tp28xx_byte_write(chip, 0x35, 0x25);
    tp28xx_byte_write(chip, 0x38, 0x00);
    tp28xx_byte_write(chip, 0x39, 0x04);
    tp28xx_byte_write(chip, 0x3a, 0x32);
    tp28xx_byte_write(chip, 0x3B, 0x25);

    tp28xx_byte_write(chip, 0x18, 0x17);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
}
static void TP2829_V1_DataSet(unsigned char chip)
{
    unsigned char tmp;
    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x20, 0x30);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x30);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0xbb);
    tp28xx_byte_write(chip, 0x32, 0x2e);
    tp28xx_byte_write(chip, 0x33, 0x90);
    //tp28xx_byte_write(chip, 0x35, 0x05);
    tp28xx_byte_write(chip, 0x38, 0x00);
    tp28xx_byte_write(chip, 0x39, 0x1c);
    tp28xx_byte_write(chip, 0x3a, 0x32);
    tp28xx_byte_write(chip, 0x3B, 0x25);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
}
static void TP2829_V2_DataSet(unsigned char chip)
{
    unsigned char tmp;
    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x13);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x20, 0x30);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x30);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0xbb);
    tp28xx_byte_write(chip, 0x32, 0x2e);
    tp28xx_byte_write(chip, 0x33, 0x90);
    //tp28xx_byte_write(chip, 0x35, 0x25);
    tp28xx_byte_write(chip, 0x38, 0x00);
    tp28xx_byte_write(chip, 0x39, 0x18);
    tp28xx_byte_write(chip, 0x3a, 0x32);
    tp28xx_byte_write(chip, 0x3B, 0x25);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
}

/////HDA QHD30
static void TP2829_AQHDP30_DataSet(unsigned char chip)
{
    unsigned char tmp;
#if 0
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x40;
    tp28xx_byte_write(chip, 0x14, tmp);
#else
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
    tmp = tp28xx_byte_read(chip, 0x1c);
    tmp |= 0x80;
    tp28xx_byte_write(chip, 0x1c, tmp);
    tp28xx_byte_write(chip, 0x0d, 0x70);
#endif
    tp28xx_byte_write(chip, 0x13, 0x00);
    tp28xx_byte_write(chip, 0x15, 0x23);
    tp28xx_byte_write(chip, 0x16, 0x16);
    tp28xx_byte_write(chip, 0x18, 0x32);

    tp28xx_byte_write(chip, 0x20, 0x80);
    tp28xx_byte_write(chip, 0x21, 0x86);
    tp28xx_byte_write(chip, 0x22, 0x36);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xad);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2d, 0xa0);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0x6a);
    tp28xx_byte_write(chip, 0x32, 0xbe);
    tp28xx_byte_write(chip, 0x33, 0x80);
    //tp28xx_byte_write(chip, 0x35, 0x15);
    tp28xx_byte_write(chip, 0x39, 0x40);
}

/////HDA QHD25
static void TP2829_AQHDP25_DataSet(unsigned char chip)
{
    unsigned char tmp;
#if 0
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x40;
    tp28xx_byte_write(chip, 0x14, tmp);
#else
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
    tmp = tp28xx_byte_read(chip, 0x1c);
    tmp |= 0x80;
    tp28xx_byte_write(chip, 0x1c, tmp);
    tp28xx_byte_write(chip, 0x0d, 0x70);
#endif

    tp28xx_byte_write(chip, 0x13, 0x00);
    tp28xx_byte_write(chip, 0x15, 0x23);
    tp28xx_byte_write(chip, 0x16, 0x16);
    tp28xx_byte_write(chip, 0x18, 0x32);

    tp28xx_byte_write(chip, 0x20, 0x80);
    tp28xx_byte_write(chip, 0x21, 0x86);
    tp28xx_byte_write(chip, 0x22, 0x36);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xad);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2d, 0xa0);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0x6f);
    tp28xx_byte_write(chip, 0x32, 0xb5);
    tp28xx_byte_write(chip, 0x33, 0x80);
    //tp28xx_byte_write(chip, 0x35, 0x15);
    tp28xx_byte_write(chip, 0x39, 0x40);
}

/////HDA QXGA30
static void TP2829_AQXGAP30_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x40;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x13, 0x00);

    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x20, 0x90);
    tp28xx_byte_write(chip, 0x21, 0x86);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x01);
    tp28xx_byte_write(chip, 0x27, 0xad);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0xa0);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0x68);
    tp28xx_byte_write(chip, 0x32, 0xbe);
    tp28xx_byte_write(chip, 0x33, 0x80);

    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x40);
    tp28xx_byte_write(chip, 0x3a, 0x12);
    //tp28xx_byte_write(chip, 0x3b, 0x26);
}

/////HDA QXGA25
static void TP2829_AQXGAP25_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x60;
    tp28xx_byte_write(chip, 0x14, tmp);
    tp28xx_byte_write(chip, 0x13, 0x00);

    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x20, 0x90);
    tp28xx_byte_write(chip, 0x21, 0x86);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x01);
    tp28xx_byte_write(chip, 0x27, 0xad);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0xa0);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0x6c);
    tp28xx_byte_write(chip, 0x32, 0xbe);
    tp28xx_byte_write(chip, 0x33, 0x80);

    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x40);
    tp28xx_byte_write(chip, 0x3a, 0x12);
    //tp28xx_byte_write(chip, 0x3b, 0x26);
}


/////HDC QHD30
static void TP2829_CQHDP30_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x13, 0x40);
    tp28xx_byte_write(chip, 0x15, 0x13);
    tp28xx_byte_write(chip, 0x16, 0xfa);
    tp28xx_byte_write(chip, 0x18, 0x38);
    tp28xx_byte_write(chip, 0x1c, 0x0c);
    tp28xx_byte_write(chip, 0x1d, 0x80);

    tp28xx_byte_write(chip, 0x20, 0x50);
    tp28xx_byte_write(chip, 0x21, 0x86);
    tp28xx_byte_write(chip, 0x22, 0x38);

    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xda);

    tp28xx_byte_write(chip, 0x2d, 0x6c);
    tp28xx_byte_write(chip, 0x2e, 0x50);

    tp28xx_byte_write(chip, 0x30, 0x75);
    tp28xx_byte_write(chip, 0x31, 0x39);
    tp28xx_byte_write(chip, 0x32, 0xc0);
    tp28xx_byte_write(chip, 0x33, 0x31);

    tp28xx_byte_write(chip, 0x39, 0x48);
}

/////HDC QHD25
static void TP2829_CQHDP25_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x13, 0x40);
    tp28xx_byte_write(chip, 0x15, 0x13);
    tp28xx_byte_write(chip, 0x16, 0xd8);
    tp28xx_byte_write(chip, 0x18, 0x30);
    tp28xx_byte_write(chip, 0x1c, 0x0c);
    tp28xx_byte_write(chip, 0x1d, 0x80);

    tp28xx_byte_write(chip, 0x20, 0x50);
    tp28xx_byte_write(chip, 0x21, 0x86);
    tp28xx_byte_write(chip, 0x22, 0x38);

    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xda);

    tp28xx_byte_write(chip, 0x2d, 0x6c);
    tp28xx_byte_write(chip, 0x2e, 0x50);

    tp28xx_byte_write(chip, 0x30, 0x75);
    tp28xx_byte_write(chip, 0x31, 0x39);
    tp28xx_byte_write(chip, 0x32, 0xc0);
    tp28xx_byte_write(chip, 0x33, 0x3b);

    tp28xx_byte_write(chip, 0x39, 0x48);
}

///////HDA QHD15
static void TP2829_AQHDP15_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x40;
    tp28xx_byte_write(chip, 0x14, tmp);
    tp28xx_byte_write(chip, 0x13, 0x00);

    tp28xx_byte_write(chip, 0x20, 0x38);
    tp28xx_byte_write(chip, 0x21, 0x46);

    tp28xx_byte_write(chip, 0x25, 0xfe);
    tp28xx_byte_write(chip, 0x26, 0x01);

    tp28xx_byte_write(chip, 0x2d, 0x44);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x29);
    tp28xx_byte_write(chip, 0x31, 0x68);
    tp28xx_byte_write(chip, 0x32, 0x78);
    tp28xx_byte_write(chip, 0x33, 0x10);

    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x40);
    tp28xx_byte_write(chip, 0x3a, 0x12);
}

/////HDA QXGA18
static void TP2829_AQXGAP18_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x40;
    tp28xx_byte_write(chip, 0x14, tmp);
    tp28xx_byte_write(chip, 0x13, 0x00);

    tp28xx_byte_write(chip, 0x15, 0x13);
    tp28xx_byte_write(chip, 0x16, 0x10);
    tp28xx_byte_write(chip, 0x18, 0x68);

    tp28xx_byte_write(chip, 0x20, 0x48);
    tp28xx_byte_write(chip, 0x21, 0x46);
    tp28xx_byte_write(chip, 0x25, 0xfe);
    tp28xx_byte_write(chip, 0x26, 0x05);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2d, 0x52);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x29);
    tp28xx_byte_write(chip, 0x31, 0x65);
    tp28xx_byte_write(chip, 0x32, 0x2b);
    tp28xx_byte_write(chip, 0x33, 0xd0);

    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x40);
    tp28xx_byte_write(chip, 0x3a, 0x12);
}

/////TVI QHD30/QHD25
static void TP2829_QHDP30_25_DataSet(unsigned char chip)
{
    unsigned char tmp;

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);

    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x15, 0x23);
    tp28xx_byte_write(chip, 0x16, 0x1b);
    tp28xx_byte_write(chip, 0x18, 0x38);

    tp28xx_byte_write(chip, 0x20, 0x50);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xad);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x58);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x74);
    tp28xx_byte_write(chip, 0x31, 0x58);
    tp28xx_byte_write(chip, 0x32, 0x9f);
    tp28xx_byte_write(chip, 0x33, 0x60);

    //tp28xx_byte_write(chip, 0x35, 0x05);
    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x48);
    tp28xx_byte_write(chip, 0x3a, 0x12);
    //tp28xx_byte_write(chip, 0x3b, 0x26);
}

/////TVI 5M20
static void TP2829_5MP20_DataSet(unsigned char chip)
{
    unsigned char tmp;
    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x20, 0x50);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xad);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x54);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x74);
    tp28xx_byte_write(chip, 0x31, 0xa7);
    tp28xx_byte_write(chip, 0x32, 0x18);
    tp28xx_byte_write(chip, 0x33, 0x50);

    //tp28xx_byte_write(chip, 0x35, 0x05);
    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x48);
    tp28xx_byte_write(chip, 0x3a, 0x12);
    //tp28xx_byte_write(chip, 0x3b, 0x26);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
}

/////HDA 5M20
static void TP2829_A5MP20_DataSet(unsigned char chip)
{
    unsigned char tmp;
#if 0
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x40;
    tp28xx_byte_write(chip, 0x14, tmp);
#else
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
    tmp = tp28xx_byte_read(chip, 0x1c);
    tmp |= 0x80;
    tp28xx_byte_write(chip, 0x1c, tmp);
    tp28xx_byte_write(chip, 0x0d, 0x70);
#endif
    tp28xx_byte_write(chip, 0x20, 0x80);
    tp28xx_byte_write(chip, 0x21, 0x86);
    tp28xx_byte_write(chip, 0x22, 0x36);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xad);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2d, 0xA0);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0x77);
    tp28xx_byte_write(chip, 0x32, 0x0e);
    tp28xx_byte_write(chip, 0x33, 0xa0);
    tp28xx_byte_write(chip, 0x39, 0x48);
}

/////TVI 8M15
static void TP2829_8MP15_DataSet(unsigned char chip)
{
    unsigned char tmp;
    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x20, 0x60);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xad);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x58);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x74);
    tp28xx_byte_write(chip, 0x31, 0x59);
    tp28xx_byte_write(chip, 0x32, 0xbd);
    tp28xx_byte_write(chip, 0x33, 0x60);

    //tp28xx_byte_write(chip, 0x35, 0x05);
    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x48);
    tp28xx_byte_write(chip, 0x3a, 0x12);
    //tp28xx_byte_write(chip, 0x3b, 0x26);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
}
///////HDA 5M12.5
static void TP2829_A5MP12_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x40;
    tp28xx_byte_write(chip, 0x14, tmp);
    tp28xx_byte_write(chip, 0x13, 0x00);

    tp28xx_byte_write(chip, 0x20, 0x38);
    tp28xx_byte_write(chip, 0x21, 0x46);

    tp28xx_byte_write(chip, 0x25, 0xfe);
    tp28xx_byte_write(chip, 0x26, 0x01);

    tp28xx_byte_write(chip, 0x2d, 0x44);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x29);
    tp28xx_byte_write(chip, 0x31, 0x68);
    tp28xx_byte_write(chip, 0x32, 0x72);
    tp28xx_byte_write(chip, 0x33, 0xb0);

    tp28xx_byte_write(chip, 0x16, 0x10);
    tp28xx_byte_write(chip, 0x18, 0x1a);
    tp28xx_byte_write(chip, 0x1d, 0xb8);
    tp28xx_byte_write(chip, 0x36, 0xbc);

    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x40);
    tp28xx_byte_write(chip, 0x3a, 0x12);
}
///////////////////////////////////////////////////////////////////
static void TP2829_A720P30_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x02);
    tmp |= 0x04;
    tp28xx_byte_write(chip, 0x02, tmp);

    tp28xx_byte_write(chip, 0x0d, 0x70);

    tp28xx_byte_write(chip, 0x20, 0x40);
    tp28xx_byte_write(chip, 0x21, 0x46);

    tp28xx_byte_write(chip, 0x25, 0xfe);
    tp28xx_byte_write(chip, 0x26, 0x01);

    tp28xx_byte_write(chip, 0x2c, 0x3a);
    tp28xx_byte_write(chip, 0x2d, 0x5a);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x9d);
    tp28xx_byte_write(chip, 0x31, 0xca);
    tp28xx_byte_write(chip, 0x32, 0x01);
    tp28xx_byte_write(chip, 0x33, 0xd0);
}
static void TP2829_A720P25_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x02);
    tmp |= 0x04;
    tp28xx_byte_write(chip, 0x02, tmp);

    tp28xx_byte_write(chip, 0x0d, 0x71);

    tp28xx_byte_write(chip, 0x20, 0x40);
    tp28xx_byte_write(chip, 0x21, 0x46);

    tp28xx_byte_write(chip, 0x25, 0xfe);
    tp28xx_byte_write(chip, 0x26, 0x01);

    tp28xx_byte_write(chip, 0x2c, 0x3a);
    tp28xx_byte_write(chip, 0x2d, 0x5a);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x9e);
    tp28xx_byte_write(chip, 0x31, 0x20);
    tp28xx_byte_write(chip, 0x32, 0x10);
    tp28xx_byte_write(chip, 0x33, 0x90);
}
static void TP2829_A1080P30_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x02);
    tmp |= 0x04;
    tp28xx_byte_write(chip, 0x02, tmp);

    tp28xx_byte_write(chip, 0x15, 0x01);
    tp28xx_byte_write(chip, 0x16, 0xf0);

    tp28xx_byte_write(chip, 0x0d, 0x72);

    tp28xx_byte_write(chip, 0x20, 0x38);
    tp28xx_byte_write(chip, 0x21, 0x46);

    tp28xx_byte_write(chip, 0x25, 0xfe);
    tp28xx_byte_write(chip, 0x26, 0x0d);

    tp28xx_byte_write(chip, 0x2c, 0x3a);
    tp28xx_byte_write(chip, 0x2d, 0x54);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0xa5);
    tp28xx_byte_write(chip, 0x31, 0x95);
    tp28xx_byte_write(chip, 0x32, 0xe0);
    tp28xx_byte_write(chip, 0x33, 0x60);
}
static void TP2829_A1080P25_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x02);
    tmp |= 0x04;
    tp28xx_byte_write(chip, 0x02, tmp);

    tp28xx_byte_write(chip, 0x15, 0x01);
    tp28xx_byte_write(chip, 0x16, 0xf0);

    tp28xx_byte_write(chip, 0x0d, 0x73);

    tp28xx_byte_write(chip, 0x20, 0x3c);
    tp28xx_byte_write(chip, 0x21, 0x46);

    tp28xx_byte_write(chip, 0x25, 0xfe);
    tp28xx_byte_write(chip, 0x26, 0x0d);

    tp28xx_byte_write(chip, 0x2c, 0x3a);
    tp28xx_byte_write(chip, 0x2d, 0x54);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0xa5);
    tp28xx_byte_write(chip, 0x31, 0x86);
    tp28xx_byte_write(chip, 0x32, 0xfb);
    tp28xx_byte_write(chip, 0x33, 0x60);
}
static void TP2829_1080P60_DataSet(unsigned char chip)
{
    unsigned char tmp;

    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x15, 0x03);
    tp28xx_byte_write(chip, 0x16, 0xe2);
    tp28xx_byte_write(chip, 0x17, 0x80);
    tp28xx_byte_write(chip, 0x18, 0x27);
    tp28xx_byte_write(chip, 0x19, 0x38);
    tp28xx_byte_write(chip, 0x1a, 0x47);
    tp28xx_byte_write(chip, 0x1c, 0x08);
    tp28xx_byte_write(chip, 0x1d, 0x96);

    tp28xx_byte_write(chip, 0x20, 0x50);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xad);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x40);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x74);
    tp28xx_byte_write(chip, 0x31, 0x9b);
    tp28xx_byte_write(chip, 0x32, 0xa5);
    tp28xx_byte_write(chip, 0x33, 0xe0);

    //tp28xx_byte_write(chip, 0x35, 0x05);
    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x48);
    tp28xx_byte_write(chip, 0x3a, 0x12);
    //tp28xx_byte_write(chip, 0x3b, 0x26);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
}
//HDC 8M15
static void TP2829_C8MP15_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x13, 0x40);

    tp28xx_byte_write(chip, 0x20, 0x80);
    tp28xx_byte_write(chip, 0x21, 0x86);
    tp28xx_byte_write(chip, 0x22, 0x38);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xda);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x90);
    tp28xx_byte_write(chip, 0x2e, 0x50);

    tp28xx_byte_write(chip, 0x30, 0x75);
    tp28xx_byte_write(chip, 0x31, 0x39);
    tp28xx_byte_write(chip, 0x32, 0xc0);
    tp28xx_byte_write(chip, 0x33, 0x32);

    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x68);
    tp28xx_byte_write(chip, 0x3a, 0x12);
}

/////HDC 8M12
static void TP2829_C8MP12_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x13, 0x40);
    tp28xx_byte_write(chip, 0x15, 0x23);
    tp28xx_byte_write(chip, 0x16, 0xf8);
    tp28xx_byte_write(chip, 0x18, 0x50);

    tp28xx_byte_write(chip, 0x20, 0x80);
    tp28xx_byte_write(chip, 0x21, 0x86);
    tp28xx_byte_write(chip, 0x22, 0x38);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xda);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x9c);
    tp28xx_byte_write(chip, 0x2e, 0x50);

    tp28xx_byte_write(chip, 0x30, 0x75);
    tp28xx_byte_write(chip, 0x31, 0x39);
    tp28xx_byte_write(chip, 0x32, 0xc0);
    tp28xx_byte_write(chip, 0x33, 0x32);

    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x68);
    tp28xx_byte_write(chip, 0x3a, 0x12);
    //tp28xx_byte_write(chip, 0x3b, 0x26);
}
/////HDA 8M15
static void TP2829_A8MP15_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x40;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tp28xx_byte_write(chip, 0x15, 0x13);
    tp28xx_byte_write(chip, 0x16, 0x74);
    //tp28xx_byte_write(chip, 0x18, 0x32);

    tp28xx_byte_write(chip, 0x20, 0x80);
    tp28xx_byte_write(chip, 0x21, 0x86);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xad);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x58);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0x68);
    tp28xx_byte_write(chip, 0x32, 0x43);
    tp28xx_byte_write(chip, 0x33, 0x00);

    //tp28xx_byte_write(chip, 0x35, 0x15);
    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x68);
    tp28xx_byte_write(chip, 0x3a, 0x12);
}
static void TP2829_1080P50_DataSet(unsigned char chip)
{
    unsigned char tmp;

    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x15, 0x03);
    tp28xx_byte_write(chip, 0x16, 0xe2);
    tp28xx_byte_write(chip, 0x17, 0x80);
    tp28xx_byte_write(chip, 0x18, 0x27);
    tp28xx_byte_write(chip, 0x19, 0x38);
    tp28xx_byte_write(chip, 0x1a, 0x47);
    tp28xx_byte_write(chip, 0x1c, 0x0a);
    tp28xx_byte_write(chip, 0x1d, 0x50);

    tp28xx_byte_write(chip, 0x20, 0x50);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xad);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x40);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x74);
    tp28xx_byte_write(chip, 0x31, 0x9b);
    tp28xx_byte_write(chip, 0x32, 0xa5);
    tp28xx_byte_write(chip, 0x33, 0xe0);

    //tp28xx_byte_write(chip, 0x35, 0x05);
    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x48);
    tp28xx_byte_write(chip, 0x3a, 0x12);
    //tp28xx_byte_write(chip, 0x3b, 0x26);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
}



void TP2831_PLL_Reset( int chip)
{
#if 0 // These registers may not be needed. But thermal testing on -40C degree is needed before removing these registers.
    tp28xx_byte_write(chip, 0x40, 0x04);
    tp28xx_byte_write(chip, 0x3b, 0x20);
    tp28xx_byte_write(chip, 0x3d, 0xe0);
    tp28xx_byte_write(chip, 0x3d, 0x60);
    tp28xx_byte_write(chip, 0x3b, 0x25);
    tp28xx_byte_write(chip, 0x40, 0x40);
    tp28xx_byte_write(chip, 0x7a, 0x20);
    tp28xx_byte_write(chip, 0x3c, 0x20);
    tp28xx_byte_write(chip, 0x3c, 0x00);
    tp28xx_byte_write(chip, 0x7a, 0x25);
    tp28xx_byte_write(chip, 0x40, 0x00);
#endif

    //tp28xx_byte_write(chip, 0x41, 0x00);
    tp28xx_byte_write(chip, 0x42, 0x80);

    if(DDR_2CH == output[chip] || DDR_4CH == output[chip] || DDR_1CH == output[chip])
        tp28xx_byte_write(chip, 0x44, 0x07);
    else
        tp28xx_byte_write(chip, 0x44, 0x17);

    tp28xx_byte_write(chip, 0x43, 0x12);
    tp28xx_byte_write(chip, 0x45, 0x09);
}

static void TP2831_Audio_DataSet(unsigned char chip)
{

    unsigned int bank;

    bank = tp28xx_byte_read(chip, 0x40);
    tp28xx_byte_write(chip, 0x40, 0x40);

    tp2833_audio_config_rmpos(chip, AUDIO_FORMAT , AUDIO_CHN);

    tp28xx_byte_write(chip, 0x17, 0x00|(DATA_BIT<<2));
    tp28xx_byte_write(chip, 0x1B, 0x01|(DATA_BIT<<6));

#if(AUDIO_CHN == 20)
    tp28xx_byte_write(chip, 0x18, 0xd0|(SAMPLE_RATE));
#else
    tp28xx_byte_write(chip, 0x18, 0xc0|(SAMPLE_RATE));
#endif

#if(AUDIO_CHN >= 8)
    tp28xx_byte_write(chip, 0x19, 0x1F);
#else
    tp28xx_byte_write(chip, 0x19, 0x0F);
#endif

    tp28xx_byte_write(chip, 0x1A, 0x15);

    tp28xx_byte_write(chip, 0x37, 0x20);
    tp28xx_byte_write(chip, 0x38, 0x38);
    tp28xx_byte_write(chip, 0x3E, 0x00);
    tp28xx_byte_write(chip, 0x7a, 0x25);

    //tp28xx_byte_write(chip, 0x1D, 0x08);
    //msleep(100);
    //tp28xx_byte_write(chip, 0x1D, 0x00);

    //tp28xx_byte_write(chip, 0x3C, 0x3f);
    //msleep(200);
    //tp28xx_byte_write(chip, 0x3C, 0x00);

    tp28xx_byte_write(chip, 0x3d, 0x01);//audio reset

    //tp28xx_byte_write(chip, 0x18, 0x90);//audio
    //tp28xx_byte_write(chip, 0x1a, 0x1b);//

    tp28xx_byte_write(chip, 0x40, bank);

}

static void TP2831_RX_init(unsigned char chip, unsigned char mod)
{

    int i, index=0;
    unsigned char regA7=0x00;
    unsigned char regA8=0x00;

    //regC9~D7
    static const unsigned char PTZ_RX_dat[][15]=
    {
        {0x00,0x00,0x07,0x08,0x00,0x00,0x04,0x00,0x00,0x60,0x10,0x06,0xbe,0x39,0x27}, //TVI command
        {0x00,0x00,0x07,0x08,0x09,0x0a,0x04,0x00,0x00,0x60,0x10,0x06,0xbe,0x39,0x27}, //TVI burst
        {0x00,0x00,0x06,0x07,0x08,0x09,0x05,0xbf,0x11,0x60,0x0d,0x04,0xf0,0xd8,0x2f}, //ACP1 0.525
        {0x00,0x00,0x06,0x07,0x08,0x09,0x03,0x48,0x9b,0x60,0x18,0x04,0xf0,0xd8,0x2f}, //ACP2 0.6
        //{0x00,0x00,0x06,0x07,0x08,0x09,0x04,0xec,0xe9,0x60,0x10,0x04,0xf0,0xd8,0x17}, //ACP3 0.35
        //{0x00,0x00,0x07,0x08,0x09,0x0a,0x09,0xd9,0xd3,0x60,0x08,0x04,0xf0,0xd8,0x2f}, //ACP3 0.3
        {0x00,0x00,0x07,0x08,0x09,0x0a,0x04,0xc9,0xe3,0x60,0x0e,0x04,0xf0,0xd8,0x2f}, //ACP3 0.3
        {0x00,0x00,0x06,0x07,0x08,0x09,0x03,0x52,0x53,0x60,0x10,0x04,0xf0,0xd8,0x17}  //ACP1 0.525
    };

        if(PTZ_RX_TVI_CMD == mod)
        {
            index = 0;
            regA7 = 0x03;
            regA8 = 0x00;
        }
        else if(PTZ_RX_TVI_BURST == mod)
        {
            index = 1;
            regA7 = 0x03;
            regA8 = 0x00;
        }
        else if(PTZ_RX_ACP1 == mod)
        {
            index = 2;
            regA7 = 0x03;
            regA8 = 0x00;
        }
        else if(PTZ_RX_ACP2 == mod)
        {
            index = 3;
            regA7 = 0x03;
            regA8 = 0x00;
        }
        else if(PTZ_RX_ACP3 == mod)
        {
            index = 4;
            regA7 = 0x03;
            regA8 = 0x00;
        }
        else if(PTZ_RX_TEST == mod)
        {
            index = 5;
            regA7 = 0x03;
            regA8 = 0x00;
        }

        for(i = 0; i < 15; i++)
        {
            tp28xx_byte_write(chip, 0xc9+i, PTZ_RX_dat[index][i]);
            tp28xx_byte_write(chip, 0xa8, regA8);
            tp28xx_byte_write(chip, 0xa7, regA7);
        }

}

static void TP2831_PTZ_mode(unsigned char chip, unsigned char ch, unsigned char mod)
{
    unsigned int tmp, i, index=0;

    static const unsigned char PTZ_reg[13]=
    {
        0x6f,0x70,0x71,0x72,0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8
    };
    static const unsigned char PTZ_dat[][13]=
    {
        {0x42,0x40,0x00,0x00,0x00,0x00,0x0b,0x0c,0x0d,0x0e,0x19,0x78,0x21}, //TVI1.0
        {0x42,0x40,0x00,0x00,0x00,0x00,0x0b,0x0c,0x0d,0x0e,0x33,0xdf,0x21}, //TVI2.0
        {0x42,0x40,0x00,0x00,0x00,0x00,0x0e,0x0f,0x10,0x11,0x26,0xf0,0x57}, //A1080p for 2829 0.525
        {0x42,0x40,0x00,0x00,0x00,0x00,0x0e,0x0f,0x00,0x00,0x26,0xe0,0xef}, //A720p for 2829 0.525
        {0x42,0x40,0x00,0x00,0x00,0x00,0x0f,0x10,0x00,0x00,0x4a,0xf0,0x6f}, //960H for 2829
        {0x4a,0x5d,0x80,0x00,0x00,0x00,0x10,0x11,0x12,0x13,0x15,0xb8,0x9f}, //HDC for 2829
        {0x42,0x40,0x00,0x00,0x00,0x00,0x0f,0x10,0x11,0x12,0x2c,0xf0,0x57}, //ACP 3M18 for 2829 8+0.6
        {0x42,0x40,0x00,0x00,0x00,0x00,0x0f,0x10,0x11,0x12,0x19,0xd0,0x17}, //ACP 3M2530 for 2829 4+0.35
        {0x46,0x5f,0x00,0x00,0x00,0x00,0x0f,0x10,0x12,0x16,0x19,0xd0,0x17}, //ACP 4M2530_5M20 for 2829 7+0.3
        {0x46,0x5f,0x00,0x00,0x00,0x00,0x0f,0x10,0x12,0x16,0x2d,0x50,0x97}, //ACP 4M15 5M12.5 for 2829 8+0.6
        {0x4a,0x5d,0x80,0x01,0x00,0x00,0x16,0x17,0x18,0x19,0x2a,0xf0,0x17}, //HDC QHD25/30 for 2829
        {0x4a,0x5f,0x80,0x00,0x00,0x00,0x1a,0x1b,0x1c,0x1d,0x36,0x50,0x5f}, //HDC 4K12.5 for 2829
        {0x4a,0x5f,0x80,0x01,0x00,0x00,0x1a,0x1b,0x1c,0x1d,0x36,0x50,0x57}, //HDC 4K15 for 2829
        {0x4a,0x5f,0x80,0x01,0x00,0x00,0x1a,0x1b,0x1c,0x1d,0x2c,0x50,0x57}  //HDC 6M20 for 2829
    };
    tp28xx_byte_write(chip, 0x40, ch); //reg bank1 switch for 2826

        if(PTZ_TVI == mod)
        {
            tmp = tp28xx_byte_read(chip, 0xf5); //check TVI 1 or 2
            if( (tmp >>ch) & 0x01)
            {
                index = 1;
            }
            else
            {
                index = 0;
            }
        }
        else if(PTZ_HDA_1080P == mod) //HDA 1080p
        {
                index = 2;
        }
        else if(PTZ_HDA_720P == mod) //HDA 720p
        {
                index = 3;
        }
        else if(PTZ_HDA_CVBS == mod) //HDA CVBS
        {
                index = 4;
        }
        else if(PTZ_HDC == mod) // test
        {
                index = 5;
        }
        else if(PTZ_HDA_3M18 == mod) // test
        {
                index = 6;
        }
        else if(PTZ_HDA_3M25 == mod) // test
        {
                index = 7;
        }
        else if(PTZ_HDA_4M25 == mod) // test
        {
                index = 8;
        }
        else if(PTZ_HDA_4M15 == mod) // test
        {
                index = 9;
        }
        else if(PTZ_HDC_QHD == mod) // test
        {
                index = 10;
        }
        else if(PTZ_HDC_8M12 == mod) // test
        {
                index = 11;
        }
        else if(PTZ_HDC_8M15 == mod) // test
        {
                index = 12;
        }
        else if(PTZ_HDC_6M20 == mod) // test
        {
                index = 13;
        }


     for(i = 0; i < 13; i++)
     {
         tp28xx_byte_write(chip, PTZ_reg[i], PTZ_dat[index][i]);
     }

    tmp = tp28xx_byte_read(chip, 0x71);
    tmp |= 0x20;
    tp28xx_byte_write(chip, 0x71, tmp);
}
static void TP2831_PTZ_mode_new(unsigned char chip, unsigned char ch, unsigned char mod)
{
    unsigned int tmp;

    TP2831_PTZ_mode(chip, ch, mod);

    //tmp = tp28xx_byte_read(chip, 0x6f);
    //tmp &= 0xbf;
    //tp28xx_byte_write(chip, 0x6f, tmp);
    tmp = tp28xx_byte_read(chip, 0x71);
    tmp |= 0x20;
    tp28xx_byte_write(chip, 0x71, tmp);

}
static void TP2831_output(unsigned char chip)
{
    unsigned int tmp;

    tp28xx_byte_write(chip, 0xF5, 0xf0); //
    tp28xx_byte_write(chip, 0xF1, 0x00);
    tp28xx_byte_write(chip, 0x4D, 0x0f);
    tp28xx_byte_write(chip, 0x4E, 0x0f);
    tp28xx_byte_write(chip, 0x4f, 0x03);

    if( SDR_1CH == output[chip] )
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x88);
        tp28xx_byte_write(chip, 0xF4, 0x80); //output clock 148.5M
        tp28xx_byte_write(chip, 0xF6, 0x00);
        tp28xx_byte_write(chip, 0xF7, 0x11);
        tp28xx_byte_write(chip, 0xF8, 0x22);
        tp28xx_byte_write(chip, 0xF9, 0x33);
        tp28xx_byte_write(chip, 0x50, 0x00); //
        tp28xx_byte_write(chip, 0x51, 0x00); //
        tp28xx_byte_write(chip, 0x52, 0x00); //
        tp28xx_byte_write(chip, 0x53, 0x00); //
        tp28xx_byte_write(chip, 0xF3, 0x77);
        tp28xx_byte_write(chip, 0xF2, 0x77);
        if(TP2802_720P25V2 == mode || TP2802_720P30V2 == mode || TP2802_PAL == mode || TP2802_NTSC == mode )
        {
            tmp = tp28xx_byte_read(chip, 0xFA);
            tmp &= 0x88;
            tmp |= 0x11;
            tp28xx_byte_write(chip, 0xFA, tmp);
            tmp = tp28xx_byte_read(chip, 0xFB);
            tmp &= 0x88;
            tmp |= 0x11;
            tp28xx_byte_write(chip, 0xFB, tmp);
        }
        else if(FLAG_HALF_MODE == (mode & FLAG_HALF_MODE) )
        {
            tmp = tp28xx_byte_read(chip, 0xFA);
            tmp &= 0x88;
            tmp |= 0x43;
            tp28xx_byte_write(chip, 0xFA, tmp);
            tmp = tp28xx_byte_read(chip, 0xFB);
            tmp &= 0x88;
            tmp |= 0x65;
            tp28xx_byte_write(chip, 0xFB, tmp);
        }
    }
    else if(SDR_2CH == output[chip])
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x88);
        tp28xx_byte_write(chip, 0xF4, 0x80); //output clock 148.5M
        tp28xx_byte_write(chip, 0xF6, 0x10);
        tp28xx_byte_write(chip, 0xF7, 0x23);
        tp28xx_byte_write(chip, 0xF8, 0x10);
        tp28xx_byte_write(chip, 0xF9, 0x23);
        tp28xx_byte_write(chip, 0x50, 0x00); //
        tp28xx_byte_write(chip, 0x51, 0x00); //
        tp28xx_byte_write(chip, 0x52, 0x00); //
        tp28xx_byte_write(chip, 0x53, 0x00); //
        tp28xx_byte_write(chip, 0xF3, 0x77);
        tp28xx_byte_write(chip, 0xF2, 0x77);
    }
    else if(DDR_2CH == output[chip])
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x88);
        //tp28xx_byte_write(chip, 0x45, 0x54); //PLL 297M
        tp28xx_byte_write(chip, 0xF4, 0xa0); //output clock 148.5M
        tp28xx_byte_write(chip, 0xF6, 0x10); //
        tp28xx_byte_write(chip, 0xF7, 0x23); //
        tp28xx_byte_write(chip, 0xF8, 0x10); //
        tp28xx_byte_write(chip, 0xF9, 0x23); //
        tp28xx_byte_write(chip, 0x50, 0x00); //
        tp28xx_byte_write(chip, 0x51, 0x00); //
        tp28xx_byte_write(chip, 0x52, 0x00); //
        tp28xx_byte_write(chip, 0x53, 0x00); //
        tp28xx_byte_write(chip, 0xF3, 0x77);
        tp28xx_byte_write(chip, 0xF2, 0x77);

    }
    else if(DDR_4CH == output[chip])
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x88);
        //tp28xx_byte_write(chip, 0x45, 0x54); //PLL 297M
        tp28xx_byte_write(chip, 0xF4, 0xa0); //output clock 148.5M
        tp28xx_byte_write(chip, 0xF6, 0x10); //
        tp28xx_byte_write(chip, 0xF7, 0x10); //
        tp28xx_byte_write(chip, 0xF8, 0x10); //
        tp28xx_byte_write(chip, 0xF9, 0x10); //
        tp28xx_byte_write(chip, 0x50, 0xB2); //
        tp28xx_byte_write(chip, 0x51, 0xB2); //
        tp28xx_byte_write(chip, 0x52, 0xB2); //
        tp28xx_byte_write(chip, 0x53, 0xB2); //
        tp28xx_byte_write(chip, 0xF3, 0x77);
        tp28xx_byte_write(chip, 0xF2, 0x77);
    }
    else if( DDR_1CH == output[chip] )
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x88);
        //tp28xx_byte_write(chip, 0x45, 0x54); //PLL 297M
        tp28xx_byte_write(chip, 0xF4, 0xa0); //output clock 148.5M
        tp28xx_byte_write(chip, 0xF6, 0x04);
        tp28xx_byte_write(chip, 0xF7, 0x15);
        tp28xx_byte_write(chip, 0xF8, 0x26);
        tp28xx_byte_write(chip, 0xF9, 0x37);
        tp28xx_byte_write(chip, 0x50, 0x00); //
        tp28xx_byte_write(chip, 0x51, 0x00); //
        tp28xx_byte_write(chip, 0x52, 0x00); //
        tp28xx_byte_write(chip, 0x53, 0x00); //
        tp28xx_byte_write(chip, 0xF3, 0x77);
        tp28xx_byte_write(chip, 0xF2, 0x77);
    }
}
static void TP2830_output(unsigned char chip)
{
    unsigned int tmp;
    tp28xx_byte_write(chip, 0xF5, 0xf0); // Changed value from 0x00 to 0xF0 - 11/12/2018
    tp28xx_byte_write(chip, 0xF1, 0x04);
    //tp28xx_byte_write(chip, 0x4D, 0x07);
    //tp28xx_byte_write(chip, 0x4E, 0x05);
    tp28xx_byte_write(chip, 0x4f, 0x03);

    if( SDR_1CH == output[chip] )
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x88);
        //tp28xx_byte_write(chip, 0x45, 0x09); //PLL 148.5M
        tp28xx_byte_write(chip, 0xF4, 0x80); //output clock 148.5M
        tp28xx_byte_write(chip, 0xF6, 0x00);
        tp28xx_byte_write(chip, 0xF7, 0x11);
        tp28xx_byte_write(chip, 0xF8, 0x22);
        tp28xx_byte_write(chip, 0xF9, 0x33);
        tp28xx_byte_write(chip, 0x50, 0x00); //
        tp28xx_byte_write(chip, 0x51, 0x00); //
        tp28xx_byte_write(chip, 0x52, 0x00); //
        tp28xx_byte_write(chip, 0x53, 0x00); //
        tp28xx_byte_write(chip, 0xF3, 0x07);
        tp28xx_byte_write(chip, 0xF2, 0x07); //delay 0~0xf
        if(TP2802_720P25V2 == mode || TP2802_720P30V2 == mode || TP2802_PAL == mode || TP2802_NTSC == mode )
        {
            tmp = tp28xx_byte_read(chip, 0xFA);
            tmp &= 0x88;
            tmp |= 0x11;
            tp28xx_byte_write(chip, 0xFA, tmp);
            tmp = tp28xx_byte_read(chip, 0xFB);
            tmp &= 0x88;
            tmp |= 0x11;
            tp28xx_byte_write(chip, 0xFB, tmp);
        }
        else if(FLAG_HALF_MODE == (mode & FLAG_HALF_MODE) )
        {
            tmp = tp28xx_byte_read(chip, 0xFA);
            tmp &= 0x88;
            tmp |= 0x43;
            tp28xx_byte_write(chip, 0xFA, tmp);
            tmp = tp28xx_byte_read(chip, 0xFB);
            tmp &= 0x88;
            tmp |= 0x65;
            tp28xx_byte_write(chip, 0xFB, tmp);
        }
    }
    else if(SDR_2CH == output[chip])
    {
        tp28xx_byte_write(chip, 0xFA, 0x88); //0x88 148.5M, 0x99 74.25
        tp28xx_byte_write(chip, 0xFB, 0x88); //0x88 148.5M, 0x99 74.25
        //tp28xx_byte_write(chip, 0x45, 0x09); //PLL 148.5M
        tp28xx_byte_write(chip, 0xF4, 0x80); //output clock 148.5M
        tp28xx_byte_write(chip, 0xF6, 0x10);
        tp28xx_byte_write(chip, 0xF7, 0x10);
        tp28xx_byte_write(chip, 0xF8, 0x23);
        tp28xx_byte_write(chip, 0xF9, 0x23);
        tp28xx_byte_write(chip, 0x50, 0x00); //
        tp28xx_byte_write(chip, 0x51, 0x00); //
        tp28xx_byte_write(chip, 0x52, 0x00); //
        tp28xx_byte_write(chip, 0x53, 0x00); //
        tp28xx_byte_write(chip, 0xF3, 0x07);
        tp28xx_byte_write(chip, 0xF2, 0x07);
    }
    else if(DDR_2CH == output[chip])
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x88);
        //tp28xx_byte_write(chip, 0x45, 0x09); //PLL 297M
        tp28xx_byte_write(chip, 0xF4, 0xa0); //output clock 148.5M
        tp28xx_byte_write(chip, 0xF6, 0x10); //
        tp28xx_byte_write(chip, 0xF7, 0x10); //
        tp28xx_byte_write(chip, 0xF8, 0x23); //
        tp28xx_byte_write(chip, 0xF9, 0x23); //
        tp28xx_byte_write(chip, 0x50, 0x00); //
        tp28xx_byte_write(chip, 0x51, 0x00); //
        tp28xx_byte_write(chip, 0x52, 0x00); //
        tp28xx_byte_write(chip, 0x53, 0x00); //
        tp28xx_byte_write(chip, 0xF3, 0x07);
        tp28xx_byte_write(chip, 0xF2, 0x07);

    }
    else if(DDR_4CH == output[chip])
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x88);
        //tp28xx_byte_write(chip, 0x45, 0x09); //PLL 297M
        tp28xx_byte_write(chip, 0xF4, 0xa0); //output clock 148.5M
        tp28xx_byte_write(chip, 0xF6, 0x10); //
        tp28xx_byte_write(chip, 0xF7, 0x10); //
        tp28xx_byte_write(chip, 0xF8, 0x10); //
        tp28xx_byte_write(chip, 0xF9, 0x10); //
        tp28xx_byte_write(chip, 0x50, 0xB2); //
        tp28xx_byte_write(chip, 0x51, 0xB2); //
        tp28xx_byte_write(chip, 0x52, 0xB2); //
        tp28xx_byte_write(chip, 0x53, 0xB2); //
        tp28xx_byte_write(chip, 0xF3, 0x04); //07
        tp28xx_byte_write(chip, 0xF2, 0x04); //07
    }
    else if( DDR_1CH == output[chip] )
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x88);
        //tp28xx_byte_write(chip, 0x45, 0x09); //PLL 297M
        tp28xx_byte_write(chip, 0xF4, 0xa0); //output clock 148.5M
        tp28xx_byte_write(chip, 0xF6, 0x04);
        tp28xx_byte_write(chip, 0xF7, 0x15);
        tp28xx_byte_write(chip, 0xF8, 0x26);
        tp28xx_byte_write(chip, 0xF9, 0x37);
        tp28xx_byte_write(chip, 0x50, 0x00); //
        tp28xx_byte_write(chip, 0x51, 0x00); //
        tp28xx_byte_write(chip, 0x52, 0x00); //
        tp28xx_byte_write(chip, 0x53, 0x00); //
        tp28xx_byte_write(chip, 0xF3, 0x07);
        tp28xx_byte_write(chip, 0xF2, 0x07);
    }
    tp28xx_byte_write(chip, 0x4D, 0x33);
    tp28xx_byte_write(chip, 0x4E, 0x11);
}


static void TP2831_reset_default(unsigned char chip, unsigned char ch)
{
    unsigned int tmp;
    //tp28xx_byte_write(chip, 0x26, 0x04);
    tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);
    tp28xx_byte_write(chip, 0x0e, 0x00);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x38, 0x00);
    tp28xx_byte_write(chip, 0x39, 0x1C);
    tp28xx_byte_write(chip, 0x3a, 0x32);
    tp28xx_byte_write(chip, 0x3B, 0x25); // Changed value from 0x25 (default) to 0x26 - 11/13/2018

    tmp = tp28xx_byte_read(chip, 0x26);
    tmp &= 0xfe;
    tp28xx_byte_write(chip, 0x26, tmp);

    tmp = tp28xx_byte_read(chip, 0x06);
    tmp &= 0xfb;
    tp28xx_byte_write(chip, 0x06, tmp);

    //tp28xx_byte_write(chip, 0x42, 0x80); // Changed value from 0x00 (default) to 0x80 - 11/12/2018

    //tp28xx_byte_write(chip, 0x80, 0x52);
    //tp28xx_byte_write(chip, 0x83, 0x4a);
}

static void TP2831_NTSC_DataSet(unsigned char chip)
{
    unsigned char tmp;
    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x13);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x20, 0x40);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x70);
    tp28xx_byte_write(chip, 0x2c, 0x2a);
    tp28xx_byte_write(chip, 0x2d, 0x68);
    tp28xx_byte_write(chip, 0x2e, 0x57);

    tp28xx_byte_write(chip, 0x30, 0x62);
    tp28xx_byte_write(chip, 0x31, 0xbb);
    tp28xx_byte_write(chip, 0x32, 0x96);
    tp28xx_byte_write(chip, 0x33, 0xc0);
    //tp28xx_byte_write(chip, 0x35, 0x25);
    tp28xx_byte_write(chip, 0x38, 0x00);
    tp28xx_byte_write(chip, 0x39, 0x04);
    tp28xx_byte_write(chip, 0x3a, 0x32);
    tp28xx_byte_write(chip, 0x3B, 0x25);

    tp28xx_byte_write(chip, 0x18, 0x12);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
}
static void TP2831_PAL_DataSet(unsigned char chip)
{
    unsigned char tmp;
    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x13);
    tp28xx_byte_write(chip, 0x0d, 0x51);

    tp28xx_byte_write(chip, 0x20, 0x48);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x22, 0x37);
    tp28xx_byte_write(chip, 0x23, 0x3f);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x70);
    tp28xx_byte_write(chip, 0x2c, 0x2a);
    tp28xx_byte_write(chip, 0x2d, 0x64);
    tp28xx_byte_write(chip, 0x2e, 0x56);

    tp28xx_byte_write(chip, 0x30, 0x7a);
    tp28xx_byte_write(chip, 0x31, 0x4a);
    tp28xx_byte_write(chip, 0x32, 0x4d);
    tp28xx_byte_write(chip, 0x33, 0xf0);
    //tp28xx_byte_write(chip, 0x35, 0x25);
    tp28xx_byte_write(chip, 0x38, 0x00);
    tp28xx_byte_write(chip, 0x39, 0x04);
    tp28xx_byte_write(chip, 0x3a, 0x32);
    tp28xx_byte_write(chip, 0x3B, 0x25);

    tp28xx_byte_write(chip, 0x18, 0x17);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
}
static void TP2831_V1_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);

    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x20, 0x30);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x30);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0xbb);
    tp28xx_byte_write(chip, 0x32, 0x2e);
    tp28xx_byte_write(chip, 0x33, 0x90);
    //tp28xx_byte_write(chip, 0x35, 0x05);
    tp28xx_byte_write(chip, 0x38, 0x00);
    tp28xx_byte_write(chip, 0x39, 0x1c);
    tp28xx_byte_write(chip, 0x3a, 0x32);
    tp28xx_byte_write(chip, 0x3B, 0x25);

    tp28xx_byte_write(chip, 0x80, 0x52);
    tp28xx_byte_write(chip, 0x81, 0x10);
    tp28xx_byte_write(chip, 0x82, 0x18);
    tp28xx_byte_write(chip, 0x83, 0x48);
    tp28xx_byte_write(chip, 0x84, 0x14);
    tp28xx_byte_write(chip, 0x88, 0x58);

    TP2831_RX_init(chip, PTZ_RX_TVI_CMD);
}
static void TP2831_V2_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);

    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x13);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x20, 0x30);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x30);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0xbb);
    tp28xx_byte_write(chip, 0x32, 0x2e);
    tp28xx_byte_write(chip, 0x33, 0x90);
    //tp28xx_byte_write(chip, 0x35, 0x25);
    tp28xx_byte_write(chip, 0x38, 0x00);
    tp28xx_byte_write(chip, 0x39, 0x18);
    tp28xx_byte_write(chip, 0x3a, 0x32);
    tp28xx_byte_write(chip, 0x3B, 0x25);

    TP2831_RX_init(chip, PTZ_RX_TVI_CMD);
}

static void TP2831_AQHDP30_DataSet(unsigned char chip)
{
    unsigned char tmp;
#if 0
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x40;
    tp28xx_byte_write(chip, 0x14, tmp);
#else
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
    tmp = tp28xx_byte_read(chip, 0x1c);
    tmp |= 0x80;
    tp28xx_byte_write(chip, 0x1c, tmp);
    tp28xx_byte_write(chip, 0x0d, 0x70);
    tp28xx_byte_write(chip, 0x0e, 0x0b);
#endif
    tp28xx_byte_write(chip, 0x13, 0x00);
    tp28xx_byte_write(chip, 0x15, 0x23);
    tp28xx_byte_write(chip, 0x16, 0x16);
    tp28xx_byte_write(chip, 0x18, 0x32);

    tp28xx_byte_write(chip, 0x20, 0x80);
    tp28xx_byte_write(chip, 0x21, 0x86);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xad);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0xa0);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0x6a);
    tp28xx_byte_write(chip, 0x32, 0xbe);
    tp28xx_byte_write(chip, 0x33, 0x80);
    //tp28xx_byte_write(chip, 0x35, 0x15);
    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x60);
    tp28xx_byte_write(chip, 0x3a, 0x12);
    tp28xx_byte_write(chip, 0x3B, 0x25);

    TP2831_RX_init(chip, PTZ_RX_ACP3);

}

/////HDA QHD25
static void TP2831_AQHDP25_DataSet(unsigned char chip)
{
    unsigned char tmp;
#if 0
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x40;
    tp28xx_byte_write(chip, 0x14, tmp);
#else
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
    tmp = tp28xx_byte_read(chip, 0x1c);
    tmp |= 0x80;
    tp28xx_byte_write(chip, 0x1c, tmp);
    tp28xx_byte_write(chip, 0x0d, 0x70);
    tp28xx_byte_write(chip, 0x0e, 0x0b);
#endif

    tp28xx_byte_write(chip, 0x13, 0x00);
    tp28xx_byte_write(chip, 0x15, 0x23);
    tp28xx_byte_write(chip, 0x16, 0x16);
    tp28xx_byte_write(chip, 0x18, 0x32);

    tp28xx_byte_write(chip, 0x20, 0x80);
    tp28xx_byte_write(chip, 0x21, 0x86);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xad);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0xa0);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0x6f);
    tp28xx_byte_write(chip, 0x32, 0xb5);
    tp28xx_byte_write(chip, 0x33, 0x80);
    //tp28xx_byte_write(chip, 0x35, 0x15);
    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x60);
    tp28xx_byte_write(chip, 0x3a, 0x12);
    tp28xx_byte_write(chip, 0x3B, 0x25);

    TP2831_RX_init(chip, PTZ_RX_ACP3);

}

/////HDA QXGA30
static void TP2831_AQXGAP30_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x40;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x13, 0x00);

    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x20, 0x90);
    tp28xx_byte_write(chip, 0x21, 0x86);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x01);
    tp28xx_byte_write(chip, 0x27, 0xad);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0xa0);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0x68);
    tp28xx_byte_write(chip, 0x32, 0xbe);
    tp28xx_byte_write(chip, 0x33, 0x80);

    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x40);
    tp28xx_byte_write(chip, 0x3a, 0x12);
    tp28xx_byte_write(chip, 0x3b, 0x25);

    TP2831_RX_init(chip, PTZ_RX_ACP3);
}

/////HDA QXGA25
static void TP2831_AQXGAP25_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x60;
    tp28xx_byte_write(chip, 0x14, tmp);
    tp28xx_byte_write(chip, 0x13, 0x00);

    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x20, 0x90);
    tp28xx_byte_write(chip, 0x21, 0x86);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x01);
    tp28xx_byte_write(chip, 0x27, 0xad);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0xa0);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0x6c);
    tp28xx_byte_write(chip, 0x32, 0xbe);
    tp28xx_byte_write(chip, 0x33, 0x80);

    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x40);
    tp28xx_byte_write(chip, 0x3a, 0x12);
    tp28xx_byte_write(chip, 0x3b, 0x25);

    TP2831_RX_init(chip, PTZ_RX_ACP3);
}


/////HDC QHD30
static void TP2831_CQHDP30_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x13, 0x40);
    tp28xx_byte_write(chip, 0x15, 0x13);
    tp28xx_byte_write(chip, 0x16, 0xfa);
    tp28xx_byte_write(chip, 0x18, 0x38);
    tp28xx_byte_write(chip, 0x1c, 0x0c);
    tp28xx_byte_write(chip, 0x1d, 0x80);

    tp28xx_byte_write(chip, 0x20, 0x50);
    tp28xx_byte_write(chip, 0x21, 0x86);
    tp28xx_byte_write(chip, 0x22, 0x38);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xda);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x6c);
    tp28xx_byte_write(chip, 0x2e, 0x50);


    tp28xx_byte_write(chip, 0x30, 0x75);
    tp28xx_byte_write(chip, 0x31, 0x39);
    tp28xx_byte_write(chip, 0x32, 0xc0);
    tp28xx_byte_write(chip, 0x33, 0x31);

    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x68);
    tp28xx_byte_write(chip, 0x3a, 0x12);
    tp28xx_byte_write(chip, 0x3b, 0x25);
}

/////HDC QHD25
static void TP2831_CQHDP25_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x13, 0x40);
    tp28xx_byte_write(chip, 0x15, 0x13);
    tp28xx_byte_write(chip, 0x16, 0xd8);
    tp28xx_byte_write(chip, 0x18, 0x30);
    tp28xx_byte_write(chip, 0x1c, 0x0c);
    tp28xx_byte_write(chip, 0x1d, 0x80);

    tp28xx_byte_write(chip, 0x20, 0x50);
    tp28xx_byte_write(chip, 0x21, 0x86);
    tp28xx_byte_write(chip, 0x22, 0x38);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xda);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x6c);
    tp28xx_byte_write(chip, 0x2e, 0x50);

    tp28xx_byte_write(chip, 0x30, 0x75);
    tp28xx_byte_write(chip, 0x31, 0x39);
    tp28xx_byte_write(chip, 0x32, 0xc0);
    tp28xx_byte_write(chip, 0x33, 0x3b);

    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x68);
    tp28xx_byte_write(chip, 0x3a, 0x12);
    tp28xx_byte_write(chip, 0x3b, 0x25);
}

///////HDA QHD15
static void TP2831_AQHDP15_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x40;
    tp28xx_byte_write(chip, 0x14, tmp);
    tp28xx_byte_write(chip, 0x13, 0x00);

    tp28xx_byte_write(chip, 0x20, 0x38);
    tp28xx_byte_write(chip, 0x21, 0x46);

    tp28xx_byte_write(chip, 0x25, 0xfe);
    tp28xx_byte_write(chip, 0x26, 0x01);

    tp28xx_byte_write(chip, 0x2d, 0x44);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x29);
    tp28xx_byte_write(chip, 0x31, 0x68);
    tp28xx_byte_write(chip, 0x32, 0x78);
    tp28xx_byte_write(chip, 0x33, 0x10);

    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x40);
    tp28xx_byte_write(chip, 0x3a, 0x12);
    TP2831_RX_init(chip, PTZ_RX_ACP1);
}

/////HDA QXGA18
static void TP2831_AQXGAP18_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x40;
    tp28xx_byte_write(chip, 0x14, tmp);
    tp28xx_byte_write(chip, 0x13, 0x00);

    tp28xx_byte_write(chip, 0x15, 0x13);
    tp28xx_byte_write(chip, 0x16, 0x10);
    tp28xx_byte_write(chip, 0x18, 0x68);

    tp28xx_byte_write(chip, 0x20, 0x48);
    tp28xx_byte_write(chip, 0x21, 0x46);
    tp28xx_byte_write(chip, 0x25, 0xfe);
    tp28xx_byte_write(chip, 0x26, 0x05);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2d, 0x52);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x29);
    tp28xx_byte_write(chip, 0x31, 0x65);
    tp28xx_byte_write(chip, 0x32, 0x2b);
    tp28xx_byte_write(chip, 0x33, 0xd0);

    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x40);
    tp28xx_byte_write(chip, 0x3a, 0x12);
    TP2831_RX_init(chip, PTZ_RX_ACP1);
}

/////TVI QHD30/QHD25
static void TP2831_QHDP30_25_DataSet(unsigned char chip)
{
    unsigned char tmp;

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);

    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    //tp28xx_byte_write(chip, 0x15, 0x23);
    //tp28xx_byte_write(chip, 0x16, 0x1b);
    //tp28xx_byte_write(chip, 0x18, 0x38);

    tp28xx_byte_write(chip, 0x20, 0x50);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xad);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x58);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x74);
    tp28xx_byte_write(chip, 0x31, 0x58);
    tp28xx_byte_write(chip, 0x32, 0x9f);
    tp28xx_byte_write(chip, 0x33, 0x60);

    //tp28xx_byte_write(chip, 0x35, 0x05);
    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x68);
    tp28xx_byte_write(chip, 0x3a, 0x13);
    tp28xx_byte_write(chip, 0x3b, 0x25);

    tp28xx_byte_write(chip, 0x80, 0x52);
    tp28xx_byte_write(chip, 0x81, 0x10);
    tp28xx_byte_write(chip, 0x82, 0x30);
    tp28xx_byte_write(chip, 0x83, 0xbe);
    tp28xx_byte_write(chip, 0x84, 0x28);
    tp28xx_byte_write(chip, 0x88, 0x58);

    TP2831_RX_init(chip, PTZ_RX_TVI_CMD);
}

/////TVI 5M20
static void TP2831_5MP20_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);

    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x20, 0x50);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xad);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x54);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x74);
    tp28xx_byte_write(chip, 0x31, 0xa7);
    tp28xx_byte_write(chip, 0x32, 0x18);
    tp28xx_byte_write(chip, 0x33, 0x50);

    //tp28xx_byte_write(chip, 0x35, 0x05);
    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x68);
    tp28xx_byte_write(chip, 0x3a, 0x13);
    tp28xx_byte_write(chip, 0x3b, 0x25);

    tp28xx_byte_write(chip, 0x80, 0x52);
    tp28xx_byte_write(chip, 0x81, 0x10);
    tp28xx_byte_write(chip, 0x82, 0x20);
    tp28xx_byte_write(chip, 0x83, 0xba);
    tp28xx_byte_write(chip, 0x84, 0x28);
    tp28xx_byte_write(chip, 0x88, 0x58);

    TP2831_RX_init(chip, PTZ_RX_TVI_CMD);
}

/////HDA 5M20
static void TP2831_A5MP20_DataSet(unsigned char chip)
{
    unsigned char tmp;
#if 0
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x40;
    tp28xx_byte_write(chip, 0x14, tmp);
#else
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
    tmp = tp28xx_byte_read(chip, 0x1c);
    tmp |= 0x80;
    tp28xx_byte_write(chip, 0x1c, tmp);
    tp28xx_byte_write(chip, 0x0d, 0x70);
    tp28xx_byte_write(chip, 0x0e, 0x0b);
#endif
    tp28xx_byte_write(chip, 0x20, 0x80);
    tp28xx_byte_write(chip, 0x21, 0x86);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xad);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0xA0);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0x77);
    tp28xx_byte_write(chip, 0x32, 0x0e);
    tp28xx_byte_write(chip, 0x33, 0xa0);

    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x68);
    tp28xx_byte_write(chip, 0x3a, 0x12);
    tp28xx_byte_write(chip, 0x3B, 0x25);
    TP2831_RX_init(chip, PTZ_RX_ACP3);
}

/////TVI 8M15
static void TP2831_8MP15_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);

    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x20, 0x60);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xad);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x54);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x74);
    tp28xx_byte_write(chip, 0x31, 0x59);
    tp28xx_byte_write(chip, 0x32, 0xbd);
    tp28xx_byte_write(chip, 0x33, 0x60);

    //tp28xx_byte_write(chip, 0x35, 0x05);
    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x68);
    tp28xx_byte_write(chip, 0x3a, 0x13);
    tp28xx_byte_write(chip, 0x3b, 0x25);

    tp28xx_byte_write(chip, 0x80, 0x52);
    tp28xx_byte_write(chip, 0x81, 0x17);
    tp28xx_byte_write(chip, 0x82, 0x40);
    tp28xx_byte_write(chip, 0x83, 0x8e);
    tp28xx_byte_write(chip, 0x84, 0x28);
    tp28xx_byte_write(chip, 0x88, 0x58);

    TP2831_RX_init(chip, PTZ_RX_TVI_CMD);
}
///////HDA 5M12.5
static void TP2831_A5MP12_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x40;
    tp28xx_byte_write(chip, 0x14, tmp);
    tp28xx_byte_write(chip, 0x13, 0x00);

    tp28xx_byte_write(chip, 0x20, 0x38);
    tp28xx_byte_write(chip, 0x21, 0x46);

    tp28xx_byte_write(chip, 0x25, 0xfe);
    tp28xx_byte_write(chip, 0x26, 0x01);

    tp28xx_byte_write(chip, 0x2d, 0x44);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x29);
    tp28xx_byte_write(chip, 0x31, 0x68);
    tp28xx_byte_write(chip, 0x32, 0x72);
    tp28xx_byte_write(chip, 0x33, 0xb0);

    tp28xx_byte_write(chip, 0x16, 0x10);
    tp28xx_byte_write(chip, 0x18, 0x1a);
    tp28xx_byte_write(chip, 0x1d, 0xb8);
    tp28xx_byte_write(chip, 0x36, 0xbc);

    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x40);
    tp28xx_byte_write(chip, 0x3a, 0x12);

    TP2831_RX_init(chip, PTZ_RX_ACP1);
}
///////////////////////////////////////////////////////////////////
static void TP2831_A720P30_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x02);
    tmp |= 0x04;
    tp28xx_byte_write(chip, 0x02, tmp);

    tp28xx_byte_write(chip, 0x0d, 0x70);

    tp28xx_byte_write(chip, 0x20, 0x40);
    tp28xx_byte_write(chip, 0x21, 0x46);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xfe);
    tp28xx_byte_write(chip, 0x26, 0x01);
    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x3a);
    tp28xx_byte_write(chip, 0x2d, 0x5a);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x9d);
    tp28xx_byte_write(chip, 0x31, 0xca);
    tp28xx_byte_write(chip, 0x32, 0x01);
    tp28xx_byte_write(chip, 0x33, 0xd0);

    tp28xx_byte_write(chip, 0x38, 0x00);
    tp28xx_byte_write(chip, 0x39, 0x18);
    tp28xx_byte_write(chip, 0x3a, 0x32);
    tp28xx_byte_write(chip, 0x3b, 0x25);

    TP2831_RX_init(chip, PTZ_RX_ACP1);
}
static void TP2831_A720P25_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x02);
    tmp |= 0x04;
    tp28xx_byte_write(chip, 0x02, tmp);

    tp28xx_byte_write(chip, 0x0d, 0x71);

    tp28xx_byte_write(chip, 0x20, 0x40);
    tp28xx_byte_write(chip, 0x21, 0x46);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xfe);
    tp28xx_byte_write(chip, 0x26, 0x01);
    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x3a);
    tp28xx_byte_write(chip, 0x2d, 0x5a);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x9e);
    tp28xx_byte_write(chip, 0x31, 0x20);
    tp28xx_byte_write(chip, 0x32, 0x10);
    tp28xx_byte_write(chip, 0x33, 0x90);

    tp28xx_byte_write(chip, 0x38, 0x00);
    tp28xx_byte_write(chip, 0x39, 0x18);
    tp28xx_byte_write(chip, 0x3a, 0x32);
    tp28xx_byte_write(chip, 0x3b, 0x25);

    TP2831_RX_init(chip, PTZ_RX_ACP1);
}
static void TP2831_A1080P30_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x02);
    tmp |= 0x04;
    tp28xx_byte_write(chip, 0x02, tmp);

    tp28xx_byte_write(chip, 0x15, 0x01);
    tp28xx_byte_write(chip, 0x16, 0xf0);

    tp28xx_byte_write(chip, 0x0d, 0x72);

    tp28xx_byte_write(chip, 0x20, 0x38);
    tp28xx_byte_write(chip, 0x21, 0x46);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xfe);
    tp28xx_byte_write(chip, 0x26, 0x01);
    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x3a);
    tp28xx_byte_write(chip, 0x2d, 0x54);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0xa5);
    tp28xx_byte_write(chip, 0x31, 0x95);
    tp28xx_byte_write(chip, 0x32, 0xe0);
    tp28xx_byte_write(chip, 0x33, 0x60);

    tp28xx_byte_write(chip, 0x38, 0x00);
    tp28xx_byte_write(chip, 0x39, 0x1c);
    tp28xx_byte_write(chip, 0x3a, 0x32);
    tp28xx_byte_write(chip, 0x3b, 0x25);

    TP2831_RX_init(chip, PTZ_RX_ACP1);
}
static void TP2831_A1080P25_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x02);
    tmp |= 0x04;
    tp28xx_byte_write(chip, 0x02, tmp);

    tp28xx_byte_write(chip, 0x15, 0x01);
    tp28xx_byte_write(chip, 0x16, 0xf0);

    tp28xx_byte_write(chip, 0x0d, 0x73);

    tp28xx_byte_write(chip, 0x20, 0x3c);
    tp28xx_byte_write(chip, 0x21, 0x46);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xfe);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x3a);
    tp28xx_byte_write(chip, 0x2d, 0x54);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0xa5);
    tp28xx_byte_write(chip, 0x31, 0x86);
    tp28xx_byte_write(chip, 0x32, 0xfb);
    tp28xx_byte_write(chip, 0x33, 0x60);

    tp28xx_byte_write(chip, 0x38, 0x00);
    tp28xx_byte_write(chip, 0x39, 0x1c);
    tp28xx_byte_write(chip, 0x3a, 0x32);
    tp28xx_byte_write(chip, 0x3b, 0x25);

    TP2831_RX_init(chip, PTZ_RX_ACP1);
}
static void TP2831_1080P60_DataSet(unsigned char chip)
{
    unsigned char tmp;

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);

    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x15, 0x03);
    tp28xx_byte_write(chip, 0x16, 0xf0);
    tp28xx_byte_write(chip, 0x17, 0x80);
    tp28xx_byte_write(chip, 0x18, 0x12);
    tp28xx_byte_write(chip, 0x19, 0x38);
    tp28xx_byte_write(chip, 0x1a, 0x47);
    tp28xx_byte_write(chip, 0x1c, 0x08);
    tp28xx_byte_write(chip, 0x1d, 0x96);

    tp28xx_byte_write(chip, 0x20, 0x38);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xad);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x40);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x74);
    tp28xx_byte_write(chip, 0x31, 0x9b);
    tp28xx_byte_write(chip, 0x32, 0xa5);
    tp28xx_byte_write(chip, 0x33, 0xe0);

    //tp28xx_byte_write(chip, 0x35, 0x05);
    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x68);
    tp28xx_byte_write(chip, 0x3a, 0x13);
    tp28xx_byte_write(chip, 0x3b, 0x25);

    TP2831_RX_init(chip, PTZ_RX_TVI_CMD);
}
static void TP2831_1080P50_DataSet(unsigned char chip)
{
    unsigned char tmp;

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);

    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x15, 0x03);
    tp28xx_byte_write(chip, 0x16, 0xe2);
    tp28xx_byte_write(chip, 0x17, 0x80);
    tp28xx_byte_write(chip, 0x18, 0x27);
    tp28xx_byte_write(chip, 0x19, 0x38);
    tp28xx_byte_write(chip, 0x1a, 0x47);
    tp28xx_byte_write(chip, 0x1c, 0x0a);
    tp28xx_byte_write(chip, 0x1d, 0x50);

    tp28xx_byte_write(chip, 0x20, 0x38);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xad);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x40);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x74);
    tp28xx_byte_write(chip, 0x31, 0x9b);
    tp28xx_byte_write(chip, 0x32, 0xa5);
    tp28xx_byte_write(chip, 0x33, 0xe0);

    //tp28xx_byte_write(chip, 0x35, 0x05);
    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x68);
    tp28xx_byte_write(chip, 0x3a, 0x13);
    tp28xx_byte_write(chip, 0x3b, 0x25);

    TP2831_RX_init(chip, PTZ_RX_TVI_CMD);
}
//HDC 8M15
static void TP2831_C8MP15_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x13, 0x40);

    tp28xx_byte_write(chip, 0x20, 0x80);
    tp28xx_byte_write(chip, 0x21, 0x86);
    tp28xx_byte_write(chip, 0x22, 0x38);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xda);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x90);
    tp28xx_byte_write(chip, 0x2e, 0x50);

    tp28xx_byte_write(chip, 0x30, 0x75);
    tp28xx_byte_write(chip, 0x31, 0x39);
    tp28xx_byte_write(chip, 0x32, 0xc0);
    tp28xx_byte_write(chip, 0x33, 0x32);

    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x68);
    tp28xx_byte_write(chip, 0x3a, 0x12);
    tp28xx_byte_write(chip, 0x3b, 0x25);
}

/////HDC 8M12
static void TP2831_C8MP12_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x13, 0x40);
    tp28xx_byte_write(chip, 0x15, 0x23);
    tp28xx_byte_write(chip, 0x16, 0xf8);
    tp28xx_byte_write(chip, 0x18, 0x50);

    tp28xx_byte_write(chip, 0x20, 0x80);
    tp28xx_byte_write(chip, 0x21, 0x86);
    tp28xx_byte_write(chip, 0x22, 0x38);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xda);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x9c);
    tp28xx_byte_write(chip, 0x2e, 0x50);

    tp28xx_byte_write(chip, 0x30, 0x75);
    tp28xx_byte_write(chip, 0x31, 0x39);
    tp28xx_byte_write(chip, 0x32, 0xc0);
    tp28xx_byte_write(chip, 0x33, 0x32);

    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x68);
    tp28xx_byte_write(chip, 0x3a, 0x12);
    tp28xx_byte_write(chip, 0x3b, 0x25);
}
/////HDA 8M15
static void TP2831_A8MP15_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x40;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tp28xx_byte_write(chip, 0x15, 0x13);
    tp28xx_byte_write(chip, 0x16, 0x74);
    //tp28xx_byte_write(chip, 0x18, 0x32);

    tp28xx_byte_write(chip, 0x20, 0x80);
    tp28xx_byte_write(chip, 0x21, 0x86);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xad);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x58);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0x68);
    tp28xx_byte_write(chip, 0x32, 0x43);
    tp28xx_byte_write(chip, 0x33, 0x00);

    //tp28xx_byte_write(chip, 0x35, 0x15);
    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x68);
    tp28xx_byte_write(chip, 0x3a, 0x12);
    tp28xx_byte_write(chip, 0x3b, 0x25);

    TP2831_RX_init(chip, PTZ_RX_ACP3);
}
/////TVI 8M15V2
static void TP2831_8MP15V2_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);

    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x15, 0x13);
    tp28xx_byte_write(chip, 0x16, 0xbd);
    tp28xx_byte_write(chip, 0x17, 0xc0);
    tp28xx_byte_write(chip, 0x18, 0x14);
    tp28xx_byte_write(chip, 0x19, 0x90);
    tp28xx_byte_write(chip, 0x1a, 0x9c);
    tp28xx_byte_write(chip, 0x1c, 0x0f);
    tp28xx_byte_write(chip, 0x1d, 0x76);

    tp28xx_byte_write(chip, 0x20, 0x50);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xad);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x54);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x74);
    tp28xx_byte_write(chip, 0x31, 0x59);
    tp28xx_byte_write(chip, 0x32, 0xbd);
    tp28xx_byte_write(chip, 0x33, 0x60);

    tp28xx_byte_write(chip, 0x35, 0x19);
    tp28xx_byte_write(chip, 0x36, 0xc4);

    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x68);
    tp28xx_byte_write(chip, 0x3a, 0x13);
    tp28xx_byte_write(chip, 0x3b, 0x25);

    TP2831_RX_init(chip, PTZ_RX_TVI_CMD);
}

/////TVI 5M12
static void TP2831_5MP12_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);

    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x20, 0x30);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x30);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0xbb);
    tp28xx_byte_write(chip, 0x32, 0x2e);
    tp28xx_byte_write(chip, 0x33, 0x90);
    //tp28xx_byte_write(chip, 0x35, 0x05);
    tp28xx_byte_write(chip, 0x38, 0x00);
    tp28xx_byte_write(chip, 0x39, 0x1c);
    tp28xx_byte_write(chip, 0x3a, 0x32);
    tp28xx_byte_write(chip, 0x3B, 0x25);

    tp28xx_byte_write(chip, 0x80, 0x52);
    tp28xx_byte_write(chip, 0x81, 0x1a);
    tp28xx_byte_write(chip, 0x82, 0x10);
    tp28xx_byte_write(chip, 0x83, 0x6d);
    tp28xx_byte_write(chip, 0x84, 0x14);
    tp28xx_byte_write(chip, 0x88, 0x58);

    TP2831_RX_init(chip, PTZ_RX_TVI_CMD);
}
/////TVI 4M15
static void TP2831_4MP15_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);

    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x20, 0x30);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x30);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0xbb);
    tp28xx_byte_write(chip, 0x32, 0x2e);
    tp28xx_byte_write(chip, 0x33, 0x90);
    //tp28xx_byte_write(chip, 0x35, 0x05);
    tp28xx_byte_write(chip, 0x38, 0x00);
    tp28xx_byte_write(chip, 0x39, 0x1c);
    tp28xx_byte_write(chip, 0x3a, 0x32);
    tp28xx_byte_write(chip, 0x3B, 0x25);

    tp28xx_byte_write(chip, 0x80, 0x52);
    tp28xx_byte_write(chip, 0x81, 0x1a);
    tp28xx_byte_write(chip, 0x82, 0x30);
    tp28xx_byte_write(chip, 0x83, 0x4a);
    tp28xx_byte_write(chip, 0x84, 0x28);
    tp28xx_byte_write(chip, 0x88, 0x58);

    TP2831_RX_init(chip, PTZ_RX_TVI_CMD);
}
/////TVI QHD15
static void TP2831_QHDP15_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);

    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x03);

    tp28xx_byte_write(chip, 0x15, 0x23);

    tp28xx_byte_write(chip, 0x20, 0x50);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x48);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0xbb);
    tp28xx_byte_write(chip, 0x32, 0x2e);
    tp28xx_byte_write(chip, 0x33, 0x90);
    //tp28xx_byte_write(chip, 0x35, 0x05);
    tp28xx_byte_write(chip, 0x38, 0x00);
    tp28xx_byte_write(chip, 0x39, 0x1c);
    tp28xx_byte_write(chip, 0x3a, 0x32);
    tp28xx_byte_write(chip, 0x3B, 0x25);

    tp28xx_byte_write(chip, 0x80, 0x52);
    tp28xx_byte_write(chip, 0x81, 0x18);
    tp28xx_byte_write(chip, 0x82, 0x30);
    tp28xx_byte_write(chip, 0x83, 0xd8);
    tp28xx_byte_write(chip, 0x84, 0x28);
    tp28xx_byte_write(chip, 0x88, 0x58);

    TP2831_RX_init(chip, PTZ_RX_TVI_CMD);
}
/////TVI 5M20V2
static void TP2831_5MP20V2_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);

    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x15, 0x23);
    tp28xx_byte_write(chip, 0x16, 0x14);
    tp28xx_byte_write(chip, 0x17, 0x90);
    tp28xx_byte_write(chip, 0x18, 0x43);
    tp28xx_byte_write(chip, 0x19, 0x7c);
    tp28xx_byte_write(chip, 0x1a, 0x6b);
    tp28xx_byte_write(chip, 0x1c, 0x10);
    tp28xx_byte_write(chip, 0x1d, 0x96);

    tp28xx_byte_write(chip, 0x20, 0x50);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0xad);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x58);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x74);
    tp28xx_byte_write(chip, 0x31, 0xa7);
    tp28xx_byte_write(chip, 0x32, 0x18);
    tp28xx_byte_write(chip, 0x33, 0x50);

    tp28xx_byte_write(chip, 0x35, 0x16);
    tp28xx_byte_write(chip, 0x36, 0xd4);

    tp28xx_byte_write(chip, 0x38, 0x40);
    tp28xx_byte_write(chip, 0x39, 0x68);
    tp28xx_byte_write(chip, 0x3a, 0x13);
    tp28xx_byte_write(chip, 0x3b, 0x25);

    tp28xx_byte_write(chip, 0x80, 0x52);
    tp28xx_byte_write(chip, 0x81, 0x18);
    tp28xx_byte_write(chip, 0x82, 0x40);
    tp28xx_byte_write(chip, 0x83, 0xcc);
    tp28xx_byte_write(chip, 0x84, 0x28);
    tp28xx_byte_write(chip, 0x85, 0x0e);
    tp28xx_byte_write(chip, 0x86, 0x1f);
    tp28xx_byte_write(chip, 0x87, 0x4a);
    tp28xx_byte_write(chip, 0x88, 0x58);

    TP2831_RX_init(chip, PTZ_RX_TVI_CMD);
}

/////TVI 8M7.5
static void TP2831_A8MP7_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x40;
    tp28xx_byte_write(chip, 0x14, tmp);

    //tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x15, 0x03);
    tp28xx_byte_write(chip, 0x16, 0xc0);
    tp28xx_byte_write(chip, 0x17, 0x00);
    tp28xx_byte_write(chip, 0x18, 0x50);
    tp28xx_byte_write(chip, 0x19, 0x70);
    tp28xx_byte_write(chip, 0x1a, 0x8f);
    tp28xx_byte_write(chip, 0x1c, 0x0f);
    tp28xx_byte_write(chip, 0x1d, 0xa0);


    tp28xx_byte_write(chip, 0x20, 0x30);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x38);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x24);
    tp28xx_byte_write(chip, 0x31, 0x34);
    tp28xx_byte_write(chip, 0x32, 0x21);
    tp28xx_byte_write(chip, 0x33, 0x80);

    tp28xx_byte_write(chip, 0x35, 0x19);
    tp28xx_byte_write(chip, 0x36, 0xab);

    tp28xx_byte_write(chip, 0x38, 0x00);
    tp28xx_byte_write(chip, 0x39, 0x1c);
    tp28xx_byte_write(chip, 0x3a, 0x32);
    tp28xx_byte_write(chip, 0x3b, 0x25);

}

static int tp2802_set_video_mode(unsigned char chip, unsigned char mode, unsigned char ch, unsigned char std)
{
    int err=0;
    unsigned int tmp;

    if(STD_HDA_DEFAULT == std) std = STD_HDA;

    // Set Page Register to the appropriate Channel
    tp2802_set_reg_page(chip, ch);

    //if(id[chip] >= TP2822) tp28xx_byte_write(chip, 0x35, 0x05);

    //switch(mode)
    switch(mode&(~FLAG_HALF_MODE))
    {
    case TP2802_HALF1080P25:
        //tp28xx_byte_write(chip, 0x35, 0x45);
    case TP2802_1080P25:
        if(id[chip] >= TP2822) tp28xx_byte_write(chip, 0x35, 0x05);
        tp2802_set_work_mode_1080p25(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);
        if(TP2823  == id[chip]) TP2823_V1_DataSet(chip);
        if(TP2834  == id[chip]) TP2834_V1_DataSet(chip);
        if(TP2833  == id[chip]) TP2833_V1_DataSet(chip);
        if(TP2853C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2833C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2823C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2826  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2816  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2827  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2828  == id[chip]) TP2829_V1_DataSet(chip);
        if(TP2830  == id[chip]) TP2831_V1_DataSet(chip); // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) TP2831_V1_DataSet(chip); // Added TP2831 ID - 10/15/2018

        if( STD_HDA == std)
        {
            if(TP2834 == id[chip])
            {
                TP2833_A1080P_DataSet(chip);
            }
            if(TP2833 == id[chip])
            {
                TP2833_A1080P_DataSet(chip);
            }
            if(TP2853C == id[chip])
            {
                TP2853C_A1080P25_DataSet(chip);
            }
            if(TP2833C == id[chip])
            {
                TP2853C_A1080P25_DataSet(chip);
            }
            if(TP2823C == id[chip])
            {
                TP2853C_A1080P25_DataSet(chip);
            }
            if(TP2826 == id[chip])
            {
                TP2826_A1080P25_DataSet(chip);
            }
            if(TP2816 == id[chip])
            {
                TP2826_A1080P25_DataSet(chip);
            }
            if(TP2827 == id[chip])
            {
                TP2826_A1080P25_DataSet(chip);
            }
            if(TP2828 == id[chip] || TP2829 == id[chip])
            {
                TP2829_A1080P25_DataSet(chip);
            }
            if(TP2830 == id[chip] || TP2831 == id[chip]) // Added TP2830/31 ID - 10/15/2018
            {
                TP2831_A1080P25_DataSet(chip);
            }
        }
        else if(STD_HDC == std || STD_HDC_DEFAULT == std)
        {
            if(TP2853C == id[chip])
            {
                TP2853C_C1080P25_DataSet(chip);
            }
            if(TP2833C == id[chip])
            {
                TP2853C_C1080P25_DataSet(chip);
            }
            if(TP2823C == id[chip])
            {
                TP2853C_C1080P25_DataSet(chip);
            }
            if(TP2833 == id[chip])
            {
                TP2853C_C1080P25_DataSet(chip);
            }
            if(TP2834 == id[chip])
            {
                TP2834_C1080P25_DataSet(chip);
            }
            if(TP2826 == id[chip])
            {
                TP2826_C1080P25_DataSet(chip);
            }
            if(TP2816 == id[chip])
            {
                TP2826_C1080P25_DataSet(chip);
            }
            if(TP2827 == id[chip])
            {
                TP2826_C1080P25_DataSet(chip);
            }
            if(TP2827C == id[chip] || TP2826C == id[chip])
            {
                TP2826_C1080P25_DataSet(chip);
            }
            if(TP2828 == id[chip] || TP2829 == id[chip])
            {
                TP2826_C1080P25_DataSet(chip);
            }
            if(TP2830 == id[chip] || TP2831 == id[chip]) // Added TP2830/31 ID - 10/15/2018
            {
                TP2826_C1080P25_DataSet(chip);
            }

            if(STD_HDC == std) //HDC 1080p25 position adjust
               {
                    tp28xx_byte_write(chip, 0x15, 0x13);
                    tp28xx_byte_write(chip, 0x16, 0x84);
                    if(TP2827 == id[chip] || TP2826 == id[chip] || TP2816 == id[chip] || TP2827C == id[chip] || TP2826C == id[chip] || TP2828 == id[chip] || TP2829 == id[chip] || TP2830 == id[chip] || TP2831 == id[chip] ) // Added TP2830/31 ID - 10/15/2018
                    {
                        tp28xx_byte_write(chip, 0x15, 0x13);
                        tp28xx_byte_write(chip, 0x16, 0x60);
                        tp28xx_byte_write(chip, 0x17, 0x80);
                        tp28xx_byte_write(chip, 0x18, 0x29);
                        tp28xx_byte_write(chip, 0x19, 0x38);
                        tp28xx_byte_write(chip, 0x1A, 0x47);
                        //tp28xx_byte_write(chip, 0x1C, 0x0a);
                        //tp28xx_byte_write(chip, 0x1D, 0x50);
                        tp28xx_byte_write(chip, 0x1C, 0x09);
                        tp28xx_byte_write(chip, 0x1D, 0x60);
                    }
               }
        }

        tp282x_SYSCLK_V1(chip, ch);
        break;

    case TP2802_HALF1080P30:
        //tp28xx_byte_write(chip, 0x35, 0x45);
    case TP2802_1080P30:
        if(id[chip] >= TP2822) tp28xx_byte_write(chip, 0x35, 0x05);
        tp2802_set_work_mode_1080p30(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);
        if(TP2823  == id[chip]) TP2823_V1_DataSet(chip);
        if(TP2834  == id[chip]) TP2834_V1_DataSet(chip);
        if(TP2833  == id[chip]) TP2833_V1_DataSet(chip);
        if(TP2853C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2833C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2823C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2826  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2816  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2827  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2828  == id[chip]) TP2829_V1_DataSet(chip);
        if(TP2830  == id[chip]) TP2831_V1_DataSet(chip); // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) TP2831_V1_DataSet(chip); // Added TP2831 ID - 10/15/2018


        if( STD_HDA == std)
        {
            if(TP2834 == id[chip])
            {
                TP2833_A1080N_DataSet(chip);
            }
            if(TP2833 == id[chip])
            {
                TP2833_A1080N_DataSet(chip);
            }
            if(TP2853C == id[chip])
            {
                TP2853C_A1080P30_DataSet(chip);
            }
            if(TP2833C == id[chip])
            {
                TP2853C_A1080P30_DataSet(chip);
            }
            if(TP2823C == id[chip])
            {
                TP2853C_A1080P30_DataSet(chip);
            }
            if(TP2826 == id[chip])
            {
                TP2826_A1080P30_DataSet(chip);
            }
            if(TP2816 == id[chip])
            {
                TP2826_A1080P30_DataSet(chip);
            }
            if(TP2827 == id[chip])
            {
                TP2826_A1080P30_DataSet(chip);
            }
            if(TP2828 == id[chip] || TP2829 == id[chip])
            {
                TP2829_A1080P30_DataSet(chip);
            }
            if(TP2830 == id[chip] || TP2831 == id[chip]) // Added TP2830/31 ID - 10/15/2018
            {
                TP2831_A1080P30_DataSet(chip);
            }
        }
        else if(STD_HDC == std || STD_HDC_DEFAULT == std)
        {
            if(TP2853C == id[chip])
            {
                TP2853C_C1080P30_DataSet(chip);
            }
            if(TP2833C == id[chip])
            {
                TP2853C_C1080P30_DataSet(chip);
            }
            if(TP2823C == id[chip])
            {
                TP2853C_C1080P30_DataSet(chip);
            }
            if(TP2833 == id[chip])
            {
                TP2853C_C1080P30_DataSet(chip);
            }
            if(TP2834 == id[chip])
            {
                TP2834_C1080P30_DataSet(chip);
            }
            if(TP2826 == id[chip])
            {
                TP2826_C1080P30_DataSet(chip);
            }
            if(TP2816 == id[chip])
            {
                TP2826_C1080P30_DataSet(chip);
            }
            if(TP2827 == id[chip])
            {
                TP2826_C1080P30_DataSet(chip);
            }
            if(TP2827C == id[chip] || TP2826C == id[chip])
            {
                TP2826_C1080P30_DataSet(chip);
            }
            if(TP2828 == id[chip] || TP2829 == id[chip])
            {
                TP2826_C1080P30_DataSet(chip);
            }
            if(TP2830 == id[chip] || TP2831 == id[chip]) // Added TP2830/31 ID - 10/15/2018
            {
                TP2826_C1080P30_DataSet(chip);
            }
            if(STD_HDC == std) //HDC 1080p30 position adjust
               {
                    tp28xx_byte_write(chip, 0x15, 0x13);
                    tp28xx_byte_write(chip, 0x16, 0x44);
                    if(TP2827 == id[chip] || TP2826 == id[chip] || TP2816 == id[chip] || TP2827C == id[chip] || TP2826C == id[chip] || TP2828 == id[chip] || TP2829 == id[chip] || TP2830 == id[chip] || TP2831 == id[chip] ) // Added TP2830/31 ID - 10/15/2018
                    {
                        tp28xx_byte_write(chip, 0x15, 0x13);
                        tp28xx_byte_write(chip, 0x16, 0x60);
                        tp28xx_byte_write(chip, 0x17, 0x80);
                        tp28xx_byte_write(chip, 0x18, 0x29);
                        tp28xx_byte_write(chip, 0x19, 0x38);
                        tp28xx_byte_write(chip, 0x1A, 0x47);
                        tp28xx_byte_write(chip, 0x1C, 0x09);
                        tp28xx_byte_write(chip, 0x1D, 0x60);
                    }
               }
        }

        tp282x_SYSCLK_V1(chip, ch);
        break;

    case TP2802_HALF720P25:
        //tp28xx_byte_write(chip, 0x35, 0x45);
    case TP2802_720P25:
        if(id[chip] >= TP2822) tp28xx_byte_write(chip, 0x35, 0x05);
        tp2802_set_work_mode_720p25(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tmp |=0x02;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);
        if(TP2823 == id[chip]) TP2823_V1_DataSet(chip);
        if(TP2834 == id[chip]) TP2834_V1_DataSet(chip);
        if(TP2833 == id[chip]) TP2833_V1_DataSet(chip);
        if(TP2853C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2833C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2823C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2826  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2816  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2827  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2828  == id[chip]) TP2829_V1_DataSet(chip);
        if(TP2830  == id[chip]) TP2831_V1_DataSet(chip); // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) TP2831_V1_DataSet(chip); // Added TP2831 ID - 10/15/2018

        tp282x_SYSCLK_V1(chip, ch);
        break;

    case TP2802_HALF720P30:
        //tp28xx_byte_write(chip, 0x35, 0x45);
    case TP2802_720P30:
        if(id[chip] >= TP2822) tp28xx_byte_write(chip, 0x35, 0x05);
        tp2802_set_work_mode_720p30(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tmp |=0x02;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);
        if(TP2823 == id[chip]) TP2823_V1_DataSet(chip);
        if(TP2834 == id[chip]) TP2834_V1_DataSet(chip);
        if(TP2833 == id[chip]) TP2833_V1_DataSet(chip);
        if(TP2853C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2833C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2823C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2826  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2816  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2827  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2828  == id[chip]) TP2829_V1_DataSet(chip);
        if(TP2830  == id[chip]) TP2831_V1_DataSet(chip); // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) TP2831_V1_DataSet(chip); // Added TP2831 ID - 10/15/2018

        tp282x_SYSCLK_V1(chip, ch);
        break;

    case TP2802_HALF720P50:
        //tp28xx_byte_write(chip, 0x35, 0x45);
    case TP2802_720P50:
        if(id[chip] >= TP2822) tp28xx_byte_write(chip, 0x35, 0x05);
        tp2802_set_work_mode_720p50(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tmp |=0x02;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);
        if(TP2823 == id[chip]) TP2823_V1_DataSet(chip);
        if(TP2834 == id[chip]) TP2834_V1_DataSet(chip);
        if(TP2833 == id[chip]) TP2833_V1_DataSet(chip);
        if(TP2853C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2833C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2823C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2826  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2816  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2827  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2828  == id[chip]) TP2829_V1_DataSet(chip);
        if(TP2830  == id[chip]) TP2831_V1_DataSet(chip); // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) TP2831_V1_DataSet(chip); // Added TP2831 ID - 10/15/2018

        if(STD_HDA == std)
        {

        }
        else if(STD_HDC == std || STD_HDC_DEFAULT == std)
        {
            if(TP2853C == id[chip])
            {
                TP2853C_C720P50_DataSet(chip);
            }
            if(TP2833C == id[chip])
            {
                TP2853C_C720P50_DataSet(chip);
            }
            if(TP2823C == id[chip])
            {
                TP2853C_C720P50_DataSet(chip);
            }
            if(TP2833 == id[chip])
            {
                TP2853C_C720P50_DataSet(chip);
            }
            if(TP2834 == id[chip])
            {
                TP2834_C720P50_DataSet(chip);
            }
            if(TP2826 == id[chip])
            {
                TP2826_C720P50_DataSet(chip);
            }
            if(TP2816 == id[chip])
            {
                TP2826_C720P50_DataSet(chip);
            }
            if(TP2827 == id[chip])
            {
                TP2826_C720P50_DataSet(chip);
            }
            if(TP2827C == id[chip] || TP2826C == id[chip])
            {
                TP2826_C720P50_DataSet(chip);
            }
            if(TP2828 == id[chip] || TP2829 == id[chip])
            {
                TP2826_C720P50_DataSet(chip);
            }
            if(TP2830 == id[chip] || TP2831 == id[chip]) // Added TP2830/31 ID - 10/15/2018
            {
                TP2826_C720P50_DataSet(chip);
            }
            if(STD_HDC == std) //HDC 720p50 position adjust
               {
                    tp28xx_byte_write(chip, 0x16, 0x40);
                    if(TP2827 == id[chip] || TP2826 == id[chip] || TP2816 == id[chip] || TP2827C == id[chip] || TP2826C == id[chip] || TP2828 == id[chip] || TP2829 == id[chip] || TP2828 == id[chip] || TP2829 == id[chip] ) // Added TP2830/31 ID - 10/15/2018
                    {
                        tp28xx_byte_write(chip, 0x15, 0x13);
                        tp28xx_byte_write(chip, 0x16, 0x0a);
                        tp28xx_byte_write(chip, 0x17, 0x00);
                        tp28xx_byte_write(chip, 0x18, 0x19);
                        tp28xx_byte_write(chip, 0x19, 0xd0);
                        tp28xx_byte_write(chip, 0x1A, 0x25);
                        tp28xx_byte_write(chip, 0x1C, 0x06);
                        tp28xx_byte_write(chip, 0x1D, 0x7a);
                    }
               }
        }

        tp282x_SYSCLK_V1(chip, ch);
        break;

    case TP2802_HALF720P60:
        //tp28xx_byte_write(chip, 0x35, 0x45);
    case TP2802_720P60:
        if(id[chip] >= TP2822) tp28xx_byte_write(chip, 0x35, 0x05);
        tp2802_set_work_mode_720p60(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tmp |=0x02;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);
        if(TP2823 == id[chip]) TP2823_V1_DataSet(chip);
        if(TP2834 == id[chip]) TP2834_V1_DataSet(chip);
        if(TP2833 == id[chip]) TP2833_V1_DataSet(chip);
        if(TP2853C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2833C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2823C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2826  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2816  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2827  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2828  == id[chip]) TP2829_V1_DataSet(chip);
        if(TP2830  == id[chip]) TP2831_V1_DataSet(chip); // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) TP2831_V1_DataSet(chip); // Added TP2831 ID - 10/15/2018

        if(STD_HDA == std)
        {
            if(TP2853C == id[chip])
            {
                TP2853C_A720P60_DataSet(chip);
            }
            if(TP2833C == id[chip])
            {
                TP2853C_A720P60_DataSet(chip);
            }
            if(TP2823C == id[chip])
            {
                TP2853C_A720P60_DataSet(chip);
            }
            if(TP2833 == id[chip])
            {
                TP2833_A720P60_DataSet(chip);
            }
            if(TP2834 == id[chip])
            {
                TP2833_A720P60_DataSet(chip);
            }
            if(TP2826 == id[chip])
            {
                TP2826_A720P60_DataSet(chip);
            }
            if(TP2816 == id[chip])
            {
                TP2826_A720P60_DataSet(chip);
            }
            if(TP2827 == id[chip])
            {
                TP2826_A720P60_DataSet(chip);
            }
            if(TP2827C == id[chip] || TP2826C == id[chip])
            {
                TP2826_A720P60_DataSet(chip);
            }
            if(TP2828 == id[chip] || TP2829 == id[chip])
            {
                TP2826_A720P60_DataSet(chip);
            }
            if(TP2830 == id[chip] || TP2831 == id[chip]) // Added TP2830/31 ID - 10/15/2018
            {
                TP2826_A720P60_DataSet(chip);
            }
        }
        else if(STD_HDC == std || STD_HDC_DEFAULT == std)
        {
            if(TP2853C == id[chip])
            {
                TP2853C_C720P60_DataSet(chip);
            }
            if(TP2833C == id[chip])
            {
                TP2853C_C720P60_DataSet(chip);
            }
            if(TP2823C == id[chip])
            {
                TP2853C_C720P60_DataSet(chip);
            }
            if(TP2833 == id[chip])
            {
                TP2853C_C720P60_DataSet(chip);
            }
            if(TP2834 == id[chip])
            {
                TP2834_C720P60_DataSet(chip);
            }
            if(TP2826 == id[chip])
            {
                TP2826_C720P60_DataSet(chip);
            }
            if(TP2816 == id[chip])
            {
                TP2826_C720P60_DataSet(chip);
            }
            if(TP2827 == id[chip])
            {
                TP2826_C720P60_DataSet(chip);
            }
            if(TP2827C == id[chip] || TP2826C == id[chip])
            {
                TP2826_C720P60_DataSet(chip);
            }
            if(TP2828 == id[chip] || TP2829 == id[chip])
            {
                TP2826_C720P60_DataSet(chip);
            }
            if(TP2830 == id[chip] || TP2831 == id[chip]) // Added TP2830/31 ID - 10/15/2018
            {
                TP2826_C720P60_DataSet(chip);
            }
            if(STD_HDC == std) //HDC 720p60 position adjust
               {
                    tp28xx_byte_write(chip, 0x16, 0x02);
                    if(TP2827 == id[chip] || TP2826 == id[chip] || TP2816 == id[chip] || TP2827C == id[chip] || TP2826C == id[chip] || TP2828 == id[chip] || TP2829 == id[chip] || TP2830 == id[chip] || TP2831 == id[chip] ) // Added TP2830/31 ID - 10/15/2018
                    {
                        tp28xx_byte_write(chip, 0x15, 0x13);
                        tp28xx_byte_write(chip, 0x16, 0x08);
                        tp28xx_byte_write(chip, 0x17, 0x00);
                        tp28xx_byte_write(chip, 0x18, 0x19);
                        tp28xx_byte_write(chip, 0x19, 0xd0);
                        tp28xx_byte_write(chip, 0x1A, 0x25);
                        tp28xx_byte_write(chip, 0x1C, 0x06);
                        tp28xx_byte_write(chip, 0x1D, 0x72);
                    }
               }

        }

        tp282x_SYSCLK_V1(chip, ch);
        break;

    case TP2802_720P30V2:
        if(id[chip] >= TP2822) tp28xx_byte_write(chip, 0x35, 0x25);
        tp2802_set_work_mode_720p60(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tmp |=0x02;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp |= SYS_MODE[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);
        if(TP2823 == id[chip]) TP2823_V2_DataSet(chip);
        if(TP2834 == id[chip]) TP2834_V2_DataSet(chip);
        if(TP2833 == id[chip]) TP2833_V2_DataSet(chip);
        if(TP2853C == id[chip]) TP2853C_V2_DataSet(chip);
        if(TP2833C == id[chip]) TP2853C_V2_DataSet(chip);
        if(TP2823C == id[chip]) TP2853C_V2_DataSet(chip);
        if(TP2826  == id[chip]) TP2826_V2_DataSet(chip);
        if(TP2816  == id[chip]) TP2826_V2_DataSet(chip);
        if(TP2827  == id[chip]) TP2826_V2_DataSet(chip);
        if(TP2828  == id[chip]) TP2829_V2_DataSet(chip);
        if(TP2830  == id[chip]) TP2831_V2_DataSet(chip); // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) TP2831_V2_DataSet(chip); // Added TP2831 ID - 10/15/2018

        if(STD_HDA == std)
        {
            if(TP2834 == id[chip])
            {
                TP2833_A720N_DataSet(chip);
            }
            if(TP2833 == id[chip])
            {
                TP2833_A720N_DataSet(chip);
            }
            if(TP2853C == id[chip])
            {
                TP2853C_A720P30_DataSet(chip);
            }
            if(TP2833C == id[chip])
            {
                TP2853C_A720P30_DataSet(chip);
            }
            if(TP2823C == id[chip])
            {
                TP2853C_A720P30_DataSet(chip);
            }
            if(TP2826 == id[chip])
            {
                TP2826_A720P30_DataSet(chip);
            }
            if(TP2816 == id[chip])
            {
                TP2826_A720P30_DataSet(chip);
            }
            if(TP2827 == id[chip])
            {
                TP2826_A720P30_DataSet(chip);
            }
            if(TP2828 == id[chip] || TP2829 == id[chip])
            {
                TP2829_A720P30_DataSet(chip);
            }
            if(TP2830 == id[chip] || TP2831 == id[chip]) // Added TP2830/31 ID - 10/15/2018
            {
                TP2831_A720P30_DataSet(chip);
            }
        }
        else if(STD_HDC == std || STD_HDC_DEFAULT == std)
        {
            if(TP2853C == id[chip])
            {
                TP2853C_C720P30_DataSet(chip);
            }
            if(TP2833C == id[chip])
            {
                TP2853C_C720P30_DataSet(chip);
            }
            if(TP2823C == id[chip])
            {
                TP2853C_C720P30_DataSet(chip);
            }
            if(TP2833 == id[chip])
            {
                TP2853C_C720P30_DataSet(chip);
            }
            if(TP2834 == id[chip])
            {
                TP2834_C720P30_DataSet(chip);
            }
            if(TP2826 == id[chip])
            {
                TP2826_C720P30_DataSet(chip);
            }
            if(TP2816 == id[chip])
            {
                TP2826_C720P30_DataSet(chip);
            }
            if(TP2827 == id[chip])
            {
                TP2826_C720P30_DataSet(chip);
            }
            if(TP2827C == id[chip] || TP2826C == id[chip])
            {
                TP2826_C720P30_DataSet(chip);
            }
            if(TP2828 == id[chip] || TP2829 == id[chip])
            {
                TP2826_C720P30_DataSet(chip);
            }
            if(TP2830 == id[chip] || TP2831 == id[chip]) // Added TP2830/31 ID - 10/15/2018
            {
                TP2826_C720P30_DataSet(chip);
            }
            if(STD_HDC == std) //HDC 720p30 position adjust
               {
                    tp28xx_byte_write(chip, 0x16, 0x02);
                    if(TP2827 == id[chip] || TP2826 == id[chip] || TP2816 == id[chip] || TP2827C == id[chip] || TP2826C == id[chip] || TP2828 == id[chip] || TP2829 == id[chip] || TP2830 == id[chip] || TP2831 == id[chip] ) // Added TP2830/31 ID - 10/15/2018
                    {
                        tp28xx_byte_write(chip, 0x15, 0x13);
                        tp28xx_byte_write(chip, 0x16, 0x08);
                        tp28xx_byte_write(chip, 0x17, 0x00);
                        tp28xx_byte_write(chip, 0x18, 0x19);
                        tp28xx_byte_write(chip, 0x19, 0xd0);
                        tp28xx_byte_write(chip, 0x1A, 0x25);
                        tp28xx_byte_write(chip, 0x1C, 0x06);
                        tp28xx_byte_write(chip, 0x1D, 0x72);
                    }
               }
        }

        tp282x_SYSCLK_V2(chip, ch);
        break;

    case TP2802_720P25V2:
        if(id[chip] >= TP2822) tp28xx_byte_write(chip, 0x35, 0x25);
        tp2802_set_work_mode_720p50(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tmp |=0x02;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp |= SYS_MODE[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);
        if(TP2823 == id[chip]) TP2823_V2_DataSet(chip);
        if(TP2834 == id[chip]) TP2834_V2_DataSet(chip);
        if(TP2833 == id[chip]) TP2833_V2_DataSet(chip);
        if(TP2853C == id[chip]) TP2853C_V2_DataSet(chip);
        if(TP2833C == id[chip]) TP2853C_V2_DataSet(chip);
        if(TP2823C == id[chip]) TP2853C_V2_DataSet(chip);
        if(TP2826  == id[chip]) TP2826_V2_DataSet(chip);
        if(TP2816  == id[chip]) TP2826_V2_DataSet(chip);
        if(TP2827  == id[chip]) TP2826_V2_DataSet(chip);
        if(TP2828  == id[chip]) TP2829_V2_DataSet(chip);
        if(TP2830  == id[chip]) TP2831_V2_DataSet(chip); // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) TP2831_V2_DataSet(chip); // Added TP2831 ID - 10/15/2018

        if(STD_HDA == std)
        {
            if(TP2834 == id[chip])
            {
                TP2833_A720P_DataSet(chip);
            }
            if(TP2833 == id[chip])
            {
                TP2833_A720P_DataSet(chip);
            }
            if(TP2853C == id[chip])
            {
                TP2853C_A720P25_DataSet(chip);
            }
            if(TP2833C == id[chip])
            {
                TP2853C_A720P25_DataSet(chip);
            }
            if(TP2823C == id[chip])
            {
                TP2853C_A720P25_DataSet(chip);
            }
            if(TP2826 == id[chip])
            {
                TP2826_A720P25_DataSet(chip);
            }
            if(TP2816 == id[chip])
            {
                TP2826_A720P25_DataSet(chip);
            }
            if(TP2827 == id[chip])
            {
                TP2826_A720P25_DataSet(chip);
            }
            if(TP2828 == id[chip] || TP2829 == id[chip])
            {
                TP2829_A720P25_DataSet(chip);
            }
            if(TP2830 == id[chip] || TP2831 == id[chip]) // Added TP2830/31 ID - 10/15/2018
            {
                TP2831_A720P25_DataSet(chip);
            }
        }
        else if(STD_HDC == std || STD_HDC_DEFAULT == std)
        {
            if(TP2853C == id[chip])
            {
                TP2853C_C720P25_DataSet(chip);
            }
            if(TP2833C == id[chip])
            {
                TP2853C_C720P25_DataSet(chip);
            }
            if(TP2823C == id[chip])
            {
                TP2853C_C720P25_DataSet(chip);
            }
            if(TP2833 == id[chip])
            {
                TP2853C_C720P25_DataSet(chip);
            }
            if(TP2834 == id[chip])
            {
                TP2834_C720P25_DataSet(chip);
            }
            if(TP2826 == id[chip])
            {
                TP2826_C720P25_DataSet(chip);
            }
            if(TP2816 == id[chip])
            {
                TP2826_C720P25_DataSet(chip);
            }
            if(TP2827 == id[chip])
            {
                TP2826_C720P25_DataSet(chip);
            }
            if(TP2827C == id[chip] || TP2826C == id[chip])
            {
                TP2826_C720P25_DataSet(chip);
            }
            if(TP2828 == id[chip] || TP2829 == id[chip])
            {
                TP2826_C720P25_DataSet(chip);
            }
            if(TP2830 == id[chip] || TP2831 == id[chip]) // Added TP2830/31 ID - 10/15/2018
            {
                TP2826_C720P25_DataSet(chip);
            }
            if(STD_HDC == std) //HDC 720p25 position adjust
               {
                    tp28xx_byte_write(chip, 0x16, 0x40);
                    if(TP2827 == id[chip] || TP2826 == id[chip] || TP2816 == id[chip] || TP2827C == id[chip] || TP2826C == id[chip] || TP2828 == id[chip] || TP2829 == id[chip] || TP2830 == id[chip] || TP2831 == id[chip] ) // Added TP2830/31 ID - 10/15/2018
                    {
                        tp28xx_byte_write(chip, 0x15, 0x13);
                        tp28xx_byte_write(chip, 0x16, 0x0a);
                        tp28xx_byte_write(chip, 0x17, 0x00);
                        tp28xx_byte_write(chip, 0x18, 0x19);
                        tp28xx_byte_write(chip, 0x19, 0xd0);
                        tp28xx_byte_write(chip, 0x1A, 0x25);
                        tp28xx_byte_write(chip, 0x1C, 0x06);
                        tp28xx_byte_write(chip, 0x1D, 0x7a);
                    }
               }

        }

        tp282x_SYSCLK_V2(chip, ch);
        break;

    case TP2802_PAL:
        tp28xx_byte_write(chip, 0x35, 0x25);
        tp2802_set_work_mode_PAL(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tmp |=0x07;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp |= SYS_MODE[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);
        if(TP2823 == id[chip]) TP2823_PAL_DataSet(chip);
        if(TP2834 == id[chip]) TP2834_PAL_DataSet(chip);
        if(TP2833 == id[chip]) TP2833_PAL_DataSet(chip);
        if(TP2853C == id[chip]) TP2853C_PAL_DataSet(chip);
        if(TP2833C == id[chip]) TP2853C_PAL_DataSet(chip);
        if(TP2823C == id[chip]) TP2853C_PAL_DataSet(chip);
        if(TP2826  == id[chip]) TP2826_PAL_DataSet(chip);
        if(TP2816  == id[chip]) TP2826_PAL_DataSet(chip);
        if(TP2827  == id[chip]) TP2826_PAL_DataSet(chip);
        if(TP2828  == id[chip]) TP2829_PAL_DataSet(chip);
        if(TP2830  == id[chip]) TP2831_PAL_DataSet(chip); // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) TP2831_PAL_DataSet(chip); // Added TP2831 ID - 10/15/2018

        tp282x_SYSCLK_V2(chip, ch);
        break;

    case TP2802_NTSC:
        tp28xx_byte_write(chip, 0x35, 0x25);
        tp2802_set_work_mode_NTSC(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tmp |=0x07;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp |= SYS_MODE[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);
        if(TP2823 == id[chip]) TP2823_NTSC_DataSet(chip);
        if(TP2834 == id[chip]) TP2834_NTSC_DataSet(chip);
        if(TP2833 == id[chip]) TP2833_NTSC_DataSet(chip);
        if(TP2853C == id[chip]) TP2853C_NTSC_DataSet(chip);
        if(TP2833C == id[chip]) TP2853C_NTSC_DataSet(chip);
        if(TP2823C == id[chip]) TP2853C_NTSC_DataSet(chip);
        if(TP2826  == id[chip]) TP2826_NTSC_DataSet(chip);
        if(TP2816  == id[chip]) TP2826_NTSC_DataSet(chip);
        if(TP2827  == id[chip]) TP2826_NTSC_DataSet(chip);
        if(TP2828  == id[chip]) TP2829_NTSC_DataSet(chip);
        if(TP2830  == id[chip]) TP2831_NTSC_DataSet(chip); // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) TP2831_NTSC_DataSet(chip); // Added TP2831 ID - 10/15/2018

        tp282x_SYSCLK_V2(chip, ch);
        break;

    case TP2802_3M18:
        tp28xx_byte_write(chip, 0x35, 0x16);
        tp28xx_byte_write(chip, 0x36, 0x30);
        tp2802_set_work_mode_3M(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);
        if(TP2834 == id[chip]) TP2834_V1_DataSet(chip);
        if(TP2833 == id[chip]) TP2833_V1_DataSet(chip);
        if(TP2853C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2826  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2827  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2828  == id[chip]) TP2829_V1_DataSet(chip);
        if(TP2830  == id[chip]) TP2831_V1_DataSet(chip); // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) TP2831_V1_DataSet(chip); // Added TP2831 ID - 10/15/2018

        tp282x_SYSCLK_V1(chip, ch);
        break;

    case TP2802_5M12:
        tp28xx_byte_write(chip, 0x35, 0x17);
        tp28xx_byte_write(chip, 0x36, 0xD0);
        tp2802_set_work_mode_5M(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);
        if(TP2834 == id[chip]) TP2834_V1_DataSet(chip);
        if(TP2833 == id[chip]) TP2833_V1_DataSet(chip);
        if(TP2853C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2826  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2827  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2828  == id[chip]) TP2829_V1_DataSet(chip);
        if(TP2830  == id[chip]) TP2831_5MP12_DataSet(chip); // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) TP2831_5MP12_DataSet(chip); // Added TP2831 ID - 10/15/2018

        if( STD_HDA == std)
        {
            if(TP2828 == id[chip] || TP2829 == id[chip])
            {
                TP2829_A5MP12_DataSet(chip);
            }
            if(TP2830 == id[chip] || TP2831 == id[chip]) // Added TP2830/31 ID - 10/15/2018
            {
                TP2831_A5MP12_DataSet(chip);
            }
        }
        tp282x_SYSCLK_V1(chip, ch);
        break;

    case TP2802_4M15:
        tp28xx_byte_write(chip, 0x35, 0x16);
        tp28xx_byte_write(chip, 0x36, 0x72);
        tp2802_set_work_mode_4M(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);
        if(TP2834 == id[chip]) TP2834_V1_DataSet(chip);
        if(TP2833 == id[chip]) TP2833_V1_DataSet(chip);
        if(TP2853C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2826  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2827  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2828  == id[chip]) TP2829_V1_DataSet(chip);
        if(TP2830  == id[chip]) TP2831_4MP15_DataSet(chip); // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) TP2831_4MP15_DataSet(chip); // Added TP2831 ID - 10/15/2018

        tp282x_SYSCLK_V1(chip, ch);
        break;

    case TP2802_3M20:
        tp28xx_byte_write(chip, 0x35, 0x16);
        tp28xx_byte_write(chip, 0x36, 0x72);
        tp2802_set_work_mode_3M20(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);
        if(TP2834 == id[chip]) TP2834_V1_DataSet(chip);
        if(TP2833 == id[chip]) TP2833_V1_DataSet(chip);
        if(TP2853C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2826  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2827  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2828  == id[chip]) TP2829_V1_DataSet(chip);
        if(TP2830  == id[chip]) TP2831_V1_DataSet(chip); // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) TP2831_V1_DataSet(chip); // Added TP2831 ID - 10/15/2018

        tp28xx_byte_write(chip, 0x2d, 0x26);

        tp282x_SYSCLK_V1(chip, ch);
        break;

    case TP2802_4M12:
        tp28xx_byte_write(chip, 0x35, 0x17);
        tp28xx_byte_write(chip, 0x36, 0x08);
        tp2802_set_work_mode_4M12(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);
        if(TP2834 == id[chip]) TP2834_V1_DataSet(chip);
        if(TP2833 == id[chip]) TP2833_V1_DataSet(chip);
        if(TP2853C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2826  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2827  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2828  == id[chip]) TP2829_V1_DataSet(chip);
        if(TP2830  == id[chip]) TP2831_V1_DataSet(chip); // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) TP2831_V1_DataSet(chip); // Added TP2831 ID - 10/15/2018

        tp282x_SYSCLK_V1(chip, ch);
        break;

    case TP2802_6M10:
        tp28xx_byte_write(chip, 0x35, 0x17);
        tp28xx_byte_write(chip, 0x36, 0xbc);
        tp2802_set_work_mode_6M10(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);
        if(TP2834 == id[chip]) TP2834_V1_DataSet(chip);
        if(TP2833 == id[chip]) TP2833_V1_DataSet(chip);
        if(TP2853C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2826  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2827  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2828  == id[chip]) TP2829_V1_DataSet(chip);
        if(TP2830  == id[chip]) TP2831_V1_DataSet(chip); // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) TP2831_V1_DataSet(chip); // Added TP2831 ID - 10/15/2018

        tp282x_SYSCLK_V1(chip, ch);
        break;

    case TP2802_QHD30:
        tp28xx_byte_write(chip, 0x35, 0x15);
        tp28xx_byte_write(chip, 0x36, 0xdc);
        //tp2802_set_work_mode_4MH30(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);

        if(TP2828  == id[chip]) {tp2802_set_work_mode_QHD30(chip); TP2829_QHDP30_25_DataSet(chip);}
        if(TP2830  == id[chip]) {tp2802_set_work_mode_QHD30(chip); TP2831_QHDP30_25_DataSet(chip);} // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) {tp2802_set_work_mode_QHD30(chip); TP2831_QHDP30_25_DataSet(chip);} // Added TP2831 ID - 10/15/2018

        if( STD_HDA == std)
        {
            if(TP2828 == id[chip]) TP2829_AQHDP30_DataSet(chip);
            if(TP2830 == id[chip]) TP2831_AQHDP30_DataSet(chip); // Added TP2830 ID - 10/15/2018
            if(TP2831  == id[chip]) TP2831_AQHDP30_DataSet(chip); // Added TP2831 ID - 10/15/2018
        }
        else if(STD_HDC == std || STD_HDC_DEFAULT == std)
        {
            if(TP2828  == id[chip]) TP2829_CQHDP30_DataSet(chip);
            if(TP2830  == id[chip]) TP2831_CQHDP30_DataSet(chip); // Added TP2830 ID - 10/15/2018
            if(TP2831  == id[chip]) TP2831_CQHDP30_DataSet(chip); // Added TP2831 ID - 10/15/2018
        }

        tp282x_SYSCLK_V3(chip, ch);
        break;
    case TP2802_QHD25:
        tp28xx_byte_write(chip, 0x35, 0x15);
        tp28xx_byte_write(chip, 0x36, 0xdc);
        //tp2802_set_work_mode_4MH25(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);

        if(TP2828  == id[chip]) {tp2802_set_work_mode_QHD25(chip); TP2829_QHDP30_25_DataSet(chip);}
        if(TP2830  == id[chip]) {tp2802_set_work_mode_QHD25(chip); TP2831_QHDP30_25_DataSet(chip);} // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) {tp2802_set_work_mode_QHD25(chip); TP2831_QHDP30_25_DataSet(chip);} // Added TP2831 ID - 10/15/2018
        if( STD_HDA == std)
        {
            if(TP2828  == id[chip]) TP2829_AQHDP25_DataSet(chip);
            if(TP2830  == id[chip]) TP2831_AQHDP25_DataSet(chip); // Added TP2830 ID - 10/15/2018
            if(TP2831  == id[chip]) TP2831_AQHDP25_DataSet(chip); // Added TP2831 ID - 10/15/2018
        }
        else if(STD_HDC == std || STD_HDC_DEFAULT == std)
        {
            if(TP2828  == id[chip]) TP2829_CQHDP25_DataSet(chip);
            if(TP2830  == id[chip]) TP2831_CQHDP25_DataSet(chip); // Added TP2830 ID - 10/15/2018
            if(TP2831  == id[chip]) TP2831_CQHDP25_DataSet(chip); // Added TP2831 ID - 10/15/2018
        }

        tp282x_SYSCLK_V3(chip, ch);
        break;
    case TP2802_QHD15:
        tp28xx_byte_write(chip, 0x35, 0x15);
        tp28xx_byte_write(chip, 0x36, 0xdc);
        tp2802_set_work_mode_QHD15(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);
        if(TP2834 == id[chip]) TP2834_V1_DataSet(chip);
        if(TP2833 == id[chip]) TP2833_V1_DataSet(chip);
        if(TP2853C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2826  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2827  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2828  == id[chip]) TP2829_V1_DataSet(chip);
        if(TP2830  == id[chip]) TP2831_V1_DataSet(chip); // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) TP2831_V1_DataSet(chip); // Added TP2831 ID - 10/15/2018
        if( STD_HDA == std)
        {
            if(TP2828 == id[chip] || TP2829 == id[chip])
            {
                TP2829_AQHDP15_DataSet(chip);
            }
            if(TP2830 == id[chip] || TP2831 == id[chip]) // Added TP2830/31 ID - 10/15/2018
            {
                TP2831_AQHDP15_DataSet(chip);
            }
        }
        tp282x_SYSCLK_V1(chip, ch);
        break;
    case TP2802_QXGA18:
        tp28xx_byte_write(chip, 0x35, 0x16);
        tp28xx_byte_write(chip, 0x36, 0x72);
        tp2802_set_work_mode_3M(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);
        if(TP2834 == id[chip]) TP2834_V1_DataSet(chip);
        if(TP2833 == id[chip]) TP2833_V1_DataSet(chip);
        if(TP2853C == id[chip]) TP2853C_V1_DataSet(chip);
        if(TP2826  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2827  == id[chip]) TP2826_V1_DataSet(chip);
        if(TP2828  == id[chip]) TP2829_V1_DataSet(chip);
        if(TP2830  == id[chip]) TP2831_V1_DataSet(chip); // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) TP2831_V1_DataSet(chip); // Added TP2831 ID - 10/15/2018
        if( STD_HDA == std )
        {
            if(TP2828 == id[chip] || TP2829 == id[chip])
            {
                TP2829_AQXGAP18_DataSet(chip);
            }
            if(TP2830 == id[chip] || TP2831 == id[chip]) // Added TP2830/31 ID - 10/15/2018
            {
                TP2831_AQXGAP18_DataSet(chip);
            }
        }
        tp282x_SYSCLK_V1(chip, ch);
        break;
    case TP2802_QXGA25:
        tp28xx_byte_write(chip, 0x35, 0x16);
        tp28xx_byte_write(chip, 0x36, 0x72);
        //tp2802_set_work_mode_QXGAH25(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);

        if(TP2853C == id[chip]) {tp2802_set_work_mode_QXGAH25(chip); TP2853C_V1_DataSet(chip);}
        if(TP2828  == id[chip]) {tp2802_set_work_mode_QXGA25(chip); TP2829_AQXGAP25_DataSet(chip);}
        if(TP2830  == id[chip]) {tp2802_set_work_mode_QXGA25(chip); TP2831_AQXGAP25_DataSet(chip);} // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) {tp2802_set_work_mode_QXGA25(chip); TP2831_AQXGAP25_DataSet(chip);} // Added TP2831 ID - 10/15/2018

        tp282x_SYSCLK_V3(chip, ch);
        break;
    case TP2802_QXGA30:
        tp28xx_byte_write(chip, 0x35, 0x16);
        tp28xx_byte_write(chip, 0x36, 0x71);
        //tp2802_set_work_mode_QXGAH30(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);

        if(TP2853C == id[chip]) {tp2802_set_work_mode_QXGAH30(chip); TP2853C_V1_DataSet(chip);}
        if(TP2828  == id[chip]) {tp2802_set_work_mode_QXGA30(chip); TP2829_AQXGAP30_DataSet(chip);}
        if(TP2830  == id[chip]) {tp2802_set_work_mode_QXGA30(chip); TP2831_AQXGAP30_DataSet(chip);} // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) {tp2802_set_work_mode_QXGA30(chip); TP2831_AQXGAP30_DataSet(chip);} // Added TP2831 ID - 10/15/2018

        tp282x_SYSCLK_V3(chip, ch);
        break;

    case TP2802_5M20:
        tp28xx_byte_write(chip, 0x35, 0x17);
        tp28xx_byte_write(chip, 0x36, 0xbc);
        //tp2802_set_work_mode_QXGAH30(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);

        if(TP2828  == id[chip]) {tp2802_set_work_mode_5M20(chip); TP2829_5MP20_DataSet(chip);}
        if(TP2830  == id[chip]) {tp2802_set_work_mode_5M20(chip); TP2831_5MP20_DataSet(chip);} // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) {tp2802_set_work_mode_5M20(chip); TP2831_5MP20_DataSet(chip);} // Added TP2831 ID - 10/15/2018
        if( STD_HDA == std)
        {
            if(TP2828  == id[chip]) TP2829_A5MP20_DataSet(chip);
            if(TP2830  == id[chip]) TP2831_A5MP20_DataSet(chip);
            if(TP2831  == id[chip]) TP2831_A5MP20_DataSet(chip);
        }
        else if(STD_HDC == std || STD_HDC_DEFAULT == std)
        {
            if(TP2826  == id[chip]) TP2827C_C5MP20_DataSet(chip);
            if(TP2827C  == id[chip]) TP2827C_C5MP20_DataSet(chip);
            if(TP2826C  == id[chip]) TP2827C_C5MP20_DataSet(chip);
            if(TP2828  == id[chip]) TP2827C_C5MP20_DataSet(chip);
            if(TP2829  == id[chip]) TP2827C_C5MP20_DataSet(chip);
            if(TP2830  == id[chip]) TP2827C_C5MP20_DataSet(chip);
            if(TP2831  == id[chip]) TP2827C_C5MP20_DataSet(chip);
        }

        tp282x_SYSCLK_V3(chip, ch);

        break;

    case TP2802_8M15:
        tp28xx_byte_write(chip, 0x35, 0x18);
        tp28xx_byte_write(chip, 0x36, 0xca);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);

        if(TP2828  == id[chip]) {tp2802_set_work_mode_8M15(chip); TP2829_8MP15_DataSet(chip);}
        if(TP2830  == id[chip]) {tp2802_set_work_mode_8M15(chip); TP2831_8MP15_DataSet(chip);} // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) {tp2802_set_work_mode_8M15(chip); TP2831_8MP15_DataSet(chip);} // Added TP2831 ID - 10/15/2018
        if( STD_HDA == std)
        {
            if(TP2828  == id[chip]) TP2829_A8MP15_DataSet(chip);
            if(TP2830  == id[chip]) TP2831_A8MP15_DataSet(chip); // Added TP2830 ID - 10/15/2018
            if(TP2831  == id[chip]) TP2831_A8MP15_DataSet(chip); // Added TP2831 ID - 10/15/2018
        }
        else if(STD_HDC_DEFAULT == std)
        {
            if(TP2828  == id[chip]) TP2829_C8MP15_DataSet(chip);
            if(TP2830  == id[chip]) TP2831_C8MP15_DataSet(chip); // Added TP2830 ID - 10/15/2018
            if(TP2831  == id[chip]) TP2831_C8MP15_DataSet(chip); // Added TP2831 ID - 10/15/2018
        }
        else if(STD_HDC == std)
        {
            if(TP2828  == id[chip] || TP2829  == id[chip])
            {
                TP2829_C8MP15_DataSet(chip);
            }
            if(TP2830  == id[chip] || TP2831  == id[chip]) // Added TP2830/31 ID - 10/15/2018
            {
                TP2831_C8MP15_DataSet(chip);
                tp28xx_byte_write(chip, 0x15, 0x33);
                tp28xx_byte_write(chip, 0x16, 0x58);
                tp28xx_byte_write(chip, 0x1c, 0x11);
                tp28xx_byte_write(chip, 0x1d, 0x2e);
            }
        }
        tp282x_SYSCLK_V3(chip, ch);
        break;

    case TP2802_8M12:
        tp28xx_byte_write(chip, 0x35, 0x18);
        tp28xx_byte_write(chip, 0x36, 0xca);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);

        if(TP2828  == id[chip]) {tp2802_set_work_mode_8M12(chip); TP2829_8MP15_DataSet(chip);}
        if(TP2830  == id[chip]) {tp2802_set_work_mode_8M12(chip); TP2831_8MP15_DataSet(chip);} // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) {tp2802_set_work_mode_8M12(chip); TP2831_8MP15_DataSet(chip);} // Added TP2831 ID - 10/15/2018
        if( STD_HDA == std)
        {

        }
        else if(STD_HDC_DEFAULT == std)
        {
            if(TP2828  == id[chip]) TP2829_C8MP12_DataSet(chip);
            if(TP2830  == id[chip]) TP2831_C8MP12_DataSet(chip); // Added TP2830 ID - 10/15/2018
            if(TP2831  == id[chip]) TP2831_C8MP12_DataSet(chip); // Added TP2831 ID - 10/15/2018
        }
        else if(STD_HDC == std)
        {
            if(TP2828  == id[chip] || TP2829  == id[chip])
            {
                TP2829_C8MP12_DataSet(chip);
                tp28xx_byte_write(chip, 0x1c, 0x13);
                tp28xx_byte_write(chip, 0x1d, 0x10);
            }
            if(TP2830  == id[chip] || TP2831  == id[chip]) // Added TP2830/31 ID - 10/15/2018
            {
                TP2831_C8MP12_DataSet(chip);
                tp28xx_byte_write(chip, 0x15, 0x23);
                tp28xx_byte_write(chip, 0x16, 0xb8);
                tp28xx_byte_write(chip, 0x1c, 0x12);
                tp28xx_byte_write(chip, 0x1d, 0xc0);
            }
        }

        tp282x_SYSCLK_V3(chip, ch);
        break;

    case TP2802_1080P60:
        tp28xx_byte_write(chip, 0x35, 0x05);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);
        if(TP2828  == id[chip]) {TP2829_1080P60_DataSet(chip);}
        if(TP2830  == id[chip]) {TP2831_1080P60_DataSet(chip);}
        if(TP2831  == id[chip]) {TP2831_1080P60_DataSet(chip);}
        tp282x_SYSCLK_V3(chip, ch);
        break;

    case TP2802_1080P50:
        tp28xx_byte_write(chip, 0x35, 0x05);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);
        if(TP2828  == id[chip]) {TP2829_1080P50_DataSet(chip);}
        if(TP2830  == id[chip]) {TP2831_1080P50_DataSet(chip);}
        if(TP2831  == id[chip]) {TP2831_1080P50_DataSet(chip);}
        tp282x_SYSCLK_V3(chip, ch);
        break;
    case TP2802_720P14:
        tp28xx_byte_write(chip, 0x35, 0x33);
        tp28xx_byte_write(chip, 0x36, 0x20);
        tp2802_set_work_mode_720p30(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tmp |=0x02;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp |= SYS_MODE[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);

        if(TP2826  == id[chip]) TP2826_V2_DataSet(chip);
        if(TP2816  == id[chip]) TP2826_V2_DataSet(chip);
        if(TP2827  == id[chip]) TP2826_V2_DataSet(chip);
        if(TP2828  == id[chip]) TP2829_V2_DataSet(chip);
        if(TP2830  == id[chip]) {TP2831_V2_DataSet(chip);}
        if(TP2831  == id[chip]) {TP2831_V2_DataSet(chip);}
        tp282x_SYSCLK_V2(chip, ch);
        break;
    case TP2802_720P30HDR:
        tp28xx_byte_write(chip, 0x35, 0x33);
        tp28xx_byte_write(chip, 0x36, 0x39);
        tp2802_set_work_mode_720p30HDR(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tmp |=0x02;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp |= SYS_MODE[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);
        if(TP2853C == id[chip]) TP2853C_V2_DataSet(chip);
        if(TP2833C == id[chip]) TP2853C_V2_DataSet(chip);
        if(TP2823C == id[chip]) TP2853C_V2_DataSet(chip);
        if(TP2826  == id[chip]) TP2826_V2_DataSet(chip);
        if(TP2816  == id[chip]) TP2826_V2_DataSet(chip);
        if(TP2827  == id[chip]) TP2826_V2_DataSet(chip);
        if(TP2828  == id[chip]) TP2829_V2_DataSet(chip);
        if(TP2830  == id[chip]) TP2831_V2_DataSet(chip); // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) TP2831_V2_DataSet(chip); // Added TP2831 ID - 10/15/2018
        tp28xx_byte_write(chip, 0x2d, 0x28);
        tp282x_SYSCLK_V2(chip, ch);
        break;

    case TP2802_6M20:
        tp28xx_byte_write(chip, 0x35, 0x17);
        tp28xx_byte_write(chip, 0x36, 0xd0);
        tp2802_set_work_mode_6M20(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);

        if(STD_HDC == std || STD_HDC_DEFAULT == std)
        {
                    if(TP2826  == id[chip]) TP2827C_C6MP20_DataSet(chip);
                    if(TP2827C  == id[chip]) TP2827C_C6MP20_DataSet(chip);
                    if(TP2826C  == id[chip]) TP2827C_C6MP20_DataSet(chip);
                    if(TP2828  == id[chip]) TP2827C_C6MP20_DataSet(chip);
                    if(TP2829  == id[chip]) TP2827C_C6MP20_DataSet(chip);
                    if(TP2830  == id[chip]) TP2827C_C6MP20_DataSet(chip);
                    if(TP2831  == id[chip]) TP2827C_C6MP20_DataSet(chip);
        }

        tp282x_SYSCLK_V3(chip, ch);
        break;
    case TP2802_8M15V2:
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);
        if(TP2828  == id[chip]) {TP2831_8MP15V2_DataSet(chip);}
        if(TP2829  == id[chip]) {TP2831_8MP15V2_DataSet(chip);}
        if(TP2830  == id[chip]) {TP2831_8MP15V2_DataSet(chip);}
        if(TP2831  == id[chip]) {TP2831_8MP15V2_DataSet(chip);}
        tp282x_SYSCLK_V3(chip, ch);
        break;

    case TP2802_1080P15:
        tp28xx_byte_write(chip, 0x35, 0x25);
        tp2802_set_work_mode_1080p30(chip);
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp |= SYS_MODE[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);
        if(TP2823  == id[chip]) TP2823_V2_DataSet(chip);
        if(TP2834  == id[chip]) TP2834_V2_DataSet(chip);
        if(TP2833  == id[chip]) TP2833_V2_DataSet(chip);
        if(TP2853C == id[chip]) TP2853C_V2_DataSet(chip);
        if(TP2833C == id[chip]) TP2853C_V2_DataSet(chip);
        if(TP2823C == id[chip]) TP2853C_V2_DataSet(chip);
        if(TP2826  == id[chip]) TP2826_V2_DataSet(chip);
        if(TP2816  == id[chip]) TP2826_V2_DataSet(chip);
        if(TP2827  == id[chip]) TP2826_V2_DataSet(chip);
        if(TP2828  == id[chip]) TP2829_V2_DataSet(chip);
        if(TP2830  == id[chip]) TP2831_V2_DataSet(chip); // Added TP2830 ID - 10/15/2018
        if(TP2831  == id[chip]) TP2831_V2_DataSet(chip); // Added TP2831 ID - 10/15/2018
        tp282x_SYSCLK_V2(chip, ch);
        break;

    case TP2802_5M20V2:
        tmp = tp28xx_byte_read(chip, 0x02);
        tmp &=0xF8;
        tp28xx_byte_write(chip, 0x02, tmp);
        tmp = tp28xx_byte_read(chip, 0xf5);
        tmp &= SYS_AND[ch];
        tp28xx_byte_write(chip, 0xf5, tmp);

        if(TP2827C  == id[chip]) TP2831_5MP20V2_DataSet(chip);
        if(TP2826C  == id[chip]) TP2831_5MP20V2_DataSet(chip);
        if(TP2828  == id[chip]) TP2831_5MP20V2_DataSet(chip);
        if(TP2829  == id[chip]) TP2831_5MP20V2_DataSet(chip);
        if(TP2830  == id[chip]) TP2831_5MP20V2_DataSet(chip);
        if(TP2831  == id[chip]) TP2831_5MP20V2_DataSet(chip);

        tp282x_SYSCLK_V3(chip, ch);
        break;

    default:
        err = -1;
        break;
    }

    // already set the reg0x35 in SYSCLK_V1/V2/V3 according to output mode.
    //if(mode&FLAG_HALF_MODE)
    //{
    //    tmp = tp28xx_byte_read(chip, 0x35);
    //    tmp |=0x40;
    //    tp28xx_byte_write(chip, 0x35, tmp);
    //}

    return err;
}


static void tp2802_set_reg_page(unsigned char chip, unsigned char ch)
{
    switch(ch)
    {
    case CH_1:
        tp28xx_byte_write(chip, 0x40, 0x00);
        break;  // VIN1 registers
    case CH_2:
        tp28xx_byte_write(chip, 0x40, 0x01);
        break;  // VIN2 registers
    case CH_3:
        tp28xx_byte_write(chip, 0x40, 0x02);
        break;  // VIN3 registers
    case CH_4:
        tp28xx_byte_write(chip, 0x40, 0x03);
        break;  // VIN4 registers
    case CH_ALL:
        tp28xx_byte_write(chip, 0x40, 0x04);
        break;  // Write All VIN1-4 registers
    case AUDIO_PAGE:
        tp28xx_byte_write(chip, 0x40, 0x40);
        break;  // Audio
    case DATA_PAGE:
        tp28xx_byte_write(chip, 0x40, 0x10);
        break;  // PTZ data
    default:
        tp28xx_byte_write(chip, 0x40, 0x04);
        break;
    }
}
static void tp2802_manual_agc(unsigned char chip, unsigned char ch)
{
    unsigned int agc, tmp;

    tp28xx_byte_write(chip, 0x2F, 0x02);
    agc = tp28xx_byte_read(chip, 0x04);
    printf("AGC=0x%04x ch%02x\r\n", agc, ch);
    agc += tp28xx_byte_read(chip, 0x04);
    agc += tp28xx_byte_read(chip, 0x04);
    agc += tp28xx_byte_read(chip, 0x04);
    agc &= 0x3f0;
    agc >>=1;
    if(agc > 0x1ff) agc = 0x1ff;
#if (DEBUG)
    printf("AGC=0x%04x ch%02x\r\n", agc, ch);
#endif
    tp28xx_byte_write(chip, 0x08, agc&0xff);
    tmp = tp28xx_byte_read(chip, 0x06);
    tmp &=0xf9;
    tmp |=(agc>>7)&0x02;
    tmp |=0x04;
    tp28xx_byte_write(chip, 0x06,tmp);
}

static void tp282x_SYSCLK_V2(unsigned char chip, unsigned char ch)
{
    unsigned char tmp, i;


    if(SDR_2CH == output[chip] || DDR_4CH == output[chip])
    {

    }
    else if(DDR_2CH == output[chip] )
    {
        if( TP2826 == id[chip] || TP2826C == id[chip] || TP2828 == id[chip] || TP2830 == id[chip] ) // Added TP2830 ID - 10/15/2018
        {
            tmp = tp28xx_byte_read(chip,0x46);
            tmp |= TP2826_DDR2CH_MUX[ch];
            tp28xx_byte_write(chip, 0x46, tmp);
                tmp = tp28xx_byte_read(chip,0x47);
                tmp |= TP2826_DDR2CH_MUX[ch];
                tp28xx_byte_write(chip, 0x47, tmp);
                tmp = tp28xx_byte_read(chip,0x49);
                tmp |= TP2826_DDR2CH_MUX[ch];
                tp28xx_byte_write(chip, 0x49, tmp);
/*
            tmp = tp28xx_byte_read(chip,0x46);
            tmp |= SYS_MODE[ch];
            tp28xx_byte_write(chip, 0x46, tmp);

            if(CH_1 == ch || CH_2 == ch)
            {
                tmp = tp28xx_byte_read(chip,0x47);
                tmp |= SYS_MODE[ch];
                tp28xx_byte_write(chip, 0x47, tmp);
            }
            else if(CH_3 == ch || CH_4 == ch)
            {
                tmp = tp28xx_byte_read(chip,0x49);
                tmp |= SYS_MODE[ch];
                tp28xx_byte_write(chip, 0x49, tmp);
            }
            else //ch_all
            {
                tp28xx_byte_write(chip, 0x47, 0x03);
                tp28xx_byte_write(chip, 0x49, 0x0c);
            }
*/
        }
        else if( TP2827 == id[chip] || TP2827C == id[chip] || TP2829 == id[chip] || TP2831 == id[chip] ) // Added TP2831 ID - 10/15/2018
        {
            tmp = tp28xx_byte_read(chip,0x46);
            tmp |= SYS_MODE[ch];
            tp28xx_byte_write(chip, 0x46, tmp);

                tmp = tp28xx_byte_read(chip,0x47);
                tmp |= DDR_2CH_MUX[ch];
                tp28xx_byte_write(chip, 0x47, tmp);
                tmp = tp28xx_byte_read(chip,0x49);
                tmp |= DDR_2CH_MUX[ch];
                tp28xx_byte_write(chip, 0x49, tmp);

        }
        else if( id[chip] > TP2834 || id[chip] == TP2833)
        {
            tmp = tp28xx_byte_read(chip,0x47);
            tmp |= DDR_2CH_MUX[ch];
            //tp28xx_byte_write(chip, 0x46, tmp);
            tp28xx_byte_write(chip, 0x47, tmp);
        }

    }
    else if(SDR_1CH == output[chip])
    {
        if(TP2826 == id[chip] || TP2826C == id[chip] || TP2828 == id[chip] || TP2830 == id[chip]) // Added TP2830 ID - 10/15/2018
        {
            if( ch >= CH_ALL)
            {
                for(i = 0; i < 2; i++) //two ports
                {
                    tmp = tp28xx_byte_read(chip,CLK_ADDR[i*2]);
                    tmp &= CLK_AND[i*2];
                    tmp |= CLK_MODE[i*2];
                    tp28xx_byte_write(chip, CLK_ADDR[i*2], tmp);
                }
            }
            else
            {
                tmp = tp28xx_byte_read(chip,CLK_ADDR[(ch%2)*2]);
                tmp &= CLK_AND[(ch%2)*2];
                tmp |= CLK_MODE[(ch%2)*2];
                tp28xx_byte_write(chip, CLK_ADDR[(ch%2)*2], tmp);
            }
        }
        else if( TP2831 == id[chip] || TP2829 == id[chip] || TP2827C == id[chip] || TP2827 == id[chip] || TP2834 == id[chip] || id[chip] < TP2822 ) // Added TP2831 ID - 10/15/2018
        {
            if( ch >= CH_ALL ) //four ports
            {
                for(i = 0; i < 4; i++)
                {
                    tmp = tp28xx_byte_read(chip,CLK_ADDR[i]);
                    tmp &= CLK_AND[i];
                    tmp |= CLK_MODE[i];
                    tp28xx_byte_write(chip, CLK_ADDR[i], tmp);
                }
            }
            else
            {
                tmp = tp28xx_byte_read(chip,CLK_ADDR[ch]);
                tmp &= CLK_AND[ch];
                tmp |= CLK_MODE[ch];
                tp28xx_byte_write(chip, CLK_ADDR[ch], tmp);
            }
        }
        else  //if( TP2823C || TP2853C || TP2833C || TP2833  || TP2823  || TP2822 )
        {
            if( ch >= CH_ALL)
            {
                for(i = 0; i < 2; i++) //two ports
                {
                    tmp = tp28xx_byte_read(chip,CLK_ADDR[i]);
                    tmp &= CLK_AND[i];
                    tmp |= CLK_MODE[i];
                    tp28xx_byte_write(chip, CLK_ADDR[i], tmp);
                }
            }
            else
            {
                tmp = tp28xx_byte_read(chip,CLK_ADDR[ch%2]);
                tmp &= CLK_AND[ch];
                tmp |= CLK_MODE[ch];
                tp28xx_byte_write(chip, CLK_ADDR[ch%2], tmp);
            }
        }

    }
    else if(DDR_1CH == output[chip])
    {
        if(TP2827 == id[chip] || TP2827C == id[chip] || TP2829 == id[chip] || TP2831 == id[chip]) // Added TP2831 ID - 10/15/2018
        {
            if( ch >= CH_ALL ) //four ports
            {
                for(i = 0; i < 4; i++)
                {
                    tmp = tp28xx_byte_read(chip,CLK_ADDR[i]);
                    tmp &= CLK_AND[i];
                    tmp |= CLK_MODE[i];
                    tp28xx_byte_write(chip, CLK_ADDR[i], tmp);
                    //tp28xx_byte_write(chip, 0xf6+i, SDR1_SEL[i]);
                    tp28xx_byte_write(chip, DAT_ADDR[i], SDR1_SEL[i]);
                }
            }
            else
            {
                tmp = tp28xx_byte_read(chip,CLK_ADDR[ch]);
                tmp &= CLK_AND[ch];
                tmp |= CLK_MODE[ch];
                tp28xx_byte_write(chip, CLK_ADDR[ch], tmp);
                //tp28xx_byte_write(chip, 0xf6+ch, SDR1_SEL[ch]);
                tp28xx_byte_write(chip, DAT_ADDR[ch], SDR1_SEL[ch]);
            }
        }
        else if(TP2826 == id[chip] || TP2826C == id[chip] || TP2828 == id[chip] || TP2830 == id[chip]) // Added TP2830 ID - 10/15/2018
        {
            if( ch >= CH_ALL)
            {
                for(i = 0; i < 2; i++) //two ports
                {
                    tmp = tp28xx_byte_read(chip,CLK_ADDR[i*2]);
                    tmp &= CLK_AND[i*2];
                    tmp |= CLK_MODE[i*2];
                    tp28xx_byte_write(chip, CLK_ADDR[i*2], tmp);
                    //tp28xx_byte_write(chip, 0xf6+i*2, SDR1_SEL[i]);
                    tp28xx_byte_write(chip, DAT_ADDR[i*2], SDR1_SEL[i]);
                }
            }
            else
            {
                tmp = tp28xx_byte_read(chip,CLK_ADDR[(ch%2)*2]);
                tmp &= CLK_AND[(ch%2)*2];
                tmp |= CLK_MODE[(ch%2)*2];
                tp28xx_byte_write(chip, CLK_ADDR[(ch%2)*2], tmp);
                //tp28xx_byte_write(chip, 0xf6+(ch%2)*2, SDR1_SEL[ch]);
                tp28xx_byte_write(chip, DAT_ADDR[(ch%2)*2], SDR1_SEL[ch]);
            }
        }
    }

}
static void tp282x_SYSCLK_V1(unsigned char chip, unsigned char ch)
{
    unsigned char tmp, i;

    if(SDR_2CH == output[chip] || DDR_4CH == output[chip])
    {
        if( id[chip] >= TP2822 )
        {
            tmp = tp28xx_byte_read(chip, 0x35); //to match 3M/5M
            tmp |= 0x40;
            tp28xx_byte_write(chip, 0x35, tmp);
        }
    }
    else if(DDR_2CH == output[chip] )
    {
        if( TP2826 == id[chip] || TP2826C == id[chip] || TP2828 == id[chip] || TP2830 == id[chip]) // Added TP2830 ID - 10/15/2018
        {
            tmp = tp28xx_byte_read(chip,0x46);
            tmp &= ~TP2826_DDR2CH_MUX[ch];
            tp28xx_byte_write(chip, 0x46, tmp);
                tmp = tp28xx_byte_read(chip,0x47);
                tmp &= ~TP2826_DDR2CH_MUX[ch];
                tp28xx_byte_write(chip, 0x47, tmp);
                tmp = tp28xx_byte_read(chip,0x49);
                tmp &= ~TP2826_DDR2CH_MUX[ch];
                tp28xx_byte_write(chip, 0x49, tmp);
/*
            tmp = tp28xx_byte_read(chip,0x46);
            tmp &= ~SYS_MODE[ch];
            tp28xx_byte_write(chip, 0x46, tmp);

            if(CH_1 == ch || CH_2 == ch)
            {
                tmp = tp28xx_byte_read(chip,0x47);
                tmp &= ~SYS_MODE[ch];
                tp28xx_byte_write(chip, 0x47, tmp);
            }
            else if(CH_3 == ch || CH_4 == ch)
            {
                tmp = tp28xx_byte_read(chip,0x49);
                tmp &= ~SYS_MODE[ch];
                tp28xx_byte_write(chip, 0x49, tmp);
            }
            else //ch_all
            {
                tp28xx_byte_write(chip, 0x47, 0x00);
                tp28xx_byte_write(chip, 0x49, 0x00);
            }
*/
        }
        else if( TP2827 == id[chip] || TP2827C == id[chip] || TP2829 == id[chip] || TP2831 == id[chip] ) // Added TP2831 ID - 10/15/2018
        {
            tmp = tp28xx_byte_read(chip,0x46);
            tmp &= ~SYS_MODE[ch];
            tp28xx_byte_write(chip, 0x46, tmp);

                tmp = tp28xx_byte_read(chip,0x47);
                tmp &= ~DDR_2CH_MUX[ch];
                tp28xx_byte_write(chip, 0x47, tmp);

                tmp = tp28xx_byte_read(chip,0x49);
                tmp &= ~DDR_2CH_MUX[ch];
                tp28xx_byte_write(chip, 0x49, tmp);

        }
        else if( id[chip] > TP2834 || id[chip] == TP2833 ) //TP28x3C, TP2833B
        {
            tmp = tp28xx_byte_read(chip,0x47);
            tmp &= ~DDR_2CH_MUX[ch];
            tp28xx_byte_write(chip, 0x47, tmp);
        }
    }
    else if(SDR_1CH == output[chip])
    {
        if(TP2826 == id[chip] || TP2826C == id[chip] || TP2828 == id[chip] || TP2830 == id[chip]) // Added TP2830 ID - 10/15/2018
        {
            if( ch >= CH_ALL)
            {
                for(i = 0; i < 2; i++) //two ports
                {
                    tmp = tp28xx_byte_read(chip,CLK_ADDR[i*2]);
                    tmp &= CLK_AND[i*2];
                    tp28xx_byte_write(chip, CLK_ADDR[i*2], tmp);
                }
            }
            else
            {
                tmp = tp28xx_byte_read(chip,CLK_ADDR[(ch%2)*2]);
                tmp &= CLK_AND[(ch%2)*2];
                tp28xx_byte_write(chip, CLK_ADDR[(ch%2)*2], tmp);
            }
        }
        else if(TP2831 == id[chip] || TP2829 == id[chip] || TP2827C == id[chip] || TP2827 == id[chip] || TP2834 == id[chip] || id[chip] < TP2822) // Added TP2831 ID - 10/15/2018
        {
            if( ch >= CH_ALL)
            {
                for(i = 0; i < 4; i++) //four ports
                {
                    tmp = tp28xx_byte_read(chip,CLK_ADDR[i]);
                    tmp &= CLK_AND[i];
                    tp28xx_byte_write(chip, CLK_ADDR[i], tmp);
                }
            }
            else
            {
                tmp = tp28xx_byte_read(chip,CLK_ADDR[ch]);
                tmp &= CLK_AND[ch];
                tp28xx_byte_write(chip, CLK_ADDR[ch], tmp);
            }
        }
        else //if( TP2823C || TP2853C || TP2833C || TP2833  || TP2823  || TP2822 )
        {
            if( ch >= CH_ALL)
            {
                for(i = 0; i < 2; i++) //two ports
                {
                    tmp = tp28xx_byte_read(chip,CLK_ADDR[i]);
                    tmp &= CLK_AND[i];
                    tp28xx_byte_write(chip, CLK_ADDR[i], tmp);
                }
            }
            else
            {
                tmp = tp28xx_byte_read(chip,CLK_ADDR[ch%2]);
                tmp &= CLK_AND[ch];
                tp28xx_byte_write(chip, CLK_ADDR[ch%2], tmp);
            }
        }
    }
    else if(DDR_1CH == output[chip])
    {
        if( TP2827 == id[chip] || TP2827C == id[chip] || TP2829 == id[chip] || TP2831 == id[chip]) // Added TP2831 ID - 10/15/2018
        {
            if( ch >= CH_ALL)
            {
                for(i = 0; i < 4; i++) //four ports
                {
                    tmp = tp28xx_byte_read(chip,CLK_ADDR[i]);
                    tmp &= CLK_AND[i];
                    tp28xx_byte_write(chip, CLK_ADDR[i], tmp);
                    //tp28xx_byte_write(chip, 0xf6+i, SDR1_SEL[i]);
                    tp28xx_byte_write(chip, DAT_ADDR[i], SDR1_SEL[i]);
                }
            }
            else
            {
                tmp = tp28xx_byte_read(chip,CLK_ADDR[ch]);
                tmp &= CLK_AND[ch];
                tp28xx_byte_write(chip, CLK_ADDR[ch], tmp);
                //tp28xx_byte_write(chip, 0xf6+ch, SDR1_SEL[ch]);
                tp28xx_byte_write(chip, DAT_ADDR[ch], SDR1_SEL[ch]);
            }
        }
        else if(TP2826 == id[chip] || TP2826C == id[chip] || TP2828 == id[chip] || TP2830 == id[chip]) // Added TP2830 ID - 10/15/2018
        {
            if( ch >= CH_ALL)
            {
                for(i = 0; i < 2; i++) //two ports
                {
                    tmp = tp28xx_byte_read(chip,CLK_ADDR[i*2]);
                    tmp &= CLK_AND[i*2];
                    tp28xx_byte_write(chip, CLK_ADDR[i*2], tmp);
                    //tp28xx_byte_write(chip, 0xf6+i*2, SDR1_SEL[i]);
                    tp28xx_byte_write(chip, DAT_ADDR[i*2], SDR1_SEL[i]);
                }
            }
            else
            {
                tmp = tp28xx_byte_read(chip,CLK_ADDR[(ch%2)*2]);
                tmp &= CLK_AND[(ch%2)*2];
                tp28xx_byte_write(chip, CLK_ADDR[(ch%2)*2], tmp);
                //tp28xx_byte_write(chip, 0xf6+(ch%2)*2, SDR1_SEL[ch]);
                tp28xx_byte_write(chip, DAT_ADDR[(ch%2)*2], SDR1_SEL[ch]);
            }
        }

    }

}
static void tp282x_SYSCLK_V3(unsigned char chip, unsigned char ch)
{
    unsigned char tmp, i;

    if(DDR_1CH == output[chip])
    {
        if(TP2827C == id[chip] || TP2829 == id[chip] || TP2831 == id[chip]) // Added TP2831 ID - 10/15/2018
        {
            if( ch >= CH_ALL)
            {
                for(i = 0; i < 4; i++) //four ports
                {
                    tmp = tp28xx_byte_read(chip,CLK_ADDR[i]);
                    tmp &= CLK_AND[i];
                    tp28xx_byte_write(chip, CLK_ADDR[i], tmp);
                    tp28xx_byte_write(chip, DAT_ADDR[i], TP2827C_DDR1_SEL[i]);
                }
            }
            else
            {
                tmp = tp28xx_byte_read(chip,CLK_ADDR[ch]);
                tmp &= CLK_AND[ch];
                tp28xx_byte_write(chip, CLK_ADDR[ch], tmp);
                tp28xx_byte_write(chip, DAT_ADDR[ch], TP2827C_DDR1_SEL[ch]);
            }
        }
        else if(TP2827 == id[chip])
        {
            if( ch >= CH_ALL)
            {
                for(i = 0; i < 4; i++) //four ports
                {
                    tmp = tp28xx_byte_read(chip,CLK_ADDR[i]);
                    tmp &= CLK_AND[i];
                    tp28xx_byte_write(chip, CLK_ADDR[i], tmp);
                    tp28xx_byte_write(chip, DAT_ADDR[i], DDR1_SEL[i]);
                }
            }
            else
            {
                tmp = tp28xx_byte_read(chip,CLK_ADDR[ch]);
                tmp &= CLK_AND[ch];
                tp28xx_byte_write(chip, CLK_ADDR[ch], tmp);
                tp28xx_byte_write(chip, DAT_ADDR[ch], DDR1_SEL[ch]);
            }
        }
        else if(TP2826C == id[chip] || TP2828 == id[chip] || TP2830 == id[chip]) // Added TP2830 ID - 10/15/2018
        {
            if( ch >= CH_ALL)
            {
                for(i = 0; i < 2; i++) //two ports
                {
                    tmp = tp28xx_byte_read(chip,CLK_ADDR[i*2]);
                    tmp &= CLK_AND[i*2];
                    tp28xx_byte_write(chip, CLK_ADDR[i*2], tmp);
                    tp28xx_byte_write(chip, DAT_ADDR[i*2], TP2827C_DDR1_SEL[i]);
                }
            }
            else
            {
                tmp = tp28xx_byte_read(chip,CLK_ADDR[(ch%2)*2]);
                tmp &= CLK_AND[(ch%2)*2];
                tp28xx_byte_write(chip, CLK_ADDR[(ch%2)*2], tmp);
                tp28xx_byte_write(chip, DAT_ADDR[(ch%2)*2], TP2827C_DDR1_SEL[ch]);
            }
        }
        else if(TP2826 == id[chip])
        {
            if( ch >= CH_ALL)
            {
                for(i = 0; i < 2; i++) //two ports
                {
                    tmp = tp28xx_byte_read(chip,CLK_ADDR[i*2]);
                    tmp &= CLK_AND[i*2];
                    tp28xx_byte_write(chip, CLK_ADDR[i*2], tmp);
                    tp28xx_byte_write(chip, DAT_ADDR[i*2], DDR1_SEL[i]);
                }
            }
            else
            {
                tmp = tp28xx_byte_read(chip,CLK_ADDR[(ch%2)*2]);
                tmp &= CLK_AND[(ch%2)*2];
                tp28xx_byte_write(chip, CLK_ADDR[(ch%2)*2], tmp);
                tp28xx_byte_write(chip, DAT_ADDR[(ch%2)*2], DDR1_SEL[ch]);
            }
        }
    }
    else if(DDR_2CH == output[chip])
    {
        if(TP2826 == id[chip] || TP2826C == id[chip] || TP2828 == id[chip] || TP2830 == id[chip]) // Added TP2830 ID - 10/15/2018
        {
            tmp = tp28xx_byte_read(chip, 0x35);
            tmp |= 0x40;
            tp28xx_byte_write(chip, 0x35, tmp);

            tmp = tp28xx_byte_read(chip,0x46);
            tmp &= ~TP2826_DDR2CH_MUX[ch];
            tp28xx_byte_write(chip, 0x46, tmp);
                tmp = tp28xx_byte_read(chip,0x47);
                tmp &= ~TP2826_DDR2CH_MUX[ch];
                tp28xx_byte_write(chip, 0x47, tmp);
                tmp = tp28xx_byte_read(chip,0x49);
                tmp &= ~TP2826_DDR2CH_MUX[ch];
                tp28xx_byte_write(chip, 0x49, tmp);
/*
            tmp = tp28xx_byte_read(chip,0x46);
            tmp &= ~SYS_MODE[ch];
            tp28xx_byte_write(chip, 0x46, tmp);

            if(CH_1 == ch || CH_2 == ch)
            {
                tmp = tp28xx_byte_read(chip,0x47);
                tmp &= ~SYS_MODE[ch];
                tp28xx_byte_write(chip, 0x47, tmp);
            }
            else if(CH_3 == ch || CH_4 == ch)
            {
                tmp = tp28xx_byte_read(chip,0x49);
                tmp &= ~SYS_MODE[ch];
                tp28xx_byte_write(chip, 0x49, tmp);
            }
            else //ch_all
            {
                tp28xx_byte_write(chip, 0x47, 0x00);
                tp28xx_byte_write(chip, 0x49, 0x00);
            }
*/
        }
        else if(TP2853C == id[chip])
        {
            tmp = tp28xx_byte_read(chip,0x47);
            tmp &= ~DDR_2CH_MUX[ch];
            tp28xx_byte_write(chip, 0x47, tmp);

        }
        else if(TP2827 == id[chip] || TP2827C == id[chip] || TP2829 == id[chip] || TP2831 == id[chip]) // Added TP2831 ID - 10/18/2018
        {
            tmp = tp28xx_byte_read(chip, 0x35); //
            tmp |= 0x40;
            tp28xx_byte_write(chip, 0x35, tmp);

            tmp = tp28xx_byte_read(chip,0x46);
            tmp &= ~SYS_MODE[ch];
            tp28xx_byte_write(chip, 0x46, tmp);

                tmp = tp28xx_byte_read(chip,0x47);
                tmp &= ~DDR_2CH_MUX[ch];
                tp28xx_byte_write(chip, 0x47, tmp);

                tmp = tp28xx_byte_read(chip,0x49);
                tmp &= ~DDR_2CH_MUX[ch];
                tp28xx_byte_write(chip, 0x49, tmp);
        }
    }
    else if(SDR_1CH == output[chip])
    {
        if( TP2827 == id[chip] || TP2827C == id[chip] || TP2829 == id[chip] || TP2831 == id[chip]) // Added TP2831 ID - 10/15/2018
        {
            tmp = tp28xx_byte_read(chip, 0x35); //
            tmp |= 0x40;
            tp28xx_byte_write(chip, 0x35, tmp);

            if( ch >= CH_ALL)
            {
                for(i = 0; i < 4; i++) //four ports
                {
                    tmp = tp28xx_byte_read(chip,CLK_ADDR[i]);
                    tmp &= CLK_AND[i];
                    tp28xx_byte_write(chip, CLK_ADDR[i], tmp);
                    //tp28xx_byte_write(chip, DAT_ADDR[i], SDR1_SEL[i]);
                }
            }
            else
            {
                tmp = tp28xx_byte_read(chip,CLK_ADDR[ch]);
                tmp &= CLK_AND[ch];
                tp28xx_byte_write(chip, CLK_ADDR[ch], tmp);
                //tp28xx_byte_write(chip, DAT_ADDR[ch], SDR1_SEL[ch]);
            }
        }
        else if(TP2826 == id[chip] || TP2826C == id[chip] || TP2828 == id[chip] || TP2830 == id[chip]) // Added TP2830 ID - 10/15/2018
        {
            tmp = tp28xx_byte_read(chip, 0x35); //
            tmp |= 0x40;
            tp28xx_byte_write(chip, 0x35, tmp);

            if( ch >= CH_ALL)
            {
                for(i = 0; i < 2; i++) //two ports
                {
                    tmp = tp28xx_byte_read(chip,CLK_ADDR[i*2]);
                    tmp &= CLK_AND[i*2];
                    tp28xx_byte_write(chip, CLK_ADDR[i*2], tmp);
                }
            }
            else
            {
                tmp = tp28xx_byte_read(chip,CLK_ADDR[(ch%2)*2]);
                tmp &= CLK_AND[(ch%2)*2];
                tp28xx_byte_write(chip, CLK_ADDR[(ch%2)*2], tmp);
            }
        }
        else if( TP2853C == id[chip] )
        {
            if( ch >= CH_ALL)
            {
                for(i = 0; i < 2; i++) //two ports
                {
                    tmp = tp28xx_byte_read(chip,CLK_ADDR[i]);
                    tmp &= CLK_AND[i];
                    tp28xx_byte_write(chip, CLK_ADDR[i], tmp);
                }
            }
            else
            {
                tmp = tp28xx_byte_read(chip,CLK_ADDR[ch%2]);
                tmp &= CLK_AND[ch];
                tp28xx_byte_write(chip, CLK_ADDR[ch%2], tmp);
            }
        }
    }
}
static void tp2802D_reset_default(unsigned char chip, unsigned char ch)
{
    unsigned int tmp;
    tp28xx_byte_write(chip, 0x3A, 0x01);
    tp28xx_byte_write(chip, 0x0B, 0xC0);
    tp28xx_byte_write(chip, 0x07, 0xC0);
    tp28xx_byte_write(chip, 0x2e, 0x70);
    tp28xx_byte_write(chip, 0x39, 0x42);
    tp28xx_byte_write(chip, 0x09, 0x24);
    tmp = tp28xx_byte_read(chip, 0x06);   // soft reset and auto agc when cable is unplug
    tmp &= 0x7b;
    tp28xx_byte_write(chip, 0x06, tmp);

    tmp = tp28xx_byte_read(chip, 0xf5);
    tmp &= SYS_AND[ch];
    tp28xx_byte_write(chip, 0xf5, tmp);
    tmp = tp28xx_byte_read(chip,CLK_ADDR[ch]);
    tmp &= CLK_AND[ch];
    tp28xx_byte_write(chip, CLK_ADDR[ch], tmp);

}

static void tp2822_reset_default(unsigned char chip, unsigned char ch)
{
    unsigned int tmp;

    tmp = tp28xx_byte_read(chip, 0x06);
    tmp &= 0xfb;
    tp28xx_byte_write(chip, 0x06, tmp);
    tp28xx_byte_write(chip, 0x07, 0x40);
}
static void tp2823_reset_default(unsigned char chip, unsigned char ch)
{
    unsigned int tmp;

    tmp = tp28xx_byte_read(chip, 0x06);
    tmp &= 0xfb;
    tp28xx_byte_write(chip, 0x06, tmp);
    tp28xx_byte_write(chip, 0x07, 0x40);
    //tp28xx_byte_write(chip, 0x26, 0x01);
    tmp = tp28xx_byte_read(chip, 0x26);
    tmp &= 0xfc;
    tmp |= 0x01;
    tp28xx_byte_write(chip, 0x26, tmp);
}
static void tp2834_reset_default(unsigned char chip, unsigned char ch)
{
    unsigned int tmp;
    //tp28xx_byte_write(chip, 0x26, 0x01);
    tp28xx_byte_write(chip, 0x07, 0xC0);
    tp28xx_byte_write(chip, 0x0B, 0xC0);
    tp28xx_byte_write(chip, 0x22, 0x35);

    tmp = tp28xx_byte_read(chip, 0x26);
    tmp &= 0xfc;
    tmp |= 0x01;
    tp28xx_byte_write(chip, 0x26, tmp);

    tmp = tp28xx_byte_read(chip, 0x06);
    tmp &= 0xfb;
    tp28xx_byte_write(chip, 0x06, tmp);
}
static void tp2833_reset_default(unsigned char chip, unsigned char ch)
{
    unsigned int tmp;
    //tp28xx_byte_write(chip, 0x26, 0x04);
    tp28xx_byte_write(chip, 0x07, 0xC0);
    tp28xx_byte_write(chip, 0x0B, 0xC0);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x38, 0x40);

    tmp = tp28xx_byte_read(chip, 0x26);
    tmp &= 0xfe;
    tp28xx_byte_write(chip, 0x26, tmp);

    tmp = tp28xx_byte_read(chip, 0x06);
    tmp &= 0xfb;
    tp28xx_byte_write(chip, 0x06, tmp);
}
static void tp2853C_reset_default(unsigned char chip, unsigned char ch)
{
    unsigned int tmp;
    //tp28xx_byte_write(chip, 0x26, 0x04);
    tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x38, 0x00);
    tp28xx_byte_write(chip, 0x3a, 0x32);
    tp28xx_byte_write(chip, 0x3B, 0x25);

    tmp = tp28xx_byte_read(chip, 0x26);
    tmp &= 0xfe;
    tp28xx_byte_write(chip, 0x26, tmp);

    tmp = tp28xx_byte_read(chip, 0x06);
    tmp &= 0xfb;
    tp28xx_byte_write(chip, 0x06, tmp);
}
static void TP2826_reset_default(unsigned char chip, unsigned char ch)
{
    unsigned int tmp;
    //tp28xx_byte_write(chip, 0x26, 0x04);
    tp28xx_byte_write(chip, 0x07, 0xc0);
    tp28xx_byte_write(chip, 0x0b, 0xc0);
    tp28xx_byte_write(chip, 0x21, 0x88);
    tp28xx_byte_write(chip, 0x38, 0x00);
    tp28xx_byte_write(chip, 0x39, 0x0C);
    tp28xx_byte_write(chip, 0x3a, 0x22);
    tp28xx_byte_write(chip, 0x3B, 0x27);

    tmp = tp28xx_byte_read(chip, 0x26);
    tmp &= 0xfe;
    tp28xx_byte_write(chip, 0x26, tmp);

    tmp = tp28xx_byte_read(chip, 0x06);
    tmp &= 0xfb;
    tp28xx_byte_write(chip, 0x06, tmp);
}
static void TP28xx_reset_default(int chip, unsigned char ch)
{
    if(TP2823 == id[chip] )
    {
        tp2823_reset_default(chip, ch);
    }
    else if(TP2822 == id[chip] )
    {
        tp2822_reset_default(chip, ch);
    }
    else if(TP2802D == id[chip] )
    {
        tp2802D_reset_default(chip, ch);
    }
    else if(TP2834 == id[chip] )
    {
        tp2834_reset_default(chip, ch);
    }
    else if(TP2833 == id[chip] )
    {
        tp2833_reset_default(chip, ch);
    }
    else if(TP2853C == id[chip] )
    {
        tp2853C_reset_default(chip, ch);
    }
    else if(TP2833C == id[chip] )
    {
        tp2853C_reset_default(chip, ch);
    }
    else if(TP2823C == id[chip] )
    {
        tp2853C_reset_default(chip, ch);
    }
    else if(TP2826 == id[chip] )
    {
        TP2826_reset_default(chip, ch);
    }
    else if(TP2816 == id[chip] )
    {
        TP2826_reset_default(chip, ch);
    }
    else if(TP2827 == id[chip] )
    {
        TP2826_reset_default(chip, ch);
    }
    else if(TP2828 == id[chip] )
    {
        TP2829_reset_default(chip, ch);
    }
    else if(TP2830 == id[chip] ) // Added TP2830 ID - 10/15/2018
    {
        TP2831_reset_default(chip, ch);
    }
    else if(TP2831 == id[chip] ) // Added TP2831 ID - 10/15/2018
    {
        TP2831_reset_default(chip, ch);
    }
}

static void TP2823_NTSC_DataSet(unsigned char chip)
{
    tp28xx_byte_write(chip, 0x0c, 0x53);
    tp28xx_byte_write(chip, 0x0d, 0x10);
    tp28xx_byte_write(chip, 0x20, 0xa0);
    tp28xx_byte_write(chip, 0x26, 0x12);
    tp28xx_byte_write(chip, 0x2d, 0x68);
    tp28xx_byte_write(chip, 0x2e, 0x5e);

    tp28xx_byte_write(chip, 0x30, 0x62);
    tp28xx_byte_write(chip, 0x31, 0xbb);
    tp28xx_byte_write(chip, 0x32, 0x96);
    tp28xx_byte_write(chip, 0x33, 0xc0);
    //tp28xx_byte_write(chip, 0x35, 0x25);
    tp28xx_byte_write(chip, 0x39, 0x10);

}
static void TP2823_PAL_DataSet(unsigned char chip)
{
    tp28xx_byte_write(chip, 0x0c, 0x53);
    tp28xx_byte_write(chip, 0x0d, 0x11);
    tp28xx_byte_write(chip, 0x20, 0xb0);
    tp28xx_byte_write(chip, 0x26, 0x02);
    tp28xx_byte_write(chip, 0x2d, 0x60);
    tp28xx_byte_write(chip, 0x2e, 0x5e);

    tp28xx_byte_write(chip, 0x30, 0x7a);
    tp28xx_byte_write(chip, 0x31, 0x4a);
    tp28xx_byte_write(chip, 0x32, 0x4d);
    tp28xx_byte_write(chip, 0x33, 0xf0);
    //tp28xx_byte_write(chip, 0x35, 0x25);
    tp28xx_byte_write(chip, 0x39, 0x10);

}
static void TP2823_V1_DataSet(unsigned char chip)
{

    tp28xx_byte_write(chip, 0x0c, 0x43);
    tp28xx_byte_write(chip, 0x0d, 0x10);
    tp28xx_byte_write(chip, 0x20, 0x60);
    tp28xx_byte_write(chip, 0x26, 0x02);
    tp28xx_byte_write(chip, 0x2d, 0x30);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0xbb);
    tp28xx_byte_write(chip, 0x32, 0x2e);
    tp28xx_byte_write(chip, 0x33, 0x90);
    //tp28xx_byte_write(chip, 0x35, 0x05);
    tp28xx_byte_write(chip, 0x39, 0x30);

}
static void TP2823_V2_DataSet(unsigned char chip)
{
    tp28xx_byte_write(chip, 0x0c, 0x53);
    tp28xx_byte_write(chip, 0x0d, 0x10);
    tp28xx_byte_write(chip, 0x20, 0x60);
    tp28xx_byte_write(chip, 0x26, 0x02);
    tp28xx_byte_write(chip, 0x2d, 0x30);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0xbb);
    tp28xx_byte_write(chip, 0x32, 0x2e);
    tp28xx_byte_write(chip, 0x33, 0x90);
    //tp28xx_byte_write(chip, 0x35, 0x25);
    tp28xx_byte_write(chip, 0x39, 0x20);

}
static void TP2823_Audio_DataSet(unsigned char chip)
{

    unsigned int bank;
    bank = tp28xx_byte_read(chip, 0x40);
    tp28xx_byte_write(chip, 0x40, 0x40);

    tp2833_audio_config_rmpos(chip, AUDIO_FORMAT , AUDIO_CHN);

    tp28xx_byte_write(chip, 0x17, 0x00|(DATA_BIT<<2));
    tp28xx_byte_write(chip, 0x1B, 0x01|(DATA_BIT<<6));

#if(AUDIO_CHN == 20)
    tp28xx_byte_write(chip, 0x18, 0x10|(SAMPLE_RATE));
#else
    tp28xx_byte_write(chip, 0x18, 0x00|(SAMPLE_RATE));
#endif

#if(AUDIO_CHN >= 8)
    tp28xx_byte_write(chip, 0x19, 0x1F);
#else
    tp28xx_byte_write(chip, 0x19, 0x0F);
#endif

    tp28xx_byte_write(chip, 0x1A, 0x15);

    tp28xx_byte_write(chip, 0x37, 0x20);
    tp28xx_byte_write(chip, 0x38, 0x38);
    tp28xx_byte_write(chip, 0x3E, 0x06);

    tp28xx_byte_write(chip, 0x1D, 0x08);
    msleep(100);
    tp28xx_byte_write(chip, 0x1D, 0x00);

    tp28xx_byte_write(chip, 0x3C, 0x3f);
    msleep(200);
    tp28xx_byte_write(chip, 0x3C, 0x00);

    tp28xx_byte_write(chip, 0x3d, 0x01);//audio reset

    tp28xx_byte_write(chip, 0x40, bank);

}
////////////////////////////////////////////////////////////////
static void TP2834_NTSC_DataSet(unsigned char chip)
{
    unsigned int tmp;
    tp28xx_byte_write(chip, 0x0c, 0x43);
    tp28xx_byte_write(chip, 0x0d, 0x10);
    tp28xx_byte_write(chip, 0x20, 0xa0);
    tp28xx_byte_write(chip, 0x26, 0x12);
    tp28xx_byte_write(chip, 0x2b, 0x50);
    tp28xx_byte_write(chip, 0x2d, 0x68);
    tp28xx_byte_write(chip, 0x2e, 0x5e);

    tp28xx_byte_write(chip, 0x30, 0x62);
    tp28xx_byte_write(chip, 0x31, 0xbb);
    tp28xx_byte_write(chip, 0x32, 0x96);
    tp28xx_byte_write(chip, 0x33, 0xc0);
    //tp28xx_byte_write(chip, 0x35, 0x25);
    tp28xx_byte_write(chip, 0x39, 0x84);
    tp28xx_byte_write(chip, 0x2c, 0x2a);

    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
}
static void TP2834_PAL_DataSet(unsigned char chip)
{
    unsigned int tmp;
    tp28xx_byte_write(chip, 0x0c, 0x53);
    tp28xx_byte_write(chip, 0x0d, 0x11);
    tp28xx_byte_write(chip, 0x20, 0xb0);
    tp28xx_byte_write(chip, 0x26, 0x02);
    tp28xx_byte_write(chip, 0x2b, 0x50);
    tp28xx_byte_write(chip, 0x2d, 0x60);
    tp28xx_byte_write(chip, 0x2e, 0x5e);

    tp28xx_byte_write(chip, 0x30, 0x7a);
    tp28xx_byte_write(chip, 0x31, 0x4a);
    tp28xx_byte_write(chip, 0x32, 0x4d);
    tp28xx_byte_write(chip, 0x33, 0xf0);
    //tp28xx_byte_write(chip, 0x35, 0x25);
    tp28xx_byte_write(chip, 0x39, 0x84);
    tp28xx_byte_write(chip, 0x2c, 0x2a);

    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
}
static void TP2834_V1_DataSet(unsigned char chip)
{
    unsigned int tmp;
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x10);
    tp28xx_byte_write(chip, 0x20, 0x60);
    tp28xx_byte_write(chip, 0x26, 0x02);
    tp28xx_byte_write(chip, 0x2b, 0x58);
    tp28xx_byte_write(chip, 0x2d, 0x30);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0xbb);
    tp28xx_byte_write(chip, 0x32, 0x2e);
    tp28xx_byte_write(chip, 0x33, 0x90);
    //tp28xx_byte_write(chip, 0x35, 0x05);
    tp28xx_byte_write(chip, 0x39, 0x8C);
    tp28xx_byte_write(chip, 0x2c, 0x0a);

    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
}
static void TP2834_V2_DataSet(unsigned char chip)
{
    unsigned int tmp;
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x10);
    tp28xx_byte_write(chip, 0x20, 0x60);
    tp28xx_byte_write(chip, 0x26, 0x02);
    tp28xx_byte_write(chip, 0x2b, 0x58);
    tp28xx_byte_write(chip, 0x2d, 0x30);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0xbb);
    tp28xx_byte_write(chip, 0x32, 0x2e);
    tp28xx_byte_write(chip, 0x33, 0x90);
    //tp28xx_byte_write(chip, 0x35, 0x25);
    tp28xx_byte_write(chip, 0x39, 0x88);
    tp28xx_byte_write(chip, 0x2c, 0x0a);

    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
}
static void TP2834_C1080P25_DataSet(unsigned char chip)
{

    tp28xx_byte_write(chip, 0x13, 0x40);
    //tp28xx_byte_write(chip, 0x15, 0x13);
    //tp28xx_byte_write(chip, 0x16, 0x84);

    //tp28xx_byte_write(chip, 0x20, 0x50);
    tp28xx_byte_write(chip, 0x20, 0xa0);
    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2d, 0x54);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x41);
    tp28xx_byte_write(chip, 0x31, 0x82);
    tp28xx_byte_write(chip, 0x32, 0x27);
    tp28xx_byte_write(chip, 0x33, 0xa6);

    tp28xx_byte_write(chip, 0x28, 0x04);
    tp28xx_byte_write(chip, 0x07, 0x80);
    tp28xx_byte_write(chip, 0x27, 0x5a);
}
static void TP2834_C720P25_DataSet(unsigned char chip)
{

    tp28xx_byte_write(chip, 0x13, 0x40);
    //tp28xx_byte_write(chip, 0x16, 0x40);

    //tp28xx_byte_write(chip, 0x20, 0x3a);
    tp28xx_byte_write(chip, 0x20, 0x74);
    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2d, 0x42);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0x67);
    tp28xx_byte_write(chip, 0x32, 0x6f);
    tp28xx_byte_write(chip, 0x33, 0x31);

    tp28xx_byte_write(chip, 0x28, 0x04);
    tp28xx_byte_write(chip, 0x07, 0x80);
    tp28xx_byte_write(chip, 0x27, 0x5a);
}
static void TP2834_C720P50_DataSet(unsigned char chip)
{

    tp28xx_byte_write(chip, 0x13, 0x40);
    //tp28xx_byte_write(chip, 0x16, 0x40);

    //tp28xx_byte_write(chip, 0x20, 0x3a);
    tp28xx_byte_write(chip, 0x20, 0x74);
    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2d, 0x42);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x41);
    tp28xx_byte_write(chip, 0x31, 0x82);
    tp28xx_byte_write(chip, 0x32, 0x27);
    tp28xx_byte_write(chip, 0x33, 0xa6);

    tp28xx_byte_write(chip, 0x28, 0x04);
    tp28xx_byte_write(chip, 0x07, 0x80);
    tp28xx_byte_write(chip, 0x27, 0x5a);
}
static void TP2834_C1080P30_DataSet(unsigned char chip)
{

    tp28xx_byte_write(chip, 0x13, 0x40);

    //tp28xx_byte_write(chip, 0x15, 0x13);
    //tp28xx_byte_write(chip, 0x16, 0x44);

    //tp28xx_byte_write(chip, 0x20, 0x40);
    tp28xx_byte_write(chip, 0x20, 0x80);
    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2d, 0x47);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x41);
    tp28xx_byte_write(chip, 0x31, 0x82);
    tp28xx_byte_write(chip, 0x32, 0x27);
    tp28xx_byte_write(chip, 0x33, 0xa6);

    tp28xx_byte_write(chip, 0x28, 0x04);
    tp28xx_byte_write(chip, 0x07, 0x80);
    tp28xx_byte_write(chip, 0x27, 0x5a);
}
static void TP2834_C720P30_DataSet(unsigned char chip)
{

    tp28xx_byte_write(chip, 0x13, 0x40);
    //tp28xx_byte_write(chip, 0x16, 0x02);

    //tp28xx_byte_write(chip, 0x20, 0x30);
    tp28xx_byte_write(chip, 0x20, 0x60);
    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2d, 0x37);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0x67);
    tp28xx_byte_write(chip, 0x32, 0x6f);
    tp28xx_byte_write(chip, 0x33, 0x31);

    tp28xx_byte_write(chip, 0x28, 0x04);
    tp28xx_byte_write(chip, 0x07, 0x80);
    tp28xx_byte_write(chip, 0x27, 0x5a);
}
static void TP2834_C720P60_DataSet(unsigned char chip)
{

    tp28xx_byte_write(chip, 0x13, 0x40);
    //tp28xx_byte_write(chip, 0x16, 0x02);

    //tp28xx_byte_write(chip, 0x20, 0x30);
    tp28xx_byte_write(chip, 0x20, 0x60);
    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2d, 0x37);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x41);
    tp28xx_byte_write(chip, 0x31, 0x82);
    tp28xx_byte_write(chip, 0x32, 0x27);
    tp28xx_byte_write(chip, 0x33, 0xa6);

    tp28xx_byte_write(chip, 0x28, 0x04);
    tp28xx_byte_write(chip, 0x07, 0x80);
    tp28xx_byte_write(chip, 0x27, 0x5a);
}
static void TP2834_Audio_DataSet(unsigned char chip)
{

    unsigned int bank;

    bank = tp28xx_byte_read(chip, 0x40);
    tp28xx_byte_write(chip, 0x40, 0x40);

    tp2833_audio_config_rmpos(chip, AUDIO_FORMAT , AUDIO_CHN);

    tp28xx_byte_write(chip, 0x17, 0x00|(DATA_BIT<<2));
    tp28xx_byte_write(chip, 0x1B, 0x01|(DATA_BIT<<6));

#if(AUDIO_CHN == 20)
    tp28xx_byte_write(chip, 0x18, 0x10|(SAMPLE_RATE));
#else
    tp28xx_byte_write(chip, 0x18, 0x00|(SAMPLE_RATE));
#endif

#if(AUDIO_CHN >= 8)
    tp28xx_byte_write(chip, 0x19, 0x1F);
#else
    tp28xx_byte_write(chip, 0x19, 0x0F);
#endif

    tp28xx_byte_write(chip, 0x1A, 0x15);

    tp28xx_byte_write(chip, 0x37, 0x20);
    tp28xx_byte_write(chip, 0x38, 0x38);
    tp28xx_byte_write(chip, 0x3E, 0x00);

    tp28xx_byte_write(chip, 0x1D, 0x08);
    msleep(100);
    tp28xx_byte_write(chip, 0x1D, 0x00);

    tp28xx_byte_write(chip, 0x3C, 0x3f);
    msleep(200);
    tp28xx_byte_write(chip, 0x3C, 0x00);

    tp28xx_byte_write(chip, 0x3d, 0x01);//audio reset

    tp28xx_byte_write(chip, 0x40, bank);

}
static void TP28xx_audio_cascade(void)
{
    int i,j;
    int status_first,status_last;

   for (i = chips; i > 0; i --)
    {
        tp28xx_byte_write(i-1, 0x40, 0x40);
    }

    tp28xx_byte_write(chips-1, 0x47, 0x01);
    msleep(200);

    for(j = 0; j < 10; j++ )
    {
        for (i = chips; i > 0; i --)
        {
            tp28xx_byte_write(i-1, 0x3d, 0x01);
        }
        msleep(200);
        status_last = tp28xx_byte_read(chips-1, 0x5f);
        status_first = tp28xx_byte_read(0, chips+0x5e);
        if((status_first == status_last)&&(status_first != 0))
        {
            break;
        }

    }
    tp28xx_byte_write(chips-1, 0x47, 0x00);

}

static void TP2833_NTSC_DataSet(unsigned char chip)
{
    unsigned int tmp;
    tp28xx_byte_write(chip, 0x0c, 0x13);
    tp28xx_byte_write(chip, 0x0d, 0x50);
    tp28xx_byte_write(chip, 0x20, 0x40);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x2b, 0x70);
    tp28xx_byte_write(chip, 0x2d, 0x68);
    tp28xx_byte_write(chip, 0x2e, 0x5e);

    tp28xx_byte_write(chip, 0x30, 0x62);
    tp28xx_byte_write(chip, 0x31, 0xbb);
    tp28xx_byte_write(chip, 0x32, 0x96);
    tp28xx_byte_write(chip, 0x33, 0xc0);
    //tp28xx_byte_write(chip, 0x35, 0x25);
    tp28xx_byte_write(chip, 0x39, 0x04);
    tp28xx_byte_write(chip, 0x18, 0x11);
    tp28xx_byte_write(chip, 0x2c, 0x2a);

    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
}
static void TP2833_PAL_DataSet(unsigned char chip)
{
    unsigned int tmp;
    tp28xx_byte_write(chip, 0x0c, 0x13);
    tp28xx_byte_write(chip, 0x0d, 0x51);
    tp28xx_byte_write(chip, 0x20, 0x48);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x2b, 0x70);
    tp28xx_byte_write(chip, 0x2d, 0x60);
    tp28xx_byte_write(chip, 0x2e, 0x5e);

    tp28xx_byte_write(chip, 0x30, 0x7a);
    tp28xx_byte_write(chip, 0x31, 0x4a);
    tp28xx_byte_write(chip, 0x32, 0x4d);
    tp28xx_byte_write(chip, 0x33, 0xf0);
    //tp28xx_byte_write(chip, 0x35, 0x25);
    tp28xx_byte_write(chip, 0x39, 0x04);
    tp28xx_byte_write(chip, 0x18, 0x11);
    tp28xx_byte_write(chip, 0x2c, 0x2a);

    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
}
static void TP2833_V1_DataSet(unsigned char chip)
{
    unsigned int tmp;
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);
    tp28xx_byte_write(chip, 0x20, 0x30);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x2b, 0x58);
    tp28xx_byte_write(chip, 0x2d, 0x30);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0xbb);
    tp28xx_byte_write(chip, 0x32, 0x2e);
    tp28xx_byte_write(chip, 0x33, 0x90);
    //tp28xx_byte_write(chip, 0x35, 0x05);
    tp28xx_byte_write(chip, 0x39, 0x0c);
    tp28xx_byte_write(chip, 0x2c, 0x0a);

    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);

}
static void TP2833_V2_DataSet(unsigned char chip)
{
    unsigned int tmp;
    tp28xx_byte_write(chip, 0x0c, 0x13);
    tp28xx_byte_write(chip, 0x0d, 0x50);
    tp28xx_byte_write(chip, 0x20, 0x30);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x2b, 0x58);
    tp28xx_byte_write(chip, 0x2d, 0x30);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0xbb);
    tp28xx_byte_write(chip, 0x32, 0x2e);
    tp28xx_byte_write(chip, 0x33, 0x90);
    //tp28xx_byte_write(chip, 0x35, 0x25);
    tp28xx_byte_write(chip, 0x39, 0x08);
    tp28xx_byte_write(chip, 0x2c, 0x0a);

    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
}
static void TP2833_A720N_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x40;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x2d, 0x48);
    tp28xx_byte_write(chip, 0x2e, 0x5e);

    tp28xx_byte_write(chip, 0x30, 0x27);
    tp28xx_byte_write(chip, 0x31, 0x72);
    tp28xx_byte_write(chip, 0x32, 0x80);
    tp28xx_byte_write(chip, 0x33, 0x77);
    tp28xx_byte_write(chip, 0x07, 0x80);

}
static void TP2833_A720P_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x40;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x2d, 0x48);
    tp28xx_byte_write(chip, 0x2e, 0x5e);

    tp28xx_byte_write(chip, 0x30, 0x27);
    tp28xx_byte_write(chip, 0x31, 0x88);
    tp28xx_byte_write(chip, 0x32, 0x04);
    tp28xx_byte_write(chip, 0x33, 0x23);
    tp28xx_byte_write(chip, 0x07, 0x80);

}
static void TP2833_A1080N_DataSet(unsigned char chip)
{
    unsigned char tmp;
    //tp28xx_byte_write(chip, 0x14, 0x60);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x60;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x2d, 0x45);
    tp28xx_byte_write(chip, 0x2e, 0x50);

    tp28xx_byte_write(chip, 0x30, 0x29);
    tp28xx_byte_write(chip, 0x31, 0x65);
    tp28xx_byte_write(chip, 0x32, 0x78);
    tp28xx_byte_write(chip, 0x33, 0x16);
    tp28xx_byte_write(chip, 0x07, 0x80);

}
static void TP2833_A1080P_DataSet(unsigned char chip)
{
    unsigned char tmp;
    //tp28xx_byte_write(chip, 0x14, 0x60);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x60;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x2d, 0x45);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x29);
    tp28xx_byte_write(chip, 0x31, 0x61);
    tp28xx_byte_write(chip, 0x32, 0x78);
    tp28xx_byte_write(chip, 0x33, 0x16);
    tp28xx_byte_write(chip, 0x07, 0x80);
}
static void TP2833_A720P60_DataSet(unsigned char chip)
{
    unsigned char tmp;
    //tp28xx_byte_write(chip, 0x14, 0x60);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x60;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x2d, 0x45);
    tp28xx_byte_write(chip, 0x2e, 0x50);

    tp28xx_byte_write(chip, 0x30, 0x29);
    tp28xx_byte_write(chip, 0x31, 0x62);
    tp28xx_byte_write(chip, 0x32, 0x78);
    tp28xx_byte_write(chip, 0x33, 0x16);
    tp28xx_byte_write(chip, 0x07, 0x80);

}
static void TP2853C_NTSC_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tp28xx_byte_write(chip, 0x0c, 0x13);
    tp28xx_byte_write(chip, 0x0d, 0x50);
    tp28xx_byte_write(chip, 0x20, 0x40);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x2b, 0x70);
    tp28xx_byte_write(chip, 0x2d, 0x68);
    tp28xx_byte_write(chip, 0x2e, 0x57);

    tp28xx_byte_write(chip, 0x30, 0x62);
    tp28xx_byte_write(chip, 0x31, 0xbb);
    tp28xx_byte_write(chip, 0x32, 0x96);
    tp28xx_byte_write(chip, 0x33, 0xc0);
    //tp28xx_byte_write(chip, 0x35, 0x25);
    tp28xx_byte_write(chip, 0x39, 0x04);
    tp28xx_byte_write(chip, 0x18, 0x12);
    tp28xx_byte_write(chip, 0x2c, 0x2a);

    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x25, 0xff);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
}
static void TP2853C_PAL_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tp28xx_byte_write(chip, 0x0c, 0x13);
    tp28xx_byte_write(chip, 0x0d, 0x51);
    tp28xx_byte_write(chip, 0x20, 0x48);
    tp28xx_byte_write(chip, 0x22, 0x37);
    tp28xx_byte_write(chip, 0x23, 0x3f);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x2b, 0x70);
    tp28xx_byte_write(chip, 0x2d, 0x60);
    tp28xx_byte_write(chip, 0x2e, 0x56);

    tp28xx_byte_write(chip, 0x30, 0x7a);
    tp28xx_byte_write(chip, 0x31, 0x4a);
    tp28xx_byte_write(chip, 0x32, 0x4d);
    tp28xx_byte_write(chip, 0x33, 0xf0);
    //tp28xx_byte_write(chip, 0x35, 0x25);
    tp28xx_byte_write(chip, 0x39, 0x04);
    tp28xx_byte_write(chip, 0x18, 0x17);
    tp28xx_byte_write(chip, 0x2c, 0x2a);

    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x25, 0xff);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
}
static void TP2853C_V1_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);
    tp28xx_byte_write(chip, 0x20, 0x30);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2d, 0x30);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0xbb);
    tp28xx_byte_write(chip, 0x32, 0x2e);
    tp28xx_byte_write(chip, 0x33, 0x90);
    //tp28xx_byte_write(chip, 0x35, 0x05);
    tp28xx_byte_write(chip, 0x39, 0x0c);
    tp28xx_byte_write(chip, 0x2c, 0x0a);

    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x25, 0xff);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
}
static void TP2853C_V2_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tp28xx_byte_write(chip, 0x0c, 0x13);
    tp28xx_byte_write(chip, 0x0d, 0x50);
    tp28xx_byte_write(chip, 0x20, 0x30);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2d, 0x30);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0xbb);
    tp28xx_byte_write(chip, 0x32, 0x2e);
    tp28xx_byte_write(chip, 0x33, 0x90);
    //tp28xx_byte_write(chip, 0x35, 0x25);
    tp28xx_byte_write(chip, 0x39, 0x08);
    tp28xx_byte_write(chip, 0x2c, 0x0a);

    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x21, 0x84);
    tp28xx_byte_write(chip, 0x25, 0xff);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
}
static void TP2853C_A720P30_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x40;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x2d, 0x48);
    tp28xx_byte_write(chip, 0x2e, 0x5e);

    tp28xx_byte_write(chip, 0x30, 0x27);
    tp28xx_byte_write(chip, 0x31, 0x72);
    tp28xx_byte_write(chip, 0x32, 0x80);
    tp28xx_byte_write(chip, 0x33, 0x77);

    tp28xx_byte_write(chip, 0x21, 0x44);
    tp28xx_byte_write(chip, 0x25, 0xf0);
    tp28xx_byte_write(chip, 0x26, 0x01);
    tp28xx_byte_write(chip, 0x07, 0x80);

    tp28xx_byte_write(chip, 0x3B, 0x05);
}
static void TP2853C_A720P25_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x40;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x2d, 0x48);
    tp28xx_byte_write(chip, 0x2e, 0x5e);

    tp28xx_byte_write(chip, 0x30, 0x27);
    tp28xx_byte_write(chip, 0x31, 0x88);
    tp28xx_byte_write(chip, 0x32, 0x04);
    tp28xx_byte_write(chip, 0x33, 0x23);

    tp28xx_byte_write(chip, 0x21, 0x44);
    tp28xx_byte_write(chip, 0x25, 0xf0);
    tp28xx_byte_write(chip, 0x26, 0x01);
    tp28xx_byte_write(chip, 0x07, 0x80);

    tp28xx_byte_write(chip, 0x3B, 0x05);
}
static void TP2853C_A1080P30_DataSet(unsigned char chip)
{
    unsigned char tmp;

    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x60;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x2d, 0x45);
    tp28xx_byte_write(chip, 0x2e, 0x50);

    tp28xx_byte_write(chip, 0x30, 0x29);
    tp28xx_byte_write(chip, 0x31, 0x65);
    tp28xx_byte_write(chip, 0x32, 0x78);
    tp28xx_byte_write(chip, 0x33, 0x16);

    tp28xx_byte_write(chip, 0x21, 0x44);
    tp28xx_byte_write(chip, 0x25, 0xf0);
    tp28xx_byte_write(chip, 0x26, 0x01);
    tp28xx_byte_write(chip, 0x07, 0x80);

    tp28xx_byte_write(chip, 0x3B, 0x05);
}
static void TP2853C_A1080P25_DataSet(unsigned char chip)
{
    unsigned char tmp;

    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x60;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x2d, 0x45);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x29);
    tp28xx_byte_write(chip, 0x31, 0x61);
    tp28xx_byte_write(chip, 0x32, 0x78);
    tp28xx_byte_write(chip, 0x33, 0x16);

    tp28xx_byte_write(chip, 0x21, 0x44);
    tp28xx_byte_write(chip, 0x25, 0xf0);
    tp28xx_byte_write(chip, 0x26, 0x01);
    tp28xx_byte_write(chip, 0x07, 0x80);

    tp28xx_byte_write(chip, 0x3B, 0x05);
}
static void TP2853C_A720P60_DataSet(unsigned char chip)
{
    unsigned char tmp;

    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x60;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x2d, 0x45);
    tp28xx_byte_write(chip, 0x2e, 0x50);

    tp28xx_byte_write(chip, 0x30, 0x29);
    tp28xx_byte_write(chip, 0x31, 0x62);
    tp28xx_byte_write(chip, 0x32, 0x78);
    tp28xx_byte_write(chip, 0x33, 0x16);

    tp28xx_byte_write(chip, 0x21, 0x44);
    tp28xx_byte_write(chip, 0x25, 0xf0);
    tp28xx_byte_write(chip, 0x26, 0x01);
    tp28xx_byte_write(chip, 0x07, 0x80);

    tp28xx_byte_write(chip, 0x3B, 0x05);
}
static void TP2853C_C1080P25_DataSet(unsigned char chip)
{

    tp28xx_byte_write(chip, 0x13, 0x40);

    //tp28xx_byte_write(chip, 0x15, 0x13);
    //tp28xx_byte_write(chip, 0x16, 0x84);

    tp28xx_byte_write(chip, 0x20, 0x50);
    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2d, 0x54);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x41);
    tp28xx_byte_write(chip, 0x31, 0x82);
    tp28xx_byte_write(chip, 0x32, 0x27);
    tp28xx_byte_write(chip, 0x33, 0xa6);

    tp28xx_byte_write(chip, 0x28, 0x04);
    tp28xx_byte_write(chip, 0x26, 0x01);
    tp28xx_byte_write(chip, 0x07, 0x80);
    tp28xx_byte_write(chip, 0x27, 0x5a);
}
static void TP2853C_C720P25_DataSet(unsigned char chip)
{

    tp28xx_byte_write(chip, 0x13, 0x40);

    //tp28xx_byte_write(chip, 0x16, 0x40);

    tp28xx_byte_write(chip, 0x20, 0x3a);
    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2d, 0x42);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0x67);
    tp28xx_byte_write(chip, 0x32, 0x6f);
    tp28xx_byte_write(chip, 0x33, 0x31);

    tp28xx_byte_write(chip, 0x28, 0x04);
    tp28xx_byte_write(chip, 0x26, 0x01);
    tp28xx_byte_write(chip, 0x07, 0x80);
    tp28xx_byte_write(chip, 0x27, 0x5a);
}
static void TP2853C_C720P50_DataSet(unsigned char chip)
{

    tp28xx_byte_write(chip, 0x13, 0x40);

    //tp28xx_byte_write(chip, 0x16, 0x40);

    tp28xx_byte_write(chip, 0x20, 0x3a);
    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2d, 0x42);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x41);
    tp28xx_byte_write(chip, 0x31, 0x82);
    tp28xx_byte_write(chip, 0x32, 0x27);
    tp28xx_byte_write(chip, 0x33, 0xa6);

    tp28xx_byte_write(chip, 0x28, 0x04);
    tp28xx_byte_write(chip, 0x26, 0x01);
    tp28xx_byte_write(chip, 0x07, 0x80);
    tp28xx_byte_write(chip, 0x27, 0x5a);
}
static void TP2853C_C1080P30_DataSet(unsigned char chip)
{

    tp28xx_byte_write(chip, 0x13, 0x40);

    //tp28xx_byte_write(chip, 0x15, 0x13);
    //tp28xx_byte_write(chip, 0x16, 0x44);

    tp28xx_byte_write(chip, 0x20, 0x40);
    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2d, 0x47);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x41);
    tp28xx_byte_write(chip, 0x31, 0x82);
    tp28xx_byte_write(chip, 0x32, 0x27);
    tp28xx_byte_write(chip, 0x33, 0xa6);

    tp28xx_byte_write(chip, 0x28, 0x04);
    tp28xx_byte_write(chip, 0x26, 0x01);
    tp28xx_byte_write(chip, 0x07, 0x80);

    tp28xx_byte_write(chip, 0x27, 0x5a);
}
static void TP2853C_C720P30_DataSet(unsigned char chip)
{

    tp28xx_byte_write(chip, 0x13, 0x40);

    //tp28xx_byte_write(chip, 0x16, 0x02);

    tp28xx_byte_write(chip, 0x20, 0x30);
    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2d, 0x37);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0x67);
    tp28xx_byte_write(chip, 0x32, 0x6f);
    tp28xx_byte_write(chip, 0x33, 0x31);

    tp28xx_byte_write(chip, 0x28, 0x04);
    tp28xx_byte_write(chip, 0x26, 0x01);
    tp28xx_byte_write(chip, 0x07, 0x80);

    tp28xx_byte_write(chip, 0x27, 0x5a);
}
static void TP2853C_C720P60_DataSet(unsigned char chip)
{

    tp28xx_byte_write(chip, 0x13, 0x40);

    //tp28xx_byte_write(chip, 0x16, 0x02);

    tp28xx_byte_write(chip, 0x20, 0x30);
    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2d, 0x37);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x41);
    tp28xx_byte_write(chip, 0x31, 0x82);
    tp28xx_byte_write(chip, 0x32, 0x27);
    tp28xx_byte_write(chip, 0x33, 0xa6);

    tp28xx_byte_write(chip, 0x28, 0x04);
    tp28xx_byte_write(chip, 0x26, 0x01);
    tp28xx_byte_write(chip, 0x07, 0x80);

    tp28xx_byte_write(chip, 0x27, 0x5a);
}
//////////////////////////////////////////////////////////////
static void TP2826_NTSC_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tp28xx_byte_write(chip, 0x0c, 0x13);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x20, 0x40);
    tp28xx_byte_write(chip, 0x21, 0x88);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x70);
    tp28xx_byte_write(chip, 0x2c, 0x2a);
    tp28xx_byte_write(chip, 0x2d, 0x68);
    tp28xx_byte_write(chip, 0x2e, 0x57);

    tp28xx_byte_write(chip, 0x30, 0x62);
    tp28xx_byte_write(chip, 0x31, 0xbb);
    tp28xx_byte_write(chip, 0x32, 0x96);
    tp28xx_byte_write(chip, 0x33, 0xc0);
    //tp28xx_byte_write(chip, 0x35, 0x25);
    tp28xx_byte_write(chip, 0x39, 0x04);
    tp28xx_byte_write(chip, 0x18, 0x12);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
}
static void TP2826_PAL_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tp28xx_byte_write(chip, 0x0c, 0x13);
    tp28xx_byte_write(chip, 0x0d, 0x51);

    tp28xx_byte_write(chip, 0x20, 0x48);
    tp28xx_byte_write(chip, 0x21, 0x88);
    tp28xx_byte_write(chip, 0x22, 0x37);
    tp28xx_byte_write(chip, 0x23, 0x3f);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x70);
    tp28xx_byte_write(chip, 0x2c, 0x2a);
    tp28xx_byte_write(chip, 0x2d, 0x64);
    tp28xx_byte_write(chip, 0x2e, 0x56);

    tp28xx_byte_write(chip, 0x30, 0x7a);
    tp28xx_byte_write(chip, 0x31, 0x4a);
    tp28xx_byte_write(chip, 0x32, 0x4d);
    tp28xx_byte_write(chip, 0x33, 0xf0);
    //tp28xx_byte_write(chip, 0x35, 0x25);
    tp28xx_byte_write(chip, 0x39, 0x04);
    tp28xx_byte_write(chip, 0x18, 0x17);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
}
static void TP2826_V1_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tp28xx_byte_write(chip, 0x0c, 0x03);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x20, 0x30);
    tp28xx_byte_write(chip, 0x21, 0x88);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x30);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0xbb);
    tp28xx_byte_write(chip, 0x32, 0x2e);
    tp28xx_byte_write(chip, 0x33, 0x90);
    //tp28xx_byte_write(chip, 0x35, 0x05);
    tp28xx_byte_write(chip, 0x39, 0x1c);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
}
static void TP2826_V2_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tp28xx_byte_write(chip, 0x0c, 0x13);
    tp28xx_byte_write(chip, 0x0d, 0x50);

    tp28xx_byte_write(chip, 0x20, 0x30);
    tp28xx_byte_write(chip, 0x21, 0x88);
    tp28xx_byte_write(chip, 0x22, 0x36);
    tp28xx_byte_write(chip, 0x23, 0x3c);

    tp28xx_byte_write(chip, 0x25, 0xff);
    tp28xx_byte_write(chip, 0x26, 0x05);
    tp28xx_byte_write(chip, 0x27, 0x2d);
    tp28xx_byte_write(chip, 0x28, 0x00);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2c, 0x0a);
    tp28xx_byte_write(chip, 0x2d, 0x30);
    tp28xx_byte_write(chip, 0x2e, 0x70);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0xbb);
    tp28xx_byte_write(chip, 0x32, 0x2e);
    tp28xx_byte_write(chip, 0x33, 0x90);
    //tp28xx_byte_write(chip, 0x35, 0x25);
    tp28xx_byte_write(chip, 0x39, 0x18);

    tp28xx_byte_write(chip, 0x13, 0x00);
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp &= 0x9f;
    tp28xx_byte_write(chip, 0x14, tmp);
}

static void TP2826_A720P30_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x40;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x20, 0x40);
    tp28xx_byte_write(chip, 0x21, 0x46);

    tp28xx_byte_write(chip, 0x25, 0xfe);
    tp28xx_byte_write(chip, 0x26, 0x01);

    tp28xx_byte_write(chip, 0x2d, 0x48);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x27);
    tp28xx_byte_write(chip, 0x31, 0x72);
    tp28xx_byte_write(chip, 0x32, 0x80);
    tp28xx_byte_write(chip, 0x33, 0x70);



    //tp28xx_byte_write(chip, 0x3B, 0x05);
}
static void TP2826_A720P25_DataSet(unsigned char chip)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x40;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x20, 0x40);
    tp28xx_byte_write(chip, 0x21, 0x46);

    tp28xx_byte_write(chip, 0x25, 0xfe);
    tp28xx_byte_write(chip, 0x26, 0x01);

    tp28xx_byte_write(chip, 0x2d, 0x48);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x27);
    tp28xx_byte_write(chip, 0x31, 0x88);
    tp28xx_byte_write(chip, 0x32, 0x04);
    tp28xx_byte_write(chip, 0x33, 0x20);

    //tp28xx_byte_write(chip, 0x3B, 0x05);
}
static void TP2826_A1080P30_DataSet(unsigned char chip)
{
    unsigned char tmp;

    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x60;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x20, 0x38);
    tp28xx_byte_write(chip, 0x21, 0x46);

    tp28xx_byte_write(chip, 0x25, 0xfe);
    tp28xx_byte_write(chip, 0x26, 0x01);

    tp28xx_byte_write(chip, 0x2d, 0x44);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x29);
    tp28xx_byte_write(chip, 0x31, 0x65);
    tp28xx_byte_write(chip, 0x32, 0x78);
    tp28xx_byte_write(chip, 0x33, 0x10);

    //tp28xx_byte_write(chip, 0x3B, 0x05);
}
static void TP2826_A720P60_DataSet(unsigned char chip)
{
    unsigned char tmp;

    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x60;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x20, 0x38);
    tp28xx_byte_write(chip, 0x21, 0x46);

    tp28xx_byte_write(chip, 0x25, 0xfe);
    tp28xx_byte_write(chip, 0x26, 0x01);

    tp28xx_byte_write(chip, 0x2d, 0x44);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x29);
    tp28xx_byte_write(chip, 0x31, 0x62);
    tp28xx_byte_write(chip, 0x32, 0x78);
    tp28xx_byte_write(chip, 0x33, 0x10);

    //tp28xx_byte_write(chip, 0x3B, 0x05);
}
static void TP2826_A1080P25_DataSet(unsigned char chip)
{
    unsigned char tmp;

    tmp = tp28xx_byte_read(chip, 0x14);
    tmp |= 0x60;
    tp28xx_byte_write(chip, 0x14, tmp);

    tp28xx_byte_write(chip, 0x20, 0x3C);
    tp28xx_byte_write(chip, 0x21, 0x46);

    tp28xx_byte_write(chip, 0x25, 0xfe);
    tp28xx_byte_write(chip, 0x26, 0x01);

    tp28xx_byte_write(chip, 0x2d, 0x44);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x29);
    tp28xx_byte_write(chip, 0x31, 0x61);
    tp28xx_byte_write(chip, 0x32, 0xbe);
    tp28xx_byte_write(chip, 0x33, 0xd0);

    //tp28xx_byte_write(chip, 0x3B, 0x05);
}
static void TP2826_C1080P25_DataSet(unsigned char chip)
{

    tp28xx_byte_write(chip, 0x13, 0x40);

    tp28xx_byte_write(chip, 0x20, 0x50);

    tp28xx_byte_write(chip, 0x26, 0x01);
    tp28xx_byte_write(chip, 0x27, 0x5a);
    tp28xx_byte_write(chip, 0x28, 0x04);

    tp28xx_byte_write(chip, 0x2b, 0x60);

    tp28xx_byte_write(chip, 0x2d, 0x54);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x41);
    tp28xx_byte_write(chip, 0x31, 0x82);
    tp28xx_byte_write(chip, 0x32, 0x27);
    tp28xx_byte_write(chip, 0x33, 0xa2);

}
static void TP2826_C720P25_DataSet(unsigned char chip)
{

    tp28xx_byte_write(chip, 0x13, 0x40);

    tp28xx_byte_write(chip, 0x20, 0x3a);

    tp28xx_byte_write(chip, 0x26, 0x01);
    tp28xx_byte_write(chip, 0x27, 0x5a);
    tp28xx_byte_write(chip, 0x28, 0x04);

    tp28xx_byte_write(chip, 0x2b, 0x60);
    tp28xx_byte_write(chip, 0x2d, 0x36);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0x67);
    tp28xx_byte_write(chip, 0x32, 0x6f);
    tp28xx_byte_write(chip, 0x33, 0x33);

}
static void TP2826_C720P50_DataSet(unsigned char chip)
{

    tp28xx_byte_write(chip, 0x13, 0x40);

    tp28xx_byte_write(chip, 0x20, 0x3a);

    tp28xx_byte_write(chip, 0x26, 0x01);
    tp28xx_byte_write(chip, 0x27, 0x5a);
    tp28xx_byte_write(chip, 0x28, 0x04);

    tp28xx_byte_write(chip, 0x2b, 0x60);

    tp28xx_byte_write(chip, 0x2d, 0x42);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x41);
    tp28xx_byte_write(chip, 0x31, 0x82);
    tp28xx_byte_write(chip, 0x32, 0x27);
    tp28xx_byte_write(chip, 0x33, 0xa3);

}
static void TP2826_C1080P30_DataSet(unsigned char chip)
{

    tp28xx_byte_write(chip, 0x13, 0x40);

    tp28xx_byte_write(chip, 0x20, 0x3c);

    tp28xx_byte_write(chip, 0x26, 0x01);
    tp28xx_byte_write(chip, 0x27, 0x5a);
    tp28xx_byte_write(chip, 0x28, 0x04);

    tp28xx_byte_write(chip, 0x2b, 0x60);

    tp28xx_byte_write(chip, 0x2d, 0x4c);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x41);
    tp28xx_byte_write(chip, 0x31, 0x82);
    tp28xx_byte_write(chip, 0x32, 0x27);
    tp28xx_byte_write(chip, 0x33, 0xa4);

}
static void TP2826_C720P30_DataSet(unsigned char chip)
{

    tp28xx_byte_write(chip, 0x13, 0x40);

    tp28xx_byte_write(chip, 0x20, 0x30);

    tp28xx_byte_write(chip, 0x26, 0x01);
    tp28xx_byte_write(chip, 0x27, 0x5a);
    tp28xx_byte_write(chip, 0x28, 0x04);

    tp28xx_byte_write(chip, 0x2b, 0x60);

    tp28xx_byte_write(chip, 0x2d, 0x37);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x48);
    tp28xx_byte_write(chip, 0x31, 0x67);
    tp28xx_byte_write(chip, 0x32, 0x6f);
    tp28xx_byte_write(chip, 0x33, 0x30);

}
static void TP2826_C720P60_DataSet(unsigned char chip)
{

    tp28xx_byte_write(chip, 0x13, 0x40);

    tp28xx_byte_write(chip, 0x20, 0x30);

    tp28xx_byte_write(chip, 0x26, 0x01);
    tp28xx_byte_write(chip, 0x27, 0x5a);
    tp28xx_byte_write(chip, 0x28, 0x04);

    tp28xx_byte_write(chip, 0x2b, 0x60);

    tp28xx_byte_write(chip, 0x2d, 0x37);
    tp28xx_byte_write(chip, 0x2e, 0x40);

    tp28xx_byte_write(chip, 0x30, 0x41);
    tp28xx_byte_write(chip, 0x31, 0x82);
    tp28xx_byte_write(chip, 0x32, 0x27);
    tp28xx_byte_write(chip, 0x33, 0xa0);

}
////////////////////////////////////////////////////////////////////////////
static void TP2822_PTZ_init(unsigned char chip)
{
    unsigned int i;

    for( i = 0; i < chips; i++)
    {
        tp28xx_byte_write(chip, 0x40, i<<4); //bank

        tp28xx_byte_write(chip, 0xc9, 0x00);
        tp28xx_byte_write(chip, 0xca, 0x00);
        tp28xx_byte_write(chip, 0xcb, 0x06);
        tp28xx_byte_write(chip, 0xcc, 0x07);
        tp28xx_byte_write(chip, 0xcd, 0x08);
        tp28xx_byte_write(chip, 0xce, 0x09); //line6,7,8,9
        tp28xx_byte_write(chip, 0xcf, 0x03);
        tp28xx_byte_write(chip, 0xd0, 0x48);
        tp28xx_byte_write(chip, 0xd1, 0x34); //39 clock per bit 0.526us
        tp28xx_byte_write(chip, 0xd2, 0x60);
        tp28xx_byte_write(chip, 0xd3, 0x10);
        tp28xx_byte_write(chip, 0xd4, 0x04); //
        tp28xx_byte_write(chip, 0xd5, 0xf0);
        tp28xx_byte_write(chip, 0xd6, 0xd8);
        tp28xx_byte_write(chip, 0xd7, 0x17); //24bit

        tp28xx_byte_write(chip, 0xe1, 0x00);
        tp28xx_byte_write(chip, 0xe2, 0x00);
        tp28xx_byte_write(chip, 0xe3, 0x05);
        tp28xx_byte_write(chip, 0xe4, 0x06);
        tp28xx_byte_write(chip, 0xe5, 0x08);
        tp28xx_byte_write(chip, 0xe6, 0x09); //line6,7,8,9
        tp28xx_byte_write(chip, 0xe7, 0x03);
        tp28xx_byte_write(chip, 0xe8, 0x48);
        tp28xx_byte_write(chip, 0xe9, 0x34); //39 clock per bit 0.526us
        tp28xx_byte_write(chip, 0xea, 0x60);
        tp28xx_byte_write(chip, 0xeb, 0x10);
        tp28xx_byte_write(chip, 0xec, 0x04); //
        tp28xx_byte_write(chip, 0xed, 0xf0);
        tp28xx_byte_write(chip, 0xee, 0xd8);
        tp28xx_byte_write(chip, 0xef, 0x17); //24bit
    }

    tp28xx_byte_write(chip, 0x7E, 0x0F);   //TX channel enable
    tp28xx_byte_write(chip, 0xB9, 0x0F);   //RX channel enable
}
static void TP2822_PTZ_mode(unsigned char chip, unsigned char ch, unsigned char mod)
{
    unsigned int tmp, i;

    static const unsigned char PTZ_bank[4]= {0x00,0x00,0x10,0x10};
    static const unsigned char PTZ_reg[4][7]=
    {
        {0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8},
        {0xda,0xdb,0xdc,0xdd,0xde,0xdf,0xe0},
        {0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8},
        {0xda,0xdb,0xdc,0xdd,0xde,0xdf,0xe0}
    };
    static const unsigned char PTZ_dat[ ][7]=
    {
        {0x0b,0x0c,0x0d,0x0e,0x19,0x78,0x21}, //TVI1.0
        {0x0b,0x0c,0x0d,0x0e,0x33,0xf0,0x21}, //TVI2.0
        {0x0e,0x0f,0x10,0x11,0x66,0xf0,0x17}, //A1080p for 2833B 0.525
        {0x0e,0x0f,0x10,0x11,0x26,0xf0,0x57}, //A1080p for 2833C 0.525
        {0x0e,0x0f,0x00,0x00,0x26,0xe0,0xef}, //A720p for 2833C 0.525
        {0x0f,0x10,0x00,0x00,0x48,0xf0,0x6f}, //960H for 2833C
        {0x10,0x11,0x12,0x13,0x15,0xb8,0xa2}, //HDC 2833C
        {0x10,0x11,0x12,0x13,0x95,0xb8,0x22}, //HDC 2833B
        {0x0f,0x10,0x11,0x12,0x2c,0xf0,0x57}, //ACP 3M18 for 2853C 8+0.6
        {0x0f,0x10,0x11,0x12,0x19,0xf8,0x17},  //ACP 3M25 for 2853C 4+0.35
        {0x0f,0x10,0x12,0x16,0x19,0xf8,0x17},  //ACP 4M25 for 2853C 4+0.35
        {0x0f,0x10,0x12,0x16,0x2c,0xf0,0x57}, //ACP 4M15 5M12.5 for 2853C 8+0.6
        {0x16,0x17,0x18,0x19,0x2a,0xff,0x1a}, //HDC QHD25/30 for 2853C
    };

    //tmp = tp28xx_byte_read(chip, 0x40);
    //tmp &= 0xaf;
    //tmp |=PTZ_bank[ch];
    //tp28xx_byte_write(chip, 0x40, tmp); //reg bank1 switch for 2822
    tp28xx_byte_write(chip, 0x40, PTZ_bank[ch]); //reg bank1 switch for 2822

    for(i = 0; i < 7; i++)
    {
        if(PTZ_TVI == mod)
        {
            tmp = tp28xx_byte_read(chip, 0xf5); //check TVI 1 or 2
            if( (tmp >>ch) & 0x01)
            {
                tp28xx_byte_write(chip, PTZ_reg[ch][i], PTZ_dat[1][i]);
            }
            else
            {
                tp28xx_byte_write(chip, PTZ_reg[ch][i], PTZ_dat[0][i]);
            }
        }
        else if(PTZ_HDA_1080P == mod) //HDA 1080p
        {
            if(id[chip] > TP2834)
                tp28xx_byte_write(chip, PTZ_reg[ch][i], PTZ_dat[3][i]);
            else
                tp28xx_byte_write(chip, PTZ_reg[ch][i], PTZ_dat[2][i]);

        }
        else if(PTZ_HDA_720P == mod) //HDA 720p
        {
            if(id[chip] > TP2834)
                tp28xx_byte_write(chip, PTZ_reg[ch][i], PTZ_dat[4][i]);
        }
        else if(PTZ_HDA_CVBS == mod) //HDA CVBS
        {
            if(id[chip] > TP2834)
                tp28xx_byte_write(chip, PTZ_reg[ch][i], PTZ_dat[5][i]);
        }
        else if(PTZ_HDC == mod) // test
        {
            if(id[chip] > TP2834)
                tp28xx_byte_write(chip, PTZ_reg[ch][i], PTZ_dat[6][i]);
            else
                tp28xx_byte_write(chip, PTZ_reg[ch][i], PTZ_dat[7][i]);
        }
        else if(PTZ_HDA_3M18 == mod) // test
        {
            if(id[chip] > TP2834)
                tp28xx_byte_write(chip, PTZ_reg[ch][i], PTZ_dat[8][i]);
        }
        else if(PTZ_HDA_3M25 == mod) // test
        {
            if(id[chip] > TP2834)
                tp28xx_byte_write(chip, PTZ_reg[ch][i], PTZ_dat[9][i]);
        }
        else if(PTZ_HDA_4M25 == mod) // test
        {
            if(id[chip] > TP2834)
                tp28xx_byte_write(chip, PTZ_reg[ch][i], PTZ_dat[10][i]);
        }
        else if(PTZ_HDA_4M15 == mod) // test
        {
            if(id[chip] > TP2834)
                tp28xx_byte_write(chip, PTZ_reg[ch][i], PTZ_dat[11][i]);
        }
        else if(PTZ_HDC_QHD == mod) // test
        {
            if(id[chip] > TP2834)
                tp28xx_byte_write(chip, PTZ_reg[ch][i], PTZ_dat[12][i]);
        }
    }
    tp28xx_byte_write(chip, 0xB7, 0x0f); //enable TX interrupt flag

}
static void TP2826_RX_init(unsigned char chip)
{

        tp28xx_byte_write(chip, 0xc9, 0x00);
        tp28xx_byte_write(chip, 0xca, 0x00);
        tp28xx_byte_write(chip, 0xcb, 0x06);
        tp28xx_byte_write(chip, 0xcc, 0x07);
        tp28xx_byte_write(chip, 0xcd, 0x08);
        tp28xx_byte_write(chip, 0xce, 0x09); //line6,7,8,9
        tp28xx_byte_write(chip, 0xcf, 0x03);
        tp28xx_byte_write(chip, 0xd0, 0x48);
        tp28xx_byte_write(chip, 0xd1, 0x34); //39 clock per bit 0.526us
        tp28xx_byte_write(chip, 0xd2, 0x60);
        tp28xx_byte_write(chip, 0xd3, 0x10);
        tp28xx_byte_write(chip, 0xd4, 0x04); //
        tp28xx_byte_write(chip, 0xd5, 0xf0);
        tp28xx_byte_write(chip, 0xd6, 0xd8);
        tp28xx_byte_write(chip, 0xd7, 0x17); //24bit

}
static void TP2826_PTZ_mode(unsigned char chip, unsigned char ch, unsigned char mod)
{
    unsigned int tmp, i;

    static const unsigned char PTZ_reg[12]=
    {
        0x6f,0x70,0x71,0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8
    };
    static const unsigned char PTZ_dat[][12]=
    {
        {0x42,0x00,0x00,0x00,0x00,0x0b,0x0c,0x0d,0x0e,0x19,0x78,0x21}, //TVI1.0
        {0x42,0x00,0x00,0x00,0x00,0x0b,0x0c,0x0d,0x0e,0x33,0xf0,0x21}, //TVI2.0
        {0x42,0x00,0x00,0x00,0x00,0x0e,0x0f,0x10,0x11,0x26,0xf0,0x57}, //A1080p for 2826 0.525
        {0x42,0x00,0x00,0x00,0x00,0x0e,0x0f,0x00,0x00,0x26,0xe0,0xef}, //A720p for 2826 0.525
        {0x42,0x00,0x00,0x00,0x00,0x0f,0x10,0x00,0x00,0x4a,0xf0,0x6f}, //960H for 2826
        {0x42,0x00,0x00,0x00,0x00,0x10,0x11,0x12,0x13,0x15,0xb8,0xa2}, //HDC for 2826
        {0x42,0x00,0x00,0x00,0x00,0x0f,0x10,0x11,0x12,0x2c,0xf0,0x57}, //ACP 3M18 for 2826 8+0.6
        {0x42,0x00,0x00,0x00,0x00,0x0f,0x10,0x11,0x12,0x19,0xd0,0x17}, //ACP 3M2530 for 2826 4+0.35
        {0x42,0x00,0x00,0x00,0x00,0x0f,0x10,0x12,0x16,0x19,0xd0,0x17}, //ACP 4M2530_5M20 for 2826 4+0.35
        {0x42,0x00,0x00,0x00,0x00,0x0f,0x10,0x12,0x16,0x2c,0xf0,0x97}, //ACP 4M15 5M12.5 for 2826 8+0.6
        {0x42,0x00,0x00,0x00,0x00,0x16,0x17,0x18,0x19,0x2a,0xff,0x1a}, //HDC QHD25/30 for 2826
        {0x42,0x40,0x00,0x00,0x00,0x1a,0x1b,0x1c,0x1d,0x36,0x50,0x62}, //HDC 4K12.5 for 2826
        {0x42,0x40,0x00,0x00,0x00,0x1a,0x1b,0x1c,0x1d,0x36,0x50,0x5a}  //HDC 4K15 for 2826
    };

    tp28xx_byte_write(chip, 0x40, ch); //reg bank1 switch for 2826

    for(i = 0; i < 12; i++)
    {
        if(PTZ_TVI == mod)
        {
            tmp = tp28xx_byte_read(chip, 0xf5); //check TVI 1 or 2
            if( (tmp >>ch) & 0x01)
            {
                tp28xx_byte_write(chip, PTZ_reg[i], PTZ_dat[1][i]);
            }
            else
            {
                tp28xx_byte_write(chip, PTZ_reg[i], PTZ_dat[0][i]);
            }
        }
        else if(PTZ_HDA_1080P == mod) //HDA 1080p
        {
                tp28xx_byte_write(chip, PTZ_reg[i], PTZ_dat[2][i]);

        }
        else if(PTZ_HDA_720P == mod) //HDA 720p
        {
                tp28xx_byte_write(chip, PTZ_reg[i], PTZ_dat[3][i]);
        }
        else if(PTZ_HDA_CVBS == mod) //HDA CVBS
        {
                tp28xx_byte_write(chip, PTZ_reg[i], PTZ_dat[4][i]);
        }
        else if(PTZ_HDC == mod) // test
        {
                tp28xx_byte_write(chip, PTZ_reg[i], PTZ_dat[5][i]);

        }
        else if(PTZ_HDA_3M18 == mod) // test
        {
                tp28xx_byte_write(chip, PTZ_reg[i], PTZ_dat[6][i]);

        }
        else if(PTZ_HDA_3M25 == mod) // test
        {
                tp28xx_byte_write(chip, PTZ_reg[i], PTZ_dat[7][i]);

        }
        else if(PTZ_HDA_4M25 == mod) // test
        {
                tp28xx_byte_write(chip, PTZ_reg[i], PTZ_dat[8][i]);

        }
        else if(PTZ_HDA_4M15 == mod) // test
        {
                tp28xx_byte_write(chip, PTZ_reg[i], PTZ_dat[9][i]);

        }
        else if(PTZ_HDC_QHD == mod) // test
        {
                tp28xx_byte_write(chip, PTZ_reg[i], PTZ_dat[10][i]);

        }
        else if(PTZ_HDC_8M12 == mod) // test
        {
                tp28xx_byte_write(chip, PTZ_reg[i], PTZ_dat[11][i]);

        }
        else if(PTZ_HDC_8M15 == mod) // test
        {
                tp28xx_byte_write(chip, PTZ_reg[i], PTZ_dat[12][i]);

        }
    }

    tmp = tp28xx_byte_read(chip, 0x71);
    tmp |= 0x20;
    tp28xx_byte_write(chip, 0x71, tmp);

}
static void TP2826_PTZ_new(unsigned char chip, unsigned char ch, unsigned char mod)
{
    unsigned int tmp;

    TP2826_PTZ_mode(chip, ch, mod);

    //tmp = tp28xx_byte_read(chip, 0x6f);
    //tmp &= 0xbf;
    //tp28xx_byte_write(chip, 0x6f, tmp);
    tmp = tp28xx_byte_read(chip, 0x71);
    tmp |= 0x20;
    tp28xx_byte_write(chip, 0x71, tmp);

}

static void TP2826_output(unsigned char chip)
{
    unsigned int tmp;

    tp28xx_byte_write(chip, 0xF1, 0x07);
    //tp28xx_byte_write(chip, 0x4D, 0x07);
    //tp28xx_byte_write(chip, 0x4E, 0x05);
    tp28xx_byte_write(chip, 0x4f, 0x03);

    if( SDR_1CH == output[chip] )
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x88);
        tp28xx_byte_write(chip, 0xF4, 0x00); //
        tp28xx_byte_write(chip, 0xF6, 0x00);
        tp28xx_byte_write(chip, 0xF8, 0x11);
        tp28xx_byte_write(chip, 0x50, 0x00); //
        tp28xx_byte_write(chip, 0x52, 0x00); //
        tp28xx_byte_write(chip, 0xF3, 0x00);
        tp28xx_byte_write(chip, 0xF2, 0x00);
        if(TP2802_720P25V2 == mode || TP2802_720P30V2 == mode || TP2802_PAL == mode || TP2802_NTSC == mode )
        {
            tmp = tp28xx_byte_read(chip, 0xFA);
            tmp &= 0x88;
            tmp |= 0x11;
            tp28xx_byte_write(chip, 0xFA, tmp);
            tmp = tp28xx_byte_read(chip, 0xFB);
            tmp &= 0x88;
            tmp |= 0x11;
            tp28xx_byte_write(chip, 0xFB, tmp);
        }
        else if(FLAG_HALF_MODE == (mode & FLAG_HALF_MODE) )
        {
            tmp = tp28xx_byte_read(chip, 0xFA);
            tmp &= 0x88;
            tmp |= 0x43;
            tp28xx_byte_write(chip, 0xFA, tmp);
            tmp = tp28xx_byte_read(chip, 0xFB);
            tmp &= 0x88;
            tmp |= 0x65;
            tp28xx_byte_write(chip, 0xFB, tmp);
        }
    }
    else if(SDR_2CH == output[chip])
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x88);
        tp28xx_byte_write(chip, 0xF4, 0x00);
        tp28xx_byte_write(chip, 0xF6, 0x10);
        tp28xx_byte_write(chip, 0xF8, 0x23);
        tp28xx_byte_write(chip, 0x50, 0x00); //
        tp28xx_byte_write(chip, 0x52, 0x00); //
        tp28xx_byte_write(chip, 0xF3, 0x00);
        tp28xx_byte_write(chip, 0xF2, 0x00);
    }
    else if(DDR_2CH == output[chip])
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x00);
        //tp28xx_byte_write(chip, 0x45, 0x54); //PLL 297M
        tp28xx_byte_write(chip, 0xF4, 0xA0); //output clock 148.5M
        tp28xx_byte_write(chip, 0xF6, 0x10); //
        tp28xx_byte_write(chip, 0xF8, 0x23); //
        tp28xx_byte_write(chip, 0x50, 0x00); //
        tp28xx_byte_write(chip, 0x52, 0x00); //
        tp28xx_byte_write(chip, 0xF3, 0x07);
        tp28xx_byte_write(chip, 0xF2, 0x07);

    }
    else if(DDR_4CH == output[chip])
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x00);
        //tp28xx_byte_write(chip, 0x45, 0x54); //PLL 297M
        tp28xx_byte_write(chip, 0xF4, 0xA0); //output clock 148.5M
        tp28xx_byte_write(chip, 0xF6, 0x10); //
        tp28xx_byte_write(chip, 0xF8, 0x10); //
        tp28xx_byte_write(chip, 0x50, 0xB2); //
        tp28xx_byte_write(chip, 0x52, 0xB2); //
        tp28xx_byte_write(chip, 0xF3, 0x07);
        tp28xx_byte_write(chip, 0xF2, 0x07);
    }
    else if( DDR_1CH == output[chip] )
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x88);
        //tp28xx_byte_write(chip, 0x45, 0x54); //PLL 297M
        tp28xx_byte_write(chip, 0xF4, 0xA0); //output clock 148.5M
        tp28xx_byte_write(chip, 0xF6, 0x40);
        tp28xx_byte_write(chip, 0xF8, 0x51);
        tp28xx_byte_write(chip, 0x50, 0x00); //
        tp28xx_byte_write(chip, 0x52, 0x00); //
        tp28xx_byte_write(chip, 0xF3, 0x77);
        tp28xx_byte_write(chip, 0xF2, 0x77);
    }
    tp28xx_byte_write(chip, 0x4D, 0x07);
    tp28xx_byte_write(chip, 0x4E, 0x05);
}
static void TP2823_output(unsigned char chip)
{
    unsigned int tmp;

    if(DDR_1CH == output[chip]) output[chip] = SDR_1CH;

    if( SDR_1CH == output[chip] )
    {

        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xF6, 0x00);
        tp28xx_byte_write(chip, 0xF7, 0x11);
        if(TP2802_720P25V2 == mode || TP2802_720P30V2 == mode || TP2802_PAL == mode || TP2802_NTSC == mode )
        {
            tmp = tp28xx_byte_read(chip, 0xFA);
            tmp &= 0x88;
            tmp |= 0x11;
            tp28xx_byte_write(chip, 0xFA, tmp);
        }
        else if(FLAG_HALF_MODE == (mode & FLAG_HALF_MODE) )
        {
            tmp = tp28xx_byte_read(chip, 0xFA);
            tmp &= 0x88;
            tmp |= 0x43;
            tp28xx_byte_write(chip, 0xFA, tmp);
        }
    }
    else if(SDR_2CH == output[chip])
    {
        tp28xx_byte_write(chip, 0xFA, 0x00);
        tp28xx_byte_write(chip, 0xF6, 0x10);
        tp28xx_byte_write(chip, 0xF7, 0x23);

    }
    else if(DDR_2CH == output[chip])
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xF6, 0x18); //
        tp28xx_byte_write(chip, 0xF7, 0x23);
        tp28xx_byte_write(chip, 0x45, 0x54); //PLL 297M
        tp28xx_byte_write(chip, 0xF4, 0xA0); //output clock 148.5M

        tp28xx_byte_write(chip, 0xF3, 0x22);
        tp28xx_byte_write(chip, 0x46, DDR_2CH_MUX[CH_ALL]);

    }
    else if(DDR_4CH == output[chip])
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xF6, 0x09); //
        tp28xx_byte_write(chip, 0xF7, 0x01);
        tp28xx_byte_write(chip, 0x50, 0xA3); //
        tp28xx_byte_write(chip, 0x51, 0xA3);
        tp28xx_byte_write(chip, 0x45, 0x54); //PLL 297M
        tp28xx_byte_write(chip, 0xF4, 0xA0); //output clock 148.5M

        tp28xx_byte_write(chip, 0xF3, 0x11);
        tp28xx_byte_write(chip, 0x46, 0xff);
    }
}
static void TP2834_output(unsigned char chip)
{
    unsigned int tmp;

    if(DDR_1CH == output[chip]) output[chip] = SDR_1CH;

    if( SDR_1CH == output[chip] )
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x88);
        tp28xx_byte_write(chip, 0xF6, 0x00);
        tp28xx_byte_write(chip, 0xF7, 0x11);
        tp28xx_byte_write(chip, 0xF8, 0x22);
        tp28xx_byte_write(chip, 0xF9, 0x33);
        if(TP2802_720P25V2 == mode || TP2802_720P30V2 == mode || TP2802_PAL == mode || TP2802_NTSC == mode )
        {
            tmp = tp28xx_byte_read(chip, 0xFA);
            tmp &= 0x88;
            tmp |= 0x11;
            tp28xx_byte_write(chip, 0xFA, tmp);
            tmp = tp28xx_byte_read(chip, 0xFB);
            tmp &= 0x88;
            tmp |= 0x11;
            tp28xx_byte_write(chip, 0xFB, tmp);
        }
        else if(FLAG_HALF_MODE == (mode & FLAG_HALF_MODE) )
        {
            tmp = tp28xx_byte_read(chip, 0xFA);
            tmp &= 0x88;
            tmp |= 0x43;
            tp28xx_byte_write(chip, 0xFA, tmp);
            tmp = tp28xx_byte_read(chip, 0xFB);
            tmp &= 0x88;
            tmp |= 0x65;
            tp28xx_byte_write(chip, 0xFB, tmp);
        }
    }
    else if(SDR_2CH == output[chip])
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x88);
        tp28xx_byte_write(chip, 0xF6, 0x10);
        tp28xx_byte_write(chip, 0xF7, 0x32);
        tp28xx_byte_write(chip, 0xF8, 0x10);
        tp28xx_byte_write(chip, 0xF9, 0x23);

    }
    else if(DDR_2CH == output[chip])
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x88);
        tp28xx_byte_write(chip, 0x45, 0x54); //PLL 297M
        tp28xx_byte_write(chip, 0xF4, 0xA0); //output clock 148.5M
        tp28xx_byte_write(chip, 0xF6, 0x18); //
        tp28xx_byte_write(chip, 0xF7, 0x23);
        tp28xx_byte_write(chip, 0xF8, 0x10); //
        tp28xx_byte_write(chip, 0xF9, 0x23);
        tp28xx_byte_write(chip, 0xF3, 0x22);
        tp28xx_byte_write(chip, 0xF2, 0x22);

    }
    else if(DDR_4CH == output[chip])
    {
        tp28xx_byte_write(chip, 0xFA, 0x88);
        tp28xx_byte_write(chip, 0xFB, 0x88);
        tp28xx_byte_write(chip, 0x45, 0x54); //PLL 297M
        tp28xx_byte_write(chip, 0xF4, 0xA0); //output clock 148.5M
        tp28xx_byte_write(chip, 0xF6, 0x28); //
        tp28xx_byte_write(chip, 0xF7, 0x02);
        tp28xx_byte_write(chip, 0xF8, 0x20); //
        tp28xx_byte_write(chip, 0xF9, 0x02);
        tp28xx_byte_write(chip, 0x50, 0xB1); //
        tp28xx_byte_write(chip, 0x51, 0x93);
        tp28xx_byte_write(chip, 0x52, 0xB1); //
        tp28xx_byte_write(chip, 0x53, 0x93);
        tp28xx_byte_write(chip, 0xF3, 0x11);
        tp28xx_byte_write(chip, 0xF2, 0x11);

    }
}
static void TP28xx_ChannelID(unsigned char chip)
{

    tp28xx_byte_write(chip, 0x40, 0x00);
    tp28xx_byte_write(chip, 0x34, 0x10);
    tp28xx_byte_write(chip, 0x40, 0x01);
    tp28xx_byte_write(chip, 0x34, 0x11);
    if(DDR_4CH == output[chip])
    {
        tp28xx_byte_write(chip, 0x40, 0x02);
        tp28xx_byte_write(chip, 0x34, 0x12);
        tp28xx_byte_write(chip, 0x40, 0x03);
        tp28xx_byte_write(chip, 0x34, 0x13);
    }
    else
    {
        tp28xx_byte_write(chip, 0x40, 0x02);
        tp28xx_byte_write(chip, 0x34, 0x10);
        tp28xx_byte_write(chip, 0x40, 0x03);
        tp28xx_byte_write(chip, 0x34, 0x11);
    }

}

///////////////////////////////////////////////////////////////
static void tp2802_comm_init( int chip)
{
    unsigned int val;

    tp2802_set_reg_page(chip, CH_ALL);

    if(TP2831 == id[chip] ) // Added TP2831 ID - 10/15/2018
    {
        TP2831_reset_default(chip, CH_ALL);

        TP2831_output(chip);

        if(SDR_1CH == output[chip])
        {
            tp28xx_byte_write(chip, 0x02, 0xC0|SAV_HEADER_1MUX); //BT1120/BT656 header
        }
        else
        {
            tp28xx_byte_write(chip, 0x02, 0xC8); //BT656 header
        }

        tp2802_set_video_mode(chip, mode, CH_ALL, STD_TVI);

#if(WDT)
        tp28xx_byte_write(chip, 0x26, 0x04);
#endif

        TP28xx_ChannelID(chip);

        TP2831_Audio_DataSet(chip);
    }
    else if(TP2830 == id[chip] ) // Added TP2830 ID - 10/15/2018
    {
        TP2831_reset_default(chip, CH_ALL);

        TP2830_output(chip);

        if(SDR_1CH == output[chip])
        {
            tp28xx_byte_write(chip, 0x02, 0xC0|SAV_HEADER_1MUX); //BT1120/BT656 header
        }
        else
        {
            tp28xx_byte_write(chip, 0x02, 0xC8); //BT656 header
        }

        tp2802_set_video_mode(chip, mode, CH_ALL, STD_HDC);

#if(WDT)
        tp28xx_byte_write(chip, 0x26, 0x04);
#endif

        TP28xx_ChannelID(chip);

        TP2831_Audio_DataSet(chip);

    }
    else if(TP2828 == id[chip] )
    {
        TP2829_reset_default(chip, CH_ALL);

        TP2828_output(chip);

        if(SDR_1CH == output[chip])
        {
            tp28xx_byte_write(chip, 0x02, 0xC0|SAV_HEADER_1MUX); //BT1120/BT656 header
        }
        else
        {
            tp28xx_byte_write(chip, 0x02, 0xC8); //BT656 header
        }

        tp2802_set_video_mode(chip, mode, CH_ALL, STD_TVI);

#if(WDT)
        tp28xx_byte_write(chip, 0x26, 0x04);
#endif

        TP2829_RX_init(chip, PTZ_RX_ACP1);

        TP28xx_ChannelID(chip);

        TP2829_Audio_DataSet(chip);

    }
    else if(TP2826 == id[chip] || TP2816 == id[chip])
    {

        TP2826_reset_default(chip, CH_ALL);

        TP2826_output(chip);

        if(SDR_1CH == output[chip])
        {
            tp28xx_byte_write(chip, 0x02, 0xC0|SAV_HEADER_1MUX); //BT1120/BT656 header
        }
        else
        {
            tp28xx_byte_write(chip, 0x02, 0xC8); //BT656 header
        }

        tp2802_set_video_mode(chip, mode, CH_ALL, STD_TVI);

#if(WDT)
        tp28xx_byte_write(chip, 0x26, 0x04);
#endif

        TP2826_RX_init(chip);

        TP28xx_ChannelID(chip);

        TP2834_Audio_DataSet(chip);

    }
    else if(TP2853C == id[chip] || TP2833C == id[chip] || TP2823C == id[chip])
    {
        //PLL reset
        //val = tp28xx_byte_read(chip, 0x44);
        //tp28xx_byte_write(chip, 0x44, val|0x40);
        //msleep(10);
        //tp28xx_byte_write(chip, 0x44, val);

        //MUX output
        TP2823_output(chip);

        val = tp28xx_byte_read(chip, 0xf5);         //TP28x3C F5 MSB must be 1
        tp28xx_byte_write(chip, 0xf5, 0xf0|val);
/*
        tp28xx_byte_write(chip, 0x07, 0xC0);
        tp28xx_byte_write(chip, 0x0B, 0xC0);
        tp28xx_byte_write(chip, 0x21, 0x84);
        tp28xx_byte_write(chip, 0x38, 0x00);
        tp28xx_byte_write(chip, 0x3A, 0x32);
        //tp28xx_byte_write(chip, 0x3B, 0x05);
        tp28xx_byte_write(chip, 0x3B, 0x25);
*/
        tp2853C_reset_default(chip, CH_ALL);

        if(SDR_1CH == output[chip])
        {
            tp28xx_byte_write(chip, 0x02, 0xC0|SAV_HEADER_1MUX); //BT1120/BT656 header
        }
        else
        {
            tp28xx_byte_write(chip, 0x02, 0xC8); //BT656 header
        }

        tp2802_set_video_mode(chip, mode, CH_ALL, STD_TVI);

#if(WDT)
        tp28xx_byte_write(chip, 0x26, 0x04);
#endif

        tp28xx_byte_write(chip, 0x4D, 0x03);
        tp28xx_byte_write(chip, 0x4E, 0x03);
        tp28xx_byte_write(chip, 0x4F, 0x01);


#if(TP28XX_EVK)
        tp28xx_byte_write(chip, 0xF3, 0x88); //for EVK
#endif

        //channel ID
        TP28xx_ChannelID(chip);

        TP2822_PTZ_init(chip);

        TP2834_Audio_DataSet(chip);

    }
    else if(TP2834 == id[chip])
    {
/*
        tp28xx_byte_write(chip, 0x07, 0xC0);
        tp28xx_byte_write(chip, 0x0B, 0xC0);
        //tp28xx_byte_write(chip, 0x3A, 0x70);
        tp28xx_byte_write(chip, 0x22, 0x35);
*/
        tp2834_reset_default(chip, CH_ALL);

        if(SDR_1CH == output[chip])
        {
            tp28xx_byte_write(chip, 0x02, 0xC0|SAV_HEADER_1MUX); //BT1120/BT656 header
        }
        else
        {
            tp28xx_byte_write(chip, 0x02, 0xC8); //BT656 header
        }

        tp2802_set_video_mode(chip, mode, CH_ALL, STD_TVI);

#if (WDT)
        if(TP2802_NTSC == mode )
        {
            tp28xx_byte_write(chip, 0x26, 0x11);
        }
        else
        {
            tp28xx_byte_write(chip, 0x26, 0x01);
        }
#endif

        tp28xx_byte_write(chip, 0x4D, 0x0f);
        tp28xx_byte_write(chip, 0x4E, 0x0f);

        //MUX output
        TP2834_output(chip);

#if(TP28XX_EVK)
        tp28xx_byte_write(chip, 0xF3, 0x88); //for EVK
#endif
        //channel ID
        TP28xx_ChannelID(chip);

        TP2822_PTZ_init(chip);

        TP2834_Audio_DataSet(chip);

        //ADC reset
        tp28xx_byte_write(chip, 0x40, CH_ALL); //write 4 channels
        tp28xx_byte_write(chip, 0x3B, 0x33);
        tp28xx_byte_write(chip, 0x3B, 0x03);

        //soft reset
        tp28xx_byte_write(chip, 0x40, CH_ALL); //write 4 channels
        val = tp28xx_byte_read(chip, 0x06);
        tp28xx_byte_write(chip, 0x06, 0x80|val);
    }
    else if(TP2833 == id[chip])
    {

/*
        tp28xx_byte_write(chip, 0x07, 0xC0);
        tp28xx_byte_write(chip, 0x0B, 0xC0);
        tp28xx_byte_write(chip, 0x22, 0x36);
        tp28xx_byte_write(chip, 0x38, 0x40);
*/
        tp2833_reset_default(chip, CH_ALL);

        if(SDR_1CH == output[chip])
        {
            tp28xx_byte_write(chip, 0x02, 0xC0|SAV_HEADER_1MUX); //BT1120/BT656 header
        }
        else
        {
            tp28xx_byte_write(chip, 0x02, 0xC8); //BT656 header
        }

        tp2802_set_video_mode(chip, mode, CH_ALL, STD_TVI);

#if(WDT)
        tp28xx_byte_write(chip, 0x26, 0x04);
#endif

        tp28xx_byte_write(chip, 0x4D, 0x03);
        tp28xx_byte_write(chip, 0x4E, 0x03);
        tp28xx_byte_write(chip, 0x4F, 0x01);

        //MUX output
        TP2823_output(chip);

#if(TP28XX_EVK)
        tp28xx_byte_write(chip, 0xF3, 0x88); //for EVK
#endif

        //channel ID
        TP28xx_ChannelID(chip);

        TP2822_PTZ_init(chip);

        TP2834_Audio_DataSet(chip);

        //ADC reset
        tp28xx_byte_write(chip, 0x40, CH_ALL); //write 4 channels
        tp28xx_byte_write(chip, 0x3B, 0x33);
        tp28xx_byte_write(chip, 0x3B, 0x03);

        //soft reset
        tp28xx_byte_write(chip, 0x40, CH_ALL); //write 4 channels
        val = tp28xx_byte_read(chip, 0x06);
        tp28xx_byte_write(chip, 0x06, 0x80|val);

    }
    else if(TP2823 == id[chip])
    {

        tp28xx_byte_write(chip, 0x0b, 0x60);
        tp28xx_byte_write(chip, 0x22, 0x34);
        tp28xx_byte_write(chip, 0x23, 0x44);
        tp28xx_byte_write(chip, 0x38, 0x40);

        if(SDR_1CH == output[chip])
        {
            tp28xx_byte_write(chip, 0x02, 0xC0|SAV_HEADER_1MUX); //BT1120/BT656 header
        }
        else
        {
            tp28xx_byte_write(chip, 0x02, 0xC8); //BT656 header
        }
        tp2802_set_video_mode(chip, mode, CH_ALL, STD_TVI);

#if (WDT)
        if(TP2802_NTSC == mode )
        {
            tp28xx_byte_write(chip, 0x26, 0x11);
        }
        else
        {
            tp28xx_byte_write(chip, 0x26, 0x01);
        }

#endif
        tp28xx_byte_write(chip, 0x4D, 0x03);
        tp28xx_byte_write(chip, 0x4E, 0x33);

        //output
        TP2823_output(chip);

#if(TP28XX_EVK)
        tp28xx_byte_write(chip, 0xF3, 0x88); //for EVK
#endif

        //channel ID
        TP28xx_ChannelID(chip);

        TP2822_PTZ_init(chip);

        TP2823_Audio_DataSet(chip);

        //ADC reset
        tp28xx_byte_write(chip, 0x40, CH_ALL); //write 4 channels
        tp28xx_byte_write(chip, 0x3B, 0x33);
        tp28xx_byte_write(chip, 0x3B, 0x03);

        //soft reset
        tp28xx_byte_write(chip, 0x40, CH_ALL); //write 4 channels
        val = tp28xx_byte_read(chip, 0x06);
        tp28xx_byte_write(chip, 0x06, 0x80|val);

    }
    else if(TP2822 == id[chip])
    {
        //tp28xx_byte_write(chip, 0x07, 0xC0);
        //tp28xx_byte_write(chip, 0x0B, 0xC0);
        tp28xx_byte_write(chip, 0x22, 0x38);
        tp28xx_byte_write(chip, 0x2E, 0x60);

        tp28xx_byte_write(chip, 0x30, 0x48);
        tp28xx_byte_write(chip, 0x31, 0xBB);
        tp28xx_byte_write(chip, 0x32, 0x2E);
        tp28xx_byte_write(chip, 0x33, 0x90);

        //tp28xx_byte_write(chip, 0x35, 0x45);
        tp28xx_byte_write(chip, 0x39, 0x00);
        tp28xx_byte_write(chip, 0x3A, 0x01);
        tp28xx_byte_write(chip, 0x3D, 0x08);
        tp28xx_byte_write(chip, 0x4D, 0x03);
        tp28xx_byte_write(chip, 0x4E, 0x33);

        if(SDR_1CH == output[chip])
        {
            tp28xx_byte_write(chip, 0x02, 0xC0|SAV_HEADER_1MUX); //BT1120/BT656 header
        }
        else
        {
            tp28xx_byte_write(chip, 0x02, 0xC8); //BT656 header
        }

        tp2802_set_video_mode(chip, mode, CH_ALL, STD_TVI);

        //output
        TP2823_output(chip);

        //channel ID
        TP28xx_ChannelID(chip);

        //bank 1 for TX/RX 3,4
        TP2822_PTZ_init(chip);

        tp28xx_byte_write(chip, 0x40, CH_ALL); //all ch reset
        tp28xx_byte_write(chip, 0x3D, 0x00);

        //ADC reset
        tp28xx_byte_write(chip, 0x40, CH_ALL); //write 4 channels
        tp28xx_byte_write(chip, 0x3B, 0x33);
        tp28xx_byte_write(chip, 0x3B, 0x03);

        //soft reset
        tp28xx_byte_write(chip, 0x40, CH_ALL); //write 4 channels
        val = tp28xx_byte_read(chip, 0x06);
        tp28xx_byte_write(chip, 0x06, 0x80|val);

    }
    else if(TP2802D == id[chip])
    {
        tp28xx_byte_write(chip, 0x02, 0xC0|SAV_HEADER_1MUX); //8 bit BT1120/BT656 mode
        tp28xx_byte_write(chip, 0x07, 0xC0);
        tp28xx_byte_write(chip, 0x0B, 0xC0);
        tp28xx_byte_write(chip, 0x2b, 0x4a);
        tp28xx_byte_write(chip, 0x2E, 0x60);

        tp28xx_byte_write(chip, 0x30, 0x48);
        tp28xx_byte_write(chip, 0x31, 0xBB);
        tp28xx_byte_write(chip, 0x32, 0x2E);
        tp28xx_byte_write(chip, 0x33, 0x90);

        tp28xx_byte_write(chip, 0x23, 0x50);
        tp28xx_byte_write(chip, 0x39, 0x42);
        tp28xx_byte_write(chip, 0x3A, 0x01);
        tp28xx_byte_write(chip, 0x4d, 0x0f);
        tp28xx_byte_write(chip, 0x4e, 0xff);


        //now TP2801A just support 2 lines, to disable line3&4, else IRQ is in trouble.
        tp28xx_byte_write(chip, 0x40, 0x01);
        tp28xx_byte_write(chip, 0x50, 0x00);
        tp28xx_byte_write(chip, 0x51, 0x00);
        tp28xx_byte_write(chip, 0x52, 0x00);
        tp28xx_byte_write(chip, 0x7F, 0x00);
        tp28xx_byte_write(chip, 0x80, 0x00);
        tp28xx_byte_write(chip, 0x81, 0x00);

        //0x50~0xb2 need bank switch
        tp28xx_byte_write(chip, 0x40, 0x00);
        //TX total 34bit, valid load 32bit
        tp28xx_byte_write(chip, 0x50, 0x00);
        tp28xx_byte_write(chip, 0x51, 0x0b);
        tp28xx_byte_write(chip, 0x52, 0x0c);
        tp28xx_byte_write(chip, 0x53, 0x19);
        tp28xx_byte_write(chip, 0x54, 0x78);
        tp28xx_byte_write(chip, 0x55, 0x21);
        tp28xx_byte_write(chip, 0x7e, 0x0f);   //

        // RX total 40bit, valid load 39bit
        tp28xx_byte_write(chip, 0x7F, 0x00);
        tp28xx_byte_write(chip, 0x80, 0x07);
        tp28xx_byte_write(chip, 0x81, 0x08);
        tp28xx_byte_write(chip, 0x82, 0x04);
        tp28xx_byte_write(chip, 0x83, 0x00);
        tp28xx_byte_write(chip, 0x84, 0x00);
        tp28xx_byte_write(chip, 0x85, 0x60);
        tp28xx_byte_write(chip, 0x86, 0x10);
        tp28xx_byte_write(chip, 0x87, 0x06);
        tp28xx_byte_write(chip, 0x88, 0xBE);
        tp28xx_byte_write(chip, 0x89, 0x39);
        tp28xx_byte_write(chip, 0x8A, 0x27);   //
        tp28xx_byte_write(chip, 0xB9, 0x0F);   //RX channel enable

        tp2802_set_video_mode(chip, mode, CH_ALL, STD_TVI);

        //PLL reset //only need for TP2802C/TP2802D
        val = tp28xx_byte_read(chip, 0x44);
        tp28xx_byte_write(chip, 0x44, val|0x40);
        msleep(10);
        tp28xx_byte_write(chip, 0x44, val);

        //ADC reset
        tp28xx_byte_write(chip, 0x40, CH_ALL); //write 4 channels
        tp28xx_byte_write(chip, 0x3B, 0x33);
        tp28xx_byte_write(chip, 0x3B, 0x03);

        //soft reset
        tp28xx_byte_write(chip, 0x40, CH_ALL); //write 4 channels
        val = tp28xx_byte_read(chip, 0x06);
        tp28xx_byte_write(chip, 0x06, 0x80|val);
    }
}

void TP2816_PLL_Reset( int chip)
{
    int i,val;

    if(DDR_2CH == output[chip] || DDR_4CH == output[chip] || DDR_1CH == output[chip])
        tp28xx_byte_write(chip, 0x45, 0xd4);
    else
        tp28xx_byte_write(chip, 0x45, 0xc9);

    for(i = 0; i < 6; i++ )
    {
        tp28xx_byte_write(chip, 0x44, 0x47);
        tp28xx_byte_write(chip, 0x42, 0x0C);
        tp28xx_byte_write(chip, 0x44, 0x07);
        tp28xx_byte_write(chip, 0x42, 0x00);
         msleep(1);
        val = tp28xx_byte_read(chip, 0x01);
        if(0x08 != val) break;
    }
}

int tp2830_dump_reg()
{
    char u8Value;
    int i = 0;
    printf("\n\n");
    printf("start dump 2830 reg !!!!!!!!!!!\n");
    for(i = 0; i < 256; i++)
    {
        u8Value = tp28xx_byte_read(0, i);
        printf("reg%d value: 0x%x  \n", i, u8Value);
        if((1+i)/16 > 0)
            printf("\n");
    }
    printf("end dump 2830 reg !!!!!!!!!!!\n");
    return 0;
}

////////////////////////////////////////////////////////////
int tp2830_module_init(int res)
{
    int ret = 0, i = 0, val = 0;
    int scsysid = 0;
    signed int s32TP28XX_Num = 0;

    printf("Compiled %s %s\n", __DATE__, __TIME__);
    printf("Please enter the number of TP2830 used(range1-2):\n");
    scanf("%d", &s32TP28XX_Num);

    if((s32TP28XX_Num < 1)||(s32TP28XX_Num > 2))
    {
        printf("Entered number invalid!!!\n");
        return FAILURE;
    }
    chips = s32TP28XX_Num;
    printf("chips %d\n", chips);

    if (chips <= 0 || chips > MAX_CHIPS)
    {
        printf("TP2802 module param 'chips' invalid value:%d\n", chips);
        return FAILURE;
    }

    pthread_mutex_init(&lock, NULL);

    i2cFd = open(FILE_NAME, O_RDWR);
    if (!i2cFd)
    {
        printf("can not open file %s\n", FILE_NAME);
        return 0;
    }
    for (i = 0; i < chips; i ++)
    {
        if(0 == res)
            output[i] = SDR_1CH;
        else if(1 == res)
            output[i] = SDR_2CH;
        else if(2 == res)
            output[i] = DDR_2CH;
        else if(3 == res)
            output[i] = DDR_4CH;
    }

    /* initize each tp2802*/
    for (i = 0; i < chips; i ++)
    {
        //page reset
        tp28xx_byte_write(i, 0x40, 0x00);
        //output disable
        tp28xx_byte_write(i, 0x4d, 0x00);
        tp28xx_byte_write(i, 0x4e, 0x00);
        //PLL reset
        val = tp28xx_byte_read(i, 0x44);
        tp28xx_byte_write(i, 0x44, val|0x40);
        msleep(10);
        tp28xx_byte_write(i, 0x44, val);

        val = tp28xx_byte_read(i, 0xfe);
        if(0x28 == val)
            printf("Detected TP28xx \n");
        else
            printf("Invalid chip %2x\n", val);

        id[i] = tp28xx_byte_read(i, 0xff);
        id[i] <<=8;
        id[i] +=tp28xx_byte_read(i, 0xfd);

        if(TP2826 == id[i] || TP2816 == id[i] || TP2826C == id[i] || TP2827C == id[i])
        {
            TP2816_PLL_Reset(i);
        }
        if(TP2828 == id[i] || TP2829 == id[i])
        {
            TP2829_PLL_Reset(i);
        }
        if(TP2830 == id[i] || TP2831 == id[i]) // Added TP2830/31 ID - 10/15/2018
        {
            TP2831_PLL_Reset(i);
        }

        printf("Detected ID&revision %04x\n", id[i]);

        tp2802_comm_init(i);

    }
    /* audio cascade for each tp2802*/
/*
    for (i = 0; i < chips; i ++)
    {
        if(id[i] > TP2822)  TP28xx_audio_cascade(i);
    }
*/

    if(id[0] > TP2822 && id[0] < TP2827C )
    {
        if(id[0] != TP2826C) TP28xx_audio_cascade();
    }



#if (WDT)
    ret = TP2802_watchdog_init();
    if (ret)
    {
        printf("ERROR: could not create watchdog\n");
        return ret;
    }
#endif

    printf("TP2802 Driver Init Successful!\n");
    //msleep(3000);
    //tp2830_dump_reg();

    return SUCCESS;
}

void tp2830_module_exit(void)
{
#if (WDT)
    TP2802_watchdog_exit();
#endif

    close(i2cFd);

    pthread_mutex_destroy(&lock);
}

void tp283x_egain(unsigned int chip, unsigned int CGAIN_STD)
{
    unsigned int tmp, cgain;
    unsigned int retry = 30;

    tp28xx_byte_write(chip, 0x2f, 0x06);
    cgain = tp28xx_byte_read(chip, 0x04);
#if (DEBUG)
    printf("Cgain=0x%02x \r\n", cgain );
#endif

    if(cgain < CGAIN_STD )
    {
        while(retry)
        {
            retry--;

            tmp = tp28xx_byte_read(chip, 0x07);
            tmp &=0x3f;
            while(abs(CGAIN_STD-cgain))
            {
                if(tmp) tmp--;
                else break;
                cgain++;
            }

            tp28xx_byte_write(chip, 0x07, 0x80|tmp);
            if(0 == tmp) break;
            msleep(40);
            tp28xx_byte_write(chip, 0x2f, 0x06);
            cgain = tp28xx_byte_read(chip, 0x04);
#if (DEBUG)
            printf("Cgain=0x%02x \r\n", cgain );
#endif
            if(cgain > (CGAIN_STD+1))
            {
                tmp = tp28xx_byte_read(chip, 0x07);
                tmp &=0x3f;
                tmp +=0x02;
                if(tmp > 0x3f) tmp = 0x3f;
                tp28xx_byte_write(chip, 0x07, 0x80|tmp);
                if(0x3f == tmp) break;
                msleep(40);
                cgain = tp28xx_byte_read(chip, 0x04);
#if (DEBUG)
                printf("Cgain=0x%02x \r\n", cgain);
#endif
            }
            if(abs(cgain-CGAIN_STD) < 2)  break;
        }

    }
}
/////////////////////////////////////////////////////////////////
unsigned char tp28xx_read_egain(unsigned char chip)
{
    unsigned char gain;

        if(id[chip] > TP2823)
        {
            tp28xx_byte_write(chip, 0x2f, 0x00);
            gain = tp28xx_byte_read(chip, 0x04);
        }
        else
        {
            gain = tp28xx_byte_read(chip, 0x03);
            gain >>= 4;
        }
        return gain;
}

//////////////////////////////////////////////////////////////////
/******************************************************************************
 *
 * TP2802_watchdog_deamon()

 *
 ******************************************************************************/
void *TP2802_watchdog_deamon(void *data)
{
    //unsigned long flags;
    int iChip, i = 0;

    tp2802wd_info* wdi;

    //struct timeval start, end;
    //int interval;
    unsigned char status, cvstd, gain, agc, tmp,flag_locked;
    unsigned char rx1,rx2;

    const unsigned char TP2802D_CGAIN[16]   = {0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x60,0x50,0x40,0x38,0x30};
    const unsigned char TP2802D_CGAINMAX[16]= {0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x44,0x44,0x44,0x44,0x44};
    const unsigned char TP2823_CGAINMAX[16] = {0x4a,0x47,0x46,0x45,0x44,0x44,0x43,0x43,0x42,0x42,0x42,0x42,0x40,0x40,0x40,0x40};

    printf("TP2802_watchdog_deamon: start!\n");

    while (watchdog_state != WATCHDOG_EXIT)
    {
        pthread_mutex_lock(&lock);

        //do_gettimeofday(&start);

        for(iChip = 0; iChip < chips; iChip++)
        {

            wdi = &watchdog_info[iChip];

            for(i=0; i<CHANNELS_PER_CHIP; i++)//scan four inputs:
            {
                if(SCAN_DISABLE == wdi->scan[i]) continue;

                tp2802_set_reg_page(iChip,i);

                status = tp28xx_byte_read(iChip, 0x01);

                //state machine for video checking
                if(status & FLAG_LOSS) //no video
                {

                    if(VIDEO_UNPLUG != wdi->state[i]) //switch to no video
                    {
                        wdi->state[i] = VIDEO_UNPLUG;
                        wdi->count[i] = 0;
                        if(SCAN_MANUAL != wdi->scan[i]) wdi->mode[i] = INVALID_FORMAT;
#if (DEBUG)
                        printf("video loss ch%02x chip%2x\r\n", i, iChip );
#endif
                    }

                    if( 0 == wdi->count[i]) //first time into no video
                    {
                        //if(SCAN_MANUAL != wdi->scan[i]) TP28xx_reset_default(iChip, i);
                        //tp2802_set_video_mode(iChip, DEFAULT_FORMAT, i, STD_TVI);
                        TP28xx_reset_default(iChip, i);
                        wdi->count[i]++;
                    }
                    else
                    {
                        if(wdi->count[i] < MAX_COUNT) wdi->count[i]++;
                        continue;
                    }

                }
                else //there is video
                {
                    if( TP2802_PAL == wdi->mode[i] || TP2802_NTSC == wdi->mode[i] )
                        flag_locked = FLAG_HV_LOCKED;
                    else
                        flag_locked = FLAG_HV_LOCKED;

                    if( flag_locked == (status & flag_locked) ) //video locked
                    {
                        if(VIDEO_LOCKED == wdi->state[i])
                        {
                            if(wdi->count[i] < MAX_COUNT) wdi->count[i]++;
                        }
                        else if(VIDEO_UNPLUG == wdi->state[i])
                        {
                            wdi->state[i] = VIDEO_IN;
                            wdi->count[i] = 0;
#if (DEBUG)
                            printf("1video in ch%02x chip%2x\r\n", i, iChip);
#endif
                        }
                        else if(wdi->mode[i] != INVALID_FORMAT)
                        {
                            //if( FLAG_HV_LOCKED == (FLAG_HV_LOCKED & status) )//H&V locked
                            {
                                wdi->state[i] = VIDEO_LOCKED;
                                wdi->count[i] = 0;
#if (DEBUG)
                                printf("video locked %02x ch%02x chip%2x\r\n", status, i, iChip);
#endif
                            }

                        }
                    }
                    else  //video in but unlocked
                    {
                        if(VIDEO_UNPLUG == wdi->state[i])
                        {
                            wdi->state[i] = VIDEO_IN;
                            wdi->count[i] = 0;
#if (DEBUG)
                            printf("2video in ch%02x chip%2x\r\n", i, iChip);
#endif
                        }
                        else if(VIDEO_LOCKED == wdi->state[i])
                        {
                            wdi->state[i] = VIDEO_UNLOCK;
                            wdi->count[i] = 0;
#if (DEBUG)
                            printf("video unstable ch%02x chip%2x\r\n", i, iChip);
#endif
                        }
                        else
                        {
                            if(wdi->count[i] < MAX_COUNT) wdi->count[i]++;
                            if(VIDEO_UNLOCK == wdi->state[i] && wdi->count[i] > 2)
                            {
                                wdi->state[i] = VIDEO_IN;
                                wdi->count[i] = 0;
                                if(SCAN_MANUAL != wdi->scan[i]) TP28xx_reset_default(iChip, i);
#if (DEBUG)
                                printf("video unlocked ch%02x chip%2x\r\n",i, iChip);
#endif
                            }
                        }
                    }

                    if( wdi->force[i] ) //manual reset for V1/2 switching
                    {

                        wdi->state[i] = VIDEO_UNPLUG;
                        wdi->count[i] = 0;
                        wdi->mode[i] = INVALID_FORMAT;
                        wdi->force[i] = 0;
                        TP28xx_reset_default(iChip, i);
                        //tp2802_set_video_mode(iChip, DEFAULT_FORMAT, i);
                    }

                }

                //printf("video state %2x detected ch%02x count %4x\r\n", wdi->state[i], i, wdi->count[i] );
                if( VIDEO_IN == wdi->state[i] )
                {
                    if(SCAN_MANUAL != wdi->scan[i])
                    {

                        cvstd = tp28xx_byte_read(iChip, 0x03);
#if (DEBUG)
                        printf("video format %2x detected ch%02x chip%2x count%2x\r\n", cvstd, i, iChip, wdi->count[i]);
#endif
                        cvstd &= 0x0f;

                        {

                            wdi-> std[i] = STD_TVI;
/*
                            if( TP2802_SD == (cvstd&0x07) )
                            {
                                if( id[iChip] > TP2822 )
                                {
                                    if(wdi->count[i] & 0x01)
                                    {
                                        wdi-> mode[i] = TP2802_PAL;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, STD_TVI);

                                        if(TP2823 == id[iChip])
                                        {
                                            if(0x02==i) tp28xx_byte_write(iChip, 0x0d, 0x1);
                                        }

                                    }
                                    else
                                    {
                                        wdi-> mode[i] = TP2802_NTSC;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, STD_TVI);

                                        if(TP2823 == id[iChip])
                                        {
                                            if(0x02==i) tp28xx_byte_write(iChip, 0x0d, 0x0);
                                        }

                                    }

                                }

                            }

                            else
*/
                            if((cvstd&0x07) < 4 )
                            {

                                if(SCAN_HDA == wdi->scan[i] || SCAN_HDC == wdi->scan[i] )
                                {
                                    if(SCAN_HDA == wdi->scan[i])          wdi-> std[i] = STD_HDA;
                                    else if(SCAN_HDC == wdi->scan[i])     wdi-> std[i] = STD_HDC;

                                    if( TP2802_720P25 == (cvstd&0x07) )
                                    {
                                        wdi-> mode[i] = TP2802_720P25V2;
                                    }
                                    else if( TP2802_720P30 == (cvstd&0x07) )
                                    {
                                        wdi-> mode[i] = TP2802_720P30V2;
                                    }
                                    else
                                    {
                                        wdi-> mode[i] = cvstd&0x07;
                                    }
                                    tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);

                                }
                                else //TVI
                                {
                                    //if( TP2826 == id[iChip] || TP2823C == id[iChip] || TP2853C == id[iChip]||TP2833C == id[iChip]||TP2833 == id[iChip] || TP2834 == id[iChip])
                                    if( id[iChip] > TP2823 )
                                    {
                                        if(TP2802_720P25V2 == (cvstd&0x0f) || TP2802_720P30V2 == (cvstd&0x0f) )
                                        {
                                            wdi-> mode[i] = cvstd&0x0f;
                                        }
                                        else
                                        {
                                            wdi-> mode[i] = cvstd&0x07;
                                        }
                                    }
                                    else
                                    {
                                        wdi-> mode[i] = cvstd&0x07;

                                    }
                                    tp2802_set_video_mode(iChip, wdi-> mode[i], i, STD_TVI);

                                }

                            }
                            else //format is 7
                            {
                                if(SCAN_HDA == wdi->scan[i] ) wdi-> std[i] = STD_HDA;
                                if(SCAN_HDC == wdi->scan[i] ) wdi-> std[i] = STD_HDC;

                                if( TP2853C == id[iChip] || TP2826 == id[iChip] || TP2827 == id[iChip] || TP2826C == id[iChip] || TP2827C == id[iChip] || TP2828 == id[iChip] || TP2829 == id[iChip] || TP2830 == id[iChip] || TP2831 == id[iChip])//3M auto detect test,  Added TP2830/31 ID -10/15/2018
                                {
                                    tp28xx_byte_write(iChip, 0x2f, 0x09);
                                    tmp = tp28xx_byte_read(iChip, 0x04);
#if (DEBUG)
                                    printf("detection %02x  ch%02x chip%2x\r\n", tmp, i,iChip);
#endif
                                    //if((tmp > 0x48) && (tmp < 0x55))
                                    if(0x4e == tmp)
                                    {
                                        //wdi-> mode[i] = TP2802_3M;
                                        if(SCAN_HDA == wdi->scan[i] || SCAN_AUTO == wdi->scan[i])
                                            wdi-> mode[i] = TP2802_QXGA18;
                                        else
                                            wdi-> mode[i] = TP2802_3M18;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                    //else if((tmp > 0x55) && (tmp < 0x62))
                                    else if(0x5d == tmp)
                                    {
                                        if((wdi->count[i] % 3) ==0)
                                        {
                                            wdi-> mode[i] = TP2802_5M12;
                                            wdi-> std[i] = STD_HDA;
                                            tp2802_set_video_mode(iChip, wdi-> mode[i], i,  wdi-> std[i]);
                                        }
                                        else if((wdi->count[i] % 3) ==1)
                                        {
                                            wdi-> mode[i] = TP2802_4M15;
                                            tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                        }
                                        else
                                        {
                                            wdi-> mode[i] = TP2802_720P30HDR;
                                            tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                        }
                                    }
                                    else if(0x5c == tmp)
                                    {
                                        wdi-> mode[i] = TP2802_5M12;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                    //else if((tmp > 0x70) && (tmp < 0x80))
                                    else if(0x75 == tmp)
                                    {
                                        wdi-> mode[i] = TP2802_6M10;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                    //else if((tmp > 0x34) && (tmp < 0x40))
                                    else if(0x38 == tmp)
                                    {
                                        wdi-> mode[i] = TP2802_QXGA25; //current only HDA
                                        wdi-> std[i] = STD_HDA;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                    //else if((tmp > 0x28) && (tmp < 0x34))
                                    else if(0x2e == tmp)
                                    {
                                        wdi-> mode[i] = TP2802_QXGA30; //current only HDA
                                        wdi-> std[i] = STD_HDA;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                    else if(0x3a == tmp)  // invalid for TP2853C
                                    {
                                        if(TP2802_5M20 != wdi-> mode[i])
                                        {
                                            wdi-> mode[i] = TP2802_5M20;
                                            tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                            //soft reset
                                            agc = tp28xx_byte_read(iChip, 0x06);
                                            agc |=0x80;
                                            tp28xx_byte_write(iChip, 0x06, agc);

                                        }
                                    }
                                    else if(0x39 == tmp)  // invalid for TP2853C
                                    {
                                        if(TP2802_6M20 != wdi-> mode[i])
                                        {
                                            wdi-> mode[i] = TP2802_6M20;
                                            wdi-> std[i] = STD_HDC;
                                            tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                        }
                                    }
                                    else if(0x89 == tmp)
                                    {
                                        wdi-> mode[i] = TP2802_1080P15;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                    else if(0x22 == tmp)
                                    {
                                        wdi-> mode[i] = TP2802_1080P60;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                    else if(0x29 == tmp)
                                    {
                                        wdi-> mode[i] = TP2802_1080P50;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                    else if(0xce == tmp)
                                    {
                                        wdi-> mode[i] = TP2802_720P14;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                    else if(0x42 == tmp)
                                    {
                                        wdi-> mode[i] = TP2802_5M20V2;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                    else if(0x93 == tmp)
                                    {
                                        wdi-> mode[i] = TP2802_NTSC;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                    else if(0x94 == tmp)
                                    {
                                        wdi-> mode[i] = TP2802_PAL;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }

                                    else if(0x67 == tmp)
                                    {
                                        wdi-> mode[i] = TP2802_720P30V2;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                    else if(0x7b == tmp)
                                    {
                                        wdi-> mode[i] = TP2802_720P25V2;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                    else if(0x7d == tmp)
                                    {
                                        wdi-> mode[i] = TP2802_8M7;
                                        wdi-> std[i] = STD_HDA;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }

                                }
                            }

                            if( TP2826 == id[iChip] || TP2827 == id[iChip] || TP2826C == id[iChip] || TP2827C == id[iChip] || TP2828 == id[iChip] || TP2829 == id[iChip] || TP2830 == id[iChip] || TP2831 == id[iChip])// new 3M20/4M12 auto detect test, Added TP2830/31 ID - 10/15/2018
                            {
                                    if( (wdi->count[i] & 1) && (wdi-> mode[i] == TP2802_1080P25))
                                    {
                                        wdi-> mode[i] = TP2802_8M12;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                    if( (wdi->count[i] & 1) && (wdi-> mode[i] == TP2802_1080P30))
                                    {
                                        wdi-> mode[i] = TP2802_8M15;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                    if( (wdi->count[i] & 1) && (wdi-> mode[i] == TP2802_720P30))
                                    {
                                        wdi-> mode[i] = TP2802_4M12;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                    if( (wdi->count[i] & 1) && (wdi-> mode[i] == TP2802_720P60))
                                    {
                                        wdi-> mode[i] = TP2802_QHD30;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                    if( wdi-> mode[i] == TP2802_720P50)
                                    {

                                        if( (wdi->count[i] %3) ==1 )
                                        {
                                            wdi-> mode[i] = TP2802_QHD25;
                                            tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                        }
                                        else if( (wdi->count[i] %3) ==2 )
                                        {
                                            wdi-> mode[i] = TP2802_8M15V2;
                                            tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                        }

                                    }

                                    if( wdi-> mode[i] == TP2802_720P30V2 )
                                    {
                                        if( (wdi->count[i] &1) ==1 )
                                        {
                                            wdi-> mode[i] = TP2802_QHD15;
                                            tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                        }

                                    }

                                    if( (wdi->count[i] > 2) && (wdi-> mode[i] == TP2802_QXGA18) )
                                    {

                                        if(SCAN_AUTO == wdi->scan[i])
                                        {
                                            wdi-> mode[i] = TP2802_3M18;
                                            tp2802_set_video_mode(iChip, wdi-> mode[i], i,  wdi-> std[i]);
                                        }
                                    }

                            }
                            else if( TP2853C == id[iChip])
                            {
                                    if( (wdi->count[i] & 1) && (wdi-> mode[i] == TP2802_720P30))
                                    {
                                        wdi-> mode[i] = TP2802_4M12;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                    if( (wdi->count[i] & 1) && (wdi-> mode[i] == TP2802_720P60))
                                    {
                                        wdi-> mode[i] = TP2802_QHD30;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                    if( (wdi->count[i] & 1) && (wdi-> mode[i] == TP2802_720P50))
                                    {
                                        wdi-> mode[i] = TP2802_QHD25;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                    if( (wdi->count[i] & 1) && (wdi-> mode[i] == TP2802_720P30V2))
                                    {
                                        wdi-> mode[i] = TP2802_QHD15;
                                        //wdi-> std[i] = STD_HDA;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i,  wdi-> std[i]);
                                    }
                                    if( (wdi->count[i] > 2) && (wdi-> mode[i] == TP2802_QXGA18) )
                                    {

                                        if(SCAN_AUTO == wdi->scan[i])
                                        {
                                            wdi-> mode[i] = TP2802_3M18;
                                            tp2802_set_video_mode(iChip, wdi-> mode[i], i,  wdi-> std[i]);
                                        }
                                    }
                                    if( (wdi->count[i] %5) ==1 && (wdi-> mode[i] == TP2802_1080P30))
                                    {
                                        wdi-> mode[i] = TP2802_QXGA30;
                                        //wdi-> std[i] = STD_HDA;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                    if( (wdi->count[i] %5) ==2 && (wdi-> mode[i] == TP2802_1080P30))
                                    {
                                        wdi-> mode[i] = TP2802_QXGA25;
                                        //wdi-> std[i] = STD_HDA;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                    if( (wdi->count[i] %5) ==3 && (wdi-> mode[i] == TP2802_1080P30))
                                    {
                                        wdi-> mode[i] = TP2802_5M20;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                    if( (wdi->count[i] %5) ==4 && (wdi-> mode[i] == TP2802_1080P30))
                                    {
                                        wdi-> mode[i] = TP2802_8M15;
                                        //wdi-> std[i] = STD_HDA;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                    if( (wdi->count[i] & 1) && (wdi-> mode[i] == TP2802_1080P25))
                                    {
                                        wdi-> mode[i] = TP2802_8M12;
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                            }

                        }

                    }

                }

#define EQ_COUNT 10
                if( VIDEO_LOCKED == wdi->state[i]) //check signal lock
                {
                    if(0 == wdi->count[i] )
                    {
                        //if(SCAN_MANUAL == wdi->scan[i])
                        {
                            if(TP2823 == id[iChip] || TP2834 == id[iChip] )
                            {
                                tmp = tp28xx_byte_read(iChip, 0x26);
                                tmp &= 0xfc;
                                tmp |= 0x02;
                                tp28xx_byte_write(iChip, 0x26, tmp);
                            }
                            //else if(TP2833 == id[iChip] || TP2833C == id[iChip] || TP2853C == id[iChip] || TP2823C == id[iChip] || TP2826 == id[iChip] || TP2816 == id[iChip] || TP2827 == id[iChip] || TP2826C == id[iChip] || TP2827C == id[iChip])
                            else
                            {
                                tmp = tp28xx_byte_read(iChip, 0x26);
                                tmp |= 0x01;
                                tp28xx_byte_write(iChip, 0x26, tmp);
                            }
                        }

                        if( (SCAN_AUTO == wdi->scan[i] || SCAN_TVI == wdi->scan[i] ))
                        {

                            //wdi-> std[i] = STD_TVI;

                            if( (TP2802_720P30V2==wdi-> mode[i]) || (TP2802_720P25V2==wdi-> mode[i]) )
                            {
                                tmp = tp28xx_byte_read(iChip, 0x03);
#if (DEBUG)
                                printf("CVSTD%02x  ch%02x chip%2x\r\n", tmp, i,iChip);
#endif
                                if( ! (0x08 & tmp) )
                                {
#if (DEBUG)
                                    printf("720P V1 Detected ch%02x chip%2x\r\n",i,iChip);
#endif
                                    wdi-> mode[i] &= 0xf7;
                                    tp2802_set_video_mode(iChip, wdi-> mode[i], i, STD_TVI); //to speed the switching

                                }
                            }
                            else if( (TP2802_720P30==wdi-> mode[i]) || (TP2802_720P25==wdi-> mode[i]) )
                            {
                                tmp = tp28xx_byte_read(iChip, 0x03);
#if (DEBUG)
                                printf("CVSTD%02x  ch%02x chip%2x\r\n", tmp, i,iChip);
#endif
                                if( 0x08 & tmp)
                                {
#if (DEBUG)
                                    printf("720P V2 Detected ch%02x chip%2x\r\n",i,iChip);
#endif
                                    wdi-> mode[i] |= 0x08;
                                    tp2802_set_video_mode(iChip, wdi-> mode[i], i, STD_TVI); //to speed the switching

                                }

                            }

                            //these code need to keep bottom
                            if( id[iChip] >= TP2822 )
                            {

                                if( TP2826 == id[iChip] || TP2816 == id[iChip] || TP2827 == id[iChip] || TP2826C == id[iChip] || TP2827C == id[iChip] || TP2828 == id[iChip] || TP2829 == id[iChip] || TP2830 == id[iChip] || TP2831 == id[iChip]) // Added TP2830/31 ID - 10/15/2018
                                {
                                    tmp = tp28xx_byte_read(iChip, 0xa7);
                                    tmp &= 0xfe;
                                    tp28xx_byte_write(iChip, 0xa7, tmp);
                                    //tp28xx_byte_write(iChip, 0x2f, 0x0f);
                                    tp28xx_byte_write(iChip, 0x1f, 0x06);
                                    tp28xx_byte_write(iChip, 0x1e, 0x60);
                                }
                                else
                                {
                                    tmp = tp28xx_byte_read(iChip, 0xb9);
                                    tp28xx_byte_write(iChip, 0xb9, tmp&SYS_AND[i]);
                                    tp28xx_byte_write(iChip, 0xb9, tmp|SYS_MODE[i]);
                                }

                            }

                        }
#if (HALF_FLAG_ENABLE)
                            if( id[iChip] >= TP2822 )
                            {

                                tmp = tp28xx_byte_read(iChip, 0x35); //half flag
                                if(tmp & 0x40) wdi-> mode[i] |= FLAG_HALF_MODE; //please remark it if want same video mode at half mode
                            }
#endif

                    }
                    else if(1 == wdi->count[i])
                    {
                                if( TP2826 == id[iChip] || TP2816 == id[iChip] || TP2827 == id[iChip] || TP2826C == id[iChip] || TP2827C == id[iChip] || TP2828 == id[iChip] || TP2829 == id[iChip] || TP2830 == id[iChip] || TP2831 == id[iChip]) // Added TP2830/31 ID - 10/15/2018
                                {
                                    tmp = tp28xx_byte_read(iChip, 0xa7);
                                    tmp |= 0x01;
                                    tp28xx_byte_write(iChip, 0xa7, tmp);
                                }

#if (DEBUG)
                        tmp = tp28xx_byte_read(iChip, 0x01);
                        printf("status%02x  ch%02x\r\n", tmp, i);
                        tmp = tp28xx_byte_read(iChip, 0x03);
                        printf("CVSTD%02x  ch%02x\r\n", tmp, i);
#endif
                    }
                    else if( wdi->count[i] < EQ_COUNT-3)
                    {

                        if( id[iChip] > TP2823  && SCAN_AUTO == wdi->scan[i] )
                        {

                            if( STD_TVI == wdi-> std[i])
                            {
                                tmp = tp28xx_byte_read(iChip, 0x01);

                                if((TP2802_PAL == wdi-> mode[i]) || (TP2802_NTSC == wdi-> mode[i]))
                                {
                                    //nothing to do
                                }
                                else if(TP2802_QXGA18 == wdi-> mode[i])
                                {
                                    if(0x60 == (tmp&0x64) )
                                    {
                                        wdi-> std[i] = STD_HDA; //no CVI QXGA18
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                }
                                else if(TP2802_QHD15 == wdi-> mode[i] || TP2802_5M12 == wdi-> mode[i])
                                {
                                    if(0x60 == (tmp&0x64) )
                                    {
                                        wdi-> std[i] = STD_HDA; //no CVI QHD15/5M20/5M12.5
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    }
                                }
                                else if( TP2802_QHD25 == wdi-> mode[i] || TP2802_QHD30 == wdi-> mode[i]
                                        ||TP2802_8M12 == wdi-> mode[i] || TP2802_8M15 == wdi-> mode[i]
                                        ||TP2802_5M20 == wdi-> mode[i]
                                        )
                                {
                                    agc = tp28xx_byte_read(iChip, 0x10);
                                    tp28xx_byte_write(iChip, 0x10, 0x00);

                                    tp28xx_byte_write(iChip, 0x2f, 0x0f);
                                    rx1 = tp28xx_byte_read(iChip, 0x04);
    #if (DEBUG)
                                        printf("RX1=%02x ch%02x\r\n", rx1, i);
    #endif
                                    tp28xx_byte_write(iChip, 0x10, agc);

                                    if(rx1 > 0x30)               wdi-> std[i] = STD_HDA;
                                    else if(0x60 == (tmp&0x64) ) wdi-> std[i] = STD_HDC;
                                    if(STD_TVI != wdi->std[i]) tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                    else if(TP2802_8M12 == wdi-> mode[i] || TP2802_8M15 == wdi-> mode[i]) tp28xx_byte_write(iChip, 0x20, 0x50); //restore TVI clamping
                                }
                                else if(0x60 == (tmp&0x64) )
                                {
                                    if( TP2826 == id[iChip] || TP2816 == id[iChip] || TP2827 == id[iChip] || TP2827C == id[iChip] || TP2828 == id[iChip] || TP2829 == id[iChip] || TP2830 == id[iChip] || TP2831 == id[iChip]) // Added TP2830/31 ID - 10/15/2018
                                    {
                                        rx2 = tp28xx_byte_read(iChip, 0x94); //capture line7 to match 3M/4M RT
                                    }
                                    else
                                    {
                                        rx2 = tp28xx_byte_read(iChip, 0x94+10*i);
                                    }

                                    if(HDC_enable)
                                    {
                                        if     (0xff == rx2)                wdi-> std[i] = STD_HDC;
                                        else if(0x00 == rx2)                wdi-> std[i] = STD_HDC_DEFAULT;
                                        else                                wdi-> std[i] = STD_HDA;
                                    }
                                    else
                                    {
                                        wdi-> std[i] = STD_HDA;
                                    }

                                    if(STD_TVI != wdi->std[i])
                                    {
                                        tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
    #if (DEBUG)
                                        printf("RX=%02x standard to %02x  ch%02x\r\n", rx2, wdi-> std[i], i);
    #endif
                                    }

                                }
                            }

                        }

                    }
                    else if( wdi->count[i] < EQ_COUNT) //skip
                    {

                        //wdi->gain[i][EQ_COUNT-wdi->count[i]] = tp28xx_read_egain(iChip);
                        wdi->gain[i][3] = wdi->gain[i][2];
                        wdi->gain[i][2] = wdi->gain[i][1];
                        wdi->gain[i][1] = wdi->gain[i][0];

                        wdi->gain[i][0] = tp28xx_read_egain(iChip);

                    }
                    else if( wdi->count[i] < EQ_COUNT+EQ_COUNT ) //add timeout handle
                    {
                        wdi->gain[i][3] = wdi->gain[i][2];
                        wdi->gain[i][2] = wdi->gain[i][1];
                        wdi->gain[i][1] = wdi->gain[i][0];

                        wdi->gain[i][0] = tp28xx_read_egain(iChip);

                        //if(abs(wdi->gain[i][3] - wdi->gain[i][0])< 0x20 && abs(wdi->gain[i][2] - wdi->gain[i][0])< 0x20 && abs(wdi->gain[i][1] - wdi->gain[i][0])< 0x20 )
                        if(abs(wdi->gain[i][3] - wdi->gain[i][0])< 0x02 && abs(wdi->gain[i][2] - wdi->gain[i][0])< 0x02 && abs(wdi->gain[i][1] - wdi->gain[i][0])< 0x02 )
                        {
                            wdi->count[i] = EQ_COUNT+EQ_COUNT-1; // exit when EQ stable
                        }
                    }
                    else if( wdi->count[i] == EQ_COUNT+EQ_COUNT )
                    {
                        gain = tp28xx_read_egain(iChip);

                        if( STD_TVI != wdi-> std[i] )
                        {
                                if( TP2830 == id[iChip] || TP2831 == id[iChip]) // Added TP2830/31 ID - 10/15/2018
                                {
                                    gain >>=2;
                                    if(gain > 0x33) gain = 0x3f;
                                    else if(gain > 0x0f) gain += 0x0c;
                                    tp28xx_byte_write(iChip, 0x07, 0x80|(gain));  // manual mode
                                }
                                else if( TP2828 == id[iChip] || TP2829 == id[iChip])
                                {
                                    gain >>=2;
                                    if(gain > 0x33) gain = 0x3f;
                                    else if(gain > 0x0f) gain += 0x0c;
                                    tp28xx_byte_write(iChip, 0x07, 0x80|(gain));  // manual mode
                                }
                                else
                                {
                                    tp28xx_byte_write(iChip, 0x07, 0x80|(gain>>2));  // manual mode
                                }

                        }
                        else //TVI
                        {
                            if( id[iChip] == TP2833 || id[iChip] == TP2834)
                            {
                                if( wdi-> mode[i] & FLAG_MEGA_MODE )
                                {
                                    tp28xx_byte_write(iChip, 0x07, 0x80|(gain/3));  // manual mode
                                }

                            }
                            else if(TP2822 == id[iChip])
                            {
                                tp28xx_byte_write(iChip, 0x07, gain<<2);  // manual mode
                                tp28xx_byte_write(iChip, 0x2b, TP2823_CGAINMAX[gain]);
                            }
                            else if(TP2802D == id[iChip] )
                            {
                                tp28xx_byte_write(iChip, 0x2e, TP2802D_CGAIN[gain]);
                                tp28xx_byte_write(iChip, 0x2b, TP2802D_CGAINMAX[gain]);

                                if(gain < 0x02)
                                {
                                    tp28xx_byte_write(iChip, 0x3A, TP2802D_EQ_SHORT);  // for short cable
                                    tp28xx_byte_write(iChip, 0x2e, TP2802D_CGAIN_SHORT);
                                }
                                if(gain > 0x03 && ((TP2802_720P30V2==wdi-> mode[i]) || (TP2802_720P25V2==wdi-> mode[i])) )
                                {
                                    tp28xx_byte_write(iChip, 0x3A, 0x09);  // long cable
                                    tp28xx_byte_write(iChip, 0x07, 0x40);  // for long cable
                                    tp28xx_byte_write(iChip, 0x09, 0x20);  // for long cable
                                    tmp = tp28xx_byte_read(iChip, 0x06);
                                    tmp |=0x80;
                                    tp28xx_byte_write(iChip, 0x06,tmp);
                                }
                            }
                            else if(TP2823 == id[iChip])
                            {
                                if((TP2802_PAL == wdi-> mode[i]) || (TP2802_NTSC == wdi-> mode[i]))
                                {
                                    tp28xx_byte_write(iChip, 0x2b, 0x70);
                                }
                                else
                                {
                                    tp28xx_byte_write(iChip, 0x2b, TP2823_CGAINMAX[gain]);
                                }
                            }
                        }

                    }
                    else if(wdi->count[i] == EQ_COUNT+EQ_COUNT+1)
                    {

                        if(TP2822 == id[iChip] || TP2823 == id[iChip])
                        {
                            tp2802_manual_agc(iChip, i);

                        }
                        else if( TP2802D == id[iChip])
                        {
                            if( (TP2802_720P30V2!=wdi-> mode[i]) && (TP2802_720P25V2!=wdi-> mode[i]) )
                            {
                                tp2802_manual_agc(iChip, i);
                            }
                        }
                        else if( id[iChip] > TP2823 )
                        {
                            if(  SCAN_AUTO == wdi->scan[i])
                            {

                                if( HDC_enable )
                                {
                                        if(STD_HDC_DEFAULT == wdi->std[i] )
                                        {
                                            tp28xx_byte_write(iChip, 0x2f,0x0c);
                                            tmp = tp28xx_byte_read(iChip, 0x04);
                                            status = tp28xx_byte_read(iChip, 0x01);

                                            //if(0x10 == (0x11 & status) && (tmp < 0x18 || tmp > 0xf0))
                                            if(0x10 == (0x11 & status))
                                            //if((tmp < 0x18 || tmp > 0xf0))
                                            {
                                                wdi-> std[i] = STD_HDC;
                                            }
                                            else
                                            {
                                                wdi-> std[i] = STD_HDA;
                                            }
                                            tp2802_set_video_mode(iChip, wdi-> mode[i], i, wdi-> std[i]);
                                            #if (DEBUG)
                                            printf("reg01=%02x reg04@2f=0c %02x std%02x ch%02x\r\n", status, tmp, wdi-> std[i], i );
                                            #endif
                                        }
                                }
                            }

                            if( STD_TVI != wdi-> std[i] )
                            {
                                    if(TP2853C == id[iChip] || TP2833C == id[iChip] || TP2823C == id[iChip] || TP2833 == id[iChip] || TP2834 == id[iChip])
                                    {
                                        tp2802_manual_agc(iChip, i); //fix AGC when PTZ operation, it can be removed on TP2826/2827
                                    }
                                    tp283x_egain(iChip, 0x0c);

                                    if(STD_HDC == wdi-> std[i]) //fine tune HDC
                                    {
                                        if(TP2853C == id[iChip] || TP2833C == id[iChip] || TP2823C == id[iChip])
                                        {
                                            tmp = tp28xx_byte_read(iChip, 0x07);
                                            tmp &=0x3f;
                                            tmp +=0x08;
                                            if(tmp > 0x3f) tmp = 0x3f;
                                            tp28xx_byte_write(iChip, 0x07, 0x80|tmp);
                                        }

                                    }
                                    else // HDA
                                    {

                                    }
                            }
                            else //TVI
                            {
                                if( id[iChip] == TP2833 || id[iChip] == TP2834)
                                {
                                    if( wdi-> mode[i] & FLAG_MEGA_MODE )
                                    {
                                        tp283x_egain(iChip, 0x16);
                                    }
                                }

                            }

                        }


                    }
                    else if(wdi->count[i] == EQ_COUNT+EQ_COUNT+5)
                    {
                        if( TP2802D == id[iChip])
                        {
                            if( (TP2802_720P30V2==wdi-> mode[i]) || (TP2802_720P25V2==wdi-> mode[i]) )
                            {
                                tp2802_manual_agc(iChip, i);
                            }
                        }

                    }
                    else
                    {
                        if( SCAN_AUTO == wdi->scan[i])
                        {

                            if((wdi-> mode[i] & (~FLAG_HALF_MODE)) < TP2802_3M18)
                            {
                                            tmp = tp28xx_byte_read(iChip, 0x03); //
                                            tmp &= 0x07;
                                            if(tmp != (wdi-> mode[i]&0x07) && tmp < TP2802_SD)
                                            {
                                            #if (DEBUG)
                                            printf("correct %02x from %02x ch%02x\r\n", tmp, wdi-> mode[i], i );
                                            #endif

                                                wdi->force[i] = 1;
                                            }
                            }
                        }


                    }

                }
            }
        }

        //do_gettimeofday(&end);

        //interval = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);

        //printf("WDT elapsed time %d.%dms\n", interval/1000, interval%1000);
        pthread_mutex_unlock(&lock);

        /* sleep 0.5 seconds */
        //schedule_timeout_interruptible(msecs_to_jiffies(700)+1);
        msleep(500);
    }

    printf("TP2802_watchdog_deamon: exit!\n");

    return 0;

}


/******************************************************************************
 *
 * cx25930_watchdog_init()

 *
 ******************************************************************************/
int TP2802_watchdog_init(void)
{
    int i, j;
    watchdog_state = WATCHDOG_RUNNING;
    memset(&watchdog_info, 0, sizeof(watchdog_info));

    for(i=0; i<MAX_CHIPS; i++)
    {
        watchdog_info[i].addr = tp2802_i2c_addr[i];
        for(j=0; j<CHANNELS_PER_CHIP; j++)
        {
            watchdog_info[i].count[j] = 0;
            watchdog_info[i].force[j] = 0;
            //watchdog_info[i].loss[j] = 1;
            watchdog_info[i].mode[j] = INVALID_FORMAT;
            watchdog_info[i].scan[j] = SCAN_AUTO;
            watchdog_info[i].state[j] = VIDEO_UNPLUG;
            watchdog_info[i].std[j] = STD_TVI;
        }

    }
    pthread_create(&ptWatchdog, NULL, TP2802_watchdog_deamon, NULL);

    printf("TP2802_watchdog_init: done!\n");

    return 0;
}

/******************************************************************************
 *
 * cx25930_watchdog_exit()

 *
 ******************************************************************************/
void TP2802_watchdog_exit(void)
{

    watchdog_state = WATCHDOG_EXIT;

    pthread_join(ptWatchdog, NULL);
    ptWatchdog = NULL;

    printf("TP2802_watchdog_exit: done!\n");
}


