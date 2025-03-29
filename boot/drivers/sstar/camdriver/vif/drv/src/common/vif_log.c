/*
 * vif_log.c - Sigmastar
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

#include <vif_log.h>

static u32 g_uLogEn = E_VIF_LOG_ERR | E_VIF_LOG_INFO;
static u32 g_pdbgEn = 0;

void VIFLogSetEn(int id, int en)
{
    u32 mask = id;
    if (en)
    {
        g_uLogEn = g_uLogEn | mask;
    }
    else
    {
        g_uLogEn = g_uLogEn & ~mask;
    }
}

inline unsigned int VIFLogGetEn(void)
{
    return g_uLogEn;
}

void VIFPipeDbgSetEn(u8 pipe, u8 en)
{
    u32 mask = (1 << pipe);
    if (en)
    {
        g_pdbgEn = g_pdbgEn | mask;
    }
    else
    {
        g_pdbgEn = g_pdbgEn & ~mask;
    }
}

u32 VIFPipeDbgGetEn(u8 pipe)
{
    return g_pdbgEn & (1 << pipe);
}
