/*
 * hal_miu_if.c - Sigmastar
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
#if defined(CAM_OS_LINUX_KERNEL)
#include <linux/slab.h>
#include <linux/sched/clock.h>
#elif defined(CAM_OS_RTK)
#include <string.h>
#endif
#include "drv_miu.h"
#include "hal_miu.h"
#include "hal_miu_if.h"

#if defined(CAM_OS_RTK)
#define EINVAL    1
#define max(x, y) (x) > (y) ? (x) : (y)
#define min(x, y) (x) > (y) ? (y) : (x)
#endif

#define HAL_MIU_PROTECT_KERNEL_WHITELIST                                                                          \
    {                                                                                                             \
        MIU_CLIENTW_CPU_CA32, MIU_CLIENTW_XZ_DECODE, MIU_CLIENTW_XZ_DECODE2, MIU_CLIENTW_BDMA, MIU_CLIENTW_BDMA2, \
            MIU_CLIENTW_LOW_SPEED1, MIU_CLIENTW_SD, MIU_CLIENTW_USB20, MIU_CLIENTW_LOW_SPEED0,                    \
            MIU_CLIENTW_RIU_RECODER, MIU_CLIENTW_PMIMI, MIU_CLIENTW_AESDMA, MIU_CLIENTW_MIIC, MIU_CLIENTW_EMAC,   \
            MIU_CLIENTW_EMAC1, MIU_CLIENTW_BWLA, MIU_CLIENTW_BACH, MIU_CLIENTW_NULL                               \
    }

#define HAL_MIU_PROTECT_KERNEL_BLACKLIST \
    {                                    \
        MIU_CLIENTW_NULL                 \
    }

// update time: 2023/11/08
static struct hal_miu_ip __ip_table[] = {
    // Group SC0
    {"OVERALL", 0x00, true, true},
    {"XZ_DECODE", 0x01, false, true},
    {"XZ_DECODE2", 0x02, true, true},
    {"BACH", 0x03, true, true},
    {"CMDQ", 0x04, true, false},
    {"BDMA", 0x05, true, true},
    {"BDMA2", 0x06, true, true},
    {"JPE", 0x07, true, true},
    {"LOW_SPEED1", 0x08, true, true},
    {"SD", 0x09, true, true},
    {"CMDQ1", 0x0A, true, false},
    {"USB20", 0x0B, true, true},
    {"GOP_SC0", 0x0C, true, false},
    {"LOW_SPEED0", 0x0D, true, true},
    {"RIU_RECODER", 0x0E, false, true},
    {"BIST0", 0x0F, true, true},

    // Group SC1
    {"PMIMI", 0x10, true, true},
    {"VENC0_CODEC", 0x11, true, true},
    {"VENC0_SAD", 0x12, true, true},
    {"ISP0_CMDQ", 0x13, true, false},
    {"AESDMA", 0x14, true, true},
    {"LDC", 0x15, true, true},
    {"SC_WDMA0", 0x16, false, true},
    {"SC_RDMA0_WDMA2", 0x17, true, true},
    {"MIIC", 0x18, true, true},
    {"SC_RDMA1", 0x19, true, false},
    {"SC_WDMA1", 0x1A, false, true},
    {"GOP_JPE0", 0x1B, true, false},
    {"IVE", 0x1C, true, true},
    {"EMAC", 0x1D, true, true},
    {"EMAC1", 0x1E, true, true},
    {"BIST1", 0x1F, true, true},

    // Group ISP0
    {"ISP_3DNR", 0x20, true, true},
    {"ISP_DMAG0", 0x21, true, true},
    {"ISP_DMAG1", 0x22, false, true},
    {"ISP_STA", 0x23, false, true},
    {"ISP_IMG", 0x24, false, true},
    {"ISP_ROT", 0x25, true, true},
    {"ISP_WDR", 0x26, true, true},
    {"ISP_IIR", 0x27, true, true},
    {"ISP_MLOAD", 0x28, true, false},
    {"ISP_LDC", 0x29, true, false},
    {"BIST2", 0x2F, true, true},

    // High Way
    {"IPU", 0x71, true, true, true},
    {"CPU_CA32", 0x72, true, true, true},
    {"BWLA", 0x75, true, true, true},
};

static u16 PA_HIGH_BL[HAL_MIU_MAX_QOS_LEVEL] = {0x02, 0x04, 0x4, 0x08, 0x08, 0x10, 0x08, 0x10};

static u16 GARB_BL[4]   = {0x14, 0x10, 0x0c, 0x08};
static u16 PARB_BL      = 0x20;
static u16 PA_NORMAL_BL = 0x80;
static u8  g_dbg_en     = 1;

// static u32 g_grpqos_sum[GRP_ARB_NUM] = {0, 0, 0};
static bool g_arb_qos_initialize = 0;
//=================================================================================================
//                                     Hal MIU Common Function
//=================================================================================================

bool hal_miu_mem_alloc(struct hal_miu_mem* mem)
{
#if defined(CAM_OS_LINUX_KERNEL)
    struct page* page;
#endif

    if (!mem || !mem->size)
    {
        hal_miu_err("mem alloc failed for size\r\n");
        return false;
    }

    if (mem->phy && mem->virt)
    {
        return true;
    }

    mem->flag = false;
    mem->phy  = CamOsContiguousMemAlloc(mem->size);

    if (!mem->phy)
    {
#if defined(CAM_OS_LINUX_KERNEL)
        mem->virt = kmalloc(mem->size, GFP_KERNEL);
        if (!mem->virt)
        {
            return false;
        }
        page      = virt_to_page(mem->virt);
        mem->phy  = page_to_phys(page);
        mem->flag = true;
#elif defined(CAM_OS_RTK)
        return false;
#endif
    }
    else
    {
        mem->virt = CamOsMemMap(mem->phy, mem->size, 0);
    }

    mem->miu = CamOsMemPhysToMiu(mem->phy);

    return true;
}

void hal_miu_mem_free(struct hal_miu_mem* mem)
{
    int ret;

    if (!mem || (!mem->phy && !mem->virt))
    {
        return;
    }

#if defined(CAM_OS_LINUX_KERNEL)
    if (mem->flag)
    {
        kfree(mem->virt);
    }
    else
#endif
    {
        CamOsMemUnmap(mem->virt, mem->size);

        ret = CamOsContiguousMemRelease(mem->phy);
        if (ret != 0)
            hal_miu_err("%s %d: Memory release fail\r\n", __FUNCTION__, __LINE__);
    }

    mem->phy  = 0;
    mem->miu  = 0;
    mem->virt = NULL;
}

//#if defined(CAM_OS_LINUX_KERNEL)
struct hal_miu_float hal_miu_divide(u64 numerator, u64 denominator)
{
    struct hal_miu_float res = {numerator * 100 / denominator, numerator * 100000 / denominator};

    // The 64-bit remainder operation will be wrong, it is fixed in commit 315106,
    // so avoid using the remainder operation here
    res.decimal -= res.integer * 1000;

    return res;
}

#if defined(CAM_OS_LINUX_KERNEL)
static int hal_miu_tabulate_init(struct hal_miu_tabulate* table, char* buf, char* end, char* header)
{
    int i;
    int j   = 0;
    int col = 0;

    table->buf    = buf;
    table->end    = end;
    table->header = header;

    for (i = 1; header[i] != '\0'; i++)
    {
        col += header[i] == '|';
    }

    table->column = col;
    table->cell   = CamOsMemAlloc(col * sizeof(int));
    if (!table->cell)
    {
        return -1;
    }
    CamOsMemset(table->cell, 0, col * sizeof(int));

    col = 0;
    for (i = 1; header[i] != '\0'; i++)
    {
        if (header[i] == '|')
        {
            table->cell[col++] = i - j - 1;
            j                  = i;
        }
    }

    table->width = strlen(header) + 2; // resverd for '\n' and '\0'
    table->split = CamOsMemAlloc(table->width);
    if (!table->split)
    {
        CamOsMemRelease(table->cell);
        return -1;
    }

    table->row = CamOsMemAlloc(table->width);
    if (!table->row)
    {
        CamOsMemRelease(table->cell);
        CamOsMemRelease(table->split);
        return -1;
    }

    j = 0;
    for (i = 0; i < table->column; i++)
    {
        table->split[j++] = '+';
        col               = table->cell[i];
        while (col--)
        {
            table->split[j++] = '-';
        }
    }
    table->split[j++] = '+';
    table->split[j++] = '\0';

    table->buf += CamOsSnprintf(table->buf, table->end - table->buf, "%s\n", table->split);
    table->buf += CamOsSnprintf(table->buf, table->end - table->buf, "%s\n", header);
    table->buf += CamOsSnprintf(table->buf, table->end - table->buf, "%s\n", table->split);

    return 0;
}

static char* hal_miu_tabulate_finish(struct hal_miu_tabulate* table)
{
    table->buf += CamOsSnprintf(table->buf, table->end - table->buf, "%s\n", table->split);

    if (table->split)
    {
        CamOsMemRelease(table->split);
    }

    if (table->cell)
    {
        CamOsMemRelease(table->cell);
    }

    if (table->row)
    {
        CamOsMemRelease(table->row);
    }

    return table->buf;
}

static int hal_miu_tabulate_add_row_real(struct hal_miu_tabulate* table, const char* fmt, va_list args)
{
    int   i = 0;
    char  format[10];
    char* now  = table->row;
    char* last = now;

    vsnprintf(table->row, table->width, fmt, args);

    if (*now == '|')
    {
        now++;
        last++;
    }

    while (*now)
    {
        if (*now == '|')
        {
            *now = '\0';
            CamOsSprintf(format, "| %%-%ds ", table->cell[i++] - 2);
            table->buf += CamOsSnprintf(table->buf, table->end - table->buf, format, last);
            last = now + 1;
        }
        now++;
    }

    table->buf += CamOsSnprintf(table->buf, table->end - table->buf, "|\n");
    return 0;
}

static char* hal_miu_tabulate_add_row(struct hal_miu_tabulate* table, const char* fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    hal_miu_tabulate_add_row_real(table, fmt, args);
    va_end(args);

    return table->buf;
}
#endif // defined(CAM_OS_LINUX_KERNEL)

//=================================================================================================
//                                     Hal MIU Client Function
//=================================================================================================
static int hal_miu_client_id_valid(struct hal_miu_client* client, int id, enum hal_miu_rw_mode rw)
{
    struct hal_miu_ip* ip;

    for_each_ip(client, ip)
    {
        if (ip->id == id)
        {
            if (rw == HAL_MIU_RW || (rw == HAL_MIU_RO && ip->read) || (rw == HAL_MIU_WO && ip->write))
            {
                return ip->id;
            }
        }
    }

    return -1;
}

static char* hal_miu_client_id_to_name(struct hal_miu_client* client, int id, enum hal_miu_rw_mode rw)
{
    struct hal_miu_ip* ip;

    for_each_ip(client, ip)
    {
        if (ip->id == id)
        {
            if (rw == HAL_MIU_RW || (rw == HAL_MIU_RO && ip->read) || (rw == HAL_MIU_WO && ip->write))
            {
                return ip->name;
            }
        }
    }

    return NULL;
}

static int hal_miu_client_name_to_id(struct hal_miu_client* client, char* name, enum hal_miu_rw_mode rw)
{
    struct hal_miu_ip* ip;

    for_each_ip(client, ip)
    {
        if (!strcmp(name, ip->name))
        {
            if (rw == HAL_MIU_RW || (rw == HAL_MIU_RO && ip->read) || (rw == HAL_MIU_WO && ip->write))
            {
                return ip->id;
            }
        }
    }

    return -1;
}

static struct hal_miu_ip* hal_miu_client_id_to_ip(struct hal_miu_client* client, int id, enum hal_miu_rw_mode rw)
{
    struct hal_miu_ip* ip;

    for_each_ip(client, ip)
    {
        if (ip->id == id)
        {
            if (rw == HAL_MIU_RW || (rw == HAL_MIU_RO && ip->read) || (rw == HAL_MIU_WO && ip->write))
            {
                return ip;
            }
        }
    }

    return NULL;
}

static int hal_miu_client_module_reset(struct hal_miu_client* client, int id, enum hal_miu_rw_mode rw)
{
    struct hal_miu_ip* ip;
    int                ret = -1;

    if ((id == 0x72) || (id == 0x75))
    {
        // CPU_CA32/BWLA not support
        return ret;
    }

    for_each_ip(client, ip)
    {
        if (ip->id == id)
        {
            if (rw == HAL_MIU_RW || (rw == HAL_MIU_RO && ip->read) || (rw == HAL_MIU_WO && ip->write))
            {
                ret = reg_miu_client_module_reset(ip->id, rw == HAL_MIU_WO);
                if (ret == -1)
                    hal_miu_err("client id %X miu module reset failed\r\n", id);
                return ret;
            }
        }
    }

    return -1;
}

#if defined(CAM_OS_LINUX_KERNEL)
int hal_miu_client_show_ip_table(struct hal_miu_client* client, char* buf, char* end)
{
    char*                   str = buf;
    struct hal_miu_ip*      ip;
    struct hal_miu_tabulate table;

    hal_miu_tabulate_init(&table, buf, end, "| ID   | read                | write               |");
    for_each_ip(client, ip)
    {
        hal_miu_tabulate_add_row(&table, "|%#04x|%s|%s|", ip->id, ip->read ? ip->name : " ",
                                 ip->write ? ip->name : " ");
    }
    str = hal_miu_tabulate_finish(&table);

    return (str - buf);
}

static int hal_miu_client_mask(struct hal_miu_ip* ip, enum hal_miu_rw_mode rw, const char* buf, u32 n)
{
    int enable;

#if defined(CAM_OS_LINUX_KERNEL)
    if (sscanf(buf, "%d", &enable) != 1)
    {
        return -EINVAL;
    }
#elif defined(CAM_OS_RTK)
    if (CamOsSscanf(buf, "%d", &enable) != 1)
    {
        return -EINVAL;
    }
#endif

    if (ip->high_way)
    {
        reg_miu_client_set_mask_high(ip->id, rw == HAL_MIU_WO, enable);
    }
    else
    {
        reg_miu_client_set_mask_normal(ip->id, rw == HAL_MIU_WO, enable);
    }

    if (rw == HAL_MIU_WO)
        ip->write_attr.mask = enable;
    else
        ip->read_attr.mask = enable;

    hal_miu_info("0x%x:%s %s mask=%d\n", ip->id, ip->name, hal_miu_rw_str(rw), enable);

    return 0;
}

static int hal_miu_client_burst(struct hal_miu_ip* ip, enum hal_miu_rw_mode rw, const char* buf, u32 n)
{
    u32 burst;

    if (ip->high_way)
    {
#if defined(CAM_OS_LINUX_KERNEL)
        if (sscanf(buf, "%x", &burst) != 1 || burst > HAL_MIU_MAX_BURST_HIGH)
        {
            hal_miu_info("vaild range: 0x0 <= burst <= %#x\n", HAL_MIU_MAX_BURST_HIGH);
            return -EINVAL;
        }
#elif defined(CAM_OS_RTK)
        if (CamOsSscanf(buf, "%x", &burst) != 1 || burst > HAL_MIU_MAX_BURST_HIGH)
        {
            hal_miu_info("vaild range: 0x0 <= burst <= %#x\n", HAL_MIU_MAX_BURST_HIGH);
            return -EINVAL;
        }
#endif

        reg_miu_client_set_burst_high(ip->id, rw == HAL_MIU_WO, burst);
    }
    else
    {
#if defined(CAM_OS_LINUX_KERNEL)
        if (sscanf(buf, "%x", &burst) != 1 || burst > HAL_MIU_MAX_BURST_NORMAL)
        {
            hal_miu_info("vaild range: 0x0 <= burst <= %#x\n", HAL_MIU_MAX_BURST_NORMAL);
            return -EINVAL;
        }
#elif defined(CAM_OS_RTK)
        if (CamOsSscanf(buf, "%x", &burst) != 1 || burst > HAL_MIU_MAX_BURST_NORMAL)
        {
            hal_miu_info("vaild range: 0x0 <= burst <= %#x\n", HAL_MIU_MAX_BURST_NORMAL);
            return -EINVAL;
        }
#endif

        reg_miu_client_set_burst_sel_normal(ip->id, rw == HAL_MIU_WO, burst);
    }

    if (rw == HAL_MIU_WO)
        ip->write_attr.limit = burst;
    else
        ip->read_attr.limit = burst;

    hal_miu_info("0x%x:%s %s burst=%u\n", ip->id, ip->name, hal_miu_rw_str(rw), burst);

    return 0;
}

static int hal_miu_client_priority(struct hal_miu_ip* ip, enum hal_miu_rw_mode rw, const char* buf, u32 n)
{
    u32 priority;

    if (ip->high_way)
    {
#if defined(CAM_OS_LINUX_KERNEL)
        if (sscanf(buf, "%x", &priority) != 1 || priority > HAL_MIU_MAX_PRIORITY_HIGH)
        {
            hal_miu_info("vaild range: 0x0 <= priority <= %#x\n", HAL_MIU_MAX_PRIORITY_HIGH);
            return -EINVAL;
        }
#elif defined(CAM_OS_RTK)
        if (CamOsSscanf(buf, "%x", &priority) != 1 || priority > HAL_MIU_MAX_PRIORITY_HIGH)
        {
            hal_miu_info("vaild range: 0x0 <= priority <= %#x\n", HAL_MIU_MAX_PRIORITY_HIGH);
            return -EINVAL;
        }
#endif

        reg_miu_client_set_priority_high(ip->id, rw == HAL_MIU_WO, priority);
    }
    else
    {
#if defined(CAM_OS_LINUX_KERNEL)
        if (sscanf(buf, "%x", &priority) != 1 || priority > HAL_MIU_MAX_PRIORITY_NORMAL)
        {
            hal_miu_info("vaild range: 0x0 <= priority <= %#x\n", HAL_MIU_MAX_PRIORITY_NORMAL);
            return -EINVAL;
        }
#elif defined(CAM_OS_RTK)
        if (CamOsSscanf(buf, "%x", &priority) != 1 || priority > HAL_MIU_MAX_PRIORITY_NORMAL)
        {
            hal_miu_info("vaild range: 0x0 <= priority <= %#x\n", HAL_MIU_MAX_PRIORITY_NORMAL);
            return -EINVAL;
        }
#endif
        reg_miu_client_set_priority_normal(ip->id, rw == HAL_MIU_WO, priority);
    }

    if (rw == HAL_MIU_WO)
        ip->write_attr.priority = priority;
    else
        ip->read_attr.priority = priority;

    hal_miu_info("0x%x:%s %s priority=%u\n", ip->id, ip->name, hal_miu_rw_str(rw), priority);
    return 0;
}
#endif

static int hal_miu_client_vp(struct hal_miu_ip* ip, enum hal_miu_rw_mode rw, const char* buf, u32 n)
{
    u32 vp;
#if defined(CAM_OS_LINUX_KERNEL)
    if (sscanf(buf, "%x", &vp) != 1 || vp > (HAL_MIU_MAX_VP + 1))
    {
        hal_miu_info("vaild range:  0x0 <= vp <= %#x\n", HAL_MIU_MAX_VP + 1);
        return -EINVAL;
    }
#elif defined(CAM_OS_RTK)
    if (CamOsSscanf(buf, "%x", &vp) != 1 || vp > (HAL_MIU_MAX_VP + 1))
    {
        hal_miu_info("vaild range:  0x0 <= vp <= %#x\n", HAL_MIU_MAX_VP + 1);
        return -EINVAL;
    }
#endif

    reg_miu_client_set_vp(ip->id, rw == HAL_MIU_WO, vp);

    if (rw == HAL_MIU_WO)
        ip->write_attr.vp = vp;
    else
        ip->read_attr.vp = vp;

    if (g_dbg_en)
        hal_miu_info("0x%x:%s %s vp=%u\n", ip->id, ip->name, hal_miu_rw_str(rw), vp);

    return 0;
}

#if defined(CAM_OS_LINUX_KERNEL)
static int hal_miu_client_flowctrl_check(struct hal_miu_ip* ip, int mask_period, int pass_period)
{
    if (ip->high_way)
    {
        if (mask_period < 1 || mask_period > HAL_MIU_MAX_MASK_PERIOD_HIGH || pass_period < 0
            || pass_period > HAL_MIU_MAX_PASS_PERIOD_HIGH)
        {
            hal_miu_info("vaild range: 0 < mask_period <= %#x, 0 <= pass_period <= %#x\n", HAL_MIU_MAX_MASK_PERIOD_HIGH,
                         HAL_MIU_MAX_PASS_PERIOD_HIGH);
            return -EINVAL;
        }
    }
    else
    {
        if (mask_period < 1 || mask_period > HAL_MIU_MAX_MASK_PERIOD_NORMAL || pass_period < 0
            || pass_period > HAL_MIU_MAX_PASS_PERIOD_NORMAL)
        {
            hal_miu_info("vaild range: 0 < mask_period <= %#x, 0 <= pass_period <= %#x\n",
                         HAL_MIU_MAX_MASK_PERIOD_NORMAL, HAL_MIU_MAX_PASS_PERIOD_NORMAL);
            return -EINVAL;
        }
    }

    return 0;
}

static int hal_miu_client_flowctrl(struct hal_miu_ip* ip, enum hal_miu_rw_mode rw, const char* buf, u32 n)
{
    u32 ret         = 0;
    u32 enable      = 1;
    u32 mask_period = 0;
    u32 pass_period = 0;

#if defined(CAM_OS_LINUX_KERNEL)
    sscanf(buf, "%d", &enable);
#elif defined(CAM_OS_RTK)
    CamOsSscanf(buf, "%d", &enable);
#endif

    if (enable)
    {
#if defined(CAM_OS_LINUX_KERNEL)
        if (sscanf(buf, "%d%x%x", &enable, &mask_period, &pass_period) != 3)
#elif defined(CAM_OS_RTK)
        if (CamOsSscanf(buf, "%d%x%x", &enable, &mask_period, &pass_period) != 3)
#endif
        {
            return -EINVAL;
        }

        if (hal_miu_client_flowctrl_check(ip, mask_period, pass_period) < 0)
        {
            return -EINVAL;
        }

        hal_miu_info("0x%x:%s %s flowctrl mask_period=0x%x, pass_period=%#x\n", ip->id, ip->name, hal_miu_rw_str(rw),
                     mask_period, pass_period);
    }
    else
    {
        hal_miu_info("0x%x:%s %s flowctrl off\n", ip->id, ip->name, hal_miu_rw_str(rw));
    }

    if (ip->high_way)
    {
        ret = reg_miu_client_set_flowctrl_high(ip->id, rw == HAL_MIU_WO, enable, mask_period, pass_period);
    }
    else
    {
        ret = reg_miu_client_set_flowctrl_normal(ip->id, rw == HAL_MIU_WO, enable, mask_period, pass_period);
    }

    if (ret < 0)
    {
        hal_miu_info("[flowctrl] not empty flowctrl slot!\n");
    }

    if (rw == HAL_MIU_WO)
    {
        ip->write_attr.mask_period = mask_period;
        ip->write_attr.pass_period = pass_period;
    }
    else
    {
        ip->read_attr.mask_period = mask_period;
        ip->read_attr.pass_period = pass_period;
    }

    hal_miu_info("0x%x:%s %s enable=%d, mask_period=%x, pass_period=%x\n", ip->id, ip->name, hal_miu_rw_str(rw), enable,
                 mask_period, pass_period);

    return ret;
}

static int hal_miu_client_urgent(struct hal_miu_ip* ip, enum hal_miu_rw_mode rw, const char* buf, u32 n)
{
    u32 enable;

#if defined(CAM_OS_LINUX_KERNEL)
    sscanf(buf, "%d", &enable);
#elif defined(CAM_OS_RTK)
    CamOsSscanf(buf, "%d", &enable);
#endif

    if (ip->high_way)
    {
        reg_miu_client_set_urgent_high(ip->id, rw == HAL_MIU_WO, !enable);
    }
    else
    {
        reg_miu_client_set_urgent_normal(ip->id, rw == HAL_MIU_WO, !enable);
    }

    if (rw == HAL_MIU_WO)
        ip->write_attr.urgent = !enable;
    else
        ip->read_attr.urgent = !enable;

    hal_miu_info("0x%x:%s %s urgent(hp)=%d\n", ip->id, ip->name, hal_miu_rw_str(rw), enable);

    return 0;
}

static int hal_miu_client_stall(struct hal_miu_ip* ip, enum hal_miu_rw_mode rw, const char* buf, u32 n)
{
    u32 enable;

#if defined(CAM_OS_LINUX_KERNEL)
    sscanf(buf, "%d", &enable);
#elif defined(CAM_OS_RTK)
    CamOsSscanf(buf, "%d", &enable);
#endif

    reg_miu_client_set_stall(ip->id, rw == HAL_MIU_WO, enable);

    hal_miu_info("0x%x:%s %s stall enable=%d\n", ip->id, ip->name, hal_miu_rw_str(rw), enable);

    return 0;
}
#endif

static int hal_miu_client_qos(struct hal_miu_ip* ip, enum hal_miu_rw_mode rw, const char* buf, u32 n)
{
    u32                i, qos_sum, qos_num;
    u32                qos, priority, burst_sel;
    u8                 idx_start, idx_end;
    struct hal_miu_ip* ip_tmp;

#if defined(CAM_OS_LINUX_KERNEL)
    sscanf(buf, "%d", &qos);
#elif defined(CAM_OS_RTK)
    CamOsSscanf(buf, "%d", &qos);
#endif

    if (qos < 0 || qos > (HAL_MIU_MAX_QOS_LEVEL - 1))
    {
        hal_miu_err("vaild range: 0 <= qos <= %#x\n", HAL_MIU_MAX_QOS_LEVEL - 1);
        return -EINVAL;
    }

    if (ip->high_way)
    {
        reg_miu_client_set_burst_high(ip->id, rw == HAL_MIU_WO, PA_HIGH_BL[qos]);
        if (qos > 7)
            priority = 0;
        else
            priority = (7 - qos) >> 1;
        reg_miu_client_set_priority_high(ip->id, rw == HAL_MIU_WO, priority);
    }
    else
    {
        switch (qos)
        {
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
                priority  = 0;
                burst_sel = 0;
                break;
            case 6:
                priority  = 0;
                burst_sel = 1;
                break;
            case 5:
                priority  = 1;
                burst_sel = 0;
                break;
            case 4:
                priority  = 1;
                burst_sel = 1;
                break;
            case 3:
                priority  = 2;
                burst_sel = 1;
                break;
            case 2:
                priority  = 2;
                burst_sel = 2;
                break;
            case 1:
                priority  = 3;
                burst_sel = 2;
                break;
            case 0:
                priority  = 3;
                burst_sel = 3;
                break;
        }
        // GARB priority/burst select
        reg_miu_client_set_priority_normal(ip->id, rw == HAL_MIU_WO, priority);
        reg_miu_client_set_burst_sel_normal(ip->id, rw == HAL_MIU_WO, burst_sel);

        // PARB priority/burst
        // calculate average qos of group
        qos_sum   = 0;
        qos_num   = 0;
        idx_start = 0;
        idx_end   = 0;
        switch (GROUP(ip->id))
        {
            case 0:
                idx_start = 0x00;
                idx_end   = 0x10;
                break;
            case 1:
                idx_start = 0x10;
                idx_end   = 0x20;
                break;
            case 2:
                idx_start = 0x20;
                idx_end   = 0x2B;
                break;
        }

        for (i = idx_start; i < idx_end; i++)
        {
            ip_tmp = (struct hal_miu_ip*)&__ip_table[i];
            // printk("%s, %d: i=%x, id=%x\r\n", __FUNCTION__, __LINE__, i, ip_tmp->id);
            // if (ip_tmp->id != i)
            //     continue;

            if (ip_tmp->id == ip->id)
            {
                if (rw == HAL_MIU_WO)
                {
                    if (ip_tmp->write)
                    {
                        qos_sum += qos;
                        qos_num++;
                    }
                }
                else
                {
                    if (ip_tmp->read)
                    {
                        qos_sum += qos;
                        qos_num++;
                    }
                }
            }
            else
            {
                if (rw == HAL_MIU_WO)
                {
                    if (ip_tmp->write)
                    {
                        qos_sum += ip_tmp->write_attr.qos;
                        qos_num++;
                    }
                }
                else
                {
                    if (ip_tmp->read)
                    {
                        qos_sum += ip_tmp->read_attr.qos;
                        qos_num++;
                    }
                }
            }
        }

        qos_sum = qos_sum / qos_num;
        // hal_miu_info("rw=%d, qos_sum=%d, qos_num=%d\n", rw, qos_sum, qos_num);
        if (qos_sum > 6)
            reg_miu_client_set_parb_priority_normal(ip->id, rw == HAL_MIU_WO, 0x00);
        else if ((qos_sum <= 6) && (qos_sum > 3))
            reg_miu_client_set_parb_priority_normal(ip->id, rw == HAL_MIU_WO, 0x01);
        else if ((qos_sum <= 3) && (qos_sum > 1))
            reg_miu_client_set_parb_priority_normal(ip->id, rw == HAL_MIU_WO, 0x02);
        else
            reg_miu_client_set_parb_priority_normal(ip->id, rw == HAL_MIU_WO, 0x03);
        reg_miu_client_set_parb_burst_normal(ip->id, rw == HAL_MIU_WO, PARB_BL);

        // pa burst for normal way
        reg_miu_client_set_burst_high(0x00, rw == HAL_MIU_WO, PA_NORMAL_BL);
        reg_miu_client_set_priority_high(0x00, rw == HAL_MIU_WO, 0x01);

        // set vp for qos 8~11
        if (qos == 8)
            hal_miu_client_vp(ip, rw, "4", 1);
        else if (qos == 9)
            hal_miu_client_vp(ip, rw, "3", 1);
        else if (qos == 10)
            hal_miu_client_vp(ip, rw, "2", 1);
        else if (qos == 11)
            hal_miu_client_vp(ip, rw, "1", 1);
        else
            hal_miu_client_vp(ip, rw, "0", 1);
    }

    if (rw == HAL_MIU_WO)
        ip->write_attr.qos = qos;
    else
        ip->read_attr.qos = qos;

    if (g_dbg_en)
        hal_miu_info("0x%x:%s %s qos=%d\n", ip->id, ip->name, hal_miu_rw_str(rw), qos);

    return 0;
}

static int hal_miu_arb_qos_initialize(struct hal_miu_ip* ip, enum hal_miu_rw_mode rw, const char* buf, u32 n)
{
    u32                i, j, ip_num;
    char               qos_buf[8];
    char*              buf_tmp = qos_buf;
    struct hal_miu_ip* ip_tmp;

    if (g_arb_qos_initialize)
    {
        hal_miu_info("qos already initialize\n");
        return 0;
    }
    g_dbg_en = 0;

    // set garb burst, 0:0x14, 1:0x10, 2:0x0c, 3:0x08
    for (i = 0; i < GRP_ARB_NUM; i++)
    {
        for (j = 0; j < HAL_MIU_MAX_GRP_BURST_NUM; j++)
        {
            reg_miu_client_set_burst_opt_normal(i, false, j, GARB_BL[j]);
            reg_miu_client_set_burst_opt_normal(i, true, j, GARB_BL[j]);
        }
    }

    // set parb burst0~2
    reg_miu_client_set_parb_burst_normal(0x00, false, PARB_BL);
    reg_miu_client_set_parb_burst_normal(0x00, true, PARB_BL);
    reg_miu_client_set_parb_burst_normal(0x10, false, PARB_BL);
    reg_miu_client_set_parb_burst_normal(0x10, true, PARB_BL);
    reg_miu_client_set_parb_burst_normal(0x20, false, PARB_BL);
    reg_miu_client_set_parb_burst_normal(0x20, true, PARB_BL);

    ip_num = sizeof(__ip_table) / sizeof(__ip_table[0]);
    for (i = 1; i < ip_num; i++)
    {
        ip_tmp = (struct hal_miu_ip*)&__ip_table[i];
        if (ip_tmp->id == 0x72 || ip_tmp->id == 0x75)
        {
            ip_tmp->read_attr.qos  = 5;
            ip_tmp->write_attr.qos = 5;
        }
        else if (ip_tmp->id == 0x16 || ip_tmp->id == 0x17 || ip_tmp->id == 0x19 || ip_tmp->id == 0x1a)
        {
            // scl module
            ip_tmp->read_attr.qos  = 7;
            ip_tmp->write_attr.qos = 7;
        }
        else if ((ip_tmp->id >= 0x20) && (ip_tmp->id <= 0x2E))
        {
            // isp module
            ip_tmp->read_attr.qos  = 7;
            ip_tmp->write_attr.qos = 7;
        }
        else
        {
            ip_tmp->read_attr.qos  = 3;
            ip_tmp->write_attr.qos = 3;
        }

        if (ip_tmp->read)
        {
            buf_tmp = qos_buf;
            CamOsSprintf(buf_tmp, "%d ", ip_tmp->read_attr.qos);
            // hal_miu_info("i=%d read, ip name=%s, id=%x qos=%s\n", i, ip_tmp->name, ip_tmp->id, buf_tmp);
            hal_miu_client_qos(ip_tmp, HAL_MIU_RO, buf_tmp, strlen(buf_tmp));
        }

        if (ip_tmp->write)
        {
            buf_tmp = qos_buf;
            CamOsSprintf(buf_tmp, "%d ", ip_tmp->write_attr.qos);
            hal_miu_client_qos(ip_tmp, HAL_MIU_WO, buf_tmp, strlen(buf_tmp));
        }
    }

    g_dbg_en             = 1;
    g_arb_qos_initialize = 0;
    hal_miu_info("Do qos-initialize\n");

    return 0;
}

#if defined(CAM_OS_RTK)
int hal_miu_qos_init(void)
{
    hal_miu_arb_qos_initialize(0, 0, 0, 0);
    return 0;
}
#endif

#if defined(CAM_OS_LINUX_KERNEL)
static void hal_miu_client_mem_free(struct hal_miu_client* client)
{
    if (client->func_buf)
    {
        CamOsMemRelease(client->func_buf);
        client->func_buf = NULL;
    }
}

static int hal_miu_client_mem_alloc(struct hal_miu_client* client)
{
    if (!client->func_buf)
    {
        client->func_buf = CamOsMemAlloc(HAL_MIU_CLIENT_BUF_MAX);
        if (!client->func_buf)
        {
            return -1;
        }
        CamOsMemset(client->func_buf, 0, HAL_MIU_CLIENT_BUF_MAX);
    }

    return 0;
}

static char* client_header1 =
    "+--------------------------+------------------------------------------------"
    "+------------------------------------------------+\n"
    "|                          |                    Read                        "
    "|                    Write                       |\n";

static char* client_header2 =
    "|   ID:Name                | mask | qos | burst | pri | vpr | flowctrl | hp "
    "| mask | qos | burst | pri | vpw | flowctrl | hp |";

static int hal_miu_client_format_attr(char* buf, bool enable, struct hal_miu_ip_attr* attr)
{
    if (enable)
    {
        return CamOsSprintf(buf, "%u|%u|%u|%u|%u|0x%02x/%x|%u|", attr->mask, attr->qos, attr->limit, attr->priority,
                            attr->vp, attr->mask_period, attr->pass_period, attr->urgent);
    }
    else
    {
        return CamOsSprintf(buf, "-|-|-|-|-|-|-|");
    }
}

static void hal_miu_client_read_attr(struct hal_miu_ip* ip, bool write, struct hal_miu_ip_attr* attr)
{
    if (ip->high_way)
    {
        attr->mask        = reg_miu_client_get_mask_high(ip->id, write);
        attr->urgent      = !reg_miu_client_get_urgent_high(ip->id, write);
        attr->priority    = reg_miu_client_get_priority_high(ip->id, write);
        attr->limit       = reg_miu_client_get_burst_high(ip->id, write);
        attr->mask_period = reg_miu_client_get_mask_period_high(ip->id, write);
        attr->pass_period = reg_miu_client_get_pass_period_high(ip->id, write);
    }
    else
    {
        attr->mask        = reg_miu_client_get_mask_normal(ip->id, write);
        attr->urgent      = !reg_miu_client_get_urgent_normal(ip->id, write);
        attr->priority    = reg_miu_client_get_priority_normal(ip->id, write);
        attr->limit       = reg_miu_client_get_burst_sel_normal(ip->id, write);
        attr->mask_period = reg_miu_client_get_mask_period_normal(ip->id, write);
        attr->pass_period = reg_miu_client_get_pass_period_normal(ip->id, write);
    }
    attr->vp = reg_miu_client_get_vp(ip->id, write);
}

static int hal_miu_client_read_all_attr(struct hal_miu_client* client)
{
    struct hal_miu_ip*      ip;
    struct hal_miu_tabulate table;
    char                    line[200];
    char*                   buf = line;
    char*                   func_buf;

    if (hal_miu_client_mem_alloc(client) < 0)
    {
        return -1;
    }

    func_buf = client->func_buf;
    func_buf += CamOsSprintf(func_buf, client_header1);
    hal_miu_tabulate_init(&table, func_buf, client->func_buf + HAL_MIU_CLIENT_BUF_MAX, client_header2);
    for_each_ip(client, ip)
    {
        hal_miu_client_read_attr(ip, false, &ip->read_attr);
        hal_miu_client_read_attr(ip, true, &ip->write_attr);
        buf = line;
        buf += CamOsSprintf(buf, "|0x%02x:%s|", ip->id, ip->name);
        buf += hal_miu_client_format_attr(buf, ip->read, &ip->read_attr);
        buf += hal_miu_client_format_attr(buf, ip->write, &ip->write_attr);
        hal_miu_tabulate_add_row(&table, line);
    }
    hal_miu_tabulate_finish(&table);

    return 0;
}

static int hal_miu_client_show(struct hal_miu_client* client, loff_t offset, char* buf, char* end)
{
    int ret  = 0;
    int size = 0;

    if (offset == 0)
    {
        ret = hal_miu_client_read_all_attr(client);
    }

    if (ret < 0 || !client->func_buf)
    {
        return 0;
    }

    if (offset < strlen(client->func_buf))
    {
        size = min((long long)(end - buf), strlen(client->func_buf) - offset);
        memcpy(buf, client->func_buf + offset, size);

        return size;
    }

    hal_miu_client_mem_free(client);

    return 0;
}

static int hal_miu_client_store(struct hal_miu_client* client, const char* buf, u32 n)
{
    return 0;
}
#endif

#if defined(CAM_OS_LINUX_KERNEL)
static struct hal_miu_client_func __ip_debug_func[] = {
    {"mask", "[0/1]", hal_miu_client_mask},
    {"burst", "[0/1/2/3]", hal_miu_client_burst},
    {"priority", "[0/1/2/3]", hal_miu_client_priority},
    {"vp", "[0/1/2/3/4]", hal_miu_client_vp},
    {"flowctrl", "[0/1] [mask_period] [pass_period]", hal_miu_client_flowctrl},
    {"urgent", "[0/1]", hal_miu_client_urgent},
    {"qos", "[0~7]", hal_miu_client_qos},
    {"stall", "[0/1]", hal_miu_client_stall},
    {"qos_init", "", hal_miu_arb_qos_initialize}, // need to put the end
};
#endif

int hal_miu_client_init(hal_miu_object obj, struct hal_miu_client* __client)
{
    struct hal_miu_client* client = (struct hal_miu_client*)obj;

    struct hal_miu_client temp_client =
    {
        .ip_num = sizeof(__ip_table) / sizeof(__ip_table[0]),
        .callback =
            {
                .id_valid   = hal_miu_client_id_valid,
                .id_to_name = hal_miu_client_id_to_name,
                .name_to_id = hal_miu_client_name_to_id,
                .id_to_ip   = hal_miu_client_id_to_ip,
#if defined(CAM_OS_LINUX_KERNEL)
                .show_ip_table = hal_miu_client_show_ip_table,
                .show_client   = hal_miu_client_show,
                .store_client  = hal_miu_client_store,
#endif
                .module_reset = hal_miu_client_module_reset,
            },
        .ip_table = __ip_table,
#if defined(CAM_OS_LINUX_KERNEL)
        .ip_func  = __ip_debug_func,
        .func_num = sizeof(__ip_debug_func) / sizeof(__ip_debug_func[0]),
        .func_buf = NULL,
#endif
    };

    *client = temp_client;

    return 0;
}

int hal_miu_client_free(hal_miu_object obj)
{
    return 0;
}

//=================================================================================================
//                                     Hal MIU DRAM Function
//=================================================================================================

int hal_miu_dram_info(struct miu_dram_info* info)
{
    info->size        = reg_miu_dram_get_size() << 20; // MB to Bytes
    info->dram_freq   = reg_miu_dram_get_freq();
    info->miupll_freq = reg_miu_dram_get_pll();
    info->type        = reg_miu_dram_get_type();
    info->data_rate   = reg_miu_dram_get_data_rate();
    info->bus_width   = reg_miu_dram_get_bus_width();
    info->ssc         = reg_miu_dram_get_ssc();

    return 0;
}

int hal_miu_dram_read_info(struct hal_miu_dram* dram)
{
    hal_miu_dram_info((struct miu_dram_info*)&dram->info);

    return 0;
}

int hal_miu_dram_show_info(struct hal_miu_dram* dram, char* buf, char* end)
{
    char* str = buf;

    buf += CamOsSnprintf(buf, end - buf, "%-13s", "DRAM Type:");
    switch (dram->info.type)
    {
        case HAL_MIU_DRAM_DDR4:
            buf += CamOsSnprintf(buf, end - buf, "DDR4\n");
            break;
        case HAL_MIU_DRAM_DDR3:
            buf += CamOsSnprintf(buf, end - buf, "DDR3\n");
            break;
        case HAL_MIU_DRAM_DDR2:
            buf += CamOsSnprintf(buf, end - buf, "DDR2\n");
            break;
        case HAL_MIU_DRAM_LPDDR4:
            buf += CamOsSnprintf(buf, end - buf, "LPDDR4\n");
            break;
        case HAL_MIU_DRAM_LPDDR3:
            buf += CamOsSnprintf(buf, end - buf, "LPDDR3\n");
            break;
        case HAL_MIU_DRAM_LPDDR2:
            buf += CamOsSnprintf(buf, end - buf, "LPDDR2\n");
            break;
        case HAL_MIU_DRAM_LPDDR4X:
            buf += CamOsSnprintf(buf, end - buf, "LPDDR4X\n");
            break;
        default:
            buf += CamOsSnprintf(buf, end - buf, "Unknow\n");
            break;
    }

    buf += CamOsSnprintf(buf, end - buf, "%-13s%dMB\n", "DRAM Size:", dram->info.size >> 20);
    buf += CamOsSnprintf(buf, end - buf, "%-13s%dMHz\n", "DRAM Freq:", dram->info.dram_freq);
    buf += CamOsSnprintf(buf, end - buf, "%-13s%dMHz\n", "MIUPLL Freq:", dram->info.miupll_freq);
    buf += CamOsSnprintf(buf, end - buf, "%-13s%dX\n", "Data Rate::", dram->info.data_rate);
    buf += CamOsSnprintf(buf, end - buf, "%-13s%dbit\n", "Bus Width:", dram->info.bus_width);
    buf += CamOsSnprintf(buf, end - buf, "%-13s%s\n", "SSC:", (dram->info.ssc == 0) ? "OFF" : "ON");

    return (buf - str);
}

int hal_miu_dram_set_size(struct hal_miu_dram* dram, int size)
{
    int val = 0;

    if (size < HAL_MIU_DRAM_MIN_SIZE || size > HAL_MIU_DRAM_MAX_SIZE)
    {
        hal_miu_err("[DRAM] size error! valid range: [%u, %u]\n", HAL_MIU_DRAM_MIN_SIZE, HAL_MIU_DRAM_MAX_SIZE);
        return -1;
    }

    dram->info.size = size;

    while (size >> 1)
    {
        val++;
        size >>= 1;
    }

    reg_miu_dram_set_size(val);

    return 0;
}

int hal_miu_dram_init(hal_miu_object obj, struct hal_miu_client* __client)
{
    struct hal_miu_dram* dram = (struct hal_miu_dram*)obj;

    struct hal_miu_dram temp_dram = {.callback = {
                                         .read_info = hal_miu_dram_read_info,
                                         .show_info = hal_miu_dram_show_info,
                                         .set_size  = hal_miu_dram_set_size,
                                     }};

    *dram = temp_dram;
    dram->callback.read_info(dram);

    return 0;
}

int hal_miu_dram_free(hal_miu_object obj)
{
    return 0;
}

//=================================================================================================
//                                     Hal MIU Protect Function
//=================================================================================================
static bool hal_miu_protect_id_set(struct hal_miu_protect* protect, u16* id_list, u16 src_id, u16 dst_id)
{
    int i;

    for (i = 0; i < protect->id_num; i++)
    {
        if (id_list[i] == src_id)
        {
            id_list[i] = dst_id;
            return true;
        }
    }

    return false;
}

static bool hal_miu_protect_id_exist(struct hal_miu_protect* protect, u16* id_list, u16 id)
{
    return hal_miu_protect_id_set(protect, id_list, id, id);
}

static bool hal_miu_protect_id_insert(struct hal_miu_protect* protect, int index, u16* ids)
{
    int  i;
    bool ret = true;
    u16* backup_id;
    u16* id_list = protect->protect_id;

#if defined(CONFIG_MIU_HW_MMU)
    if (index >= HAL_MIU_MMU_PROTECT_MIN)
    {
        id_list = protect->mmu_protect_id;
    }
#endif

    backup_id = CamOsMemAlloc(protect->id_num * sizeof(id_list[0]));
    if (backup_id == NULL)
    {
        hal_miu_err("[PROTECT] Alloc memory for protect backup id failed!\n");
        return false;
    }
    CamOsMemcpy(backup_id, id_list, protect->id_num * sizeof(id_list[0]));

    for (i = 0; ids[i] != HAL_MIU_PROTECT_INVLID_ID; i++)
    {
        if (!hal_miu_protect_id_exist(protect, id_list, ids[i])
            && !hal_miu_protect_id_set(protect, id_list, HAL_MIU_PROTECT_INVLID_ID, ids[i]))
        {
            hal_miu_err("[PROTECT] protect id(0x%x) update failed!\n", ids[i]);
            ret = false;
        }
    }

    if (!ret)
    {
        CamOsMemcpy(id_list, backup_id, protect->id_num * sizeof(id_list[0]));
        CamOsMemset(protect->block[index].whitelist, 0, protect->id_num * sizeof(protect->block[index].whitelist[0]));
    }

    CamOsMemRelease(backup_id);

    return ret;
}

static bool hal_miu_protect_id_remove(struct hal_miu_protect* protect, int index)
{
    int  i, j, k;
    u16  id_use[HAL_MIU_PROTECT_MAX_ID] = {0};
    u16* id_list                        = protect->protect_id;
    u16  blk_num, blk_start;

    if (index >= protect->block_num)
        return false;

    blk_start = 0;
    blk_num   = protect->block_num / 2;

#if defined(CONFIG_MIU_HW_MMU)
    if (index >= HAL_MIU_MMU_PROTECT_MIN && index < HAL_MIU_PROTECT_MAX_BLOCK)
    {
        id_list   = protect->mmu_protect_id;
        blk_start = HAL_MIU_MMU_PROTECT_MIN;
        blk_num   = protect->block_num; // block max = 64
    }
    else
        return false;
#endif

    // 1. find all id in use after remove block[index]
    for (i = blk_start; i < blk_num; i++)
    {
        if (i == index || !protect->block[i].enable)
            continue;

        for (j = 0; j < protect->block[i].white_num; j++)
        {
            for (k = 0; k < protect->id_num; k++)
            {
                if (protect->block[i].whitelist[j] == id_list[k])
                {
                    id_use[k] = 1;
                }
            }
        }
    }

    // 2. update id list
    for (i = 0; i < HAL_MIU_PROTECT_MAX_ID; i++)
    {
        if (id_use[i] == 0)
        {
            id_list[i] = HAL_MIU_PROTECT_INVLID_ID;
        }
    }

    return true;
}

static bool hal_miu_protect_block_check(struct hal_miu_protect* protect, struct hal_miu_protect_block* block)
{
    int i;

    if (block->index >= protect->block_num)
    {
        hal_miu_err("[PROTECT] Invalid index: %u\n", block->index);
        return false;
    }

    if (!block->enable)
    {
        return true;
    }

    if (block->start_addr >= block->end_addr)
    {
        hal_miu_err("[PROTECT] Start address is greater than or equal to the end address, start: 0x%llx, end: 0x%llx\n",
                    block->start_addr, block->end_addr);
        return false;
    }

    if ((block->start_addr & (HAL_MIU_PROTECT_SHIFT - 1)) || (block->end_addr & (HAL_MIU_PROTECT_SHIFT - 1)))
    {
        hal_miu_err("[PROTECT] Protect address must be aligned to %dKB, start: 0x%llx, end: 0x%llx\n",
                    HAL_MIU_PROTECT_SHIFT >> 10, block->start_addr, block->end_addr);
        return false;
    }

    if (!block->whitelist)
    {
        hal_miu_err("[PROTECT] Protect whitelist should not be NULL\n");
        return false;
    }

    for (i = 0; block->whitelist[i] != HAL_MIU_PROTECT_INVLID_ID; i++)
    {
        if (protect->client->callback.id_valid(protect->client, block->whitelist[i], HAL_MIU_WO) < 0)
        {
            hal_miu_err("[PROTECT] Invalid protect ID: 0x%x\n", block->whitelist[i]);
            return false;
        }
    }

    return true;
}

u8 hal_miu_protect_query(struct hal_miu_protect* protect, bool mmu)
{
    int i   = 0;
    int end = protect->block_num;

#if defined(CONFIG_MIU_HW_MMU)
    if (mmu)
    {
        i = HAL_MIU_MMU_PROTECT_MIN;
    }
    else
    {
        end = HAL_MIU_MMU_PROTECT_MIN;
    }
#endif

    for (; i < end; i++)
    {
        if (!protect->block[i].enable)
        {
            return i;
        }
    }

    return MIU_PROTECT_INVLID_BLOCK;
}

static int hal_miu_protect_disable(struct hal_miu_protect* protect, u8 index)
{
    if (index >= protect->block_num)
    {
        hal_miu_err("[PROTECT] Invalid index: %u\n", index);
        return -1;
    }

    if (!protect->block[index].enable)
    {
        return 0;
    }

    hal_miu_protect_id_remove(protect, index);
    CamOsMemRelease(protect->block[index].whitelist);
    protect->block[index].whitelist = NULL;
    protect->block[index].enable    = false;

#if defined(CONFIG_MIU_HW_MMU)
    if (index >= HAL_MIU_MMU_PROTECT_MIN)
    {
        reg_mmu_protect_set_enable(index - HAL_MIU_MMU_PROTECT_MIN, 0);
        reg_mmu_protect_set_id_list(protect->mmu_protect_id, protect->id_num);

        return 0;
    }
#endif

    reg_miu_protect_set_enable(index, 0);
    reg_miu_protect_set_id_list(protect->protect_id, protect->id_num);

    return 0;
}

static int hal_miu_protect_replace_block(struct hal_miu_protect* protect, struct hal_miu_protect_block* block)
{
    if (protect->block[block->index].whitelist)
    {
        CamOsMemRelease(protect->block[block->index].whitelist);
    }

    protect->block[block->index].whitelist = CamOsMemAlloc(sizeof(block->whitelist[0]) * block->white_num);
    if (protect->block[block->index].whitelist == NULL)
    {
        hal_miu_err("[PROTECT] Alloc memory for whitelist failed!\n");
        return -1;
    }
    CamOsMemcpy(protect->block[block->index].whitelist, block->whitelist,
                sizeof(block->whitelist[0]) * block->white_num);

    block->whitelist             = protect->block[block->index].whitelist;
    protect->block[block->index] = *block;

    return 0;
}

static int hal_miu_protect_enable(struct hal_miu_protect* protect, struct hal_miu_protect_block* block)
{
    int  i, j;
    u32  id_enable = 0;
    u16* id_list   = protect->protect_id;

    block->start_addr = CamOsMemPhysToMiu(block->start_addr);
    block->end_addr   = CamOsMemPhysToMiu(block->end_addr);

    if (!hal_miu_protect_block_check(protect, block))
    {
        return -1;
    }

    if (!protect->block[block->index].enable)
    {
        if (!hal_miu_protect_id_insert(protect, block->index, block->whitelist))
        {
            return -1;
        }
    }
    else
    {
        hal_miu_protect_id_remove(protect, block->index);
        if (!hal_miu_protect_id_insert(protect, block->index, block->whitelist))
        {
            if (protect->block[block->index].enable)
            {
                hal_miu_protect_id_insert(protect, block->index, protect->block[block->index].whitelist);
            }
            return -1;
        }
    }

    if (hal_miu_protect_replace_block(protect, block) < 0)
    {
        return -1;
    }

#if defined(CONFIG_MIU_HW_MMU)
    if (block->index >= HAL_MIU_MMU_PROTECT_MIN)
    {
        id_list = protect->mmu_protect_id;
    }
#endif

    for (i = 0; block->whitelist[i] != HAL_MIU_PROTECT_INVLID_ID; i++)
    {
        for (j = 0; j < protect->id_num; j++)
        {
            if (id_list[j] == block->whitelist[i])
            {
                id_enable |= 1 << j;
                break;
            }
        }
    }

#if defined(CONFIG_MIU_HW_MMU)
    if (block->index >= HAL_MIU_MMU_PROTECT_MIN)
    {
        reg_mmu_protect_set_id_list(id_list, protect->id_num);
        reg_mmu_protect_block(block->index - HAL_MIU_MMU_PROTECT_MIN, block->enable, block->invert, block->start_addr,
                              block->end_addr, id_enable);
        reg_miu_protect_log_clr(1, 1);
        reg_miu_protect_log_clr(1, 0);

        return 0;
    }
#endif

    reg_miu_protect_set_id_list(id_list, protect->id_num);
    reg_miu_protect_block(block->index, block->enable, block->invert, block->start_addr, block->end_addr, id_enable);
    reg_miu_protect_log_clr(0, 1);
    reg_miu_protect_log_clr(0, 0);

    return 0;
}

static bool hal_miu_protect_contain(struct hal_miu_protect* protect, u8 index, u16 id)
{
    int i;

    if (index >= protect->block_num)
    {
        return false;
    }

    for (i = 0; i < protect->block[index].white_num; i++)
    {
        if (protect->block[index].whitelist[i] == id)
        {
            return true;
        }
    }

    return false;
}

static int hal_miu_protect_append_id(struct hal_miu_protect* protect, u8 index, u16 id)
{
    int                           ret;
    void*                         pPtr;
    struct hal_miu_protect_block* block = protect->callback.get_block(protect, index);
    struct hal_miu_protect_block  new_block;

    if (!block)
    {
        return -1;
    }

    if (protect->callback.contain(protect, index, id))
    {
        return 0;
    }

    new_block = *block;
    new_block.white_num++;
    pPtr                = CamOsMemAlloc(new_block.white_num * sizeof(u16));
    new_block.whitelist = pPtr;
    if (new_block.whitelist == NULL)
    {
        return -1;
    }
    memcpy(new_block.whitelist, block->whitelist, block->white_num * sizeof(u16));

    new_block.whitelist[new_block.white_num - 2] = id;
    new_block.whitelist[new_block.white_num - 1] = HAL_MIU_PROTECT_INVLID_ID;

    new_block.start_addr = CamOsMemMiuToPhys(block->start_addr);
    new_block.end_addr   = CamOsMemMiuToPhys(block->end_addr);

    ret = protect->callback.enable(protect, &new_block);

    CamOsMemRelease(pPtr);

    return ret;
}

static int hal_miu_protect_remove_id(struct hal_miu_protect* protect, u8 index, u16 id)
{
    int                           i = 0;
    int                           j = 0;
    int                           ret;
    void*                         pPtr;
    struct hal_miu_protect_block* block = protect->callback.get_block(protect, index);
    struct hal_miu_protect_block  new_block;

    if (!block)
    {
        return -1;
    }

    if (!protect->callback.contain(protect, index, id))
    {
        return 0;
    }

    new_block = *block;
    new_block.white_num--;
    pPtr                = CamOsMemAlloc(new_block.white_num * sizeof(u16));
    new_block.whitelist = pPtr;
    if (new_block.whitelist == NULL)
    {
        return -1;
    }

    while (block->whitelist[j] != HAL_MIU_PROTECT_INVLID_ID)
    {
        if (block->whitelist[j] == id)
        {
            j++;
        }
        new_block.whitelist[i++] = block->whitelist[j++];
    }

    new_block.whitelist[new_block.white_num - 1] = HAL_MIU_PROTECT_INVLID_ID;

    new_block.start_addr = CamOsMemMiuToPhys(block->start_addr);
    new_block.end_addr   = CamOsMemMiuToPhys(block->end_addr);

    ret = protect->callback.enable(protect, &new_block);

    CamOsMemRelease(pPtr);

    return ret;
}

struct hal_miu_protect_block* hal_miu_protect_get_block(struct hal_miu_protect* protect, u8 index)
{
    if (index >= protect->block_num || !protect->block[index].enable)
    {
        return NULL;
    }

    return &protect->block[index];
}

static u16* hal_miu_protect_get_kernel_whitelist(struct hal_miu_protect* protect)
{
    return protect->kernel_whitelist;
}

static u16* hal_miu_protect_get_kernel_blakclist(struct hal_miu_protect* protect)
{
    return protect->kernel_blacklist;
}

static int hal_miu_protect_get_hit_info_real(struct hal_miu_protect* protect, struct hal_miu_protect_hit_info* info,
                                             bool mmu)
{
    int  ret = 0;
    int  block, id;
    int  max_block = protect->block_num;
    bool b_wr      = 0;
    int  rw_status;

    reg_miu_protect_irq_mask(mmu, 1);

    info->hit     = reg_miu_protect_flag(mmu, &b_wr);
    info->hit_mmu = mmu;

    hal_miu_err("%d, %d\r\n", info->hit, info->hit_mmu);
    if (!info->hit)
    {
        ret = -1;
        goto GET_FAIL;
    }

    if (b_wr)
        rw_status = HAL_MIU_WO;
    else
        rw_status = HAL_MIU_RO;
    info->IsWrite = b_wr;

    info->hit_addr = reg_miu_protect_hit_addr(mmu, b_wr);

    block = reg_miu_protect_hit_block(mmu, b_wr);
    if (mmu)
    {
        block += HAL_MIU_MMU_PROTECT_MIN;
    }
    else
    {
        max_block = HAL_MIU_MMU_PROTECT_MIN;
    }

    if (block < max_block)
    {
        info->hit_block   = &protect->block[block];
        info->out_of_dram = false;
    }
    else
    {
        info->hit_block   = NULL;
        info->out_of_dram = true;
    }

    id           = reg_miu_protect_hit_id(mmu, b_wr);
    info->hit_ip = protect->client->callback.id_to_ip(protect->client, id, rw_status);

    if (!info->hit_ip)
    {
        hal_miu_err("[PROTECT] Unknow IP hit protect: %#x.\n", id);
    }

    protect->hit_count++;

GET_FAIL:
    reg_miu_protect_log_clr(mmu, 1);
    reg_miu_protect_flag(mmu, &b_wr);
    reg_miu_protect_log_clr(mmu, 0);
    reg_miu_protect_irq_mask(mmu, 0);
    hal_miu_err("\r\n");

    return ret;
}

// void reg_miu_protect_dump_bank(u32 bank)
// {
//     int i;

//     hal_miu_info(KERN_CONT "BANK:0x%x\n00: ", bank);
//     bank = GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, bank << 8);
//     for (i = 0; i <= 0x7F; i++)
//     {
//         if (i && (i & 0x7) == 0)
//         {
//             hal_miu_info(KERN_CONT "\n%02x: ", i);
//         }
//         hal_miu_info(KERN_CONT "%#06x ", reg_miu_read(bank, i));
//     }
// }

static int hal_miu_protect_get_hit_info(struct hal_miu_protect* protect, struct hal_miu_protect_hit_info* info)
{
    int ret;

    // reg_miu_protect_dump_bank(0x1659); // for debug

#if defined(CONFIG_MIU_HW_MMU)
    if (reg_miu_mmu_enable_status())
    {
        ret = hal_miu_protect_get_hit_info_real(protect, info, 1);
        if (info->hit)
        {
            return 0;
        }
    }
#endif

    ret = hal_miu_protect_get_hit_info_real(protect, info, 0);

    return ret;
}

static int hal_miu_protect_show_hit_info(struct hal_miu_protect* protect, struct hal_miu_protect_hit_info* info)
{
    int            i;
    char           protect_str[HAL_MIU_PROTECT_BUF] = {0};
    char*          buf                              = protect_str;
    char*          end                              = protect_str + HAL_MIU_PROTECT_BUF;
    ss_phys_addr_t start_addr                       = info->hit_addr * HAL_MIU_PROTECT_ADDR_UNIT;
    ss_phys_addr_t end_addr                         = start_addr + HAL_MIU_PROTECT_ADDR_UNIT - 1;

    if (info->out_of_dram)
    {
        buf += CamOsSnprintf(buf, end - buf, "[PROTECT] IP 0x%02x - %s, %s out of dram.\n", info->hit_ip->id,
                             info->hit_ip->name, info->IsWrite ? "write" : "read");
    }
    else
    {
        buf += CamOsSnprintf(buf, end - buf, "[PROTECT] IP hit protect address.\n");
        buf += CamOsSnprintf(buf, end - buf, "Hit block %d, address: 0x%llx<->0x%llx, %s: [", info->hit_block->index,
                             info->hit_block->start_addr, info->hit_block->end_addr,
                             info->hit_block->invert ? "blacklist" : "whitelist");

        if (info->hit_block->whitelist)
        {
            for (i = 0; info->hit_block->whitelist[i] != HAL_MIU_PROTECT_INVLID_ID; i++)
            {
                buf += CamOsSnprintf(
                    buf, end - buf, "%s, ",
                    protect->client->callback.id_to_name(protect->client, info->hit_block->whitelist[i], HAL_MIU_RW));
            }
        }

        buf += CamOsSnprintf(buf, end - buf, "]\n");
    }

    buf += CamOsSnprintf(buf, end - buf, "Hit count: %llu\n", protect->hit_count);
    if (info->hit_ip)
    {
        buf += CamOsSnprintf(buf, end - buf, "Hit IP : 0x%02x - %s\n", info->hit_ip->id, info->hit_ip->name);
    }
    else
    {
        buf += CamOsSnprintf(buf, end - buf, "Hit IP : Unknow\n");
    }
    buf += CamOsSnprintf(buf, end - buf, "Hit %s address: 0x%llx<->0x%llx, IsWrite=%d\n", info->hit_mmu ? "MMU" : "MIU",
                         start_addr, end_addr, info->IsWrite);

    hal_miu_err("%s", protect_str);

    return 0;
}

static int hal_miu_protect_show_block_info(struct hal_miu_protect* protect, struct hal_miu_protect_block* block)
{
    return 0;
}

static int hal_miu_protect_clear_interrupt(struct hal_miu_protect* protect)
{
    int ret;

    ret = reg_miu_protect_clear_irq();

    return ret;
}

static int hal_miu_protect_block_init(struct hal_miu_protect* protect, struct hal_miu_protect_block* block)
{
    int j, k = 0;
    u32 id_enable;

    block->enable = reg_miu_protect_get_enable(block->index);
    if (!block->enable)
    {
        return 0;
    }

    block->invert = reg_miu_protect_get_invert(block->index);
    reg_miu_protect_get_addr(block->index, &block->start_addr, &block->end_addr);

    id_enable = reg_miu_protect_get_id_enable(block->index);
    for (j = 0; j < protect->id_num; j++)
    {
        if (id_enable & (1 << j))
        {
            block->white_num++;
        }
    }
    block->white_num++;

    block->whitelist = CamOsMemAlloc(block->white_num * sizeof(u16));
    if (!block->whitelist)
    {
        return -1;
    }

    for (j = 0; j < protect->id_num; j++)
    {
        if (id_enable & (1 << j))
        {
            block->whitelist[k++] = protect->protect_id[j];
        }
    }
    block->whitelist[k] = HAL_MIU_PROTECT_INVLID_ID;

    return 0;
}

int hal_miu_protect_init(hal_miu_object obj, struct hal_miu_client* client)
{
    int                           i;
    struct hal_miu_protect_block* block;
    struct hal_miu_protect*       protect      = (struct hal_miu_protect*)obj;
    struct hal_miu_protect        temp_protect = {
        .block_num        = HAL_MIU_PROTECT_MAX_BLOCK,
        .id_num           = HAL_MIU_PROTECT_MAX_ID,
        .protect_id       = {0},
        .kernel_whitelist = HAL_MIU_PROTECT_KERNEL_WHITELIST,
        .kernel_blacklist = HAL_MIU_PROTECT_KERNEL_BLACKLIST,
        .hit_count        = 0,
        .block            = {{0}},
        .callback =
            {
                .query                = hal_miu_protect_query,
                .enable               = hal_miu_protect_enable,
                .disable              = hal_miu_protect_disable,
                .contain              = hal_miu_protect_contain,
                .append_id            = hal_miu_protect_append_id,
                .remove_id            = hal_miu_protect_remove_id,
                .get_block            = hal_miu_protect_get_block,
                .get_kernel_whitelist = hal_miu_protect_get_kernel_whitelist,
                .get_kernel_blacklist = hal_miu_protect_get_kernel_blakclist,
                .get_hit_info         = hal_miu_protect_get_hit_info,
                .show_hit_info        = hal_miu_protect_show_hit_info,
                .show_block_info      = hal_miu_protect_show_block_info,
                .clear_interrupt      = hal_miu_protect_clear_interrupt,
            },
        .client = client,
    };

    *protect = temp_protect;

    reg_miu_protect_get_id_list(protect->protect_id, protect->id_num);
#if defined(CONFIG_MIU_HW_MMU)
    reg_mmu_protect_get_id_list(protect->mmu_protect_id, protect->id_num);
#endif

    for (i = 0; i < protect->block_num; i++)
    {
        block        = &protect->block[i];
        block->index = i;
        hal_miu_protect_block_init(protect, block);
    }

    reg_miu_protect_log_clr(0, 1);
    reg_miu_protect_flag(0, (bool*)&i);
    reg_miu_protect_log_clr(0, 0);

    return 0;
}

int hal_miu_protect_free(hal_miu_object obj)
{
    int                     i;
    struct hal_miu_protect* protect = (struct hal_miu_protect*)obj;

    for (i = 0; i < protect->block_num; i++)
    {
        if (protect->block[i].whitelist)
        {
            CamOsMemRelease(protect->block[i].whitelist);
        }
    }

    return 0;
}

//=================================================================================================
//                                     Hal MIU MMU Function
//=================================================================================================

#if defined(CONFIG_MIU_HW_MMU)
static int hal_miu_mmu_set_page_size(struct hal_miu_mmu* mmu, u8 mode)
{
    u32 size;

    if (mode >= HAL_MIU_MAX_PGSZ_NUM)
    {
        hal_miu_err("[MMU] not such page size mode: 0x%u\n", mode);
        return -1;
    }

    mmu->info.page_mode = mode;
    mmu->info.page_size = mmu->info.page_list[mode];
    mmu->info.page_bit  = 0;

    size = mmu->info.page_size - 1;
    while (size)
    {
        ++mmu->info.page_bit;
        size >>= 1;
    }

    mmu->info.region_bit  = HAL_MIU_MMU_ADDR_BIT - HAL_MIU_MMU_ENTRY_BIT - mmu->info.page_bit;
    mmu->info.region_size = 1 << (mmu->info.page_bit + HAL_MIU_MMU_ENTRY_BIT);
    mmu->info.max_region  = 1 << mmu->info.region_bit;

    reg_miu_mmu_set_page_size(mode);

    return 0;
}

static u32 hal_miu_mmu_get_page_size(struct hal_miu_mmu* mmu)
{
    return mmu->info.page_size;
}

int hal_miu_mmu_set_region(struct hal_miu_mmu* mmu, u16 vpa_region, u16 pa_region)
{
    if ((vpa_region >> mmu->info.region_bit) || (pa_region >> mmu->info.region_bit))
    {
        hal_miu_err("[MMU] region invaild!\n");
        return -1;
    }

    reg_miu_mmu_set_region(vpa_region, pa_region);

    mmu->info.vpa_region = vpa_region;
    mmu->info.pa_region  = pa_region;

    return 0;
}

static int hal_miu_mmu_set_entry(struct hal_miu_mmu* mmu, u16 vpa_entry, u16 pa_entry)
{
    if ((vpa_entry >> mmu->info.entry_bit) || (pa_entry >> mmu->info.entry_bit))
    {
        hal_miu_err("[MMU] entry invaild!\n");
        return -1;
    }

    reg_miu_mmu_set_entry(vpa_entry, pa_entry);

    return 0;
}

static int hal_miu_mmu_map_entry(struct hal_miu_mmu* mmu, u16 vpa_entry, u16 pa_entry)
{
    return hal_miu_mmu_set_entry(mmu, vpa_entry, pa_entry);
}

static int hal_miu_mmu_unmap_entry(struct hal_miu_mmu* mmu, u16 vpa_entry)
{
    return hal_miu_mmu_set_entry(mmu, vpa_entry, HAL_MIU_MMU_INVALID_ENTRY);
}

static u16 hal_miu_mmu_query_entry(struct hal_miu_mmu* mmu, u16 vpa_entry)
{
    if (vpa_entry >> mmu->info.entry_bit)
    {
        hal_miu_err("[MMU] entry invaild!\n");
        return HAL_MIU_MMU_INVALID_ENTRY;
    }

    return reg_miu_mmu_get_entry(vpa_entry);
}

static int hal_miu_mmu_enable(struct hal_miu_mmu* mmu, bool enable)
{
    mmu->info.enable = enable;
    reg_miu_mmu_enable(enable);

    return 0;
}

static int hal_miu_mmu_reset(struct hal_miu_mmu* mmu)
{
    if (reg_miu_mmu_reset())
    {
        hal_miu_err("[MMU] reset entry fail!\n");
        return 1;
    }

    return 0;
}

static int hal_miu_mmu_get_irq_status(struct hal_miu_mmu* mmu, u16* entry, u16* id, u8* is_write)
{
    u32 ret;
    reg_miu_mmu_irq_mask(1);
    ret = reg_miu_mmu_get_irq_status(entry, id, is_write);
    reg_miu_mmu_irq_mask(0);
    reg_miu_mmu_irq_clr(1);
    reg_miu_mmu_irq_clr(0);

    return ret;
}

int hal_miu_mmu_init(hal_miu_object obj, struct hal_miu_client* __client)
{
    struct hal_miu_mmu* mmu      = (struct hal_miu_mmu*)obj;
    struct hal_miu_mmu  temp_mmu = {
        .info =
            {
                .enable     = false,
                .entry_bit  = HAL_MIU_MMU_ENTRY_BIT,
                .entry_size = (1 << HAL_MIU_MMU_ENTRY_BIT),
                .max_entry  = (1 << HAL_MIU_MMU_ENTRY_BIT) - 1,

                .page_list = {MMU_PAGE_SIZE_32K, MMU_PAGE_SIZE_64K, MMU_PAGE_SIZE_128K},
            },
        .callback = {.enable         = hal_miu_mmu_enable,
                     .reset          = hal_miu_mmu_reset,
                     .set_page_size  = hal_miu_mmu_set_page_size,
                     .get_page_size  = hal_miu_mmu_get_page_size,
                     .set_region     = hal_miu_mmu_set_region,
                     .map_entry      = hal_miu_mmu_map_entry,
                     .unmap_entry    = hal_miu_mmu_unmap_entry,
                     .query_entry    = hal_miu_mmu_query_entry,
                     .get_irq_status = hal_miu_mmu_get_irq_status},
    };

    *mmu = temp_mmu;

    mmu->callback.set_page_size(mmu, mmu->info.page_list[0]);

    return 0;
}

int hal_miu_mmu_free(hal_miu_object obj)
{
    return 0;
}
#endif

#if defined(CAM_OS_LINUX_KERNEL)
//=================================================================================================
//                                     Hal MIU Bist Function
//=================================================================================================

static int hal_miu_bist_start_one(struct hal_miu_bist* bist, int index)
{
    struct hal_miu_bist_info* info = &bist->bist[index];

    reg_miu_bist_init(info->group_bank, info->effi_bank);
    reg_miu_bist_set_base(info->effi_bank, bist->addr.miu);
    reg_miu_bist_set_length(info->effi_bank, bist->addr.size);
    reg_miu_bist_set_loop(info->effi_bank, info->loop);
    reg_miu_bist_set_cmd_mode(info->effi_bank, info->cmd_mode);
    if (info->mode != HAL_MIU_RW)
    {
        reg_miu_bist_set_rw_only(info->effi_bank, info->mode == HAL_MIU_RO);
    }
    reg_miu_bist_start(info->effi_bank);

    return 0;
}

static int hal_miu_bist_wait_done(struct hal_miu_bist* bist, int index)
{
    while (1)
    {
        if (reg_miu_bist_is_finish(bist->bist[index].effi_bank))
        {
            break;
        }
    }

    return 0;
}

static bool hal_miu_bist_is_all_stop(struct hal_miu_bist* bist)
{
    int i;
    for (i = 0; i < bist->bist_num; i++)
    {
        if (bist->bist[i].start)
        {
            return false;
        }
    }

    return true;
}

static int hal_miu_bist_enable_protect(struct hal_miu_bist* bist, struct hal_miu_protect* protect, bool* mask)
{
    int                           i;
    int                           block_id = -1;
    struct hal_miu_protect_block* block;

    for (i = 0; i < protect->block_num; i++)
    {
        block = protect->callback.get_block(protect, i);
        if ((block->start_addr <= bist->addr.miu) && (bist->addr.miu <= block->end_addr))
        {
            block_id = i;
            break;
        }
    }

    if (!block || !block->enable || block_id < 0)
    {
        return 0;
    }

    for (i = 0; i < bist->bist_num; i++)
    {
        if (!mask[i])
        {
            continue;
        }
        hal_miu_info("add bist %d to kernel protect list!\n", i);
        if (protect->callback.append_id(protect, block_id, bist->callback.get_id(bist, i)) < 0)
        {
            hal_miu_err("[BIST] Add bist to protect whitelist failed!\n");
            return -1;
        }
    }

    return 0;
}

static int hal_miu_bist_disable_protect(struct hal_miu_bist* bist, struct hal_miu_protect* protect)
{
    int                           i;
    int                           block_id = -1;
    struct hal_miu_protect_block* block;

    for (i = 0; i < protect->block_num; i++)
    {
        block = protect->callback.get_block(protect, i);
        if ((block->start_addr <= bist->addr.miu) && (bist->addr.miu <= block->end_addr))
        {
            block_id = i;
            break;
        }
    }

    if (!block || !block->enable || block_id < 0)
    {
        return 0;
    }

    for (i = 0; i < bist->bist_num; i++)
    {
        if (bist->bist[i].start)
        {
            continue;
        }
        hal_miu_info("remove bist %d from kernel protect list!\n", i);
        protect->callback.remove_id(protect, block_id, bist->callback.get_id(bist, i));
    }

    return 0;
}

static int hal_miu_bist_memory_free(struct hal_miu_bist* bist)
{
    if (bist->callback.is_all_stop(bist))
    {
        hal_miu_info("All BIST stop, free memory.\n");
        hal_miu_mem_free(&bist->addr);
        return 0;
    }

    return -1;
}

static int hal_miu_bist_start(struct hal_miu_bist* bist, bool loop, enum hal_miu_rw_mode mode, bool* mask,
                              struct hal_miu_protect* protect)
{
    int i;
    u16 result;
    u64 start_time, bist_time;

    if (!hal_miu_mem_alloc(&bist->addr))
    {
        hal_miu_err("[BIST] alloc memory failed!\n");
        return -1;
    }
#if 0
    else
    {
        U16                          whitelist[4] = {0x0f, 0x1f, 0x2f, 0x00};
        struct hal_miu_protect_block bist_block   = {
            .index      = HAL_MIU_PROTECT_MAX_ID - 1,
            .enable     = true,
            .invert     = 1,
            .white_num  = 3,
            .whitelist  = &whitelist[0],
            .start_addr = 0x20000000,
            .end_addr   = bist->addr.phy,
        };

        hal_miu_err("%s %d: addr=%llX ~ %llX\r\n", __FUNCTION__, __LINE__, bist_block.start_addr, bist_block.end_addr);
        hal_miu_protect_enable(protect, &bist_block);

        bist_block.index      = HAL_MIU_PROTECT_MAX_ID - 2;
        bist_block.start_addr = bist->addr.phy + bist->addr.size;
        bist_block.end_addr   = 0x40000000;
        printk("%s %d: addr=%llX ~ %llX\r\n", __FUNCTION__, __LINE__, bist_block.start_addr, bist_block.end_addr);
        hal_miu_protect_enable(protect, &bist_block);
    }
#endif
    if (hal_miu_bist_enable_protect(bist, protect, mask) < 0)
    {
        return -1;
    }

    for (i = 0; i < bist->bist_num; i++)
    {
        if (!mask[i])
        {
            continue;
        }

        bist->bist[i].loop  = loop;
        bist->bist[i].mode  = mode;
        bist->bist[i].start = true;
        hal_miu_info("BIST %d start at 0x%llX, size %uKB, mode: %s, %s\n", i, bist->addr.miu, bist->addr.size >> 10,
                     loop ? "loop" : "oneshot", mode == HAL_MIU_RW ? "rw" : (mode == HAL_MIU_RO ? "ro" : "wo"));
        hal_miu_bist_start_one(bist, i);
        if (!loop)
        {
            start_time = sched_clock();
            hal_miu_bist_wait_done(bist, i);
            bist_time             = sched_clock() - start_time;
            bist->bist[i].start   = false;
            bist->bist[i].success = reg_miu_bist_is_success(bist->bist[i].effi_bank);
            result                = reg_miu_bist_get_result(bist->bist[i].effi_bank);
            hal_miu_info("BIST result: %s(ret=0x%x), time: %llu(ns)\r\n", bist->bist[i].success ? "success" : "failed",
                         result, bist_time);
        }
    }

    hal_miu_bist_disable_protect(bist, protect);
    hal_miu_bist_memory_free(bist);

    return 0;
}

static int hal_miu_bist_stop(struct hal_miu_bist* bist, bool* mask, struct hal_miu_protect* protect)
{
    int i;
    u16 result;

    for (i = 0; i < bist->bist_num; i++)
    {
        if (mask[i] && bist->bist[i].start) // mask[i] == true means stop
        {
            bist->bist[i].start   = false;
            bist->bist[i].success = reg_miu_bist_is_success(bist->bist[i].effi_bank);
            reg_miu_bist_stop(bist->bist[i].effi_bank);
            hal_miu_bist_wait_done(bist, i);
            bist->bist[i].success &= reg_miu_bist_is_success(bist->bist[i].effi_bank);
            result = reg_miu_bist_get_result(bist->bist[i].effi_bank);
            hal_miu_info("BIST %d Stop. result: %s(ret=0x%x)\n", i, bist->bist[i].success ? "success" : "failed",
                         result);
        }
    }
    hal_miu_bist_disable_protect(bist, protect);
    hal_miu_bist_memory_free(bist);

    return 0;
}

static int hal_miu_bist_set_size(struct hal_miu_bist* bist, u32 size)
{
    int i;

    for (i = 0; i < bist->bist_num; i++)
    {
        if (bist->bist[i].start)
        {
            hal_miu_err("[BIST] Please stop all bist before change size!\n");
            return -1;
        }
    }

    bist->addr.size = size * 1024;

    return 0;
}

static int hal_miu_bist_get_id(struct hal_miu_bist* bist, int index)
{
    if (index >= bist->bist_num)
    {
        return -1;
    }

    return bist->bist[index].id;
}

static int hal_miu_bist_cmd_mode_set(struct hal_miu_bist* bist, u16 mode)
{
    int i;

    hal_miu_info("set cmd_mode=0x%x\n", mode);

    if (mode > 0xF)
    {
        hal_miu_err("[BIST] set wrong value for 0 <= cmd_mode <= 15\n");
        return -1;
    }

    for (i = 0; i < bist->bist_num; i++)
    {
        if (bist->bist[i].start)
        {
            hal_miu_err("[BIST] Please stop all bist before change size!\n");
            return -1;
        }

        bist->bist[i].cmd_mode = mode;
    }

    return 0;
}

static int hal_miu_bist_show(struct hal_miu_bist* bist, char* buf, char* end)
{
    int                       i;
    char*                     str = buf;
    struct hal_miu_tabulate   table;
    struct hal_miu_bist_info* bist_info;

    hal_miu_tabulate_init(&table, buf, end, "| Bist | type    | mode         | addr       | size     | cmd mode |");

    for (i = 0; i < bist->bist_num; i++)
    {
        bist_info = &bist->bist[i];
        hal_miu_tabulate_add_row(
            &table, "|%d|%s|%s|%#llx|%dKB|0x%X|", i, bist_info->start ? (bist_info->loop ? "loop" : "oneshot") : "stop",
            bist_info->mode == HAL_MIU_RW ? "read & write"
                                          : (bist_info->mode == HAL_MIU_RO ? "read only" : "write only"),
            bist->addr.miu, bist->addr.size >> 10, bist->bist[i].cmd_mode);
    }

    str = hal_miu_tabulate_finish(&table);

    return (str - buf);
}

int hal_miu_bist_init(hal_miu_object obj, struct hal_miu_client* client)
{
    struct hal_miu_bist* bist      = (struct hal_miu_bist*)obj;
    struct hal_miu_bist  temp_bist = {
        .bist_num = HAL_MIU_BIST_NUM,
        .addr     = {.size = HAL_MIU_BIST_LENGTH},
        .mask     = {0},
        .callback =
            {
                .start       = hal_miu_bist_start,
                .stop        = hal_miu_bist_stop,
                .is_all_stop = hal_miu_bist_is_all_stop,
                .set_size    = hal_miu_bist_set_size,
                .get_id      = hal_miu_bist_get_id,
                .cmd_mode    = hal_miu_bist_cmd_mode_set,
                .show        = hal_miu_bist_show,
            },
        .bist =
            {
                {.chip_top   = false,
                 .group_bank = BASE_REG_MIU_GRP_SC0,
                 .effi_bank  = BASE_REG_MIU_EFFI_SC0,
                 .id         = MIU_CLIENTW_BIST0,
                 .cmd_mode   = 0},
                {.chip_top   = false,
                 .group_bank = BASE_REG_MIU_GRP_SC1,
                 .effi_bank  = BASE_REG_MIU_EFFI_SC1,
                 .id         = MIU_CLIENTW_BIST1,
                 .cmd_mode   = 0},
                {.chip_top   = false,
                 .group_bank = BASE_REG_MIU_GRP_ISP0,
                 .effi_bank  = BASE_REG_MIU_EFFI_ISP0,
                 .id         = MIU_CLIENTW_BIST2,
                 .cmd_mode   = 0},
            },
    };

    *bist = temp_bist;

    return 0;
}

int hal_miu_bist_free(hal_miu_object obj)
{
    return 0;
}

#endif // defined(CAM_OS_LINUX_KERNEL)
//=================================================================================================
//                                     Hal MIU BW Function
//=================================================================================================
#if defined(CONFIG_MIU_BWLA)
static int hal_miu_bw_mem_free(struct hal_miu_bw* bw)
{
    if (bw->bw_buf)
    {
        CamOsMemRelease(bw->bw_buf);
        bw->bw_buf = NULL;
    }

#if defined(CONFIG_MIU_BWLA)
    hal_miu_mem_free(&bw->mem);
    if (bw->statis)
    {
        CamOsMemRelease(bw->statis);
        bw->statis = NULL;
    }
#endif

    return 0;
}

static int hal_miu_bw_mem_alloc(struct hal_miu_bw* bw)
{
#if defined(CONFIG_MIU_BWLA)
    int i;
#endif

    if (!bw->bw_buf)
    {
        bw->bw_buf = CamOsMemAlloc(HAL_MIU_BW_BUF_MAX);
        if (!bw->bw_buf)
        {
            goto ALLOC_FAIL;
        }
        CamOsMemset(bw->bw_buf, 0, HAL_MIU_BW_BUF_MAX);
    }

#if defined(CONFIG_MIU_BWLA)
    bw->mem.size = bw->round * HAL_MIU_BWLA_ROUND_SIZE;
    if (!hal_miu_mem_alloc(&bw->mem))
    {
        goto ALLOC_FAIL;
    }

    // alloc memory for parsed data
    bw->statis = (struct hal_miu_bw_statis*)CamOsMemAlloc(sizeof(struct hal_miu_bw_statis) * bw->client->ip_num);
    if (bw->statis == NULL)
    {
        goto ALLOC_FAIL;
    }

    memset(bw->statis, 0, sizeof(struct hal_miu_bw_statis) * bw->client->ip_num);
    for (i = 0; i < bw->client->ip_num; i++)
    {
        bw->statis[i].effi_min.integer = 200;
    }
#endif // defined(CONFIG_MIU_BWLA)

    return 0;

ALLOC_FAIL:
    hal_miu_bw_mem_free(bw);
    return -1;
}

static int hal_miu_bwla_start_measure(struct hal_miu_bw* bw)
{
    int                i;
    int                max_wait_us = 6000000;
    struct hal_miu_ip* ip;

    reg_miu_bwla_reset();
    reg_miu_bwla_init();
    reg_miu_bwla_set_mode(bw->sample_mode);

    reg_miu_bwla_set_addr(bw->mem.miu);
    reg_miu_bwla_set_round(bw->round);
    reg_miu_bwla_set_enable(1);

    for (i = 0; i < bw->client->ip_num; i++)
    {
        ip = &bw->client->ip_table[i];
        reg_miu_bwla_set_ip(i, ip->id);
    }

    while (!reg_miu_bwla_is_done() && max_wait_us)
    {
        CamOsUsDelay(1);
        max_wait_us--;
    }

    reg_miu_bwla_set_enable(0);
    if (max_wait_us <= 0)
    {
        // TODO(quanming.wu): err log
        return -1;
    }

    return 0;
}

static int hal_miu_bwla_parse_date(struct hal_miu_bw* bw)
{
    int                         i, j, sample_time;
    struct hal_miu_float        effi_tmp;
    struct hal_miu_bwla_ip_raw* ip;
    struct hal_miu_bw_statis*   statis;

    sample_time = (1 << (20 - bw->sample_mode)) - 1;

    // parse data
    for (i = 0; i < bw->round; i++)
    {
        ip = (struct hal_miu_bwla_ip_raw*)bw->mem.virt + (i * HAL_MIU_BWLA_MAX_IP);
        for (j = 0; j < bw->client->ip_num; j++)
        {
            statis = &(bw->statis[j]);

            statis->bw_total.integer += ip[j].total;

            statis->bw_read_avg.integer += ip[j].read;
            statis->bw_read_max.integer = max((u64)ip[j].read, statis->bw_read_max.integer);

            statis->bw_write_avg.integer += ip[j].write;
            statis->bw_write_max.integer = max((u64)ip[j].write, statis->bw_write_max.integer);

            statis->util_avg.integer += ip[j].util;
            statis->util_max.integer = max((u64)ip[j].util, statis->util_max.integer);

            effi_tmp         = hal_miu_divide(ip[j].total, ip[j].util);
            statis->effi_min = hal_miu_float_compare(statis->effi_min, effi_tmp) ? effi_tmp : statis->effi_min;
            statis->effi_max = hal_miu_float_compare(effi_tmp, statis->effi_max) ? effi_tmp : statis->effi_max;
        }
    }

    // clac avg
    for (j = 0; j < bw->client->ip_num; j++)
    {
        statis = &(bw->statis[j]);

        statis->effi_avg = hal_miu_divide(statis->bw_total.integer, statis->util_avg.integer);

        statis->bw_read_max = hal_miu_divide(statis->bw_read_max.integer * 16, sample_time);
        statis->bw_read_avg = hal_miu_divide(statis->bw_read_avg.integer * 16, sample_time * bw->round);

        statis->bw_write_max = hal_miu_divide(statis->bw_write_max.integer * 16, sample_time);
        statis->bw_write_avg = hal_miu_divide(statis->bw_write_avg.integer * 16, sample_time * bw->round);

        statis->util_max = hal_miu_divide(statis->util_max.integer * 16, sample_time);
        statis->util_avg = hal_miu_divide(statis->util_avg.integer * 16, sample_time * bw->round);
    }

    return 0;
}

static const char* bwla_header =
    "+--------------------+-----------------+-----------------+--------------------------+-----------------+\n"
    "|   Rounds = %-3d     |     BW Read     |     BW Write    |           EFFI           |       UTIL      |\n"
    "+--------------------+-----------------+--------+--------+--------+--------+--------+--------+--------+\n"
    "|   ID:Name          |  avg   |  max   |  avg   |  max   |  avg   |  min   |  max   |  avg   |  max   |\n"
    "+--------------------+--------+--------+--------+--------+--------+--------+--------+--------+--------+\n";

static const char* bwla_tailer =
    "+--------------------+--------+--------+--------+--------+--------+--------+--------+--------+--------+\n";

static int hal_miu_bwla_format_float(char* str, char* end, struct hal_miu_float* num)
{
    if (num == NULL)
    {
        return CamOsSnprintf(str, end - str, " %6s |", "  X  ");
    }
    if (num->integer > 99)
    {
        return CamOsSnprintf(str, end - str, " %3llu.%02u |", num->integer, num->decimal);
    }

    return CamOsSnprintf(str, end - str, " %2llu.%03u |", num->integer, num->decimal);
}

static int hal_miu_bwla_output_result(struct hal_miu_bw* bw)
{
    int                       i;
    struct hal_miu_ip*        ip;
    struct hal_miu_bw_statis* statis;
    struct hal_miu_bw_statis  zero = {0};
    char*                     str  = bw->bw_buf;
    char*                     end  = bw->bw_buf + HAL_MIU_BW_BUF_MAX;

    // header
    str += CamOsSnprintf(str, end - str, bwla_header, bw->round);
    // output result
    for (i = 0; i < bw->client->ip_num; i++)
    {
        ip     = &bw->client->ip_table[i];
        statis = &(bw->statis[i]);

        // statis all 0 will not show
        if (memcmp(statis, &zero, sizeof(zero)) == 0)
        {
            continue;
        }

        str += CamOsSnprintf(str, end - str, "| 0x%02x:%-13s |", ip->id, ip->name);

        if (ip->read)
        {
            str += hal_miu_bwla_format_float(str, end, &statis->bw_read_avg);
            str += hal_miu_bwla_format_float(str, end, &statis->bw_read_max);
        }
        else
        {
            str += hal_miu_bwla_format_float(str, end, NULL);
            str += hal_miu_bwla_format_float(str, end, NULL);
        }

        if (ip->write)
        {
            str += hal_miu_bwla_format_float(str, end, &statis->bw_write_avg);
            str += hal_miu_bwla_format_float(str, end, &statis->bw_write_max);
        }
        else
        {
            str += hal_miu_bwla_format_float(str, end, NULL);
            str += hal_miu_bwla_format_float(str, end, NULL);
        }

        str += hal_miu_bwla_format_float(str, end, &statis->effi_avg);
        str += hal_miu_bwla_format_float(str, end, &statis->effi_min);
        str += hal_miu_bwla_format_float(str, end, &statis->effi_max);
        str += hal_miu_bwla_format_float(str, end, &statis->util_avg);
        str += hal_miu_bwla_format_float(str, end, &statis->util_max);
        str += CamOsSnprintf(str, end - str, "\n");
    }

    str += CamOsSnprintf(str, end - str, bwla_tailer);

    return 0;
}

static int hal_miu_bw_start(struct hal_miu_bw* bw)
{
    int ret = 0;

    if (hal_miu_bw_mem_alloc(bw) < 0)
    {
        hal_miu_err("[BW] alloc memory for bwla failed!\n");
        return -1;
    }

    if (hal_miu_bwla_start_measure(bw) < 0)
    {
        hal_miu_err("[BW] bwla start measure failed.\n");
        goto BWLA_EXIT;
    }

    if (hal_miu_bwla_parse_date(bw))
    {
        hal_miu_err("[BW] bwla parse date failed.\n");
        goto BWLA_EXIT;
    }

    ret = hal_miu_bwla_output_result(bw);

    return 0;

BWLA_EXIT:
    hal_miu_bw_mem_free(bw);

    return ret;
}

int hal_miu_bwla_store(struct hal_miu_bw* bw, int round)
{
    bw->round = round;

    hal_miu_info("bwla round = %d\n", round);

    return 0;
}

static int hal_miu_bw_tool_start(struct hal_miu_bw* bw)
{
    int ret = 0;

    if (hal_miu_bw_mem_alloc(bw) < 0)
    {
        hal_miu_err("[BW] alloc memory for bwla failed!\n");
        return -1;
    }

    if (hal_miu_bwla_start_measure(bw) < 0)
    {
        hal_miu_err("[BW] bwla start measure failed.\n");
        goto BWLA_EXIT;
    }

    return 0;

BWLA_EXIT:
    hal_miu_bw_mem_free(bw);

    return ret;
}

static int hal_miu_bw_tool_stop(struct hal_miu_bw* bw)
{
    int ret = 0;

    hal_miu_bw_mem_free(bw);

    return ret;
}

static int hal_miu_bw_tool_show(struct hal_miu_bw* bw, char* buf, char* end)
{
    char*              str = buf;
    struct hal_miu_ip* ip;
    u64                miu_addr;

    if (bw->bw_buf)
    {
        miu_addr = bw->mem.miu;
        str += CamOsSnprintf(str, end - str, "miu_addr=0x%x, bw_round=%d, qos_num=%d", (unsigned int)miu_addr,
                             bw->round, bw->client->ip_num);

        for_each_ip(bw->client, ip)
        {
            if (ip->read)
            {
                str += CamOsSnprintf(str, end - str, " qosr_%x=%d", ip->id, ip->read_attr.qos);
            }
            else
                str += CamOsSnprintf(str, end - str, " qosr_%x=-1", ip->id);
        }

        for_each_ip(bw->client, ip)
        {
            if (ip->write)
            {
                str += CamOsSnprintf(str, end - str, " qosw_%x=%d", ip->id, ip->write_attr.qos);
            }
            else
                str += CamOsSnprintf(str, end - str, " qosw_%x=-1", ip->id);
        }
    }
    else
        str += CamOsSnprintf(str, end - str, "miu_addr=null, bw_round=%d, qos_num=%d", bw->round, bw->client->ip_num);

    str += CamOsSnprintf(str, end - str, "\r\n");

    return (str - buf);
}
#if 0
//#else
static int hal_miu_bw_start_one(struct hal_miu_bw* bw, u16 id, enum hal_enum hal_miu_rw_mode rw,
                                struct hal_miu_bw_statis* statis)
{
    if (rw == HAL_MIU_RW)
    {
        reg_miu_bw_init(id, BW_CMD_RW_MODE);
    }
    else if (rw == HAL_MIU_RO)
    {
        reg_miu_bw_init(id, BW_CMD_RO_MODE);
    }
    else
    {
        reg_miu_bw_init(id, BW_CMD_WO_MODE);
    }

    reg_miu_bw_config(BW_CMD_GET_EFFI_AVG);
    CamOsMsSleep(HAL_MIU_BW_WAIT_MS);
    statis->effi_avg = hal_miu_divide((u64)reg_miu_bw_read(), 1024);

    reg_miu_bw_config(BW_CMD_GET_BW_AVG);
    CamOsMsSleep(HAL_MIU_BW_WAIT_MS);
    statis->bw_avg = hal_miu_divide((u64)reg_miu_bw_read(), 1024);

    reg_miu_bw_config(BW_CMD_GET_BW_MAX);
    CamOsMsSleep(HAL_MIU_BW_WAIT_MS);
    statis->bw_max = hal_miu_divide((u64)reg_miu_bw_read(), 1024);

    reg_miu_bw_config(BW_CMD_GET_UTIL_AVG);
    CamOsMsSleep(HAL_MIU_BW_WAIT_MS);
    statis->util_avg = hal_miu_divide((u64)reg_miu_bw_read(), 1024);

    reg_miu_bw_config(BW_CMD_GET_UTIL_MAX);
    CamOsMsSleep(HAL_MIU_BW_WAIT_MS);
    statis->util_max = hal_miu_divide((u64)reg_miu_bw_read(), 1024);

    return 0;
}

static int hal_miu_bw_start(struct hal_miu_bw* bw)
{
    char                     name[20];
    int                      len;
    struct hal_miu_ip*       ip;
    struct hal_miu_bw_statis statis = {0};
    struct hal_miu_tabulate  table;
    char*                    header = "|   ID:client          | EFFI   | BWavg  | BWmax  | UTILavg  | UTILmax  |";

    if (hal_miu_bw_mem_alloc(bw) < 0)
    {
        hal_miu_err("[BW] alloc memory for BW failed!\n");
        return -1;
    }

    hal_miu_tabulate_init(&table, bw->bw_buf, bw->bw_buf + HAL_MIU_BW_BUF_MAX, header);

    for_each_ip(bw->client, ip)
    {
        if (!ip->bw_enable)
        {
            continue;
        }

        strcpy(name, ip->name);
        len = strlen(name);
        if (name[len - 2] == '_' && (name[len - 1] == 'R' || name[len - 1] == 'W'))
        {
            name[len - 2] = '\0';
        }

        if (ip->read)
        {
            hal_miu_bw_start_one(bw, ip->id, HAL_MIU_RO, &statis);
            hal_miu_tabulate_add_row(&table, "|%#04x:%s_R|%2llu.%d|%2llu.%d|%2llu.%d|%2llu.%d|%2llu.%d|", ip->id, name,
                                     statis.effi_avg.integer, statis.effi_avg.decimal, statis.bw_avg.integer,
                                     statis.bw_avg.decimal, statis.bw_max.integer, statis.bw_max.decimal,
                                     statis.util_avg.integer, statis.util_avg.decimal, statis.util_max.integer,
                                     statis.util_max.decimal);
        }
        if (ip->write)
        {
            hal_miu_bw_start_one(bw, ip->id, HAL_MIU_WO, &statis);
            hal_miu_tabulate_add_row(&table, "|%#04x:%s_W|%2llu.%d|%2llu.%d|%2llu.%d|%2llu.%d|%2llu.%d|", ip->id, name,
                                     statis.effi_avg.integer, statis.effi_avg.decimal, statis.bw_avg.integer,
                                     statis.bw_avg.decimal, statis.bw_max.integer, statis.bw_max.decimal,
                                     statis.util_avg.integer, statis.util_avg.decimal, statis.util_max.integer,
                                     statis.util_max.decimal);
        }
    }

    hal_miu_tabulate_finish(&table);

    return 0;
}

static int hal_miu_bw_show_enable(struct hal_miu_bw* bw, char* buf, char* end)
{
    char*                   str = buf;
    struct hal_miu_ip*      ip;
    struct hal_miu_tabulate table;

    hal_miu_tabulate_init(&table, buf, end, "| ID   | name          | enable |");
    for_each_ip(bw->client, ip)
    {
        hal_miu_tabulate_add_row(&table, "|%#04x|%s|%d|", ip->id, ip->name, ip->bw_enable);
    }
    str = hal_miu_tabulate_finish(&table);

    return (str - buf);
}

static int hal_miu_bw_set_enable(struct hal_miu_bw* bw, int id, bool enable, bool all)
{
    struct hal_miu_ip* ip;

    for_each_ip(bw->client, ip)
    {
        if (all || ip->id == id)
        {
            hal_miu_info("id: %#04x, name: %s, enable: %d\n", ip->id, ip->name, enable);
            ip->bw_enable = enable;
            continue;
        }
    }

    return 0;
}
#endif

static int hal_miu_bw_show(struct hal_miu_bw* bw, loff_t offset, char* buf, char* end)
{
    int ret  = 0;
    int size = 0;

    if (offset == 0)
    {
        ret = hal_miu_bw_start(bw);
    }

    if (ret < 0 || !bw->bw_buf)
    {
        return 0;
    }

    if (offset < strlen(bw->bw_buf))
    {
        size = min((long long)(end - buf), strlen(bw->bw_buf) - offset);
        memcpy(buf, bw->bw_buf + offset, size);

        return size;
    }

    hal_miu_bw_mem_free(bw);

    return 0;
}

int hal_miu_bw_init(hal_miu_object obj, struct hal_miu_client* client)
{
    struct hal_miu_bw* bw = (struct hal_miu_bw*)obj;
    struct hal_miu_bw  temp_bw =
    {
        .client = client,
        .callback =
            {
                .show = hal_miu_bw_show,
#if defined(CONFIG_MIU_BWLA)
                .store      = hal_miu_bwla_store,
                .tool_start = hal_miu_bw_tool_start,
                .tool_stop  = hal_miu_bw_tool_stop,
                .tool_show  = hal_miu_bw_tool_show,
#else
                .show_enable = hal_miu_bw_show_enable,
                .set_enable  = hal_miu_bw_set_enable,
#endif
            },
#if defined(CONFIG_MIU_BWLA)
        .round       = HAL_MIU_BWLA_ROUNDS,
        .sample_mode = 0, // 0/1/2/3: 1M/512K/256K/128K
#endif
    };

#if !defined(CONFIG_MIU_BWLA)
    client->callback.id_to_ip(client, 0, HAL_MIU_RW)->bw_enable = true;
#endif

    *bw = temp_bw;

    return 0;
}

int hal_miu_bw_free(hal_miu_object obj)
{
    return 0;
}
#endif // defined(CONFIG_MIU_BWLA)
//=================================================================================================
//                                     Hal MIU Init
//=================================================================================================

int hal_miu_init(void)
{
    return 0;
}

//=================================================================================================
