/*
 * hal_iic.h- Sigmastar
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

#ifndef _HAL_IIC_H_
#define _HAL_IIC_H_

#include "asm/arch/mach/sstar_types.h"
#include <linux/string.h>
#ifdef IS_LINUX_KERNEL
#include <linux/wait.h>
#include <linux/completion.h>
#endif

#define DRV_I2C_SRCCLK_72M (72000000)
#define DRV_I2C_SRCCLK_54M (54000000)
#define DRV_I2C_SRCCLK_12M (12000000)

#define DRV_I2C_CNT_DEC_12M (8)
#define DRV_I2C_CNT_DEC_54M (5)
#define DRV_I2C_CNT_DEC_72M (4)

#define DRV_I2C_SPEED_200KHZ  (200000)
#define DRV_I2C_SPEED_700KHZ  (700000)
#define DRV_I2C_SPEED_1500KHZ (1500000)

#define I2C_SET_WRITEBIT_INDATA (0xFFFE)
#define I2C_SET_READBIT_INDATA  (0x0001)

#define I2C_DMA_TIMEOUT_MS (2000)

typedef enum _en_iic_hal_err
{
    E_ERR = 1,
    E_ERR_INIT,         // INIT ERR
    E_ERR_MST_SETUP,    // I2C MASTER SET UP ERR
    E_ERR_CNT_SETUP,    // COUNT SET UP ERR
    E_ERR_DMA_SETUP,    // DMA SET UP ERR
    E_ERR_SRCCLK_SETUP, // SET SOURCE CLK ERR
    E_ERR_WRITE,        // WRITE ERR
    E_ERR_READ,         // READ ERR
    E_ERR_PARAMETER,    // PARAMETER ERR
    E_ERR_TIMEOUT,      // TIMEOUT ERR
    E_ERR_RETRY,
} EN_I2C_HAL_ERRNUM;

struct _hal_iic_clkcnt
{
    U16 u16CntForStp;
    U16 u16CntHighPerid;
    U16 u16CntLowPerid;
    U16 u16CntBtwnFalEdg;
    U16 u16CntForStart;
    U16 u16CntDataLatchTim;
    U16 u16CntTimoutIntrDly;
};

typedef enum _hal_dma_addr_mode
{
    E_DMA_ADDRMODE_NORMAL = 0,
    E_DMA_ADDRMODE_10BIT,
    E_DMA_ADDRMODE_MAX,
} EN_HAL_DMA_ADRMODE;

typedef enum _hal_dma_miu_priority
{
    E_DMA_MIUPRI_LOW = 0,
    E_DMA_MIUPRI_HIGH,
    E_DMA_MIUPRI_MAX,
} EN_HAL_DMA_MIUPRIORITY;

typedef enum _hal_dma_miu_channel
{
    E_DMA_MIU_CHANNEL0 = 0,
    E_DMA_MIU_CHANNEL1,
    E_DMA_MIU_MAX,
} EN_HAL_DMA_MIUCHANNEL;

typedef struct _hal_dma_addr
{
    phys_addr_t DmaAdrPhy;
    U8 *        pu8DmaAdrVirtu;
    phys_addr_t DmaAdrMiu;
} ST_HAL_DMA_ADDR;

struct _hal_dma_ctrl
{
    EN_HAL_DMA_ADRMODE     enDmaAdrMod;
    EN_HAL_DMA_MIUPRIORITY enDmaMiuPri;
    EN_HAL_DMA_MIUCHANNEL  enDmaMiuChanel;
    ST_HAL_DMA_ADDR        stDmaMiuAdr;
    BOOL                   bDmaCfgEnIntr;
};

typedef enum _i2c_speed_mode
{
    E_I2C_SPEED_MODE_NORMAL = 0,
    E_I2C_SPEED_MODE_HIGH,
    E_I2C_SPEED_MODE_ULTRA,
    E_I2C_SPEED_MODE_MAX,
} EN_I2C_SPEED_MODE;

struct _hal_iic
{
    phys_addr_t            BankBase;
    U8                     u8I2cConfig;
    U32                    u32EnDma;
    U32                    u32Speed;
    U32                    u32Group;
    U16                    u32PadmuxMod;
    struct _hal_iic_clkcnt stI2cClkCnt;
    struct _hal_dma_ctrl   stI2cDmaCtrl;
    S32 (*calbak_i2c_set_srcclk)(void *paraI2cBase, U32 paraClkSel);
    BOOL bDmaDetctMod; // 0:poll; 1:intr
};
/******************************************************************************/
/***********************    function declaration    ***************************/
/******************************************************************************/
extern S32 HAL_I2C_DmaTrDone(struct _hal_iic *para_i2c_base, BOOL para_boolval);
extern S32 HAL_I2C_CntSet(struct _hal_iic *para_i2c_base);
extern S32 HAL_I2C_SrclkSelect(struct _hal_iic *para_i2c_base);
extern s32 HAL_I2C_DmaStopFmt(struct _hal_iic *para_i2c_base, BOOL para_stop);
extern S32 HAL_I2C_Write(struct _hal_iic *para_i2c_base, U16 para_slvadr, U8 *para_pdata, U32 para_len);
extern S32 HAL_I2C_Read(struct _hal_iic *para_i2c_base, U16 para_slvadr, U8 *para_pdata, U32 para_len);
extern S32 HAL_I2C_Release(struct _hal_iic *para_i2c_base);
extern S32 HAL_I2C_Init(struct _hal_iic *para_i2c_base);

#endif
