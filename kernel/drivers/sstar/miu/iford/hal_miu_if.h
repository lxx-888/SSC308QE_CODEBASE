/*
 * hal_miu_if.h - Sigmastar
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
#ifndef __HAL_MIU_IF_H__
#define __HAL_MIU_IF_H__

#include <cam_os_wrapper.h>
#include <drv_miu.h>

//=================================================================================================
//                                     Hal MIU Chip Releated
//=================================================================================================

// #ifndef CONFIG_MIU_HW_MMU
// #define CONFIG_MIU_HW_MMU
// #endif

#if defined(CONFIG_MIU_HW_MMU)
#define HAL_MIU_MAX_PGSZ_NUM  (3)
#define HAL_MIU_MMU_ADDR_BIT  (36)
#define HAL_MIU_MMU_ENTRY_BIT (12)

#define HAL_MIU_MMU_INVALID_ENTRY ((1 << HAL_MIU_MMU_ENTRY_BIT) - 1)

#define HAL_MIU_MMU_PROTECT_MIN (32)
#endif

#define HAL_MIU_PROTECT_MAX_ID    (32)
#define HAL_MIU_PROTECT_MAX_BLOCK (64)
#define HAL_MIU_PROTECT_INVLID_ID (0)

#define HAL_MIU_PROTECT_ADDR_UNIT (0x10)    // 16KB
#define HAL_MIU_PROTECT_SHIFT     (1 << 12) // Unit for MIU protect (4KB)

#define HAL_MIU_CLIENT_MAX_ID (0x7f)

#define HAL_MIU_DRAM_MIN_SIZE (2)         // MB
#define HAL_MIU_DRAM_MAX_SIZE (64 * 1024) // MB

#define HAL_MIU_BIST_NUM         (3)
#define HAL_MIU_MAX_BURST_HIGH   (0xff)
#define HAL_MIU_MAX_BURST_NORMAL (0x03)

#define HAL_MIU_MAX_PRIORITY_HIGH   (0x03)
#define HAL_MIU_MAX_PRIORITY_NORMAL (0x03)
#define HAL_MIU_MAX_VP              (0x03)

#define HAL_MIU_MAX_MASK_PERIOD_HIGH   (0xff)
#define HAL_MIU_MAX_MASK_PERIOD_NORMAL (0xff)

// 0:16T, 1:8T, 2:4T, 3:1T
#define HAL_MIU_MAX_PASS_PERIOD_HIGH   (0x03)
#define HAL_MIU_MAX_PASS_PERIOD_NORMAL (0x03)

#define HAL_MIU_MAX_QOS_LEVEL     (0x0c)
#define HAL_MIU_MAX_GRP_BURST_NUM (0x04)
//================================================================================================
//                                     Hal MIU Code Releated
//=================================================================================================
#define HAL_MIU_PROTECT_BUF (PAGE_SIZE >> 2) // 1KB

#define HAL_MIU_BIST_LENGTH (512 * 1024) // 512KB

#define HAL_MIU_BW_BUF_MAX (PAGE_SIZE << 2) // 16KB
#define HAL_MIU_BW_WAIT_MS (10)

#define HAL_MIU_TABLE_CELL_MAX (30)

#define HAL_MIU_CLIENT_BUF_MAX (PAGE_SIZE << 2) // 16KB

#define HAL_MIU_BWLA_ROUNDS     (32)
#define HAL_MIU_BWLA_ROUND_SIZE (512)
#define HAL_MIU_BWLA_MAX_IP     (64)

#define hal_miu_err(fmt, args...)  CamOsPrintf(KERN_ERR "[HAL MIU ERR] [%s@%d] " fmt, __FUNCTION__, __LINE__, ##args)
#define hal_miu_info(fmt, args...) CamOsPrintf(KERN_INFO fmt, ##args)

//=================================================================================================
//                                     Hal MIU Enum
//=================================================================================================

enum hal_miu_rw_mode
{
    HAL_MIU_RO = 1,
    HAL_MIU_WO = 2,
    HAL_MIU_RW = 3,
};

#define hal_miu_rw_str(rw) rw == HAL_MIU_RO ? "read" : "write"

enum hal_miu_dram_type
{
    HAL_MIU_DRAM_DDR4    = 0,
    HAL_MIU_DRAM_DDR3    = 1,
    HAL_MIU_DRAM_DDR2    = 2,
    HAL_MIU_DRAM_LPDDR4  = 4,
    HAL_MIU_DRAM_LPDDR3  = 5,
    HAL_MIU_DRAM_LPDDR2  = 6,
    HAL_MIU_DRAM_LPDDR4X = 7,
};

//=================================================================================================
//                                     Hal MIU Common
//=================================================================================================
#if defined(CAM_OS_RTK)
typedef long long loff_t;
#endif
struct hal_miu_client;
typedef void* hal_miu_object;

#define HAL_MIU_OBJECT(obj)                                                        \
    int hal_miu_##obj##_init(hal_miu_object obj, struct hal_miu_client* __client); \
    int hal_miu_##obj##_free(hal_miu_object obj);

struct hal_miu_mem
{
    bool           flag;
    u32            size;
    ss_phys_addr_t phy;
    ss_miu_addr_t  miu;
    void*          virt;
};

struct hal_miu_float
{
    u64 integer;
    u16 decimal;
};

#define hal_miu_float_compare(a, b) \
    (a.integer > b.integer ? 1 : (a.integer < b.integer ? 0 : (a.decimal > b.decimal ? 1 : 0)))

struct hal_miu_tabulate
{
    char* buf;
    char* end;
    char* header;
    char* row;
    char* split;
    int*  cell;
    int   column;
    int   width;
};
//=================================================================================================
//                                     Hal MIU Client Struct
//=================================================================================================
struct hal_miu_client;

struct hal_miu_ip_attr
{
    bool mask;
    u8   qos;
    u8   limit;
    u8   priority;
    u8   vp;
    u8   mask_period;
    u8   pass_period;
    bool urgent;
};

struct hal_miu_ip
{
    char*                  name;
    u8                     id;
    bool                   read;
    bool                   write;
    bool                   high_way;
    bool                   bw_enable;
    struct hal_miu_ip_attr read_attr;
    struct hal_miu_ip_attr write_attr;
};

#if defined(CAM_OS_LINUX_KERNEL)
struct hal_miu_client_func
{
    char* name;
    char* help;
    int (*call)(struct hal_miu_ip* ip, enum hal_miu_rw_mode rw, const char* buf, size_t n);
};
#endif

struct hal_miu_client_callback
{
    int (*id_valid)(struct hal_miu_client* client, int id, enum hal_miu_rw_mode rw);
    char* (*id_to_name)(struct hal_miu_client* client, int id, enum hal_miu_rw_mode rw);
    int (*name_to_id)(struct hal_miu_client* client, char* name, enum hal_miu_rw_mode rw);
    struct hal_miu_ip* (*id_to_ip)(struct hal_miu_client* client, int id, enum hal_miu_rw_mode rw);
#if defined(CAM_OS_LINUX_KERNEL)
    int (*show_ip_table)(struct hal_miu_client* client, char* buf, char* end);
    int (*show_client)(struct hal_miu_client* client, loff_t offset, char* buf, char* end);
    int (*store_client)(struct hal_miu_client* client, const char* buf, u32 size);
#endif
    int (*module_reset)(struct hal_miu_client* client, int id, enum hal_miu_rw_mode rw);
};

struct hal_miu_client
{
    int                            ip_num;
    struct hal_miu_ip*             ip_table;
    struct hal_miu_client_callback callback;
#if defined(CAM_OS_LINUX_KERNEL)
    struct hal_miu_client_func* ip_func;
    int                         func_num;
    char*                       func_buf;
#endif
};

#define for_each_ip_index(client, i, ip) for (i = 0; i < client->ip_num; ip = &client->ip_table[++i])
#define for_each_ip(client, ip)          for (ip = client->ip_table; ip < client->ip_table + client->ip_num; ip++)

HAL_MIU_OBJECT(client);

//=================================================================================================
//                                     Hal MIU DRAM Struct
//=================================================================================================
struct hal_miu_dram;

struct hal_miu_dram_info
{
    u64 size;        // Bytes
    u32 dram_freq;   // MHz
    u32 miupll_freq; // MHz
    u8  type;        // 2:DDR2, 3:DDR3
    u8  data_rate;   // 4:4x mode, 8:8x mode,
    u8  bus_width;   // 16:16bit, 32:32bit, 64:64bit
    u8  ssc;         // 0:off, 1:on
};

struct hal_miu_dram_callbck
{
    int (*read_info)(struct hal_miu_dram* dram);
    int (*show_info)(struct hal_miu_dram* dram, char* buf, char* end);
    int (*set_size)(struct hal_miu_dram* dram, int size);
};

struct hal_miu_dram
{
    struct hal_miu_dram_info    info;
    struct hal_miu_dram_callbck callback;
};

int hal_miu_dram_info(struct miu_dram_info* info);

HAL_MIU_OBJECT(dram);

//=================================================================================================
//                                     Hal MIU Protect Struct
//=================================================================================================
struct hal_miu_protect;

struct hal_miu_protect_block
{
    u8             index;
    bool           enable;
    bool           invert;
    u8             white_num;
    u16*           whitelist;
    ss_phys_addr_t start_addr;
    ss_phys_addr_t end_addr;
};

struct hal_miu_protect_hit_info
{
    bool                          hit;
    bool                          hit_mmu;
    bool                          IsWrite;
    bool                          out_of_dram;
    ss_phys_addr_t                hit_addr;
    struct hal_miu_protect_block* hit_block;
    struct hal_miu_ip*            hit_ip;
};

struct hal_miu_protect_callback
{
    u8 (*query)(struct hal_miu_protect* protect, bool mmu);
    int (*enable)(struct hal_miu_protect* protect, struct hal_miu_protect_block* block);
    int (*disable)(struct hal_miu_protect* protect, u8 index);
    bool (*contain)(struct hal_miu_protect* protect, u8 index, u16 id);
    int (*append_id)(struct hal_miu_protect* protect, u8 index, u16 id);
    int (*remove_id)(struct hal_miu_protect* protect, u8 index, u16 id);
    u16* (*get_kernel_whitelist)(struct hal_miu_protect* protect);
    u16* (*get_kernel_blacklist)(struct hal_miu_protect* protect);
    struct hal_miu_protect_block* (*get_block)(struct hal_miu_protect* protect, u8 index);
    int (*get_hit_info)(struct hal_miu_protect* protect, struct hal_miu_protect_hit_info* info);
    int (*show_hit_info)(struct hal_miu_protect* protect, struct hal_miu_protect_hit_info* info);
    int (*show_block_info)(struct hal_miu_protect* protect, struct hal_miu_protect_block* block);
    int (*clear_interrupt)(struct hal_miu_protect* protect);
};

struct hal_miu_protect
{
    int block_num;
    int id_num;
    u16 protect_id[HAL_MIU_PROTECT_MAX_ID];
#if defined(CONFIG_MIU_HW_MMU)
    u16 mmu_protect_id[HAL_MIU_PROTECT_MAX_ID];
#endif
    u16                             kernel_whitelist[HAL_MIU_PROTECT_MAX_ID];
    u16                             kernel_blacklist[HAL_MIU_PROTECT_MAX_ID];
    struct hal_miu_protect_block    block[HAL_MIU_PROTECT_MAX_BLOCK];
    u64                             hit_count;
    struct hal_miu_protect_hit_info hit_info;
    struct hal_miu_client*          client;
    struct hal_miu_protect_callback callback;
};

HAL_MIU_OBJECT(protect);

//=================================================================================================
//                                     Hal MIU MMU Struct
//=================================================================================================
#if defined(CONFIG_MIU_HW_MMU)
struct hal_miu_mmu;

struct hal_mmu_info
{
    bool enable;

    u32 page_list[HAL_MIU_MAX_PGSZ_NUM];

    u8  page_mode;
    u32 page_size;
    u32 page_bit;

    u32 entry_size;
    u32 entry_bit;
    u32 max_entry;

    u32 region_size;
    u32 region_bit;
    u32 max_region;

    u16 vpa_region;
    u16 pa_region;
};

struct hal_mmu_callback
{
    int (*set_page_size)(struct hal_miu_mmu* mmu, u8 mode);
    u32 (*get_page_size)(struct hal_miu_mmu* mmu);
    int (*set_region)(struct hal_miu_mmu* mmu, u16 vpa_region, u16 pa_region);
    int (*map_entry)(struct hal_miu_mmu* mmu, u16 vpa_entry, u16 pa_entry);
    int (*unmap_entry)(struct hal_miu_mmu* mmu, u16 vpa_entry);
    u16 (*query_entry)(struct hal_miu_mmu* mmu, u16 vpa_entry);
    int (*enable)(struct hal_miu_mmu* mmu, bool enable);
    int (*reset)(struct hal_miu_mmu* mmu);
    int (*get_irq_status)(struct hal_miu_mmu* mmu, u16* entry, u16* id, u8* is_write);
};

struct hal_miu_mmu
{
    struct hal_mmu_info     info;
    struct hal_mmu_callback callback;
};

HAL_MIU_OBJECT(mmu);
#endif

#if defined(CAM_OS_LINUX_KERNEL)
//=================================================================================================
//                                     Hal MIU Bist Struct
//=================================================================================================

struct hal_miu_bist;

struct hal_miu_bist_info
{
    bool                 chip_top;
    bool                 start;
    bool                 success;
    bool                 loop;
    enum hal_miu_rw_mode mode;
    u8                   id;
    u32                  group_bank;
    u32                  effi_bank;
    u16                  cmd_mode;
};

struct hal_miu_bist_callback
{
    int (*start)(struct hal_miu_bist* bist, bool loop, enum hal_miu_rw_mode rw, bool* mask,
                 struct hal_miu_protect* protect);
    int (*stop)(struct hal_miu_bist* bist, bool* mask, struct hal_miu_protect* protect);
    int (*set_size)(struct hal_miu_bist* bist, u32 size);
    int (*get_id)(struct hal_miu_bist* bist, int index);
    int (*cmd_mode)(struct hal_miu_bist* bist, u16 mode);
    int (*show)(struct hal_miu_bist* bist, char* buf, char* end);
    bool (*is_all_stop)(struct hal_miu_bist* bist);
};

struct hal_miu_bist
{
    int                          bist_num;
    struct hal_miu_mem           addr;
    bool                         mask[HAL_MIU_BIST_NUM];
    struct hal_miu_bist_info     bist[HAL_MIU_BIST_NUM];
    struct hal_miu_bist_callback callback;
};

HAL_MIU_OBJECT(bist);
#endif // defined(CAM_OS_LINUX_KERNEL)

//=================================================================================================
//                                     Hal MIU BW Struct
//=================================================================================================

struct hal_miu_bw;

#if defined(CONFIG_MIU_BWLA)
struct hal_miu_bwla_ip_raw
{
    u16 write;
    u16 read;
    u16 util;
    u16 total;
};

struct hal_miu_bw_statis
{
    struct hal_miu_float bw_total;
    struct hal_miu_float bw_read_avg;
    struct hal_miu_float bw_write_avg;
    struct hal_miu_float bw_read_max;
    struct hal_miu_float bw_write_max;
    struct hal_miu_float effi_avg;
    struct hal_miu_float effi_max;
    struct hal_miu_float effi_min;
    struct hal_miu_float util_avg;
    struct hal_miu_float util_max;
    struct hal_miu_float bw_util;
};
#else
struct hal_miu_bw_statis
{
    struct hal_miu_float effi_avg;
    struct hal_miu_float bw_avg;
    struct hal_miu_float bw_max;
    struct hal_miu_float util_avg;
    struct hal_miu_float util_max;
};
#endif

struct hal_miu_bw_callback
{
    int (*show)(struct hal_miu_bw* bw, loff_t offset, char* buf, char* end);
#if defined(CONFIG_MIU_BWLA)
    int (*store)(struct hal_miu_bw* bw, int round);
    int (*tool_start)(struct hal_miu_bw*);
    int (*tool_stop)(struct hal_miu_bw*);
    int (*tool_show)(struct hal_miu_bw* bw, char* buf, char* end);
#else
    int (*show_enable)(struct hal_miu_bw* bw, char* buf, char* end);
    int (*set_enable)(struct hal_miu_bw* bw, int id, bool enable, bool all);
#endif
};

struct hal_miu_bw
{
    struct hal_miu_client* client;
#if defined(CONFIG_MIU_BWLA)
    int                       round;
    int                       sample_mode;
    struct hal_miu_mem        mem;
    struct hal_miu_bw_statis* statis;
#endif
    struct hal_miu_bw_callback callback;
    char*                      bw_buf;
};

HAL_MIU_OBJECT(bw);

//=================================================================================================
//                                     Hal MIU API
//=================================================================================================

int hal_miu_init(void);

#if defined(CAM_OS_RTK)
int hal_miu_qos_init(void);
#endif

#endif // #ifndef __HAL_MIU_IF_H__
