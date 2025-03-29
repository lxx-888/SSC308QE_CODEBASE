/*
 * sstar_android_bootreason.h - Sigmastar
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

#ifndef _SSTAR_ANDROID_BOOTREASON_
#define _SSTAR_ANDROID_BOOTREASON_

extern char *sstar_get_reboot_reason(void);

enum knwon_boot_reason_types {
	ANDR_BOOT_REASON_COLD,
	ANDR_BOOT_REASON_HARD,
	ANDR_BOOT_REASON_WARM,
	ANDR_BOOT_REASON_WATCHDOG,
	ANDR_BOOT_REASON_REBOOT,
	ANDR_BOOT_REASON_TYPES,
};

#endif /* _SSTAR_ANDROID_BOOTREASON_ */

