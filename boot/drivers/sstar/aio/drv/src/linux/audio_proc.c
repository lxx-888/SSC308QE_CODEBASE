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

// Hpf
static void _AudTdmProcHelp(void)
{
    printk("Usage:\n");
    printk("version:2.5 \n");
    printk(
        "echo rx nTdm -pgm   0 >    /proc/audio/tdm => 0:OFF 1:ON program ## nTdm -> 0:TDM_RXA  1:TDM_RXB  2:TDM_RXC  "
        "3:TDM_RXD \n");
    printk(
        "echo rx nTdm -width 0 >    /proc/audio/tdm => value, width = value + 1 ## nTdm -> 0:TDM_RXA  1:TDM_RXB  "
        "2:TDM_RXC 3:TDM_RXD \n");
    printk(
        "echo rx nTdm -inv   0 >    /proc/audio/tdm => 0:Normal 1: Inverse Wck ## nTdm -> 0:TDM_RXA  1:TDM_RXB  "
        "2:TDM_RXC  "
        "3:TDM_RXD  \n");
    printk(
        "echo rx nTdm -invbck   0 >    /proc/audio/tdm => 0:Normal 1: Inverse Bck ## nTdm -> 0:TDM_RXA  1:TDM_RXB  "
        "2:TDM_RXC  "
        "3:TDM_RXD  \n");
    printk(
        "echo rx nTdm -swap  0 0 0>  /proc/audio/tdm => Swap 0:OFF 1:ON  ## nTdm -> 0:TDM_RXA  1:TDM_RXB  2:TDM_RXC  "
        "3:TDM_RXD  \n");
    printk("echo rx/tx nTdm -swap <0 0 0> => DATA0|DATA1|DATA2|DATA3|DATA4|DATA5|DATA6|DATA7 \n");
    printk("echo rx/tx nTdm -swap <0 1 0> => DATA2|DATA3|DATA0|DATA1|DATA6|DATA7|DATA4|DATA5 \n");
    printk("echo rx/tx nTdm -swap <1 0 0> => DATA4|DATA5|DATA6|DATA7|DATA0|DATA1|DATA2|DATA3 \n");
    printk("echo rx/tx nTdm -swap <1 1 0> => DATA6|DATA7|DATA4|DATA5|DATA2|DATA3|DATA0|DATA1 \n");
    printk("echo rx/tx nTdm -swap <0 0 1> => DATA0|DATA2|DATA4|DATA6|DATA1|DATA3|DATA5|DATA7 \n");
    printk("echo rx/tx nTdm -swap <0 1 1> => DATA2|DATA0|DATA6|DATA4|DATA3|DATA1|DATA7|DATA5 \n");
    printk("echo rx/tx nTdm -swap <1 0 1> => DATA4|DATA6|DATA0|DATA2|DATA5|DATA7|DATA1|DATA3 \n");
    printk("echo rx/tx nTdm -swap <1 1 1> => DATA6|DATA4|DATA2|DATA0|DATA7|DATA5|DATA3|DATA1 \n");
    printk("echo tx nTdm -pgm   0 >    /proc/audio/tdm => 0:OFF 1:ON program ## nTdm -> 0:TDM_TXA  1:TDM_TXB \n");
    printk("echo tx nTdm -width 0 >    /proc/audio/tdm => value, width = value + 1 ## nTdm -> 0:TDM_TXA  1:TDM_TXB \n");
    printk("echo tx nTdm -inv   0 >    /proc/audio/tdm => 0:Normal 1: Inverse Wck ## nTdm -> 0:TDM_TXA  1:TDM_TXB \n");
    printk(
        "echo tx nTdm -invbck   0 >    /proc/audio/tdm => 0:Normal 1: Inverse Bck ## nTdm -> 0:TDM_TXA  1:TDM_TXB \n");
    printk("echo tx nTdm -swap  0 0 0>  /proc/audio/tdm => Swap 0:OFF 1:ON ## nTdm -> 0:TDM_TXA  1:TDM_TXB \n");
    printk(
        "echo tx nTdm -slot  0x03 > /proc/audio/tdm => value:0x00 ~ 0xFF (bit0->slot0, bit1->slot1, ...) ## nTdm -> "
        "0:TDM_RXA  "
        "1:TDM_RXB  2:TDM_RXC  3:TDM_RXD  4:TDM_TXA  5:TDM_TXB\n");
    printk(
        "echo rx nTdm -i2s_rx_mode   0 >    /proc/audio/tdm => 0:OFF 1:ON program ## nTdm ->0:TDM_RXA  1:TDM_RXB  "
        "2:TDM_RXC  3:TDM_RXD\n");
}

static void _AudTdmProcStore(AudStrConfig_t *pstStringCfg)
{
    char *pEnd;
    int   i, tmp, tmp1, tmp2, nTdm;

    if (pstStringCfg->argc < 2)
    {
        _AudTdmProcHelp();
        return;
    }
    if (strcmp(pstStringCfg->argv[0], "rx") == 0)
    {
        nTdm = simple_strtoul(pstStringCfg->argv[1], &pEnd, 10);

        for (i = 2; i < pstStringCfg->argc; i++)
        {
            if (strcmp(pstStringCfg->argv[i], "-pgm") == 0)
            {
                tmp = simple_strtoul(pstStringCfg->argv[i + 1], &pEnd, 10);
                HalAudI2sSaveTdmWsPgm(nTdm, tmp);
            }
            else if (strcmp(pstStringCfg->argv[i], "-width") == 0)
            {
                tmp = simple_strtoul(pstStringCfg->argv[i + 1], &pEnd, 10);
                HalAudI2sSaveTdmWsWidth(nTdm, tmp);
            }
            else if (strcmp(pstStringCfg->argv[i], "-inv") == 0)
            {
                tmp = simple_strtoul(pstStringCfg->argv[i + 1], &pEnd, 10);
                HalAudI2sSaveTdmWsInv(nTdm, tmp);
            }
            else if (strcmp(pstStringCfg->argv[i], "-invbck") == 0)
            {
                tmp = simple_strtoul(pstStringCfg->argv[i + 1], &pEnd, 10);
                HalAudI2sSaveTdmBckInv(nTdm, tmp);
            }
            else if (strcmp(pstStringCfg->argv[i], "-swap") == 0)
            {
                tmp  = simple_strtoul(pstStringCfg->argv[i + 1], &pEnd, 10);
                tmp1 = simple_strtoul(pstStringCfg->argv[i + 2], &pEnd, 10);
                tmp2 = simple_strtoul(pstStringCfg->argv[i + 3], &pEnd, 10);
                HalAudI2sSaveTdmChSwap(nTdm, tmp, tmp1, tmp2);
            }
            else if (strcmp(pstStringCfg->argv[i], "-i2s_rx_mode") == 0)
            {
                tmp = simple_strtoul(pstStringCfg->argv[i + 1], &pEnd, 10);
                HalAudI2sSetRxMode(tmp);
            }
        }
    }
    else if (strcmp(pstStringCfg->argv[0], "tx") == 0)
    {
        nTdm = simple_strtoul(pstStringCfg->argv[1], &pEnd, 10) + E_AUDIO_TDM_RX_END + 1;

        for (i = 2; i < pstStringCfg->argc; i++)
        {
            if (strcmp(pstStringCfg->argv[i], "-pgm") == 0)
            {
                tmp = simple_strtoul(pstStringCfg->argv[i + 1], &pEnd, 10);
                HalAudI2sSaveTdmWsPgm(nTdm, tmp);
            }
            else if (strcmp(pstStringCfg->argv[i], "-width") == 0)
            {
                tmp = simple_strtoul(pstStringCfg->argv[i + 1], &pEnd, 10);
                HalAudI2sSaveTdmWsWidth(nTdm, tmp);
            }
            else if (strcmp(pstStringCfg->argv[i], "-inv") == 0)
            {
                tmp = simple_strtoul(pstStringCfg->argv[i + 1], &pEnd, 10);
                HalAudI2sSaveTdmWsInv(nTdm, tmp);
            }
            else if (strcmp(pstStringCfg->argv[i], "-invbck") == 0)
            {
                tmp = simple_strtoul(pstStringCfg->argv[i + 1], &pEnd, 10);
                HalAudI2sSaveTdmBckInv(nTdm, tmp);
            }
            else if (strcmp(pstStringCfg->argv[i], "-swap") == 0)
            {
                tmp  = simple_strtoul(pstStringCfg->argv[i + 1], &pEnd, 10);
                tmp1 = simple_strtoul(pstStringCfg->argv[i + 2], &pEnd, 10);
                tmp2 = simple_strtoul(pstStringCfg->argv[i + 3], &pEnd, 10);
                HalAudI2sSaveTdmChSwap(nTdm, tmp, tmp1, tmp2);
            }
            else if (strcmp(pstStringCfg->argv[i], "-slot") == 0)
            {
                tmp = simple_strtoul(pstStringCfg->argv[i + 1], &pEnd, 16);
                HalAudI2sSaveTxTdmActiveSlot(tmp);
            }
        }
    }
    else
    {
        _AudTdmProcHelp();
        return;
    }
}

static int _AudioTdmProcShow(struct seq_file *m, void *v)
{
    unsigned char txSlot;
    int           i       = 0;
    long *        nPgm    = NULL;
    long *        nWidth  = NULL;
    long *        nInv    = NULL;
    long *        nBckInv = NULL;
    long *        nSwap   = NULL;
    long *        nRxMode = NULL;

    nPgm    = HalAudI2sGetTdmWsPgm();
    nWidth  = HalAudI2sGetTdmWsWidth();
    nInv    = HalAudI2sGetTdmWsInv();
    nSwap   = HalAudI2sGetTdmChSwap();
    nBckInv = HalAudI2sGetTdmBckInv();
    nRxMode = HalAudI2sGetRxMode();

    HalAudI2sGetTxTdmActiveSlot(&txSlot);

    for (i = E_AUDIO_TDM_START; i <= E_AUDIO_TDM_END; i++)
    {
        switch (i)
        {
            case E_AUDIO_TDM_RXA:
                seq_printf(m, "TDM RxA\n");
                break;
            case E_AUDIO_TDM_RXB:
                seq_printf(m, "TDM RxB\n");
                break;
            case E_AUDIO_TDM_RXC:
                seq_printf(m, "TDM RxC\n");
                break;
            case E_AUDIO_TDM_RXD:
                seq_printf(m, "TDM RxD\n");
                break;
            case E_AUDIO_TDM_TXA:
                seq_printf(m, "TDM TxA\n");
                break;
            case E_AUDIO_TDM_TXB:
                seq_printf(m, "TDM TxB\n");
                break;
            default:
                seq_printf(m, "TDM  failure\n");
                goto FAIL;
                break;
        }

        seq_printf(m, "pgm         = <%ld>\n", nPgm[i]);
        seq_printf(m, "width       = <%ld>\n", nWidth[i]);
        seq_printf(m, "Wck inv     = <%ld>\n", nInv[i]);
        seq_printf(m, "Bck inv     = <%ld>\n", nBckInv[i]);
        seq_printf(m, "ch-swap     = <%ld %ld %ld>\n", (nSwap[i] & ~0xfd) >> 1, (nSwap[i] & ~0xfe),
                   (nSwap[i] & ~0xfb) >> 2);
        if (i > E_AUDIO_TDM_TXA)
        {
            seq_printf(m, "tx active slot = <0x%02x>\n", txSlot);
        }
        if (i == E_AUDIO_TDM_RXC || i == E_AUDIO_TDM_RXD)
        {
            seq_printf(m, "rx mode = <%ld>\n", *nRxMode);
        }
        seq_printf(m, "\n");
    }

FAIL:
    return 0;
}

static ssize_t _AudioTdmProcWrite(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    char           buf[200];
    size_t         len = min(sizeof(buf) - 1, count);
    AudStrConfig_t audStr;

    if (copy_from_user(buf, buffer, len))
    {
        return count;
    }

    buf[len] = 0;
    len      = strnlen(buf, len);
    _AudProcFsParsingString((char *)buf, &audStr);

    _AudTdmProcStore(&audStr);

    return len;
}

static int _AudioTdmProcOpen(struct inode *inode, struct file *file)
{
    return single_open(file, _AudioTdmProcShow, NULL);
}

static const struct proc_ops g_audioTdmProcOps = {
    .proc_open    = _AudioTdmProcOpen,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
    .proc_write   = _AudioTdmProcWrite,
};

//
static int _AudioAiProcShow(struct seq_file *m, void *v)
{
#if 0
    int i = 0, dma = 0;
    unsigned int nSampleRate = 0;

    seq_printf(m, "\n[AUDIO AI]\n");

    for (i = 0; i < E_MHAL_AI_DMA_TOTAL; i++)
    {
        dma = card.ai_devs[i].info->dma;
        HalAudDmaGetRate(dma, NULL, &nSampleRate);

        seq_printf(m, "--------\n");
        seq_printf(m, "Dev ID       = %d\n", i);
        seq_printf(m, "Enable       = %d\n", g_audProInfoAiList[i].enable);
        seq_printf(m, "SampleRate   = %d\n", nSampleRate);
        seq_printf(m, "Channel      = %d\n", g_audProInfoAiList[i].channels);
        seq_printf(m, "BitWidth     = %d\n", g_audProInfoAiList[i].bit_width);
    }
#endif

    return 0;
}

static ssize_t _AudioAiProcWrite(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    char          buf[] = "0x00000000\n";
    size_t        len   = min(sizeof(buf) - 1, count);
    unsigned long val;

    if (copy_from_user(buf, buffer, len))
    {
        return count;
    }

    buf[len] = 0;

    if (sscanf(buf, "%li", &val) != 1)
    {
        printk(": %s is not in hex or decimal form.\n", buf);
    }
    else
    {
        if (val >= E_MHAL_AI_DMA_TOTAL)
        {
            printk(": %lu is invalid AI ID.\n", val);
        }
        else
        {
            // gAudioAiProcDevId = val;
        }
    }

    return strnlen(buf, len);
}

static int _AudioAiProcOpen(struct inode *inode, struct file *file)
{
    return single_open(file, _AudioAiProcShow, NULL);
}

static const struct proc_ops g_audioAiProcOps = {
    .proc_open    = _AudioAiProcOpen,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
    .proc_write   = _AudioAiProcWrite,
};

//
static int _AudioAoProcShow(struct seq_file *m, void *v)
{
#if 0
    int i = 0, dma = 0;
    unsigned int nSampleRate = 0;

    seq_printf(m, "\n[AUDIO AO]\n");

    for (i = 0; i < E_MHAL_AO_DMA_TOTAL; i++)
    {
        dma = card.ao_devs[i].info->dma;
        HalAudDmaGetRate(dma, NULL, &nSampleRate);

        seq_printf(m, "--------\n");
        seq_printf(m, "Dev ID       = %d\n", i);
        seq_printf(m, "Enable       = %d\n", g_audProInfoAoList[i].enable);
        seq_printf(m, "SampleRate   = %d\n", nSampleRate);
        seq_printf(m, "Channel      = %d\n", g_audProInfoAoList[i].channels);
        seq_printf(m, "BitWidth     = %d\n", g_audProInfoAoList[i].bit_width);
    }
#endif

    return 0;
}

static ssize_t _AudioAoProcWrite(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    char          buf[] = "0x00000000\n";
    size_t        len   = min(sizeof(buf) - 1, count);
    unsigned long val;

    if (copy_from_user(buf, buffer, len))
    {
        return count;
    }

    buf[len] = 0;

    if (sscanf(buf, "%li", &val) != 1)
    {
        printk(": %s is not in hex or decimal form.\n", buf);
    }
    else
    {
        if (val >= E_MHAL_AO_DMA_TOTAL)
        {
            printk(": %lu is invalid AO ID.\n", val);
        }
        else
        {
            // gAudioAoProcDevId = val;
        }
    }

    return strnlen(buf, len);
}

static int _AudioAoProcOpen(struct inode *inode, struct file *file)
{
    return single_open(file, _AudioAoProcShow, NULL);
}

static const struct proc_ops g_audioAoProcOps = {
    .proc_open    = _AudioAoProcOpen,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
    .proc_write   = _AudioAoProcWrite,
};

// Sine Gen
static int _AudioSineGenProcShow(struct seq_file *m, void *v)
{
    BOOL bEn    = 0;
    U8   u8Freq = 0;
    S8   s8Gain = 0;

    seq_printf(m, "\n[AUDIO Sine Gen]\n");

    HalAudDmaSineGenGetEnable(g_eAudProInfoCurrSineGen, &bEn);
    HalAudDmaSineGenGetSetting(g_eAudProInfoCurrSineGen, &u8Freq, &s8Gain);

    seq_printf(m, "--------\n");
    seq_printf(m, "SineGen ID   = %d\n", g_eAudProInfoCurrSineGen);
    seq_printf(m, "Enable       = %d\n", bEn);
    seq_printf(m, "Freq         = %d\n", u8Freq);
    seq_printf(m, "Gain         = %d\n", s8Gain);

    return 0;
}

static ssize_t _AudioSineGenProcWrite(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    char          buf[] = "0x00000000\n";
    size_t        len   = min(sizeof(buf) - 1, count);
    unsigned long val;

    if (copy_from_user(buf, buffer, len))
    {
        return count;
    }

    buf[len] = 0;

    if (sscanf(buf, "%li", &val) != 1)
    {
        printk(": %s is not in hex or decimal form.\n", buf);
    }
    else
    {
        g_eAudProInfoCurrSineGen = val;
    }

    return strnlen(buf, len);
}

static int _AudioSineGenProcOpen(struct inode *inode, struct file *file)
{
    return single_open(file, _AudioSineGenProcShow, NULL);
}

static const struct proc_ops g_audioSineGenProcOps = {
    .proc_open    = _AudioSineGenProcOpen,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
    .proc_write   = _AudioSineGenProcWrite,
};

// Sine Gen Enable
static int _AudioSineGenEnableProcShow(struct seq_file *m, void *v)
{
    BOOL bEn    = 0;
    U8   u8Freq = 0;
    S8   s8Gain = 0;

    seq_printf(m, "\n[AUDIO Sine Gen Enable]\n");

    HalAudDmaSineGenGetEnable(g_eAudProInfoCurrSineGen, &bEn);
    HalAudDmaSineGenGetSetting(g_eAudProInfoCurrSineGen, &u8Freq, &s8Gain);

    seq_printf(m, "--------\n");
    seq_printf(m, "SineGen ID   = %d\n", g_eAudProInfoCurrSineGen);
    seq_printf(m, "Enable       = %d\n", bEn);
    seq_printf(m, "Freq         = %d\n", u8Freq);
    seq_printf(m, "Gain         = %d\n", s8Gain);

    return 0;
}

static ssize_t _AudioSineGenEnableProcWrite(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    char          buf[] = "0x00000000\n";
    size_t        len   = min(sizeof(buf) - 1, count);
    unsigned long val;

    if (copy_from_user(buf, buffer, len))
    {
        return count;
    }

    buf[len] = 0;

    if (sscanf(buf, "%li", &val) != 1)
    {
        printk(": %s is not in hex or decimal form.\n", buf);
    }
    else
    {
        if (val >= 2)
        {
            printk(": %lu is invalid parameter.\n", val);
        }
        else
        {
            HalAudDmaSineGenEnable(g_eAudProInfoCurrSineGen, val);
        }
    }

    return strnlen(buf, len);
}

static int _AudioSineGenEnableProcOpen(struct inode *inode, struct file *file)
{
    return single_open(file, _AudioSineGenEnableProcShow, NULL);
}

static const struct proc_ops g_audioSineGenEnableProcOps = {
    .proc_open    = _AudioSineGenEnableProcOpen,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
    .proc_write   = _AudioSineGenEnableProcWrite,
};

// Sine Gen Freq
static int _AudioSineGenFreqProcShow(struct seq_file *m, void *v)
{
    BOOL bEn    = 0;
    U8   u8Freq = 0;
    S8   s8Gain = 0;

    seq_printf(m, "\n[AUDIO Sine Gen Freq]\n");

    HalAudDmaSineGenGetEnable(g_eAudProInfoCurrSineGen, &bEn);
    HalAudDmaSineGenGetSetting(g_eAudProInfoCurrSineGen, &u8Freq, &s8Gain);

    seq_printf(m, "--------\n");
    seq_printf(m, "SineGen ID   = %d\n", g_eAudProInfoCurrSineGen);
    seq_printf(m, "Enable       = %d\n", bEn);
    seq_printf(m, "Freq         = %d\n", u8Freq);
    seq_printf(m, "Gain         = %d\n", s8Gain);

    return 0;
}

static ssize_t _AudioSineGenFreqProcWrite(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    char          buf[] = "0x00000000\n";
    size_t        len   = min(sizeof(buf) - 1, count);
    unsigned long val;
    int           ret = AIO_OK;

    if (copy_from_user(buf, buffer, len))
    {
        return count;
    }

    buf[len] = 0;

    if (sscanf(buf, "%li", &val) != 1)
    {
        printk(": %s is not in hex or decimal form.\n", buf);
    }
    else
    {
        ret |= HalAudDmaSineGenSetFreq(g_eAudProInfoCurrSineGen, (U8)val);
        if (ret != AIO_OK)
        {
            printk(": SineGen SetFreq Fail.\n");
        }
    }

    return strnlen(buf, len);
}

static int _AudioSineGenFreqProcOpen(struct inode *inode, struct file *file)
{
    return single_open(file, _AudioSineGenFreqProcShow, NULL);
}

static const struct proc_ops g_audioSineGenFreqProcOps = {
    .proc_open    = _AudioSineGenFreqProcOpen,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
    .proc_write   = _AudioSineGenFreqProcWrite,
};

// Sine Gen Gain
static int _AudioSineGenGainProcShow(struct seq_file *m, void *v)
{
    BOOL bEn    = 0;
    U8   u8Freq = 0;
    S8   s8Gain = 0;

    seq_printf(m, "\n[AUDIO Sine Gen Gain]\n");

    HalAudDmaSineGenGetEnable(g_eAudProInfoCurrSineGen, &bEn);
    HalAudDmaSineGenGetSetting(g_eAudProInfoCurrSineGen, &u8Freq, &s8Gain);

    seq_printf(m, "--------\n");
    seq_printf(m, "SineGen ID   = %d\n", g_eAudProInfoCurrSineGen);
    seq_printf(m, "Enable       = %d\n", bEn);
    seq_printf(m, "Freq         = %d\n", u8Freq);
    seq_printf(m, "Gain         = %d\n", s8Gain);

    return 0;
}

static ssize_t _AudioSineGenGainProcWrite(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    char          buf[] = "0x00000000\n";
    size_t        len   = min(sizeof(buf) - 1, count);
    unsigned long val;
    int           ret = AIO_OK;

    if (copy_from_user(buf, buffer, len))
    {
        return count;
    }

    buf[len] = 0;

    if (sscanf(buf, "%li", &val) != 1)
    {
        printk(": %s is not in hex or decimal form.\n", buf);
    }
    else
    {
        ret |= HalAudDmaSineGenSetGain(g_eAudProInfoCurrSineGen, (S8)val);
        if (ret != AIO_OK)
        {
            printk(": SineGen SetGain Fail.\n");
        }
    }

    return strnlen(buf, len);
}

static int _AudioSineGenGainProcOpen(struct inode *inode, struct file *file)
{
    return single_open(file, _AudioSineGenGainProcShow, NULL);
}

static const struct proc_ops g_audioSineGenGainProcOps = {
    .proc_open    = _AudioSineGenGainProcOpen,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
    .proc_write   = _AudioSineGenGainProcWrite,
};

//
int AudioProcInit(void)
{
    struct proc_dir_entry *pde;

    if (g_audioProcInited)
    {
        goto FAIL;
    }

    g_audioProcInited = 1;

    printk("%s %d\n", __FUNCTION__, __LINE__); //

    g_pRootAudioDir = proc_mkdir("audio", NULL);
    if (!g_pRootAudioDir)
    {
        printk("[AUDIO PROC] proc_mkdir fail\n");
        goto FAIL;
    }

    //
    pde = proc_create("ai", S_IRUGO, g_pRootAudioDir, &g_audioAiProcOps);
    if (!pde)
    {
        printk("[AUDIO PROC] proc_create fail - ai\n");
        goto out_ai;
    }
    pde = proc_create("ao", S_IRUGO, g_pRootAudioDir, &g_audioAoProcOps);
    if (!pde)
    {
        printk("[AUDIO PROC] proc_create fail - ao\n");
        goto out_ao;
    }

    pde = proc_create("sinegen", S_IRUGO, g_pRootAudioDir, &g_audioSineGenProcOps);
    if (!pde)
    {
        printk("[AUDIO PROC] proc_create fail - sinegen\n");
        goto out_sinegen;
    }

    pde = proc_create("sinegen_enable", S_IRUGO, g_pRootAudioDir, &g_audioSineGenEnableProcOps);
    if (!pde)
    {
        printk("[AUDIO PROC] proc_create fail - sinegen enable\n");
        goto out_sinegen_enable;
    }

    pde = proc_create("sinegen_freq", S_IRUGO, g_pRootAudioDir, &g_audioSineGenFreqProcOps);
    if (!pde)
    {
        printk("[AUDIO PROC] proc_create fail - sinegen freq\n");
        goto out_sinegen_freq;
    }

    pde = proc_create("sinegen_gain", S_IRUGO, g_pRootAudioDir, &g_audioSineGenGainProcOps);
    if (!pde)
    {
        printk("[AUDIO PROC] proc_create fail - sinegen gain\n");
        goto out_sinegen_gain;
    }

    pde = proc_create("tdm", S_IRUGO, g_pRootAudioDir, &g_audioTdmProcOps);
    if (!pde)
    {
        printk("[AUDIO PROC] proc_create fail - sinegen gain\n");
        goto out_tdm;
    }

    return AIO_OK;

out_tdm:
    remove_proc_entry("tdm", g_pRootAudioDir);
    remove_proc_entry("sinegen_freq", g_pRootAudioDir);
    remove_proc_entry("sinegen_enable", g_pRootAudioDir);
    remove_proc_entry("sinegen", g_pRootAudioDir);
    remove_proc_entry("ai", g_pRootAudioDir);
    remove_proc_entry("ao", g_pRootAudioDir);
    goto FAIL;

out_sinegen_gain:
    remove_proc_entry("sinegen_freq", g_pRootAudioDir);
    remove_proc_entry("sinegen_enable", g_pRootAudioDir);
    remove_proc_entry("sinegen", g_pRootAudioDir);
    remove_proc_entry("ai", g_pRootAudioDir);
    remove_proc_entry("ao", g_pRootAudioDir);
    goto FAIL;

out_sinegen_freq:
    remove_proc_entry("sinegen_enable", g_pRootAudioDir);
    remove_proc_entry("sinegen", g_pRootAudioDir);
    remove_proc_entry("ai", g_pRootAudioDir);
    remove_proc_entry("ao", g_pRootAudioDir);
    goto FAIL;

out_sinegen_enable:
    remove_proc_entry("sinegen", g_pRootAudioDir);
    remove_proc_entry("ai", g_pRootAudioDir);
    remove_proc_entry("ao", g_pRootAudioDir);
    goto FAIL;

out_sinegen:
    remove_proc_entry("ai", g_pRootAudioDir);
    remove_proc_entry("ao", g_pRootAudioDir);
    goto FAIL;

out_ao:
    remove_proc_entry("ai", g_pRootAudioDir);
    goto FAIL;

out_ai:
    goto FAIL;

FAIL:

    return AIO_NG;
}

int AudioProcDeInit(void)
{
    if (!g_audioProcInited)
    {
        goto FAIL;
    }

    g_audioProcInited = 0;

    printk("%s %d\n", __FUNCTION__, __LINE__); //

    remove_proc_entry("tdm", g_pRootAudioDir);
    remove_proc_entry("sinegen_gain", g_pRootAudioDir);
    remove_proc_entry("sinegen_freq", g_pRootAudioDir);
    remove_proc_entry("sinegen_enable", g_pRootAudioDir);
    remove_proc_entry("sinegen", g_pRootAudioDir);
    remove_proc_entry("ai", g_pRootAudioDir);
    remove_proc_entry("ao", g_pRootAudioDir);
    remove_proc_entry("audio", NULL);

    return AIO_OK;

FAIL:

    return AIO_NG;
}
