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

/*
 * AudioVadProcess.h
 *
 *  Created on: 2019¦~4¤ë24¤é
 *      Author: Ray-BR.Chen
 */
#ifdef __cplusplus
extern "C" {
#endif
#ifndef AUDIOVADPROCESS_H_
#define AUDIOVADPROCESS_H_

typedef void* VAD_HANDLE;

//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================


//==============================================================================
//
//                              ENUMERATIONS
//
//==============================================================================

typedef enum {
    IAA_VAD_SAMPLE_RATE_8000  =  8000 ,
    IAA_VAD_SAMPLE_RATE_16000 = 16000 ,
    IAA_VAD_SAMPLE_RATE_48000 = 48000
}IAA_VAD_SAMPLE_RATE;

typedef enum {
    VAD_SEN_LOW,
    VAD_SEN_MID,
    VAD_SEN_HIGH
} VadSensitivity;

typedef struct {
    unsigned int point_number;
    unsigned int channel;
    IAA_VAD_SAMPLE_RATE sample_rate;
}VadInit;


typedef struct {
    unsigned int vote_frame;
    VadSensitivity sensitivity;
}VadConfig;

unsigned int IaaVad_GetBufferSize();
VAD_HANDLE IaaVad_Init(char* const working_buffer_address, VadInit *vad_init);
int IaaVad_Run(VAD_HANDLE,short* pss_audio_in);
int IaaVad_Config(VAD_HANDLE, VadConfig *vad_config);
int IaaVad_SetMode(VAD_HANDLE, int mode);
int IaaVad_Free(VAD_HANDLE handle);

#endif /* AUDIOVADPROCESS_H_ */
#ifdef __cplusplus
}
#endif
