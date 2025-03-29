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


#ifndef __AUDIOSEPROCESS_H__
#define __AUDIOSEPROCESS_H__

#ifdef __cplusplus
extern "C"
{
#endif


#define ALGO_SE_RET_SUCCESS                  0x00000000
#define ALGO_SE_RET_INVALID_LICENSE          0x70000401
#define ALGO_SE_RET_INVALID_HANDLE           0x70000402
#define ALGO_SE_RET_INVALID_SAMPLERATE       0x70000403
#define ALGO_SE_RET_INVALID_BITWIDTH         0x70000404
#define ALGO_SE_RET_INVALID_CHANNEL          0x70000405
#define ALGO_SE_RET_INVALID_INTENSITY        0x70000406
#define ALGO_SE_RET_INVALID_NOISETYPE        0x70000407
#define ALGO_SE_RET_INVALID_NORMALIZE_TARGET 0x70000408
#define ALGO_SE_RET_INVALID_NORMALIZE_SMOOTH 0x70000409
#define ALGO_SE_RET_INVALID_NORMALIZE_POS    0x7000040A
#define ALGO_SE_RET_INVALID_NORMALIZE_MODE   0x7000040B
#define ALGO_SE_RET_INVALID_NORMALIZE_VADTHR 0x7000040C

typedef enum{
    IAA_SE_OFFICE_NOISE = 0,
    IAA_SE_TRAFFIC_NOISE,
    IAA_SE_ALL_NOISE,
}AudioSeNoiseType_e;

typedef struct{
    int sampleRate;
    int bitWidth;
    int channel;
}AudioSeInit_t;

typedef struct{
    AudioSeNoiseType_e noiseType;
    int intensity;

    int normalize;
    int smooth;
    int normalizeMode;
    int normalizeVadThreshold;
    int normalizePosition;
}AudioSeConfig_t;

typedef void *SE_HANDLE;
int IaaSe_GetBufferSize();
SE_HANDLE IaaSe_Init(char* workBufAddress, AudioSeInit_t *seInit);
int IaaSe_SetConfig(SE_HANDLE handle, AudioSeConfig_t *seConfig);
int IaaSe_GetConfig(SE_HANDLE handle);
int IaaSe_GetInputSamples(SE_HANDLE handle, int *samples);
int IaaSe_Run(SE_HANDLE handle, short *input);
int IaaSe_Free(SE_HANDLE);
int IaaSe_setCallbackFunc(int(*log)(const char *szFmt, ...),
                         int(*envSet)(char *key, char *par),
                         int(*envGetString)(char *var, char *buf, unsigned int size),
                         int(*envSave)(void),
                         int(*readUuid)(unsigned long long *u64Uuid));


#ifdef __cplusplus
}
#endif

#endif /* __AUDIOSEPROCESS_H__ */
