/*
 * sstar_bach.c - Sigmastar
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

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/clk.h>
#include <linux/pm_runtime.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>
#include <sound/pcm_params.h>
#include <sound/dmaengine_pcm.h>
#include <linux/of_irq.h>
#include <linux/of.h>
#include <sound/tlv.h>
#include <linux/of_address.h>
#include "sstar_pcm.h"
#include "sstar_common.h"

#include "hal_audio_common.h"
#include "hal_audio_types.h"
#include "hal_audio_sys.h"
#include "hal_audio_os_api.h"

static const struct of_device_id sstar_bach_dt_match[] = {
    {.compatible = "sstar,bach", .data = (void *)0},
    {},
};
MODULE_DEVICE_TABLE(of, sstar_bach_dt_match);

#if 0
static int              g_sidetone_value[2]                           = {511, 511};
#endif
static const char *const sstar_virtual_mux_text[] = {"VIR_DETACH", "DAC", "I2S_TXA", "ECHO_0"};
static const char *const sstar_ao_mux_text[]      = {"AO_DETACH", "DMA"};
static const char *const sstar_ai_mux_text[]      = {"AI_DETACH",  "ADC_A",      "DMIC_01",    "ECHO_01",
                                                "I2S_RXA_01", "I2S_RXA_23", "I2S_RXA_45", "I2S_RXA_67"};
static const char *const channel_mode_text[]      = {"STEREO",   "DOUBLE_MONO", "DOUBLE_LEFT", "DOUBLE_RIGHT",
                                                "EXCHANGE", "ONLY_LEFT",   "ONLY_RIGHT",  "DOUBLE_NONE"};

static const char *const audio_delay_text[] = {"CLOSE", "OPEN"};

static const char *const i2s_enable_text[] = {"DISABLE", "ENABLE"};

static const struct soc_enum aif_mux_enum =
    SOC_ENUM_SINGLE(SND_SOC_NOPM, 0, ARRAY_SIZE(sstar_ai_mux_text), sstar_ai_mux_text);

static const struct soc_enum aof_mux_enum =
    SOC_ENUM_SINGLE(SND_SOC_NOPM, 0, ARRAY_SIZE(sstar_ao_mux_text), sstar_ao_mux_text);

static const struct soc_enum vir_mux_enum =
    SOC_ENUM_SINGLE(SND_SOC_NOPM, 0, ARRAY_SIZE(sstar_virtual_mux_text), sstar_virtual_mux_text);

static const struct soc_enum channel_mode_enum = SOC_ENUM_SINGLE_VIRT(ARRAY_SIZE(channel_mode_text), channel_mode_text);

static const struct soc_enum audio_delay_enum = SOC_ENUM_SINGLE_VIRT(ARRAY_SIZE(audio_delay_text), audio_delay_text);

static const struct soc_enum i2s_enable_enum = SOC_ENUM_SINGLE_VIRT(ARRAY_SIZE(i2s_enable_text), i2s_enable_text);

static int sstar_get_mch_mux(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
    int                        i;
    struct sstar_platform_dev *platform       = sstar_dapm_kcontrol_to_platform(kcontrol);
    sstar_aio_dev_t *          ai_path_select = platform->ai_path_select;

    for (i = 0; i < ARRAY_SIZE(platform->ai_path_select); i++)
    {
        if (strstr(ucontrol->id.name, ai_path_select[i].name))
        {
            ucontrol->value.enumerated.item[0] = ai_path_select[i].action;
        }
    }

    return 0;
}

static int sstar_put_mch_mux(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
    int                          i;
    struct snd_soc_dapm_context *dapm           = snd_soc_dapm_kcontrol_dapm(kcontrol);
    struct soc_enum *            e              = (struct soc_enum *)kcontrol->private_value;
    unsigned int *               item           = ucontrol->value.enumerated.item;
    struct sstar_platform_dev *  platform       = snd_soc_component_get_drvdata(dapm->component);
    sstar_aio_dev_t *            ai_path_select = platform->ai_path_select;

    if (item[0] >= e->items)
    {
        printk("set item error:[%d] >= [%d]\n", item[0], e->items);
        return 0;
    }

    for (i = 0; i < ARRAY_SIZE(platform->ai_path_select); i++)
    {
        if (strstr(ucontrol->id.name, ai_path_select[i].name))
        {
            // If want change the path directly, first detach the path.
            if (item[0] != 0)
            {
                if (ai_path_select[i].status && ai_path_select[i].action != item[0])
                {
                    ai_path_select[i].action = 0;
                    snd_soc_dapm_mux_update_power(dapm, kcontrol, 0, e, NULL);
                }
            }
            ai_path_select[i].action = item[0];
            ai_path_select[i].status = true;
        }
    }

    snd_soc_dapm_mux_update_power(dapm, kcontrol, item[0], e, NULL);

    return 0;
}

static int sstar_get_ao_out_mux(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
    int                        i;
    struct sstar_platform_dev *platform       = sstar_dapm_kcontrol_to_platform(kcontrol);
    sstar_aio_dev_t *          ao_path_select = platform->ao_path_select;

    for (i = 0; i < ARRAY_SIZE(platform->ao_path_select); i++)
    {
        if (strstr(ucontrol->id.name, ao_path_select[i].name))
        {
            // printk("name = %s, sname = %s, action = %d\n", ucontrol->id.name, ao_path_select[i].name,
            // ao_path_select[i].action);
            ucontrol->value.enumerated.item[0] = ao_path_select[i].action;
            break;
        }
    }

    return 0;
}

static int sstar_put_ao_out_mux(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
    struct snd_soc_dapm_context *dapm = snd_soc_dapm_kcontrol_dapm(kcontrol);
    struct soc_enum *            e    = (struct soc_enum *)kcontrol->private_value;
    unsigned int *               item = ucontrol->value.enumerated.item;
    int                          i;
    struct sstar_platform_dev *  platform       = snd_soc_component_get_drvdata(dapm->component);
    sstar_aio_dev_t *            ao_path_select = platform->ao_path_select;

    if (item[0] >= e->items)
    {
        printk("set item error:[%d] >= [%d]\n", item[0], e->items);
        return 0;
    }

    for (i = 0; i < ARRAY_SIZE(platform->ao_path_select); i++)
    {
        if (strstr(ucontrol->id.name, ao_path_select[i].name))
        {
            // If want change the path directly, first detach the path.
            if (item[0] != 0 && ao_path_select[i].status && ao_path_select[i].action != item[0])
            {
                snd_soc_dapm_mux_update_power(dapm, kcontrol, 0, e, NULL);
                ao_path_select[i].ref--;
            }
            if (item[0] == 0)
            {
                ao_path_select[i].ref--;
            }
            else
            {
                ao_path_select[i].ref++;
            }
            // Set the range:[0, 10]
            if (ao_path_select[i].ref < 0)
            {
                ao_path_select[i].ref = 0;
            }
            else if (ao_path_select[i].ref > 10)
            {
                ao_path_select[i].ref = 10;
            }
            if (item[0] == 0 && ao_path_select[i].ref != 0)
            {
                return 0;
            }
            ao_path_select[i].action = item[0];
            ao_path_select[i].status = true;
            break;
        }
    }

    snd_soc_dapm_mux_update_power(dapm, kcontrol, item[0], e, NULL);

    return 0;
}

static int _sstar_plat_convert_ai_act_to_enum(int action)
{
    MHAL_AI_IF_e ai_interface = E_MHAL_AI_IF_NONE;

    switch (action)
    {
        case E_ADC_01:
            ai_interface = E_MHAL_AI_IF_ADC_A_0_B_0;
            break;
        case E_DMIC_A_01:
            ai_interface = E_MHAL_AI_IF_DMIC_A_0_1;
            break;
        case E_ECHO_01:
            ai_interface = E_MHAL_AI_IF_ECHO_A_0_1;
            break;
        case E_I2S_RXA_01:
            ai_interface = E_MHAL_AI_IF_I2S_RX_A_0_1;
            break;
        case E_I2S_RXA_23:
            ai_interface = E_MHAL_AI_IF_I2S_RX_A_2_3;
            break;
        case E_I2S_RXA_45:
            ai_interface = E_MHAL_AI_IF_I2S_RX_A_4_5;
            break;
        case E_I2S_RXA_67:
            ai_interface = E_MHAL_AI_IF_I2S_RX_A_6_7;
            break;
        default:
            break;
    }

    return ai_interface;
}

static int sstar_codec_aif_power_manage(struct snd_soc_dapm_widget *w, struct snd_kcontrol *kcontrol, int event)
{
    int                        ret, i;
    MHAL_AI_IF_e               ai_interface   = E_MHAL_AI_IF_NONE;
    struct sstar_platform_dev *platform       = sstar_widget_to_platform(w);
    sstar_aio_dev_t *          ai_path_select = platform->ai_path_select;

    for (i = 0; i < ARRAY_SIZE(platform->tAiAttach.aenAiIf); i++)
    {
        platform->tAiAttach.aenAiIf[i] = E_MHAL_AI_IF_NONE;
    }

    switch (event)
    {
        case SND_SOC_DAPM_POST_PMU:
            for (i = 0; i < ARRAY_SIZE(platform->ai_path_select); i++)
            {
                // printk("ai_path_select[%d].status = %d, action = %d\n", i, ai_path_select[i].status,
                //         ai_path_select[i].action);
                if (!ai_path_select[i].status || !ai_path_select[i].action)
                {
                    continue;
                }

                ai_interface = _sstar_plat_convert_ai_act_to_enum(ai_path_select[i].action);
                // attach
                if (i == E_AI_VIR_TRACE)
                {
                    platform->tAiAttach.enAiDma    = E_MHAL_AI_DMA_DIRECT_A + platform->pcm_engine.wdma;
                    platform->tAiAttach.aenAiIf[0] = ai_interface;
                    ret                            = mhal_audio_ai_attach(&platform->tAiAttach);
                    break;
                }
                else
                {
                    platform->tAiAttach.enAiDma    = E_MHAL_AI_DMA_A + platform->pcm_engine.wdma;
                    platform->tAiAttach.aenAiIf[i] = ai_interface;
                    ret                            = mhal_audio_ai_attach(&platform->tAiAttach);
                }
            }
            break;
        case SND_SOC_DAPM_POST_PMD:
            for (i = 0; i < ARRAY_SIZE(platform->ai_path_select); i++)
            {
                // printk("%s %d [%d]status[%d], action[%d]\n", __func__, __LINE__, i, ai_path_select[i].status,
                //        ai_path_select[i].action);
                if ((!ai_path_select[i].status) || (ai_path_select[i].action != 0))
                {
                    continue;
                }

                if (strcmp(w->name, ai_path_select[i].name))
                {
                    continue;
                }

                ai_path_select[i].status = false;

                if (i == E_AI_VIR_TRACE)
                {
                    platform->tAiAttach.enAiDma = E_MHAL_AI_DMA_DIRECT_A + platform->pcm_engine.wdma;
                    platform->tAiAttach.aenAiIf[0] =
                        E_MHAL_AI_IF_ADC_A_0_B_0; // Just tell the mhal the channel i need detach.
                    ret = mhal_audio_ai_detach(&platform->tAiAttach);
                }
                else
                {
                    platform->tAiAttach.enAiDma = E_MHAL_AI_DMA_A + platform->pcm_engine.wdma;
                    platform->tAiAttach.aenAiIf[i] =
                        E_MHAL_AI_IF_ADC_A_0_B_0; // Just tell the mhal the channel i need detach.
                    ret = mhal_audio_ai_detach(&platform->tAiAttach);
                }
                ai_path_select[i].action = E_AI_MCH_DETACH;
                break;
            }
            break;
        default:
            break;
    }

    return 0;
}

static int sstar_codec_aof_power_manage(struct snd_soc_dapm_widget *w, struct snd_kcontrol *kcontrol, int event)
{
    static MHAL_AO_IF_e ao_l_if;
    static MHAL_AO_IF_e ao_r_if;
    int                 ret, i, nsrc = 0;
    MHAL_AO_Attach_t    tAoAttach = {
        .enAoDma = E_MHAL_AO_DMA_TOTAL, .nAoDmaCh = 0, .enAoIf = E_MHAL_AO_IF_NONE, .nUseSrc = false};
    struct sstar_platform_dev *platform       = sstar_widget_to_platform(w);
    sstar_aio_dev_t *          ao_path_select = platform->ao_path_select;

    switch (event)
    {
        case SND_SOC_DAPM_POST_PMU:
            for (i = 0; i < ARRAY_SIZE(platform->ao_path_select); i++)
            {
                if ((!ao_path_select[i].status) || i == E_AUD_VIR_TRACE || !ao_path_select[i].action)
                {
                    continue;
                }

                switch (i)
                {
                    case E_AUD_DAC_01:
                        ao_l_if = E_MHAL_AO_IF_DAC_A_0;
                        ao_r_if = E_MHAL_AO_IF_DAC_B_0;
                        break;
                    case E_AUD_I2S_TXA:
                        ao_l_if = E_MHAL_AO_IF_I2S_TX_A_0;
                        ao_r_if = E_MHAL_AO_IF_I2S_TX_A_1;
                        break;
                    case E_AUD_ECHO_01:
                        ao_l_if = E_MHAL_AO_IF_ECHO_A_0;
                        ao_r_if = E_MHAL_AO_IF_ECHO_A_1;
                        break;
                    default:
                        return -1;
                }

                tAoAttach.enAoDma  = E_MHAL_AO_DMA_A + platform->pcm_engine.rdma;
                tAoAttach.nAoDmaCh = 0;
                tAoAttach.enAoIf   = ao_l_if;
                tAoAttach.nUseSrc  = nsrc;
                ret                = mhal_audio_ao_attach(&tAoAttach);
                tAoAttach.nAoDmaCh = 1;
                tAoAttach.enAoIf   = ao_r_if;
                tAoAttach.nUseSrc  = nsrc;
                ret                = mhal_audio_ao_attach(&tAoAttach);
            }

            if (ao_path_select[E_AUD_VIR_TRACE].status == true && ao_path_select[E_AUD_VIR_TRACE].action)
            {
                switch (ao_path_select[E_AUD_VIR_TRACE].action)
                {
                    case E_DAC_A:
                        ao_l_if = E_MHAL_AO_IF_DAC_A_0;
                        ao_r_if = E_MHAL_AO_IF_DAC_B_0;
                        break;
                    case E_ECHO_0:
                        ao_l_if = E_MHAL_AO_IF_ECHO_A_0;
                        ao_r_if = E_MHAL_AO_IF_ECHO_A_1;
                        break;
                    case E_I2S_TXA:
                        ao_l_if = E_MHAL_AO_IF_I2S_TX_A_0;
                        ao_r_if = E_MHAL_AO_IF_I2S_TX_A_1;
                        break;
                    default:
                        return -1;
                }

                platform->ao_virtual_status = ao_path_select[E_AUD_VIR_TRACE].action;

                tAoAttach.enAoDma  = E_MHAL_AO_DMA_DIRECT_A + platform->pcm_engine.rdma; // AUD_AO_DMA_A;
                tAoAttach.nAoDmaCh = 0;
                tAoAttach.enAoIf   = ao_l_if;
                ret                = mhal_audio_ao_attach(&tAoAttach);
                tAoAttach.nAoDmaCh = 1;
                tAoAttach.enAoIf   = ao_r_if;
                ret                = mhal_audio_ao_attach(&tAoAttach);
            }

            break;
        case SND_SOC_DAPM_POST_PMD:
            for (i = 0; i < ARRAY_SIZE(platform->ao_path_select); i++)
            {
                if ((!ao_path_select[i].status) || i == E_AUD_VIR_TRACE)
                {
                    continue;
                }

                if (ao_path_select[i].action != 0)
                {
                    continue;
                }
                switch (i)
                {
                    case E_AUD_DAC_01:
                        ao_l_if = E_MHAL_AO_IF_DAC_A_0;
                        ao_r_if = E_MHAL_AO_IF_DAC_B_0;
                        break;
                    case E_AUD_I2S_TXA:
                        ao_l_if = E_MHAL_AO_IF_I2S_TX_A_0;
                        ao_r_if = E_MHAL_AO_IF_I2S_TX_A_1;
                        break;
                    case E_AUD_ECHO_01:
                        ao_l_if = E_MHAL_AO_IF_ECHO_A_0;
                        ao_r_if = E_MHAL_AO_IF_ECHO_A_1;
                        break;
                    default:
                        return -1;
                }

                tAoAttach.enAoDma  = E_MHAL_AO_DMA_A + platform->pcm_engine.rdma;
                tAoAttach.nAoDmaCh = 0;
                tAoAttach.enAoIf   = ao_l_if;
                ret                = mhal_audio_ao_detach(&tAoAttach);
                tAoAttach.nAoDmaCh = 1;
                tAoAttach.enAoIf   = ao_r_if;
                ret                = mhal_audio_ao_detach(&tAoAttach);

                ao_path_select[i].status = false;
            }

            if (ao_path_select[E_AUD_VIR_TRACE].status == true && ao_path_select[E_AUD_VIR_TRACE].action == 0)
            {
                switch (platform->ao_virtual_status)
                {
                    case E_DAC_A:
                        ao_l_if = E_MHAL_AO_IF_DAC_A_0;
                        ao_r_if = E_MHAL_AO_IF_DAC_B_0;
                        break;
                    case E_ECHO_0:
                        ao_l_if = E_MHAL_AO_IF_ECHO_A_0;
                        ao_r_if = E_MHAL_AO_IF_ECHO_A_1;
                        break;
                    case E_I2S_TXA:
                        ao_l_if = E_MHAL_AO_IF_I2S_TX_A_0;
                        ao_r_if = E_MHAL_AO_IF_I2S_TX_A_1;
                        break;
                    default:
                        return -1;
                }

                tAoAttach.enAoDma  = E_MHAL_AO_DMA_DIRECT_A + platform->pcm_engine.rdma;
                tAoAttach.nAoDmaCh = 0;
                tAoAttach.enAoIf   = ao_l_if;
                ret                = mhal_audio_ao_detach(&tAoAttach);
                tAoAttach.nAoDmaCh = 1;
                tAoAttach.enAoIf   = ao_r_if;
                ret                = mhal_audio_ao_detach(&tAoAttach);

                ao_path_select[E_AUD_VIR_TRACE].status = false;
                platform->ao_virtual_status            = E_AO_DETACH;
            }

            break;
        default:
            return -1;
    }

    return 0;
}

static int snd_soc_sstar_put_amp_state(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
    int                       val  = ucontrol->value.integer.value[0];
    int                       val1 = ucontrol->value.integer.value[1];
    int                       ret  = 0;
    struct soc_mixer_control *mc   = (struct soc_mixer_control *)kcontrol->private_value;

    ret = mhal_audio_amp_state_set(val, mc->shift);
    ret |= mhal_audio_amp_state_set(val1, mc->rshift);
    if (ret == 0)
    {
        return 0;
    }

    return -1;
}

static int snd_soc_sstar_get_amp_state(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
    int                       val  = 0;
    int                       val1 = 0;
    int                       ret  = 0;
    struct soc_mixer_control *mc   = (struct soc_mixer_control *)kcontrol->private_value;

    ret = mhal_audio_amp_state_get(&val, mc->shift);
    ret |= mhal_audio_amp_state_get(&val1, mc->rshift);
    if (ret)
    {
        return -1;
    }

    ucontrol->value.integer.value[0] = val;
    ucontrol->value.integer.value[1] = val1;

    return 0;
}

static int snd_soc_sstar_analog_gain_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
    struct sstar_platform_dev *platform = sstar_kcontrol_to_platform(kcontrol);
    struct soc_mixer_control * mc       = (struct soc_mixer_control *)kcontrol->private_value;
    int                        interface;
    interface = mc->reg;

    ucontrol->value.integer.value[0] = platform->interface_gain[interface - 1][0];
    ucontrol->value.integer.value[1] = platform->interface_gain[interface - 1][1];

    return 0;
}

static int snd_soc_sstar_analog_gain_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
    struct sstar_platform_dev *platform = sstar_kcontrol_to_platform(kcontrol);
    struct soc_mixer_control * mc       = (struct soc_mixer_control *)kcontrol->private_value;
    int                        interface;
    static char                var1, var2;
    static char                count = 0;
    int                        ret   = 0;

    interface = mc->reg;
    if (count == 0)
    {
        var1 = ucontrol->value.integer.value[0];
    }
    else
    {
        var2                                       = ucontrol->value.integer.value[1];
        ret                                        = mhal_audio_ai_if_gain(interface, var1, var2);
        platform->interface_gain[interface - 1][0] = var1;
        platform->interface_gain[interface - 1][1] = var2;
        count                                      = 0;
        return ret;
    }
    count++;
    return ret;
}

static int snd_soc_sstar_put_volsw(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
    struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
    struct soc_mixer_control *mc        = (struct soc_mixer_control *)kcontrol->private_value;
    unsigned int              reg       = mc->reg;
    unsigned int              reg2      = mc->rreg;
    unsigned int              shift     = mc->shift;
    unsigned int              rshift    = mc->rshift;
    int                       max       = mc->max / 2;
    int                       min       = mc->min;
    unsigned int              mask      = (1 << fls(mc->max)) - 1;
    unsigned int              sign_bit  = mc->sign_bit;
    unsigned int              invert    = mc->invert;
    int                       err;
    bool                      type_2r = false;
    unsigned int              val2    = 0;
    signed int                val;
    unsigned int              val_mask;

    if (sign_bit)
        mask = BIT(sign_bit + 1) - 1;

    val = ((ucontrol->value.integer.value[0] + min) & mask);
    if (invert)
        val = max - val;
    val_mask = mask << shift;
    val      = val << shift;
    if (snd_soc_volsw_is_stereo(mc))
    {
        val2 = ((ucontrol->value.integer.value[1] + min) & mask);
        if (invert)
            val2 = max - val2;
        if (reg == reg2)
        {
            val_mask |= mask << rshift;
            val &= ~(mask << rshift);
            val |= (val2 << rshift);
        }
        else
        {
            val2    = val2 << shift;
            type_2r = true;
        }
    }
    err = snd_soc_component_update_bits(component, reg, val_mask, val);
    if (err < 0)
        return err;

    if (type_2r)
    {
        err = snd_soc_component_update_bits(component, reg2, val_mask, val2);
    }

    return err;
}

static int snd_soc_sstar_put_volsw_fadding(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
    struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
    struct soc_mixer_control *mc        = (struct soc_mixer_control *)kcontrol->private_value;
    int                       err;
    bool                      type_2r = false;
    if (snd_soc_volsw_is_stereo(mc) && mc->reg != mc->rreg)
    {
        type_2r = true;
    }
    err = snd_soc_sstar_put_volsw(kcontrol, ucontrol);
    if (err < 0)
    {
        return err;
    }
    // FIXME: set fading
    err |= snd_soc_component_update_bits(component, mc->reg, AUDIO_BANK2_REG_MMC1_DPGA1_CFG0_MMC1_1_FADING_EN_MASK,
                                         AUDIO_BANK2_REG_MMC1_DPGA1_CFG0_MMC1_1_FADING_EN_MASK);
    if (type_2r)
    {
        err |= snd_soc_component_update_bits(component, mc->rreg, AUDIO_BANK2_REG_MMC1_DPGA1_CFG1_MMC1_1_STEP_MASK,
                                             E_MHAL_AUDIO_GAIN_FADING_7);
    }
    return err;
}

static int snd_soc_sstar_get_volsw(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
    struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
    struct soc_mixer_control *mc        = (struct soc_mixer_control *)kcontrol->private_value;
    unsigned int              reg       = mc->reg;
    unsigned int              reg2      = mc->rreg;
    unsigned int              shift     = mc->shift;
    unsigned int              rshift    = mc->rshift;
    int                       max       = mc->max / 2;
    int                       min       = mc->min;
    unsigned int              mask      = (1 << fls(mc->max)) - 1;
    int                       sign_bit  = mc->sign_bit;
    unsigned int              invert    = mc->invert;
    int                       val;
    int                       i;

    if (sign_bit)
        mask = BIT(sign_bit + 1) - 1;

    for (i = 0; i < 2; i++)
    {
        if (i == 1)
        {
            if (!snd_soc_volsw_is_stereo(mc))
            {
                break;
            }
            if (reg == reg2)
            {
                shift = rshift;
            }
            else
            {
                reg = reg2;
            }
        }
        val                              = snd_soc_component_read(component, reg);
        ucontrol->value.integer.value[i] = ((val >> shift) & mask) - min;
        if (ucontrol->value.integer.value[i] > (mask >> 1))
        {
            ucontrol->value.integer.value[i] -= mask + 1;
        }

        if (invert)
        {
            ucontrol->value.integer.value[i] = max - ucontrol->value.integer.value[i];
        }
    }
    return 0;
}

static int sstar_get_channel_mode_mux(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
    struct sstar_platform_dev *platform = sstar_kcontrol_to_platform(kcontrol);

    ucontrol->value.enumerated.item[0] = platform->channel_mode;

    return 0;
}

static int sstar_put_channel_mode_mux(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
    struct sstar_platform_dev *platform     = sstar_kcontrol_to_platform(kcontrol);
    int                        channel_mode = (MHAL_AO_ChMode_e)ucontrol->value.enumerated.item[0];

    if (channel_mode >= E_MHAL_AO_CH_MODE_MAX || channel_mode < 0)
    {
        return -1;
    }

    platform->channel_mode = channel_mode;

    mhal_audio_ao_set_channel_mode(E_MHAL_AO_DMA_A + platform->pcm_engine.rdma, channel_mode);

    return 0;
}

static const SNDRV_CTL_TLVD_DECLARE_DB_SCALE(sstar_vol_tlv, -63875, 125, 63875);
static const SNDRV_CTL_TLVD_DECLARE_DB_SCALE(sstar_vol_prescale_tlv, -7875, 125, 7875);

static const struct snd_kcontrol_new sstar_bach_controls[] = {
    // AO DPGA
    SOC_DOUBLE_R_EXT_TLV("DAC Playback Volume", DAC_DPGA_L_ADDR, DAC_DPGA_R_ADDR,
                         AUDIO_BANK2_REG_MMC1_DPGA1_CFG0_MMC1_1_REG_GAIN_L_SHIFT, /* shift */
                         AUDIO_BANK2_REG_MMC1_DPGA1_CFG0_MMC1_1_REG_GAIN_L_MASK
                             >> AUDIO_BANK2_REG_MMC1_DPGA1_CFG0_MMC1_1_REG_GAIN_L_SHIFT,
                         1, /* invert */ snd_soc_sstar_get_volsw, snd_soc_sstar_put_volsw_fadding, sstar_vol_tlv),
    SOC_DOUBLE_R_EXT_TLV("I2S_TXA_01 Playback Volume", I2S_TXA_01_DPGA_L_ADDR, I2S_TXA_01_DPGA_R_ADDR,
                         AUDIO_BANK2_REG_I2S_TDM_DPGA_CFG1_REG_I2S_TDM_REG_GAIN_L_SHIFT, /* shift */
                         AUDIO_BANK2_REG_I2S_TDM_DPGA_CFG1_REG_I2S_TDM_REG_GAIN_L_MASK
                             >> AUDIO_BANK2_REG_I2S_TDM_DPGA_CFG1_REG_I2S_TDM_REG_GAIN_L_SHIFT,
                         1, /* invert */ snd_soc_sstar_get_volsw, snd_soc_sstar_put_volsw_fadding, sstar_vol_tlv),
    // AI DPGA
    SOC_DOUBLE_R_EXT_TLV("ADC_A Capture Volume", ADC_A_DPGA_L_ADDR, ADC_A_DPGA_R_ADDR,
                         AUDIO_BANK2_REG_ADC1_DPGA_CFG1_ADC1_REG_GAIN_L_SHIFT, /* shift */
                         AUDIO_BANK2_REG_ADC1_DPGA_CFG1_ADC1_REG_GAIN_L_MASK
                             >> AUDIO_BANK2_REG_ADC1_DPGA_CFG1_ADC1_REG_GAIN_L_SHIFT,
                         1, /* invert */ snd_soc_sstar_get_volsw, snd_soc_sstar_put_volsw_fadding, sstar_vol_tlv),
    SOC_DOUBLE_R_EXT_TLV("DMIC_01 Capture Volume", DMIC_01_DPGA_L_ADDR, DMIC_01_DPGA_R_ADDR,
                         AUDIO_BANK2_REG_DMIC_CH01_DPGA_CFG1_DMIC0_REG_GAIN_L_SHIFT, /* shift */
                         AUDIO_BANK2_REG_DMIC_CH01_DPGA_CFG1_DMIC0_REG_GAIN_L_MASK
                             >> AUDIO_BANK2_REG_DMIC_CH01_DPGA_CFG1_DMIC0_REG_GAIN_L_SHIFT,
                         1, /* invert */ snd_soc_sstar_get_volsw, snd_soc_sstar_put_volsw_fadding, sstar_vol_tlv),
    SOC_DOUBLE_R_EXT_TLV("ECHO_01 Capture Volume", ECHO_01_DPGA_L_ADDR, ECHO_01_DPGA_R_ADDR,
                         AUDIO_BANK2_REG_MMCDEC1_DPGA_CFG1_MMCDEC1_REG_GAIN_L_SHIFT, /* shift */
                         AUDIO_BANK2_REG_MMCDEC1_DPGA_CFG1_MMCDEC1_REG_GAIN_L_MASK
                             >> AUDIO_BANK2_REG_MMCDEC1_DPGA_CFG1_MMCDEC1_REG_GAIN_L_SHIFT,
                         1, /* invert */ snd_soc_sstar_get_volsw, snd_soc_sstar_put_volsw_fadding, sstar_vol_tlv),
#if 0
    SOC_DOUBLE_EXT_TLV("ADC_A PRESCALE Volume", (AUDIO_BANK2_REG_ADC1_DPGA_CFG3_ADDR + BANK2_ADDR_OFFSET) << 1,
                       AUDIO_BANK2_REG_ADC1_DPGA_CFG3_ADC1_REG_OFFSET_L_SHIFT,
                       AUDIO_BANK2_REG_ADC1_DPGA_CFG3_ADC1_REG_OFFSET_R_SHIFT,
                       AUDIO_BANK2_REG_ADC1_DPGA_CFG3_ADC1_REG_OFFSET_L_MASK, 1, snd_soc_sstar_get_volsw,
                       snd_soc_sstar_put_volsw, sstar_vol_prescale_tlv),
    SOC_DOUBLE_EXT_TLV("DMIC_01 PRESCALE Volume", (AUDIO_BANK2_REG_DMIC_CH01_DPGA_CFG3_ADDR + BANK2_ADDR_OFFSET) << 1,
                       AUDIO_BANK2_REG_DMIC_CH01_DPGA_CFG3_DMIC0_REG_OFFSET_L_SHIFT,
                       AUDIO_BANK2_REG_DMIC_CH01_DPGA_CFG3_DMIC0_REG_OFFSET_R_SHIFT,
                       AUDIO_BANK2_REG_DMIC_CH01_DPGA_CFG3_DMIC0_REG_OFFSET_L_MASK, 1, snd_soc_sstar_get_volsw,
                       snd_soc_sstar_put_volsw, sstar_vol_prescale_tlv),
    SOC_DOUBLE_EXT_TLV("DAC PRESCALE Volume", (AUDIO_BANK2_REG_MMC1_DPGA1_CFG2_ADDR + BANK2_ADDR_OFFSET) << 1,
                       AUDIO_BANK2_REG_MMC1_DPGA1_CFG2_MMC1_1_REG_OFFSET_L_SHIFT,
                       AUDIO_BANK2_REG_MMC1_DPGA1_CFG2_MMC1_1_REG_OFFSET_R_SHIFT,
                       AUDIO_BANK2_REG_MMC1_DPGA1_CFG2_MMC1_1_REG_OFFSET_L_MASK, 1, snd_soc_sstar_get_volsw,
                       snd_soc_sstar_put_volsw, sstar_vol_prescale_tlv),
    SOC_DOUBLE_EXT_TLV("I2S_TXA_01 PRESCALE Volume", (AUDIO_BANK2_REG_I2S_TDM_DPGA_CFG3_ADDR + BANK2_ADDR_OFFSET) << 1,
                       AUDIO_BANK2_REG_I2S_TDM_DPGA_CFG3_REG_I2S_TDM_REG_OFFSET_L_SHIFT,
                       AUDIO_BANK2_REG_I2S_TDM_DPGA_CFG3_REG_I2S_TDM_REG_OFFSET_R_SHIFT,
                       AUDIO_BANK2_REG_I2S_TDM_DPGA_CFG3_REG_I2S_TDM_REG_OFFSET_L_MASK, 1, snd_soc_sstar_get_volsw,
                       snd_soc_sstar_put_volsw, sstar_vol_prescale_tlv),
#endif
    SOC_DOUBLE_EXT("ADC_A ANOLG GAIN", E_MHAL_AI_IF_ADC_A_0_B_0, 0x0, 0x8, 0x13, 0x0, snd_soc_sstar_analog_gain_get,
                   snd_soc_sstar_analog_gain_put),
    SOC_DAPM_ENUM_EXT("CHANNEL_MODE_PLAYBACK", channel_mode_enum, sstar_get_channel_mode_mux,
                      sstar_put_channel_mode_mux),
    SOC_DOUBLE_EXT("AMP_CTL", SND_SOC_NOPM, 0x0, 0x1, 1, 0, snd_soc_sstar_get_amp_state, snd_soc_sstar_put_amp_state),
    // SOC_DOUBLE_EXT("HP_CTL", SND_SOC_NOPM, 0x2, 0x3, 1, 0, snd_soc_sstar_get_amp_state, snd_soc_sstar_put_amp_state),
};

static const struct snd_kcontrol_new ao_virtual_mux =
    SOC_DAPM_ENUM_EXT("AO_VIRTUAL_MUX_SEL", vir_mux_enum, sstar_get_ao_out_mux, sstar_put_ao_out_mux);
static const struct snd_kcontrol_new dac_mux =
    SOC_DAPM_ENUM_EXT("DAC_MUX_SEL", aof_mux_enum, sstar_get_ao_out_mux, sstar_put_ao_out_mux);
static const struct snd_kcontrol_new echo_mux =
    SOC_DAPM_ENUM_EXT("ECHO_MUX_SEL", aof_mux_enum, sstar_get_ao_out_mux, sstar_put_ao_out_mux);
static const struct snd_kcontrol_new i2s_txa_mux =
    SOC_DAPM_ENUM_EXT("I2S_TXA_MUX_SEL", aof_mux_enum, sstar_get_ao_out_mux, sstar_put_ao_out_mux);
static const struct snd_kcontrol_new multi_ch01_mux =
    SOC_DAPM_ENUM_EXT("MULTI_CH01_MUX_SEL", aif_mux_enum, sstar_get_mch_mux, sstar_put_mch_mux);
static const struct snd_kcontrol_new multi_ch23_mux =
    SOC_DAPM_ENUM_EXT("MULTI_CH23_MUX_SEL", aif_mux_enum, sstar_get_mch_mux, sstar_put_mch_mux);
static const struct snd_kcontrol_new multi_ch45_mux =
    SOC_DAPM_ENUM_EXT("MULTI_CH45_MUX_SEL", aif_mux_enum, sstar_get_mch_mux, sstar_put_mch_mux);
static const struct snd_kcontrol_new multi_ch67_mux =
    SOC_DAPM_ENUM_EXT("MULTI_CH67_MUX_SEL", aif_mux_enum, sstar_get_mch_mux, sstar_put_mch_mux);
static const struct snd_kcontrol_new multi_ch89_mux =
    SOC_DAPM_ENUM_EXT("MULTI_CH89_MUX_SEL", aif_mux_enum, sstar_get_mch_mux, sstar_put_mch_mux);
static const struct snd_kcontrol_new multi_chab_mux =
    SOC_DAPM_ENUM_EXT("MULTI_CHAB_MUX_SEL", aif_mux_enum, sstar_get_mch_mux, sstar_put_mch_mux);
static const struct snd_kcontrol_new multi_chcd_mux =
    SOC_DAPM_ENUM_EXT("MULTI_CHCD_MUX_SEL", aif_mux_enum, sstar_get_mch_mux, sstar_put_mch_mux);
static const struct snd_kcontrol_new multi_chef_mux =
    SOC_DAPM_ENUM_EXT("MULTI_CHEF_MUX_SEL", aif_mux_enum, sstar_get_mch_mux, sstar_put_mch_mux);
static const struct snd_kcontrol_new ai_virtual_mux =
    SOC_DAPM_ENUM_EXT("AI_VIRTUAL_MUX_SEL", aif_mux_enum, sstar_get_mch_mux, sstar_put_mch_mux);

static const struct snd_soc_dapm_widget sstar_bach_dapm_widgets[] = {
    SND_SOC_DAPM_MUX_E("AI_MCH_01_SEL", SND_SOC_NOPM, 0, 0, &multi_ch01_mux, sstar_codec_aif_power_manage,
                       SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_POST_PMD),
    SND_SOC_DAPM_MUX_E("AI_MCH_23_SEL", SND_SOC_NOPM, 0, 0, &multi_ch23_mux, sstar_codec_aif_power_manage,
                       SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_POST_PMD),
    SND_SOC_DAPM_MUX_E("AI_MCH_45_SEL", SND_SOC_NOPM, 0, 0, &multi_ch45_mux, sstar_codec_aif_power_manage,
                       SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_POST_PMD),
    SND_SOC_DAPM_MUX_E("AI_MCH_67_SEL", SND_SOC_NOPM, 0, 0, &multi_ch67_mux, sstar_codec_aif_power_manage,
                       SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_POST_PMD),
    SND_SOC_DAPM_MUX_E("AI_MCH_89_SEL", SND_SOC_NOPM, 0, 0, &multi_ch89_mux, sstar_codec_aif_power_manage,
                       SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_POST_PMD),
    SND_SOC_DAPM_MUX_E("AI_MCH_AB_SEL", SND_SOC_NOPM, 0, 0, &multi_chab_mux, sstar_codec_aif_power_manage,
                       SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_POST_PMD),
    SND_SOC_DAPM_MUX_E("AI_MCH_CD_SEL", SND_SOC_NOPM, 0, 0, &multi_chcd_mux, sstar_codec_aif_power_manage,
                       SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_POST_PMD),
    SND_SOC_DAPM_MUX_E("AI_MCH_EF_SEL", SND_SOC_NOPM, 0, 0, &multi_chef_mux, sstar_codec_aif_power_manage,
                       SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_POST_PMD),
    SND_SOC_DAPM_MUX_E("AI_MCH_VIR_SEL", SND_SOC_NOPM, 0, 0, &ai_virtual_mux, sstar_codec_aif_power_manage,
                       SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_POST_PMD),
    SND_SOC_DAPM_MUX_E("AO_VIR_MUX_SEL", SND_SOC_NOPM, 0, 0, &ao_virtual_mux, sstar_codec_aof_power_manage,
                       SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_POST_PMD),
    SND_SOC_DAPM_MUX_E("DAC_SEL", SND_SOC_NOPM, 0, 0, &dac_mux, sstar_codec_aof_power_manage,
                       SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_POST_PMD),
    SND_SOC_DAPM_MUX_E("ECHO_SEL", SND_SOC_NOPM, 0, 0, &echo_mux, sstar_codec_aof_power_manage,
                       SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_POST_PMD),
    SND_SOC_DAPM_MUX_E("I2S_TXA_SEL", SND_SOC_NOPM, 0, 0, &i2s_txa_mux, sstar_codec_aof_power_manage,
                       SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_POST_PMD),
    SND_SOC_DAPM_OUTPUT("WDMA"),
    SND_SOC_DAPM_OUTPUT("VIR_WDMA"),
    SND_SOC_DAPM_INPUT("AUDIO_IN"),
    SND_SOC_DAPM_OUTPUT("AUDIO_OUT"),
    SND_SOC_DAPM_INPUT("RDMA")};

static const struct snd_soc_dapm_route sstar_bach_audio_map[] = {
    {"WDMA", NULL, "AI_MCH_01_SEL"},
    {"AI_MCH_01_SEL", "ADC_A", "AUDIO_IN"},
    {"AI_MCH_01_SEL", "DMIC_01", "AUDIO_IN"},
    {"AI_MCH_01_SEL", "ECHO_01", "AUDIO_IN"},
    {"AI_MCH_01_SEL", "I2S_RXA_01", "AUDIO_IN"},
    {"AI_MCH_01_SEL", "I2S_RXA_23", "AUDIO_IN"},
    {"AI_MCH_01_SEL", "I2S_RXA_45", "AUDIO_IN"},
    {"AI_MCH_01_SEL", "I2S_RXA_67", "AUDIO_IN"},
    {"WDMA", NULL, "AI_MCH_23_SEL"},
    {"AI_MCH_23_SEL", "ADC_A", "AUDIO_IN"},
    {"AI_MCH_23_SEL", "DMIC_01", "AUDIO_IN"},
    {"AI_MCH_23_SEL", "ECHO_01", "AUDIO_IN"},
    {"AI_MCH_23_SEL", "I2S_RXA_01", "AUDIO_IN"},
    {"AI_MCH_23_SEL", "I2S_RXA_23", "AUDIO_IN"},
    {"AI_MCH_23_SEL", "I2S_RXA_45", "AUDIO_IN"},
    {"AI_MCH_23_SEL", "I2S_RXA_67", "AUDIO_IN"},
    {"WDMA", NULL, "AI_MCH_45_SEL"},
    {"AI_MCH_45_SEL", "ADC_A", "AUDIO_IN"},
    {"AI_MCH_45_SEL", "DMIC_01", "AUDIO_IN"},
    {"AI_MCH_45_SEL", "ECHO_01", "AUDIO_IN"},
    {"AI_MCH_45_SEL", "I2S_RXA_01", "AUDIO_IN"},
    {"AI_MCH_45_SEL", "I2S_RXA_23", "AUDIO_IN"},
    {"AI_MCH_45_SEL", "I2S_RXA_45", "AUDIO_IN"},
    {"AI_MCH_45_SEL", "I2S_RXA_67", "AUDIO_IN"},
    {"WDMA", NULL, "AI_MCH_67_SEL"},
    {"AI_MCH_67_SEL", "ADC_A", "AUDIO_IN"},
    {"AI_MCH_67_SEL", "DMIC_01", "AUDIO_IN"},
    {"AI_MCH_67_SEL", "ECHO_01", "AUDIO_IN"},
    {"AI_MCH_67_SEL", "I2S_RXA_01", "AUDIO_IN"},
    {"AI_MCH_67_SEL", "I2S_RXA_23", "AUDIO_IN"},
    {"AI_MCH_67_SEL", "I2S_RXA_45", "AUDIO_IN"},
    {"AI_MCH_67_SEL", "I2S_RXA_67", "AUDIO_IN"},
    {"WDMA", NULL, "AI_MCH_89_SEL"},
    {"AI_MCH_89_SEL", "ADC_A", "AUDIO_IN"},
    {"AI_MCH_89_SEL", "DMIC_01", "AUDIO_IN"},
    {"AI_MCH_89_SEL", "ECHO_01", "AUDIO_IN"},
    {"AI_MCH_89_SEL", "I2S_RXA_01", "AUDIO_IN"},
    {"AI_MCH_89_SEL", "I2S_RXA_23", "AUDIO_IN"},
    {"AI_MCH_89_SEL", "I2S_RXA_45", "AUDIO_IN"},
    {"AI_MCH_89_SEL", "I2S_RXA_67", "AUDIO_IN"},
    {"WDMA", NULL, "AI_MCH_AB_SEL"},
    {"AI_MCH_AB_SEL", "ADC_A", "AUDIO_IN"},
    {"AI_MCH_AB_SEL", "DMIC_01", "AUDIO_IN"},
    {"AI_MCH_AB_SEL", "ECHO_01", "AUDIO_IN"},
    {"AI_MCH_AB_SEL", "I2S_RXA_01", "AUDIO_IN"},
    {"AI_MCH_AB_SEL", "I2S_RXA_23", "AUDIO_IN"},
    {"AI_MCH_AB_SEL", "I2S_RXA_45", "AUDIO_IN"},
    {"AI_MCH_AB_SEL", "I2S_RXA_67", "AUDIO_IN"},
    {"WDMA", NULL, "AI_MCH_CD_SEL"},
    {"AI_MCH_CD_SEL", "ADC_A", "AUDIO_IN"},
    {"AI_MCH_CD_SEL", "DMIC_01", "AUDIO_IN"},
    {"AI_MCH_CD_SEL", "ECHO_01", "AUDIO_IN"},
    {"AI_MCH_CD_SEL", "I2S_RXA_01", "AUDIO_IN"},
    {"AI_MCH_CD_SEL", "I2S_RXA_23", "AUDIO_IN"},
    {"AI_MCH_CD_SEL", "I2S_RXA_45", "AUDIO_IN"},
    {"AI_MCH_CD_SEL", "I2S_RXA_67", "AUDIO_IN"},
    {"WDMA", NULL, "AI_MCH_EF_SEL"},
    {"AI_MCH_EF_SEL", "ADC_A", "AUDIO_IN"},
    {"AI_MCH_EF_SEL", "DMIC_01", "AUDIO_IN"},
    {"AI_MCH_EF_SEL", "ECHO_01", "AUDIO_IN"},
    {"AI_MCH_EF_SEL", "I2S_RXA_01", "AUDIO_IN"},
    {"AI_MCH_EF_SEL", "I2S_RXA_23", "AUDIO_IN"},
    {"AI_MCH_EF_SEL", "I2S_RXA_45", "AUDIO_IN"},
    {"AI_MCH_EF_SEL", "I2S_RXA_67", "AUDIO_IN"},
    {"VIR_WDMA", NULL, "AI_MCH_VIR_SEL"},
    {"AI_MCH_VIR_SEL", "ADC_A", "AUDIO_IN"},
    {"AI_MCH_VIR_SEL", "DMIC_01", "AUDIO_IN"},
    {"AI_MCH_VIR_SEL", "ECHO_01", "AUDIO_IN"},
    {"AI_MCH_VIR_SEL", "I2S_RXA_01", "AUDIO_IN"},
    {"AI_MCH_VIR_SEL", "I2S_RXA_23", "AUDIO_IN"},
    {"AI_MCH_VIR_SEL", "I2S_RXA_45", "AUDIO_IN"},
    {"AI_MCH_VIR_SEL", "I2S_RXA_67", "AUDIO_IN"},
    {"AUDIO_OUT", NULL, "DAC_SEL"},
    {"DAC_SEL", "DMA", "RDMA"},
    {"AUDIO_OUT", NULL, "ECHO_SEL"},
    {"ECHO_SEL", "DMA", "RDMA"},
    {"AUDIO_OUT", NULL, "I2S_TXA_SEL"},
    {"I2S_TXA_SEL", "DMA", "RDMA"},
    {"AO_VIR_MUX_SEL", "DAC", "RDMA"},
    {"AO_VIR_MUX_SEL", "I2S_TXA", "RDMA"},
    {"AO_VIR_MUX_SEL", "ECHO_0", "RDMA"},
};

static MHAL_AUDIO_I2sCfg_t _sstar_get_config_from_params(struct snd_pcm_hw_params *params)
{
    MHAL_AUDIO_I2sCfg_t i2s_cfg;
    // int drate;
    memset(&i2s_cfg, 0, sizeof(MHAL_AUDIO_I2sCfg_t));

    i2s_cfg.commonCfg.u32Rate = params_rate(params);

    switch (sstar_params_dma_rate(params))
    {
        case 8000:
        case 11025:
        case 16000:
        case 22050:
        case 32000:
        case 44100:
        case 48000:
            i2s_cfg.commonCfg.u32DmaRate = sstar_params_dma_rate(params);
            break;
        default:
            i2s_cfg.commonCfg.u32DmaRate = params_rate(params);
            break;
    }

    switch (sstar_params_format(params))
    {
        case E_MHAL_AUDIO_I2S_FMT_I2S:
        case E_MHAL_AUDIO_I2S_FMT_LEFT_JUSTIFY:
            i2s_cfg.enFmt = sstar_params_format(params);
            break;
        default:
            break;
    }

    switch (sstar_params_wire_mode(params))
    {
        case E_MHAL_AUDIO_4WIRE_ON:
        case E_MHAL_AUDIO_4WIRE_OFF:
            i2s_cfg.en4WireMode = sstar_params_wire_mode(params);
            break;
        default:
            break;
    }

    switch (sstar_params_ms_mode(params))
    {
        case E_MHAL_AUDIO_MODE_TDM_MASTER:
        case E_MHAL_AUDIO_MODE_TDM_SLAVE:
            i2s_cfg.enMode = sstar_params_ms_mode(params);
            break;
        default:
            break;
    }

    i2s_cfg.u16Width = params_width(params);

    switch (sstar_params_i2s_channel(params))
    {
        case 16:
            i2s_cfg.u16Channels = E_MHAL_AUDIO_CHN_16;
            break;
        case 8:
            i2s_cfg.u16Channels = E_MHAL_AUDIO_CHN_8;
            break;
        case 4:
            i2s_cfg.u16Channels = E_MHAL_AUDIO_CHN_4;
            break;
        case 2:
            i2s_cfg.u16Channels = E_MHAL_AUDIO_CHN_2;
            break;
        case 1:
            i2s_cfg.u16Channels = E_MHAL_AUDIO_CHN_1;
            break;
        default:
            i2s_cfg.u16Channels = E_MHAL_AUDIO_CHN_0;
            break;
    }

    return i2s_cfg;
}

static int _sstar_ao_interface_config_set(struct sstar_platform_dev *platform, MHAL_AUDIO_I2sCfg_t *i2sCfg)
{
    MHAL_AO_PATH_RATE_t aoAttribute;

    aoAttribute.nIfSampleRate  = i2sCfg->commonCfg.u32DmaRate;
    aoAttribute.nDmaSampleRate = i2sCfg->commonCfg.u32Rate;

    if (platform->ao_virtual_status)
    {
        aoAttribute.nDma = E_MHAL_AO_DMA_DIRECT_A;
    }
    else
    {
        aoAttribute.nDma = E_MHAL_AO_DMA_A + platform->pcm_engine.rdma;
    }

    if (platform->ao_path_select[E_AUD_I2S_TXA].status == true)
    {
        mhal_audio_ao_if_setting(E_MHAL_AO_IF_I2S_TX_A_0, i2sCfg, &aoAttribute);
    }

    if (platform->ao_path_select[E_AUD_ECHO_01].status == true)
    {
        mhal_audio_ao_if_setting(E_MHAL_AO_IF_ECHO_A_0, NULL, &aoAttribute);
    }

    if (platform->ao_path_select[E_AUD_DAC_01].status == true)
    {
        mhal_audio_ao_if_setting(E_MHAL_AO_IF_DAC_A_0, NULL, &aoAttribute);
    }

    return 0;
}

static int _sstar_ai_interface_config_set(struct sstar_platform_dev *platform, MHAL_AUDIO_I2sCfg_t *i2sCfg)
{
    int          i;
    MHAL_AI_IF_e ai_interface;

    for (i = 0; i < ARRAY_SIZE(platform->ai_path_select); i++)
    {
        if (!platform->ai_path_select[i].status)
        {
            continue;
        }

        ai_interface = _sstar_plat_convert_ai_act_to_enum(platform->ai_path_select[i].action);

        if (ai_interface > E_MHAL_AI_IF_I2S_RX_START && ai_interface < E_MHAL_AI_IF_I2S_RX_END)
        {
            mhal_audio_ai_if_setting(ai_interface, i2sCfg);
        }
        else if (ai_interface != E_MHAL_AI_IF_NONE)
        {
            mhal_audio_ai_if_setting(ai_interface, &(i2sCfg->commonCfg));
        }
    }

    return 0;
}

static int sstar_component_suspend(struct snd_soc_component *component)
{
    struct sstar_platform_dev *platform = snd_soc_component_get_drvdata(component);
    struct device *            dev      = platform->dev;
    int                        ret;

    // 1.Close AMP.
    mhal_audio_amp_state_set(0, 0);
    mhal_audio_amp_state_set(0, 1);

    // 2.Cache regmap register
    if (platform->pcm_engine.rdma == 0)
    {
        // close fadding
        int i     = 0;
        int reg[] = {DAC_DPGA_L_ADDR,     DAC_DPGA_R_ADDR,    I2S_TXA_01_DPGA_L_ADDR, I2S_TXA_01_DPGA_R_ADDR,
                     ADC_A_DPGA_L_ADDR,   ADC_A_DPGA_R_ADDR,  DMIC_01_DPGA_L_ADDR,    DMIC_01_DPGA_R_ADDR,
                     ECHO_01_DPGA_L_ADDR, ECHO_01_DPGA_R_ADDR};
        for (i = 0; i < ARRAY_SIZE(reg); i++)
        {
            int val = 0;
            // Read by volatile_reg
            platform->suspended = false;
            val                 = snd_soc_component_read(component, reg[i]);

            // Set it to regcache
            platform->suspended = true;
            snd_soc_component_update_bits(component, reg[i], 0xffff, val);
            if (i % 2 != 0)
            {
                snd_soc_component_update_bits(component, reg[i], AUDIO_BANK2_REG_MMC1_DPGA1_CFG1_MMC1_1_STEP_MASK,
                                              E_MHAL_AUDIO_GAIN_FADING_0);
            }
        }
        regcache_cache_only(platform->regmap, true);
    }
    platform->suspended = true;

    // 3.Mhal deinit
    ret = mhal_audio_deinit();
    if (ret)
    {
        dev_err(dev, "suspend: mhal_audio_deinit false\n");
    }

    // 4.Close PLL
    platform->runtime_suspend(dev);

    return 0;
}

static int sstar_component_resume(struct snd_soc_component *component)
{
    struct sstar_platform_dev *platform = snd_soc_component_get_drvdata(component);
    struct device *            dev      = platform->dev;
    int                        ret;
    struct sstar_pcm_engine *  ss_pcm = &platform->pcm_engine;
    struct sstar_pcm_stream *  ss_pcm_stream;

    // 1.Open PLL
    platform->runtime_resume(dev);

    // 2.Restore regmap register
    if (platform->pcm_engine.rdma == 0)
    {
        regcache_cache_only(platform->regmap, false);
        regcache_mark_dirty(platform->regmap);

        ret = regcache_sync(platform->regmap);
        if (ret)
        {
            dev_err(dev, "Could not register component\n");
        }
    }

    // 3.Mhal init
    ret = mhal_audio_init(NULL);
    if (ret)
    {
        dev_err(dev, "resume: mhal_audio_init false\n");
        return 0;
    }

    // 4.Set specal settings: channel mode & anlog gain.
    ret = mhal_audio_ao_set_channel_mode(E_MHAL_AO_DMA_A + platform->pcm_engine.rdma, platform->channel_mode);
    if (ret)
    {
        dev_err(dev, "%d mhal_audio_ao_set_channel_mode error\n", E_MHAL_AO_DMA_A + platform->pcm_engine.rdma);
    }

    ret = mhal_audio_ai_if_gain(E_MHAL_AI_IF_ADC_A_0_B_0, platform->interface_gain[E_MHAL_AI_IF_ADC_A_0_B_0 - 1][0],
                                platform->interface_gain[E_MHAL_AI_IF_ADC_A_0_B_0 - 1][1]);
    ret = mhal_audio_ai_if_gain(E_MHAL_AI_IF_ADC_C_0_D_0, platform->interface_gain[E_MHAL_AI_IF_ADC_C_0_D_0 - 1][0],
                                platform->interface_gain[E_MHAL_AI_IF_ADC_C_0_D_0 - 1][1]);
    if (ret)
    {
        dev_err(dev, "mhal_audio_ai_if_gain error\n");
    }

    // 5.Resume AI & AO.
    //  AO
    ss_pcm_stream = &ss_pcm->tx;
    dev_dbg(ss_pcm->dev, "AO suspend = [%d]\n", ss_pcm_stream->is_suspend);
    if (ss_pcm_stream->is_suspend)
    {
        dev_dbg(ss_pcm->dev, "[AO]rate[%d] chanl[%d], bufferbyte[%d], dma[%d]\n", ss_pcm_stream->pcm.u32Rate,
                ss_pcm_stream->pcm.u16Channels, ss_pcm_stream->pcm.u32BufferSize, ss_pcm_stream->dma_id);
        _sstar_ao_interface_config_set(platform, &(ss_pcm_stream->i2s_cfg));
        mhal_audio_ao_config(ss_pcm_stream->dma_id, &ss_pcm_stream->pcm);
        mhal_audio_ao_open(ss_pcm_stream->dma_id);
    }
    // AI
    ss_pcm_stream = &ss_pcm->rx;
    dev_dbg(ss_pcm->dev, "AI suspend = [%d]\n", ss_pcm_stream->is_suspend);
    if (ss_pcm_stream->is_suspend)
    {
        dev_dbg(ss_pcm->dev, "[AI]rate[%d] chanl[%d], bufferbyte[%d], dma[%d]\n", ss_pcm_stream->pcm.u32Rate,
                ss_pcm_stream->pcm.u16Channels, ss_pcm_stream->pcm.u32BufferSize, ss_pcm_stream->dma_id);
        _sstar_ai_interface_config_set(platform, &(ss_pcm_stream->i2s_cfg));
        mhal_audio_ai_config(ss_pcm_stream->dma_id, &ss_pcm_stream->pcm);
        mhal_audio_ai_open(ss_pcm_stream->dma_id);
    }
    platform->suspended = false;
    return 0;
}

static struct snd_soc_component_driver sstar_bach_component[] = {
    {
        .name             = "sstar-bach",
        .dapm_widgets     = sstar_bach_dapm_widgets,
        .num_dapm_widgets = ARRAY_SIZE(sstar_bach_dapm_widgets),
        .dapm_routes      = sstar_bach_audio_map,
        .num_dapm_routes  = ARRAY_SIZE(sstar_bach_audio_map),
        .controls         = sstar_bach_controls,
        .num_controls     = ARRAY_SIZE(sstar_bach_controls),
        .suspend          = sstar_component_suspend,
        .resume           = sstar_component_resume,
    },
    {
        .name             = "sstar-bach2",
        .dapm_widgets     = sstar_bach_dapm_widgets,
        .num_dapm_widgets = ARRAY_SIZE(sstar_bach_dapm_widgets),
        .dapm_routes      = sstar_bach_audio_map,
        .num_dapm_routes  = ARRAY_SIZE(sstar_bach_audio_map),
        .controls         = sstar_bach_controls,
        .num_controls     = ARRAY_SIZE(sstar_bach_controls),
        .suspend          = sstar_component_suspend,
        .resume           = sstar_component_resume,
    },
    {
        .name             = "sstar-bach3",
        .dapm_widgets     = sstar_bach_dapm_widgets,
        .num_dapm_widgets = ARRAY_SIZE(sstar_bach_dapm_widgets),
        .dapm_routes      = sstar_bach_audio_map,
        .num_dapm_routes  = ARRAY_SIZE(sstar_bach_audio_map),
        .controls         = sstar_bach_controls,
        .num_controls     = ARRAY_SIZE(sstar_bach_controls),
        .suspend          = sstar_component_suspend,
        .resume           = sstar_component_resume,
    }};

static int sstar_bach_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params,
                                struct snd_soc_dai *dai)
{
    int ret = 0;

    struct sstar_platform_dev *platform      = sstar_dai_to_platform(dai);
    struct sstar_pcm_engine *  ss_pcm        = &platform->pcm_engine;
    struct sstar_pcm_stream *  ss_pcm_stream = NULL;
    MHAL_AUDIO_I2sCfg_t        tI2sCfg       = _sstar_get_config_from_params(params);

    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    {
        ss_pcm_stream = &ss_pcm->tx;
        _sstar_ao_interface_config_set(platform, &tI2sCfg);
    }
    else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
    {
        ss_pcm_stream = &ss_pcm->rx;
        _sstar_ai_interface_config_set(platform, &tI2sCfg);
    }
    if (ss_pcm_stream != NULL)
    {
        memcpy(&(ss_pcm_stream->i2s_cfg), &tI2sCfg, sizeof(MHAL_AUDIO_I2sCfg_t));
    }

    return ret;
}

static int sstar_bach_trigger(struct snd_pcm_substream *substream, int cmd, struct snd_soc_dai *dai)
{
    int ret = 0;

    switch (cmd)
    {
        case SNDRV_PCM_TRIGGER_START:
        case SNDRV_PCM_TRIGGER_RESUME:
        case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
            break;
        case SNDRV_PCM_TRIGGER_SUSPEND:
        case SNDRV_PCM_TRIGGER_STOP:
        case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
            break;
        default:
            ret = -EINVAL;
            break;
    }

    return ret;
}

static int sstar_bach_set_fmt(struct snd_soc_dai *cpu_dai, unsigned int fmt)
{
    struct sstar_platform_dev *platform = sstar_dai_to_platform(cpu_dai);

    platform->format = fmt;

    return 0;
}

static int sstar_bach_setup(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
    struct sstar_platform_dev * platform  = sstar_dai_to_platform(dai);
    struct snd_soc_pcm_runtime *rtd       = asoc_substream_to_rtd(substream);
    struct snd_soc_dai *        codec_dai = NULL;
    int                         mclk0, ret;
    MHAL_MCLK_ID_e              i = 0;
    int                         j = 0;

    // Create a dapm for power_manage to get drv_data.
    struct snd_soc_dapm_widget  w;
    struct snd_soc_dapm_context dapm;
    dapm.component = dai->component;
    w.dapm         = &dapm;
    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    {
        sstar_codec_aof_power_manage(&w, NULL, SND_SOC_DAPM_POST_PMU);
    }
    else
    {
        sstar_codec_aif_power_manage(&w, NULL, SND_SOC_DAPM_POST_PMU);
    }

    mclk0 = mhal_audio_get_dts_value(I2S_MCK0);
    if (mclk0 == 0)
    {
        mclk0 = platform->i2s_mclk0;
    }
    for (i = 0; i < E_MHAL_MCLK_TOTAL; i++)
    {
        ret = mhal_audio_mclk_setting(i, mclk0, TRUE);
        if (ret)
        {
            dev_err(platform->dev, "set %d mclk %d failure\n", i, mclk0);
            return ret;
        }
    }
    dev_info(platform->dev, "mclk codecs_num = %d, mclk = %d\n", rtd->num_codecs, mclk0);
    for_each_rtd_codec_dais(rtd, j, codec_dai)
    {
        dev_info(platform->dev, "codec_dai.name = %s\n", codec_dai->name);
        if ((strncmp("soc:dummy", codec_dai->name, 8)))
        {
            ret = snd_soc_dai_set_sysclk(codec_dai, 0, mclk0, SND_SOC_CLOCK_OUT);
            if (ret)
            {
                dev_err(platform->dev, "set sys mclk[%s] failure mclk0[%d]\n", codec_dai->name, mclk0);
                return ret;
            }
        }
    }

    return 0;
}

static void sstar_dai_bach_shutdown(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
    struct sstar_platform_dev *platform = sstar_dai_to_platform(dai);
    int                        mclk0, ret;

    mclk0 = platform->i2s_mclk0;

    if (mclk0) // sstar dumpy codec & mclk=0 not need
    {
        ret = mhal_audio_mclk_setting(E_MHAL_MCLK_ID_0, mclk0, FALSE);
    }
}

static const struct snd_soc_dai_ops sstar_bach_dai_ops = {
    .hw_params = sstar_bach_hw_params,
    .trigger   = sstar_bach_trigger,
    .set_fmt   = sstar_bach_set_fmt,
    .startup   = sstar_bach_setup,
    .shutdown  = sstar_dai_bach_shutdown,
};

static int sstar_bach_dai_probe(struct snd_soc_dai *dai)
{
    return 0;
}

static struct snd_soc_dai_driver sstar_bach_dai[] = {
    // sstar_bach1
    [0] =
        {
            .probe = sstar_bach_dai_probe,
            .playback =
                {
                    .stream_name  = "Playback",
                    .channels_min = 1,
                    .channels_max = 8,
                    .rates        = SSTAR_BACH_RATES,
                    .formats      = SSTAR_BACH_FORMATS,
                },
            .capture =
                {
                    .stream_name  = "Capture",
                    .channels_min = 1,
                    .channels_max = 16,
                    .rates        = SSTAR_BACH_RATES,
                    .formats      = SSTAR_BACH_FORMATS,
                },
            .ops = &sstar_bach_dai_ops,
        },
};

static bool sstar_bach_writeable_reg(struct device *dev, unsigned int reg)
{
    switch (reg)
    {
        case DAC_DPGA_L_ADDR:
        case DAC_DPGA_R_ADDR:
        case I2S_TXA_01_DPGA_L_ADDR:
        case I2S_TXA_01_DPGA_R_ADDR:
        case ADC_A_DPGA_L_ADDR:
        case ADC_A_DPGA_R_ADDR:
        case DMIC_01_DPGA_L_ADDR:
        case DMIC_01_DPGA_R_ADDR:
        case ECHO_01_DPGA_L_ADDR:
        case ECHO_01_DPGA_R_ADDR:
            return true;
    }
    return false;
}

static bool sstar_bach_readable_reg(struct device *dev, unsigned int reg)
{
    return true;
}

static bool sstar_bach_volatile_reg(struct device *dev, unsigned int reg)
{
    struct sstar_platform_dev *platform = dev_get_drvdata(dev);
    if (platform->suspended == true)
    {
        return false;
    }
    return true;
}

static const struct regmap_config sstar_bach_regmap_config = {
    .reg_bits      = 32,
    .reg_stride    = 4,
    .val_bits      = 32,
    .max_register  = ((AUDIO_MAX_REGISTER_OFFSET - 1) << 2),
    .writeable_reg = sstar_bach_writeable_reg,
    .readable_reg  = sstar_bach_readable_reg,
    .volatile_reg  = sstar_bach_volatile_reg,
    .cache_type    = REGCACHE_RBTREE,
    //.cache_type = REGCACHE_FLAT,
};

static const sstar_aio_dev_t g_ao_paths[] = {
    {.name = "AUX_NULL", .action = 0, .status = false},       {.name = "DAC_SEL", .action = 0, .status = false},
    {.name = "I2S_TXA_SEL", .action = 0, .status = false},    {.name = "ECHO_SEL", .action = 0, .status = false},
    {.name = "AO_VIR_MUX_SEL", .action = 0, .status = false},
};
static const sstar_aio_dev_t g_ai_paths[] = {
    {.name = "AI_MCH_01_SEL", .action = 0, .status = false},  {.name = "AI_MCH_23_SEL", .action = 0, .status = false},
    {.name = "AI_MCH_45_SEL", .action = 0, .status = false},  {.name = "AI_MCH_67_SEL", .action = 0, .status = false},
    {.name = "AI_MCH_89_SEL", .action = 0, .status = false},  {.name = "AI_MCH_AB_SEL", .action = 0, .status = false},
    {.name = "AI_MCH_CD_SEL", .action = 0, .status = false},  {.name = "AI_MCH_EF_SEL", .action = 0, .status = false},
    {.name = "AI_MCH_VIR_SEL", .action = 0, .status = false},
};

static int sstar_bach_probe(struct platform_device *pdev)
{
    struct device_node *       np = pdev->dev.of_node;
    struct sstar_platform_dev *platform;
    int                        ret;
    struct device_node *       np_dma;
    struct resource *          res;
    void __iomem *             regs;
    const struct of_device_id *of_id;
    kernel_ulong_t             index = 0;

    of_id = of_match_device(sstar_bach_dt_match, &pdev->dev);
    if (!of_id)
        return -EINVAL;
    index = (kernel_ulong_t)of_id->data;

    platform = devm_kzalloc(&pdev->dev, sizeof(struct sstar_platform_dev), GFP_KERNEL);
    if (!platform)
    {
        return -ENOMEM;
    }
    memset(platform, 0, sizeof(struct sstar_platform_dev));

    memcpy(platform->ao_path_select, g_ao_paths, sizeof(g_ao_paths));
    memcpy(platform->ai_path_select, g_ai_paths, sizeof(g_ai_paths));

    platform->dev = &pdev->dev;

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (index == 0)
    {
        regs = devm_ioremap_resource(&pdev->dev, res);
    }
    else
    {
        regs = of_iomap(np, 0);
    }

    if (IS_ERR(regs))
        return PTR_ERR(regs);

    platform->regmap = devm_regmap_init_mmio(&pdev->dev, regs, &sstar_bach_regmap_config);

    np_dma                      = of_parse_phandle(np, "sound-dma", 0);
    platform->pcm_engine.irq_id = of_irq_to_resource(np_dma, 0, NULL);
    of_property_read_s32(np_dma, "rdma", &platform->pcm_engine.rdma);
    of_property_read_s32(np_dma, "wdma", &platform->pcm_engine.wdma);
    of_property_read_s32(np, "mclk0-freq", &platform->i2s_mclk0);
    platform->pcm_engine.dev = &pdev->dev;

    platform->runtime_resume  = sstar_bach_runtime_resume;
    platform->runtime_suspend = sstar_bach_runtime_suspend;

    dev_set_drvdata(&pdev->dev, platform);

    mhal_audio_module_init();
    ret = mhal_audio_init(NULL);
    if (ret)
    {
        dev_err(&pdev->dev, "audio hw init failure\n");
        goto err_init_hw;
    }

    pm_runtime_enable(&pdev->dev);
    if (!pm_runtime_enabled(&pdev->dev))
    {
        ret = sstar_bach_runtime_resume(&pdev->dev);
        if (ret)
            goto err_pm_runtime;
    }

    ret = sstar_pcmengine_register(&sstar_bach_component[index]);
    if (ret)
    {
        dev_err(&pdev->dev, "Could not register pcmengine\n");
        goto err_pm_suspend;
    }

    ret = devm_snd_soc_register_component(&pdev->dev, &sstar_bach_component[index], &sstar_bach_dai[index], 1);
    if (ret)
    {
        dev_err(&pdev->dev, "Could not register component\n");
        goto err_pm_suspend;
    }

    return 0;

err_pm_suspend:
    if (!pm_runtime_status_suspended(&pdev->dev))
        sstar_bach_runtime_suspend(&pdev->dev);
err_pm_runtime:
    pm_runtime_disable(&pdev->dev);
    mhal_audio_deinit();
err_init_hw:
    mhal_audio_module_deinit();

    return ret;
}

static int sstar_bach_remove(struct platform_device *pdev)
{
    pm_runtime_disable(&pdev->dev);
    if (!pm_runtime_status_suspended(&pdev->dev))
        sstar_bach_runtime_suspend(&pdev->dev);

    return 0;
}

static void sstar_bach_shutdown(struct platform_device *pdev)
{
    // Close amp power
    mhal_audio_amp_state_set(0, 0);
    mhal_audio_amp_state_set(0, 1);
    mhal_audio_amp_state_set(0, 2);
    mhal_audio_amp_state_set(0, 3);
}

static const struct dev_pm_ops sstar_bach_pm_ops = {
    SET_RUNTIME_PM_OPS(sstar_bach_runtime_suspend, sstar_bach_runtime_resume, NULL)};

static struct platform_driver sstar_bach_driver = {
    .probe    = sstar_bach_probe,
    .remove   = sstar_bach_remove,
    .shutdown = sstar_bach_shutdown,
    .driver =
        {
            .name           = "sstar-bach",
            .pm             = &sstar_bach_pm_ops,
            .of_match_table = sstar_bach_dt_match,
        },
};
module_platform_driver(sstar_bach_driver);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("SigmaStar BACH ASoC platform driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:sstar-bach");
