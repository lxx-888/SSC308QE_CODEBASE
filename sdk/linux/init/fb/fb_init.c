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

//=============================================================================
// Include Files
//=============================================================================
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <asm/string.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/pfn.h>
#include <linux/delay.h>
#include <linux/compat.h>
#include <linux/version.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/compiler.h>
#include <linux/proc_fs.h>

#include "mstar_chip.h"
#include "cam_os_wrapper.h"
#include "cam_device_wrapper.h"
#include "cam_proc_wrapper.h"
#include "mi_fb.h"

#define ASCII_COLOR_CYAN   "\033[1;36m"
#define ASCII_COLOR_YELLOW "\033[1;33m"
#define ASCII_COLOR_RED    "\033[1;31m"
#define ASCII_COLOR_END    "\033[0m"
#define DBG_INFO(fmt, args...)                                                                                         \
    (                                                                                                                  \
        {                                                                                                              \
            do                                                                                                         \
            {                                                                                                          \
                if (g_debugSwitch)                                                                                     \
                {                                                                                                      \
                    printk(KERN_DEBUG ASCII_COLOR_CYAN "%s[%d] " fmt ASCII_COLOR_END, __FUNCTION__, __LINE__, ##args); \
                }                                                                                                      \
            } while (0);                                                                                               \
        })
#define DBG_WRN(fmt, args...) \
    printk(KERN_WARNING ASCII_COLOR_YELLOW "%s[%d] " fmt ASCII_COLOR_END, __FUNCTION__, __LINE__, ##args);
#define DBG_ERR(fmt, args...) \
    printk(KERN_ERR ASCII_COLOR_RED "%s[%d] " fmt ASCII_COLOR_END, __FUNCTION__, __LINE__, ##args);

#define FB_NUM_MAX 16

#if defined(CONFIG_SIGMASTAR_CHIP_TIRAMISU) && (CONFIG_SIGMASTAR_CHIP_TIRAMISU == 1)
#define FBDEV_CPU_NO_SUPPORT_MIU_MAPP_MIN_ADDR 0x60000000
#define FBDEV_LPAE_BASE_ADDR                   0x100000000
#define FBDEV_MIU_TO_CPU_BUS_ADDR(miu_phy_addr)                                               \
    (                                                                                         \
        {                                                                                     \
            MI_PHY phyCpuBusAddr = -1ULL;                                                     \
            if (miu_phy_addr >= (MI_PHY)ARM_MIU2_BASE_ADDR)                                   \
            {                                                                                 \
                phyCpuBusAddr = miu_phy_addr + ARM_MIU2_BUS_BASE - ARM_MIU2_BASE_ADDR;        \
            }                                                                                 \
            else if (miu_phy_addr >= (MI_PHY)ARM_MIU1_BASE_ADDR)                              \
            {                                                                                 \
                phyCpuBusAddr = miu_phy_addr + ARM_MIU1_BUS_BASE - ARM_MIU1_BASE_ADDR;        \
            }                                                                                 \
            else if (miu_phy_addr >= (MI_PHY)ARM_MIU0_BASE_ADDR)                              \
            {                                                                                 \
                if (miu_phy_addr >= FBDEV_CPU_NO_SUPPORT_MIU_MAPP_MIN_ADDR)                   \
                {                                                                             \
                    phyCpuBusAddr = miu_phy_addr + FBDEV_LPAE_BASE_ADDR - ARM_MIU0_BASE_ADDR; \
                }                                                                             \
                else                                                                          \
                {                                                                             \
                    phyCpuBusAddr = miu_phy_addr + ARM_MIU0_BUS_BASE - ARM_MIU0_BASE_ADDR;    \
                }                                                                             \
            }                                                                                 \
            phyCpuBusAddr;                                                                    \
        })
#else
#define FBDEV_MIU_TO_CPU_BUS_ADDR(miu_phy_addr) CamOsMemMiuToPhys((ss_miu_addr_t)miu_phy_addr)
#endif

static MI_U32                 g_u32FirstFbNode = 0;
static MI_U32                 g_u32LayerCnt    = 0;
static struct fb_info *       g_pastFbInfo[FB_NUM_MAX];
static int                    g_debugSwitch = 0;
static struct proc_dir_entry *gp_stFbDir;

static void _MDRV_FB_RecvVarScreenInfo(MI_FB_VarScreenInfo_t *pstVarScreenInfo, struct fb_var_screeninfo *var)
{
    pstVarScreenInfo->u32Xres         = var->xres;
    pstVarScreenInfo->u32Yres         = var->yres;
    pstVarScreenInfo->u32Xres_virtual = var->xres_virtual;
    pstVarScreenInfo->u32Yres_virtual = var->yres_virtual;
    pstVarScreenInfo->u32Xoffset      = var->xoffset;
    pstVarScreenInfo->u32Yoffset      = var->yoffset;
    pstVarScreenInfo->u32BitsPerPixel = var->bits_per_pixel;
    pstVarScreenInfo->u32Grayscale    = var->grayscale;

    pstVarScreenInfo->stBitFieldRed.u32Offset      = var->red.offset;
    pstVarScreenInfo->stBitFieldRed.u32Length      = var->red.length;
    pstVarScreenInfo->stBitFieldRed.u32MsbRight    = var->red.msb_right;
    pstVarScreenInfo->stBitFieldGreen.u32Offset    = var->green.offset;
    pstVarScreenInfo->stBitFieldGreen.u32Length    = var->green.length;
    pstVarScreenInfo->stBitFieldGreen.u32MsbRight  = var->green.msb_right;
    pstVarScreenInfo->stBitFieldBlue.u32Offset     = var->blue.offset;
    pstVarScreenInfo->stBitFieldBlue.u32Length     = var->blue.length;
    pstVarScreenInfo->stBitFieldBlue.u32MsbRight   = var->blue.msb_right;
    pstVarScreenInfo->stBitFieldTransp.u32Offset   = var->transp.offset;
    pstVarScreenInfo->stBitFieldTransp.u32Length   = var->transp.length;
    pstVarScreenInfo->stBitFieldTransp.u32MsbRight = var->transp.msb_right;

    pstVarScreenInfo->u32Nonstd = var->nonstd;
    pstVarScreenInfo->u32Rotate = var->rotate;
}

#if 0
static void _MDRV_FB_RecvFixScreenInfo(MI_FB_FixScreenInfo_t *pstFixScreenInfo, struct fb_fix_screeninfo *fix)
{
    strncpy(pstFixScreenInfo->u8Id, fix->id, sizeof(pstFixScreenInfo->u8Id));
    pstFixScreenInfo->phySmemStart   = fix->smem_start;
    pstFixScreenInfo->u32SmemLen     = fix->smem_len;
    pstFixScreenInfo->u32Type        = fix->type;
    pstFixScreenInfo->u32TypeAux     = fix->type_aux;
    pstFixScreenInfo->u32Visual      = fix->visual;
    pstFixScreenInfo->u16XpanStep    = fix->xpanstep;
    pstFixScreenInfo->u16YpanStep    = fix->ypanstep;
    pstFixScreenInfo->u16YwrapStep   = fix->ywrapstep;
    pstFixScreenInfo->u32LineLength  = fix->line_length;
    pstFixScreenInfo->phyMmioStart   = fix->mmio_start;
    pstFixScreenInfo->u32MmioLen     = fix->mmio_len;
    pstFixScreenInfo->u32Accel       = fix->accel;
}
#endif

static void _MDRV_FB_TranVarScreenInfo(struct fb_var_screeninfo *var, MI_FB_VarScreenInfo_t *pstVarScreenInfo)
{
    var->xres           = pstVarScreenInfo->u32Xres;
    var->yres           = pstVarScreenInfo->u32Yres;
    var->xres_virtual   = pstVarScreenInfo->u32Xres_virtual;
    var->yres_virtual   = pstVarScreenInfo->u32Yres_virtual;
    var->xoffset        = pstVarScreenInfo->u32Xoffset;
    var->yoffset        = pstVarScreenInfo->u32Yoffset;
    var->bits_per_pixel = pstVarScreenInfo->u32BitsPerPixel;
    var->grayscale      = pstVarScreenInfo->u32Grayscale;

    var->red.offset       = pstVarScreenInfo->stBitFieldRed.u32Offset;
    var->red.length       = pstVarScreenInfo->stBitFieldRed.u32Length;
    var->red.msb_right    = pstVarScreenInfo->stBitFieldRed.u32MsbRight;
    var->green.offset     = pstVarScreenInfo->stBitFieldGreen.u32Offset;
    var->green.length     = pstVarScreenInfo->stBitFieldGreen.u32Length;
    var->green.msb_right  = pstVarScreenInfo->stBitFieldGreen.u32MsbRight;
    var->blue.offset      = pstVarScreenInfo->stBitFieldBlue.u32Offset;
    var->blue.length      = pstVarScreenInfo->stBitFieldBlue.u32Length;
    var->blue.msb_right   = pstVarScreenInfo->stBitFieldBlue.u32MsbRight;
    var->transp.offset    = pstVarScreenInfo->stBitFieldTransp.u32Offset;
    var->transp.length    = pstVarScreenInfo->stBitFieldTransp.u32Length;
    var->transp.msb_right = pstVarScreenInfo->stBitFieldTransp.u32MsbRight;

    var->nonstd = pstVarScreenInfo->u32Nonstd;
    var->rotate = pstVarScreenInfo->u32Rotate;
}

static void _MDRV_FB_TranFixScreenInfo(struct fb_fix_screeninfo *fix, MI_FB_FixScreenInfo_t *pstFixScreenInfo)
{
    strncpy(fix->id, (char *)pstFixScreenInfo->u8Id, sizeof(fix->id) - 1);
    fix->id[sizeof(fix->id) - 1] = '\0';
    fix->smem_start              = pstFixScreenInfo->phySmemStart;
    fix->smem_len                = pstFixScreenInfo->u32SmemLen;
    fix->type                    = pstFixScreenInfo->u32Type;
    fix->type_aux                = pstFixScreenInfo->u32TypeAux;
    fix->visual                  = pstFixScreenInfo->u32Visual;
    fix->xpanstep                = pstFixScreenInfo->u16XpanStep;
    fix->ypanstep                = pstFixScreenInfo->u16YpanStep;
    fix->ywrapstep               = pstFixScreenInfo->u16YwrapStep;
    fix->line_length             = pstFixScreenInfo->u32LineLength;
    fix->mmio_start              = pstFixScreenInfo->phyMmioStart;
    fix->mmio_len                = pstFixScreenInfo->u32MmioLen;
    fix->accel                   = pstFixScreenInfo->u32Accel;
}

static void _MDRV_FB_PrintDisplayAttrInfo(MI_U8 u8FbId, MI_FB_DisplayLayerAttr_t *pstDisplayLayerAttr, MI_BOOL bIsSet)
{
    DBG_INFO(
        "[fb:%d] %s display attr => "
        "(x:%d,y:%d,dstw:%d,dsth:%d,buffw:%d,buffh:%d,timing:(%dx%d),premul:%d,fmt:%d,outclr:%d,dstplane:%d,mask:0x%x)"
        "\n",
        u8FbId, (bIsSet ? "set" : "get"), pstDisplayLayerAttr->u32Xpos, pstDisplayLayerAttr->u32YPos,
        pstDisplayLayerAttr->u32dstWidth, pstDisplayLayerAttr->u32dstHeight, pstDisplayLayerAttr->u32DisplayWidth,
        pstDisplayLayerAttr->u32DisplayHeight, pstDisplayLayerAttr->u32ScreenWidth,
        pstDisplayLayerAttr->u32ScreenHeight, pstDisplayLayerAttr->bPreMul, pstDisplayLayerAttr->eFbColorFmt,
        pstDisplayLayerAttr->eFbOutputColorSpace, pstDisplayLayerAttr->eFbDestDisplayPlane,
        pstDisplayLayerAttr->u32SetAttrMask);
}

static void _MDRV_FB_PrintCursorAttrInfo(MI_U8 u8FbId, MI_FB_CursorAttr_t *pstCursorAttr, MI_BOOL bIsSet)
{
    DBG_INFO(
        "[fb:%d] %s cursor info => (xpos:%d,ypos:%d,hotspotx:%d,hotspoty:%d, "
        "alpha(mode:%d,alpha0:%d,alpha1:%d,galpha:%d,planealpha:%d), "
        "clrkey(en:%d,r:%d,g:%d,b:%d), bshown:%d, "
        "image(w:%d,h:%d,pitch:%d,fmt:%d,data:%p),mask:0x%x)\n",
        u8FbId, (bIsSet ? "set" : "get"), pstCursorAttr->u32XPos, pstCursorAttr->u32YPos, pstCursorAttr->u32HotSpotX,
        pstCursorAttr->u32HotSpotY, pstCursorAttr->stAlpha.u8AlphaMode, pstCursorAttr->stAlpha.u8Alpha0,
        pstCursorAttr->stAlpha.u8Alpha1, pstCursorAttr->stAlpha.u8GlobalAlpha, pstCursorAttr->stAlpha.u16PlaneAlpha,
        pstCursorAttr->stColorKey.bKeyEnable, pstCursorAttr->stColorKey.u8Red, pstCursorAttr->stColorKey.u8Green,
        pstCursorAttr->stColorKey.u8Blue, pstCursorAttr->bShown, pstCursorAttr->stCursorImageInfo.u32Width,
        pstCursorAttr->stCursorImageInfo.u32Height, pstCursorAttr->stCursorImageInfo.u32Pitch,
        pstCursorAttr->stCursorImageInfo.eColorFmt, pstCursorAttr->stCursorImageInfo.data,
        pstCursorAttr->u16CursorAttrMask);
}

static int _MDRV_FB_SetPar(struct fb_info *info)
{
    MI_U8                 u8FbId = info->node - g_u32FirstFbNode;
    MI_FB_VarScreenInfo_t stVarScreenInfo;
    MI_FB_FixScreenInfo_t stFixScreenInfo;

    memset(&stVarScreenInfo, 0, sizeof(MI_FB_VarScreenInfo_t));
    memset(&stFixScreenInfo, 0, sizeof(MI_FB_FixScreenInfo_t));
    _MDRV_FB_RecvVarScreenInfo(&stVarScreenInfo, &info->var);

    if (MI_SUCCESS != MI_FB_SetVarScreenInfo(u8FbId, &stVarScreenInfo))
    {
        DBG_ERR("[fb:%d] set var info failed\n", u8FbId);
        return -EINVAL;
    }

    if (MI_SUCCESS != MI_FB_GetVarScreenInfo(u8FbId, &stVarScreenInfo)
        || MI_SUCCESS != MI_FB_GetFixScreenInfo(u8FbId, &stFixScreenInfo))
    {
        DBG_ERR("[fb:%d] get screen info failed\n", u8FbId);
        return -EINVAL;
    }
    _MDRV_FB_TranVarScreenInfo(&info->var, &stVarScreenInfo);
    _MDRV_FB_TranFixScreenInfo(&info->fix, &stFixScreenInfo);

    return 0;
}

static int _MDRV_FB_PanDisplay(struct fb_var_screeninfo *var, struct fb_info *info)
{
    MI_U8                 u8FbId = info->node - g_u32FirstFbNode;
    MI_FB_VarScreenInfo_t stVarScreenInfo;

    memset(&stVarScreenInfo, 0, sizeof(MI_FB_VarScreenInfo_t));
    _MDRV_FB_RecvVarScreenInfo(&stVarScreenInfo, var);

    DBG_INFO(
        "[fb:%d] pan display var info => (xres:%d,yres:%d,xvir:%d,yvir:%d,xoffset:%d,yoffset:%d,bits_per_pixel:%d)\n",
        u8FbId, stVarScreenInfo.u32Xres, stVarScreenInfo.u32Yres, stVarScreenInfo.u32Xres_virtual,
        stVarScreenInfo.u32Yres_virtual, stVarScreenInfo.u32Xoffset, stVarScreenInfo.u32Yoffset,
        stVarScreenInfo.u32BitsPerPixel);
    if (MI_SUCCESS != MI_FB_PanDisplay(u8FbId, &stVarScreenInfo))
    {
        DBG_ERR("[fb:%d] pan display failed\n", u8FbId);
        return -EINVAL;
    }

    return 0;
}

static int _MDRV_FB_CheckVar(struct fb_var_screeninfo *var, struct fb_info *info)
{
    UNUSED(var);
    UNUSED(info);
    return 0;
}

static int _MDRV_FB_Blank(int blank, struct fb_info *info)
{
    UNUSED(blank);
    UNUSED(info);
    return 0;
}

static int _MDRV_FB_SetColReg(unsigned regno, unsigned red, unsigned green, unsigned blue, unsigned transp,
                              struct fb_info *info)
{
    UNUSED(regno);
    UNUSED(red);
    UNUSED(green);
    UNUSED(blue);
    UNUSED(transp);
    UNUSED(info);
    return 0;
}

static int _MDRV_FB_SetCmap(struct fb_cmap *cmap, struct fb_info *info)
{
    MI_U8        u8FbId = info->node - g_u32FirstFbNode;
    MI_FB_Cmap_t stCmap;

    stCmap.u32Start   = cmap->start;
    stCmap.u32Len     = cmap->len;
    stCmap.pu16Red    = cmap->red;
    stCmap.pu16Green  = cmap->green;
    stCmap.pu16Blue   = cmap->blue;
    stCmap.pu16Transp = cmap->transp;

    DBG_INFO("[fb:%d] cmap:(start:%d, len:%d, r:%p, g:%p, b:%p, transp:%p)\n", u8FbId, stCmap.u32Start, stCmap.u32Len,
             stCmap.pu16Red, stCmap.pu16Green, stCmap.pu16Blue, stCmap.pu16Transp);
    if (MI_SUCCESS != MI_FB_SetCmap(u8FbId, &stCmap))
    {
        DBG_ERR("[fb:%d] set cmap failed\n", u8FbId);
        return -EINVAL;
    }
    return 0;
}

static int _MDRV_FB_Mmap(struct fb_info *info, struct vm_area_struct *vma)
{
    size_t size = 0;

    if (NULL == info)
    {
        DBG_ERR("info is NULL pointer !\n");
        return -ENOTTY;
    }

    if (NULL == vma)
    {
        DBG_ERR("vma is NULL pointer !\n");
        return -ENOTTY;
    }

    if (0 == info->fix.smem_start)
    {
        DBG_ERR("physical addr is NULL pointer !\n");
        return -ENOTTY;
    }

    size = vma->vm_end - vma->vm_start;

    // Do not known what's mean, comment but keep this statement.
    // vma->vm_pgoff = FBDEV_MIU_TO_CPU_BUS_ADDR(info->fix.smem_start) >> PAGE_SHIFT;

    vma->vm_pgoff = FBDEV_MIU_TO_CPU_BUS_ADDR(info->fix.smem_start) >> PAGE_SHIFT;
    // set page to no cache
#if defined(CONFIG_MIPS)
    {
        pgprot_val(vma->vm_page_prot) &= ~_CACHE_MASK;
        pgprot_val(vma->vm_page_prot) |= _CACHE_UNCACHED;
    }
#elif defined(CONFIG_ARM) || defined(CONFIG_ARM64)
    {
        vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
    }
#endif
    // Remap-pfn-range will mark the range VM_IO and VM_RESERVED
    if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, size, vma->vm_page_prot))
    {
        return -EAGAIN;
    }
    DBG_INFO("_MDRV_FB_Mmap vma->vm_start=%x\n vma->vm_end=%x\n vma->vm_pgoff =%x\n", (unsigned int)vma->vm_start,
             (unsigned int)vma->vm_end, (unsigned int)vma->vm_pgoff);

    return 0;
}
static int _MDRV_FB_Open(struct fb_info *info, int user)
{
    int   retval = 0;
    MI_U8 u8FbId = info->node - g_u32FirstFbNode;
    UNUSED(user);

    retval = MI_FB_Open(u8FbId);
    if (!retval)
    {
        MI_FB_VarScreenInfo_t stVarScreenInfo;
        MI_FB_FixScreenInfo_t stFixScreenInfo;

        memset(&stVarScreenInfo, 0, sizeof(MI_FB_VarScreenInfo_t));
        memset(&stFixScreenInfo, 0, sizeof(MI_FB_FixScreenInfo_t));

        if (MI_FB_GetVarScreenInfo(u8FbId, &stVarScreenInfo) == MI_SUCCESS
            && MI_FB_GetFixScreenInfo(u8FbId, &stFixScreenInfo) == MI_SUCCESS)
        {
            _MDRV_FB_TranVarScreenInfo(&info->var, &stVarScreenInfo);
            _MDRV_FB_TranFixScreenInfo(&info->fix, &stFixScreenInfo);
            if (info->fix.smem_start && info->fix.smem_len && !info->screen_base)
            {
                MI_U32 u32MemLen     = info->fix.smem_len;
                MI_PHY phyMiuBusAddr = CamOsMemMiuToPhys(info->fix.smem_start);
                info->screen_base    = CamOsMemMap(phyMiuBusAddr, CAM_OS_ALIGN_UP(u32MemLen, PAGE_SIZE), FALSE);
            }
        }
        else
        {
            DBG_ERR("[fb:%d] get screen info failed\n", u8FbId);
        }
        DBG_INFO("[fb:%d] open success\n", u8FbId);
    }

    return retval;
}
static int _MDRV_FB_Release(struct fb_info *info, int user)
{
    int   retval = 0;
    MI_U8 u8FbId = info->node - g_u32FirstFbNode;
    UNUSED(user);

    retval = MI_FB_Close(u8FbId);
    if (!retval)
    {
        DBG_INFO("[fb:%d] release success\n", u8FbId);
    }

    return retval;
}
static void _MDRV_FB_Destroy(struct fb_info *info)
{
    MI_U8 u8FbId = info->node - g_u32FirstFbNode;
    if (info->screen_base)
    {
        CamOsMemUnmap(info->screen_base, info->fix.smem_len);
        info->screen_base = NULL;
    }
    DBG_INFO("[fb:%d] destroy success\n", u8FbId);
}
static int _MDRV_FB_Ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg)
{
    int          retval = 0;
    unsigned int dir;
    MI_U8        u8FbId = info->node - g_u32FirstFbNode;

    union
    {
        MI_FB_Rectangle_t        stRecTangle;
        MI_FB_GlobalAlpha_t      stGlobalAlpha;
        MI_FB_ColorKey_t         stColorKey;
        MI_FB_DisplayLayerAttr_t stDisplayLayerAttr;
        MI_FB_CursorAttr_t       stCursorAttr;
        MI_FB_Compression_t      stCompression;
        MI_BOOL                  bShown;
    } data;

    memset(&data, 0, sizeof(data));
    if (_IOC_TYPE(cmd) != FB_IOC_MAGIC)
    {
        return -ENOTTY;
    }

    if (_IOC_SIZE(cmd) > sizeof(data))
    {
        return -EINVAL;
    }

    dir = _IOC_DIR(cmd);

    if ((dir & _IOC_WRITE) && cmd != FBIO_WAITFORVSYNC)
    {
        if (copy_from_user(&data, (void __user *)arg, _IOC_SIZE(cmd)))
        {
            return -EFAULT;
        }
    }

    switch (cmd)
    {
        case FBIO_WAITFORVSYNC:
        {
            if (MI_SUCCESS != MI_FB_WaitForVsync(u8FbId))
            {
                DBG_ERR("[fb:%d] wait for vsync failed\n", u8FbId);
                retval = -EINVAL;
            }
        }
        break;
        case FBIOGET_SCREEN_LOCATION:
        {
            if (MI_SUCCESS == MI_FB_GetScreenLocation(u8FbId, &data.stRecTangle))
            {
                DBG_INFO("[fb:%d] get screen location => (x:%d,y:%d,w:%d,h:%d)\n", u8FbId, data.stRecTangle.u16Xpos,
                         data.stRecTangle.u16Ypos, data.stRecTangle.u16Width, data.stRecTangle.u16Height);
                retval = copy_to_user((MI_FB_Rectangle_t __user *)arg, &data.stRecTangle, sizeof(MI_FB_Rectangle_t));
            }
            else
            {
                DBG_ERR("[fb:%d] get screen location failed\n]", u8FbId);
                retval = -EINVAL;
            }
        }
        break;
        case FBIOSET_SCREEN_LOCATION:
        {
            DBG_INFO("[fb:%d] set screen location => (x:%d,y:%d,w:%d,h:%d)\n", u8FbId, data.stRecTangle.u16Xpos,
                     data.stRecTangle.u16Ypos, data.stRecTangle.u16Width, data.stRecTangle.u16Height);
            if (MI_SUCCESS != MI_FB_SetScreenLocation(u8FbId, &data.stRecTangle))
            {
                DBG_ERR("[fb:%d] set screen location failed\n", u8FbId);
                retval = -EINVAL;
            }
        }
        break;
        case FBIOGET_SHOW:
        {
            if (MI_SUCCESS == MI_FB_GetShow(u8FbId, &data.bShown))
            {
                retval = copy_to_user((MI_BOOL __user *)arg, &data.bShown, sizeof(MI_BOOL));
            }
            else
            {
                DBG_ERR("[fb:%d] get show status failed\n", u8FbId);
                retval = -EINVAL;
            }
        }
        break;
        case FBIOSET_SHOW:
        {
            if (MI_SUCCESS != MI_FB_SetShow(u8FbId, data.bShown))
            {
                DBG_ERR("[fb:%d] set show status failed\n", u8FbId);
                retval = -EINVAL;
            }
        }
        break;
        case FBIOGET_GLOBAL_ALPHA:
        {
            if (MI_SUCCESS == MI_FB_GetAlpha(u8FbId, &data.stGlobalAlpha))
            {
                DBG_INFO("[fb:%d] get alpha => (mode:%d,alpha0:%d,alpha1:%d,galpha:%d,planealpha:%d)\n", u8FbId,
                         data.stGlobalAlpha.u8AlphaMode, data.stGlobalAlpha.u8Alpha0, data.stGlobalAlpha.u8Alpha1,
                         data.stGlobalAlpha.u8GlobalAlpha, data.stGlobalAlpha.u16PlaneAlpha);
                retval =
                    copy_to_user((MI_FB_GlobalAlpha_t __user *)arg, &data.stGlobalAlpha, sizeof(MI_FB_GlobalAlpha_t));
            }
            else
            {
                DBG_ERR("[fb:%d] get alpha failed\n", u8FbId);
                retval = -EINVAL;
            }
        }
        break;
        case FBIOSET_GLOBAL_ALPHA:
        {
            DBG_INFO("[fb:%d] set alpha => (mode:%d,alpha0:%d,alpha1:%d,galpha:%d,planealpha:%d)\n", u8FbId,
                     data.stGlobalAlpha.u8AlphaMode, data.stGlobalAlpha.u8Alpha0, data.stGlobalAlpha.u8Alpha1,
                     data.stGlobalAlpha.u8GlobalAlpha, data.stGlobalAlpha.u16PlaneAlpha);
            if (MI_SUCCESS != MI_FB_SetAlpha(u8FbId, &data.stGlobalAlpha))
            {
                DBG_ERR("[fb:%d] set alpha failed\n", u8FbId);
                retval = -EINVAL;
            }
        }
        break;
        case FBIOGET_COLORKEY:
        {
            if (MI_SUCCESS == MI_FB_GetColorKey(u8FbId, &data.stColorKey))
            {
                DBG_INFO("[fb:%d] get colorkey => (en:%d,r:%d,g:%d,b:%d)\n", u8FbId, data.stColorKey.bKeyEnable,
                         data.stColorKey.u8Red, data.stColorKey.u8Green, data.stColorKey.u8Blue);
                retval = copy_to_user((MI_FB_ColorKey_t __user *)arg, &data.stColorKey, sizeof(MI_FB_ColorKey_t));
            }
            else
            {
                DBG_ERR("[fb:%d] get color key failed\n", u8FbId);
                retval = -EINVAL;
            }
        }
        break;
        case FBIOSET_COLORKEY:
        {
            DBG_INFO("[fb:%d] set colorkey => (en:%d,r:%d,g:%d,b:%d)\n", u8FbId, data.stColorKey.bKeyEnable,
                     data.stColorKey.u8Red, data.stColorKey.u8Green, data.stColorKey.u8Blue);
            if (MI_SUCCESS != MI_FB_SetColorKey(u8FbId, &data.stColorKey))
            {
                DBG_ERR("[fb:%d] set color key failed\n", u8FbId);
                retval = -EINVAL;
            }
        }
        break;
        case FBIOGET_DISPLAYLAYER_ATTRIBUTES:
        {
            if (MI_SUCCESS == MI_FB_GetDisplayLayerAttr(u8FbId, &data.stDisplayLayerAttr))
            {
                _MDRV_FB_PrintDisplayAttrInfo(u8FbId, &data.stDisplayLayerAttr, FALSE);
                retval = copy_to_user((MI_FB_DisplayLayerAttr_t __user *)arg, &data.stDisplayLayerAttr,
                                      sizeof(MI_FB_DisplayLayerAttr_t));
            }
            else
            {
                DBG_ERR("[fb:%d] get display layer attr failed\n", u8FbId);
                retval = -EINVAL;
            }
        }
        break;
        case FBIOSET_DISPLAYLAYER_ATTRIBUTES:
        {
            _MDRV_FB_PrintDisplayAttrInfo(u8FbId, &data.stDisplayLayerAttr, TRUE);
            if (MI_SUCCESS == MI_FB_SetDisplayLayerAttr(u8FbId, &data.stDisplayLayerAttr))
            {
                if (data.stDisplayLayerAttr.u32SetAttrMask
                    & (E_MI_FB_DISPLAYLAYER_ATTR_MASK_BUFFER_SIZE | E_MI_FB_DISPLAYLAYER_ATTR_MASK_COLOR_FMB))
                {
                    MI_FB_VarScreenInfo_t stVarScreenInfo;
                    MI_FB_FixScreenInfo_t stFixScreenInfo;

                    memset(&stVarScreenInfo, 0, sizeof(MI_FB_VarScreenInfo_t));
                    memset(&stFixScreenInfo, 0, sizeof(MI_FB_FixScreenInfo_t));

                    if (MI_FB_GetVarScreenInfo(u8FbId, &stVarScreenInfo) == MI_SUCCESS
                        && MI_FB_GetFixScreenInfo(u8FbId, &stFixScreenInfo) == MI_SUCCESS)
                    {
                        _MDRV_FB_TranVarScreenInfo(&info->var, &stVarScreenInfo);
                        _MDRV_FB_TranFixScreenInfo(&info->fix, &stFixScreenInfo);
                    }
                    else
                    {
                        DBG_ERR("[fb:%d] get screen info failed\n", u8FbId);
                    }
                }
            }
            else
            {
                DBG_ERR("[fb:%d] set display layer attr failed\n", u8FbId);
                retval = -EINVAL;
            }
        }
        break;
        case FBIOGET_CURSOR_ATTRIBUTE:
        {
            if (MI_SUCCESS == MI_FB_GetCursorAttr(u8FbId, &data.stCursorAttr))
            {
                _MDRV_FB_PrintCursorAttrInfo(u8FbId, &data.stCursorAttr, FALSE);
                retval = copy_to_user((MI_FB_CursorAttr_t __user *)arg, &data.stCursorAttr, sizeof(MI_FB_CursorAttr_t));
            }
            else
            {
                DBG_ERR("[fb:%d] get cursor attr failed\n", u8FbId);
                retval = -EINVAL;
            }
        }
        break;
        case FBIOSET_CURSOR_ATTRIBUTE:
        {
            _MDRV_FB_PrintCursorAttrInfo(u8FbId, &data.stCursorAttr, TRUE);
            if (MI_SUCCESS != MI_FB_SetCursorAttr(u8FbId, &data.stCursorAttr))
            {
                DBG_ERR("[fb:%d] set cursor attr failed\n", u8FbId);
                retval = -EINVAL;
            }
        }
        break;
        case FBIOGET_COMPRESSIONINFO:
        {
            if (MI_SUCCESS == MI_FB_GetCompressionInfo(u8FbId, &data.stCompression))
            {
                DBG_INFO(
                    "[fb:%d] get compression info => ( en:%d, split:%d, colorTransform:%d, fmt:%d, w:%d, h%d )\n",
                    u8FbId, data.stCompression.bEnable, data.stCompression.bBlockSplit,
                    data.stCompression.bColorTransform, data.stCompression.eColorFmt, data.stCompression.u16Width,
                    data.stCompression.u16Height);
                retval =
                    copy_to_user((MI_FB_Compression_t __user *)arg, &data.stCursorAttr, sizeof(MI_FB_Compression_t));
            }
            else
            {
                DBG_ERR("[Fb:%d] get compression info failed\n", u8FbId);
                retval = -EINVAL;
            }
        }
        break;
        case FBIOSET_COMPRESSIONINFO:
        {
            DBG_INFO("[fb:%d] set compression info => ( en:%d, split:%d, colorTransform:%d, fmt:%d, w:%d, h%d )\n",
                       u8FbId, data.stCompression.bEnable, data.stCompression.bBlockSplit,
                       data.stCompression.bColorTransform, data.stCompression.eColorFmt, data.stCompression.u16Width,
                       data.stCompression.u16Height);
            if (MI_SUCCESS != MI_FB_SetCompressionInfo(u8FbId, &data.stCompression))
            {
                DBG_ERR("[Fb:%d] set compression info failed\n", u8FbId);
                retval = -EINVAL;
            }
        }
        break;
        default:
        {
            DBG_ERR("[fb:%d] invalid cmd:0x%x\n", u8FbId, cmd);
            retval = -EINVAL;
            break;
        }
    }

    return retval;
}

static void _MDRV_FB_FillRect(struct fb_info *info, const struct fb_fillrect *rect)
{
#ifdef CONFIG_FB_VIRTUAL
    sys_fillrect(info, rect);
#endif
}

static void _MDRV_FB_CopyArea(struct fb_info *info, const struct fb_copyarea *area)
{
#ifdef CONFIG_FB_VIRTUAL
    sys_copyarea(info, area);
#endif
}

static void _MDRV_FB_ImageBlit(struct fb_info *info, const struct fb_image *image)
{
#ifdef CONFIG_FB_VIRTUAL
    sys_imageblit(info, image);
#endif
}

static struct fb_ops g_stFbOps = {
    .owner           = THIS_MODULE,
    .fb_open         = _MDRV_FB_Open,
    .fb_release      = _MDRV_FB_Release,
    .fb_mmap         = _MDRV_FB_Mmap,
    .fb_set_par      = _MDRV_FB_SetPar,
    .fb_check_var    = _MDRV_FB_CheckVar,
    .fb_blank        = _MDRV_FB_Blank,
    .fb_pan_display  = _MDRV_FB_PanDisplay,
    .fb_setcolreg    = _MDRV_FB_SetColReg,
    .fb_setcmap      = _MDRV_FB_SetCmap,
    .fb_fillrect     = _MDRV_FB_FillRect,
    .fb_copyarea     = _MDRV_FB_CopyArea,
    .fb_imageblit    = _MDRV_FB_ImageBlit,
    .fb_destroy      = _MDRV_FB_Destroy,
    .fb_ioctl        = _MDRV_FB_Ioctl,
    .fb_compat_ioctl = _MDRV_FB_Ioctl
};

static void _MDRV_FB_ProcFileShow(CamProcSeqFile_t *m, void *pData)
{
    CamProcSeqPrintf(m, "debug_switch=%d\nPlease input `echo 1/0 > /proc/sstar_fb/debug_switch` to on/off debug.\n",
                     *((int *)pData));
}

static void _MDRV_FB_ProcDebugWrite(char __user *pBuf, int nLen, void *pData)
{
    int *debugSwitch = (int *)pData;
    UNUSED(nLen);

    if (pBuf == NULL)
    {
        printk("input buffer is NULL\n");
        return;
    }
    *debugSwitch = CamOsStrtoul(pBuf, NULL, 10);
    if (*debugSwitch > 1)
    {
        *debugSwitch = 1;
    }
}

int MDRV_FB_Probe(struct device *dev)
{
    int                    retval = 0;
    MI_U8                  u8FbId = 0;
    MI_U8                  i      = 0;
    MI_FB_VarScreenInfo_t  stVarScreenInfo;
    MI_FB_FixScreenInfo_t  stFixScreenInfo;
    struct proc_dir_entry *pstFbFile = NULL;

    memset(&stVarScreenInfo, 0, sizeof(MI_FB_VarScreenInfo_t));
    memset(&stFixScreenInfo, 0, sizeof(MI_FB_FixScreenInfo_t));

    if (MI_SUCCESS != MI_FB_Init())
    {
        DBG_ERR("mi_fb init failed\n");
        return -EINVAL;
    }

    for (u8FbId = 0; u8FbId < FB_NUM_MAX; u8FbId++)
    {
        if (MI_SUCCESS != MI_FB_GetVarScreenInfo(u8FbId, &stVarScreenInfo)
            || MI_SUCCESS != MI_FB_GetFixScreenInfo(u8FbId, &stFixScreenInfo))
        {
            if (u8FbId == 0)
            {
                retval = -EINVAL;
                DBG_ERR("no found fb ini param\n");
                goto get_screen_info_err;
            }
            break;
        }

        g_pastFbInfo[u8FbId] = framebuffer_alloc(0, dev);
        if (!g_pastFbInfo[u8FbId])
        {
            DBG_ERR("[fb:%d] alloc fb info failed\n", u8FbId);
            retval = -ENOMEM;
            goto alloc_fb_info_err;
        }

        g_pastFbInfo[u8FbId]->fbops = &g_stFbOps;
        g_pastFbInfo[u8FbId]->flags = FBINFO_FLAG_DEFAULT;
        _MDRV_FB_TranVarScreenInfo(&g_pastFbInfo[u8FbId]->var, &stVarScreenInfo);
        _MDRV_FB_TranFixScreenInfo(&g_pastFbInfo[u8FbId]->fix, &stFixScreenInfo);

        retval = fb_alloc_cmap(&g_pastFbInfo[u8FbId]->cmap, 256, 0);
        if (retval < 0)
        {
            DBG_ERR("fb alloc cmap failed\n");
            goto alloc_cmap_err;
        }

        retval = register_framebuffer(g_pastFbInfo[u8FbId]);
        if (retval < 0)
        {
            DBG_ERR("register framebuffer failed\n");
            goto register_framebuffer_err;
        }

        if (dev)
        {
            dev_set_drvdata(dev, g_pastFbInfo);
        }
        if (u8FbId == 0)
        {
            g_u32FirstFbNode = g_pastFbInfo[u8FbId]->node;
        }

        g_u32LayerCnt++;
    }

    if (gp_stFbDir == NULL)
    {
        gp_stFbDir = CamProcMkdir("sstar_fb", 0);
        if (gp_stFbDir == NULL)
        {
            DBG_ERR("Create proc dir \"sstar_fb\" failed\n");
            return -EPERM;
        }
        if (pstFbFile == NULL)
        {
            pstFbFile = CamProcCreate("debug_switch", gp_stFbDir, _MDRV_FB_ProcFileShow, _MDRV_FB_ProcDebugWrite,
                                      &g_debugSwitch);
            if (pstFbFile == NULL)
            {
                CamProcRemoveEntry("sstar_fb", NULL);
                DBG_ERR("Create proc file \"debug_switch\" failed\n");
                return -EPERM;
            }
        }
    }
    return 0;

get_screen_info_err:
    return retval;
register_framebuffer_err:
    u8FbId++;
alloc_cmap_err:
alloc_fb_info_err:
    for (i = 0; i < u8FbId; i++)
    {
        fb_dealloc_cmap(&g_pastFbInfo[i]->cmap);
    }
    for (i = 0; i < FB_NUM_MAX; i++)
    {
        if (g_pastFbInfo[i])
        {
            framebuffer_release(g_pastFbInfo[i]);
            g_pastFbInfo[i] = NULL;
        }
    }
    return retval;
}

int MDRV_FB_Remove(struct device *dev)
{
    MI_U8 u8FbId = 0;

    if (remove_proc_subtree("sstar_fb", NULL) != 0)
    {
        DBG_WRN("remove dir \"sstar_fb\" failed\n");
    }
    gp_stFbDir = NULL;

    for (u8FbId = 0; u8FbId < g_u32LayerCnt; u8FbId++)
    {
        unregister_framebuffer(g_pastFbInfo[u8FbId]);
        fb_dealloc_cmap(&g_pastFbInfo[u8FbId]->cmap);
        framebuffer_release(g_pastFbInfo[u8FbId]);
    }

    if (dev)
    {
        dev_set_drvdata(dev, NULL);
    }

    MI_FB_DeInit();

    return 0;
}

#define DEVICE_NAME "sstarfb"                          ///< The device will appear at /dev/sstarfb using this value
#define CLASS_NAME  "fbdev"                            ///< The device class -- this is a character device driver
static int                    g_majorNumber;           ///< Stores the device number -- determined automatically
static struct CamClass *         gp_sstarFbClass  = NULL; ///< The device-driver class struct pointer
static struct CamDevice *        gp_sstarFbDevice = NULL; ///< The device-driver device struct pointer
static struct file_operations g_stOps          = {

};

__attribute__((weak)) int fb__module_init(void);

__attribute__((weak)) void fb__module_exit(void);


static int __init _MDRV_FB_Init(void)
{
    int retval = 0;

    g_majorNumber = register_chrdev(0, DEVICE_NAME, &g_stOps);
    if (g_majorNumber < 0)
    {
        return g_majorNumber;
    }

    gp_sstarFbClass = CamClassCreate(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(gp_sstarFbClass))
    {
        DBG_ERR("Failed to create device class\n");
        retval = PTR_ERR(gp_sstarFbClass);
        goto create_class_failed;
    }
    gp_sstarFbDevice = CamDeviceCreate(gp_sstarFbClass, NULL, MKDEV(g_majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(gp_sstarFbDevice))
    {
        DBG_ERR("Failed to create the device\n");
        retval = PTR_ERR(gp_sstarFbDevice);
        goto create_dev_failed;
    }

    if (fb__module_init)
    {
        fb__module_init();
    }

    if (MDRV_FB_Probe(CamDeviceGetInternalDevice(gp_sstarFbDevice)))
    {
        DBG_ERR("Failed to probe fb dev\n");
        retval = -EFAULT;
        goto probe_failed;
    }
    CamOsPrintf("module [%s] init\n", "fb");
    return 0;

probe_failed:
    CamDeviceDestroy(gp_sstarFbClass, MKDEV(g_majorNumber, 0));
create_dev_failed:
    CamClassDestroy(gp_sstarFbClass);
create_class_failed:
    unregister_chrdev(g_majorNumber, DEVICE_NAME);
    return retval;
}

static void __exit _MDRV_FB_Exit(void)
{
    CamOsPrintf("module [%s] deinit\n", "fb");
    MDRV_FB_Remove(CamDeviceGetInternalDevice(gp_sstarFbDevice));
    if (fb__module_exit)
    {
        fb__module_exit();
    }
    CamDeviceDestroy(gp_sstarFbClass, MKDEV(g_majorNumber, 0));
    CamClassDestroy(gp_sstarFbClass);
    unregister_chrdev(g_majorNumber, DEVICE_NAME);
}

module_init(_MDRV_FB_Init);
module_exit(_MDRV_FB_Exit);
#ifdef NON_COMMERCIAL_FOR_INTERNAL_TEST_ONLY
MODULE_LICENSE("GPL v2");
#else
MODULE_LICENSE("Proprietary");
#endif

MODULE_DESCRIPTION("GRAPHIC ioctrl driver");
MODULE_AUTHOR("Sigmastar");
