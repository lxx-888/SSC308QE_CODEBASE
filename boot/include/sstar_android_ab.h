/*
* sstar_android_ab.h - Sigmastar
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

#ifndef _SSTAR_ANDROID_AB_H_
#define _SSTAR_ANDROID_AB_H_

#include <sstar_avb_verify.h>
#include <android_bootloader_message.h>

#define ANDROID_ARG_SLOT_SUFFIX "androidboot.slot_suffix="

AvbIOResult sstar_ab_control_read_metadata(AvbABOps* ab_ops, struct bootloader_control* abc);
AvbIOResult sstar_ab_control_write_metadata(AvbABOps* ab_ops, const struct bootloader_control* abc);
int sstar_ab_control_get_current_slot(struct bootloader_control *abc);
void sstar_ab_control_default(struct bootloader_control *abc);

#endif /* _SSTAR_ANDROID_AB_H_ */
