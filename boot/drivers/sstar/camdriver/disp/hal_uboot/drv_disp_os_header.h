/*
 * drv_disp_os_header.h- Sigmastar
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
#ifndef _DRV_DISP_OS_HEADER_H_
#define _DRV_DISP_OS_HEADER_H_

#include "cam_os_wrapper.h"
#include <common.h>
#include <command.h>
#include <config.h>
#include <malloc.h>
#include <stdlib.h>
#include <linux/delay.h>

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define CamOsPrintf     printf
#define CamOsMemcpy     memcpy
#define CamOsMemAlloc   malloc
#define CamOsMemRelease free
#define CamOsMemset     memset
#define CamOsUsSleep    udelay
#define CamOsUsDelay    udelay
#define CamOsMsSleep    mdelay

#define CamProcSeqPrintf(str, _fmt, _args...)

#define DISP_IO_ADDRESS(x) ((void *)(x + 0UL))

/* read register by byte */
#define disp_readb(a) (*(volatile MI_U8 *)DISP_IO_ADDRESS(a))

/* read register by word */
#define disp_readw(a) (*(volatile MI_U16 *)DISP_IO_ADDRESS(a))

/* read register by long */
#define disp_readl(a) (*(volatile MI_U32 *)DISP_IO_ADDRESS(a))

/* write register by byte */
#define disp_writeb(v, a) (*(volatile MI_U8 *)DISP_IO_ADDRESS(a) = (v))

/* write register by word */
#define disp_writew(v, a) (*(volatile MI_U16 *)DISP_IO_ADDRESS(a) = (v))

/* write register by long */
#define disp_writel(v, a) (*(volatile MI_U32 *)DISP_IO_ADDRESS(a) = (v))

#define READ_BYTE(x)     disp_readb(x)
#define READ_WORD(x)     disp_readw(x)
#define READ_LONG(x)     disp_readl(x)
#define WRITE_BYTE(x, y) disp_writeb((MI_U8)(y), x)
#define WRITE_WORD(x, y) disp_writew((MI_U16)(y), x)
#define WRITE_LONG(x, y) disp_writel((MI_U32)(y), x)

#define DISP_TIMEZONE_ISR_SUPPORT HAL_DISP_TIMEZONE_ISR_SUPPORT_UBOOT
#define DISP_VGA_HPD_ISR_SUPPORT  HAL_DISP_VGA_HPD_ISR_SUPPORT_UBOOT
#define DISP_REG_ACCESS_MODE      HAL_DISP_REG_ACCESS_MD_UBOOT

#define DISP_OS_STRING_MAX 128

#define DISP_IRQNUM_Default 0

#define DISP_OS_VIR_RIUBASE                          0x1F000000
#define DISPDEBUG_SPRINTF(str, size, _fmt, _args...) snprintf((char *)str, size, _fmt, ##_args)
#define CamOsChipRevision()                          (0)

//-------------------------------------------------------------------------------------------------
//  Structure & Emu
//-------------------------------------------------------------------------------------------------

// Task
typedef void *(*DISP_TASK_ENTRY_CB)(void *argv);

typedef struct
{
    MI_S32 s32Id;
} DRV_DISP_OS_TaskConfig_t;

typedef struct
{
    void *pFile;
} DRV_DISP_OS_FileConfig_t;

typedef struct
{
    MI_S32 argc;
    MI_U8 *argv[DISP_OS_STRING_MAX];
} DRV_DISP_OS_StrConfig_t;

typedef struct
{
    MI_U8 pu8Name[64];
    MI_U8 pu8Value[64];
} DRV_DISP_OS_TextItemConfig_t;

typedef struct
{
    DRV_DISP_OS_TextItemConfig_t *pstItem;
    MI_U32                        u32Size;
} DRV_DISP_OS_TextConfig_t;

typedef struct
{
    MI_S32 s32Id;
} DRV_DISP_OS_MutexConfig_t;

#endif
