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
//==============================================================================
//
//  File        : AudioAnrProcess.h
//  Description : Audio Noise reduction function
//  Author      : Ray-BR.chen
//  Revision    : 1.0
//
//==============================================================================
#ifndef AUDIOANRPROCESS_H_
#define AUDIOANRPROCESS_H_
typedef void* ANR_HANDLE;

#define _NR_BAND_NUM                    (7)
#define ANR_API_VERSION                  	{'1','2'}
//==============================================================================
//
//                              ENUMERATIONS
//
//==============================================================================

typedef enum {
	ALGO_ANR_RET_SUCCESS,
	ALGO_ANR_RET_INIT_ERROR,
	ALGO_ANR_RET_INVALID_HANDLE,
	ALGO_ANR_RET_INVALID_SAMPLE_RATE,
	ALGO_ANR_RET_INVALID_POINT_NUMBER,
	ALGO_ANR_RET_INVALID_CHANNEL,
	ALGO_ANR_RET_INVALID_ENABLE,
	ALGO_ANR_RET_INVALID_MODE,
	ALGO_ANR_RET_INVALID_INTENSITY,
	ALGO_ANR_RET_INVALID_SMOOTH_LEVEL,
	ALGO_ANR_RET_INVALID_CONVERGE_SPEED,
	ALGO_ANR_RET_API_CONFLICT,
	ALGO_ANR_RET_INVALID_CALLING,
	ALGO_ANR_RET_INVALID_FILTER_MODE,
	ALGO_ANR_RET_FAILED
}ALGO_ANR_RET;

typedef enum {
    IAA_ANR_SAMPLE_RATE_8000  =  8000 ,
    IAA_ANR_SAMPLE_RATE_16000 = 16000 ,
	IAA_ANR_SAMPLE_RATE_48000 = 48000 ,
}IAA_ANR_SAMPLE_RATE;

typedef enum {
    NR_SPEED_LOW,
    NR_SPEED_MID,
    NR_SPEED_HIGH
} NR_CONVERGE_SPEED;

/*ANR config structure*/

typedef struct{
    unsigned int anr_enable;
    unsigned int user_mode;
    int anr_intensity_band[_NR_BAND_NUM-1];
    int anr_intensity[_NR_BAND_NUM];
    unsigned int anr_smooth_level; //range 0-10
    NR_CONVERGE_SPEED anr_converge_speed; //0 1 2 higher the speed more fast
    int anr_filter_mode; // 0~5
}AudioAnrConfig;

/*ANR init structure*/
typedef struct {
    unsigned int point_number;
    unsigned int channel;
    IAA_ANR_SAMPLE_RATE sample_rate;
}AudioAnrInit;

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================
unsigned int IaaAnr_GetBufferSize(void);
ANR_HANDLE IaaAnr_Init(char* working_buffer_address, AudioAnrInit *anr_init);
ALGO_ANR_RET IaaAnr_Config(ANR_HANDLE handle, AudioAnrConfig *anr_config);
ALGO_ANR_RET IaaAnr_Run(ANR_HANDLE, short* pss_audio_in);
ALGO_ANR_RET IaaAnr_Free(ANR_HANDLE);
ANR_HANDLE IaaAnr_Reset(char* working_buffer_address, AudioAnrInit *anr_init);
ALGO_ANR_RET IaaAnr_GetConfig(ANR_HANDLE handle, AudioAnrInit *anr_init, AudioAnrConfig *anr_config);
#endif
// #ifndef _AUDIOANRPROCESS_H_
