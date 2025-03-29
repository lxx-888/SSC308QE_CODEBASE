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
#ifndef __MI_AI_DATATYPE_H__
#define __MI_AI_DATATYPE_H__

#include "mi_common_datatype.h"
#include "mi_aio_datatype.h"

#ifdef __cplusplus
extern "C"
{
#endif

//=============================================================================
// Include files
//=============================================================================

//=============================================================================
// Extern definition
//=============================================================================

//=============================================================================
// Macro definition
//=============================================================================
#define MI_AI_ERR_INVALID_DEVID    (MI_DEF_ERR(E_MI_MODULE_ID_AI, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_DEVID))
#define MI_AI_ERR_INVALID_CHNGRPID (MI_DEF_ERR(E_MI_MODULE_ID_AI, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_CHNID))
#define MI_AI_ERR_ILLEGAL_PARAM    (MI_DEF_ERR(E_MI_MODULE_ID_AI, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_ILLEGAL_PARAM))
#define MI_AI_ERR_NOT_ENABLED      (MI_DEF_ERR(E_MI_MODULE_ID_AI, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_ENABLE))
#define MI_AI_ERR_NULL_PTR         (MI_DEF_ERR(E_MI_MODULE_ID_AI, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NULL_PTR))
#define MI_AI_ERR_NOT_CONFIG       (MI_DEF_ERR(E_MI_MODULE_ID_AI, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_CONFIG))
#define MI_AI_ERR_NOT_SUPPORT      (MI_DEF_ERR(E_MI_MODULE_ID_AI, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_SUPPORT))
#define MI_AI_ERR_NOT_PERM         (MI_DEF_ERR(E_MI_MODULE_ID_AI, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_PERM))
#define MI_AI_ERR_NOMEM            (MI_DEF_ERR(E_MI_MODULE_ID_AI, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOMEM))
#define MI_AI_ERR_NOBUF            (MI_DEF_ERR(E_MI_MODULE_ID_AI, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOBUF))
#define MI_AI_ERR_BUF_EMPTY        (MI_DEF_ERR(E_MI_MODULE_ID_AI, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUF_EMPTY))
#define MI_AI_ERR_BUF_FULL         (MI_DEF_ERR(E_MI_MODULE_ID_AI, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUF_FULL))
#define MI_AI_ERR_SYS_NOTREADY     (MI_DEF_ERR(E_MI_MODULE_ID_AI, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SYS_NOTREADY))
#define MI_AI_ERR_BUSY             (MI_DEF_ERR(E_MI_MODULE_ID_AI, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUSY))

#define MI_AI_MAX_CHN_NUM (16) /* Depend on multi-channel hardware ability */

#define MI_AI_DEV_1 (0) /* DMA Writer 1 */
#define MI_AI_DEV_2 (1) /* DMA Writer 2 */
#define MI_AI_DEV_3 (2) /* DMA Writer 3 */
#define MI_AI_DEV_4 (3) /* DMA Writer 4 */
#define MI_AI_DEV_5 (4) /* DMA Writer 5 */

#define MI_AI_ECHO_CHN_GROUP_ID (0xFF) /* Only used to set the echo gain */

//=============================================================================
// Data type definition
//=============================================================================
typedef struct MI_AI_Attr_s
{
    MI_AUDIO_Format_e     enFormat;      /* The sample format of a PCM */
    MI_AUDIO_SoundMode_e  enSoundMode;   /* The number of channels in a channel group */
    MI_AUDIO_SampleRate_e enSampleRate;  /* The number of frames per second */
    MI_U32                u32PeriodSize; /* The number of frames in a period */
    MI_BOOL               bInterleaved;  /* Whether the received data is interleaved */
} MI_AI_Attr_t;

typedef enum
{
    E_MI_AI_IF_NONE      = 0,
    E_MI_AI_IF_ADC_AB    = 1,  /* ADC A 0 channel and B 0 channel */
    E_MI_AI_IF_ADC_CD    = 2,  /* ADC C 0 channel and D 0 channel */
    E_MI_AI_IF_DMIC_A_01 = 3,  /* DMIC A 0 and 1 channel */
    E_MI_AI_IF_DMIC_A_23 = 4,  /* DMIC A 2 and 3 channel */
    E_MI_AI_IF_I2S_A_01  = 5,  /* I2S A 0 and 1 channel */
    E_MI_AI_IF_I2S_A_23  = 6,  /* I2S A 2 and 3 channel */
    E_MI_AI_IF_I2S_A_45  = 7,  /* I2S A 4 and 5 channel */
    E_MI_AI_IF_I2S_A_67  = 8,  /* I2S A 6 and 7 channel */
    E_MI_AI_IF_I2S_A_89  = 9,  /* I2S A 8 and 9 channel */
    E_MI_AI_IF_I2S_A_ab  = 10, /* I2S A 10 and 11 channel */
    E_MI_AI_IF_I2S_A_cd  = 11, /* I2S A 12 and 13 channel */
    E_MI_AI_IF_I2S_A_ef  = 12, /* I2S A 14 and 15 channel */
    E_MI_AI_IF_I2S_B_01  = 13, /* I2S B 0 and 1 channel */
    E_MI_AI_IF_I2S_B_23  = 14, /* I2S B 2 and 3 channel */
    E_MI_AI_IF_I2S_B_45  = 15, /* I2S B 4 and 5 channel */
    E_MI_AI_IF_I2S_B_67  = 16, /* I2S B 6 and 7 channel */
    E_MI_AI_IF_I2S_B_89  = 17, /* I2S B 8 and 9 channel */
    E_MI_AI_IF_I2S_B_ab  = 18, /* I2S B 10 and 11 channel */
    E_MI_AI_IF_I2S_B_cd  = 19, /* I2S B 12 and 13 channel */
    E_MI_AI_IF_I2S_B_ef  = 20, /* I2S B 14 and 15 channel */
    E_MI_AI_IF_I2S_C_01  = 21, /* I2S C 0 and 1 channel */
    E_MI_AI_IF_I2S_C_23  = 22, /* I2S C 2 and 3 channel */
    E_MI_AI_IF_I2S_C_45  = 23, /* I2S C 4 and 5 channel */
    E_MI_AI_IF_I2S_C_67  = 24, /* I2S C 6 and 7 channel */
    E_MI_AI_IF_I2S_C_89  = 25, /* I2S C 8 and 9 channel */
    E_MI_AI_IF_I2S_C_ab  = 26, /* I2S C 10 and 11 channel */
    E_MI_AI_IF_I2S_C_cd  = 27, /* I2S C 12 and 13 channel */
    E_MI_AI_IF_I2S_C_ef  = 28, /* I2S C 14 and 15 channel */
    E_MI_AI_IF_I2S_D_01  = 29, /* I2S D 0 and 1 channel */
    E_MI_AI_IF_I2S_D_23  = 30, /* I2S D 2 and 3 channel */
    E_MI_AI_IF_I2S_D_45  = 31, /* I2S D 4 and 5 channel */
    E_MI_AI_IF_I2S_D_67  = 32, /* I2S D 6 and 7 channel */
    E_MI_AI_IF_I2S_D_89  = 33, /* I2S D 8 and 9 channel */
    E_MI_AI_IF_I2S_D_ab  = 34, /* I2S D 10 and 11 channel */
    E_MI_AI_IF_I2S_D_cd  = 35, /* I2S D 12 and 13 channel */
    E_MI_AI_IF_I2S_D_ef  = 36, /* I2S D 14 and 15 channel */
    E_MI_AI_IF_ECHO_A    = 37, /* Echo A left and right channel */
    E_MI_AI_IF_HDMI_A    = 38, /* HDMI(CEC) A left and right channel */
    E_MI_AI_IF_DMIC_A_45 = 39, /* DMIC A 4 and 5 channel */
    E_MI_AI_IF_MAX,

    /* Add new one below, and the order above os not allowed to change. */
} MI_AI_If_e;

typedef struct MI_AI_Data_s
{
    void *apvBuffer[MI_AI_MAX_CHN_NUM]; /* data address per channel group. if bInterleaved = true, only one data
                                           address */
    MI_U32 u32Byte[MI_AI_MAX_CHN_NUM];  /* data size per channel group */
    MI_U64 u64Pts;                      /* timestamp */
    MI_U64 u64Seq;                      /* data sequence per channel group */
} MI_AI_Data_t;

typedef struct MI_AI_InitParam_s
{
    MI_U32 u32DevId;
    MI_U8 *u8Data;
} MI_AI_InitParam_t;

//=============================================================================
// Variable definition
//=============================================================================

//=============================================================================
// Global function definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif //__MI_AI_DATATYPE_H__
