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
//  File        : AudioEqProcess.h
//  Description : Audio Noise reduction function
//  Author      : Ray-BR.chen
//  Revision    : 1.0
//
//==============================================================================

#ifdef __cplusplus
extern "C" {
#endif


#ifndef AUDIOEQPROCESS_H_
#define AUDIOEQPROCESS_H_
typedef void* EQ_HANDLE;
//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================

#define _EQ_BAND_NUM                    (129)
#define API_VERSION                      {'1','5'}
//==============================================================================
//
//                              ENUMERATIONS
//
//==============================================================================
typedef enum {
    ALGO_EQ_RET_SUCCESS,
    ALGO_EQ_RET_INIT_ERROR,
    ALGO_EQ_RET_INVALID_HANDLE,
    ALGO_EQ_RET_INVALID_SAMPLE_RATE,
    ALGO_EQ_RET_INVALID_POINT_NUMBER,
    ALGO_EQ_RET_INVALID_CHANNEL,
    ALGO_EQ_RET_INVALID_ENABLE,
    ALGO_EQ_RET_INVALID_MODE,
    ALGO_EQ_RET_INVALID_TABLE,
    ALGO_HPF_RET_INVALID_ENABLE,
    ALGO_HPF_RET_INVALID_MODE,
    ALGO_HPF_RET_INVALID_TABLE,
    ALGO_EQ_RET_API_CONFLICT,
    ALGO_EQ_RET_INVALID_CALLING,
    ALGO_EQ_RET_FAILED
}ALGO_EQ_RET;

typedef enum {
    IAA_EQ_SAMPLE_RATE_8000  =  8000 ,
    IAA_EQ_SAMPLE_RATE_16000 = 16000 ,
    IAA_EQ_SAMPLE_RATE_32000 = 32000 ,
    IAA_EQ_SAMPLE_RATE_48000 = 48000 ,
}IAA_EQ_SAMPLE_RATE;

typedef enum {
    AUDIO_HPF_FREQ_80 , /* 80Hz*/
    AUDIO_HPF_FREQ_120, /*120Hz*/
    AUDIO_HPF_FREQ_150, /*150Hz*/
    AUDIO_HPF_FREQ_BUTT,
}IAA_HPF_FREQ;

typedef enum {
    EQ_NOTCH_FILTER,
    EQ_HIGH_PASS_FILTER,
    EQ_LOW_PASS_FILTER
}EQ_FILTER_TYPE;

/*EQ config structure*/
typedef struct{
    unsigned int eq_enable;
    unsigned int user_mode;
    short eq_gain_db[_EQ_BAND_NUM];
}AudioEqConfig;

/*HPF config structure*/
typedef struct{
    unsigned int hpf_enable;
    unsigned int user_mode;
    IAA_HPF_FREQ cutoff_frequency;
}AudioHpfConfig;


typedef struct{
    EQ_FILTER_TYPE type;
    int f0;
    int q_factor;
    int gain;
}AudioFilterConfig;

/*ANR init structure*/
typedef struct {
    unsigned int point_number;
    unsigned int channel;
    IAA_EQ_SAMPLE_RATE sample_rate;
}AudioEqInit;

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================
unsigned int IaaEq_GetBufferSize(void);
EQ_HANDLE IaaEq_Init(char* working_buffer_address, AudioEqInit *eq_init);
ALGO_EQ_RET IaaEq_Config(EQ_HANDLE handle, AudioHpfConfig *hpf_config, AudioEqConfig *eq_config);
ALGO_EQ_RET IaaEq_Run(EQ_HANDLE handle, short* pss_audio_in);
ALGO_EQ_RET IaaEq_Free(EQ_HANDLE handle);
EQ_HANDLE IaaEq_Reset(char* working_buffer_address, AudioEqInit *eq_init);
ALGO_EQ_RET IaaEq_GetConfig(EQ_HANDLE handle, AudioEqInit *eq_init, AudioHpfConfig *hpf_config, AudioEqConfig *eq_config);
ALGO_EQ_RET IaaEq_FilterDesign(EQ_HANDLE handle, int filter_cnt, AudioFilterConfig* filter_config);
ALGO_EQ_RET IaaEq_FilterApply(EQ_HANDLE handle, short* pss_audio_in);
ALGO_EQ_RET IaaEq_FilterFreqz(EQ_HANDLE handle, float* frequency_response);
ALGO_EQ_RET IaaEq_SetHandleId(EQ_HANDLE handle, int id);
ALGO_EQ_RET IaaEq_EnableComfortNoise(EQ_HANDLE handle, int enable, int dB);
unsigned int IaaEq_GetJsonFileSize(char* jsonfile);
ALGO_EQ_RET IaaEq_ReadJson(EQ_HANDLE handle, char* jsonBuffer, char* jsonfile, unsigned int buffSize);
ALGO_EQ_RET IaaEq_InitReadFromJson(AudioEqInit *eq_init, char* jsonBuffer, char* jsonfile, unsigned int buffSize);
ALGO_EQ_RET IaaEq_ConfigReadFromJson(AudioEqConfig *eq_config, AudioHpfConfig *hpf_config, char* jsonBuffer, char* jsonfile, unsigned int buffSize);
ALGO_EQ_RET IaaEq_FilterDesignReadFromJson(EQ_HANDLE handle, char* jsonBuffer, char* jsonfile, unsigned int buffSize);
#endif// #ifndef _AUDIOEQPROCESS_H_


#ifdef __cplusplus
}
#endif


