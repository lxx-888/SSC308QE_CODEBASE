/*
 * hal_audio_os_api.h - Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */

#ifndef __HAL_AUD_OS_API_H__
#define __HAL_AUD_OS_API_H__

#if 1
//-----------------------------------------------------------------------------------------------------
// Debug Level
//-----------------------------------------------------------------------------------------------------
#define AUDIO_DBG_LEVEL_TEST     (AIO_BIT0)  //
#define AUDIO_DBG_LEVEL_DMA      (AIO_BIT1)  // debug DMA status
#define AUDIO_DBG_LEVEL_ATOP     (AIO_BIT2)  // debug ATOP
#define AUDIO_DBG_LEVEL_I2S      (AIO_BIT3)  // debug I2S
#define AUDIO_DBG_LEVEL_DMIC     (AIO_BIT4)  // debug DMIC
#define AUDIO_DBG_LEVEL_IRQ      (AIO_BIT5)  // debug irq status
#define AUDIO_DBG_LEVEL_DELAY    (AIO_BIT6)  // debug audio delay status
#define AUDIO_DBG_LEVEL_PATH     (AIO_BIT7)  // debug Indicates the creation status of a path
#define AUDIO_DBG_LEVEL_POWER    (AIO_BIT8)  // debug power
#define AUDIO_DBG_LEVEL_CLOCK    (AIO_BIT9)  // debug audio clk
#define AUDIO_DBG_LEVEL_DUMP_PCM (AIO_BIT10) // debug dump pcm
#define AUDIO_DBG_LEVEL_SPDIF    (AIO_BIT11) // debug spdif

#define PRINT_NONE   "\033[0m \n"
#define PRINT_RED    "\033[1;31m"
#define PRINT_YELLOW "\033[1;33m"
#define PRINT_GREEN  "\033[1;32m"

#define ERRMSG(_fmt, _args...) \
    CamOsPrintf(PRINT_RED "[ALSA ERROR][%s][%d]: "_fmt PRINT_NONE, __FUNCTION__, __LINE__, ##_args)

#define DBGMSG(dbglv, _fmt, _args...)                                                                               \
    do                                                                                                              \
    {                                                                                                               \
        if ((HalAudApiGetDtsValue(DEBUG_LEVEL) & (dbglv)) == (dbglv))                                               \
        {                                                                                                           \
            if ((dbglv)&AUDIO_DBG_LEVEL_TEST)                                                                       \
            {                                                                                                       \
                CamOsPrintf(PRINT_GREEN "[ALSA DEBUG][%s][%d]: " _fmt PRINT_NONE, __FUNCTION__, __LINE__, ##_args); \
            }                                                                                                       \
            else                                                                                                    \
            {                                                                                                       \
                CamOsPrintf(PRINT_YELLOW "[ALSA INFO][%s][%d]: "_fmt PRINT_NONE, __FUNCTION__, __LINE__, ##_args);  \
            }                                                                                                       \
        }                                                                                                           \
    } while (0)
#else
#define HAL_DEBUG_ON 0

#define ERRMSG(fmt, args...) CamOsPrintf("[AUDIO HAL]" fmt, ##args)

#if HAL_DEBUG_ON
#define DBGMSG(fmt, args...) CamOsPrintf("[AUDIO HAL DEBUG]" fmt, ##args)
#else
#define DBGMSG(fmt, args...)
#endif
#endif

#define GENERATE_CASE(ENUM_VALUE, ENUM_STRING) \
    case (ENUM_VALUE):                         \
    {                                          \
        return #ENUM_STRING;                   \
    }
#define GENERATE_CASE_DEF(ENUM_VALUE) GENERATE_CASE(ENUM_VALUE, ENUM_VALUE)

extern int  HalAudApiDtsInit(void);
extern int  HalAudApiAoAmpEnable(BOOL bEnable, S8 s8Ch);
extern int  HalAudApiAoSpdifEnable(CHIP_SWITCH_e enState);
extern BOOL _HalAudArcSetMute(AO_CH_e eAoCh, BOOL bEnable);
extern int  HalAudApiAoAmpStateGet(int *bEnable, U8 s8Ch);
extern int  HalAudApiGetIrqId(unsigned int *pIrqId);

typedef union DTS_CONFIG_VALUE
{
    U32 *values;
    U32  value;
} DTS_CONFIG_VALUE_un;

extern DTS_CONFIG_VALUE_un dts_cfg_value[DTS_CONFIG_KEY_TOTAL];
extern inline U32          HalAudApiGetDtsValue(DTS_CONFIG_KEY_e key)
{
    return dts_cfg_value[key].value;
}

extern inline U32 HalAudApiGetDtsValues(DTS_CONFIG_KEY_e key, int index)
{
    if (dts_cfg_value[key].values != NULL)
    {
        return dts_cfg_value[key].values[index];
    }

    return AIO_NG;
}

extern inline int HalAudApiSetDtsValues(DTS_CONFIG_KEY_e key, int index, int value)
{
    if (dts_cfg_value[key].values != NULL)
    {
        dts_cfg_value[key].values[index] = value;
        return AIO_OK;
    }

    return AIO_NG;
}
extern inline int HalAudApiSetDtsValue(DTS_CONFIG_KEY_e key, int value)
{
    dts_cfg_value[key].value = value;

    return AIO_OK;
}

extern inline void _UDelay(U32 u32MicroSeconds)
{
    CamOsUsDelay(u32MicroSeconds);
}

extern inline void _MSleep(U32 u32MilliSeconds)
{
    CamOsMsSleep(u32MilliSeconds);
}

extern inline AudRate_e HalAudApiRateToEnum(U32 rate)
{
    AudRate_e eRate = E_AUD_RATE_NULL;
    switch (rate)
    {
        case 8000:
            eRate = E_AUD_RATE_8K;
            break;
        case 11000:
        case 11025:
            eRate = E_AUD_RATE_11K;
            break;
        case 12000:
            eRate = E_AUD_RATE_12K;
            break;
        case 16000:
            eRate = E_AUD_RATE_16K;
            break;
        case 22000:
        case 22050:
            eRate = E_AUD_RATE_22K;
            break;
        case 24000:
            eRate = E_AUD_RATE_24K;
            break;
        case 32000:
            eRate = E_AUD_RATE_32K;
            break;
        case 44000:
        case 44100:
            eRate = E_AUD_RATE_44K;
            break;
        case 48000:
            eRate = E_AUD_RATE_48K;
            break;
        case 88000:
        case 88200:
            eRate = E_AUD_RATE_88K;
            break;
        case 96000:
            eRate = E_AUD_RATE_96K;
            break;
        case 192000:
            eRate = E_AUD_RATE_192K;
            break;
        default:
            // ERRMSG("rate %d Fail !\n", rate);
            break;
    }
    return eRate;
}

extern inline U32 HalAudApiEnumToRate(AudRate_e eRate)
{
    U32 rate = 0;
    switch (eRate)
    {
        case E_AUD_RATE_192K:
            rate = 192000;
            break;
        case E_AUD_RATE_96K:
            rate = 96000;
            break;
        case E_AUD_RATE_88K:
            rate = 88200;
            break;
        case E_AUD_RATE_48K:
            rate = 48000;
            break;
        case E_AUD_RATE_44K:
            rate = 44100;
            break;
        case E_AUD_RATE_32K:
            rate = 32000;
            break;
        case E_AUD_RATE_24K:
            rate = 24000;
            break;
        case E_AUD_RATE_22K:
            rate = 22000;
            break;
        case E_AUD_RATE_16K:
            rate = 16000;
            break;
        case E_AUD_RATE_12K:
            rate = 12000;
            break;
        case E_AUD_RATE_11K:
            rate = 11000;
            break;
        case E_AUD_RATE_8K:
            rate = 8000;
            break;
        default:
            ERRMSG("eRate %d Fail !\n", eRate);
            break;
    }
    return rate;
}

extern inline U32 HalAudApiEnumToBitWidth(AudBitWidth_e eI2sWidth)
{
    U32 width = 0;
    switch (eI2sWidth)
    {
        case E_AUD_BITWIDTH_16:
            width = 16;
            break;
        case E_AUD_BITWIDTH_24:
            width = 24;
            break;
        case E_AUD_BITWIDTH_32:
            width = 32;
            break;
        default:
            ERRMSG("eI2sWidth %d Fail !\n", eI2sWidth);
            break;
    }
    return width;
}

extern inline AudBitWidth_e HalAudApiBitWidthToEnum(U32 width)
{
    AudBitWidth_e enI2sWidth = E_AUD_BITWIDTH_NULL;
    switch (width)
    {
        case 16:
            enI2sWidth = E_AUD_BITWIDTH_16;
            break;
        case 24:
            enI2sWidth = E_AUD_BITWIDTH_24;
            break;
        case 32:
            enI2sWidth = E_AUD_BITWIDTH_32;
            break;
        default:
            ERRMSG("bitwidth[%d] error\n", width);
            break;
    }
    return enI2sWidth;
}

extern inline AudI2sMck_e HalAudApiMckToEnum(U32 nMckFreq)
{
    switch (nMckFreq)
    {
        case 12288000:
            return E_AUD_I2S_MCK_12_288M;
        case 16384000:
            return E_AUD_I2S_MCK_16_384M;
        case 18432000:
            return E_AUD_I2S_MCK_18_432M;
        case 22579200:
            return E_AUD_I2S_MCK_22_5792M;
        case 24576000:
            return E_AUD_I2S_MCK_24_576M;
        case 32768000:
            return E_AUD_I2S_MCK_32_768M;
        case 36864000:
            return E_AUD_I2S_MCK_36_864M;
        case 49152000:
            return E_AUD_I2S_MCK_49_152M;
        case 76800000:
            return E_AUD_I2S_MCK_76_8M;
        case 24000000:
            return E_AUD_I2S_MCK_24M;
        case 48000000:
            return E_AUD_I2S_MCK_48M;
        case 0:
        default:
            break;
    }
    return E_AUD_I2S_MCK_NULL;
}
extern inline U32 HalAudApiEnumToMck(AudI2sMck_e eMck)
{
    switch (eMck)
    {
        case E_AUD_I2S_MCK_12_288M:
            return 12288000;
        case E_AUD_I2S_MCK_16_384M:
            return 16384000;
        case E_AUD_I2S_MCK_18_432M:
            return 18432000;
        case E_AUD_I2S_MCK_22_5792M:
            return 22579200;
        case E_AUD_I2S_MCK_24_576M:
            return 24576000;
        case E_AUD_I2S_MCK_32_768M:
            return 32768000;
        case E_AUD_I2S_MCK_36_864M:
            return 36864000;
        case E_AUD_I2S_MCK_49_152M:
            return 49152000;
        case E_AUD_I2S_MCK_76_8M:
            return 76800000;
        case E_AUD_I2S_MCK_24M:
            return 24000000;
        case E_AUD_I2S_MCK_48M:
            return 48000000;
        case E_AUD_I2S_MCK_NULL:
        default:
            break;
    }
    return 0;
}

#endif //__HAL_AUD_OS_API_H__
