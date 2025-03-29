/*
 * sstar_base64_uart.c - Sigmastar
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

#include <linux/string.h>
#include "sstar_base64_uart.h"

void Base64EncodeToUart(const unsigned char *in, u32 len)
{
    u32        elen;
    u32        i;
    u32        v;
    char       out[BASE64_MIME_CHAR_PER_LINE + 1] = {0};
    const char b64chars[]                         = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    u32        charCnt                            = 0;

    if (in == NULL || len == 0)
        return;

    elen = len;
    if (len % 3 != 0)
    {
        elen += 3 - (len % 3);
    }
    elen /= 3;
    elen *= 4;

    for (i = 0; i < len; i += 3)
    {
        v = in[i];
        v = i + 1 < len ? v << 8 | in[i + 1] : v << 8;
        v = i + 2 < len ? v << 8 | in[i + 2] : v << 8;

        out[charCnt++] = b64chars[(v >> 18) & 0x3F];
        out[charCnt++] = b64chars[(v >> 12) & 0x3F];
        if (i + 1 < len)
        {
            out[charCnt++] = b64chars[(v >> 6) & 0x3F];
        }
        else
        {
            out[charCnt++] = '=';
        }
        if (i + 2 < len)
        {
            out[charCnt++] = b64chars[v & 0x3F];
        }
        else
        {
            out[charCnt++] = '=';
        }

        if (charCnt == BASE64_MIME_CHAR_PER_LINE)
        {
            CamOsPrintf(KERN_EMERG "%s\n", out);
            memset(out, 0, sizeof(out));
            charCnt = 0;
        }
    }

    if (charCnt)
    {
        CamOsPrintf(KERN_EMERG "%s\n", out);
    }

    return;
}

void Base64UartEncInit(void *handle)
{
    Base64UartEnc_t *enc = (Base64UartEnc_t *)handle;

    memset(enc, 0, sizeof(Base64UartEnc_t));
    enc->func = Base64EncodeToUart;
}

void Base64UartEncWrite(void *handle, const unsigned char *in, u32 len)
{
    Base64UartEnc_t *enc     = (Base64UartEnc_t *)handle;
    u32              cnt     = 0;
    u32              pushLen = 0;
    u32              total   = len;
    unsigned char *  inPtr   = (unsigned char *)in;

    if (enc->func == NULL)
        return;

    if (inPtr == NULL || total == 0)
    {
        return;
    }

    pushLen = CAM_OS_MIN(total, sizeof(enc->buf) - enc->cnt);
    memcpy((void *)&enc->buf[enc->cnt], (void *)inPtr + cnt, pushLen);
    inPtr += pushLen;
    enc->cnt += pushLen;
    total -= pushLen;

    while (enc->cnt == sizeof(enc->buf))
    {
        enc->func(enc->buf, enc->cnt);

        memset(enc->buf, 0, sizeof(enc->buf));
        enc->cnt = 0;

        pushLen = CAM_OS_MIN(total, sizeof(enc->buf) - enc->cnt);
        memcpy((void *)&enc->buf[enc->cnt], (void *)inPtr + cnt, pushLen);
        inPtr += pushLen;
        enc->cnt += pushLen;
        total -= pushLen;
    }

    return;
}

void Base64UartEncFinish(void *handle)
{
    Base64UartEnc_t *enc = (Base64UartEnc_t *)handle;

    if (enc->cnt)
    {
        enc->func(enc->buf, enc->cnt);
        memset(enc->buf, 0, sizeof(enc->buf));
        enc->cnt = 0;
    }
}
