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

#include "ptree_preload.h"
#include "ptree_maker.h"

void PTREE_PRELOAD_CLear(void)
{
    PTREE_MAKER_Clear();
}

void PTREE_PRELOAD_Setup(void)
{
#ifdef INTERFACE_SENSOR
    PTREE_MAKER_SETUP(SNR);
#endif
#ifdef INTERFACE_VIF
    PTREE_MAKER_SETUP(VIF);
#endif
#ifdef INTERFACE_ISP
    PTREE_MAKER_SETUP(ISP);
    PTREE_MAKER_MOD_SETUP(AESTABLE);
#endif
#ifdef INTERFACE_SCL
    PTREE_MAKER_SETUP(SCL);
    PTREE_MAKER_MOD_SETUP(SCL_STRETCH);
#endif
#ifdef INTERFACE_VENC
    PTREE_MAKER_SETUP(VENC);
#endif
#ifdef INTERFACE_SYS
    PTREE_MAKER_MOD_SETUP(POOL);
#endif
#ifdef INTERFACE_LDC
    PTREE_MAKER_SETUP(LDC);
#endif
#ifdef INTERFACE_RGN
    PTREE_MAKER_MOD_SETUP(RGN);
#endif
#ifdef INTERFACE_DISP
    PTREE_MAKER_MOD_SETUP(DISP);
#endif
#ifdef INTERFACE_VDF
    PTREE_MAKER_SETUP(VDF);
#endif
#ifdef INTERFACE_AI
    PTREE_MAKER_SETUP(AI);
#endif
#ifdef INTERFACE_AO
    PTREE_MAKER_SETUP(AO);
#endif
#ifdef INTERFACE_IPU
    PTREE_MAKER_MOD_SETUP(DET);
//#ifdef INTERFACE_SHADOW
//    PTREE_MAKER_SETUP(HSEG);
//#endif
#endif
#if defined(CONFIG_CUS3A_SUPPORT) || defined(INTERFACE_CUS3A)
    PTREE_MAKER_MOD_SETUP(IQ);
#endif
#ifdef PTREE_MOD_EMPTY
    PTREE_MAKER_SETUP(EMPTY, EMPTY0, EMPTY1, EMPTY2, EMPTY3, EMPTY4);
#endif
#ifdef PTREE_MOD_FILE
    PTREE_MAKER_SETUP(FILE);
#endif
#ifdef PTREE_MOD_TICK
    PTREE_MAKER_SETUP(TICK);
#endif
#ifdef PTREE_MOD_SYNC
    PTREE_MAKER_MOD_SETUP(SYNC);
#endif
#ifdef PTREE_MOD_STDIO
    PTREE_MAKER_SETUP(STDIO);
#endif
#ifdef PTREE_MOD_PASS
    PTREE_MAKER_SETUP(PASS);
#endif
#ifdef CONFIG_USB_GADGET_UVC_SUPPORT
    PTREE_MAKER_SETUP(UVC);
#endif
}
