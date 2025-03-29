/*
 * sstar_swla.c - Sigmastar
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

#if CONFIG_SSTAR_SWLA
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/smp.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <clocksource/arm_arch_timer.h>
#include "cam_os_wrapper.h"
#include "cam_fs_wrapper.h"
#include "sstar_base64_uart.h"
#include "sstar_swla.h"
#if defined(CONFIG_SS_DUALOS)
#include "cam_inter_os.h"
#endif

#define SWLA_DEBUG 0

#if SWLA_DEBUG
#define SWLA_DBG(fmt, arg...) printk(KERN_EMERG fmt, ##arg)
#else
#define SWLA_DBG(fmt, arg...)
#endif

static u32 swla_buf_size_kb = 128;
static u32 swla_auto_enable = 0;

static int __init get_swla_buf_size(char *str)
{
    u32 buf_size_kb;

    get_option(&str, &buf_size_kb);
    if (buf_size_kb)
    {
        swla_buf_size_kb = buf_size_kb;
    }
    return 0;
}
early_param("swla_buf_kb", get_swla_buf_size);

static int __init get_swla_auto_enable(char *str)
{
    u32 buf_auto_enable;

    get_option(&str, &buf_auto_enable);
    if (buf_auto_enable < 3)
    {
        swla_auto_enable = buf_auto_enable;
    }
    return 0;
}
early_param("swla_auto_en", get_swla_auto_enable);

typedef struct
{
    u8 *sys_swla_header;
#ifdef CONFIG_SS_DUALOS
    ss_phys_addr_t sys_swla_remote_buf_pa;
    CamOsWorkQueue work_queue;
#endif
    u8 *          sys_swla_buf;
    u8            sys_swla_log_start;
    u8            sys_swla_log_start_type;
    u8            sys_swla_log_stop_type;
    u8            sys_swla_log_overwrite;
    CamOsAtomic_t sys_swla_buf_idx_tick;
} SS_SWLA_OPT_t;

static u32           g_ss_swla_init = 0;
static SS_SWLA_OPT_t g_ss_swla_opt  = {0};
static u32           sys_swla_buf_size_filter;

#ifdef CONFIG_SS_DUALOS
static u32 sys_swla_ctrl_from_remote(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
    SWLA_DBG("[Linux]remote swla: 0x%x 0x%x 0x%x 0x%x\n", arg0, arg1, arg2, arg3);

    switch (arg1)
    {
        case SS_SWLA_REMOTE_CTRL_SYNC_BUF_ADDR:
            g_ss_swla_opt.sys_swla_remote_buf_pa = (u64)arg2 << 32 | (u64)arg3;
            SWLA_DBG("[Linux]remote swla: pa = 0x%llx\n", g_ss_swla_opt.sys_swla_remote_buf_pa);
            break;
        case SS_SWLA_REMOTE_CTRL_START:
            sys_swla_start(SS_SWLA_REC_LOCAL, arg2);
            SWLA_DBG("[Linux]remote swla: start overwrite = %u\n", arg2);
            break;
        case SS_SWLA_REMOTE_CTRL_STOP:
            sys_swla_stop(SS_SWLA_REC_LOCAL);
            SWLA_DBG("[Linux]remote swla: stop\n");
            break;
        case SS_SWLA_REMOTE_CTRL_DUMP_NOTIFY:
            if (g_ss_swla_opt.sys_swla_header)
            {
                CamOsMemFlush((void *)g_ss_swla_opt.sys_swla_header,
                              (swla_buf_size_kb << 10)
                                  + CAM_OS_ALIGN_UP(sizeof(SS_SWLA_LOG_HEAD_t), ARCH_DMA_MINALIGN));
            }
            break;
        default:
            break;
    }

    return 0;
}

static int __init sys_swla_dualos_init(void)
{
    ss_phys_addr_t     log_buf_pa = (ss_phys_addr_t)sys_swla_get_info(SS_SWLA_GET_BUF_PA);
    CamOsThreadAttrb_t wq_attr    = {.nPriority = 95, .nStackSize = 2048, .szName = "swla_wq"};

    if (0 != CamInterOsSignalReg(INTEROS_SC_R2L_SWLA_CTRL, sys_swla_ctrl_from_remote, "swla_ctrl"))
    {
        printk(KERN_EMERG "Linux SWLA: register signal callback fail\n");
    }

    CamInterOsSignal(INTEROS_SC_L2R_SWLA_CTRL, SS_SWLA_REMOTE_CTRL_SYNC_BUF_ADDR, (u32)(log_buf_pa >> 32),
                     (u32)(log_buf_pa & 0xFFFFFFFF));

    CamOsWorkQueueCreateExt(&g_ss_swla_opt.work_queue, &wq_attr, 5);

    return 0;
}
late_initcall(sys_swla_dualos_init);
#endif

void sys_swla_init(void)
{
    SS_SWLA_LOG_HEAD_t *psLogHeader = NULL;

    if (g_ss_swla_init)
        return;

    g_ss_swla_init = 1;

    swla_buf_size_kb = (1 << (CAM_OS_FLS(swla_buf_size_kb) - 1));

    if (NULL
        == (g_ss_swla_opt.sys_swla_header =
                kmalloc((swla_buf_size_kb << 10) + CAM_OS_ALIGN_UP(sizeof(SS_SWLA_LOG_HEAD_t), ARCH_DMA_MINALIGN),
                        ARCH_DMA_MINALIGN)))
    {
        printk(KERN_EMERG "[%s] memory allocation fail!!!\r\n", __FUNCTION__);
        return;
    }

    // init swla log header
    psLogHeader                = (SS_SWLA_LOG_HEAD_t *)g_ss_swla_opt.sys_swla_header;
    psLogHeader->magic_number  = SWLA_LOG_MAGIC_NUMBER;
    psLogHeader->header_size   = sizeof(SS_SWLA_LOG_HEAD_t);
    psLogHeader->major_version = SWLA_LOG_MAJOR_VERSION;
    psLogHeader->minor_version = SWLA_LOG_MINOR_VERSION;
    // arch timer rate is not set in this stage
    // psLogHeader->tick_freq     = arch_timer_get_rate();
    psLogHeader->log_format   = SWLA_LOG_FORMAT_TYPE;
    psLogHeader->os_type      = SS_SWLA_OS_LINUX;
    psLogHeader->buf_kb_shift = (u8)(CAM_OS_FLS(swla_buf_size_kb) - 1);
    memset(psLogHeader->reserved, 0, sizeof(psLogHeader->reserved));

    g_ss_swla_opt.sys_swla_buf =
        g_ss_swla_opt.sys_swla_header + CAM_OS_ALIGN_UP(sizeof(SS_SWLA_LOG_HEAD_t), ARCH_DMA_MINALIGN);
    sys_swla_buf_size_filter = (swla_buf_size_kb << 10) - 1;

#ifdef CONFIG_SS_DUALOS
    g_ss_swla_opt.sys_swla_remote_buf_pa = 0;
#endif

    if (swla_auto_enable == 1) // auto enable with overwrite
        sys_swla_start(SS_SWLA_REC_LOCAL, 1);
    else if (swla_auto_enable == 2) // auto enable without overwrite
        sys_swla_start(SS_SWLA_REC_LOCAL, 0);

    SWLA_DBG("%s\n", __FUNCTION__);
}

static void _sys_swla_start(void *data)
{
#ifdef CONFIG_SS_DUALOS
    if (g_ss_swla_opt.sys_swla_log_start_type != SS_SWLA_REC_LOCAL)
    {
        CamInterOsSignal(INTEROS_SC_L2R_SWLA_CTRL, SS_SWLA_REMOTE_CTRL_START, (u32)g_ss_swla_opt.sys_swla_log_overwrite,
                         0);
    }
#endif
    if (g_ss_swla_opt.sys_swla_log_start_type != SS_SWLA_REC_REMOTE)
    {
        if (NULL == g_ss_swla_opt.sys_swla_buf)
        {
            printk(KERN_EMERG "[%s] no swla buffer!!!\r\n", __FUNCTION__);
            return;
        }

        if (g_ss_swla_opt.sys_swla_log_start == 0)
        {
            ((SS_SWLA_LOG_HEAD_t *)g_ss_swla_opt.sys_swla_header)->start_tick = arch_timer_read_counter();
            CamOsAtomicSet(&g_ss_swla_opt.sys_swla_buf_idx_tick, (-1 * sizeof(SS_SWLA_LOG_t)));
            memset(g_ss_swla_opt.sys_swla_buf, 0, swla_buf_size_kb << 10);
            g_ss_swla_opt.sys_swla_log_start = 1;
        }
    }

    SWLA_DBG("%s\n", __FUNCTION__);
}

void sys_swla_start(SS_SWLA_REC_TYPE_e type, u32 enable_overwrite)
{
    g_ss_swla_opt.sys_swla_log_start_type = type;
    g_ss_swla_opt.sys_swla_log_overwrite  = enable_overwrite;

#ifdef CONFIG_SS_DUALOS
    if (type != SS_SWLA_REC_LOCAL && CamOsInInterrupt() == CAM_OS_OK) // in ISR
    {
        CamOsWorkQueueAdd(g_ss_swla_opt.work_queue, _sys_swla_start, NULL, 0);
    }
    else
#endif
    {
        _sys_swla_start(NULL);
    }

    SWLA_DBG("%s\n", __FUNCTION__);
}

static void _sys_swla_stop(void *data)
{
#ifdef CONFIG_SS_DUALOS
    if (g_ss_swla_opt.sys_swla_log_stop_type != SS_SWLA_REC_LOCAL)
    {
        CamInterOsSignal(INTEROS_SC_L2R_SWLA_CTRL, SS_SWLA_REMOTE_CTRL_STOP, 0, 0);
    }
#endif

    if (g_ss_swla_opt.sys_swla_log_stop_type != SS_SWLA_REC_REMOTE)
    {
        if (g_ss_swla_opt.sys_swla_log_start != 0)
        {
            sys_swla_log_add_event(SWLA_LOG_STOP_LABEL, SS_SWLA_LOG_LABEL);
            g_ss_swla_opt.sys_swla_log_start = 0;
        }

        CamOsMemFlush((void *)g_ss_swla_opt.sys_swla_header,
                      (swla_buf_size_kb << 10) + CAM_OS_ALIGN_UP(sizeof(SS_SWLA_LOG_HEAD_t), ARCH_DMA_MINALIGN));
    }
    SWLA_DBG("%s\n", __FUNCTION__);
}

void sys_swla_stop(SS_SWLA_REC_TYPE_e type)
{
    g_ss_swla_opt.sys_swla_log_stop_type = type;

#ifdef CONFIG_SS_DUALOS
    if (type != SS_SWLA_REC_LOCAL && CamOsInInterrupt() == CAM_OS_OK) // in ISR
    {
        CamOsWorkQueueAdd(g_ss_swla_opt.work_queue, _sys_swla_stop, NULL, 0);
    }
    else
#endif
    {
        _sys_swla_stop(NULL);
    }

    SWLA_DBG("%s\n", __FUNCTION__);
}

static void _sys_swla_log_add(const char *log_name, const SS_SWLA_LOG_TYPE_e log_type)
{
    s32            nBufOffset, nLogNameLen;
    SS_SWLA_LOG_t *pLogEntry;
    u64            nTicks = arch_timer_read_counter();

    nBufOffset = CamOsAtomicAddReturn(&g_ss_swla_opt.sys_swla_buf_idx_tick, sizeof(SS_SWLA_LOG_t));
    if ((nBufOffset + sizeof(SS_SWLA_LOG_t)) > (s32)sys_swla_buf_size_filter)
    {
        if (!g_ss_swla_opt.sys_swla_log_overwrite)
        {
            g_ss_swla_opt.sys_swla_log_start = 0;
            pLogEntry                        = (SS_SWLA_LOG_t *)(g_ss_swla_opt.sys_swla_buf + nBufOffset);
            memset(pLogEntry->log_name, 0, SWLA_LOG_NAME_LENGTH);
            memcpy(pLogEntry->log_name, SWLA_LOG_STOP_LABEL, strlen(SWLA_LOG_STOP_LABEL));
            pLogEntry->log_type       = SS_SWLA_LOG_LABEL;
            pLogEntry->timestamp_tick = nTicks & 0x03FFFFFFFFFFFFFF;
            pLogEntry->cpu_id         = smp_processor_id();
            return;
        }

        nBufOffset &= sys_swla_buf_size_filter;
    }
    pLogEntry   = (SS_SWLA_LOG_t *)(g_ss_swla_opt.sys_swla_buf + nBufOffset);
    nLogNameLen = strlen(log_name);
    if (nLogNameLen > SWLA_LOG_NAME_LENGTH)
        nLogNameLen = SWLA_LOG_NAME_LENGTH;
    memset(pLogEntry->log_name, 0, SWLA_LOG_NAME_LENGTH);
    memcpy(pLogEntry->log_name, log_name, nLogNameLen);
    pLogEntry->log_type       = log_type & 0x3;
    pLogEntry->timestamp_tick = nTicks & 0x03FFFFFFFFFFFFFF;
    pLogEntry->cpu_id         = smp_processor_id();
#if 0
    SWLA_DBG("[%p] %8.8s %6d %16llu\n", pLogEntry, (char *)pLogEntry->log_name, pLogEntry->log_type,
             pLogEntry->timestamp_tick);
#endif
}

void _sys_swla_log_switch_in(void *tcb, const char *event_name)
{
    sys_swla_log_add_event(event_name, SS_SWLA_LOG_SWITCH_IN);
}

void sys_swla_log_add_event(const char *event_name, const SS_SWLA_LOG_TYPE_e log_type)
{
    if (!g_ss_swla_opt.sys_swla_log_start)
        return;

#if !defined(CONFIG_SSTAR_SWLA_SYS_LOG_ENABLE)
    if (log_type == SS_SWLA_LOG_SWITCH_IN)
        return;
#endif

#if !defined(CONFIG_SSTAR_SWLA_USR_LOG_ENABLE)
    if (log_type != SS_SWLA_LOG_SWITCH_IN)
        return;
#endif
    _sys_swla_log_add(event_name, log_type);
}

void sys_swla_log_add_irq(const unsigned int irq_num, const SS_SWLA_LOG_TYPE_e log_type)
{
#if defined(CONFIG_SSTAR_SWLA_SYS_LOG_ENABLE)
    char log_name[SWLA_LOG_NAME_LENGTH] = "ISR-";
    char num2ch;

    if (!g_ss_swla_opt.sys_swla_log_start)
        return;

    num2ch      = (irq_num >> 12) & 0xF;
    log_name[4] = (num2ch > 9) ? (num2ch + 'W') : (num2ch + '0');
    num2ch      = (irq_num >> 8) & 0xF;
    log_name[5] = (num2ch > 9) ? (num2ch + 'W') : (num2ch + '0');
    num2ch      = (irq_num >> 4) & 0xF;
    log_name[6] = (num2ch > 9) ? (num2ch + 'W') : (num2ch + '0');
    num2ch      = irq_num & 0xF;
    log_name[7] = (num2ch > 9) ? (num2ch + 'W') : (num2ch + '0');

    _sys_swla_log_add(log_name, log_type);
#endif
}

static const char *dump_file_name = NULL;
static CamFsFd     dump_file_fd;
static void        _sys_swla_fs_init(void *handle)
{
    if (dump_file_name && CAM_FS_OK != CamFsOpen(&dump_file_fd, dump_file_name, CAM_FS_O_RDWR | CAM_FS_O_CREAT, 0644))
    {
        printk(KERN_EMERG "[%s] FsOpen %s fail\r\n", __FUNCTION__, dump_file_name);
        return;
    }

    CamFsSeek(dump_file_fd, 0, CAM_FS_SEEK_SET);
}

static void _sys_swla_fs_write(void *handle, const unsigned char *in, u32 len)
{
    if (CamFsWrite(dump_file_fd, (void *)in, len) != len)
    {
        printk(KERN_EMERG "[%s] FsWrite %s failed\r\n", __FUNCTION__, dump_file_name);
    }
}

static void _sys_swla_fs_finish(void *handle)
{
    CamFsClose(dump_file_fd);
}

void sys_swla_dump(SS_SWLA_DUMP_METHOD_e dump_method, const char *file_name)
{
    s32 nBufOffset =
        (CamOsAtomicRead(&g_ss_swla_opt.sys_swla_buf_idx_tick) + sizeof(SS_SWLA_LOG_t)) & sys_swla_buf_size_filter;
    SS_SWLA_LOG_t *     pLogNext = (SS_SWLA_LOG_t *)(g_ss_swla_opt.sys_swla_buf + nBufOffset);
    SS_SWLA_LOG_t *     pLogEntry;
    SwlaOutputOp_t      swladumpOp  = {0};
    Base64UartEnc_t     b64Uart     = {0};
    SS_SWLA_LOG_HEAD_t *psLogHeader = (SS_SWLA_LOG_HEAD_t *)g_ss_swla_opt.sys_swla_header;
#ifdef CONFIG_SS_DUALOS
    u32 remote_log_buf_size;
#endif

    if (g_ss_swla_opt.sys_swla_log_start)
    {
        switch (dump_method)
        {
            case SS_SWLA_DUMP_UART_LOCAL:
            case SS_SWLA_DUMP_FILE_LOCAL:
                sys_swla_stop(SS_SWLA_REC_LOCAL);
                break;
            case SS_SWLA_DUMP_FILE_REMOTE:
                sys_swla_stop(SS_SWLA_REC_REMOTE);
                break;
            case SS_SWLA_DUMP_FILE_DUALOS:
                sys_swla_stop(SS_SWLA_REC_DUALOS);
                break;
            default:
                break;
        }
    }
    else
    {
        if (dump_method == SS_SWLA_DUMP_FILE_REMOTE)
            sys_swla_stop(SS_SWLA_REC_REMOTE);
    }

    if (NULL == g_ss_swla_opt.sys_swla_buf)
    {
        printk(KERN_EMERG "[%s] no swla buffer!!!\r\n", __FUNCTION__);
        return;
    }

    if (dump_method == SS_SWLA_DUMP_UART_LOCAL)
    {
        swladumpOp.handle = (void *)&b64Uart;
        swladumpOp.init   = Base64UartEncInit;
        swladumpOp.write  = Base64UartEncWrite;
        swladumpOp.finish = Base64UartEncFinish;
    }
    else
    {
        if (!file_name)
        {
            printk(KERN_EMERG "%s: file name is null\n", __FUNCTION__);
            return;
        }
        dump_file_name    = file_name;
        swladumpOp.init   = _sys_swla_fs_init;
        swladumpOp.write  = _sys_swla_fs_write;
        swladumpOp.finish = _sys_swla_fs_finish;
    }

    if (!swladumpOp.write)
    {
        printk(KERN_EMERG "%s Error, invalidate write function\n", __FUNCTION__);
        return;
    }

    if (swladumpOp.init)
        swladumpOp.init(swladumpOp.handle);

    if (dump_method == SS_SWLA_DUMP_UART_LOCAL)
        printk(KERN_EMERG "\n----------------------------- SWLA LOG BEGIN ------------------------------\n\n");

    // Dump local log
    if (dump_method != SS_SWLA_DUMP_FILE_REMOTE)
    {
        if (CamOsAtomicRead(&g_ss_swla_opt.sys_swla_buf_idx_tick) >= 0)
        {
            pLogEntry  = (SS_SWLA_LOG_t *)g_ss_swla_opt.sys_swla_buf;
            nBufOffset = 0;

            // write log header
            psLogHeader->tick_freq = arch_timer_get_rate();
            swladumpOp.write(swladumpOp.handle, (unsigned char *)psLogHeader, sizeof(SS_SWLA_LOG_HEAD_t));

            // write log buffer
            if (dump_method == SS_SWLA_DUMP_UART_LOCAL && pLogNext->timestamp_tick == 0)
                swladumpOp.write(swladumpOp.handle, (unsigned char *)pLogEntry,
                                 (pLogNext - pLogEntry) * sizeof(SS_SWLA_LOG_t));
            else
                swladumpOp.write(swladumpOp.handle, (unsigned char *)pLogEntry, swla_buf_size_kb << 10);
        }
    }

#ifdef CONFIG_SS_DUALOS
    // Dump remote log
    if (dump_method != SS_SWLA_DUMP_UART_LOCAL && dump_method != SS_SWLA_DUMP_FILE_LOCAL
        && g_ss_swla_opt.sys_swla_remote_buf_pa != 0)
    {
        CamInterOsSignal(INTEROS_SC_L2R_SWLA_CTRL, SS_SWLA_REMOTE_CTRL_DUMP_NOTIFY, 0, 0);
        psLogHeader =
            (SS_SWLA_LOG_HEAD_t *)CamOsMemMap(g_ss_swla_opt.sys_swla_remote_buf_pa, sizeof(SS_SWLA_LOG_HEAD_t), 1);
        CamOsMemInvalidate((void *)psLogHeader, sizeof(SS_SWLA_LOG_HEAD_t));
        // write remote log header
        swladumpOp.write(swladumpOp.handle, (unsigned char *)psLogHeader, sizeof(SS_SWLA_LOG_HEAD_t));
        CamOsMemUnmap((void *)psLogHeader, sizeof(SS_SWLA_LOG_HEAD_t));

        remote_log_buf_size = (1 << psLogHeader->buf_kb_shift) << 10;
        pLogEntry = (SS_SWLA_LOG_t *)CamOsMemMap(g_ss_swla_opt.sys_swla_remote_buf_pa + sizeof(SS_SWLA_LOG_HEAD_t),
                                                 remote_log_buf_size, 1);
        CamOsMemInvalidate((void *)pLogEntry, remote_log_buf_size);
        // write remote log buffer
        swladumpOp.write(swladumpOp.handle, (unsigned char *)pLogEntry, remote_log_buf_size);
        CamOsMemUnmap((void *)pLogEntry, remote_log_buf_size);
    }
#endif

    if (swladumpOp.finish)
        swladumpOp.finish(swladumpOp.handle);

    if (dump_method == SS_SWLA_DUMP_UART_LOCAL)
        printk(KERN_EMERG "\n----------------------------- SWLA LOG END ------------------------------\n\n");
}

u64 sys_swla_get_info(SS_SWLA_GET_INFO_e option)
{
    u64 ret_info = 0;

    switch (option)
    {
        case SS_SWLA_GET_BUF_PA:
            ret_info = (u64)CamOsMemVirtToPhys((void *)g_ss_swla_opt.sys_swla_header);
            break;
        case SS_SWLA_GET_BUF_SIZE_KB:
            ret_info = (u64)swla_buf_size_kb;
            break;
        case SS_SWLA_GET_START_STATUS:
            ret_info = (u64)g_ss_swla_opt.sys_swla_log_start;
            break;
        case SS_SWLA_GET_START_TYPE:
            ret_info = (u64)g_ss_swla_opt.sys_swla_log_start_type;
            break;
        case SS_SWLA_GET_START_OVERWRITE:
            ret_info = (u64)g_ss_swla_opt.sys_swla_log_overwrite;
            break;
        default:
            break;
    }

    return ret_info;
}
#endif // CONFIG_SSTAR_SWLA
