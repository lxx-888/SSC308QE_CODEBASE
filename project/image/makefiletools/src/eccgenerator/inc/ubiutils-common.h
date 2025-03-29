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

#ifndef __UBI_UTILS_COMMON_H__
#define __UBI_UTILS_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

long long ubiutils_get_bytes(const char *str);
void ubiutils_print_bytes(long long bytes, int bracket);
void ubiutils_print_text(FILE *stream, const char *txt, int len);
int ubiutils_srand(void);

#ifdef __cplusplus
}
#endif

#endif /* !__UBI_UTILS_COMMON_H__ */

