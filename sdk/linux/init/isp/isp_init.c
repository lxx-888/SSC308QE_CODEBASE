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
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/hrtimer.h>
#include <linux/input.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/stringify.h>
#include <linux/kernel.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/kobject.h>
#include <linux/dcache.h>
#include <linux/namei.h>
#include <linux/sysfs.h>
#include <linux/kernel.h>
#include <linux/vmalloc.h>

#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/workqueue.h>

#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/clk.h>
#include <linux/miscdevice.h>
#include <linux/mod_devicetable.h>
#include <linux/clk-provider.h>

#include <linux/io.h>

// #include <drv_isp_ioctls.h>
// #include <hal_isp_shadow.h>
// #include <drv_isp.h>
#include "irqs.h"
#include <ms_msys.h>
#include <cam_os_wrapper.h>
// #include <drv_isp_work_pool.h>
// #include "hal_interrupt_handler.h"
// #include <isplog.h>
// #include <isp_mem_api.h>
#include "cam_clkgen.h"
#include "cam_sysfs.h"

#include "isp_init.h"
#include <linux/platform_device.h>


#ifndef EXTRA_MODULE_NAME
#define EXTRA_MODULE_NAME isp
#endif
#include "mi_common_internal.h"

//#include "mi_isp_impl.h"
//#include "mi_isp_datatype.h"
#ifndef UNUSED
#define UNUSED(var) (void)((var) = (var))
#endif

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (1)
#endif

int  MLOAD_MODULE_Init(void);    // NOLINT
void MLOAD_MODULE_Cleanup(void); // NOLINT

int  DRV_ISRCB_EarlyInit(void); // NOLINT
int  DRV_ISP_MODULE_IspDriverInit(void); // NOLINT
void DRV_ISP_MODULE_IspDriverExit(void); // NOLINT

int  LIBCAMERA_MODULE_DriverInit(void); // NOLINT
void LIBCAMERA_MODULE_DriverExit(void); // NOLINT

//mhal

//decoupling tmp start
//extern CamClass_t *msys_get_sysfs_camclass(void);
//extern int CamDeviceCreateFile(struct CamDevice *dev, const CamDeiveAttribute_t *attr);
//extern void CamDeviceRemoveFile(struct CamDevice *dev, const CamDeiveAttribute_t *attr);
//decoupling tmp end

extern int DRV_ISP_PROXY_TopIsp0Isr(int num, void *priv);
extern int DRV_ISP_PROXY_Isp0Probe(unsigned int u32DevId);
extern int DRV_ISP_PROXY_Isp0Remove(unsigned int u32DevId);
extern unsigned int DRV_ISP_PROXY_GetSupportClockNum(unsigned int u32DevId);
extern unsigned int DRV_ISP_PROXY_GetDefaultClock(unsigned int u32DevId, unsigned int *pu32DefClkIdx);
extern unsigned int DRV_ISP_PROXY_SetClockRate(unsigned int u32DevId, int s32ClkRate);
extern int DRV_ISP_PROXY_DriverInit(unsigned int u32DevId);
extern int DRV_ISP_PROXY_DriverExit(unsigned int u32DevId);
extern int DRV_ISP_PROXY_GetFwriteParamNum(void);
extern int DRV_ISP_PROXY_Isp0Fwrite(char *pstrCmd, int *ps8Param);
extern int DRV_ISP_PROXY_Isp0Ioctl(unsigned int u32Cmd);
extern int DRV_ISP_PROXY_Isp0Suspend(unsigned int u32DevId);
extern int DRV_ISP_PROXY_Isp0Resume(unsigned int u32DevId);


extern void DRV_ISP_CLK_PreSetClkIdx(s32 s32ClockIdx);
extern void DRV_ISP_CLK_Init(u32 nDevId);
extern void DRV_ISP_CLK_DeInit(void);
extern s32 DRV_ISP_CLK_GetNodeIdx(u32 nDevId, u32 u32ClkNode);
extern s32 DRV_ISP_CLK_GetPreClkIdx(void);


#if 0
#include <mdrv_ms_version.h>
#if defined(MVXV_EXT)
#define MVXV_V2 "MVX" MVXV_HEAD_VER MVXV_LIB_TYPE MVXV_CHIP_ID MVXV_CHANGELIST MVXV_COMP_ID MVXV_EXT "#XVM"
#else
#define MVXV_V2 "MVX" MVXV_HEAD_VER MVXV_LIB_TYPE MVXV_CHIP_ID MVXV_CHANGELIST MVXV_COMP_ID "#XVM"
#endif
    const char* LX_VERSION = MVXV_V2;
#endif

// ========================================================

#include <linux/io.h>

#define ISP_DEV_MAX (1)

static unsigned long g_uRegBase                                   = 0;
static unsigned int  g_u32IspClockResumeState[ISP_DEV_MAX] = {0};
static int           g_as32Major[ISP_DEV_MAX];
static dev_t         g_astDevNo[ISP_DEV_MAX];
static dev_t         g_astDev[ISP_DEV_MAX];

/* virtual to riu base */
inline u32 IspSysRegV2R(void *virt)
{
    return (unsigned long)virt - g_uRegBase;
}

void *IspSysGetRegBase(void)
{
    if (!g_uRegBase)
    {
#if 1
        g_uRegBase = (unsigned long)ioremap(0x1F000000, 0x400000);
#else
        g_uRegBase = (unsigned long)IO_ADDRESS;
#endif
    }

    return (void *)g_uRegBase;
}

// ========================================================
extern int g_s32IspIsrNum[ISP_DEV_MAX];

int DRV_ISP_MODULE_GetIrqNum(u32 nDevId)
{
    return g_s32IspIsrNum[nDevId];
}

irqreturn_t DRV_ISP_MODULE_TopIsp0Isr(int num, void *priv)
{
    int s32Ret = 0;

    if (num != g_s32IspIsrNum[0]) {
        ;
    }

    if (g_s32IspIsrNum[0] & 0x8000)
    {
        return IRQ_HANDLED;
    }

    s32Ret = DRV_ISP_PROXY_TopIsp0Isr(num, priv);

    if (s32Ret < 0) {
        return IRQ_NONE;
    } else {
        return IRQ_HANDLED;
    }

    return IRQ_NONE;
}

// ========================================================

//#define MAJOR_ISP_NUM               234
//#define MINOR_ISP_NUM               128
//#define MINOR_CSI_NUM               127

static int          _DRV_ISP_MODULE_Isp0Probe(struct platform_device *pdev);
//static int          _DRV_ISP_MODULE_Isp0Suspend(struct platform_device *pdev, pm_message_t state);
//static int          _DRV_ISP_MODULE_Isp0Resume(struct platform_device *pdev);
static int          _DRV_ISP_MODULE_Isp0Remove(struct platform_device *pdev);


int DRV_ISP_MODULE_IspClockSuspend(u32 nDevId, struct platform_device *pdev);
int DRV_ISP_MODULE_IspClockResume(u32 nDevId, struct platform_device *pdev);

static struct of_device_id g_stIsp0DeviceTableIds[] = {{.compatible = "isp"}, {}};
MODULE_DEVICE_TABLE(of, g_stIsp0DeviceTableIds);

static struct platform_device *g_pstIspDevice[ISP_DEV_MAX];
static struct platform_driver  g_stIsp0Driver = {
    .probe   = _DRV_ISP_MODULE_Isp0Probe,
    .remove  = _DRV_ISP_MODULE_Isp0Remove,
    //.suspend = _DRV_ISP_MODULE_Isp0Suspend,
    //.resume  = _DRV_ISP_MODULE_Isp0Resume,
    .driver =
        {
            .name           = "isp",
            .owner          = THIS_MODULE,
            .of_match_table = of_match_ptr(g_stIsp0DeviceTableIds),
        },
};

typedef struct DRV_ISP_MODULE_IspDevData_s
{
    struct miscdevice isp_dev;
    struct  CamDevice *   sysfs_dev; // for node /sys/class/sstar/
} IspDevData_t;

#if 0
static int _DRV_ISP_MODULE_Isp0Suspend(struct platform_device *pdev, pm_message_t state)
{
    u32 nDevId = 0;
    if (!pdev)
    {
        return -EINVAL;
    }

    DRV_ISP_PROXY_Isp0Suspend(nDevId);
    DRV_ISP_MODULE_IspClockSuspend(nDevId, pdev);

    UNUSED(state);

    return 0;
}

static int _DRV_ISP_MODULE_Isp0Resume(struct platform_device *pdev)
{
    u32 nDevId = 0;
    if (!pdev)
    {
        return -EINVAL;
    }

    DRV_ISP_MODULE_IspClockResume(nDevId, pdev);
    DRV_ISP_PROXY_Isp0Resume(nDevId);

    return 0;
}
#endif

u32 g_u32PreSetClkRate[ISP_DEV_MAX] = {0};
s32 g_s32PreSetClkIdx[ISP_DEV_MAX]  = {-1};

////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////

void DRV_ISP_MODULE_IspClkGenInit(u32 nDevId, struct platform_device *pdev, s32 s32ClkIdx)
{
    DRV_ISP_CLK_PreSetClkIdx(s32ClkIdx);
    DRV_ISP_CLK_Init(nDevId);
}

void DRV_ISP_MODULE_IspClkGenDeinit(u32 nDevId, struct platform_device *pdev)
{
    DRV_ISP_CLK_DeInit();
}

int DRV_ISP_MODULE_IspClockSuspend(u32 nDevId, struct platform_device *pdev)
{
    if (g_u32IspClockResumeState[nDevId] == TRUE)
    {
        DRV_ISP_MODULE_IspClkGenDeinit(nDevId, pdev);
        g_u32IspClockResumeState[nDevId] = FALSE;
    }
    else
    {
        CamOsPrintf("ISP clock suspend state already!!!\n");
    }

    return 0;
}

int DRV_ISP_MODULE_IspClockResume(u32 nDevId, struct platform_device *pdev)
{
    if (g_u32IspClockResumeState[nDevId] == FALSE)
    {
        DRV_ISP_MODULE_IspClkGenInit(nDevId, pdev, DRV_ISP_CLK_GetNodeIdx(nDevId, DRV_ISP_CLK_GetPreClkIdx()));
        g_u32IspClockResumeState[nDevId] = TRUE;
    }
    else
    {
        CamOsPrintf("ISP clock resume state already!!!\n");
    }

    return 0;
}

static int _DRV_ISP_MODULE_Isp0Probe(struct platform_device *pdev)
{
    int           err = 0;
    int           ret = 0;
    #if 0 //Coverity #1660788
    IspDevData_t *data;
    #endif
    u32           u32PreClkIdx = 0;
    s32           s32CurClkIdx = -1;
    u32           nDevId       = 0;
    char          s8DevName[16]= {0x00};
    struct CamDevice * pstCamDevice = NULL;
    struct device* orgDev = NULL;

    // CamOsPrintf("=== [%s] ===\n", __FUNCTION__);

    #if 0 //Coverity #1660788
    data = kzalloc(sizeof(IspDevData_t), GFP_KERNEL);
    if (NULL == data)
    {
        CamOsPrintf("%s, line:%d error!\n", __FUNCTION__, __LINE__);
        return -ENOENT;
    }
    #endif

    g_pstIspDevice[nDevId] = pdev;

    if (0 == CamofPropertyReadU32(pdev->dev.of_node, "clock-frequency-index", &u32PreClkIdx))
    {
        s32CurClkIdx = (s32)u32PreClkIdx;
    }

#ifdef __I7_ISP__
    s32CurClkIdx = 7; // change to SYS_PLL 600MHz as default temporily, need to modify it in DTS
#endif

    // CamOsPrintf("[_DRV_ISP_MODULE_Isp0Probe][s32CurClkIdx] = %d\n", s32CurClkIdx);

    DRV_ISP_MODULE_IspClkGenInit(nDevId, pdev, s32CurClkIdx);
    g_u32IspClockResumeState[nDevId] = TRUE;

/*
    // setup kernel i2c
    data->isp_dev.minor  = MISC_DYNAMIC_MINOR;
    data->isp_dev.name   = "isp";
#if 0 //DEV_MOVE_TO_MI
    data->isp_dev.fops   = &g_stIsp0FileOps;
#else
    data->isp_dev.fops   = NULL;
#endif
    data->isp_dev.parent = &pdev->dev;
    misc_register(&data->isp_dev);

    ret = alloc_chrdev_region(&g_astDevNo[nDevId], 0, 1, "isp0");
    if (ret < 0)
    {
        CamOsPrintf("isp major number allocation is failed\n");
    }

    g_as32Major[nDevId] = MAJOR(g_astDevNo[nDevId]);
    g_astDev[nDevId]    = MKDEV(g_as32Major[nDevId], 0);
    CamOsStrncpy(s8DevName, "isp0", CamOsStrlen("isp0"));
    pstCamDevice = CamDeviceCreate(msys_get_sysfs_camclass(), NULL, g_astDev[nDevId], NULL, s8DevName);
    data->sysfs_dev = pstCamDevice;
    orgDev = CamDeviceGetInternalDevice(pstCamDevice);
    // isp_create_bin_file(data->sysfs_dev);
    err                            = CamSysfsCreateLink(&pdev->dev.parent->kobj, &orgDev->kobj,
                                                        "isp0"); // create symlink for older firmware version
    orgDev->platform_data = pdev->dev.platform_data = (void *)data;
*/
    DRV_ISP_PROXY_Isp0Probe(nDevId);

    /* Prepare ISP ISR */
    g_s32IspIsrNum[nDevId] = CamIrqOfParseAndMap(pdev->dev.of_node, 0);
    CamOsPrintf("[ISP] Request IRQ Num: %d\n", g_s32IspIsrNum[nDevId]);
#ifndef __DISABLE_MHAL_ISR_REGISTER__
    {
        int nResult;
        CamOsPrintf("== [ISP] Register ISR ==\n");
        nResult = request_irq(g_s32IspIsrNum[nDevId], DRV_ISP_MODULE_TopIsp0Isr, 0, "isp interrupt", (void *)0);
        if (nResult)
        {
            CamOsPrintf("[ISP] isp interrupt failed by %d\n", nResult);
        }
        else
        {
            CamOsIrqSetAffinityHint(g_s32IspIsrNum[nDevId], ((const struct CamOsCpuMask *)&__cpu_online_mask));
        }
    }
#endif

#if 0 //DEV_MOVE_TO_MI
    if (DRV_ISP_SYSFS_CreateAttrFile(nDevId, pstCamDevice) == -EINVAL)
    {
        CamOsPrintf("[ISP] Create isp attr file fail!!!\n");
    }
#endif

    return 0;
}

static int _DRV_ISP_MODULE_Isp0Remove(struct platform_device *pdev)
{
    IspDevData_t *data   = dev_get_platdata(&pdev->dev);
    u32           nDevId = 0;
    struct device* orgDev = NULL;
    orgDev = data->sysfs_dev->device;

    if (DRV_ISP_PROXY_Isp0Remove(nDevId) < 0) {
        CamOsPrintf("isp0 never existed");
    }

    DRV_ISP_MODULE_IspClkGenDeinit(nDevId, pdev);
    g_u32IspClockResumeState[nDevId] = FALSE;

/*
    misc_deregister(&data->isp_dev);
    CamSysfsDeleteLink(&pdev->dev.parent->kobj, &orgDev->kobj, "isp0");
    CamDeviceDestroy(msys_get_sysfs_camclass(), g_astDev[nDevId]);
    unregister_chrdev_region(g_astDev[nDevId], 1);
*/
    kfree(data);
    return 0;
}

int DRV_ISP_MODULE_IspDriverInit(void)
{
    int ret = 0;

    CamOsPrintf("[ISP Driver init]\n");
    DRV_ISP_PROXY_DriverInit(0);
    ret = CamPlatformDriverRegister(&g_stIsp0Driver);
    if (ret)
    {
        CamOsPrintf("[%s] ISP0 Driver register fail !!!\n", __FUNCTION__);
        return ret;
    }
    return 0;
}

void DRV_ISP_MODULE_IspDriverExit(void)
{
    CamPlatformDriverUnregister(&g_stIsp0Driver);
    DRV_ISP_PROXY_DriverExit(0);
    CamOsPrintf("[ISP Driver exit]\n");
}

void *DRV_ISP_MODULE_GetClkDev(u32 uDevId)
{
    struct device_node *pstDevNode = (g_pstIspDevice[uDevId] != NULL) ? ((struct platform_device *)g_pstIspDevice[uDevId])->dev.of_node : NULL;

    return pstDevNode;
}


//mhal end

//mhal sysfs
#define DrvIspOsScnprintf(buf, size, _fmt, _args...) CamOsSnprintf(buf, size, _fmt, ##_args)

// extern int DrvIspForceIpReset(u32 nDevId, DRV_ISP_HANDLE handle, u32 u32ResetFlag);

// extern IspIsrStatis_t g_stIspInts;


typedef struct DRV_ISP_SYSFS_IspDevMap_s
{
    struct  CamDevice *sysfs_dev; // for node /sys/class/sstar/
} IspDevMap_t;

static IspDevMap_t g_stIspDevMap[ISP_DEV_MAX] = {0};


struct mi_isp_internal_api_t;
extern struct mi_isp_internal_api_t g_mi_isp_internal_api;
EXPORT_SYMBOL(g_mi_isp_internal_api);

//mhal sysfs end

#pragma message("compile mi and mhal")
DECLEAR_MODULE_INIT_EXIT


MI_MODULE_LICENSE();
MODULE_AUTHOR("Sigmastar");


