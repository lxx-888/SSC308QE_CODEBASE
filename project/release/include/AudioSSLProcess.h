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


#ifndef __AUDIOSSLPROCESS_H__
#define __AUDIOSSLPROCESS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define ALGO_SSL_RET_SUCCESS                0x00000000
#define ALGO_SSL_RET_INVALID_LICENSE        0x20000401
#define ALGO_SSL_RET_INVALID_HANDLE         0x20000402
#define ALGO_SSL_RET_INVALID_SAMPLERATE     0x20000403
#define ALGO_SSL_RET_INVALID_BITWIDTH       0x20000404
#define ALGO_SSL_RET_INVALID_MIC_COORDINATE 0x20000405
#define ALGO_SSL_RET_INVALID_VAD_THRESHOLD  0x20000406
#define ALGO_SSL_RET_INVALID_FREQRANGE      0x20000407
#define ALGO_SSL_RET_INVALID_RESOLUTION     0x20000408
#define ALGO_SSL_RET_INVALID_INTERVAL       0x20000409
#define ALGO_SSL_RET_INVALID_TRACK_SPEED    0x20000410

#define SSL_MAX_MIC_CHANNEL_NUM             (6)
#define SSL_COORDINATE_DIM                  (2)

typedef enum
{
    IAA_SSL_SAMPLE_RATE_8000 = 8000,
    IAA_SSL_SAMPLE_RATE_16000 = 16000,
    IAA_SSL_SAMPLE_RATE_24000 = 24000,
    IAA_SSL_SAMPLE_RATE_32000 = 32000,
    IAA_SSL_SAMPLE_RATE_48000 = 48000,
}SslSampleRate_e;


typedef enum{
    IAA_SSL_LINEAR_ARRAY = 0,
    IAA_SSL_CIRCULAR_ARRAY,
}SslMicGeometry_e;


typedef enum
{
    IAA_SSL_TRACK_SPEED_SLOW = 0,
    IAA_SSL_TRACK_SPEED_NORM,
    IAA_SSL_TRACK_SPEED_FAST,
}SslTrackSpeed_e;

typedef struct{
    int sampleRate;
    int bitWidth;
    int micNum;
    float soundSpeed;
    float micCoordinate[SSL_MAX_MIC_CHANNEL_NUM][SSL_COORDINATE_DIM];
    SslMicGeometry_e micGeometry;
}AudioSslInit_t;


typedef struct{
    int vadThreshold;
    int lowFreq;
    int highFreq;
    int sslResolution;
    int interval;
    int trackSpeed;
}AudioSslConfig_t;


typedef void *SSL_HANDLE;
int IaaSsl_GetBufferSize();
SSL_HANDLE IaaSsl_Init(char* workBufAddress, AudioSslInit_t *sslInit);
int IaaSsl_SetConfig(SSL_HANDLE handle, AudioSslConfig_t *sslConfig);
int IaaSsl_GetConfig(SSL_HANDLE handle);
int IaaSsl_GetInputSamples(SSL_HANDLE handle, int *samples);
int IaaSsl_Run(SSL_HANDLE handle, const short *data, int *doa);
int IaaSsl_Free(SSL_HANDLE handle);
int IaaSsl_setCallbackFunc(int(*log)(const char *szFmt, ...),
                         int(*envSet)(char *key, char *par),
                         int(*envGetString)(char *var, char *buf, unsigned int size),
                         int(*envSave)(void),
                         int(*readUuid)(unsigned long long *u64Uuid));


#ifdef __cplusplus
}
#endif

#endif /* __AUDIOSSLPROCESS_H__ */
