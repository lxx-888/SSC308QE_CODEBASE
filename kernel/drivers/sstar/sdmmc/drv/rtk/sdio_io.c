/*
 * sdio_io.c- Sigmastar
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
 * FileName sdio_io.c
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
#include "sd.h"
#include "sdio.h"
#include "drv_sdmmc_rtk.h"
#include "drv_sdmmc_common.h"
#include "hal_sdmmc_platform_pri_config.h"

#if 1
/// SDIO command argument (in CMD)
#define RW_FLAG(x)    ((U32_T)x << 31)
#define FUN_NUM(x)    ((U32_T)x << 28)
#define RAW_FLAG(x)   ((U32_T)x << 27)
#define REG_ADD(x)    ((U32_T)x << 9)
#define WTITE_DATA(x) ((U32_T)x)
#define BLK_MODE(x)   ((U32_T)x << 27)
#define OP_CODE(x)    ((U32_T)x << 26)

/// SDIO return status (in DATA)
#define COM_CRC_ERROR   0x8000
#define ILLEGAL_COMMAND 0x4000
#define UNKNOW_ERROR    0x0800
#define INV_FUN_NUM     0x0200
#define OUT_OF_RANGE    0x0100

// MMPF_OS_SEMID mSDIOBusySemID[MMPF_SD_DEV_NUM];
// extern MMP_USHORT MMPF_SDIO_BusRelease(stSDMMCHandler *SDMMCArg);
// extern MMP_ERR MMPF_SDIO_BusAcquire(stSDMMCHandler *SDMMCArg);

/// Add for SDIO NONBLOCKING CPU mode (pending for Block Transfer Done interrupt)
//#define SDIO_CPU_NONBLOCKING (1)

// extern MMPF_OS_FLAGID SYS_Flag_Hif;
#endif /* (ENABLE_SDIO_FEATURE==1) */

#define DEFINE_SDIO_LOWSPEED  (400)
#define DEFINE_SDIO_FULLSPEED (25000)
#define DEFINE_SDIO_HIGHSPEED (50000)

#define SDIO_CLASS_NONE   0x00 /* Not a SDIO standard interface */
#define SDIO_CLASS_UART   0x01 /* standard UART interface */
#define SDIO_CLASS_BT_A   0x02 /* Type-A BlueTooth std interface */
#define SDIO_CLASS_BT_B   0x03 /* Type-B BlueTooth std interface */
#define SDIO_CLASS_GPS    0x04 /* GPS standard interface */
#define SDIO_CLASS_CAMERA 0x05 /* Camera standard interface */
#define SDIO_CLASS_PHS    0x06 /* PHS standard interface */
#define SDIO_CLASS_WLAN   0x07 /* WLAN interface */
#define SDIO_CLASS_ATA    0x08 /* Embedded SDIO-ATA std interface */
#define SDIO_CLASS_BT_AMP 0x09 /* Type-A Bluetooth AMP interface */

//
#define R4_18V_PRESENT    (1 << 24)
#define R4_MEMORY_PRESENT (1 << 27)

//
#define SDIO_MAX_BLOCK_COUNT 512
#define SDIO_MAX_BLOCK_SIZE  512

//
sdio_irq_callback *sdio_irq_cb[SDMMC_NUM_TOTAL] = {NULL};

//
struct mmc_command gsCmd[SDMMC_NUM_TOTAL];
struct mmc_data    gsData[SDMMC_NUM_TOTAL];
struct mmc_command gsCmdStop[SDMMC_NUM_TOTAL];
struct mmc_host    gsMMCHost[SDMMC_NUM_TOTAL];

struct msSt_mmc_sdio SDIO_info[SDMMC_NUM_TOTAL];

extern struct sstar_mmc_priv *gp_mmc_priv[SDMMC_NUM_TOTAL];

static const U8_T  speed_val[16] = {0, 10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80};
static const U32_T speed_unit[8] = {10000, 100000, 1000000, 10000000, 0, 0, 0, 0};

static inline int mmc_card_nonstd_func_interface(const struct msSt_mmc_sdio *c)
{
    return c->quirks & MMC_SDIO_QUIRK_NONSTD_FUNC_IF;
};

#define BUG_ON(cond)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define min(a, b)     ((a) > (b) ? (b) : (a))

static int _Cis_Tpl_Parse(struct msSt_mmc_sdio *ms_card, struct msSt_sdio_func *func, const char *tpl_descr,
                          const struct msSt_cis_tpl *tpl, int tpl_count, U8_T code, const U8_T *pu8_buf, unsigned size);

static int _Cistpl_Funce_Func(struct msSt_mmc_sdio *ms_card, struct msSt_sdio_func *func, const U8_T *pu8_buf,
                              unsigned size)
{
    U32_T vsn;
    U32_T min_size;

    if (!func)
        return -EINVAL;

    // This tuple has a different length depending on the SDIO spec version.

    vsn      = func->card->cccr.sdio_vsn;
    min_size = (vsn == SDIO_SDIO_SPEC_VER_1_00) ? 28 : 42;

    if (size == 28 && vsn == SDIO_SDIO_SPEC_VER_1_10)
    {
        CamOsPrintf("%s: ms_card has broken SDIO 1.1 CIS, forcing SDIO 1.0\n", __FUNCTION__);
        vsn = SDIO_SDIO_SPEC_VER_1_00;
    }
    else if (size < min_size)
    {
        return -EINVAL;
    }

    func->max_blksize = pu8_buf[12] | (pu8_buf[13] << 8);

    if (vsn > SDIO_SDIO_SPEC_VER_1_00)
        func->enable_timeout = (pu8_buf[28] | (pu8_buf[29] << 8)) * 10;
    else
        func->enable_timeout = 100000; // jiffies_to_msecs(HZ);

    return 0;
}

static int cistpl_funce_common(struct msSt_mmc_sdio *ms_card, struct msSt_sdio_func *func, const U8_T *pu8_buf,
                               unsigned size)
{
    /* Only valid for the common CIS (function 0) */
    if (func)
        return -EINVAL;

    /* TPLFE_FN0_BLK_SIZE */
    ms_card->cis.blksize = pu8_buf[1] | (pu8_buf[2] << 8);

    /* TPLFE_MAX_TRAN_SPEED */
    ms_card->cis.max_dtr = speed_val[(pu8_buf[3] >> 3) & 15] * speed_unit[pu8_buf[3] & 7];

    return 0;
}

static const struct msSt_cis_tpl ms_Cis_Tpl_Funce_List[] = {
    {0x00, 4, cistpl_funce_common},
    {0x01, 0, _Cistpl_Funce_Func},
    {0x04, 1 + 1 + 6, /* CISTPL_FUNCE_LAN_NODE_ID */},
};

static int _Cistpl_Vers_One(struct msSt_mmc_sdio *ms_card, struct msSt_sdio_func *func, const U8_T *pu8_buf,
                            unsigned size)
{
    unsigned unr_strings;
    char **  buffer, *string;
    U32_T    u32_i;

    pu8_buf += 2;
    size -= 2;

    unr_strings = 0;
    for (u32_i = 0; u32_i < size; u32_i++)
    {
        if (pu8_buf[u32_i] == 0xff)
            break;
        if (pu8_buf[u32_i] == 0)
            unr_strings++;
    }
    if (unr_strings == 0)
        return 0;

    size = u32_i;

    buffer = CamOsMemAlloc(sizeof(char *) * unr_strings + size);
    if (!buffer)
        return -ENOMEM;

    string = (char *)(buffer + unr_strings);

    for (u32_i = 0; u32_i < unr_strings; u32_i++)
    {
        buffer[u32_i] = string;
        strcpy(string, (const char *)pu8_buf);
        string += strlen((const char *)string) + 1;
        pu8_buf += strlen((const char *)pu8_buf) + 1;
    }

    if (func)
    {
        func->num_info = unr_strings;
        func->info     = (const char **)buffer;
    }
    else
    {
        ms_card->num_info = unr_strings;
        ms_card->info     = (const char **)buffer;
    }

    return 0;
}

static int cistpl_manfid(struct msSt_mmc_sdio *ms_card, struct msSt_sdio_func *func, const U8_T *pu8_buf, unsigned size)
{
    U32_T vendor, device;

    /* TPLMID_MANF */
    vendor = pu8_buf[0] | (pu8_buf[1] << 8);

    /* TPLMID_CARD */
    device = pu8_buf[2] | (pu8_buf[3] << 8);

    if (func)
    {
        func->vendor = vendor;
        func->device = device;
    }
    else
    {
        ms_card->cis.vendor = vendor;
        ms_card->cis.device = device;
    }

    return 0;
}

static int cistpl_funce(struct msSt_mmc_sdio *ms_card, struct msSt_sdio_func *func, const U8_T *pu8_buf, unsigned size)
{
    if (size < 1)
        return -EINVAL;

    return _Cis_Tpl_Parse(ms_card, func, "CISTPL_FUNCE", ms_Cis_Tpl_Funce_List, ARRAY_SIZE(ms_Cis_Tpl_Funce_List),
                          pu8_buf[0], pu8_buf, size);
}

static const struct msSt_cis_tpl ms_Cis_Tpl_List[] = {
    {0x15, 3, _Cistpl_Vers_One}, {0x20, 4, cistpl_manfid},         {0x21, 2, /* cistpl_funcid */},
    {0x22, 0, cistpl_funce},     {0x91, 2, /* cistpl_sdio_std */},
};

// -------------------------------------------------------------------------
static int MMPF_SDIO_SendCommand(U8_T u8Slot, U8_T command, U32_T argument)
{
    struct mmc_host *   p_mmc_host;
    struct mmc_request  gSDIONormalReq;
    struct mmc_request  gSDIODataRWReq;
    struct mmc_request *pSDReq;

    p_mmc_host         = &gsMMCHost[u8Slot];
    p_mmc_host->slotNo = u8Slot;

    if (SD_IO_RW_DIRECT == command)
    {
        gSDIONormalReq.cmd  = &gsCmd[u8Slot];
        gSDIONormalReq.data = NULL;
        pSDReq              = &gSDIONormalReq;
    }
    else
    {
        if (argument & 0x80000000)
            gsData[u8Slot].flags = MMC_DATA_WRITE;
        else
            gsData[u8Slot].flags = MMC_DATA_READ;

        gsData[u8Slot].error = 0;
        gSDIODataRWReq.cmd   = &gsCmd[u8Slot];
        gSDIODataRWReq.data  = &gsData[u8Slot];
        pSDReq               = &gSDIODataRWReq;
    }
    pSDReq->stop = NULL;
    pSDReq->sbc  = NULL;
    // gsCmd[u8Slot].u8Slot = u8Slot;
    gsCmd[u8Slot].opcode = command;
    gsCmd[u8Slot].arg    = argument; /* SD command's argument */
    gsCmd[u8Slot].flags  = MMC_RSP_R5;
    gsCmd[u8Slot].error  = 0;
    // gsData[u8Slot].pu8Buf = (volatile char   *)(MsVA2PA(m_ulSDDmaAddr[SDMMCArg->id] - 0x0400));

    sstar_sdmmc_request(p_mmc_host, pSDReq);

    return pSDReq->cmd->error ? pSDReq->cmd->error : pSDReq->data->error; // MMP_SD_ERR_COMMAND_FAILED;
}

U32_T MMPF_SD_GetCmdRspU32(U8_T u8Slot)
{
    return gsCmd[u8Slot].resp[0];
}

static int MMPF_SDIO_ReadReg(U8_T u8Slot, U8_T fun_num, U32_T reg_addr, U8_T *p_dst)
{
    unsigned short err;

#if 0
    if(MMP_ERR_NONE != MMPF_SDIO_BusAcquire(SDMMCArg)) {
        DBG_S(0, "MMPF_SDIO_EnableOutputClk aquire sem failed\r\n");
        return MMP_SD_ERR_BUSY;
    }
#endif

    err = MMPF_SDIO_SendCommand(u8Slot, SD_IO_RW_DIRECT,
                                (RW_FLAG(0) | FUN_NUM(fun_num) | RAW_FLAG(0) | REG_ADD(reg_addr) | 0));

    if (p_dst != NULL)
    {
        //*p_dst = ( (MMPF_SD_GetCmdRspU32(SDMMCArg->id)) & (0xFF) );//pSD->SD_RESP.D[3] & 0xFF;

        Hal_SDIO_GetRsp8U(gp_mmc_priv[u8Slot]->mmc_PMuxInfo.u8_ipOrder, p_dst);
        // Hal_SDIO_GetRsp8U(u8Slot, p_dst);
    }

#if 0
    MMPF_SDIO_BusRelease(SDMMCArg);
#endif

    if (err)
    {
        return -1;
    }

    return 0;
}

static int MMPF_SDIO_WriteReg(U8_T u8Slot, U8_T fun_num, U32_T reg_addr, U8_T src)
{
    unsigned short err;

#if 0
    if(MMP_ERR_NONE != MMPF_SDIO_BusAcquire(SDMMCArg)) {
        DBG_S(0, "MMPF_SDIO_EnableOutputClk aquire sem failed\r\n");
        return MMP_SD_ERR_BUSY;
    }
#endif

    err = MMPF_SDIO_SendCommand(u8Slot, SD_IO_RW_DIRECT,
                                (RW_FLAG(1) | FUN_NUM(fun_num) | RAW_FLAG(1) | REG_ADD(reg_addr) | src));

#if 0
    MMPF_SDIO_BusRelease(SDMMCArg);
#endif

    if (err)
    {
        return -1;
    }

    return 0;
}

static int sdio_read_cccr(U8_T u8Slot, struct msSt_mmc_sdio *ms_card, U32_T ocr)
{
    int  err;
    U8_T u8Data;
#if DIS_SDIO_SIMPLIFY
    int  SdioVsn;
    int  uhs = ocr & R4_18V_PRESENT;
    U8_T u8Speed;

    err = MMPF_SDIO_ReadReg(u8Slot, 0, SDIO_CCCR_SDIO_REV, &u8Data);
    if (err)
        return err;

    SdioVsn = u8Data & 0x0f;

    if (SdioVsn > SDIO_CCCR_SPEC_VER_3_00)
    {
        CamOsPrintf("%s: unrecognised CCCR structure version %d\n", __FUNCTION__, SdioVsn);
        return -EINVAL;
    }

    ms_card->cccr.sdio_vsn = (u8Data & 0xf0) >> 4;

    err = MMPF_SDIO_ReadReg(u8Slot, 0, SDIO_CCCR_CARD_CAPS, &u8Data);
    if (err)
        return err;

    if ((u8Data & SDIO_CCCR_CARD_CAP_LSC) != 0)
        ms_card->cccr.low_speed = 1;

    if ((u8Data & SDIO_CCCR_CARD_CAP_4BLS) != 0)
        ms_card->cccr.wide_bus = 1;

    if ((u8Data & SDIO_CCCR_CARD_CAP_SMB) != 0)
        ms_card->cccr.multi_block = 1;

    if (SdioVsn >= SDIO_CCCR_SPEC_VER_1_10)
    {
        err = MMPF_SDIO_ReadReg(u8Slot, 0, SDIO_CCCR_POWER_CTRL, &u8Data);
        if (err)
            return err;

        if ((u8Data & SDIO_POWER_CTRL_SMPC) != 0)
            ms_card->cccr.high_power = 1;
    }

    if (SdioVsn >= SDIO_CCCR_SPEC_VER_1_20)
    {
        err = MMPF_SDIO_ReadReg(u8Slot, 0, SDIO_CCCR_SPEED, &u8Speed);
        if (err)
            return err;

        ms_card->sw_caps.sd3BusMode = 0;
        ms_card->sw_caps.sd3DrvType = 0;
        ms_card->scr.sd_spec3       = 0;

        if (SdioVsn >= SDIO_CCCR_SPEC_VER_3_00 && uhs)
        {
            ms_card->scr.sd_spec3 = 1;
            err                   = MMPF_SDIO_ReadReg(u8Slot, 0, SDIO_CCCR_UHS, &u8Data);
            if (err)
                return err;

            err = MMPF_SDIO_ReadReg(u8Slot, 0, SDIO_CCCR_DRV_STRENGTH, &u8Data);
            if (err)
                return err;

            if ((u8Data & SDIO_DRIVE_SDTA) != 0)
                ms_card->sw_caps.sd3DrvType |= SD_DRIVER_STRENGTH_TYPE_A;

            if ((u8Data & SDIO_DRIVE_SDTC) != 0)
                ms_card->sw_caps.sd3DrvType |= SD_DRIVER_STRENGTH_TYPE_C;

            if ((u8Data & SDIO_DRIVE_SDTD) != 0)
                ms_card->sw_caps.sd3DrvType |= SD_DRIVER_STRENGTH_TYPE_D;
        }

        /* if no uhs mode ensure we check for high speed */
        if (!ms_card->sw_caps.sd3BusMode)
        {
            if ((u8Speed & SDIO_SPEED_SHS) != 0)
            {
                ms_card->sw_caps.hs_max_dtr = 50000000;
                ms_card->cccr.high_speed    = 1;
            }
            else
            {
                ms_card->sw_caps.hs_max_dtr = 25000000;
                ms_card->cccr.high_speed    = 0;
            }
        }
    }
#endif
    err = MMPF_SDIO_ReadReg(u8Slot, 0, SDIO_CCCR_IO_ENABLE, &u8Data);
    if (err)
        return err;

    err = MMPF_SDIO_ReadReg(u8Slot, 0, SDIO_CCCR_IO_READY, &u8Data);
    if (err)
        return err;
    ms_card->func_enable = u8Data;

    err = MMPF_SDIO_ReadReg(u8Slot, 0, SDIO_CCCR_FN0_BLKSIZE, &u8Data);
    if (err)
        return err;
    ms_card->curr_blksize = u8Data;

    err = MMPF_SDIO_ReadReg(u8Slot, 0, SDIO_CCCR_FN0_BLKSIZE + 1, &u8Data);
    if (err)
        return err;
    ms_card->curr_blksize += (u8Data << 8);

    return err;
}

static int _Cis_Tpl_Parse(struct msSt_mmc_sdio *ms_card, struct msSt_sdio_func *func, const char *tpl_descr,
                          const struct msSt_cis_tpl *tpl, int tpl_count, U8_T code, const U8_T *pu8_buf, unsigned size)
{
    int i, err;

    /* look for a matching code in the table */
    for (i = 0; i < tpl_count; i++, tpl++)
    {
        if (tpl->code == code)
            break;
    }
    if (i < tpl_count)
    {
        if (size >= tpl->min_size)
        {
            if (tpl->parse)
                err = tpl->parse(ms_card, func, pu8_buf, size);
            else
                err = -EILSEQ; /* known tuple, not parsed */
        }
        else
        {
            /* invalid tuple */
            err = -EINVAL;
        }
        if (err && err != -EILSEQ && err != -ENOENT)
        {
            CamOsPrintf("%s: bad %s tuple 0x%02x (%u bytes)\n", __FUNCTION__, tpl_descr, code, size);
        }
    }
    else
    {
        /* unknown tuple */
        err = -ENOENT;
    }

    return err;
}

static int sdio_read_cis(U8_T u8Slot, struct msSt_mmc_sdio *ms_card, struct msSt_sdio_func *ms_func)
{
    int                          error;
    struct msSt_sdio_func_tuple *this_fbr, **prev;
    unsigned                     i, ptr = 0;

    /*
     * Note that this works for the common CIS (function number 0) as
     * well as a function's CIS * since SDIO_CCCR_CIS and SDIO_FBR_POINT_CIS
     * have the same offset.
     */
    for (i = 0; i < 3; i++)
    {
        U8_T x, fn;

        if (ms_func)
            fn = ms_func->num;
        else
            fn = 0;

        error = MMPF_SDIO_ReadReg(u8Slot, 0, SDIO_FBR_BASE_ADDR(fn) + SDIO_FBR_CIS_POINT + i, &x);
        if (error)
            return error;
        ptr |= x << (i * 8);
    }

    if (ms_func)
        prev = &ms_func->tuples;
    else
        prev = &ms_card->tuples;

    BUG_ON(*prev);

    do
    {
        U8_T tpl_code, tpl_link;

        error = MMPF_SDIO_ReadReg(u8Slot, 0, ptr++, &tpl_code);
        if (error)
            break;

        /* 0xff means we're done */
        if (tpl_code == 0xff)
            break;

        /* null entries have no link field or data */
        if (tpl_code == 0x00)
            continue;

        error = MMPF_SDIO_ReadReg(u8Slot, 0, ptr++, &tpl_link);
        if (error)
            break;

        /* a size of 0xff also means we're done */
        if (tpl_link == 0xff)
            break;

        this_fbr = CamOsMemAlloc(sizeof(*this_fbr) + tpl_link);
        if (!this_fbr)
            return -ENOMEM;

        for (i = 0; i < tpl_link; i++)
        {
            error = MMPF_SDIO_ReadReg(u8Slot, 0, ptr + i, &this_fbr->data[i]);
            if (error)
                break;
        }
        if (error)
        {
            CamOsMemRelease(this_fbr);
            break;
        }

        error = _Cis_Tpl_Parse(ms_card, ms_func, "CIS", ms_Cis_Tpl_List, ARRAY_SIZE(ms_Cis_Tpl_List), tpl_code,
                               this_fbr->data, tpl_link);
        if (error == -EILSEQ || error == -ENOENT)
        {
            *prev          = this_fbr;
            prev           = &this_fbr->next;
            this_fbr->code = tpl_code;
            this_fbr->size = tpl_link;
            this_fbr->next = NULL;

            if (error == -ENOENT)
                CamOsPrintf("[%s] queuing unknown CIS tuple 0x%02x (%u bytes)\n", __FUNCTION__, tpl_code, tpl_link);

            error = 0;
        }
        else
            CamOsMemRelease(this_fbr);

        ptr += tpl_link;
    } while (!error);

    if (ms_func)
        *prev = ms_card->tuples;

    return error;
}

int sdio_read_common_cis(U8_T u8Slot, struct msSt_mmc_sdio *ms_card)
{
    return sdio_read_cis(u8Slot, ms_card, NULL);
}

static int sdio_read_fbr(U8_T u8Slot, struct msSt_sdio_func *func)
{
    int  err;
    U8_T u8Data;
#if DIS_SDIO_SIMPLIFY
    if (mmc_card_nonstd_func_interface(func->card))
    {
        func->class = SDIO_CLASS_NONE;
        return 0;
    }

    err = MMPF_SDIO_ReadReg(u8Slot, 0, SDIO_FBR_BASE_ADDR(func->num) + SDIO_FBR_STD_SDIO_FUNC_IF, &u8Data);
    if (err)
        return err;

    u8Data &= 0x0f;

    if (u8Data == 0x0f)
    {
        err = MMPF_SDIO_ReadReg(u8Slot, 0, SDIO_FBR_BASE_ADDR(func->num) + SDIO_FBR_EXT_STD_SDIO_FUNC_IF, &u8Data);
        if (err)
            return err;
    }

    func->class = u8Data;
#endif
    err = MMPF_SDIO_ReadReg(u8Slot, 0, SDIO_FBR_BASE_ADDR(func->num) + SDIO_FBR_IO_BLKSIZE, &u8Data);
    if (err)
        return err;
    func->cur_blksize = u8Data;

    err = MMPF_SDIO_ReadReg(u8Slot, 0, SDIO_FBR_BASE_ADDR(func->num) + SDIO_FBR_IO_BLKSIZE + 1, &u8Data);
    if (err)
        return err;
    func->cur_blksize += u8Data << 8;

    return err;
}

int sdio_read_func_cis(U8_T u8Slot, struct msSt_sdio_func *func)
{
    int err;

    err = sdio_read_cis(u8Slot, func->card, func);
    if (err)
        return err;

    /*
     * Since we've linked to tuples in the ms_card structure,
     * we must make sure we have a reference to it.
     */
    //  get_device(&func->card->dev);

    /*
     * Vendor/device id is optional for function CIS, so
     * copy it from the ms_card structure as needed.
     */
    if (func->vendor == 0)
    {
        func->vendor = func->card->cis.vendor;
        func->device = func->card->cis.device;
    }

    return 0;
}

struct msSt_sdio_func *sdio_alloc_func(struct msSt_mmc_sdio *ms_card)
{
    struct msSt_sdio_func *func;

    func = CamOsMemAlloc(sizeof(struct msSt_sdio_func));
    if (!func)
        return (void *)-ENOMEM;

    /*
     * allocate buffer separately to make sure it's properly aligned for
     * DMA usage (incl. 64 bit DMA)
     */
    func->tmpbuf = CamOsMemAlloc(4);
    if (!func->tmpbuf)
    {
        CamOsMemRelease(func);
        return (void *)-ENOMEM;
    }

    func->card = ms_card;

    return func;
}

static void sdio_remove_func(struct msSt_sdio_func *func)
{
    if (!func)
        return;

    if (func->tmpbuf)
        CamOsMemRelease(func->tmpbuf);

    CamOsMemRelease(func);
}

static int sdio_init_func(U8_T u8Slot, struct msSt_mmc_sdio *ms_card, U32_T fn)
{
    int                    err;
    struct msSt_sdio_func *func;

    BUG_ON(fn > SDIO_MAX_FUNCS);

    func = sdio_alloc_func(ms_card);
    if (!(func))
    {
        CamOsPrintf("func alloc failed!\n");
        return -ENOMEM;
    }

    func->num = fn;

    if (!(ms_card->quirks & MMC_SDIO_QUIRK_NONSTD_SDIO))
    {
        err = sdio_read_fbr(u8Slot, func);
        if (err)
        {
            CamOsPrintf("sdio func read fbr error: %x!\n", err);
            goto fail;
        }
#if DIS_SDIO_SIMPLIFY
        err = sdio_read_func_cis(u8Slot, func);
        if (err)
        {
            CamOsPrintf("sdio func read func cis error: %x!\n", err);
            goto fail;
        }
#endif
    }
    else
    {
        func->vendor      = func->card->cis.vendor;
        func->device      = func->card->cis.device;
        func->max_blksize = func->card->cis.blksize;
    }

    ms_card->sdio_func[fn - 1] = func;

    return 0;

fail:
    /*
     * It is okay to remove the function here even though we hold
     * the host lock as we haven't registered the device yet.
     */
    sdio_remove_func(func);
    return err;
}

// Sdio driver init
// static int sdio_init(void)
// {
//     return ms_sdmmc_probe();
// }

// Power Off/On, CMD5, CMD3, CMD7, Set Speed, Set bus width
int sdio_card_init(U8_T u8Slot)
{
    // U32_T ulSDclk = 0, divValue = 0;
    U8_T           u8Data, u8LowSpeed = 0, u8SetHSCnt = 10;
    unsigned short err = 0;

    struct msSt_SD_IOS SETSDIOS = {0, 0, 0, 0, 0, 0}; //{NULL};

    U32_T ulCardAddr = 0, count = 0, funcs, i;

    if (!MS_SD_Check_SDSlot(u8Slot))
    {
        CamOsPrintf("Sdmmc ERR: invalid slot number: %d !\n", u8Slot);
        return -1;
    }

    SETSDIOS.clock = 400000;
#if DIS_SDIO_SIMPLIFY
    SETSDIOS.power_mode = MMC_POWER_OFF;
#endif
    SETSDIOS.bus_width = MMC_BUS_WIDTH_1;
    SETSDIOS.timing    = MMC_TIMING_LEGACY;
    SETSDIOS.pad_volt  = SD_PAD_VOLT_330;
    MS_SD_SET_IOS(gp_mmc_priv[u8Slot], &SETSDIOS, TRUE);
    SETSDIOS.power_mode = MMC_POWER_UP;
    MS_SD_SET_IOS(gp_mmc_priv[u8Slot], &SETSDIOS, FALSE);
    SETSDIOS.power_mode = MMC_POWER_ON;
    MS_SD_SET_IOS(gp_mmc_priv[u8Slot], &SETSDIOS, FALSE);

#if 1
    err = MMPF_SDIO_ReadReg(u8Slot, 0, 0x06, &u8Data);
    if (err)
    {
        CamOsPrintf("------------ sdio CMD 52 get reset warning \r\n");
    }

    err = MMPF_SDIO_WriteReg(u8Slot, 0, 0x06, u8Data | 0x08);
    if (err)
    {
        CamOsPrintf("------------ sdio CMD 52 set reset warning \r\n");
    }
#endif

    // CMD 5
    err = MMPF_SD_SendCommand(u8Slot, SD_IO_SEND_OP_COND, 0);
    if (err)
    {
        CamOsPrintf("[SDIO]%s line:%d - ERR, CMD 5 usErr = %d\n", __FUNCTION__, __LINE__, err);
        goto ERR_HANDLE;
    }

    SDIO_info[u8Slot].ocr = MMPF_SD_GetCmdRspU32(u8Slot);
    // CMD 5
    do
    {
        err = MMPF_SD_SendCommand(u8Slot, SD_IO_SEND_OP_COND, SDIO_info[u8Slot].ocr);
        if (err)
        {
            CamOsPrintf("[SDIO]%s line:%d - ERR, CMD 5 usErr = %d\n", __FUNCTION__, __LINE__, err);
            goto ERR_HANDLE;
        }

        SDIO_info[u8Slot].ocr = MMPF_SD_GetCmdRspU32(u8Slot);
#if DIS_SDIO_SIMPLIFY
        CamOsUsSleep(10);
#endif
    } while (!(SDIO_info[u8Slot].ocr & BIT31_T) && count++ < 100);

    // CMD 3 (I/O State:idle->stby)
    err = MMPF_SD_SendCommand(u8Slot, SD_SEND_RELATIVE_ADDR, 0);
    if (err)
    {
        CamOsPrintf("[SDIO]%s line:%d - ERR, CMD 3 usErr = %d\n", __FUNCTION__, __LINE__, err);
        goto ERR_HANDLE;
    }
    //
    ulCardAddr            = MMPF_SD_GetCmdRspU32(u8Slot);
    SDIO_info[u8Slot].rca = ulCardAddr;

    // CMD 7 with the RCA. (I/O State:stby->cmd)
    err = MMPF_SD_SendCommand(u8Slot, MMC_SELECT_CARD, ulCardAddr);
    if (err)
    {
        CamOsPrintf("[SDIO]%s line:%d - ERR, CMD 7 usErr = %d\n", __FUNCTION__, __LINE__, err);
        goto ERR_HANDLE;
    }

    // Start of initializing normal operation parameters
    u8Data = 0;
    err    = MMPF_SDIO_ReadReg(u8Slot, 0, SDIO_CCCR_CARD_CAPS, &u8Data);
    if (err)
    {
        CamOsPrintf("[SDIO]%s line:%d - ERR, usErr = %d\n", __FUNCTION__, __LINE__, err);
        goto ERR_HANDLE;
    }

    if (u8Data & SDIO_CCCR_CARD_CAP_LSC)
    {
        // LowSpeed
        u8LowSpeed = 1;
    }

#if SDIO_FORCE_1_BIT_MODE
    //
#else
    if ((u8Data & (SDIO_CCCR_CARD_CAP_LSC | SDIO_CCCR_CARD_CAP_4BLS)) == SDIO_CCCR_CARD_CAP_LSC)
    {
        // only support 1 bit mode
    }
    else
    {
        // changing DATA bus width on both Host & Device sides
        err = MMPF_SDIO_ReadReg(u8Slot, 0, SDIO_CCCR_BUS_IF_CTRL, &u8Data);
        if (err)
        {
            CamOsPrintf("[SDIO]%s line:%d - ERR, usErr = %d\n", __FUNCTION__, __LINE__, err);
            goto ERR_HANDLE;
        }

        err = MMPF_SDIO_WriteReg(u8Slot, 0, SDIO_CCCR_BUS_IF_CTRL,
                                 (u8Data & (~SDIO_BUS_WIDTH_MASK)) | SDIO_BUS_WIDTH_4BIT); // select 4-bit bus
        if (err)
        {
            CamOsPrintf("[SDIO]%s line:%d - ERR, usErr = %d\n", __FUNCTION__, __LINE__, err);
            goto ERR_HANDLE;
        }

        SETSDIOS.bus_width = MMC_BUS_WIDTH_4;
        MS_SD_SET_IOS(gp_mmc_priv[u8Slot], &SETSDIOS, FALSE);
    }
#endif

    if (u8LowSpeed)
    {
        //
    }
    else
    {
        // changing DATA bus speed on both Host & Device sides
        err = MMPF_SDIO_ReadReg(u8Slot, 0, SDIO_CCCR_SPEED, &u8Data);

        if ((u8Data & 0x01) == 0)
        {
            SETSDIOS.clock = DEFINE_SDIO_FULLSPEED * 1000;
            MS_SD_SET_IOS(gp_mmc_priv[u8Slot], &SETSDIOS, FALSE);
        }
        else
        {
            err = MMPF_SDIO_WriteReg(u8Slot, 0, SDIO_CCCR_SPEED,
                                     (u8Data & (~SDIO_SPEED_EHS)) | SDIO_SPEED_EHS); // select HIGH SPEED
            if (err)
            {
                CamOsPrintf("[SDIO]%s line:%d - ERR, usErr = %d\n", __FUNCTION__, __LINE__, err);
                goto ERR_HANDLE;
            }
            do
            {
                err = MMPF_SDIO_ReadReg(u8Slot, 0, SDIO_CCCR_SPEED, &u8Data);
                if (err)
                {
                    CamOsPrintf("[SDIO]%s line:%d - ERR, usErr = %d\n", __FUNCTION__, __LINE__, err);
                    goto ERR_HANDLE;
                }

                if ((u8Data & (SDIO_SPEED_EHS | SDIO_SPEED_SHS)) == (SDIO_SPEED_EHS | SDIO_SPEED_SHS))
                {
                    SETSDIOS.clock = DEFINE_SDIO_HIGHSPEED * 1000;
                    MS_SD_SET_IOS(gp_mmc_priv[u8Slot], &SETSDIOS, FALSE);
                    break;
                }
                u8SetHSCnt--;
#if DIS_SDIO_SIMPLIFY
                CamOsUsSleep(10);
#endif
            } while (u8SetHSCnt);
        }
    }

    // CMD 52 for CCCR
    err = sdio_read_cccr(u8Slot, &SDIO_info[u8Slot], SDIO_info[u8Slot].ocr);
    if (err)
    {
        CamOsPrintf("[SDIO]%s line:%d - ERR, read cccr usErr = %d\n", __FUNCTION__, __LINE__, err);
        goto ERR_HANDLE;
    }
#if DIS_SDIO_SIMPLIFY
    err = sdio_read_common_cis(u8Slot, &SDIO_info[u8Slot]);
    if (err)
    {
        CamOsPrintf("[SDIO]%s line:%d - ERR, read common cis usErr = %d\n", __FUNCTION__, __LINE__, err);
        goto ERR_HANDLE;
    }
#endif

    funcs                        = (SDIO_info[u8Slot].ocr & 0x70000000) >> 28;
    SDIO_info[u8Slot].sdio_funcs = 0;
    /*
     * Initialize (but don't add) all present functions.
     */
    for (i = 0; i < funcs; i++, SDIO_info[u8Slot].sdio_funcs++)
    {
        err = sdio_init_func(u8Slot, &SDIO_info[u8Slot], i + 1);
        if (err)
            goto ERR_HANDLE;
        /*
         * Enable Runtime PM for this func (if supported)
         */
    }

    return 0;

ERR_HANDLE:

    return -1;
}

// CMD5, CMD3, CMD7, Set Speed, Set bus width
int sdio_card_reset(U8_T u8Slot)
{
    // U32_T ulSDclk = 0, divValue = 0;
    U8_T           u8Data, LowSpeed = 0, SetHSCnt = 10;
    unsigned short err = 0;

    struct msSt_SD_IOS SETSDIOS = {0, 0, 0, 0, 0, 0}; //{NULL};

    U32_T ulCardAddr = 0;

    if (!MS_SD_Check_SDSlot(u8Slot))
    {
        CamOsPrintf("Sdmmc ERR: invalid slot number: %d !\n", u8Slot);
        return -1;
    }

    SETSDIOS.clock      = 400000;
    SETSDIOS.power_mode = MMC_POWER_ON; // Keep power on
    SETSDIOS.bus_width  = MMC_BUS_WIDTH_1;
    SETSDIOS.timing     = MMC_TIMING_LEGACY; // Be the same as sdio_card_init()
    SETSDIOS.pad_volt   = SD_PAD_VOLT_330;   // Be the same as sdio_card_init()
    MS_SD_SET_IOS(gp_mmc_priv[u8Slot], &SETSDIOS, FALSE);

    //

#if 1
    // CMD 0, it is not necesary for IO Only card.
    err = MMPF_SD_SendCommand(u8Slot, MMC_GO_IDLE_STATE, 0);
    if (err)
    {
        CamOsPrintf("------------ sdio CMD 0 error \r\n");
        goto ERR_HANDLE;
    }
#endif

    // CMD 5
    err = MMPF_SD_SendCommand(u8Slot, SD_IO_SEND_OP_COND, 0);
    if (err)
    {
        CamOsPrintf("[SDIO]%s line:%d - ERR, CMD 5 usErr = %d\n", __FUNCTION__, __LINE__, err);
        goto ERR_HANDLE;
    }

    // CMD 5
    err = MMPF_SD_SendCommand(u8Slot, SD_IO_SEND_OP_COND, 0x300000);
    if (err)
    {
        CamOsPrintf("[SDIO]%s line:%d - ERR, CMD 5 usErr = %d\n", __FUNCTION__, __LINE__, err);
        goto ERR_HANDLE;
    }

    // CMD 3 (I/O State:idle->stby)
    err = MMPF_SD_SendCommand(u8Slot, SD_SEND_RELATIVE_ADDR, 0);
    if (err)
    {
        CamOsPrintf("[SDIO]%s line:%d - ERR, CMD 3 usErr = %d\n", __FUNCTION__, __LINE__, err);
        goto ERR_HANDLE;
    }
    //
    ulCardAddr = MMPF_SD_GetCmdRspU32(u8Slot);

    // CMD 7 with the RCA. (I/O State:stby->cmd)
    err = MMPF_SD_SendCommand(u8Slot, MMC_SELECT_CARD, ulCardAddr);
    if (err)
    {
        CamOsPrintf("[SDIO]%s line:%d - ERR, CMD 7 usErr = %d\n", __FUNCTION__, __LINE__, err);
        goto ERR_HANDLE;
    }

    // Start of initializing normal operation parameters
    u8Data = 0;
    err    = MMPF_SDIO_ReadReg(u8Slot, 0, 0x08, &u8Data);
    if (err)
    {
        CamOsPrintf("[SDIO]%s line:%d - ERR, usErr = %d\n", __FUNCTION__, __LINE__, err);
        goto ERR_HANDLE;
    }

    if (u8Data & 0x40)
    {
        // LowSpeed
        LowSpeed = 1;
    }

#if SDIO_FORCE_1_BIT_MODE
    //
#else
    if ((u8Data & 0xC0) == 0x40)
    {
        // only support 1 bit mode
    }
    else
    {
        // changing DATA bus width on both Host & Device sides
        err = MMPF_SDIO_ReadReg(u8Slot, 0, 0x07, &u8Data);
        if (err)
        {
            CamOsPrintf("[SDIO]%s line:%d - ERR, usErr = %d\n", __FUNCTION__, __LINE__, err);
            goto ERR_HANDLE;
        }

        err = MMPF_SDIO_WriteReg(u8Slot, 0, 0x07, (u8Data & 0xfc) | 0x02); // select 4-bit bus
        if (err)
        {
            CamOsPrintf("[SDIO]%s line:%d - ERR, usErr = %d\n", __FUNCTION__, __LINE__, err);
            goto ERR_HANDLE;
        }

        SETSDIOS.bus_width = MMC_BUS_WIDTH_4;
        MS_SD_SET_IOS(gp_mmc_priv[u8Slot], &SETSDIOS, FALSE);
    }
#endif

    if (LowSpeed)
    {
        //
    }
    else
    {
        // changing DATA bus speed on both Host & Device sides
        err = MMPF_SDIO_ReadReg(u8Slot, 0, 0x13, &u8Data);

        if ((u8Data & 0x01) == 0)
        {
            SETSDIOS.clock = DEFINE_SDIO_FULLSPEED * 1000;
            MS_SD_SET_IOS(gp_mmc_priv[u8Slot], &SETSDIOS, FALSE);
        }
        else
        {
            do
            {
                err = MMPF_SDIO_WriteReg(u8Slot, 0, 0x13, (u8Data & 0xFD) | 0x02); // select HIGH SPEED
                if (err)
                {
                    CamOsPrintf("[SDIO]%s line:%d - ERR, usErr = %d\n", __FUNCTION__, __LINE__, err);
                    goto ERR_HANDLE;
                }

                err = MMPF_SDIO_ReadReg(u8Slot, 0, 0x13, &u8Data);
                if (err)
                {
                    CamOsPrintf("[SDIO]%s line:%d - ERR, usErr = %d\n", __FUNCTION__, __LINE__, err);
                    goto ERR_HANDLE;
                }

                if ((u8Data & 0x03) == 0x03)
                {
                    SETSDIOS.clock = DEFINE_SDIO_HIGHSPEED * 1000;
                    MS_SD_SET_IOS(gp_mmc_priv[u8Slot], &SETSDIOS, FALSE);
                    break;
                }
                SetHSCnt--;

                CamOsMsSleep(1);
            } while (SetHSCnt);
        }
    }

    return 0;

ERR_HANDLE:

    return -1;
}

int sdio_enable_func(U8_T u8Slot, struct msSt_sdio_func *func)
{
    int  err, i = 0;
    U8_T reg;
    //  unsigned long timeout;

    if (!func)
        return -EINVAL;

    if ((func->card->func_enable) & (1 << func->num))
    {
        return 0;
    }

    err = MMPF_SDIO_ReadReg(u8Slot, 0, SDIO_CCCR_IO_ENABLE, &reg);
    if (err)
        goto err;

    reg |= 1 << func->num;
    err = MMPF_SDIO_WriteReg(u8Slot, 0, SDIO_CCCR_IO_ENABLE, reg);
    if (err)
        goto err;

    while (1)
    {
        err = MMPF_SDIO_ReadReg(u8Slot, 0, SDIO_CCCR_IO_READY, &reg);
        if (err)
            goto err;
        if (reg & (1 << func->num))
            break;
        /*
        u32_Err = -ETIME;
        if (time_after(jiffies, timeout))
            goto err;
            */
        if (i++ > 100)
            return -1;
        CamOsMsSleep(10);
    }
    func->card->func_enable |= (1 << func->num);
    // r_debug("SDIO: Enabled device %s\n", sdio_func_id(func));
    return 0;

err:
    CamOsPrintf("SDIO: Failed to enable device %d\n", func->num);
    return err;
}

int sdio_set_block_size(U8_T u8Slot, struct msSt_sdio_func *func, U32_T blksz, U8_T u8FuncNum)
{
    int err;

    err = MMPF_SDIO_WriteReg(u8Slot, 0, SDIO_FBR_BASE_ADDR(u8FuncNum) + SDIO_FBR_IO_BLKSIZE, blksz & 0xff);
    if (err)
        return err;
    err = MMPF_SDIO_WriteReg(u8Slot, 0, SDIO_FBR_BASE_ADDR(u8FuncNum) + SDIO_FBR_IO_BLKSIZE + 1, (blksz >> 8) & 0xff);
    if (err)
        return err;

    if (u8FuncNum)
        func->cur_blksize = blksz;
    else
        SDIO_info[u8Slot].curr_blksize = blksz;

    return 0;
}

// Read single byte
int sdio_read_byte(U8_T u8Slot, U8_T func, U32_T addr, U8_T *r_buf)
{
    unsigned short err;

    if (!MS_SD_Check_SDSlot(u8Slot))
    {
        CamOsPrintf("Sdmmc ERR: invalid slot number: %d !\n", u8Slot);
        return -1;
    }

    if (func < 0 || func > SDIO_info[u8Slot].sdio_funcs)
    {
        CamOsPrintf("[SDIO]%s - ERROR: function %d is out of range, and total is %d !\n", __FUNCTION__, func,
                    SDIO_info[u8Slot].sdio_funcs);
        return -1;
    }

#if 0
    if(MMP_ERR_NONE != MMPF_SDIO_BusAcquire(SDMMCArg)) {
        DBG_S(0, "MMPF_SDIO_EnableOutputClk aquire sem failed\r\n");
        return MMP_SD_ERR_BUSY;
    }
#endif

    err =
        MMPF_SDIO_SendCommand(u8Slot, SD_IO_RW_DIRECT, (RW_FLAG(0) | FUN_NUM(func) | RAW_FLAG(0) | REG_ADD(addr) | 0));

    if (r_buf != NULL)
    {
        Hal_SDIO_GetRsp8U(gp_mmc_priv[u8Slot]->mmc_PMuxInfo.u8_ipOrder, r_buf);
        // Hal_SDIO_GetRsp8U(u8_slot, r_buf);
    }

#if 0
    MMPF_SDIO_BusRelease(SDMMCArg);
#endif

    if (err)
    {
        return -1;
    }

    return 0;
}

// Write single byte
int sdio_write_byte(U8_T u8Slot, U8_T func, U32_T addr, U8_T w_value)
{
    unsigned short err;

    if (!MS_SD_Check_SDSlot(u8Slot))
    {
        CamOsPrintf("Sdmmc ERR: invalid slot number: %d !\n", u8Slot);
        return -1;
    }

    if (func < 0 || func > SDIO_info[u8Slot].sdio_funcs)
    {
        CamOsPrintf("[SDIO]%s - ERROR: function %d is out of range, and total is %d !\n", __FUNCTION__, func,
                    SDIO_info[u8Slot].sdio_funcs);
        return -1;
    }
#if 0
    if(MMP_ERR_NONE != MMPF_SDIO_BusAcquire(SDMMCArg)) {
        DBG_S(0, "MMPF_SDIO_EnableOutputClk aquire sem failed\r\n");
        return MMP_SD_ERR_BUSY;
    }
#endif

    err = MMPF_SDIO_SendCommand(u8Slot, SD_IO_RW_DIRECT,
                                (RW_FLAG(1) | FUN_NUM(func) | RAW_FLAG(0) | REG_ADD(addr) | w_value));

#if 0
    MMPF_SDIO_BusRelease(SDMMCArg);
#endif

    if (err)
    {
        return -1;
    }

    return 0;
}

// Read multiple bytes, blk_mode=0
int sdio_read_byte_multi(U8_T u8Slot, U8_T func, U8_T op_code, U32_T addr, U32_T count, U8_T *r_buf)
{
    unsigned short err;

    if (!MS_SD_Check_SDSlot(u8Slot))
    {
        CamOsPrintf("Sdmmc ERR: invalid slot number: %d !\n", u8Slot);
        return -1;
    }

    if (func < 0 || func > SDIO_info[u8Slot].sdio_funcs)
    {
        CamOsPrintf("[SDIO]%s - ERROR: function %d is out of range, and total is %d !\n", __FUNCTION__, func,
                    SDIO_info[u8Slot].sdio_funcs);
        return -1;
    }
    // 1 ~ 512
    if ((count < 1) || (count > SDIO_MAX_BLOCK_COUNT))
    {
        CamOsPrintf("[SDIO]%s - ERR, count = %d\n", __FUNCTION__, count);
        return -1;
    }

#if 0
    if(MMP_ERR_NONE != MMPF_SDIO_BusAcquire(SDMMCArg)) {
        DBG_S(0, "MMPF_SDIO_EnableOutputClk aquire sem failed\r\n");
        return MMP_SD_ERR_BUSY;
    }
#endif

    //
    if (count == SDIO_MAX_BLOCK_COUNT)
    {
        count = 0;
    }
    if (func && (func <= SDIO_info[u8Slot].sdio_funcs))
    {
        if (sdio_enable_func(u8Slot, SDIO_info[u8Slot].sdio_func[func - 1]))
            return -1;
    }
    //
    gsData[u8Slot].blksz      = count;
    gsData[u8Slot].blocks     = 1;
    gsData[u8Slot].sg->length = 1 * count;
    gsData[u8Slot].sg_len     = 1;
    gsData[u8Slot].pu8Buf     = (volatile char *)r_buf;

    err = MMPF_SDIO_SendCommand(u8Slot, SD_IO_RW_EXTENDED,
                                (RW_FLAG(0) | FUN_NUM(func) | BLK_MODE(0) | OP_CODE(op_code) | REG_ADD(addr) | count));

    if (err)
    {
        return -1;
    }

    return 0;
}

// Write multiple bytes, blk_mode=0
int sdio_write_byte_multi(U8_T u8Slot, U8_T func, U8_T op_code, U32_T addr, U32_T count, U8_T *w_buf)
{
    unsigned short err;

    if (!MS_SD_Check_SDSlot(u8Slot))
    {
        CamOsPrintf("Sdmmc ERR: invalid slot number: %d !\n", u8Slot);
        return -1;
    }

    if (func < 0 || func > SDIO_info[u8Slot].sdio_funcs)
    {
        CamOsPrintf("[SDIO]%s - ERROR: function %d is out of range, and total is %d !\n", __FUNCTION__, func,
                    SDIO_info[u8Slot].sdio_funcs);
        return -1;
    }
    // 1 ~ 512
    if ((count < 1) || (count > SDIO_MAX_BLOCK_COUNT))
    {
        CamOsPrintf("[SDIO]%s - ERR, count = %d\n", __FUNCTION__, count);
        return -1;
    }

#if 0
    if(MMP_ERR_NONE != MMPF_SDIO_BusAcquire(SDMMCArg)) {
        DBG_S(0, "MMPF_SDIO_EnableOutputClk aquire sem failed\r\n");
        return MMP_SD_ERR_BUSY;
    }
#endif

    //
    if (count == SDIO_MAX_BLOCK_COUNT)
    {
        count = 0;
    }
    if (func && (func <= SDIO_info[u8Slot].sdio_funcs))
    {
        if (sdio_enable_func(u8Slot, SDIO_info[u8Slot].sdio_func[func - 1]))
            return -1;
    }

    //
    gsData[u8Slot].blksz      = count;
    gsData[u8Slot].blocks     = 1;
    gsData[u8Slot].sg->length = 1 * count;
    gsData[u8Slot].sg_len     = 1;
    gsData[u8Slot].pu8Buf     = (volatile char *)w_buf;

    err = MMPF_SDIO_SendCommand(u8Slot, SD_IO_RW_EXTENDED,
                                (RW_FLAG(1) | FUN_NUM(func) | BLK_MODE(0) | OP_CODE(op_code) | REG_ADD(addr) | count));

    return err;
}

// Read multiple blocks, blk_mode=1
int sdio_read_block_multi(U8_T u8Slot, U8_T func, U8_T op_code, U32_T addr, U32_T count, U32_T blk_size, U8_T *r_buf)
{
    unsigned short err;

    if (!MS_SD_Check_SDSlot(u8Slot))
    {
        CamOsPrintf("Sdmmc ERR: invalid slot number: %d !\n", u8Slot);
        return -1;
    }

    if (func < 0 || func > SDIO_info[u8Slot].sdio_funcs)
    {
        CamOsPrintf("[SDIO]%s - ERROR: function %d is out of range, and total is %d !\n", __FUNCTION__, func,
                    SDIO_info[u8Slot].sdio_funcs);
        return -1;
    }
    // 1 ~ 511
    if ((count < 1) || (count > (SDIO_MAX_BLOCK_COUNT - 1)))
    {
        CamOsPrintf("[SDIO]%s - ERR, count = %d\n", __FUNCTION__, count);
        return -1;
    }

    // 1 ~ 512
    if ((blk_size < 1) || (blk_size > SDIO_MAX_BLOCK_SIZE))
    {
        CamOsPrintf("[SDIO]%s - ERR, blk_size = %d\n", __FUNCTION__, blk_size);
        return -1;
    }

#if 0
    if(MMP_ERR_NONE != MMPF_SDIO_BusAcquire(SDMMCArg)) {
        DBG_S(0, "MMPF_SDIO_EnableOutputClk aquire sem failed\r\n");
        return MMP_SD_ERR_BUSY;
    }
#endif
    if (func && (func <= SDIO_info[u8Slot].sdio_funcs))
    {
        if (sdio_enable_func(u8Slot, SDIO_info[u8Slot].sdio_func[func - 1]))
            return -1;
    }
    if ((func && (func <= SDIO_info[u8Slot].sdio_funcs)
         && (blk_size != SDIO_info[u8Slot].sdio_func[func - 1]->cur_blksize))
        || ((func == 0) && blk_size != SDIO_info[u8Slot].curr_blksize)) // func 0
    {
        if (sdio_set_block_size(u8Slot, SDIO_info[u8Slot].sdio_func[func - 1], blk_size, func))
            return -1;
    }

    gsData[u8Slot].blksz      = blk_size;
    gsData[u8Slot].blocks     = count;
    gsData[u8Slot].sg->length = blk_size * count;
    gsData[u8Slot].sg_len     = 1;
    gsData[u8Slot].pu8Buf     = (volatile char *)r_buf;

    err = MMPF_SDIO_SendCommand(u8Slot, SD_IO_RW_EXTENDED,
                                (RW_FLAG(0) | FUN_NUM(func) | BLK_MODE(1) | OP_CODE(op_code) | REG_ADD(addr) | count));

    if (err)
    {
        return -1;
    }

    return 0;
}

// Write multiple blocks, blk_mode=1
int sdio_write_block_multi(U8_T u8Slot, U8_T func, U8_T op_code, U32_T addr, U32_T count, U32_T blk_size, U8_T *w_buf)
{
    unsigned short err;

    if (!MS_SD_Check_SDSlot(u8Slot))
    {
        CamOsPrintf("Sdmmc ERR: invalid slot number: %d !\n", u8Slot);
        return -1;
    }

    if (func < 0 || func > SDIO_info[u8Slot].sdio_funcs)
    {
        CamOsPrintf("[SDIO]%s - ERROR: function %d is out of range, and total is %d !\n", __FUNCTION__, func,
                    SDIO_info[u8Slot].sdio_funcs);
        return -1;
    }
    // 1 ~ 511
    if ((count < 1) || (count > (SDIO_MAX_BLOCK_COUNT - 1)))
    {
        CamOsPrintf("[SDIO]%s - ERR, count = %d\n", __FUNCTION__, count);
        return -1;
    }

    // 1 ~ 512
    if ((blk_size < 1) || (blk_size > SDIO_MAX_BLOCK_SIZE))
    {
        CamOsPrintf("[SDIO]%s - ERR, blk_size = %d\n", __FUNCTION__, blk_size);
        return -1;
    }

#if 0
    if(MMP_ERR_NONE != MMPF_SDIO_BusAcquire(SDMMCArg)) {
        DBG_S(0, "MMPF_SDIO_EnableOutputClk aquire sem failed\r\n");
        return MMP_SD_ERR_BUSY;
    }
#endif
    if (func && (func <= SDIO_info[u8Slot].sdio_funcs))
    {
        if (sdio_enable_func(u8Slot, SDIO_info[u8Slot].sdio_func[func - 1]))
            return -1;
    }
    if ((func && (func <= SDIO_info[u8Slot].sdio_funcs)
         && (blk_size != SDIO_info[u8Slot].sdio_func[func - 1]->cur_blksize))
        || ((func == 0) && blk_size != SDIO_info[u8Slot].curr_blksize)) // func 0
    {
        if (sdio_set_block_size(u8Slot, SDIO_info[u8Slot].sdio_func[func - 1], blk_size, func))
            return -1;
    }
    gsData[u8Slot].blksz      = blk_size;
    gsData[u8Slot].blocks     = count;
    gsData[u8Slot].sg->length = blk_size * count;
    gsData[u8Slot].sg_len     = 1;
    gsData[u8Slot].pu8Buf     = (volatile char *)w_buf;

    err = MMPF_SDIO_SendCommand(u8Slot, SD_IO_RW_EXTENDED,
                                (RW_FLAG(1) | FUN_NUM(func) | BLK_MODE(1) | OP_CODE(op_code) | REG_ADD(addr) | count));

    if (err)
    {
        return -1;
    }

    return 0;
}

// SDIO interrupt irq, enable:1 disable:0
int sdio_irq_enable(U8_T u8Slot, int enable)
{
    if (!MS_SD_Check_SDSlot(u8Slot))
    {
        CamOsPrintf("Sdmmc ERR: invalid slot number: %d !\n", u8Slot);
        return -1;
    }

    Hal_SDMMC_SDIOIntDetCtrl(gp_mmc_priv[u8Slot]->mmc_PMuxInfo.u8_ipOrder, (BOOL_T)enable);

    return 0;
}

// SDIO set interrupt irq callback
int sdio_set_irq_callback(U8_T u8Slot, sdio_irq_callback *irq_cb)
{
    if (!MS_SD_Check_SDSlot(u8Slot))
    {
        CamOsPrintf("Sdmmc ERR: invalid slot number: %d !\n", u8Slot);
        return -1;
    }

    sdio_irq_cb[u8Slot] = irq_cb;

    return 0;
}

#include "sdio_test.c"
