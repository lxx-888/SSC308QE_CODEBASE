/*
 * drv_sdmmc_command.h- Sigmastar
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
 * FileName drv_sdmmc_command.h
 *     @author jeremy.wang (2013/07/26)
 * Desc:
 *     This file is the header file of ms_sdmmc_drv.c.
 *
 ***************************************************************************************************************/
#ifndef __SS_SDMMC_COMMAND_H
#define __SS_SDMMC_COMMAND_H

#include "drv_sdmmc_lnx.h"
#include "hal_sdmmc_base.h"
#include "hal_sdmmc_v5.h"

//***********************************************************************************************************
// Config Setting (Externel)
//***********************************************************************************************************
#define V_SDMMC_CARDNUMS 1
#define V_SDMMC1_MAX_CLK 48000000
#define V_SDMMC2_MAX_CLK 48000000
#define V_SDMMC3_MAX_CLK 48000000 // Dummy Setting

#define EN_SDMMC_CDZREV              (TRUE)
#define SDMMC_FEATURE_RELIABLE_WRITE (FALSE)

//    #define WT_POWERUP                 20 //(ms)
//    #define WT_POWERON                 60 //(ms)
//    #define WT_POWEROFF                25 //(ms)

//============================================
// OCR Register
//============================================
#define R_OCR_LOW   BIT07_T
#define R_OCR_27_28 BIT15_T
#define R_OCR_28_29 BIT16_T
#define R_OCR_29_30 BIT17_T
#define R_OCR_30_31 BIT18_T
#define R_OCR_31_32 BIT19_T
#define R_OCR_32_33 BIT20_T
#define R_OCR_33_34 BIT21_T
#define R_OCR_34_35 BIT22_T
#define R_OCR_35_36 BIT23_T
#define R_OCR_S18   BIT24_T
#define R_OCR_CCS   BIT30_T
#define R_OCR_READY BIT31_T

#define SD_OCR_RANGE (R_OCR_27_28 | R_OCR_28_29 | R_OCR_29_30 | R_OCR_30_31 | R_OCR_31_32 | R_OCR_32_33)

#define CLK_DEF_SPEED    25000000
#define CLK_HIGH_SPEED   50000000
#define CLK_SDR104_SPEED 208000000
#define CLK_SDR50_SPEED  100000000
#define CLK_DDR50_SPEED  CLK_HIGH_SPEED
#define CLK_SDR25_SPEED  CLK_HIGH_SPEED
#define CLK_SDR12_SPEED  CLK_DEF_SPEED

#define DEF_SPEED_BUS_SPEED  0
#define UHS_SDR12_BUS_SPEED  0
#define HIGH_SPEED_BUS_SPEED 1
#define UHS_SDR25_BUS_SPEED  1
#define UHS_SDR50_BUS_SPEED  2
#define UHS_SDR104_BUS_SPEED 3
#define UHS_DDR50_BUS_SPEED  4

#define SD_MODE_HIGH_SPEED (1 << HIGH_SPEED_BUS_SPEED)
#define SD_MODE_UHS_SDR12  (1 << UHS_SDR12_BUS_SPEED)
#define SD_MODE_UHS_SDR25  (1 << UHS_SDR25_BUS_SPEED)
#define SD_MODE_UHS_SDR50  (1 << UHS_SDR50_BUS_SPEED)
#define SD_MODE_UHS_SDR104 (1 << UHS_SDR104_BUS_SPEED)
#define SD_MODE_UHS_DDR50  (1 << UHS_DDR50_BUS_SPEED)

#define UHS_B_DRV_TYPE 0
#define UHS_A_DRV_TYPE 1
#define UHS_C_DRV_TYPE 2
#define UHS_D_DRV_TYPE 3
#ifndef SD_DRIVER_TYPE_B
#define SD_DRIVER_TYPE_B (1 << UHS_B_DRV_TYPE)
#endif
#ifndef SD_DRIVER_TYPE_A
#define SD_DRIVER_TYPE_A (1 << UHS_A_DRV_TYPE)
#endif
#ifndef SD_DRIVER_TYPE_C
#define SD_DRIVER_TYPE_C (1 << UHS_C_DRV_TYPE)
#endif
#ifndef SD_DRIVER_TYPE_D
#define SD_DRIVER_TYPE_D (1 << UHS_D_DRV_TYPE)
#endif
#define UHS_200_CURR_LMT 0
#define UHS_400_CURR_LMT 1
#define UHS_600_CURR_LMT 2
#define UHS_800_CURR_LMT 3

#define SD_CURR_LMT_200 (1 << UHS_200_CURR_LMT)
#define SD_CURR_LMT_400 (1 << UHS_400_CURR_LMT)
#define SD_CURR_LMT_600 (1 << UHS_600_CURR_LMT)
#define SD_CURR_LMT_800 (1 << UHS_800_CURR_LMT)

#define eMMC_ExtCSD_SetBit 1
#define eMMC_ExtCSD_ClrBit 2
#define eMMC_ExtCSD_WByte  3

#define eMMC_PwrOffNotif_OFF   0
#define eMMC_PwrOffNotif_ON    1
#define eMMC_PwrOffNotif_SHORT 2
#define eMMC_PwrOffNotif_LONG  3

#define eMMC_FLAG_TRIM      BIT00_T
#define eMMC_FLAG_HPI_CMD12 BIT01_T
#define eMMC_FLAG_HPI_CMD13 BIT02_T

#define BITS_MSK_TIMING  0x0F
#define eMMC_SPEED_OLD   0 // ECSD[185]
#define eMMC_SPEED_HIGH  1
#define eMMC_SPEED_HS200 2
#define eMMC_SPEED_HS400 3

#define SECTOR_512BYTE_BITS     9
#define eMMC_SECTOR_BUF_BLK_CNT 0x20 // 16k

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
    EV_HCS = R_OCR_CCS,
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
    U8_T  ERASE_GRP_SIZE;
    U8_T  ERASE_GRP_MULT;
    U8_T  PERM_W_PROTECT;
    U8_T  TEMP_W_PROTECT;
    U32_T CAPCITY;

} CARDCSDStruct;

typedef struct
{
    U32_T u32SEC_COUNT;
    U32_T u32BOOT_SEC_COUNT;

    U8_T  u8BUS_WIDTH;
    U8_T  u8ErasedMemContent;
    U16_T u16ReliableWBlkCnt;
    U8_T  u8ECSD185_HsTiming, u8ECSD192_Ver, u8ECSD196_DevType, u8ECSD197_DriverStrength;
    U8_T  u8ECSD248_CMD6TO, u8ECSD247_PwrOffLongTO, u8ECSD34_PwrOffCtrl;
    U8_T  u8ECSD160_PartSupField, u8ECSD224_HCEraseGRPSize, u8ECSD221_HCWpGRPSize, u8ECSD175_ERASE_GROUP_DEF;
    U8_T  u8ECSD174_BootWPStatus, u8ECSD173_BootWP, u8ECSD171_UserWP;
    U8_T  u8ECSD159_MaxEnhSize_2, u8ECSD158_MaxEnhSize_1, u8ECSD157_MaxEnhSize_0;
    U8_T  u8ECSD155_PartSetComplete, u8ECSD166_WrRelParam;
    U8_T  u8ECSD184_Stroe_Support;

} CARDECSDStruct;

typedef struct
{
    U32_T          u32RCAArg; // Relative Card Address
    U32_T          u32OCR;
    U32_T          u32CardStatus;
    U32_T          u32MaxClk;
    U8_T           u8Initial;
    U8_T           u8SpecVer;
    U8_T           u8SpecVer1;
    U8_T           u8BusWidth;
    U8_T           u8AccessMode;
    U8_T           u8DrvStrength;
    U8_T           u8CurrMax;
    U8_T           u8SD3BusMode;
    U8_T           u8SD3DrvType;
    U8_T           u8SD3CurrLimit;
    U8_T           u8IfSectorMode;
    U32_T          u32WP_group_size; //
    U32_T          u32eMMCFlag;      //
    U32_T          u32EraseUnitSize;
    CardType       eCardType;
    SDCAPEmType    eHCS;
    CARDCIDStruct  stCID;
    CARDCSDStruct  stCSD;
    CARDECSDStruct stECSD;

} SDMMCInfoStruct;

typedef enum
{
    EV_POWER_OFF = 0,
    EV_POWER_ON  = 1,
    EV_POWER_UP  = 2,

} PowerEmType;

void       Apply_PAGE_BUFFER(void);
dma_addr_t SDMMC_DMA_MAP_address(struct sstar_sdmmc_slot *p_sdmmc_slot, uintptr_t ulongBuffer, U32_T u32_ByteCnt,
                                 CmdEmType eCmdType);
void       SDMMC_DMA_UNMAP_address(struct sstar_sdmmc_slot *p_sdmmc_slot, dma_addr_t dma_DMAAddr, U32_T u32_ByteCnt,
                                   CmdEmType eCmdType);
U16_T      SDMMC_CMD0(IpOrder eIP);
U16_T      SDMMC_CMD16(IpOrder eIP, U32_T u32BlkLength);
U16_T      SDMMC_CMD23(IpOrder eIP, U16_T u16BlkCount);
U16_T      eMMC_CMD28_CMD29(IpOrder eIP, U32_T u32CardBlkAddr, U8_T u8CmdIdx);
U16_T      eMMC_CMD30_CMD31_DMA(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32CardBlkAddr, U8_T *pu8DataBuf,
                                U8_T u8CmdIdx);

U16_T eMMC_GetExtCSD(struct sstar_sdmmc_slot *p_sdmmc_slot, U8_T *pu8DataBuf);
U16_T eMMC_EraseCMDSeq(IpOrder eIP, U32_T u32CardBlkAddrStart, U32_T u32CardBlkAddrEnd);
U16_T eMMC_ExtCSD_Config(struct sstar_sdmmc_slot *p_sdmmc_slot, volatile U8_T *pu8DataBuf);
U16_T eMMC_ModifyExtCSD(IpOrder eIP, U8_T u8AccessMode, U8_T u8ByteIdx, U8_T u8Value);
U16_T eMMC_SetBusSpeed(IpOrder eIP, U8_T u8BusSpeed);
U16_T eMMC_SetBusWidth(IpOrder eIP, U8_T u8BusWidth, U8_T u8IfDDR);
U16_T eMMC_USER_WriteProtect_Option(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32CardBlkAddr, U32_T u16BlkCnt,
                                    U8_T u8Mode);

void SDMMC_SwitchPAD(struct sstar_sdmmc_slot *p_sdmmc_slot);

void  SDMMC_SetPower(struct sstar_mmc_priv *p_mmc_priv, PowerEmType ePower);
U32_T SDMMC_SetClock(struct sstar_mmc_priv *p_mmc_priv, U32_T u32ReffClk);
void  SDMMC_SetBusTiming(struct sstar_mmc_priv *p_mmc_priv, BusTimingEmType eBusTiming);

BOOL_T SDMMC_CardDetect(struct sstar_sdmmc_slot *p_sdmmc_slot);

U16_T SDMMC_SetWideBus(IpOrder eIP);
U16_T SDMMC_SwitchHighBus(struct sstar_sdmmc_slot *p_sdmmc_slot);
U16_T SDMMC_Init(struct sstar_sdmmc_slot *p_sdmmc_slot);

U16_T SDMMC_CIF_BLK_R(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32CardBlkAddr, volatile U8_T *pu8DataBuf);
U16_T SDMMC_CIF_BLK_W(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32CardBlkAddr, volatile U8_T *pu8DataBuf);
U16_T SDMMC_ReadData(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32CardBlkAddr, U16_T u16BlkCnt, U8_T *pu8DataBuf);
U16_T SDMMC_WriteData(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32CardBlkAddr, U16_T u16BlkCnt, U8_T *pu8DataBuf);
U16_T eMMC_WriteData(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32CardBlkAddr, U16_T u16BlkCnt, U8_T *pu8DataBuf);
U16_T eMMC_EraseBlock(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32BlkAddrStart, U32_T u32BlkAddrEnd);

//###########################################################################################################

//###########################################################################################################
U16_T SDMMC_ADMA_BLK_R(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32CardAddr, dma_addr_t *puArrDAddr,
                       U16_T *pu16ArrBCnt, U16_T u16ItemCnt, volatile void *pDMATable);
U16_T SDMMC_ADMA_BLK_W(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32CardAddr, dma_addr_t *puArrDAddr,
                       U16_T *pu16ArrBCnt, U16_T u16ItemCnt, volatile void *pDMATable);

#endif // End of __SS_SDMMC_COMMAND_H
