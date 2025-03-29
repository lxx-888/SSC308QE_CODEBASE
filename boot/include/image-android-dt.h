/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2018 Linaro Ltd.
 * Sam Protsenko <semen.protsenko@linaro.org>
 */

#ifndef IMAGE_ANDROID_DT_H
#define IMAGE_ANDROID_DT_H

#include <linux/types.h>

bool android_dt_check_header(ulong hdr_addr);
bool android_dt_get_fdt_by_index(ulong hdr_addr, u32 index, ulong *addr,
				 u32 *size);

#if !defined(CONFIG_SPL_BUILD)
void android_dt_print_contents(ulong hdr_addr);
#endif

#ifdef CONFIG_SSTAR_DTB_OVERLAY_SUPPORT
#include <part.h>
#include <blk.h>
bool android_dt_get_fdt_by_compatible(ulong hdr_addr, char *compatible,
		ulong *addr, u32 *size);
int android_dt_overlay_apply_verbose(void *dtb_addr, char *dtbo_list,
		struct blk_desc *dev_desc);
#endif

#endif /* IMAGE_ANDROID_DT_H */
