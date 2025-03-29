/*
 * gyro_internal.h- Sigmastar
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
#ifndef _GYRO_INTERNAL_H
#define _GYRO_INTERNAL_H

#include <linux/list.h>
#include <linux/rwsem.h>

#include "gyro.h"
#include "gyro_core.h"

#ifndef UNUSED
#define UNUSED(var) (void)((var) = (var))
#endif

#define TRACE_POINT   \
    do                \
    {                 \
        GYRO_TRACE(); \
    } while (0)

#define PARAM_SAFE_OPS(attr, ill_value, excp_ops) \
    CHECK_SUCCESS((attr != ill_value), excp_ops, #attr " parameter is illega")

#define PARAM_SAFE(attr, ill_value) CHECK_SUCCESS((attr != ill_value), return -1;, #attr " parameter is illega")

#define CHECK_SUCCESS(condition, excp_ops, err_log, ...) \
    do                                                   \
    {                                                    \
        if (unlikely(!(condition)))                      \
        {                                                \
            GYRO_ERR(err_log, ##__VA_ARGS__);            \
            excp_ops;                                    \
        }                                                \
    } while (0)

#define I2C_TRANSFER 0
#define SPI_TRANSFER 1
#define MAX_TRANSFER (SPI_TRANSFER + 1)

#define SENSOR_TYPE    0
#define MAX_SENSOR_NUM (SENSOR_TYPE + 1)

#define LIMIT_OF_MAX_SUPPORT_DEV_NUM 16
#define ERROR_DEV_NUM                (LIMIT_OF_MAX_SUPPORT_DEV_NUM + 1)
#define DEFAULT_SUPPORT_DEV_NUM      1 // only support one dev on souffle

#ifdef CONFIG_SUPPORT_MAXNUM_OF_GYRODEV
#define MAX_SUPPORT_DEV_NUM CONFIG_SUPPORT_MAXNUM_OF_GYRODEV
#else
#define MAX_SUPPORT_DEV_NUM DEFAULT_SUPPORT_DEV_NUM
#endif

#if (MAX_SUPPORT_DEV_NUM > LIMIT_OF_MAX_SUPPORT_DEV_NUM)
#error "max supporting devices number of 16"
#endif

#if 1
#define LOCK_DEV(pdev)   mutex_lock(&pdev->lock)
#define UNLOCK_DEV(pdev) mutex_unlock(&pdev->lock)
#else
#define LOCK_DEV(pdev)
#define UNLOCK_DEV(pdev)
#endif

#define W_LOCK_LIST(p_res)   down_write(&p_res->rwsem)
#define W_UNLOCK_LIST(p_res) up_write(&p_res->rwsem)

#define R_LOCK_LIST(p_res)   down_read(&p_res->rwsem)
#define R_UNLOCK_LIST(p_res) up_read(&p_res->rwsem)

typedef enum
{
    E_TRANSFER_RES = 0,
    E_SENSOR_RES,
    E_DEV_RES,
    E_MAX_RES,
} RES_TYPE_E;

typedef struct gyro_transfer_context_s
{
    unsigned char       u8Type;
    struct list_head    list_head;
    struct gyro_reg_ops ops;

    int (*init)(void);                        // add i2c/spi driver
    void (*deinit)(void);                     // deleta i2c/spi driver
    int (*attach_dev)(struct gyro_dev *dev);  // alloc gyro_dev mem and initi the varible, and enable hw on probe()
    void (*detach_dev)(struct gyro_dev *dev); // disable hw, release gyro_dev mem
} gyro_transfer_context_t;

typedef struct gyro_sensor_context_s
{
    struct list_head       list_head;
    struct gyro_sensor_ops ops;
} gyro_sensor_context_t;

typedef struct gyro_res_s
{
    RES_TYPE_E             eType;
    volatile unsigned char nodenum;
    struct rw_semaphore    rwsem;
    struct list_head       list;
} gyro_res_t;

extern gyro_transfer_context_t **g_gyro_trans_context[MAX_TRANSFER];
extern gyro_sensor_context_t **  g_gyro_sensor_context[MAX_SENSOR_NUM];

/*!
 * 1. At compile step, it does not support context definitions
 *    with different names for the same index twice.
 * 2. At compile step, it is not allowed to add more elements
 *    than the length of the array.
 */
#define SAFE_CHECK_ON_COMPILE(name, index, max) \
    void safe_##name##_##index##_##max(void)    \
    {                                           \
        char __dummy1[max - index - 1] = {};    \
        UNUSED(__dummy1[0]);                    \
    }

#define _TRANSFER_CONTEXT_POINTERNAME(index) p_gyro_##index##_transcontext
#define _SENSOR_CONTEXT_POINTERNAME(index)   p_gyro_##index##_sensorcontext

/* use on context */
#define TRANSFER_CONTEXT_POINTERNAME(index) _TRANSFER_CONTEXT_POINTERNAME(index)
#define SENSOR_CONTEXT_POINTERNAME(index)   _SENSOR_CONTEXT_POINTERNAME(index)

#define DEFINE_POINTEROF_TRANSFER_CONTEXT(index, context) \
    gyro_transfer_context_t *TRANSFER_CONTEXT_POINTERNAME(index) = &context;

#define DEFINE_POINTEROF_SENSOR_CONTEXT(index, context) \
    gyro_sensor_context_t *SENSOR_CONTEXT_POINTERNAME(index) = &context;

#define ADD_TRANSFER_CONTEXT(index, context)           \
    DEFINE_POINTEROF_TRANSFER_CONTEXT(index, context); \
    SAFE_CHECK_ON_COMPILE(g_gyro_trans_context, index, MAX_TRANSFER);

#define ADD_SENSOR_CONTEXT(index, context)          \
    DEFINE_POINTEROF_SENSOR_CONTEXT(index, context) \
    SAFE_CHECK_ON_COMPILE(g_gyro_sensor_context, index, MAX_SENSOR_NUM);

int         gyro_internal_init(void);
void        gyro_internal_deinit(void);
int         detect_deinit_pre_dev_bydev(struct device *p_device);
int         init_attach_pre_dev(struct device *p_device, gyro_transfer_context_t *p_transfer);
gyro_res_t *get_gyro_reslist(RES_TYPE_E eType);

#endif /* _GYRO_INTERNAL_H */
