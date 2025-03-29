/*
* sstar_dummy_codec.c- Sigmastar
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
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <sound/soc.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <sound/jack.h>
#include <linux/gpio.h>

#define DRV_NAME "sstar-dummy-codec1"

#define CODEC_RATES    SNDRV_PCM_RATE_8000_192000
#define CODEC_FORMATS    (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S24_3LE | SNDRV_PCM_FMTBIT_S32_LE | SNDRV_PCM_FMTBIT_FLOAT)
#define HEADSET_INSERT    0x1
#define HEADSET_PULL_OUT    0x0

struct sstar_codec_priv {
    struct snd_soc_component *component;
    struct regmap *regmap;
    struct clk *mclk;
    struct device dev;
    u32 headset_det_gpio;
    u32 irq_id;
    int plugged;
    struct work_struct jack_work;
    struct snd_soc_jack *jack;
};

static const struct snd_soc_dapm_widget sstar_dummy_widgets[] = {
    SND_SOC_DAPM_OUTPUT("dummy-out"),
    SND_SOC_DAPM_INPUT("dummy-in"),
};

static int sstar_set_jack(struct snd_soc_component *component,
                          struct snd_soc_jack *jack, void *data)
{
    struct sstar_codec_priv *codec = snd_soc_component_get_drvdata(component);

    if(codec)
    {
        codec->jack = jack;
        codec->component = component;
    }
    return 0;
}

static struct snd_soc_component_driver soc_sstar_dummy_codec = {
    .dapm_widgets = sstar_dummy_widgets,
    .set_jack = sstar_set_jack,
    .num_dapm_widgets = ARRAY_SIZE(sstar_dummy_widgets),
};

static const struct of_device_id sstar_dummy_codec_dt_ids[] = {
    { .compatible = "sstar,dummy-codec1", .data = (void *)0 },
    { .compatible = "sstar,dummy-codec2", .data = (void *)1 },
    { .compatible = "sstar,dummy-codec3", .data = (void *)2 },
    {},
};

static struct snd_soc_dai_driver soc_sstar_dummy_codec_dai[] = {
    {
        .name = "sstar-dummy-codec1",
        .playback = {
            .stream_name    = "Playback",
            .channels_min    = 1,
            .channels_max    = 8,
            .rates        = CODEC_RATES,
            .formats    = CODEC_FORMATS,
        },
        .capture = {
            .stream_name    = "Capture",
            .channels_min    = 1,
            .channels_max    = 16,
            .rates        = CODEC_RATES,
            .formats    = CODEC_FORMATS,
        },
    },
    {
        .name = "sstar-dummy-codec2",
        .playback = {
            .stream_name    = "Playback",
            .channels_min    = 1,
            .channels_max    = 8,
            .rates        = CODEC_RATES,
            .formats    = CODEC_FORMATS,
        },
        .capture = {
            .stream_name    = "Capture",
            .channels_min    = 1,
            .channels_max    = 16,
            .rates        = CODEC_RATES,
            .formats    = CODEC_FORMATS,
        },
    },
    {
        .name = "sstar-dummy-codec3",
        .playback = {
            .stream_name    = "Playback",
            .channels_min    = 1,
            .channels_max    = 8,
            .rates        = CODEC_RATES,
            .formats    = CODEC_FORMATS,
        },
        .capture = {
            .stream_name    = "Capture",
            .channels_min    = 1,
            .channels_max    = 16,
            .rates        = CODEC_RATES,
            .formats    = CODEC_FORMATS,
        },
    }
};

static int sstar_dummy_codec_probe(struct platform_device *pdev)
{
    const struct of_device_id *of_id;
    kernel_ulong_t             index = 0;

    of_id = of_match_device(sstar_dummy_codec_dt_ids, &pdev->dev);
    if (!of_id)
        return -EINVAL;
    index = (kernel_ulong_t)of_id->data;

    return devm_snd_soc_register_component(&pdev->dev,
            &soc_sstar_dummy_codec,
            &soc_sstar_dummy_codec_dai[index], 1);
}

MODULE_DEVICE_TABLE(of, sstar_dummy_codec_dt_ids);

static struct platform_driver sstar_dummy_codec_driver = {
    .probe = sstar_dummy_codec_probe,
    .driver = {
        .name        = DRV_NAME,
        .of_match_table    = sstar_dummy_codec_dt_ids,
    },
};

module_platform_driver(sstar_dummy_codec_driver);

MODULE_AUTHOR("xiao.lin@sigmastar.com.cn");
MODULE_DESCRIPTION("SigmaStar dummy codec driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRV_NAME);
