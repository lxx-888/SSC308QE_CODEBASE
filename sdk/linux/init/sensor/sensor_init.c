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
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/kdev_t.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/miscdevice.h>
#include <asm/string.h>

#include <ms_msys.h>

#include <cam_sysfs.h>
#include <cam_clkgen.h>
#include <cam_os_wrapper.h>
#include <drv_ss_cus_sensor.h>
#include <mi_common_internal.h>

void DrvSensorIfUt(const char *pCmdStr);
s32  HalSensorSetI2CMaster(u32 nGroup, CUS_SENIF_BUS eBusIf, u32 nI2CMaster, u32 nNumLane);
void DRV_SENSOR_IF_CtxInit(void);
void DRV_SENSOR_IF_InitRxPadAttr(void);
s32  DRV_SENSOR_IF_RegBankAddrMap(void);
void DRV_SENSOR_IF_Dev2SnrPadCtxInit(void);
s32  DRV_SENSOR_IF_InitCommon(void);
s32  DRV_SENSOR_IF_DeinitCommon(void);

static struct device_node *pstDevNode = NULL;

int SensorOfPropertyReadU32(const char *propname, u32 *out_value)
{
    if (pstDevNode == NULL)
    {
        printk("[%s %d]ERROR: SNR pstDevNode is NULL!\n", __func__, __LINE__);
        return -1;
    }
    else
    {
        return CamofPropertyReadU32(pstDevNode, propname, out_value);
    }
}

int SensorOfPropertyReadU32Array(const char *propname, u32 *out_values, size_t sz)
{
    if (pstDevNode == NULL)
    {
        printk("[%s %d]ERROR: SNR pstDevNode is NULL!\n", __func__, __LINE__);
        return -1;
    }
    else
    {
        return CamOfPropertyReadU32Array(pstDevNode, propname, out_values, sz);
    }
}

void *sensorif_module_get_dev(void)
{
    return (void *)pstDevNode;
}

static int _SENSORIF_MODULE_Open(struct inode *inode, struct file *fp)
{
    return 0;
}

static int _SENSORIF_MODULE_Release(struct inode *inode, struct file *fp)
{
    return 0;
}

static ssize_t _SENSORIF_MODULE_FRead(struct file *fp, char __user *buf, size_t size, loff_t *ppos)
{
    return 0;
}

static ssize_t _SENSORIF_MODULE_FWrite(struct file *fp, const char __user *buf, size_t size, loff_t *ppos)
{
#ifdef __ENABLE_SENSOR_UT__ // if needed eddie20230201
    char  cmd[128];
    char *pKBuf = NULL;

    pKBuf = vmalloc(size);

    if (!copy_from_user((void *)pKBuf, (void __user *)buf, size))
    {
        sscanf(pKBuf, "%s", cmd);
        CamOsPrintf("[%s] CMD : %s\n", __FUNCTION__, cmd);
    }
    else
    {
        CamOsPrintf("[%s] copy user data failed!!!\n", __FUNCTION__);
        goto SENSORIF_WRITE_EXIT;
    }

    DrvSensorIfUt(pKBuf); // TODO: how to delete

SENSORIF_WRITE_EXIT:
    if (pKBuf)
    {
        vfree(pKBuf);
        pKBuf = NULL;
    }
#endif
    return size;
}

static long _SENSORIF_MODULE_Ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
    return 0;
}

struct file_operations g_stSensorIfFOps = {
    .owner          = THIS_MODULE,
    .open           = _SENSORIF_MODULE_Open,
    .release        = _SENSORIF_MODULE_Release,
    .read           = _SENSORIF_MODULE_FRead,
    .write          = _SENSORIF_MODULE_FWrite,
    .unlocked_ioctl = _SENSORIF_MODULE_Ioctl,
};

void SensorWARN_ON(void)
{
    WARN_ON(1);
}

void *SensorIoremap(u32 addr, u32 size)
{
    return ioremap(addr, size);
}

void SensorIoUnmap(void *addr)
{
    iounmap(addr);
}

typedef struct
{
    struct miscdevice stSensorIfDev;
    dev_t             g_stDevNo;   // for dynamic major number allocation
    dev_t             dev;         // for dynamic major number allocation
} SensorIfData_t;

int SENSORIF_MODULE_SstarSensorIfProbe(struct platform_device *pdev)
{
    SensorIfData_t    *pdata;
    struct miscdevice *pSensorIfDev;
    u32                proval;
    char               aBufName[48];
    u32                nSnrCnt;

    // init Rx pad info
    pstDevNode = pdev->dev.of_node;
    DRV_SENSOR_IF_InitRxPadAttr();
    DRV_SENSOR_IF_RegBankAddrMap();

    pdata = CamOsMemAlloc(sizeof(SensorIfData_t));
    CamOsMemset(pdata, 0, sizeof(SensorIfData_t));
    pdev->dev.platform_data = (void *)pdata;

    pSensorIfDev         = &pdata->stSensorIfDev;
    pSensorIfDev->minor  = MISC_DYNAMIC_MINOR;
    pSensorIfDev->name   = "sensorif";
    pSensorIfDev->fops   = &g_stSensorIfFOps;
    pSensorIfDev->parent = &pdev->dev;
    misc_register(pSensorIfDev);

    DRV_SENSOR_IF_CtxInit(); // TODO:these API can not be here

    DRV_SENSOR_IF_InitCommon();

    DRV_SENSOR_IF_Dev2SnrPadCtxInit();

    // skip frame status init
    // DRV_SENSOR_IF_SkipframeInit();

    return 0;
}

int SENSORIF_MODULE_SstarSensorIfRemove(struct platform_device *pdev)
{
    SensorIfData_t    *pdata        = dev_get_platdata(&pdev->dev);
    struct miscdevice *pSensorIfDev = &pdata->stSensorIfDev;

    /*replace device_remove_file -> CamDeviceRemoveFile*/

    DRV_SENSOR_IF_DeinitCommon();

    misc_deregister(pSensorIfDev);

    kfree(pdata);
    return 0;
}

/* Match table for of_platform binding */
static const struct of_device_id G_ST_SSTAR_SENSOR_IF_MATCH[] = {
    {.compatible = "sstar,sensorif", 0},
    {},
};
MODULE_DEVICE_TABLE(of, G_ST_SSTAR_SENSOR_IF_MATCH);

static struct platform_driver g_stSstarSensorIfDriver = {
    .probe  = SENSORIF_MODULE_SstarSensorIfProbe,
    .remove = SENSORIF_MODULE_SstarSensorIfRemove,
    .driver =
        {
            .name           = "sstar,sensorif",
            .owner          = THIS_MODULE,
            .of_match_table = G_ST_SSTAR_SENSOR_IF_MATCH,
        },
};

int SENSORIF_MODULE_SstarSensorIfInitDriver(void)
{
    return CamPlatformDriverRegister(&g_stSstarSensorIfDriver);
}

void SENSORIF_MODULE_SstarSensorIfExitDriver(void)
{
    CamPlatformDriverUnregister(&g_stSstarSensorIfDriver);
}

#pragma message("compile mi and mhal")
DECLEAR_MODULE_INIT_EXIT

MI_MODULE_LICENSE();
