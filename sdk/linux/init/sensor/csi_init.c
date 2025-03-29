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
#include <cam_os_wrapper.h>
#include "cam_sysfs.h"

#define BANK_TO_ADDR32(b)        (b << 9)
#define SSTAR_CSI_MAX_SENSOR_NUM 4u

void DrvCsiDeInitBank(void);
void DrvCsiInitBank(u32 u32IoPhyBase);

void CSI_UT_cmd(const char *pCmdStr);
int  DRV_CSI_Init(u32 id);
int  DRV_CSI_Uninit(u32 id);
s32  DRV_CSI_ISR_IntShow(u32 nSnrPad, char *sBuf, unsigned int nStrLen);
s32  DRV_CSI_ISR_IsrHandler(u32 nSnrPad, void *para);

void         HalCsiSetLaneNum(u32 nSNRPadID, u8 nLaneNum);
void         Csi_ISR(void);
unsigned int Get1LaneGroupNum(void);
unsigned int Get2LaneGroupNum(void);
unsigned int Get4LaneGroupNum(void);
unsigned int GetCsiMaxSensorNum(void);
u32         *GetAnaLaneSwap(unsigned int num);
u32         *GetAnaPnSwap(unsigned int num);

static dev_t               g_stDevNo;
static struct device_node *pstDevNode = NULL;

static struct of_device_id g_stCsiDtIds[] = {{.compatible = "sstar,csi"}, {}};

typedef struct
{
    u64        count;
    u32        fs_int_count;  // frame start interrput count
    u32        vc0_int_count; // virtual channel 0 count
    spinlock_t lock;
    u8         frame_end; // frame end interrupt flag
    u8         frame_start;

    wait_queue_head_t *p_wq_fe;
    wait_queue_head_t *p_wq_fs;

    // HAL
    void *reg_base;
    void *hal_handle;

    struct miscdevice csi_dev;

    int irq[SSTAR_CSI_MAX_SENSOR_NUM];
} CsiDevData_t;

static int _DRV_CSI_MODULE_Open(struct inode *inode, struct file *fp)
{
    return 0;
}

static int _DRV_CSI_MODULE_Release(struct inode *inode, struct file *fp)
{
    return 0;
}
static ssize_t _DRV_CSI_MODULE_FRread(struct file *fp, char __user *buf, size_t size, loff_t *ppos)
{
    return 0;
}

static ssize_t _DRV_CSI_MODULE_FWrite(struct file *fp, const char __user *buf, size_t size, loff_t *ppos)
{
#ifdef __ENABLE_CSI_UT__
    char  cmd[64];
    char *pcKBuf = NULL;
    pcKBuf       = vzalloc(size);

    if (!copy_from_user((void *)pcKBuf, (void __user *)buf, size))
    {
        sscanf(pcKBuf, "%s", cmd);
        printk("[%s] CMD : %s\n", __FUNCTION__, cmd);
    }
    else
    {
        printk("[%s] copy user data failed!!!\n", __FUNCTION__);
        goto CSI_WRITE_EXIT;
    }

    CSI_UT_cmd(pcKBuf);

CSI_WRITE_EXIT:
    if (pcKBuf)
    {
        vfree(pcKBuf);
        pcKBuf = NULL;
    }
#endif
    return size;
}

static long _DRV_CSI_MODULE_IoCtl(struct file *fp, unsigned int cmd, unsigned long arg)
{
    printk("_DRV_CSI_MODULE_IoCtl not implement\n");
    return 0;
}

struct file_operations g_stCsiFOps = {
    .owner          = THIS_MODULE,
    .open           = _DRV_CSI_MODULE_Open,
    .release        = _DRV_CSI_MODULE_Release,
    .read           = _DRV_CSI_MODULE_FRread,
    .write          = _DRV_CSI_MODULE_FWrite,
    .unlocked_ioctl = _DRV_CSI_MODULE_IoCtl,
    .poll           = NULL,
};

static int _DRV_CSI_MODULE_Suspend(struct platform_device *pdev, pm_message_t state)
{
    return 0;
}

static int _DRV_CSI_MODULE_Resume(struct platform_device *pdev)
{
    return 0;
}

irqreturn_t DRV_CSI_MODULE_Isr(int num, void *priv)
{
    Csi_ISR();

    return IRQ_HANDLED;
}

int csi_devnode_init(void)
{
    pstDevNode = of_find_compatible_node(NULL, NULL, "sstar,csi");
    if (pstDevNode == NULL)
    {
        printk("[%s %d]ERROR: VIF of dts node failed!\n", __func__, __LINE__);
        return -ENODEV;
    }

    return 0;
}

void *csi_module_get_dev(void)
{
    return (void *)pstDevNode;
}

static int _DRV_CSI_MODULE_Probe(struct platform_device *pdev)
{
    u32           u32IoPhyBase, iloop;
    u32           proval = 0;
    CsiDevData_t *data;
    char          cBufName[32];
    u32           nSnrCnt;
    int           ret;
    struct device* orgDev = NULL;

    data = CamOsMemAlloc(sizeof(CsiDevData_t));
    if (!data)
    {
        return -ENOENT;
    }

    if (csi_devnode_init())
    {
        CamOsMemRelease(data);
        return -1;
    }

    if (CamOfPropertyReadU32Index(pdev->dev.of_node, "io_phy_addr", 0, &u32IoPhyBase) != 0)
    {
        printk("[CSI] io_phy_addr not set in dts!\n");
    }

    CamOsPrintf(KERN_DEBUG "[CSI] sensor num:%u\n", GetCsiMaxSensorNum());
    for (nSnrCnt = 0; nSnrCnt < GetCsiMaxSensorNum(); nSnrCnt++)
    {
        CamOsSnprintf(cBufName, sizeof(cBufName), "csi_sr%d_lane_num", nSnrCnt);
        if (!CamofPropertyReadU32(pdev->dev.of_node, cBufName, &proval))
        {
            HalCsiSetLaneNum(nSnrCnt, (u8)proval);
            CamOsPrintf(KERN_DEBUG "[CSI] lane num:%hhu\n", (u8)proval);

            CamOsSnprintf(cBufName, sizeof(cBufName), "csi_sr%d_lane_select", nSnrCnt);
            if (!CamOfPropertyReadU32Array(pdev->dev.of_node, cBufName, GetAnaLaneSwap(nSnrCnt), proval + 1))
            {
                for (iloop = 0; iloop < proval + 1; iloop++)
                {
                    CamOsPrintf(KERN_DEBUG "[CSI] %s:%u\n", cBufName, (GetAnaLaneSwap(nSnrCnt))[iloop]);
                }
            }
            CamOsSnprintf(cBufName, sizeof(cBufName), "csi_sr%d_lane_pn_swap", nSnrCnt);
            if (!CamOfPropertyReadU32Array(pdev->dev.of_node, cBufName, GetAnaPnSwap(nSnrCnt), proval + 1))
            {
                for (iloop = 0; iloop < proval + 1; iloop++)
                {
                    CamOsPrintf(KERN_DEBUG "[CSI] %s:%u\n", cBufName, (GetAnaPnSwap(nSnrCnt))[iloop]);
                }
            }
        }
    }

    DrvCsiInitBank(u32IoPhyBase);

    // dev
    data->csi_dev.minor  = MISC_DYNAMIC_MINOR;
    data->csi_dev.name   = "csi";
    data->csi_dev.fops   = &g_stCsiFOps;
    data->csi_dev.parent = &pdev->dev;
    misc_register(&data->csi_dev);

    pdev->dev.platform_data = (void *)data;

    for (iloop = 0; iloop < (Get1LaneGroupNum() + Get2LaneGroupNum() + Get4LaneGroupNum()); ++iloop)
    {
        data->irq[iloop] = CamIrqOfParseAndMap(pdev->dev.of_node, iloop);

        if (!data->irq[iloop])
        {
            break;
        }

        if (request_irq(data->irq[iloop], DRV_CSI_MODULE_Isr, 0, "CSI interrupt", (void *)data) != 0)
        {
            printk("Failed to request IRQ[%d]#%d\n", iloop, data->irq[iloop]);
        }
    }

    DRV_CSI_Init(0);

    return 0;
}

static int _DRV_CSI_MODULE_Remove(struct platform_device *pdev)
{
    CsiDevData_t *data = (CsiDevData_t *)pdev->dev.platform_data;
    u32           iloop;

    DRV_CSI_Uninit(0);

    for (iloop = 0; iloop < Get1LaneGroupNum(); ++iloop)
    {
        if (!data->irq[iloop])
        {
            break;
        }
        free_irq(data->irq[iloop], data);
    }

    for (iloop = Get1LaneGroupNum(); iloop < (Get1LaneGroupNum() + Get2LaneGroupNum()); ++iloop)
    {
        if (!data->irq[iloop])
        {
            break;
        }
        free_irq(data->irq[iloop], data);
    }

    for (iloop = Get2LaneGroupNum(); iloop < (Get2LaneGroupNum() + Get4LaneGroupNum()); ++iloop)
    {
        if (!data->irq[iloop])
        {
            break;
        }
        free_irq(data->irq[iloop], data);
    }

    DrvCsiDeInitBank();

    misc_deregister(&data->csi_dev);
    CamOsMemRelease(data);
    return 0;
}

static struct platform_driver g_stCsiDriver = {
    .probe   = _DRV_CSI_MODULE_Probe,
    .remove  = _DRV_CSI_MODULE_Remove,
    .suspend = _DRV_CSI_MODULE_Suspend,
    .resume  = _DRV_CSI_MODULE_Resume,
    .driver =
        {
            .name           = "sstar,csi",
            .owner          = THIS_MODULE,
            .of_match_table = of_match_ptr(g_stCsiDtIds),
        },
};

// int DRV_CSI_MODULE_CsiInit(void)
int DRV_CSI_MODULE_Init(void)
{
    int ret;

    ret = CamPlatformDriverRegister(&g_stCsiDriver);
    if (ret)
    {
        printk("[CSI] register driver fail");
        CamPlatformDriverUnregister(&g_stCsiDriver);
        return ret;
    }

    return 0;
}

void DRV_CSI_MODULE_Exit(void)
{
    CamPlatformDriverUnregister(&g_stCsiDriver);
    printk("[CSI] exit\n");
}
