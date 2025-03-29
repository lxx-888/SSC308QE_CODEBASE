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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "ss_nalu.h"

#define NALU_SIZE(pu8Data, index) (pu8Data[index]<<24)|(pu8Data[index+1]<<16)|(pu8Data[index+2]<<8)|(pu8Data[index+3])

static int _ReadFile(char *data, unsigned int size, void *user)
{
    int fd = (int)(long)user;
    return read(fd, data, size);
}

static int _WriteOneFrame(int fd, int addHead, SS_NALU_EsFrame_t *frame)
{
    SS_NALU_EsSliceData_t *slice;

    if (addHead)
    {
        char header[16];
        memset(header, 0, 16);
        header[0] = 0x1;
        header[4] = (frame->totalDataSize & 0xFF000000) >> 24;
        header[5] = (frame->totalDataSize & 0xFF0000) >> 16;
        header[6] = (frame->totalDataSize & 0xFF00) >> 8;
        header[7] = (frame->totalDataSize & 0xFF);
        write(fd, header, 16);
    }
    list_for_each_entry(slice, &frame->sliceGrp, sliceGrp)
    {
        int startCode = 0;
        if (slice->startCodeSize == 3)
        {
            startCode = 0x010000;
            write(fd, &startCode, 3);
        }
        else if (slice->startCodeSize == 4)
        {
            startCode = 0x01000000;
            write(fd, &startCode, 4);
        }
        write(fd, slice->data, slice->size);
    }
    return 0;
}

static int _RemoveHead(int fd, int fdOut, int isWrite2End, int writeCnt)
{
    unsigned char header[16];
    unsigned int  size = 0;
    int           ret  = 0;
    char *        buf  = NULL;

    while (isWrite2End || writeCnt--)
    {
        ret = read(fd, header, 16);
        if (ret != 16)
        {
            break;
        }
        if (ret < 0)
        {
            printf("read header error!\n");
            return ret;
        }
        if (header[0] != 0x1)
        {
            printf("es header error!\n");
            return -1;
        }
        size = NALU_SIZE(header, 4);
        buf = (char *)malloc(size);
        if (!buf)
        {
            printf("Allod buf size %d fail!\n", size);
            return -1;
        }
        ret = read(fd, buf, size);
        if (ret != size)
        {
            free(buf);
            printf("read nalu data error!\n");
            return -1;
        }
        ret = write(fdOut, buf, size);
        if (ret != size)
        {
            free(buf);
            printf("write nalu data error!\n");
            return -1;
        }
        free(buf);
    }
    return 0;
}

int main(int argc, char **argv)
{
    char *in = NULL, *out = NULL;
    int len = 0, ret = -1, fd = 0, result = 0, fdOut = 0;
    void *parserHandle = NULL;
    int optIndex    = 0;
    int writeCnt    = 0;
    int addHead     = 0;
    int removeHead  = 0;
    int isWrite2End = 1; // Default write to the end.
    SS_NALU_DataParse_t parserCfg;
    SS_NALU_EsFrame_t frame;

    struct option longOpt[] = {
        {"input",  required_argument, 0, 'i'},
        {"output", required_argument, 0, 'o'},
        {"format", required_argument, 0, 'f'},
        {"bs",     required_argument, 0, 's'},
        {"count",  required_argument, 0, 'c'},
        {"head",   no_argument, 0, 'h'},
        {"remove", no_argument, 0, 'r'},
        {"add",    no_argument, 0, 'a'},
        {0, 0, 0, 0}
    };
    memset(&parserCfg, 0, sizeof(SS_NALU_DataParse_t));
    while ((result = getopt_long(argc, argv, "i:o:f:s:c:hra", longOpt, &optIndex)) != -1)
    {
        switch (result)
        {
        case 'i':
            len = strlen(optarg);
            if (len)
            {
                in = (char *)malloc(len + 1);
                assert(in);
                strcpy(in, optarg);
            }
            break;
        case 'o':
            len = strlen(optarg);
            if (len)
            {
                out = (char *)malloc(len + 1);
                assert(out);
                strcpy(out, optarg);
            }
            break;
        case 'f':
            if (!strcmp(optarg, "h264"))
            {
                parserCfg.format = E_SS_NALU_DATA_FORMAT_H264;
            }
            else if (!strcmp(optarg, "h265"))
            {
                parserCfg.format = E_SS_NALU_DATA_FORMAT_H265;
            }
            break;
        case 's':
            parserCfg.dataBlockSize = atoi(optarg);
            break;
        case 'c':
            writeCnt    = atoi(optarg);
            isWrite2End = 0;
            break;
        case 'h':
            addHead = 1;
            break;
        case 'r':
            removeHead = 1;
            break;
        case 'a':
            parserCfg.bRepeatInfo = 1;
            break;
        default:
            break;
        }
    }
    if (!in || !out || ((parserCfg.format == E_SS_NALU_DATA_FORMAT_NULL
        || !parserCfg.dataBlockSize) && !removeHead ))
    {
        printf("PARAMETER: '-h|--head'            : Add private 'NALU' head.\n");
        printf("           '-r|--remove'          : Remove private 'NALU' head.\n");
        printf("           '-c|--count  [cnt]'    : Write count of es frame data.\n");
        printf("           '-s|--bs     [size]'   : 'NALU' parsing block size.\n");
        printf("           '-f|--format [format]' : Source file format, 'h264' or 'h265'.\n");
        printf("           '-i|--input  [path]'   : Input file path.\n");
        printf("           '-o|--output [path]'   : Output file path.\n");
        printf("           '-a|--add'             : Add sps/pps/vps before IDR frame if stream hasn't such things.\n");
        printf("EXAMPLE : %s -i input.es -o output.es -f h265 -s 524288 -c 100 -h\n", argv[0]);
        goto PATH;
    }
    fd = open(in, O_RDONLY);
    if (fd < 0)
    {
        printf("Open %s error!\n", in);
        goto PATH;
    }
    fdOut = open(out, O_TRUNC | O_CREAT | O_WRONLY, 0644);
    if (fdOut < 0)
    {
        printf("Open write file error");
        goto PATH1;
    }
    if (removeHead)
    {
        ret = _RemoveHead(fd, fdOut, isWrite2End, writeCnt);
        goto PATH2;
    }
    parserCfg.userData        = (void *)(long)fd;
    parserCfg.fpReadInputData = _ReadFile;
    parserHandle              = SS_NALU_CreateParser(&parserCfg);
    if (!parserHandle)
    {
        printf("Create NALU_Parser fail!\n");
        goto PATH2;
    }
    while (isWrite2End || writeCnt--)
    {
        ret = SS_NALU_GetOneFrame(parserHandle, &frame);
        if (ret == -1)
        {
            break;
        }
        if (list_empty(&frame.sliceGrp))
        {
            printf("Write end!\n");
            break;
        }
        _WriteOneFrame(fdOut, addHead, &frame);
        SS_NALU_PutOneFrame(parserHandle, &frame);
    }
    SS_NALU_DestroyParser(parserHandle);
    ret = 0;
PATH2:
    close(fdOut);
PATH1:
    close(fd);
PATH:
    if (in)
    {
        free(in);
    }
    if (out)
    {
        free(out);
    }
    return ret;
}

