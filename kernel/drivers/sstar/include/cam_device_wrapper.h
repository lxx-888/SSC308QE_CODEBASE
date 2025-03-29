/*
 * cam_device_wrapper.h- Sigmastar
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

#ifndef __CAM_OS_DEVICE_H__
#define __CAM_OS_DEVICE_H__
#include "cam_os_wrapper.h"

typedef u32 dev_t;

typedef struct
{
    void *poll_table;
    void *data;
} CamOsPoll_t;

typedef struct CamOsVm
{
    void *vm;
} CamOsVm_t;

//////////////////////////////////////////
// FIXME: 后续需要把Wait接口移到CamOsWrapper中
typedef struct CamOsWait
{
    void *wait;
} CamOsWait_t;

#define CAM_OS_POLLIN     0x0001U
#define CAM_OS_POLLPRI    0x0002U
#define CAM_OS_POLLOUT    0x0004U
#define CAM_OS_POLLERR    0x0008U
#define CAM_OS_POLLHUP    0x0010U
#define CAM_OS_POLLNVAL   0x0020U
#define CAM_OS_POLLRDNORM 0x0040U
#define CAM_OS_POLLRDBAND 0x0080U
#define CAM_OS_POLLWRNORM 0x0100U

typedef int (*CamOsWaitCondFunc_t)(const void *param);
int  CamOsWaitInit(CamOsWait_t *wait);
void CamOsWaitDestroy(CamOsWait_t *wait);
void CamOsWaitWakeup(CamOsWait_t *wait);
int  CamOsWaitInterruptible(CamOsWait_t *wait, CamOsWaitCondFunc_t func, const void *param);
int  CamOsWaitUnInterruptible(CamOsWait_t *wait, CamOsWaitCondFunc_t func, const void *param);
int  CamOsWaitTimeoutInterruptible(CamOsWait_t *wait, CamOsWaitCondFunc_t func, const void *param, unsigned long ms);
int  CamOsWaitTimeoutUnInterruptible(CamOsWait_t *wait, CamOsWaitCondFunc_t func, const void *param, unsigned long ms);
//////////////////////////////////////////

// FIXME:
typedef struct CamFileops
{
    int (*open)(void *handle);
    int (*read)(char *buf, int size, long *offset, void *handle);
    int (*write)(const char *buf, int size, long *offset, void *handle);
    long (*llseek)(long offset, int whence, void *handle);
    int (*release)(void *handle);
    long (*unlocked_ioctl)(unsigned int cmd, unsigned long arg, void *handle);
    unsigned int (*poll)(CamOsPoll_t *camOsPoll, void *handle);
    int (*mmap)(void *vm, unsigned long start, unsigned long end, unsigned long vm_pgoff, void *handle);
#ifdef CONFIG_COMPAT
    long (*compat_ioctl)(unsigned int cmd, unsigned long arg, void *handle);
#endif
    void *ops_data;
} CamFileops_t;
typedef struct CamDevice CamDevice_t;
typedef struct CamClass  CamClass_t;
typedef struct CamOsPmops
{
    // Will fill ops, if it is needed
    // ....
    int (*pm_suspend)(CamDevice_t *dev);
    int (*pm_resume)(CamDevice_t *dev);
    int (*pm_suspend_late)(CamDevice_t *dev);
    int (*pm_resume_early)(CamDevice_t *dev);
    // ....
} CamOsPmops_t;

typedef struct CamClass
{
    void *class;
} CamClass_t;

typedef struct CamDevice
{
    void *device;
} CamDevice_t;

typedef void CamDeiveAttribute_t;
typedef void CamKObject_t;

struct module;
struct CamClass *__must_check CamClassCreate(struct module *owner, const char *name);
void                          CamClassDestroy(struct CamClass *cls);
void *                        CamClassGetInternalClass(struct CamClass *cls);
void                          CamClassRegistePmops(CamClass_t *ccls, struct CamOsPmops *pmops);

struct CamDevice *CamDeviceCreate(CamClass_t *class, struct CamDevice *parent, dev_t devt, void *drvdata,
                                  const char *fmt, ...);
void              CamDeviceDestroy(struct CamClass *class, dev_t devt);
void *            CamDeviceGetInternalDevice(struct CamDevice *dev);
void *            CamDeviceGetDrvData(struct CamDevice *dev);
const char *      CamDeviceName(struct CamDevice *dev);
int               CamDeviceCreateFile(struct CamDevice *dev, const CamDeiveAttribute_t *attr);
void              CamDeviceRemoveFile(struct CamDevice *dev, const CamDeiveAttribute_t *attr);
int               CamSysfsCreateLink(CamKObject_t *kobj, CamKObject_t *target, const char *name);
void              CamSysfsDeleteLink(CamKObject_t *kobj, CamKObject_t *target, const char *name);

void CamUnRegisterChrdev(unsigned int major, char *name);
int  CamRegisterChrdev(unsigned int major, unsigned int baseminor, unsigned int count, const char *name,
                       struct CamFileops *fops);

void         CamFileOpsSetPrivateDataByHandle(void *handle, void *data);
void *       CamFileOpsGetPrivateDataByHandle(void *handle);
unsigned int CamFileOpsGetFlagByHandle(void *handle);
unsigned int CamFileOpsGetMinorByHandle(void *handle);
int          CamFileOpsIsReadable(void *handle);
int          CamFileOpsIsWritable(void *handle);

// it's for poll function.in poll ops, call it to init wait,as poll_wait(kernel interface)
void CamOsPollWait(CamOsPoll_t *table, CamOsWait_t *wait);
#ifdef MODULE
extern struct module __this_module;
#define CAM_THIS_MODULE (&__this_module)
#else
#define CAM_THIS_MODULE ((struct module *)0)
#endif
#define CAM_DEVICE_MINORBITS 20
#define CAM_DEVICE_MINORMASK ((1U << CAM_DEVICE_MINORBITS) - 1)

#define CAM_DEVICE_MAJOR(dev)    ((unsigned int)((dev) >> CAM_DEVICE_MINORBITS))
#define CAM_DEVICE_MINOR(dev)    ((unsigned int)((dev)&CAM_DEVICE_MINORMASK))
#define CAM_DEVICE_MKDEV(ma, mi) (((ma) << CAM_DEVICE_MINORBITS) | (mi))

#endif // __CAM_OS_DEVICE_H__
