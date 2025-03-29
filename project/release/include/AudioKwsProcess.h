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

#ifndef __AUDIOKWSPROCESS_H__
#define __AUDIOKWSPROCESS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define ALGO_KWS_RET_SUCCESS                0x00000000
#define ALGO_KWS_RET_INVALID_LICENSE        0x90000401
#define ALGO_KWS_RET_INVALID_HANDLE         0x90000402
#define ALGO_KWS_RET_INVALID_SAMPLERATE     0x90000403
#define ALGO_KWS_RET_INVALID_BITWIDTH       0x90000404
#define ALGO_KWS_RET_INVALID_CHANNEL        0x90000405
#define ALGO_KWS_RET_INVALID_VAD_MODE       0x90000406
#define ALGO_KWS_RET_INVALID_SKIP_DURATION  0x90000407
#define ALGO_KWS_RET_INVALID_KWS_THRESHOLD  0x90000408
#define ALGO_KWS_RET_INVALID_VAD_THRESHOLD  0x90000409
#define ALGO_KWS_RET_INVALID_BREAK_DURATION 0x9000040A

#define KWS_MAX_FILE_NAME_LEN               (256)
#define KWS_MAX_NUM                         (100)
#define KWS_MAX_LEN                         (20)

typedef struct{
    int sampleRate;
    int bitWidth;
    int channel;
    int ipuMaxSize;
    int keywordNum;
    char keywords[KWS_MAX_NUM][KWS_MAX_LEN];
    char modelPath[KWS_MAX_FILE_NAME_LEN];
    char graphPath[KWS_MAX_FILE_NAME_LEN];
    char dictPath[KWS_MAX_FILE_NAME_LEN];
}AudioKwsInit_t;

typedef struct{
    int vadMode;
    int breakDuration;
    float skipDuration;
    float vadThreshold;
    float kwsThreshold[KWS_MAX_NUM];
}AudioKwsConfig_t;

typedef void *KWS_HANDLE;
KWS_HANDLE IaaKws_Init(AudioKwsInit_t *kwsInit);
int IaaKws_GetBufferSize();
int IaaKws_SetConfig(KWS_HANDLE handle, AudioKwsConfig_t *kwsConfig);
int IaaKws_ResetConfig(KWS_HANDLE handle, AudioKwsConfig_t *kwsConfig);
int IaaKws_GetConfig(KWS_HANDLE handle);
int IaaKws_GetInputSamples(KWS_HANDLE handle, int *samples);
int IaaKws_Run(KWS_HANDLE handle, short *input, int *kwsIndex);
int IaaKws_Free(KWS_HANDLE);
int IaaKws_setCallbackFunc(int(*log)(const char *szFmt, ...),
                         int(*envSet)(char *key, char *par),
                         int(*envGetString)(char *var, char *buf, unsigned int size),
                         int(*envSave)(void),
                         int(*readUuid)(unsigned long long *u64Uuid));

#ifdef __cplusplus
}
#endif


#endif /* __AUDIOKWSPROCESS_H__ */
