/*
 * mbx_protocol.h- Sigmastar
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

#ifndef _MBX_PROTOCOL_H_
#define _MBX_PROTOCOL_H_
#define MBX_OS_CLASS_BASE 200

typedef enum
{
    E_MBX_CLASS_ALARM = MBX_OS_CLASS_BASE,
    E_MBX_CLASS_ALARM_ACK,
    E_MBX_CLASS_DUMP_LOG,
    E_MBX_CLASS_MAX
} mbx_os_class_e;

#endif
