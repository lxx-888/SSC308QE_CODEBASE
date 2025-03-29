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
extern "C" {
#endif
#ifndef _AUDIOBFPROCESS_H_
#define _AUDIOBFPROCESS_H_
#define BF_API_VERSION             {'1','5'}

typedef struct
{
	unsigned int point_number;
	unsigned int sample_rate;
	float mic_distance; //cm
	unsigned int channel;
}AudioBfInit;

typedef struct
{
	unsigned int temperature;
	int noise_gate_dbfs;
	int noise_estimation;
	float output_gain;
	int vad_enable;
	int diagonal_loading;
}AudioBfConfig;

typedef enum
{
	BF_WORKFLOW_AFTER_APC = 0,
	BF_WORKFLOW_AFTER_BF = 1,
}BF_WORKFLOW;

typedef enum
{
	BF_CALIBRATION_DISABLE = 0,
	BF_CALIBRATION_ENABLE  = 1,
}BF_CALIBRATION;

typedef enum {
	ALGO_BF_RET_SUCCESS                     = 0x00000000,
	ALGO_BF_RET_INIT_ERROR                  = 0x10000301,
	ALGO_BF_RET_INVALID_CONFIG				= 0x10000302,
	ALGO_BF_RET_INVALID_HANDLE              = 0x10000303,
	ALGO_BF_RET_INVALID_NOISE_ESTIMATION	= 0x10000304,
	ALGO_BF_RET_INVALID_VAD_ENABLE			= 0x10000305,
	ALGO_BF_RET_INVALID_OUTPUT_GAIN			= 0x10000306,
	ALGO_BF_RET_INVALID_INPUT_POINTER		= 0x10000307,
	ALGO_BF_RET_INVALID_SAMPLERATE          = 0x10000308,
	ALGO_BF_RET_INVALID_POINTNUMBER 		= 0x10000309,
	ALGO_BF_RET_INVALID_CHANNEL 			= 0x10000310,
	ALGO_BF_RET_INVALID_CALLING			 	= 0x10000311,
	ALGO_BF_RET_API_CONFLICT				= 0x10000312,
	ALGO_BF_RET_INVALID_GEOMETRY_TYPE       = 0x10000313,
	ALGO_BF_RET_INVALID_MIC_DISTANCE        = 0x10000314,
	ALGO_BF_RET_INVALLD_DYNAMIC_BAND		= 0x10000315,
	ALGO_BF_RET_INVALID_SETMODE			= 0x10000316,
	ALGO_BF_RET_INVALID_GETPOSITION     = 0x10000317,
	ALGO_BF_RET_INVALID_SETINTENSITY    = 0x10000318,
	ALGO_BF_RET_INVALID_SETCALLBACK     = 0x10000319,
	ALGO_BF_RET_INVALID_SETPFFLOW		= 0x10000320,
	ALGO_BF_RET_INVALID_CALIBRATION		= 0x10000321,
} ALGO_BF_RET;

typedef void* BF_HANDLE;
unsigned int IaaBf_GetBufferSize(void);
BF_HANDLE IaaBf_Init(char* working_buffer,AudioBfInit* bf_init);
ALGO_BF_RET IaaBf_SetConfig(BF_HANDLE handle,AudioBfConfig* bf_config);
ALGO_BF_RET IaaBf_GetConfig(BF_HANDLE handle,AudioBfConfig* bf_config);
ALGO_BF_RET IaaBf_Run(BF_HANDLE handle,short* microphone_input,short* microphone_output,float* microphone_doa);
BF_HANDLE IaaBf_Reset(BF_HANDLE working_buffer,AudioBfInit* bf_init);
ALGO_BF_RET IaaBf_Free(BF_HANDLE handle);
ALGO_BF_RET IaaBf_SetShape(BF_HANDLE handle,int shape);
ALGO_BF_RET IaaBf_GetArrayPosition(BF_HANDLE handle,float* array_pos);
ALGO_BF_RET IaaBf_DynamicLoading(BF_HANDLE handle,int* band,int* dl_intensity,int is_dynamic);
ALGO_BF_RET IaaBf_SetMode(BF_HANDLE handle,float choose_doa,int mode);
ALGO_BF_RET IaaBf_SetAdaptiveIntensity(BF_HANDLE handle,int intensity);
ALGO_BF_RET IaaBf_SetArbitraryShape(BF_HANDLE handle,float* array_pos);
ALGO_BF_RET IaaBf_SetCallbackFunction(int(*log)(const char *szFmt, ...),int(*envSet)(char *key, char *par),int(*envGetString)(char *var, char *buf, unsigned int size),int(*envSave)(void),int(*readUuid)(unsigned long long *u64Uuid));
ALGO_BF_RET IaaBf_GetAPIVersion(unsigned short* major, unsigned short* minor);
ALGO_BF_RET IaaBf_SetPfFlow(BF_HANDLE handle, BF_WORKFLOW bf_workflow);
ALGO_BF_RET IaaBf_SetCalibration(BF_HANDLE handle, BF_CALIBRATION calibration_flag);
#endif
#ifdef __cplusplus
}
#endif
