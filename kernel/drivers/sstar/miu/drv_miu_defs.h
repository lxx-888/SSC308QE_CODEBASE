/*
 * drv_miu_defs.h - Sigmastar
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

#ifndef __DRV_MIU_DEFS_H__
#define __DRV_MIU_DEFS_H__

#include <cam_os_wrapper.h>
#if defined(CAM_OS_LINUX_KERNEL)
#include <linux/platform_device.h>
#endif
#include <drv_miu.h>
#include <hal_miu_if.h>

#if defined(CAM_OS_LINUX_KERNEL)
#define MIU_EXPORT_SYMBOL(func) EXPORT_SYMBOL(func)
#elif defined(CAM_OS_RTK)
#define MIU_EXPORT_SYMBOL(func)
#endif

#define sstar_miu_err(fmt, args...)  CamOsPrintf("[MIU ERR] [%s@%d] " fmt, __FUNCTION__, __LINE__, ##args)
#define sstar_miu_info(fmt, args...) CamOsPrintf(fmt, ##args)
#define sstar_miu_run_init(init, ret) \
    ret = init();                     \
    if (ret < 0)                      \
    {                                 \
        return ret;                   \
    }

struct miu_object_init
{
    char*          name;
    hal_miu_object obj;
    int (*init)(hal_miu_object object, struct hal_miu_client* client);
    int (*free)(hal_miu_object object);
};

struct miu_device
{
    int             index;
    CamOsSpinlock_t mdev_lock;
    bool            is_irq_init;

#if defined(CAM_OS_LINUX_KERNEL)
    struct bus_type     sub_sysfs;
    struct device       dev_sysfs;
    struct hal_miu_bist bist;
#endif
    struct hal_miu_bw bw;

    struct hal_miu_dram   dram;
    struct hal_miu_client client;

    bool                   protect_panic_on;
    u32                    protect_irq_num;
    u32                    protect_irq_status;
    struct hal_miu_protect protect;

#if defined(CONFIG_MIU_HW_MMU)
    CamOsSpinlock_t mmu_lock;
    u32             mmu_irq_num;
    u32             mmu_irq_status;
    void (*mmu_irq_callback)(u32, u16, u16, u8);
    struct hal_miu_mmu mmu;
#endif

    int                     obj_num;
    struct miu_object_init* obj_init;
};

struct miu_device* sstar_get_miu_device(void);
void               sstar_set_miu_device(struct miu_device* mdev);
#if defined(CAM_OS_LINUX_KERNEL)
struct miu_device* sstar_to_miu_device(struct device* dev);
#endif
int sstar_miu_sysfs_init(void);

#endif // #ifndef __DRV_MIU_DEFS_H__
