/*
 * sstar_base64_uart.h - Sigmastar
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

#ifndef __SSTAR_BASE64_UART_H__
#define __SSTAR_BASE64_UART_H__

#include "cam_os_wrapper.h"

#define BASE64_MIME_CHAR_PER_LINE        76
#define BASE64_SEQ_ENC_RAW_DATA_PER_LINE (BASE64_MIME_CHAR_PER_LINE / 4 * 3)

typedef struct
{
    u32           cnt;
    unsigned char buf[BASE64_SEQ_ENC_RAW_DATA_PER_LINE];
    void (*func)(const unsigned char *, u32);
} Base64UartEnc_t;

void Base64EncodeToUart(const unsigned char *in, u32 len);

void Base64UartEncInit(void *handle);
void Base64UartEncWrite(void *handle, const unsigned char *in, u32 len);
void Base64UartEncFinish(void *handle);

#endif //__SSTAR_BASE64_UART_H__