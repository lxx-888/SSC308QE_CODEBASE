/*
 * cam_sysfs.h- Sigmastar
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

///////////////////////////////////////////////////////////////////////////////
/// @file      cam_sysfs.h
/// @brief     Cam sysfs Wrapper Header File for
///            1. RTK OS
///            3. Linux Kernel Space
///////////////////////////////////////////////////////////////////////////////

#ifndef __CAM_SYSFS_H__
#define __CAM_SYSFS_H__

#include <linux/module.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/kobject.h>

typedef unsigned char      u8;
typedef signed char        s8;
typedef unsigned short     u16;
typedef signed short       s16;
typedef unsigned int       u32;
typedef signed int         s32;
typedef unsigned long long u64;
typedef signed long long   s64;

void CamModulePlatformDriver(struct platform_driver);
int  CamOfPropertyReadU32Array(const struct device_node *np, const char *propname, u32 *out_values, size_t sz);
struct kobject *        CamKobjectCreateAndAdd(const char *name, struct kobject *parent);
void                    CamSysfsRemoveFiles(struct kobject *kobj, const struct attribute **attr);
void                    CamSysfsRemoveFile(struct kobject *kobj, const struct attribute *attr, const void *ns);
int                     CamSysfsCreateFiles(struct kobject *kobj, const struct attribute **ptr);
int __must_check        CamSysfsCreateFile(struct kobject *kobj, const struct attribute *attr);
int                     CamPlatformGetIrqByname(struct platform_device *dev, const char *name);
void                    CamPlatformDriverUnregister(struct platform_driver *drv);
int                     CamPlatformDriverRegister(struct platform_driver *drv);
struct platform_device *CamPlatformDeviceAlloc(const char *name, int id);
int                     CamPlatformDeviceAdd(struct platform_device *pdev);
void                    CamPlatformDevicePut(struct platform_device *pdev);
int                     CamPlatformDeviceRegister(struct platform_device *pdev);
void                    CamPlatformDeviceUnregister(struct platform_device *pdev);
struct resource *       CamPlatformGetResource(struct platform_device *dev, unsigned int type, unsigned int num);
int                     CamOfAddressToResource(struct device_node *dev, int index, struct resource *r);
int                     CamOfIrqToResource(struct device_node *dev, int index, struct resource *r);
int          CamOfPropertyReadU32Index(const struct device_node *np, const char *propname, u32 index, u32 *out_value);
int          CamOfPropertyReadVariableU32Array(const struct device_node *np, const char *propname, u32 *out_values,
                                               size_t sz_min, size_t sz_max);
int          CamofPropertyReadU32(const struct device_node *np, const char *propname, u32 *out_value);
int          CamIoremapPageRange(unsigned long addr, unsigned long end, phys_addr_t phys_addr, pgprot_t prot);
void         CamDevmKfree(struct device *dev, void *p);
void *       CamDevmKmalloc(struct device *dev, size_t size, gfp_t gfp);
int          CamGpioRequest(unsigned gpio, const char *label);
unsigned int CamIrqOfParseAndMap(struct device_node *dev, int index);
struct workqueue_struct *CamCreatesiglethreadWorkqueue(const char *fmt);
int                      CamGetUserPagesFast(unsigned long start, int nr_pages, int write, struct page **pages);
int                      CamSprintSymbol(char *buffer, unsigned long address, bool offset);
void                     CamVmUnmapAliases(void);
int                      CamRegisterPmNotifier(struct notifier_block *nb);
int                      CamUnRegisterPmNotifier(struct notifier_block *nb);
int                      CamTryToFreeze(void);
#endif /* __CAM_SYSFS_H__ */
