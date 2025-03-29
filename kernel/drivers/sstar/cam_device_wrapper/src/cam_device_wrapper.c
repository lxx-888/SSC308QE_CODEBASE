/*
 * cam_device_wrapper.c- Sigmastar
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

#include <linux/device.h>
#include <linux/module.h>
#include <cam_device_wrapper.h>
#include <linux/poll.h>

typedef struct CamOsFileopsInternalData
{
    struct CamOsListHead_t opsList;
    unsigned int           major;
    unsigned int           baseminor;
    unsigned int           count;
    void *                 private_data;
    CamFileops_t *         fileOps;
} CamOsFileopsInternalData_t;

typedef struct CamFileHandle
{
    struct file * file;
    CamFileops_t *fileOps;
    void *        private_data;
} CamFileHandle_t;

typedef struct CamClassInternal
{
    struct class internalClass;
    struct CamOsPmops *   pmops;
    struct lock_class_key __key;
    CamClass_t *          pCamClass;
} CamClassInternal_t;

typedef struct CamDeviceInternal
{
    struct device      internalDevice;
    struct CamOsPmops *pmops;
    CamDevice_t *      pCamDevice;
} CamDeviceInternal_t;

struct CamOsListHead_t g_fileOpsList;
CamOsTsem_t            g_fileOpsListSemlock;

int g_bEnableDebug = 0;
module_param(g_bEnableDebug, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

#define cam_dev_debug(fmt, args...)                                        \
    do                                                                     \
    {                                                                      \
        if (g_bEnableDebug)                                                \
        {                                                                  \
            printk("\033[1;31m[CAM DEV][%s][%d]", __FUNCTION__, __LINE__); \
            printk(KERN_CONT fmt, ##args);                                 \
            printk(KERN_CONT "\033[0m");                                   \
        }                                                                  \
    } while (0)

#define PRINT_FILENAME                                               \
    do                                                               \
    {                                                                \
        if (g_bEnableDebug)                                          \
        {                                                            \
            const char *filename = file->f_path.dentry->d_name.name; \
            cam_dev_debug("filename:%s.\n", filename);               \
        }                                                            \
    } while (0)

static int CamDeviceWrapper_pm_suspend(struct device *dev)
{
    int                 retval      = -1;
    struct class *      cls         = dev->class;
    CamClassInternal_t *clsInternal = CAM_OS_CONTAINER_OF(cls, CamClassInternal_t, internalClass);
    if (clsInternal->pmops && clsInternal->pmops->pm_suspend)
    {
        CamDeviceInternal_t *devInternal = CAM_OS_CONTAINER_OF(dev, CamDeviceInternal_t, internalDevice);
        retval                           = clsInternal->pmops->pm_suspend(devInternal->pCamDevice);
    }
    return retval;
}
static int CamDeviceWrapper_pm_resume(struct device *dev)
{
    int                 retval      = -1;
    struct class *      cls         = dev->class;
    CamClassInternal_t *clsInternal = CAM_OS_CONTAINER_OF(cls, CamClassInternal_t, internalClass);
    if (clsInternal->pmops && clsInternal->pmops->pm_resume)
    {
        CamDeviceInternal_t *devInternal = CAM_OS_CONTAINER_OF(dev, CamDeviceInternal_t, internalDevice);
        retval                           = clsInternal->pmops->pm_resume(devInternal->pCamDevice);
    }
    return retval;
}
static int CamDeviceWrapper_pm_suspend_late(struct device *dev)
{
    int                 retval      = -1;
    struct class *      cls         = dev->class;
    CamClassInternal_t *clsInternal = CAM_OS_CONTAINER_OF(cls, CamClassInternal_t, internalClass);
    if (clsInternal->pmops && clsInternal->pmops->pm_suspend_late)
    {
        CamDeviceInternal_t *devInternal = CAM_OS_CONTAINER_OF(dev, CamDeviceInternal_t, internalDevice);
        retval                           = clsInternal->pmops->pm_suspend_late(devInternal->pCamDevice);
    }
    return retval;
}
static int CamDeviceWrapper_pm_resume_early(struct device *dev)
{
    int                 retval      = -1;
    struct class *      cls         = dev->class;
    CamClassInternal_t *clsInternal = CAM_OS_CONTAINER_OF(cls, CamClassInternal_t, internalClass);
    if (clsInternal->pmops && clsInternal->pmops->pm_resume_early)
    {
        CamDeviceInternal_t *devInternal = CAM_OS_CONTAINER_OF(dev, CamDeviceInternal_t, internalDevice);
        retval                           = clsInternal->pmops->pm_resume_early(devInternal->pCamDevice);
    }
    return retval;
}

static struct dev_pm_ops g_CamDeviceWrapper_pm_ops = {
    .suspend      = CamDeviceWrapper_pm_suspend,
    .resume       = CamDeviceWrapper_pm_resume,
    .resume_early = CamDeviceWrapper_pm_resume_early,
    .suspend_late = CamDeviceWrapper_pm_suspend_late,
};

void CamClassRegistePmops(CamClass_t *ccls, struct CamOsPmops *pmops)
{
    // struct device* devc = (struct device*)dev->device;
    // devc->pm = &g_CamDeviceWrapper_pm_ops;
    ((CamClassInternal_t *)(ccls->class))->pmops            = pmops;
    ((CamClassInternal_t *)(ccls->class))->internalClass.pm = &g_CamDeviceWrapper_pm_ops;
    return;
}

// from kernel/drivers/base/core.c
static void device_create_release(struct device *dev)
{
    CamDeviceInternal_t *devInternal = CAM_OS_CONTAINER_OF(dev, CamDeviceInternal_t, internalDevice);
    CamDevice_t *        pCamDevice  = devInternal->pCamDevice;
    cam_dev_debug("device: '%s': %s\n", dev_name(dev), __func__);
    CamOsMemRelease(devInternal);
    CamOsMemRelease(pCamDevice);
}
static struct device *CamDevice_device_create_groups_vargs(struct device *dev, struct class *class,
                                                           struct device *parent, dev_t devt, void *drvdata,
                                                           const struct attribute_group **groups, const char *fmt,
                                                           va_list args)
{
    int retval = -ENODEV;

    if (class == NULL || IS_ERR(class))
        goto error;
    device_initialize(dev);
    dev->devt    = devt;
    dev->class   = class;
    dev->parent  = parent;
    dev->groups  = groups;
    dev->release = device_create_release;
    dev_set_drvdata(dev, drvdata);

    retval = kobject_set_name_vargs(&dev->kobj, fmt, args);
    if (retval)
        goto error;

    retval = device_add(dev);
    if (retval)
        goto error;

    return dev;

error:
    put_device(dev);
    return ERR_PTR(retval);
}

void *CamDeviceGetInternalDevice(struct CamDevice *dev)
{
    CamDeviceInternal_t *camDeviceInternal = (CamDeviceInternal_t *)dev->device;
    return &camDeviceInternal->internalDevice;
}

void *CamDeviceGetDrvData(struct CamDevice *dev)
{
    CamDeviceInternal_t *internalDevice = (CamDeviceInternal_t *)dev->device;
    struct device *      device         = &(internalDevice->internalDevice);
    return dev_get_drvdata(device);
}

struct CamDevice *CamDeviceCreate(CamClass_t *camClass, struct CamDevice *parent, dev_t devt, void *drvdata,
                                  const char *fmt, ...)
{
    va_list           vargs;
    struct CamDevice *dev;
    struct device *   _dev;
    void *            idev;

    dev = CamOsMemAlloc(sizeof(CamDevice_t));
    if (IS_ERR_OR_NULL(dev))
    {
        goto _out;
    }
    va_start(vargs, fmt);

    idev = dev->device = CamOsMemAlloc(sizeof(CamDeviceInternal_t));
    if (IS_ERR_OR_NULL(dev->device))
    {
        CamOsMemRelease(dev);
        dev = idev;
    }
    else
    {
        _dev                                               = &(((CamDeviceInternal_t *)(dev->device))->internalDevice);
        ((CamDeviceInternal_t *)(dev->device))->pCamDevice = dev;
        ((CamDeviceInternal_t *)(dev->device))->pmops      = NULL;
        CamDevice_device_create_groups_vargs(_dev, camClass->class, NULL, devt, drvdata, NULL, fmt, vargs);
    }
    va_end(vargs);
_out:
    return dev;
}

void CamDeviceDestroy(struct CamClass *class, dev_t devt)
{
    device_destroy(class->class, devt);
}

// From kernel/drivers/base/class.c
static void class_create_release(struct class *cls)
{
    CamClassInternal_t *clsInternal = CAM_OS_CONTAINER_OF(cls, CamClassInternal_t, internalClass);
    CamClass_t *        pCamClass   = clsInternal->pCamClass;
    cam_dev_debug("%s called for %s\n", __func__, cls->name);
    CamOsMemRelease(clsInternal);
    CamOsMemRelease(pCamClass);
}

// copy from class.c
int _CamClassInternalInit(CamClassInternal_t *clsInternal, struct module *owner, const char *name)
{
    int           retval = 0;
    struct class *cls    = &(clsInternal->internalClass);
    clsInternal->pmops   = NULL;
    cls->name            = name;
    cls->owner           = owner;
    cls->class_release   = class_create_release;
    lockdep_register_key(&clsInternal->__key);
    retval = __class_register(cls, &clsInternal->__key);
    if (retval)
    {
        cam_dev_debug("__class_register retval:%d.\n", retval);
    }
    return retval;
}

struct CamClass *__must_check CamClassCreate(struct module *owner, const char *name)
{
    int retval = 0;
    struct CamClass *class;
    void *iclass;
    // static struct lock_class_key __key;
    class = CamOsMemAlloc(sizeof(struct CamClass));

    if (IS_ERR_OR_NULL(class))
    {
        goto error_out3;
    }

    // class->class = class_create(owner, name);
    iclass = class->class = CamOsMemAlloc(sizeof(CamClassInternal_t));
    if (IS_ERR_OR_NULL(class->class))
    {
        goto error_out2;
    }
    ((CamClassInternal_t *)class->class)->pCamClass = class;
    retval = _CamClassInternalInit((CamClassInternal_t *)class->class, owner, name);
    if (retval)
    {
        goto error_out1;
    }
    return class;
error_out1:
    CamOsMemRelease(class->class);
    iclass = ERR_PTR(retval);
error_out2:
    CamOsMemRelease(class);
    class = iclass;
error_out3:
    return class;
}

void CamClassDestroy(struct CamClass *cls)
{
    class_destroy(cls->class);
}

void *CamClassGetInternalClass(struct CamClass *cls)
{
    CamClassInternal_t *camClassInternal = (CamClassInternal_t *)cls->class;
    return &(camClassInternal->internalClass);
}

const char *CamDeviceName(struct CamDevice *dev)
{
    CamDeviceInternal_t *internalDevice = (CamDeviceInternal_t *)dev->device;
    struct device *      device         = &(internalDevice->internalDevice);
    return dev_name(device);
}

int CamDeviceCreateFile(struct CamDevice *dev, const CamDeiveAttribute_t *attr)
{
    return device_create_file(CamDeviceGetInternalDevice(dev), (struct device_attribute *)attr);
}

void CamDeviceRemoveFile(struct CamDevice *dev, const CamDeiveAttribute_t *attr)
{
    device_remove_file(CamDeviceGetInternalDevice(dev), (struct device_attribute *)attr);
}

int CamSysfsCreateLink(CamKObject_t *kobj, CamKObject_t *target, const char *name)
{
    return sysfs_create_link((struct kobject *)kobj, (struct kobject *)target, name);
}

void CamSysfsDeleteLink(CamKObject_t *kobj, CamKObject_t *target, const char *name)
{
#if !IS_MODULE(CONFIG_CAM_DEVICE_WRAPPER)
    sysfs_delete_link((struct kobject *)kobj, (struct kobject *)target, name);
#else
    sysfs_remove_link((struct kobject *)kobj, name);
#endif
}

void CamFileOpsSetPrivateDataByHandle(void *handle, void *data)
{
    CamFileHandle_t *pHandle = (CamFileHandle_t *)handle;
    pHandle->private_data    = data;
}
void *CamFileOpsGetPrivateDataByHandle(void *handle)
{
    CamFileHandle_t *pHandle = (CamFileHandle_t *)handle;
    return pHandle->private_data;
}
unsigned int CamFileOpsGetFlagByHandle(void *handle)
{
    CamFileHandle_t *pHandle = (CamFileHandle_t *)handle;
    return pHandle->file->f_flags;
}
unsigned int CamFileOpsGetMinorByHandle(void *handle)
{
    CamFileHandle_t *pHandle = (CamFileHandle_t *)handle;
    return iminor(pHandle->file->f_inode);
}

int CamFileOpsIsReadable(void *handle)
{
    CamFileHandle_t *pHandle = (CamFileHandle_t *)handle;
    return (pHandle->file->f_flags & O_RDONLY || pHandle->file->f_flags & O_RDWR) ? 1 : 0;
}

int CamFileOpsIsWritable(void *handle)
{
    CamFileHandle_t *pHandle = (CamFileHandle_t *)handle;
    return (pHandle->file->f_flags & O_WRONLY || pHandle->file->f_flags & O_RDWR) ? 1 : 0;
}

static int _CamFileOpsopen(struct inode *inode, struct file *file)
{
    int                              retval           = -1;
    struct CamOsFileopsInternalData *pos              = NULL;
    struct CamOsFileopsInternalData *findInternalData = NULL;
    CamFileHandle_t *                pHandle          = NULL;
    int                              major            = imajor(inode);
    int                              minor            = iminor(inode);
    file->private_data                                = NULL;
    PRINT_FILENAME;
    CamOsTsemDown(&g_fileOpsListSemlock);
    CAM_OS_LIST_FOR_EACH_ENTRY(pos, &g_fileOpsList, opsList)
    {
        cam_dev_debug("loop major:%d, baseminor:%d, count:%d.\n", pos->major, pos->baseminor, pos->count);
        if (pos->major == major)
        {
            if (pos->baseminor <= minor && pos->baseminor + pos->count >= minor)
            {
                pHandle               = CamOsMemAlloc(sizeof(CamFileHandle_t));
                pHandle->file         = file;
                pHandle->fileOps      = pos->fileOps;
                pHandle->private_data = NULL;
                file->private_data    = pHandle;
                findInternalData      = pos;
                break;
            }
        }
    }
    CamOsTsemUp(&g_fileOpsListSemlock);
    cam_dev_debug("enter _CamFileOpsopen, arg major:%d, minor:%d.\n", major, minor);
    if (findInternalData && findInternalData->fileOps && findInternalData->fileOps->open)
    {
        cam_dev_debug("find pos->major:%d.\n", findInternalData->major);
        retval = findInternalData->fileOps->open((void *)pHandle);
    }
    return retval;
}

ssize_t _CamFileOpsRead(struct file *file, char __user *buf, size_t size, loff_t *offset)
{
    CamFileHandle_t *pHandle = file->private_data;
    CamFileops_t *   fileOps = pHandle->fileOps;
    int              ret     = -2;
    PRINT_FILENAME;
    if (fileOps && fileOps->read)
    {
        ret = fileOps->read(buf, size, (long *)offset, pHandle);
    }
    cam_dev_debug("_CamFileOpsRead...\n");
    return ret;
}

ssize_t _CamFileOpsWrite(struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
    int              ret     = -1;
    CamFileHandle_t *pHandle = file->private_data;
    CamFileops_t *   fileOps = pHandle->fileOps;
    PRINT_FILENAME;
    if (fileOps && fileOps->write)
    {
        ret = fileOps->write(buf, size, (long *)offset, pHandle);
    }
    cam_dev_debug("_CamFileOpsWrite....\n");
    return ret;
}
int _CamFileOpsRelease(struct inode *inode, struct file *file)
{
    int              ret     = -1;
    CamFileHandle_t *pHandle = file->private_data;
    CamFileops_t *   fileOps = pHandle->fileOps;
    PRINT_FILENAME;
    if (fileOps && fileOps->release)
    {
        ret = fileOps->release(pHandle);
    }
    CamOsMemRelease(pHandle);
    cam_dev_debug("_CamFileOpsRelease...\n");
    return ret;
}
long _CamFileOpsUnlocked_ioctl(struct file *file, unsigned int cmd, unsigned long ptr)
{
    long             ret     = -1;
    CamFileHandle_t *pHandle = file->private_data;
    CamFileops_t *   fileOps = pHandle->fileOps;
    PRINT_FILENAME;
    if (fileOps && fileOps->unlocked_ioctl)
    {
        ret = fileOps->unlocked_ioctl(cmd, ptr, pHandle);
    }
    cam_dev_debug("_CamFileOpsUnlocked_ioctl... ret:%lx, cmd:%x.\n", ret, cmd);
    return ret;
}
loff_t _CamFileOpsLlseek(struct file *file, loff_t offset, int whence)
{
    loff_t           ret     = -1;
    CamFileHandle_t *pHandle = file->private_data;
    CamFileops_t *   fileOps = pHandle->fileOps;
    PRINT_FILENAME;
    if (fileOps && fileOps->llseek)
    {
        ret = fileOps->llseek(offset, whence, pHandle);
    }
    if (fileOps && fileOps->llseek == NULL)
    {
        ret = noop_llseek(file, offset, whence);
    }
    cam_dev_debug("_CamFileOpsUnlocked_ioctl...\n");
    return ret;
}
static unsigned int _CamFileOpsPoll(struct file *file, struct poll_table_struct *table)
{
    unsigned int     ret = -1;
    unsigned int     req_events;
    CamFileHandle_t *pHandle = file->private_data;
    CamFileops_t *   fileOps = pHandle->fileOps;
    CamOsPoll_t      t;
    t.poll_table = table;
    t.data       = file;
    req_events   = poll_requested_events(table);
    PRINT_FILENAME;
    if (fileOps && fileOps->poll)
    {
        ret = fileOps->poll(&t, pHandle);
    }
    cam_dev_debug("_CamFileOpsPoll... ret:0x%x, req_events:0x%x.\n", ret, req_events);

    return req_events & ret;
}
static int _CamFileOpsMmap(struct file *file, struct vm_area_struct *vm)
{
    int              ret     = -1;
    CamFileHandle_t *pHandle = file->private_data;
    CamFileops_t *   fileOps = pHandle->fileOps;
    PRINT_FILENAME;
    if (fileOps && fileOps->mmap)
    {
        ret = fileOps->mmap((void *)vm, vm->vm_start, vm->vm_end, vm->vm_pgoff, pHandle);
    }
    cam_dev_debug("_CamFileOpsMmap...\n");
    return ret;
}
static struct file_operations g_CamFileops = {.owner          = THIS_MODULE,
                                              .open           = _CamFileOpsopen,
                                              .read           = _CamFileOpsRead,
                                              .write          = _CamFileOpsWrite,
                                              .release        = _CamFileOpsRelease,
                                              .unlocked_ioctl = _CamFileOpsUnlocked_ioctl,
                                              .llseek         = _CamFileOpsLlseek,
                                              .poll           = _CamFileOpsPoll,
                                              .mmap           = _CamFileOpsMmap};

void CamOsPollWait(CamOsPoll_t *table, CamOsWait_t *wait)
{
    if ((table != NULL) && (wait != NULL))
    {
        poll_wait((struct file *)table->data, (wait_queue_head_t *)(wait->wait), table->poll_table);
    }
    else
    {
        cam_dev_debug("table:%px, wait:%px.\n", table, wait);
    }
}
void CamUnRegisterChrdev(unsigned int major, char *name)
{
    CamOsFileopsInternalData_t *     FoundInternalData = NULL;
    struct CamOsFileopsInternalData *pos               = NULL;
    CamOsTsemDown(&g_fileOpsListSemlock);
    CAM_OS_LIST_FOR_EACH_ENTRY(pos, &g_fileOpsList, opsList)
    {
        if (pos->major == major)
        {
            FoundInternalData = pos;
            break;
        }
    }
    CamOsTsemUp(&g_fileOpsListSemlock);
    if (FoundInternalData)
    {
        __unregister_chrdev(FoundInternalData->major, FoundInternalData->baseminor, FoundInternalData->count, name);
    }
    CamOsTsemDown(&g_fileOpsListSemlock);
    CAM_OS_LIST_DEL(&FoundInternalData->opsList);
    CamOsTsemUp(&g_fileOpsListSemlock);
    CamOsMemRelease(FoundInternalData);
}
int CamRegisterChrdev(unsigned int major, unsigned int baseminor, unsigned int count, const char *name,
                      struct CamFileops *fops)
{
    int                              retval       = -1;
    CamOsFileopsInternalData_t *     internalData = NULL;
    struct CamOsFileopsInternalData *pos          = NULL;
    CamOsTsemDown(&g_fileOpsListSemlock);
    CAM_OS_LIST_FOR_EACH_ENTRY(pos, &g_fileOpsList, opsList)
    {
        if (pos->major == major)
        {
            if (baseminor >= pos->baseminor && baseminor <= pos->baseminor + pos->count)
            {
                return -EBUSY;
            }
        }
    }
    CamOsTsemUp(&g_fileOpsListSemlock);

    retval = __register_chrdev(major, baseminor, count, name, &g_CamFileops);
    if (retval < 0)
    {
        goto out;
    }
    internalData            = CamOsMemAlloc(sizeof(CamOsFileopsInternalData_t));
    fops->ops_data          = (void *)internalData;
    internalData->major     = retval;
    internalData->baseminor = baseminor;
    internalData->count     = count;
    internalData->fileOps   = fops;
    cam_dev_debug("internalData Addr:%px, fops Addr:%px, name:%s.\n", internalData, fops, name);
    CamOsTsemDown(&g_fileOpsListSemlock);
    CAM_OS_LIST_ADD_TAIL(&internalData->opsList, &g_fileOpsList);
    CAM_OS_LIST_FOR_EACH_ENTRY(pos, &g_fileOpsList, opsList)
    {
        cam_dev_debug("pos Addr:%px, major:%d, baseminor%d, fileOps Addr:%px.\n", pos, pos->major, pos->baseminor,
                      pos->fileOps);
    }
    CamOsTsemUp(&g_fileOpsListSemlock);
out:
    return retval;
}

static int __init _camDeviceInit(void)
{
    int ret = 0;
    CAM_OS_INIT_LIST_HEAD(&g_fileOpsList);
    CamOsTsemInit(&g_fileOpsListSemlock, 1);

    return ret;
}

static void __exit _camDeviceExit(void)
{
    CamOsTsemDeinit(&g_fileOpsListSemlock);
    if (CAM_OS_LIST_EMPTY(&g_fileOpsList))
    {
        CamOsPrintf("Warnning:g_fileOpsList is not empty!\n");
    }
}

module_init(_camDeviceInit);
module_exit(_camDeviceExit);
MODULE_LICENSE("GPL");

EXPORT_SYMBOL(CamClassDestroy);
EXPORT_SYMBOL(CamClassCreate);
EXPORT_SYMBOL(CamClassGetInternalClass);
EXPORT_SYMBOL(CamClassRegistePmops);
EXPORT_SYMBOL(CamDeviceCreate);
EXPORT_SYMBOL(CamDeviceGetInternalDevice);
EXPORT_SYMBOL(CamDeviceGetDrvData);
EXPORT_SYMBOL(CamDeviceDestroy);
EXPORT_SYMBOL(CamDeviceName);
EXPORT_SYMBOL(CamDeviceCreateFile);
EXPORT_SYMBOL(CamDeviceRemoveFile);
EXPORT_SYMBOL(CamSysfsCreateLink);
EXPORT_SYMBOL(CamSysfsDeleteLink);
EXPORT_SYMBOL(CamRegisterChrdev);
EXPORT_SYMBOL(CamUnRegisterChrdev);
EXPORT_SYMBOL(CamFileOpsSetPrivateDataByHandle);
EXPORT_SYMBOL(CamFileOpsGetPrivateDataByHandle);
EXPORT_SYMBOL(CamFileOpsGetFlagByHandle);
EXPORT_SYMBOL(CamFileOpsGetMinorByHandle);
EXPORT_SYMBOL(CamFileOpsIsReadable);
EXPORT_SYMBOL(CamFileOpsIsWritable);
EXPORT_SYMBOL(CamOsPollWait);

///////////////////////////////////////////////////////
// FIXME: 临时写出来为满足poll的需求, 拆分到另外一个文件里，与CamOsConditon_t合并
int CamOsWaitInit(CamOsWait_t *wait)
{
    wait_queue_head_t *wq = NULL;
    if (wait == NULL)
    {
        cam_dev_debug("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    wq = (wait_queue_head_t *)CamOsMemAlloc(sizeof(wait_queue_head_t));
    if (wq == NULL)
    {
        cam_dev_debug("%s - kmalloc error!\n", __FUNCTION__);
        return -1;
    }
    init_waitqueue_head(wq);
    wait->wait = wq;
    return 0;
}
EXPORT_SYMBOL(CamOsWaitInit);

void CamOsWaitDestroy(CamOsWait_t *wait)
{
    wait_queue_head_t *wq = NULL;

    wq = (wait_queue_head_t *)(wait->wait);
    if (wq == NULL)
    {
        cam_dev_debug("%s - wait->wait is NULL!\n", __FUNCTION__);
        return;
    }
    CamOsMemRelease(wq);
    wait->wait = NULL;
}
EXPORT_SYMBOL(CamOsWaitDestroy);

void CamOsWaitWakeup(CamOsWait_t *wait)
{
    wait_queue_head_t *wq = NULL;

    wq = (wait_queue_head_t *)(wait->wait);
    if (wq == NULL)
    {
        cam_dev_debug("%s - wait->wait is NULL!\n", __FUNCTION__);
        return;
    }
    wake_up_all(wq);
}
EXPORT_SYMBOL(CamOsWaitWakeup);

int CamOsWaitInterruptible(CamOsWait_t *wait, CamOsWaitCondFunc_t func, const void *param)
{
    wait_queue_head_t *wq = NULL;
    DEFINE_WAIT(__wait);
    long ret       = 0;
    int  condition = 0;

    if (wait == NULL)
    {
        cam_dev_debug("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }

    wq = (wait_queue_head_t *)(wait->wait);
    if (wq == NULL)
    {
        cam_dev_debug("%s - wait->wait is NULL!\n", __FUNCTION__);
        return -1;
    }
    prepare_to_wait(wq, &__wait, TASK_INTERRUPTIBLE);
    /* if wakeup the queue before prepare_to_wait, the func will return true. And will not go to schedule */
    if (func != NULL)
    {
        condition = func(param);
    }

    if (!condition)
    {
        if (!signal_pending(current))
        {
            schedule();
        }
        if (signal_pending(current))
        {
            ret = -ERESTARTSYS;
        }
    }

    finish_wait(wq, &__wait);
    return ret;
}
EXPORT_SYMBOL(CamOsWaitInterruptible);

int CamOsWaitUnInterruptible(CamOsWait_t *wait, CamOsWaitCondFunc_t func, const void *param)
{
    wait_queue_head_t *wq = NULL;
    DEFINE_WAIT(__wait);
    long ret       = 0;
    int  condition = 0;

    if (wait == NULL)
    {
        cam_dev_debug("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }

    wq = (wait_queue_head_t *)(wait->wait);
    if (wq == NULL)
    {
        cam_dev_debug("%s - wait->wait is NULL!\n", __FUNCTION__);
        return -1;
    }
    prepare_to_wait(wq, &__wait, TASK_UNINTERRUPTIBLE);
    /* if wakeup the queue before prepare_to_wait, the func will return true. And will not go to schedule */
    if (func != NULL)
    {
        condition = func(param);
    }

    if (!condition)
    {
        schedule();
    }

    finish_wait(wq, &__wait);
    return ret;
}
EXPORT_SYMBOL(CamOsWaitUnInterruptible);

int CamOsWaitTimeoutInterruptible(CamOsWait_t *wait, CamOsWaitCondFunc_t func, const void *param, unsigned long ms)
{
    wait_queue_head_t *wq = NULL;
    DEFINE_WAIT(__wait);
    long ret       = ms;
    int  condition = 0;

    if (wait == NULL)
    {
        cam_dev_debug("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }

    wq = (wait_queue_head_t *)(wait->wait);
    if (wq == NULL)
    {
        cam_dev_debug("%s - wait->wait is NULL!\n", __FUNCTION__);
        return -1;
    }
    prepare_to_wait(wq, &__wait, TASK_INTERRUPTIBLE);
    /* if wakeup the queue before prepare_to_wait, the func will return true. And will not go to schedule */
    if (func != NULL)
    {
        condition = func(param);
    }

    if (!condition)
    {
        if (!signal_pending(current))
        {
            ret = schedule_timeout(msecs_to_jiffies(ret));
            ret = jiffies_to_msecs(ret);
        }
        if (signal_pending(current))
        {
            ret = -ERESTARTSYS;
        }
    }

    finish_wait(wq, &__wait);

    return ret;
}

EXPORT_SYMBOL(CamOsWaitTimeoutInterruptible);

int CamOsWaitTimeoutUnInterruptible(CamOsWait_t *wait, CamOsWaitCondFunc_t func, const void *param, unsigned long ms)
{
    wait_queue_head_t *wq = NULL;
    DEFINE_WAIT(__wait);
    long ret       = ms;
    int  condition = 0;

    if (wait == NULL)
    {
        cam_dev_debug("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }

    wq = (wait_queue_head_t *)(wait->wait);
    if (wq == NULL)
    {
        cam_dev_debug("%s - wait->wait is NULL!\n", __FUNCTION__);
        return -1;
    }
    prepare_to_wait(wq, &__wait, TASK_UNINTERRUPTIBLE);
    /* if wakeup the queue before prepare_to_wait, the func will return true. And will not go to schedule */
    if (func != NULL)
    {
        condition = func(param);
    }

    if (!condition)
    {
        ret = schedule_timeout(msecs_to_jiffies(ret));
        ret = jiffies_to_msecs(ret);
    }

    finish_wait(wq, &__wait);

    return ret;
}
EXPORT_SYMBOL(CamOsWaitTimeoutUnInterruptible);
