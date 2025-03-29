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

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>
#include <string.h>
#include "AudioKwsProcess.h"

int main(int argc, char **argv)
{
    short            input[1024];
    FILE *           in;
    int              ret;
    int              input_len;
    int              kws_index;
    KWS_HANDLE       handle;
    AudioKwsInit_t   kwsInit;
    AudioKwsConfig_t kwsConfig;
    char keywords[][KWS_MAX_LEN] = {"你好问问"};
    /*********************User change section start*******************/
    kwsInit.sampleRate = 16000;
    kwsInit.bitWidth = 16;
    kwsInit.channel = 1;
    kwsInit.keywordNum = 1;
    kwsInit.ipuMaxSize = 0;

    for(int i=0; i<kwsInit.keywordNum; i++)
    {
        strcpy(kwsInit.keywords[i], keywords[i]);
        kwsConfig.kwsThreshold[i] = -25;
    }

    kwsConfig.skipDuration = 1.0;
    kwsConfig.vadThreshold = -45;
    kwsConfig.vadMode = 0;
    kwsConfig.breakDuration = 0;

    char *input_audio = "resource/input/audio/kws_keyword.wav";
    sprintf(kwsInit.modelPath, "%s", "resource/input/audio/kws_c32m.img");
    sprintf(kwsInit.graphPath, "%s", "resource/input/audio/kws.graph");
    sprintf(kwsInit.dictPath, "%s", "resource/input/audio/kws.dict");
    /*********************User change section end*******************/

    printf("input_audio   = %s\n", input_audio);
    printf("input_model2  = %s\n", kwsInit.modelPath);
    printf("input_graph   = %s\n", kwsInit.graphPath);
    printf("input_dict    = %s\n\n", kwsInit.dictPath);

    handle = IaaKws_Init(&kwsInit);
    if (handle == NULL)
    {
        printf("KWS init error !\n");
        return -1;
    }

    ret = IaaKws_SetConfig(handle, &kwsConfig);
    if (ret != 0)
    {
        printf("KWS set error !\n");
        return -1;
    }

    in = fopen(input_audio, "rb");
    if (!in)
    {
        perror(input_audio);
        return -1;
    }

    fread(input, sizeof(char), 44, in);

    IaaKws_GetInputSamples(handle, &input_len);

    bool flag_kws = false;
    while (fread(input, sizeof(short), input_len, in))
    {
        //(4)IaaSe_Run
        ret = IaaKws_Run(handle, input, &kws_index);
        if (ret != 0)
        {
            printf("KWS check error !\n");
            return -1;
        }

        if (0 == kws_index)
        {
            flag_kws = true;
            printf("\nKeywords detected\n");
        }
    }

    if (false == flag_kws)
    {
        ret = -1;
        printf("\nNo Keywords detected\n");
    }

    IaaKws_Free(handle);
    fclose(in);
    return ret;
}
