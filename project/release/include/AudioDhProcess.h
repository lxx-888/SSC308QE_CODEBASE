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
//  File        : AudioDhProcess.h
//  Description : Audio De-howling function
//  Author      : Tzuyun.huang
//  Revision    : 1.0
//
//==============================================================================
#ifndef AUDIODHPROCESS_H_
#define AUDIODHPROCESS_H_
typedef void* DH_HANDLE;
//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================

#define DH_API_VERSION                 {'1','2'}

//==============================================================================
//
//                              ENUMERATIONS
//
//==============================================================================

typedef enum {
    IAA_DH_SAMPLE_RATE_8000  =  8000 ,
    IAA_DH_SAMPLE_RATE_16000 = 16000 ,
    IAA_DH_SAMPLE_RATE_48000 = 48000 ,
}IAA_DH_SAMPLE_RATE;


/*DH config structure*/
typedef struct{
    unsigned int dh_enable;
    unsigned int lower_freq;
    int PTPR_thres;
    int PAPR_thres;
    int PHPR_thres;
    int suppression_thres;
    int supbins;
    int excess_supbins_time;
    int within_supbins_time;
}AudioDhConfig;

/*DH init structure*/
typedef struct {
    unsigned int point_number;
    unsigned int channel;
    IAA_DH_SAMPLE_RATE sample_rate;
}AudioDhInit;

typedef enum {
    ALGO_DH_RET_SUCCESS = 0x00000000,
    ALGO_DH_RET_INVALID_CONFIG = 0x10000701,
    ALGO_DH_RET_INVALID_HANDLE = 0x10000702,
    ALGO_DH_RET_INVALID_SAMPLERATE = 0x10000703,
    ALGO_DH_RET_INVALID_POINTNUMBER = 0x10000704,
    ALGO_DH_RET_INVALID_CHANNEL = 0x10000705,
    ALGO_DH_RET_INVALID_ENABLE = 0x10000706,
    ALGO_DH_RET_INVALID_LOWERFREQ = 0x10000707,
    ALGO_DH_RET_INVALID_PTPR = 0x10000708,
    ALGO_DH_RET_INVALID_PAPR = 0x10000709,
    ALGO_DH_RET_INVALID_PHPR = 0x10000710,
    ALGO_DH_RET_INVALID_SUPTHRESHLOD = 0x10000711,
    ALGO_DH_RET_INVALID_SUPBINS = 0x10000712,
    ALGO_DH_RET_INVALID_EXCESS_SUPBINS_TIME = 0x10000713,
    ALGO_DH_RET_INVALID_WITHIN_SUPBINS_TIME = 0x10000714,
    ALGO_DH_RET_INVALID_CALLING = 0x10000715,
    ALGO_DH_RET_API_CONFLICT = 0x10000716,
    ALGO_DH_RET_FAILED = 0x10000717
} ALGO_DH_RET;


//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================
unsigned int IaaDh_GetBufferSize(void);
DH_HANDLE IaaDh_Init(char* working_buffer_address, AudioDhInit *eq_init);
ALGO_DH_RET IaaDh_Config(DH_HANDLE handle, AudioDhConfig *eq_config);
ALGO_DH_RET IaaDh_SetLatency(DH_HANDLE handle, int latency_ms);
ALGO_DH_RET IaaDh_Run(DH_HANDLE handle, unsigned char* lower_vol, short* pss_audio_in);
ALGO_DH_RET IaaDh_Free(DH_HANDLE handle);
DH_HANDLE IaaDh_Reset(char* working_buffer_address, AudioDhInit *dh_init);
ALGO_DH_RET IaaDh_GetConfig(DH_HANDLE handle, AudioDhInit *dh_init, AudioDhConfig *dh_config);
ALGO_DH_RET IaaDh_GetAPIVersion(unsigned short* major, unsigned short* minor);
unsigned int IaaDh_GetJsonFileSize(char* jsonfile);
ALGO_DH_RET IaaDh_InitReadFromJson(AudioDhInit* dh_init, char* jsonBuffer, char* jsonfile, unsigned int buffSize);
ALGO_DH_RET IaaDh_ConfigReadFromJson(AudioDhConfig* dh_config, char* jsonBuffer, char* jsonfile, unsigned int buffSize);
#endif
// #ifndef _AUDIODHPROCESS_H_
