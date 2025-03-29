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

#ifndef __SS_NALU__
#define __SS_NALU__

#ifdef __cplusplus
extern "C" {
#endif

#include "list.h"

enum SS_NALU_DataFormat_e
{
    E_SS_NALU_DATA_FORMAT_NULL,
    E_SS_NALU_DATA_FORMAT_H264,
    E_SS_NALU_DATA_FORMAT_H265
};

typedef struct SS_NALU_DataParse_s
{
    enum SS_NALU_DataFormat_e format;
    unsigned int              dataBlockSize;
    void *                    userData;
    unsigned char             bRepeatInfo; /* Auto make up sps/pps/vps if not parsed. */
    int (*fpReadInputData)(char *data, unsigned int size, void *user);
} SS_NALU_DataParse_t;

typedef struct SS_NALU_EsSliceData_s
{
    struct list_head sliceGrp;
    unsigned char    startCodeSize;
    unsigned char    bFrameStart;
    unsigned char    bFrameIdr;
    unsigned char    bPrefixInfo;
    unsigned char    bSuffixInfo;
    char *           data;
    unsigned int     size;
} SS_NALU_EsSliceData_t;

typedef struct SS_NALU_EsFrame_s
{
    // Head of SS_NALU_EsSliceData_t
    struct list_head sliceGrp;
    unsigned int     totalDataSize; // Size with 'NALU' start code.
} SS_NALU_EsFrame_t;

void *SS_NALU_CreateParser(const SS_NALU_DataParse_t *parseConfig);
int   SS_NALU_GetOneFrame(void *handle, SS_NALU_EsFrame_t *listData);
int   SS_NALU_PutOneFrame(void *handle, SS_NALU_EsFrame_t *listData);
int   SS_NALU_DestroyParser(void *handle);


#ifdef __cplusplus
}
#endif

#endif // __SS_NALU__
