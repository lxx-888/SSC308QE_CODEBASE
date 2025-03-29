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
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/version.h>
#include <linux/err.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/of.h>
#include <asm/io.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/miscdevice.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>
#include <asm/string.h>

#include <cam_os_wrapper.h>
// #include <libcamera.h>
//#include <ispmid_io.h>

// #include <mhal_common.h>
// #include <mhal_cmdq.h>
// #include <mdrv_mload.h>
//#include <libcamera_iq_ioctl.h>

// #include <isplog.h>
// #include <libcamera_io.h>
#include <ms_msys.h>

// #include <drv_ms_isp_general.h> //test only
// #include <drv_isp.h>
// #include <libcamera_io.h>
// #include <libcamera_private.h>
#include "cam_clkgen.h"
#include "cam_sysfs.h"
#include "cam_fs_wrapper.h"

// #include "isp_cus3a_sync.h"
#include "cam_device_wrapper.h"

//decoupling tmp start
//extern CamClass_t *msys_get_sysfs_class(void);
//extern int CamDeviceCreateFile(struct CamDevice *dev, const CamDeiveAttribute_t *attr);
//extern void CamDeviceRemoveFile(struct CamDevice *dev, const CamDeiveAttribute_t *attr);
//decoupling tmp end


//#define MAJOR_ISPMID_NUM               235
//#define MINOR_ISPMID_NUM               129

#ifndef UNUSED
#define UNUSED(var) (void)((var) = (var))
#endif

#if 0 //DEV_MOVE_TO_MI
static int   g_major;
static dev_t g_devNo;
static dev_t g_dev;
#endif
extern char                   g_IspRoot[128];

void LIBCAMERA_IspMidEarlyInit(void);    // NOLINT
void LIBCAMERA_IspMidFinalRelease(void); // NOLINT

#if 0 //DEV_MOVE_TO_MI
static int _LIBCAMERA_MODULE_IspmidProbe(struct platform_device *pdev);
static int _LIBCAMERA_MODULE_IspmidRemove(struct platform_device *pdev);
static int _LIBCAMERA_MODULE_IspmidSuspend(struct platform_device *pdev, pm_message_t state);
static int _LIBCAMERA_MODULE_IspmidResume(struct platform_device *pdev);
#endif
// =====================================================================================================================

#if 0 //defined(ENABLE_ISP_UT_APP)
#include "mi_isp_impl_datatype.h"

MS_BOOL CommonIspInit(MS_U32 uDevId); // NOLINT
// NOLINTNEXTLINE
int COMMON_ISP_AllocDmaBuffer(MS_U32 uDevId, void *ptMemAlloc, char *pName, MS_U32 nReqSize, ss_phys_addr_t *pPhysAddr,
                              void **pVirtAddr, ss_miu_addr_t *pMiuAddr, MS_BOOL bCache); // NOLINT
MS_BOOL COMMON_ISP_Open(MI_ISP_IMPL_SubChnRes_t *pstCommonIspChnRes);                     // NOLINT

MI_ISP_IMPL_SubChnRes_t g_stIspUtSubChnRes;

int CameraIspUt_SetMd5Check(void *pMd5CheckFunc)
{
    UNUSED(pMd5CheckFunc);

    return 0;
}

CameraMemAlloctor_t *gp_tIspMidUtMemAlloc = NULL;

int CameraIspUt_SetMemAllocator(CameraMemAlloctor_t *pMemAllocator)
{
    if (gp_tIspMidUtMemAlloc == NULL && pMemAllocator)
    {
        gp_tIspMidUtMemAlloc = (CameraMemAlloctor_t *)CamOsMemAlloc(sizeof(CameraMemAlloctor_t));

        if (gp_tIspMidUtMemAlloc)
        {
            gp_tIspMidUtMemAlloc->alloc       = pMemAllocator->alloc;
            gp_tIspMidUtMemAlloc->free        = pMemAllocator->free;
            gp_tIspMidUtMemAlloc->map         = pMemAllocator->map;
            gp_tIspMidUtMemAlloc->unmap       = pMemAllocator->unmap;
            gp_tIspMidUtMemAlloc->flush_cache = pMemAllocator->flush_cache;

            CamOsPrintf("[%s] g_ptIspMidUtMemAlloc=0x%x, pMemAllocator=0x%x, alloc=0x%x, free=0x%x ^^^^^^", __FUNCTION__,
                       gp_tIspMidUtMemAlloc, pMemAllocator, gp_tIspMidUtMemAlloc->alloc, gp_tIspMidUtMemAlloc->free);

            {
                // CamOsMemcpy(&g_tIspMidHandler[0].tMemAlloc, g_ptIspMidUtMemAlloc, sizeof(g_tIspMidHandler[0].tMemAlloc));
            }
        }
    }
    else if (gp_tIspMidUtMemAlloc && pMemAllocator == NULL)
    {
        CamOsPrintf("[%s] g_ptIspMidUtMemAlloc = NULL !!!\n");
        CamOsMemRelease((void *)gp_tIspMidUtMemAlloc);
        gp_tIspMidUtMemAlloc = NULL;
    }

    return 0;
}

#ifndef __DISABLE_CMDQ__
#define DRVCMDQ_CMDQBUFFER_SIZE (512 * 1024)

MHAL_CMDQ_Mmap_Info_t             g_stIspUtCmdqMmapInfo;
struct MHAL_CMDQ_CmdqInterface_s *g_pstIspUtVpeCmdqHnd[COMMON_ISP_DEV_MAX]       = {NULL};
static MS_S32                     g_uIspUtVpeCmdqAvailableID[COMMON_ISP_DEV_MAX] = {-1};

void LIBCAMERA_MODULE_AllocateBuffer(MHAL_CMDQ_Mmap_Info_t *pCmdqMmapInfo)
{
    CamOsRet_e     eRet;
    void *         pVirtPtr  = NULL;
    ss_miu_addr_t  pMiuAddr  = 0;
    ss_phys_addr_t pPhysAddr = 0;

    eRet = COMMON_ISP_AllocDmaBuffer(0, gp_tIspMidUtMemAlloc, "CMDMEM", DRVCMDQ_CMDQBUFFER_SIZE, &pPhysAddr, &pVirtPtr,
                                     &pMiuAddr, FALSE); // change to non-cacheable due to cmdq driver's change
    if (eRet != CAM_OS_OK)
    {
        CamOsPrintf("[CMDQ]can't allocate cmdq memory\n");
        return;
    }
    pCmdqMmapInfo->nCmdqMmapMiuAddr = pMiuAddr;
    pCmdqMmapInfo->pCmdqMmapVirAddr = pVirtPtr;
    pCmdqMmapInfo->u32CmdqMmapSize  = DRVCMDQ_CMDQBUFFER_SIZE;

    CamOsMemset(pVirtPtr, 0x0, DRVCMDQ_CMDQBUFFER_SIZE);
}

void LIBCAMERA_MODULE_ReleaseBuffer(MHAL_CMDQ_Mmap_Info_t *pCmdqMmapInfo)
{
    ss_phys_addr_t pPtr;
    pPtr = CamOsMemMiuToPhys(((ss_miu_addr_t)pCmdqMmapInfo->nCmdqMmapMiuAddr));
    CamOsContiguousMemRelease(pPtr);
}

int LIBCAMERA_MODULE_GetVpeCmdqService(u32 nDevId)
{
    int sRet = SUCCESS;

    if (nDevId >= COMMON_ISP_DEV_MAX)
    {
        return FAIL;
    }

    if (g_pstIspUtVpeCmdqHnd[nDevId] == NULL)
    {
        MHAL_CMDQ_BufDescript_t tCmdqBufDesp;

        CamOsMemset(&tCmdqBufDesp, 0x0, sizeof(MHAL_CMDQ_BufDescript_t));

        tCmdqBufDesp.u32CmdqBufSizeAlign = 32;
        tCmdqBufDesp.u32CmdqBufSize      = 32768 * 4; //(128 * (19+6) * 8);

        g_uIspUtVpeCmdqAvailableID[nDevId] = MHAL_CMDQ_GetFreeId();
        g_pstIspUtVpeCmdqHnd[nDevId] =
            MHAL_CMDQ_GetSysCmdqService(g_uIspUtVpeCmdqAvailableID[nDevId], &tCmdqBufDesp, false);
        if (g_pstIspUtVpeCmdqHnd[nDevId] == NULL)
        {
            CamOsPrintf("[%s] Error, Can't get VPE CmdQ Service!! CmdQ ID = %d\n", __FUNCTION__,
                      g_uIspUtVpeCmdqAvailableID[nDevId]);
            sRet = FAIL;
        }
        else
        {
            CamOsPrintf("nDevId = %d, VPE CmdQ Free ID = %d, VpeCmdQHnd = %p\n", nDevId,
                       g_uIspUtVpeCmdqAvailableID[nDevId], g_pstIspUtVpeCmdqHnd[nDevId]);
        }
    }

    return sRet;
}

void LIBCAMERA_MODULE_ReleaseVpeCmdqService(u32 nDevId)
{
    if (nDevId >= COMMON_ISP_DEV_MAX)
    {
        return;
    }

    if (g_pstIspUtVpeCmdqHnd[nDevId])
    {
        MHAL_CMDQ_ReleaseSysCmdqService(g_uIspUtVpeCmdqAvailableID[nDevId]);
        MHAL_CMDQ_ReleaseId(g_uIspUtVpeCmdqAvailableID[nDevId]);

        g_pstIspUtVpeCmdqHnd[nDevId]       = NULL;
        g_uIspUtVpeCmdqAvailableID[nDevId] = -1;
    }
}
#endif // #ifndef __DISABLE_CMDQ__

#endif // #ifdef ENABLE_ISP_UT_APP

// =====================================================================================================================
// =====================================================================================================================
// =====================================================================================================================


#if 0 //DEV_MOVE_TO_MI
extern int LIBCAMERA_PROXY_IspmidFwrite(char *buf, int size, void (*Accessor)(unsigned int, void*));
extern int LIBCAMERA_PROXY_IspmidInfoShow(unsigned int u32DevId, char *buf);
#endif
extern int LIBCAMERA_PROXY_DriverInit(void);
extern int LIBCAMERA_PROXY_DriverExit(void);


unsigned long LIBCAMERA_MODULE_IspGetRiuAddrBase(void)
{
    // return 0xfd000000;
#if 1
    return (unsigned long)ioremap(0x1F000000, 0x400000);
#else
    return (unsigned long)IO_ADDRESS;
#endif
}

#define MIU_SEL_BOUNDRY (0x80000000) //>= 0x80000000 MIU1 buffer; < 0x80000000 MIU0 buffer

u32 LIBCAMERA_MODULE_GetMiuSelBoundry(void)
{
    return MIU_SEL_BOUNDRY;
}

const char *LIBCAMERA_MODULE_GetIspRootPath(void)
{
    return g_IspRoot;
}


#if 0 //DEV_MOVE_TO_MI


/* Match table for of_platform binding */
static const struct of_device_id SSTAR_ISPMID_OF_MATCH[] = {
    {.compatible = "sstar,ispmid", 0},
    {},
};
MODULE_DEVICE_TABLE(of, SSTAR_ISPMID_OF_MATCH);

static struct platform_device *gp_ispmidDevice;
static struct platform_driver  g_sstarIspmidDriver = {
    .probe   = _LIBCAMERA_MODULE_IspmidProbe,
    .remove  = _LIBCAMERA_MODULE_IspmidRemove,
    .suspend = _LIBCAMERA_MODULE_IspmidSuspend,
    .resume  = _LIBCAMERA_MODULE_IspmidResume,
    .driver =
        {
            .name           = "sstar-ispmid",
            .owner          = THIS_MODULE,
            .of_match_table = SSTAR_ISPMID_OF_MATCH,
        },
};

typedef struct
{
    struct miscdevice ispmid_dev;
    struct  CamDevice *   sysfs_dev; // for node /sys/class/sstar/
} ispmid_data;                   // NOLINT


// struct miscdevice ispmid_dev;
static int     _LIBCAMERA_MODULE_IspmidOpen(struct inode *inode, struct file *fp);
static int     _LIBCAMERA_MODULE_IspmidRelease(struct inode *inode, struct file *fp);
static ssize_t _LIBCAMERA_MODULE_IspmidFread(struct file *fp, char __user *buf, size_t size, loff_t *ppos);
static ssize_t _LIBCAMERA_MODULE_IspmidFwrite(struct file *fp, const char __user *buf, size_t size, loff_t *ppos);
static long    _LIBCAMERA_MODULE_IspmidIoctl(struct file *fp, unsigned int cmd, unsigned long arg);
struct file_operations g_ispmidFops = {
    .owner          = THIS_MODULE,
    .open           = _LIBCAMERA_MODULE_IspmidOpen,
    .release        = _LIBCAMERA_MODULE_IspmidRelease,
    .read           = _LIBCAMERA_MODULE_IspmidFread,
    .write          = _LIBCAMERA_MODULE_IspmidFwrite,
    .unlocked_ioctl = _LIBCAMERA_MODULE_IspmidIoctl,
};

static int _LIBCAMERA_MODULE_IspmidOpen(struct inode *inode, struct file *fp)
{
    UNUSED(inode);
    UNUSED(fp);

    return 0;
}

static int _LIBCAMERA_MODULE_IspmidRelease(struct inode *inode, struct file *fp)
{
    UNUSED(inode);
    UNUSED(fp);

    return 0;
}

static ssize_t _LIBCAMERA_MODULE_IspmidFread(struct file *fp, char __user *buf, size_t size, loff_t *ppos)
{
    UNUSED(fp);
    UNUSED(buf);
    UNUSED(size);
    UNUSED(ppos);

    return 0;
}

#define LIBCAMERA_ACC_ID_ISP_ROOT (0x0001)
#define LIBCAMERA_ACC_ACTION_SET  (0x10000)
#define LIBCAMERA_ACC_ACTION_GET  (0x00000)
#define LIBCAMERA_ACC_ID(id) (id & 0xffff)

static void _LIBCAMERA_MODULE_GlobalAccessor(unsigned int u32AccessId, void *pData)
{
    switch (LIBCAMERA_ACC_ID(u32AccessId)) {
        case LIBCAMERA_ACC_ID_ISP_ROOT: {
            if (u32AccessId & LIBCAMERA_ACC_ACTION_SET) {
                CamOsStrncpy(g_IspRoot, pData, CamOsStrlen(pData) + 1);
            } else {
                CamOsStrncpy(pData, g_IspRoot, CamOsStrlen(g_IspRoot) + 1);
            }
            break;
        }
        default:
            break;
    }
    return;
}

#undef LIBCAMERA_ACC_ID_ISP_ROOT
#undef LIBCAMERA_ACC_ACTION_SET
#undef LIBCAMERA_ACC_ACTION_GET
#undef LIBCAMERA_ACC_ID

static ssize_t _LIBCAMERA_MODULE_IspmidFwrite(struct file *fp, const char __user *buf, size_t size, loff_t *ppos)
{
    char  cmd[32];
    char *kBuf     = NULL;
    int s32Ret = 0;

    kBuf = vmalloc(size);
    if (!CamOsCopyFromUpperLayer((void *)kBuf, (void __user *)buf, size))
    {
        sscanf(kBuf, "%s", cmd);
        CamOsPrintf("[%s] CMD : %s\n", __FUNCTION__, cmd);
    }
    else
    {
        CamOsPrintf("[%s] copy user data failed!!!\n", __FUNCTION__);
        goto ISPMID_WRITE_EXIT;
    }

    s32Ret = LIBCAMERA_PROXY_IspmidFwrite(kBuf, size, _LIBCAMERA_MODULE_GlobalAccessor);
    if (s32Ret < 0) {
        CamOsPrintf("Invalid command %s !!!\n", cmd);
    }

ISPMID_WRITE_EXIT:

    if (kBuf)
    {
        vfree(kBuf);
        kBuf = NULL;
    }

    UNUSED(fp);
    UNUSED(ppos);

    return size;
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================

static long _LIBCAMERA_MODULE_IspmidIoctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    UNUSED(fp);
    UNUSED(cmd);
    UNUSED(arg);

    return ret;
}

static ssize_t _LIBCAMERA_MODULE_IspmidInfoShow(struct device *dev, struct device_attribute *attr, char *buf)
{
    UNUSED(dev);
    UNUSED(attr);

    return LIBCAMERA_PROXY_IspmidInfoShow(0, buf);
}
DEVICE_ATTR(ispmid_info, 0444, _LIBCAMERA_MODULE_IspmidInfoShow, NULL);



static ssize_t _LIBCAMERA_MODULE_SclenShow(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    // char *end = buf + PAGE_SIZE;

    UNUSED(dev);
    UNUSED(attr);

    return (str - buf);
}

static ssize_t _LIBCAMERA_MODULE_SclenStore(struct device *dev, struct device_attribute *attr, const char *buf,
                                            size_t count)
{
    char *   str = (char *)buf;
    char *   end = (char *)buf + PAGE_SIZE;
    uint32_t assignedValue;

    assignedValue = simple_strtoul((const char *)str, &end, 10);

    UNUSED(dev);
    UNUSED(attr);

    return count;
}

DEVICE_ATTR(ispmid_sclen, S_IRUSR | S_IWUSR, _LIBCAMERA_MODULE_SclenShow, _LIBCAMERA_MODULE_SclenStore);

static ssize_t _LIBCAMERA_MODULE_IsprootShow(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    str += CamOsSnprintf(str, PAGE_SIZE, "%s", g_IspRoot);

    UNUSED(dev);
    UNUSED(attr);

    return (str - buf);
}

static ssize_t _LIBCAMERA_MODULE_IsprootStore(struct device *dev, struct device_attribute *attr, const char *buf,
                                              size_t count)
{
    int n;
    /*copy string*/
    CamOsStrncpy(g_IspRoot, buf, CamOsStrlen(buf) + 1);
    /*Remove new line from the string*/
    for (n = 0; n < sizeof(g_IspRoot); ++n)
    {
        if (g_IspRoot[n] == 0) // if null terminate
        {
            break;
        }

        if (g_IspRoot[n] == '\n' || g_IspRoot[n] == '\r') // if new line
        {
            g_IspRoot[n] = 0;
            break;
        }
    }
    CamOsPrintf("set isp root: %s\n", g_IspRoot);

    UNUSED(dev);
    UNUSED(attr);

    return count;
}

DEVICE_ATTR(isproot, S_IRUSR | S_IWUSR, _LIBCAMERA_MODULE_IsprootShow, _LIBCAMERA_MODULE_IsprootStore);

static int _LIBCAMERA_MODULE_CreateAttrFile(struct  CamDevice *dev)
{
    int error = -EINVAL;
    if (dev != NULL)
    {
        error = CamDeviceCreateFile(dev, &dev_attr_ispmid_info);
        // CamOsPrintf("Create device file. %s,%d\n", "ispmid_info", error);

        error = CamDeviceCreateFile(dev, &dev_attr_ispmid_sclen);
        // CamOsPrintf("Create device file. %s,%d\n", "ispmid_sclen", error);

        error = CamDeviceCreateFile(dev, &dev_attr_isproot);
    }
    return error;
}
#endif

void LIBCAMERA_POLL_IspAddFeNode(void);           // NOLINT
void LIBCAMERA_POLL_IspRemoveFeNode(void);        // NOLINT

#if 0 //DEV_MOVE_TO_MI
static int _LIBCAMERA_MODULE_IspmidProbe(struct platform_device *pdev)
{
    int          err = 0;
    int          ret = 0;
    ispmid_data *ispmid;
    struct CamDevice * pstCamDevice = NULL;
    struct device* orgDev = NULL;
    ispmid                  = kzalloc(sizeof(ispmid_data), GFP_KERNEL);
    pdev->dev.platform_data = ispmid;

    gp_ispmidDevice = pdev;

    // CamOsPrintf("=== [ispmid probe] ===\n");
    ispmid->ispmid_dev.minor  = MISC_DYNAMIC_MINOR;
    ispmid->ispmid_dev.name   = "ispmid";
    ispmid->ispmid_dev.fops   = &g_ispmidFops;
    ispmid->ispmid_dev.parent = &pdev->dev;
    misc_register(&ispmid->ispmid_dev);

    /*Create frame start node for polling*/
    LIBCAMERA_POLL_IspAddFeNode();

    ret = alloc_chrdev_region(&g_devNo, 0, 1, "ispmid0");
    if (ret < 0)
    {
        CamOsPrintf("ispmid major number allocation is failed\n");
    }

    g_major                          = MAJOR(g_devNo);
    g_dev                            = MKDEV(g_major, 0);
    // ispmid->sysfs_dev                = CamDeviceCreate(msys_get_sysfs_camclass(), NULL, g_dev, NULL, "ispmid0");
    pstCamDevice                     = CamDeviceCreate(msys_get_sysfs_camclass(), NULL, g_dev, NULL, "ispmid0");
    ispmid->sysfs_dev                = pstCamDevice;
    orgDev = CamDeviceGetInternalDevice(pstCamDevice);
    err                              = CamSysfsCreateLink(&pdev->dev.parent->kobj, &orgDev->kobj,
                                                          "ispmid0"); // create symlink for older firmware version
    // ispmid->sysfs_dev->platform_data = pdev->dev.platform_data = (void *)ispmid;
    orgDev->platform_data = pdev->dev.platform_data = (void *)ispmid;

    _LIBCAMERA_MODULE_CreateAttrFile(ispmid->sysfs_dev);

    return 0;
}

static int _LIBCAMERA_MODULE_IspmidRemove(struct platform_device *pdev)
{
    ispmid_data *ispmid = (ispmid_data *)dev_get_platdata(&pdev->dev);
    struct device* orgDev = NULL;
    orgDev = ispmid->sysfs_dev->device;

    misc_deregister(&ispmid->ispmid_dev);
    CamSysfsDeleteLink(&pdev->dev.parent->kobj, &orgDev->kobj, "ispmid0");
    CamDeviceDestroy(msys_get_sysfs_camclass(), g_dev);
    unregister_chrdev_region(g_dev, 1);
    kfree(ispmid);
    LIBCAMERA_POLL_IspRemoveFeNode();
    return 0;
}

static int _LIBCAMERA_MODULE_IspmidSuspend(struct platform_device *pdev, pm_message_t state)
{
    if (!pdev)
    {
        return -EINVAL;
    }

    UNUSED(state);

    return 0;
}

static int _LIBCAMERA_MODULE_IspmidResume(struct platform_device *pdev)
{
    if (!pdev)
    {
        return -EINVAL;
    }
    return 0;
}
#endif
#if 0
void libcamera_init(LibcameraIF_t* pInterface)
{
    pInterface->CameraOpen      = CameraOpen;
    pInterface->CameraClose     = CameraClose;
    pInterface->LIBCAMERA_CameraIspRawFetch  = LIBCAMERA_CameraIspRawFetch;
    pInterface->LIBCAMERA_CameraIspRawFetchTrigger  = LIBCAMERA_CameraIspRawFetchTrigger;
    pInterface->LIBCAMERA_CameraIspRawStore  = LIBCAMERA_CameraIspRawStore;
    pInterface->CameraRawStoreTrigger = 0;//CameraRawStoreTrigger;
    pInterface->CameraChangeCmdqIF = CameraChangeIspRegCmdqIF;
    pInterface->CameraContextSwitch = CameraContextSwitch;
    pInterface->MS_CAM_IspApiSet = MS_CAM_IspApiSet;
    pInterface->MS_CAM_IspApiGet = MS_CAM_IspApiGet;
    pInterface->CameraInit      = CameraInit;
    pInterface->CameraDeInit      = CameraDeInit;
}
#endif

int LIBCAMERA_MODULE_CameraReadIqData(int nCh, void *pBuf, unsigned int nBufSize)
{
    int     nRet = 0;
    char    szFileName[64];
    CamFsFd fIspBinFile = NULL;

    CamOsPrintf("[%s] isp root is %s, nCh = %d\n", __FUNCTION__, g_IspRoot, nCh);
    CamOsSprintf(szFileName, "%s/iqfile%d.bin", g_IspRoot, nCh);
    CamFsOpen(&fIspBinFile, szFileName, CAM_FS_O_RDONLY, 0);
    if (fIspBinFile)
    {
        nRet = CamFsRead(fIspBinFile, pBuf, nBufSize);
        //CamOsPrintf("*** open iq file %s, size=%d, read:%d ****\n", szFileName, nBufSize, nRet);
        CamFsClose(fIspBinFile);
    }
    else
    {
        CamOsPrintf("*** failed to open iq file %s ****\n", szFileName);
    }
    return nRet;
}

int LIBCAMERA_MODULE_DriverInit(void)
{
    int ret = 0;

    // LibcameraIfInit(libcamera_init);
    LIBCAMERA_PROXY_DriverInit();
    // Default ISP root path
    CamOsStrncpy(g_IspRoot, "/config/iqfile", CamOsStrlen("/config/iqfile") + 1);
#if 0 //DEV_MOVE_TO_MI
    ret = CamPlatformDriverRegister(&g_sstarIspmidDriver);
    if (ret)
    {
        CamOsPrintf("[%s] Driver register fail !!!\n", __FUNCTION__);
        return ret;
    }
#else
    /*Create frame start node for polling*/
    LIBCAMERA_POLL_IspAddFeNode();
#endif

    return 0;
}

void LIBCAMERA_MODULE_DriverExit(void)
{
    LIBCAMERA_PROXY_DriverExit();
#if 0 //DEV_MOVE_TO_MI
    CamPlatformDriverUnregister(&g_sstarIspmidDriver);
#else
    LIBCAMERA_POLL_IspRemoveFeNode();
#endif
    CamOsPrintf("[%s]\n", __FUNCTION__);
}

extern void  _LIBCAMERA_POLL_IspmidFeLockInit(u32 nDev);
extern void _LIBCAMERA_POLL_IspmidFeLockRelease(u32 nDev);
extern void _LIBCAMERA_POLL_IspmidFeLock(u32 nDev);
extern void _LIBCAMERA_POLL_IspmidFeUnlock(u32 nDev);

typedef struct
{
    u32 u32ChStatus;
    u32 u32ChAeEn;
    u32 u32ChAwbEn;
    u32 u32ChAfEn;
    u32 u32ChReopen;
    u32 u32ChResume;
    u64 u64DbgCmd;
    u32 u32Ch3aMask[3]; //0:AE , 1:AWB, 2:AF
} Cus3A_FrameStart_Priv_Data_t;

Cus3A_FrameStart_Priv_Data_t g_stCus3aChStatus[2] = {{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}}; // for 2 ISP device

static struct list_head g_stIspFeWqList[2] = {LIST_HEAD_INIT(g_stIspFeWqList[0]), LIST_HEAD_INIT(g_stIspFeWqList[1])};
typedef struct
{
    struct list_head             list;
    wait_queue_head_t            waitq;
    Cus3A_FrameStart_Priv_Data_t pdata;
    int                          ready;
    u32                          nDev;
} IspIrqLink_t;

static int _LIBCAMERA_POLL_IspFeOpen(struct inode *inode, struct file *fp)
{
    IspIrqLink_t *data = kmalloc(sizeof(IspIrqLink_t), GFP_KERNEL);
    if (!data)
    {
        return -ENOENT;
    }

    CamOsMemset((void *)data, 0, sizeof(IspIrqLink_t));
    data->nDev = 0;
    init_waitqueue_head(&data->waitq);

    _LIBCAMERA_POLL_IspmidFeLock(data->nDev);
    list_add(&data->list, &g_stIspFeWqList[data->nDev]);
    _LIBCAMERA_POLL_IspmidFeUnlock(data->nDev);

    fp->private_data = (void *)data;
    // CamOsPrintf("=====[%s]\n", __FUNCTION__);

    UNUSED(inode);

    return 0;
}

static int _LIBCAMERA_POLL_IspFeOpen1(struct inode *inode, struct file *fp)
{
    IspIrqLink_t *data = kmalloc(sizeof(IspIrqLink_t), GFP_KERNEL);
    if (!data)
    {
        return -ENOENT;
    }

    CamOsMemset((void *)data, 0, sizeof(IspIrqLink_t));
    data->nDev = 1;
    init_waitqueue_head(&data->waitq);

    _LIBCAMERA_POLL_IspmidFeLock(data->nDev);
    list_add(&data->list, &g_stIspFeWqList[data->nDev]);
    _LIBCAMERA_POLL_IspmidFeUnlock(data->nDev);

    fp->private_data = (void *)data;
    // CamOsPrintf("=====[%s]\n", __FUNCTION__);

    UNUSED(inode);

    return 0;
}

static int _LIBCAMERA_POLL_IspFeRelease(struct inode *inode, struct file *fp)
{
    IspIrqLink_t *data = (IspIrqLink_t *)fp->private_data;

    if (fp == NULL || data == NULL)
    {
        return 0;
    }

    _LIBCAMERA_POLL_IspmidFeLock(data->nDev);
    list_del(&data->list);
    _LIBCAMERA_POLL_IspmidFeUnlock(data->nDev);
    kfree(data);
    // CamOsPrintf("=====[%s]\n", __FUNCTION__);

    UNUSED(inode);

    return 0;
}

static ssize_t _LIBCAMERA_POLL_IspFeFread(struct file *fp, char __user *buf, size_t size, loff_t *ppos)
{
    IspIrqLink_t *data = (IspIrqLink_t *)fp->private_data;

    if (fp == NULL || buf == NULL || data == NULL)
    {
        return 0;
    }

    // CamOsPrintf("=====[%s]\n", __FUNCTION__);
    _LIBCAMERA_POLL_IspmidFeLock(data->nDev);
    data->pdata.u32ChAeEn  = g_stCus3aChStatus[data->nDev].u32ChAeEn;
    data->pdata.u32ChAwbEn = g_stCus3aChStatus[data->nDev].u32ChAwbEn;
    data->pdata.u32ChAfEn  = g_stCus3aChStatus[data->nDev].u32ChAfEn;
    if (CamOsCopyToUpperLayer(buf, (void *)&data->pdata, size > sizeof(data->pdata) ? sizeof(data->pdata) : size))
    {
        size = 0;
    }
    data->pdata.u32ChStatus = 0;
    data->pdata.u32ChReopen = 0;
    data->pdata.u32ChResume = 0;
    data->pdata.u64DbgCmd   = 0;
    CamOsMemset(data->pdata.u32Ch3aMask, 0, sizeof(data->pdata.u32Ch3aMask));
    _LIBCAMERA_POLL_IspmidFeUnlock(data->nDev);

    UNUSED(ppos);

    return size;
}

static unsigned int _LIBCAMERA_POLL_IspFePoll(struct file *fp, poll_table *wait)
{
    unsigned int  mask = 0;
    IspIrqLink_t *data = (IspIrqLink_t *)fp->private_data;

    if (fp == NULL || wait == NULL || data == NULL)
    {
        return 0;
    }

    poll_wait(fp, &data->waitq, wait);
    if (data->ready)
    {
        data->ready = 0;
        mask |= POLLIN | POLLRDNORM;
    }
    // CamOsPrintf("=====[%s]\n", __FUNCTION__);
    return mask;
}

struct file_operations g_stIspFeFops = {
    .owner          = THIS_MODULE,
    .open           = _LIBCAMERA_POLL_IspFeOpen,
    .release        = _LIBCAMERA_POLL_IspFeRelease,
    .read           = _LIBCAMERA_POLL_IspFeFread,
    .write          = 0,
    .unlocked_ioctl = 0,
    .poll           = _LIBCAMERA_POLL_IspFePoll,
};

struct file_operations g_stIspFeFops1 = {
    .owner          = THIS_MODULE,
    .open           = _LIBCAMERA_POLL_IspFeOpen1,
    .release        = _LIBCAMERA_POLL_IspFeRelease,
    .read           = _LIBCAMERA_POLL_IspFeFread,
    .write          = 0,
    .unlocked_ioctl = 0,
    .poll           = _LIBCAMERA_POLL_IspFePoll,
};

struct miscdevice g_stIspFeDev[2];
void LIBCAMERA_POLL_IspAddFeNode(void)
{
    _LIBCAMERA_POLL_IspmidFeLockInit(0);
    g_stIspFeDev[0].minor  = MISC_DYNAMIC_MINOR;
    g_stIspFeDev[0].name   = "isp_fe";
    g_stIspFeDev[0].fops   = &g_stIspFeFops;
    g_stIspFeDev[0].parent = 0;
    misc_register(&g_stIspFeDev[0]);

    _LIBCAMERA_POLL_IspmidFeLockInit(1);
    g_stIspFeDev[1].minor  = MISC_DYNAMIC_MINOR;
    g_stIspFeDev[1].name   = "isp1_fe";
    g_stIspFeDev[1].fops   = &g_stIspFeFops1;
    g_stIspFeDev[1].parent = 0;
    misc_register(&g_stIspFeDev[1]);
}

void LIBCAMERA_POLL_IspRemoveFeNode(void)
{
    misc_deregister(&g_stIspFeDev[0]);
    _LIBCAMERA_POLL_IspmidFeLockRelease(0);
    misc_deregister(&g_stIspFeDev[1]);
    _LIBCAMERA_POLL_IspmidFeLockRelease(1);
}


#if !(defined(E_ALGO_TYPE_AE) || defined(E_ALGO_TYPE_AWB) || defined(E_ALGO_TYPE_AF))
typedef enum
{
    E_ALGO_TYPE_AE = 0x0,
    E_ALGO_TYPE_AWB,
    E_ALGO_TYPE_AF,
    E_ALGO_TYPE_MAX
}CUS3A_ALGO_TYPE_e;
#endif

void LIBCAMERA_POLL_IspmidConfigFrameEvent(u32 nDev, u32 nCh, u32 u3aMask)
{
    struct list_head *pos;
    IspIrqLink_t *elm;

    _LIBCAMERA_POLL_IspmidFeLock(nDev);
    list_for_each(pos, &g_stIspFeWqList[nDev])
    {
        u32 n=0;
        elm = list_entry(pos, IspIrqLink_t, list);
        elm->pdata.u32ChStatus |= 0x1 << nCh;
        for(n=0; n<=(E_ALGO_TYPE_AF-E_ALGO_TYPE_AE); ++n)
            elm->pdata.u32Ch3aMask[n] |= ((u3aMask&(0x1<<n))>>n) << nCh;
    }
    _LIBCAMERA_POLL_IspmidFeUnlock(nDev);
}

/*To notify user space frame end happen*/
void LIBCAMERA_POLL_IspmidFrameEvent(u32 nDev, u32 nCh, u32 u3aMask)
{
    struct list_head *pos;
    IspIrqLink_t *    elm;

    _LIBCAMERA_POLL_IspmidFeLock(nDev);
    list_for_each(pos, &g_stIspFeWqList[nDev])
    {
        u32 n=0;
        elm = list_entry(pos, IspIrqLink_t, list);
        elm->pdata.u32ChStatus |= 0x1 << nCh;
        for(n=0; n<E_ALGO_TYPE_MAX; ++n)
            elm->pdata.u32Ch3aMask[n] |= ((u3aMask&(0x1<<n))>>n) << nCh;
        elm->ready = 1;
        wake_up_interruptible_all(&elm->waitq);
    }
    _LIBCAMERA_POLL_IspmidFeUnlock(nDev);

    UNUSED(u3aMask);
}

void LIBCAMERA_POLL_IspmidReopenEvent(u32 nDev, u32 nCh)
{
    struct list_head *pos;
    IspIrqLink_t *    elm;

    _LIBCAMERA_POLL_IspmidFeLock(nDev);
    list_for_each(pos, &g_stIspFeWqList[nDev])
    {
        elm = list_entry(pos, IspIrqLink_t, list);
        elm->pdata.u32ChReopen |= 0x1 << nCh;
        elm->ready = 1;
        wake_up_interruptible_all(&elm->waitq);
    }
    _LIBCAMERA_POLL_IspmidFeUnlock(nDev);
}

void LIBCAMERA_POLL_IspmidResumeEvent(u32 nDev, u32 nCh)
{
    struct list_head *pos;
    IspIrqLink_t *    elm;

    _LIBCAMERA_POLL_IspmidFeLock(nDev);
    list_for_each(pos, &g_stIspFeWqList[nDev])
    {
        elm = list_entry(pos, IspIrqLink_t, list);
        elm->pdata.u32ChResume |= 0x1 << nCh;
        elm->ready = 1;
        wake_up_interruptible_all(&elm->waitq);
    }
    _LIBCAMERA_POLL_IspmidFeUnlock(nDev);
}



void LIBCAMERA_POLL_IspmidAlgoEvent(u32 nDev, u32 nCh, CUS3A_ALGO_TYPE_e eType, u8 bEn)
{
    struct list_head *pos;
    IspIrqLink_t *    elm;
    u32 *             pEn = NULL;

    _LIBCAMERA_POLL_IspmidFeLock(nDev);

    switch (eType)
    {
        case E_ALGO_TYPE_AE:
            pEn = &g_stCus3aChStatus[nDev].u32ChAeEn;
            break;
        case E_ALGO_TYPE_AWB:
            pEn = &g_stCus3aChStatus[nDev].u32ChAwbEn;
            break;
        case E_ALGO_TYPE_AF:
            pEn = &g_stCus3aChStatus[nDev].u32ChAfEn;
            break;
        default:
            CamOsPrintf("[%s] AlgoType Error %d\n", __FUNCTION__, eType);
            break;
    }

    if (bEn)
    {
        *pEn |= (0x1 << nCh);
    }
    else
    {
        *pEn &= ~(0x1 << nCh);
    }

    list_for_each(pos, &g_stIspFeWqList[nDev])
    {
        elm        = list_entry(pos, IspIrqLink_t, list);
        elm->ready = 1;
        wake_up_interruptible_all(&elm->waitq);
    }
    _LIBCAMERA_POLL_IspmidFeUnlock(nDev);
}

/*To broadcast user space there is debug command need to process*/
void LIBCAMERA_POLL_IspmidDebugEvent(u32 nDev, u16 uCh, u16 uCtrl, u32 uParam)
{
    typedef struct
    {
        u16 uCh;
        u16 uCtrl;
        u32 uParam;
    } Cus3aDbgParam_t;

    struct list_head *pos;
    IspIrqLink_t *    elm;

    // CamOsPrintf("[%s] dev=%d ch=%d ctrl=0x%X, param=0x%X\n", __FUNCTION__, nDev, uCh, uCtrl, uParam);
    _LIBCAMERA_POLL_IspmidFeLock(nDev);
    list_for_each(pos, &g_stIspFeWqList[nDev])
    {
        Cus3aDbgParam_t *pCus3aParam;
        elm = list_entry(pos, IspIrqLink_t, list);
        // elm->pdata.u64DbgCmd = uCmd;
        pCus3aParam         = (Cus3aDbgParam_t *)&elm->pdata.u64DbgCmd;
        pCus3aParam->uCh    = uCh;
        pCus3aParam->uCtrl  = uCtrl;
        pCus3aParam->uParam = uParam;
        elm->ready          = 1;
        wake_up_interruptible_all(&elm->waitq);
    }
    _LIBCAMERA_POLL_IspmidFeUnlock(nDev);
}
//------------ Customer 3A API Set/Get signal device ----------------------

//mhal ispmid poll end

