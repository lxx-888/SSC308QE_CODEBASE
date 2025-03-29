/*
 * drv_sdmmc_debug.c- Sigmastar
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
 * FileName drv_sdmmc_debug.c
 *     @author jeremy.wang (2015/10/01)
 * Desc:
 *     This layer is between UBOOT Common API layer and SDMMC Driver layer.
 *     (1) The goal is we could modify any verification flow but don't modify any sdmmc driver code.
 *     (2) Timer Test, PAD Test, Init Test, CIFD/DMA Test, Burning Test
 *
 ***************************************************************************************************************/
#include "cam_os_wrapper.h"
#include <linux/mmc/host.h>
#include "core.h"
#include "drv_sdmmc_lnx.h"
#include "hal_sdmmc_v5.h"
#include "hal_sdmmc_platform.h"
#include "hal_sdmmc_platform_pri_config.h"
#include "drv_sdmmc_command.h"
#include "drv_sdmmc_debug.h"

//***********************************************************************************************************
// Config Setting (Internel)
//***********************************************************************************************************
#if 1
#define prtstring(fmt, arg...) printk(KERN_CONT fmt, ##arg)
#else
#define prtstring(fmt, arg...)
#endif
extern CamOsMutex_t     sdmmc_mutex[SDMMC_NUM_TOTAL];
extern struct list_head sdmmc_command_list;
extern U32_T            gu32_SdmmcClk[SDMMC_NUM_TOTAL];
extern U32_T            gu32_SdmmcStatus[SDMMC_NUM_TOTAL];
extern S32_T            gu32_SdmmcCurCMD[SDMMC_NUM_TOTAL];
extern U8_T *           gu8EMMC_SectorBuf; // 512 bytes

//###########################################################################################################
//###########################################################################################################
void _Get_Sdmmc_Status(U32_T *u32Status, U8_T *u8Buf)
{
    switch (*u32Status)
    {
        case EV_STS_OK:
            sprintf(u8Buf, "%s", "OK");
            break;
        case EV_STS_RD_CERR:
            sprintf(u8Buf, "%s", "Err_<Read CRC Error>");
            break;
        case EV_STS_WD_CERR:
            sprintf(u8Buf, "%s", "Err_<Write CRC Error>");
            break;
        case EV_STS_WR_TOUT:
            sprintf(u8Buf, "%s", "Err_<Write Timeout>");
            break;
        case EV_STS_NORSP:
            sprintf(u8Buf, "%s", "Err_<CMD No Response>");
            break;
        case EV_STS_RSP_CERR:
            sprintf(u8Buf, "%s", "Err_<Response CRC Error>");
            break;
        case EV_STS_RD_TOUT:
            sprintf(u8Buf, "%s", "Err_<Read Timeout>");
            break;
        case EV_STS_DAT0_BUSY:
            sprintf(u8Buf, "%s", "Err_<Card Busy>");
            break;
        case EV_STS_MIE_TOUT:
            sprintf(u8Buf, "%s", "Err_<Wait Event Timeout>");
            break;

        default:
            sprintf(u8Buf, "%s", "Err_<Card Not Recognized>");
            break;
    }
}

static ssize_t sdmmc_get_clk_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char                     clk_buf[255];
    struct sstar_sdmmc_host *p_sdmmc_host = dev_get_drvdata(dev);
    IpOrder                  eIP          = p_sdmmc_host->sdmmc_slot->ipOrder;

    sprintf(clk_buf, " >> sdmmc_%d : %dKHz\n", eIP, gu32_SdmmcClk[eIP] / 1000);

    return sprintf(buf, "%s\n", clk_buf);
}
DEVICE_ATTR(debug_get_sdmmc_clock, S_IRUSR, sdmmc_get_clk_show, NULL);

static ssize_t sdmmc_get_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    U8_T                     u8Buf[32];
    char                     clk_buf[255];
    struct sstar_sdmmc_host *p_sdmmc_host = dev_get_drvdata(dev);
    IpOrder                  eIP          = p_sdmmc_host->sdmmc_slot->ipOrder;

    _Get_Sdmmc_Status(&gu32_SdmmcStatus[eIP], u8Buf);

    if (gu32_SdmmcCurCMD[eIP] == -1)
    {
        sprintf(clk_buf, " >> sdmmc_%d card status : %s\n", eIP, u8Buf);
    }
    else if ((gu32_SdmmcCurCMD[eIP] == 17) || (gu32_SdmmcCurCMD[eIP] == 18))
    {
        sprintf(clk_buf, " >> sdmmc_%d read status :  CMD_%d %s\n", eIP, gu32_SdmmcCurCMD[eIP], u8Buf);
    }
    else
    {
        sprintf(clk_buf, " >> sdmmc_%d write status :  CMD_%d %s\n", eIP, gu32_SdmmcCurCMD[eIP], u8Buf);
    }

    return sprintf(buf, "%s\n", clk_buf);
}
DEVICE_ATTR(debug_get_sdmmc_status, S_IRUSR, sdmmc_get_status_show, NULL);

#if defined(CONFIG_SUPPORT_SDMMC_COMMAND)
static ssize_t emmc_bootbus_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct sstar_sdmmc_host *p_sdmmc_host = dev_get_drvdata(dev);
    U16_T                    u16Err       = 0;
    U16_T                    u16SlotNo    = p_sdmmc_host->sdmmc_slot->slotNo;

    CamOsMutexLock(&sdmmc_mutex[u16SlotNo]);
    Apply_PAGE_BUFFER();
    u16Err = eMMC_GetExtCSD(p_sdmmc_host->sdmmc_slot, gu8EMMC_SectorBuf);
    CamOsMutexUnlock(&sdmmc_mutex[u16SlotNo]);

    if (u16Err)
        pr_err("eMMC_%d read boot_bus error:0x%08x ! \n", u16SlotNo, u16Err);

    pr_err("eMMC_%d boot_bus : %d bit!\n", u16SlotNo,
           ((gu8EMMC_SectorBuf[EXT_CSD_BOOT_BUS_WIDTH] & 0x3) == 0)   ? 1
           : ((gu8EMMC_SectorBuf[EXT_CSD_BOOT_BUS_WIDTH] & 0x3) == 1) ? 4
           : ((gu8EMMC_SectorBuf[EXT_CSD_BOOT_BUS_WIDTH] & 0x3) == 2) ? 8
                                                                      : 0xffff);
    return 0;
}

static ssize_t eMMC_bootbus_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct sstar_sdmmc_host *p_sdmmc_host = dev_get_drvdata(dev);
    U32_T                    bootbus, temp;
    U16_T                    u16SlotNo = p_sdmmc_host->sdmmc_slot->slotNo;
    U16_T                    u16Err    = 0;

    if (buf == NULL)
        return -EINVAL;

    temp = sscanf(buf, "%d", &bootbus);
    if (temp != 1)
        goto EPMT;

    if (bootbus == 0x0)
    {
        pr_err("\r\n>> eMMC_%d Select 1x bus width <<\r\n", u16SlotNo);
    }
    else if (bootbus == 0x1)
    {
        pr_err("\r\n>> eMMC_%d Select 4x bus width <<\r\n", u16SlotNo);
    }
    else if (bootbus == 0x2)
    {
        pr_err("\r\n>> eMMC_%d Select 8x bus width <<\r\n", u16SlotNo);
    }
    else
    {
        pr_err("\r\n>> eMMC_%d Invalid parameter <<\r\n", u16SlotNo);
        goto EPMT;
    }

    CamOsMutexLock(&sdmmc_mutex[u16SlotNo]);
    u16Err = eMMC_ModifyExtCSD(p_sdmmc_host->sdmmc_slot->ipOrder, eMMC_ExtCSD_WByte, EXT_CSD_BOOT_BUS_WIDTH,
                               EXT_CSD_BOOT_BUS_WIDTH_MODE(0) | EXT_CSD_BOOT_BUS_WIDTH_RESET(0)
                                   | EXT_CSD_BOOT_BUS_WIDTH_WIDTH(bootbus));
    CamOsMutexUnlock(&sdmmc_mutex[u16SlotNo]);

    if (u16Err)
    {
        pr_err("eMMC_%d error(%X): set boot bus width fail ! \n", u16SlotNo, u16Err);
        return u16Err;
    }

    return count;

EPMT:
    pr_err("%s usage:\n", __FUNCTION__);
    pr_err("echo 'bootbus' > eMMC_bootbus \n");
    pr_err("[bootbus] [0]-> 1 bit mode, [1]-> 4 bit mode, [2]-> 8 bit mode\n");
    return -EINVAL;
}

DEVICE_ATTR(eMMC_bootbus, S_IRUSR | S_IWUSR, emmc_bootbus_show, eMMC_bootbus_store);

static ssize_t emmc_partconf_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct sstar_sdmmc_host *p_sdmmc_host = dev_get_drvdata(dev);
    U16_T                    u16Err       = 0;
    U16_T                    u16SlotNo    = p_sdmmc_host->sdmmc_slot->slotNo;

    CamOsMutexLock(&sdmmc_mutex[u16SlotNo]);
    Apply_PAGE_BUFFER();
    u16Err = eMMC_GetExtCSD(p_sdmmc_host->sdmmc_slot, gu8EMMC_SectorBuf);
    CamOsMutexUnlock(&sdmmc_mutex[u16SlotNo]);

    if (u16Err)
        prtstring("eMMC_%d read boot part config error:0x%08x ! \n", u16SlotNo, u16Err);

    prtstring("eMMC_%d partconf : %s!\n", u16SlotNo,
              (((gu8EMMC_SectorBuf[EXT_CSD_PART_CONF] >> 3) & 7) == 0)   ? "don't support boot"
              : (((gu8EMMC_SectorBuf[EXT_CSD_PART_CONF] >> 3) & 7) == 1) ? "boot1"
              : (((gu8EMMC_SectorBuf[EXT_CSD_PART_CONF] >> 3) & 7) == 2) ? "boot2"
              : (((gu8EMMC_SectorBuf[EXT_CSD_PART_CONF] >> 3) & 7) == 7) ? "UDA"
                                                                         : "Unknow");
    return 0;
}

static ssize_t eMMC_partconf_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct sstar_sdmmc_host *p_sdmmc_host = dev_get_drvdata(dev);
    U32_T                    partconf, temp;
    U16_T                    u16SlotNo = p_sdmmc_host->sdmmc_slot->slotNo;
    U16_T                    u16Err    = 0;

    if (buf == NULL)
        return -EINVAL;

    temp = sscanf(buf, "%d", &partconf);
    if (temp != 1)
        goto EPMT;

    if (partconf == 0x0)
    {
        pr_err("\r\n>> eMMC_%d Do not support boot <<\r\n", u16SlotNo);
    }
    else if (partconf == 0x1)
    {
        pr_err("\r\n>> eMMC_%d Select boot partition 1 <<\r\n", u16SlotNo);
    }
    else if (partconf == 0x2)
    {
        pr_err("\r\n>> eMMC_%d Select boot partition 2 <<\r\n", u16SlotNo);
    }
    else if (partconf == 0x7)
    {
        pr_err("\r\n>> eMMC_%d Select UDA partition <<\r\n", u16SlotNo);
    }
    else
    {
        pr_err("\r\n>> eMMC_%d Invalid parameter <<\r\n", u16SlotNo);
        goto EPMT;
    }

    CamOsMutexLock(&sdmmc_mutex[u16SlotNo]);
    u16Err = eMMC_ModifyExtCSD(p_sdmmc_host->sdmmc_slot->ipOrder, eMMC_ExtCSD_WByte, EXT_CSD_PART_CONF,
                               EXT_CSD_BOOT_ACK(0) | EXT_CSD_BOOT_PART_NUM(partconf) | EXT_CSD_PARTITION_ACCESS(0));
    CamOsMutexUnlock(&sdmmc_mutex[u16SlotNo]);

    if (u16Err)
    {
        pr_err("eMMC_%d error(%X): set boot part config fail ! \n", u16SlotNo, u16Err);
        return u16Err;
    }

    return count;

EPMT:
    pr_err("%s usage:\n", __FUNCTION__);
    pr_err("echo 'partconf' > eMMC_partconf \n");
    pr_err(
        "[partconf] [0]-> don't support boot, [1]-> boot0 partition, [2]-> boot1 partiton, [7]-> UDA "
        "partiton\n");
    return -EINVAL;
}

DEVICE_ATTR(eMMC_partconf, S_IRUSR | S_IWUSR, emmc_partconf_show, eMMC_partconf_store);

static ssize_t emmc_hwreset_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct sstar_sdmmc_host *p_sdmmc_host = dev_get_drvdata(dev);
    U16_T                    u16Err       = 0;
    U16_T                    u16SlotNo    = p_sdmmc_host->sdmmc_slot->slotNo;

    CamOsMutexLock(&sdmmc_mutex[u16SlotNo]);
    Apply_PAGE_BUFFER();
    u16Err = eMMC_GetExtCSD(p_sdmmc_host->sdmmc_slot, gu8EMMC_SectorBuf);
    CamOsMutexUnlock(&sdmmc_mutex[u16SlotNo]);

    if (u16Err)
        pr_err("eMMC_%d read read RST_n enable error:0x%08x ! \n", u16SlotNo, u16Err);

    prtstring("\033[7;33meMMC_%d RST_n enable : %d!\033[m\r\n", u16SlotNo,
              (gu8EMMC_SectorBuf[EXT_CSD_RST_n_FUNCTION] & 0x3));

    if ((gu8EMMC_SectorBuf[EXT_CSD_RST_n_FUNCTION] & 0x3) == 2)
        prtstring("\033[7;31mWARNING: RST_n signal is permanently disabled!!!\033[m\r\n");

    return 0;
}

static ssize_t eMMC_hwreset_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct sstar_sdmmc_host *p_sdmmc_host = dev_get_drvdata(dev);
    U32_T                    slotNum, temp;
    U16_T                    u16SlotNo = p_sdmmc_host->sdmmc_slot->slotNo;
    U16_T                    u16Err    = 0;

    if (buf == NULL)
        return -EINVAL;

    temp = sscanf(buf, "%d", &slotNum);
    if (temp != 1)
        slotNum = u16SlotNo;

    CamOsMutexLock(&sdmmc_mutex[u16SlotNo]);
    u16Err = eMMC_ModifyExtCSD(p_sdmmc_host->sdmmc_slot->ipOrder, eMMC_ExtCSD_WByte, EXT_CSD_RST_n_FUNCTION,
                               EXT_CSD_PER_ENABLED);
    CamOsMutexUnlock(&sdmmc_mutex[u16SlotNo]);

    if (u16Err)
    {
        pr_err("eMMC Err: %Xh, eMMC, set Ext_CSD[162]: %Xh fail\n", u16Err, EXT_CSD_PER_ENABLED);
        return u16Err;
    }

    prtstring("\033[7;32mset Ext_CSD[162]: %Xh, RST_n signal is permanently enabled!!!\033[m\r\n\n",
              EXT_CSD_PER_ENABLED);

    return count;
}

DEVICE_ATTR(eMMC_hwreset, S_IRUSR | S_IWUSR, emmc_hwreset_show, eMMC_hwreset_store);

/*static ssize_t emmc_get_wr_spend_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    U8 u8_i;
    prtstring();

    prtstring("eMMC Read spend time:");
    for (u8_i = 0; u8_i < 5; u8_i++)
        printk(" %llu(ns) ", gu64_Read_spend[u8_i]);
    prtstring("\n");

    prtstring("eMMC Write spend time:");
    for (u8_i = 0; u8_i < 5; u8_i++)
        printk(" %llu(ns) ", gu64_Write_spend[u8_i]);
    prtstring("\n");

    return 0;
}

DEVICE_ATTR(debug_eMMC_get_rw_time, S_IRUSR, emmc_get_wr_spend_show, NULL);*/

static ssize_t eMMC_write_protect_store(struct device *dev, struct device_attribute *attr, const char *buf,
                                        size_t count)
{
    struct sstar_sdmmc_host *p_sdmmc_host = dev_get_drvdata(dev);
    U16_T                    u16Err       = 0, u16_temp;
    U32_T                    u32AddressWp, u320ption, u32BLKCnt = 1;
    U16_T                    u16SlotNo = p_sdmmc_host->sdmmc_slot->slotNo;

    u16_temp = sscanf(buf, "%d %x %x", &u320ption, &u32AddressWp, &u32BLKCnt);
    if ((u16_temp != 3))
    {
        pr_err(
            "echo [otption] [address] <size> > eMMC_write_protect \n"
            "  [otption]\n"
            "    [0]     -Set the eMMC address of the group to be 'write protect'\n"
            "    [1]     -Clear the eMMC address of the group remove 'write protect'\n"
            "    [2]     -Ask the eMMC address of the group whether it's in 'write protect'?\n"
            "    [3]     -ASK the eMMC address of the group about the 'write protect' type\n"
            "[address] and <size> is in block\n");
        return -EINVAL;
    }

    CamOsMutexLock(&sdmmc_mutex[u16SlotNo]);

    u16Err = SDMMC_Init(p_sdmmc_host->sdmmc_slot);
    if (u16Err)
        return -EINVAL;

    u16Err = eMMC_USER_WriteProtect_Option(p_sdmmc_host->sdmmc_slot, u32AddressWp, u32BLKCnt, u320ption);
    if (u16Err < 0)
        return count;

    if (u320ption == 2)
        prtstring("eMMC_%d, USER part address(0x%08x)'s write protection is %d(0:invalid, 1:valid)\n", u16SlotNo,
                  u32AddressWp, u16Err);
    else if (u320ption == 3)
        prtstring(
            "eMMC_%d, USER part address(0x%08x)'s write protection type is %d(0:not protected, 1:temporary, "
            "2:power-on, 3:permanent)\n",
            u16SlotNo, u32AddressWp, u16Err);

    CamOsMutexUnlock(&sdmmc_mutex[u16SlotNo]);
    return count;
}

DEVICE_ATTR(eMMC_write_protect, S_IWUSR, NULL, eMMC_write_protect_store);

static ssize_t eMMC_erase_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct sstar_sdmmc_host *p_sdmmc_host = dev_get_drvdata(dev);
    U16_T                    u16Err       = 0;
    U32_T                    u32EraseStartBlk, u32EraseBlkCnt, temp;
    U16_T                    u16SlotNo = p_sdmmc_host->sdmmc_slot->slotNo;

    if (buf == NULL)
        return -EINVAL;

    temp = sscanf(buf, "%x %x", &u32EraseStartBlk, &u32EraseBlkCnt);
    if (temp != 2 || u32EraseBlkCnt == 0)
        goto EPMT;

    CamOsMutexLock(&sdmmc_mutex[u16SlotNo]);

    u16Err = SDMMC_Init(p_sdmmc_host->sdmmc_slot);
    if (u16Err)
        return -EINVAL;

    u16Err = eMMC_EraseBlock(p_sdmmc_host->sdmmc_slot, u32EraseStartBlk, u32EraseStartBlk + u32EraseBlkCnt - 1);
    if (u16Err)
        pr_err("erase emmc addr 0x%x size 0x%x fail!!\r\n", u32EraseStartBlk, u32EraseBlkCnt);
    else
        prtstring("erase emmc addr 0x%x size 0x%x blocks success.\r\n", u32EraseStartBlk, u32EraseBlkCnt);

    CamOsMutexUnlock(&sdmmc_mutex[u16SlotNo]);

    return count;

EPMT:
    pr_err("%s usage:\n", __FUNCTION__);
    pr_err("echo 'EraseStartBlk' 'EraseBlkCnt' > eMMC_bootbus \n");
    return -EINVAL;
}

DEVICE_ATTR(eMMC_erase, S_IWUSR, NULL, eMMC_erase_store);
#endif

static ssize_t sdmmc_reset_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    U32_T                    u32_slotNum  = -1, u32_temp;
    struct sstar_sdmmc_host *p_sdmmc_host = dev_get_drvdata(dev);

    u32_temp = sscanf(buf, "%d", &u32_slotNum);

    if (u32_slotNum == p_sdmmc_host->sdmmc_slot->ipOrder)
    {
        /*if (gu32_SdmmcStatus[p_sdmmc_host->sdmmc_slot->ipOrder] == EV_OTHER_ERR)
        {
            printk("Err : No card detected in current slot! \r\n");
            return -ENODEV;
        }*/

        mmc_claim_host(p_sdmmc_host->sdmmc_slot->mmc);
        mmc_hw_reset(p_sdmmc_host->sdmmc_slot->mmc);
        mmc_release_host(p_sdmmc_host->sdmmc_slot->mmc);
    }
    else
    {
        printk("Err : Please enter a current number[ %d ] to trigger the reset! \r\n",
               p_sdmmc_host->sdmmc_slot->slotNo);
    }

    return count;
}
DEVICE_ATTR(sdmmc_reset, S_IWUSR, NULL, sdmmc_reset_store);

static ssize_t sdmmc_driving_control_store(struct device *dev, struct device_attribute *attr, const char *buf,
                                           size_t count)
{
    U32_T                    u32_temp, u32_slotNum, u32_drvlevel;
    char                     signalline[10];
    DrvCtrlType              eClkDrvRes, eCmdDrvRes, eDataDrvRes;
    struct sstar_sdmmc_host *p_sdmmc_host = dev_get_drvdata(dev);
    struct sstar_mmc_priv *  p_mmc_priv   = p_sdmmc_host->sdmmc_slot->p_mmc_priv;

    u32_temp = sscanf(buf, "%d %s %d", &u32_slotNum, signalline, &u32_drvlevel);
    if (u32_temp != 3)
    {
        u32_slotNum = p_mmc_priv->u8_slotNo;
        if (u32_temp == 2)
        {
            u32_temp = sscanf(buf, "%s %d", signalline, &u32_drvlevel);
            if (u32_temp != 2)
                goto EPMT;
        }
        else if (u32_temp == 1)
        {
            strcpy(signalline, "all");
            u32_temp = sscanf(buf, "%d", &u32_drvlevel);
            if (u32_temp != 1)
                goto EPMT;
        }
        else
            goto EPMT;
    }
    else if (u32_slotNum != p_mmc_priv->u8_slotNo)
    {
        printk("Err : Please enter a current number[ %d ] to control driving! \r\n", p_mmc_priv->u8_slotNo);
        goto EPMT;
    }

    CamOsMutexLock(&sdmmc_mutex[u32_slotNum]);

    eClkDrvRes  = p_mmc_priv->mmc_pinDrv.eDrvClk;
    eCmdDrvRes  = p_mmc_priv->mmc_pinDrv.eDrvCmd;
    eDataDrvRes = p_mmc_priv->mmc_pinDrv.eDrvData;

    if ((strcmp(signalline, "clk") == 0) || (strcmp(signalline, "CLK") == 0))
    {
        p_mmc_priv->mmc_pinDrv.eDrvClk = u32_drvlevel;
        if (Hal_Check_ClkCmd_Interrelate(p_mmc_priv->mmc_PMuxInfo.u8_ipOrder, p_mmc_priv->mmc_PMuxInfo.u8_padOrder))
            p_mmc_priv->mmc_pinDrv.eDrvCmd = u32_drvlevel;
    }
    else if ((strcmp(signalline, "cmd") == 0) || (strcmp(signalline, "CMD") == 0))
    {
        p_mmc_priv->mmc_pinDrv.eDrvCmd = u32_drvlevel;
        if (Hal_Check_ClkCmd_Interrelate(p_mmc_priv->mmc_PMuxInfo.u8_ipOrder, p_mmc_priv->mmc_PMuxInfo.u8_padOrder))
            p_mmc_priv->mmc_pinDrv.eDrvClk = u32_drvlevel;
    }
    else if ((strcmp(signalline, "data") == 0) || (strcmp(signalline, "DATA") == 0))
        p_mmc_priv->mmc_pinDrv.eDrvData = u32_drvlevel;
    else if (strcmp(signalline, "all") == 0)
    {
        p_mmc_priv->mmc_pinDrv.eDrvClk  = u32_drvlevel;
        p_mmc_priv->mmc_pinDrv.eDrvCmd  = u32_drvlevel;
        p_mmc_priv->mmc_pinDrv.eDrvData = u32_drvlevel;
    }

    Hal_CARD_DrvCtrlPin(p_mmc_priv->mmc_PMuxInfo, p_mmc_priv->mmc_pinDrv);

    p_mmc_priv->mmc_pinDrv.eDrvClk  = eClkDrvRes;
    p_mmc_priv->mmc_pinDrv.eDrvCmd  = eCmdDrvRes;
    p_mmc_priv->mmc_pinDrv.eDrvData = eDataDrvRes;

    CamOsMutexUnlock(&sdmmc_mutex[u32_slotNum]);

    return count;

EPMT:
    pr_err("%s usage:\n", __FUNCTION__);
    pr_err(
        "echo [slotIndex] <signalLine> [drvLevel] > sdmmc_driving_control    set <signalLine> driving control level is "
        "[drvLevel] for slot sd[slotIndex]\n");
    pr_err(
        "echo [slotIndex]  [drvLevel] > sdmmc_driving_control                set all signal line's driving control "
        "level is [drvLevel] for slot sd[slotIndex]\n");
    pr_err(
        "echo <signalLine> [drvLevel] > sdmmc_driving_control                set <signalLine> driving control level is "
        "[drvLevel] when slotnum = 1.\n");
    pr_err(
        "echo [drvLevel] > sdmmc_driving_control                             set all signal line's driving control "
        "level is [drvLevel] when slotnum = 1.\n");
    pr_err("    operation [slotIndex]   is current slotno. \n");
    pr_err("    operation <singalLine>  is \"clk\" \"cmd\" \"data\" \"all\" \n");
    pr_err("    operation [drvLevel]    is number:0-7. \n");

    return -EINVAL;
}
DEVICE_ATTR(set_sdmmc_driving_control, S_IWUSR, NULL, sdmmc_driving_control_store);

static ssize_t sdmmc_wait_rescan_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct sstar_sdmmc_host *p_sdmmc_host = dev_get_drvdata(dev);
    struct mmc_host *        mmc;

    if (!buf || !p_sdmmc_host || !p_sdmmc_host->sdmmc_slot)
        return -EINVAL;

    mmc = p_sdmmc_host->sdmmc_slot->mmc;
    /* to make sure mmc rescan has completed */
    if (mmc)
    {
        flush_delayed_work(&mmc->detect);
    }

    return count;
}
DEVICE_ATTR(sdmmc_wait_rescan, S_IWUSR, NULL, sdmmc_wait_rescan_store);

static struct attribute *sstar_sdmmc_attr[] = {
    &dev_attr_debug_get_sdmmc_clock.attr,
    &dev_attr_debug_get_sdmmc_status.attr,
#if defined(CONFIG_SUPPORT_SDMMC_COMMAND)
    &dev_attr_eMMC_bootbus.attr,
    &dev_attr_eMMC_partconf.attr,
    &dev_attr_eMMC_hwreset.attr,
    &dev_attr_eMMC_write_protect.attr,
    &dev_attr_eMMC_erase.attr,
#endif
    &dev_attr_sdmmc_reset.attr,
    &dev_attr_set_sdmmc_driving_control.attr,
    &dev_attr_sdmmc_wait_rescan.attr,
    NULL,
};

struct attribute_group sstar_sdmmc_debug_attr_grp = {
    .attrs = sstar_sdmmc_attr,
};
