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

#ifndef __SS_EXP__
#define __SS_EXP__

#ifdef __cplusplus
extern "C" {
#endif

typedef void * WORK_HANDLE;
#define SS_EXP_PACKET_SLICE_CNT      (16)
#define SS_EXP_WORK_INFO_MSG_SIZE    (256)

typedef enum
{
    E_SS_EXP_SRC_WORK_NONE,
    E_SS_EXP_SRC_READ_ONLY,
    E_SS_EXP_SRC_WRITE_ONLY,
} SS_EXP_SourceWorkMode_e;

typedef struct SS_EXP_WorkInfo_s
{
    SS_EXP_SourceWorkMode_e enWorkMode;
    char                    workMsg[SS_EXP_WORK_INFO_MSG_SIZE];
} SS_EXP_WorkInfo_t;

typedef struct SS_EXP_TransferObject_s
{
    unsigned int packetCount;
    struct SS_EXP_TransferPakcetInfo_s
    {
        unsigned int  packetSize;
        char         *packetData;
    } packetArray[SS_EXP_PACKET_SLICE_CNT];
} SS_EXP_TransferObject_t;

typedef struct SS_EXP_WorkCfg_s
{
    long cycleSec;
    long cycleNSec;
    void *pUserData;
} SS_EXP_WorkCfg_t;

typedef struct SS_EXP_TransferActCb_s
{
    int (*fpSsExpOpen)(const char* url, void *privateData, unsigned char bRead, SS_EXP_WorkCfg_t *cfg);
    int (*fpSsExpClose)(void *pUsrData);
    int (*fpSsExpWorkMsg)(void *pUsrData, const char workMsg[SS_EXP_WORK_INFO_MSG_SIZE]); /* Only use for sink. */
    int (*fpSsExpTransfer)(void *pUsrData, SS_EXP_TransferObject_t *transferObj);
    int (*fpSsExpTransferDone)(void *pUsrData, const SS_EXP_TransferObject_t *transferObj);
    int (*fpSsExpRemoteRead)(void *pUsrData, char *pTransData, unsigned int u32DataSize);
    void *(*fpSsExpRemoteReadAlloc)(void *pUsrData, unsigned int size);
    void (*fpSsExpRemoteReadFree)(void *pUsrData, void *buf);
} SS_EXP_TransferActCb_t;

typedef struct SS_EXP_SourceAttr_s
{
    unsigned int uintPort;
    unsigned char bWithNoSync;
    void *privateData; /* Only used in bWithNoSync=1 */
} SS_EXP_SourceAttr_t;

typedef struct SS_EXP_SinkAttr_s
{
    const char *pAddress;
    void       *privateData;
} SS_EXP_SinkAttr_t;

void *      SS_EXP_SourceInit(const SS_EXP_SourceAttr_t *attr, const SS_EXP_TransferActCb_t *pstActCb);
int         SS_EXP_SourceDeinit(void *pInstance);
WORK_HANDLE SS_EXP_SourceCreateWork(void *pInstance, const char *pUrl, void *privateData, const SS_EXP_WorkInfo_t *pstWorkInfo);
int         SS_EXP_SourceDestroyWork(WORK_HANDLE hWorkHandle);
int         SS_EXP_SourceStart(void *pInstance);
int         SS_EXP_SourceStop(void *pInstance);
void *      SS_EXP_SinkInit(const SS_EXP_SinkAttr_t *attr, const SS_EXP_TransferActCb_t *pstActCb);
int         SS_EXP_SinkDeinit(void *pInstance);
int         SS_EXP_GetIpAddr(const char *interface, char *ipAddress, unsigned int addrLen);

#ifdef __cplusplus
}
#endif

#endif //__SS_EXP__
