/*
 * ss_mbx_impl.c - Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
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

#ifdef __KERNEL__
#include <linux/export.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/limits.h>
#include <cam_sysfs.h>
#else
#include "drv_sysdesc.h"
#include "initcall.h"
#endif

#include "cam_os_wrapper.h"
#include "mbx_platform.h"
#include "mbx_protocol.h"
#include "ss_mbx_impl.h"
#include "ss_mbx_debug.h"

#ifndef true
#define true (1)
#endif

#ifndef false
#define false (0)
#endif

#define MBX_DRV_WRLOCK()     CamOsRwsemDownWrite(&g_drv_rwlock)
#define MBX_DRV_TRY_WRLOCK() CamOsRwsemTryDownWrite(&g_drv_rwlock)
#define MBX_DRV_WRUNLOCK()   CamOsRwsemUpWrite(&g_drv_rwlock)
#define MBX_DRV_RDLOCK()     CamOsRwsemDownRead(&g_drv_rwlock)
#define MBX_DRV_TRY_RDLOCK() CamOsRwsemTryDownRead(&g_drv_rwlock)
#define MBX_DRV_RDUNLOCK()   CamOsRwsemUpRead(&g_drv_rwlock)

#define MBX_CLASS_WRLOCK(class)     CamOsRwsemDownWrite(&g_class_lock[(class)])
#define MBX_CLASS_TRY_WRLOCK(class) CamOsRwsemTryDownWrite(&g_class_lock[(class)])
#define MBX_CLASS_WRUNLOCK(class)   CamOsRwsemUpWrite(&g_class_lock[(class)])
#define MBX_CLASS_RDLOCK(class)     CamOsRwsemDownRead(&g_class_lock[(class)])
#define MBX_CLASS_TRY_RDLOCK(class) CamOsRwsemTryDownRead(&g_class_lock[(class)])
#define MBX_CLASS_RDUNLOCK(class)   CamOsRwsemUpRead(&g_class_lock[(class)])

#define MBX_SEND_LOCK()   CamOsMutexLock(&g_send_mutex)
#define MBX_SEND_UNLOCK() CamOsMutexUnlock(&g_send_mutex)

// reg addr = 0xfd000000 + bank*200 + offset*4
#define RIU_RD_2BYTE(base, offset)      (*(volatile unsigned short *)(base + offset))
#define RIU_WR_2BYTE(base, offset, val) *(volatile unsigned short *)(base + offset) = (val)
#define RIU_WR_2BYTE_MASK(base, offset, val, mask) \
    RIU_WR_2BYTE(base, (offset), ((RIU_RD_2BYTE(base, offset) & ~(mask)) | ((val) & (mask))));

#define MAX_MSG_NUM (128)

#define ASCII_COLOR_RED    "\033[1;31m"
#define ASCII_COLOR_GRAY   "\033[0;37m"
#define ASCII_COLOR_WHITE  "\033[1;37m"
#define ASCII_COLOR_YELLOW "\033[1;33m"
#define ASCII_COLOR_BLUE   "\033[1;36m"
#define ASCII_COLOR_GREEN  "\033[1;32m"
#define ASCII_COLOR_END    "\033[0m"

#ifdef __KERNEL__
#define MBX_OS "Linux"
#else
#define MBX_OS "Rtos"
#endif

#define MBX_ERR(fmt, args...)                                                                                  \
    do                                                                                                         \
    {                                                                                                          \
        if (g_dbg_lv >= E_MBX_DBG_ERR)                                                                         \
            CamOsPrintf(ASCII_COLOR_RED "%s,%s[%d] err: " fmt ASCII_COLOR_END, MBX_OS, __FUNCTION__, __LINE__, \
                        ##args);                                                                               \
    } while (0)

#define MBX_WRN(fmt, args...)                                                                                     \
    do                                                                                                            \
    {                                                                                                             \
        if (g_dbg_lv >= E_MBX_DBG_WRN)                                                                            \
            CamOsPrintf(ASCII_COLOR_YELLOW "%s,%s[%d] wrn: " fmt ASCII_COLOR_END, MBX_OS, __FUNCTION__, __LINE__, \
                        ##args);                                                                                  \
    } while (0)

#define MBX_INFO(fmt, args...)                                                                                    \
    do                                                                                                            \
    {                                                                                                             \
        if (g_dbg_lv >= E_MBX_DBG_INFO)                                                                           \
            CamOsPrintf(ASCII_COLOR_GREEN "%s,%s[%d] info: " fmt ASCII_COLOR_END, MBX_OS, __FUNCTION__, __LINE__, \
                        ##args);                                                                                  \
    } while (0)

#define MBX_DBG(fmt, args...)                                                             \
    do                                                                                    \
    {                                                                                     \
        if (g_dbg_lv >= E_MBX_DBG_DBG)                                                    \
            CamOsPrintf("%s,%s[%d] debug: " fmt, MBX_OS, __FUNCTION__, __LINE__, ##args); \
    } while (0)

#define MBX_CHK_NULL_PTR(ptr)                  \
    do                                         \
    {                                          \
        if (NULL == ptr)                       \
        {                                      \
            MBX_ERR("null pointer error.\n");  \
            return E_SS_MBX_RET_INVAILD_PARAM; \
        }                                      \
    } while (0)

typedef enum
{
    E_MBX_DBG_ERR = 0,
    E_MBX_DBG_WRN,
    E_MBX_DBG_INFO,
    E_MBX_DBG_DBG,
    E_MBX_DEMO_ALL
} mbx_dbglv_e;

typedef struct mbx_msg_node_s
{
    u8 class;
    SS_Mbx_Msg_t           mbx_msg;
    struct CamOsListHead_t node;
} mbx_msg_node_t;

typedef struct mbx_class_s
{
    // for recv msg
    CamOsSpinlock_t        msg_spinlock;
    struct CamOsListHead_t msg_list;
    u8                     msg_cnt;
    CamOsCondition_t       msg_receive_condition;
    u8                     class_will_disable;
    u8                     msg_receive;
} mbx_class_t;

typedef struct mbx_drv_s
{
    u8              init_refs;
    u32             irq_num;
    CamOsMemCache_t msg_node_cache;
    mbx_class_t *   mbx_class[E_MBX_CLASS_MAX]; // use ptr for reduce static size.
} mbx_drv_t;

static mbx_drv_t    g_mbx_drv_t                   = {0};
static CamOsRwsem_t g_drv_rwlock                  = {0}; // lock g_mbx_drv
static CamOsRwsem_t g_class_lock[E_MBX_CLASS_MAX] = {0}; // class lock
/* lock send msg, must be one by one. */
static CamOsMutex_t g_send_mutex;

static int g_dbg_lv = E_MBX_DBG_WRN;

#ifdef __KERNEL__
module_param_named(debug_level, g_dbg_lv, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug_level, "set/get ss_mbx driver debug level.");
#endif

/* for standard: register operation use hal api. */
static void _hal_mbx_irq_ctrl(SS_Mbx_Direct_e eDirect, u8 ctrl)
{
    if (eDirect == E_SS_MBX_DIRECT_ARM_TO_CM4)
    {
        RIU_WR_2BYTE_MASK(MBX_ARM_TO_CM4_INT_BASE, MBX_ARM_TO_CM4_INT_ADDR, ctrl << MBX_ARM_TO_CM4_INT_SHIFT,
                          MBX_ARM_TO_CM4_INT_MASK);
    }
    else if (eDirect == E_SS_MBX_DIRECT_CM4_TO_ARM)
    {
        RIU_WR_2BYTE_MASK(MBX_CM4_TO_ARM_INT_BASE, MBX_CM4_TO_ARM_INT_ADDR, ctrl << MBX_CM4_TO_ARM_INT_SHIFT,
                          MBX_CM4_TO_ARM_INT_MASK);
    }
}

static inline u16 _hal_mbx_read_irq(SS_Mbx_Direct_e eDirect)
{
    u16 ret = 0;

    if (eDirect == E_SS_MBX_DIRECT_ARM_TO_CM4)
    {
        ret = (RIU_RD_2BYTE(MBX_ARM_TO_CM4_INT_BASE, MBX_ARM_TO_CM4_INT_ADDR) & MBX_ARM_TO_CM4_INT_MASK)
              >> MBX_ARM_TO_CM4_INT_SHIFT;
    }
    else if (eDirect == E_SS_MBX_DIRECT_CM4_TO_ARM)
    {
        ret = (RIU_RD_2BYTE(MBX_CM4_TO_ARM_INT_BASE, MBX_CM4_TO_ARM_INT_ADDR) & MBX_CM4_TO_ARM_INT_MASK)
              >> MBX_CM4_TO_ARM_INT_SHIFT;
    }

    return ret;
}

static void _hal_mbx_fire_ctrl(SS_Mbx_Direct_e eDirect, u8 ctrl)
{
    if (eDirect == E_SS_MBX_DIRECT_ARM_TO_CM4)
    {
        RIU_WR_2BYTE_MASK(MBX_REG_BASE, MBX_REG_NONPM_CTRL_ADDR, ctrl << MBX_REG_NONPM_FIRE_SHIFT,
                          MBX_REG_NONPM_FIRE_MASK);
    }
    else if (eDirect == E_SS_MBX_DIRECT_CM4_TO_ARM)
    {
        RIU_WR_2BYTE_MASK(MBX_REG_BASE, MBX_REG_PM_CTRL_ADDR, ctrl << MBX_REG_PM_FIRE_SHIFT, MBX_REG_PM_FIRE_MASK);
    }
}

static inline u16 _hal_mbx_read_fire(SS_Mbx_Direct_e eDirect)
{
    u16 ret = 0;

    if (eDirect == E_SS_MBX_DIRECT_ARM_TO_CM4)
    {
        ret =
            (RIU_RD_2BYTE(MBX_REG_BASE, MBX_REG_NONPM_CTRL_ADDR) & MBX_REG_NONPM_FIRE_MASK) >> MBX_REG_NONPM_FIRE_SHIFT;
    }
    else if (eDirect == E_SS_MBX_DIRECT_CM4_TO_ARM)
    {
        ret = (RIU_RD_2BYTE(MBX_REG_BASE, MBX_REG_PM_CTRL_ADDR) & MBX_REG_PM_FIRE_MASK) >> MBX_REG_PM_FIRE_SHIFT;
    }

    return ret;
}

static void _hal_mbx_set_msg(u8 class, SS_Mbx_Msg_t *mbx_msg)
{
    if (mbx_msg->eDirect == E_SS_MBX_DIRECT_ARM_TO_CM4)
    {
        RIU_WR_2BYTE(MBX_REG_BASE, MBX_REG_NONPM_MSG_INFO_ADDR, (mbx_msg->u8ParameterCount << SHIFT_8) | class);
        RIU_WR_2BYTE(MBX_REG_BASE, MBX_REG_NONPM_MSG_P0_ADDR, mbx_msg->u16Parameters[0]);
        RIU_WR_2BYTE(MBX_REG_BASE, MBX_REG_NONPM_MSG_P1_ADDR, mbx_msg->u16Parameters[1]);
        RIU_WR_2BYTE(MBX_REG_BASE, MBX_REG_NONPM_MSG_P2_ADDR, mbx_msg->u16Parameters[2]);
        RIU_WR_2BYTE(MBX_REG_BASE, MBX_REG_NONPM_MSG_P3_ADDR, mbx_msg->u16Parameters[3]);
        RIU_WR_2BYTE(MBX_REG_BASE, MBX_REG_NONPM_MSG_P4_ADDR, mbx_msg->u16Parameters[4]);
        RIU_WR_2BYTE(MBX_REG_BASE, MBX_REG_NONPM_MSG_P5_ADDR, mbx_msg->u16Parameters[5]);

        MBX_INFO("send class:%d, param_cnt:0x%x, param:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x.\n", class,
                 mbx_msg->u8ParameterCount, mbx_msg->u16Parameters[0], mbx_msg->u16Parameters[1],
                 mbx_msg->u16Parameters[2], mbx_msg->u16Parameters[3], mbx_msg->u16Parameters[4],
                 mbx_msg->u16Parameters[5]);
    }
}

static void _hal_mbx_get_msg(u8 *p_class, SS_Mbx_Msg_t *mbx_msg)
{
    u16 val = 0;
    if (mbx_msg->eDirect == E_SS_MBX_DIRECT_CM4_TO_ARM)
    {
        // val      = RIU_RD_2BYTE(MBX_REG_BASE, MBX_REG_PM_CTRL_ADDR);
        // u8Ctrl   = val & 0xFF;
        // u8Status = (val & 0xFF00) >> SHIFT_8;

        val                       = RIU_RD_2BYTE(MBX_REG_BASE, MBX_REG_PM_MSG_INFO_ADDR);
        *p_class                  = val & 0xFF;
        mbx_msg->u8MsgClass       = *p_class;
        mbx_msg->u8ParameterCount = (val & 0xFF00) >> SHIFT_8;

        mbx_msg->u16Parameters[0] = RIU_RD_2BYTE(MBX_REG_BASE, MBX_REG_PM_MSG_P0_ADDR);
        mbx_msg->u16Parameters[1] = RIU_RD_2BYTE(MBX_REG_BASE, MBX_REG_PM_MSG_P1_ADDR);
        mbx_msg->u16Parameters[2] = RIU_RD_2BYTE(MBX_REG_BASE, MBX_REG_PM_MSG_P2_ADDR);
        mbx_msg->u16Parameters[3] = RIU_RD_2BYTE(MBX_REG_BASE, MBX_REG_PM_MSG_P3_ADDR);
        mbx_msg->u16Parameters[4] = RIU_RD_2BYTE(MBX_REG_BASE, MBX_REG_PM_MSG_P4_ADDR);
        mbx_msg->u16Parameters[5] = RIU_RD_2BYTE(MBX_REG_BASE, MBX_REG_PM_MSG_P5_ADDR);

        MBX_INFO("recv class:%d, param_cnt:0x%x, param:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x.\n", *p_class,
                 mbx_msg->u8ParameterCount, mbx_msg->u16Parameters[0], mbx_msg->u16Parameters[1],
                 mbx_msg->u16Parameters[2], mbx_msg->u16Parameters[3], mbx_msg->u16Parameters[4],
                 mbx_msg->u16Parameters[5]);
    }
}

static void _hal_mbx_clr_msg(SS_Mbx_Direct_e eDirect)
{
    if (eDirect == E_SS_MBX_DIRECT_ARM_TO_CM4)
    {
        RIU_WR_2BYTE(MBX_REG_BASE, MBX_REG_NONPM_CTRL_ADDR, 0);
        RIU_WR_2BYTE(MBX_REG_BASE, MBX_REG_NONPM_MSG_INFO_ADDR, 0);
        RIU_WR_2BYTE(MBX_REG_BASE, MBX_REG_NONPM_MSG_P0_ADDR, 0);
        RIU_WR_2BYTE(MBX_REG_BASE, MBX_REG_NONPM_MSG_P1_ADDR, 0);
        RIU_WR_2BYTE(MBX_REG_BASE, MBX_REG_NONPM_MSG_P2_ADDR, 0);
        RIU_WR_2BYTE(MBX_REG_BASE, MBX_REG_NONPM_MSG_P3_ADDR, 0);
        RIU_WR_2BYTE(MBX_REG_BASE, MBX_REG_NONPM_MSG_P4_ADDR, 0);
        RIU_WR_2BYTE(MBX_REG_BASE, MBX_REG_NONPM_MSG_P5_ADDR, 0);
    }
}

#if 1
typedef enum
{
    E_ERR_NO_ERROR,
    E_ERR_DRIVER_NON_INIT,
    E_ERR_CLASS_NOT_ENABLE,
    E_ERR_CLASS_QUEUE_FULL,
    E_ERR_SYSTEM_NO_MEM,
} send_err_code_e;

static void _hal_mbx_set_err_code(SS_Mbx_Direct_e eDirect, send_err_code_e err_code)
{
    if (eDirect == E_SS_MBX_DIRECT_CM4_TO_ARM)
    {
        RIU_WR_2BYTE_MASK(MBX_REG_BASE, MBX_REG_PM_CTRL_ADDR, err_code << MBX_REG_PM_STATUS_SHIFT,
                          MBX_REG_PM_STATUS_MASK);
    }
    if (eDirect == E_SS_MBX_DIRECT_ARM_TO_CM4)
    {
        RIU_WR_2BYTE_MASK(MBX_REG_BASE, MBX_REG_NONPM_CTRL_ADDR, err_code << MBX_REG_NONPM_STATUS_SHIFT,
                          MBX_REG_NONPM_STATUS_MASK);
    }
}

static void _hal_mbx_get_err_code(SS_Mbx_Direct_e eDirect, send_err_code_e *err_code)
{
    u16 reg_value;
    if (eDirect == E_SS_MBX_DIRECT_ARM_TO_CM4)
    {
        reg_value = RIU_RD_2BYTE(MBX_REG_BASE, MBX_REG_NONPM_CTRL_ADDR);
        *err_code = (send_err_code_e)((reg_value & MBX_REG_NONPM_STATUS_MASK) >> MBX_REG_NONPM_STATUS_SHIFT);
    }
}
#endif

#ifdef __KERNEL__
static s32 _mbx_get_irq_num(const char *irq_name, u32 *irq_num)
{
    struct device_node *    dev_node = NULL;
    struct platform_device *pdev;

    if (irq_num == NULL)
    {
        return -1;
    }

    dev_node = of_find_compatible_node(NULL, NULL, irq_name);
    if (!dev_node)
    {
        return -1;
    }

    pdev = of_find_device_by_node(dev_node);
    if (!pdev)
    {
        of_node_put(dev_node);
        return -1;
    }

    *irq_num = CamIrqOfParseAndMap(pdev->dev.of_node, 0);

    return E_SS_MBX_RET_OK;
}
#else
static s32 _mbx_get_irq_num(const char *irq_name, u32 *irq_num)
{
    u8 tmp_irq_num;
    irq_name = irq_name;
    if (E_SYS_DESC_PASS == MDrv_SysDesc_Read_U8(SYSDESC_DEV_hst0to1, SYSDESC_PRO_interrupts_u8, &tmp_irq_num))
    {
        *irq_num = tmp_irq_num;
    }

    return E_SS_MBX_RET_OK;
}
#endif

static void _mbx_class_enqmsg(mbx_msg_node_t *msg_node, mbx_class_t *mbx_class)
{
    CAM_OS_INIT_LIST_HEAD(&msg_node->node);

    CamOsSpinLockIrqSave(&mbx_class->msg_spinlock);
    CAM_OS_LIST_ADD_TAIL(&msg_node->node, &mbx_class->msg_list);
    mbx_class->msg_cnt++;
    CamOsSpinUnlockIrqRestore(&mbx_class->msg_spinlock);
}

static mbx_msg_node_t *_mbx_class_dqmsg(mbx_class_t *mbx_class)
{
    mbx_msg_node_t *msg_node = NULL;

    CamOsSpinLockIrqSave(&mbx_class->msg_spinlock);
    if (!CAM_OS_LIST_EMPTY(&mbx_class->msg_list))
    {
        msg_node = CAM_OS_CONTAINER_OF(mbx_class->msg_list.pNext, mbx_msg_node_t, node);
        CAM_OS_LIST_DEL(&msg_node->node);
        mbx_class->msg_cnt--;
        CAM_OS_INIT_LIST_HEAD(&msg_node->node);
    }
    CamOsSpinUnlockIrqRestore(&mbx_class->msg_spinlock);

    return msg_node;
}

static void _mbx_class_release_nolock(mbx_class_t **p_mbx_class)
{
    mbx_msg_node_t *msg_node  = NULL;
    mbx_class_t *   mbx_class = *p_mbx_class;
    mbx_drv_t *     mbx_drv   = &g_mbx_drv_t;

    if (mbx_class != NULL)
    {
        msg_node = _mbx_class_dqmsg(mbx_class);
        while (msg_node)
        {
            CamOsMemCacheFree(&mbx_drv->msg_node_cache, msg_node);
            msg_node = _mbx_class_dqmsg(mbx_class);
        }

        CamOsSpinDeinit(&mbx_class->msg_spinlock);
        CamOsConditionDeinit(&(mbx_class->msg_receive_condition));
        CamOsMemset(mbx_class, 0, sizeof(mbx_class_t));
        CamOsMemRelease(mbx_class);
        *p_mbx_class = NULL;
    }
}

static void _mbx_arm_isr(u32 irq_num, void *data)
{
    u8 class                 = 0;
    SS_Mbx_Msg_t    mbx_msg  = {0};
    mbx_msg_node_t *msg_node = NULL;
    mbx_drv_t *     mbx_drv  = (mbx_drv_t *)data;

    mbx_msg.eDirect = E_SS_MBX_DIRECT_CM4_TO_ARM;

    /* first show isr msg for debug. */
    _hal_mbx_get_msg(&class, &mbx_msg);
    if (0 == _hal_mbx_read_fire(mbx_msg.eDirect))
    {
        MBX_ERR("driver bug, fire flag off in isr, drop msg.\n");
        goto __exit_isr;
    }

    if (0 == _hal_mbx_read_irq(mbx_msg.eDirect))
    {
        MBX_ERR("driver bug, irq flag off in isr, maybe irq reg not sync.\n");
        goto __exit_isr;
    }

    if (mbx_drv->init_refs == 0)
    {
        MBX_WRN("mbx not init.\n");
        goto __exit_isr;
    }

    if (class >= E_MBX_CLASS_MAX)
    {
        MBX_ERR("invaild class:%d, limit:%d, drop msg.\n", class, E_MBX_CLASS_MAX - 1);
        goto __exit_isr;
    }

    if (mbx_drv->mbx_class[class] == NULL)
    {
        MBX_WRN("class:%d not enable, drop msg.\n", class);
        _hal_mbx_set_err_code(E_SS_MBX_DIRECT_CM4_TO_ARM, E_ERR_CLASS_NOT_ENABLE);
        goto __exit_isr;
    }

    else if (mbx_drv->mbx_class[class]->msg_cnt >= MAX_MSG_NUM)
    {
        MBX_WRN("class:%d msg overflow, count:%d max:%d, drop msg.\n", class, mbx_drv->mbx_class[class]->msg_cnt,
                MAX_MSG_NUM);
        _hal_mbx_set_err_code(E_SS_MBX_DIRECT_CM4_TO_ARM, E_ERR_CLASS_QUEUE_FULL);
        goto __exit_isr;
    }

    msg_node = CamOsMemCacheAlloc(&mbx_drv->msg_node_cache);
    if (!msg_node)
    {
        MBX_ERR("alloc cache failed, size:%d, drop msg.\n", sizeof(mbx_msg_node_t));
        _hal_mbx_set_err_code(E_SS_MBX_DIRECT_CM4_TO_ARM, E_ERR_SYSTEM_NO_MEM);
        goto __exit_isr;
    }
    // todo reduce this cp, directly read reg to node
    CamOsMemcpy(&msg_node->mbx_msg, &mbx_msg, sizeof(SS_Mbx_Msg_t));
    msg_node->class = class;
    _mbx_class_enqmsg(msg_node, mbx_drv->mbx_class[msg_node->class]);
    CamOsConditionWakeUpAll(&(mbx_drv->mbx_class[msg_node->class]->msg_receive_condition));

__exit_isr:
    CamOsSmpMemoryBarrier();
    _hal_mbx_fire_ctrl(mbx_msg.eDirect, 0);
    _hal_mbx_irq_ctrl(mbx_msg.eDirect, 0);

    return;
}

static SS_Mbx_Ret_e _mbx_irq_init(mbx_drv_t *mbx_drv)
{
#ifdef __KERNEL__
    if (0 != _mbx_get_irq_num("sstar,hst0to1", &mbx_drv->irq_num))
#else
    if (0 != _mbx_get_irq_num(NULL, &mbx_drv->irq_num))
#endif
    {
        MBX_ERR("get irq num err.\n");
        return E_SS_MBX_RET_FAIL;
    }

#ifdef __KERNEL__
    if (0 != CamOsIrqRequest(mbx_drv->irq_num, _mbx_arm_isr, "CM4toARMIsr", mbx_drv))
#else
    if (0 != CamOsIrqRequest(mbx_drv->irq_num, _mbx_arm_isr, "CM4toARMIsr", mbx_drv))
#endif
    {
        MBX_ERR("request irq err, irq num:%d.\n", mbx_drv->irq_num);
        return E_SS_MBX_RET_FAIL;
    }

    _hal_mbx_irq_ctrl(E_SS_MBX_DIRECT_ARM_TO_CM4, 0);
    _hal_mbx_irq_ctrl(E_SS_MBX_DIRECT_CM4_TO_ARM, 0);

    return E_SS_MBX_RET_OK;
}

static void _mbx_irq_deinit(mbx_drv_t *mbx_drv)
{
    CamOsIrqFree(mbx_drv->irq_num, mbx_drv);
}

int SS_Mailbox_IMPL_Init(void)
{
    SS_Mbx_Ret_e e_ret   = E_SS_MBX_RET_OK;
    mbx_drv_t *  mbx_drv = &g_mbx_drv_t;

    MBX_DRV_WRLOCK();
    if (mbx_drv->init_refs != 0)
    {
        MBX_INFO("mbx already inited, cur refs:%d.\n", mbx_drv->init_refs);
        mbx_drv->init_refs++;
        goto __exit_unlock;
    }

    e_ret = _mbx_irq_init(mbx_drv);
    if (e_ret != E_SS_MBX_RET_OK)
    {
        MBX_ERR("_mbx_irq_init fail, ret:%d.\n", e_ret);
        goto __exit_unlock;
    }

    CamOsMemCacheCreate(&mbx_drv->msg_node_cache, "mbx_node_cache", sizeof(mbx_msg_node_t), 1);
    _hal_mbx_clr_msg(E_SS_MBX_DIRECT_ARM_TO_CM4);

    mbx_drv->init_refs++;

__exit_unlock:
    MBX_DRV_WRUNLOCK();

    MBX_DBG("e_ret:%d.\n", e_ret);
    return e_ret;
}

int SS_Mailbox_IMPL_Deinit(void)
{
    SS_Mbx_Ret_e e_ret   = E_SS_MBX_RET_OK;
    mbx_drv_t *  mbx_drv = &g_mbx_drv_t;
    u8 class             = 0;

    MBX_DRV_WRLOCK();
    if (mbx_drv->init_refs == 0)
    {
        MBX_ERR("mbx not init.\n");
        e_ret = E_SS_MBX_RET_NOT_INIT;
        goto __exit_unlock;
    }

    mbx_drv->init_refs--;
    if (mbx_drv->init_refs == 0)
    {
        _mbx_irq_deinit(mbx_drv);
        /* check and wrn not disabled class for user. */
        for (class = 0; class < E_MBX_CLASS_MAX; class ++)
        {
            if (mbx_drv->mbx_class[class] != NULL)
            {
                MBX_WRN("deinit but not disable class:%d, force disable.\n", class);
                _mbx_class_release_nolock(&mbx_drv->mbx_class[class]);
            }
        }
        CamOsMemCacheDestroy(&mbx_drv->msg_node_cache);
        CamOsMemset(mbx_drv, 0, sizeof(mbx_drv_t));
    }

__exit_unlock:
    MBX_DRV_WRUNLOCK();

    MBX_DBG("e_ret:%d.\n", e_ret);
    return e_ret;
}

int SS_Mailbox_IMPL_Enable(u8 class)
{
    SS_Mbx_Ret_e e_ret         = E_SS_MBX_RET_OK;
    mbx_drv_t *  mbx_drv       = &g_mbx_drv_t;
    mbx_class_t *mbx_class     = NULL;
    u8           is_lock_class = false;

    if (class >= E_MBX_CLASS_MAX)
    {
        MBX_ERR("invaild param, class:%d, limit:%d.\n", class, E_MBX_CLASS_MAX - 1);
        return E_SS_MBX_RET_INVAILD_PARAM;
    }

    MBX_DRV_RDLOCK();
    if (mbx_drv->init_refs == 0)
    {
        MBX_ERR("mbx not init.\n");
        e_ret = E_SS_MBX_RET_NOT_INIT;
        goto __exit_unlock;
    }

    MBX_CLASS_WRLOCK(class);
    is_lock_class = true;

    /* note: cur not need support class->refs for multi-process enable same class. */
    if (mbx_drv->mbx_class[class] != NULL)
    {
        MBX_INFO("class:%d already enabled.\n", class);
        e_ret = E_SS_MBX_RET_OK;
        goto __exit_unlock;
    }

    mbx_class = CamOsMemCalloc(1, sizeof(mbx_class_t));
    if (!mbx_class)
    {
        MBX_ERR("alloc mem failed, size:%d.\n", sizeof(mbx_class_t));
        e_ret = E_SS_MBX_RET_NO_MEM;
        goto __exit_unlock;
    }

    mbx_class->msg_cnt = 0;
    CamOsSpinInit(&mbx_class->msg_spinlock);
    CAM_OS_INIT_LIST_HEAD(&mbx_class->msg_list);
    CamOsConditionInit(&(mbx_class->msg_receive_condition));

    mbx_drv->mbx_class[class] = mbx_class;

__exit_unlock:
    if (is_lock_class)
    {
        MBX_CLASS_WRUNLOCK(class);
    }
    MBX_DRV_RDUNLOCK();

    MBX_DBG("class:%d, e_ret:%d.\n", class, e_ret);
    return e_ret;
}

int SS_Mailbox_IMPL_Disable(u8 class)
{
    SS_Mbx_Ret_e e_ret         = E_SS_MBX_RET_OK;
    mbx_drv_t *  mbx_drv       = &g_mbx_drv_t;
    u8           is_lock_class = false;

    if (class >= E_MBX_CLASS_MAX)
    {
        MBX_ERR("invaild param, class:%d, limit:%d.\n", class, E_MBX_CLASS_MAX - 1);
        return E_SS_MBX_RET_INVAILD_PARAM;
    }

    MBX_DRV_RDLOCK();
    if (mbx_drv->init_refs == 0)
    {
        MBX_ERR("mbx not init.\n");
        e_ret = E_SS_MBX_RET_NOT_INIT;
        goto __exit_unlock;
    }

    MBX_CLASS_WRLOCK(class);
    is_lock_class = true;

    if (mbx_drv->mbx_class[class] == NULL)
    {
        MBX_WRN("class:%d already disabled.\n", class);
        e_ret = E_SS_MBX_RET_OK;
        goto __exit_unlock;
    }

    mbx_drv->mbx_class[class]->class_will_disable = true;
    MBX_CLASS_WRUNLOCK(class);
    is_lock_class = false;

    while (CAM_OS_OK != MBX_CLASS_TRY_WRLOCK(class))
    {
        if (mbx_drv->mbx_class[class]->msg_receive == true)
        {
            // for recv timeout -1 case
            CamOsConditionWakeUpAll(&mbx_drv->mbx_class[class]->msg_receive_condition);
            CamOsMsSleep(1);
        }
    }
    is_lock_class = true;

    mbx_drv->mbx_class[class]->class_will_disable = false;
    _mbx_class_release_nolock(&mbx_drv->mbx_class[class]);

__exit_unlock:
    if (is_lock_class)
    {
        MBX_CLASS_WRUNLOCK(class);
    }
    MBX_DRV_RDUNLOCK();

    MBX_DBG("class:%d, e_ret:%d.\n", class, e_ret);
    return e_ret;
}

SS_Mbx_Ret_e _send_err_code_to_result(send_err_code_e err_code)
{
    switch (err_code)
    {
        case E_ERR_NO_ERROR:
            return E_SS_MBX_RET_OK;

        case E_ERR_DRIVER_NON_INIT:
            return E_SS_MBX_RET_TIME_OUT;

        case E_ERR_CLASS_NOT_ENABLE:
            return E_SS_MBX_RET_REMOTE_CLASS_NOT_ENABLE;

        case E_ERR_CLASS_QUEUE_FULL:
            return E_SS_MBX_RET_REMOTE_CLASS_QUEUE_FULL;

        case E_ERR_SYSTEM_NO_MEM:
            return E_SS_MBX_RET_REMOTE_SYSTEM_NO_MEM;

        default:
            return E_SS_MBX_RET_TIME_OUT;
    }
}

int SS_Mailbox_IMPL_SendMsg(SS_Mbx_Msg_t *mbx_msg)
{
    SS_Mbx_Ret_e    e_ret      = E_SS_MBX_RET_OK;
    CamOsTimespec_t start_time = {0}, cur_time = {0}, end_time = {0};
    mbx_drv_t *     mbx_drv         = &g_mbx_drv_t;
    u8 class                        = 0;
    s64             time_diff       = 0;
    s64             timeout_ms      = 10;
    u8              is_send_success = false;
    send_err_code_e err_code;
    u8              is_lock_class = false;

    MBX_CHK_NULL_PTR(mbx_msg);
    class = mbx_msg->u8MsgClass;
    if (class >= E_MBX_CLASS_MAX || mbx_msg->u8ParameterCount > SS_MBX_MAX_PARAM_SIZE
        || mbx_msg->eDirect != E_SS_MBX_DIRECT_ARM_TO_CM4)
    {
        MBX_ERR("invaild param, class:%d > %d. param_cnt:%d > %d. dire:%d != %d.\n", class, E_MBX_CLASS_MAX - 1,
                mbx_msg->u8ParameterCount, SS_MBX_MAX_PARAM_SIZE, mbx_msg->eDirect, E_SS_MBX_DIRECT_ARM_TO_CM4);
        return E_SS_MBX_RET_INVAILD_PARAM;
    }

    MBX_DRV_RDLOCK();
    if (mbx_drv->init_refs == 0)
    {
        MBX_ERR("mbx not init.\n");
        e_ret = E_SS_MBX_RET_NOT_INIT;
        goto __exit_unlock;
    }

    MBX_CLASS_RDLOCK(class);
    is_lock_class = true;

    if (mbx_drv->mbx_class[class] == NULL)
    {
        MBX_ERR("class:%d not enable.\n", class);
        e_ret = E_SS_MBX_RET_NOT_ENABLE;
        goto __exit_unlock;
    }

    MBX_SEND_LOCK();
    _hal_mbx_set_err_code(E_SS_MBX_DIRECT_ARM_TO_CM4, E_ERR_NO_ERROR);
    _hal_mbx_fire_ctrl(mbx_msg->eDirect, 1);
    _hal_mbx_set_msg(class, mbx_msg);
    CamOsSmpMemoryBarrier();
    _hal_mbx_irq_ctrl(mbx_msg->eDirect, 1);

    CamOsGetMonotonicTime(&start_time);
    while (true)
    {
        if (0 == _hal_mbx_read_fire(mbx_msg->eDirect))
        {
            CamOsGetMonotonicTime(&end_time);
            MBX_DBG("send msg cost time:%lldms.\n", CamOsTimeDiff(&start_time, &end_time, CAM_OS_TIME_DIFF_MS));
            is_send_success = true;
            break;
        }

        CamOsGetMonotonicTime(&cur_time);
        time_diff = CamOsTimeDiff(&start_time, &cur_time, CAM_OS_TIME_DIFF_MS);
        if (time_diff > timeout_ms)
        {
            // check again
            if (0 == _hal_mbx_read_fire(mbx_msg->eDirect))
            {
                CamOsGetMonotonicTime(&end_time);
                MBX_DBG("send msg cost time:%lldms.\n", CamOsTimeDiff(&start_time, &end_time, CAM_OS_TIME_DIFF_MS));
                is_send_success = true;
            }
            else
            {
                MBX_DBG("send class:%d msg out of time:%lldms.\n", class, timeout_ms);
                e_ret = E_SS_MBX_RET_TIME_OUT;

                /* peer not recv msg before timeout, so do clear irq and clr msg domain.
                   peer may enter isr to recv msg after timeout, but check fire off err. */
                _hal_mbx_irq_ctrl(mbx_msg->eDirect, 0);
                _hal_mbx_clr_msg(mbx_msg->eDirect);
            }
            break;
        }
        CamOsUsSleep(10);
    }

    if (true == is_send_success)
    {
        _hal_mbx_get_err_code(E_SS_MBX_DIRECT_ARM_TO_CM4, &err_code);
        e_ret = _send_err_code_to_result(err_code);
    }

    MBX_SEND_UNLOCK();

__exit_unlock:
    if (is_lock_class)
    {
        MBX_CLASS_RDUNLOCK(class);
    }
    MBX_DRV_RDUNLOCK();

    MBX_DBG("class:%d, e_ret:%d.\n", class, e_ret);
    return e_ret;
}

int SS_Mailbox_IMPL_RecvMsg(u8 class, SS_Mbx_Msg_t *mbx_msg, s32 wait_ms)
{
    mbx_msg_node_t *msg_node = NULL;
    SS_Mbx_Ret_e    e_ret    = E_SS_MBX_RET_OK;
    mbx_drv_t *     mbx_drv  = &g_mbx_drv_t;
#ifndef U64_MAX
#define U64_MAX ((u64)~0ULL)
#endif

#ifndef S64_MAX
#define S64_MAX ((s64)(U64_MAX >> 1))
#endif
    s64        timeout_ms = wait_ms < 0 ? S64_MAX : wait_ms;
    CamOsRet_e e_camos_ret;
    u8         is_lock_class = false;

    MBX_CHK_NULL_PTR(mbx_msg);
    if (class >= E_MBX_CLASS_MAX || mbx_msg->u8ParameterCount > SS_MBX_MAX_PARAM_SIZE)
    {
        MBX_ERR("invaild param, class:%d > %d. param_cnt:%d > %d.\n", class, E_MBX_CLASS_MAX - 1,
                mbx_msg->u8ParameterCount, SS_MBX_MAX_PARAM_SIZE);
        return E_SS_MBX_RET_INVAILD_PARAM;
    }

    MBX_DRV_RDLOCK();
    if (mbx_drv->init_refs == 0)
    {
        MBX_ERR("mbx not init.\n");
        e_ret = E_SS_MBX_RET_NOT_INIT;
        goto __exit_unlock;
    }

    MBX_CLASS_RDLOCK(class);
    is_lock_class = true;

    if (mbx_drv->mbx_class[class] == NULL)
    {
        MBX_ERR("class:%d not enable.\n", class);
        e_ret = E_SS_MBX_RET_NOT_ENABLE;
        goto __exit_unlock;
    }

    if (mbx_drv->mbx_class[class]->class_will_disable == true)
    {
        MBX_WRN("class:%d is disabling.\n", class);
        e_ret = E_SS_MBX_RET_FAIL;
        goto __exit_unlock;
    }

    while (true)
    {
        if (mbx_drv->mbx_class[class]->msg_cnt > 0)
        {
            msg_node = _mbx_class_dqmsg(mbx_drv->mbx_class[class]);
            CamOsMemcpy(mbx_msg, &msg_node->mbx_msg, sizeof(SS_Mbx_Msg_t));
            CamOsMemCacheFree(&mbx_drv->msg_node_cache, msg_node);
            break;
        }

        mbx_drv->mbx_class[class]->msg_receive = true;
        e_camos_ret                            = CamOsConditionTimedWait(
                                       &mbx_drv->mbx_class[class]->msg_receive_condition,
                                       ((mbx_drv->mbx_class[class]->msg_cnt > 0) || (mbx_drv->mbx_class[class]->class_will_disable == true)),
                                       timeout_ms);
        if (e_camos_ret == CAM_OS_TIMEOUT)
        {
            MBX_DBG("recv class:%d msg out of time:%lldms.\n", class, timeout_ms);
            e_ret = E_SS_MBX_RET_TIME_OUT;
            break;
        }
        else if (mbx_drv->mbx_class[class]->class_will_disable == true)
        {
            MBX_WRN("class:%d is disabling.\n", class);
            e_ret = E_SS_MBX_RET_FAIL;
            break;
        }
    }
    mbx_drv->mbx_class[class]->msg_receive = false;

__exit_unlock:
    if (is_lock_class)
    {
        MBX_CLASS_RDUNLOCK(class);
    }
    MBX_DRV_RDUNLOCK();

    MBX_DBG("class:%d, e_ret:%d.\n", class, e_ret);
    return e_ret;
}

#ifdef __KERNEL__
extern int        ss_mbx_cdev_init(void);
extern void       ss_mbx_cdev_deinit(void);
static s32 __init _mbx_drv_init(void)
{
    unsigned int class;
    MBX_DBG("_mbx_drv_init.\n");
    CamOsRwsemInit(&g_drv_rwlock);
    CamOsMutexInit(&g_send_mutex);
    for (class = 0; class < E_MBX_CLASS_MAX; class ++)
    {
        CamOsRwsemInit(&g_class_lock[class]);
    }
    SS_Mailbox_Debug_Init();
    return ss_mbx_cdev_init();
}

static void __exit _mbx_drv_exit(void)
{
    unsigned int class;
    MBX_DBG("_mbx_drv_exit.\n");
    ss_mbx_cdev_deinit();
    SS_Mailbox_Debug_Deinit();
    for (class = 0; class < E_MBX_CLASS_MAX; class ++)
    {
        CamOsRwsemDeinit(&g_class_lock[class]);
    }
    CamOsMutexDestroy(&g_send_mutex);
    CamOsRwsemDeinit(&g_drv_rwlock);
}

EXPORT_SYMBOL(SS_Mailbox_IMPL_Init);
EXPORT_SYMBOL(SS_Mailbox_IMPL_Deinit);
EXPORT_SYMBOL(SS_Mailbox_IMPL_Enable);
EXPORT_SYMBOL(SS_Mailbox_IMPL_Disable);
EXPORT_SYMBOL(SS_Mailbox_IMPL_SendMsg);
EXPORT_SYMBOL(SS_Mailbox_IMPL_RecvMsg);

module_init(_mbx_drv_init);
module_exit(_mbx_drv_exit);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("SSTAR MBX driver");
MODULE_LICENSE("GPL");
#else
static void _mbx_drv_init(void)
{
    unsigned int class;
    MBX_DBG("_mbx_drv_init.\n");
    CamOsRwsemInit(&g_drv_rwlock);
    CamOsMutexInit(&g_send_mutex);
    for (class = 0; class < E_MBX_CLASS_MAX; class ++)
    {
        CamOsRwsemInit(&g_class_lock[class]);
    }
}

rtos_device_initcall(_mbx_drv_init);
#endif
