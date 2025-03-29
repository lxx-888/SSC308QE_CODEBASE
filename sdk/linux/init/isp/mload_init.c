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

#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/kernel.h>
#include <linux/io.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h> // required for various structures related to files liked fops.
#include <linux/cdev.h>
#include <linux/ioctl.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/file.h>
#include <linux/unistd.h>
#include "cam_sysfs.h" //#include "ms_msys.h"
#include "cam_fs_wrapper.h"
#include "cam_device_wrapper.h"
// #include <mdrv_mload.h>
// #include <drv_mload.h>
// #include <mload_common.h>

#ifdef __ENABLE_MLOAD_UT__
#define LINUX_MLOAD_UNIT_TEST (1)
#else
#define LINUX_MLOAD_UNIT_TEST (0)
#endif

#ifndef UNUSED
#define UNUSED(var) (void)((var) = (var))
#endif

#if 0 //DEV_MOVE_TO_MI

static int           g_s32MloadMajor;
static dev_t         g_stMloadDevNo;
static dev_t         g_stMloadDev;
struct cdev *        gp_stMloadKernelCdev;
static struct CamClass *gp_stMloadC1;

struct device *gp_stMloadDev;

#if LINUX_MLOAD_UNIT_TEST
int   g_iSysfsMloadBuffSize = 0;
char *g_ptSysfsMloadBuff;

#define MAX_SYSFS_COMBIE_STATUS_NUM 60
typedef struct
{
    MLOAD_HANDLE handle;
    void *       pMloadShd;
    int          iTypes;
    int          IDs[15];
} SYSFS_COMBIE_STATUS_T;
SYSFS_COMBIE_STATUS_T g_tSysfsCombieStatus[MAX_SYSFS_COMBIE_STATUS_NUM];
int                   g_iSysfsCombieStatusIndex = 0;
int                   g_iSysfsCombieFlag        = true;

#define IOCTL_MLOAD_DOMAIN_SCL 0x01
#define IOCTL_MLOAD_DOMAIN_ISP 0x02
#define IOCTL_MLOAD_DOMAIN_ALL 0x80
int g_s32IoctlMloadDomain = (IOCTL_MLOAD_DOMAIN_SCL | IOCTL_MLOAD_DOMAIN_ISP);

MLOAD_HANDLE g_pMloadUtHnd = NULL;
void *       g_pMloadUtShd = NULL;
#endif
#endif
//==============================================================================
/* IO REMAP */
extern void *gp_pIoBase;

void MloadWarperInit(void)
{
    // gp_pIoBase = (void*) ioremap(0xFD000000, 0x400000);
    // gp_pIoBase = (void *)0xFD000000;
    if (gp_pIoBase == NULL)
    {
        gp_pIoBase = (void *)ioremap(0x1F000000, 0x400000);
    }

    // CamOsPrintf("ISP Mload base remap: 0x%X\n",(int)gp_pIoBase);
}

void MloadWarperDeInit(void)
{
    if (gp_pIoBase)
    {
        iounmap(gp_pIoBase);
    }
}
#if 0 //DEV_MOVE_TO_MI

//==============================================================================
#if LINUX_MLOAD_UNIT_TEST

// ----------------------------------------------------------------------------

static int _MloadCombieCallStore(struct device *dev, struct device_attribute *attr, const char *buf, int n)
{
    if (buf != NULL)
    {
        return n;
    }
    return 0;
}

// Uses "cat /sys/devices/virtual/chardrv/mload/combie" to call this show function
static int _MloadCombieCallShow(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *sStr = buf;
    int   i, j;

    g_iSysfsCombieFlag = false;

    sStr += CamOsSprintf(sStr, "\n[Mload]Combie...Work=%d", g_iSysfsCombieStatusIndex);

    for (i = 0; i < MAX_SYSFS_COMBIE_STATUS_NUM; i++)
    {
        sStr += CamOsSprintf(sStr, "\n[%d],hnd=%p,Shd=%p,Types=0x%x", i, g_tSysfsCombieStatus[i].handle,
                        g_tSysfsCombieStatus[i].pMloadShd, g_tSysfsCombieStatus[i].iTypes);

        for (j = 0; j < 15; j++)
        {
            sStr += CamOsSprintf(sStr, "\n=%d", g_tSysfsCombieStatus[i].IDs[j]);
            if (g_tSysfsCombieStatus[i].IDs[j] == -1)
            {
                break;
            }
        }
    }

    return (sStr - buf);
}

static DEVICE_ATTR(combie, 0644, _MloadCombieCallShow, _MloadCombieCallStore);

// ----------------------------------------------------------------------------

static int _MloadInfoCallStore(struct device *dev, struct device_attribute *attr, const char *buf, int n)
{
    if (buf != NULL)
    {
        return n;
    }
    return 0;
}

// Uses "cat /sys/devices/virtual/chardrv/mload/info" to call this show function
static int _MloadInfoCallShow(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *sStr = buf;
    int   i;

    MloadProxyInfoCallShow(buf);

    return (sStr - buf);
}

static DEVICE_ATTR(info, 0644, _MloadInfoCallShow, _MloadInfoCallStore);

// ----------------------------------------------------------------------------

static int _MloadSramCallStore(struct device *dev, struct device_attribute *attr, const char *buf, int n)
{
    if (buf != NULL)
    {
        return n;
    }
    return 0;
}

// Uses "cat /sys/devices/virtual/chardrv/mload/sram" to call this show function
static int _MloadSramCallShow(struct device *dev, struct device_attribute *attr, char *buf)
{
    int i = 0, j = 0;

    if (g_ptSysfsMloadBuff)
    {
        // CamOsMemcpy(buf, g_ptSysfsMloadBuff, g_iSysfsMloadBuffSize);

        while ((i < PAGE_SIZE) && (j < g_iSysfsMloadBuffSize))
        {
            *(buf + (i++)) = (unsigned char)*(g_ptSysfsMloadBuff + (j++));

            // if( (i%15) == 0 ) *(buf + (i++)) = '\n';
        }
    }

    // MloadFree(g_ptSysfsMloadBuff);

    // return CamOsSprintf(buf, "%s", pnl_str);
    // return g_iSysfsMloadBuffSize;
    return (i);
}

static DEVICE_ATTR(sram, 0644, _MloadSramCallShow, _MloadSramCallStore);

#endif // #if LINUX_MLOAD_UNIT_TEST
// ----------------------------------------------------------------------------

static ssize_t _MLOAD_MODULE_Fread(struct file *fp, char __user *buf, size_t size, loff_t *ppos)
{
    UNUSED(fp);
    UNUSED(buf);
    UNUSED(size);
    UNUSED(ppos);
    return 0;
}


static ssize_t _MLOAD_MODULE_Fwrite(struct file *fp, const char __user *buf, size_t size, loff_t *ppos)
{
    char  cmd[64];
    int   param[4] = {0, 0, 0, 0};
    char *ps8Kbuf  = NULL;

    UNUSED(fp);
    UNUSED(ppos);

    ps8Kbuf = vmalloc(size);
    if (!CamOsCopyFromUpperLayer((void *)ps8Kbuf, (void __user *)buf, size))
    {
        sscanf(ps8Kbuf, "%s %d %d %d %d", cmd, &param[0], &param[1], &param[2], &param[3]);
        CamOsPrintf("mload CMD:%s PARAM:%d,%d,%d,%d\n", cmd, param[0], param[1], param[2], param[3]);
    }
    else
    {
        pr_err("copy user data failed.\n");
        vfree(ps8Kbuf);
        ps8Kbuf = NULL;
        return 0;
    }
    vfree(ps8Kbuf);
    ps8Kbuf = NULL;

#if LINUX_MLOAD_UNIT_TEST

    g_iSysfsMloadBuffSize = 0;
    if (g_ptSysfsMloadBuff == NULL)
        g_ptSysfsMloadBuff = MloadMalloc(PAGE_SIZE);

    if (!strcmp(cmd, "isp_read"))
    {
        int              n                 = 0;
        int              id                = param[2];
        int              nNumData          = param[1];
        int              nDataWidthInBytes = param[0];
        ISP_MLOAD_OUTPUT data;
        u32              nDevId = param[0];
        CamOsPrintf("[Dev %d]isp_read fmt=%dBytes, size=%d, id=%d\n", nDevId, nDataWidthInBytes, nNumData, id);

        for (n = 0; n < nNumData; ++n)
        {
            if (DrvIspMloadRead(nDevId, nDataWidthInBytes, id, n, &data) == MLOAD_SUCCESS)
            {
                // CamOsPrintf("%d:0x%.4X,0x%.4X,0x%.4X\n", n, (u32)data.mload_rdata[0], (u32)data.mload_rdata[1],
                // (u32)data.mload_rdata[2]);
                CamOsPrintf("%d:%.5u,%.5u,%.5u\n", n, (u32)data.mload_rdata[0], (u32)data.mload_rdata[1],
                            (u32)data.mload_rdata[2]);

                if ((g_ptSysfsMloadBuff) && (g_iSysfsMloadBuffSize < (PAGE_SIZE - (sizeof(data)))))
                {
                    CamOsMemcpy((g_ptSysfsMloadBuff + g_iSysfsMloadBuffSize), &data, sizeof(data));
                    g_iSysfsMloadBuffSize += sizeof(data);
                }
            }
            else
            {
                CamOsPrintf("\nDrvIsp_MLoadRead FAIL\n");
            }
        }
        return size;
    }

    if (!strcmp(cmd, "scl_read"))
    {
        int              n                 = 0;
        int              id                = param[2];
        int              nNumData          = param[1];
        int              nDataWidthInBytes = param[0];
        ISP_MLOAD_OUTPUT data;
        u32              nDevId = param[3];
        CamOsPrintf("[Dev %d]scl_read fmt=%dBytes, size=%d, id=%d\n", nDevId, (nDataWidthInBytes + 1) * 2, nNumData,
                    id);

        for (n = 0; n < nNumData; ++n)
        {
            if (DrvSclMloadRead(nDevId, nDataWidthInBytes, id, n, &data) == MLOAD_SUCCESS)
            {
                // CamOsPrintf("%d:0x%.4X,0x%.4X,0x%.4X\n", n, (u32)data.mload_rdata[0], (u32)data.mload_rdata[1],
                // (u32)data.mload_rdata[2]);
                CamOsPrintf("%d:%.5u,%.5u,%.5u\n", n, (u32)data.mload_rdata[0], (u32)data.mload_rdata[1],
                            (u32)data.mload_rdata[2]);

                if ((g_ptSysfsMloadBuff) && (g_iSysfsMloadBuffSize < (PAGE_SIZE - (sizeof(data)))))
                {
                    CamOsMemcpy((g_ptSysfsMloadBuff + g_iSysfsMloadBuffSize), &data, sizeof(data));
                    g_iSysfsMloadBuffSize += sizeof(data);
                }
            }
            else
            {
                CamOsPrintf("\nDrvScl_MLoadRead FAIL\n");
            }
        }
        return size;
    }

    if (!strcmp(cmd, "mload_scl_domain"))
    {
        if (param[0] == 0)
        {
            CamOsPrintf("\nscl_domain off\n");
            g_s32IoctlMloadDomain &= ~IOCTL_MLOAD_DOMAIN_SCL;
        }
        else if (param[0] == 1)
        {
            CamOsPrintf("\nscl_domain on\n");
            g_s32IoctlMloadDomain |= IOCTL_MLOAD_DOMAIN_SCL;
        }

        return size;
    }

    if (!strcmp(cmd, "mload_isp_domain"))
    {
        if (param[0] == 0)
        {
            CamOsPrintf("\nisp_domain off\n");
            g_s32IoctlMloadDomain &= ~IOCTL_MLOAD_DOMAIN_ISP;
        }
        else if (param[0] == 1)
        {
            CamOsPrintf("\nisp_domain on\n");
            g_s32IoctlMloadDomain |= IOCTL_MLOAD_DOMAIN_ISP;
        }

        return size;
    }

    if (!strcmp(cmd, "write"))
    {
        return size;
    }

    if (!strcmp(cmd, "mload_ut_set"))
    {
        u32 nTempMloadIdActType;

        if (param[0] == 0)
        {
            nTempMloadIdActType = E_MLOAD_ID_ACT_FRAME_ACTIVE;
            CamOsPrintf("\n[m] ut frame start\n");
        }
        else
        {
            nTempMloadIdActType = E_MLOAD_ID_ACT_FRAME_BLANK;
            CamOsPrintf("\n[m] ut frame end\n");
        }

        g_s32IoctlMloadDomain |= (IOCTL_MLOAD_DOMAIN_ALL);

        if (g_pMloadUtHnd && g_pMloadUtShd)
        {
            CamOsPrintf("\n[m]ut go exec\n");
            if (IspMloadCombieSet(g_pMloadUtHnd, g_pMloadUtShd, NULL, nTempMloadIdActType) == MLOAD_SUCCESS)
            {
                CamOsPrintf("\n[m]ut SET OK\n");
            }
            else
            {
                CamOsPrintf("\n[m]ut SET NG\n");
            }
        }
        else
        {
            CamOsPrintf("\n[m]ut not exec\n");
        }
        return size;
    }

    if (!strcmp(cmd, "mload_ut_init"))
    {
        if (g_pMloadUtShd)
        {
            if (DrvMloadUtInitShdContent(g_pMloadUtShd, (char)param[0]) == MLOAD_SUCCESS)
            {
                CamOsPrintf("\n~~~ DrvMloadUtInitShdContent Init Good ~~~\n");
            }
            else
            {
                CamOsPrintf("\n!!! DrvMloadUtInitShdContent Init NG !!!\n");
            }
        }
        return size;
    }

    if (!strcmp(cmd, "mload_ut_auto"))
    {
        if (g_pMloadUtHnd && g_pMloadUtShd)
        {
            u32 nDevId = param[0];
            CamOsPrintf("\n[Dev %d] ~~~ mload_auto_ut ready to start ~~~\n", nDevId);

            if (DrvMloadUtInitShdContent(g_pMloadUtShd, 0x01) == MLOAD_SUCCESS) // write '1', means get 257
            {
                CamOsPrintf("\n~~~ mload_auto_ut InitShdContent good\n");

                g_s32IoctlMloadDomain |= (IOCTL_MLOAD_DOMAIN_ALL);

                if (IspMloadCombieSet(g_pMloadUtHnd, g_pMloadUtShd, NULL, E_MLOAD_ID_ACT_FRAME_ACTIVE) == MLOAD_SUCCESS)
                {
                    IspMloadCombieSet(g_pMloadUtHnd, g_pMloadUtShd, NULL, E_MLOAD_ID_ACT_FRAME_BLANK);

                    CamOsPrintf("\n~~~ mload_auto_ut CombieSet good\n");

                    {
#define ENABLE_MLOAD_UT_WRITE_FILE 1

                        char as8MloadUtDataBuf[128]   = "";
                        char as8MloadUtReportBuf[128] = "";

#if ENABLE_MLOAD_UT_WRITE_FILE
                        CamFsFd pMloadUtDataFile   = NULL;
                        CamFsFd pMloadUtReportFile = NULL;
                        ;
#endif

                        {
#if ENABLE_MLOAD_UT_WRITE_FILE
                            if ((CAM_FS_OK != CamFsOpen(&pMloadUtDataFile, "mload_ut_data.txt", O_CREAT | O_RDWR, 0))
                                || (CAM_FS_OK
                                    != CamFsOpen(&pMloadUtReportFile, "unittest.report", O_CREAT | O_RDWR, 0)))
                            {
                                CamOsPrintf("\n!!! 333 mload_auto_ut file open fail~~~~~~\n");
                            }
// else
#endif
                            {
                                ISP_MLOAD_OUTPUT data;
                                int              i, j;

                                for (i = 0; i < 20; i++)
                                {
                                    if (DrvIspMloadRead(nDevId, 8, i, 0, &data) == MLOAD_SUCCESS)
                                    {
                                        // Write Data file
                                        CamOsSprintf(as8MloadUtDataBuf, "------------ id = %d, data = 0x%04x ------------\n",
                                                i, (u16)data.mload_rdata[0]);
                                        CamOsPrintf("%s", as8MloadUtDataBuf);

#if ENABLE_MLOAD_UT_WRITE_FILE
                                        if (pMloadUtDataFile)
                                        {
                                            CamFsWrite(pMloadUtDataFile, as8MloadUtDataBuf, CamOsStrlen(as8MloadUtDataBuf));
                                        }
#endif

                                        // Write Report file
                                        if ((u16)data.mload_rdata[0] == 0x0101)
                                        {
                                            CamOsSprintf(as8MloadUtReportBuf, "Mload_ID_%d PASS\n", i);
                                        }
                                        else
                                        {
                                            CamOsSprintf(as8MloadUtReportBuf, "Mload_ID_%d FAIL\n", i);
                                        }
                                        CamOsPrintf("%s", as8MloadUtReportBuf);

#if ENABLE_MLOAD_UT_WRITE_FILE
                                        if (pMloadUtReportFile)
                                        {
                                            CamFsWrite(pMloadUtReportFile, as8MloadUtReportBuf,
                                                       CamOsStrlen(as8MloadUtReportBuf));
                                        }
#endif

                                        if (DrvMloadGetIdTableAmount(g_pMloadUtHnd, i) != MLOAD_FAIL)
                                        {
                                            CamOsPrintf("Mload ID = %d, amount = %d\n", i,
                                                        DrvMloadGetIdTableAmount(g_pMloadUtHnd, i));
                                            for (j = 0; j < DrvMloadGetIdTableAmount(g_pMloadUtHnd, i); j++)
                                            {
                                                if (DrvIspMloadRead(nDevId, 8, i, j, &data) == MLOAD_SUCCESS)
                                                {
                                                    CamOsPrintf("[%d]: 0x%04x, 0x%04x, 0x%04x,", j,
                                                                (u16)data.mload_rdata[0], (u16)data.mload_rdata[1],
                                                                (u16)data.mload_rdata[2]);
                                                }
                                            }
                                            CamOsPrintf("\n");
                                        }
                                    }
                                }

                                for (i = 20; i < 61; i++)
                                {
                                    if (DrvSclMloadRead(nDevId, 8, i, 0, &data) == MLOAD_SUCCESS)
                                    {
                                        // Write Data file
                                        CamOsSprintf(as8MloadUtDataBuf, "------------ id = %d, data = 0x%04x ------------\n",
                                                i, (u16)data.mload_rdata[0]);
                                        CamOsPrintf("%s", as8MloadUtDataBuf);

#if ENABLE_MLOAD_UT_WRITE_FILE
                                        if (pMloadUtDataFile)
                                        {
                                            CamFsWrite(pMloadUtDataFile, as8MloadUtDataBuf, CamOsStrlen(as8MloadUtDataBuf));
                                        }
#endif

                                        // Write Report file
                                        if ((u16)data.mload_rdata[0] == 0x0101)
                                        {
                                            CamOsSprintf(as8MloadUtReportBuf, "Mload_ID_%d PASS\n", i);
                                        }
                                        else
                                        {
                                            CamOsSprintf(as8MloadUtReportBuf, "Mload_ID_%d FAIL\n", i);
                                        }
                                        CamOsPrintf("%s", as8MloadUtReportBuf);

#if ENABLE_MLOAD_UT_WRITE_FILE
                                        if (pMloadUtReportFile)
                                        {
                                            CamFsWrite(pMloadUtReportFile, as8MloadUtReportBuf,
                                                       CamOsStrlen(as8MloadUtReportBuf));
                                        }
#endif

                                        if (DrvMloadGetIdTableAmount(g_pMloadUtHnd, i) != MLOAD_FAIL)
                                        {
                                            CamOsPrintf("Mload ID = %d, amount = %d\n", i,
                                                        DrvMloadGetIdTableAmount(g_pMloadUtHnd, i));
                                            for (j = 0; j < DrvMloadGetIdTableAmount(g_pMloadUtHnd, i); j++)
                                            {
                                                if (DrvSclMloadRead(nDevId, 8, i, j, &data) == MLOAD_SUCCESS)
                                                {
                                                    CamOsPrintf("[%d]: 0x%04x, 0x%04x, 0x%04x,", j,
                                                                (u16)data.mload_rdata[0], (u16)data.mload_rdata[1],
                                                                (u16)data.mload_rdata[2]);
                                                }
                                            }
                                            CamOsPrintf("\n");
                                        }
                                    }
                                }
                            }

#if ENABLE_MLOAD_UT_WRITE_FILE
                            CamFsClose(pMloadUtReportFile);
                            CamFsClose(pMloadUtDataFile);
#endif
                        }
                    }
                }
                else
                {
                    CamOsPrintf("\n!!! mload_auto_ut CombieSet NG\n");
                }
            }
            else
            {
                CamOsPrintf("\n!!! mload_auto_ut InitShdContent NG\n");
            }
        }
        else
        {
            CamOsPrintf("\n!!! mload_auto_ut Fail due to UT handle NULL\n");
        }
        return size;
    }
#endif // #if LINUX_MLOAD_UNIT_TEST

    pr_err("Invalid command %s!!\n", cmd);
    return size;
}

static long _MLOAD_MODULE_Ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    UNUSED(filp);
    UNUSED(cmd);
    UNUSED(arg);

    return 0;
}

int MLOAD_MODULE_Open(struct inode *inode, struct file *filp)
{
    UNUSED(inode);
    UNUSED(filp);

// IspMloadUT(NULL,1);
#if LINUX_MLOAD_UNIT_TEST
    // CamOsPrintf("\n~~~ mload g_fMloadOps open ~~~\n");
    {
        if (g_pMloadUtHnd == NULL)
        {
            CamOsPrintf("\n~~~ mload g_fMloadOps open to Init Mload UT handle ~~~\n");

            g_pMloadUtHnd = IspMLoadInit(0);

            if (g_pMloadUtHnd)
            {
                if (IspMLoadNextFrame(g_pMloadUtHnd) == MLOAD_FAIL)
                {
                    if (g_pMloadUtHnd)
                    {
                        IspMLoadDeInit(g_pMloadUtHnd);
                    }
                    g_pMloadUtHnd = NULL;
                    return 0;
                }

                if (g_pMloadUtShd == NULL)
                {
                    // g_MloadUT_Shd.tShd.pipe0_fpn[0] = 0;
                    g_pMloadUtShd = MloadMalloc(sizeof(tDrvMloadShd_t));
                    if (g_pMloadUtShd)
                    {
                        CamOsPrintf("\n~~~ g_pMloadUtShd MloadMalloc OK ~~~\n");
                        if (DrvMloadUtInitShdContent(g_pMloadUtShd, 0) == MLOAD_SUCCESS)
                        {
                            CamOsPrintf("\n~~~ DrvMloadUtInitShdContent Init Good ~~~\n");
                        }
                        else
                        {
                            CamOsPrintf("\n!!! DrvMloadUtInitShdContent Init NG !!!\n");
                            MloadFree(g_pMloadUtShd);
                            g_pMloadUtShd = NULL;
                        }
                    }
                    else
                    {
                        CamOsPrintf("\n!!! g_pMloadUtShd MloadMalloc MLOAD_FAIL  !!!\n");
                    }
                }
            }
        }
    }
#endif // #if LINUX_MLOAD_UNIT_TEST
    return 0;
}

int MLOAD_MODULE_Release(struct inode *inode, struct file *filp)
{
    UNUSED(inode);
    UNUSED(filp);

    // CamOsPrintf("\n!!! mload g_fMloadOps release !!!\n");
#if LINUX_MLOAD_UNIT_TEST
    {
        g_s32IoctlMloadDomain &= ~(IOCTL_MLOAD_DOMAIN_ALL);

        if (g_pMloadUtShd)
        {
            MloadFree(g_pMloadUtShd);
        }
        if (g_pMloadUtHnd)
        {
            IspMLoadDeInit(g_pMloadUtHnd);
        }
        g_pMloadUtShd = NULL;
        g_pMloadUtHnd = NULL;
    }
    if (g_ptSysfsMloadBuff)
    {
        MloadFree(g_ptSysfsMloadBuff);
    }
#endif // #if LINUX_MLOAD_UNIT_TEST

    return 0;
}

struct file_operations g_fMloadOps = {.unlocked_ioctl = _MLOAD_MODULE_Ioctl,
                                      .open           = MLOAD_MODULE_Open,
                                      .release        = MLOAD_MODULE_Release,
                                      .read           = _MLOAD_MODULE_Fread,
                                      .write          = _MLOAD_MODULE_Fwrite};

int MLOAD_MODULE_Init(void)
{
    int ret;
    struct CamDevice * pstCamDevice = NULL;
    gp_stMloadKernelCdev        = cdev_alloc();
    gp_stMloadKernelCdev->ops   = &g_fMloadOps;
    gp_stMloadKernelCdev->owner = THIS_MODULE;
    CamOsPrintf(" ~~~~~~~~~~~ Mload Inside init module ~~~~~~~~~~~~~~~\n");
    ret = alloc_chrdev_region(&g_stMloadDevNo, 0, 1, "chr_arr_dev");
    if (ret < 0)
    {
        CamOsPrintf("mload Major number allocation is failed\n");
    }

    g_s32MloadMajor = MAJOR(g_stMloadDevNo);
    g_stMloadDev    = MKDEV(g_s32MloadMajor, 0);
    ret             = cdev_add(gp_stMloadKernelCdev, g_stMloadDev, 1);

    gp_stMloadC1 =
        CamClassCreate(THIS_MODULE, "chardrv"); // msys_get_sysfs_class();//c.l.a.s.s._create(THIS_MODULE, "chardrv");

    if (IS_ERR(gp_stMloadC1))
    {
    }
    else
    {
        pstCamDevice =
            CamDeviceCreate(gp_stMloadC1, NULL, g_stMloadDev, NULL,
                            "mload"); // d.e.v.i.c.e._create(gp_stMloadC1, NULL, g_stMloadDev, NULL, "mload");

#if LINUX_MLOAD_UNIT_TEST
        CamDeviceCreateFile(pstCamDevice, &dev_attr_sram); // d.e.v.i.c.e._create_file(gp_stMloadDev, &dev_attr_sram);
        CamDeviceCreateFile(pstCamDevice, &dev_attr_info); // d.e.v.i.c.e._create_file(gp_stMloadDev, &dev_attr_info);
        CamDeviceCreateFile(pstCamDevice,
                            &dev_attr_combie); // d.e.v.i.c.e._create_file(gp_stMloadDev, &dev_attr_combie);
#endif
    }

    return ret;
}

extern void MLOAD_PROXY_Cleanup(void);

void MLOAD_MODULE_Cleanup(void)
{
    CamOsPrintf("mload Inside cleanup_module\n");

    MLOAD_PROXY_Cleanup();

    cdev_del(gp_stMloadKernelCdev);

#if 1
    if (gp_stMloadC1)
    {
        CamDeviceDestroy(gp_stMloadC1, g_stMloadDev); // d.e.v.i.c.e._destroy(gp_stMloadC1, g_stMloadDev);
        CamClassDestroy(gp_stMloadC1);                // c.l.a.s.s._destroy(gp_stMloadC1);
    }
    unregister_chrdev_region(g_stMloadDev, 1);
#else
    unregister_chrdev_region(g_s32MloadMajor, 1);
#endif
}
#endif
