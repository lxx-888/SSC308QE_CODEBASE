/*
 * hal_iic.c- Sigmastar
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

#include "hal_iic.h"
#include "hal_iic_reg.h"

extern void udelay(unsigned long usec);
extern int  printf(const char *fmt, ...);

#ifdef CONFIG_ARM64
#define READ_WORD(_reg)        (*(volatile U16 *)(phys_addr_t)(_reg))
#define WRITE_WORD(_reg, _val) ((*(volatile U16 *)(phys_addr_t)(_reg)) = (U16)(_val))
#define WRITE_WORD_MASK(_reg, _val, _mask)    \
    ((*(volatile U16 *)(phys_addr_t)(_reg)) = \
         ((*(volatile U16 *)(phys_addr_t)(_reg)) & ~(_mask)) | ((U16)(_val) & (_mask)))
#else
#define READ_WORD(_reg)        (*(volatile U16 *)(U32)(_reg))
#define WRITE_WORD(_reg, _val) ((*(volatile U16 *)(U32)(_reg)) = (U16)(_val))
#define WRITE_WORD_MASK(_reg, _val, _mask) \
    ((*(volatile U16 *)(U32)(_reg)) = ((*(volatile U16 *)(U32)(_reg)) & ~(_mask)) | ((U16)(_val) & (_mask)))
#endif

#define I2C_READ_REG(reg)                  READ_WORD(para_i2c_base->BankBase + ((reg) << 2))
#define I2C_WRITE_REG(reg, val)            WRITE_WORD(para_i2c_base->BankBase + ((reg) << 2), (val))
#define I2C_WRITE_REG_MASK(reg, val, mask) WRITE_WORD_MASK(para_i2c_base->BankBase + ((reg) << 2), val, mask)

#define IIC_MIU0_BUS_BASE (0x20000000)
#define IIC_MIU1_BUS_BASE (0x40000000)

#define NUMBER_TRANSFER_RETRY (2000)
#define DELAY_N_US(x) \
    do                \
    {                 \
        udelay(x);    \
    } while (0)

//#define DMSG_I2C_HAL_DEBUG
#define dmsg_i2c_halerr(fmt, ...)                                    \
    do                                                               \
    {                                                                \
        printf("[hal_i2c_err] <%s>, " fmt, __func__, ##__VA_ARGS__); \
    } while (0)
#ifdef DMSG_I2C_HAL_DEBUG
#define dmsg_i2c_halwarn(fmt, ...)                                    \
    do                                                                \
    {                                                                 \
        printf("[hal_i2c_warn] <%s>, " fmt, __func__, ##__VA_ARGS__); \
    } while (0)
#else
#define dmsg_i2c_halwarn(fmt, ...)
#endif

/*padmux register addr & mask*/
#define ADDR_BASE_PADMUX (0x1F207800)
const U16 au16PadmuxMask[]      = {0x0007, 0x00F0, 0x0007, 0x0007, 0x0070, 0x000F};
const U16 au16PadmuxRegOffset[] = {0x006F, 0x006F, 0x0070, 0x0073, 0x0074};

#define PADMUX_WRITE_REG_MASK(reg, val, mask) WRITE_WORD_MASK(ADDR_BASE_PADMUX + ((reg) << 2), val, mask)

/***************************************************************************************/
/*****************************    function definition    *******************************/
/***************************************************************************************/

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_GetRegFlag(struct _hal_iic *para_i2c_base)
{
    S32 s32Ret;

    s32Ret = I2C_READ_REG(REG_I2C_INT_FLAG);
    s32Ret = s32Ret & REG_MASK_INT_FLAG;

    return s32Ret;
}
S32 HAL_I2C_ClearInt(struct _hal_iic *para_i2c_base)
{
    I2C_WRITE_REG_MASK(REG_I2C_INT_FLAG, true, REG_MASK_INT_FLAG);
    while (HAL_I2C_GetRegFlag(para_i2c_base))
        ;

    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static inline S32 HAL_I2C_DetDmaTrDone(struct _hal_iic *para_i2c_base)
{
    S32 s32Ret;

    s32Ret = I2C_READ_REG(REG_I2C_DMA_TRANSFER_DONE);
    s32Ret &= REG_MASK_DMA_TRSFER_DONE;

    return s32Ret;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_RstSet(struct _hal_iic *para_i2c_base, BOOL para_rst)
{
    I2C_WRITE_REG_MASK(REG_I2C_CONFIG, para_rst, REG_MASK_CFG_RST);

    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_DmaCfgIntSet(struct _hal_iic *para_i2c_base, BOOL para_disenable)
{
    I2C_WRITE_REG_MASK(REG_I2C_DMA_CONFIG, para_disenable << SHIFT_BIT2, REG_MASK_DMACFG_INTFLAG);

    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_DmaCfgSofRstSet(struct _hal_iic *para_i2c_base, BOOL para_disenable)
{
    if (para_disenable)
    {
        I2C_WRITE_REG_MASK(REG_I2C_DMA_CONFIG, false, REG_MASK_DMACFG_SOFTRST);
    }
    else
    {
        I2C_WRITE_REG_MASK(REG_I2C_DMA_CONFIG, true << SHIFT_BIT1, REG_MASK_DMACFG_SOFTRST);
    }
    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_DmaCfgMiuRst(struct _hal_iic *para_i2c_base, BOOL para_disenable)
{
    I2C_WRITE_REG_MASK(REG_I2C_DMA_CONFIG, para_disenable << SHIFT_BIT3, REG_MASK_DMACFG_MIURST);

    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_DmaAdrModSet(struct _hal_iic *para_i2c_base, const EN_HAL_DMA_ADRMODE para_adr_mode)
{
    I2C_WRITE_REG_MASK(REG_I2C_DMA_10BITMODE, para_adr_mode << SHIFT_BIT10, REG_MASK_DMABITMODE);

    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_DmaMiuPriSet(struct _hal_iic *para_i2c_base, const EN_HAL_DMA_MIUPRIORITY para_miu_pri)
{
    I2C_WRITE_REG_MASK(REG_I2C_DMA_CONFIG, para_miu_pri << SHIFT_BIT4, REG_MASK_DMACFG_MIUPRI);

    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_DmaMiuChnalSet(struct _hal_iic *para_i2c_base, const EN_HAL_DMA_MIUCHANNEL para_miu_chanl)
{
    I2C_WRITE_REG_MASK(REG_I2C_DMA_MIUCHANL_SEL, para_miu_chanl << SHIFT_BIT7, REG_MASK_DMA_MIUCHANLSEL);

    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_DmaMiuAddrSet(struct _hal_iic *para_i2c_base, const U64 para_miu_addr)
{
    I2C_WRITE_REG(REG_I2C_BUFDRAMADDR_LOW, (U16)(para_miu_addr & 0x0000FFFF));
    I2C_WRITE_REG(REG_I2C_BUFDRAMADDR_HIGH, ((U16)((para_miu_addr & 0xFFFF0000) >> 16)));
    I2C_WRITE_REG_MASK(REG_I2C_BUFDRAMADDR_MSB, ((U16)((para_miu_addr & 0xF00000000) >> 32)), REG_MASK_IIC_ADDR_MSB);

    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static U32 HAL_I2C_GetDmaTc(struct _hal_iic *para_i2c_base)
{
    U32 u32Ret;
    u32Ret = I2C_READ_REG(REG_I2C_DMA_TC_2CH);
    u32Ret |= (I2C_READ_REG(REG_I2C_DMA_TC_2DH) << 16);
    return u32Ret;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_DmaTrigger(struct _hal_iic *para_i2c_base)
{
    // U32 u32Timeout;

    I2C_WRITE_REG_MASK(REG_I2C_DMA_TRIGGER, true, REG_MASK_DMATRIG);

    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_ForStpCntSet(struct _hal_iic *para_i2c_base, U16 para_cnt)
{
    I2C_WRITE_REG(REG_I2C_CNT_FOR_STOP, para_cnt);

    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_HiPerCntSet(struct _hal_iic *para_i2c_base, U16 para_cnt)
{
    I2C_WRITE_REG(REG_I2C_CNT_HIGH_PERIOD, para_cnt);

    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_LowPerCntSet(struct _hal_iic *para_i2c_base, U16 para_cnt)
{
    I2C_WRITE_REG(REG_I2C_CNT_LOW_PERIOD, para_cnt);

    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_BtwnFlEdgCntSet(struct _hal_iic *para_i2c_base, U16 para_cnt)
{
    I2C_WRITE_REG(REG_I2C_CNT_BTWN_FALEDGE, para_cnt);

    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_ForStarCntSet(struct _hal_iic *para_i2c_base, U16 para_cnt)
{
    I2C_WRITE_REG(REG_I2C_CNT_FOR_START, para_cnt);

    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_LtchTimCntSet(struct _hal_iic *para_i2c_base, U16 para_cnt)
{
    I2C_WRITE_REG(REG_I2C_CNT_DATA_LATCHTIM, para_cnt);

    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_TimoutOcrCntSet(struct _hal_iic *para_i2c_base, U16 para_cnt)
{
    I2C_WRITE_REG(REG_I2C_CNT_DELYTIMOUT_INTOCUR, para_cnt);

    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
S32 HAL_I2C_DmaTrDone(struct _hal_iic *para_i2c_base, BOOL para_boolval)
{
    S32 s32Ret;
    s32Ret = HAL_I2C_DetDmaTrDone(para_i2c_base);
    if (s32Ret)
    {
        I2C_WRITE_REG_MASK(REG_I2C_DMA_TRANSFER_DONE, para_boolval, REG_MASK_DMA_TRSFER_DONE);
        return 1;
    }
    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_SetCmdData(struct _hal_iic *para_i2c_base, U16 para_data)
{
    I2C_WRITE_REG(REG_I2C_DMA_CMD_DATA_25H, para_data);

    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:para_boolwr = 1/READ; para_boolwr = 0/WRITE
//*return:
/****************************************************/
static S32 HAL_I2C_SetCmdLen(struct _hal_iic *para_i2c_base, U16 para_len)
{
    I2C_WRITE_REG_MASK(REG_I2C_DMA_CMD_LEN_29H, para_len, REG_MASK_DMA_CMDLEN);

    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:para_boolwr = 1/READ; para_boolwr = 0/WRITE
//*return:
/****************************************************/
static S32 HAL_I2C_SetDmaDataLen(struct _hal_iic *para_i2c_base, U32 para_len)
{
    I2C_WRITE_REG(REG_I2C_TRANS_DATA_LEN_LOW, (U16)(para_len & 0x0000FFFF));
    dmsg_i2c_halwarn("set data len : %d\n", para_len);
    I2C_WRITE_REG(REG_I2C_TRANS_DATA_LEN_HIGH, (U16)((para_len & 0xFFFF0000) >> 16));

    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:para_boolwr = 1/READ; para_boolwr = 0/WRITE
//*return:
/****************************************************/
static S32 HAL_I2C_TrsfmtSet(struct _hal_iic *para_i2c_base, BOOL para_boolwr)
{
    I2C_WRITE_REG_MASK(REG_I2C_RDWR_FORMAT, para_boolwr << SHIFT_BIT6, REG_MASK_RDWR_FMT);

    return 0;
}

s32 HAL_I2C_DmaStopFmt(struct _hal_iic *para_i2c_base, BOOL para_stop)
{
    if (para_stop)
    {
        I2C_WRITE_REG_MASK(REG_I2C_STOP_FORMAT_DISABLE, 0 << 5, REG_MASK_STPFMT_DIS);
    }
    else
    {
        I2C_WRITE_REG_MASK(REG_I2C_STOP_FORMAT_DISABLE, 1 << 5, REG_MASK_STPFMT_DIS);
    }
    return 0;
}
/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_DmaSlvAddrSet(struct _hal_iic *para_i2c_base, U16 para_slvadr)
{
    if (E_DMA_ADDRMODE_10BIT == para_i2c_base->stI2cDmaCtrl.enDmaAdrMod)
    {
        I2C_WRITE_REG_MASK(REG_I2C_DMA_SLAVEADDR, para_slvadr & REG_MASK_DMASLVADDR_10BIT, REG_MASK_DMASLVADDR_10BIT);
    }
    else
    {
        I2C_WRITE_REG_MASK(REG_I2C_DMA_SLAVEADDR, para_slvadr & REG_MASK_DMASLVADDR_NORM, REG_MASK_DMASLVADDR_10BIT);
    }

    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_TranCmdStart(struct _hal_iic *para_i2c_base)
{
    I2C_WRITE_REG_MASK(REG_I2C_STAR_STOP, true, REG_MASK_CMD_START);

    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_TranCmdStop(struct _hal_iic *para_i2c_base)
{
    I2C_WRITE_REG_MASK(REG_I2C_STAR_STOP, true << SHIFT_BIT8, REG_MASK_CMD_STOP);

    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static inline S32 HAL_I2C_DetStartInt(struct _hal_iic *para_i2c_base)
{
    S32 s32Ret;

    s32Ret = I2C_READ_REG(REG_I2C_INT_STATUS) & REG_MASK_INTSTU_START;
    s32Ret = s32Ret >> SHIFT_BIT8;

    return s32Ret;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static inline S32 HAL_I2C_DetStopInt(struct _hal_iic *para_i2c_base)
{
    S32 s32Ret;

    s32Ret = I2C_READ_REG(REG_I2C_INT_STATUS) & REG_MASK_INTSTU_STOP;
    s32Ret = s32Ret >> SHIFT_BIT9;

    return s32Ret;
}

static inline S32 HAL_I2C_DetIdle(struct _hal_iic *para_i2c_base)
{
    S32 s32Ret;

    s32Ret = I2C_READ_REG(REG_I2C_INT_STATUS) & REG_MASK_INTSTU_STATE;

    return s32Ret;
}
/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static inline S32 HAL_I2C_DetRxDoneInt(struct _hal_iic *para_i2c_base)
{
    S32 s32Ret;

    s32Ret = I2C_READ_REG(REG_I2C_INT_STATUS) & REG_MASK_INTSTU_RXDONE;
    s32Ret = s32Ret >> SHIFT_BIT10;

    return s32Ret;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static inline S32 HAL_I2C_DetTxDoneInt(struct _hal_iic *para_i2c_base)
{
    S32 s32Ret;

    s32Ret = I2C_READ_REG(REG_I2C_INT_STATUS) & REG_MASK_INTSTU_TXDONE;
    s32Ret = s32Ret >> SHIFT_BIT11;

    return s32Ret;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static inline U16 HAL_I2C_GetSlvAck(struct _hal_iic *para_i2c_base)
{
    return (I2C_READ_REG(REG_I2C_WRITE_DATA) & REG_MASK_WRITE_ACK_FRMSLV);
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_ReadTrigger(struct _hal_iic *para_i2c_base)
{
    I2C_WRITE_REG_MASK(REG_I2C_READ_DATA, true << SHIFT_BIT8, REG_MASK_READ_DATA_TRIGGER);

    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_SendByte(struct _hal_iic *para_i2c_base, const u8 para_val)
{
    S32 s32Ret = 0;
    U16 u16Retry;

    I2C_WRITE_REG_MASK(REG_I2C_WRITE_DATA, para_val, REG_MASK_WRITE_DATA);
    for (u16Retry = 0; u16Retry < NUMBER_TRANSFER_RETRY; u16Retry++)
    {
        // DELAY_N_US(2);
        s32Ret |= (HAL_I2C_GetRegFlag(para_i2c_base) && HAL_I2C_DetTxDoneInt(para_i2c_base));
        if (s32Ret)
        {
            s32Ret |= HAL_I2C_ClearInt(para_i2c_base);
            u16Retry = 0;
            break;
        }
    }
    if (u16Retry >= NUMBER_TRANSFER_RETRY)
    {
        dmsg_i2c_halerr("err tx done timeout\n");
        return -E_ERR_RETRY;
    }
    for (u16Retry = 0; u16Retry < NUMBER_TRANSFER_RETRY; u16Retry++) // ask for ack
    {
        if (!HAL_I2C_GetSlvAck(para_i2c_base))
        {
            dmsg_i2c_halwarn("get an ack\n");
            return 0;
        }
    }
    dmsg_i2c_halerr("no ack!\n");
    return -E_ERR_RETRY;
}
/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_SendBuf(struct _hal_iic *para_i2c_base, u8 *para_pdata, U32 para_length)
{
    S32 s32Ret = 0;
    U16 i;
    for (i = 0; i < para_length; i++)
    {
        s32Ret |= HAL_I2C_SendByte(para_i2c_base, *(para_pdata + i));
        if (s32Ret < 0) // send byte
        {
            return s32Ret;
        }
    }

    return 0;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static int HAL_I2C_RecvByte(struct _hal_iic *para_i2c_base, u8 *Readbuf)
{
    U16 u16Retry;
    S32 s32Ret = 0;

    s32Ret |= HAL_I2C_ReadTrigger(para_i2c_base);

    for (u16Retry = 0; u16Retry < NUMBER_TRANSFER_RETRY; u16Retry++)
    {
        s32Ret |= (HAL_I2C_GetRegFlag(para_i2c_base) && HAL_I2C_DetRxDoneInt(para_i2c_base));
        if (s32Ret)
        {
            HAL_I2C_ClearInt(para_i2c_base);
            u16Retry = 0;
            break;
        }
    }
    if (u16Retry >= NUMBER_TRANSFER_RETRY)
    {
        dmsg_i2c_halerr("recv byte timeout err\n");
        s32Ret = -E_ERR_RETRY;
        goto OUT;
    }
    *Readbuf = (u8)(I2C_READ_REG(REG_I2C_READ_DATA));
    return 0;
OUT:
    HAL_I2C_ClearInt(para_i2c_base);
    return s32Ret;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return: recv byte number
/****************************************************/
static S32 HAL_I2C_RecvBuf(struct _hal_iic *para_i2c_base, u8 *para_pdata, U32 para_length)
{
    U16 i;

    for (i = 0; i < para_length; i++)
    {
        if (i == (para_length - 1))
        {
            dmsg_i2c_halwarn("return NAK\n");
            I2C_WRITE_REG_MASK(REG_I2C_READ_DATA, true << SHIFT_BIT9,
                               REG_MASK_READ_ACK_TOSLV); // don't send ack when receive last byte
        }
        else
        {
            I2C_WRITE_REG_MASK(REG_I2C_READ_DATA, false << SHIFT_BIT9, REG_MASK_READ_ACK_TOSLV);
            dmsg_i2c_halwarn("return ACK\n");
        }
        if (HAL_I2C_RecvByte(para_i2c_base, (para_pdata + i)))
        {
            dmsg_i2c_halerr("RECV BUF ERR\n");
            break;
        }
    }
    return i;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_DmaWrite(struct _hal_iic *para_i2c_base, U16 para_slvadr, u8 *para_pdata, U32 para_len)
{
    S32 s32Ret = 0;
    u8 *pu8DmaViraddr;
    U32 u32DmaTc;
    U32 u32WaitDmaDone = 10000000;

    // set read/write cmd
    s32Ret |= HAL_I2C_TrsfmtSet(para_i2c_base, false); // set write
    // set slave addr
    s32Ret |= HAL_I2C_DmaSlvAddrSet(para_i2c_base, para_slvadr); // set dma slave address
    // set cmd data
    s32Ret |= HAL_I2C_SetCmdData(para_i2c_base, 0x0000);
    // set cmd len
    s32Ret |= HAL_I2C_SetCmdLen(para_i2c_base, 0x0);
    // set miu addr
    /*done in HAL_I2C_DmaSetup*/
    // set data length
    s32Ret |= HAL_I2C_SetDmaDataLen(para_i2c_base, para_len);
    // reset and reset miu
    s32Ret |= HAL_I2C_RstSet(para_i2c_base, true);
    s32Ret |= HAL_I2C_RstSet(para_i2c_base, false);
    s32Ret |= HAL_I2C_DmaCfgMiuRst(para_i2c_base, true);
    s32Ret |= HAL_I2C_DmaCfgMiuRst(para_i2c_base, false);
    // copy data to miu addr
    pu8DmaViraddr = para_i2c_base->stI2cDmaCtrl.stDmaMiuAdr.pu8DmaAdrVirtu;
    memcpy(pu8DmaViraddr, para_pdata, para_len);
    //  set enable dma interrupt
    if (para_i2c_base->bDmaDetctMod)
    {
        s32Ret |= HAL_I2C_DmaCfgIntSet(para_i2c_base, true);
    }
    // set clear done
    s32Ret |= HAL_I2C_DmaTrDone(para_i2c_base, true);

    // set trigger
    s32Ret |= HAL_I2C_DmaTrigger(para_i2c_base);

    if (para_i2c_base->bDmaDetctMod)
    {
    } // if (para_i2c_base->bDmaDetctMod)
    else
    {
        while (u32WaitDmaDone--)
        {
            s32Ret = HAL_I2C_DetDmaTrDone(para_i2c_base);
            if (s32Ret)
            {
                HAL_I2C_DmaTrDone(para_i2c_base, true);
                break;
            }
            if (1 == u32WaitDmaDone)
            {
                dmsg_i2c_halerr("DMA WRITE POLLING TIMEOUT!\n");
                return -E_ERR_TIMEOUT;
            }
        }
    } // end  if (para_i2c_base->bDmaDetctMod)

    u32DmaTc = HAL_I2C_GetDmaTc(para_i2c_base);
    if (u32DmaTc != para_len)
    {
        dmsg_i2c_halerr("err dma WRITE transform len:%d != para_len:%d\n", u32DmaTc, para_len);
        return -E_ERR_WRITE;
    }
    dmsg_i2c_halwarn("DMA WRITE SUCCESS DONE\n");
    return 0;
} // end HAL_I2C_DmaWrite()
static S32 HAL_I2C_NormalWrite(struct _hal_iic *para_i2c_base, U16 para_slvadr, u8 *para_pdata, U32 para_len)
{
    S32 s32Ret = 0;
    U16 u16Retry;
    // reset iic
    s32Ret |= HAL_I2C_RstSet(para_i2c_base, true);
    s32Ret |= HAL_I2C_RstSet(para_i2c_base, false);
    // 1.send start
    s32Ret |= HAL_I2C_TranCmdStart(para_i2c_base); // do we need clear start bit??
    for (u16Retry = 0; u16Retry < NUMBER_TRANSFER_RETRY; u16Retry++)
    {
        if (HAL_I2C_GetRegFlag(para_i2c_base) && HAL_I2C_DetStartInt(para_i2c_base))
        {
            HAL_I2C_ClearInt(para_i2c_base);
            u16Retry = 0;
            break;
        }
    }
    if (u16Retry >= NUMBER_TRANSFER_RETRY)
    {
        s32Ret = -E_ERR_RETRY;
        dmsg_i2c_halerr("ERR NUM:%d; write start err!\n", s32Ret);
        goto err_rtn;
    }

    // 2.send slave addr << 1 | w_bit
    s32Ret |= HAL_I2C_SendByte(para_i2c_base, (u8)(para_slvadr & 0xFF));
    if (s32Ret < 0)
    {
        dmsg_i2c_halerr("ERR NUM:%d; send slave addr|write = 0x%04x; NACK from slave\n", s32Ret, para_slvadr);
        goto err_rtn;
    }

    // 3.send data
    s32Ret |= HAL_I2C_SendBuf(para_i2c_base, para_pdata, para_len);
    if (s32Ret < 0)
    {
        dmsg_i2c_halerr("ERR NUM:%d; write send data err\n", s32Ret);
        goto err_rtn;
    }
    dmsg_i2c_halwarn("FIFO MODE WRITE SUCCESS DONE\n");
    return 0;
err_rtn:
    return s32Ret;
}
/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_DmaRead(struct _hal_iic *para_i2c_base, U16 para_slvadr, u8 *para_pdata, U32 para_len)
{
    S32 s32Ret = 0;
    u8 *pu8DmaViraddr;
    U32 u32DmaTc;
    U32 u32WaitDmaDone = 10000000;

    pu8DmaViraddr = para_i2c_base->stI2cDmaCtrl.stDmaMiuAdr.pu8DmaAdrVirtu;

    // set read/write cmd
    s32Ret |= HAL_I2C_TrsfmtSet(para_i2c_base, true); // set read
    // set slave addr
    s32Ret |= HAL_I2C_DmaSlvAddrSet(para_i2c_base, para_slvadr); // set dma slave address
    // set cmd data
    s32Ret |= HAL_I2C_SetCmdData(para_i2c_base, 0x0000);
    // set cmd len
    s32Ret |= HAL_I2C_SetCmdLen(para_i2c_base, 0x00);
    // set miu addr
    /*done in HAL_I2C_DmaSetup*/
    // set data length
    s32Ret |= HAL_I2C_SetDmaDataLen(para_i2c_base, para_len);
    // reset and reset miu
    s32Ret |= HAL_I2C_RstSet(para_i2c_base, true);
    s32Ret |= HAL_I2C_RstSet(para_i2c_base, false);
    s32Ret |= HAL_I2C_DmaCfgMiuRst(para_i2c_base, true);
    s32Ret |= HAL_I2C_DmaCfgMiuRst(para_i2c_base, false);
    // set enable dma interrupt
    if (para_i2c_base->bDmaDetctMod)
    {
        s32Ret |= HAL_I2C_DmaCfgIntSet(para_i2c_base, true);
    }
    // set clear done
    s32Ret |= HAL_I2C_DmaTrDone(para_i2c_base, true);

    // set trigger
    s32Ret |= HAL_I2C_DmaTrigger(para_i2c_base);

    if (para_i2c_base->bDmaDetctMod)
    {
    }
    else
    {
        while (u32WaitDmaDone--)
        {
            s32Ret = HAL_I2C_DetDmaTrDone(para_i2c_base);
            if (s32Ret)
            {
                HAL_I2C_DmaTrDone(para_i2c_base, true);
                break;
            }
            if (1 == u32WaitDmaDone)
            {
                dmsg_i2c_halerr("DMA READ POLLING TIMEOUT!\n");
                return -E_ERR_TIMEOUT;
            }
        }
    } // end  if (para_i2c_base->bDmaDetctMod)

    u32DmaTc = HAL_I2C_GetDmaTc(para_i2c_base);
    if (u32DmaTc != para_len)
    {
        dmsg_i2c_halerr("err dma READ transform len:%d != para_len:%d\n", u32DmaTc, para_len);
        return -E_ERR_READ;
    }
    memcpy(para_pdata, pu8DmaViraddr, para_len);
    dmsg_i2c_halwarn("DMA READ SUCCESS DONE\n");
    return 0;
}
static S32 HAL_I2C_NormalRead(struct _hal_iic *para_i2c_base, U16 para_slvadr, u8 *para_pdata, U32 para_len)
{
    S32 s32Ret = 0;
    U16 u16Retry;

    // reset iic
    s32Ret |= HAL_I2C_RstSet(para_i2c_base, true);
    s32Ret |= HAL_I2C_RstSet(para_i2c_base, false);
    // 1.send start
    s32Ret |= HAL_I2C_TranCmdStart(para_i2c_base);
    for (u16Retry = 0; u16Retry < NUMBER_TRANSFER_RETRY; u16Retry++)
    {
        if (HAL_I2C_GetRegFlag(para_i2c_base) && HAL_I2C_DetStartInt(para_i2c_base))
        {
            HAL_I2C_ClearInt(para_i2c_base);
            u16Retry = 0;
            break;
        }
    }
    if (u16Retry >= NUMBER_TRANSFER_RETRY)
    {
        s32Ret = -E_ERR_RETRY;
        dmsg_i2c_halerr("ERR NUM:%d; start err!\n", s32Ret);
        goto err_rtn;
    }

    // 2.send slave addr <<  1| r_bit
    s32Ret |= HAL_I2C_SendByte(para_i2c_base, (u8)(para_slvadr & 0xFF));
    if (s32Ret)
    {
        dmsg_i2c_halerr("ERR NUM:%d; send slave addr|write = 0x%04x; NACK from slave\n", s32Ret, para_slvadr);
        goto err_rtn;
    }

    // 3.recv data
    s32Ret |= HAL_I2C_RecvBuf(para_i2c_base, para_pdata, para_len);
    if (para_len != s32Ret)
    {
        dmsg_i2c_halerr("ERR NUM:%d; read data err\n", s32Ret);
        goto err_rtn;
    }
    dmsg_i2c_halwarn("NORMAL READ SUCCESS DONE\n");
    return 0;
err_rtn:
    return s32Ret;
}
/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
S32 HAL_I2C_CntSet(struct _hal_iic *para_i2c_base)
{
    S32 s32Ret = 0;

    s32Ret |= HAL_I2C_ForStpCntSet(para_i2c_base, para_i2c_base->stI2cClkCnt.u16CntForStp);
    s32Ret |= HAL_I2C_HiPerCntSet(para_i2c_base, para_i2c_base->stI2cClkCnt.u16CntHighPerid);
    s32Ret |= HAL_I2C_LowPerCntSet(para_i2c_base, para_i2c_base->stI2cClkCnt.u16CntLowPerid);
    s32Ret |= HAL_I2C_BtwnFlEdgCntSet(para_i2c_base, para_i2c_base->stI2cClkCnt.u16CntBtwnFalEdg);
    s32Ret |= HAL_I2C_ForStarCntSet(para_i2c_base, para_i2c_base->stI2cClkCnt.u16CntForStart);
    s32Ret |= HAL_I2C_LtchTimCntSet(para_i2c_base, para_i2c_base->stI2cClkCnt.u16CntDataLatchTim);
    s32Ret |= HAL_I2C_TimoutOcrCntSet(para_i2c_base, para_i2c_base->stI2cClkCnt.u16CntTimoutIntrDly);

    return s32Ret;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_DmaSetup(struct _hal_iic *para_i2c_base)
{
    S32 s32Ret = 0;
    s32Ret |= HAL_I2C_DmaTrDone(para_i2c_base, true);
    s32Ret |= HAL_I2C_DmaAdrModSet(para_i2c_base, para_i2c_base->stI2cDmaCtrl.enDmaAdrMod);

    s32Ret |= HAL_I2C_DmaMiuPriSet(para_i2c_base, para_i2c_base->stI2cDmaCtrl.enDmaMiuPri);

    s32Ret |= HAL_I2C_DmaMiuChnalSet(para_i2c_base, para_i2c_base->stI2cDmaCtrl.enDmaMiuChanel);

    s32Ret |= HAL_I2C_DmaMiuAddrSet(para_i2c_base, para_i2c_base->stI2cDmaCtrl.stDmaMiuAdr.DmaAdrMiu);

    // set interrupt occure after miu move data done completely
    I2C_WRITE_REG_MASK(REG_I2C_DMA_RESERVED, BIT4 | BIT5 | BIT7, REG_MASK_LASTDONE);

    return s32Ret;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_MstSetup(struct _hal_iic *para_i2c_base)
{
    S32 s32Ret = 0;

    // 1.clear interrupt
    s32Ret |= HAL_I2C_ClearInt(para_i2c_base);
    // 2.reset master
    s32Ret |= HAL_I2C_RstSet(para_i2c_base, true);
    s32Ret |= HAL_I2C_RstSet(para_i2c_base, false);
    // 3.config set
    I2C_WRITE_REG(REG_I2C_CONFIG, para_i2c_base->u8I2cConfig);
    s32Ret |= HAL_I2C_DmaCfgSofRstSet(para_i2c_base, true);
    s32Ret |= HAL_I2C_DmaCfgMiuRst(para_i2c_base, true);
    s32Ret |= HAL_I2C_DmaCfgMiuRst(para_i2c_base, false);
    return s32Ret;
}

S32 HAL_I2C_Release(struct _hal_iic *para_i2c_base)
{
    S32 s32Ret = 0;

    if (0 == para_i2c_base->u32EnDma)
    {
        s32Ret |= HAL_I2C_TranCmdStop(para_i2c_base);
        DELAY_N_US(5);
    }
    s32Ret |= HAL_I2C_RstSet(para_i2c_base, true);
    s32Ret |= HAL_I2C_RstSet(para_i2c_base, false);

    return s32Ret;
}
/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
S32 HAL_I2C_Write(struct _hal_iic *para_i2c_base, U16 para_slvadr, u8 *para_pdata, U32 para_len)
{
    S32 s32Ret = 0;
    u8  u8SlvAddr;

    u8SlvAddr = (u8)((para_slvadr & 0x00FF) << 1);
    u8SlvAddr &= I2C_SET_WRITEBIT_INDATA;
    if (para_i2c_base->u32EnDma)
    {
        s32Ret |= HAL_I2C_DmaWrite(para_i2c_base, para_slvadr, para_pdata, para_len);
    }
    else
    {
        s32Ret |= HAL_I2C_NormalWrite(para_i2c_base, u8SlvAddr, para_pdata, para_len);
    }

    if (s32Ret)
    {
        s32Ret = -E_ERR_WRITE;
        dmsg_i2c_halerr("write err\n");
    }

    return s32Ret;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/

S32 HAL_I2C_Read(struct _hal_iic *para_i2c_base, U16 para_slvadr, u8 *para_pdata, U32 para_len)
{
    S32 s32Ret = 0;
    u8  u8SlvAddr;

    u8SlvAddr = (u8)((para_slvadr & 0x00FF) << 1);
    u8SlvAddr |= I2C_SET_READBIT_INDATA;

    if (para_i2c_base->u32EnDma)
    {
        s32Ret |= HAL_I2C_DmaRead(para_i2c_base, para_slvadr, para_pdata, para_len);
    }
    else
    {
        s32Ret |= HAL_I2C_NormalRead(para_i2c_base, u8SlvAddr, para_pdata, para_len);
    }
    if (s32Ret)
    {
        s32Ret = -E_ERR_READ;
        dmsg_i2c_halerr("read err\n");
    }
    return s32Ret;
}

/****************************************************/
//*func:
//*description:
//*parameter:
//*return:
/****************************************************/
static S32 HAL_I2C_CalcCnt(struct _hal_iic *paraI2cBase, U32 paraSrcClk, EN_I2C_SPEED_MODE paraSpeedMode_e)
{
    U16 u16LPeridCnt, u16HPeridCnt, u16PeridCnt;

    u16PeridCnt = (paraSrcClk / paraI2cBase->u32Speed);
    switch (paraSpeedMode_e)
    {
        case E_I2C_SPEED_MODE_NORMAL:
            u16PeridCnt -= DRV_I2C_CNT_DEC_12M;
            u16HPeridCnt = (u16PeridCnt / 2);
            u16LPeridCnt = u16PeridCnt - u16HPeridCnt;
            break;
        case E_I2C_SPEED_MODE_HIGH:
            u16PeridCnt -= DRV_I2C_CNT_DEC_54M;
            u16HPeridCnt = (u16PeridCnt / 3);
            u16LPeridCnt = u16PeridCnt - u16HPeridCnt;
            break;
        case E_I2C_SPEED_MODE_ULTRA:
            u16PeridCnt -= DRV_I2C_CNT_DEC_72M;
            u16HPeridCnt = (u16PeridCnt / 3);
            u16LPeridCnt = u16PeridCnt - u16HPeridCnt;
            break;
        default:
            u16LPeridCnt = 0;
            u16HPeridCnt = 0;
            dmsg_i2c_halerr("i2c mode err\n");
            break;
    } // end of switch(EN_SpeedMode)
    paraI2cBase->stI2cClkCnt.u16CntLowPerid  = u16LPeridCnt;
    paraI2cBase->stI2cClkCnt.u16CntHighPerid = u16HPeridCnt;

    return 0;
}
S32 HAL_I2C_SrclkSelect(struct _hal_iic *para_i2c_base)
{
    U32               u32SrcClk, u32Speed;
    S32               s32Ret = 0;
    EN_I2C_SPEED_MODE EN_SpeedMode;

    u32Speed = para_i2c_base->u32Speed;
    u32Speed = (u32Speed / 1000) * 1000;
    // select source clock
    if (u32Speed <= DRV_I2C_SPEED_200KHZ)
    {
        EN_SpeedMode = E_I2C_SPEED_MODE_NORMAL;
        u32SrcClk    = DRV_I2C_SRCCLK_12M;
    }
    else if (u32Speed <= DRV_I2C_SPEED_700KHZ)
    {
        EN_SpeedMode = E_I2C_SPEED_MODE_HIGH;
        u32SrcClk    = DRV_I2C_SRCCLK_54M;
    }
    else if (u32Speed < DRV_I2C_SPEED_1500KHZ)
    {
        EN_SpeedMode = E_I2C_SPEED_MODE_ULTRA;
        u32SrcClk    = DRV_I2C_SRCCLK_72M;
    }
    else
    {
        EN_SpeedMode = E_I2C_SPEED_MODE_HIGH;
        u32SrcClk    = DRV_I2C_SRCCLK_72M;
        u32Speed     = DRV_I2C_SPEED_1500KHZ;
    }
    para_i2c_base->u32Speed = u32Speed;
    if (para_i2c_base->calbak_i2c_set_srcclk)
    {
        s32Ret = para_i2c_base->calbak_i2c_set_srcclk((void *)para_i2c_base, u32SrcClk);
        if (s32Ret)
        {
            dmsg_i2c_halerr("clock callback func err\n");
            s32Ret = -E_ERR_SRCCLK_SETUP;
            goto err_set;
        }
    }
    else
    {
        return -E_ERR;
    }

    HAL_I2C_CalcCnt(para_i2c_base, u32SrcClk, EN_SpeedMode);

err_set:
    return s32Ret;
}
S32 HAL_I2C_Init(struct _hal_iic *para_i2c_base)
{
    S32 s32Ret = 0;

    /*reg miic config*/
    para_i2c_base->u8I2cConfig &= 0x00;
    para_i2c_base->u8I2cConfig |= (HAL_I2CONFIG_FILTER_EN | HAL_I2CONFIG_OEN_PUSH_EN);
    if (para_i2c_base->u32EnDma)
    {
        para_i2c_base->u8I2cConfig |= HAL_I2CONFIG_DMA_EN | HAL_I2CONFIG_INT_EN | HAL_I2CONFIG_TIMOUT_INT_EN;
    }
    /*reg cnt default val*/
    para_i2c_base->stI2cClkCnt.u16CntBtwnFalEdg    = 8;
    para_i2c_base->stI2cClkCnt.u16CntDataLatchTim  = 5;
    para_i2c_base->stI2cClkCnt.u16CntForStart      = 38;
    para_i2c_base->stI2cClkCnt.u16CntForStp        = 38;
    para_i2c_base->stI2cClkCnt.u16CntHighPerid     = 25;
    para_i2c_base->stI2cClkCnt.u16CntLowPerid      = 27;
    para_i2c_base->stI2cClkCnt.u16CntTimoutIntrDly = 0xff;
    /*dma confing*/
    para_i2c_base->stI2cDmaCtrl.enDmaAdrMod    = E_DMA_ADDRMODE_NORMAL;
    para_i2c_base->stI2cDmaCtrl.enDmaMiuChanel = E_DMA_MIU_CHANNEL0;
    para_i2c_base->stI2cDmaCtrl.enDmaMiuPri    = E_DMA_MIUPRI_LOW;
    para_i2c_base->stI2cDmaCtrl.bDmaCfgEnIntr  = false;
    /*set mode of detecting dma done*/
    para_i2c_base->bDmaDetctMod = 0;
    /*selcet source clk*/

    s32Ret = HAL_I2C_SrclkSelect(para_i2c_base);

    s32Ret |= HAL_I2C_MstSetup(para_i2c_base);
    s32Ret |= HAL_I2C_CntSet(para_i2c_base);
    if (para_i2c_base->u32EnDma)
        s32Ret |= HAL_I2C_DmaSetup(para_i2c_base);
    return s32Ret;
}
