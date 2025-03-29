/*
 * gyro_manager.c - Sigmastar
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
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/errno.h>
#include <linux/moduleparam.h>

#include "gyro_core.h"
#include "gyro_internal.h"

#define GET_SENSOR_NODE(list) (list.next)
#define DEVID_PROPNAME        "devid"

typedef struct gyro_manager_s
{
    bool       bInited;
    gyro_res_t transfer;
    gyro_res_t sensor;
    gyro_res_t dev;
} gyro_manager_t;

gyro_manager_t g_gyro_manager = {
    .bInited = false,
    .transfer =
        {
            .eType   = E_TRANSFER_RES,
            .nodenum = 0,
        },
    .sensor =
        {
            .eType   = E_SENSOR_RES,
            .nodenum = 0,
        },
    .dev =
        {
            .eType   = E_DEV_RES,
            .nodenum = 0,
        },
};

__attribute__((weak)) gyro_transfer_context_t *TRANSFER_CONTEXT_POINTERNAME(I2C_TRANSFER);
__attribute__((weak)) gyro_transfer_context_t *TRANSFER_CONTEXT_POINTERNAME(SPI_TRANSFER);
__attribute__((weak)) gyro_sensor_context_t *  SENSOR_CONTEXT_POINTERNAME(SENSOR_TYPE);

gyro_transfer_context_t **gpp_gyro_trans_context[MAX_TRANSFER]    = {&TRANSFER_CONTEXT_POINTERNAME(I2C_TRANSFER),
                                                                  &TRANSFER_CONTEXT_POINTERNAME(SPI_TRANSFER)};
gyro_sensor_context_t **  gpp_gyro_sensor_context[MAX_SENSOR_NUM] = {&SENSOR_CONTEXT_POINTERNAME(SENSOR_TYPE)};
static struct gyro_dev *  gp_gyrodev_slot[MAX_SUPPORT_DEV_NUM]    = {};

gyro_res_t *get_gyro_reslist(RES_TYPE_E eType)
{
    gyro_res_t *p_res = NULL;

    switch (eType)
    {
        case E_DEV_RES:
            p_res = &g_gyro_manager.dev;
            break;
        case E_SENSOR_RES:
            p_res = &g_gyro_manager.sensor;
            break;
        case E_TRANSFER_RES:
            p_res = &g_gyro_manager.transfer;
            break;
        default:
            p_res = NULL;
            break;
    }

    return p_res;
}

static inline int _init_sensor_list(gyro_res_t *p_res)
{
    unsigned char          u8Index   = 0;
    gyro_sensor_context_t *p_context = *gpp_gyro_sensor_context[SENSOR_TYPE];

    BUG_ON(!p_res);
    BUG_ON(p_res->eType != E_SENSOR_RES);

    TRACE_POINT;

    if (unlikely(!p_context))
    {
        GYRO_ERR("No specific supported sensor!!!");
        return -ENODEV;
    }

    init_rwsem(&p_res->rwsem);
    INIT_LIST_HEAD(&p_res->list);
    W_LOCK_LIST(p_res);
    do
    {
        p_context = *gpp_gyro_sensor_context[u8Index];
        if (p_context)
        {
            list_add(&p_context->list_head, &p_res->list);
            ++p_res->nodenum;
        }
        u8Index++;
    } while (u8Index < MAX_SENSOR_NUM);
    W_UNLOCK_LIST(p_res);

    return GYRO_SUCCESS;
}

static inline void _deinit_sensor_list(gyro_res_t *p_res)
{
    gyro_sensor_context_t *p_context = NULL;
    gyro_sensor_context_t *p_pos     = NULL;

    BUG_ON(!p_res);
    BUG_ON(p_res->eType != E_SENSOR_RES);

    TRACE_POINT;
    W_LOCK_LIST(p_res);
    list_for_each_entry_safe(p_context, p_pos, &p_res->list, list_head)
    {
        list_del(&p_context->list_head);
        --p_res->nodenum;
    }
    W_UNLOCK_LIST(p_res);
}

static inline int _init_transfer_list(gyro_res_t *p_res)
{
    unsigned char            u8Index   = 0;
    gyro_transfer_context_t *p_context = NULL;
    gyro_transfer_context_t *p_pos     = NULL;

    TRACE_POINT;
    BUG_ON(!p_res);
    BUG_ON(p_res->eType != E_TRANSFER_RES);

    init_rwsem(&p_res->rwsem);
    INIT_LIST_HEAD(&p_res->list);
    W_LOCK_LIST(p_res);
    do
    {
        p_context = *gpp_gyro_trans_context[u8Index];
        GYRO_DBG("transfer addr[%px]", p_context);
        if (p_context)
        {
            if (p_context->init() == GYRO_SUCCESS)
            {
                list_add(&p_context->list_head, &p_res->list);
                ++p_res->nodenum;
            }
            else
            {
                goto _release_res;
            }
        }
        u8Index++;
    } while (u8Index < MAX_TRANSFER);
    W_UNLOCK_LIST(p_res);

    return GYRO_SUCCESS;

_release_res:
    list_for_each_entry_safe(p_context, p_pos, &p_res->list, list_head)
    {
        list_del(&p_context->list_head);
        --p_res->nodenum;
        p_context->deinit();
    }
    W_UNLOCK_LIST(p_res);
    return -1;
}

static inline void _deinit_transfer_list(gyro_res_t *p_res)
{
    gyro_transfer_context_t *p_context = NULL;
    gyro_transfer_context_t *p_pos     = NULL;

    TRACE_POINT;
    BUG_ON(!p_res);
    BUG_ON(p_res->eType != E_TRANSFER_RES);

    W_LOCK_LIST(p_res);
    list_for_each_entry_safe(p_context, p_pos, &p_res->list, list_head)
    {
        list_del(&p_context->list_head);
        --p_res->nodenum;
        p_context->deinit();
    }
    W_UNLOCK_LIST(p_res);
}

static unsigned char _check_dev_support(struct device *p_device, bool checkInited)
{
    unsigned char u8_devid  = ERROR_DEV_NUM;
    u32           u32_devid = ERROR_DEV_NUM;
    int           ret       = GYRO_SUCCESS;

    // support the device which config by dts
    if (p_device->of_node)
    {
        ret = of_property_read_u32_index(p_device->of_node, DEVID_PROPNAME, 0, &u32_devid);
        if (ret != GYRO_SUCCESS)
        {
            GYRO_ERR("can't not get the %s entry from the %s device", DEVID_PROPNAME, p_device->of_node->name);
            return ERROR_DEV_NUM;
        }

        u8_devid = (unsigned char)u32_devid;
    }
    else // support the device which config by create_device function
    {
        u8_devid = (unsigned char)p_device->id;
    }

    GYRO_INFO("find the id[%u]", u8_devid);

    if (u8_devid >= MAX_SUPPORT_DEV_NUM)
    {
        GYRO_ERR("devid overflow the range[0 ~ %u]", (MAX_SUPPORT_DEV_NUM - 1));
        u8_devid = ERROR_DEV_NUM;
        goto _exit_func;
    }

    if (checkInited == true && gp_gyrodev_slot[u8_devid])
    {
        GYRO_ERR("dev[%u] has been initialised previously and cannot be initialised repeatedly with the same devid.",
                 u8_devid);
        u8_devid = ERROR_DEV_NUM;
    }

_exit_func:
    return u8_devid;
}

// need to lock the list res when call this function
static struct gyro_dev *_get_gyro_dev_nolock(struct device *p_device)
{
    unsigned char    u8_devid = 0;
    struct gyro_dev *p_dev    = NULL;
    struct gyro_dev *p_loc    = NULL;
    struct gyro_dev *p_pos    = NULL;

    if ((u8_devid = _check_dev_support(p_device, false)) >= ERROR_DEV_NUM)
    {
        return NULL;
    }

    // down_read(&g_gyro_manager.dev.rwsem);
    list_for_each_entry_safe(p_loc, p_pos, &g_gyro_manager.dev.list, list_head)
    {
        p_dev = (p_loc->u8_devid == u8_devid) ? p_loc : NULL;
    }
    // up_read(&g_gyro_manager.dev.rwsem);
    return p_dev;
}

inline int init_attach_pre_dev(struct device *p_device, gyro_transfer_context_t *p_transfer)
{
    unsigned char          u8_devid  = 0;
    struct gyro_dev *      p_gyrodev = NULL;
    gyro_sensor_context_t *p_sensor =
        container_of(GET_SENSOR_NODE(g_gyro_manager.sensor.list), gyro_sensor_context_t, list_head);

    TRACE_POINT;
    BUG_ON(p_device == NULL);
    BUG_ON(p_transfer == NULL);
    BUG_ON(p_sensor == NULL);

    if ((u8_devid = _check_dev_support(p_device, true)) >= ERROR_DEV_NUM)
    {
        return -1;
    }

    p_gyrodev = kmalloc(sizeof(struct gyro_dev), GFP_KERNEL);
    if (p_gyrodev == NULL)
    {
        GYRO_ERR("no mem for gyrodev structure");
        return -ENOMEM;
    }

    // init gyro_dev varible
    p_gyrodev->u8_devid             = u8_devid;
    p_gyrodev->reg_ops              = &p_transfer->ops;
    p_gyrodev->sensor_ops           = &p_sensor->ops;
    p_gyrodev->transfer_dev         = p_device;
    p_gyrodev->p_transfer_list_head = &p_transfer->list_head;
    mutex_init(&p_gyrodev->lock);
    atomic_set(&p_gyrodev->use_count, 0);

    // process attach
    if (p_transfer->attach_dev)
    {
        p_transfer->attach_dev(p_gyrodev);
    }

    // hw power on
    // p_gyrodev->sensor_ops->init(p_gyrodev);

    W_LOCK_LIST((&g_gyro_manager.dev));
    list_add_tail(&p_gyrodev->list_head, &g_gyro_manager.dev.list);
    ++g_gyro_manager.dev.nodenum;
    gp_gyrodev_slot[u8_devid] = p_gyrodev;
    W_UNLOCK_LIST((&g_gyro_manager.dev));

    return GYRO_SUCCESS;
}

inline int detect_deinit_pre_dev_bydev(struct device *p_device)
{
    gyro_transfer_context_t *p_transfer = NULL;
    struct gyro_dev *        p_dev      = NULL;

    TRACE_POINT;
    BUG_ON(p_device == NULL);

    W_LOCK_LIST((&g_gyro_manager.dev));
    p_dev = _get_gyro_dev_nolock(p_device);
    if (p_dev == NULL)
    {
        W_UNLOCK_LIST((&g_gyro_manager.dev));
        return -1;
    }

    list_del(&p_dev->list_head);
    --g_gyro_manager.dev.nodenum;
    gp_gyrodev_slot[p_dev->u8_devid] = NULL;
    W_UNLOCK_LIST((&g_gyro_manager.dev));

    LOCK_DEV(p_dev);
    p_transfer = container_of(p_dev->p_transfer_list_head, gyro_transfer_context_t, list_head);
    GYRO_DBG("transfer addr[%px]", p_transfer);

    if (p_transfer->detach_dev)
    {
        p_transfer->detach_dev(p_dev);
    }

    //  p_dev->sensor_ops->deinit(p_dev);

    UNLOCK_DEV(p_dev);
    kfree(p_dev);

    return GYRO_SUCCESS;
}

static inline void _detect_deinit_pre_dev_bylist(struct gyro_dev *p_dev)
{
    gyro_transfer_context_t *p_transfer = NULL;

    TRACE_POINT;
    BUG_ON(p_dev == NULL);

    W_LOCK_LIST((&g_gyro_manager.dev));
    list_del(&p_dev->list_head);
    --g_gyro_manager.dev.nodenum;
    gp_gyrodev_slot[p_dev->u8_devid] = NULL;
    W_UNLOCK_LIST((&g_gyro_manager.dev));

    LOCK_DEV(p_dev);
    p_transfer = container_of(p_dev->p_transfer_list_head, gyro_transfer_context_t, list_head);

    GYRO_DBG("transfer addr[%px]", p_transfer);
    // detect
    if (p_transfer->detach_dev)
    {
        p_transfer->detach_dev(p_dev);
    }

    // hw power off
    // p_dev->sensor_ops->deinit(p_dev);

    UNLOCK_DEV(p_dev);
    kfree(p_dev);
}

static inline int _init_dev_list(gyro_res_t *p_res)
{
    BUG_ON(!p_res);
    BUG_ON(p_res->eType != E_DEV_RES);

    TRACE_POINT;
    INIT_LIST_HEAD(&p_res->list);
    init_rwsem(&p_res->rwsem);

    return GYRO_SUCCESS;
}

static inline void _deinit_dev_list(gyro_res_t *p_res)
{
    struct gyro_dev *p_dev = NULL;
    struct gyro_dev *p_pos = NULL;

    BUG_ON(!p_res);
    BUG_ON(p_res->eType != E_DEV_RES);
    TRACE_POINT;
    // down_write(&p_res->rwsem);
    list_for_each_entry_safe(p_dev, p_pos, &p_res->list, list_head)
    {
        _detect_deinit_pre_dev_bylist(p_dev);
    }
    // up_write(&p_res->rwsem);
}

static int _early_init_pre_dev(gyro_res_t *p_res)
{
    int              ret   = GYRO_SUCCESS;
    struct gyro_dev *p_dev = NULL;
    struct gyro_dev *p_pos = NULL;

    PARAM_SAFE(p_res, NULL);
    CHECK_SUCCESS(p_res->eType == E_DEV_RES, return -EINVAL, "res type is not dev, type[%u]", p_res->eType);

    W_LOCK_LIST(p_res);
    list_for_each_entry_safe(p_dev, p_pos, &p_res->list, list_head)
    {
        // not need to lock dev
        if (p_dev->sensor_ops && p_dev->sensor_ops->early_init)
        {
            ret = p_dev->sensor_ops->early_init(p_dev);
        }

        if (ret != GYRO_SUCCESS)
        {
            goto _exit_unlock;
        }
    }

_exit_unlock:
    W_UNLOCK_LIST(p_res);
    return ret;
}

static int _final_deinit_pre_dev(gyro_res_t *p_res)
{
    int              ret   = GYRO_SUCCESS;
    struct gyro_dev *p_dev = NULL;
    struct gyro_dev *p_pos = NULL;

    PARAM_SAFE(p_res, NULL);
    CHECK_SUCCESS(p_res->eType == E_DEV_RES, return -EINVAL, "res type is not dev, type[%u]", p_res->eType);

    W_LOCK_LIST(p_res);
    list_for_each_entry_safe(p_dev, p_pos, &p_res->list, list_head)
    {
        // not need to lock dev
        if (p_dev->sensor_ops && p_dev->sensor_ops->final_deinit)
        {
            p_dev->sensor_ops->final_deinit(p_dev);
        }
    }
    W_UNLOCK_LIST(p_res);
    return ret;
}

int gyro_internal_init(void)
{
    int ret = GYRO_SUCCESS;

    ret = _init_sensor_list(&g_gyro_manager.sensor);
    if (ret != GYRO_SUCCESS)
    {
        ret = -1;
        goto _exit_func;
    }

    ret = _init_dev_list(&g_gyro_manager.dev);
    if (ret != GYRO_SUCCESS)
    {
        ret = -1;
        goto _deinit_sensor;
    }

    ret = _init_transfer_list(&g_gyro_manager.transfer);
    if (ret != GYRO_SUCCESS)
    {
        ret = -1;
        goto _deinit_dev;
    }

    ret = _early_init_pre_dev(&g_gyro_manager.dev);
    if (ret != GYRO_SUCCESS)
    {
        ret = -1;
        goto _deinit_transfer;
    }

    ret = gyro_core_init((gyro_handle *)&gp_gyrodev_slot, g_gyro_manager.dev.nodenum);
    if (ret != GYRO_SUCCESS)
    {
        ret = -1;
        goto _deinit_early;
    }

    g_gyro_manager.bInited = true;
    return GYRO_SUCCESS;

_deinit_early:
    _final_deinit_pre_dev(&g_gyro_manager.dev);
_deinit_transfer:
    _deinit_transfer_list(&g_gyro_manager.transfer);
_deinit_dev:
    _deinit_dev_list(&g_gyro_manager.dev);
_deinit_sensor:
    _deinit_sensor_list(&g_gyro_manager.sensor);
_exit_func:
    g_gyro_manager.bInited = false;
    return ret;
}

void gyro_internal_deinit(void)
{
    TRACE_POINT;
    g_gyro_manager.bInited = false;
    gyro_core_deinit();
    _final_deinit_pre_dev(&g_gyro_manager.dev);
    _deinit_transfer_list(&g_gyro_manager.transfer);
    _deinit_dev_list(&g_gyro_manager.dev);
    _deinit_sensor_list(&g_gyro_manager.sensor);
}

#ifdef CONFIG_SS_GYRO_DEBUG_ON

static unsigned char g_transferstr[MAX_TRANSFER][4] = {"I2C", "SPI"};
static void          __show_alltransfers(gyro_res_t *p_reslist)
{
    unsigned char            u8index   = 0;
    gyro_transfer_context_t *p_context = NULL;
    gyro_transfer_context_t *p_pos     = NULL;

    BUG_ON(p_reslist == NULL);
    BUG_ON(p_reslist->eType != E_TRANSFER_RES);

    pr_info("----------------------------- Start: Transfer Layer -----------------------------\n");
    R_LOCK_LIST(p_reslist);
    pr_info("\tsum of transfer type: %u\n", p_reslist->nodenum);
    pr_info("\tnodeID eType\n");
    list_for_each_entry_safe(p_context, p_pos, &p_reslist->list, list_head)
    {
        pr_info("\t%5u %5s\n", u8index++, g_transferstr[p_context->u8Type]);
    }
    R_UNLOCK_LIST(p_reslist);
    pr_info("----------------------------- End: Transfer Layer -----------------------------\n");
}

static void __show_allsensors(gyro_res_t *p_reslist)
{
    unsigned char          u8index   = 0;
    gyro_sensor_context_t *p_context = NULL;
    gyro_sensor_context_t *p_pos     = NULL;

    BUG_ON(p_reslist == NULL);
    BUG_ON(p_reslist->eType != E_SENSOR_RES);

    pr_info("----------------------------- Start: Sensor Layer -----------------------------\n");
    R_LOCK_LIST(p_reslist);
    pr_info("\tsum of Sensor type: %u\n", p_reslist->nodenum);
    pr_info("\tnodeID\n");
    list_for_each_entry_safe(p_context, p_pos, &p_reslist->list, list_head)
    {
        pr_info("\t%5u\n", u8index++);
    }
    R_UNLOCK_LIST(p_reslist);
    pr_info("----------------------------- End: Sensor Layer -----------------------------\n");
}

static void __show_alldevs(gyro_res_t *p_reslist)
{
    unsigned char            u8index    = 0;
    struct gyro_dev *        p_dev      = NULL;
    struct gyro_dev *        p_pos      = NULL;
    gyro_transfer_context_t *p_transfer = NULL;

    BUG_ON(p_reslist == NULL);
    BUG_ON(p_reslist->eType != E_DEV_RES);

    pr_info("----------------------------- Start: Dev Layer -----------------------------\n");
    R_LOCK_LIST(p_reslist);
    pr_info("\tsum of Device number: %u\n", p_reslist->nodenum);
    pr_info("\tnodeID devID useCnt transferType sensorNode\n");
    list_for_each_entry_safe(p_dev, p_pos, &p_reslist->list, list_head)
    {
        p_transfer = container_of(p_dev->p_transfer_list_head, gyro_transfer_context_t, list_head);
        pr_info("\t%5u %5u %6u %12s %10u\n", u8index++, p_dev->u8_devid, atomic_read(&p_dev->use_count),
                g_transferstr[p_transfer->u8Type], 0);
    }
    R_UNLOCK_LIST(p_reslist);
    pr_info("----------------------------- End: Dev Layer -----------------------------\n");
}

static void _show_allres(void)
{
    __show_alltransfers(&g_gyro_manager.transfer);
    __show_allsensors(&g_gyro_manager.sensor);
    __show_alldevs(&g_gyro_manager.dev);
}

static int set_param_str(const char *val, const struct kernel_param *kp)
{
    UNUSED(kp);
    _show_allres();
    printk("Show all resources whole log in kmsg\n");
    return 0;
}

static int get_param_str(char *buffer, const struct kernel_param *kp)
{
    UNUSED(kp);
    _show_allres();
    return scnprintf(buffer, PAGE_SIZE, "Show all resources whole log in kmsg\n");
}

static const struct kernel_param_ops param_ops_str = {
    .set = set_param_str,
    .get = get_param_str,
};

module_param_cb(show_allres, &param_ops_str, NULL, 0644);
MODULE_PARM_DESC(show_allres, "show all the resource on gyro_manager");
#endif