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
#ifndef __SS_FONT_H__
#define __SS_FONT_H__

#ifdef __cplusplus
extern "C"{
#endif	// __cplusplus

enum en_font_rotation
{
    FONT_ROT_NONE,
    FONT_ROT_90,
    FONT_ROT_180,
    FONT_ROT_270
};

int display_text(char *fb, int x, int y, int stride,
                       int color, int pix_byte_of_fmt,
                       const char *ascii_text, int len, enum en_font_rotation rot,
                       int font_w, int font_h, const unsigned char * font);
#ifdef __cplusplus
}
#endif	// __cplusplus

#endif//__SS_FONT_H__
