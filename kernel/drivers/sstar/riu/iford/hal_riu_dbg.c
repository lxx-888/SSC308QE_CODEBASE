/*
 * hal_riu_dbg.c- Sigmastar
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

#include "hal_riu_dbg.h"

/*
 * riu operation macro
 */
#ifdef CONFIG_ARM64
#define HAL_RIU_READ_WORD(reg)       (*(volatile u16 *)(u64)(reg))
#define HAL_RIU_WRITE_WORD(reg, val) ((*(volatile u16 *)(u64)(reg)) = (u16)(val))
#define HAL_RIU_WRITE_WORD_MASK(reg, val, mask) \
    ((*(volatile u16 *)(u64)(reg)) = ((*(volatile u16 *)(u64)(reg)) & ~(mask)) | ((u16)(val) & (mask)))
#else
#define HAL_RIU_READ_WORD(reg)       (*(volatile u16 *)(u32)(reg))
#define HAL_RIU_WRITE_WORD(reg, val) ((*(volatile u16 *)(u32)(reg)) = (u16)(val))
#define HAL_RIU_WRITE_WORD_MASK(reg, val, mask) \
    ((*(volatile u16 *)(u32)(reg)) = ((*(volatile u16 *)(u32)(reg)) & ~(mask)) | ((u16)(val) & (mask)))
#endif

#define HAL_RIU_READ(base, reg)                  HAL_RIU_READ_WORD(base + ((reg) << 2))
#define HAL_RIU_WRITE(base, reg, val)            HAL_RIU_WRITE_WORD(base + ((reg) << 2), (val))
#define HAL_RIU_WRITE_MASK(base, reg, val, mask) HAL_RIU_WRITE_WORD_MASK(base + ((reg) << 2), (val), (mask))

/*
 * register macro
 */

#define HAL_RIU_REG_CTRL        0x00
#define HAL_RIU_REG_CTRL_EN_BIT 0x0001

#define HAL_RIU_REG_SNAPSHOT_L 0x09
#define HAL_RIU_REG_SNAPSHOT_H 0x0a

#define HAL_RIU_REG_EN0 0x14
#define HAL_RIU_REG_EN1 0x15
#define HAL_RIU_REG_EN2 0x19
#define HAL_RIU_REG_EN3 0x1b
#define HAL_RIU_REG_EN4 0x1d

#define HAL_RIU_REG_SGWGRP_CTRL 0x02

#define HAL_RIU_REG_SGWDMA_CTRL      0x00
#define HAL_RIU_REG_SGWDMA_FIFO      0x01
#define HAL_RIU_REG_SGWDMA_WWIDTH    0x02
#define HAL_RIU_REG_SGWDMA_WHIGHT    0x03
#define HAL_RIU_REG_SGWDMA_WPITCH    0x04
#define HAL_RIU_REG_SGWDMA_WADDR0_L  0x06
#define HAL_RIU_REG_SGWDMA_WADDR0_H  0x07
#define HAL_RIU_REG_SGWDMA_WSIZE     0x0A
#define HAL_RIU_REG_SGWDMA_WREQTH    0x0A
#define HAL_RIU_REG_SGWDMA_WSIZE_MAX 0x0B
#define HAL_RIU_REG_SGWDMA_WINFO_SEL 0x0C
#define HAL_RIU_REG_SGWDMA_WINFO     0x0F
#define HAL_RIU_REG_SGWDMA_CLEAR     0x10
#define HAL_RIU_REG_SGWDMA_STATUS    0x10
#define HAL_RIU_REG_SGWDMA_UNMASK    0x11
#define HAL_RIU_REG_SGWDMA_LINE      0x13
#define HAL_RIU_REG_SGWDMA_MODE      0x14
#define HAL_RIU_REG_SGWDMA_LINE_MAX  0x15

#define HAL_RIU_REG_RECORD_CTRL0 0x00
#define HAL_RIU_REG_RECORD_CTRL1 0x01
#define HAL_RIU_REG_FILT_START   0x02
#define HAL_RIU_REG_CAP_INT      0x0e
#define HAL_RIU_REG_CAP_BANK     0x0f
#define HAL_RIU_REG_CAP_OFFSET   0x13
#define HAL_RIU_REG_CAP_DATA     0x15
#define HAL_RIU_REG_CAP_MASK     0x1e

/*
 * static symbols
 */
#ifdef CONFIG_SSTAR_RIU_RECORDER
struct riu_bridge_info_t
{
    u8  bridge;
    u16 from;
    u16 to;
};

static const struct riu_bridge_info_t riu_bridge_info[] = {
    // bridge 0 dig-bridge
    {.bridge = 0, .from = 0x1000, .to = 0x11ff},
    {.bridge = 0, .from = 0x1400, .to = 0x166f},
    {.bridge = 0, .from = 0x1820, .to = 0x1823},

    // bridge 1 x32-bridge
    {.bridge = 1, .from = 0x1670, .to = 0x1671},
    {.bridge = 1, .from = 0x1a64, .to = 0x1a72},
    {.bridge = 1, .from = 0x1b28, .to = 0x1b30},

    // bridge 2 cmd-bridge
    {.bridge = 2, .from = 0x1200, .to = 0x1282},
    {.bridge = 2, .from = 0x1683, .to = 0x168c},

    // bridge 3 isp-bridge
    {.bridge = 3, .from = 0x1300, .to = 0x13ff},
    {.bridge = 3, .from = 0x1678, .to = 0x1679},
};
#endif

/*
 * externel symbols
 */

#ifdef CONFIG_SSTAR_RIU_TIMEOUT
void hal_riu_timeout_enable(struct hal_riu_dbg_t *hal)
{
    u32 i;
    u64 base;

    for (i = 0; i < hal->timeout.num_bank; i++)
    {
        base = hal->timeout.banks[i];

        /* enable all bit include unused (no effect) */
        HAL_RIU_WRITE(base, HAL_RIU_REG_EN0, 0xFFFF);
        HAL_RIU_WRITE(base, HAL_RIU_REG_EN1, 0xFFFF);
        HAL_RIU_WRITE(base, HAL_RIU_REG_EN2, 0xFFFF);
        HAL_RIU_WRITE(base, HAL_RIU_REG_EN3, 0xFFFF);
        HAL_RIU_WRITE(base, HAL_RIU_REG_EN4, 0xFFFF);

        /* enable timeout counter */
        HAL_RIU_WRITE_MASK(base, HAL_RIU_REG_CTRL, HAL_RIU_REG_CTRL_EN_BIT, HAL_RIU_REG_CTRL_EN_BIT);
    }
}

void hal_riu_timeout_get_snapshot(u64 base, struct riu_timeout_snapshot_t *snapshort)
{
    snapshort->bank = (HAL_RIU_READ(base, HAL_RIU_REG_SNAPSHOT_H) << 8)
                      + ((HAL_RIU_READ(base, HAL_RIU_REG_SNAPSHOT_L) & 0xff00) >> 8);
    snapshort->offset = ((HAL_RIU_READ(base, HAL_RIU_REG_SNAPSHOT_L) & 0x00ff) >> 1);
}
#endif

#ifdef CONFIG_SSTAR_RIU_RECORDER
s8 hal_riu_bank_to_bridge(u16 bank)
{
    u8 i;

    for (i = 0; i < (sizeof(riu_bridge_info) / sizeof(riu_bridge_info[0])); i++)
    {
        if ((bank >= riu_bridge_info[i].from) && (bank <= riu_bridge_info[i].to))
            return riu_bridge_info[i].bridge;
    }
    return -1;
}

void hal_riu_record_init(struct hal_riu_dbg_t *hal)
{
    u8  i;
    u8  bridge;
    u32 lines;
    u64 record0 = hal->record.banks[0]; // 0x114B
    u64 record1 = hal->record.banks[1]; // 0x114C
    u64 sgwgrp  = hal->record.banks[2]; // 0x114D
    u64 sgwdma  = hal->record.banks[3]; // 0x114E

    HAL_RIU_WRITE_MASK(sgwgrp, HAL_RIU_REG_SGWGRP_CTRL, 0x0000, 0x00ff);

    HAL_RIU_WRITE(sgwdma, HAL_RIU_REG_SGWDMA_CTRL, 0x0000);
    HAL_RIU_WRITE(sgwdma, HAL_RIU_REG_SGWDMA_CTRL, 0x0408);
    HAL_RIU_WRITE(sgwdma, HAL_RIU_REG_SGWDMA_CTRL, 0x0400);
    HAL_RIU_WRITE(sgwdma, HAL_RIU_REG_SGWDMA_WADDR0_L, (hal->record.addr_base >> 4 & 0xFFFF));
    HAL_RIU_WRITE(sgwdma, HAL_RIU_REG_SGWDMA_WADDR0_H, ((hal->record.addr_base >> 20) & 0xFFFF));
    HAL_RIU_WRITE_MASK(sgwdma, HAL_RIU_REG_SGWDMA_MODE, 0x00a0, 0x00ff);

    lines = hal->record.addr_size / 0x2000 - 1;

    HAL_RIU_WRITE(sgwdma, HAL_RIU_REG_SGWDMA_LINE, lines);
    HAL_RIU_WRITE(sgwdma, HAL_RIU_REG_SGWDMA_FIFO, 0x0404);
    HAL_RIU_WRITE(sgwdma, HAL_RIU_REG_SGWDMA_UNMASK, 0x00f7);
    HAL_RIU_WRITE(sgwdma, HAL_RIU_REG_SGWDMA_WWIDTH, 0x0200);
    HAL_RIU_WRITE(sgwdma, HAL_RIU_REG_SGWDMA_WHIGHT, 0xffff);
    HAL_RIU_WRITE(sgwdma, HAL_RIU_REG_SGWDMA_WPITCH, 0x0200);
    HAL_RIU_WRITE(sgwdma, HAL_RIU_REG_SGWDMA_CTRL, 0x0410);
    HAL_RIU_WRITE_MASK(sgwdma, HAL_RIU_REG_SGWDMA_WSIZE, 0x0000, 0xff00);
    HAL_RIU_WRITE_MASK(sgwdma, HAL_RIU_REG_SGWDMA_WSIZE_MAX, 0x0000, 0xff00);
    HAL_RIU_WRITE_MASK(sgwdma, HAL_RIU_REG_SGWDMA_WREQTH, 0x0001, 0x00ff);
    HAL_RIU_WRITE(sgwdma, HAL_RIU_REG_SGWDMA_CTRL, 0x0511);

    /*
     * filter setting
     */
    for (bridge = 0; bridge < HAL_RIU_BRIDGE_NUM; bridge++)
    {
        for (i = 0; i < HAL_RIU_FILTER_NUM; i++)
        {
            if (i < hal->record.filt_num[bridge])
            {
                if (bridge < 4)
                {
                    HAL_RIU_WRITE(record0, ((0x20 * bridge) + (0x2 * i) + HAL_RIU_REG_FILT_START),
                                  hal->record.filt_start[bridge][i]);

                    HAL_RIU_WRITE(record0, ((0x20 * bridge) + (0x2 * i) + HAL_RIU_REG_FILT_START + 0x1),
                                  hal->record.filt_end[bridge][i]);
                }
                else
                {
                    HAL_RIU_WRITE(record1, ((0x20 * (bridge - 4)) + (0x2 * i) + HAL_RIU_REG_FILT_START),
                                  hal->record.filt_start[bridge - 4][i]);

                    HAL_RIU_WRITE(record1, ((0x20 * (bridge - 4)) + (0x2 * i) + HAL_RIU_REG_FILT_START + 0x1),
                                  hal->record.filt_end[bridge - 4][i]);
                }
            }
            else
            {
                if (bridge < 4)
                {
                    HAL_RIU_WRITE(record0, ((0x20 * bridge) + (0x2 * i) + HAL_RIU_REG_FILT_START), 0x0000);

                    HAL_RIU_WRITE(record0, ((0x20 * bridge) + (0x2 * i) + HAL_RIU_REG_FILT_START + 0x1), 0x0000);
                }
                else
                {
                    HAL_RIU_WRITE(record1, ((0x20 * (bridge - 4)) + (0x2 * i) + HAL_RIU_REG_FILT_START), 0x0000);

                    HAL_RIU_WRITE(record1, ((0x20 * (bridge - 4)) + (0x2 * i) + HAL_RIU_REG_FILT_START + 0x1), 0x0000);
                }
            }
        }

        if (bridge < 4)
        {
            HAL_RIU_WRITE_MASK(record0, ((0x20 * bridge) + HAL_RIU_REG_RECORD_CTRL1), 0x0100, 0x0100);
        }
        else
        {
            HAL_RIU_WRITE_MASK(record1, ((0x20 * (bridge - 4)) + HAL_RIU_REG_RECORD_CTRL1), 0x0100, 0x0100);
        }
    }

    /*
     * capture setting
     */
    for (bridge = 0; bridge < HAL_RIU_BRIDGE_NUM; bridge++)
    {
        for (i = 0; i < HAL_RIU_CAPTUER_NUM; i++)
        {
            if (i < hal->record.cap_num[bridge])
            {
                if (bridge < 4)
                {
                    HAL_RIU_WRITE(record0, ((0x20 * bridge) + HAL_RIU_REG_CAP_BANK + i),
                                  hal->record.cap_bank[bridge][i]);

                    HAL_RIU_WRITE_MASK(record0, ((0x20 * bridge) + HAL_RIU_REG_CAP_OFFSET + (i / 2)),
                                       (hal->record.cap_offset[bridge][i] << ((i % 2) * 8)), (0xff << ((i % 2) * 8)));

                    HAL_RIU_WRITE(record0, ((0x20 * bridge) + HAL_RIU_REG_CAP_DATA + (i * 2)),
                                  hal->record.cap_data[bridge][i] & 0xffff);

                    HAL_RIU_WRITE(record0, ((0x20 * bridge) + HAL_RIU_REG_CAP_DATA + (i * 2) + 1),
                                  (hal->record.cap_data[bridge][i] >> 16) & 0xffff);

                    HAL_RIU_WRITE_MASK(record0, ((0x20 * bridge) + HAL_RIU_REG_CAP_MASK + (i / 4)),
                                       (hal->record.cap_mask[bridge][i] << ((i % 4) * 4)), (0xf << ((i % 4) * 4)));
                }
                else
                {
                    HAL_RIU_WRITE(record1, ((0x20 * (bridge - 4)) + HAL_RIU_REG_CAP_BANK + i),
                                  hal->record.cap_bank[bridge][i]);

                    HAL_RIU_WRITE_MASK(record1, ((0x20 * (bridge - 4)) + HAL_RIU_REG_CAP_OFFSET + (i / 2)),
                                       (hal->record.cap_offset[bridge][i] << ((i % 2) * 8)), (0xff << ((i % 2) * 8)));

                    HAL_RIU_WRITE(record1, ((0x20 * (bridge - 4)) + HAL_RIU_REG_CAP_DATA + (i * 2)),
                                  hal->record.cap_data[bridge][i] & 0xffff);

                    HAL_RIU_WRITE(record1, ((0x20 * (bridge - 4)) + HAL_RIU_REG_CAP_DATA + (i * 2) + 1),
                                  (hal->record.cap_data[bridge][i] >> 16) & 0xffff);

                    HAL_RIU_WRITE_MASK(record1, ((0x20 * (bridge - 4)) + HAL_RIU_REG_CAP_MASK + (i / 4)),
                                       (hal->record.cap_mask[bridge][i] << ((i % 4) * 4)), (0xf << ((i % 4) * 4)));
                }
            }
            else
            {
                if (bridge < 4)
                {
                    HAL_RIU_WRITE(record0, ((0x20 * bridge) + HAL_RIU_REG_CAP_BANK + i), 0xffff);
                }
                else
                {
                    HAL_RIU_WRITE(record1, ((0x20 * (bridge - 4)) + HAL_RIU_REG_CAP_BANK + i), 0xffff);
                }
            }
        }

        if (hal->record.cap_num[bridge])
        {
            if (bridge < 4)
            {
                /* clear flag */
                HAL_RIU_WRITE_MASK(record0, ((0x20 * bridge) + HAL_RIU_REG_CAP_INT), 0x0040, 0x00C0);
                HAL_RIU_WRITE_MASK(record0, ((0x20 * bridge) + HAL_RIU_REG_CAP_INT), 0x0000, 0x00C0);
                /* enable interrupt */
                HAL_RIU_WRITE_MASK(record0, ((0x20 * bridge) + HAL_RIU_REG_CAP_INT), 0x0001, 0x0001);
            }
            else
            {
                HAL_RIU_WRITE_MASK(record1, ((0x20 * (bridge - 4)) + HAL_RIU_REG_CAP_INT), 0x0040, 0x00C0);
                HAL_RIU_WRITE_MASK(record1, ((0x20 * (bridge - 4)) + HAL_RIU_REG_CAP_INT), 0x0000, 0x00C0);
                HAL_RIU_WRITE_MASK(record1, ((0x20 * (bridge - 4)) + HAL_RIU_REG_CAP_INT), 0x0001, 0x0001);
            }
        }
    }
}

void hal_riu_record_enable(struct hal_riu_dbg_t *hal, u8 enable)
{
    u8  bridge;
    u64 record0 = hal->record.banks[0]; // 0x114B
    u64 record1 = hal->record.banks[1]; // 0x114C

    for (bridge = 0; bridge < HAL_RIU_BRIDGE_NUM; bridge++)
    {
        if (enable)
        {
            if (bridge < 4)
            {
                HAL_RIU_WRITE(record0, ((0x20 * bridge) + HAL_RIU_REG_RECORD_CTRL0), 0x03);
                HAL_RIU_WRITE(record0, ((0x20 * bridge) + HAL_RIU_REG_RECORD_CTRL0), 0x23);
            }
            else
            {
                HAL_RIU_WRITE(record1, ((0x20 * (bridge - 4)) + HAL_RIU_REG_RECORD_CTRL0), 0x03);
                HAL_RIU_WRITE(record1, ((0x20 * (bridge - 4)) + HAL_RIU_REG_RECORD_CTRL0), 0x23);
            }
        }
        else
        {
            if (bridge < 4)
            {
                HAL_RIU_WRITE(record0, ((0x20 * bridge) + HAL_RIU_REG_RECORD_CTRL0), 0x00);
            }
            else
            {
                HAL_RIU_WRITE(record1, ((0x20 * (bridge - 4)) + HAL_RIU_REG_RECORD_CTRL0), 0x00);
            }
        }
    }
}

static struct riu_record_snapshot_t hal_riu_record_snapshot;

void hal_riu_record_interrupt(struct hal_riu_dbg_t *hal)
{
    u16 line;
    u8  bridge;
    u16 status;

    u64 record0 = hal->record.banks[0]; // 0x114B
    u64 record1 = hal->record.banks[1]; // 0x114C
    u64 sgwdma  = hal->record.banks[3]; // 0x114E

    if (HAL_RIU_READ(sgwdma, HAL_RIU_REG_SGWDMA_STATUS) & 0x0800)
    {
        HAL_RIU_WRITE_MASK(sgwdma, HAL_RIU_REG_SGWDMA_WINFO_SEL, 0x2000, 0x3f00);
        line = HAL_RIU_READ(sgwdma, HAL_RIU_REG_SGWDMA_WINFO);
        HAL_RIU_WRITE(sgwdma, HAL_RIU_REG_SGWDMA_LINE_MAX, line - 1);
        HAL_RIU_WRITE(sgwdma, HAL_RIU_REG_SGWDMA_CLEAR, 0x0008);
        hal_riu_record_snapshot.irq_type = RIU_RECORD_WDMA_IRQ;
    }
    else
    {
        hal_riu_record_snapshot.bridge = HAL_RIU_BRIDGE_NUM;

        for (bridge = 0; bridge < HAL_RIU_BRIDGE_NUM; bridge++)
        {
            if (bridge < 4)
            {
                status = HAL_RIU_READ(record0, ((0x20 * bridge) + HAL_RIU_REG_CAP_INT)) & 0x0C00;
                if (status)
                {
                    HAL_RIU_WRITE_MASK(record0, ((0x20 * bridge) + HAL_RIU_REG_CAP_INT), 0x00C0, 0x00C0);
                    HAL_RIU_WRITE_MASK(record0, ((0x20 * bridge) + HAL_RIU_REG_CAP_INT), 0x0000, 0x00C0);
                    hal_riu_record_snapshot.bridge = bridge;
                    break;
                }
            }
            else
            {
                status = HAL_RIU_READ(record1, ((0x20 * (bridge - 4)) + HAL_RIU_REG_CAP_INT)) & 0x0C00;
                if (status)
                {
                    HAL_RIU_WRITE_MASK(record1, ((0x20 * (bridge - 4)) + HAL_RIU_REG_CAP_INT), 0x00C0, 0x00C0);
                    HAL_RIU_WRITE_MASK(record1, ((0x20 * (bridge - 4)) + HAL_RIU_REG_CAP_INT), 0x0000, 0x00C0);
                    hal_riu_record_snapshot.bridge = bridge;
                    break;
                }
            }
        }

        if (status & 0x0400)
        {
            hal_riu_record_snapshot.irq_type = RIU_RECORD_IRQ;
        }
        else
        {
            hal_riu_record_snapshot.irq_type = RIU_RECORD_WDMA_BUSY_IRQ;
        }
        hal_riu_record_snapshot.capture = HAL_RIU_CAPTUER_NUM;
    }
}

void hal_riu_record_get_snapshot(struct hal_riu_dbg_t *hal, struct riu_record_snapshot_t *snapshort)
{
    snapshort->irq_type = hal_riu_record_snapshot.irq_type;
    snapshort->bridge   = hal_riu_record_snapshot.bridge;
    snapshort->capture  = hal_riu_record_snapshot.capture;
}
#endif
