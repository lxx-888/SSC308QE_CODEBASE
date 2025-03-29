/*
 * hal_sdmmc_v5.h- Sigmastar
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
#ifndef __HAL_SDMMC_V5_H
#define __HAL_SDMMC_V5_H

#include "hal_sdmmc_base.h"
#if (D_OS == D_OS__LINUX)
#include <linux/mmc/mmc.h>
#include "drv_miu.h"
#elif (D_OS == D_OS__RTK)
#endif
#include "hal_sdmmc_regs.h"
//***********************************************************************************************************
// Config Setting (Externel)
//***********************************************************************************************************
//###########################################################################################################
/* --------------------------------------------------------
< WT_POWERUP >
SD Spec:
- This delay should be sufficient to allow the power supply to reach the minimum voltage.
HW measure:
- About 5x us is enough.

< WT_POWERON >
SD Spec:
- This delay must be at least 74 clock sizes, or 1 ms, or the time required to reach a stable voltage.

< WT_POWEROFF >
SD Spec:
- the card VDD shall be once lowered to less than 0.5Volt for a minimum period of 1ms.
HW measure:
- SD_3V3 has 2K resistance to gnd: 30 ms.
- SD_3V3 does Not have any resistance to gnd: 1500 ms.
-------------------------------------------------------- */
#define WT_POWERUP  1  //(ms)
#define WT_POWERON  1  //(ms)
#define WT_POWEROFF 30 //(ms) Here is only for default, real value will be from DTS.

// Wait Time
//-----------------------------------------------------------------------------------------------------------
#define WT_DAT0HI_END 20000 //(ms)
#define WT_EVENT_CIFD 500   //(ms)
#define WT_RESET      100   //(ms)

#define WT_EVENT_RSP   1000  //(ms)
#define WT_EVENT_READ  10000 // 2000 //(ms)
#define WT_EVENT_WRITE 10000 // 3000 //(ms)

//===========================================================
// device status (R1, R1b)
//===========================================================
#define eMMC_R1_CURRENT_STATE  (BIT12 | BIT11 | BIT10 | BIT9)
#define eMMC_R1_READY_FOR_DATA BIT8

#define SDMMC_ERR_R1_31_24                                                                                           \
    (R1_OUT_OF_RANGE | R1_ADDRESS_ERROR | R1_BLOCK_LEN_ERROR | R1_ERASE_SEQ_ERROR | R1_ERASE_PARAM | R1_WP_VIOLATION \
     | R1_LOCK_UNLOCK_FAILED)
#define SDMMC_ERR_R1_23_16 \
    (R1_COM_CRC_ERROR | R1_ILLEGAL_COMMAND | R1_CARD_ECC_FAILED | R1_CC_ERROR | R1_ERROR | R1_CID_CSD_OVERWRITE)
#define SDMMC_ERR_R1_15_8 (R1_WP_ERASE_SKIP | R1_ERASE_RESET)
#define SDMMC_ERR_R1_7_0  (R1_SWITCH_ERROR)

#define SDMMC_ERR_R1_31_0       (SDMMC_ERR_R1_31_24 | SDMMC_ERR_R1_23_16 | SDMMC_ERR_R1_15_8 | SDMMC_ERR_R1_7_0)
#define SDMMC_ERR_R1_NEED_RETRY (R1_COM_CRC_ERROR | R1_CARD_ECC_FAILED | R1_CC_ERROR | R1_ERROR | R1_SWITCH_ERROR)

//===========================================================
// error status
//===========================================================
#define NOT_MAKE_ERR     0
#define MAKE_RD_CRC_ERR  1
#define MAKE_WR_CRC_ERR  2
#define MAKE_WR_TOUT_ERR 3
#define MAKE_CMD_NO_RSP  4
#define MAKE_CMD_RSP_ERR 5
#define MAKE_RD_TOUT_ERR 6
#define MAKE_CARD_BUSY   7

//###########################################################################################################
//###########################################################################################################

typedef enum
{
    // SD_STS Reg Error
    EV_STS_OK       = 0x0000,
    EV_STS_RD_CERR  = BIT00_T,
    EV_STS_WD_CERR  = BIT01_T,
    EV_STS_WR_TOUT  = BIT02_T,
    EV_STS_NORSP    = BIT03_T,
    EV_STS_RSP_CERR = BIT04_T,
    EV_STS_RD_TOUT  = BIT05_T,

    // SD IP Error
    EV_STS_RIU_ERR   = BIT06_T,
    EV_STS_DAT0_BUSY = BIT07_T,
    EV_STS_MIE_TOUT  = BIT08_T,

    // Stop Wait Process Error
    EV_SWPROC_ERR = BIT09_T,

    // SD Check Error
    EV_CMD8_PERR     = BIT10_T,
    EV_OCR_BERR      = BIT11_T,
    EV_OUT_VOL_RANGE = BIT12_T,
    EV_STATE_ERR     = BIT13_T,

    // Other Error
    EV_OTHER_ERR = BIT15_T,

} RspErrEmType;

typedef enum
{
    EV_CMDRSP   = 0x000,
    EV_CMDREAD  = 0x001,
    EV_CMDWRITE = 0x101,

} CmdEmType;

typedef enum
{
    EV_EMP  = 0x0000,
    EV_ADMA = 0x0020, // Add at FCIE5
    EV_DMA  = 0x0080, // Change at FCIE5
    EV_CIF  = 0x1000, // Change at FCIE5

} TransEmType;

//(2bits: Rsp Mapping to SD_CTL) (4bits: Identity) (8bits: RspSize)
typedef enum
{
    EV_NO  = 0x0000, // No response type
    EV_R1  = 0x2105,
    EV_R1B = 0x2205,
    EV_R2  = 0x3310,
    EV_R3  = 0x2405,
    EV_R4  = 0x2505,
    EV_R5  = 0x2605,
    EV_R6  = 0x2705,
    EV_R7  = 0x2805,

} SDMMCRspEmType;

typedef enum
{
    EV_BUS_1BIT  = 0x00,
    EV_BUS_4BITS = 0x02,
    EV_BUS_8BITS = 0x04,

} SDMMCBusWidthEmType;

typedef enum
{
    EV_MIE  = 0x0,
    EV_CIFD = 0x1,
} IPEventEmType;

typedef enum
{
    EV_EGRP_OK    = 0x0,
    EV_EGRP_TOUT  = 0x1,
    EV_EGRP_COMM  = 0x2,
    EV_EGRP_OTHER = 0x3,

} ErrGrpEmType;

typedef struct
{
    U8_T         u8Cmd;
    U32_T        u32Arg;     // Mark for ROM
    U32_T        u32ErrLine; // Mark for ROM
    RspErrEmType eErrCode;
    U8_T         u8RspSize;           // Mark for ROM
    U8_T         u8ArrRspToken[0x10]; // U8_T u8ArrRspToken[0x10];  //Mark for ROM

} RspStruct;

typedef struct
{
    U32_T u32_End : 1;
    U32_T u32_MiuSel : 2;
    U32_T : 9;
    U32_T u32_DmaAddrMSB : 4;
    U32_T u32_JobCnt : 16;
    U32_T u32_Address;
    U32_T u32_DmaLen;
    U32_T u32_Dummy;

} AdmaDescStruct;

typedef enum
{
    EV_SDR = 0x0,
    EV_DDR = 0x1,

} BusEmType;

//  SDMMC Operation Function
//----------------------------------------------------------------------------------------------------------
void Hal_SDMMC_SetDataWidth(IpOrder eIP, SDMMCBusWidthEmType eBusWidth);
void Hal_SDMMC_SetNrcDelay(IpOrder eIP, U32_T u32RealClk);

void         Hal_SDMMC_SetCmdToken(IpOrder eIP, U8_T u8Cmd, U32_T u32Arg);
RspStruct *  Hal_SDMMC_GetRspToken(IpOrder eIP);
extern void  Hal_SDIO_GetRsp8U(IpOrder eIP, U8_T *pSrc);
void         Hal_SDMMC_TransCmdSetting(IpOrder eIP, TransEmType eTransType, U16_T u16BlkCnt, U16_T u16BlkSize,
                                       volatile dma_addr_t uBufAddr, volatile U8_T *pu8Buf);
RspErrEmType Hal_SDMMC_SendCmdAndWaitProcess(IpOrder eIP, TransEmType eTransType, CmdEmType eCmdType,
                                             SDMMCRspEmType eRspType, BOOL_T bCloseClk);
RspErrEmType Hal_SDMMC_RunBrokenDmaAndWaitProcess(IpOrder eIP, CmdEmType eCmdType);
void         Hal_SDMMC_ADMASetting(volatile void *pDMATable, U8_T u8Item, U32_T u32SubLen, U16_T u16SubBCnt,
                                   dma_addr_t uSubAddr, U8_T u8MIUSel, BOOL_T bEnd);

//  SDMMC Special Operation Function
//----------------------------------------------------------------------------------------------------------
void         Hal_SDMMC_ClkCtrl(IpOrder eIP, BOOL_T bEnable, U16_T u16DelayMs);
void         Hal_SDMMC_Reset(IpOrder eIP);
void         Hal_SDMMC_StopProcessCtrl(IpOrder eIP, BOOL_T bEnable);
BOOL_T       Hal_SDMMC_OtherPreUse(IpOrder eIP);
ErrGrpEmType Hal_SDMMC_ErrGroup(RspErrEmType eErrType);

// SDMMC Information
//----------------------------------------------------------------------------------------------------------
void  Hal_SDMMC_ErrHandler(IpOrder eIP);
void  Hal_SDMMC_DumpMemTool(U8_T u8ListNum, volatile U8_T *pu8Buf);
U8_T  Hal_SDMMC_GetDATBusLevel(IpOrder eIP);
U16_T Hal_SDMMC_GetMIEEvent(IpOrder eIP);

// SDMMC SDIO Setting
//----------------------------------------------------------------------------------------------------------
void Hal_SDMMC_SDIODeviceCtrl(IpOrder eIP, BOOL_T bEnable);
void Hal_SDMMC_SDIOIntDetCtrl(IpOrder eIP, BOOL_T bEnable);
void Hal_SDMMC_SetSDIOIntBeginSetting(IpOrder eIP, U8_T u8Cmd, U32_T u32Arg, CmdEmType eCmdType, U16_T u16BlkCnt);
void Hal_SDMMC_SetSDIOIntEndSetting(IpOrder eIP, RspErrEmType eRspErr, U16_T u16BlkCnt);

BOOL_T Hal_SDMMC_SavePassPhase(IpOrder eIP, U8_T u8Phase, BOOL_T bCleanFlag);
void   Hal_SDMMC_SetPhase(IpOrder eIP, BusEmType eSD30Bus, U8_T u8Phase);
U8_T   Hal_SDMMC_FindFitPhaseSetting(IpOrder eIP, U8_T u8ScanMaxPhase);
void   Hal_SDMMC_Dump_GoodPhases(IpOrder eIP);

// SDMMC Interrupt Setting
//----------------------------------------------------------------------------------------------------------
#if defined(CONFIG_SSTAR_SDMMC_INTERRUPT_MODE)

void Hal_SDMMC_MIEIntCtrl(IpOrder eIP, BOOL_T bEnable);

#endif // End of CONFIG_SSTAR_SDMMC_INTERRUPT_MODE

void Hal_SDMMC_SDIOinterrupt(IpOrder eIP);
void Hal_SDMMC_PowerSavingModeVerify(IpOrder eIP);

#endif // End of __HAL_SDMMC_V5_H
