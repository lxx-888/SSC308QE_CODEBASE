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

#ifndef __AUDIOVADPROCESS_H__
#define __AUDIOVADPROCESS_H__


#ifdef __cplusplus
extern "C"
{
#endif


#define ALGO_VAD_RET_SUCCESS                0x00000000
#define ALGO_VAD_RET_INVALID_LICENSE        0x40000401
#define ALGO_VAD_RET_INVALID_HANDLE         0x40000402
#define ALGO_VAD_RET_INVALID_SAMPLERATE     0x40000403
#define ALGO_VAD_RET_INVALID_BITWIDTH       0x40000404
#define ALGO_VAD_RET_INVALID_THRESHOLD      0x40000405
#define ALGO_VAD_RET_INVALID_MODE           0x40000406

typedef struct
{
    int sampleRate;
    int bitWidth;
    int frameLen;
}AudioVadInit_t;

typedef struct
{
    int vadMode;
    float vadThreshold;
}AudioVadConfig_t;

typedef void *VAD_HANDLE;
VAD_HANDLE IaaVad_Init(char* workBufAddress, AudioVadInit_t *vadInit);
int IaaVad_GetBufferSize();
int IaaVad_SetConfig(VAD_HANDLE handle, AudioVadConfig_t *vadConfig);
int IaaVad_GetConfig(VAD_HANDLE handle);
int IaaVad_GetInputSamples(VAD_HANDLE handle, int *samples);
int IaaVad_Run(VAD_HANDLE handle, const short *input, int *vadFlag);
int IaaVad_Free(VAD_HANDLE handle);
int IaaVad_setCallbackFunc(int(*log)(const char *szFmt, ...),
                         int(*envSet)(char *key, char *par),
                         int(*envGetString)(char *var, char *buf, unsigned int size),
                         int(*envSave)(void),
                         int(*readUuid)(unsigned long long *u64Uuid));


#ifdef __cplusplus
}
#endif


#endif /* __AUDIOVADPROCESS_H__ */
