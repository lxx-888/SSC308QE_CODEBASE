/*
 * drv_uart_ioctl.h- Sigmastar
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

#ifndef __DRV_UART_IOCTL_H__
#define __DRV_UART_IOCTL_H__

#define UART_CMD 0x80

#define UART_CMD_DISABLE_RX _IO('T', UART_CMD + 0x00)
#define UART_CMD_ENABLE_RX  _IO('T', UART_CMD + 0x01)
#define UART_CMD_DISABLE_TX _IO('T', UART_CMD + 0x02)
#define UART_CMD_ENABLE_TX  _IO('T', UART_CMD + 0x03)

#endif /* __DRV_UART_IOCTL_H__ */
