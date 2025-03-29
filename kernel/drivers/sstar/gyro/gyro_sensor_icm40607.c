/*
 * gyro_sensor_icm40607.c- Sigmastar
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

#include <linux/delay.h>
#include <linux/hashtable.h>
#include <linux/slab.h>
#include "gyro_internal.h"
#include "gyro_core.h"
#include "gyro.h"

#define GYROSENSOR_ICM40607_INT_LEVEL_L       0x80
#define GYROSENSOR_ICM40607_INT_LEVEL_H       0x00
#define GYROSENSOR_ICM40607_INT_OPEN_DRAIN    0x40
#define GYROSENSOR_ICM40607_INT_PUSH_PULL     0x00
#define GYROSENSOR_ICM40607_LATCH_INT_EN      0x20
#define GYROSENSOR_ICM40607_INT_READ_CLEA     0x10
#define GYROSENSOR_ICM40607_FSYNC_INT_LEVEL_L 0x08
#define GYROSENSOR_ICM40607_FSYNC_INT_LEVEL_H 0x00
#define GYROSENSOR_ICM40607_FSYNC_INT_MODEN   0x04

#define GYROSENSOR_ICM40607_INT_NONE       0x00
#define GYROSENSOR_ICM40607_INT_FIFO_FULL  0x10
#define GYROSENSOR_ICM40607_INT_DATA_READY 0x01

#define GYROSENSOR_ICM40607_FIFO_RD_EN     0x40
#define GYROSENSOR_ICM40607_SPI_INTERFACEN 0x10
#define GYROSENSOR_ICM40607_RESET_FIFO     0x04

typedef struct gyro_delay
{
#define TO_TABLE_INDEX(data) (data.ui_filt_ord)
#define TO_HASH_KEY(data)    (data.odr)
#define TO_MATCH_ID(data)    (data.ui_filt_bw)

    unsigned char     ui_filt_ord; // as hstable index
    unsigned char     odr;         // GYRO/ACCEL_OPR bitvalue,this as a key
    unsigned char     ui_filt_bw;  // as match id
    unsigned int      value;       // group delay
    struct hlist_node hnode;
} gyro_delay_t;

#define ICM40607_SENSOR_UI_FILT_ORD_GEARS (3)
#define ICM40607_SENSOR_ODR_GEARS         (10)
#define ICM40607_SENSOR_UI_FIIL_BW_GEARS  (10)
#define ICM40607_SENSOR_GROUP_DELAY_NUM \
    (ICM40607_SENSOR_UI_FILT_ORD_GEARS * ICM40607_SENSOR_ODR_GEARS * ICM40607_SENSOR_UI_FIIL_BW_GEARS)

#define INIT_GROUPDATA_PRE_ODR(database_index, odr_val, group_delay_arr)                                    \
    do                                                                                                      \
    {                                                                                                       \
        unsigned char ui_filt_bw_index = 0;                                                                 \
        for (ui_filt_bw_index = 0; ui_filt_bw_index < ICM40607_SENSOR_UI_FIIL_BW_GEARS; ui_filt_bw_index++) \
        {                                                                                                   \
            pdatabase[database_index].ui_filt_ord = ui_filt_odr;                                            \
            pdatabase[database_index].odr         = odr_val;                                                \
            pdatabase[database_index].ui_filt_bw  = ui_filt_bw[ui_filt_bw_index];                           \
            pdatabase[database_index++].value     = group_delay_arr[ui_filt_bw_index];                      \
        }                                                                                                   \
    } while (0)

#define MS2US(val) (unsigned int)(val * 1000)
#define GROUP_DELAY_MS2US_TENITEM(odr, val0, val1, val2, val3, val4, val5, val6, val7, val8, val9) \
    do                                                                                             \
    {                                                                                              \
        unsigned int group_delay[ICM40607_SENSOR_UI_FIIL_BW_GEARS] = {                             \
            MS2US(val0), MS2US(val1), MS2US(val2), MS2US(val3), MS2US(val4),                       \
            MS2US(val5), MS2US(val6), MS2US(val7), MS2US(val8), MS2US(val9)};                      \
        INIT_GROUPDATA_PRE_ODR(database_index, odr, group_delay);                                  \
    } while (0)
#define SET_UI_FILT_ODR(val) (ui_filt_odr = val)

#define INIT_ICM40607_GROUP_DELAY(database_arr)                                                              \
    do                                                                                                       \
    {                                                                                                        \
        unsigned short      database_index                               = 0;                                \
        const unsigned char ui_filt_bw[ICM40607_SENSOR_UI_FIIL_BW_GEARS] = {0, 1, 2, 3, 4, 5, 6, 7, 14, 15}; \
        gyro_delay_t *      pdatabase                                    = &(database_arr[0]);               \
        unsigned char       ui_filt_odr                                  = 0;                                \
                                                                                                             \
        SET_UI_FILT_ODR(0);                                                                                  \
        GROUP_DELAY_MS2US_TENITEM(3, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24);            \
        GROUP_DELAY_MS2US_TENITEM(4, 0.43, 0.43, 0.43, 0.43, 0.43, 0.43, 0.43, 0.43, 0.43, 0.43);            \
        GROUP_DELAY_MS2US_TENITEM(5, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80);            \
        GROUP_DELAY_MS2US_TENITEM(6, 0.57, 1.80, 2.02, 2.75, 3.08, 4.09, 4.70, 8.15, 1.55, 0.24);            \
        GROUP_DELAY_MS2US_TENITEM(15, 1.10, 3.55, 3.98, 5.45, 6.10, 8.13, 9.35, 16.24, 3.05, 0.43);          \
        GROUP_DELAY_MS2US_TENITEM(7, 2.66, 4.43, 4.97, 6.81, 7.62, 10.15, 11.67, 20.29, 3.79, 0.99);         \
        GROUP_DELAY_MS2US_TENITEM(8, 5.28, 4.43, 4.97, 6.81, 7.62, 10.15, 11.67, 20.29, 3.79, 1.92);         \
        GROUP_DELAY_MS2US_TENITEM(9, 10.50, 4.43, 4.97, 6.81, 7.62, 10.15, 11.67, 20.29, 3.79, 3.79);        \
        GROUP_DELAY_MS2US_TENITEM(10, 20.95, 4.43, 4.97, 6.81, 7.62, 10.15, 11.67, 20.29, 3.79, 7.54);       \
        GROUP_DELAY_MS2US_TENITEM(11, 20.95, 4.43, 4.97, 6.81, 7.62, 10.15, 11.67, 20.29, 3.79, 7.54);       \
                                                                                                             \
        SET_UI_FILT_ODR(1);                                                                                  \
        GROUP_DELAY_MS2US_TENITEM(3, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24);            \
        GROUP_DELAY_MS2US_TENITEM(4, 0.43, 0.43, 0.43, 0.43, 0.43, 0.43, 0.43, 0.43, 0.43, 0.43);            \
        GROUP_DELAY_MS2US_TENITEM(5, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80);            \
        GROUP_DELAY_MS2US_TENITEM(6, 0.69, 2.06, 2.36, 3.25, 3.69, 5.21, 6.14, 12.03, 1.55, 0.24);           \
        GROUP_DELAY_MS2US_TENITEM(15, 1.34, 4.07, 4.66, 6.44, 7.32, 10.36, 12.23, 24.01, 3.05, 0.43);        \
        GROUP_DELAY_MS2US_TENITEM(7, 3.26, 5.08, 5.81, 8.04, 9.14, 12.94, 15.27, 29.99, 3.79, 0.99);         \
        GROUP_DELAY_MS2US_TENITEM(8, 6.48, 5.08, 5.81, 8.04, 9.14, 12.94, 15.27, 29.99, 3.79, 1.92);         \
        GROUP_DELAY_MS2US_TENITEM(9, 12.90, 5.08, 5.81, 8.04, 9.14, 12.94, 15.27, 29.99, 3.79, 3.79);        \
        GROUP_DELAY_MS2US_TENITEM(10, 25.75, 5.08, 5.81, 8.04, 9.14, 12.94, 15.27, 29.99, 3.79, 7.54);       \
        GROUP_DELAY_MS2US_TENITEM(11, 25.75, 5.08, 5.81, 8.04, 9.14, 12.94, 15.27, 29.99, 3.79, 7.54);       \
                                                                                                             \
        SET_UI_FILT_ODR(2);                                                                                  \
        GROUP_DELAY_MS2US_TENITEM(3, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24);            \
        GROUP_DELAY_MS2US_TENITEM(4, 0.43, 0.43, 0.43, 0.43, 0.43, 0.43, 0.43, 0.43, 0.43, 0.43);            \
        GROUP_DELAY_MS2US_TENITEM(5, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80, 0.80);            \
        GROUP_DELAY_MS2US_TENITEM(6, 0.85, 2.34, 2.75, 3.97, 4.60, 6.65, 8.20, 14.09, 1.55, 0.24);           \
        GROUP_DELAY_MS2US_TENITEM(15, 1.64, 4.63, 5.45, 7.89, 9.15, 13.25, 16.35, 28.14, 3.05, 0.43);        \
        GROUP_DELAY_MS2US_TENITEM(7, 4.02, 5.77, 6.80, 9.84, 11.42, 16.54, 20.42, 35.16, 3.79, 0.99);        \
        GROUP_DELAY_MS2US_TENITEM(8, 7.99, 5.77, 6.80, 9.84, 11.42, 16.54, 20.42, 35.16, 3.79, 1.92);        \
        GROUP_DELAY_MS2US_TENITEM(9, 15.92, 5.77, 6.80, 9.84, 11.42, 16.54, 20.42, 35.16, 3.79, 3.79);       \
        GROUP_DELAY_MS2US_TENITEM(10, 31.80, 5.77, 6.80, 9.84, 11.42, 16.54, 20.42, 35.16, 3.79, 7.54);      \
        GROUP_DELAY_MS2US_TENITEM(11, 31.80, 5.77, 6.80, 9.84, 11.42, 16.54, 20.42, 35.16, 3.79, 7.54);      \
    } while (0)

typedef struct gyro_hstable gyro_hstable_t;
struct gyro_hstable
{
#define TABLE_NUM ICM40607_SENSOR_ODR_GEARS
    DECLARE_HASHTABLE(phshead, TABLE_NUM);
};

typedef struct gyro_dataset gyro_dataset_t;
struct gyro_dataset
{
    unsigned char tablenum;
    int (*insert_node)(const gyro_dataset_t *const pdataset, unsigned char tableindex, unsigned char key,
                       struct hlist_node *pnode);
    void *(*lookup_node)(const gyro_dataset_t *const pdataset, unsigned char tableindex, unsigned char key,
                         unsigned char match_id);
    void (*show_allnode)(const gyro_dataset_t *const pdataset); // for debug check
    gyro_hstable_t *phstable; // --> hashtable[tablenum], index for GYRO/ACCEL_UI_FILT_ORD
    gyro_delay_t    database[0];
};

static gyro_dataset_t *pgroup_delay_dataset = NULL;

static int __insert_group_dealy_node(const gyro_dataset_t *const pdataset, unsigned char tableindex, unsigned char odr,
                                     struct hlist_node *pnode)
{
    CHECK_SUCCESS(pdataset->tablenum > tableindex, return -EINVAL, "range overflow, tablenum[%u] size[%u]",
                  pdataset->tablenum, tableindex);

    hash_add(pdataset->phstable[tableindex].phshead, pnode, odr);

    return GYRO_SUCCESS;
}

static void __showall_group_dealy_node(const gyro_dataset_t *const pdataset)
{
#ifdef CONFIG_SS_GYRO_DEBUG_ON
    unsigned char      index        = 0;
    struct hlist_node *tmp          = NULL;
    gyro_delay_t *     pgroup_delay = NULL;
    int                bkt          = 0;
    unsigned char      old_key      = 0;

    for (index = 0; index < pdataset->tablenum; index++)
    {
        pr_info("\n+------------------------+----------------+-----------------------+-----------------+\n");
        pr_info("| GYRO/ACCEL_UI_FILT_ORD | GYRO/ACCEL_ODR | GYRO/ACCEL_UI_FILT_BW | GROUP_DELAY(us) |\n");
        pr_info("|      as table_id       |     as key     |      as match_id      |                 |\n");
        pr_info("+------------------------+----------------+-----------------------+-----------------+\n");
        hash_for_each_safe(pdataset->phstable[index].phshead, bkt, tmp, pgroup_delay, hnode)
        {
            if (old_key != pgroup_delay->odr)
            {
                pr_info("|------------------------|----------------|-----------------------|-----------------|\n");
                old_key = pgroup_delay->odr;
            }

            pr_info("| %22u | %14u | %21u | %15u |\n", pgroup_delay->ui_filt_ord, pgroup_delay->odr,
                    pgroup_delay->ui_filt_bw, pgroup_delay->value);
        }
        pr_info("+------------------------+----------------+-----------------------+-----------------+\n");
    }
#endif
}

static void *__lookup_group_dealy_node(const gyro_dataset_t *const pdataset, unsigned char tableindex,
                                       unsigned char odr, unsigned char match_ui_filt_bw)
{
    gyro_delay_t *     pgroup_delay = NULL;
    struct hlist_node *tmp          = NULL;
    CHECK_SUCCESS(pdataset->tablenum > tableindex, return NULL, "range overflow, tablenum[%u] size[%u]",
                  pdataset->tablenum, tableindex);

    hash_for_each_possible_safe(pdataset->phstable[tableindex].phshead, pgroup_delay, tmp, hnode, odr)
    {
        if (pgroup_delay->ui_filt_bw == match_ui_filt_bw)
        {
            return pgroup_delay;
        }
    }

    return NULL;
}

static int _insert_node(const gyro_dataset_t *const pdataset, unsigned char tableindex, unsigned char key,
                        struct hlist_node *pnode)
{
    PARAM_SAFE(pdataset, NULL);
    PARAM_SAFE(pnode, NULL);
    PARAM_SAFE(pdataset->insert_node, NULL);

    return pdataset->insert_node(pdataset, tableindex, key, pnode);
}

static void *_lookup_node(const gyro_dataset_t *const pdataset, unsigned char tableindex, unsigned char key,
                          unsigned char match_ui_filt_bw)
{
    PARAM_SAFE_OPS(pdataset, NULL, return NULL;);
    PARAM_SAFE_OPS(pdataset->lookup_node, NULL, return NULL;);

    return pdataset->lookup_node(pdataset, tableindex, key, match_ui_filt_bw);
}

void _show_allnode(const gyro_dataset_t *const pdataset)
{
    PARAM_SAFE_OPS(pdataset, NULL, return;);
    PARAM_SAFE_OPS(pdataset->lookup_node, NULL, return;);

    return pdataset->show_allnode(pdataset);
}

static int _init_gyro_dataset(gyro_dataset_t **ppstdataset, unsigned char tablenum, unsigned char pre_datasize,
                              unsigned short datanum)
{
    int             ret          = GYRO_SUCCESS;
    unsigned short  index        = 0;
    gyro_dataset_t *ptmp_dataset = NULL;
    gyro_hstable_t *phstablemem  = NULL;

    PARAM_SAFE(tablenum, 0);
    PARAM_SAFE(datanum, 0);
    PARAM_SAFE(pre_datasize, 0);
    PARAM_SAFE(ppstdataset, NULL);
    CHECK_SUCCESS((*ppstdataset) == NULL, return -EINVAL, "dataset is already init, dataset[%px]", ppstdataset);

    // alloc mem for dataset
    ptmp_dataset = (gyro_dataset_t *)kzalloc(sizeof(gyro_dataset_t) + pre_datasize * datanum, GFP_KERNEL);
    if (unlikely(ptmp_dataset == NULL))
    {
        GYRO_ERR("no mem for dataset");
        return -ENOMEM;
    }

    // alloc mem for hstable
    phstablemem = kzalloc(sizeof(*phstablemem) * tablenum, GFP_KERNEL);
    if (unlikely(phstablemem == NULL))
    {
        kfree(ptmp_dataset);
        GYRO_ERR("no mem for hstable");
        return -ENOMEM;
    }

    // init dataset structure
    ptmp_dataset->phstable     = phstablemem;
    ptmp_dataset->tablenum     = tablenum;
    ptmp_dataset->insert_node  = __insert_group_dealy_node;
    ptmp_dataset->lookup_node  = __lookup_group_dealy_node;
    ptmp_dataset->show_allnode = __showall_group_dealy_node;

    for (index = 0; index < tablenum; index++)
    {
        hash_init(ptmp_dataset->phstable[index].phshead);
    }

    INIT_ICM40607_GROUP_DELAY(ptmp_dataset->database);

    for (index = 0; index < datanum; index++)
    {
        ret = _insert_node(ptmp_dataset, TO_TABLE_INDEX(ptmp_dataset->database[index]),
                           TO_HASH_KEY(ptmp_dataset->database[index]), &ptmp_dataset->database[index].hnode);
        if (ret != GYRO_SUCCESS)
        {
            goto __exit_freemem;
        }
    }

    *ppstdataset = ptmp_dataset;
    return ret;

__exit_freemem:
    if (phstablemem != NULL)
    {
        kfree(phstablemem);
    }
    if (ptmp_dataset != NULL)
    {
        kfree(ptmp_dataset);
    }
    return ret;
}

static int _deinit_gyro_dataset(gyro_dataset_t **ppstdataset)
{
    gyro_dataset_t *ptmp_dataset = NULL;
    CHECK_SUCCESS(ppstdataset != NULL, return GYRO_SUCCESS;, "dataset is already deinit");

    ptmp_dataset = *ppstdataset;

    // free mem for dataset
    if (ptmp_dataset != NULL)
    {
        if (ptmp_dataset->phstable != NULL)
        {
            kfree(ptmp_dataset->phstable);
        }

        kfree(ptmp_dataset);
    }

    ppstdataset = NULL;

    return GYRO_SUCCESS;
}

static int _icm40607_early_init(struct gyro_dev *dev)
{
    int ret = GYRO_SUCCESS;

    ret = _init_gyro_dataset(&pgroup_delay_dataset, ICM40607_SENSOR_UI_FILT_ORD_GEARS, sizeof(gyro_delay_t),
                             ICM40607_SENSOR_GROUP_DELAY_NUM);
    if (ret != GYRO_SUCCESS)
    {
        return ret;
    }

    _show_allnode(pgroup_delay_dataset);
    return ret;
}

static int _icm40607_final_deinit(struct gyro_dev *dev)
{
    int ret = GYRO_SUCCESS;

    _deinit_gyro_dataset(&pgroup_delay_dataset);
    return ret;
}

enum
{
    GSEN_ICM40607_CONFIG      = 0x11,
    GSEN_ICM40607_SPI_SPEED   = 0x13,
    GSEN_ICM40607_INT_CONFIG  = 0x14,
    GSEN_ICM40607_FIFO_CONFIG = 0x16,

    GSEN_ICM40607_TEMP_OUT_H        = 0x1D,
    GSEN_ICM40607_TEMP_OUT_L        = 0x1E,
    GSEN_ICM40607_ACCEL_XOUT_H      = 0x1F,
    GSEN_ICM40607_ACCEL_XOUT_L      = 0x20,
    GSEN_ICM40607_ACCEL_YOUT_H      = 0x21,
    GSEN_ICM40607_ACCEL_YOUT_L      = 0x22,
    GSEN_ICM40607_ACCEL_ZOUT_H      = 0x23,
    GSEN_ICM40607_ACCEL_ZOUT_L      = 0x24,
    GSEN_ICM40607_GYRO_XOUT_H       = 0x25,
    GSEN_ICM40607_GYRO_XOUT_L       = 0x26,
    GSEN_ICM40607_GYRO_YOUT_H       = 0x27,
    GSEN_ICM40607_GYRO_YOUT_L       = 0x28,
    GSEN_ICM40607_GYRO_ZOUT_H       = 0x29,
    GSEN_ICM40607_GYRO_ZOUT_L       = 0x2A,
    GSEN_ICM40607_TMST_FSYNC_H      = 0x2B,
    GSEN_ICM40607_TMST_FSYNC_L      = 0x2C,
    GSEN_ICM40607_INT_STATUS        = 0x2D,
    GSEN_ICM40607_FIFO_COUNTH       = 0x2E,
    GSEN_ICM40607_FIFO_COUNTL       = 0x2F,
    GSEN_ICM40607_FIFO_DATA         = 0x30,
    GSEN_ICM40607_SIGNAL_PATH_RESET = 0x4B,

    GSEN_ICM40607_INTF_CONFIG0       = 0x4C,
    GSEN_ICM40607_PWR_MGMT_0         = 0x4E,
    GSEN_ICM40607_GYRO_CONFIG0       = 0x4F,
    GSEN_ICM40607_ACCEL_CONFIG0      = 0x50,
    GSEN_ICM40607_GYRO_CONFIG1       = 0x51,
    GSEN_ICM40607_GYRO_ACCEL_CONFIG0 = 0x52,
    GSEN_ICM40607_ACCEL_CONFIG1      = 0x53,

    GSEN_ICM40607_TMST_CONFIG  = 0x54,
    GSEN_ICM40607_SMD_CONFIG   = 0x57,
    GSEN_ICM40607_FIFO_CONFIG1 = 0x5F,
    GSEN_ICM40607_FIFO_CONFIG2 = 0x60,
    GSEN_ICM40607_FIFO_CONFIG3 = 0x61,
    GSEN_ICM40607_FSYNC_CONFIG = 0x62,
    GSEN_ICM40607_INT_CONFIG0  = 0x63,
    GSEN_ICM40607_INT_CONFIG1  = 0x64,

    GSEN_ICM40607_INT_SOURCE0 = 0x65,
    GSEN_ICM40607_INT_SOURCE1 = 0x66,
    GSEN_ICM40607_INT_SOURCE3 = 0x68,
    GSEN_ICM40607_INT_SOURCE4 = 0x69,

    GSEN_ICM40607_FIFO_LOST_PKT0   = 0x6C,
    GSEN_ICM40607_FIFO_LOST_PKT1   = 0x6D,
    GSEN_ICM40607_SELF_TEST_CONFIG = 0x70,

    GSEN_ICM40607_WHO_AM_I     = 0x75,
    GSEN_ICM40607_REG_BANK_SEL = 0x76,
    GSEN_ICM40607_OFFSET_USER0 = 0x77,
    GSEN_ICM40607_OFFSET_USER1 = 0x78,
    GSEN_ICM40607_OFFSET_USER2 = 0x79,
    GSEN_ICM40607_OFFSET_USER3 = 0x7A,
    GSEN_ICM40607_OFFSET_USER4 = 0x7B,
    GSEN_ICM40607_OFFSET_USER5 = 0x7C,
    GSEN_ICM40607_OFFSET_USER6 = 0x7D,
    GSEN_ICM40607_OFFSET_USER7 = 0x7E,
    GSEN_ICM40607_OFFSET_USER8 = 0x7F,
};

static int _icm40607_init(struct gyro_dev *dev)
{
    int ret = GYRO_SUCCESS;

    /* reset */
    ret = dev->reg_ops->write_reg(dev, GSEN_ICM40607_CONFIG, 0x01);
    if (ret < 0)
    {
        GYRO_ERR("GSEN_ICM40607_CONFIG fail, ret:[%d]", ret);
        return ret;
    }

    /*!
     * Note: Need waiting for the reset operation done
     */
    msleep(45);
    ret = dev->reg_ops->write_reg(dev, GSEN_ICM40607_PWR_MGMT_0, 0x0f);
    if (ret < 0)
    {
        GYRO_ERR("GSEN_ICM40607_PWR_MGMT_0 fail, ret:[%d]", ret);
        return ret;
    }

    /*!
     * Note: Need to wait to wake up from sleep mode
     */
    usleep_range(1, 10);

    ret = dev->reg_ops->write_reg(dev, GSEN_ICM40607_FIFO_CONFIG, 0xc0);
    if (ret < 0)
    {
        GYRO_ERR("GSEN_ICM40607_FIFO_CONFIG fail, ret:[%d]", ret);
        return ret;
    }

    ret = dev->reg_ops->write_reg(dev, GSEN_ICM40607_GYRO_CONFIG0, 0x66);
    if (ret < 0)
    {
        GYRO_ERR("GSEN_ICM40607_GYRO_CONFIG0 fail, ret:[%d]", ret);
        return ret;
    }
    ret = dev->reg_ops->write_reg(dev, GSEN_ICM40607_ACCEL_CONFIG0, 0x66);
    if (ret < 0)
    {
        GYRO_ERR("GSEN_ICM40607_ACCEL_CONFIG0 fail, ret:[%d]", ret);
        return ret;
    }

    return GYRO_SUCCESS;
}

static int _icm40607_reset_fifo(struct gyro_dev *dev);

static void _icm40607_deinit(struct gyro_dev *dev)
{
    int ret = GYRO_SUCCESS;

    _icm40607_reset_fifo(dev);
    msleep(10);

    // reset the gyro device
    ret = dev->reg_ops->write_reg(dev, GSEN_ICM40607_CONFIG, 0x01);
    if (ret < 0)
    {
        GYRO_ERR("GSEN_ICM40607_CONFIG fail, ret:[%d]", ret);
        return;
    }

    /*!
     * Note: Need waiting for the reset operation done
     */
    msleep(10);

    // turn off the gyro/accel/temperature sensor, make the gyro idle
    ret = dev->reg_ops->write_reg(dev, GSEN_ICM40607_PWR_MGMT_0, 0x20);
    if (ret < 0)
    {
        GYRO_ERR("GSEN_ICM40607_PWR_MGMT_0 fail, ret:[%d]", ret);
        return;
    }
}

static int _icm40607_enable_fifo(struct gyro_dev *dev, struct gyro_arg_dev_mode mode,
                                 struct gyro_arg_fifo_info *fifo_info)
{
    int ret    = 0;
    u8  val    = 0;
    u8  offset = 0;
    u8  tmp    = 0;
    int i      = 0;
    struct __icg40607_fifo_info
    {
        u8  fifo_type;
        u8 *axis_start;
        u8 *axis_end;
        u8  size;
        u8  reg_setting;
    } info[] = {
        {0xff, &tmp, &tmp, 1, 0x00}, // for header
        {GYROSENSOR_ZA_FIFO_EN | GYROSENSOR_YA_FIFO_EN | GYROSENSOR_XA_FIFO_EN, &fifo_info->ax_start,
         &fifo_info->ax_end, 2, 0x01},
        {GYROSENSOR_ZA_FIFO_EN | GYROSENSOR_YA_FIFO_EN | GYROSENSOR_XA_FIFO_EN, &fifo_info->ay_start,
         &fifo_info->ay_end, 2, 0x01},
        {GYROSENSOR_ZA_FIFO_EN | GYROSENSOR_YA_FIFO_EN | GYROSENSOR_XA_FIFO_EN, &fifo_info->az_start,
         &fifo_info->az_end, 2, 0x01},
        {GYROSENSOR_XG_FIFO_EN | GYROSENSOR_YG_FIFO_EN | GYROSENSOR_ZG_FIFO_EN, &fifo_info->gx_start,
         &fifo_info->gx_end, 2, 0x02},
        {GYROSENSOR_XG_FIFO_EN | GYROSENSOR_YG_FIFO_EN | GYROSENSOR_ZG_FIFO_EN, &fifo_info->gy_start,
         &fifo_info->gy_end, 2, 0x02},
        {GYROSENSOR_XG_FIFO_EN | GYROSENSOR_YG_FIFO_EN | GYROSENSOR_ZG_FIFO_EN, &fifo_info->gz_start,
         &fifo_info->gz_end, 2, 0x02},
        // {GYROSENSOR_TEMP_FIFO_EN, &fifo_info->temp_start, &fifo_info->temp_end, 1, 0x04},
        {0xff, &fifo_info->temp_start, &fifo_info->temp_end, 1, 0x00}, // temp sensor always enable
    };
    if (mode.fifo_mode)
    {
        for (i = 0; i < sizeof(info) / sizeof(info[0]); ++i)
        {
            if (mode.fifo_type & (info[i].fifo_type))
            {
                *info[i].axis_start = offset;
                *info[i].axis_end   = offset + info[i].size - 1;
                val |= info[i].reg_setting;
                offset += info[i].size;
            }
        }
        fifo_info->bytes_pre_data = offset > 8 ? 16 : offset;
        fifo_info->max_fifo_cnt   = 2048;
        fifo_info->is_big_endian  = 1;
    }
    else
    {
        val = 0;
    }
    ret = dev->reg_ops->write_reg(dev, GSEN_ICM40607_FIFO_CONFIG1, val);
    return ret;
}

static int _icm40607_set_sample_rate(struct gyro_dev *dev, struct gyro_arg_sample_rate rate)
{
    u8 gyro_cfg_val  = 0;
    u8 accel_cfg_val = 0;
    dev->reg_ops->read_reg(dev, GSEN_ICM40607_GYRO_CONFIG0, &gyro_cfg_val);
    dev->reg_ops->read_reg(dev, GSEN_ICM40607_GYRO_CONFIG0, &accel_cfg_val);
    gyro_cfg_val &= 0xf0;
    accel_cfg_val &= 0xf0;
    switch (rate.rate)
    {
        case 8000:
        {
            gyro_cfg_val |= 0x03;
            accel_cfg_val |= 0x03;
        }
        break;
        case 4000:
        {
            gyro_cfg_val |= 0x04;
            accel_cfg_val |= 0x04;
        }
        break;
        case 2000:
        {
            gyro_cfg_val |= 0x05;
            accel_cfg_val |= 0x05;
        }
        break;
        case 1000:
        {
            gyro_cfg_val |= 0x06;
            accel_cfg_val |= 0x06;
        }
        break;

        default:
        {
            GYRO_ERR("sample rate is not supported.");
            return -1;
        }
    }
    dev->reg_ops->write_reg(dev, GSEN_ICM40607_GYRO_CONFIG0, gyro_cfg_val);
    dev->reg_ops->write_reg(dev, GSEN_ICM40607_ACCEL_CONFIG0, accel_cfg_val);
    return GYRO_SUCCESS;
}
static int _icm40607_get_sample_rate(struct gyro_dev *dev, struct gyro_arg_sample_rate *rate)
{
    u8 gyro_cfg_val  = 0;
    u8 accel_cfg_val = 0;
    dev->reg_ops->read_reg(dev, GSEN_ICM40607_GYRO_CONFIG0, &gyro_cfg_val);
    dev->reg_ops->read_reg(dev, GSEN_ICM40607_GYRO_CONFIG0, &accel_cfg_val);
    gyro_cfg_val &= 0x0f;
    accel_cfg_val &= 0x0f;
    if (gyro_cfg_val != accel_cfg_val)
    {
        GYRO_ERR("sample rate is different.");
        return -1;
    }
    switch (gyro_cfg_val)
    {
        case 0x03:
            (*rate).rate = 8000;
            break;
        case 0x04:
            (*rate).rate = 4000;
            break;
        case 0x05:
            (*rate).rate = 2000;
            break;
        case 0x06:
            (*rate).rate = 1000;
            break;
        default:
            GYRO_ERR("sample rate 0x%x", gyro_cfg_val);
            return -1;
    }
    return GYRO_SUCCESS;
}
static int _icm40607_set_gyro_range(struct gyro_dev *dev, struct gyro_arg_gyro_range range)
{
    int ret = GYRO_SUCCESS;
    u8  val;

    ret = dev->reg_ops->read_reg(dev, GSEN_ICM40607_GYRO_CONFIG0, &val);
    if (ret != 0)
    {
        return ret;
    }
    val &= ~(0xe0);

    switch (range.range)
    {
        case 125:
            val |= 0x80;
            break;
        case 250:
            val |= 0x60;
            break;
        case 500:
            val |= 0x40;
            break;
        case 1000:
            val |= 0x20;
            break;
        case 2000:
            val |= 0x00;
            break;

        default:
            GYRO_ERR("gyro range is not supported.");
            return -1;
    }

    return dev->reg_ops->write_reg(dev, GSEN_ICM40607_GYRO_CONFIG0, val);
}
static int _icm40607_set_accel_range(struct gyro_dev *dev, struct gyro_arg_accel_range range)
{
    int ret = GYRO_SUCCESS;
    u8  val;

    ret = dev->reg_ops->read_reg(dev, GSEN_ICM40607_ACCEL_CONFIG0, &val);
    if (ret != 0)
    {
        return ret;
    }
    val &= ~(0xe0);

    switch (range.range)
    {
        case 2:
            val |= 0x60;
            break;
        case 4:
            val |= 0x40;
            break;
        case 8:
            val |= 0x20;
            break;
        case 16:
            val |= 0x00;
            break;

        default:
            GYRO_ERR("accel range is not supported.");
            return -1;
    }

    return dev->reg_ops->write_reg(dev, GSEN_ICM40607_ACCEL_CONFIG0, val);
}
static int _icm40607_get_gyro_range(struct gyro_dev *dev, struct gyro_arg_gyro_range *range)
{
    int ret = GYRO_SUCCESS;
    u8  val;

    ret = dev->reg_ops->read_reg(dev, GSEN_ICM40607_GYRO_CONFIG0, &val);
    if (ret != 0)
    {
        return ret;
    }
    val &= 0xe0;

    switch (val)
    {
        case 0x00:
            (*range).range = 2000;
            break;
        case 0x20:
            (*range).range = 1000;
            break;
        case 0x40:
            (*range).range = 500;
            break;
        case 0x60:
            (*range).range = 250;
            break;
        case 0x80:
            (*range).range = 125;
            break;
        default:
            GYRO_ERR("gyro range 0x%x", val);
            return -1;
    }

    return ret;
}
static int _icm40607_get_gyro_sensitivity(struct gyro_dev *dev, struct gyro_arg_sensitivity *sensitivity)
{
    int ret = GYRO_SUCCESS;
    u8  val = 0;

    ret = dev->reg_ops->read_reg(dev, GSEN_ICM40607_GYRO_CONFIG0, &val);
    if (ret != 0)
    {
        return ret;
    }
    val &= 0xe0;

    switch (val)
    {
        case 0x00:
            (*sensitivity).num = 164;
            (*sensitivity).den = 10;
            break;
        case 0x20:
            (*sensitivity).num = 328;
            (*sensitivity).den = 10;
            break;
        case 0x40:
            (*sensitivity).num = 655;
            (*sensitivity).den = 10;
            break;
        case 0x60:
            (*sensitivity).num = 131;
            (*sensitivity).den = 1;
            break;
        default:
            GYRO_ERR("gyro sensitivity 0x%x", val);
            return -1;
    }

    return ret;
}
static int _icm40607_get_accel_range(struct gyro_dev *dev, struct gyro_arg_accel_range *range)
{
    int ret = GYRO_SUCCESS;
    u8  val;

    ret = dev->reg_ops->read_reg(dev, GSEN_ICM40607_ACCEL_CONFIG0, &val);
    if (ret != 0)
    {
        return ret;
    }

    GYRO_INFO("val = %x", val);
    val &= 0xe0;

    switch (val)
    {
        case 0x00:
            (*range).range = 16;
            break;
        case 0x20:
            (*range).range = 8;
            break;
        case 0x40:
            (*range).range = 4;
            break;
        case 0x60:
            (*range).range = 2;
            break;

        default:
            GYRO_ERR("accel range 0x%x", val);
            return -1;
    }

    return ret;
}
static int _icm40607_get_accel_sensitivity(struct gyro_dev *dev, struct gyro_arg_sensitivity *sensitivity)
{
    int ret = GYRO_SUCCESS;
    u8  val = 0;

    ret = dev->reg_ops->read_reg(dev, GSEN_ICM40607_ACCEL_CONFIG0, &val);
    if (ret != 0)
    {
        return ret;
    }
    val &= 0xe0;

    switch (val)
    {
        case 0x00:
            (*sensitivity).num = 2048;
            (*sensitivity).den = 1;
            break;
        case 0x20:
            (*sensitivity).num = 4096;
            (*sensitivity).den = 1;
            break;
        case 0x40:
            (*sensitivity).num = 8192;
            (*sensitivity).den = 1;
            break;
        case 0x60:
            (*sensitivity).num = 16384;
            (*sensitivity).den = 1;
            break;

        default:
            GYRO_ERR("accel range 0x%x", val);
            return -1;
    }

    return ret;
}

static int _icm40607_get_group_delay(struct gyro_dev *dev, struct gyro_arg_group_delay *delay)
{
#define _GET_ACTUAL_VAL(val, sensorname, attrname) \
    (val = (((val)&_MASK_##sensorname(attrname)) >> _SHIFT_##sensorname(attrname)))

/* ---------------- reserve for accel sensor -----------------*/
#define _SHIFT_ACC_UI_FILT_ORD (3)
#define _MASK_ACC_UI_FILT_ORD  (u8)(0b00011000)

#define _SHIFT_ACC_ODR (0)
#define _MASK_ACC_ODR  (u8)(0b00001111)

#define _SHIFT_ACC_UI_FILT_BW (4)
#define _MASK_ACC_UI_FILT_BW  (u8)(0b11110000)

#define _SHIFT_ACC(attrname) _SHIFT_ACC_##attrname
#define _MASK_ACC(attrname)  _SHIFT_ACC_##attrname

#define GET_ACTUAL_ACC_UI_FILT_ORD(val) _GET_ACTUAL_VAL(val, ACC, UI_FILT_ORD)
#define GET_ACTUAL_ACC_UI_FILT_BW(val)  _GET_ACTUAL_VAL(val, ACC, UI_FILT_BW)
#define GET_ACTUAL_ACC_ODR(val)         _GET_ACTUAL_VAL(val, ACC, ODR)

/* ---------------- for gyro sensor -----------------*/
#define _SHIFT_GYRO_UI_FILT_ORD (2)
#define _MASK_GYRO_UI_FILT_ORD  (u8)(0b00001100)

#define _SHIFT_GYRO_ODR (0)
#define _MASK_GYRO_ODR  (u8)(0b00001111)

#define _SHIFT_GYRO_UI_FILT_BW (0)
#define _MASK_GYRO_UI_FILT_BW  (u8)(0b00001111)

#define _SHIFT_GYRO(attrname) _SHIFT_GYRO_##attrname
#define _MASK_GYRO(attrname)  _MASK_GYRO_##attrname

#define GET_ACTUAL_GYRO_UI_FILT_ORD(val) _GET_ACTUAL_VAL(val, GYRO, UI_FILT_ORD)
#define GET_ACTUAL_GYRO_UI_FILT_BW(val)  _GET_ACTUAL_VAL(val, GYRO, UI_FILT_BW)
#define GET_ACTUAL_GYRO_ODR(val)         _GET_ACTUAL_VAL(val, GYRO, ODR)

    u8            gyro_ui_filt_ord = 0;
    u8            gyro_ui_filt_bw  = 0;
    u8            gyro_odr         = 0;
    gyro_delay_t *pgroup_delay     = NULL;

    dev->reg_ops->read_reg(dev, GSEN_ICM40607_GYRO_CONFIG1, &gyro_ui_filt_ord);
    dev->reg_ops->read_reg(dev, GSEN_ICM40607_GYRO_ACCEL_CONFIG0, &gyro_ui_filt_bw);
    dev->reg_ops->read_reg(dev, GSEN_ICM40607_GYRO_CONFIG0, &gyro_odr);

    GET_ACTUAL_GYRO_UI_FILT_ORD(gyro_ui_filt_ord);
    GET_ACTUAL_GYRO_UI_FILT_BW(gyro_ui_filt_bw);
    GET_ACTUAL_GYRO_ODR(gyro_odr);

    pgroup_delay = (gyro_delay_t *)_lookup_node(pgroup_delay_dataset, gyro_ui_filt_ord, gyro_odr, gyro_ui_filt_bw);
    if (pgroup_delay)
    {
        delay->delay_us = pgroup_delay->value;
    }
    else
    {
        GYRO_ERR(
            "can't find the node from dataset,"
            "gyro_ui_filt_ord[%u] gyro_ui_filt_bw[%u] gyro_odr[%u]",
            gyro_ui_filt_ord, gyro_ui_filt_bw, gyro_odr);
        return -EINVAL;
    }

    return GYRO_SUCCESS;
}

static int _icm40607_read_fifo_cnt(struct gyro_dev *dev, u16 *cnt)
{
    u8 val_h = 0;
    u8 val_l = 0;
    dev->reg_ops->read_reg(dev, GSEN_ICM40607_FIFO_COUNTH, &val_h);
    dev->reg_ops->read_reg(dev, GSEN_ICM40607_FIFO_COUNTL, &val_l);
    *cnt = (val_h << 8) + val_l;
    return GYRO_SUCCESS;
}

static int _icm40607_read_fifo_data(struct gyro_dev *dev, u8 *fifo_data, u16 fifo_cnt)
{
    return dev->reg_ops->read_regs(dev, GSEN_ICM40607_FIFO_DATA, fifo_data, fifo_cnt);
}

static int _icm40607_reset_fifo(struct gyro_dev *dev)
{
    return dev->reg_ops->write_reg(dev, GSEN_ICM40607_SIGNAL_PATH_RESET, 0x02);
}

static int _icm40607_whoami_verify(struct gyro_dev *dev)
{
#define WHO_AM_I_VALUE 0x38
    u8  val = 0;
    int ret = GYRO_SUCCESS;
    ret     = dev->reg_ops->read_reg(dev, GSEN_ICM40607_WHO_AM_I, &val);
    if (ret != 0)
    {
        return ret;
    }

    return ret;
}

gyro_sensor_context_t icm40607_context = {
    .list_head =
        {
            .next = NULL,
            .prev = NULL,
        },
    .ops =
        {
            .early_init      = _icm40607_early_init,
            .final_deinit    = _icm40607_final_deinit,
            .init            = _icm40607_init,
            .deinit          = _icm40607_deinit,
            .enable_fifo     = _icm40607_enable_fifo,
            .set_sample_rate = _icm40607_set_sample_rate,
            .get_sample_rate = _icm40607_get_sample_rate,

            .set_gyro_range  = _icm40607_set_gyro_range,
            .set_accel_range = _icm40607_set_accel_range,

            .get_gyro_range       = _icm40607_get_gyro_range,
            .get_gyro_sensitivity = _icm40607_get_gyro_sensitivity,

            .get_accel_range       = _icm40607_get_accel_range,
            .get_accel_sensitivity = _icm40607_get_accel_sensitivity,

            .read_fifo_data      = _icm40607_read_fifo_data,
            .read_fifo_cnt       = _icm40607_read_fifo_cnt,
            .reset_fifo          = _icm40607_reset_fifo,
            .whoami_verify       = _icm40607_whoami_verify,
            .get_group_delay     = _icm40607_get_group_delay,
            .get_noise_bandwidth = NULL,
        },
};

ADD_SENSOR_CONTEXT(SENSOR_TYPE, icm40607_context);