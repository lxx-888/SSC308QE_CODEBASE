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
#ifndef __ONVIF_SERVER_H_
#define __ONVIF_SERVER_H_


#ifdef __cplusplus
extern "C"
{
#endif

#define MST_ONVIF_NAME_MAX_LEN (128)
struct MST_ONVIF_Desc
{
    char name[MST_ONVIF_NAME_MAX_LEN];
};
#define MST_ONVIF_DESC_MAX_NUM (32)
struct MST_ONVIF_Descs
{
    unsigned int          count;
    struct MST_ONVIF_Desc descs[MST_ONVIF_DESC_MAX_NUM];
};

void MST_ONVIF_Init();
void MST_ONVIF_SetDescs(const struct MST_ONVIF_Descs *pDescs);
void MST_ONVIF_StartTask();
void MST_ONVIF_StopTask();

#ifdef __cplusplus
}
#endif

#endif
