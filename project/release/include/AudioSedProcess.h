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


#ifndef __AUDIOSEDPROCESS_H__
#define __AUDIOSEDPROCESS_H__

#ifdef __cplusplus
extern "C"
{
#endif

//SED Error Code
#define ALGO_SED_RET_SUCCESS                0x00000000
#define ALGO_SED_RET_INVALID_HANDLE         0x80000401
#define ALGO_SED_RET_INVALID_LICENSE        0x80000402
#define ALGO_SED_RET_INVALID_SAMPLERATE     0x80000403
#define ALGO_SED_RET_INVALID_BITWIDTH       0x80000404
#define ALGO_SED_RET_INVALID_MODEL          0x80000405
#define ALGO_SED_RET_INVALID_DETECT_MODE    0x80000406
#define ALGO_SED_RET_INVALID_THRESHOLD      0x80000407
#define ALGO_SED_RET_INVALID_IPU_MAX_SIZE   0x80000408
#define ALGO_SED_RET_INVALID_VAD_THRESHOLD  0x80000409

#define SED_MAX_FILE_PATH_LEN               (256)
#define SED_MAX_EVENT_NUM                   (2)


typedef struct{
    int sampleRate;
    int bitWidth;
    int ipuMaxSize;
    int hadCreateDevice;
    char modelPath[SED_MAX_FILE_PATH_LEN];
}AudioSedInit_t;


typedef enum{
    IAA_SENSTITVE_MODE = 0,
    IAA_ROBUST_MODE
}AudioSedMode_e;

typedef struct{
    int smoothLength;
    float vadThreshold;
    AudioSedMode_e detectMode;
    float eventThreshold[SED_MAX_EVENT_NUM];
}AudioSedConfig_t;


typedef void *SED_HANDLE;
SED_HANDLE IaaSed_Init(AudioSedInit_t *sedInit);
int IaaSed_SetConfig(SED_HANDLE handle, AudioSedConfig_t *sedConfig);
int IaaSed_GetBufferSize();
int IaaSed_GetConfig(SED_HANDLE handle);
int IaaSed_GetInputSamples(SED_HANDLE handle, int *samples);
int IaaSed_LoadData(SED_HANDLE handle, const short *data, int *detectFlag);
int IaaSed_Run(SED_HANDLE handle, int *eventIndex);
int IaaSed_GetScore(SED_HANDLE handle, float *eventScore, int *eventNum);
int IaaSed_Free(SED_HANDLE handle);
int IaaSed_setCallbackFunc(int(*log)(const char *szFmt, ...),
                         int(*envSet)(char *key, char *par),
                         int(*envGetString)(char *var, char *buf, unsigned int size),
                         int(*envSave)(void),
                         int(*readUuid)(unsigned long long *u64Uuid));

#ifdef __cplusplus
}
#endif

#endif /* __AUDIOSEDPROCESS_H__ */
