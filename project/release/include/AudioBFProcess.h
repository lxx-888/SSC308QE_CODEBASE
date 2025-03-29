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


#ifndef __AUDIOBFPROCESS_H__
#define __AUDIOBFPROCESS_H__


#ifdef __cplusplus
extern "C"
{
#endif


#define ALGO_BF_RET_SUCCESS                 0x00000000
#define ALGO_BF_RET_INVALID_LICENSE         0x30000401
#define ALGO_BF_RET_INVALID_HANDLE          0x30000402
#define ALGO_BF_RET_INVALID_SAMPLERATE      0x30000403
#define ALGO_BF_RET_INVALID_BITWIDTH        0x30000404
#define ALGO_BF_RET_INVALID_SOUNDSPEED      0x30000405
#define ALGO_BF_RET_INVALID_MIC_NUM         0x30000406
#define ALGO_BF_RET_INVALID_MIC_GEOMETRY    0x30000407
#define ALGO_BF_RET_INVALID_BF_ALGORITHM    0x30000408
#define ALGO_BF_RET_INVALID_VAD_THRESHOLD   0x30000409
#define ALGO_BF_RET_INVALID_INTENSITY       0x30000410
#define ALGO_BF_RET_INVALID_GAIN            0x30000411


#define MAX_MIC_CHANNEL_NUM (6)
#define COORDINATE_DIM      (2)

typedef enum{
    IAA_BF_SAMPLE_RATE_8000 = 8000,
    IAA_BF_SAMPLE_RATE_16000 = 16000,
    IAA_BF_SAMPLE_RATE_24000 = 24000,
    IAA_BF_SAMPLE_RATE_32000 = 32000,
    IAA_BF_SAMPLE_RATE_48000 = 48000,
}BfSampleRate_e;


typedef enum{
    IAA_BF_LINEAR_ARRAY = 0,
    IAA_BF_CIRCULAR_ARRAY,
}BfMicGeometry_e;


typedef enum{
    IAA_FIXED_BF = 0,
    IAA_ADAPTIVE_BF = 1,
    IAA_ADAPTIVE_NORMAL_BF = 2,
    IAA_ADAPTIVE_FAST_BF = 3,
}BfAlgorithm_e;

typedef struct{
    int sampleRate;
    int bitWidth;
    int micNum;
    int soundSpeed;

    BfAlgorithm_e bfAlgorithm;
    BfMicGeometry_e micGeometry;
    float micCoordinate[MAX_MIC_CHANNEL_NUM][COORDINATE_DIM];
}AudioBfInit_t;


typedef struct{
    float vadThreshold;
    float gain;
    int intensity;
}AudioBfConfig_t;


typedef void *BF_HANDLE;
int IaaBf_GetBufferSize();
BF_HANDLE IaaBf_Init(char* workBufAddress, AudioBfInit_t *bfInit);
int IaaBf_SetConfig(BF_HANDLE handle, AudioBfConfig_t *bfConfig);
int IaaBf_GetConfig(BF_HANDLE handle);
int IaaBf_GetInputSamples(BF_HANDLE handle, int *samples);
int IaaBf_Run(BF_HANDLE handle, short *input, short *output, int angle);
int IaaBf_Free(BF_HANDLE handle);
int IaaBf_setCallbackFunc(int(*log)(const char *szFmt, ...),
                         int(*envSet)(char *key, char *par),
                         int(*envGetString)(char *var, char *buf, unsigned int size),
                         int(*envSave)(void),
                         int(*readUuid)(unsigned long long *u64Uuid));


#ifdef __cplusplus
}
#endif
#endif /* __AUDIOBFPROCESS_H__ */