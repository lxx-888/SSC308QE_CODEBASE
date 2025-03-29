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
#include <linux/cdev.h>
#include "mi_sys.h"
#include "arena.h"
#include "ptree_log.h"
#include "ptree.h"
#ifdef USING_JSON
#include "ptree_db_json.h"
#endif
#include "ptree_kernel.h"
#include "ptree_preload.h"
#include "cam_sysfs.h"
#include "cam_device_wrapper.h"

static int         major_number;
static struct cdev ptree_cdev;

struct ptree_kernel
{
    void *arena;
    void *db;
    void *arena_pool;
    void *ptree_ins;
};

static int _ptree_open(struct inode *inode, struct file *file)
{
    PTREE_Config_t *config = NULL;

    config = kmalloc(sizeof(struct ptree_kernel), GFP_KERNEL);
    if (!config)
    {
        PTREE_ERR("kmalloc error!");
        return -1;
    }
    memset(config, 0, sizeof(struct ptree_kernel));
    file->private_data = (void *)config;
    return 0;
}

static int _ptree_release(struct inode *inode, struct file *file)
{
    struct ptree_kernel *config = (struct ptree_kernel *)file->private_data;

    if (config->arena)
    {
        if (config->db)
        {
            ARENA_Destroy(config->arena);
#ifdef USING_JSON
            /* Only support json in kernel mode ptree. */
            PTREE_DB_JSON_Deinit(config->db);
#endif
        }
        else
        {
            /* Using binary. */
            ARENA_Unmap(config->arena);
            kfree(config->arena_pool);
        }
    }
    kfree(file->private_data);
    file->private_data = NULL;
    return 0;
}

static long _ptree_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    PTREE_Config_t           ptree_config;
    struct ptree_kernel *    config = NULL;
    struct ptree_user_config user_config;
    int                      type = _IOC_TYPE(cmd);
#ifdef USING_JSON
    char *file_path = NULL;
#endif

    if (type != 'k')
    {
        PTREE_ERR("Type err, %d", type);
        return -1;
    }
    config = (struct ptree_kernel *)file->private_data;
    switch (cmd)
    {
        case PTREE_IOCTL_SET_CONFIG:
            if (!arg)
            {
                PTREE_ERR("Arg error");
                return -EFAULT;
            }
            if (copy_from_user(&user_config, (struct ptree_user_config __user *)arg, sizeof(struct ptree_user_config)))
            {
                PTREE_ERR("Copy from user error");
                return -EFAULT;
            }
            if (!user_config.config_data_size)
            {
                PTREE_ERR("Config data size error!");
                return -EFAULT;
            }
            if (config->arena)
            {
                PTREE_ERR("Duplicated set config of ptree.");
                return -EFAULT;
            }
            if (user_config.is_using_binary)
            {
                config->arena_pool = kmalloc(user_config.config_data_size, GFP_KERNEL);
                if (!config->arena_pool)
                {
                    PTREE_ERR("Arena pool buf error.");
                    return -EFAULT;
                }
                if (copy_from_user(config->arena_pool, (void __user *)user_config.config_data,
                                   user_config.config_data_size))
                {
                    kfree(config->arena_pool);
                    return -EFAULT;
                }

                PTREE_DBG("Using binary size %d", user_config.config_data_size);
                ARENA_Map(&config->arena, config->arena_pool, user_config.config_data_size);
                return 0;
            }
#ifdef USING_JSON
            file_path = (char *)kmalloc(user_config.config_data_size, GFP_KERNEL);
            if (!file_path)
            {
                PTREE_ERR("Kmalloc error.");
                return -EFAULT;
            }
            if (copy_from_user(file_path, (char __user *)user_config.config_data, user_config.config_data_size))
            {
                kfree(file_path);
                return -EFAULT;
            }
            file_path[user_config.config_data_size - 1] = '\0';
            config->db                                  = PTREE_DB_JSON_Init(file_path);
            kfree(file_path);
            if (ARENA_Create(&config->arena, 0x10000) == -1)
            {
                PTREE_ERR("Arena create fail!");
                return -EFAULT;
            }
#endif
            return 0;
        case PTREE_IOCTL_CONSTRUCT_PIPELINE:
            if (!config->arena)
            {
                PTREE_ERR("Need set config first!");
                return -EFAULT;
            }
            if (config->ptree_ins)
            {
                PTREE_ERR("Duplicated construct!");
                return -EFAULT;
            }
            ptree_config.pArenaHandle = config->arena;
            ptree_config.pDbInstance  = config->db;
            config->ptree_ins         = PTREE_CreateInstance(&ptree_config);
            if (config->ptree_ins)
            {
                return PTREE_ConstructPipeline(config->ptree_ins);
            }
            return -EFAULT;
        case PTREE_IOCTL_DESTRUCT_PIPELINE:
            if (!config->arena && config->ptree_ins)
                return -EFAULT;

            if (PTREE_DestructPipeline(config->ptree_ins) == -1)
                return -EFAULT;

            if (PTREE_DestroyInstance(config->ptree_ins) == -1)
                return -EFAULT;

            config->ptree_ins = NULL;
            return 0;
        case PTREE_IOCTL_START_PIPELINE:
            if (!config->arena && config->ptree_ins)
                return -EFAULT;

            return PTREE_StartPipeline(config->ptree_ins);
        case PTREE_IOCTL_STOP_PIPELINE:
            if (!config->arena && config->ptree_ins)
                return -EFAULT;

            return PTREE_StopPipeline(config->ptree_ins);
        default:
            PTREE_ERR("Cmd err, 0x%x", cmd);
            return -ENOTTY; // Unsupported command
    }
}

static struct file_operations ptree_fops = {
    .owner          = THIS_MODULE,
    .open           = _ptree_open,
    .release        = _ptree_release,
    .unlocked_ioctl = _ptree_ioctl,
    .compat_ioctl   = compat_ptr_ioctl,
};
struct CamClass * ptree_class = NULL;
static int __init ptree_device_init(void)
{
    ptree_class = CamClassCreate(THIS_MODULE, "ptree");
    if (!ptree_class)
    {
        PTREE_ERR("Failed to create class");
        return -1;
    }
    if (alloc_chrdev_region(&major_number, 0, 1, PTREE_DEVICE_NAME) < 0)
    {
        PTREE_ERR("Failed to allocate major number");
        return -1;
    }

    cdev_init(&ptree_cdev, &ptree_fops);
    if (cdev_add(&ptree_cdev, major_number, 1) < 0)
    {
        PTREE_ERR("Failed to add character device");
        goto device_init_error;
    }
    if (!CamDeviceCreate(ptree_class, NULL, major_number, NULL, PTREE_DEVICE_NAME))
    {
        PTREE_ERR("Failed to create device class");
        goto device_init_error;
    }
#ifdef INTERFACE_SYS
    MI_SYS_Init(0);
#endif
    PTREE_PRELOAD_Setup();
    PTREE_DBG("Ptree device driver loaded, ");
    return 0;

device_init_error:
    cdev_del(&ptree_cdev);
    unregister_chrdev_region(major_number, 1);
    CamClassDestroy(ptree_class);
    return -1;
}

static void __exit ptree_device_exit(void)
{
    PTREE_PRELOAD_CLear();
#ifdef INTERFACE_SYS
    MI_SYS_Exit(0);
#endif
    CamDeviceDestroy(ptree_class, major_number);
    cdev_del(&ptree_cdev);
    unregister_chrdev_region(major_number, 1);
    CamClassDestroy(ptree_class);
    PTREE_DBG("Ptree device driver unloaded");
}

module_init(ptree_device_init);
module_exit(ptree_device_exit);

MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("pedro.peng@sigmastar.com.cn");
MODULE_DESCRIPTION("Ptree kernel device");
