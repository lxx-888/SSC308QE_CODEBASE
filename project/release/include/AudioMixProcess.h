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


#ifdef __cplusplus
extern "C"
{
#endif

#ifndef AUDIOPROCESSMIX_H_
#define AUDIOPROCESSMIX_H_
#define MIX_API_VERSION             {'2','2'}

#define MIX_MAX_CHN_NUM                     (8)

typedef enum{
    ALGO_MIX_RET_SUCCESS                =0x00000000,
    ALGO_MIX_RET_INVALID_LICENSE        =0x10000401,
    ALGO_MIX_RET_INVALID_HANDLE         =0x10000402,
    ALGO_MIX_RET_INVALID_SAMPLERATE     =0x10000403,
    ALGO_MIX_RET_INVALID_BITWIDTH       =0x10000404,
    ALGO_MIX_RET_INVALID_FRAMELENGTH    =0x10000405,
    ALGO_MIX_RET_INVALID_CHN_NUM        =0x10000406,
    ALGO_MIX_RET_INVALID_CHN_GAIN       =0x10000407,
    ALGO_MIX_RET_INVALID_STEPSIZE       =0x10000408,
    ALGO_MIX_RET_INVALID_INPUT_POINTER  =0x10000409,
    ALGO_MIX_RET_INIT_ERROR             =0x10000410,
    ALGO_MIX_RET_INVALID_CALLING        =0x10000411,
    ALGO_MIX_RET_INVALID_INPUT_TYPE     =0x10000412,
    ALGO_MIX_RET_INVALID_MODE           =0x10000413,
}ALGO_MIX_RET;


typedef enum
{
    MIX_INPUT_MONO       = 1,
    MIX_INPUT_STEREO     = 2,
}MIX_INPUT_TYPE;


typedef enum
{
    MIX_SAMPLE_TO_SAMPLE = 0,
    MIX_FRAME_TO_FRAME   = 1,
    MIX_DIRECT_ADD       = 2,
}MIX_MODE;

typedef struct
{
    int sampleRate;
    int bitWidth;
    int frameLength;
    int inputNumber;
    MIX_INPUT_TYPE inputType;
}AudioMixInit_t;

typedef struct
{
    int stepSize;
    int chnGain[MIX_MAX_CHN_NUM];
    MIX_MODE mode;
}AudioMixConfig_t;



typedef void *MIX_HANDLE;
unsigned int IaaMix_GetBufferSize();
MIX_HANDLE IaaMix_Init(char* workBufAddress, AudioMixInit_t *mixInit);
ALGO_MIX_RET IaaMix_SetConfig(MIX_HANDLE handle, AudioMixConfig_t *mixConfig);
ALGO_MIX_RET IaaMix_GetConfig(MIX_HANDLE handle, AudioMixConfig_t *mixConfig);
ALGO_MIX_RET IaaMix_Run(MIX_HANDLE handle, short *input, short *output);
ALGO_MIX_RET IaaMix_Free(MIX_HANDLE handle);
MIX_HANDLE IaaMix_Reset(char* workBufAddress, AudioMixInit_t *mixInit);


#endif /* AUDIOPROCESSMIX_H_ */

#ifdef __cplusplus
}
#endif
