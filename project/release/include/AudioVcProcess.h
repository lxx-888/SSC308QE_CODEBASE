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

#ifndef __AUDIOVCPROCESS_H__
#define __AUDIOVCPROCESS_H__

#ifdef __cplusplus
extern "C"
{
#endif


#define ALGO_VC_RET_SUCCESS                 0x00000000
#define ALGO_VC_RET_INVALID_LICENSE         0xA0000401
#define ALGO_VC_RET_INVALID_HANDLE          0xA0000402
#define ALGO_VC_RET_INVALID_SAMPLERATE      0xA0000403
#define ALGO_VC_RET_INVALID_BITWIDTH        0xA0000404
#define ALGO_VC_RET_INVALID_CHANNEL         0xA0000405
#define ALGO_VC_RET_INVALID_SPEEDFACTOR     0xA0000406
#define ALGO_VC_RET_INVALID_PITCHFACTOR     0xA0000407


typedef enum{
    IAA_VC_SAMPLE_RATE_8000 = 8000,
    IAA_VC_SAMPLE_RATE_16000 = 16000,
    IAA_VC_SAMPLE_RATE_32000 = 32000,
    IAA_VC_SAMPLE_RATE_48000 = 48000,
}AudioVcSampleRate_e;


typedef struct{
    int sampleRate;
    int bitWidth;
}AudioVcInit_t;


typedef struct{
    float speedFactor;
    float pitchFactor;
}AudioVcConfig_t;


typedef void *VC_HANDLE;
int IaaVc_GetBufferSize();
VC_HANDLE IaaVc_Init(char* workBufAddress, AudioVcInit_t *vcInit);
int IaaVc_SetConfig(VC_HANDLE handle, AudioVcConfig_t *vcConfig);
int IaaVc_GetConfig(VC_HANDLE handle);
int IaaVc_GetInputSamples(VC_HANDLE handle, int *samples);
int IaaVc_Run(VC_HANDLE handle, short *data, int *length);
int IaaVc_Free(VC_HANDLE handle);
int IaaVc_setCallbackFunc(int(*log)(const char *szFmt, ...),
                         int(*envSet)(char *key, char *par),
                         int(*envGetString)(char *var, char *buf, unsigned int size),
                         int(*envSave)(void),
                         int(*readUuid)(unsigned long long *u64Uuid));

#ifdef __cplusplus
}
#endif

#endif /* __AUDIOVCPROCESS_H__ */
