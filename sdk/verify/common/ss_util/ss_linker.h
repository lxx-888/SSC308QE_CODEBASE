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
#ifndef __SS_LINKER__
#define __SS_LINKER__

#include "ss_packet.h"

class ss_linker_base
{
public:
    explicit ss_linker_base() {}
    virtual ~ss_linker_base() {}
    virtual int enqueue(stream_packet_obj &obj) = 0;
    virtual stream_packet_obj dequeue(unsigned int time_out_ms) = 0;
};
#endif

