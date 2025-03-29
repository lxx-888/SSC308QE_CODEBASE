/*
 * hal_miu.c - Sigmastar
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
#include <cam_os_wrapper.h>
#include "hal_miu.h"

arbiter_handle g_arb_handle = {
    .group     = {(miu_reg_grp*)IO_ADDRESS(BASE_REG_MIU_GRP_SC0), (miu_reg_grp*)IO_ADDRESS(BASE_REG_MIU_GRP_SC1),
              (miu_reg_grp*)IO_ADDRESS(BASE_REG_MIU_GRP_ISP0)},
    .group_num = GRP_ARB_NUM,
    .prea      = (miu_reg_pre*)IO_ADDRESS(BASE_REG_MIU_PRE_ARB),
    .pa        = (miu_reg_pa*)IO_ADDRESS(BASE_REG_MIU_PA),
    .pa_cfg    = (miu_reg_pa_cfg*)IO_ADDRESS(BASE_REG_MIU_PA_CFG),
    .ip_clean  = {(miu_reg_ip_clean*)IO_ADDRESS(BASE_REG_MIU_EFFI_SC0),
                 (miu_reg_ip_clean*)IO_ADDRESS(BASE_REG_MIU_EFFI_SC1),
                 (miu_reg_ip_clean*)IO_ADDRESS(BASE_REG_MIU_EFFI_ISP0)},
};

#define ARB_HANDLE(dev) (&g_arb_handle)

/*  riu mcg disable, influence in write regs to bank
//  ===================================
//  riu mcg          || influence banks
//  0x1650, 0x6b[9]  || 0x1650, 0x1668, 0x161d
//  0x1651, 0x6b[9]  || 0x1651, 0x1669, 0x161e
//  0x1652, 0x6b[9]  || 0x1652, 0x166b, 0x161a
//  0x1610, 0x3d[0]  || 0x1610
//  0x1658, 0x63[9]  || 0x1658, 0x1611, 0x1619
*/

static void miu_riu_mcg_enable(void)
{
    int             i;
    arbiter_handle* arb = ARB_HANDLE(0);

    for (i = 0; i < arb->group_num; i++)
    {
        arb->group[i]->mcg_diable |= GARB_RIU_SW_MCG_EN;
    }
    arb->prea->mcg_diable |= PREA_RIU_SW_MCG_EN;
    arb->pa->mcg_diable |= PA_RIU_SW_MCG_EN;
    CamOsUsDelay(1);
}

static void miu_riu_mcg_disable(void)
{
    int             i;
    arbiter_handle* arb = ARB_HANDLE(0);

    CamOsUsDelay(1);
    for (i = 0; i < arb->group_num; i++)
    {
        arb->group[i]->mcg_diable &= ~GARB_RIU_SW_MCG_EN;
    }
    arb->prea->mcg_diable &= ~PREA_RIU_SW_MCG_EN;
    arb->pa->mcg_diable &= ~PA_RIU_SW_MCG_EN;
    CamOsUsDelay(1);
}

static void miu_grp_toggle(void)
{
    int             i;
    arbiter_handle* arb = ARB_HANDLE(0);

    for (i = 0; i < arb->group_num; i++)
    {
        arb->group[i]->mcg_diable |= GARB_RIU_SW_MCG_EN;
        CamOsUsDelay(1);

        arb->group[i]->r_toggle |= 0x19;
        arb->group[i]->w_toggle |= 0x19;

        CamOsUsDelay(1);
        arb->group[i]->mcg_diable &= ~GARB_RIU_SW_MCG_EN;
        CamOsUsDelay(1);
    }
}

static void miu_pre_toggle(void)
{
    arbiter_handle* arb = ARB_HANDLE(0);

    arb->prea->mcg_diable |= PREA_RIU_SW_MCG_EN;
    CamOsUsDelay(1);
    // cut_in_enable bit[3] function reverse, U02 will fix it.
    arb->prea->r_toggle |= 0x14;
    arb->prea->w_toggle |= 0x14;

    CamOsUsDelay(1);
    arb->prea->mcg_diable &= ~PREA_RIU_SW_MCG_EN;
    CamOsUsDelay(1);
}

static void miu_pa_toggle(void)
{
    arbiter_handle* arb = ARB_HANDLE(0);

    arb->pa->mcg_diable |= PA_RIU_SW_MCG_EN;
    CamOsUsDelay(1);

    arb->pa->r_toggle = 0x01;
    arb->pa->w_toggle = 0x01;

    CamOsUsDelay(1);
    arb->pa->mcg_diable &= ~PA_RIU_SW_MCG_EN;
    CamOsUsDelay(1);
}
//=================================================================================================
//                                     REG MIU CLIENT Function
//=================================================================================================
void reg_miu_client_set_stall(u16 id, bool write, bool enable)
{
    int             bit;
    arbiter_handle* arb  = ARB_HANDLE(0);
    miu_reg_pa*     p_pa = arb->pa;

    if (GROUP(id) == 7)
    {
        // high way
        bit = CLIENT(id);
    }
    else
    {
        // normal way
        bit = 0;
    }

    if (!write)
    {
        SET_BIT_MASK(p_pa->r_ddr_stall, 1 << bit, enable << bit);
    }
    else
    {
        SET_BIT_MASK(p_pa->w_ddr_stall, 1 << bit, enable << bit);
    }

    miu_pa_toggle();
}

void reg_miu_client_set_parb_burst_normal(u16 id, bool write, u32 burst)
{
    int             offset = GROUP(id) >> 1;
    int             bit    = (GROUP(id) & 0x01) << 3;
    arbiter_handle* arb    = ARB_HANDLE(0);
    miu_reg_pre*    prea   = arb->prea;

    if (!write)
    {
        SET_BIT_MASK(prea->r_burst[offset], 0xFF << bit, burst << bit);
    }
    else
    {
        SET_BIT_MASK(prea->w_burst[offset], 0xFF << bit, burst << bit);
    }

    miu_pre_toggle();
}

void reg_miu_client_set_burst_opt_normal(u16 grp_id, bool write, u32 burst_id, u32 burst)
{
    int             offset = burst_id >> 1;
    int             bit    = (burst_id & 0x01) << 3;
    arbiter_handle* arb    = ARB_HANDLE(0);
    miu_reg_grp*    group  = arb->group[grp_id];

    if (!write)
    {
        SET_BIT_MASK(group->r_burst_opt[offset], 0xFF << bit, burst << bit);
    }
    else
    {
        SET_BIT_MASK(group->w_burst_opt[offset], 0xFF << bit, burst << bit);
    }
    miu_grp_toggle();
}

void reg_miu_client_set_burst_sel_normal(u16 id, bool write, u32 burst)
{
    int             offset = CLIENT(id) >> 3;
    int             bit    = (CLIENT(id) << 1) & 0xF;
    arbiter_handle* arb    = ARB_HANDLE(0);
    miu_reg_grp*    group  = arb->group[GROUP(id)];

    if (!write)
    {
        SET_BIT_MASK(group->r_burst_sel[offset], 0x3 << bit, burst << bit);
    }
    else
    {
        SET_BIT_MASK(group->w_burst_sel[offset], 0x3 << bit, burst << bit);
    }

    miu_grp_toggle();
}

u32 reg_miu_client_get_burst_sel_normal(u16 id, bool write)
{
    int             offset = CLIENT(id) >> 3;
    int             bit    = (CLIENT(id) << 1) & 0xF;
    arbiter_handle* arb    = ARB_HANDLE(0);
    miu_reg_grp*    group  = arb->group[GROUP(id)];

    if (!write)
        return ((group->r_burst_sel[offset] >> bit) & 0x3);
    else
        return ((group->w_burst_sel[offset] >> bit) & 0x3);
}

void reg_miu_client_set_parb_priority_normal(u16 id, bool write, u32 priority)
{
    int             bit  = GROUP(id) << 1;
    arbiter_handle* arb  = ARB_HANDLE(0);
    miu_reg_pre*    prea = arb->prea;

    if (!write)
    {
        SET_BIT_MASK(prea->r_priority, 0x03 << bit, priority << bit);
    }
    else
    {
        SET_BIT_MASK(prea->w_priority, 0x03 << bit, priority << bit);
    }

    miu_pre_toggle();
}

void reg_miu_client_set_priority_normal(u16 id, bool write, u32 priority)
{
    int             offset = CLIENT(id) >> 3;
    int             bit    = (CLIENT(id) << 1) & 0xF;
    arbiter_handle* arb    = ARB_HANDLE(0);
    miu_reg_grp*    group  = arb->group[GROUP(id)];

    if (!write)
    {
        SET_BIT_MASK(group->r_priority[offset], 0x3 << bit, priority << bit);
    }
    else
    {
        SET_BIT_MASK(group->w_priority[offset], 0x3 << bit, priority << bit);
    }

    miu_grp_toggle();
}

u32 reg_miu_client_get_priority_normal(u16 id, bool write)
{
    int             offset = CLIENT(id) >> 3;
    int             bit    = (CLIENT(id) << 1) & 0xF;
    arbiter_handle* arb    = ARB_HANDLE(0);
    miu_reg_grp*    group  = arb->group[GROUP(id)];

    if (!write)
    {
        return ((group->r_priority[offset] >> bit) & 0x3);
    }
    else
    {
        return ((group->w_priority[offset] >> bit) & 0x3);
    }
}

void reg_miu_client_set_mask_normal(u16 id, bool write, bool mask)
{
    int             bit   = CLIENT(id);
    arbiter_handle* arb   = ARB_HANDLE(0);
    miu_reg_grp*    group = arb->group[GROUP(id)];

    if (!write)
    {
        SET_BIT_MASK(group->r_request_mask, 0x1 << bit, mask << bit);
    }
    else
    {
        SET_BIT_MASK(group->w_request_mask, 0x1 << bit, mask << bit);
    }

    miu_grp_toggle();
}

bool reg_miu_client_get_mask_normal(u16 id, bool write)
{
    int             bit   = CLIENT(id);
    arbiter_handle* arb   = ARB_HANDLE(0);
    miu_reg_grp*    group = arb->group[GROUP(id)];

    if (!write)
    {
        return ((group->r_request_mask >> bit) & 0x1);
    }
    else
    {
        return ((group->w_request_mask >> bit) & 0x1);
    }
}

void reg_miu_client_set_urgent_normal(u16 id, bool write, bool urgent)
{
    int             bit   = CLIENT(id);
    arbiter_handle* arb   = ARB_HANDLE(0);
    miu_reg_grp*    group = arb->group[GROUP(id)];
    miu_reg_pre*    prea  = arb->prea;

    if (!write)
    {
        SET_BIT_MASK(group->r_hpmask, 1 << bit, urgent << bit);
        SET_BIT_MASK(prea->r_hpmask, 1 << GROUP(id), urgent << GROUP(id));
        arb->pa->r_port[0].hp_mask = urgent;
    }
    else
    {
        SET_BIT_MASK(group->w_hpmask, 1 << bit, urgent << bit);
        SET_BIT_MASK(prea->w_hpmask, 1 << GROUP(id), urgent << GROUP(id));
        arb->pa->w_port1[0].hp_mask = urgent;
    }

    miu_grp_toggle();
    miu_pre_toggle();
    miu_pa_toggle();
}

bool reg_miu_client_get_urgent_normal(u16 id, bool write)
{
    int             bit   = CLIENT(id);
    arbiter_handle* arb   = ARB_HANDLE(0);
    miu_reg_grp*    group = arb->group[GROUP(id)];

    if (!write)
    {
        return ((group->r_hpmask >> bit) & 0x1);
    }
    else
    {
        return ((group->w_hpmask >> bit) & 0x1);
    }
}

int reg_miu_client_set_flowctrl_normal(u16 id, bool write, bool enable, u32 mask_period, u32 pass_period)
{
    int                   i;
    bool                  flag = false;
    u32*                  flow;
    miu_reg_grp_flowctrl* reg_flow;
    arbiter_handle*       arb = ARB_HANDLE(0);

    if (!write)
    {
        reg_flow = arb->group[GROUP(id)]->r_flowctrl;
        flow     = arb->group[GROUP(id)]->r_flow;
    }
    else
    {
        reg_flow = arb->group[GROUP(id)]->w_flowctrl;
        flow     = arb->group[GROUP(id)]->w_flow;
    }

    // find old setting
    for (i = 0; i < 4; i++)
    {
        if (reg_flow[i].client_id == CLIENT(id))
        {
            flag = true;
            break;
        }
    }

    if (!flag && enable)
    {
        // if not old, find empty reg
        for (i = 0; i < 4; i++)
        {
            if (reg_flow[i].enable == 0)
            {
                flag = true;
                break;
            }
        }
    }

    if (flag)
    {
        // reg_flow[i].mask_period = mask_period;
        // reg_flow[i].client_id   = enable ? CLIENT(ip->id) : 0;
        // reg_flow[i].pass_period = pass_period;
        // reg_flow[i].enable      = enable;

        flow[i] = enable | 0x02;
        flow[i] |= pass_period << 2;
        flow[i] |= enable ? CLIENT(id) << 4 : 0;
        flow[i] |= mask_period << 8;

        if (enable)
        {
            if (write)
                arb->group[GROUP(id)]->w_flowctrl_force |= (1 << i);
            else
                arb->group[GROUP(id)]->r_flowctrl_force |= (1 << i);
        }
        else
        {
            if (write)
                arb->group[GROUP(id)]->w_flowctrl_force &= ~(1 << i);
            else
                arb->group[GROUP(id)]->r_flowctrl_force &= ~(1 << i);
        }

        miu_grp_toggle();
    }
    else if (enable)
    {
        return 1;
    }

    return 0;
}

u32 reg_miu_client_get_mask_period_normal(u16 id, bool write)
{
    int                   i;
    bool                  flag = false;
    miu_reg_grp_flowctrl* reg_flow;
    arbiter_handle*       arb = ARB_HANDLE(0);

    if (!write)
    {
        reg_flow = arb->group[GROUP(id)]->r_flowctrl;
    }
    else
    {
        reg_flow = arb->group[GROUP(id)]->w_flowctrl;
    }

    // find old setting
    for (i = 0; i < 4; i++)
    {
        if (reg_flow[i].client_id == CLIENT(id))
        {
            flag = true;
            break;
        }
    }

    if (flag)
        return reg_flow[i].mask_period;

    return 0;
}

u32 reg_miu_client_get_pass_period_normal(u16 id, bool write)
{
    int                   i;
    bool                  flag = false;
    miu_reg_grp_flowctrl* reg_flow;
    arbiter_handle*       arb = ARB_HANDLE(0);

    if (!write)
    {
        reg_flow = arb->group[GROUP(id)]->r_flowctrl;
    }
    else
    {
        reg_flow = arb->group[GROUP(id)]->w_flowctrl;
    }

    // find old setting
    for (i = 0; i < 4; i++)
    {
        if (reg_flow[i].client_id == CLIENT(id))
        {
            flag = true;
            break;
        }
    }

    if (flag)
        return reg_flow[i].pass_period;

    return 0;
}

void reg_miu_client_set_burst_high(u16 id, bool write, u32 burst)
{
    int              client = CLIENT(id);
    arbiter_handle*  arb    = ARB_HANDLE(0);
    miu_reg_pa_port* w_port;

    if (!write)
    {
        arb->pa->r_port[client].limit = burst;
    }
    else
    {
        w_port        = client < 3 ? &arb->pa->w_port1[client] : &arb->pa->w_port2[client - 3];
        w_port->limit = burst;
    }

    miu_pa_toggle();
}

u32 reg_miu_client_get_burst_high(u16 id, bool write)
{
    int              client = CLIENT(id);
    arbiter_handle*  arb    = ARB_HANDLE(0);
    miu_reg_pa_port* w_port;

    if (!write)
    {
        return arb->pa->r_port[client].limit;
    }
    else
    {
        w_port = client < 3 ? &arb->pa->w_port1[client] : &arb->pa->w_port2[client - 3];
        return w_port->limit;
    }
}

void reg_miu_client_set_priority_high(u16 id, bool write, u32 priority)
{
    int              client = CLIENT(id);
    arbiter_handle*  arb    = ARB_HANDLE(0);
    miu_reg_pa_port* w_port;

    if (!write)
    {
        arb->pa->r_port[client].priority = priority;
    }
    else
    {
        w_port           = client < 3 ? &arb->pa->w_port1[client] : &arb->pa->w_port2[client - 3];
        w_port->priority = priority;
    }

    miu_pa_toggle();
}

u32 reg_miu_client_get_priority_high(u16 id, bool write)
{
    int              client = CLIENT(id);
    arbiter_handle*  arb    = ARB_HANDLE(0);
    miu_reg_pa_port* w_port;

    if (!write)
    {
        return arb->pa->r_port[client].priority;
    }
    else
    {
        w_port = client < 3 ? &arb->pa->w_port1[client] : &arb->pa->w_port2[client - 3];
        return w_port->priority;
    }
}

void reg_miu_client_set_mask_high(u16 id, bool write, bool mask)
{
    int              client = CLIENT(id);
    arbiter_handle*  arb    = ARB_HANDLE(0);
    miu_reg_pa_port* w_port;

    if (!write)
    {
        arb->pa->r_port[client].mask = mask;
    }
    else
    {
        w_port       = client < 3 ? &arb->pa->w_port1[client] : &arb->pa->w_port2[client - 3];
        w_port->mask = mask;
    }

    miu_pa_toggle();
}

bool reg_miu_client_get_mask_high(u16 id, bool write)
{
    int              client = CLIENT(id);
    arbiter_handle*  arb    = ARB_HANDLE(0);
    miu_reg_pa_port* w_port;

    if (!write)
    {
        return arb->pa->r_port[client].mask;
    }
    else
    {
        w_port = client < 3 ? &arb->pa->w_port1[client] : &arb->pa->w_port2[client - 3];
        return w_port->mask;
    }
}

void reg_miu_client_set_urgent_high(u16 id, bool write, bool urgent)
{
    int              client = CLIENT(id);
    arbiter_handle*  arb    = ARB_HANDLE(0);
    miu_reg_pa_port* w_port;

    if (!write)
    {
        arb->pa->r_port[client].hp_mask = urgent;
    }
    else
    {
        w_port          = client < 3 ? &arb->pa->w_port1[client] : &arb->pa->w_port2[client - 3];
        w_port->hp_mask = urgent;
    }

    miu_pa_toggle();
}

bool reg_miu_client_get_urgent_high(u16 id, bool write)
{
    int              client = CLIENT(id);
    arbiter_handle*  arb    = ARB_HANDLE(0);
    miu_reg_pa_port* w_port;

    if (!write)
    {
        return arb->pa->r_port[client].hp_mask;
    }
    else
    {
        w_port = client < 3 ? &arb->pa->w_port1[client] : &arb->pa->w_port2[client - 3];
        return w_port->hp_mask;
    }
}

int reg_miu_client_set_flowctrl_high(u16 id, bool write, bool enable, u32 mask_period, u32 pass_period)
{
    arbiter_handle* arb  = ARB_HANDLE(0);
    u32*            flow = NULL;

    if (!write)
    {
        flow = &arb->pa->r_flow[CLIENT(id)];
    }
    else
    {
        flow = &arb->pa->w_flow[CLIENT(id)];
    }

    *flow = enable | 0x6;
    *flow |= pass_period << 4;
    *flow |= mask_period << 8;

    miu_pa_toggle();

    return 0;
}

u32 reg_miu_client_get_mask_period_high(u16 id, bool write)
{
    arbiter_handle* arb  = ARB_HANDLE(0);
    u32*            flow = NULL;

    if (!write)
    {
        flow = &arb->pa->r_flow[CLIENT(id)];
    }
    else
    {
        flow = &arb->pa->w_flow[CLIENT(id)];
    }

    if (*flow & 0x01)
        return (*flow >> 8);

    return 0;
}

u32 reg_miu_client_get_pass_period_high(u16 id, bool write)
{
    arbiter_handle* arb  = ARB_HANDLE(0);
    u32*            flow = NULL;

    if (!write)
    {
        flow = &arb->pa->r_flow[CLIENT(id)];
    }
    else
    {
        flow = &arb->pa->w_flow[CLIENT(id)];
    }

    if (*flow & 0x01)
        return (*flow >> 4) & 0x3;

    return 0;
}

void reg_miu_client_set_vp(u16 id, bool write, u32 vp)
{
    int             offset = id >> 2;
    int             bit    = (id << 2) & 0xF;
    miu_reg_pa_vprw vprw   = {0};
    arbiter_handle* arb    = ARB_HANDLE(0);

    // printk("0x%x vp%s=%u\n", id, write ? "w" : "r", vp);
    if (vp)
    {
        vprw.enable  = 1;
        vprw.latency = vp - 1;
    }
    else
        vprw.enable = 0;

    if (!write)
    {
#if defined(CAM_OS_LINUX_KERNEL)
        SET_BIT_MASK(arb->pa_cfg->vpr[offset], 0xF << bit, (*((u32*)&vprw) << bit));
#elif defined(CAM_OS_RTK)
        int tmp;

        tmp = vprw.enable | (vprw.latency << 2);
        SET_BIT_MASK(arb->pa_cfg->vpr[offset], 0xF << bit, tmp << bit);
#endif
    }
    else
    {
#if defined(CAM_OS_LINUX_KERNEL)
        SET_BIT_MASK(arb->pa_cfg->vpw[offset], 0xF << bit, (*((u32*)&vprw) << bit));
#elif defined(CAM_OS_RTK)
        int tmp;

        tmp = vprw.enable | (vprw.latency << 2);
        SET_BIT_MASK(arb->pa_cfg->vpw[offset], 0xF << bit, tmp << bit);
#endif
    }

    miu_pa_toggle();
}

u32 reg_miu_client_get_vp(u16 id, bool write)
{
    int             offset = id >> 2;
    int             bit    = (id << 2) & 0xF;
    arbiter_handle* arb    = ARB_HANDLE(0);
    int             val    = 0;

    if (!write)
    {
        val = (arb->pa_cfg->vpr[offset] >> bit) & 0xF;
    }
    else
    {
        val = (arb->pa_cfg->vpw[offset] >> bit) & 0xF;
    }
    if (val & 0x01)
    {
        return (val >> 2) + 1;
    }

    return 0;
}

int reg_miu_client_module_reset(u16 id, bool write)
{
    int               offset = id & 0xF;
    int               grp_id = (id >> 4) & 0xF;
    int               loop   = 100000;
    u32               tmp;
    arbiter_handle*   arb = ARB_HANDLE(0);
    miu_reg_ip_clean* ip_clean;

    miu_riu_mcg_enable();
    if (grp_id <= 2)
    {
        ip_clean = arb->ip_clean[grp_id];
        // printk("%s %d grp_id=%d, offset=%x\r\n", __FUNCTION__, __LINE__, grp_id, offset);
        if (write)
        {
            SET_BIT_MASK(ip_clean->clean_wcmd[offset], 0x0001, 0x0001);
#if 0
            printk("%s %d val=%X\r\n", __FUNCTION__, __LINE__, reg_miu_read(BASE_REG_MIU_EFFI_SC0, 0x10 + offset));
            printk("%s %d val=%X\r\n", __FUNCTION__, __LINE__, reg_miu_read(BASE_REG_MIU_EFFI_SC1, 0x10 + offset));
            printk("%s %d val=%X\r\n", __FUNCTION__, __LINE__, reg_miu_read(BASE_REG_MIU_EFFI_ISP0, 0x10 + offset));
#endif
            CamOsUsDelay(1);
            while ((ip_clean->status_wcmd & (1 << offset)) && (loop > 0))
            {
                CamOsUsDelay(1);
                loop--;
            }
            SET_BIT_MASK(ip_clean->clean_wcmd[offset], 0x0001, 0x0000);
#if 0
            printk("%s %d val=%X\r\n", __FUNCTION__, __LINE__, reg_miu_read(BASE_REG_MIU_EFFI_SC0, 0x10 + offset));
            printk("%s %d val=%X\r\n", __FUNCTION__, __LINE__, reg_miu_read(BASE_REG_MIU_EFFI_SC1, 0x10 + offset));
            printk("%s %d val=%X\r\n", __FUNCTION__, __LINE__, reg_miu_read(BASE_REG_MIU_EFFI_ISP0, 0x10 + offset));
#endif
        }
        else
        {
            SET_BIT_MASK(ip_clean->clean_rcmd[offset], 0x0001, 0x0001);
#if 0
            printk("%s %d val=%X\r\n", __FUNCTION__, __LINE__, reg_miu_read(BASE_REG_MIU_EFFI_SC0, 0x20 + offset));
            printk("%s %d val=%X\r\n", __FUNCTION__, __LINE__, reg_miu_read(BASE_REG_MIU_EFFI_SC1, 0x20 + offset));
            printk("%s %d val=%X\r\n", __FUNCTION__, __LINE__, reg_miu_read(BASE_REG_MIU_EFFI_ISP0, 0x20 + offset));
#endif
            CamOsUsDelay(1);
            while ((ip_clean->status_rcmd & (1 << offset)) && (loop > 0))
            {
                CamOsUsDelay(1);
                loop--;
            }
            SET_BIT_MASK(ip_clean->clean_rcmd[offset], 0x0001, 0x0000);
#if 0
            printk("%s %d val=%X\r\n", __FUNCTION__, __LINE__, reg_miu_read(BASE_REG_MIU_EFFI_SC0, 0x20 + offset));
            printk("%s %d val=%X\r\n", __FUNCTION__, __LINE__, reg_miu_read(BASE_REG_MIU_EFFI_SC1, 0x20 + offset));
            printk("%s %d val=%X\r\n", __FUNCTION__, __LINE__, reg_miu_read(BASE_REG_MIU_EFFI_ISP0, 0x20 + offset));
#endif
        }
    }
    else
    {
        // IPU reset flow
        tmp = arb->pa->mcg_diable;
        // printk("%s %d pa val=%X\r\n", __FUNCTION__, __LINE__, reg_miu_read(BASE_REG_MIU_PA, 0x3d));
        SET_BIT_MASK(arb->pa->mcg_diable, 0x0001, 0x0001);
        // printk("%s %d pa val=%X\r\n", __FUNCTION__, __LINE__, reg_miu_read(BASE_REG_MIU_PA, 0x3d));

        if (write)
        {
            // printk("%s %d pa_cfg val=%X\r\n", __FUNCTION__, __LINE__, reg_miu_read(BASE_REG_MIU_PA_CFG, 0x03));
            SET_BIT_MASK(arb->pa_cfg->port1_ctrl, BIT10, BIT10);
            // printk("%s %d pa_cfg val=%X\r\n", __FUNCTION__, __LINE__, reg_miu_read(BASE_REG_MIU_PA_CFG, 0x03));
            CamOsUsDelay(1);
            while ((arb->pa_cfg->port1_ctrl & BIT12) && (loop > 0))
            {
                CamOsUsDelay(1);
                loop--;
            }
            SET_BIT_MASK(arb->pa_cfg->port1_ctrl, BIT10, 0);
            // printk("%s %d pa_cfg val=%X\r\n", __FUNCTION__, __LINE__, reg_miu_read(BASE_REG_MIU_PA_CFG, 0x03));
        }
        else
        {
            // printk("%s %d pa_cfg val=%X\r\n", __FUNCTION__, __LINE__, reg_miu_read(BASE_REG_MIU_PA_CFG, 0x03));
            SET_BIT_MASK(arb->pa_cfg->port1_ctrl, BIT11, BIT11);
            // printk("%s %d pa_cfg val=%X\r\n", __FUNCTION__, __LINE__, reg_miu_read(BASE_REG_MIU_PA_CFG, 0x03));
            CamOsUsDelay(1);
            while ((arb->pa_cfg->port1_ctrl & BIT13) && (loop > 0))
            {
                CamOsUsDelay(1);
                loop--;
            }
            SET_BIT_MASK(arb->pa_cfg->port1_ctrl, BIT11, 0);
            // printk("%s %d pa_cfg val=%X\r\n", __FUNCTION__, __LINE__, reg_miu_read(BASE_REG_MIU_PA_CFG, 0x03));
        }

        arb->pa->mcg_diable = tmp;
        // printk("%s %d pa val=%X\r\n", __FUNCTION__, __LINE__, reg_miu_read(BASE_REG_MIU_PA, 0x3d));
    }

    miu_riu_mcg_disable();
    if (loop == 0)
        return -1;
    else
        return 0;
}

//=================================================================================================
//                                     REG MIU DRAM Function
//=================================================================================================

u32 reg_miu_dram_get_size(void) // --> MB
{
    int size = reg_miu_read_mask(BASE_REG_MIU_MMU, REG_DRAM_SIZE, REG_DRAM_SIZE_MASK);

    if (size == 0)
    {
        size = 16;
    }

    if (size & 0xF0)
    {
        return (1 << (size & 0x0F)) + (1 << ((size & 0xF0) >> 4));
    }

    return (1 << (size & 0x0F));
}

void reg_miu_dram_set_size(u32 size) // in MB
{
}

u32 reg_miu_dram_get_freq(void)
{
    u32          ddr_region = 0;
    u32          ddfset     = 0;
    unsigned int pll_ch, pll_bank;

    pll_ch = INREGMSK16(BASE_REG_MIU_DFS1_AC + REG_ID_58, 0x01);
    if (pll_ch == 0)
        pll_bank = BASE_REG_MIU_ATOP;
    else
        pll_bank = BASE_REG_MIU_ATOP_G;

    ddfset     = (reg_miu_read_mask(pll_bank, 0x19, 0xFF) << 16) + reg_miu_read(pll_bank, 0x18);
    ddr_region = reg_miu_read_mask(pll_bank, 0x1A, 0x7C) >> 2;
    ddr_region = ddr_region ? ddr_region : 1;
    ddr_region *= 1 << reg_miu_read_mask(pll_bank, 0x1A, 0x03);

    return ((u64)432 << 19) * ddr_region / ddfset;
}

u32 reg_miu_dram_get_pll(void)
{
    return 24 * reg_miu_read_mask(BASE_REG_MIUPLL_PA, 0x3, 0x00FF)
           / ((reg_miu_read_mask(BASE_REG_MIUPLL_PA, 0x3, 0x0700) >> 8) + 2);
}

u8 reg_miu_dram_get_type(void)
{
    /*
        REG_MIU_DRAM_DDR4    = 0,
        REG_MIU_DRAM_DDR3    = 1,
        REG_MIU_DRAM_DDR2    = 2,
        REG_MIU_DRAM_LPDDR4  = 4,
        REG_MIU_DRAM_LPDDR3  = 5,
        REG_MIU_DRAM_LPDDR2  = 6,
        REG_MIU_DRAM_LPDDR4X = 7,
        REG_MIU_DRAM_UNKNOW = 15,
    */
    if (reg_miu_read_mask(BASE_REG_MIU_ATOP, 0x5a, BIT8))
    {
        if (reg_miu_read_mask(BASE_REG_MIU_ATOP, 0x5b, BIT15))
            return REG_MIU_DRAM_LPDDR3;
        else
        {
            if (reg_miu_read_mask(BASE_REG_MIU_ATOP, 0x72, BIT4))
                return REG_MIU_DRAM_LPDDR4X;
            else
                return REG_MIU_DRAM_LPDDR4;
        }
    }
    else
    {
        if (reg_miu_read_mask(BASE_REG_MIU_ATOP, 0x5b, BIT15))
            return REG_MIU_DRAM_DDR3;
        else
            return REG_MIU_DRAM_DDR4;
    }
}

u8 reg_miu_dram_get_data_rate(void)
{
    if ((reg_miu_read_mask(BASE_REG_MIU_DIG, 0x01, 0x0300) >> 8) == 2)
        return 4; // 4X
    else
        return 8; // 8X
}

u8 reg_miu_dram_get_bus_width(void)
{
    switch (reg_miu_read_mask(BASE_REG_MIU_DIG, 0x01, 0x000C) >> 2)
    {
        case 0:
            return 16;
            break;
        case 1:
            return 32;
            break;
        case 2:
            return 64;
            break;
    }

    return 16;
}

u8 reg_miu_dram_get_ssc(void)
{
    return (reg_miu_read_mask(BASE_REG_MIU_ATOP, 0x14, 0xC000) != 0x8000);
}

//=================================================================================================
//                                     REG MIU Protect Function
//=================================================================================================

int reg_miu_protect_set_id_list(u16* ids, int num)
{
    int i;

    for (i = 0; i < num; i++)
    {
        reg_miu_write_mask(BASE_REG_PROTECT_ID, REG_MIU_PROTECT0_ID0 + i, REG_MIU_PROTECT0_ID_MASK, ids[i]);
    }

    return 0;
}

int reg_miu_protect_get_id_list(u16* ids, int num)
{
    int i;

    for (i = 0; i < num; i++)
    {
        ids[i] = reg_miu_read_mask(BASE_REG_PROTECT_ID, REG_MIU_PROTECT0_ID0 + i, REG_MIU_PROTECT0_ID_MASK);
    }

    return 0;
}

int reg_miu_protect_set_addr(int index, ss_phys_addr_t start, ss_phys_addr_t end)
{
    u32 msb;
    start >>= REG_PROTECT_ADDR_ALIGN;
    end = (end >> REG_PROTECT_ADDR_ALIGN) - 1;
    msb = (start >> 16) | ((end - 1) >> 16 << 8);

    reg_miu_write(BASE_REG_MIU_PROTECT, REG_MIU_PROTECT0_START + (index << 1), start & 0xFFFF);
    reg_miu_write(BASE_REG_MIU_PROTECT, REG_MIU_PROTECT0_END + (index << 1), end & 0xFFFF);

    reg_miu_write(BASE_REG_MIU_PROTECT_SUB, REG_MIU_PROTECT0_START_23_16 + index, msb);

    return 0;
}

int reg_miu_protect_get_addr(int index, ss_phys_addr_t* start, ss_phys_addr_t* end)
{
    int hi_start, hi_end;

    *start = reg_miu_read(BASE_REG_MIU_PROTECT, REG_MIU_PROTECT0_START + (index << 1));
    *end   = reg_miu_read(BASE_REG_MIU_PROTECT, REG_MIU_PROTECT0_END + (index << 1));

    hi_start = reg_miu_read(BASE_REG_MIU_PROTECT_SUB, REG_MIU_PROTECT0_START_23_16 + index) & 0x00FF;
    hi_end   = reg_miu_read(BASE_REG_MIU_PROTECT_SUB, REG_MIU_PROTECT0_START_23_16 + index) >> 8;

    *start = (*start | (hi_start << 16)) << REG_PROTECT_ADDR_ALIGN;
    *end   = (*end | (hi_end << 16)) << REG_PROTECT_ADDR_ALIGN;

    return 0;
}

int reg_miu_protect_set_id_enable(int index, u32 id_enable)
{
    reg_miu_write_32(BASE_REG_MIU_PROTECT, REG_MIU_PROTECT0_ID_ENABLE + (index << 1), id_enable);

    return 0;
}

u32 reg_miu_protect_get_id_enable(int index)
{
    return reg_miu_read_32(BASE_REG_MIU_PROTECT, REG_MIU_PROTECT0_ID_ENABLE + (index << 1));
}

int reg_miu_protect_set_enable(int index, int enable)
{
    int shift = index & 0x3;
    reg_miu_write_mask(BASE_REG_MIU_PROTECT_SUB, REG_MIU_PROTECT0_WRITE_ENABLE + (index >> 2), 0x1 << shift,
                       enable << shift);

    return 0;
}

bool reg_miu_protect_get_enable(int index)
{
    int shift = index & 0x3;
    return reg_miu_read_mask(BASE_REG_MIU_PROTECT_SUB, REG_MIU_PROTECT0_WRITE_ENABLE + (index >> 2), 0x1 << shift);
}

int reg_miu_protect_set_invert(int index, int invert)
{
    int shift = (index & 0x3) + 8;
    reg_miu_write_mask(BASE_REG_MIU_PROTECT_SUB, REG_MIU_PROTECT0_WRITE_ENABLE + (index >> 2), 0x1 << shift,
                       invert << shift);

    return 0;
}

bool reg_miu_protect_get_invert(int index)
{
    int shift = (index & 0x3) + 8;
    return reg_miu_read_mask(BASE_REG_MIU_PROTECT_SUB, REG_MIU_PROTECT0_WRITE_ENABLE + (index >> 2), 0x1 << shift);
}

int reg_miu_protect_block(int index, bool enable, bool invert, ss_phys_addr_t start, ss_phys_addr_t end, u32 id_enable)
{
    if (!enable)
    {
        reg_miu_protect_set_enable(index, enable);
        return 0;
    }

    reg_miu_protect_set_addr(index, start, end);
    reg_miu_protect_set_id_enable(index, id_enable);
    reg_miu_protect_set_invert(index, invert);
    reg_miu_protect_set_enable(index, enable);

    return 0;
}

//=================================================================================================
//                                     REG MMU Protect Function
//=================================================================================================
int reg_mmu_protect_set_id_list(u16* ids, int num)
{
    int i;

    for (i = 0; i < num; i++)
    {
        reg_miu_write_mask(BASE_REG_PROTECT_ID, REG_MMU_PROTECT0_ID0 + i, REG_MMU_PROTECT0_ID_MASK, ids[i]);
    }

    return 0;
}

int reg_mmu_protect_get_id_list(u16* ids, int num)
{
    int i;

    for (i = 0; i < num; i++)
    {
        ids[i] = reg_miu_read_mask(BASE_REG_PROTECT_ID, REG_MMU_PROTECT0_ID0 + i, REG_MMU_PROTECT0_ID_MASK);
    }

    return 0;
}

int reg_mmu_protect_set_addr(int index, ss_phys_addr_t start, ss_phys_addr_t end)
{
    u32 msb;
    start >>= REG_PROTECT_ADDR_ALIGN;
    end = (end >> REG_PROTECT_ADDR_ALIGN) - 1;
    msb = (start >> 16) | ((end - 1) >> 16 << 8);

    reg_miu_write(BASE_REG_MMU_PROTECT, REG_MMU_PROTECT0_START + (index << 1), start & 0xFFFF);
    reg_miu_write(BASE_REG_MMU_PROTECT, REG_MMU_PROTECT0_END + (index << 1), end & 0xFFFF);

    reg_miu_write(BASE_REG_MIU_PROTECT_SUB, REG_MMU_PROTECT0_START_23_16 + index, msb);

    return 0;
}

int reg_mmu_protect_get_addr(int index, ss_phys_addr_t* start, ss_phys_addr_t* end)
{
    int hi_start, hi_end;

    *start = reg_miu_read(BASE_REG_MMU_PROTECT, REG_MMU_PROTECT0_START + (index << 1));
    *end   = reg_miu_read(BASE_REG_MMU_PROTECT, REG_MMU_PROTECT0_END + (index << 1));

    hi_start = reg_miu_read(BASE_REG_MIU_PROTECT_SUB, REG_MMU_PROTECT0_START_23_16 + index) & 0x00FF;
    hi_end   = reg_miu_read(BASE_REG_MIU_PROTECT_SUB, REG_MMU_PROTECT0_START_23_16 + index) >> 8;

    *start = (*start | (hi_start << 16)) << REG_PROTECT_ADDR_ALIGN;
    *end   = (*end | (hi_end << 16)) << REG_PROTECT_ADDR_ALIGN;

    return 0;
}

int reg_mmu_protect_set_id_enable(int index, u32 id_enable)
{
    reg_miu_write_32(BASE_REG_MMU_PROTECT, REG_MMU_PROTECT0_ID_ENABLE + (index << 1), id_enable);

    return 0;
}

u32 reg_mmu_protect_get_id_enable(int index)
{
    return reg_miu_read_32(BASE_REG_MMU_PROTECT, REG_MMU_PROTECT0_ID_ENABLE + (index << 1));
}

int reg_mmu_protect_set_enable(int index, int enable)
{
    int shift = index & 0x3;
    reg_miu_write_mask(BASE_REG_MIU_PROTECT_SUB, REG_MMU_PROTECT0_WRITE_ENABLE + (index >> 2), 0x1 << shift,
                       enable << shift);

    return 0;
}

bool reg_mmu_protect_get_enable(int index)
{
    int shift = index & 0x3;
    return reg_miu_read_mask(BASE_REG_MIU_PROTECT_SUB, REG_MMU_PROTECT0_WRITE_ENABLE + (index >> 2), 0x1 << shift);
}

int reg_mmu_protect_set_invert(int index, int invert)
{
    int shift = (index & 0x3) + 8;
    reg_miu_write_mask(BASE_REG_MIU_PROTECT_SUB, REG_MMU_PROTECT0_WRITE_ENABLE + (index >> 2), 0x1 << shift,
                       invert << shift);

    return 0;
}

bool reg_mmu_protect_get_invert(int index)
{
    int shift = (index & 0x3) + 8;
    return reg_miu_read_mask(BASE_REG_MIU_PROTECT_SUB, REG_MMU_PROTECT0_WRITE_ENABLE + (index >> 2), 0x1 << shift);
}

int reg_mmu_protect_block(int index, bool enable, bool invert, ss_phys_addr_t start, ss_phys_addr_t end, u32 id_enable)
{
    if (!enable)
    {
        reg_mmu_protect_set_enable(index, enable);
        return 0;
    }

    reg_mmu_protect_set_addr(index, start, end);
    reg_mmu_protect_set_id_enable(index, id_enable);
    reg_mmu_protect_set_invert(index, invert);
    reg_mmu_protect_set_enable(index, enable);

    return 0;
}

//=================================================================================================
//                                     REG MMU Protect Log Function
//=================================================================================================

static struct reg_miu_protect_log* reg_miu_protect_log(void)
{
    return (struct reg_miu_protect_log*)IO_ADDRESS(GET_REG_ADDR(BASE_REG_MIU_MMU, REG_PROTECT_LOG));
}

int reg_miu_protect_flag(bool mmu, bool* bwr)
{
    if (mmu)
    {
        if (reg_miu_protect_log()->mmu_r_flag)
        {
            *bwr = 0;
            return reg_miu_protect_log()->mmu_r_flag;
        }
        else
        {
            *bwr = 1;
            return reg_miu_protect_log()->mmu_w_flag;
        }
    }

    if (reg_miu_protect_log()->miu_r_flag)
    {
        *bwr = 0;
        return reg_miu_protect_log()->miu_r_flag;
    }
    else
    {
        *bwr = 1;
        return reg_miu_protect_log()->miu_w_flag;
    }
}

int reg_miu_protect_hit_block(bool mmu, bool bwr)
{
    if (mmu)
    {
        if (bwr)
            return reg_miu_protect_log()->mmu_w_block;
        else
            return reg_miu_protect_log()->mmu_r_block;
    }

    if (bwr)
        return reg_miu_protect_log()->miu_w_block;
    else
        return reg_miu_protect_log()->miu_r_block;
}

int reg_miu_protect_hit_id(bool mmu, bool bwr)
{
    if (mmu)
    {
        if (bwr)
            return reg_miu_protect_log()->mmu_w_id;
        else
            return reg_miu_protect_log()->mmu_r_id;
    }

    if (bwr)
        return reg_miu_protect_log()->miu_w_id;
    else
        return reg_miu_protect_log()->miu_r_id;
}

int reg_miu_protect_log_clr(bool mmu, int clr)
{
    if (mmu)
    {
        reg_miu_protect_log()->mmu_log_clr = clr;
    }
    else
    {
        reg_miu_protect_log()->miu_log_clr = clr;
    }

    return 0;
}

int reg_miu_protect_irq_mask(bool mmu, int mask)
{
    if (mmu)
    {
        reg_miu_protect_log()->mmu_irq_mask = mask;
    }
    else
    {
        reg_miu_protect_log()->miu_irq_mask = mask;
    }

    return 0;
}

ss_phys_addr_t reg_miu_protect_hit_addr(bool mmu, bool bwr)
{
    if (mmu)
    {
        if (bwr)
            return reg_miu_read_32(BASE_REG_MIU_MMU, REG_MMU_PROTECT_HIT_ADDR_WRITE);
        else
            return reg_miu_read_32(BASE_REG_MIU_MMU, REG_MMU_PROTECT_HIT_ADDR_READ);
    }

    if (bwr)
        return reg_miu_read_32(BASE_REG_MIU_MMU, REG_MIU_PROTECT_HIT_ADDR_WRITE);
    else
        return reg_miu_read_32(BASE_REG_MIU_MMU, REG_MIU_PROTECT_HIT_ADDR_READ);
}

int reg_miu_protect_clear_irq(void)
{
    u16 val = 0;

    if (INREG16(GET_REG_ADDR(BASE_REG_MIU_MMU, 0x24)) & BIT0)
        val = 1;
    if (INREG16(GET_REG_ADDR(BASE_REG_MIU_MMU, 0x25)) & BIT0)
        val = 1;
    if (INREG16(GET_REG_ADDR(BASE_REG_MIU_MMU, 0x34)) & BIT0)
        val = 1;
    if (INREG16(GET_REG_ADDR(BASE_REG_MIU_MMU, 0x35)) & BIT0)
        val = 1;

    if (val)
    {
        OUTREG16(GET_REG_ADDR(BASE_REG_MIU_MMU, 0x21), 0x000A);
        printk(KERN_ERR "1. bank 1659, 0x24: %04X, 0x25: %04X\r\n", INREG16(GET_REG_ADDR(BASE_REG_MIU_MMU, 0x24)),
               INREG16(GET_REG_ADDR(BASE_REG_MIU_MMU, 0x25)));
        printk(KERN_ERR "1. bank 1659, 0x34: %04X, 0x35: %04X\r\n", INREG16(GET_REG_ADDR(BASE_REG_MIU_MMU, 0x34)),
               INREG16(GET_REG_ADDR(BASE_REG_MIU_MMU, 0x35)));
#if 1
        OUTREG16(GET_REG_ADDR(BASE_REG_MIU_MMU, 0x21), 0x000B);
        printk(KERN_ERR "2. bank 1659, 0x24: %04X, 0x25: %04X\r\n", INREG16(GET_REG_ADDR(BASE_REG_MIU_MMU, 0x24)),
               INREG16(GET_REG_ADDR(BASE_REG_MIU_MMU, 0x25)));
        OUTREG16(GET_REG_ADDR(BASE_REG_MIU_MMU, 0x21), 0x000F);
        printk(KERN_ERR "2. bank 1659, 0x34: %04X, 0x35: %04X\r\n", INREG16(GET_REG_ADDR(BASE_REG_MIU_MMU, 0x34)),
               INREG16(GET_REG_ADDR(BASE_REG_MIU_MMU, 0x35)));
#endif
        OUTREG16(GET_REG_ADDR(BASE_REG_MIU_MMU, 0x21), 0x0000);

        return 1;
    }

    return 0;
}

//=================================================================================================
//                                     REG MIU MMU Function
//=================================================================================================

struct reg_miu_mmu* reg_miu_mmu(void)
{
    return (struct reg_miu_mmu*)IO_ADDRESS(GET_REG_ADDR(BASE_REG_MIU_MMU, REG_MMU_CTRL));
}

void reg_miu_mmu_set_page_size(u8 page_size)
{
    reg_miu_mmu()->page_size = page_size;
}

void reg_miu_mmu_set_region(u16 vpa_region, u16 pa_region)
{
    struct reg_miu_mmu* mmu = reg_miu_mmu();

    mmu->vpa_region = vpa_region & 0xFF;
    mmu->pa_region  = pa_region & 0xFF;

    mmu->vpa_region_msb = vpa_region >> 8;
    mmu->pa_region_msb  = pa_region >> 8;
}

void reg_miu_mmu_set_entry(u16 vpa_entry, u16 pa_entry)
{
    struct reg_miu_mmu* mmu = reg_miu_mmu();

    mmu->w_data   = pa_entry;
    mmu->entry_rw = 1;
    mmu->entry    = vpa_entry;
    mmu->access   = 0x3;
}

u16 reg_miu_mmu_get_entry(u16 vpa_entry)
{
    struct reg_miu_mmu* mmu = reg_miu_mmu();
    mmu->entry              = vpa_entry;
    mmu->entry_rw           = 0;

    mmu->access = 0x3;

    CamOsUsDelay(1);

    return mmu->r_data;
}

void reg_miu_mmu_enable(bool enable)
{
    struct reg_miu_mmu* mmu = reg_miu_mmu();

    mmu->enable          = enable;
    mmu->collison_mask_r = !enable;
    mmu->collison_mask_w = !enable;
    mmu->invalid_mask_r  = !enable;
    mmu->invalid_mask_w  = !enable;
}

u8 reg_miu_mmu_enable_status(void)
{
    struct reg_miu_mmu* mmu = reg_miu_mmu();

    return mmu->enable;
}

u8 reg_miu_mmu_reset(void)
{
    struct reg_miu_mmu* mmu       = reg_miu_mmu();
    u16                 retry_num = 2000;

    mmu->enable      = 0;
    mmu->init_done_r = 0;
    mmu->init_done_w = 0;
    mmu->reset       = 0;
    mmu->init_val    = 0;

    mmu->reset    = 1;
    mmu->init_val = 1;

    while (!mmu->init_done_r && !mmu->init_done_w)
    {
        CamOsUsDelay(1);

        retry_num--;
        if (!retry_num)
        {
            return 1;
        }
    }

    mmu->reset    = 0;
    mmu->init_val = 0;
    return 0;
}

u32 reg_miu_mmu_get_irq_status(u16* entry, u16* id, u8* is_write)
{
    u32                 status = 0;
    struct reg_miu_mmu* mmu    = reg_miu_mmu();

    if (mmu->collison_flag_r)
    {
        *entry = mmu->collison_entry_r;
        *id    = 0;

        status |= 1;
    }

    if (mmu->collison_flag_w)
    {
        *entry = mmu->collison_entry_w;
        *id    = 0;

        status |= 1;
    }

    if (mmu->invalid_flag_r)
    {
        *entry = mmu->invalid_entry_r;
        *id    = mmu->invalid_id_r;

        status |= 2;
    }

    if (mmu->invalid_flag_w)
    {
        *entry = mmu->invalid_entry_w;
        *id    = mmu->invalid_id_w;

        status |= 4;
    }

    *is_write = mmu->invalid_flag_w || mmu->collison_flag_w;

    return status;
}

void reg_miu_mmu_irq_mask(int mask)
{
    struct reg_miu_mmu* mmu = reg_miu_mmu();
    mmu->collison_mask_r    = mask;
    mmu->collison_mask_w    = mask;
    mmu->invalid_mask_r     = mask;
    mmu->invalid_mask_w     = mask;
}

void reg_miu_mmu_irq_clr(int clr)
{
    struct reg_miu_mmu* mmu = reg_miu_mmu();
    mmu->collison_clr_r     = clr;
    mmu->collison_clr_w     = clr;
    mmu->invalid_clr_r      = clr;
    mmu->invalid_clr_w      = clr;
}

#if defined(CAM_OS_LINUX_KERNEL)
//=================================================================================================
//                                     REG MIU BIST Function
//=================================================================================================

static bool reg_miu_bist_is_chip_top(u32 effi_bank)
{
    return effi_bank == BASE_REG_MIU_DIG;
}

static struct reg_miu_bist_chiptop_status* reg_miu_bist_chiptop(u32 effi_bank)
{
    return (struct reg_miu_bist_chiptop_status*)IO_ADDRESS(GET_REG_ADDR(effi_bank, REG_BIST_CHIPTOP_STATUS));
}

static struct reg_miu_bist_group_status* reg_miu_bist_group_status(u32 effi_bank)
{
    return (struct reg_miu_bist_group_status*)IO_ADDRESS(GET_REG_ADDR(effi_bank, REG_BIST_GROUP_STATUS));
}

static struct reg_miu_bist_group_result* reg_miu_bist_group_result(u32 effi_bank)
{
    return (struct reg_miu_bist_group_result*)IO_ADDRESS(GET_REG_ADDR(effi_bank, REG_BIST_GROUP_RESULT));
}

void reg_miu_bist_init(u32 group_bank, u32 effi_bank)
{
    if (reg_miu_bist_is_chip_top(effi_bank))
    {
        reg_miu_write(effi_bank, REG_BIST_CHIPTOP_BUS_SEL, 0x80e1);
        reg_miu_write(effi_bank, REG_BIST_CHIPTOP_DATA, 0x5a5b);
    }
    else
    {
        reg_miu_write(effi_bank, REG_BIST_GROUP_STATUS, 0x0000);                               // clear bist
        reg_miu_write_mask(group_bank, REG_BIST_GROUP_BIST_SEL, BIT15 | BIT14, BIT15 | BIT14); // Bist mux
        reg_miu_write(effi_bank, REG_BIST_GROUP_DATA_BYTE, 0xFFFF);
    }
}

void reg_miu_bist_set_base(u32 effi_bank, ss_miu_addr_t addr)
{
    if (reg_miu_bist_is_chip_top(effi_bank))
    {
        addr >>= REG_BIST_CHIPTOP_BASE_ALIGN;
        reg_miu_write(effi_bank, REG_BIST_CHIPTOP_BASE, addr & 0xFFFF);
        addr = (addr >> 16) << REG_BIST_CHIPTOP_BASE_EXT_SHIFT;
        reg_miu_write_mask(effi_bank, REG_BIST_CHIPTOP_BASE_EXT, REG_BIST_CHIPTOP_BASE_EXT_MASK, addr);
    }
    else
    {
        addr >>= REG_BIST_GROUP_BASE_ALIGN;
        reg_miu_write_32(effi_bank, REG_BIST_GROUP_BASE_LO, addr);
    }
}

void reg_miu_bist_set_length(u32 effi_bank, u32 length)
{
    if (reg_miu_bist_is_chip_top(effi_bank))
    {
        length >>= REG_BIST_CHIPTOP_LENGTH_ALIGN;
        reg_miu_write_32(effi_bank, REG_BIST_CHIPTOP_LENGTH_LO, length);
    }
    else
    {
        length >>= REG_BIST_GROUP_LENGTH_ALIGN;
        reg_miu_write_32(effi_bank, REG_BIST_GROUP_LENGTH_LO, length);
    }
}

void reg_miu_bist_set_loop(u32 effi_bank, bool loop)
{
    if (reg_miu_bist_is_chip_top(effi_bank))
    {
        reg_miu_bist_chiptop(effi_bank)->loop = loop;
    }
    else
    {
        reg_miu_bist_group_status(effi_bank)->loop = loop;
    }
}

void reg_miu_bist_set_rw_only(u32 effi_bank, bool read_only)
{
    if (reg_miu_bist_is_chip_top(effi_bank))
    {
        if (read_only)
            reg_miu_bist_chiptop(effi_bank)->read_only = 1;
        else
            reg_miu_bist_chiptop(effi_bank)->write_only = 1;
    }
    else
    {
        if (read_only)
            reg_miu_bist_group_status(effi_bank)->read_only = 1;
        else
            reg_miu_bist_group_status(effi_bank)->write_only = 1;
    }
}

void reg_miu_bist_set_cmd_mode(u32 effi_bank, u16 cmd_mode)
{
    if (reg_miu_bist_is_chip_top(effi_bank))
    {
    }
    else
    {
        reg_miu_bist_group_status(effi_bank)->burst_len = cmd_mode & 0x03;
        reg_miu_bist_group_status(effi_bank)->cmd_len   = (cmd_mode >> 2) & 0x03;
    }
}

static void reg_miu_bist_set_status(u32 effi_bank, bool start)
{
    if (reg_miu_bist_is_chip_top(effi_bank))
    {
        reg_miu_bist_chiptop(effi_bank)->enable = start;
    }
    else
    {
        if (start)
        {
            reg_miu_bist_group_result(effi_bank)->clear = 1;
            CamOsUsDelay(1);
        }
        reg_miu_bist_group_status(effi_bank)->enable = start;
    }
}

void reg_miu_bist_start(u32 effi_bank)
{
    reg_miu_bist_set_status(effi_bank, true);
}

void reg_miu_bist_stop(u32 effi_bank)
{
    reg_miu_bist_set_status(effi_bank, false);
}

u16 reg_miu_bist_get_result(u32 effi_bank)
{
    if (reg_miu_bist_is_chip_top(effi_bank))
    {
        return reg_miu_read(effi_bank, REG_BIST_CHIPTOP_STATUS) & 0xE000; // bit13 ~ bit15
    }
    else
    {
        return reg_miu_read(effi_bank, REG_BIST_GROUP_RESULT) & 0x0007; // bit0 ~ bit2
    }
}

bool reg_miu_bist_is_finish(u32 effi_bank)
{
    if (reg_miu_bist_is_chip_top(effi_bank))
    {
        if (reg_miu_bist_chiptop(effi_bank)->loop) // chiptop loop have not finish flag
        {
            return !reg_miu_bist_chiptop(effi_bank)->enable;
        }
        return reg_miu_bist_chiptop(effi_bank)->finish;
    }
    else
    {
        return reg_miu_bist_group_result(effi_bank)->finish;
    }

    return true;
}

bool reg_miu_bist_is_success(u32 effi_bank)
{
    if (reg_miu_bist_is_chip_top(effi_bank))
    {
        return reg_miu_bist_chiptop(effi_bank)->fail == 0;
    }
    else
    {
        return reg_miu_bist_group_result(effi_bank)->fail == 0;
    }
    return true;
}

//=================================================================================================
//                                     REG MIU BW Function
//=================================================================================================
void reg_miu_bwla_reset(void)
{
    miu_riu_mcg_enable();
    reg_miu_set(BASE_REG_MIU_PA, 0x5A, BIT4); // [3] int mask
    miu_riu_mcg_disable();
}

void reg_miu_bwla_init(void)
{
    miu_riu_mcg_enable();
    // unmask
    reg_miu_write(BASE_REG_MIU_PA, 0x42, 0x2a20);
    reg_miu_write(BASE_REG_MIU_PA, 0x1F, 0x0001);

    reg_miu_write(BASE_REG_MIU_PA, 0x59, 0x3000); // [13]icg enable
    reg_miu_write(BASE_REG_MIU_PA, 0x5A, 0x0000); // [3] int mask

    // calculate refresh time
    reg_miu_write_mask(BASE_REG_MIU_E_PA, 0x45, BIT12, BIT12);

    miu_riu_mcg_disable();
}

void reg_miu_bwla_set_mode(int mode)
{
    miu_riu_mcg_enable();
    reg_miu_write(BASE_REG_MIU_PA, 0x51, (mode << 4)); // [0] mode
    miu_riu_mcg_disable();
}

void reg_miu_bwla_set_addr(ss_miu_addr_t addr)
{
    miu_riu_mcg_enable();
    reg_miu_write(BASE_REG_MIU_PA, 0x52, (addr >> 9) & 0xffff);
    reg_miu_write(BASE_REG_MIU_PA, 0x53, (addr >> 25));
    miu_riu_mcg_disable();
}

void reg_miu_bwla_set_round(int round)
{
    miu_riu_mcg_enable();
    if (round)
        reg_miu_write(BASE_REG_MIU_PA, 0x54, round - 1);
    miu_riu_mcg_disable();
}

void reg_miu_bwla_set_enable(bool enable)
{
    miu_riu_mcg_enable();
    reg_miu_write(BASE_REG_MIU_PA, 0x50, enable);
    miu_riu_mcg_disable();
}

void reg_miu_bwla_set_ip(int index, u16 id)
{
    miu_riu_mcg_enable();
    if (index % 2)
        reg_miu_write_mask(BASE_REG_MIU_PA, (0x60 + (index / 2)), 0xFF00, (id << 8));
    else
        reg_miu_write_mask(BASE_REG_MIU_PA, (0x60 + (index / 2)), 0x00FF, (id & 0x00FF));
    miu_riu_mcg_disable();
}

bool reg_miu_bwla_is_done(void)
{
    bool ret;

    miu_riu_mcg_enable();
    ret = (reg_miu_read(BASE_REG_MIU_PA, 0x5A) & 0x14);
    miu_riu_mcg_disable();

    return ret;
}

#endif // defined(CAM_OS_LINUX_KERNEL)
