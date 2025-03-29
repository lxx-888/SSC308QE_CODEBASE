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

#include "st_dh9931.h"
#include "dh9931_cfg.h"
#include "dhc_dh9931_api.h"

#define MAX_I2C_NUM (8)

#ifdef __cplusplus
}
#endif

typedef struct Cus_I2cInfo_s
{
    int I2cFd;
}Cus_I2cInfo_t;

#define MAX_DH9931_NUM (2)
static ss_cus_sensor_t gstSensorHandle[MAX_DH9931_NUM];
static Cus_I2cInfo_t gstI2cInfo[MAX_I2C_NUM];

#define SENSOR_I2C_ADDR    0x60                   //I2C slave address

#define DH9931_CHECK_POINTER(pointer)                               \
    {                                                               \
        if (pointer == NULL)                                        \
        {                                                           \
            printf("[ERROR]%s NULL pointer!!!\n", __FUNCTION__);    \
            return -1;                                              \
        }                                                           \
    }

#define DH9931_CHECK_PARA_RANGE(para, min, max)                                             \
    {                                                                                       \
        if (para < min)                                                                     \
        {                                                                                   \
            printf("[ERROR]%s %d para too small [%d %d]!\n", __FUNCTION__, para, min, max); \
            return -1;                                                                      \
        }                                                                                   \
                                                                                            \
        if (para >= max)                                                                    \
        {                                                                                   \
            printf("[ERROR]%s %d para too big! [%d %d]\n", __FUNCTION__, para, min, max);   \
            return -1;                                                                      \
        }                                                                                   \
    }

static int i2c_write(int fd,unsigned char slave_addr, unsigned short reg_addr, unsigned char value)
{
    unsigned char outbuf[3];
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[1];

    //printf("[%s]fd %d, slave addr %x, regaddr %x value %x \n", __FUNCTION__, fd, slave_addr, reg_addr, value);

    messages[0].addr  = slave_addr>>1;
    messages[0].flags = 0;
    messages[0].len   = sizeof(outbuf);
    messages[0].buf   = outbuf;

    /* The first byte indicates which register we‘ll write */
    outbuf[0] = (reg_addr >> 8) & 0xff;
    outbuf[1] = reg_addr & 0xff;

    /*
     * The second byte indicates the value to write.  Note that for many
     * devices, we can write multiple, sequential registers at once by
     * simply making outbuf bigger.
     */
    outbuf[2] = value &0xff;

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

static int i2c_read(int fd, unsigned char slave_addr, unsigned short reg_addr, unsigned char *value)
{
    unsigned char inbuf, outbuf[2];
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

    /*
     * In order to read a register, we first do a "dummy write" by writing
     * 0 bytes to the register we want to read from.  This is similar to
     * the packet in set_i2c_register, except it‘s 1 byte rather than 2.
     */
    outbuf[0] = (reg_addr >> 8) & 0xff;
    outbuf[1] = reg_addr & 0xff;

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

int Cus_I2cBus2Id(DHC_I2C_BUS_E enI2cBus, MI_U8 *pu8I2cId)
{
    switch(enI2cBus)
    {
#if ((defined CONFIG_SIGMASTAR_CHIP_M6) && (CONFIG_SIGMASTAR_CHIP_M6 == 1))
        case DHC_I2C_BUS_0:
            *pu8I2cId = 1;
            break;
#elif ((defined CONFIG_SIGMASTAR_CHIP_I7) && (CONFIG_SIGMASTAR_CHIP_I7 == 1))
        case DHC_I2C_BUS_0:
            *pu8I2cId = 4;
            break;
        case DHC_I2C_BUS_1:
            *pu8I2cId = 5;
            break;
#elif ((defined CONFIG_SIGMASTAR_CHIP_M6P) && (CONFIG_SIGMASTAR_CHIP_M6P == 1))
        case DHC_I2C_BUS_0:
            *pu8I2cId = 1;
            break;
#elif ((defined CONFIG_SIGMASTAR_CHIP_I6C) && (CONFIG_SIGMASTAR_CHIP_I6C == 1))
        case DHC_I2C_BUS_0:
            *pu8I2cId = 1;
            break;
#elif ((defined CONFIG_SIGMASTAR_CHIP_P5) && (CONFIG_SIGMASTAR_CHIP_P5 == 1))
        case DHC_I2C_BUS_0:
            *pu8I2cId = 1;
            break;
#elif ((defined CONFIG_SIGMASTAR_CHIP_I6F) && (CONFIG_SIGMASTAR_CHIP_I6F == 1))
        case DHC_I2C_BUS_0:
            *pu8I2cId = 0;
            break;
#endif
        default:
            printf("[%s]:%d bt656 i2cBus:%d get I2cId err \n", __FUNCTION__, __LINE__, enI2cBus);
            return -1;
    }

    return 0;
}

int Cus_I2cId2Bus(MI_U8 u8I2cId, DHC_I2C_BUS_E *penI2cBus)
{
    switch(u8I2cId)
    {
#if ((defined CONFIG_SIGMASTAR_CHIP_M6) && (CONFIG_SIGMASTAR_CHIP_M6 == 1))
        case 1:
            *penI2cBus = DHC_I2C_BUS_0;
            break;
#elif ((defined CONFIG_SIGMASTAR_CHIP_I7) && (CONFIG_SIGMASTAR_CHIP_I7 == 1))
        case 4:
            *penI2cBus = DHC_I2C_BUS_0;
            break;
        case 5:
            *penI2cBus = DHC_I2C_BUS_1;
            break;
#elif ((defined CONFIG_SIGMASTAR_CHIP_M6P) && (CONFIG_SIGMASTAR_CHIP_M6P == 1))
        case 1:
            *penI2cBus = DHC_I2C_BUS_0;
            break;
#elif ((defined CONFIG_SIGMASTAR_CHIP_I6C) && (CONFIG_SIGMASTAR_CHIP_I6C == 1))
        case 1:
            *penI2cBus = DHC_I2C_BUS_0;
            break;
#elif ((defined CONFIG_SIGMASTAR_CHIP_P5) && (CONFIG_SIGMASTAR_CHIP_P5 == 1))
        case 1:
            *penI2cBus = DHC_I2C_BUS_0;
            break;
#elif ((defined CONFIG_SIGMASTAR_CHIP_I6F) && (CONFIG_SIGMASTAR_CHIP_I6F == 1))
        case 0:
            *penI2cBus = DHC_I2C_BUS_0;
            break;
#endif
        default:
            printf("[%s]:%d bt656 i2cid:%d get I2cBus err \n", __FUNCTION__, __LINE__, u8I2cId);
            return -1;
    }

    return 0;
}

static DHC_DH9931_ERR_ID_E DHC_DH9931_test_WriteByte(DHC_I2C_BUS_E enI2cBus, DHC_U8 u8ChipAddr, DHC_U16 u16RegAddr, DHC_U8 u8RegValue)
{
    //ms_cus_sensor *handle = g_handle;
    MI_U8 I2cId=0;
#if 0
    if(DHC_I2C_BUS_0 == enI2cBus)
    {
        //printf("iic w %#x  %#x %#x\n", u8ChipAddr, u16RegAddr, u8RegValue);
        // #if HISI3531A_CHANX
        // DHC_TEST_IIC_MsgWrite(u8ChipAddr, u16RegAddr, u8RegValue);
        // #endif

        // #if HISI3531A_XPB
        // hi_i2c_write(0, u8ChipAddr, u16RegAddr, u8RegValue, 2, 1);
        // #endif
        I2cId = 1;
    }
    else if(DHC_I2C_BUS_1 == enI2cBus)
    {

    }
#endif
    Cus_I2cBus2Id(enI2cBus, &I2cId);

    i2c_write(gstI2cInfo[I2cId].I2cFd, u8ChipAddr, u16RegAddr, u8RegValue);
    //printf("[%s] I2cId %d ChipAddr = %#x , RegAddr = %#x, Val = %#x\n", __FUNCTION__,I2cId, u8ChipAddr, u16RegAddr, u8RegValue);

    return 0;
}

static DHC_U8 DHC_DH9931_test_ReadByte(DHC_I2C_BUS_E enI2cBus, DHC_U8 u8ChipAddr, DHC_U16 u16RegAddr)
{
    DHC_U8 u8Tmp = 0;
    // DHC_MI_U32 MI_U32Tmp;
    MI_U16 sen_data;
    MI_U8 I2cId=0;
#if 0
    //ms_cus_sensor *handle = g_handle;
    if(DHC_I2C_BUS_0 == enI2cBus)
    {
        // #if HISI3531A_CHANX
        // DHC_TEST_IIC_MsgRead(u8ChipAddr, u16RegAddr, &u8Tmp, 1);
        // #endif

        // #if HISI3531A_XPB
        // hi_i2c_read(0, u8ChipAddr, u16RegAddr, u16RegAddr + 1, &MI_U32Tmp, 2, 1, 1);
        // u8Tmp = MI_U32Tmp & 0xff;
        // #endif

        //printf("Rd : ChipAddr = %#x , RegAddr = %#x, Val = %#x\n", u8ChipAddr, u16RegAddr, u8Tmp);
        I2cId = 1;

    }
    else if(DHC_I2C_BUS_1 == enI2cBus)
    {

    }
#endif
    Cus_I2cBus2Id(enI2cBus, &I2cId);

    /* Read Product ID */
    i2c_read(gstI2cInfo[I2cId].I2cFd, u8ChipAddr, u16RegAddr, (void *)&sen_data);
    u8Tmp = (MI_U8)sen_data;
    //printf("[%s]i2c %d ChipAddr = %#x , RegAddr = %#x, Val = %#x\n", __FUNCTION__, I2cId, u8ChipAddr, u16RegAddr, u8Tmp);

    return u8Tmp;

}

int Cus_GetI2cId(MI_U8 u8ChipIndex, MI_U8 *pu8I2cId)
{
    DH9931_CHECK_POINTER(pu8I2cId);

    switch(u8ChipIndex)
    {
#if ((defined CONFIG_SIGMASTAR_CHIP_M6) && (CONFIG_SIGMASTAR_CHIP_M6 == 1))
        case 0:
        case 1:
            *pu8I2cId = 1;
            break;
#elif ((defined CONFIG_SIGMASTAR_CHIP_I7) && (CONFIG_SIGMASTAR_CHIP_I7 == 1))
        case 0:
            *pu8I2cId = 4;
            break;
        case 1:
            *pu8I2cId = 5;
            break;
#elif ((defined CONFIG_SIGMASTAR_CHIP_M6P) && (CONFIG_SIGMASTAR_CHIP_M6P == 1))
        case 0:
        case 1:
            *pu8I2cId = 1;
            break;
#elif ((defined CONFIG_SIGMASTAR_CHIP_I6C) && (CONFIG_SIGMASTAR_CHIP_I6C == 1))
        case 0:
            *pu8I2cId = 1;
            break;
#elif ((defined CONFIG_SIGMASTAR_CHIP_P5) && (CONFIG_SIGMASTAR_CHIP_P5 == 1))
        case 0:
            *pu8I2cId = 1;
            break;
#elif ((defined CONFIG_SIGMASTAR_CHIP_I6F) && (CONFIG_SIGMASTAR_CHIP_I6F == 1))
        case 0:
            *pu8I2cId = 0;
            break;
#endif
        default:
            printf("[%s]:%d 9931 chipIndex%d get I2C id err \n", __FUNCTION__, __LINE__, u8ChipIndex);
            return -1;
    }

    return 0;
}

int Cus_SetDH9931Data(MI_U8 u8ChipIndex, MI_U16 u16RegAddr, MI_U8 u8RegValue)
{
    MI_U8 u8I2cId=0;

    DH9931_CHECK_PARA_RANGE(u8ChipIndex, 0, MAX_DH9931_NUM);
    Cus_GetI2cId(u8ChipIndex, &u8I2cId);

    //printf("[%s] I2c%d ChipAddr = %#x , RegAddr = %#x, Val = %#x\n", __FUNCTION__, u8I2cId, gstSensorHandle[u8ChipIndex].stDh9931Attr.u8ChipAddr, u16RegAddr, u8RegValue);

    i2c_write(gstI2cInfo[u8I2cId].I2cFd, gstSensorHandle[u8ChipIndex].stDh9931Attr.u8ChipAddr, u16RegAddr, u8RegValue);
    return 0;
}
int Cus_GetDH9931Data(MI_U8 u8ChipIndex, MI_U16 u16RegAddr, MI_U8* pu8RegValue)
{
    MI_U8 u8I2cId=0;

    DH9931_CHECK_PARA_RANGE(u8ChipIndex, 0, MAX_DH9931_NUM);
    Cus_GetI2cId(u8ChipIndex, &u8I2cId);

    i2c_read(gstI2cInfo[u8I2cId].I2cFd, gstSensorHandle[u8ChipIndex].stDh9931Attr.u8ChipAddr, u16RegAddr, pu8RegValue);
    //printf("[%s] I2c%d ChipAddr = %#x , RegAddr = %#x, Val = %#x\n", __FUNCTION__, u8I2cId, gstSensorHandle[u8ChipIndex].stDh9931Attr.u8ChipAddr, u16RegAddr, *pu8RegValue);
    return 0;
}

static void DHC_DH9931_test_MSleep(DHC_U8 u8MsDly)
{
    usleep(u8MsDly*1000);
}

static void DHC_DH9931_test_Printf(char *pszStr)
{
    printf("%s", pszStr);
}

static void DHC_DH9931_test_GetLock(void)
{
    //printf("DHC_DH9931_test_GetLock\n");
}

static void DHC_DH9931_test_ReleaseLock(void)
{
    //printf("DHC_DH9931_test_ReleaseLock\n");
}

void _dump_9931_reg(int I2cBus, unsigned char u8ChipAddr)
{
    //int tmp = 0, i = 0, j = 0, k = 0;
    int i = 0, k = 0;
    char banks[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 0xA, 0xB, 0x40, 0x41, 0x42, 0x43, 0x44};
    int bank = 0;
    short reg = 0x00;
    printf("********************IIC ADDR 0x%02x ********************\r\n", u8ChipAddr);

    for(k = 0; k < 4; ++k)
    {
        printf("********************PhyChn: %d ********************\r\n", k);
        for(bank = 0; bank < sizeof(banks)/sizeof(banks[0]); bank++)
        {
            printf("********************Bank[0x%x] ********************\r\n", banks[bank]);
#if 0
            for(i = 0; i <= 0xF; i++)
            {

                if(0 == i)
                    printf("    0x%02x ", i);
                else if(0xF == i)
                    printf("0x%02x\r\n", i);
                else
                    printf("0x%02x ", i);
            }
#else
            i = 0;
            printf("     0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\r\n",
                    i, i+1, i+2, i+3, i+4, i+5, i+6, i+7, i+8, i+9, i+10, i+11, i+12, i+13, i+14, i+15);
#endif
            for(i = 0; i <= 0xF; i++)
            {
#if 0
                for(j = 0; j <= 0xF; j++)
                {
                    if(banks[bank] < 0x40)
                        reg = (k << 12) | (banks[bank] << 8) | (i << 4) | j;
                    else
                        reg = (banks[bank] << 8) | (i << 4) | j;

                    tmp = DHC_DH9931_test_ReadByte(I2cBus, u8ChipAddr, reg);
                    if(0 == j)
                        printf("0x%02x-0x%02x ", (i << 4) | j, tmp);
                    else if(0xF == j)
                        printf("0x%02x\r\n", tmp);
                    else
                        printf("0x%02x ", tmp);
                }
#else
                if(banks[bank] < 0x40)
                {
                    reg = (k << 12) | (banks[bank] << 8) | (i << 4);
                }
                else
                {
                    reg = (banks[bank] << 8) | (i << 4);
                }
                printf("0x%02x-0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\r\n",
                        i << 4, DHC_DH9931_test_ReadByte(I2cBus, u8ChipAddr, reg),
                        DHC_DH9931_test_ReadByte(I2cBus, u8ChipAddr, reg|1),
                        DHC_DH9931_test_ReadByte(I2cBus, u8ChipAddr, reg|2),
                        DHC_DH9931_test_ReadByte(I2cBus, u8ChipAddr, reg|3),
                        DHC_DH9931_test_ReadByte(I2cBus, u8ChipAddr, reg|4),
                        DHC_DH9931_test_ReadByte(I2cBus, u8ChipAddr, reg|5),
                        DHC_DH9931_test_ReadByte(I2cBus, u8ChipAddr, reg|6),
                        DHC_DH9931_test_ReadByte(I2cBus, u8ChipAddr, reg|7),
                        DHC_DH9931_test_ReadByte(I2cBus, u8ChipAddr, reg|8),
                        DHC_DH9931_test_ReadByte(I2cBus, u8ChipAddr, reg|9),
                        DHC_DH9931_test_ReadByte(I2cBus, u8ChipAddr, reg|10),
                        DHC_DH9931_test_ReadByte(I2cBus, u8ChipAddr, reg|11),
                        DHC_DH9931_test_ReadByte(I2cBus, u8ChipAddr, reg|12),
                        DHC_DH9931_test_ReadByte(I2cBus, u8ChipAddr, reg|13),
                        DHC_DH9931_test_ReadByte(I2cBus, u8ChipAddr, reg|14),
                        DHC_DH9931_test_ReadByte(I2cBus, u8ChipAddr, reg|15));
#endif
            }
        }
    }
    printf("0x4500 = 0x%x \r\n", DHC_DH9931_test_ReadByte(I2cBus, u8ChipAddr, 0x4500));
    printf("********************Dump end ********************\r\n");
}

DHC_VOID DHC_DH9931_Config_Global(DHC_DH9931_USR_CFG_S *pstDh9931, MI_U8 u8ADCnt)
{
    printf("9931 init adcnt %d \n", u8ADCnt);
    pstDh9931->u8AdCount = u8ADCnt;

    pstDh9931->u8DetectPeriod = 0;

    pstDh9931->DH9931_WriteByte = DHC_DH9931_test_WriteByte;
    pstDh9931->DH9931_ReadByte = DHC_DH9931_test_ReadByte;

    pstDh9931->DH9931_MSleep = DHC_DH9931_test_MSleep;

    pstDh9931->DH9931_Printf = DHC_DH9931_test_Printf;

    pstDh9931->DH9931_GetLock = DHC_DH9931_test_GetLock;
    pstDh9931->DH9931_ReleaseLock = DHC_DH9931_test_ReleaseLock;
    DHC_DH9931_SDK_Init(pstDh9931);
}


DHC_VOID DHC_DH9931_Config_EachChip(MI_U8 u8ChipIndex, DHC_DH9931_ATTR_S *pstDh9931Attr, int intX, int intRes)
{
    //DHC_DH9931_USR_CFG_S *pstDh9931;
    int j = 0;
    // DHC_BOOL bHalf720p = 0;
    MI_U8 I2cId=0;
    // DHC_BOOL bHalf4M = 0;
    // DHC_BOOL bHalf4K = 0;
    // DHC_U8 au8Buf[8];
    // DHC_U8 u8Tmp = 0;
    DHC_DH9931_VO_MODE_E SetVoMode;
    ss_cus_sensor_t *handle =  &gstSensorHandle[u8ChipIndex];
    if(DHC_NULL == pstDh9931Attr)
    {
        /* error, do something */
        return;
    }
    //pstDh9931 = pstDh9931;
    //pstDh9931->u8AdCount = 1;

    Cus_GetI2cId(u8ChipIndex, &I2cId);
    Cus_I2cId2Bus(I2cId, &pstDh9931Attr->enI2cBus);

    if(CUS_SENIF_BUS_BT656 == handle->sif_bus)
    {
        SetVoMode = DHC_VO_MODE_BT656;
    }
    else if(CUS_SENIF_BUS_BT1120 == handle->sif_bus)
    {
        SetVoMode = DHC_VO_MODE_BT1120;
    }
    else
    {
        printf("DHC_DH9931_VO_MODE_E only support BT656/BT1120,set %d error, default use bt656\n",handle->sif_bus);
        SetVoMode = DHC_VO_MODE_BT656;
    }

    switch(u8ChipIndex)
    {
        case 0:
            pstDh9931Attr->u8ChipAddr=0x60;
            break;
        case 1:
            pstDh9931Attr->u8ChipAddr=0x62;
            break;
        case 2:
            pstDh9931Attr->u8ChipAddr=0x64;
            break;
        case 3:
            pstDh9931Attr->u8ChipAddr=0x66;
            break;
        default:
            printf("DH9931 ChipIndex%d invalid \n", u8ChipIndex);
            return;
    }

    pstDh9931Attr->u8ProtocolType = 0;
    pstDh9931Attr->bCheckChipId = DHC_TRUE;
#if ((defined CONFIG_SIGMASTAR_CHIP_I7) && (CONFIG_SIGMASTAR_CHIP_I7 == 1)) //I7 hw not support 3.3v
    pstDh9931Attr->enDriverPower = DHC_DRIVER_POWER_18V;//DHC_DRIVER_POWER_18V DHC_DRIVER_POWER_33V
#else
    pstDh9931Attr->enDriverPower = DHC_DRIVER_POWER_33V;
#endif
    pstDh9931Attr->enEqMode = DHC_INTERNAL_CASCADE;

    // #if HISI3531A_CHANX
    //pstDh9931->stDH9931Attr[0].u8ChipAddr = 0x60;
    // #endif
    // #if HISI3531A_XPB
     //pstDh9931->stDH9931Attr[0].u8ChipAddr = 0x60;
    // #endif

    /* 0: 1xģʽ */
    if(0 == intX)
    {
        printf("1x\n");

        pstDh9931Attr->enVoWorkMode = DHC_WORK_MODE_1Multiplex;

        /* 0: 720p ������*/
        if(0 == intRes)
        {
            printf("1x 720p    1280x720@25f\n");

            for(j = 0; j < DHC_DH9931_MAX_CHNS; j++)
            {
                /* ����ͬ����Ƶ�¾�ģʽ */
                pstDh9931Attr->enCoaxialMode[j] = DHC_COAXIAL_MODE_NEW;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enNetraMode = DHC_NETRA_MODE_SINGLE;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnIdMode = DHC_CHN_ID_MODE_BOTH;
                if(0 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_0;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }
                else if(1 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_1;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }
                else if(2 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_2;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }
                else if(3 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_3;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enProtocol = DHC_PROTOCOL_CVI;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enFreeRunFmt = DHC_CVI_1280x720_25HZ;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoClkEdge = DHC_VOCLK_EDGE_RISING;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoSdMode = DHC_VO_SD_MODE_960H;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoHdGmMode = DHC_VO_HD_GM_MODE_148M;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoMode = SetVoMode;

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enSdFmt = DHC_SD_FREERUN_FMT_P;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdSdSel = DHC_SD_FREERUN_HD;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdClk = DHC_SD_FREERUN_HD_74_25;
            }
        }
        /* 1: 1080p,������ */
        else if(1 == intRes)
        {
            printf("1x 1080p    1920x1080@25\n");

            for(j = 0; j < DHC_DH9931_MAX_CHNS; j++)
            {
                /* ����ͬ����Ƶ�¾�ģʽ */
                pstDh9931Attr->enCoaxialMode[j] = DHC_COAXIAL_MODE_NEW;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enNetraMode = DHC_NETRA_MODE_SINGLE;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnIdMode = DHC_CHN_ID_MODE_BOTH;
                if(0 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_0;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }
                else if(1 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_1;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }
                else if(2 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_2;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }
                else if(3 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_3;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enProtocol = DHC_PROTOCOL_CVI;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enFreeRunFmt = DHC_CVI_1920x1080_25HZ;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoClkEdge = DHC_VOCLK_EDGE_RISING;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoSdMode = DHC_VO_SD_MODE_960H;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoHdGmMode = DHC_VO_HD_GM_MODE_148M;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoMode = SetVoMode;

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enSdFmt = DHC_SD_FREERUN_FMT_P;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdSdSel = DHC_SD_FREERUN_HD;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdClk = DHC_SD_FREERUN_HD_74_25;
            }
        }
        /* 2: 1x 1080p ˫��  */
        else if(2 == intRes)
        {

            for(j = 0; j < DHC_DH9931_MAX_CHNS; j++)
            {
                pstDh9931Attr->enCoaxialMode[j] = DHC_COAXIAL_MODE_NEW;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enNetraMode = DHC_NETRA_MODE_SINGLE;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnIdMode = DHC_CHN_ID_MODE_BOTH;
                if(0 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_0;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }
                else if(1 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_1;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_YELLOW;
                }
                else if(2 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_2;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_CYAN;
                }
                else if(3 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_3;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_GREEN;
                }

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enProtocol = DHC_PROTOCOL_CVI;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enFreeRunFmt = DHC_CVI_1920x1080_25HZ;


                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoClkEdge = DHC_VOCLK_EDGE_DUAL;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoSdMode = DHC_VO_SD_MODE_960H;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoHdGmMode = DHC_VO_HD_GM_MODE_148M;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoMode = SetVoMode;

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enSdFmt = DHC_SD_FREERUN_FMT_P;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdSdSel = DHC_SD_FREERUN_HD;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdClk = DHC_SD_FREERUN_HD_148_5;
            }
        }
        /* 3: 1x 720p ˫�� */
        else if(3 == intRes)
        {
            printf("1x 720p ˫�� \n");

            for(j = 0; j < DHC_DH9931_MAX_CHNS; j++)
            {
                /* ����ͬ����Ƶ�¾�ģʽ */
                pstDh9931Attr->enCoaxialMode[j] = DHC_COAXIAL_MODE_NEW;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enNetraMode = DHC_NETRA_MODE_SINGLE;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnIdMode = DHC_CHN_ID_MODE_BOTH;
                if(0 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_0;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }
                else if(1 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_1;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_YELLOW;
                }
                else if(2 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_2;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_CYAN;
                }
                else if(3 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_3;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_GREEN;
                }

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enProtocol = DHC_PROTOCOL_CVI;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enFreeRunFmt = DHC_CVI_1280x720_25HZ;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoClkEdge = DHC_VOCLK_EDGE_DUAL;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoSdMode = DHC_VO_SD_MODE_960H;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoHdGmMode = DHC_VO_HD_GM_MODE_74M;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoMode = SetVoMode;

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enSdFmt = DHC_SD_FREERUN_FMT_P;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdSdSel = DHC_SD_FREERUN_HD;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdClk = DHC_SD_FREERUN_HD_74_25;
            }
        }
        /* 4: 960H ������ */
        else if(4 == intRes)
        {
            printf("1x 960H\n");

            for(j = 0; j < DHC_DH9931_MAX_CHNS; j++)
            {
                /* ����ͬ����Ƶ�¾�ģʽ */
                pstDh9931Attr->enCoaxialMode[j] = DHC_COAXIAL_MODE_NEW;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enNetraMode = DHC_NETRA_MODE_SINGLE;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnIdMode = DHC_CHN_ID_MODE_BOTH;
                if(0 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_0;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }
                else if(1 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_1;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_YELLOW;
                }
                else if(2 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_2;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_CYAN;
                }
                else if(3 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_3;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_GREEN;
                }

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enProtocol = DHC_PROTOCOL_CVI;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enFreeRunFmt = DHC_CVI_1280x720_25HZ;


                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoClkEdge = DHC_VOCLK_EDGE_RISING;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoSdMode = DHC_VO_SD_MODE_960H;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoHdGmMode = DHC_VO_HD_GM_MODE_148M;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoMode = SetVoMode;

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enSdFmt = DHC_SD_FREERUN_FMT_P;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdSdSel = DHC_SD_FREERUN_SD;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdClk = DHC_SD_FREERUN_HD_74_25;
            }

        }
        /* 5: D1 ������ */
        else if(5 == intRes)
        {
            printf("1x D1\n");

            for(j = 0; j < DHC_DH9931_MAX_CHNS; j++)
            {
                /* ����ͬ����Ƶ�¾�ģʽ */
                pstDh9931Attr->enCoaxialMode[j] = DHC_COAXIAL_MODE_NEW;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enNetraMode = DHC_NETRA_MODE_SINGLE;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnIdMode = DHC_CHN_ID_MODE_BOTH;
                if(0 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_0;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }
                else if(1 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_1;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_YELLOW;
                }
                else if(2 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_2;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_CYAN;
                }
                else if(3 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_3;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_GREEN;
                }

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enProtocol = DHC_PROTOCOL_CVI;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enFreeRunFmt = DHC_CVI_1280x720_25HZ;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoClkEdge = DHC_VOCLK_EDGE_RISING;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoSdMode = DHC_VO_SD_MODE_720H;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoHdGmMode = DHC_VO_HD_GM_MODE_148M;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoMode = SetVoMode;

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enSdFmt = DHC_SD_FREERUN_FMT_P;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdSdSel = DHC_SD_FREERUN_SD;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdClk = DHC_SD_FREERUN_HD_148_5;
            }

        }
        /* 6: 4M */
        else if(6 == intRes)
        {
            printf("1x 4M    2560x1440\n");

            for(j = 0; j < DHC_DH9931_MAX_CHNS; j++)
            {
                /* ����ͬ����Ƶ�¾�ģʽ */
                pstDh9931Attr->enCoaxialMode[j] = DHC_COAXIAL_MODE_NEW;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enNetraMode = DHC_NETRA_MODE_SINGLE;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnIdMode = DHC_CHN_ID_MODE_BOTH;
                if(0 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_0;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }
                else if(1 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_1;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_YELLOW;
                }
                else if(2 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_2;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_CYAN;
                }
                else if(3 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_3;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_GREEN;
                }

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enProtocol = DHC_PROTOCOL_CVI;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enFreeRunFmt = DHC_CVI_2560x1440_25HZ;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoClkEdge = DHC_VOCLK_EDGE_DUAL;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoSdMode = DHC_VO_SD_MODE_960H;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoHdGmMode = DHC_VO_HD_GM_MODE_148M;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoMode = SetVoMode;

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enSdFmt = DHC_SD_FREERUN_FMT_P;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdSdSel = DHC_SD_FREERUN_HD;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdClk = DHC_SD_FREERUN_HD_297;
            }

        }
        /* 7: 4K */
        else if(7 == intRes)
        {
            printf("1x 4K    3840x2160@12\n");

            for(j = 0; j < DHC_DH9931_MAX_CHNS; j++)
            {
                /* ����ͬ����Ƶ�¾�ģʽ */
                pstDh9931Attr->enCoaxialMode[j] = DHC_COAXIAL_MODE_NEW;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enNetraMode = DHC_NETRA_MODE_SINGLE;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnIdMode = DHC_CHN_ID_MODE_BOTH;
                if(0 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_0;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }
                else if(1 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_1;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_YELLOW;
                }
                else if(2 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_2;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_CYAN;
                }
                else if(3 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_3;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_GREEN;
                }

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enProtocol = DHC_PROTOCOL_CVI;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enFreeRunFmt = DHC_CVI_3840x2160_12HZ;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoClkEdge = DHC_VOCLK_EDGE_DUAL;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoSdMode = DHC_VO_SD_MODE_960H;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoHdGmMode = DHC_VO_HD_GM_MODE_148M;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoMode = SetVoMode;

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enSdFmt = DHC_SD_FREERUN_FMT_P;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdSdSel = DHC_SD_FREERUN_HD;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdClk = DHC_SD_FREERUN_HD_297;
            }

        }
        /* 8: 4M  */
        else if(8 == intRes)
        {
            printf("1x 4M double 2560x1440 half\n");

            for(j = 0; j < DHC_DH9931_MAX_CHNS; j++)
            {
                /* ����ͬ����Ƶ�¾�ģʽ */
                pstDh9931Attr->enCoaxialMode[j] = DHC_COAXIAL_MODE_NEW;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enNetraMode = DHC_NETRA_MODE_SINGLE;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnIdMode = DHC_CHN_ID_MODE_BOTH;
                if(0 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_0;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }
                else if(1 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_1;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_YELLOW;
                }
                else if(2 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_2;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_CYAN;
                }
                else if(3 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_3;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_GREEN;
                }

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enProtocol = DHC_PROTOCOL_CVI;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enFreeRunFmt = DHC_CVI_2560x1440_25HZ;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoClkEdge = DHC_VOCLK_EDGE_DUAL;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoSdMode = DHC_VO_SD_MODE_960H;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoHdGmMode = DHC_VO_HD_GM_MODE_148M;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoMode = SetVoMode;

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enSdFmt = DHC_SD_FREERUN_FMT_P;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdSdSel = DHC_SD_FREERUN_HD;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdClk = DHC_SD_FREERUN_HD_297;
            }
        }
        /* 9:  3M 2040x1536 */
        else if(9 == intRes)
        {
            printf("1x 3M ˫��\n");

            for(j = 0; j < DHC_DH9931_MAX_CHNS; j++)
            {
                /* ����ͬ����Ƶ�¾�ģʽ */
                pstDh9931Attr->enCoaxialMode[j] = DHC_COAXIAL_MODE_NEW;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enNetraMode = DHC_NETRA_MODE_SINGLE;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnIdMode = DHC_CHN_ID_MODE_BOTH;
                if(0 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_0;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }
                else if(1 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_1;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_YELLOW;
                }
                else if(2 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_2;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_CYAN;
                }
                else if(3 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_3;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_GREEN;
                }

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enProtocol = DHC_PROTOCOL_CVI;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enFreeRunFmt = DHC_CVI_2048x1536_30HZ;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoClkEdge = DHC_VOCLK_EDGE_DUAL;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoSdMode = DHC_VO_SD_MODE_960H;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoHdGmMode = DHC_VO_HD_GM_MODE_148M;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoMode = SetVoMode;

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enSdFmt = DHC_SD_FREERUN_FMT_P;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdSdSel = DHC_SD_FREERUN_HD;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdClk = DHC_SD_FREERUN_HD_288;
            }
        }
        /* 10: 3M 2040x1536*/
        else if(10 == intRes)
        {
            printf("1x 3M    2040x1536\n");

            for(j = 0; j < DHC_DH9931_MAX_CHNS; j++)
            {
                /* ����ͬ����Ƶ�¾�ģʽ */
                pstDh9931Attr->enCoaxialMode[j] = DHC_COAXIAL_MODE_NEW;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enNetraMode = DHC_NETRA_MODE_SINGLE;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnIdMode = DHC_CHN_ID_MODE_BOTH;
                if(0 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_0;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }
                else if(1 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_1;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_YELLOW;
                }
                else if(2 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_2;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_CYAN;
                }
                else if(3 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_3;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_GREEN;
                }

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enProtocol = DHC_PROTOCOL_CVI;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enFreeRunFmt = DHC_CVI_2048x1536_30HZ;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoClkEdge = DHC_VOCLK_EDGE_RISING;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoSdMode = DHC_VO_SD_MODE_960H;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoHdGmMode = DHC_VO_HD_GM_MODE_148M;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoMode = SetVoMode;

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enSdFmt = DHC_SD_FREERUN_FMT_P;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdSdSel = DHC_SD_FREERUN_HD;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdClk = DHC_SD_FREERUN_HD_288;
            }

        }
        /* 11: 3M 2040x1152*/
        else if(11 == intRes)
        {
            printf("1x 3M    2040x1152\n");

            for(j = 0; j < DHC_DH9931_MAX_CHNS; j++)
            {
                /* ����ͬ����Ƶ�¾�ģʽ */
                pstDh9931Attr->enCoaxialMode[j] = DHC_COAXIAL_MODE_NEW;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enNetraMode = DHC_NETRA_MODE_SINGLE;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnIdMode = DHC_CHN_ID_MODE_BOTH;
                if(0 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_0;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }
                else if(1 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_1;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_YELLOW;
                }
                else if(2 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_2;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_CYAN;
                }
                else if(3 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_3;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_GREEN;
                }

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enProtocol = DHC_PROTOCOL_CVI;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enFreeRunFmt = DHC_CVI_2048x1152_30HZ;


                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoClkEdge = DHC_VOCLK_EDGE_RISING;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoSdMode = DHC_VO_SD_MODE_960H;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoHdGmMode = DHC_VO_HD_GM_MODE_148M;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoMode = SetVoMode;

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enSdFmt = DHC_SD_FREERUN_FMT_P;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdSdSel = DHC_SD_FREERUN_HD;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdClk = DHC_SD_FREERUN_HD_288;
            }

        }
        /* 12: 3M 2304x1296*/
        else if(12 == intRes)
        {
            printf("1x 3M    2304x1296\n");

            for(j = 0; j < DHC_DH9931_MAX_CHNS; j++)
            {
                /* ����ͬ����Ƶ�¾�ģʽ */
                pstDh9931Attr->enCoaxialMode[j] = DHC_COAXIAL_MODE_NEW;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enNetraMode = DHC_NETRA_MODE_SINGLE;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_0;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnIdMode = DHC_CHN_ID_MODE_BOTH;
                if(0 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_0;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }
                else if(1 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_1;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_YELLOW;
                }
                else if(2 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_2;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_CYAN;
                }
                else if(3 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_3;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_GREEN;
                }

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enProtocol = DHC_PROTOCOL_CVI;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enFreeRunFmt = DHC_CVI_2304x1296_25HZ;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoClkEdge = DHC_VOCLK_EDGE_RISING;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoSdMode = DHC_VO_SD_MODE_960H;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoHdGmMode = DHC_VO_HD_GM_MODE_148M;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoMode = SetVoMode;

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enSdFmt = DHC_SD_FREERUN_FMT_P;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdSdSel = DHC_SD_FREERUN_HD;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdClk = DHC_SD_FREERUN_HD_297;
            }

        }
        /* 13: 5M 2560x1920*/
        else if(13 == intRes)
        {
            printf("1x 5M     2560x1920\n");

            for(j = 0; j < DHC_DH9931_MAX_CHNS; j++)
            {
                /* ����ͬ����Ƶ�¾�ģʽ */
                pstDh9931Attr->enCoaxialMode[j] = DHC_COAXIAL_MODE_NEW;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enNetraMode = DHC_NETRA_MODE_SINGLE;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnIdMode = DHC_CHN_ID_MODE_BOTH;
                if(0 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_0;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }
                else if(1 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_1;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_YELLOW;
                }
                else if(2 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_2;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_CYAN;
                }
                else if(3 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_3;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_GREEN;
                }

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enProtocol = DHC_PROTOCOL_CVI;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enFreeRunFmt = DHC_CVI_2560x1920_20HZ;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoClkEdge = DHC_VOCLK_EDGE_RISING;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoSdMode = DHC_VO_SD_MODE_960H;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoHdGmMode = DHC_VO_HD_GM_MODE_148M;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoMode = SetVoMode;

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enSdFmt = DHC_SD_FREERUN_FMT_P;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdSdSel = DHC_SD_FREERUN_HD;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdClk = DHC_SD_FREERUN_HD_288;
            }

        }
        /* 14: 5M 3072x1728*/
        else if(14 == intRes)
        {
            printf("1x 5M     3072x1728\n");

            for(j = 0; j < DHC_DH9931_MAX_CHNS; j++)
            {
                /* ����ͬ����Ƶ�¾�ģʽ */
                pstDh9931Attr->enCoaxialMode[j] = DHC_COAXIAL_MODE_NEW;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enNetraMode = DHC_NETRA_MODE_SINGLE;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnIdMode = DHC_CHN_ID_MODE_BOTH;
                if(0 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_0;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }
                else if(1 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_1;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_YELLOW;
                }
                else if(2 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_2;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_CYAN;
                }
                else if(3 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_3;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_GREEN;
                }

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enProtocol = DHC_PROTOCOL_CVI;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enFreeRunFmt = DHC_CVI_3072x1728_20HZ;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoClkEdge = DHC_VOCLK_EDGE_RISING;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoSdMode = DHC_VO_SD_MODE_960H;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoHdGmMode = DHC_VO_HD_GM_MODE_148M;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoMode = SetVoMode;

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enSdFmt = DHC_SD_FREERUN_FMT_P;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdSdSel = DHC_SD_FREERUN_HD;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdClk = DHC_SD_FREERUN_HD_288;
            }

        }
        /* 15: 6M */
        else if(15 == intRes)
        {
            printf("1x 6M    2880x1920\n");

            for(j = 0; j < DHC_DH9931_MAX_CHNS; j++)
            {
                /* ����ͬ����Ƶ�¾�ģʽ */
                pstDh9931Attr->enCoaxialMode[j] = DHC_COAXIAL_MODE_NEW;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enNetraMode = DHC_NETRA_MODE_SINGLE;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_0;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnIdMode = DHC_CHN_ID_MODE_BOTH;
                if(0 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_0;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }
                else if(1 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_1;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_YELLOW;
                }
                else if(2 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_2;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_CYAN;
                }
                else if(3 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_3;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_GREEN;
                }

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enProtocol = DHC_PROTOCOL_CVI;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enFreeRunFmt = DHC_CVI_2880x1920_20HZ;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoClkEdge = DHC_VOCLK_EDGE_RISING;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoSdMode = DHC_VO_SD_MODE_960H;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoHdGmMode = DHC_VO_HD_GM_MODE_148M;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoMode = SetVoMode;

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enSdFmt = DHC_SD_FREERUN_FMT_P;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdSdSel = DHC_SD_FREERUN_HD;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdClk = DHC_SD_FREERUN_HD_288;
            }
        }
        /* 16: 8M */
        else if(16 == intRes)
        {
            printf("1x 8M    3840x2160@15\n");

            for(j = 0; j < DHC_DH9931_MAX_CHNS; j++)
            {
                /* ����ͬ����Ƶ�¾�ģʽ */
                pstDh9931Attr->enCoaxialMode[j] = DHC_COAXIAL_MODE_NEW;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enNetraMode = DHC_NETRA_MODE_SINGLE;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnIdMode = DHC_CHN_ID_MODE_BOTH;
                if(0 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_0;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }
                else if(1 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_1;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_YELLOW;
                }
                else if(2 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_2;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_CYAN;
                }
                else if(3 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_3;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_GREEN;
                }

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enProtocol = DHC_PROTOCOL_CVI;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enFreeRunFmt = DHC_CVI_3840x2160_15HZ;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoClkEdge = DHC_VOCLK_EDGE_DUAL;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoSdMode = DHC_VO_SD_MODE_960H;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoHdGmMode = DHC_VO_HD_GM_MODE_148M;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoMode = SetVoMode;

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enSdFmt = DHC_SD_FREERUN_FMT_P;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdSdSel = DHC_SD_FREERUN_HD;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdClk = DHC_SD_FREERUN_HD_297;
            }

        }
    }
    else if(1 == intX)
    {
        printf("2x\n");

        pstDh9931Attr->enVoWorkMode = DHC_WORK_MODE_2Multiplex;

        /* 720p */
        if(0 == intRes)
        {
            printf("2x 720p 1280x720\n");

            for(j = 0; j < DHC_DH9931_MAX_CHNS; j++)
            {
                /* ����ͬ����Ƶ�¾�ģʽ */
                pstDh9931Attr->enCoaxialMode[j] = DHC_COAXIAL_MODE_NEW;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enNetraMode = DHC_NETRA_MODE_SINGLE;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnIdMode = DHC_CHN_ID_MODE_BOTH;
                if(0 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_0;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }
                else if(1 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_1;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_YELLOW;
                }
                else if(2 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_2;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_CYAN;
                }
                else if(3 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_3;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_GREEN;
                }

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enProtocol = DHC_PROTOCOL_CVI;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enFreeRunFmt = DHC_CVI_1280x720_25HZ;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoClkEdge = DHC_VOCLK_EDGE_DUAL;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoSdMode = DHC_VO_SD_MODE_960H;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoHdGmMode = DHC_VO_HD_GM_MODE_148M;//DHC_VO_HD_GM_MODE_74M;//DHC_VO_HD_GM_MODE_148M;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoMode = SetVoMode;

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enSdFmt = DHC_SD_FREERUN_FMT_P;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdSdSel = DHC_SD_FREERUN_HD;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdClk = DHC_SD_FREERUN_HD_148_5;//DHC_SD_FREERUN_HD_74_25;
            }
        }
        /* 1080p */
        if(1 == intRes)
        {
            printf("2x 1080p 1920x1080\n");

            for(j = 0; j < DHC_DH9931_MAX_CHNS; j++)
            {
                /* ����ͬ����Ƶ�¾�ģʽ */
                pstDh9931Attr->enCoaxialMode[j] = DHC_COAXIAL_MODE_NEW;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enNetraMode = DHC_NETRA_MODE_SINGLE;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnIdMode = DHC_CHN_ID_MODE_BOTH;
                if(0 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_0;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_YELLOW;
                }
                else if(1 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_1;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_CYAN;
                }
                else if(2 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_2;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_GREEN;
                }
                else if(3 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_3;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enProtocol = DHC_PROTOCOL_CVI;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enFreeRunFmt = DHC_CVI_1920x1080_25HZ;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoClkEdge = DHC_VOCLK_EDGE_DUAL;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoSdMode = DHC_VO_SD_MODE_960H;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoHdGmMode = DHC_VO_HD_GM_MODE_148M;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoMode = SetVoMode;

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enSdFmt = DHC_SD_FREERUN_FMT_P;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdSdSel = DHC_SD_FREERUN_HD;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdClk = DHC_SD_FREERUN_HD_148_5;
            }
        }
    }
    else if(2 == intX)
    {
        printf("4x\n");

        pstDh9931Attr->enVoWorkMode = DHC_WORK_MODE_4Multiplex;

        /* 720p */
        if(0 == intRes)
        {
            printf("4x 720p 1280x720\n");

            for(j = 0; j < DHC_DH9931_MAX_CHNS; j++)
            {
                /* ����ͬ����Ƶ�¾�ģʽ */
                pstDh9931Attr->enCoaxialMode[j] = DHC_COAXIAL_MODE_NEW;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enNetraMode = DHC_NETRA_MODE_SINGLE;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnIdMode = DHC_CHN_ID_MODE_BOTH;

                if(0 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_0;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_YELLOW;
                }
                else if(1 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_1;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_CYAN;
                }
                else if(2 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_2;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_GREEN;
                }
                else if(3 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_3;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enProtocol = DHC_PROTOCOL_CVI;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enFreeRunFmt = DHC_CVI_1280x720_25HZ;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoClkEdge = DHC_VOCLK_EDGE_RISING;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoSdMode = DHC_VO_SD_MODE_960H;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoHdGmMode = DHC_VO_HD_GM_MODE_74M;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoMode = SetVoMode;

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enSdFmt = DHC_SD_FREERUN_FMT_P;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdSdSel = DHC_SD_FREERUN_HD;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdClk = DHC_SD_FREERUN_HD_74_25;
            }
        }
        else if(1 == intRes)
        {
            printf("4x 720p 1280x720  double \n");

            for(j = 0; j < DHC_DH9931_MAX_CHNS; j++)
            {
                /* ����ͬ����Ƶ�¾�ģʽ */
                pstDh9931Attr->enCoaxialMode[j] = DHC_COAXIAL_MODE_NEW;


                pstDh9931Attr->stVideoAttr[j].stVoCfg.enNetraMode = DHC_NETRA_MODE_SINGLE;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnIdMode = DHC_CHN_ID_MODE_BOTH;

                if(0 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_0;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_YELLOW;
                }
                else if(1 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_1;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_CYAN;
                }
                else if(2 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_2;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_GREEN;
                }
                else if(3 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_3;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enProtocol = DHC_PROTOCOL_CVI;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enFreeRunFmt = DHC_CVI_1280x720_25HZ;//DHC_CVI_1280x720_30HZ

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoClkEdge = DHC_VOCLK_EDGE_DUAL;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoSdMode = DHC_VO_SD_MODE_960H;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoHdGmMode = DHC_VO_HD_GM_MODE_74M;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoMode = SetVoMode;

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enSdFmt = DHC_SD_FREERUN_FMT_P;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdSdSel = DHC_SD_FREERUN_HD;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdClk = DHC_SD_FREERUN_HD_148_5;
            }
        }
        else if(2 == intRes)
        {
            printf("4x 1080p half 1920x1080 \n");

            for(j = 0; j < DHC_DH9931_MAX_CHNS; j++)
            {
                /* ����ͬ����Ƶ�¾�ģʽ */
                pstDh9931Attr->enCoaxialMode[j] = DHC_COAXIAL_MODE_NEW;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enNetraMode = DHC_NETRA_MODE_SINGLE;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnIdMode = DHC_CHN_ID_MODE_BOTH;

                if(0 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_0;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_YELLOW;
                }
                else if(1 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_1;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_1;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_CYAN;
                }
                else if(2 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_2;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_2;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_GREEN;
                }
                else if(3 == j)
                {
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enSecondInputChn = DHC_INPUT_CHN_0;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enFirstInputChn = DHC_INPUT_CHN_3;
                    pstDh9931Attr->stVideoAttr[j].stVoCfg.enChnId= DHC_CHN_ID_3;
                    pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enColor = DHC_FREERUN_COLOR_WHITE;
                }

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enProtocol = DHC_PROTOCOL_CVI;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enFreeRunFmt = DHC_CVI_1920x1080_25HZ;

                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoClkEdge = DHC_VOCLK_EDGE_DUAL;//DHC_VOCLK_EDGE_RISING
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoSdMode = DHC_VO_SD_MODE_960H;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoHdGmMode = DHC_VO_HD_GM_MODE_74M;
                pstDh9931Attr->stVideoAttr[j].stVoCfg.enVoMode = SetVoMode;

                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enSdFmt = DHC_SD_FREERUN_FMT_P;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdSdSel = DHC_SD_FREERUN_HD;
                pstDh9931Attr->stVideoAttr[j].stFreeRunCfg.enHdClk = DHC_SD_FREERUN_HD_148_5;
            }
        }
    }

    /* ������ƵCodec����Ƶ�� */
    pstDh9931Attr->stAudioAttr.enSampleRate = DHC_AUDIO_SAMPLERATE_8K;

    pstDh9931Attr->stAudioAttr.stConnectMode.u8CascadeNum = 0;
    pstDh9931Attr->stAudioAttr.stConnectMode.u8CascadeStage = 0;

    pstDh9931Attr->stAudioAttr.stEncCfg.bEncEn = DHC_TRUE;
    pstDh9931Attr->stAudioAttr.stEncCfg.enEncMode = DHC_ENC_MODE_MASTER;
    pstDh9931Attr->stAudioAttr.stEncCfg.bEncFreqEn = DHC_TRUE;
    pstDh9931Attr->stAudioAttr.stEncCfg.enSampleRate = DHC_AUDIO_SAMPLERATE_8K;
    pstDh9931Attr->stAudioAttr.stEncCfg.enSyncMode = DHC_AUDIO_SYNC_MODE_I2S;

    #if 1
    pstDh9931Attr->stAudioAttr.stEncCfg.enChnSel[0] = DHC_AUDIO_LINE_IN;
    pstDh9931Attr->stAudioAttr.stEncCfg.enChnSel[1] = DHC_AUDIO_LINE_IN;
    pstDh9931Attr->stAudioAttr.stEncCfg.enChnSel[2] = DHC_AUDIO_LINE_IN;
    pstDh9931Attr->stAudioAttr.stEncCfg.enChnSel[3] = DHC_AUDIO_LINE_IN;
    #else
    pstDh9931Attr->stAudioAttr.stEncCfg.enChnSel[0] = DHC_AUDIO_COAXIAL;
    pstDh9931Attr->stAudioAttr.stEncCfg.enChnSel[1] = DHC_AUDIO_COAXIAL;
    pstDh9931Attr->stAudioAttr.stEncCfg.enChnSel[2] = DHC_AUDIO_COAXIAL;
    pstDh9931Attr->stAudioAttr.stEncCfg.enChnSel[3] = DHC_AUDIO_COAXIAL;
    #endif

    pstDh9931Attr->stAudioAttr.stDecCfg.bDecEn = DHC_TRUE;
    pstDh9931Attr->stAudioAttr.stDecCfg.enSyncMode = DHC_AUDIO_SYNC_MODE_I2S;
    pstDh9931Attr->stAudioAttr.stDecCfg.enDecMode = DHC_DEC_MODE_MASTER;

    pstDh9931Attr->stAudioAttr.stDecCfg.u8OutputChnSel = 0x14;
}

/////////////////// sensor hardware dependent //////////////
static int Cus_poweron(MI_U8 u8ChipIndex, MI_U32 idx)//unused
{/*
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    sensor_if->SetIOPad(idx, CUS_SENIF_BUS_BT656, 0);

    sensor_if->Reset(idx, !handle->reset_POLARITY );

    SENSOR_UDELAY(2000);

    sensor_if->Reset(idx, handle->reset_POLARITY );
    SENSOR_UDELAY(20);
*/
    return 0;
}

static int pCus_poweroff(MI_U8 u8ChipIndex, MI_U32 idx)//unused
{
/*
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    printf("[%s] reset low\n", __FUNCTION__);
    sensor_if->Reset(idx, !handle->reset_POLARITY);
*/
    return 0;
}

int Cus_GetVideoStatus(MI_U8 u8ChipIndex, DHC_U8 u8Chn, DHC_DH9931_VIDEO_STATUS_S *pstVideoStatus)
{
    ss_cus_sensor_t *handle = NULL;

    DH9931_CHECK_POINTER(pstVideoStatus);
    DH9931_CHECK_PARA_RANGE(u8ChipIndex, 0, MAX_DH9931_NUM);

    handle = &gstSensorHandle[u8ChipIndex];
    if(DHC_ERRID_SUCCESS != DHC_DH9931_SDK_GetVideoStatus(handle->u8ADIndex, u8Chn, pstVideoStatus))
    {
        return -1;
    }

    return 0;
}

int Cus_GetVideoResNum(MI_U8 u8ChipIndex, MI_U32 *ulres_num)
{
    ss_cus_sensor_t *handle = NULL;

    DH9931_CHECK_POINTER(ulres_num);
    DH9931_CHECK_PARA_RANGE(u8ChipIndex, 0, MAX_DH9931_NUM);

    handle = &gstSensorHandle[u8ChipIndex];
    *ulres_num = handle->video_res_supported.num_res;
    return 0;
}

int Cus_GetVideoRes(MI_U8 u8ChipIndex, MI_U32 res_idx, cus_camsensor_res *res)
{
    ss_cus_sensor_t *handle = NULL;
    MI_U32 num_res = 0;

    DH9931_CHECK_POINTER(res);
    DH9931_CHECK_PARA_RANGE(u8ChipIndex, 0, MAX_DH9931_NUM);

    handle = &gstSensorHandle[u8ChipIndex];
    num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        return -1;
    }

    memcpy(res, &handle->video_res_supported.res[res_idx], sizeof(cus_camsensor_res));
    return 0;
}

int Cus_GetCurMultiplex(MI_U8 u8ChipIndex, CUS_BT656_MULTIPLEX_e *eMultiplex)
{
    ss_cus_sensor_t *handle = NULL;

    DH9931_CHECK_POINTER(eMultiplex);
    DH9931_CHECK_PARA_RANGE(u8ChipIndex, 0, MAX_DH9931_NUM);

    handle = &gstSensorHandle[u8ChipIndex];
    *eMultiplex = (CUS_BT656_MULTIPLEX_e)handle->stDh9931Attr.enVoWorkMode;

    return 0;
}

int Cus_GetCurVideoRes(MI_U8 u8ChipIndex, MI_U32 *cur_idx, cus_camsensor_res *res)
{
    ss_cus_sensor_t *handle = NULL;
    MI_U32 num_res = 0;

    DH9931_CHECK_POINTER(cur_idx);
    DH9931_CHECK_POINTER(res);
    DH9931_CHECK_PARA_RANGE(u8ChipIndex, 0, MAX_DH9931_NUM);

    handle = &gstSensorHandle[u8ChipIndex];
    num_res = handle->video_res_supported.num_res;

    *cur_idx = handle->video_res_supported.ulcur_res;

    if (*cur_idx >= num_res) {
        return -1;
    }

    res = &handle->video_res_supported.res[*cur_idx];

    return 0;
}

int Cus_SetHalfMode(MI_U8 u8ChipIndex, DHC_U8 u8Chn, DHC_BOOL bHalfEn, DHC_DH9931_HALFMODE_E enHalfMode)
{
    ss_cus_sensor_t *handle = NULL;

    DH9931_CHECK_PARA_RANGE(u8ChipIndex, 0, MAX_DH9931_NUM);
    handle = &gstSensorHandle[u8ChipIndex];

    if(DHC_ERRID_SUCCESS != DHC_DH9931_SDK_SetHalfMode(handle->u8ADIndex, u8Chn, bHalfEn, enHalfMode))
    {
        return -1;
    }

    return 0;
}
int Cus_SetInterface(MI_U8 u8ChipIndex, CUS_SENIF_BUS VoMode)
{
    ss_cus_sensor_t *handle = NULL;

    DH9931_CHECK_PARA_RANGE(u8ChipIndex, 0, MAX_DH9931_NUM);
    DH9931_CHECK_PARA_RANGE(VoMode, CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MAX);
    handle = &gstSensorHandle[u8ChipIndex];

    handle->sif_bus = VoMode;

    return 0;
}

int Cus_SetVideoRes(MI_U8 u8ChipIndex, MI_U32 res_idx)
{
    ss_cus_sensor_t *handle = NULL;
    MI_U32 num_res = 0;

    DH9931_CHECK_PARA_RANGE(u8ChipIndex, 0, MAX_DH9931_NUM);

    handle = &gstSensorHandle[u8ChipIndex];
    num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        printf("Not support res_idx %d\n", res_idx);
        return -1;
    }

    switch (res_idx) {
        case 0://720p, 1x
            handle->video_res_supported.ulcur_res = 0;
            handle->lib_res = 0;
            handle->multiplex = 1;
            handle->bt656_clkedge = CUS_BT656_CLK_EDGE_SINGLE_UP;
        break;
        case 1://1080p, 1x
            handle->video_res_supported.ulcur_res = 1;
            handle->lib_res = 1;
            handle->multiplex = 1;
            handle->bt656_clkedge = CUS_BT656_CLK_EDGE_SINGLE_UP;
        break;
        case 2://720p, 2x
            handle->video_res_supported.ulcur_res = 2;
            handle->lib_res = 0;
            handle->multiplex = 2;
            handle->bt656_clkedge = CUS_BT656_CLK_EDGE_DOUBLE;
        break;
        case 3://1080p, 2x
            handle->video_res_supported.ulcur_res = 3;
            handle->lib_res = 1;
            handle->multiplex = 2;
            handle->bt656_clkedge = CUS_BT656_CLK_EDGE_DOUBLE;
        break;
        case 4://720p, 4x
            handle->video_res_supported.ulcur_res = 4;
            handle->lib_res = 1;
            handle->multiplex = 4;
            handle->bt656_clkedge = CUS_BT656_CLK_EDGE_DOUBLE;
        break;
        case 5://1080n, 4x
            handle->video_res_supported.ulcur_res = 5;
            handle->lib_res = 2;
            handle->multiplex = 4;
            handle->bt656_clkedge = CUS_BT656_CLK_EDGE_DOUBLE;
        break;
        case 6://4M half, 1x
            handle->video_res_supported.ulcur_res = 6;
            handle->lib_res = 8;
            handle->multiplex = 1;
            handle->bt656_clkedge = CUS_BT656_CLK_EDGE_DOUBLE;
        break;
        case 7://4M , 1x
            handle->video_res_supported.ulcur_res = 7;
            handle->lib_res = 6;
            handle->multiplex = 1;
            handle->bt656_clkedge = CUS_BT656_CLK_EDGE_DOUBLE;
        break;
        case 8://8M , 1x
            handle->video_res_supported.ulcur_res = 8;
            handle->lib_res = 16;//3840x2160@15
            handle->multiplex = 1;
            handle->bt656_clkedge = CUS_BT656_CLK_EDGE_DOUBLE;
        break;
        default:
        break;
    }

    DHC_DH9931_Config_EachChip(u8ChipIndex, &handle->stDh9931Attr, handle->multiplex/2, handle->lib_res);
    handle->bUsed = TRUE;

    return 0;
}

int Cus_SnrEnable(void)
{
    MI_U8 i=0,j=0;
    DHC_DH9931_USR_CFG_S stDh9931UsrCfg;
    DHC_DH9931_VIDEO_COLOR_S stVideoColor;
    MI_U8 u8ADCnt=0;
    memset(&stDh9931UsrCfg, 0x0, sizeof(DHC_DH9931_USR_CFG_S));
    memset(&stVideoColor, 0x0, sizeof(DHC_DH9931_VIDEO_COLOR_S));

    for(i=0; i<MAX_DH9931_NUM; i++)
    {
        ss_cus_sensor_t *handle = &gstSensorHandle[i];
        if(handle->bUsed == TRUE)
        {
            memcpy(&stDh9931UsrCfg.stDH9931Attr[u8ADCnt], &handle->stDh9931Attr, sizeof(DHC_DH9931_ATTR_S));
            handle->u8ADIndex = u8ADCnt;
            printf("sensorhandle:%d -> dh9931 u8ADIndex:%d \n", i, handle->u8ADIndex);
            u8ADCnt++;
        }
    }

    DHC_DH9931_Config_Global(&stDh9931UsrCfg, u8ADCnt);

    for(i=0; i<MAX_DH9931_NUM; i++)
    {
        ss_cus_sensor_t *handle = &gstSensorHandle[i];

        if(handle->bUsed != TRUE)
            continue;

        if(0 == handle->multiplex/2)
        {
            if(4 == handle->lib_res)
            {
                //�������� 960H
                DHC_DH9931_test_WriteByte(handle->stDh9931Attr.enI2cBus, handle->stDh9931Attr.u8ChipAddr, 0x0800, 0x1e);
                DHC_DH9931_test_WriteByte(handle->stDh9931Attr.enI2cBus, handle->stDh9931Attr.u8ChipAddr, 0x0507, 0x54);
                DHC_DH9931_test_WriteByte(handle->stDh9931Attr.enI2cBus, handle->stDh9931Attr.u8ChipAddr, 0x0801, 0x2);

                DHC_DH9931_test_WriteByte(handle->stDh9931Attr.enI2cBus, handle->stDh9931Attr.u8ChipAddr, 0x1800, 0x1e);
                DHC_DH9931_test_WriteByte(handle->stDh9931Attr.enI2cBus, handle->stDh9931Attr.u8ChipAddr, 0x1507, 0x54);
                DHC_DH9931_test_WriteByte(handle->stDh9931Attr.enI2cBus, handle->stDh9931Attr.u8ChipAddr, 0x1801, 0x2);

                DHC_DH9931_test_WriteByte(handle->stDh9931Attr.enI2cBus, handle->stDh9931Attr.u8ChipAddr, 0x2800, 0x1e);
                DHC_DH9931_test_WriteByte(handle->stDh9931Attr.enI2cBus, handle->stDh9931Attr.u8ChipAddr, 0x2507, 0x54);
                DHC_DH9931_test_WriteByte(handle->stDh9931Attr.enI2cBus, handle->stDh9931Attr.u8ChipAddr, 0x2801, 0x2);

                DHC_DH9931_test_WriteByte(handle->stDh9931Attr.enI2cBus, handle->stDh9931Attr.u8ChipAddr, 0x3800, 0x1e);
                DHC_DH9931_test_WriteByte(handle->stDh9931Attr.enI2cBus, handle->stDh9931Attr.u8ChipAddr, 0x3507, 0x54);
                DHC_DH9931_test_WriteByte(handle->stDh9931Attr.enI2cBus, handle->stDh9931Attr.u8ChipAddr, 0x3801, 0x2);
            }
            else if(5 == handle->lib_res)
            {
                //720H
                DHC_DH9931_test_WriteByte(handle->stDh9931Attr.enI2cBus, handle->stDh9931Attr.u8ChipAddr, 0x0800, 0x1e);
                DHC_DH9931_test_WriteByte(handle->stDh9931Attr.enI2cBus, handle->stDh9931Attr.u8ChipAddr, 0x0507, 0x44);
                DHC_DH9931_test_WriteByte(handle->stDh9931Attr.enI2cBus, handle->stDh9931Attr.u8ChipAddr, 0x0801, 0x2);

                DHC_DH9931_test_WriteByte(handle->stDh9931Attr.enI2cBus, handle->stDh9931Attr.u8ChipAddr, 0x1800, 0x1e);
                DHC_DH9931_test_WriteByte(handle->stDh9931Attr.enI2cBus, handle->stDh9931Attr.u8ChipAddr, 0x1507, 0x44);
                DHC_DH9931_test_WriteByte(handle->stDh9931Attr.enI2cBus, handle->stDh9931Attr.u8ChipAddr, 0x1801, 0x2);

                DHC_DH9931_test_WriteByte(handle->stDh9931Attr.enI2cBus, handle->stDh9931Attr.u8ChipAddr, 0x2800, 0x1e);
                DHC_DH9931_test_WriteByte(handle->stDh9931Attr.enI2cBus, handle->stDh9931Attr.u8ChipAddr, 0x2507, 0x44);
                DHC_DH9931_test_WriteByte(handle->stDh9931Attr.enI2cBus, handle->stDh9931Attr.u8ChipAddr, 0x2801, 0x2);

                DHC_DH9931_test_WriteByte(handle->stDh9931Attr.enI2cBus, handle->stDh9931Attr.u8ChipAddr, 0x3800, 0x1e);
                DHC_DH9931_test_WriteByte(handle->stDh9931Attr.enI2cBus, handle->stDh9931Attr.u8ChipAddr, 0x3507, 0x44);
                DHC_DH9931_test_WriteByte(handle->stDh9931Attr.enI2cBus, handle->stDh9931Attr.u8ChipAddr, 0x3801, 0x2);
            }
        }

        if(handle->lib_res == 2 && handle->multiplex == 4) //4x 1920x1080 half
        {
            DHC_DH9931_SDK_SetHalfMode(handle->u8ADIndex, 0, 1, 1);
            DHC_DH9931_SDK_SetHalfMode(handle->u8ADIndex, 1, 1, 1);
            DHC_DH9931_SDK_SetHalfMode(handle->u8ADIndex, 2, 1, 1);
            DHC_DH9931_SDK_SetHalfMode(handle->u8ADIndex, 3, 1, 1);
        }
        else if(handle->lib_res == 8 && handle->multiplex == 1) //1x 4m(2560x1440) half
        {
            DHC_DH9931_SDK_SetHalfMode(handle->u8ADIndex, 0, 1, 2);
            DHC_DH9931_SDK_SetHalfMode(handle->u8ADIndex, 1, 1, 2);
            DHC_DH9931_SDK_SetHalfMode(handle->u8ADIndex, 2, 1, 2);
            DHC_DH9931_SDK_SetHalfMode(handle->u8ADIndex, 3, 1, 2);
        }

        for(j = 0; j < DHC_DH9931_MAX_CHNS; j++)
        {
            DHC_DH9931_SDK_SetCableType(handle->u8ADIndex, j, DHC_CABLE_TYPE_COAXIAL);

            stVideoColor.u8Brightness = 0x32;
            stVideoColor.u8Contrast = 0x32;
            stVideoColor.u8Hue = 0x32;
            stVideoColor.u8Saturation = 0x32;
            stVideoColor.u8Sharpness = 0x00;

            DHC_DH9931_SDK_SetColor(handle->u8ADIndex, j, &stVideoColor, DHC_SET_MODE_USER);

        }

        //DHC_DH9931_SDK_SetMsgLevel(4);

#if ((defined CONFIG_SIGMASTAR_CHIP_M6) && (CONFIG_SIGMASTAR_CHIP_M6 == 1))
        DHC_DH9931_test_WriteByte(handle->stDh9931Attr.enI2cBus, handle->stDh9931Attr.u8ChipAddr, 0x1802, 0x10);
#endif
        /*usleep(5000 * 1000);
        _dump_9931_reg(pstDh9931Attr->enI2cBus, pstDh9931Attr->u8ChipAddr);
        usleep(2000 * 1000);*/
    }
    return 0;
}

int Cus_SnrDisable(MI_U8 u8ChipIndex)
{
    DH9931_CHECK_PARA_RANGE(u8ChipIndex, 0, MAX_DH9931_NUM);

    if(gstSensorHandle[u8ChipIndex].bUsed == TRUE)
    {
        //Cus_poweroff();
        DHC_DH9931_SDK_DeInit();
        DHC_DH9931_DeInit(u8ChipIndex);
    }
    return 0;
}

int Cus_TransDH9931IntToMIInt(MI_U8 u8DH9931Intf, MI_U8 *pu8MIIntf)
{
    DH9931_CHECK_POINTER(pu8MIIntf);
    DH9931_CHECK_PARA_RANGE(u8DH9931Intf, CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MAX);

    switch(u8DH9931Intf)
    {
        case CUS_SENIF_BUS_BT656:
            *pu8MIIntf = E_MI_VIF_MODE_BT656;
            break;
        case CUS_SENIF_BUS_BT1120:
            *pu8MIIntf = E_MI_VIF_MODE_BT1120_STANDARD;
            break;
        default:
            printf("DH9931 interface %d err\n", u8DH9931Intf);
            return -1;
    }

    return 0;
}

int Cus_GetSnrPadInfo(MI_U8 u8ChipIndex, MI_SNR_PADInfo_t  *pstPadInfo)
{
    ss_cus_sensor_t *handle = NULL;
    MI_U8 u8IntfMode;

    DH9931_CHECK_POINTER(pstPadInfo);
    DH9931_CHECK_PARA_RANGE(u8ChipIndex, 0, MAX_DH9931_NUM);

    handle = &gstSensorHandle[u8ChipIndex];
    Cus_TransDH9931IntToMIInt(handle->sif_bus,&u8IntfMode);

    pstPadInfo->u32PlaneCount = 0;
    pstPadInfo->eIntfMode = (MI_SNR_IntfMode_e)u8IntfMode;
    pstPadInfo->eHDRMode = (MI_SNR_HDRType_e)E_MI_VIF_HDR_TYPE_OFF;
    pstPadInfo->unIntfAttr.stBt656Attr.eClkEdge = (MI_SNR_ClkEdge_e)handle->bt656_clkedge;

    return 0;
}

int Cus_GetSnrPlaneInfo(MI_U8 u8ChipIndex, MI_U32  u32PlaneID, MI_SNR_PlaneInfo_t *pstPlaneInfo)
{
    ss_cus_sensor_t *handle = NULL;
    cus_camsensor_res *pstCusRes = NULL;
    MI_U32 cur_idx = 0;

    DH9931_CHECK_POINTER(pstPlaneInfo);
    DH9931_CHECK_PARA_RANGE(u8ChipIndex, 0, MAX_DH9931_NUM);

    handle = &gstSensorHandle[u8ChipIndex];
    cur_idx = handle->video_res_supported.ulcur_res;
    pstCusRes = &handle->video_res_supported.res[cur_idx];

    pstPlaneInfo->u32PlaneID = 0;
    pstPlaneInfo->stCapRect.u16X = pstCusRes->stCropRect.u16X;
    pstPlaneInfo->stCapRect.u16Y = pstCusRes->stCropRect.u16Y;
    pstPlaneInfo->stCapRect.u16Width = pstCusRes->stCropRect.u16Width;
    pstPlaneInfo->stCapRect.u16Height = pstCusRes->stCropRect.u16Height;

    pstPlaneInfo->eBayerId = E_MI_SYS_PIXEL_BAYERID_MAX;
    pstPlaneInfo->ePixPrecision = E_MI_SYS_DATA_PRECISION_16BPP;
    pstPlaneInfo->ePixel = E_MI_SYS_PIXEL_FRAME_YUV422_UYVY;

    return 0;
}

int DHC_DH9931_Init(MI_U8       u8ChipIndex)
{
    int fd =0;
    MI_U8 I2cId=0;
    char I2cFilePath[64];
    ss_cus_sensor_t *handle = NULL;

    DH9931_CHECK_PARA_RANGE(u8ChipIndex, 0, MAX_DH9931_NUM);
    handle = &gstSensorHandle[u8ChipIndex];

    Cus_GetI2cId(u8ChipIndex, &I2cId);

    if(gstI2cInfo[I2cId].I2cFd == 0)
    {
        sprintf(I2cFilePath, "/dev/i2c-%d", I2cId);
        fd = open(I2cFilePath, O_RDWR);
        if (!fd)
        {
            printf("can not open file %s\n", I2cFilePath);
            return -1;
        }

        gstI2cInfo[I2cId].I2cFd = fd;
    }

    if (!handle) {
        printf("[%s] not enough memory!\n", __FUNCTION__);
        return -1;
    }

    handle->I2cId = I2cId;
    handle->sif_bus     = CUS_SENIF_BUS_BT656;//CUS_SENIF_BUS_PARL;
    handle->data_prec   = CUS_DATAPRECISION_16;  //CUS_DATAPRECISION_8;
    handle->bayer_id    = CUS_BAYER_MAX;   //CUS_BAYER_GB;
    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.num_res = 1;
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    handle->video_res_supported.res[0].u32MaxFps= 30;
    handle->video_res_supported.res[0].u32MinFps= 30;
    handle->video_res_supported.res[0].stCropRect.u16X= 0;
    handle->video_res_supported.res[0].stCropRect.u16Y= 0;
    handle->video_res_supported.res[0].stCropRect.u16Width = 1280;
    handle->video_res_supported.res[0].stCropRect.u16Height = 720;
    handle->video_res_supported.res[0].stOutputSize.u16Width = 1280;
    handle->video_res_supported.res[0].stOutputSize.u16Height = 720;
    sprintf(handle->video_res_supported.res[0].strResDesc, "1280x720@30fps[1x]");

    handle->video_res_supported.num_res = 2;
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    handle->video_res_supported.res[1].u32MaxFps= 30;
    handle->video_res_supported.res[1].u32MinFps= 30;
    handle->video_res_supported.res[1].stCropRect.u16X= 0;
    handle->video_res_supported.res[1].stCropRect.u16Y= 0;
    handle->video_res_supported.res[1].stCropRect.u16Width = 1920;
    handle->video_res_supported.res[1].stCropRect.u16Height = 1080;
    handle->video_res_supported.res[1].stOutputSize.u16Width = 1920;
    handle->video_res_supported.res[1].stOutputSize.u16Height = 1080;
    sprintf(handle->video_res_supported.res[1].strResDesc, "1920x1080@30fps[1x] single");

    handle->video_res_supported.num_res = 3;
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    handle->video_res_supported.res[2].u32MaxFps= 30;
    handle->video_res_supported.res[2].u32MinFps= 30;
    handle->video_res_supported.res[2].stCropRect.u16X= 0;
    handle->video_res_supported.res[2].stCropRect.u16Y= 0;
    handle->video_res_supported.res[2].stCropRect.u16Width = 1280;
    handle->video_res_supported.res[2].stCropRect.u16Height = 720;
    handle->video_res_supported.res[2].stOutputSize.u16Width = 1280;
    handle->video_res_supported.res[2].stOutputSize.u16Height = 720;
    sprintf(handle->video_res_supported.res[2].strResDesc, "1280x720@30fps[2x]");

    handle->video_res_supported.num_res = 4;
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    handle->video_res_supported.res[3].u32MaxFps= 30;
    handle->video_res_supported.res[3].u32MinFps= 30;
    handle->video_res_supported.res[3].stCropRect.u16X= 0;
    handle->video_res_supported.res[3].stCropRect.u16Y= 0;
    handle->video_res_supported.res[3].stCropRect.u16Width = 1920;
    handle->video_res_supported.res[3].stCropRect.u16Height = 1080;
    handle->video_res_supported.res[3].stOutputSize.u16Width = 1920;
    handle->video_res_supported.res[3].stOutputSize.u16Height = 1080;
    sprintf(handle->video_res_supported.res[3].strResDesc, "1920x1080@30fps[2x]");

    handle->video_res_supported.num_res = 5;
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    handle->video_res_supported.res[4].u32MaxFps= 30;
    handle->video_res_supported.res[4].u32MinFps= 30;
    handle->video_res_supported.res[4].stCropRect.u16X= 0;
    handle->video_res_supported.res[4].stCropRect.u16Y= 0;
    handle->video_res_supported.res[4].stCropRect.u16Width = 1280;
    handle->video_res_supported.res[4].stCropRect.u16Height = 720;
    handle->video_res_supported.res[4].stOutputSize.u16Width = 1280;
    handle->video_res_supported.res[4].stOutputSize.u16Height = 720;
    sprintf(handle->video_res_supported.res[4].strResDesc, "1280x720@30fps[4x]");

    handle->video_res_supported.num_res = 6;
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    handle->video_res_supported.res[5].u32MaxFps= 30;
    handle->video_res_supported.res[5].u32MinFps= 30;
    handle->video_res_supported.res[5].stCropRect.u16X= 0;
    handle->video_res_supported.res[5].stCropRect.u16Y= 0;
    handle->video_res_supported.res[5].stCropRect.u16Width = 960;
    handle->video_res_supported.res[5].stCropRect.u16Height = 1080;
    handle->video_res_supported.res[5].stOutputSize.u16Width = 960;
    handle->video_res_supported.res[5].stOutputSize.u16Height = 1080;
    sprintf(handle->video_res_supported.res[5].strResDesc, "960x1080@30fps[4x]");

    handle->video_res_supported.num_res = 7;
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    handle->video_res_supported.res[6].u32MaxFps= 30;
    handle->video_res_supported.res[6].u32MinFps= 30;
    handle->video_res_supported.res[6].stCropRect.u16X= 0;
    handle->video_res_supported.res[6].stCropRect.u16Y= 0;
    handle->video_res_supported.res[6].stCropRect.u16Width = 1280;
    handle->video_res_supported.res[6].stCropRect.u16Height = 1440;
    handle->video_res_supported.res[6].stOutputSize.u16Width = 1280;
    handle->video_res_supported.res[6].stOutputSize.u16Height = 1440;
    sprintf(handle->video_res_supported.res[6].strResDesc, "1280x1440@30fps[1x] double");

    handle->video_res_supported.num_res = 8;
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    handle->video_res_supported.res[7].u32MaxFps= 30;
    handle->video_res_supported.res[7].u32MinFps= 30;
    handle->video_res_supported.res[7].stCropRect.u16X= 0;
    handle->video_res_supported.res[7].stCropRect.u16Y= 0;
    handle->video_res_supported.res[7].stCropRect.u16Width = 2560;
    handle->video_res_supported.res[7].stCropRect.u16Height = 1440;
    handle->video_res_supported.res[7].stOutputSize.u16Width = 2560;
    handle->video_res_supported.res[7].stOutputSize.u16Height = 1440;
    sprintf(handle->video_res_supported.res[7].strResDesc, "2560x1440@30fps[1x] double");

    handle->video_res_supported.num_res = 9;
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    handle->video_res_supported.res[8].u32MaxFps= 15;
    handle->video_res_supported.res[8].u32MinFps= 15;
    handle->video_res_supported.res[8].stCropRect.u16X= 0;
    handle->video_res_supported.res[8].stCropRect.u16Y= 0;
    handle->video_res_supported.res[8].stCropRect.u16Width = 3840;
    handle->video_res_supported.res[8].stCropRect.u16Height = 2160;
    handle->video_res_supported.res[8].stOutputSize.u16Width = 3840;
    handle->video_res_supported.res[8].stOutputSize.u16Height = 2160;
    sprintf(handle->video_res_supported.res[8].strResDesc, "3840x2160@15fps[1x] double");

    return 0;
}

int DHC_DH9931_DeInit(MI_U8 u8ChipIndex)
{
    DH9931_CHECK_PARA_RANGE(u8ChipIndex, 0, MAX_DH9931_NUM);

    close(gstI2cInfo[gstSensorHandle[u8ChipIndex].I2cId].I2cFd);
    memset(&gstI2cInfo[gstSensorHandle[u8ChipIndex].I2cId], 0x0, sizeof(Cus_I2cInfo_t));
    memset(&gstSensorHandle[u8ChipIndex], 0x0, sizeof(ss_cus_sensor_t));

    return 0;
}
