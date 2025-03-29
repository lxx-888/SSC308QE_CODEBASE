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
#ifndef _AUDIOSSLPROCESS_H_
#define _AUDIOSSLPROCESS_H_
#define SSL_API_VERSION             {'1','4'}

typedef struct
{
	unsigned int point_number;
	unsigned int sample_rate;
	float mic_distance;
	unsigned int bf_mode;
	int channel;
}AudioSslInit;


typedef struct
{
	unsigned int temperature;
	int noise_gate_dbfs;
	int direction_frame_num;
}AudioSslConfig;

typedef enum {
	ALGO_SSL_RET_SUCCESS                     = 0x00000000,
	ALGO_SSL_RET_INIT_ERROR                  = 0x10000101,
	ALGO_SSL_RET_INVALID_CONFIG				 = 0x10000102,
	ALGO_SSL_RET_INVALID_HANDLE              = 0x10000103,
	ALGO_SSL_RET_INVALID_SAMPLERATE          = 0x10000104,
	ALGO_SSL_RET_INVALID_POINTNUMBER 		 = 0x10000105,
	ALGO_SSL_RET_INVALID_BFMODE		 		 = 0x10000106,
	ALGO_SSL_RET_DELAY_SAMPLE_TOO_LARGE		 = 0x10000107,
	ALGO_SSL_RET_INVALID_CALLING			 = 0x10000108,
	ALGO_SSL_RET_API_CONFLICT				 = 0x10000109,
	ALGO_SSL_RET_INVALID_CHANNEL             = 0x10000110,
	ALGO_SSL_RET_INVALID_GEOMETRY_TYPE       = 0x10000111,
	ALGO_SSL_RET_INVALID_ARRAY_TYPE          = 0x10000112,
	ALGO_SSL_RET_BELOW_NOISE_GATE            = 0x10000113,
	ALGO_SSL_RET_RESULT_UNRELIABLE           = 0x10000114,
	ALGO_SSL_RET_INVALID_MODE                = 0x10000115,
	ALGO_SSL_RET_INVALID_CNN                 = 0x10000116,
	ALGO_SSL_RET_INVALID_SETCALLBACK         = 0x10000117,
	ALGO_SSL_RET_FAILED                      = 0x10000118,
	ALGO_SSL_RET_INVALID_GAIN                = 0x10000119,
} ALGO_SSL_RET;

typedef void* SSL_HANDLE;

unsigned int IaaSsl_GetBufferSize(void);
SSL_HANDLE IaaSsl_Init(char* working_buffer,AudioSslInit* ssl_init);
ALGO_SSL_RET IaaSsl_Config(SSL_HANDLE handle,AudioSslConfig* ssl_config);
ALGO_SSL_RET IaaSsl_Get_Config(SSL_HANDLE handle,AudioSslConfig *ssl_config);
ALGO_SSL_RET IaaSsl_Run(SSL_HANDLE handle,short* microphone_input,int *delay_sample);
ALGO_SSL_RET IaaSsl_Get_Direction(SSL_HANDLE handle,int* direction);
SSL_HANDLE IaaSsl_Reset(SSL_HANDLE working_buffer,AudioSslInit* ssl_init);
ALGO_SSL_RET IaaSsl_Free(SSL_HANDLE handle);
ALGO_SSL_RET IaaSsl_Set_Shape(SSL_HANDLE handle,int shape);
ALGO_SSL_RET IaaSsl_Cal_Params(SSL_HANDLE handle);
ALGO_SSL_RET IaaSsl_Reset_Mapping(SSL_HANDLE handle);
ALGO_SSL_RET IaaSsl_SetMode(SSL_HANDLE handle, int mode,unsigned int angular_distance,unsigned int convidence);
ALGO_SSL_RET IaaSsl_SetCallbackFunction(int(*log)(const char *szFmt, ...),int(*envSet)(char *key, char *par),int(*envGetString)(char *var, char *buf, unsigned int size),int(*envSave)(void),int(*readUuid)(unsigned long long *u64Uuid));
ALGO_SSL_RET IaaSsl_ApplyDigitalGain(SSL_HANDLE handle,short* microphone_input,int gain);
ALGO_SSL_RET IaaSsl_GetAPIVersion(unsigned short* major, unsigned short* minor);
#endif
#ifdef __cplusplus
}
#endif
