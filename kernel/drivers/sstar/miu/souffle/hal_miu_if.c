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

#define HAL_MIU_PROTECT_KERNEL_WHITELIST                                                                       \
    {                                                                                                          \
        MIU_CLIENTW_CA55, MIU_CLIENTW_USB30, MIU_CLIENTW_AESDMA, MIU_CLIENTW_MIIC_TOP, MIU_CLIENTW_GMAC0,      \
            MIU_CLIENTW_GMAC1, MIU_CLIENTW_GMAC_TOE0, MIU_CLIENTW_GMAC_TOE1, MIU_CLIENTW_GMAC_TOE2,            \
            MIU_CLIENTW_GMAC_TOE3, MIU_CLIENTW_GMAC_TOE4, MIU_CLIENTW_GMAC_TOE5, MIU_CLIENTW_GMAC_TOE6,        \
            MIU_CLIENTW_GMAC_TOE7, MIU_CLIENTW_XZ_DECODE0, MIU_CLIENTW_XZ_DECODE1, MIU_CLIENTW_BDMA,           \
            MIU_CLIENTW_BDMA2, MIU_CLIENTW_BDMA3, MIU_CLIENTW_LOW_SPEED1, MIU_CLIENTW_SD, MIU_CLIENTW_FCIE,    \
            MIU_CLIENTW_SDIO, MIU_CLIENTW_LOW_SPEED0, MIU_CLIENTW_BWLA, MIU_CLIENTW_DBG_WDMA, MIU_CLIENTW_NULL \
    }

#define HAL_MIU_PROTECT_KERNEL_BLACKLIST                                                                               \
    {                                                                                                                  \
        MIU_CLIENTW_SC_WDMA0, MIU_CLIENTW_SC_WDMA1, MIU_CLIENTW_SC_WDMA2, MIU_CLIENTW_SC_WDMA3, MIU_CLIENTW_SC_WDMA4,  \
            MIU_CLIENTW_SC_WDMA5, MIU_CLIENTW_SC_WDMA6, MIU_CLIENTW_JPE, MIU_CLIENTW_JPD0, MIU_CLIENTW_IVE,            \
            MIU_CLIENTW_SC_COL_INV, MIU_CLIENTW_SGDMA, MIU_CLIENTW_BACH0, MIU_CLIENTW_BDMA, MIU_CLIENTW_CVS,           \
            MIU_CLIENTW_BACH1, MIU_CLIENTW_LDC, MIU_CLIENTW_ISP_DMAG0, MIU_CLIENTW_ISP_DMAG1, MIU_CLIENTW_ISP_DMAG2,   \
            MIU_CLIENTW_ISP_DMAG3, MIU_CLIENTW_ISP_WDR, MIU_CLIENTW_ISP_ROT, MIU_CLIENTW_ISP_STA, MIU_CLIENTW_ISP_IMG, \
            MIU_CLIENTW_ISP_TNR, MIU_CLIENTW_ISP_VIF_STA, MIU_CLIENTW_VENC0, MIU_CLIENTW_IPU, MIU_CLIENTW_VENC,        \
            MIU_CLIENTW_NULL                                                                                           \
    }

// update time: 2023/11/08
static struct hal_miu_ip __ip_table[] = {
    {"OVERALL", 0x00, true, true},
    {"GOP_JPE0_SC_WDMA0", 0x10, true, true},
    {"SC_WDMA1", 0x11, false, true},
    {"SC_WDMA2", 0x12, false, true},
    {"SC_WDMA3", 0x13, false, true},
    {"USB30", 0x14, true, true},
    {"SC_RDMA0_SC_WDMA4", 0x15, true, true},
    {"GOP0_SC_SC_WMDA5", 0x16, true, true},
    {"JPE", 0x17, true, true},
    {"GOP1_SC_SC_WDMA6", 0x18, true, true},
    {"JPD0", 0x19, true, true},
    {"IVE", 0x1a, true, true},
    {"AESDMA", 0x1b, true, true},
    {"SC_COL_INV", 0x1c, true, true},
    {"SC_RDMA1", 0x1d, true, false},
    {"MIIC_TOP", 0x1e, true, true},
    {"BIST_SC0", 0x1f, true, true},

    {"GMAC0", 0x30, true, true},
    {"GMAC1", 0x31, true, true},
    {"GMAC_TOE0", 0x32, true, true},
    {"GMAC_TOE1", 0x33, true, true},
    {"GMAC_TOE2", 0x34, true, true},
    {"GMAC_TOE3", 0x35, true, true},
    {"GMAC_TOE4", 0x36, true, true},
    {"GMAC_TOE5", 0x3a, true, true},
    {"GMAC_TOE6", 0x3b, true, true},
    {"GMAC_TOE7", 0x3c, true, true},
    {"SGDMA", 0x3d, false, true},
    {"BIST_SC1", 0x3f, false, true},

    {"XZ_DECODE", 0x40, true, true},
    {"CMDQ1_XZ_DECODE", 0x41, true, true},
    {"ISP0_CMDQ", 0x42, true, false},
    {"BACH", 0x43, true, true},
    {"CMDQ_dbg_wdma", 0x44, true, true},
    {"BDMA", 0x45, true, true},
    {"BDMA2", 0x46, true, true},
    {"BDMA3", 0x47, true, true},
    {"LOW_SPEED1", 0x48, true, true},
    {"SD", 0x49, true, true},
    {"FCIE", 0x4a, true, true},
    {"SDIO", 0x4b, true, true},
    {"CVS", 0x4c, true, true},
    {"BACH", 0x4d, true, true},
    {"LOW_SPEED0", 0x4e, true, true},
    {"LDC", 0x4f, true, true},

    {"ISP_DMAG", 0x50, true, true},
    {"ISP_MLOAD_ISP_DMAG", 0x51, true, true},
    {"ISP_3DNR", 0x52, true, true},
    {"ISP_TNR_ISP_WDR", 0x53, true, true},
    {"ISP_ROT", 0x54, true, true},
    {"ISP_WDR_ISP_STA", 0x55, true, true},
    {"ISP_DMAG_ISP_IMG", 0x56, true, true},
    {"ISP_DMAG", 0x57, true, true},
    {"ISP_DMAG", 0x58, true, true},
    {"ISP_FPN_ISP_TNR", 0x59, true, true},
    {"ISP_VIF_STA", 0x5a, false, true},
    {"VENC0", 0x5e, true, true},
    {"BIST_ISP", 0x5f, true, true},

    {"ipu", 0x71, true, true, true},
    {"CPU_CA55", 0x72, true, true, true},
    {"VENC0", 0x73, true, true, true},
    {"BWLA", 0x75, true, true, true},
};

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
        CamOsContiguousMemRelease(mem->phy);
    }

    mem->phy  = 0;
    mem->miu  = 0;
    mem->virt = NULL;
}

#if defined(CAM_OS_LINUX_KERNEL)
struct hal_miu_float hal_miu_divide(u64 numerator, u64 denominator)
{
    struct hal_miu_float res = {numerator * 100 / denominator, numerator * 100000 / denominator};

    // The 64-bit remainder operation will be wrong, it is fixed in commit 315106,
    // so avoid using the remainder operation here
    res.decimal -= res.integer * 1000;

    return res;
}

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
    // TODO
    hal_miu_err("Not support!!\n");

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
    return 0;
}

static int hal_miu_client_burst(struct hal_miu_ip* ip, enum hal_miu_rw_mode rw, const char* buf, u32 n)
{
    return 0;
}

static int hal_miu_client_priority(struct hal_miu_ip* ip, enum hal_miu_rw_mode rw, const char* buf, u32 n)
{
    return 0;
}

static int hal_miu_client_vp(struct hal_miu_ip* ip, enum hal_miu_rw_mode rw, const char* buf, u32 n)
{
    return 0;
}

static int hal_miu_client_flowctrl(struct hal_miu_ip* ip, enum hal_miu_rw_mode rw, const char* buf, u32 n)
{
    return 0;
}

static int hal_miu_client_urgent(struct hal_miu_ip* ip, enum hal_miu_rw_mode rw, const char* buf, u32 n)
{
    return 0;
}

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
    "+--------------------------+------------------------------------------------+-------------------------------------"
    "------"
    "-----+\n"
    "|                          |                    Read                        |                    Write            "
    "      "
    "     |\n";

static char* client_header2 =
    "|   ID:Name                | mask | qos | burst | pri | vpr | flowctrl | hp | mask | qos | burst | pri | vpw | "
    "flowctrl "
    "| hp |\n";

static int hal_miu_client_format_attr(char* buf, bool enable, struct hal_miu_ip_attr* attr)
{
    if (enable)
    {
        return CamOsSprintf(buf, "|%u|%u|%-3u|%u|%u|0x%02x/%x|%u|", attr->mask, attr->qos, attr->limit, attr->priority,
                            attr->vp, attr->mask_period, attr->pass_period, attr->urgent);
    }
    else
    {
        return CamOsSprintf(buf, "|-|-|-|-|-|-|-|");
    }
}

static int hal_miu_client_read_all_attr(struct hal_miu_client* client)
{
    struct hal_miu_ip*      ip;
    struct hal_miu_tabulate table;
    char                    line[200];
    char*                   buf = line;

    if (hal_miu_client_mem_alloc(client) < 0)
    {
        return -1;
    }

    client->func_buf += CamOsSprintf(client->func_buf, client_header1);
    hal_miu_tabulate_init(&table, client->func_buf, client->func_buf + HAL_MIU_CLIENT_BUF_MAX, client_header2);
    for_each_ip(client, ip)
    {
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

    CamOsSnprintf(buf, end - buf, "%-13s", "DRAM Type:");
    switch (dram->info.type)
    {
        case HAL_MIU_DRAM_DDR4:
            CamOsSnprintf(buf, end - buf, "DDR4\n");
            break;
        case HAL_MIU_DRAM_DDR3:
            CamOsSnprintf(buf, end - buf, "DDR3\n");
            break;
        case HAL_MIU_DRAM_DDR2:
            CamOsSnprintf(buf, end - buf, "DDR2\n");
            break;
        case HAL_MIU_DRAM_LPDDR4:
            CamOsSnprintf(buf, end - buf, "LPDDR4\n");
            break;
        case HAL_MIU_DRAM_LPDDR3:
            CamOsSnprintf(buf, end - buf, "LPDDR3\n");
            break;
        case HAL_MIU_DRAM_LPDDR2:
            CamOsSnprintf(buf, end - buf, "LPDDR2\n");
            break;
        default:
            CamOsSnprintf(buf, end - buf, "Unknow\n");
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

#if defined(CONFIG_MIU_HW_MMU)
    if (index >= HAL_MIU_MMU_PROTECT_MIN)
    {
        id_list = protect->mmu_protect_id;
    }
#endif

    // 1. find all id in use after remove block[index]
    for (i = 0; i < protect->block_num; i++)
    {
        if (i == index || !protect->block[i].enable)
            continue;

        for (j = 0; j < protect->id_num; j++)
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
    new_block.whitelist = CamOsMemAlloc(new_block.white_num * sizeof(u16));
    if (new_block.whitelist == NULL)
    {
        return -1;
    }
    memcpy(new_block.whitelist, block->whitelist, block->white_num * sizeof(u16));

    new_block.whitelist[new_block.white_num - 2] = id;
    new_block.whitelist[new_block.white_num - 1] = HAL_MIU_PROTECT_INVLID_ID;

    new_block.start_addr = CamOsMemMiuToPhys(block->start_addr);
    new_block.end_addr   = CamOsMemMiuToPhys(block->end_addr);

    return protect->callback.enable(protect, &new_block);
}

static int hal_miu_protect_remove_id(struct hal_miu_protect* protect, u8 index, u16 id)
{
    int                           i     = 0;
    int                           j     = 0;
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
    new_block.whitelist = CamOsMemAlloc(new_block.white_num * sizeof(u16));
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

    return protect->callback.enable(protect, &new_block);
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
    int ret = 0;
    int block, id;
    int max_block = protect->block_num;

    reg_miu_protect_irq_mask(mmu, 1);

    info->hit     = reg_miu_protect_flag(mmu);
    info->hit_mmu = mmu;

    if (!info->hit)
    {
        ret = -1;
        goto GET_FAIL;
    }

    info->hit_addr = reg_miu_protect_hit_addr(mmu);

    block = reg_miu_protect_hit_block(mmu);
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

    id           = reg_miu_protect_hit_id(mmu);
    info->hit_ip = protect->client->callback.id_to_ip(protect->client, id, HAL_MIU_WO);

    if (!info->hit_ip)
    {
        hal_miu_err("[PROTECT] Unknow IP hit protect: %#x.\n", id);
    }

    protect->hit_count++;

GET_FAIL:
    reg_miu_protect_log_clr(mmu, 1);
    reg_miu_protect_flag(mmu);
    reg_miu_protect_log_clr(mmu, 0);
    reg_miu_protect_irq_mask(mmu, 0);

    return ret;
}

// void reg_miu_protect_dump_bank(u32 bank)
// {
//     int i;

//     printk(KERN_CONT "BANK:0x%x\n00: ", bank);
//     bank = GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, bank << 8);
//     for (i = 0; i <= 0x7F; i++)
//     {
//         if (i && (i & 0x7) == 0)
//         {
//             printk(KERN_CONT "\n%02x: ", i);
//         }
//         printk(KERN_CONT "%#06x ", reg_miu_read(bank, i));
//     }
// }

static int hal_miu_protect_get_hit_info(struct hal_miu_protect* protect, struct hal_miu_protect_hit_info* info)
{
    int ret;

    // reg_miu_protect_dump_bank(0x1659); // for debug

    ret = hal_miu_protect_get_hit_info_real(protect, info, 1);
    if (ret == 0)
    {
        return 0;
    }

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
        buf += CamOsSnprintf(buf, end - buf, "[PROTECT] IP write out of dram.\n");
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
    buf += CamOsSnprintf(buf, end - buf, "Hit %s address: 0x%llx<->0x%llx\n", info->hit_mmu ? "MMU" : "MIU", start_addr,
                         end_addr);

    hal_miu_err("%s", protect_str);

    return 0;
}

static int hal_miu_protect_show_block_info(struct hal_miu_protect* protect, struct hal_miu_protect_block* block)
{
    return 0;
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
        block->white_num += id_enable & (1 << j);
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
    reg_miu_protect_flag(0);
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
    // int i;
    // u16 entry;

    if (enable == mmu->info.enable)
    {
        return 0;
    }

    // if (enable)
    // {
    //     for (i = 0; i < mmu->info.max_entry; ++i)
    //     {
    //         entry = hal_miu_mmu_query_entry(mmu, i);
    //         if (entry == HAL_MIU_MMU_INVALID_ENTRY)
    //         {
    //             hal_miu_mmu_map_entry(mmu, i, i);
    //             if (hal_miu_mmu_query_entry(mmu, i) != i)
    //             {
    //                 CamOsPanic("mmu map entry failed!\n");
    //             }
    //         }
    //     }
    // }

    mmu->info.enable = enable;
    reg_miu_mmu_enable(enable);

    return 0;
}

static int hal_miu_mmu_reset(struct hal_miu_mmu* mmu)
{
    reg_miu_mmu_reset();

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
    struct hal_miu_protect_block* block = protect->callback.get_block(protect, 0);

    if (!block || !block->enable)
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
        if (protect->callback.append_id(protect, 0, bist->callback.get_id(bist, i)) < 0)
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
    struct hal_miu_protect_block* block = protect->callback.get_block(protect, 0);

    if (!block || !block->enable)
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
        protect->callback.remove_id(protect, 0, bist->callback.get_id(bist, i));
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
        if (mask[i]) // mask[i] == true means stop
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
            hal_miu_info("[BIST] Please stop all bist before change size!\n");
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

static int hal_miu_bist_show(struct hal_miu_bist* bist, char* buf, char* end)
{
    int                       i;
    char*                     str = buf;
    struct hal_miu_tabulate   table;
    struct hal_miu_bist_info* bi;

    hal_miu_tabulate_init(&table, buf, end, "| Bist | type    | mode         | addr       | size     |");

    for (i = 0; i < bist->bist_num; i++)
    {
        bi = &bist->bist[i];
        hal_miu_tabulate_add_row(
            &table, "|%d|%s|%s|%#llx|%dKB|", i, bi->start ? (bi->loop ? "loop" : "oneshot") : "stop",
            bi->mode == HAL_MIU_RW ? "read & write" : (bi->mode == HAL_MIU_RO ? "read only" : "write only"),
            bist->addr.miu, bist->addr.size >> 10);
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
                .show        = hal_miu_bist_show,
            },
        .bist =
            {
                {.chip_top   = false,
                 .group_bank = BASE_REG_MIU_GRP_SC0,
                 .effi_bank  = BASE_REG_MIU_EFFI_SC0,
                 .id         = MIU_CLIENTW_BIST_SC0},
                {.chip_top   = false,
                 .group_bank = BASE_REG_MIU_GRP_SC1,
                 .effi_bank  = BASE_REG_MIU_EFFI_SC1,
                 .id         = MIU_CLIENTW_BIST_SC1},
                {.chip_top   = false,
                 .group_bank = BASE_REG_MIU_GRP_MISC0,
                 .effi_bank  = BASE_REG_MIU_EFFI_MISC0,
                 .id         = MIU_CLIENTW_BIST_MISC},
                {.chip_top   = false,
                 .group_bank = BASE_REG_MIU_GRP_ISP0,
                 .effi_bank  = BASE_REG_MIU_EFFI_ISP0,
                 .id         = MIU_CLIENTW_BIST_ISP},
            },
    };

    *bist = temp_bist;

    return 0;
}

int hal_miu_bist_free(hal_miu_object obj)
{
    return 0;
}

//=================================================================================================
//                                     Hal MIU BW Function
//=================================================================================================

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
    int i;

    if (!bw->bw_buf)
    {
        bw->bw_buf = CamOsMemAlloc(HAL_MIU_BW_BUF_MAX);
        if (!bw->bw_buf)
        {
            return -1;
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

#if defined(CONFIG_MIU_BWLA)

static int hal_miu_bwla_start_measure(struct hal_miu_bw* bw)
{
    int                i;
    int                max_wait_us = 6000000;
    struct hal_miu_ip* ip;

    reg_miu_bwla_reset();
    reg_miu_bwla_init();

    reg_miu_bwla_set_addr(bw->mem.miu);
    reg_miu_bwla_set_round(bw->round);
    reg_miu_bwla_set_enable(1);

    for_each_ip_index(bw->client, i, ip)
    {
        reg_miu_bwla_set_ip(i, ip->id);
    }

    while (!reg_miu_bwla_is_done() && max_wait_us)
    {
        CamOsUsDelay(1);
        max_wait_us--;
    }

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
    for_each_ip_index(bw->client, i, ip)
    {
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

    printk("bwla round = %d\n", round);

    return 0;
}
#else
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
    struct hal_miu_bw* bw      = (struct hal_miu_bw*)obj;
    struct hal_miu_bw  temp_bw = {
        .client = client,
        .callback =
            {
                .show = hal_miu_bw_show,
#if defined(CONFIG_MIU_BWLA)
                .store = hal_miu_bwla_store,
#else
                .show_enable = hal_miu_bw_show_enable,
                .set_enable  = hal_miu_bw_set_enable,
#endif
            },
        .round       = HAL_MIU_BWLA_ROUNDS,
        .sample_mode = 0,
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
#endif // defined(CAM_OS_LINUX_KERNEL)

//=================================================================================================
//                                     Hal MIU Init
//=================================================================================================

int hal_miu_init(void)
{
    reg_miu_bw_init();

    return 0;
}

//=================================================================================================
