/*
 * audio_proc.c - Sigmastar
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

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <asm/uaccess.h>

//#include "cam_os_wrapper.h"
#include "audio_proc.h"

//
#include "hal_audio_common.h"
#include "hal_audio_config.h"
#include "hal_audio_sys.h"
#include "hal_audio_types.h"
#include "hal_audio_api.h"
#include "hal_audio_os_api.h"
#include "hal_audio_reg.h"

AudProcInfoAi_t g_audProInfoAiList[E_MHAL_AI_DMA_TOTAL] = {{0}};
AudProcInfoAo_t g_audProInfoAoList[E_MHAL_AO_DMA_TOTAL] = {{0}};

MHAL_SineGen_e g_eAudProInfoCurrSineGen = 0;

static u32             g_audioProcInited = 0;
struct proc_dir_entry *g_pRootAudioDir;

typedef struct
{
    int   argc;
    char *argv[200];
} AudStrConfig_t;

typedef struct
{
    DTS_CONFIG_KEY_e nMode;
    DTS_CONFIG_KEY_e nFmt;
    DTS_CONFIG_KEY_e nChannel;
    DTS_CONFIG_KEY_e nWireMode;
    DTS_CONFIG_KEY_e nPgm;
    DTS_CONFIG_KEY_e nWidth;
    DTS_CONFIG_KEY_e nWckInv;
    DTS_CONFIG_KEY_e nBckInv;
    DTS_CONFIG_KEY_e nSwap;
    DTS_CONFIG_KEY_e nShortFF;
    DTS_CONFIG_KEY_e nTxSlot;
    DTS_CONFIG_KEY_e nMck;
} AudProcI2sDtsKey_t;
#define PROC_CREATE(NAME)                                                                                     \
    static int __Audio##NAME##ProcShow(struct inode *inode, struct file *file)                                \
    {                                                                                                         \
        return single_open(file, _Audio##NAME##ProcShow, NULL);                                               \
    }                                                                                                         \
    static ssize_t __Audio##NAME##ProcWrite(struct file *file, const char *buffer, size_t count, loff_t *pos) \
    {                                                                                                         \
        char           buf[200];                                                                              \
        size_t         len = min(sizeof(buf) - 1, count);                                                     \
        AudStrConfig_t audStr;                                                                                \
                                                                                                              \
        if (copy_from_user(buf, buffer, len))                                                                 \
        {                                                                                                     \
            return count;                                                                                     \
        }                                                                                                     \
        buf[len] = 0;                                                                                         \
        len      = strnlen(buf, len);                                                                         \
        _AudProcFsParsingString((char *)buf, &audStr);                                                        \
                                                                                                              \
        if (audStr.argc < 2)                                                                                  \
        {                                                                                                     \
            goto FAIL;                                                                                        \
        }                                                                                                     \
                                                                                                              \
        if (_Audio##NAME##ProcWrite(&audStr) < 0)                                                             \
        {                                                                                                     \
            goto FAIL;                                                                                        \
        }                                                                                                     \
                                                                                                              \
        return len;                                                                                           \
    FAIL:                                                                                                     \
        _Audio##NAME##ProcHelp();                                                                             \
        return len;                                                                                           \
    }                                                                                                         \
    static const struct proc_ops g_audio##NAME##ProcOps = {                                                   \
        .proc_open    = __Audio##NAME##ProcShow,                                                              \
        .proc_read    = seq_read,                                                                             \
        .proc_lseek   = seq_lseek,                                                                            \
        .proc_release = single_release,                                                                       \
        .proc_write   = __Audio##NAME##ProcWrite,                                                             \
    };
static int _AudioTdmGetDtsKey(AUDIO_TDM_e nTdm, AudProcI2sDtsKey_t *pKeys)
{
    switch (nTdm)
    {
        case E_AUDIO_TDM_RXA:
            pKeys->nMode     = I2SA_RX0_TDM_MODE;
            pKeys->nFmt      = I2SA_RX0_TDM_FMT;
            pKeys->nChannel  = I2SA_RX0_CHANNEL;
            pKeys->nWireMode = I2SA_RX0_TDM_WIREMODE;
            pKeys->nPgm      = I2SA_RX0_TDM_WS_PGM;
            pKeys->nWidth    = I2SA_RX0_TDM_WS_WIDTH;
            pKeys->nWckInv   = I2SA_RX0_TDM_WS_INV;
            pKeys->nSwap     = I2SA_RX0_TDM_CH_SWAP;
            pKeys->nBckInv   = I2SA_RX0_TDM_BCK_INV;
            pKeys->nShortFF  = I2SA_RX0_SHORT_FF_MODE;
            break;
        case E_AUDIO_TDM_TXA:
            pKeys->nMode     = I2SA_TX0_TDM_MODE;
            pKeys->nFmt      = I2SA_TX0_TDM_FMT;
            pKeys->nChannel  = I2SA_TX0_CHANNEL;
            pKeys->nWireMode = I2SA_TX0_TDM_WIREMODE;
            pKeys->nPgm      = I2SA_TX0_TDM_WS_PGM;
            pKeys->nWidth    = I2SA_TX0_TDM_WS_WIDTH;
            pKeys->nWckInv   = I2SA_TX0_TDM_WS_INV;
            pKeys->nSwap     = I2SA_TX0_TDM_CH_SWAP;
            pKeys->nBckInv   = I2SA_TX0_TDM_BCK_INV;
            pKeys->nShortFF  = I2SA_TX0_SHORT_FF_MODE;
            pKeys->nTxSlot   = I2SA_TX0_TDM_ACTIVE_SLOT;
            break;
        default:
            return AIO_NG;
    }
    pKeys->nMck = I2S_MCK0;
    return AIO_OK;
}

static int _AudProcFsSplit(char **arr, char *str, char *del)
{
    char *cur   = str;
    char *token = NULL;
    int   cnt   = 0;

    token = strsep(&cur, del);
    while (token)
    {
        arr[cnt] = token;
        token    = strsep(&cur, del);
        cnt++;
    }

    return cnt;
}

static void _AudProcFsParsingString(char *str, AudStrConfig_t *pstStrCfg)
{
    char del[] = " ";
    int  len;

    pstStrCfg->argc                               = _AudProcFsSplit(pstStrCfg->argv, str, del);
    len                                           = strlen(pstStrCfg->argv[pstStrCfg->argc - 1]);
    pstStrCfg->argv[pstStrCfg->argc - 1][len - 1] = '\0';
}
//

// Tdm
static void _AudioTdmProcHelp(void)
{
    printk("Usage:\n");
    printk("version:0.0 \n");
    printk("echo rx nTdm -shortff   0 >    /proc/audio/tdm => 0/1/2/3  short FF ## nTdm -> 0:TDM_RXA   1:TDM_RXB \n");
    printk("echo rx nTdm -pgm   0 >    /proc/audio/tdm => 0:OFF 1:ON program ## nTdm -> 0:TDM_RXA  1:TDM_RXB  \n");
    printk("echo rx nTdm -width 0 >    /proc/audio/tdm => value, width = value + 1 ## nTdm -> 0:TDM_RXA 1:TDM_RXB  \n");
    printk("echo rx nTdm -inv  0 >    /proc/audio/tdm => 0:Normal 1: Inverse Wck ## nTdm -> 0:TDM_RXA 1:TDM_RXB \n");
    printk("echo rx nTdm -invbck  0 >    /proc/audio/tdm => 0:Normal 1: Inverse Bck ## nTdm -> 0:TDM_RXA 1:TDM_RXB\n");
    printk("echo rx nTdm -swap  0 0 0 0 >  /proc/audio/tdm => Swap 0:OFF 1:ON  ## nTdm -> 0:TDM_RXA 1:TDM_RXB \n");
    printk("echo rx nTdm -fremonitor  0  >  /proc/audio/tdm =>  0:OFF 1:ON ## nTdm -> 0:TDM_RXA 1:TDM_RXB \n");
    printk("echo rx/tx nTdm -swap <0 0 0 0> => DATA0|DATA1|DATA2|DATA3|DATA4|DATA5|DATA6|DATA7\n");
    printk("echo rx/tx nTdm -swap <0 1 0 0> => DATA2|DATA3|DATA0|DATA1|DATA6|DATA7|DATA4|DATA5\n");
    printk("echo rx/tx nTdm -swap <1 0 0 0> => DATA4|DATA5|DATA6|DATA7|DATA0|DATA1|DATA2|DATA3\n");
    printk("echo rx/tx nTdm -swap <1 1 0 0> => DATA6|DATA7|DATA4|DATA5|DATA2|DATA3|DATA0|DATA1\n");
    printk("echo rx/tx nTdm -swap <0 0 1 0> => DATA0|DATA2|DATA4|DATA6|DATA1|DATA3|DATA5|DATA7\n");
    printk("echo rx/tx nTdm -swap <0 1 1 0> => DATA2|DATA0|DATA6|DATA4|DATA3|DATA1|DATA7|DATA5\n");
    printk("echo rx/tx nTdm -swap <1 0 1 0> => DATA4|DATA6|DATA0|DATA2|DATA5|DATA7|DATA1|DATA3\n");
    printk("echo rx/tx nTdm -swap <1 1 1 0> => DATA6|DATA4|DATA2|DATA0|DATA7|DATA5|DATA3|DATA1\n");
    printk(
        "echo rx  nTdm  -mclk n  >  /proc/audio/tdm   0: 0M  1:11.2896M  2:12.288M  3:16.384M  4:19.2M  5:24M  6:48M");
    //
    printk("echo tx nTdm -shortff   0 >  /proc/audio/tdm => 0:OFF 1:ON shortFF ## nTdm -> 0:TDM_TXA  1:TDM_TXB \n");
    printk("echo tx nTdm -pgm   0 >  /proc/audio/tdm => 0:OFF 1:ON program ## nTdm -> 0:TDM_TXA  1:TDM_TXB \n");
    printk("echo tx nTdm -width 0 >  /proc/audio/tdm => value, width = value + 1 ## nTdm -> 0:TDM_TXA 1:TDM_TXB \n");
    printk("echo tx nTdm -inv   0 >  /proc/audio/tdm => 0:Normal 1: Inverse Wck ## nTdm -> 0:TDM_TXA  1:TDM_TXB \n");
    printk("echo tx nTdm -invbck 0 > /proc/audio/tdm => 0:Normal 1: Inverse Bck ## nTdm -> 0:TDM_TXA  1:TDM_TXB\n");
    printk("echo tx nTdm -swap  0 0 0 0 >  /proc/audio/tdm => Swap 0:OFF 1:ON ## nTdm -> 0:TDM_TXA 1:TDM_TXB \n");
    printk("echo tx nTdm -fremonitor  0  >  /proc/audio/tdm =>  0:OFF 1:ON ## nTdm -> 0:TDM_TXA 1:TDM_TXB \n");
    printk("echo tx nTdm -datamute  0  >  /proc/audio/tdm =>  0:OFF 1:ON ## nTdm -> 0:TDM_TXA 1:TDM_TXB \n");
    printk("echo tx nTdm -spdif_bypass  0  >  /proc/audio/tdm =>  0:OFF 1:ON ## nTdm -> 0:TDM_TXA 1:TDM_TXB \n");
    printk("echo tx nTdm -soundbar  0  >  /proc/audio/tdm =>  0:OFF 1:ON ## nTdm -> 0:TDM_TXA 1:TDM_TXB \n");
    printk(
        "echo tx nTdm -slot  0x03 > /proc/audio/tdm => value:0x00 ~ 0xFF (bit0->slot0, bit1->slot1, ...) ## nTdm -> "
        "0:TDM_RXA 1:TDM_TXB\n");
}

static int _AudioTdmProcWrite(AudStrConfig_t *pstStringCfg)
{
    char *             pEnd;
    int                i, tmp, tmp1, tmp2, tmp3, nTdm;
    int                ret  = AIO_NG;
    AudProcI2sDtsKey_t keys = {0};

    if (strcmp(pstStringCfg->argv[0], "rx") == 0)
    {
        nTdm = simple_strtoul(pstStringCfg->argv[1], &pEnd, 10);
    }
    else if (strcmp(pstStringCfg->argv[0], "tx") == 0)
    {
        nTdm = simple_strtoul(pstStringCfg->argv[1], &pEnd, 10) + E_AUDIO_TDM_RX_END + 1;
    }
    else
    {
        goto FAIL;
    }
    ret = _AudioTdmGetDtsKey(nTdm, &keys);
    if (ret == AIO_NG)
    {
        goto FAIL;
    }

    for (i = 2; i < pstStringCfg->argc; i++)
    {
        if (strcmp(pstStringCfg->argv[i], "-pgm") == 0)
        {
            tmp = simple_strtoul(pstStringCfg->argv[i + 1], &pEnd, 10);
            HalAudApiSetDtsValue(keys.nPgm, tmp);
        }
        else if (strcmp(pstStringCfg->argv[i], "-width") == 0)
        {
            tmp = simple_strtoul(pstStringCfg->argv[i + 1], &pEnd, 10);
            HalAudApiSetDtsValue(keys.nWidth, tmp);
        }
        else if (strcmp(pstStringCfg->argv[i], "-inv") == 0)
        {
            tmp = simple_strtoul(pstStringCfg->argv[i + 1], &pEnd, 10);
            HalAudApiSetDtsValue(keys.nWckInv, tmp);
        }
        else if (strcmp(pstStringCfg->argv[i], "-invbck") == 0)
        {
            tmp = simple_strtoul(pstStringCfg->argv[i + 1], &pEnd, 10);
            HalAudApiSetDtsValue(keys.nBckInv, tmp);
        }
        else if (strcmp(pstStringCfg->argv[i], "-swap") == 0)
        {
            tmp  = simple_strtoul(pstStringCfg->argv[i + 1], &pEnd, 10);
            tmp1 = simple_strtoul(pstStringCfg->argv[i + 2], &pEnd, 10);
            tmp2 = simple_strtoul(pstStringCfg->argv[i + 3], &pEnd, 10);
            tmp3 = simple_strtoul(pstStringCfg->argv[i + 4], &pEnd, 10);
            HalAudApiSetDtsValues(keys.nSwap, 0, tmp);
            HalAudApiSetDtsValues(keys.nSwap, 1, tmp1);
            HalAudApiSetDtsValues(keys.nSwap, 2, tmp2);
            HalAudApiSetDtsValues(keys.nSwap, 3, tmp3);
        }
        else if (strcmp(pstStringCfg->argv[i], "-shortff") == 0)
        {
            tmp = simple_strtoul(pstStringCfg->argv[i + 1], &pEnd, 10);
            HalAudApiSetDtsValue(keys.nShortFF, tmp);
        }
        else if (strcmp(pstStringCfg->argv[i], "-mclk") == 0)
        {
            tmp = simple_strtoul(pstStringCfg->argv[i + 1], &pEnd, 10);
            HalAudApiSetDtsValue(keys.nMck, tmp);
        }
        else if (strcmp(pstStringCfg->argv[i], "-slot") == 0)
        {
            tmp = simple_strtoul(pstStringCfg->argv[i + 1], &pEnd, 16);
            HalAudApiSetDtsValue(keys.nTxSlot, tmp);
        }
        else if (strcmp(pstStringCfg->argv[i], "-datamute") == 0)
        {
            tmp = simple_strtoul(pstStringCfg->argv[i + 1], &pEnd, 10);
            HalAudI2sTxDataMute(nTdm, tmp);
        }
        else if (strcmp(pstStringCfg->argv[i], "-soundbar") == 0)
        {
            tmp = simple_strtoul(pstStringCfg->argv[i + 1], &pEnd, 10);
            HalAudApiSetDtsValue(I2SA_TX0_SOUNDBAR_MODE, tmp);
        }
    }
    return 0;
FAIL:
    return -1;
}

static int _AudioTdmProcShow(struct seq_file *m, void *v)
{
    int i = 0;

    AudProcI2sDtsKey_t keys = {0};

    for (i = E_AUDIO_TDM_START; i <= E_AUDIO_TDM_END; i++)
    {
        _AudioTdmGetDtsKey(i, &keys);
        switch (i)
        {
            case E_AUDIO_TDM_RXA:
                seq_printf(m, "TDM RxA\n");
                break;
            case E_AUDIO_TDM_TXA:
                seq_printf(m, "TDM TxA\n");
                break;
            default:
                continue;
                break;
        }
        seq_printf(m, "Mck         = <%d>\n", HalAudApiGetDtsValue(keys.nMck));
        seq_printf(m, "Mode        = <%s>\n", HalAudApiGetDtsValue(keys.nMode) == 1 ? "Master" : "Slave");
        seq_printf(m, "Fmt         = <%s>\n", HalAudApiGetDtsValue(keys.nFmt) == 1 ? "I2S" : "Left");
        seq_printf(m, "WireMode    = <%s>\n", HalAudApiGetDtsValue(keys.nWireMode) == 1 ? "4" : "6");
        seq_printf(m, "Channel     = <%d>\n", HalAudApiGetDtsValue(keys.nChannel));
        seq_printf(m, "Pgm         = <%s>\n", HalAudApiGetDtsValue(keys.nPgm) == 1 ? "On" : "Off");
        seq_printf(m, "Width       = <%d>\n", HalAudApiGetDtsValue(keys.nWidth));
        seq_printf(m, "WckInv      = <%s>\n", HalAudApiGetDtsValue(keys.nWckInv) == 1 ? "Invert" : "Normal");
        seq_printf(m, "BckInv      = <%s>\n", HalAudApiGetDtsValue(keys.nBckInv) == 1 ? "Invert" : "Normal");
        seq_printf(m, "Short FF    = <%d>\n", HalAudApiGetDtsValue(keys.nShortFF));
        seq_printf(m, "ChSwap      = <%d %d %d %d>\n", HalAudApiGetDtsValues(keys.nSwap, 0),
                   HalAudApiGetDtsValues(keys.nSwap, 1), HalAudApiGetDtsValues(keys.nSwap, 2),
                   HalAudApiGetDtsValues(keys.nSwap, 3));
        if (keys.nTxSlot)
        {
            seq_printf(m, "tx active slot = <0x%02x>\n", HalAudApiGetDtsValue(keys.nTxSlot));
        }
        seq_printf(m, "------------------------------\n");
    }
    return 0;
}

// DMA
static int _AudioDmaProcShow(struct seq_file *m, void *v)
{
    seq_printf(m, "\n");

    return 0;
}

static void _AudioDmaProcHelp(void)
{
    printk("Usage:\n");
    printk("version:0.0 \n");
}

static int _AudioDmaProcWrite(AudStrConfig_t *pstStringCfg)
{
    char *pEnd;
    int   i, nTdm, tmp;

    if (strcmp(pstStringCfg->argv[0], "rx") == 0)
    {
        nTdm = simple_strtoul(pstStringCfg->argv[1], &pEnd, 10);

        for (i = 2; i < pstStringCfg->argc; i++)
        {
        }
    }
    else if (strcmp(pstStringCfg->argv[0], "local") == 0)
    {
        if (strcmp(pstStringCfg->argv[1], "rx") == 0)
        {
            tmp = simple_strtoul(pstStringCfg->argv[2], &pEnd, 10);
            printk("local rx test %d us\n", tmp);
            HalAudDmaIntLocalEnable(E_CHIP_AIO_DMA_AI_A, 1);
            HalAudDmaLocalDebug(E_CHIP_AIO_DMA_AI_A, tmp);
            // HalAudDmaIntLocalEnable(E_CHIP_AIO_DMA_AI_A, 0);
        }
        else if (strcmp(pstStringCfg->argv[1], "tx") == 0)
        {
            tmp = simple_strtoul(pstStringCfg->argv[2], &pEnd, 10);
            printk("local tx test %d us\n", tmp);
            HalAudDmaIntLocalEnable(E_CHIP_AIO_DMA_AO_A, 1);
            HalAudDmaLocalDebug(E_CHIP_AIO_DMA_AO_A, tmp);
            // HalAudDmaIntLocalEnable(E_CHIP_AIO_DMA_AO_A, 0);
        }
    }
    else
    {
        goto FAIL;
    }
    return 0;
FAIL:
    return -1;
}

// Sine Gen
static struct stSineGen
{
    const char name[10];
    SINE_GEN_e sineGen;
} g_sinegens[] = {{"AI_DMA_A", E_MHAL_SINEGEN_AI_DMA_A},   {"AI_DMA_B", E_MHAL_SINEGEN_AI_DMA_B},
                  {"AO_DMA_A", E_MHAL_SINEGEN_AO_DMA_A},   {"AO_DMA_B", E_MHAL_SINEGEN_AO_DMA_B},
                  {"DMIC_A", E_MHAL_SINEGEN_AI_IF_DMIC_A}, {"ADC_A", E_MHAL_SINEGEN_AI_IF_AMIC_A},
                  {"ADC_B", E_MHAL_SINEGEN_AI_IF_AMIC_B},  {"I2S_RXA", E_MHAL_SINEGEN_AI_IF_I2S_A},
                  {"I2S_RXB", E_MHAL_SINEGEN_AI_IF_I2S_B}, {"I2S_RXC", E_MHAL_SINEGEN_AI_IF_I2S_C}};

static void _AudioSineGenProcHelp(void)
{
    U8   i        = 0;
    char str[256] = {0};
    printk("Usage: echo nDma [options] > /proc/audio/sinegen\n");
    for (i = 0; i < ARRAY_SIZE(g_sinegens); i++)
    {
        strcat(str, " ");
        strcat(str, g_sinegens[i].name);
    }
    printk("\tnDma    |%s\n", str);
    printk("\t-enable | enable the sinegen 0/1\n");
    printk("\t-freq   | freq 0~15\n");
    printk("\t-gain   | gain 0~15\n");
}

static int _AudioSineGenProcShow(struct seq_file *m, void *v)
{
    BOOL bEn    = 0;
    U8   u8Freq = 0;
    S8   s8Gain = 0;
    U8   i, j;

    seq_printf(m, "\n[AUDIO Sine Gen]\n");

    for (i = 0; i < E_MHAL_SINEGEN_TOTAL; i++)
    {
        HalAudSineGenGetEnable(i, &bEn);

        if (bEn)
        {
            HalAudSineGenGetSetting(i, &u8Freq, &s8Gain);
            seq_printf(m, "--------\n");
            for (j = 0; j < ARRAY_SIZE(g_sinegens); j++)
            {
                if (g_sinegens[j].sineGen == i)
                {
                    seq_printf(m, "Name         = %s\n", g_sinegens[j].name);
                    seq_printf(m, "Freq         = %d\n", u8Freq);
                    seq_printf(m, "Gain         = %d\n", s8Gain);
                }
            }
        }
    }

    return 0;
}

static int _AudioSineGenProcWrite(AudStrConfig_t *audStr)
{
    char *     pEnd;
    U8         i, tmp;
    SINE_GEN_e enSineGen = -1;
    printk("audStr.argv[0] = %s\n", audStr->argv[0]);

    for (i = 0; i < ARRAY_SIZE(g_sinegens); i++)
    {
        if (!strcmp(audStr->argv[0], g_sinegens[i].name))
        {
            enSineGen = g_sinegens[i].sineGen;
            break;
        }
    }
    if (enSineGen < 0)
    {
        goto FAIL;
    }

    for (i = 1; i < audStr->argc; i++)
    {
        if (!strcmp(audStr->argv[i], "-enable"))
        {
            tmp = simple_strtoul(audStr->argv[i + 1], &pEnd, 10);
            HalAudSineGenEnable(enSineGen, tmp);
        }
        else if (!strcmp(audStr->argv[i], "-freq"))
        {
            tmp = simple_strtoul(audStr->argv[i + 1], &pEnd, 10);
            HalAudSineGenSetFreq(enSineGen, tmp);
        }
        else if (!strcmp(audStr->argv[i], "-gain"))
        {
            tmp = simple_strtoul(audStr->argv[i + 1], &pEnd, 10);
            HalAudSineGenSetGain(enSineGen, tmp);
        }
    }
    return 0;
FAIL:
    return -1;
}

// Debug
static struct stDebugLevels
{
    const char name[10];
    U32        level;
} g_dbg_levels[] = {
    {"test", AUDIO_DBG_LEVEL_TEST},   {"dma", AUDIO_DBG_LEVEL_DMA},           {"atop", AUDIO_DBG_LEVEL_ATOP},
    {"i2s", AUDIO_DBG_LEVEL_I2S},     {"dmic", AUDIO_DBG_LEVEL_DMIC},         {"irq", AUDIO_DBG_LEVEL_IRQ},
    {"delay", AUDIO_DBG_LEVEL_DELAY}, {"path", AUDIO_DBG_LEVEL_PATH},         {"power", AUDIO_DBG_LEVEL_POWER},
    {"clock", AUDIO_DBG_LEVEL_CLOCK}, {"dump_pcm", AUDIO_DBG_LEVEL_DUMP_PCM}, {"spdif", AUDIO_DBG_LEVEL_SPDIF}};

static void _AudioDebugProcHelp(void)
{
    U8   i;
    char str[256] = {0};
    printk("Usage: echo nDma [options] > /proc/audio/sinegen\n");
    for (i = 0; i < ARRAY_SIZE(g_dbg_levels); i++)
    {
        strcat(str, " ");
        strcat(str, g_dbg_levels[i].name);
    }
    printk("\tnModule | \t%s\n", str);
    printk("\t-enable | \t enable the debug level 0/1\n");
}

static int _AudioDebugProcShow(struct seq_file *m, void *v)
{
    U8  i;
    U64 level = HalAudApiGetDtsValue(DEBUG_LEVEL);

    seq_printf(m, "\n[AUDIO Debug]\n");
    seq_printf(m, "----------------------\n");
    seq_printf(m, "Log module       Level\n");
    seq_printf(m, "----------------------\n");

    for (i = 0; i < ARRAY_SIZE(g_dbg_levels); i++)
    {
        seq_printf(m, "%s          \t[%s]\n", g_dbg_levels[i].name,
                   (level & g_dbg_levels[i].level) == 0 ? "Disable" : "Enable");
    }

    return 0;
}

static int _AudioDebugProcWrite(AudStrConfig_t *audStr)
{
    char *pEnd;
    U8    i, tmp;
    S32   level = -1;

    if (!strcmp(audStr->argv[0], "reset"))
    {
        HalAudMainInit();
        return 0;
    }

    for (i = 0; i < ARRAY_SIZE(g_dbg_levels); i++)
    {
        if (!strcmp(audStr->argv[0], g_dbg_levels[i].name))
        {
            level = g_dbg_levels[i].level;
            break;
        }
    }
    if (level < 0)
    {
        goto FAIL;
    }

    for (i = 1; i < audStr->argc; i++)
    {
        if (!strcmp(audStr->argv[i], "-enable"))
        {
            U32 tempVal = HalAudApiGetDtsValue(DEBUG_LEVEL);
            tmp         = simple_strtoul(audStr->argv[i + 1], &pEnd, 10);
            if (tmp)
            {
                tempVal |= level;
            }
            else
            {
                tempVal &= ~level;
            }
            printk("tempVal = %d\n", tempVal);
            HalAudApiSetDtsValue(DEBUG_LEVEL, tempVal);
            break;
        }
    }
    return 0;
FAIL:
    return -1;
}

// DMIC
static void _AudioDmicProcHelp(void)
{
    printk("Usage: \n");
    printk("echo extmode [0~1] > /proc/audio/dmic\n");
    printk("echo bckmode [0~3] [0~15] > /proc/audio/dmic\n");
}

static int _AudioDmicProcShow(struct seq_file *m, void *v)
{
    U8    i;
    char *rates[] = {"8K", "16K", "32K", "48K"};

    seq_printf(m, "\n[AUDIO Dmic]\n");
    seq_printf(m, "----------------------\n");
    seq_printf(m, "SampleRate       Mode\n");

    for (i = 0; i < ARRAY_SIZE(rates); i++)
    {
        seq_printf(m, "%s    \t[%d]\n", rates[i], HalAudApiGetDtsValues(DMIC_BCK_MODE, i));
    }
    seq_printf(m, "ExtMode       = [%d]\n", HalAudApiGetDtsValue(DMIC_BCK_EXT_MODE));
    seq_printf(m, "----------------------\n");

    return 0;
}

static int _AudioDmicProcWrite(AudStrConfig_t *audStr)
{
    char *pEnd;
    U8    i, tmp, tmp1;

    for (i = 0; i < audStr->argc; i++)
    {
        if (!strcmp(audStr->argv[i], "extmode"))
        {
            tmp = simple_strtoul(audStr->argv[i + 1], &pEnd, 10);
            HalAudApiSetDtsValue(DMIC_BCK_EXT_MODE, !!tmp);
            break;
        }
        else if (!strcmp(audStr->argv[i], "bckmode"))
        {
            tmp  = simple_strtoul(audStr->argv[i + 1], &pEnd, 10);
            tmp1 = simple_strtoul(audStr->argv[i + 2], &pEnd, 10);
            HalAudApiSetDtsValues(DMIC_BCK_MODE, tmp, tmp1);
        }
    }
    return 0;
}

// Clock
static void _AudioClockProcHelp(void)
{
    printk("Usage: \n");
    printk("echo offset [-10~10]  > /proc/audio/clock\n");
    printk("echo reset 1  > /proc/audio/clock\n");
}

static int _AudioClockProcShow(struct seq_file *m, void *v)
{
    int value_lo = HalBachReadReg(E_BACH_PLL_BANK, 0x16);
    int value_hi = HalBachReadReg(E_BACH_PLL_BANK, 0x18);
    U32 value    = (value_hi << 16) | value_lo;
    seq_printf(m, "ClockRate = [%d]\n", value);

    return 0;
}

static int _AudioClockProcWrite(AudStrConfig_t *audStr)
{
    char *pEnd;
    int   i, tmp;

    for (i = 0; i < audStr->argc; i++)
    {
        if (!strcmp(audStr->argv[i], "offset"))
        {
            int value_lo = HalBachReadReg(E_BACH_PLL_BANK, 0x16);
            int value_hi = HalBachReadReg(E_BACH_PLL_BANK, 0x18);
            U32 value    = (value_hi << 16) | value_lo;
            tmp          = simple_strtol(audStr->argv[i + 1], &pEnd, 10);
            if (tmp > 10 || tmp < -10)
            {
                _AudioClockProcHelp();
                return 0;
            }

            value += tmp;

            HalBachWriteReg(E_BACH_PLL_BANK, 0x16, 0xFFFF, value & 0xFFFF);
            HalBachWriteReg(E_BACH_PLL_BANK, 0x18, 0xFFFF, value >> 16);
        }
        else if (!strcmp(audStr->argv[i], "reset"))
        {
            // Reset to the default value.
            HalBachWriteReg(E_BACH_PLL_BANK, 0x16, 0xFFFF, 0x999A);
            HalBachWriteReg(E_BACH_PLL_BANK, 0x18, 0xFFFF, 0x15);
        }
    }
    return 0;
}

//

PROC_CREATE(SineGen);
PROC_CREATE(Debug);
PROC_CREATE(Dma);
PROC_CREATE(Dmic);
PROC_CREATE(Tdm);
PROC_CREATE(Clock);

static AudProcDir_t g_proc_dir[] = {
    PROC_CONFIG("debug", &g_audioDebugProcOps), PROC_CONFIG("sinegen", &g_audioSineGenProcOps),
    PROC_CONFIG("tdm", &g_audioTdmProcOps),     PROC_CONFIG("dma", &g_audioDmaProcOps),
    PROC_CONFIG("dmic", &g_audioDmicProcOps),   PROC_CONFIG("clock", &g_audioClockProcOps),
};
//
int AudioProcInit(void)
{
    int i = 0;

    if (g_audioProcInited)
    {
        return AIO_OK;
    }

    g_audioProcInited = 1;

    g_pRootAudioDir = proc_mkdir(PROC_DIR_NAME, NULL);
    if (!g_pRootAudioDir)
    {
        printk("[AUDIO PROC] proc_mkdir fail\n");
        return AIO_OK;
    }

    for (i = 0; i < ARRAY_SIZE(g_proc_dir); i++)
    {
        proc_create(g_proc_dir[i].name, S_IRUGO, g_pRootAudioDir, g_proc_dir[i].ops);
    }

    return AIO_OK;
}

int AudioProcDeInit(void)
{
    int i = 0;

    if (!g_audioProcInited)
    {
        goto FAIL;
    }

    g_audioProcInited = 0;

    printk("%s %d\n", __FUNCTION__, __LINE__);

    for (i = 0; i < ARRAY_SIZE(g_proc_dir); i++)
    {
        remove_proc_entry(g_proc_dir[i].name, g_pRootAudioDir);
    }
    remove_proc_entry(PROC_DIR_NAME, NULL);

    return AIO_OK;

FAIL:

    return AIO_NG;
}
