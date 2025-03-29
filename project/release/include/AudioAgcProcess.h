/* SigmaStar trade secret */
/* Copyright (c) [2019~2022] SigmaStar Technology.
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

//==============================================================================
//
//  File        : AudioAnrProcess.h
//  Description : Audio Noise reduction function
//  Author      : Ray-BR.chen
//  Revision    : 1.0
//
//==============================================================================

#ifdef __cplusplus
extern "C" {
#endif

#ifndef AUDIOAGCPROCESS_H_
#define AUDIOAGCPROCESS_H_
typedef void* AGC_HANDLE;
//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================
#define _AGC_CR_NUM                      (7)
#define _AGC_FREQ_BAND_NUM                (3)
#define API_VERSION                      {'1','7'}
//==============================================================================
//
//                              ENUMERATIONS
//
//==============================================================================

typedef enum {
    ALGO_AGC_RET_SUCCESS,
    ALGO_AGC_RET_INIT_ERROR,
    ALGO_AGC_RET_INVALID_HANDLE,
    ALGO_AGC_RET_INVALID_SAMPLE_RATE,
    ALGO_AGC_RET_INVALID_POINT_NUMBER,
    ALGO_AGC_RET_INVALID_CHANNEL,
    ALGO_AGC_RET_INVALID_ENABLE,
    ALGO_AGC_RET_INVALID_MODE,
    ALGO_AGC_RET_INVALID_COMPRESSION_RATIO,
    ALGO_AGC_RET_INVALID_DROP_GAIN_MAX,
    ALGO_AGC_RET_INVALID_GAIN_STEP,
    ALGO_AGC_RET_INVALID_RELEASE_TIME,
    ALGO_AGC_RET_INVALID_ATTACK_TIME,
    ALGO_AGC_RET_INVALID_NOISE_GATE,
    ALGO_AGC_RET_INVALID_NOISE_ATTENU,
    ALGO_AGC_RET_INVALID_DROP_GAIN_LEVEL,
    ALGO_AGC_RET_INVALID_GAIN_INFO,
    ALGO_AGC_RET_API_CONFLICT,
    ALGO_AGC_RET_INVALID_CALLING,
    ALGO_AGC_RET_FAILED,
}ALGO_AGC_RET;

typedef enum {
    IAA_AGC_SAMPLE_RATE_8000  =  8000 ,
    IAA_AGC_SAMPLE_RATE_16000 = 16000 ,
    IAA_AGC_SAMPLE_RATE_32000 = 32000 ,
    IAA_AGC_SAMPLE_RATE_48000 = 48000,
}IAA_AGC_SAMPLE_RATE;

/*AGC gain info*/
typedef struct
{
    int gain_max;  //gain maximum
    int gain_min;  //gain minimum
    int gain_init; //default gain (initial gain)
}AgcGainInfo;

/*AGC config structure*/
typedef struct
{
    unsigned int agc_enable;
    unsigned int user_mode;

    //gain setting
    AgcGainInfo gain_info;
    unsigned int drop_gain_max;

    //attack time, release time
    unsigned int attack_time;
    unsigned int release_time;

    //target level
    short compression_ratio_input[_AGC_CR_NUM];
    short compression_ratio_output[_AGC_CR_NUM];
    int drop_gain_threshold;

    // noise gate
    int noise_gate_db;
    unsigned int noise_gate_attenuation_db;

    //Put Last
    unsigned int gain_step;

}AudioAgcConfig;

/*ANR init structure*/
typedef struct {
    unsigned int point_number;
    unsigned int channel;
    IAA_AGC_SAMPLE_RATE sample_rate;
}AudioAgcInit;

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================
unsigned int IaaAgc_GetBufferSize(void);
AGC_HANDLE IaaAgc_Init(char* working_buffer_address, AudioAgcInit *agc_init);
ALGO_AGC_RET IaaAgc_Config(AGC_HANDLE handle, AudioAgcConfig *agc_config);
ALGO_AGC_RET IaaAgc_Run(AGC_HANDLE handle, short* pss_audio_in);
ALGO_AGC_RET IaaAgc_Free(AGC_HANDLE handle);
AGC_HANDLE IaaAgc_Reset(char* working_buffer_address, AudioAgcInit *anr_init);
ALGO_AGC_RET IaaAgc_GetConfig(AGC_HANDLE handle, AudioAgcInit *agc_init, AudioAgcConfig *agc_config);
ALGO_AGC_RET IaaAgc_SetAgcFreqBand(AGC_HANDLE handle, int* frequency_band);
ALGO_AGC_RET IaaAgc_SetLowFreqCompressionRatioCurve(AGC_HANDLE handle, int *CompressRatioInput, int *CompressRatioOutput);
ALGO_AGC_RET IaaAgc_SetMidFreqCompressionRatioCurve(AGC_HANDLE handle, int *CompressRatioInput, int *CompressRatioOutput);
ALGO_AGC_RET IaaAgc_SetHighFreqCompressionRatioCurve(AGC_HANDLE handle, int *CompressRatioInput, int *CompressRatioOutput);
ALGO_AGC_RET IaaAgc_SetCompressionRatioCurve(AGC_HANDLE handle, short *CompressRatioInput, short *CompressRatioOutput, int curve_len);
ALGO_AGC_RET IaaAgc_SetHandleId(AGC_HANDLE handle, int id);
ALGO_AGC_RET IaaAgc_EnableComfortNoise(AGC_HANDLE handle, int enable, int dB);
unsigned int IaaAgc_GetJsonFileSize(char* jsonfile);
ALGO_AGC_RET IaaAgc_ReadJson(AGC_HANDLE handle, char* jsonBuffer, char* jsonfile, unsigned int buffSize);
ALGO_AGC_RET IaaAgc_InitReadFromJson(AudioAgcInit *agc_init, char* jsonBuffer, char* jsonfile, unsigned int buffSize);
ALGO_AGC_RET IaaAgc_ConfigReadFromJson(AudioAgcConfig *agc_config, char* jsonBuffer, char* jsonfile, unsigned int buffSize);
ALGO_AGC_RET IaaAgc_SetAgcReadFromJson(int *freqBand, int *compressionRatioArrayLowInput, int *compressionRatioArrayLowOutput, int *compressionRatioArrayMidInput, int *compressionRatioArrayMidOutput, int *compressionRatioArrayHighInput, int *compressionRatioArrayHighOutput, char* jsonBuffer, char* jsonfile, unsigned int buffSize);
#endif
// #ifndef _AUDIOAGCPROCESS_H_
#ifdef __cplusplus
}
#endif
