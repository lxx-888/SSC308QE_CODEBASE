/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#include <stdio.h>
#include <string.h>
#include "ss_font.h"

int display_text(char *fb, int x, int y, int stride,
                       int color, int pix_byte_of_fmt,
                       const char *ascii_text, int len, enum en_font_rotation rot,
                       int font_w, int font_h, const unsigned char * font) {
    char *p = NULL, *q = NULL, *c = NULL;
    unsigned char word;
    int line_offset = 0;
    int i = 0, j = 0, k = 0, l = 0;

    p = fb + x * pix_byte_of_fmt + y * stride;
    for (i = 0; i < len; i++) {
        for (j = 0; j < font_h * font_w / 8; j++) {
            word = font[(unsigned char)ascii_text[i] * (font_w * font_h / 8) + j]; //get a byte info.
            if (word){
                switch (rot){
                    case FONT_ROT_NONE:
                        line_offset = font_w * i * pix_byte_of_fmt + ((j * 8) % font_w) * pix_byte_of_fmt + (j * 8)/ font_w * stride;
                        break;
                    case FONT_ROT_90:
                        line_offset = -font_w * i * stride - ((j * 8) % font_w) * stride + (j * 8)/ font_w  * pix_byte_of_fmt;
                        break;
                    case FONT_ROT_180:
                        line_offset = -font_w * i * pix_byte_of_fmt - ((j * 8) % font_w) * pix_byte_of_fmt - (j * 8)/ font_w * stride;
                        break;
                    case FONT_ROT_270:
                        line_offset = font_w * i * stride + ((j * 8) % font_w) * stride - (j * 8)/ font_w  * pix_byte_of_fmt;
                        break;
                    default:
                        return -1;
                }
                for (k = 0; k < sizeof(word) * 8; k++) {
                    switch (rot){
                        case FONT_ROT_NONE:
                            q = p + line_offset + k * pix_byte_of_fmt;
                            break;
                        case FONT_ROT_90:
                            q = p + line_offset - k * stride;
                            break;
                        case FONT_ROT_180:
                            q = p + line_offset - k * pix_byte_of_fmt;
                            break;
                        case FONT_ROT_270:
                            q = p + line_offset + k * stride;
                            break;
                        default:
                            return -1;
                    }
                    if (word & (1 << k)) {
                        //Draw point
                        c = (char *)&color;
                        for (l = 0; l < pix_byte_of_fmt; l++) {
                            *q++ = *c++;
                        }
                    }
                } 
            }
        }
    }

    return 0;
}
