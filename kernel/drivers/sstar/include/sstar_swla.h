/*
 * sstar_swla.h - Sigmastar
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

#ifndef __SSTAR_SWLA_H__
#define __SSTAR_SWLA_H__

#define SWLA_LOG_MAJOR_VERSION (2)
#define SWLA_LOG_MINOR_VERSION (0)

#define SWLA_LOG_FORMAT_TYPE (1) // 0 is little weight log(16bytes), 1 is full log(32bytes)
#if SWLA_LOG_FORMAT_TYPE == 0
#define SWLA_LOG_NAME_LENGTH (8)
#else
#define SWLA_LOG_NAME_LENGTH (16)
#endif
#define SWLA_LOG_MAGIC_NUMBER (0x535353574C417632)

#define SWLA_LOG_STOP_LABEL "_LaStop_"
#ifdef CONFIG_SS_DUALOS
#define INTEROS_SC_L2R_SWLA_CTRL 0xF1050000
#define INTEROS_SC_R2L_SWLA_CTRL 0xF1050001

typedef enum
{
    SS_SWLA_REMOTE_CTRL_SYNC_BUF_ADDR = 0,
    SS_SWLA_REMOTE_CTRL_START,
    SS_SWLA_REMOTE_CTRL_STOP,
    SS_SWLA_REMOTE_CTRL_DUMP_NOTIFY,
    SS_SWLA_REMOTE_CTRL_MAX
} SS_SWLA_REMOTE_CTRL_OPTION_e;
#endif

typedef enum
{
    SS_SWLA_LOG_START     = 0,
    SS_SWLA_LOG_STOP      = 1,
    SS_SWLA_LOG_LABEL     = 2,
    SS_SWLA_LOG_SWITCH_IN = 3
} SS_SWLA_LOG_TYPE_e;

typedef enum
{
    SS_SWLA_DUMP_FILE_LOCAL = 0,
    SS_SWLA_DUMP_FILE_REMOTE,
    SS_SWLA_DUMP_FILE_DUALOS,
    SS_SWLA_DUMP_UART_LOCAL,
    SS_SWLA_DUMP_METHOD_MAX
} SS_SWLA_DUMP_METHOD_e;

typedef enum
{
    SS_SWLA_OS_RTOS  = 0,
    SS_SWLA_OS_LINUX = 1,
    SS_SWLA_OS_TYPE_MAX
} SS_SWLA_OS_TYPE_e;

typedef enum
{
    SS_SWLA_REC_LOCAL = 0,
    SS_SWLA_REC_REMOTE,
    SS_SWLA_REC_DUALOS,
    SS_SWLA_REC_TYPE_MAX
} SS_SWLA_REC_TYPE_e;

typedef enum
{
    SS_SWLA_GET_BUF_PA = 0,
    SS_SWLA_GET_BUF_SIZE_KB,
    SS_SWLA_GET_START_STATUS,
    SS_SWLA_GET_START_TYPE,
    SS_SWLA_GET_START_OVERWRITE,
    SS_SWLA_GET_INFO_MAX
} SS_SWLA_GET_INFO_e;

typedef struct __attribute__((packed))
{
    u64 magic_number;
    u16 header_size;
    u8  major_version;
    u8  minor_version;
    u32 tick_freq;
    u64 start_tick;
    u8  log_format;
    u8  os_type;
    u8  buf_kb_shift;
    u8  reserved[37];
} SS_SWLA_LOG_HEAD_t;

typedef struct __attribute__((packed))
{
    u8  log_type : 2;
    u8  cpu_id : 4;
    u64 timestamp_tick : 58;
    u8  log_name[SWLA_LOG_NAME_LENGTH];
#if SWLA_LOG_FORMAT_TYPE == 1
    u8 reserved[8];
#endif
} SS_SWLA_LOG_t;

typedef struct
{
    void *handle;
    void (*init)(void *);
    void (*write)(void *, const unsigned char *, u32);
    void (*finish)(void *);
} SwlaOutputOp_t;

#if CONFIG_SSTAR_SWLA
void sys_swla_init(void);
void sys_swla_start(SS_SWLA_REC_TYPE_e type, u32 enable_overwrite);
void sys_swla_stop(SS_SWLA_REC_TYPE_e type);
void sys_swla_log_add_event(const char *event_name, const SS_SWLA_LOG_TYPE_e log_type);
void sys_swla_log_add_irq(const unsigned int irq_num, const SS_SWLA_LOG_TYPE_e log_type);
void sys_swla_dump(SS_SWLA_DUMP_METHOD_e dump_method, const char *file_name);
u64  sys_swla_get_info(SS_SWLA_GET_INFO_e option);
#else
#define sys_swla_init(...)
#define sys_swla_start(...)
#define sys_swla_stop(...)
#define sys_swla_log_add_event(...)
#define sys_swla_log_add_irq(...)
#define sys_swla_dump(...)
#define sys_swla_get_info(...) (u64)(0)
#endif

#endif //__SSTAR_SWLA_H__