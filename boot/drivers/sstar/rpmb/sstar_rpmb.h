/*
 * sstar_rpmb.h - Sigmastar
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

#ifndef _SSTAR_RPMB_H_
#define _SSTAR_RPMB_H_

#include <linux/types.h>

#ifndef CONFIG_BLK
#define get_mmc_hwpart(mmc) mmc->block_dev.hwpart
#else
#define get_mmc_hwpart(mmc) mmc_get_blk_desc(mmc)->hwpart
#endif

#define RPMB_SZ_DATA                  256
#define ROLLBACK_INDEX_LOCATION_COUNT 16

enum sstar_rpmb_fields
{
    RPMB_FIELD_FASTBOOT_LOCK_FLAGS,
    RPMB_FIELD_ROLLBACK_INDEX,
    RPMB_FIELD_MAX
};

extern const unsigned char _binary_drivers_sstar_rpmb_rpmb_key_start;
extern const unsigned char _binary_drivers_sstar_rpmb_rpmb_key_end;

extern int sstar_rpmb_set_key(void);
extern int sstar_rpmb_get_lock_flags(uint64_t *out_flags);
extern int sstar_rpmb_set_lock_flags(uint64_t flags);
extern int sstar_rpmb_get_rollback_index(uint8_t location, uint64_t *out_rollback_index);
extern int sstar_rpmb_set_rollback_index(uint8_t location, uint64_t rollback_index);
#endif /*_SSTAR_RPMB_H_*/
