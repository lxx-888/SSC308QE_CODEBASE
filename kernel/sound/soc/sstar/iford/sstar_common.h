/*
 * sstar_common.h - Sigmastar
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

#ifndef __SOUND_SOC_COMMON_H
#define __SOUND_SOC_COMMON_H

#include <sound/asound.h>
#include <linux/pm_runtime.h>

#define SNDRV_SSTAR_HW_PARAM_DMA_RATE    0
#define SNDRV_SSTAR_HW_PARAM_WIRE_MODE   1
#define SNDRV_SSTAR_HW_PARAM_MSMODE      2
#define SNDRV_SSTAR_HW_PARAM_FORMAT      3
#define SNDRV_SSTAR_HW_PARAM_I2S_CHANNEL 4
#define SNDRV_SSTAR_HW_PARAM_TIME_OUT    5

#define AUDIO_MAX_REGISTER_OFFSET 0x0300

#define BANK1_ADDR_OFFSET 0x000 // 1502
#define BANK2_ADDR_OFFSET 0x100 // 1503
#define BANK3_ADDR_OFFSET 0x200 // 1504
#define BANK4_ADDR_OFFSET 0x300 // 1505
#define BANK5_ADDR_OFFSET 0x500 // 1507
#define ERROR_VAL         "ERROR"
#define ERROR_ID          (-1)
#define NLPCM_SEL         (3)
#define LPCM_SEL          (2)

#define SSTAR_BACH_RATES   (SNDRV_PCM_RATE_8000_192000)
#define SSTAR_BACH_FORMATS (SNDRV_PCM_FMTBIT_S16_LE)

#define DAC_DPGA_L_ADDR        (AUDIO_BANK2_REG_MMC1_DPGA1_CFG0_ADDR + BANK2_ADDR_OFFSET) << 1
#define DAC_DPGA_R_ADDR        (AUDIO_BANK2_REG_MMC1_DPGA1_CFG1_ADDR + BANK2_ADDR_OFFSET) << 1
#define I2S_TXA_01_DPGA_L_ADDR (AUDIO_BANK2_REG_I2S_TDM_DPGA_CFG1_ADDR + BANK2_ADDR_OFFSET) << 1
#define I2S_TXA_01_DPGA_R_ADDR (AUDIO_BANK2_REG_I2S_TDM_DPGA_CFG2_ADDR + BANK2_ADDR_OFFSET) << 1
#define ADC_A_DPGA_L_ADDR      (AUDIO_BANK2_REG_ADC1_DPGA_CFG1_ADDR + BANK2_ADDR_OFFSET) << 1
#define ADC_A_DPGA_R_ADDR      (AUDIO_BANK2_REG_ADC1_DPGA_CFG2_ADDR + BANK2_ADDR_OFFSET) << 1
#define DMIC_01_DPGA_L_ADDR    (AUDIO_BANK2_REG_DMIC_CH01_DPGA_CFG1_ADDR + BANK2_ADDR_OFFSET) << 1
#define DMIC_01_DPGA_R_ADDR    (AUDIO_BANK2_REG_DMIC_CH01_DPGA_CFG2_ADDR + BANK2_ADDR_OFFSET) << 1
#define ECHO_01_DPGA_L_ADDR    (AUDIO_BANK2_REG_MMCDEC1_DPGA_CFG1_ADDR + BANK2_ADDR_OFFSET) << 1
#define ECHO_01_DPGA_R_ADDR    (AUDIO_BANK2_REG_MMCDEC1_DPGA_CFG2_ADDR + BANK2_ADDR_OFFSET) << 1

enum
{
    E_AO_DETACH = 0,
    E_DAC_A,
    E_I2S_TXA,
    E_ECHO_0,
};

enum
{
    E_AI_MCH_DETACH = 0,
    E_ADC_01,
    E_DMIC_A_01,
    E_ECHO_01,
    E_I2S_RXA_01,
    E_I2S_RXA_23,
    E_I2S_RXA_45,
    E_I2S_RXA_67,
};

enum
{
    E_AUD_NULL = 0,
    E_AUD_DAC_01,
    E_AUD_I2S_TXA,
    E_AUD_ECHO_01,
    E_AUD_VIR_TRACE,
    E_AUD_AO_IF_TOTAL,
};

enum
{
    E_AUD_AI_MCH_01 = 0,
    E_AUD_AI_MCH_23,
    E_AUD_AI_MCH_45,
    E_AUD_AI_MCH_67,
    E_AUD_AI_MCH_89,
    E_AUD_AI_MCH_AB,
    E_AUD_AI_MCH_CD,
    E_AUD_AI_MCH_EF,
    E_AI_VIR_TRACE,
    E_AUD_AI_MCH_TOTAL,
};

typedef struct sstar_aio_dev
{
    char name[15];
    int  action;
    bool status;
    int  ref;
} sstar_aio_dev_t;

struct sstar_platform_dev
{
    struct device *         dev;
    struct sstar_pcm_engine pcm_engine;

    int            format;
    struct regmap *regmap;

    int i2s_mclk0;

    int (*runtime_suspend)(struct device *dev);
    int (*runtime_resume)(struct device *dev);
    bool suspended;

    sstar_aio_dev_t  ao_path_select[E_AUD_AO_IF_TOTAL];
    sstar_aio_dev_t  ai_path_select[E_AUD_AI_MCH_TOTAL];
    u8               audio_delay_ctl;
    MHAL_AO_ChMode_e channel_mode;
    u8               ao_virtual_status;
    MHAL_AI_Attach_t tAiAttach;
    int              sidetone_value[2];
    char             interface_gain[E_MHAL_AI_IF_ADC_C_0_D_0][2];
};

static inline const struct snd_interval *hw_param_sstar_interval_c(const struct snd_pcm_hw_params *params,
                                                                   snd_pcm_hw_param_t              var)
{
    return &params->ires[var];
}

static inline unsigned int sstar_params_time_out(const struct snd_pcm_hw_params *p)
{
    return hw_param_sstar_interval_c(p, SNDRV_SSTAR_HW_PARAM_TIME_OUT)->min;
}

static inline unsigned int sstar_params_dma_rate(const struct snd_pcm_hw_params *p)
{
    return hw_param_sstar_interval_c(p, SNDRV_SSTAR_HW_PARAM_DMA_RATE)->min;
}

static inline unsigned int sstar_params_wire_mode(const struct snd_pcm_hw_params *p)
{
    return hw_param_sstar_interval_c(p, SNDRV_SSTAR_HW_PARAM_WIRE_MODE)->min;
}

static inline unsigned int sstar_params_ms_mode(const struct snd_pcm_hw_params *p)
{
    return hw_param_sstar_interval_c(p, SNDRV_SSTAR_HW_PARAM_MSMODE)->min;
}

static inline unsigned int sstar_params_i2s_channel(const struct snd_pcm_hw_params *p)
{
    return hw_param_sstar_interval_c(p, SNDRV_SSTAR_HW_PARAM_I2S_CHANNEL)->min;
}

static inline unsigned int sstar_params_format(const struct snd_pcm_hw_params *p)
{
    return hw_param_sstar_interval_c(p, SNDRV_SSTAR_HW_PARAM_FORMAT)->min;
}

static inline struct sstar_platform_dev *sstar_dai_to_platform(struct snd_soc_dai *dai)
{
    return snd_soc_dai_get_drvdata(dai);
}

static inline struct sstar_platform_dev *sstar_dapm_kcontrol_to_platform(struct snd_kcontrol *kcontrol)
{
    struct snd_soc_dapm_context *dapm = snd_soc_dapm_kcontrol_dapm(kcontrol);
    return snd_soc_component_get_drvdata(dapm->component);
}

static inline struct sstar_platform_dev *sstar_kcontrol_to_platform(struct snd_kcontrol *kcontrol)
{
    struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
    return snd_soc_component_get_drvdata(component);
}

static inline struct sstar_platform_dev *sstar_widget_to_platform(struct snd_soc_dapm_widget *w)
{
    return snd_soc_component_get_drvdata(w->dapm->component);
}

static inline int __maybe_unused sstar_bach_runtime_suspend(struct device *dev)
{
    int ret;

    // close clock
    ret = mhal_audio_runtime_power_ctl(false);
    if (ret)
    {
        dev_err(dev, "mhal_audio_runtime_power_ctl[close]  false\n");
    }

    return 0;
}

static inline int __maybe_unused sstar_bach_runtime_resume(struct device *dev)
{
    int ret;

    // open clock
    ret = mhal_audio_runtime_power_ctl(true);
    if (ret)
    {
        dev_err(dev, "Could not register component\n");
    }
    return ret;
}

#endif /* __SOUND_SOC_COMMON_H */
