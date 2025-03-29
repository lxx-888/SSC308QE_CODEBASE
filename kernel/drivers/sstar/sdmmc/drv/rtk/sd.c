/*
 * sd.c- Sigmastar
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
/***************************************************************************************************************
 *
 * FileName sd.c
 *     @author jeremy.wang (2012/01/10)
 * Desc:
 *     This layer between Linux SD Driver layer and IP Hal layer.
 *     (1) The goal is we don't need to change any Linux SD Driver code, but we can handle here.
 *     (2) You could define Function/Ver option for using, but don't add Project option here.
 *     (3) You could use function option by Project option, but please add to ms_sdmmc.h
 *
 ***************************************************************************************************************/

#include "cam_os_wrapper.h"
#include "hal_int_ctrl_pub.h"
#include "hal_sdmmc_v5.h"
//#include "sdio.h"
#include "sd.h"
#include "hal_sdmmc_timer.h"
#include "initcall.h"
#include "drv_sdmmc_rtk.h"
#include "drv_sdmmc_common.h"
#include "hal_sdmmc_platform_pri_config.h"

#define SD_OCR_RANGE (BIT15_T | BIT16_T | BIT17_T | BIT18_T | BIT19_T | BIT20_T)

#define CLK_DEF_SPEED    25000000
#define CLK_HIGH_SPEED   50000000
#define CLK_SDR104_SPEED 208000000
#define CLK_SDR50_SPEED  100000000
#define CLK_DDR50_SPEED  CLK_HIGH_SPEED
#define CLK_SDR25_SPEED  CLK_HIGH_SPEED
#define CLK_SDR12_SPEED  CLK_DEF_SPEED

#define UHS_B_DRV_TYPE 0
#define UHS_A_DRV_TYPE 1
#define UHS_C_DRV_TYPE 2
#define UHS_D_DRV_TYPE 3

#define UHS_200_CURR_LMT 0
#define UHS_400_CURR_LMT 1
#define UHS_600_CURR_LMT 2
#define UHS_800_CURR_LMT 3

#define SD_CURR_LMT_200 (1 << UHS_200_CURR_LMT)
#define SD_CURR_LMT_400 (1 << UHS_400_CURR_LMT)
#define SD_CURR_LMT_600 (1 << UHS_600_CURR_LMT)
#define SD_CURR_LMT_800 (1 << UHS_800_CURR_LMT)

typedef enum
{
    EV_NOCARD = 0,  // Unknow memory card
    EV_MMC    = 1,  // MMC card
    EV_SDIO   = 5,  // SDIO
    EV_SD     = 41, // SD card

} CARDTypeEmType;

typedef enum
{
    EV_SCS = 0,
    EV_HCS = BIT30_T,
} SDCAPEmType;

typedef struct
{
    U8_T  MID;
    U8_T  OID[2];
    U8_T  PNM[6];
    U8_T  PRV;
    U32_T PSN;

} CARDCIDStruct;

typedef struct
{
    U8_T  CSDSTR;
    U8_T  SPECVERS;
    U32_T TAAC_NS;
    U8_T  NSAC;
    U8_T  R2W_FACTOR;
    U32_T TRAN_KB;
    U16_T CCC;
    U8_T  R_BLK_SIZE;
    U8_T  W_BLK_SIZE;
    U8_T  W_BLK_MISALIGN;
    U8_T  R_BLK_MISALIGN;
    U8_T  PERM_W_PROTECT;
    U8_T  TEMP_W_PROTECT;
    U64_T CAPCITY;

} CARDCSDStruct;

typedef struct
{
    U32_T          u32RCAArg; // Relative Card Address
    U32_T          u32OCR;
    U32_T          u32CardStatus;
    U32_T          u32MaxClk;
    U8_T           u8SpecVer;
    U8_T           u8SpecVer1;
    U8_T           u8BusWidth;
    U8_T           u8AccessMode;
    U8_T           u8DrvStrength;
    U8_T           u8CurrMax;
    U8_T           u8SD3BusMode;
    U8_T           u8SD3DrvType;
    U8_T           u8SD3CurrLimit;
    SDCAPEmType    eHCS;
    CARDCIDStruct  stCID;
    CARDCSDStruct  stCSD;
    CARDTypeEmType eCardType;

} SDMMCInfoStruct;

#define CamOsMemFullFlush(ptr, len)        \
    do                                     \
    {                                      \
        CamOsMemFlush((void *)(ptr), len); \
        CamOsMiuPipeFlush();               \
    } while (0)

#if 1
#define A_DMA_DATA_BUF 0x21000000

// MMPF_OS_SEMID mSDIOBusySemID[MMPF_SD_DEV_NUM];
// extern MMP_USHORT MMPF_SDIO_BusRelease(stSDMMCHandler *SDMMCArg);
// extern MMP_ERR MMPF_SDIO_BusAcquire(stSDMMCHandler *SDMMCArg);

/// Add for SDIO NONBLOCKING CPU mode (pending for Block Transfer Done interrupt)
//#define SDIO_CPU_NONBLOCKING (1)

// extern MMPF_OS_FLAGID SYS_Flag_Hif;
#endif /* (ENABLE_SDIO_FEATURE==1) */

#if 0
#define CamOsPrintf_info(fmt, arg...) CamOsPrintf(fmt, ##arg)
#else
#define CamOsPrintf_info(fmt, arg...)
#endif

#define D_SDMMC_BUSMODE (SD_MODE_HIGH_SPEED | SD_MODE_UHS_SDR25 /*|SD_MODE_UHS_DDR50*/)

extern struct sstar_mmc_priv *gp_mmc_priv[SDMMC_NUM_TOTAL];
extern CamOsMutex_t           sdmmc_init_mutex[SDMMC_NUM_TOTAL];
extern volatile BOOL_T        gb_SDCardInitFlag[SDMMC_NUM_TOTAL];

//
struct mmc_command gsCmd[2];
struct mmc_data    gsData[2];
struct mmc_command gsCmdStop[2];
struct mmc_host    gsMMCHost[2];

static SDMMCInfoStruct _stSDMMCInfo[SDMMC_NUM_TOTAL];
static volatile U8_T   gu8RspBuf[SDMMC_NUM_TOTAL][512];

int MMPF_SD_SendCommand(U8_T u8Slot, U8_T command, U32_T argument)
{
    struct mmc_host *   p_mmc_host;
    struct mmc_request *pSDReq;
    struct mmc_request  NormalReq;
    struct mmc_request  DataRWReq;
#if 0
    struct mmc_request   BurstRWReq;
#endif
    p_mmc_host         = &gsMMCHost[u8Slot];
    p_mmc_host->slotNo = u8Slot;

    //
    NormalReq.cmd  = &gsCmd[u8Slot];
    NormalReq.sbc  = NULL;
    NormalReq.data = NULL;
    NormalReq.stop = NULL;
    DataRWReq.cmd  = &gsCmd[u8Slot];
    DataRWReq.data = &gsData[u8Slot];
    DataRWReq.sbc  = NULL;
    DataRWReq.stop = NULL;
#if 0
    BurstRWReq.cmd = &gsCmd[u8Slot];
    BurstRWReq.sbc = NULL;
    BurstRWReq.data = &gsData[u8Slot];
    BurstRWReq.stop = &gsCmdStop[u8Slot];
#endif

    //
    // gsCmd[u8_slot].u8Slot = u8_slot;
    gsCmd[u8Slot].opcode = command;
    gsCmd[u8Slot].arg    = argument;

    //
    if ((command == SD_IO_RW_EXTENDED) && ((argument & 0x80000000) == 0x80000000))
        gsData[u8Slot].flags = MMC_DATA_WRITE;
    else if ((command == SD_IO_RW_EXTENDED) && ((argument & 0x80000000) == 0))
        gsData[u8Slot].flags = MMC_DATA_READ;

    //
    switch (command)
    {
        case MMC_WRITE_BLOCK:
        case MMC_WRITE_MULTIPLE_BLOCK:
        case SD_IO_RW_EXTENDED:
        case MMC_READ_SINGLE_BLOCK:
        case MMC_READ_MULTIPLE_BLOCK:
            pSDReq = &DataRWReq;
            if (_stSDMMCInfo[u8Slot].eHCS == EV_SCS)
                gsCmd[u8Slot].arg = argument << 9;
            break;

        default: /* others */
            pSDReq = &NormalReq;
            break;
    }

    //
    switch (command)
    {
        case MMC_GO_IDLE_STATE:
            gsCmd[u8Slot].flags = MMC_RSP_NONE;
            break;

        case MMC_STOP_TRANSMISSION:
        case MMC_ERASE:
            gsCmdStop[u8Slot].flags = MMC_RSP_R1B;
            gsCmd[u8Slot].flags     = MMC_RSP_R1B;
            break;

        case MMC_SELECT_CARD:
            gsCmd[u8Slot].flags = MMC_RSP_R1;
            break;

        case SD_SEND_RELATIVE_ADDR:
            gsCmd[u8Slot].flags = MMC_RSP_R6;
            break;

        case SD_IO_SEND_OP_COND:
            gsCmd[u8Slot].flags = MMC_RSP_R4;
            break;

        case SD_APP_OP_COND:
            gsCmd[u8Slot].flags = MMC_RSP_R3;
            break;

        case MMC_SEND_CSD:
        case MMC_ALL_SEND_CID:
            gsCmd[u8Slot].flags = MMC_RSP_R2;
            break;
#if 1
        case SD_SWITCH:
        case MMC_WRITE_BLOCK:
        case MMC_READ_SINGLE_BLOCK:
        case MMC_WRITE_MULTIPLE_BLOCK:
        case MMC_READ_MULTIPLE_BLOCK:
        case MMC_SEND_STATUS:
        case MMC_APP_CMD:
        case SD_ERASE_WR_BLK_END:
        case SD_ERASE_WR_BLK_START:
            gsCmd[u8Slot].flags = MMC_RSP_R1;
            break;

        case SD_SEND_IF_COND:
            gsCmd[u8Slot].flags = MMC_RSP_R7;
            break;
#else
        case MMC_APP_CMD:
            gsCmd[slot].flags = MMC_RSP_R7;
            break;

        case SD_SEND_IF_COND:
            gsCmd[slot].flags = MMC_RSP_R1;
            break;
#endif
        case SD_IO_RW_DIRECT:
        case SD_IO_RW_EXTENDED:
            gsCmd[u8Slot].flags = MMC_RSP_R5;
            break;

        default: /* others */
            gsCmd[u8Slot].flags = (MMC_RSP_R3 | MMC_RSP_R4 | MMC_RSP_R5 | MMC_RSP_R6);
            break;
    }

    //
    sstar_sdmmc_request(p_mmc_host, pSDReq);

    return pSDReq->cmd->error;
}

static int MMPF_SD_SendDataCommand(U8_T u8Slot, U8_T command, U32_T argument, U16_T u16BlkCnt, U16_T u16BlkSize,
                                   volatile U8_T *pu8Buf)
{
    struct mmc_host *   p_mmc_host;
    struct mmc_request *pSDReq;
    struct mmc_request  DataRWReq;
#if 0
    struct mmc_request   BurstRWReq;
#endif
    p_mmc_host         = &gsMMCHost[u8Slot];
    p_mmc_host->slotNo = u8Slot;

    DataRWReq.cmd  = &gsCmd[u8Slot];
    DataRWReq.data = &gsData[u8Slot];
    DataRWReq.sbc  = NULL;
    if (u16BlkCnt > 1)
    {
        DataRWReq.stop = &gsCmdStop[u8Slot];
        // gsCmdStop[u8_slot].u8Slot = u8_slot;
        gsCmdStop[u8Slot].opcode = MMC_STOP_TRANSMISSION;
        gsCmdStop[u8Slot].arg    = 0;
        gsCmdStop[u8Slot].flags  = MMC_RSP_R1B;
    }
    else
        DataRWReq.stop = NULL;
#if 0
    BurstRWReq.cmd = &gsCmd[u8_slot];
    BurstRWReq.sbc = NULL;
    BurstRWReq.data = &gsData[u8_slot];
    BurstRWReq.stop = &gsCmdStop[u8_slot];
#endif
    //
    // gsCmd[u8_slot].u8Slot = u8_slot;
    gsCmd[u8Slot].opcode = command;
    if (_stSDMMCInfo[u8Slot].eHCS == EV_SCS)
        gsCmd[u8Slot].arg = argument << 9;
    else
        gsCmd[u8Slot].arg = argument;

    gsData[u8Slot].flags = MMC_DATA_READ;

    //
    if ((command == MMC_WRITE_BLOCK) || (command == MMC_WRITE_MULTIPLE_BLOCK))
        gsData[u8Slot].flags = MMC_DATA_WRITE;
    //
    gsCmd[u8Slot].flags       = MMC_RSP_R1;
    gsCmd[u8Slot].error       = 0;
    gsData[u8Slot].blocks     = u16BlkCnt;
    gsData[u8Slot].blksz      = u16BlkSize;
    gsData[u8Slot].sg->length = u16BlkSize * u16BlkCnt;
    gsData[u8Slot].sg_len     = 1;
    gsData[u8Slot].pu8Buf     = (volatile char *)pu8Buf;
    gsData[u8Slot].error      = 0;
    //
    pSDReq = &DataRWReq;

    sstar_sdmmc_request(p_mmc_host, pSDReq);

    return pSDReq->cmd->error ? pSDReq->cmd->error
                              : (pSDReq->data->error ? pSDReq->data->error : (pSDReq->stop ? pSDReq->stop->error : 0));
}

static int MMPF_SD_SendACommand(U8_T u8Slot, U8_T command, U32_T argument, U32_T rca)
{
    unsigned short err = 0;
    err                = MMPF_SD_SendCommand(u8Slot, MMC_APP_CMD, rca);
    if (err)
    {
        CamOsPrintf("------------ SD CMD 55 error \r\n");
        return err;
    }

    err = MMPF_SD_SendCommand(u8Slot, command, argument);
    if (err)
    {
        CamOsPrintf("------------ SD ACMD %d error \r\n", command);
        return err;
    }

    return err;
}

static int MMPF_SD_SendSCR(U8_T u8Slot, U8_T command, U32_T argument, U32_T rca, volatile U8_T *pu8DataBuf)
{
    unsigned short err = 0, i;
    err                = MMPF_SD_SendCommand(u8Slot, MMC_APP_CMD, rca);
    if (err)
    {
        CamOsPrintf("------------ SD CMD 55 error \r\n");
        return err;
    }

    err = MMPF_SD_SendDataCommand(u8Slot, command, argument, 1, 8, pu8DataBuf);
    if (err)
    {
        CamOsPrintf("------------ SD ACMD %d error \r\n", command);
        return err;
    }
    for (i = 0; i < 8; i++)
        CamOsPrintf_info("scr databuf: 0x%02x\n", *(pu8DataBuf + i));
    _stSDMMCInfo[u8Slot].u8SpecVer = (*pu8DataBuf) & 0x7;
    CamOsPrintf_info("specver: 0x%02x\n", _stSDMMCInfo[u8Slot].u8SpecVer);
    _stSDMMCInfo[u8Slot].u8SpecVer1 = ((*(pu8DataBuf + 2)) & 0x80) >> 7;
    CamOsPrintf_info("specver1: 0x%02x\n", _stSDMMCInfo[u8Slot].u8SpecVer1);

    if ((*(pu8DataBuf + 1)) & 0x4)
        _stSDMMCInfo[u8Slot].u8BusWidth = 4;

    return err;
}

static unsigned int MMPF_SD_GetCmdRspU32(U8_T u8Slot)
{
    return gsCmd[u8Slot].resp[0];
}

static void _SDMMC_InfoInit(U8_T u8Slot)
{
    // Before _SDMMC_SEND_IF_COND
    _stSDMMCInfo[u8Slot].eHCS = 0; // Init HCS=0

    // Before _SDMMC_SEND_OP_COND
    _stSDMMCInfo[u8Slot].eCardType = EV_NOCARD;
    _stSDMMCInfo[u8Slot].u32RCAArg = 0;
    _stSDMMCInfo[u8Slot].u32OCR    = 0;

    // Before _SDMMC_SEND_SCR
    _stSDMMCInfo[u8Slot].u8BusWidth = 1;
    _stSDMMCInfo[u8Slot].u8SpecVer  = 0;
    _stSDMMCInfo[u8Slot].u8SpecVer1 = 0;

    // Before SDMMC_SwitchHighBus
    _stSDMMCInfo[u8Slot].u8AccessMode   = 0; // DEF_SPEED_BUS_SPEED
    _stSDMMCInfo[u8Slot].u8DrvStrength  = 0;
    _stSDMMCInfo[u8Slot].u8CurrMax      = 0;
    _stSDMMCInfo[u8Slot].u8SD3BusMode   = 0;
    _stSDMMCInfo[u8Slot].u8SD3DrvType   = 0;
    _stSDMMCInfo[u8Slot].u8SD3CurrLimit = 0;
    _stSDMMCInfo[u8Slot].u32MaxClk      = 25000000;
}

static void MMPF_SD_GetCIDInfo(U8_T u8Slot, U32_T *u32Arr)
{
    U8_T u8Pos = 0, u8Offset = 0;

    _stSDMMCInfo[u8Slot].stCID.MID = UNSTUFF_BITS(u32Arr, 120, 8);
    CamOsPrintf_info("CID.MID: 0x%02x\n", _stSDMMCInfo[u8Slot].stCID.MID);
    //-----------------------------------------------------------------------------------------------------------------
    if (_stSDMMCInfo[u8Slot].eCardType == EV_SD)
    {
        _stSDMMCInfo[u8Slot].stCID.OID[0] = UNSTUFF_BITS(u32Arr, 112, 8);
        CamOsPrintf_info("CID.OID[0]: 0x%02x\n", _stSDMMCInfo[u8Slot].stCID.OID[0]);
        _stSDMMCInfo[u8Slot].stCID.OID[1] = UNSTUFF_BITS(u32Arr, 104, 8);
        CamOsPrintf_info("CID.OID[1]: 0x%02x\n", _stSDMMCInfo[u8Slot].stCID.OID[1]);
    }
    else if (_stSDMMCInfo[u8Slot].eCardType == EV_MMC)
    {
        _stSDMMCInfo[u8Slot].stCID.OID[0] = UNSTUFF_BITS(u32Arr, 104, 8);
        CamOsPrintf_info("CID.OID[0]: 0x%02x\n", _stSDMMCInfo[u8Slot].stCID.OID[0]);
    }
    //-----------------------------------------------------------------------------------------------------------------

    for (u8Pos = 0; u8Pos < 5; u8Pos++)
    {
        _stSDMMCInfo[u8Slot].stCID.PNM[u8Pos] = UNSTUFF_BITS(u32Arr, 96 - u8Pos * 8, 8);
        CamOsPrintf_info("CID.PNM[%d]: 0x%02x\n", u8Pos, _stSDMMCInfo[u8Slot].stCID.PNM[u8Pos]);
    }

    if (_stSDMMCInfo[u8Slot].eCardType == EV_SD)
    {
        u8Offset                          = 7;
        _stSDMMCInfo[u8Slot].stCID.PNM[5] = '_';
        CamOsPrintf_info("CID.PNM[5]: 0x%02x\n", _stSDMMCInfo[u8Slot].stCID.PNM[5]);
    }
    else if (_stSDMMCInfo[u8Slot].eCardType == EV_MMC)
    {
        u8Offset                          = 8;
        _stSDMMCInfo[u8Slot].stCID.PNM[5] = UNSTUFF_BITS(u32Arr, 128 - (u8Offset + 1) * 8, 8);
        CamOsPrintf_info("CID.PNM[5]: 0x%02x\n", _stSDMMCInfo[u8Slot].stCID.PNM[5]);
    }

    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[u8Slot].stCID.PRV = UNSTUFF_BITS(u32Arr, 128 - (u8Offset + 1 + 1) * 8, 8);
    CamOsPrintf_info("CID.PRV: 0x%02x\n", _stSDMMCInfo[u8Slot].stCID.PRV);
    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[u8Slot].stCID.PSN = UNSTUFF_BITS(u32Arr, 128 - (u8Offset + 2 + 4) * 8, 32);
    CamOsPrintf_info("CID.PSN: 0x%08x\n", _stSDMMCInfo[u8Slot].stCID.PSN);
}

static void MMPF_SD_GetCSDInfo(U8_T u8Slot, U32_T *u32Arr)
{
    U16_T ReffArr[0x10] = {0, 10, 12, 13, 14, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80};
    U64_T u32CSIZE;
    U8_T  u8CSIZEMULTI, u8Multi;

    //-----------------------------------------------------------------------------------------------------------------
    _stSDMMCInfo[u8Slot].stCSD.CSDSTR = UNSTUFF_BITS(u32Arr, 126, 2);
    CamOsPrintf_info("CSD.CSDSTR: 0x%02x\n", _stSDMMCInfo[u8Slot].stCSD.CSDSTR);

    //-----------------------------------------------------------------------------------------------------------------
    _stSDMMCInfo[u8Slot].stCSD.TAAC_NS = 1;
    for (u8Multi = 1; u8Multi <= (UNSTUFF_BITS(u32Arr, 112, 8) & 0x07); u8Multi++)
        _stSDMMCInfo[u8Slot].stCSD.TAAC_NS *= 10;
    CamOsPrintf_info("times is %d  ---  CSD.TAAC_NS: 0x%02x  ---  REFFARR: %d\n", (UNSTUFF_BITS(u32Arr, 112, 8) & 0x07),
                     _stSDMMCInfo[u8Slot].stCSD.TAAC_NS, (UNSTUFF_BITS(u32Arr, 115, 5)) & 0xF);
    _stSDMMCInfo[u8Slot].stCSD.TAAC_NS *= ReffArr[(UNSTUFF_BITS(u32Arr, 115, 5)) & 0xF];
    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[u8Slot].stCSD.NSAC = UNSTUFF_BITS(u32Arr, 104, 8);
    CamOsPrintf_info("CSD.NSAC: 0x%02x\n", _stSDMMCInfo[u8Slot].stCSD.NSAC);
    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[u8Slot].stCSD.TRAN_KB = 100;
    for (u8Multi = 1; u8Multi <= (UNSTUFF_BITS(u32Arr, 96, 8) & 0x07); u8Multi++)
        _stSDMMCInfo[u8Slot].stCSD.TRAN_KB *= 10;
    CamOsPrintf_info("times is %d  ---  CSD.TRAN_KB: 0x%02x  ---  REFFARR: %d\n", (UNSTUFF_BITS(u32Arr, 96, 8) & 0x07),
                     _stSDMMCInfo[u8Slot].stCSD.TRAN_KB, (UNSTUFF_BITS(u32Arr, 99, 5)) & 0xF);
    _stSDMMCInfo[u8Slot].stCSD.TRAN_KB *= ReffArr[(UNSTUFF_BITS(u32Arr, 99, 5)) & 0xF];
    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[u8Slot].stCSD.CCC = (UNSTUFF_BITS(u32Arr, 88, 8) << 4) | (UNSTUFF_BITS(u32Arr, 84, 4));
    CamOsPrintf_info("CSD.CCC: 0x%02x\n", _stSDMMCInfo[u8Slot].stCSD.CCC);
    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[u8Slot].stCSD.R_BLK_SIZE = (UNSTUFF_BITS(u32Arr, 80, 8)) & 0XF;
    CamOsPrintf_info("CSD.R_BLK_SIZE: 0x%02x\n", _stSDMMCInfo[u8Slot].stCSD.R_BLK_SIZE);
    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[u8Slot].stCSD.W_BLK_MISALIGN = (UNSTUFF_BITS(u32Arr, 78, 2)) & 0x1;
    CamOsPrintf_info("CSD.W_BLK_MISALIGN: 0x%02x\n", _stSDMMCInfo[u8Slot].stCSD.W_BLK_MISALIGN);
    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[u8Slot].stCSD.R_BLK_MISALIGN = (UNSTUFF_BITS(u32Arr, 77, 3)) & 0x1;
    CamOsPrintf_info("CSD.R_BLK_MISALIGN: 0x%02x\n", _stSDMMCInfo[u8Slot].stCSD.R_BLK_MISALIGN);
    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[u8Slot].stCSD.R2W_FACTOR = (UNSTUFF_BITS(u32Arr, 24, 8) & 0x1C) >> 2;
    CamOsPrintf_info("CSD.R2W_FACTOR: 0x%02x\n", _stSDMMCInfo[u8Slot].stCSD.R2W_FACTOR);
    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[u8Slot].stCSD.W_BLK_SIZE = ((UNSTUFF_BITS(u32Arr, 24, 8) & 0x03) << 2) | (UNSTUFF_BITS(u32Arr, 22, 2));
    CamOsPrintf_info("CSD.W_BLK_SIZE: 0x%02x\n", _stSDMMCInfo[u8Slot].stCSD.W_BLK_SIZE);
    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[u8Slot].stCSD.PERM_W_PROTECT = (UNSTUFF_BITS(u32Arr, 13, 3)) & 0x1;
    CamOsPrintf_info("CSD.PERM_W_PROTECT: 0x%02x\n", _stSDMMCInfo[u8Slot].stCSD.PERM_W_PROTECT);
    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[u8Slot].stCSD.TEMP_W_PROTECT = (UNSTUFF_BITS(u32Arr, 12, 4)) & 0x1;
    CamOsPrintf_info("CSD.TEMP_W_PROTECT: 0x%02x\n", _stSDMMCInfo[u8Slot].stCSD.TEMP_W_PROTECT);
    //-----------------------------------------------------------------------------------------------------------------

    if ((_stSDMMCInfo[u8Slot].stCSD.CSDSTR == 1) && (_stSDMMCInfo[u8Slot].eCardType == EV_SD))
    {
        u32CSIZE = ((UNSTUFF_BITS(u32Arr, 64, 8) & 0x3F) << 16) | (UNSTUFF_BITS(u32Arr, 56, 8) << 8)
                   | UNSTUFF_BITS(u32Arr, 48, 8);
        _stSDMMCInfo[u8Slot].stCSD.CAPCITY = (u32CSIZE + 1) * 524288;
        CamOsPrintf_info("CSD.CAPCITY: 0x%llx   -----   u32csize: 0x%llx\n", _stSDMMCInfo[u8Slot].stCSD.CAPCITY,
                         u32CSIZE);
    }
    else // SD1.X and //MMC
    {
        u32CSIZE = ((UNSTUFF_BITS(u32Arr, 72, 8) & 0x3) << 10) | (UNSTUFF_BITS(u32Arr, 64, 8) << 2)
                   | (UNSTUFF_BITS(u32Arr, 62, 2));
        u8CSIZEMULTI = ((UNSTUFF_BITS(u32Arr, 48, 8) & 0x3) << 1) | (UNSTUFF_BITS(u32Arr, 47, 1));
        _stSDMMCInfo[u8Slot].stCSD.CAPCITY =
            (U64_T)(u32CSIZE + 1) * (1 << (u8CSIZEMULTI + 2)) * (1 << _stSDMMCInfo[u8Slot].stCSD.R_BLK_SIZE);
        CamOsPrintf_info("CSD.CAPCITY: 0x%016x   -----   u32csize: 0x%llx, u8csizemulti: 0x%08x\n",
                         _stSDMMCInfo[u8Slot].stCSD.CAPCITY, u32CSIZE, u8CSIZEMULTI);
    }
}

static U16_T MMPF_SD_SEND_OP_COND(U8_T u8Slot, CARDTypeEmType eCardType)
{
    U16_T u16Count = 0, u16Err = 0;
    _stSDMMCInfo[u8Slot].u32OCR = SD_OCR_RANGE | _stSDMMCInfo[u8Slot].eHCS;

    do
    {
        if (eCardType == EV_SD)
        {
            u16Err = MMPF_SD_SendACommand(u8Slot, SD_APP_OP_COND, _stSDMMCInfo[u8Slot].u32OCR,
                                          _stSDMMCInfo[u8Slot].u32RCAArg);
            if (u16Err)
            {
                CamOsPrintf("------------ SD send ACMD 41 set stby error \r\n");
                return u16Err;
            }

            _stSDMMCInfo[u8Slot].u32OCR = MMPF_SD_GetCmdRspU32(u8Slot);
        }

        Hal_Timer_mDelay(1);
        u16Count++;

    } while (!(_stSDMMCInfo[u8Slot].u32OCR & BIT31_T) && (u16Count < 1000)); // Card powered ready (0:busy,1:ready)

    _stSDMMCInfo[u8Slot].eHCS = (SDCAPEmType)(_stSDMMCInfo[u8Slot].u32OCR & BIT30_T);

    if (u16Count >= 1000)
        u16Err = EV_OCR_BERR; // Card is still busy
    else if (!(_stSDMMCInfo[u8Slot].u32OCR & SD_OCR_RANGE & 0x3FFFFFFF))
        u16Err = EV_OUT_VOL_RANGE; // Double Confirm Voltage Range

    if (!u16Err)
        _stSDMMCInfo[u8Slot].eCardType = eCardType;

    return u16Err;
}

static int sd_card_init_process(U8_T u8Slot)
{
    unsigned short err = 0;

    _SDMMC_InfoInit(u8Slot);

    // cmd0 -> reset, not neccessary after power up
    err = MMPF_SD_SendCommand(u8Slot, MMC_GO_IDLE_STATE, 0);

    // CMD8 -> HC support
    err = MMPF_SD_SendCommand(u8Slot, SD_SEND_IF_COND, 0x01AA);
    if (err)
    {
        CamOsPrintf("------------ SD CMD 8 error \r\n");
        goto ERR_HANDLE;
    }

    if ((gsCmd[u8Slot].resp[0] & 0xFF) == 0xAA)
        _stSDMMCInfo[u8Slot].eHCS = BIT30_T;

    // ACMD41 -> OCR
    _stSDMMCInfo[u8Slot].u32OCR = BIT15_T | BIT16_T | BIT17_T | BIT18_T | BIT19_T | BIT20_T | _stSDMMCInfo[u8Slot].eHCS;
    // ACMD41, ARG=OCR -> card ready
    err = MMPF_SD_SEND_OP_COND(u8Slot, EV_SD);
    if (err)
    {
        CamOsPrintf("------------ SD ACMD41 -> std by state error \r\n");
        goto ERR_HANDLE;
    }

    // cmd2 -> cid
    err = MMPF_SD_SendCommand(u8Slot, MMC_ALL_SEND_CID, 0);
    if (err)
    {
        CamOsPrintf("------------ SD CMD 2 error \r\n");
        goto ERR_HANDLE;
    }
    MMPF_SD_GetCIDInfo(u8Slot, gsCmd[u8Slot].resp);

    // cmd3 -> rca
    err = MMPF_SD_SendCommand(u8Slot, SD_SEND_RELATIVE_ADDR, 0);
    if (err)
    {
        CamOsPrintf("------------ SD CMD 3 error \r\n");
        goto ERR_HANDLE;
    }
    else
        _stSDMMCInfo[u8Slot].u32RCAArg = MMPF_SD_GetCmdRspU32(u8Slot) & 0xFFFF0000;

    // cmd9 -> csd
    err = MMPF_SD_SendCommand(u8Slot, MMC_SEND_CSD, _stSDMMCInfo[u8Slot].u32RCAArg);
    if (err)
    {
        CamOsPrintf("------------ SD CMD 9 error \r\n");
        goto ERR_HANDLE;
    }
    // get csd info
    MMPF_SD_GetCSDInfo(u8Slot, gsCmd[u8Slot].resp);

    // cmd13 -> status
    err = MMPF_SD_SendCommand(u8Slot, MMC_SEND_STATUS, _stSDMMCInfo[u8Slot].u32RCAArg);
    if (err)
    {
        CamOsPrintf("------------ SD CMD 13 error \r\n");
        goto ERR_HANDLE;
    }
    else
    {
        _stSDMMCInfo[u8Slot].u32CardStatus = MMPF_SD_GetCmdRspU32(u8Slot);
        if (!(((_stSDMMCInfo[u8Slot].u32CardStatus & (BIT09_T | BIT10_T | BIT11_T | BIT12_T)) >> 9) & 0x03))
        {
            CamOsPrintf("------------ SD stand by state: state_error \r\n");
            goto ERR_HANDLE;
        }
    }

    // cmd7 -> select card
    err = MMPF_SD_SendCommand(u8Slot, MMC_SELECT_CARD, _stSDMMCInfo[u8Slot].u32RCAArg);
    if (err)
    {
        CamOsPrintf("------------ SD CMD 7 error \r\n");
        goto ERR_HANDLE;
    }

#if 0
     //cmd13 -> status
    usErr = MMPF_SD_SendCommand(u8_slot, CMD_SEND_STATUS, _stSDMMCInfo[u8_slot].u32RCAArg );
    if (usErr)
    {
       CamOsPrintf("------------ SD CMD 13 error \r\n");
       goto ERR_HANDLE;
    }
    else
    {
        _stSDMMCInfo[u8_slot].u32CardStatus = MMPF_SD_GetCmdRspU32(u8_slot, 1);
        if ( !( ((_stSDMMCInfo[u8_slot].u32CardStatus & (BIT09_T|BIT10_T|BIT11_T|BIT12_T) ) >> 9 ) & 0x03 ) )
        {
            CamOsPrintf("------------ SD stand by state: state_error \r\n");
            goto ERR_HANDLE;
        }
    }

#endif
    // Acmd51 -> scr
    err = MMPF_SD_SendSCR(u8Slot, SD_APP_SEND_SCR, 0, _stSDMMCInfo[u8Slot].u32RCAArg, gu8RspBuf[u8Slot]);
    if (err)
    {
        CamOsPrintf("------------ SD ACMD 51 for OCR error \r\n");
        goto ERR_HANDLE;
    }

    return 0;

ERR_HANDLE:
    return -1;
}

// set high bus
static int _sd_switch_func(U8_T u8Slot, BOOL_T bSetMode, U8_T u8Group, U8_T u8Value, volatile U8_T *pu8DataBuf)
{
    U32_T u32Arg = (bSetMode << 31) | 0x00FFFFFF;
    u32Arg &= ~(0xF << (u8Group * 4));
    u32Arg |= u8Value << (u8Group * 4);
    CamOsPrintf_info("switch func args: 0x%08x\n", u32Arg);
    //------------------------------------------------------------------------------------------------------------
    return MMPF_SD_SendDataCommand(u8Slot, 6, u32Arg, 1, 64, pu8DataBuf); // CMD6; Query
    //------------------------------------------------------------------------------------------------------------
}

U16_T SDMMC_SwitchHighBus(U8_T u8Slot, volatile U8_T *u8databuf)
{
    U8_T  u8BusMode = 0, u8DrvType = 0, u8CurrLimit = 0;
    U32_T u32MaxClk = 25000000;
    //    volatile U8_T *u8databuf = (volatile U8_T *)(A_DMA_DATA_BUF);
    int err = 0;

    CamOsPrintf_info("rspbuf addr: 0x%08x\n", u8databuf);
    if (_stSDMMCInfo[u8Slot].eCardType == EV_SD)
    {
        if (_stSDMMCInfo[u8Slot].u8SpecVer == 0) // SD 1.0=> Not Support SD CMD6 for HS
            return 0;

        //************************************** Check Function ***********************************************
        //------------------------------------------------------------------------------------------------------------
        err = _sd_switch_func(u8Slot, 0, 0, 1, u8databuf); // Query Group 1
        //------------------------------------------------------------------------------------------------------------
        if (err)
            return err;

        u8BusMode = *(u8databuf + 13);

        // SD 3.0
        if (_stSDMMCInfo[u8Slot].u8SpecVer1)
        {
            _stSDMMCInfo[u8Slot].u8SD3BusMode = u8BusMode;

            //------------------------------------------------------------------------------------------------------------
            err = _sd_switch_func(u8Slot, 0, 2, 1, u8databuf); // Query Group 3
            //------------------------------------------------------------------------------------------------------------

            if (err)
                return (U16_T)err;

            _stSDMMCInfo[u8Slot].u8SD3DrvType = *(u8databuf + 9);

            //------------------------------------------------------------------------------------------------------------
            err = _sd_switch_func(u8Slot, 0, 3, 1, u8databuf); // Query Group 4
            //------------------------------------------------------------------------------------------------------------
            if (err)
                return (U16_T)err;

            _stSDMMCInfo[u8Slot].u8SD3CurrLimit = *(u8databuf + 7);
        }

        // CamOsPrintf("===> gu8RspBuf[13,9,7] = (0x%02X)(0x%02X)(0x%02X)\n", u8BusMode,
        // _stSDMMCInfo[u8_slot].u8SD3DrvType, _stSDMMCInfo[u8_slot].u8SD3CurrLimit);

        //************************************** Set Funciton ***********************************************
        if (!(_stSDMMCInfo[u8Slot].u32OCR & BIT24_T)) // SD 2.0, SD3.0 => SD2.0
        {
            u8BusMode = u8BusMode & D_SDMMC_BUSMODE;
            if (u8BusMode & SD_MODE_HIGH_SPEED) // Support High Speed
            {
                _stSDMMCInfo[u8Slot].u8AccessMode = BUS_SPEED_HIGH_SPEED;
                //------------------------------------------------------------------------------------------------------------
                err = _sd_switch_func(u8Slot, 1, 0, _stSDMMCInfo[u8Slot].u8AccessMode, u8databuf); // Set Group 1
                //------------------------------------------------------------------------------------------------------------
                if (err)
                    return (U16_T)err;

                if ((*(u8databuf + 16) & 0xF) != _stSDMMCInfo[u8Slot].u8AccessMode)
                    CamOsPrintf("_[sdmmc_%u] Warning: Problem switching high bus speed mode!\n", u8Slot);
                else
                    _stSDMMCInfo[u8Slot].u32MaxClk = CLK_HIGH_SPEED;
            }
        }
        else // SD3.0
        {
            u8DrvType = _stSDMMCInfo[u8Slot].u8SD3DrvType & SD_DRIVER_STRENGTH_TYPE_B;

            if (u8DrvType & SD_DRIVER_STRENGTH_TYPE_B)
                _stSDMMCInfo[u8Slot].u8DrvStrength = UHS_B_DRV_TYPE;
            else if (u8DrvType & SD_DRIVER_STRENGTH_TYPE_A)
                _stSDMMCInfo[u8Slot].u8DrvStrength = UHS_A_DRV_TYPE;
            else if (u8DrvType & SD_DRIVER_STRENGTH_TYPE_C)
                _stSDMMCInfo[u8Slot].u8DrvStrength = UHS_C_DRV_TYPE;
            else if (u8DrvType & SD_DRIVER_STRENGTH_TYPE_D)
                _stSDMMCInfo[u8Slot].u8DrvStrength = UHS_D_DRV_TYPE;

            //------------------------------------------------------------------------------------------------------------
            err = _sd_switch_func(u8Slot, 1, 2, _stSDMMCInfo[u8Slot].u8DrvStrength, u8databuf); // Set Group 3
            //------------------------------------------------------------------------------------------------------------
            if (err)
                return (U16_T)err;

            if ((*(u8databuf + 15) & 0xF) != _stSDMMCInfo[u8Slot].u8DrvStrength)
                CamOsPrintf("_[sdmmc_%u] Warning: Problem switching drive strength!\n", u8Slot);

            u8CurrLimit = _stSDMMCInfo[u8Slot].u8SD3CurrLimit & SD_CURR_LMT_200;

            if (u8CurrLimit & SD_CURR_LMT_800)
                _stSDMMCInfo[u8Slot].u8CurrMax = UHS_800_CURR_LMT;
            else if (u8CurrLimit & SD_CURR_LMT_600)
                _stSDMMCInfo[u8Slot].u8CurrMax = UHS_600_CURR_LMT;
            else if (u8CurrLimit & SD_CURR_LMT_400)
                _stSDMMCInfo[u8Slot].u8CurrMax = UHS_400_CURR_LMT;
            else if (u8CurrLimit & SD_CURR_LMT_200)
                _stSDMMCInfo[u8Slot].u8CurrMax = UHS_200_CURR_LMT;

            //------------------------------------------------------------------------------------------------------------
            err = _sd_switch_func(u8Slot, 1, 3, _stSDMMCInfo[u8Slot].u8CurrMax, u8databuf); // Set Group 4
            //------------------------------------------------------------------------------------------------------------
            if (err)
                return (U16_T)err;
            if (((*(u8databuf + 15) >> 4) & 0xF) != _stSDMMCInfo[u8Slot].u8DrvStrength)
                CamOsPrintf("_[sdmmc_%u] Warning: Problem switching current limit!\n", u8Slot);

            u8BusMode = _stSDMMCInfo[u8Slot].u8SD3BusMode & D_SDMMC_BUSMODE;

            if (u8BusMode & SD_MODE_UHS_SDR104) // Support SDR 104
            {
                _stSDMMCInfo[u8Slot].u8AccessMode = BUS_SPEED_UHS_SDR104;
                u32MaxClk                         = CLK_SDR104_SPEED;
            }
            else if (u8BusMode & SD_MODE_UHS_DDR50) // Support DDR 50
            {
                _stSDMMCInfo[u8Slot].u8AccessMode = BUS_SPEED_UHS_DDR50;
                u32MaxClk                         = CLK_DDR50_SPEED;
            }
            else if (u8BusMode & SD_MODE_UHS_SDR50) // Support SDR 50
            {
                _stSDMMCInfo[u8Slot].u8AccessMode = BUS_SPEED_UHS_SDR50;
                u32MaxClk                         = CLK_SDR50_SPEED;
            }
            else if (u8BusMode & SD_MODE_UHS_SDR25) // Support SDR 25
            {
                _stSDMMCInfo[u8Slot].u8AccessMode = BUS_SPEED_UHS_SDR25;
                u32MaxClk                         = CLK_SDR25_SPEED;
            }
            else if (u8BusMode & SD_MODE_UHS_SDR12) // Support SDR 12
            {
                _stSDMMCInfo[u8Slot].u8AccessMode = BUS_SPEED_UHS_SDR12;
                u32MaxClk                         = CLK_SDR12_SPEED;
            }

            //------------------------------------------------------------------------------------------------------------
            err = _sd_switch_func(u8Slot, 1, 0, _stSDMMCInfo[u8Slot].u8AccessMode, u8databuf); // Set Group 1
            //------------------------------------------------------------------------------------------------------------
            if (err)
                return (U16_T)err;

            if ((*(u8databuf + 16) & 0xF) != _stSDMMCInfo[u8Slot].u8AccessMode)
                CamOsPrintf("_[sdmmc_%u] Warning: Problem switching bus speed mode!\n", u8Slot);
            else
                _stSDMMCInfo[u8Slot].u32MaxClk = u32MaxClk;
        }
    }

    return 0;
}

// sd card init cmd0,cmd8,acmd41,cmd2,cmd3,cmd9,cmd13,cmd7,acmd51,acmd6
int sd_card_init(U8_T u8Slot)
{
    struct msSt_SD_IOS SETSDIOS = {0, 0, 0, 0, 0, 0}; //{NULL};

    if (!MS_SD_Check_SDSlot(u8Slot))
    {
        CamOsPrintf("Sdmmc ERR: invalid slot number: %d !\n", u8Slot);
        return -1;
    }

    CamOsMutexLock(&sdmmc_init_mutex[u8Slot]);
    if (gb_SDCardInitFlag[u8Slot])
    {
        CamOsMutexUnlock(&sdmmc_init_mutex[u8Slot]);
        return 0;
    }

    gsData[u8Slot].sg = (struct scatterlist *)CamOsMemAlloc(sizeof(struct scatterlist));

    SETSDIOS.clock      = 400000;
    SETSDIOS.power_mode = MMC_POWER_OFF;
    SETSDIOS.bus_width  = MMC_BUS_WIDTH_1;
    SETSDIOS.timing     = MMC_TIMING_LEGACY;
    SETSDIOS.pad_volt   = SD_PAD_VOLT_330;
    MS_SD_SET_IOS(gp_mmc_priv[u8Slot], &SETSDIOS, TRUE);
    Hal_Timer_mDelay(30);
    SETSDIOS.power_mode = MMC_POWER_UP;
    MS_SD_SET_IOS(gp_mmc_priv[u8Slot], &SETSDIOS, FALSE);
    Hal_Timer_mDelay(10);
    SETSDIOS.power_mode = MMC_POWER_ON;
    MS_SD_SET_IOS(gp_mmc_priv[u8Slot], &SETSDIOS, FALSE);
    Hal_Timer_mDelay(10);

    if (sd_card_init_process(u8Slot))
        goto error;

    SDMMC_SwitchHighBus(u8Slot, gu8RspBuf[u8Slot]);

    // acmd6, ->4bit_mode
    if (gp_mmc_priv[u8Slot]->mmc_PMuxInfo.u8_busWidth == 4)
    {
        if (MMPF_SD_SendACommand(u8Slot, SD_SWITCH, 0x02, _stSDMMCInfo[u8Slot].u32RCAArg))
            CamOsPrintf("------------ SD ACMD 6 set bus mode error \r\n");
        else
            SETSDIOS.bus_width = MMC_BUS_WIDTH_4;
    }

    SETSDIOS.clock  = _stSDMMCInfo[u8Slot].u32MaxClk;
    SETSDIOS.timing = MMC_TIMING_UHS_SDR25;
    MS_SD_SET_IOS(gp_mmc_priv[u8Slot], &SETSDIOS, FALSE);

    gb_SDCardInitFlag[u8Slot] = TRUE;
    CamOsMutexUnlock(&sdmmc_init_mutex[u8Slot]);
    return 0;

error:
    SETSDIOS.power_mode = MMC_POWER_OFF;
    SETSDIOS.timing     = MMC_TIMING_LEGACY;
    MS_SD_SET_IOS(gp_mmc_priv[u8Slot], &SETSDIOS, TRUE);

    gb_SDCardInitFlag[u8Slot] = FALSE;
    CamOsMutexUnlock(&sdmmc_init_mutex[u8Slot]);
    return 1;
}

/// sd card reset
int sd_card_reset(U8_T u8Slot)
{
    struct msSt_SD_IOS SETSDIOS = {0, 0, 0, 0, 0, 0}; //{NULL};

    if (!MS_SD_Check_SDSlot(u8Slot))
    {
        CamOsPrintf("Sdmmc ERR: invalid slot number: %d !\n", u8Slot);
        return -1;
    }

    CamOsMutexLock(&sdmmc_init_mutex[u8Slot]);

    SETSDIOS.clock      = 400000;
    SETSDIOS.power_mode = MMC_POWER_ON;
    SETSDIOS.bus_width  = MMC_BUS_WIDTH_1;
    SETSDIOS.timing     = MMC_TIMING_LEGACY;
    SETSDIOS.pad_volt   = SD_PAD_VOLT_330;
    MS_SD_SET_IOS(gp_mmc_priv[u8Slot], &SETSDIOS, FALSE);
    // Hal_Timer_mDelay(10);

    if (sd_card_init_process(u8Slot))
        goto error;

    SDMMC_SwitchHighBus(u8Slot, gu8RspBuf[u8Slot]);

    // acmd6, ->4bit_mode
    if (MMPF_SD_SendACommand(u8Slot, SD_SWITCH, 0x02, _stSDMMCInfo[u8Slot].u32RCAArg))
        CamOsPrintf("------------ SD ACMD 6 set bus mode error \r\n");
    else
        SETSDIOS.bus_width = MMC_BUS_WIDTH_4;

    SETSDIOS.clock = 50000000;
    MS_SD_SET_IOS(gp_mmc_priv[u8Slot], &SETSDIOS, FALSE);

    gb_SDCardInitFlag[u8Slot] = TRUE;
    CamOsMutexUnlock(&sdmmc_init_mutex[u8Slot]);
    return 0;

error:
    SETSDIOS.power_mode = MMC_POWER_OFF;
    SETSDIOS.timing     = MMC_TIMING_LEGACY;
    MS_SD_SET_IOS(gp_mmc_priv[u8Slot], &SETSDIOS, TRUE);

    gb_SDCardInitFlag[u8Slot] = FALSE;
    CamOsMutexUnlock(&sdmmc_init_mutex[u8Slot]);
    return 1;
}

// cmd17, read signal block
int sd_read_block(U8_T u8Slot, u32 addr, volatile U8_T *r_buf)
{
    unsigned short ret;
    if (!MS_SD_Check_SDSlot(u8Slot))
    {
        CamOsPrintf("Sdmmc ERR: invalid slot number: %d !\n", u8Slot);
        return -1;
    }

    if (!gb_SDCardInitFlag[u8Slot])
    {
        if (sd_card_init(u8Slot))
        {
            CamOsPrintf("SD_%d card is not ready! \n", u8Slot);
            return 1;
        }
    }
    ret = MMPF_SD_SendDataCommand(u8Slot, MMC_READ_SINGLE_BLOCK, addr, 1, 512, r_buf);

    return ret ? 1 : 0;
}

// cmd24, write signal block
int sd_write_block(U8_T u8Slot, u32 addr, volatile U8_T *r_buf)
{
    unsigned short ret;
    if (!MS_SD_Check_SDSlot(u8Slot))
    {
        CamOsPrintf("Sdmmc ERR: invalid slot number: %d !\n", u8Slot);
        return -1;
    }

    if (!gb_SDCardInitFlag[u8Slot])
    {
        if (sd_card_init(u8Slot))
        {
            CamOsPrintf("SD_%d card is not ready! \n", u8Slot);
            return 1;
        }
    }
    ret = MMPF_SD_SendDataCommand(u8Slot, MMC_WRITE_BLOCK, addr, 1, 512, r_buf);

    return ret ? 1 : 0;
}

// cmd18, read multi block
int sd_read_block_multi(U8_T u8Slot, u32 addr, U16_T u16BlkCnt, volatile U8_T *r_buf)
{
    unsigned short ret;
    if (!MS_SD_Check_SDSlot(u8Slot))
    {
        CamOsPrintf("Sdmmc ERR: invalid slot number: %d !\n", u8Slot);
        return -1;
    }

    if (!gb_SDCardInitFlag[u8Slot])
    {
        if (sd_card_init(u8Slot))
        {
            CamOsPrintf("SD_%d card is not ready! \n", u8Slot);
            return 1;
        }
    }
    if (u16BlkCnt > 1)
        ret = MMPF_SD_SendDataCommand(u8Slot, MMC_READ_MULTIPLE_BLOCK, addr, u16BlkCnt, 512, r_buf);
    else if (u16BlkCnt == 1)
        ret = MMPF_SD_SendDataCommand(u8Slot, MMC_READ_SINGLE_BLOCK, addr, 1, 512, r_buf);
    else
    {
        CamOsPrintf("read_block_multi error:invlite blkcnt! \n");
        ret = 1;
    }

    return ret ? 1 : 0;
}

// cmd25, write multi block
int sd_write_block_multi(U8_T u8Slot, u32 addr, U16_T u16BlkCnt, volatile U8_T *r_buf)
{
    unsigned short ret;
    if (!MS_SD_Check_SDSlot(u8Slot))
    {
        CamOsPrintf("Sdmmc ERR: invalid slot number: %d !\n", u8Slot);
        return -1;
    }

    if (!gb_SDCardInitFlag[u8Slot])
    {
        if (sd_card_init(u8Slot))
        {
            CamOsPrintf("SD_%d card is not ready! \n", u8Slot);
            return 1;
        }
    }
    if (u16BlkCnt > 1)
        ret = MMPF_SD_SendDataCommand(u8Slot, MMC_WRITE_MULTIPLE_BLOCK, addr, u16BlkCnt, 512, r_buf);
    else if (u16BlkCnt == 1)
        ret = MMPF_SD_SendDataCommand(u8Slot, MMC_WRITE_BLOCK, addr, 1, 512, r_buf);
    else
    {
        CamOsPrintf("read_block_multi error:invlite blkcnt! \n");
        ret = 1;
    }

    return ret ? 1 : 0;
}

// get card capacity
U64_T sd_get_capacity(U8_T u8Slot)
{
    if (!MS_SD_Check_SDSlot(u8Slot))
    {
        CamOsPrintf("Sdmmc ERR: invalid slot number: %d !\n", u8Slot);
        return -1;
    }

    if (!gb_SDCardInitFlag[u8Slot])
    {
        if (sd_card_init(u8Slot))
        {
            CamOsPrintf("SD_%d card is not ready! \n", u8Slot);
            return 1;
        }
    }
    return _stSDMMCInfo[u8Slot].stCSD.CAPCITY;
}

// erase
int sd_erase_data(U8_T u8Slot, U32_T u32EraseDataStart, U32_T u32EraseDataEnd)
{
    U32_T u32Err;
    if (!MS_SD_Check_SDSlot(u8Slot))
    {
        CamOsPrintf("Sdmmc ERR: invalid slot number: %d !\n", u8Slot);
        return -1;
    }

    if (!gb_SDCardInitFlag[u8Slot])
    {
        if (sd_card_init(u8Slot))
        {
            CamOsPrintf("SD_%d card is not ready! \n", u8Slot);
            return 1;
        }
    }
    u32Err = MMPF_SD_SendCommand(u8Slot, SD_ERASE_WR_BLK_START, u32EraseDataStart);
    if (u32Err)
        goto exit;
    u32Err = MMPF_SD_SendCommand(u8Slot, SD_ERASE_WR_BLK_END, u32EraseDataEnd);
    if (u32Err)
        goto exit;
    u32Err = MMPF_SD_SendCommand(u8Slot, MMC_ERASE, 0);
    if (u32Err)
        goto exit;

    u32Err = MMPF_SD_SendCommand(u8Slot, MMC_SEND_STATUS, _stSDMMCInfo[u8Slot].u32RCAArg);
    if (u32Err)
        goto exit;

exit:
    return u32Err ? 1 : 0;
}
