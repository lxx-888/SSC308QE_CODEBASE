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
#ifndef _SS_UVC_XU_H_
#define _SS_UVC_XU_H_

#include <stdint.h>
#include "ss_uvc_xu_datatype.h"
#include "list.h"

#define EU_ISP_CMD_OK            0x00
#define EU_ISP_CMD_NOT_SUPPORT    0x80

typedef struct _VC_CMD_CFG
{
    unsigned long bCmdCap;
    unsigned char bInfoCap;
    unsigned short bCmdLen;
    unsigned char bInfoLen;
    /*If cmd > 4, the following data is a pointer*/
    unsigned long  dwResVal;
    unsigned long  dwDefVal;
    unsigned long  dwMinVal;
    unsigned long  dwMaxVal;
    unsigned long  dwCurVal;
} VC_CMD_CFG;

typedef struct SS_UVC_XuRoiNode_t
{
    struct list_head list;
    int16_t u16Roix;
    int16_t u16Roiy;
    int16_t u16RoiWidth;
    int16_t u16RoiHeight;
}SS_UVC_XuRoiNode_t;

void usb_vc_cmd_cfg(uint8_t req, VC_CMD_CFG *cfg, unsigned long cur_val, struct uvc_request_data *resp);

// process PU, CT, XU job.
extern uint8_t pu_brightness_data[2];
extern uint8_t ct_roi_data[10];
extern uint8_t ct_scanning_mode_data;

int8_t UVC_GetRoiData(SS_UVC_Device_t *pdev, SS_UVC_XuRoiNode_t *pRoiReadListNode);
int8_t usb_vc_ct_cs_out(SS_UVC_Device_t *pdev, uint8_t entity_id, uint8_t cs, uint32_t len, struct uvc_request_data *data);
int8_t usb_vc_pu_cs_out(SS_UVC_Device_t *pdev, uint8_t entity_id, uint8_t cs, uint32_t len, struct uvc_request_data *data);

int8_t usb_vc_eu1_cs_out(SS_UVC_Device_t *pdev, uint8_t entity_id, uint8_t cs, uint32_t len, struct uvc_request_data *data);
int8_t usb_vc_eu1_cs(uint8_t cs, uint8_t req, struct uvc_request_data *resp);

int8_t usb_vc_eu2_cs_out(uint8_t entity_id, uint8_t cs, uint32_t len, struct uvc_request_data *data);
int8_t usb_vc_eu2_cs(uint8_t cs, uint8_t req, struct uvc_request_data *resp);

int8_t usb_vc_eu3_cs_out(uint8_t entity_id, uint8_t cs, uint32_t len, struct uvc_request_data *data);
int8_t usb_vc_eu3_cs(uint8_t cs, uint8_t req, struct uvc_request_data *resp);

typedef struct _DataExchangeParam
{
    unsigned short id;            //command ID
    unsigned long data_len;        //transfer lens
    unsigned long cur_offset;    //current read/write offset
    char *ptr;                    //data buffer
} DataExchangeParam;
#endif // _SS_UVC_XU_H_
