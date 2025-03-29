/*
 * sstar_swla_procfs.c - Sigmastar
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
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/module.h>

/* procfs */
#include "cam_proc_wrapper.h"
#include "sstar_swla.h"

struct CamProcEntry_t *gpSWLARootDir = NULL;

static void swla_buf_size_kb_read(CamProcSeqFile_t *pSeq, void *pPrivData)
{
    CamProcSeqPrintf(pSeq, "%llu KB\n", sys_swla_get_info(SS_SWLA_GET_BUF_SIZE_KB));
    CamProcSeqPrintf(pSeq, "Limitation: SWLA buffer size must fit 2^n KB\n");
    return;
}

static void swla_start_read(CamProcSeqFile_t *pSeq, void *pPrivData)
{
    CamProcSeqPrintf(pSeq, "record enable: %llu\n", sys_swla_get_info(SS_SWLA_GET_START_STATUS));
    CamProcSeqPrintf(pSeq, "type         : %llu\n", sys_swla_get_info(SS_SWLA_GET_START_TYPE));
    CamProcSeqPrintf(pSeq, "overwrite    : %llu\n", sys_swla_get_info(SS_SWLA_GET_START_OVERWRITE));
    CamProcSeqPrintf(pSeq, "Usage: echo [type] [overwrite] > /proc/swla/start\n");
    CamProcSeqPrintf(pSeq, "       [type] 0: local, 1: remote, 2: dualos\n");
    CamProcSeqPrintf(pSeq, "       [overwrite] 0: disable, 1: enable\n");
    return;
}

static void swla_start_write(char *pBuf, int nLen, void *pPrivData)
{
    unsigned long type, overwrite;
    char *        token = pBuf;

    type      = CamOsStrtoul(token, &token, 10);
    overwrite = CamOsStrtoul(token + 1, NULL, 10);
    if (type < SS_SWLA_REC_TYPE_MAX && overwrite < 2)
        sys_swla_start(type, overwrite);

    return;
}

static void swla_stop_read(CamProcSeqFile_t *pSeq, void *pPrivData)
{
    CamProcSeqPrintf(pSeq, "record enable: %llu\n", sys_swla_get_info(SS_SWLA_GET_START_STATUS));
    CamProcSeqPrintf(pSeq, "Usage: echo [type] > /proc/swla/stop\n");
    CamProcSeqPrintf(pSeq, "       [type] 0: local, 1: remote, 2: dualos\n");
    return;
}

static void swla_stop_write(char *pBuf, int nLen, void *pPrivData)
{
    unsigned long type = CamOsStrtoul(pBuf, NULL, 10);

    if (type < SS_SWLA_REC_TYPE_MAX)
        sys_swla_stop(type);

    return;
}

static void swla_event_read(CamProcSeqFile_t *pSeq, void *pPrivData)
{
    CamProcSeqPrintf(pSeq, "Usage: echo [type] [event_name] > /proc/swla/event\n");
    CamProcSeqPrintf(pSeq, "       [type] 0: start, 1: stop, 2: label\n");
    return;
}

static void swla_event_write(char *pBuf, int nLen, void *pPrivData)
{
    char *        token = pBuf;
    unsigned long type  = CamOsStrtoul(token, &token, 10);

    if (type < SS_SWLA_LOG_SWITCH_IN)
        sys_swla_log_add_event(token + 1, type);

    return;
}

static void swla_dump_read(CamProcSeqFile_t *pSeq, void *pPrivData)
{
    CamProcSeqPrintf(pSeq, "Usage: echo [type] [file_path] > /proc/swla/dump\n");
    CamProcSeqPrintf(pSeq, "       [type] 0: file local, 1: file remote, 2: file dualos, 3: uart local\n");
    return;
}

static void swla_dump_write(char *pBuf, int nLen, void *pPrivData)
{
    char *        token = pBuf;
    unsigned long type  = CamOsStrtoul(token, &token, 10);

    if (type == SS_SWLA_DUMP_UART_LOCAL)
        sys_swla_dump(type, NULL);
    else
        sys_swla_dump(type, token + 1);

    return;
}

static int __init swla_proc_init(void)
{
    struct CamProcEntry_t *pTmpEntry;

    gpSWLARootDir = CamProcMkdir("swla", NULL);

    if (!gpSWLARootDir)
    {
        CamOsPrintf("%s L:%d, Mkdir fail in CamProcMkdir\n", __FUNCTION__, __LINE__);
    }

    pTmpEntry = CamProcCreate("buf_size_kb", gpSWLARootDir, swla_buf_size_kb_read, NULL, NULL);

    if (!pTmpEntry)
    {
        CamOsPrintf("%s L:%d, Create entry fail in CamProcCreate\n", __FUNCTION__, __LINE__);
    }

    pTmpEntry = CamProcCreate("start", gpSWLARootDir, swla_start_read, swla_start_write, NULL);

    if (!pTmpEntry)
    {
        CamOsPrintf("%s L:%d, Create entry fail in CamProcCreate\n", __FUNCTION__, __LINE__);
    }

    pTmpEntry = CamProcCreate("stop", gpSWLARootDir, swla_stop_read, swla_stop_write, NULL);

    if (!pTmpEntry)
    {
        CamOsPrintf("%s L:%d, Create entry fail in CamProcCreate\n", __FUNCTION__, __LINE__);
    }

    pTmpEntry = CamProcCreate("event", gpSWLARootDir, swla_event_read, swla_event_write, NULL);

    if (!pTmpEntry)
    {
        CamOsPrintf("%s L:%d, Create entry fail in CamProcCreate\n", __FUNCTION__, __LINE__);
    }

    pTmpEntry = CamProcCreate("dump", gpSWLARootDir, swla_dump_read, swla_dump_write, NULL);

    if (!pTmpEntry)
    {
        CamOsPrintf("%s L:%d, Create entry fail in CamProcCreate\n", __FUNCTION__, __LINE__);
    }

    return 0;
}

static void __exit swla_proc_exit(void)
{
    CamProcRemoveEntry("swla", NULL);
}

module_init(swla_proc_init);
module_exit(swla_proc_exit);
