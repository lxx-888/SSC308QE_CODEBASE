/*
	Copyright (c) 2013-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "g726.h"
#include "st_common.h"

/**************************addWaveHeader***************************/
typedef struct WAVE_FORMAT
{
    signed short wFormatTag;
    signed short wChannels;
    unsigned int dwSamplesPerSec;
    unsigned int dwAvgBytesPerSec;
    signed short wBlockAlign;
    signed short wBitsPerSample;
} WaveFormat_t;

typedef struct WAVEFILEHEADER
{
    signed char  chRIFF[4];
    unsigned int dwRIFFLen;
    signed char  chWAVE[4];
    signed char  chFMT[4];
    unsigned int dwFMTLen;
    WaveFormat_t wave;
    signed char  chDATA[4];
    unsigned int dwDATALen;
} WaveFileHeader_t;

typedef enum {
    E_AENC_TYPE_G711A = 0,
    E_AENC_TYPE_G711U,
    E_AENC_TYPE_G726_16,
    E_AENC_TYPE_G726_24,
    E_AENC_TYPE_G726_32,
    E_AENC_TYPE_G726_40,
    PCM,
} AencType_e;

typedef enum {
    E_SOUND_MODE_MONO   = 0, /* mono */
    E_SOUND_MODE_STEREO = 1, /* stereo */
} SoundMode_e;

typedef enum {
    E_SAMPLE_RATE_8000  = 8000,  /* 8kHz sampling rate */
    E_SAMPLE_RATE_16000 = 16000, /* 16kHz sampling rate */
    E_SAMPLE_RATE_32000 = 32000, /* 32kHz sampling rate */
    E_SAMPLE_RATE_48000 = 48000, /* 48kHz sampling rate */
} SampleRate_e;

int addWaveHeader(WaveFileHeader_t *tWavHead, AencType_e eAencType, SoundMode_e eSoundMode, SampleRate_e eSampleRate,
                  int raw_len)
{
    tWavHead->chRIFF[0] = 'R';
    tWavHead->chRIFF[1] = 'I';
    tWavHead->chRIFF[2] = 'F';
    tWavHead->chRIFF[3] = 'F';

    tWavHead->chWAVE[0] = 'W';
    tWavHead->chWAVE[1] = 'A';
    tWavHead->chWAVE[2] = 'V';
    tWavHead->chWAVE[3] = 'E';

    tWavHead->chFMT[0] = 'f';
    tWavHead->chFMT[1] = 'm';
    tWavHead->chFMT[2] = 't';
    tWavHead->chFMT[3] = 0x20;
    tWavHead->dwFMTLen = 0x10;

    if (eAencType == E_AENC_TYPE_G711A)
    {
        tWavHead->wave.wFormatTag = 0x06;
    }

    if (eAencType == E_AENC_TYPE_G711U)
    {
        tWavHead->wave.wFormatTag = 0x07;
    }

    if (eAencType == E_AENC_TYPE_G711U || eAencType == E_AENC_TYPE_G711A)
    {
        if (eSoundMode == E_SOUND_MODE_MONO)
            tWavHead->wave.wChannels = 0x01;
        else
            tWavHead->wave.wChannels = 0x02;

        tWavHead->wave.wBitsPerSample  = 8;
        tWavHead->wave.dwSamplesPerSec = eSampleRate;
        tWavHead->wave.dwAvgBytesPerSec =
            (tWavHead->wave.wBitsPerSample * tWavHead->wave.dwSamplesPerSec * tWavHead->wave.wChannels) / 8;
        tWavHead->wave.wBlockAlign = (tWavHead->wave.wBitsPerSample * tWavHead->wave.wChannels) / 8;
    }
    else if (eAencType == PCM)
    {
        if (eSoundMode == E_SOUND_MODE_MONO)
            tWavHead->wave.wChannels = 0x01;
        else
            tWavHead->wave.wChannels = 0x02;

        tWavHead->wave.wFormatTag      = 0x1;
        tWavHead->wave.wBitsPerSample  = 16; // 16bit
        tWavHead->wave.dwSamplesPerSec = eSampleRate;
        tWavHead->wave.dwAvgBytesPerSec =
            (tWavHead->wave.wBitsPerSample * tWavHead->wave.dwSamplesPerSec * tWavHead->wave.wChannels) / 8;
        tWavHead->wave.wBlockAlign = 1024;
    }
    else // g726
    {
        if (eSoundMode == E_SOUND_MODE_MONO)
            tWavHead->wave.wChannels = 0x01;
        else
            tWavHead->wave.wChannels = 0x02;

        tWavHead->wave.wFormatTag = 0x45;
        switch (eAencType)
        {
            case E_AENC_TYPE_G726_40:
                tWavHead->wave.wBitsPerSample = 5;
                tWavHead->wave.wBlockAlign    = 5;
                break;
            case E_AENC_TYPE_G726_32:
                tWavHead->wave.wBitsPerSample = 4;
                tWavHead->wave.wBlockAlign    = 4;
                break;
            case E_AENC_TYPE_G726_24:
                tWavHead->wave.wBitsPerSample = 3;
                tWavHead->wave.wBlockAlign    = 3;
                break;
            case E_AENC_TYPE_G726_16:
                tWavHead->wave.wBitsPerSample = 2;
                tWavHead->wave.wBlockAlign    = 2;
                break;
            default:
                printf("eAencType error:%d\n", eAencType);
                return -1;
        }

        tWavHead->wave.dwSamplesPerSec = eSampleRate;
        tWavHead->wave.dwAvgBytesPerSec =
            (tWavHead->wave.wBitsPerSample * tWavHead->wave.dwSamplesPerSec * tWavHead->wave.wChannels) / 8;
    }

    tWavHead->chDATA[0] = 'd';
    tWavHead->chDATA[1] = 'a';
    tWavHead->chDATA[2] = 't';
    tWavHead->chDATA[3] = 'a';
    tWavHead->dwDATALen = raw_len;
    tWavHead->dwRIFFLen = raw_len + sizeof(WaveFileHeader_t) - 8;

    return 0;
}

g726_state_t *g726_state_;
AencType_e    bps_;

int AvG726_Init(AencType_e bps)
{
    g726_state_ = NULL;
    bps_        = E_AENC_TYPE_G726_32;
    g726_state_ = (g726_state_t *)malloc(sizeof(g726_state_t));
    if (g726_state_)
    {
        bps_        = bps;
        g726_state_ = g726_init(g726_state_, 8000 * bps);
    }
    return 0;
}

int AvG726_Deinit()
{
    free(g726_state_);
    return 0;
}

int encode(unsigned char **odata, unsigned char *idata, int ilen)
{
    if (g726_state_ && ilen > 0)
    {
        int olen = (int)((bps_ * 8000.) / 128000. * ilen);
        *odata   = (unsigned char *)malloc(sizeof(unsigned char) * olen);
        if (*odata)
            return g726_encode(g726_state_, *odata, (short *)idata, ilen / 2);
    }
    return -1;
}

int decode(unsigned char **odata, unsigned char *idata, int ilen)
{
    if (g726_state_ && ilen > 0)
    {
        int olen = (int)(128000. / (bps_ * 8000.) * ilen);
        *odata   = (unsigned char *)malloc(sizeof(unsigned char) * olen);
        if (*odata)
            return (2 * g726_decode(g726_state_, (short *)(*odata), idata, ilen));
    }
    return -1;
}

void free_output_data(unsigned char *odata)
{
    free(odata);
}

int main(int argc, char *argv[])
{
    WaveFileHeader_t stWavHead;
    AencType_e       eWavAencType  = PCM;
    SoundMode_e      eWavSoundMode = E_SOUND_MODE_MONO;
    SampleRate_e     eSampleRate   = E_SAMPLE_RATE_8000;

    char        src_file[128] = {0};
    char        dst_file[128] = {0};
    char        option[128]   = {0};
    const char *operation[5]  = {"encode_to16k", "encode_to24k", "encode_to32k", "encode_to40k", "decode"};

    FILE *fpIn;  // input file
    FILE *fpOut; // output file
    long  encSize = 0;

    // analyse parameter
    if (4 != argc)
    {
        strcpy(src_file, "resource/input/audio/g726_8K_16bit_MONO_30s.pcm");
        strcpy(dst_file, "out/audio/g726_8K_4bit_MONO_30s.pcm");
        strcpy(option, "encode_to32k");
    }
    else
    {
        sscanf(argv[1], "%s", src_file);
        sscanf(argv[2], "%s", dst_file);
        sscanf(argv[3], "%s", option);
    }

    printf("option      = %s\n", option);

    fpIn = fopen(src_file, "rb");
    if (NULL == fpIn)
    {
        printf("fopen in_file failed !\n");
        return -1;
    }
    printf("input_file  = %s\n", src_file);

    ST_Common_CheckMkdirOutFile(dst_file);
    fpOut = fopen(dst_file, "wb+");
    if (NULL == fpOut)
    {
        printf("fopen out_file failed !\n");
        return -1;
    }
    printf("output_file = %s\n\n", dst_file);

    int ret, option_flag = 0;
    if ((ret = strcmp(option, operation[0])) == 0)
    {
        // encode_to16k
        eWavAencType = E_AENC_TYPE_G726_16;
    }
    else if ((ret = strcmp(option, operation[1])) == 0)
    {
        // encode_to24k
        eWavAencType = E_AENC_TYPE_G726_24;
    }
    else if ((ret = strcmp(option, operation[2])) == 0)
    {
        // encode_to32k
        eWavAencType = E_AENC_TYPE_G726_32;
    }
    else if ((ret = strcmp(option, operation[3])) == 0)
    {
        // encode_to40k
        eWavAencType = E_AENC_TYPE_G726_40;
    }
    else if ((ret = strcmp(option, operation[4])) == 0)
    {
        // decode
        option_flag                  = 1;
        WaveFileHeader_t *tempBuffer = (WaveFileHeader_t *)malloc(sizeof(WaveFileHeader_t) + 1);
        if (NULL == tempBuffer)
        {
            printf("tempBuffer malloc failed\n");
            return -1;
        }
        fread(tempBuffer, 1, sizeof(WaveFileHeader_t), fpIn);
        memcpy(&stWavHead, tempBuffer, sizeof(WaveFileHeader_t));
        printf("16k--2;\n24k--3;\n32k--4;\n40k--5;\n");
        printf("stWavHead.wave.wBitsPerSample = %d !!\n\n", stWavHead.wave.wBitsPerSample);

        eWavAencType = (AencType_e)stWavHead.wave.wBitsPerSample;
        fseek(fpIn, sizeof(WaveFileHeader_t), SEEK_SET);
    }
    else
    {
        printf("argv[3]:Please enter the correct parameters!\n");
    }

    if (!option_flag)
    {
        memset(&stWavHead, 0x0, sizeof(stWavHead));
        fwrite(&stWavHead, sizeof(stWavHead), 1, fpOut);
    }

    AvG726_Init(eWavAencType);
    // AvG726 g726(eWavAencType);
    unsigned char ibuf[200] = {0};
    int           rr        = 1;
    while (rr > 0)
    {
        rr = fread(ibuf, 1, 200, fpIn);
        if (rr > 0)
        {
            unsigned char *obuf = NULL;
            int            len  = 0;
            if (0 == option_flag)
            {
                len = encode(&obuf, ibuf, rr);
                // len = g726.encode(&obuf, ibuf, rr);
            }
            else if (1 == option_flag)
            {
                len = decode(&obuf, ibuf, rr);
                // len = g726.decode(&obuf, ibuf, rr);
            }
            encSize += len;
            fwrite(obuf, 1, len, fpOut);
            free_output_data(obuf);
            // g726.free_output_data(obuf);
            memset(ibuf, 0, sizeof(ibuf));
        }
    }
    if (!option_flag)
    {
        addWaveHeader(&stWavHead, eWavAencType, eWavSoundMode, eSampleRate, encSize);
        fseek(fpOut, 0, SEEK_SET);
        fwrite(&stWavHead, 1, sizeof(WaveFileHeader_t), fpOut);
    }
    fclose(fpIn);
    fclose(fpOut);
    AvG726_Deinit();
    return 0;
}
