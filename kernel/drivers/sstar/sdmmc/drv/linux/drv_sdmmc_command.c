/*
 * drv_sdmmc_command.c- Sigmastar
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
 * FileName drv_sdmmc_command.c
 *     @author jeremy.wang (2013/07/26)
 * Desc:
 *     This layer is a simple sdmmc driver for special purpose (IP Verify etc...)
 *     (1) The goal is we don't need to change any HAL Driver code, but we can handle here.
 *
 ***************************************************************************************************************/
#include <linux/mmc/host.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include "drv_sdmmc_lnx.h"
#include "hal_sdmmc_platform.h"
#include "hal_sdmmc_timer.h"
#include "drv_sdmmc_command.h"
#include "hal_sdmmc_v5.h"
#include "hal_sdmmc_platform_pri_config.h"
//###########################################################################################################

//***********************************************************************************************************
// Config Setting (Internel)
//***********************************************************************************************************
#define EN_SDMMC_TRFUNC (FALSE)

#define D_SDMMC_DRVTYPE (SD_DRIVER_TYPE_B)
#define D_SDMMC_CURRLMT (1)
#define D_SDMMC_BUSMODE (SD_MODE_HIGH_SPEED | SD_MODE_UHS_SDR25 /*|SD_MODE_UHS_DDR50*/)

// Global Variable
//-----------------------------------------------------------------------------------------------------------
static SDMMCInfoStruct _stSDMMCInfo[3];
static RspStruct *     _pstRsp[3];
static volatile U8_T   gu8RspBuf[3][512]; // 512 bits
struct page *          eMMC_SectorPage   = 0;
U8_T *                 gu8EMMC_SectorBuf = 0; // 512 bytes

// Trace Funcion
//-----------------------------------------------------------------------------------------------------------
#if (EN_SDMMC_TRFUNC)
#define pr_sd_main(fmt, arg...) printk(fmt, ##arg)
#else
#define pr_sd_main(fmt, arg...) // printk(fmt, ##arg)
#endif

#if 1
#define prtstring(fmt, arg...) printk(KERN_CONT fmt, ##arg)
#else
#define prtstring(fmt, arg...)
#endif

//============================================
// Mask Value
//============================================
#define M_SDMMC_CURRSTATE(u32Val) ((u32Val & (BIT12_T | BIT11_T | BIT10_T | BIT09_T)) >> 9)

extern volatile CardType geCardType[SDMMC_NUM_TOTAL];
dma_addr_t               dma_test_addr0, dma_test_addr1;
dma_addr_t               AdmaScriptsAddr;
extern U8_T              u8MakeStsErr;

void Apply_PAGE_BUFFER(void)
{
    if (gu8EMMC_SectorBuf == NULL)
    {
        eMMC_SectorPage = alloc_pages(__GFP_COMP, 2); // Request 2 ^ order consecutive page boxes
        if (eMMC_SectorPage == NULL)
        {
            pr_err("Err allocate page 1 fails\n");
            while (1)
                ;
        }
        gu8EMMC_SectorBuf = (U8_T *)kmap(eMMC_SectorPage);
    }
    return;
}
dma_addr_t SDMMC_DMA_MAP_address(struct sstar_sdmmc_slot *p_sdmmc_slot, uintptr_t ulongBuffer, U32_T u32_ByteCnt,
                                 CmdEmType eCmdType)
{
    dma_addr_t dma_addr;

    if (eCmdType == EV_CMDWRITE) // write
    {
        dma_addr =
            dma_map_single(&p_sdmmc_slot->parent_sdmmc->pdev->dev, (void *)ulongBuffer, u32_ByteCnt, DMA_TO_DEVICE);
    }
    else
    {
        dma_addr =
            dma_map_single(&p_sdmmc_slot->parent_sdmmc->pdev->dev, (void *)ulongBuffer, u32_ByteCnt, DMA_FROM_DEVICE);
    }

    if (dma_mapping_error(&p_sdmmc_slot->parent_sdmmc->pdev->dev, dma_addr))
    {
        dma_unmap_single(&p_sdmmc_slot->parent_sdmmc->pdev->dev, dma_addr, u32_ByteCnt,
                         (eCmdType == EV_CMDWRITE) ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
        pr_err("SDMMC_DMA_MAP_address: Kernel can't mapping dma correctly\n");
    }

    return dma_addr;
}

void SDMMC_DMA_UNMAP_address(struct sstar_sdmmc_slot *p_sdmmc_slot, dma_addr_t dma_DMAAddr, U32_T u32_ByteCnt,
                             CmdEmType eCmdType)
{
    if (eCmdType == EV_CMDWRITE) // write
    {
        dma_unmap_single(&p_sdmmc_slot->parent_sdmmc->pdev->dev, dma_DMAAddr, u32_ByteCnt, DMA_TO_DEVICE);
    }
    else
    {
        dma_unmap_single(&p_sdmmc_slot->parent_sdmmc->pdev->dev, dma_DMAAddr, u32_ByteCnt, DMA_FROM_DEVICE);
    }
}

static U32_T _RetArrToU32(U8_T *u8Arr)
{
    return ((*(u8Arr) << 24) | (*(u8Arr + 1) << 16) | (*(u8Arr + 2) << 8) | (*(u8Arr + 3)));
}

static void _SDMMC_InfoInit(struct sstar_sdmmc_slot *p_sdmmc_slot)
{
    IpOrder eIP = p_sdmmc_slot->ipOrder;

    _stSDMMCInfo[eIP].u8Initial = 0;
    // Before _SDMMC_CMD8
    _stSDMMCInfo[eIP].eHCS = 0; // Init HCS=0

    // Before _SDMMC_ACMD41/CMD1
    if (!(p_sdmmc_slot->mmc->caps2 & MMC_CAP2_NO_MMC))
    {
        _stSDMMCInfo[eIP].eCardType = CARD_TYPE_EMMC;
        _stSDMMCInfo[eIP].u32RCAArg = 0x10000;
    }
    else if (!(p_sdmmc_slot->mmc->caps2 & MMC_CAP2_NO_SD))
    {
        _stSDMMCInfo[eIP].eCardType = CARD_TYPE_SD;
        _stSDMMCInfo[eIP].u32RCAArg = 0;
    }
    _stSDMMCInfo[eIP].u32OCR = 0;

    // Before _SDMMC_CMD9
    _stSDMMCInfo[eIP].u8BusWidth = 1;
    _stSDMMCInfo[eIP].u8SpecVer  = 0;
    _stSDMMCInfo[eIP].u8SpecVer1 = 0;

    // Before SDMMC_SwitchHighBus
    _stSDMMCInfo[eIP].u8AccessMode   = 0; // DEF_SPEED_BUS_SPEED
    _stSDMMCInfo[eIP].u8DrvStrength  = 0;
    _stSDMMCInfo[eIP].u8CurrMax      = 0;
    _stSDMMCInfo[eIP].u8SD3BusMode   = 0;
    _stSDMMCInfo[eIP].u8SD3DrvType   = 0;
    _stSDMMCInfo[eIP].u8SD3CurrLimit = 0;
    _stSDMMCInfo[eIP].u32MaxClk      = CLK_DEF_SPEED;

    //_stSDMMCInfo[u8Slot].stCID;
    //_stSDMMCInfo[u8Slot].stCSD;
    //_stSDMMCInfo[u8Slot].u32CardStatus = 0;
}

static void _SDMMC_GetCIDInfo(IpOrder eIP, U8_T *u8Arr)
{
    U8_T u8Pos = 0, u8Offset = 0;

    pr_sd_main("====================== [ CID Info for IP: %u] =======================\n", eIP);

    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[eIP].stCID.MID = *u8Arr;
    pr_sd_main("[MID]         => (0x%02X)\n", _stSDMMCInfo[eIP].stCID.MID);
    //-----------------------------------------------------------------------------------------------------------------
    if (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_SD)
    {
        _stSDMMCInfo[eIP].stCID.OID[0] = *(u8Arr + 1);
        _stSDMMCInfo[eIP].stCID.OID[1] = *(u8Arr + 2);
    }
    else if (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_EMMC)
        _stSDMMCInfo[eIP].stCID.OID[0] = *(u8Arr + 2);

    pr_sd_main("[OID]         => [%c][%c]\n", _stSDMMCInfo[eIP].stCID.OID[0], _stSDMMCInfo[eIP].stCID.OID[1]);
    //-----------------------------------------------------------------------------------------------------------------

    for (u8Pos = 0; u8Pos < 5; u8Pos++)
        _stSDMMCInfo[eIP].stCID.PNM[u8Pos] = *(u8Arr + 3 + u8Pos);

    if (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_SD)
    {
        u8Offset                       = 7;
        _stSDMMCInfo[eIP].stCID.PNM[5] = '_';
    }
    else if (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_EMMC)
    {
        u8Offset                       = 8;
        _stSDMMCInfo[eIP].stCID.PNM[5] = *(u8Arr + u8Offset);
    }

    pr_sd_main("[PNM]         => [%c][%c][%c][%c][%c][%c]\n", _stSDMMCInfo[eIP].stCID.PNM[0],
               _stSDMMCInfo[eIP].stCID.PNM[1], _stSDMMCInfo[eIP].stCID.PNM[2], _stSDMMCInfo[eIP].stCID.PNM[3],
               _stSDMMCInfo[eIP].stCID.PNM[4], _stSDMMCInfo[eIP].stCID.PNM[5]);
    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[eIP].stCID.PRV = *(u8Arr + u8Offset + 1);
    pr_sd_main("[PRV]         => (0x%02X)\n", _stSDMMCInfo[eIP].stCID.PRV);
    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[eIP].stCID.PSN = _RetArrToU32(u8Arr + u8Offset + 2);
    pr_sd_main("[PSN]         => (0x%08X)\n", _stSDMMCInfo[eIP].stCID.PSN);

    pr_sd_main("======================================================================\n");
}

static void _SDMMC_GetCSDInfo(IpOrder eIP, U8_T *u8Arr)
{
    U16_T ReffArr[0x10] = {0, 10, 12, 13, 14, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80};
    U32_T u32CSIZE;
    U8_T  u8CSIZEMULTI, uMulti;

    pr_sd_main("====================== [ CSD Info for IP: %u] =======================\n", eIP);
    //-----------------------------------------------------------------------------------------------------------------
    _stSDMMCInfo[eIP].stCSD.CSDSTR = (*u8Arr) >> 6;
    pr_sd_main("[CSD_STR]      => (0x%02X)\n", _stSDMMCInfo[eIP].stCSD.CSDSTR);
    //-----------------------------------------------------------------------------------------------------------------
    if (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_EMMC)
        _stSDMMCInfo[eIP].stCSD.SPECVERS = (*u8Arr >> 2) & 0xF;
    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[eIP].stCSD.TAAC_NS = 1;
    for (uMulti = 1; uMulti <= (*(u8Arr + 1) & 0x07); uMulti++)
        _stSDMMCInfo[eIP].stCSD.TAAC_NS *= 10;

    pr_sd_main("[TACC]         => (0x%02X)", *(u8Arr + 1));
    pr_sd_main(" ==>CAL TU(%u) x TV(%u/10) = ", _stSDMMCInfo[eIP].stCSD.TAAC_NS, ReffArr[(*(u8Arr + 1) >> 3) & 0xF]);
    _stSDMMCInfo[eIP].stCSD.TAAC_NS *= ReffArr[(*(u8Arr + 1) >> 3) & 0xF];
    pr_sd_main(" %u (ns)\n", _stSDMMCInfo[eIP].stCSD.TAAC_NS / 10);

    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[eIP].stCSD.NSAC = *(u8Arr + 2);
    pr_sd_main("[NSAC]         => (0x%02X) ==>CAL %u (clks)\n", _stSDMMCInfo[eIP].stCSD.NSAC,
               _stSDMMCInfo[eIP].stCSD.NSAC);
    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[eIP].stCSD.TRAN_KB = 100;
    for (uMulti = 1; uMulti <= (*(u8Arr + 3) & 0x07); uMulti++)
        _stSDMMCInfo[eIP].stCSD.TRAN_KB *= 10;

    pr_sd_main("[TRAN_SPD]     => (0x%02X)", *(u8Arr + 3));
    pr_sd_main("==>CAL TU(%u) x TV(%u/10) = ", _stSDMMCInfo[eIP].stCSD.TRAN_KB, ReffArr[(*(u8Arr + 3) >> 3) & 0xF]);
    _stSDMMCInfo[eIP].stCSD.TRAN_KB *= ReffArr[(*(u8Arr + 3) >> 3) & 0xF];
    pr_sd_main(" %u (Kbit)\n", _stSDMMCInfo[eIP].stCSD.TRAN_KB / 10);
    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[eIP].stCSD.CCC = (*(u8Arr + 4) << 4) | (*(u8Arr + 5) >> 4);
    pr_sd_main("[CCC]          => (0x%04X)\n", _stSDMMCInfo[eIP].stCSD.CCC);
    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[eIP].stCSD.R_BLK_SIZE = *(u8Arr + 5) & 0XF;
    pr_sd_main("[R_BLK_LEN]    => (2E_%u)\n", _stSDMMCInfo[eIP].stCSD.R_BLK_SIZE);
    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[eIP].stCSD.W_BLK_MISALIGN = (*(u8Arr + 6) >> 6) & 0x1;
    pr_sd_main("[W_BLK_MIS_ALG]=> (%u)\n", _stSDMMCInfo[eIP].stCSD.W_BLK_MISALIGN);
    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[eIP].stCSD.R_BLK_MISALIGN = (*(u8Arr + 6) >> 5) & 0x1;
    pr_sd_main("[R_BLK_MIS_ALG]=> (%u)\n", _stSDMMCInfo[eIP].stCSD.R_BLK_MISALIGN);
    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[eIP].stCSD.R2W_FACTOR = (*(u8Arr + 12) & 0x1C) >> 2;
    pr_sd_main("[R2W_FACTOR]   => (%u)\n", _stSDMMCInfo[eIP].stCSD.R2W_FACTOR);
    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[eIP].stCSD.W_BLK_SIZE = ((*(u8Arr + 12) & 0x03) << 2) | (*(u8Arr + 13) >> 6);
    pr_sd_main("[W_BLK_LEN]    => (2E_%u)\n", _stSDMMCInfo[eIP].stCSD.W_BLK_SIZE);

    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[eIP].stCSD.PERM_W_PROTECT = (*(u8Arr + 14) >> 5) & 0x1;
    pr_sd_main("[PERM_W_PRO]   => (%u)\n", _stSDMMCInfo[eIP].stCSD.PERM_W_PROTECT);

    //-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[eIP].stCSD.TEMP_W_PROTECT = (*(u8Arr + 14) >> 4) & 0x1;
    pr_sd_main("[TMP_W_PRO]    => (%u)\n", _stSDMMCInfo[eIP].stCSD.TEMP_W_PROTECT);
    //-----------------------------------------------------------------------------------------------------------------

    if ((_stSDMMCInfo[eIP].stCSD.CSDSTR == 1) && (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_SD))
    {
        u32CSIZE                        = ((*(u8Arr + 7) & 0x3F) << 16) | (*(u8Arr + 8) << 8) | *(u8Arr + 9);
        _stSDMMCInfo[eIP].stCSD.CAPCITY = (u32CSIZE + 1) * 524288;
    }
    else // SD1.X and //MMC
    {
        u32CSIZE     = ((*(u8Arr + 6) & 0x3) << 10) | (*(u8Arr + 7) << 2) | (*(u8Arr + 8) >> 6);
        u8CSIZEMULTI = ((*(u8Arr + 9) & 0x3) << 1) | (*(u8Arr + 10) >> 7);
        _stSDMMCInfo[eIP].stCSD.CAPCITY =
            (u32CSIZE + 1) * (1 << (u8CSIZEMULTI + 2)) * (1 << _stSDMMCInfo[eIP].stCSD.R_BLK_SIZE);
    }

    if (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_EMMC)
    {
        _stSDMMCInfo[eIP].stCSD.ERASE_GRP_SIZE = (*(u8Arr + 9) & 0x7C) >> 2;
        _stSDMMCInfo[eIP].stCSD.ERASE_GRP_MULT = ((*(u8Arr + 9) & 0x03) << 3) + ((*(u8Arr + 10) & 0xE0) >> 5);
        _stSDMMCInfo[eIP].u32EraseUnitSize =
            (_stSDMMCInfo[eIP].stCSD.ERASE_GRP_SIZE + 1) * (_stSDMMCInfo[eIP].stCSD.ERASE_GRP_MULT + 1);
    }

    pr_sd_main("[CAPACITY]     => %u (Bytes)\n", _stSDMMCInfo[eIP].stCSD.CAPCITY);

    pr_sd_main("======================================================================\n");
}

static RspStruct *_SDMMC_CMDReq(IpOrder eIP, U8_T u8Cmd, U32_T u32Arg, SDMMCRspEmType eRspType)
{
    // RspErrEmType eErr = EV_STS_OK;
    RspStruct *eRspSt;

    pr_sd_main("_[sdmmc_%u] CMD_%u (0x%08X)", eIP, u8Cmd, u32Arg);

    Hal_SDMMC_SetCmdToken(eIP, u8Cmd, u32Arg);
    Hal_SDMMC_SendCmdAndWaitProcess(eIP, EV_EMP, EV_CMDRSP, eRspType, TRUE);
    eRspSt = Hal_SDMMC_GetRspToken(eIP);

    pr_sd_main("=> (Err: 0x%04X)\n", (U16_T)eRspSt->eErrCode);

    return eRspSt;
}

static RspStruct *_SDMMC_DATAReq(struct sstar_sdmmc_slot *p_sdmmc_slot, U8_T u8Cmd, U32_T u32Arg, U16_T u16BlkCnt,
                                 U16_T u16BlkSize, TransEmType eTransType, volatile U8_T *pu8Buf)
{
    // RspErrEmType eErr  = EV_STS_OK;
    CmdEmType  eCmdType = EV_CMDREAD;
    RspStruct *eRspSt;
    IpOrder    eIP = (IpOrder)p_sdmmc_slot->ipOrder;

    BOOL_T bCloseClock = FALSE;
    BOOL_T bIsMCGOff   = FALSE;

    pr_sd_main("_[sdmmc_%u] CMD_%u (0x%08X)__(TB: %u)(BSz: %u)", eIP, u8Cmd, u32Arg, u16BlkCnt, u16BlkSize);

    if ((u8Cmd == 24) || (u8Cmd == 25))
        eCmdType = EV_CMDWRITE;

    if (u16BlkCnt > 1)
        bCloseClock = FALSE;
    else
        bCloseClock = TRUE;

    if (p_sdmmc_slot->p_mmc_priv->u8_cifdMCGOff && (eTransType == EV_CIF))
    {
        bIsMCGOff = (CARD_REG(A_CLK_EN_REG(eIP)) & R_MCG_DISABLE) ? TRUE : FALSE;
        if (!bIsMCGOff)
            CARD_REG_SETBIT(A_CLK_EN_REG(eIP), R_MCG_DISABLE);
    }

    Hal_SDMMC_SetCmdToken(eIP, u8Cmd, u32Arg);
    Hal_SDMMC_TransCmdSetting(eIP, eTransType, u16BlkCnt, u16BlkSize,
                              Hal_CARD_TransMIUAddr((dma_addr_t)(uintptr_t)pu8Buf, NULL), pu8Buf);
    Hal_SDMMC_SendCmdAndWaitProcess(eIP, eTransType, eCmdType, EV_R1, bCloseClock);
    eRspSt = Hal_SDMMC_GetRspToken(eIP);

    if ((p_sdmmc_slot->p_mmc_priv->u8_cifdMCGOff && (eTransType == EV_CIF)) && (!bIsMCGOff))
        CARD_REG_CLRBIT(A_CLK_EN_REG(eIP), R_MCG_DISABLE);

    pr_sd_main("=> (Err: 0x%04X)\n", (U16_T)eRspSt->eErrCode);
    return eRspSt;
}

static RspStruct *_SDMMC_DATAReq_Ext(struct sstar_sdmmc_slot *p_sdmmc_slot, U8_T u8Cmd, U32_T u32Arg, U16_T u16BlkCnt,
                                     U16_T u16BlkSize, TransEmType eTransType, volatile U8_T *pu8Buf,
                                     dma_addr_t pPhyAddr)
{
    // RspErrEmType eErr  = EV_STS_OK;
    CmdEmType      eCmdType = EV_CMDREAD;
    RspStruct *    eRspSt;
    SDMMCRspEmType eRspType = EV_R1;
    IpOrder        eIP      = (IpOrder)p_sdmmc_slot->ipOrder;

    BOOL_T bCloseClock = FALSE;
    BOOL_T bIsMCGOff   = FALSE;

    pr_sd_main("_[sdmmc_%u] CMD_%u (0x%08X)__(TB: %u)(BSz: %u)", eIP, u8Cmd, u32Arg, u16BlkCnt, u16BlkSize);

    if ((u8Cmd == 24) || (u8Cmd == 25))
        eCmdType = EV_CMDWRITE;

    if (u16BlkCnt > 1)
        bCloseClock = FALSE;
    else
        bCloseClock = TRUE;

#if defined(CONFIG_SUPPORT_SDMMC_UT_VERIFY)
    if (u8MakeStsErr == MAKE_CMD_RSP_ERR)
        eRspType = EV_R2;
#endif

    if (p_sdmmc_slot->p_mmc_priv->u8_cifdMCGOff && (eTransType == EV_CIF))
    {
        bIsMCGOff = (CARD_REG(A_CLK_EN_REG(eIP)) & R_MCG_DISABLE) ? TRUE : FALSE;
        if (!bIsMCGOff)
            CARD_REG_SETBIT(A_CLK_EN_REG(eIP), R_MCG_DISABLE);
    }

    Hal_SDMMC_SetCmdToken(eIP, u8Cmd, u32Arg);
    Hal_SDMMC_TransCmdSetting(eIP, eTransType, u16BlkCnt, u16BlkSize, Hal_CARD_TransMIUAddr(pPhyAddr, NULL), pu8Buf);
    Hal_SDMMC_SendCmdAndWaitProcess(eIP, eTransType, eCmdType, eRspType, bCloseClock);
    eRspSt = Hal_SDMMC_GetRspToken(eIP);

    if ((p_sdmmc_slot->p_mmc_priv->u8_cifdMCGOff && (eTransType == EV_CIF)) && (!bIsMCGOff))
        CARD_REG_CLRBIT(A_CLK_EN_REG(eIP), R_MCG_DISABLE);

    pr_sd_main("=> (Err: 0x%04X)\n", (U16_T)eRspSt->eErrCode);
    return eRspSt;
}

U16_T SDMMC_CMD0(IpOrder eIP)
{
    SDMMCRspEmType eRspType = EV_NO;
#if defined(CONFIG_SUPPORT_SDMMC_UT_VERIFY)
    if (u8MakeStsErr == MAKE_CMD_NO_RSP)
        eRspType = EV_R1;
#endif
    _pstRsp[eIP] = _SDMMC_CMDReq(eIP, 0, 0x00000000, eRspType);

    return (U16_T)_pstRsp[eIP]->eErrCode;
}

static RspStruct *_SDMMC_OCR_CMD1(IpOrder eIP, CardType eCardType)
{
    //[7]     1.7-1.95V 0:high 1:dual
    //[14:08] 2.0-2.6 V
    //[23:15] 2.7-3.6 V
    //[30:29] 2.0-2.6 V (00b:10b)/(BYTE:SEC)
    U32_T u32Arg =
        BIT30_T | (BIT23_T | BIT22_T | BIT21_T | BIT20_T | BIT19_T | BIT18_T | BIT17_T | BIT16_T | BIT15_T | BIT07_T);
    U16_T u16Count = 0;

    do
    {
        if (eCardType == CARD_TYPE_EMMC)
        {
            //------------------------------------------------------------------------------------------------------------
            _pstRsp[eIP] = _SDMMC_CMDReq(eIP, 1, u32Arg, EV_R3);
            //------------------------------------------------------------------------------------------------------------
            if (_pstRsp[eIP]->eErrCode)
                return _pstRsp[eIP];

            _stSDMMCInfo[eIP].u32OCR = _RetArrToU32(_pstRsp[eIP]->u8ArrRspToken + 1);
        }

        Hal_Timer_mDelay(1);
        u16Count++;

    } while (!(_stSDMMCInfo[eIP].u32OCR & R_OCR_READY) && (u16Count < 1000)); // Card powered ready (0:busy,1:ready)

    if (u16Count >= 1000)
        _pstRsp[eIP]->eErrCode = EV_OCR_BERR; // Card is still busy
    else if (!(_stSDMMCInfo[eIP].u32OCR & 0xFF8000))
        _pstRsp[eIP]->eErrCode = EV_OUT_VOL_RANGE; // Double Confirm Voltage Range

    if (!_pstRsp[eIP]->eErrCode)
    {
        _stSDMMCInfo[eIP].eCardType      = eCardType;
        _stSDMMCInfo[eIP].u8IfSectorMode = (_stSDMMCInfo[eIP].u32OCR & BIT30_T);
    }

    return _pstRsp[eIP];
}

static RspStruct *_SDMMC_SD_CMD8(IpOrder eIP)
{
    _pstRsp[eIP] = _SDMMC_CMDReq(eIP, 8, (BIT00_T << 8) | 0xAA, EV_R7); // CMD8 ==> Alwasy Support STD Voltage

    if (_pstRsp[eIP]->eErrCode)
    {
        return _pstRsp[eIP];
    }
    if (_pstRsp[eIP]->u8ArrRspToken[4] != 0xAA)
    {
        _pstRsp[eIP]->eErrCode = EV_CMD8_PERR;
        return _pstRsp[eIP];
    }

    _stSDMMCInfo[eIP].eHCS = EV_HCS; // HCS=1

    return _pstRsp[eIP];
}

static RspStruct *_SDMMC_OCR_ACMD41(IpOrder eIP, CardType eCardType)
{
    U32_T u32ReqOCR = SD_OCR_RANGE | _stSDMMCInfo[eIP].eHCS;
    U16_T u16Count  = 0;

    do
    {
        if (eCardType == CARD_TYPE_SD)
        {
            //------------------------------------------------------------------------------------------------------------
            _pstRsp[eIP] = _SDMMC_CMDReq(eIP, 55, _stSDMMCInfo[eIP].u32RCAArg, EV_R1); // CMD55;
            //------------------------------------------------------------------------------------------------------------
            if (_pstRsp[eIP]->eErrCode)
                return _pstRsp[eIP];

            //------------------------------------------------------------------------------------------------------------
            _pstRsp[eIP] = _SDMMC_CMDReq(eIP, 41, u32ReqOCR, EV_R3); // ACMD41;
            //------------------------------------------------------------------------------------------------------------
            if (_pstRsp[eIP]->eErrCode)
                return _pstRsp[eIP];

            _stSDMMCInfo[eIP].u32OCR = _RetArrToU32(_pstRsp[eIP]->u8ArrRspToken + 1);
        }

        Hal_Timer_mDelay(1);
        u16Count++;

    } while (!(_stSDMMCInfo[eIP].u32OCR & R_OCR_READY) && (u16Count < 1000)); // Card powered ready (0:busy,1:ready)

    _stSDMMCInfo[eIP].eHCS = (SDCAPEmType)(_stSDMMCInfo[eIP].u32OCR & R_OCR_CCS);

    if (u16Count >= 1000)
        _pstRsp[eIP]->eErrCode = EV_OCR_BERR; // Card is still busy
    else if (!(_stSDMMCInfo[eIP].u32OCR & SD_OCR_RANGE & 0x3FFFFFFF))
        _pstRsp[eIP]->eErrCode = EV_OUT_VOL_RANGE; // Double Confirm Voltage Range

    if (!_pstRsp[eIP]->eErrCode)
        _stSDMMCInfo[eIP].eCardType = eCardType;

    return _pstRsp[eIP];
}

static RspStruct *_SDMMC_CID_CMD2(IpOrder eIP)
{
    _pstRsp[eIP] = _SDMMC_CMDReq(eIP, 2, 0x00000000, EV_R2);

    if (!_pstRsp[eIP]->eErrCode)
        _SDMMC_GetCIDInfo(eIP, _pstRsp[eIP]->u8ArrRspToken + 1);

    return _pstRsp[eIP];
}

static RspStruct *_SDMMC_CMD3_CMD7(IpOrder eIP, U8_T u8CmdIdx, SDMMCRspEmType eRspType)
{
    return _SDMMC_CMDReq(eIP, u8CmdIdx, _stSDMMCInfo[eIP].u32RCAArg, eRspType);
}

static RspStruct *_SDMMC_CSD_CMD9(IpOrder eIP)
{
    _pstRsp[eIP] = _SDMMC_CMDReq(eIP, 9, _stSDMMCInfo[eIP].u32RCAArg, EV_R2);

    if (!_pstRsp[eIP]->eErrCode)
        _SDMMC_GetCSDInfo(eIP, _pstRsp[eIP]->u8ArrRspToken + 1);

    return _pstRsp[eIP];
}

static RspStruct *_SDMMC_ACMD51(struct sstar_sdmmc_slot *p_sdmmc_slot, volatile U8_T *pu8DataBuf)
{
    IpOrder eIP = (IpOrder)p_sdmmc_slot->ipOrder;
    //------------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_CMDReq(eIP, 55, _stSDMMCInfo[eIP].u32RCAArg, EV_R1); // CMD55;
    //------------------------------------------------------------------------------------------------------------
    if (_pstRsp[eIP]->eErrCode)
        return _pstRsp[eIP];

    //------------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_DATAReq(p_sdmmc_slot, 51, 0x00000000, 1, 8, EV_CIF, pu8DataBuf); // ACMD51;
    //------------------------------------------------------------------------------------------------------------
    if (_pstRsp[eIP]->eErrCode)
        return _pstRsp[eIP];

    pr_sd_main("====================== [ SCR Info for Slot] =======================\n");

    _stSDMMCInfo[eIP].u8SpecVer  = (*pu8DataBuf) & 0x7;
    _stSDMMCInfo[eIP].u8SpecVer1 = ((*(pu8DataBuf + 2)) & 0x80) >> 7;

    if ((*(pu8DataBuf + 1)) & 0x4)
        _stSDMMCInfo[eIP].u8BusWidth = 4;

    pr_sd_main("[SPECVER]      => (0x%02X)\n", _stSDMMCInfo[eIP].u8SpecVer);
    pr_sd_main("[SPECVER1]     => (0x%02X)\n", _stSDMMCInfo[eIP].u8SpecVer1);
    pr_sd_main("[BUSWIDTH]     => (0x%02X)\n", _stSDMMCInfo[eIP].u8BusWidth);

    pr_sd_main("======================================================================\n");

    return _pstRsp[eIP];
}

static RspStruct *_SDMMC_CMD13(IpOrder eIP)
{
    if (_stSDMMCInfo[eIP].u8Initial != 1)
    {
        if (geCardType[eIP] == CARD_TYPE_EMMC)
            _stSDMMCInfo[eIP].u32RCAArg = 0x10000;
        else
            _stSDMMCInfo[eIP].u32RCAArg = 0;
    }
    //--------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_CMDReq(eIP, 13, _stSDMMCInfo[eIP].u32RCAArg, EV_R1); // CMD13
    //--------------------------------------------------------------------------------------------------------
    return _pstRsp[eIP];
}

static RspStruct *_SDMMC_ACMD6(IpOrder eIP)
{
    if (_stSDMMCInfo[eIP].u8Initial != 1)
    {
        if (geCardType[eIP] == CARD_TYPE_EMMC)
            _stSDMMCInfo[eIP].u32RCAArg = 0x10000;
        else
            _stSDMMCInfo[eIP].u32RCAArg = 0;
    }

    //------------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_CMDReq(eIP, 55, _stSDMMCInfo[eIP].u32RCAArg, EV_R1); // CMD55;
    //------------------------------------------------------------------------------------------------------------
    if (_pstRsp[eIP]->eErrCode)
        return _pstRsp[eIP];

    //------------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_CMDReq(eIP, 6, 0x00000002, EV_R1); // ACMD6;
    //------------------------------------------------------------------------------------------------------------

    return _pstRsp[eIP];
}

static RspStruct *_SDMMC_EMMC_CMD6(IpOrder eIP, U32_T u32Arg)
{
    return _SDMMC_CMDReq(eIP, 6, u32Arg, EV_R1B);
}

static RspStruct *_SDMMC_SD_CMD6(struct sstar_sdmmc_slot *p_sdmmc_slot, BOOL_T bSetMode, U8_T u8Group, U8_T u8Value,
                                 volatile U8_T *pu8DataBuf)
{
    IpOrder eIP = (IpOrder)p_sdmmc_slot->ipOrder;

    U32_T u32Arg = (bSetMode << 31) | 0x00FFFFFF;
    u32Arg &= ~(0xF << (u8Group * 4));
    u32Arg |= u8Value << (u8Group * 4);

    //------------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_DATAReq(p_sdmmc_slot, 6, u32Arg, 1, 64, EV_CIF, pu8DataBuf); // CMD6; Query
    //------------------------------------------------------------------------------------------------------------

    return _pstRsp[eIP];
}

// CMD8: send EXT_CSD
static RspStruct *_SDMMC_EMMC_CMD8_DMA(struct sstar_sdmmc_slot *p_sdmmc_slot, U8_T *pu8DataBuf)
{
    U32_T      u32Arg = 0;
    dma_addr_t dmaDataMapAddr;
    // -------------------------------
    dmaDataMapAddr = SDMMC_DMA_MAP_address(p_sdmmc_slot, (uintptr_t)pu8DataBuf, 512, EV_CMDREAD);
    _pstRsp[p_sdmmc_slot->ipOrder] =
        _SDMMC_DATAReq_Ext(p_sdmmc_slot, 8, u32Arg, 1, 512, EV_DMA, pu8DataBuf, dmaDataMapAddr);
    SDMMC_DMA_UNMAP_address(p_sdmmc_slot, dmaDataMapAddr, 512, EV_CMDREAD);

    return _pstRsp[p_sdmmc_slot->ipOrder];
}

#if 0
static RspStruct *_SDMMC_CMD8_EMMC_CIFD(IpOrder eIP, U8_T *pu8DataBuf)
{
    //------------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_DATAReq(eIP, 8, 0, 1, 512, EV_CIF, pu8DataBuf);
    //------------------------------------------------------------------------------------------------------------
    return _pstRsp[eIP];

}
#endif
U16_T SDMMC_CMD16(IpOrder eIP, U32_T u32BlkLength)
{
    U32_T u32Arg = u32BlkLength;
    _pstRsp[eIP] = _SDMMC_CMDReq(eIP, 16, u32Arg, EV_R1);

    return (U16_T)_pstRsp[eIP]->eErrCode;
}

static RspStruct *_SDMMC_CMD17_DMA(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32CardBlkAddr, U8_T *pu8DataBuf)
{
    dma_addr_t dmaDataMapAddr;
    U32_T      u32Arg = u32CardBlkAddr;

    if (_stSDMMCInfo[p_sdmmc_slot->ipOrder].eCardType == CARD_TYPE_EMMC)
        u32Arg = u32CardBlkAddr << (_stSDMMCInfo[p_sdmmc_slot->ipOrder].u8IfSectorMode ? 0 : 9);
    else if (_stSDMMCInfo[p_sdmmc_slot->ipOrder].eCardType == CARD_TYPE_SD)
        u32Arg = u32CardBlkAddr << ((_stSDMMCInfo[p_sdmmc_slot->ipOrder].eHCS == EV_HCS) ? 0 : 9);

    if (M_REG_IMISEL(p_sdmmc_slot->ipOrder))
        dmaDataMapAddr = (dma_addr_t)__virt_to_phys((uintptr_t)pu8DataBuf);
    else
        dmaDataMapAddr = SDMMC_DMA_MAP_address(p_sdmmc_slot, (uintptr_t)pu8DataBuf, 512, EV_CMDREAD);

    _pstRsp[p_sdmmc_slot->ipOrder] =
        _SDMMC_DATAReq_Ext(p_sdmmc_slot, 17, u32Arg, 1, 512, EV_DMA, pu8DataBuf, dmaDataMapAddr);

    if ((M_REG_IMISEL(p_sdmmc_slot->ipOrder)) == 0)
        SDMMC_DMA_UNMAP_address(p_sdmmc_slot, dmaDataMapAddr, 512, EV_CMDREAD);

    return _pstRsp[p_sdmmc_slot->ipOrder];
}

static RspStruct *_SDMMC_CMD17_CIFD(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32CardBlkAddr,
                                    volatile U8_T *pu8DataBuf)
{
    U32_T   u32Arg = u32CardBlkAddr;
    IpOrder eIP    = (IpOrder)p_sdmmc_slot->ipOrder;

    if (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_EMMC)
        u32Arg = u32CardBlkAddr << (_stSDMMCInfo[eIP].u8IfSectorMode ? 0 : 9);
    else if (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_SD)
        u32Arg = u32CardBlkAddr << ((_stSDMMCInfo[eIP].eHCS == EV_HCS) ? 0 : 9);

    //------------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_DATAReq(p_sdmmc_slot, 17, u32Arg, 1, 512, EV_CIF, pu8DataBuf); // CMD17
    //------------------------------------------------------------------------------------------------------------
    return _pstRsp[eIP];
}

static RspStruct *_SDMMC_CMD18_DMA(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32CardBlkAddr, U8_T *pu8DataBuf,
                                   U16_T u16BlkCnt)
{
    dma_addr_t dmaDataMapAddr;
    U32_T      u32Arg = u32CardBlkAddr;

    if (_stSDMMCInfo[p_sdmmc_slot->ipOrder].eCardType == CARD_TYPE_EMMC)
        u32Arg = u32CardBlkAddr << (_stSDMMCInfo[p_sdmmc_slot->ipOrder].u8IfSectorMode ? 0 : 9);
    else if (_stSDMMCInfo[p_sdmmc_slot->ipOrder].eCardType == CARD_TYPE_SD)
        u32Arg = u32CardBlkAddr << ((_stSDMMCInfo[p_sdmmc_slot->ipOrder].eHCS == EV_HCS) ? 0 : 9);

    if (M_REG_IMISEL(p_sdmmc_slot->ipOrder))
        dmaDataMapAddr = (dma_addr_t)__virt_to_phys((uintptr_t)pu8DataBuf);
    else
        dmaDataMapAddr = SDMMC_DMA_MAP_address(p_sdmmc_slot, (uintptr_t)pu8DataBuf, 512 * u16BlkCnt, EV_CMDREAD);

    _pstRsp[p_sdmmc_slot->ipOrder] =
        _SDMMC_DATAReq_Ext(p_sdmmc_slot, 18, u32Arg, u16BlkCnt, 512, EV_DMA, pu8DataBuf, dmaDataMapAddr);

    if ((M_REG_IMISEL(p_sdmmc_slot->ipOrder)) == 0)
        SDMMC_DMA_UNMAP_address(p_sdmmc_slot, dmaDataMapAddr, 512 * u16BlkCnt, EV_CMDREAD);

    _pstRsp[p_sdmmc_slot->ipOrder] = _SDMMC_CMDReq(p_sdmmc_slot->ipOrder, 12, 0x00000000, EV_R1B); // CMD12;

    return _pstRsp[p_sdmmc_slot->ipOrder];
}

// enable Reliable Write
U16_T SDMMC_CMD23(IpOrder eIP, U16_T u16BlkCount)
{
    U32_T u32Arg;

    u32Arg = u16BlkCount & 0xFFFF; // don't set BIT24
#if defined(SDMMC_FEATURE_RELIABLE_WRITE) && SDMMC_FEATURE_RELIABLE_WRITE
    u32Arg |= BIT31; // don't set BIT24
#endif

    _pstRsp[eIP] = _SDMMC_CMDReq(eIP, 23, u32Arg, EV_R1);

    return (U16_T)_pstRsp[eIP]->eErrCode;
}

static RspStruct *_SDMMC_CMD24_DMA(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32CardBlkAddr, U8_T *pu8DataBuf)
{
    dma_addr_t dmaDataMapAddr;
    U32_T      u32Arg = u32CardBlkAddr;

    if (_stSDMMCInfo[p_sdmmc_slot->ipOrder].eCardType == CARD_TYPE_EMMC)
        u32Arg = u32CardBlkAddr << (_stSDMMCInfo[p_sdmmc_slot->ipOrder].u8IfSectorMode ? 0 : 9);
    else if (_stSDMMCInfo[p_sdmmc_slot->ipOrder].eCardType == CARD_TYPE_SD)
        u32Arg = u32CardBlkAddr << ((_stSDMMCInfo[p_sdmmc_slot->ipOrder].eHCS == EV_HCS) ? 0 : 9);

    if (M_REG_IMISEL(p_sdmmc_slot->ipOrder))
        dmaDataMapAddr = (dma_addr_t)__virt_to_phys((uintptr_t)pu8DataBuf);
    else
        dmaDataMapAddr = SDMMC_DMA_MAP_address(p_sdmmc_slot, (uintptr_t)pu8DataBuf, 512, EV_CMDWRITE);

    _pstRsp[p_sdmmc_slot->ipOrder] =
        _SDMMC_DATAReq_Ext(p_sdmmc_slot, 24, u32Arg, 1, 512, EV_DMA, pu8DataBuf, dmaDataMapAddr);

    if ((M_REG_IMISEL(p_sdmmc_slot->ipOrder)) == 0)
        SDMMC_DMA_UNMAP_address(p_sdmmc_slot, dmaDataMapAddr, 512, EV_CMDWRITE);

    return _pstRsp[p_sdmmc_slot->ipOrder];
}

static RspStruct *_SDMMC_CMD24_CIFD(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32CardBlkAddr,
                                    volatile U8_T *pu8DataBuf)
{
    U32_T   u32Arg = u32CardBlkAddr;
    IpOrder eIP    = (IpOrder)p_sdmmc_slot->ipOrder;

    if (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_EMMC)
        u32Arg = u32CardBlkAddr << (_stSDMMCInfo[eIP].u8IfSectorMode ? 0 : 9);
    else if (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_SD)
        u32Arg = u32CardBlkAddr << ((_stSDMMCInfo[eIP].eHCS == EV_HCS) ? 0 : 9);

    //------------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_DATAReq(p_sdmmc_slot, 24, u32Arg, 1, 512, EV_CIF, pu8DataBuf); // CMD17
    //------------------------------------------------------------------------------------------------------------
    return _pstRsp[eIP];
}

static RspStruct *_SDMMC_CMD25_DMA(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32CardBlkAddr, U8_T *pu8DataBuf,
                                   U16_T u16BlkCnt)
{
    dma_addr_t dmaDataMapAddr;
    U32_T      u32Arg = u32CardBlkAddr;

    if (_stSDMMCInfo[p_sdmmc_slot->ipOrder].eCardType == CARD_TYPE_EMMC)
        u32Arg = u32CardBlkAddr << (_stSDMMCInfo[p_sdmmc_slot->ipOrder].u8IfSectorMode ? 0 : 9);
    else if (_stSDMMCInfo[p_sdmmc_slot->ipOrder].eCardType == CARD_TYPE_SD)
        u32Arg = u32CardBlkAddr << ((_stSDMMCInfo[p_sdmmc_slot->ipOrder].eHCS == EV_HCS) ? 0 : 9);

    if (M_REG_IMISEL(p_sdmmc_slot->ipOrder))
        dmaDataMapAddr = (dma_addr_t)__virt_to_phys((uintptr_t)pu8DataBuf);
    else
        dmaDataMapAddr = SDMMC_DMA_MAP_address(p_sdmmc_slot, (uintptr_t)pu8DataBuf, 512 * u16BlkCnt, EV_CMDWRITE);

    _pstRsp[p_sdmmc_slot->ipOrder] =
        _SDMMC_DATAReq_Ext(p_sdmmc_slot, 25, u32Arg, u16BlkCnt, 512, EV_DMA, pu8DataBuf, dmaDataMapAddr);

    if ((M_REG_IMISEL(p_sdmmc_slot->ipOrder)) == 0)
        SDMMC_DMA_UNMAP_address(p_sdmmc_slot, dmaDataMapAddr, 512 * u16BlkCnt, EV_CMDWRITE);

    _pstRsp[p_sdmmc_slot->ipOrder] = _SDMMC_CMDReq(p_sdmmc_slot->ipOrder, 12, 0x00000000, EV_R1B); // CMD12;

    return _pstRsp[p_sdmmc_slot->ipOrder];
}

U16_T eMMC_CMD28_CMD29(IpOrder eIP, U32_T u32CardBlkAddr, U8_T u8CmdIdx)
{
    U32_T u32Arg;

    if (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_EMMC)
        u32Arg = u32CardBlkAddr << (_stSDMMCInfo[eIP].u8IfSectorMode ? 0 : 9);
    else if (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_SD)
    {
        pr_err("_SDMMC_CMD28_CMD29: SDHC Card don't support write protection\n");
        return (U16_T)_pstRsp[eIP]->eErrCode;
    }
    _pstRsp[eIP] = _SDMMC_CMDReq(eIP, u8CmdIdx, u32Arg, EV_R1B);

    return (U16_T)_pstRsp[eIP]->eErrCode;
}

U16_T eMMC_CMD30_CMD31_DMA(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32CardBlkAddr, U8_T *pu8DataBuf, U8_T u8CmdIdx)
{
    U32_T      u32Arg, u32Blksize;
    dma_addr_t dmaDataMapAddr;
    IpOrder    eIP = p_sdmmc_slot->ipOrder;

    if (u8CmdIdx == 30)
        u32Blksize = 4;
    else
        u32Blksize = 8;

    if (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_EMMC)
        u32Arg = u32CardBlkAddr << (_stSDMMCInfo[eIP].u8IfSectorMode ? 0 : 9);
    else if (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_SD)
    {
        pr_err("_SDMMC_CMD30_CMD31_DMA: SDHC Card don't support write protection\n");
        return (U16_T)_pstRsp[eIP]->eErrCode;
    }

    dmaDataMapAddr = SDMMC_DMA_MAP_address(p_sdmmc_slot, (uintptr_t)pu8DataBuf, 512, EV_CMDREAD);
    _pstRsp[eIP] =
        _SDMMC_DATAReq_Ext(p_sdmmc_slot, u8CmdIdx, u32Arg, 1, u32Blksize, EV_DMA, pu8DataBuf, dmaDataMapAddr);
    SDMMC_DMA_UNMAP_address(p_sdmmc_slot, dmaDataMapAddr, 512, EV_CMDREAD);

    return (U16_T)_pstRsp[eIP]->eErrCode;
}

static RspStruct *eMMC_CMD35_CMD36(IpOrder eIP, U32_T u32CardBlkAddr, U8_T u8CmdIdx)
{
    U32_T u32Arg;

    if (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_EMMC)
        u32Arg = u32CardBlkAddr << (_stSDMMCInfo[eIP].u8IfSectorMode ? 0 : 9);
    else if (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_SD)
    {
        pr_err("_SDMMC_CMD35_CMD36: SD Card don't support this command\n");
        return _pstRsp[eIP];
    }
    return _SDMMC_CMDReq(eIP, u8CmdIdx, u32Arg, EV_R1);
}

static RspStruct *eMMC_CMD38(IpOrder eIP)
{
    U32_T u32Arg;

    if (_stSDMMCInfo[eIP].u32eMMCFlag & eMMC_FLAG_TRIM)
        u32Arg = 0x1;
    else
        u32Arg = 0x0;
    return _SDMMC_CMDReq(eIP, 38, u32Arg, EV_R1B);
}

/* Erase api */
U16_T eMMC_EraseCMDSeq(IpOrder eIP, U32_T u32CardBlkAddrStart, U32_T u32CardBlkAddrEnd)
{
    _pstRsp[eIP] = eMMC_CMD35_CMD36(eIP, u32CardBlkAddrStart, 35);
    if (_pstRsp[eIP]->eErrCode)
        return (U16_T)_pstRsp[eIP]->eErrCode;

    _pstRsp[eIP] = eMMC_CMD35_CMD36(eIP, u32CardBlkAddrEnd, 36);
    if (_pstRsp[eIP]->eErrCode)
        return (U16_T)_pstRsp[eIP]->eErrCode;

    _pstRsp[eIP] = eMMC_CMD38(eIP);
    if (_pstRsp[eIP]->eErrCode)
        return (U16_T)_pstRsp[eIP]->eErrCode;

    return (U16_T)_pstRsp[eIP]->eErrCode;
}

/*----------------------------------------------------------------------------------------------------------
*
* Function: SDMMC_ReadData
*     @author jeremy.wang (2015/7/23)
* Desc: SDMMC use DMA to Read Blocks
*
* @param p_sdmmc_slot :
* @param u32CardBlkAddr : Card Address(block)
* @param u16BlkCnt : Block Count
* @param pu8DataBuf : Data Buffer Pointer
*
* @return U16_T  : U16 Error Code
----------------------------------------------------------------------------------------------------------*/
U16_T SDMMC_ReadData(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32CardBlkAddr, U16_T u16BlkCnt, U8_T *pu8DataBuf)
{
    IpOrder eIP = p_sdmmc_slot->ipOrder;

    if (u16BlkCnt == 1)
        _pstRsp[eIP] = _SDMMC_CMD17_DMA(p_sdmmc_slot, u32CardBlkAddr, pu8DataBuf);
    else
        _pstRsp[eIP] = _SDMMC_CMD18_DMA(p_sdmmc_slot, u32CardBlkAddr, pu8DataBuf, u16BlkCnt);

    return (U16_T)_pstRsp[eIP]->eErrCode;
}

/*----------------------------------------------------------------------------------------------------------
*
* Function: SDMMC_WriteData
*     @author jeremy.wang (2015/7/23)
* Desc: SDMMC use DMA to Write Blocks
*
* @param p_sdmmc_slot :
* @param u32CardBlkAddr : Card Address(block)
* @param u16BlkCnt : Block Count
* @param pu8DataBuf : Data Buffer Pointer
*
* @return U16_T  : U16 Error Code
----------------------------------------------------------------------------------------------------------*/
U16_T SDMMC_WriteData(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32CardBlkAddr, U16_T u16BlkCnt, U8_T *pu8DataBuf)
{
    IpOrder eIP = p_sdmmc_slot->ipOrder;

    if (u16BlkCnt == 1)
        _pstRsp[eIP] = _SDMMC_CMD24_DMA(p_sdmmc_slot, u32CardBlkAddr, pu8DataBuf);
    else
        _pstRsp[eIP] = _SDMMC_CMD25_DMA(p_sdmmc_slot, u32CardBlkAddr, pu8DataBuf, u16BlkCnt);

    return (U16_T)_pstRsp[eIP]->eErrCode;
}

// for emmc write
U16_T eMMC_WriteData(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32CardBlkAddr, U16_T u16BlkCnt, U8_T *pu8DataBuf)
{
    volatile U16_T u16_BlkCnt, u16Err = 0;
    IpOrder        eIP = p_sdmmc_slot->ipOrder;

    if (NULL == pu8DataBuf)
    {
        pr_err("SDMMC Err: w data buf is NULL: %ph \n", pu8DataBuf);
        return -1;
    }

    if (u32CardBlkAddr + u16BlkCnt > _stSDMMCInfo[eIP].stECSD.u32SEC_COUNT)
    {
        pr_err("SDMMC Err: invalid data range, %Xh > %Xh \n", u32CardBlkAddr + u16BlkCnt,
               _stSDMMCInfo[eIP].stECSD.u32SEC_COUNT);
        return -1;
    }

    // write data
    while (u16BlkCnt)
    {
        if (u16BlkCnt < M_REG_SD_JOB_BLK_CNT_MASK)
            u16_BlkCnt = u16BlkCnt;
        else
            u16_BlkCnt = M_REG_SD_JOB_BLK_CNT_MASK;

        u16Err = SDMMC_WriteData(p_sdmmc_slot, u32CardBlkAddr, u16_BlkCnt, pu8DataBuf);

        if (u16Err)
        {
            pr_err("eMMC Err: W fail: %Xh\n", u16Err);
            break;
        }

        u32CardBlkAddr += u16_BlkCnt;
        pu8DataBuf += u16_BlkCnt << SECTOR_512BYTE_BITS;
        u16BlkCnt -= u16_BlkCnt;
    }

    return u16Err;
}

U16_T eMMC_GetExtCSD(struct sstar_sdmmc_slot *p_sdmmc_slot, U8_T *pu8DataBuf)
{
    IpOrder eIP  = p_sdmmc_slot->ipOrder;
    _pstRsp[eIP] = _SDMMC_EMMC_CMD8_DMA(p_sdmmc_slot, pu8DataBuf);

    return (U16_T)_pstRsp[eIP]->eErrCode;
}

/* Ext CSD get--set relevant */
U16_T eMMC_ExtCSD_Config(struct sstar_sdmmc_slot *p_sdmmc_slot, volatile U8_T *pu8DataBuf)
{
    IpOrder eIP = p_sdmmc_slot->ipOrder;

    _pstRsp[eIP] = _SDMMC_EMMC_CMD8_DMA(p_sdmmc_slot, (U8_T *)pu8DataBuf);
    if (_pstRsp[eIP]->eErrCode)
        return (U16_T)_pstRsp[eIP]->eErrCode;

    //--------------------------------
    if (0 == _stSDMMCInfo[eIP].stECSD.u32SEC_COUNT)
        _stSDMMCInfo[eIP].stECSD.u32SEC_COUNT =
            ((pu8DataBuf[215] << 24) | (pu8DataBuf[214] << 16) | (pu8DataBuf[213] << 8) | (pu8DataBuf[212]))
            - 8; //-8: Toshiba CMD18 access the last block report out of range error

    //-------------------------------
    if (0 == _stSDMMCInfo[eIP].stECSD.u32BOOT_SEC_COUNT)
        _stSDMMCInfo[eIP].stECSD.u32BOOT_SEC_COUNT = pu8DataBuf[226] * 128 * 2;

    //--------------------------------
    if (!_stSDMMCInfo[eIP].stECSD.u8BUS_WIDTH)
    {
        _stSDMMCInfo[eIP].stECSD.u8BUS_WIDTH = pu8DataBuf[183];
        switch (_stSDMMCInfo[eIP].stECSD.u8BUS_WIDTH)
        {
            case 0:
                _stSDMMCInfo[eIP].stECSD.u8BUS_WIDTH = EV_BUS_1BIT;
                break;
            case 1:
                _stSDMMCInfo[eIP].stECSD.u8BUS_WIDTH = EV_BUS_4BITS >> 1;
                break;
            case 2:
                _stSDMMCInfo[eIP].stECSD.u8BUS_WIDTH = EV_BUS_8BITS >> 1;
                break;
            default:
                pr_err("eMMC Err: eMMC BUS_WIDTH not support\n");
                while (1)
                    ;
        }
    }
    //--------------------------------
    if (pu8DataBuf[231] & BIT04_T) // TRIM
        _stSDMMCInfo[eIP].u32eMMCFlag |= eMMC_FLAG_TRIM;
    else
        _stSDMMCInfo[eIP].u32eMMCFlag &= ~eMMC_FLAG_TRIM;

    //--------------------------------
    if (pu8DataBuf[503] & BIT00_T) // HPI
    {
        if (pu8DataBuf[503] & BIT01_T)
            _stSDMMCInfo[eIP].u32eMMCFlag |= eMMC_FLAG_HPI_CMD12;
        else
            _stSDMMCInfo[eIP].u32eMMCFlag |= eMMC_FLAG_HPI_CMD13;
    }
    else
        _stSDMMCInfo[eIP].u32eMMCFlag &= ~(eMMC_FLAG_HPI_CMD12 | eMMC_FLAG_HPI_CMD13);

    //--------------------------------
    if (pu8DataBuf[166] & BIT02_T) // Reliable Write
        _stSDMMCInfo[eIP].stECSD.u16ReliableWBlkCnt = M_REG_SD_JOB_BLK_CNT_MASK;
    else
    {
#if 0
        g_eMMCDrv[emmc_ip].u16_ReliableWBlkCnt = gau8_eMMC_SectorBuf[222];
#else
        if ((pu8DataBuf[503] & BIT00_T) && 1 == pu8DataBuf[222])
            _stSDMMCInfo[eIP].stECSD.u16ReliableWBlkCnt = 1;
        else if (0 == (pu8DataBuf[503] & BIT00_T))
            _stSDMMCInfo[eIP].stECSD.u16ReliableWBlkCnt = pu8DataBuf[222];
        else
        {
            // eMMC_debug(0,1,"eMMC Warn: not support dynamic  Reliable-W\n");
            _stSDMMCInfo[eIP].stECSD.u16ReliableWBlkCnt = 0; // can not support Reliable Write
        }
#endif
    }

    //--------------------------------
    _stSDMMCInfo[eIP].stECSD.u8ErasedMemContent = pu8DataBuf[181];

    //--------------------------------
    _stSDMMCInfo[eIP].stECSD.u8ECSD184_Stroe_Support  = pu8DataBuf[184];
    _stSDMMCInfo[eIP].stECSD.u8ECSD185_HsTiming       = pu8DataBuf[185];
    _stSDMMCInfo[eIP].stECSD.u8ECSD192_Ver            = pu8DataBuf[192];
    _stSDMMCInfo[eIP].stECSD.u8ECSD196_DevType        = pu8DataBuf[196];
    _stSDMMCInfo[eIP].stECSD.u8ECSD197_DriverStrength = pu8DataBuf[197];
    _stSDMMCInfo[eIP].stECSD.u8ECSD248_CMD6TO         = pu8DataBuf[248];
    _stSDMMCInfo[eIP].stECSD.u8ECSD247_PwrOffLongTO   = pu8DataBuf[247];
    _stSDMMCInfo[eIP].stECSD.u8ECSD34_PwrOffCtrl      = pu8DataBuf[34];

    // for GP Partition
    _stSDMMCInfo[eIP].stECSD.u8ECSD160_PartSupField   = pu8DataBuf[160];
    _stSDMMCInfo[eIP].stECSD.u8ECSD224_HCEraseGRPSize = pu8DataBuf[224];
    _stSDMMCInfo[eIP].stECSD.u8ECSD221_HCWpGRPSize    = pu8DataBuf[221];

    /*_stSDMMCInfo[eIP].stECSD.GP_Part[0].u32_PartSize =
        ((pu8DataBuf[145] << 16) | (pu8DataBuf[144] << 8) | (pu8DataBuf[143]))
        * (_stSDMMCInfo[eIP].stECSD.u8ECSD224_HCEraseGRPSize * _stSDMMCInfo[eIP].stECSD.u8ECSD221_HCWpGRPSize *
    0x80000);

    _stSDMMCInfo[eIP].stECSD.GP_Part[1].u32_PartSize =
        ((pu8DataBuf[148] << 16) | (pu8DataBuf[147] << 8) | (pu8DataBuf[146]))
        * (_stSDMMCInfo[eIP].stECSD.u8ECSD224_HCEraseGRPSize * _stSDMMCInfo[eIP].stECSD.u8ECSD221_HCWpGRPSize *
    0x80000);

    _stSDMMCInfo[eIP].stECSD.GP_Part[2].u32_PartSize =
        ((pu8DataBuf[151] << 16) | (pu8DataBuf[150] << 8) | (pu8DataBuf[149]))
        * (_stSDMMCInfo[eIP].stECSD.u8ECSD224_HCEraseGRPSize * _stSDMMCInfo[eIP].stECSD.u8ECSD221_HCWpGRPSize *
    0x80000);

    _stSDMMCInfo[eIP].stECSD.GP_Part[3].u32_PartSize =
        ((pu8DataBuf[154] << 16) | (pu8DataBuf[153] << 8) | (pu8DataBuf[152]))
        * (_stSDMMCInfo[eIP].stECSD.u8ECSD224_HCEraseGRPSize * _stSDMMCInfo[eIP].stECSD.u8ECSD221_HCWpGRPSize *
    0x80000);*/

    // for Max Enhance Size
    _stSDMMCInfo[eIP].stECSD.u8ECSD157_MaxEnhSize_0 = pu8DataBuf[157];
    _stSDMMCInfo[eIP].stECSD.u8ECSD158_MaxEnhSize_1 = pu8DataBuf[158];
    _stSDMMCInfo[eIP].stECSD.u8ECSD159_MaxEnhSize_2 = pu8DataBuf[159];

    _stSDMMCInfo[eIP].stECSD.u8ECSD155_PartSetComplete = pu8DataBuf[155];
    _stSDMMCInfo[eIP].stECSD.u8ECSD166_WrRelParam      = pu8DataBuf[166];

    _stSDMMCInfo[eIP].stECSD.u8ECSD175_ERASE_GROUP_DEF = pu8DataBuf[175];

    // for WP
    _stSDMMCInfo[eIP].stECSD.u8ECSD174_BootWPStatus = pu8DataBuf[174];
    _stSDMMCInfo[eIP].stECSD.u8ECSD173_BootWP       = pu8DataBuf[173];
    _stSDMMCInfo[eIP].stECSD.u8ECSD171_UserWP       = pu8DataBuf[171];
    if (_stSDMMCInfo[eIP].u8IfSectorMode)
        _stSDMMCInfo[eIP].u32WP_group_size = (_stSDMMCInfo[eIP].stECSD.u8ECSD221_HCWpGRPSize
                                              * _stSDMMCInfo[eIP].stECSD.u8ECSD224_HCEraseGRPSize * 0x400);

    //--------------------------------
    // set HW RST
    if (0 == pu8DataBuf[162])
    {
        _pstRsp[eIP]->eErrCode = eMMC_ModifyExtCSD(eIP, eMMC_ExtCSD_WByte, 162, BIT00_T); // RST_FUNC
        if (_pstRsp[eIP]->eErrCode)
        {
            pr_err("eMMC Err: %Xh, eMMC, set Ext_CSD[162]: %Xh fail\n", _pstRsp[eIP]->eErrCode, BIT00_T);
            return (U16_T)_pstRsp[eIP]->eErrCode;
        }
    }

    return (U16_T)_pstRsp[eIP]->eErrCode;
}

U16_T eMMC_ModifyExtCSD(IpOrder eIP, U8_T u8AccessMode, U8_T u8ByteIdx, U8_T u8Value)
{
    U32_T u32Arg;
    U8_T  u8_retry_prg = 0;

    u32Arg = ((u8AccessMode & 3) << 24) | (u8ByteIdx << 16) | (u8Value << 8);

    _pstRsp[eIP] = _SDMMC_EMMC_CMD6(eIP, u32Arg);
    if (_pstRsp[eIP]->eErrCode)
    {
        pr_err("eMMC Err: _SDMMC_EMMC_CMD6: %Xh \n", (U16_T)_pstRsp[eIP]->eErrCode);
        return (U16_T)_pstRsp[eIP]->eErrCode;
    }

    do
    {
        if (u8_retry_prg > 5)
            break;

        _pstRsp[eIP]                    = _SDMMC_CMD13(eIP);
        _stSDMMCInfo[eIP].u32CardStatus = _RetArrToU32(_pstRsp[eIP]->u8ArrRspToken + 1);
        if (_pstRsp[eIP]->eErrCode && !(_stSDMMCInfo[eIP].u32CardStatus & BIT07_T)) // tran state
        {
            pr_err("eMMC Warn: %Xh \n", _stSDMMCInfo[eIP].u32CardStatus);
            return (U16_T)_pstRsp[eIP]->eErrCode;
        }
        else if ((M_SDMMC_CURRSTATE(_stSDMMCInfo[eIP].u32CardStatus) & 0x04)) // tran state
            break;

        u8_retry_prg++;
    } while (!(M_SDMMC_CURRSTATE(_stSDMMCInfo[eIP].u32CardStatus) & 0x04));

    return (U16_T)_pstRsp[eIP]->eErrCode;
}

U16_T eMMC_SetBusSpeed(IpOrder eIP, U8_T u8BusSpeed)
{
    _stSDMMCInfo[eIP].stECSD.u8ECSD185_HsTiming &= ~BITS_MSK_TIMING;
    _stSDMMCInfo[eIP].stECSD.u8ECSD185_HsTiming |= u8BusSpeed;

    _pstRsp[eIP]->eErrCode =
        eMMC_ModifyExtCSD(eIP, eMMC_ExtCSD_WByte, 185, _stSDMMCInfo[eIP].stECSD.u8ECSD185_HsTiming);
    if (_pstRsp[eIP]->eErrCode)
        pr_err("eMMC Err: eMMC_SetBusSpeed: %Xh \n", (U16_T)_pstRsp[eIP]->eErrCode);
    else
        _stSDMMCInfo[eIP].u32MaxClk = CLK_HIGH_SPEED;

    return (U16_T)_pstRsp[eIP]->eErrCode;
}

// Use CMD6 to set ExtCSD[183] BUS_WIDTH
U16_T eMMC_SetBusWidth(IpOrder eIP, U8_T u8BusWidth, U8_T u8IfDDR)
{
    U8_T u8Value;

    // -------------------------------
    switch (u8BusWidth)
    {
        case 1:
            u8Value = 0;
            break;
        case 4:
            u8Value = 1;
            break;
        case 8:
            u8Value = 2;
            break;
        default:
            pr_err("eMMC_SetBusWidth Err: u8BusWidth: %dh is not support, please input 1/4/8 \n", u8BusWidth);
            return 1;
    }

    if (u8IfDDR)
        u8Value |= BIT02_T;

    if (u8IfDDR == 2)
    {
        pr_err("Enhance Strobe\n");
        u8Value |= BIT07_T; // Enhanced Storbe
    }

    // -------------------------------
    _pstRsp[eIP]->eErrCode = eMMC_ModifyExtCSD(eIP, eMMC_ExtCSD_WByte, 183, u8Value); // BUS_WIDTH
    if (_pstRsp[eIP]->eErrCode)
        pr_err("eMMC Err: eMMC_SetBusWidth: %Xh \n", (U16_T)_pstRsp[eIP]->eErrCode);
    else
        Hal_SDMMC_SetDataWidth(eIP, (u8BusWidth >> 1));

    return (U16_T)_pstRsp[eIP]->eErrCode;
}

// Set the target memory in the user partition to be write-protected
U16_T eMMC_USER_WriteProtect_Option(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32CardBlkAddr, U32_T u16BlkCnt,
                                    U8_T u8Mode)
{
    U16_T   u16Err = 0;
    U32_T   u32Start;
    IpOrder eIP = p_sdmmc_slot->ipOrder;

    // check if u32_DataByteCnt is 512B boundary
    if (u32CardBlkAddr + u16BlkCnt > _stSDMMCInfo[eIP].stECSD.u32SEC_COUNT)
    {
        prtstring("eMMC Err: invalid data range, %Xh > %Xh \n", u32CardBlkAddr + u16BlkCnt,
                  _stSDMMCInfo[eIP].stECSD.u32SEC_COUNT);
        return -1;
    }

    if (_stSDMMCInfo[eIP].stECSD.u8ECSD171_UserWP & (BIT02_T | BIT00_T))
    {
        prtstring("eMMC is not in tmp WP mode (%X)\n", _stSDMMCInfo[eIP].stECSD.u8ECSD171_UserWP & (BIT02_T | BIT00_T));
        return -1;
    }

    switch (u8Mode)
    {
        case 0:
        case 1:
            u32Start = ((u32CardBlkAddr / _stSDMMCInfo[eIP].u32WP_group_size) * _stSDMMCInfo[eIP].u32WP_group_size);
            while (u32Start <= (u32CardBlkAddr + u16BlkCnt - 1))
            {
                u16Err = eMMC_CMD28_CMD29(eIP, u32Start, u8Mode ? 29 : 28);
                if (u16Err)
                    return u16Err;

                u32Start += _stSDMMCInfo[eIP].u32WP_group_size;
            }
            break;

        case 2:
        case 3:
            u16Err =
                eMMC_CMD30_CMD31_DMA(p_sdmmc_slot, u32CardBlkAddr, (U8_T *)gu8RspBuf[eIP], (u8Mode == 2) ? 30 : 31);
            if (u16Err)
                return -1;

            if (u8Mode == 2)
                u16Err = (gu8RspBuf[eIP][3] & BIT00_T);
            else
                u16Err = (gu8RspBuf[eIP][7] & 0x03);

            break;

        default:
            prtstring("eMMC_%d mode(%d) is not support.\n", eIP, u8Mode);
            break;
    }

    return u16Err;
}

U16_T eMMC_EraseBlock(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32BlkAddrStart, U32_T u32BlkAddrEnd)
{
    U16_T   u16Err;
    U32_T   u32SectorCnt, u32_i, u32_j;
    IpOrder eIP = p_sdmmc_slot->ipOrder;

    if (_stSDMMCInfo[eIP].u32eMMCFlag & eMMC_FLAG_TRIM)
    {
        u16Err = eMMC_EraseCMDSeq(eIP, u32BlkAddrStart, u32BlkAddrEnd);
        if (u16Err)
        {
            pr_err("eMMC Err: EraseCMDSeq fail 0: %Xh \n", u16Err);
            return u16Err;
        }
    }
    else
    {
        Apply_PAGE_BUFFER();
        for (u32_i = 0; u32_i < (eMMC_SECTOR_BUF_BLK_CNT * 0x200); u32_i++)
            gu8EMMC_SectorBuf[u32_i] = _stSDMMCInfo[eIP].stECSD.u8ErasedMemContent;

        // erase blocks before EraseUnitSize
        u32SectorCnt = u32BlkAddrStart / _stSDMMCInfo[eIP].u32EraseUnitSize;
        u32SectorCnt = (u32SectorCnt + 1) * _stSDMMCInfo[eIP].u32EraseUnitSize;
        u32SectorCnt -= u32BlkAddrStart;
        u32SectorCnt =
            u32SectorCnt > (u32BlkAddrEnd - u32BlkAddrStart) ? (u32BlkAddrEnd - u32BlkAddrStart) : u32SectorCnt;

        for (u32_i = 0; u32_i < u32SectorCnt; u32_i++)
        {
            u32_j = (u32SectorCnt - u32_i) > eMMC_SECTOR_BUF_BLK_CNT ? eMMC_SECTOR_BUF_BLK_CNT : (u32SectorCnt - u32_i);

            u16Err = eMMC_WriteData(p_sdmmc_slot, u32BlkAddrStart + u32_i, u32_j, gu8EMMC_SectorBuf);
            if (u16Err)
            {
                pr_err("eMMC Err: WriteData fail 0, %Xh\n", u16Err);
                return u16Err;
            }

            u32_i += u32_j;
        }
        if ((u32BlkAddrEnd - u32BlkAddrStart) == u32SectorCnt)
            goto LABEL_END_OF_ERASE;

        // erase blocks
        u32_i = (u32BlkAddrEnd - (u32BlkAddrStart + u32SectorCnt)) / _stSDMMCInfo[eIP].u32EraseUnitSize;
        if (u32_i)
        {
            u16Err = eMMC_EraseCMDSeq(eIP, (u32BlkAddrStart + u32SectorCnt),
                                      (u32BlkAddrStart + u32SectorCnt) + u32_i * _stSDMMCInfo[eIP].u32EraseUnitSize);
            if (u16Err)
            {
                pr_err("eMMC Err: EraseCMDSeq fail 1, %Xh\n", u16Err);
                return u16Err;
            }
        }

        // erase blocks after EraseUnitSize
        u32BlkAddrStart = (u32BlkAddrStart + u32SectorCnt) + u32_i * _stSDMMCInfo[eIP].u32EraseUnitSize;

        while (u32BlkAddrStart < u32BlkAddrEnd)
        {
            u32_j = (u32BlkAddrEnd - u32BlkAddrStart) > eMMC_SECTOR_BUF_BLK_CNT ? eMMC_SECTOR_BUF_BLK_CNT
                                                                                : (u32BlkAddrEnd - u32BlkAddrStart);

            u16Err = eMMC_WriteData(p_sdmmc_slot, u32BlkAddrStart, u32_j, gu8EMMC_SectorBuf);
            if (u16Err)
            {
                pr_err("eMMC Err: WriteData fail 1, %Xh\n", u16Err);
                return u16Err;
            }
            u32BlkAddrStart += u32_j;
        }
    }

LABEL_END_OF_ERASE:
    return u16Err;
}

static RspStruct *_SDMMC_Identification(IpOrder eIP)
{
    //------------------------------------------------------------------------------------------------------------
    _pstRsp[eIP]->eErrCode = SDMMC_CMD0(eIP); // CMD0;
    //------------------------------------------------------------------------------------------------------------
    if (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_SD)
    {
        _pstRsp[eIP] = _SDMMC_SD_CMD8(eIP); // CMD8
        //------------------------------------------------------------------------------------------------------------
        if (_pstRsp[eIP]->eErrCode && !(_pstRsp[eIP]->eErrCode & EV_STS_NORSP))
        {
            return _pstRsp[eIP];
        }
        //------------------------------------------------------------------------------------------------------------
        _pstRsp[eIP] = _SDMMC_OCR_ACMD41(eIP, CARD_TYPE_SD); // ACMD41
        //------------------------------------------------------------------------------------------------------------
        if (_pstRsp[eIP]->eErrCode)
            return _pstRsp[eIP];
    }
    else if (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_EMMC)
    {
        //------------------------------------------------------------------------------------------------------------
        _pstRsp[eIP] = _SDMMC_OCR_CMD1(eIP, CARD_TYPE_EMMC); // CMD1
        //------------------------------------------------------------------------------------------------------------
        if (_pstRsp[eIP]->eErrCode)
            return _pstRsp[eIP];
    }

    //--------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_CID_CMD2(eIP); // CMD2;
    //--------------------------------------------------------------------------------------------------------
    if (_pstRsp[eIP]->eErrCode)
        return _pstRsp[eIP];

    //--------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_CMD3_CMD7(eIP, 3, (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_SD) ? EV_R6 : EV_R1); // CMD3;
    //--------------------------------------------------------------------------------------------------------
    if (_pstRsp[eIP]->eErrCode)
        return _pstRsp[eIP];

    if (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_SD)
        _stSDMMCInfo[eIP].u32RCAArg = _RetArrToU32(_pstRsp[eIP]->u8ArrRspToken + 1) & 0xFFFF0000;

    return _pstRsp[eIP];
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: SDMMC_SwitchPAD
 *     @author jeremy.wang (2013/7/30)
 * Desc:
 *
 * @param u8Slot :
 ----------------------------------------------------------------------------------------------------------*/
void SDMMC_SwitchPAD(struct sstar_sdmmc_slot *p_sdmmc_slot)
{
    // Hal_CARD_IPOnceSetting(p_sdmmc_slot->p_mmc_priv->mmc_PMuxInfo);  // use clk_set_rate(), It's best not to use this
    Hal_CARD_ConfigSdPad(p_sdmmc_slot->p_mmc_priv->mmc_PMuxInfo);
    Hal_CARD_InitPADPin(p_sdmmc_slot->p_mmc_priv->mmc_PMuxInfo);

    if (!(p_sdmmc_slot->mmc->caps2 & MMC_CAP2_NO_MMC))
        Hal_eMMC_HardWare_Reset(p_sdmmc_slot->p_mmc_priv->mmc_PMuxInfo);
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: SDMMC_SetPower
 *     @author jeremy.wang (2013/7/30)
 * Desc:
 *
 * @param u8Slot :
 * @param ePower :
 ----------------------------------------------------------------------------------------------------------*/
void SDMMC_SetPower(struct sstar_mmc_priv *p_mmc_priv, PowerEmType ePower)
{
    IpOrder eIP = p_mmc_priv->mmc_PMuxInfo.u8_ipOrder;

    if (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_EMMC)
    {
        return; // no need
    }

    if (ePower == EV_POWER_OFF) // Power Off
    {
        Hal_SDMMC_ClkCtrl(eIP, FALSE, 0);
        Hal_CARD_PullPADPin(p_mmc_priv->mmc_PMuxInfo, EV_PULLDOWN);
        Hal_CARD_PowerOff(p_mmc_priv->mmc_PMuxInfo, WT_POWEROFF); // For SD PAD
    }
    else if (ePower == EV_POWER_ON) // Power Up
    {
        Hal_CARD_PullPADPin(p_mmc_priv->mmc_PMuxInfo, EV_PULLUP);
        Hal_CARD_PowerOn(p_mmc_priv->mmc_PMuxInfo, WT_POWERUP);
    }
    else if (ePower == EV_POWER_UP) // Power On
    {
        Hal_SDMMC_ClkCtrl(eIP, TRUE, WT_POWERON);
        Hal_SDMMC_Reset(eIP); // For SRAM Issue
    }
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: SDMMC_SetClock
 *     @author jeremy.wang (2013/8/15)
 * Desc:
 *
 * @param u8Slot :
 * @param u32ReffClk :
 * @param u8PassLevel :
 *
 * @return U32_T  :
 ----------------------------------------------------------------------------------------------------------*/
U32_T SDMMC_SetClock(struct sstar_mmc_priv *p_mmc_priv, U32_T u32ReffClk)
{
    IpOrder eIP        = p_mmc_priv->mmc_PMuxInfo.u8_ipOrder;
    U32_T   u32RealClk = 0;

    if (u32ReffClk == 0) // Not Set
        u32ReffClk = _stSDMMCInfo[eIP].u32MaxClk;

    if (u32ReffClk > p_mmc_priv->u32_maxClk)
        u32ReffClk = p_mmc_priv->u32_maxClk;

    //************ Set Bus Clock ******************
    u32RealClk = Hal_CARD_FindClockSetting(eIP, u32ReffClk);
    Hal_CARD_SetClock(&p_mmc_priv->mmc_PMuxInfo, u32RealClk);
    Hal_SDMMC_SetNrcDelay(eIP, u32RealClk);

    pr_sd_main("====================== [ Clk Info for Slot: %u ==>(%u Hz)] \n", eIP, u32RealClk);

    return u32RealClk;
}

/*----------------------------------------------------------------------------------------------------------
*
* Function: SDMMC_SetBusTiming
*     @author jeremy.wang (2015/10/2)
* Desc:
*
* @param u8Slot :
* @param u8BusSpdMode :
----------------------------------------------------------------------------------------------------------*/
void SDMMC_SetBusTiming(struct sstar_mmc_priv *p_mmc_priv, BusTimingEmType eBusTiming)
{
    Hal_CARD_SetBustiming(&p_mmc_priv->mmc_PMuxInfo, eBusTiming);
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: SDMMC_CardDetect
 *     @author jeremy.wang (2013/7/29)
 * Desc:
 *
 * @return BOOL_T  :
 ----------------------------------------------------------------------------------------------------------*/
BOOL_T SDMMC_CardDetect(struct sstar_sdmmc_slot *p_sdmmc_slot)
{
    if (!(p_sdmmc_slot->mmc->caps2 & MMC_CAP2_NO_SD))
        return Hal_CARD_GetCdzState(p_sdmmc_slot->p_mmc_priv->mmc_PMuxInfo);
    else
        return TRUE;
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: SDMMC_SetWideBus
 *     @author jeremy.wang (2013/8/15)
 * Desc:
 *
 * @param u8Slot :
 *
 * @return U16_T  :
 ----------------------------------------------------------------------------------------------------------*/
U16_T SDMMC_SetWideBus(IpOrder eIP)
{
    if (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_SD)
    {
        if (_stSDMMCInfo[eIP].u8BusWidth == 4)
        {
            _pstRsp[eIP] = _SDMMC_ACMD6(eIP);

            if (_pstRsp[eIP]->eErrCode)
                return (U16_T)_pstRsp[eIP]->eErrCode;

            Hal_SDMMC_SetDataWidth(eIP, EV_BUS_4BITS);
        }
    }

    return 0;
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: SDMMC_SwitchHighBus
 *     @author jeremy.wang (2013/8/15)
 * Desc:
 *
 * @param u8Slot :
 *
 * @return U16_T  :
 ----------------------------------------------------------------------------------------------------------*/
U16_T SDMMC_SwitchHighBus(struct sstar_sdmmc_slot *p_sdmmc_slot)
{
    IpOrder eIP = (IpOrder)p_sdmmc_slot->ipOrder;

    U8_T  u8BusMode = 0, u8DrvType = 0, u8CurrLimit = 0;
    U32_T u32MaxClk = CLK_HIGH_SPEED;

    if (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_SD)
    {
        if (_stSDMMCInfo[eIP].u8SpecVer == 0) // SD 1.0=> Not Support SD CMD6 for HS
            return 0;

        //************************************** Check Function ***********************************************

        //------------------------------------------------------------------------------------------------------------
        _pstRsp[eIP] = _SDMMC_SD_CMD6(p_sdmmc_slot, 0, 0, 1, gu8RspBuf[eIP]); // Query Group 1
        //------------------------------------------------------------------------------------------------------------
        if (_pstRsp[eIP]->eErrCode)
            return (U16_T)_pstRsp[eIP]->eErrCode;

        u8BusMode = gu8RspBuf[eIP][13];

        // SD 3.0
        if (_stSDMMCInfo[eIP].u8SpecVer1)
        {
            _stSDMMCInfo[eIP].u8SD3BusMode = u8BusMode;

            //------------------------------------------------------------------------------------------------------------
            _pstRsp[eIP] = _SDMMC_SD_CMD6(p_sdmmc_slot, 0, 2, 1, gu8RspBuf[eIP]); // Query Group 3
            //------------------------------------------------------------------------------------------------------------
            if (_pstRsp[eIP]->eErrCode)
                return (U16_T)_pstRsp[eIP]->eErrCode;

            _stSDMMCInfo[eIP].u8SD3DrvType = gu8RspBuf[eIP][9];
            //------------------------------------------------------------------------------------------------------------
            _pstRsp[eIP] = _SDMMC_SD_CMD6(p_sdmmc_slot, 0, 3, 1, gu8RspBuf[eIP]); // Query Group 4
            //------------------------------------------------------------------------------------------------------------
            if (_pstRsp[eIP]->eErrCode)
                return (U16_T)_pstRsp[eIP]->eErrCode;

            _stSDMMCInfo[eIP].u8SD3CurrLimit = gu8RspBuf[eIP][7];
        }

        // printk("===> gu8RspBuf[13,9,7] = (0x%02X)(0x%02X)(0x%02X)\n", u8BusMode, _stSDMMCInfo[u8Slot].u8SD3DrvType,
        // _stSDMMCInfo[u8Slot].u8SD3CurrLimit);

        //************************************** Set Funciton ***********************************************

        if (!(_stSDMMCInfo[eIP].u32OCR & R_OCR_S18)) // SD 2.0, SD3.0 => SD2.0
        {
            u8BusMode = u8BusMode & D_SDMMC_BUSMODE;

            if (u8BusMode & SD_MODE_HIGH_SPEED) // Support High Speed
            {
                _stSDMMCInfo[eIP].u8AccessMode = HIGH_SPEED_BUS_SPEED;

                //------------------------------------------------------------------------------------------------------------
                _pstRsp[eIP] = _SDMMC_SD_CMD6(p_sdmmc_slot, 1, 0, _stSDMMCInfo[eIP].u8AccessMode,
                                              gu8RspBuf[eIP]); // Set Group 1
                //------------------------------------------------------------------------------------------------------------
                if (_pstRsp[eIP]->eErrCode)
                    return (U16_T)_pstRsp[eIP]->eErrCode;

                if ((gu8RspBuf[eIP][16] & 0xF) != _stSDMMCInfo[eIP].u8AccessMode)
                    printk("_[sdmmc_%u] Warning: Problem switching high bus speed mode!\n", eIP);
                else
                    _stSDMMCInfo[eIP].u32MaxClk = CLK_HIGH_SPEED;
            }
        }
        else // SD3.0
        {
            u8DrvType = _stSDMMCInfo[eIP].u8SD3DrvType & D_SDMMC_DRVTYPE;

            if (u8DrvType & SD_DRIVER_TYPE_B)
                _stSDMMCInfo[eIP].u8DrvStrength = UHS_B_DRV_TYPE;
            else if (u8DrvType & SD_DRIVER_TYPE_A)
                _stSDMMCInfo[eIP].u8DrvStrength = UHS_A_DRV_TYPE;
            else if (u8DrvType & SD_DRIVER_TYPE_C)
                _stSDMMCInfo[eIP].u8DrvStrength = UHS_C_DRV_TYPE;
            else if (u8DrvType & SD_DRIVER_TYPE_D)
                _stSDMMCInfo[eIP].u8DrvStrength = UHS_D_DRV_TYPE;

            //------------------------------------------------------------------------------------------------------------
            _pstRsp[eIP] = _SDMMC_SD_CMD6(p_sdmmc_slot, 1, 2, _stSDMMCInfo[eIP].u8DrvStrength,
                                          gu8RspBuf[eIP]); // Set Group 3
            //------------------------------------------------------------------------------------------------------------
            if (_pstRsp[eIP]->eErrCode)
                return (U16_T)_pstRsp[eIP]->eErrCode;

            if ((gu8RspBuf[eIP][15] & 0xF) != _stSDMMCInfo[eIP].u8DrvStrength)
                printk("_[sdmmc_%u] Warning: Problem switching drive strength!\n", eIP);

            u8CurrLimit = _stSDMMCInfo[eIP].u8SD3CurrLimit & D_SDMMC_CURRLMT;

            if (u8CurrLimit & SD_CURR_LMT_800)
                _stSDMMCInfo[eIP].u8CurrMax = UHS_800_CURR_LMT;
            else if (u8CurrLimit & SD_CURR_LMT_600)
                _stSDMMCInfo[eIP].u8CurrMax = UHS_600_CURR_LMT;
            else if (u8CurrLimit & SD_CURR_LMT_400)
                _stSDMMCInfo[eIP].u8CurrMax = UHS_400_CURR_LMT;
            else if (u8CurrLimit & SD_CURR_LMT_200)
                _stSDMMCInfo[eIP].u8CurrMax = UHS_200_CURR_LMT;

            //------------------------------------------------------------------------------------------------------------
            _pstRsp[eIP] =
                _SDMMC_SD_CMD6(p_sdmmc_slot, 1, 3, _stSDMMCInfo[eIP].u8CurrMax, gu8RspBuf[eIP]); // Set Group 4
            //------------------------------------------------------------------------------------------------------------
            if (_pstRsp[eIP]->eErrCode)
                return (U16_T)_pstRsp[eIP]->eErrCode;
            if (((gu8RspBuf[eIP][15] >> 4) & 0xF) != _stSDMMCInfo[eIP].u8DrvStrength)
                printk("_[sdmmc_%u] Warning: Problem switching current limit!\n", eIP);

            u8BusMode = _stSDMMCInfo[eIP].u8SD3BusMode & D_SDMMC_BUSMODE;

            if (u8BusMode & SD_MODE_UHS_SDR104) // Support SDR 104
            {
                _stSDMMCInfo[eIP].u8AccessMode = UHS_SDR104_BUS_SPEED;
                u32MaxClk                      = CLK_SDR104_SPEED;
            }
            else if (u8BusMode & SD_MODE_UHS_DDR50) // Support DDR 50
            {
                _stSDMMCInfo[eIP].u8AccessMode = UHS_DDR50_BUS_SPEED;
                u32MaxClk                      = CLK_DDR50_SPEED;
            }
            else if (u8BusMode & SD_MODE_UHS_SDR50) // Support SDR 50
            {
                _stSDMMCInfo[eIP].u8AccessMode = UHS_SDR50_BUS_SPEED;
                u32MaxClk                      = CLK_SDR50_SPEED;
            }
            else if (u8BusMode & SD_MODE_UHS_SDR25) // Support SDR 25
            {
                _stSDMMCInfo[eIP].u8AccessMode = UHS_SDR25_BUS_SPEED;
                u32MaxClk                      = CLK_SDR25_SPEED;
            }
            else if (u8BusMode & SD_MODE_UHS_SDR12) // Support SDR 12
            {
                _stSDMMCInfo[eIP].u8AccessMode = UHS_SDR12_BUS_SPEED;
                u32MaxClk                      = CLK_SDR12_SPEED;
            }

            //------------------------------------------------------------------------------------------------------------
            _pstRsp[eIP] = _SDMMC_SD_CMD6(p_sdmmc_slot, 1, 0, _stSDMMCInfo[eIP].u8AccessMode,
                                          gu8RspBuf[eIP]); // Set Group 1
            //------------------------------------------------------------------------------------------------------------
            if (_pstRsp[eIP]->eErrCode)
                return (U16_T)_pstRsp[eIP]->eErrCode;

            if ((gu8RspBuf[eIP][16] & 0xF) != _stSDMMCInfo[eIP].u8AccessMode)
                printk("_[sdmmc_%u] Warning: Problem switching bus speed mode!\n", eIP);
            else
                _stSDMMCInfo[eIP].u32MaxClk = u32MaxClk;
        }
    }

    return 0;
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: SDMMC_Init
 *     @author jeremy.wang (2013/7/30)
 * Desc: SDMMC driver init
 *
 * @param u8Slot : Slot ID
 *
 * @return U16_T  : U16 Error Code
 ----------------------------------------------------------------------------------------------------------*/
U16_T SDMMC_Init(struct sstar_sdmmc_slot *p_sdmmc_slot)
{
    IpOrder eIP = p_sdmmc_slot->ipOrder;

    _SDMMC_InfoInit(p_sdmmc_slot);

    if (!(p_sdmmc_slot->mmc->caps2 & MMC_CAP2_NO_MMC))
        Hal_eMMC_HardWare_Reset(p_sdmmc_slot->p_mmc_priv->mmc_PMuxInfo);

    Hal_SDMMC_Reset(eIP);
    SDMMC_SwitchPAD(p_sdmmc_slot);
    SDMMC_SetPower(p_sdmmc_slot->p_mmc_priv, EV_POWER_OFF);
    SDMMC_SetPower(p_sdmmc_slot->p_mmc_priv, EV_POWER_ON);
    SDMMC_SetPower(p_sdmmc_slot->p_mmc_priv, EV_POWER_UP);

    SDMMC_SetClock(p_sdmmc_slot->p_mmc_priv, 400000);
    SDMMC_SetBusTiming(p_sdmmc_slot->p_mmc_priv, EV_BUS_LOW);

    Hal_SDMMC_SetDataWidth(eIP, EV_BUS_1BIT);
    // Hal_SDMMC_SetSDIOClk(eIP, TRUE); //For Measure Clock, Don't Stop Clock

    //--------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_Identification(eIP);

    //--------------------------------------------------------------------------------------------------------
    if (_pstRsp[eIP]->eErrCode)
        pr_sd_main(" errCmd:%d:x%x\n", _pstRsp[eIP]->u8Cmd, _pstRsp[eIP]->eErrCode);
    if (_pstRsp[eIP]->eErrCode)
        return (U16_T)_pstRsp[eIP]->eErrCode;

    //--------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_CSD_CMD9(eIP); // CMD9
    //--------------------------------------------------------------------------------------------------------
    if (_pstRsp[eIP]->eErrCode)
        return (U16_T)_pstRsp[eIP]->eErrCode;

    if (geCardType[eIP] == CARD_TYPE_EMMC)
    {
        //--------------------------------------------------------------------------------------------------------
        _pstRsp[eIP] = _SDMMC_CMD13(eIP); // CMD13
        //--------------------------------------------------------------------------------------------------------
        if (!_pstRsp[eIP]->eErrCode)
        {
            _stSDMMCInfo[eIP].u32CardStatus = _RetArrToU32(_pstRsp[eIP]->u8ArrRspToken + 1);
            if (!(M_SDMMC_CURRSTATE(_stSDMMCInfo[eIP].u32CardStatus) & 0x03)) // stand by state
            {
                _pstRsp[eIP]->eErrCode = EV_STATE_ERR;
                return (U16_T)_pstRsp[eIP]->eErrCode;
            }
        }
        else
            return (U16_T)_pstRsp[eIP]->eErrCode;
    }

    //--------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_CMD3_CMD7(eIP, 7, EV_R1B);
    ; // CMD7;
    //--------------------------------------------------------------------------------------------------------
    if (_pstRsp[eIP]->eErrCode)
        return (U16_T)_pstRsp[eIP]->eErrCode;

    if (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_SD)
        _pstRsp[eIP] = _SDMMC_ACMD51(p_sdmmc_slot, gu8RspBuf[eIP]);

    if (_stSDMMCInfo[eIP].eCardType == CARD_TYPE_EMMC)
        _pstRsp[eIP]->eErrCode = eMMC_ExtCSD_Config(p_sdmmc_slot, gu8RspBuf[eIP]);

    if (!_pstRsp[eIP]->eErrCode)
        _stSDMMCInfo[eIP].u8Initial = 1;

    return (U16_T)_pstRsp[eIP]->eErrCode;
}

//###########################################################################################################

//###########################################################################################################
/*----------------------------------------------------------------------------------------------------------
*
* Function: SDMMC_CIF_BLK_R
*     @author jeremy.wang (2015/7/24)
* Desc: SDMMC use CIFD to Read Blocks
*
* @param eIP : eIP ID
* @param u32CardBlkAddr : Card Address (block)
* @param pu8DataBuf :  Data Buffer Pointer
*
* @return U16_T  : U16 Error Code
----------------------------------------------------------------------------------------------------------*/
U16_T SDMMC_CIF_BLK_R(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32CardBlkAddr, volatile U8_T *pu8DataBuf)
{
    IpOrder eIP = (IpOrder)p_sdmmc_slot->ipOrder;

    //------------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_CMD17_CIFD(p_sdmmc_slot, u32CardBlkAddr, pu8DataBuf); // CMD17
    //------------------------------------------------------------------------------------------------------------
    return (U16_T)_pstRsp[eIP]->eErrCode;
}

/*----------------------------------------------------------------------------------------------------------
*
* Function: SDMMC_CIF_BLK_W
*     @author jeremy.wang (2015/7/24)
* Desc: SDMMC use CIFD to Write Blocks
*
* @param eIP : eIP ID
* @param u32CardBlkAddr : Card Address (block)
* @param pu8DataBuf : Data Buffer Pointer
*
* @return U16_T  : U16 Error Code
----------------------------------------------------------------------------------------------------------*/
U16_T SDMMC_CIF_BLK_W(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32CardBlkAddr, volatile U8_T *pu8DataBuf)
{
    IpOrder eIP = (IpOrder)p_sdmmc_slot->ipOrder;

    //------------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_CMD24_CIFD(p_sdmmc_slot, u32CardBlkAddr, pu8DataBuf); // CMD24
    //------------------------------------------------------------------------------------------------------------
    return (U16_T)_pstRsp[eIP]->eErrCode;
}

/*----------------------------------------------------------------------------------------------------------
*
* Function: SDMMC_ADMA_BLK_R
*     @author jeremy.wang (2015/7/24)
* Desc:
*
* @param u8Slot : Slot ID
* @param u32CardAddr : Card Address
* @param pu32ArrDAddr : DMA Address Array
* @param pu16ArrBCnt : DMA Block Count Array
* @param u16ItemCnt : DMA Item
* @param pDMATable : Memory Pointer to Find DMA Table
*
* @return U16_T  : U16 Error Code
----------------------------------------------------------------------------------------------------------*/
U16_T SDMMC_ADMA_BLK_R(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32CardAddr, dma_addr_t *puArrDAddr,
                       U16_T *pu16ArrBCnt, U16_T u16ItemCnt, volatile void *pDMATable)
{
    U8_T    u8CMD  = 17;
    U16_T   u16Ret = 0, u16Item = 0, u16BlkCnt = 0;
    IpOrder eIP = (IpOrder)p_sdmmc_slot->ipOrder;

    BOOL_T bEnd = (FALSE);

    if (_stSDMMCInfo[eIP].eHCS == EV_HCS)
        u32CardAddr >>= 9;

    for (u16Item = 0; u16Item < u16ItemCnt; u16Item++)
    {
        if (u16Item == (u16ItemCnt - 1))
            bEnd = (TRUE);

        u16BlkCnt += pu16ArrBCnt[u16Item];

        Hal_SDMMC_ADMASetting((volatile void *)pDMATable, u16Item, pu16ArrBCnt[u16Item] << 9, pu16ArrBCnt[u16Item],
                              Hal_CARD_TransMIUAddr(puArrDAddr[u16Item], NULL), 0, bEnd);
    }

    if (u16BlkCnt > 1)
        u8CMD = 18;

    //------------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_DATAReq_Ext(p_sdmmc_slot, u8CMD, u32CardAddr, u16BlkCnt, 512, EV_ADMA, pDMATable,
                                      AdmaScriptsAddr); // CMD17, CMD18
    //------------------------------------------------------------------------------------------------------------
    u16Ret = _pstRsp[eIP]->eErrCode;

    if (u16BlkCnt > 1)
        //--------------------------------------------------------------------------------------------------------
        _pstRsp[eIP] = _SDMMC_CMDReq(eIP, 12, 0x00000000, EV_R1B); // CMD12;
    //--------------------------------------------------------------------------------------------------------

    return u16Ret;
}

/*----------------------------------------------------------------------------------------------------------
*
* Function: SDMMC_ADMA_BLK_W
*     @author jeremy.wang (2015/7/24)
* Desc:
*
* @param u8Slot : Slot ID
* @param u32CardAddr : Card Address
* @param pu32ArrDAddr : DMA Address Array
* @param pu16ArrBCnt : DMA Block Count Array
* @param u16ItemCnt : DMA Item
* @param pDMATable : Memory Pointer to Find DMA Table
*
* @return U16_T  : U16 Error Code
----------------------------------------------------------------------------------------------------------*/
U16_T SDMMC_ADMA_BLK_W(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32CardAddr, dma_addr_t *puArrDAddr,
                       U16_T *pu16ArrBCnt, U16_T u16ItemCnt, volatile void *pDMATable)
{
    U8_T    u8CMD  = 24;
    U16_T   u16Ret = 0, u16Item = 0, u16BlkCnt = 0;
    IpOrder eIP = (IpOrder)p_sdmmc_slot->ipOrder;

    BOOL_T bEnd = (FALSE);

    if (_stSDMMCInfo[eIP].eHCS == EV_HCS)
        u32CardAddr >>= 9;

    for (u16Item = 0; u16Item < u16ItemCnt; u16Item++)
    {
        if (u16Item == (u16ItemCnt - 1))
            bEnd = (TRUE);

        u16BlkCnt += pu16ArrBCnt[u16Item];

        Hal_SDMMC_ADMASetting((volatile void *)pDMATable, u16Item, pu16ArrBCnt[u16Item] << 9, pu16ArrBCnt[u16Item],
                              Hal_CARD_TransMIUAddr(puArrDAddr[u16Item], NULL), 0, bEnd);
    }

    if (u16BlkCnt > 1)
        u8CMD = 25;

    //------------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_DATAReq_Ext(p_sdmmc_slot, u8CMD, u32CardAddr, u16BlkCnt, 512, EV_ADMA, pDMATable,
                                      AdmaScriptsAddr); // CMD24, CMD25
    //------------------------------------------------------------------------------------------------------------
    u16Ret = _pstRsp[eIP]->eErrCode;

    if (u16BlkCnt > 1)
        //--------------------------------------------------------------------------------------------------------
        _pstRsp[eIP] = _SDMMC_CMDReq(eIP, 12, 0x00000000, EV_R1B); // CMD12;
    //--------------------------------------------------------------------------------------------------------

    return u16Ret;
}
