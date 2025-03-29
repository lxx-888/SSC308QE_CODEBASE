/*
 * hal_fsp_qspi.h- Sigmastar
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

#ifndef _HAL_FSP_QSPI_H_
#define _HAL_FSP_QSPI_H_

typedef signed char        s8;
typedef unsigned char      u8;
typedef signed short       s16;
typedef unsigned short     u16;
typedef signed int         s32;
typedef unsigned int       u32;
typedef signed long long   s64;
typedef unsigned long long u64;

struct fsp_qspi_ctrl_hal
{
    u8 rate_str;
    u8 rate_dtr;
    u8 need_autok;
    u8 need_phase;
    u8 have_phase;
    u8 phase;

    u8 setup_rate;
    u8 setup_cmd;
};

struct fsp_qspi_hal
{
    /* register bank */
    unsigned long fsp_base;
    unsigned long qspi_base;

    /* software flag */
    u8  use_sw_cs;
    u8  interrupt_en;
    u8  cs_select;
    u8  rate;
    u8  cs_num;
    u8  cs_ext_num;
    u32 cs_ext[16];

    /* state machine */
    u8  wd_index;
    u8  rd_index;
    u16 wd_reg;
    u16 rd_reg;
    u16 wbf_cmd_size;
    u16 rbf_reply_size;
    u16 cs_timeout_en;
    u32 cs_timeout_value;

    struct fsp_qspi_ctrl_hal *ctrl;
    s32 (*wait_done)(struct fsp_qspi_hal *);
    s32 (*wait_bdma_done)(struct fsp_qspi_hal *);
    void (*bdma_callback)(void *);
    void (*set_rate)(struct fsp_qspi_hal *, u8, u8);
};

struct fsp_qspi_command
{
    u8  cmd;
    u32 address;
    u8  addr_bytes;
    u8  dummy;
};

u8   hal_fsp_qspi_is_boot_storage(struct fsp_qspi_hal *hal);
void hal_fsp_qspi_init(struct fsp_qspi_hal *hal);
void hal_fsp_qspi_set_rate(struct fsp_qspi_hal *hal, u8 cs_select, u8 rate, u8 cmd);
void hal_fsp_qspi_set_phase(struct fsp_qspi_hal *hal, u8 phase);
u8   hal_fsp_qspi_try_phase(struct fsp_qspi_hal *hal, u8 (*parrtern_check)(void));
void hal_fsp_qspi_set_timeout(struct fsp_qspi_hal *hal, u8 enable, u32 val);
void hal_fsp_qspi_use_sw_cs(struct fsp_qspi_hal *hal, u8 enabled);
void hal_fsp_qspi_cz2_enable(void);
void hal_fsp_qspi_pull_cs(struct fsp_qspi_hal *hal, u8 pull_high);
void hal_fsp_qspi_chip_select(struct fsp_qspi_hal *hal, u8 cs_select, u8 cmd);
u32  hal_fsp_qspi_write(struct fsp_qspi_hal *hal, struct fsp_qspi_command *cmd, u8 *buf, u32 size);
u32  hal_fsp_qspi_read(struct fsp_qspi_hal *hal, struct fsp_qspi_command *cmd, u8 *buf, u32 size);
u32  hal_fsp_qspi_bdma_write(struct fsp_qspi_hal *hal, struct fsp_qspi_command *cmd, u8 *buf, u32 size);
u32  hal_fsp_qspi_bdma_read(struct fsp_qspi_hal *hal, struct fsp_qspi_command *cmd, u8 *buf, u32 size);
u32  hal_fsp_qspi_bdma_read_to_xzdec(struct fsp_qspi_hal *hal, struct fsp_qspi_command *cmd, u8 *buf, u32 size);

#endif /* _HAL_FSP_QSPI_H_ */
