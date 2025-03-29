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
#include <unistd.h>
#include <stdlib.h>
#include "list.h"
#include "ss_nalu.h"

enum SS_NALU_ParseState_e
{
    E_SS_NALU_PARSE_STATE_0 = 0,
    E_SS_NALU_PARSE_STATE_1,
    E_SS_NALU_PARSE_STATE_2,
    E_SS_NALU_PARSE_STATE_3,
    E_SS_NALU_PARSE_STATE_START_CODE,
    E_SS_NALU_PARSE_STATE_FRAME_START1,
    E_SS_NALU_PARSE_STATE_FRAME_START2,
    E_SS_NALU_PARSE_STATE_END
};

/* state machine for construct one frame */
enum SS_NALU_FrameState_e
{
    E_SS_NALU_FRAME_IDLE = 0,
    E_SS_NALU_FRAME_PREFIX,
    E_SS_NALU_FRAME_START,
    E_SS_NALU_FRAME_SLICE,
    E_SS_NALU_FRAME_SUFFIX,
    E_SS_NALU_FRAME_DONE,
    E_SS_NALU_FRAME_DROP
};

typedef struct SS_NALU_DataCheck_s
{
    enum SS_NALU_ParseState_e state;
    unsigned char startCodeSize;
    unsigned char (*fpCheckIdr)(char naluType);
    unsigned char (*fpCheckType)(char naluType);
    unsigned char (*fpCheckPrefixInfo)(char naluVal);
    unsigned char (*fpCheckSuffixInfo)(char naluVal);
    unsigned char (*fpCheckFrameStart1)(char naluVal);
    unsigned char (*fpCheckFrameStart2)(char naluVal);
} SS_NALU_DataCheck_t;

typedef struct SS_NALU_DataBlockNode_s
{
    struct list_head blockGrp;
    struct list_head sliceGrp;
    char *           data;
    unsigned int     size;
    char             gap[E_SS_NALU_PARSE_STATE_END - 1];
} SS_NALU_DataBlockNode_t;

typedef struct SS_NALU_DataBlock_s
{
    struct list_head blockGrp;
    struct list_head repeatInfoSliceGrp;
    unsigned int     nodeCnt;
} SS_NALU_DataBlock_t;

typedef struct SS_NALU_Object_s
{
    SS_NALU_DataParse_t parseCfg;
    SS_NALU_DataCheck_t checkOpt;
    SS_NALU_DataBlock_t dataBlock;
} SS_NALU_Object_t;

enum SS_NALU_AVCNalUnitType_e
{
    SS_NALU_AVC_NAL_UNIT_NONIDR = 1,
    SS_NALU_AVC_NAL_UNIT_IDR    = 5,
    SS_NALU_AVC_NAL_UNIT_SEI,   //6
    SS_NALU_AVC_NAL_UNIT_SPS,   //7
    SS_NALU_AVC_NAL_UNIT_PPS,   //8
    SS_NALU_AVC_NAL_UNIT_AUD,   //9
};

enum SS_NALU_HEVCNalUnitType
{
    SS_NALU_HEVC_NAL_UNIT_CODED_SLICE_TRAIL_N = 0,   // 0
    SS_NALU_HEVC_NAL_UNIT_CODED_SLICE_TRAIL_R,       // 1

    SS_NALU_HEVC_NAL_UNIT_CODED_SLICE_TSA_N,         // 2
    SS_NALU_HEVC_NAL_UNIT_CODED_SLICE_TLA,           // 3    Current name in the spec: TSA_R

    SS_NALU_HEVC_NAL_UNIT_CODED_SLICE_STSA_N,        // 4
    SS_NALU_HEVC_NAL_UNIT_CODED_SLICE_STSA_R,        // 5

    SS_NALU_HEVC_NAL_UNIT_CODED_SLICE_RADL_N,        // 6
    SS_NALU_HEVC_NAL_UNIT_CODED_SLICE_DLP,           // 7    Current name in the spec: RADL_R

    SS_NALU_HEVC_NAL_UNIT_CODED_SLICE_RASL_N,        // 8
    SS_NALU_HEVC_NAL_UNIT_CODED_SLICE_TFD,           // 9    Current name in the spec: RASL_R

    SS_NALU_HEVC_NAL_UNIT_CODED_SLICE_BLA = 16,      // 16   Current name in the spec: BLA_W_LP
    SS_NALU_HEVC_NAL_UNIT_CODED_SLICE_BLANT,         // 17   Current name in the spec: BLA_W_DLP
    SS_NALU_HEVC_NAL_UNIT_CODED_SLICE_BLA_N_LP,      // 18
    SS_NALU_HEVC_NAL_UNIT_CODED_SLICE_IDR,           // 19   Current name in the spec: IDR_W_DLP
    SS_NALU_HEVC_NAL_UNIT_CODED_SLICE_IDR_N_LP,      // 20
    SS_NALU_HEVC_NAL_UNIT_CODED_SLICE_CRA,           // 21
    SS_NALU_HEVC_NAL_UNIT_RESERVED_22,
    SS_NALU_HEVC_NAL_UNIT_RESERVED_23,

    SS_NALU_HEVC_NAL_UNIT_VPS = 32,                  // 32
    SS_NALU_HEVC_NAL_UNIT_SPS,                       // 33
    SS_NALU_HEVC_NAL_UNIT_PPS,                       // 34
    SS_NALU_HEVC_NAL_UNIT_ACCESS_UNIT_DELIMITER,     // 35
    SS_NALU_HEVC_NAL_UNIT_EOS,                       // 36
    SS_NALU_HEVC_NAL_UNIT_EOB,                       // 37
    SS_NALU_HEVC_NAL_UNIT_FILLER_DATA,               // 38
    SS_NALU_HEVC_NAL_UNIT_SEI,                       // 39   Prefix SEI
    SS_NALU_HEVC_NAL_UNIT_SEI_SUFFIX,                // 40   Suffix SEI
};

static unsigned char _SS_NALU_H264CheckIdr(char type)
{
    unsigned char naluType = (unsigned char)type & 0x1F;
    return (SS_NALU_AVC_NAL_UNIT_IDR == naluType);
}

static unsigned char _SS_NALU_H264CheckType(char type)
{
    unsigned char naluType = (unsigned char)type & 0x1F;
    return (SS_NALU_AVC_NAL_UNIT_NONIDR == naluType || SS_NALU_AVC_NAL_UNIT_IDR == naluType);
}

static unsigned char _SS_NALU_H264CheckPrefixInfo(char type)
{
    unsigned char naluType = (unsigned char)type & 0x1F;
    return (SS_NALU_AVC_NAL_UNIT_SPS == naluType || SS_NALU_AVC_NAL_UNIT_PPS == naluType);
}

static unsigned char _SS_NALU_H264CheckSuffixInfo(char type)
{
    return 0;
}

static unsigned char _SS_NALU_H264CheckFrameStart1(char val)
{
    return !!(val & 0x80);
}

static unsigned char _SS_NALU_H264CheckFrameStart2(char val)
{
    return 0;
}

static unsigned char _SS_NALU_H265CheckIdr(char type)
{
    unsigned char naluType = ((unsigned char)type >> 1) & 0x3f;
    return (naluType >= SS_NALU_HEVC_NAL_UNIT_CODED_SLICE_BLA && naluType <= SS_NALU_HEVC_NAL_UNIT_CODED_SLICE_CRA);
}

static unsigned char _SS_NALU_H265CheckType(char type)
{
    unsigned char naluType = ((unsigned char)type >> 1) & 0x3f;
    return  naluType <= SS_NALU_HEVC_NAL_UNIT_CODED_SLICE_CRA;
}

static unsigned char _SS_NALU_H265CheckPrefixInfo(char type)
{
    unsigned char naluType = ((unsigned char)type >> 1) & 0x3f;
    return (SS_NALU_HEVC_NAL_UNIT_VPS == naluType || SS_NALU_HEVC_NAL_UNIT_SPS == naluType
            || SS_NALU_HEVC_NAL_UNIT_PPS == naluType);
}

static unsigned char _SS_NALU_H265CheckSuffixInfo(char type)
{
    unsigned char naluType = ((unsigned char)type >> 1) & 0x3f;
    return (SS_NALU_HEVC_NAL_UNIT_SEI_SUFFIX == naluType);
}

static unsigned char _SS_NALU_H265CheckFrameStart1(char val)
{
    return 0;
}

static unsigned char _SS_NALU_H265CheckFrameStart2(char val)
{
    return !!(val & 0x80);
}

static int _SS_NALU_ContinueParsing(SS_NALU_Object_t *obj)
{
    char *bs = NULL;
    char *ls = NULL;
    SS_NALU_EsSliceData_t   *slice = NULL, *lastSlice= NULL;
    SS_NALU_DataBlockNode_t *block = NULL;
    int ret = 0;
    unsigned char bEndLoop = 0;
    unsigned int shiftBack = 0;

    if (!obj)
    {
        printf("Error input\n");
        return -1;
    }
    if (obj->checkOpt.state == E_SS_NALU_PARSE_STATE_0)
    {
        // Most of the time state is 0, nothing to do here.
        return 0;
    }
    if (!obj->checkOpt.fpCheckIdr || !obj->checkOpt.fpCheckType
        || !obj->checkOpt.fpCheckPrefixInfo|| !obj->checkOpt.fpCheckSuffixInfo
        || !obj->checkOpt.fpCheckFrameStart1 || !obj->checkOpt.fpCheckFrameStart2)
    {
        printf("Error checkOpt\n");
        return -1;
    }
    if (list_empty(&obj->dataBlock.blockGrp))
    {
        printf("Block group is empty");
        return -1;
    }
    block = list_last_entry(&obj->dataBlock.blockGrp, SS_NALU_DataBlockNode_t, blockGrp);
    slice = (SS_NALU_EsSliceData_t *)malloc(sizeof(SS_NALU_EsSliceData_t));
    if (!slice)
    {
        printf("Alloc Slice data ERROR!!!\n");
        return -1;
    }
    memset(slice, 0, sizeof(SS_NALU_EsSliceData_t));
    slice->data          = &block->gap[0];
    slice->size          = 0;
    slice->startCodeSize = 0;
    slice->bFrameStart   = 0;
    bs                   = slice->data;
    ls                   = slice->data + E_SS_NALU_PARSE_STATE_END - 2;
    while (bs <= ls)
    {
        ret = obj->parseCfg.fpReadInputData(bs, 1, obj->parseCfg.userData);
        if (ret != 1)
        {
            free(slice);
            return 0;
        }
        //printf("ST: %d bs %x\n", obj->checkOpt.state, *bs & 0xff);
        //getchar();
        slice->size++;
        switch (obj->checkOpt.state)
        {
            default:
                printf("Error state!\n");
                free(slice);
                return -1;
            case E_SS_NALU_PARSE_STATE_1:
                if (0x00 == *bs)
                {
                    obj->checkOpt.state = E_SS_NALU_PARSE_STATE_2;
                    break;
                }
                bEndLoop = 1;
                break;
            case E_SS_NALU_PARSE_STATE_2:
                if (0x01 == *bs)
                {
                    obj->checkOpt.startCodeSize = 3;
                    obj->checkOpt.state = E_SS_NALU_PARSE_STATE_START_CODE;
                    break;
                }
                if (0x00 == *bs)
                {
                    obj->checkOpt.state = E_SS_NALU_PARSE_STATE_3;
                    break;
                }
                bEndLoop = 1;
                break;
            case E_SS_NALU_PARSE_STATE_3:
                if (0x01 == *bs)
                {
                    obj->checkOpt.startCodeSize = 4;
                    obj->checkOpt.state = E_SS_NALU_PARSE_STATE_START_CODE;
                    break;
                }
                bEndLoop = 1;
                break;
            case E_SS_NALU_PARSE_STATE_START_CODE:
                slice->startCodeSize = obj->checkOpt.startCodeSize;
                slice->data          = (char *)bs;
                shiftBack            = slice->startCodeSize - slice->size + 1;
                slice->size          = 1;
                if (obj->checkOpt.fpCheckType(*bs))
                {
                    obj->checkOpt.state = E_SS_NALU_PARSE_STATE_FRAME_START1;
                    if (obj->checkOpt.fpCheckIdr(*bs))
                    {
                        slice->bFrameIdr = 1;
                    }
                    break;
                }
                if (obj->checkOpt.fpCheckPrefixInfo(*bs))
                {
                    slice->bPrefixInfo = 1;
                }
                else if (obj->checkOpt.fpCheckSuffixInfo(*bs))
                {
                    slice->bSuffixInfo = 1;
                }
                bEndLoop = 1;
                break;
            case E_SS_NALU_PARSE_STATE_FRAME_START1:
                if (obj->checkOpt.fpCheckFrameStart1(*bs))
                {
                    bEndLoop = 1;
                    if (shiftBack == 0) // means state start from this.
                    {
                        if (list_empty(&block->sliceGrp))
                        {
                            free(slice);
                            printf("slice data shouldn't be empty!\n");
                            return -1;
                        }
                        lastSlice              = list_last_entry(&block->sliceGrp, SS_NALU_EsSliceData_t, sliceGrp);
                        lastSlice->bFrameStart = 1;
                        break;
                    }
                    slice->bFrameStart = 1;
                    break;
                }
                obj->checkOpt.state = E_SS_NALU_PARSE_STATE_FRAME_START2;
                break;
            case E_SS_NALU_PARSE_STATE_FRAME_START2:
                bEndLoop = 1;
                if (obj->checkOpt.fpCheckFrameStart2(*bs))
                {
                    if (shiftBack == 0) // means state start from this.
                    {
                        if (list_empty(&block->sliceGrp))
                        {
                            free(slice);
                            printf("slice data shouldn't be empty!\n");
                            return -1;
                        }
                        lastSlice              = list_last_entry(&block->sliceGrp, SS_NALU_EsSliceData_t, sliceGrp);
                        lastSlice->bFrameStart = 1;
                        break;
                    }
                    slice->bFrameStart = 1;
                }
                break;
        }
        if (bEndLoop)
        {
            obj->checkOpt.state         = E_SS_NALU_PARSE_STATE_0;
            obj->checkOpt.startCodeSize = 0;
            break;
        }
        bs++;
    }
    if (shiftBack)
    {
        if (list_empty(&block->sliceGrp))
        {
            free(slice);
            printf("slice data shouldn't be empty!\n");
            return -1;
        }
        lastSlice = list_last_entry(&block->sliceGrp, SS_NALU_EsSliceData_t, sliceGrp);
        if (lastSlice->size < shiftBack)
        {
            free(slice);
            printf("last slice data not enough, that's not gona be happened\n");
            return -1;
        }
        if (lastSlice->size == shiftBack)
        {
            // It contain start code only, but no valid data here,
            list_del(&lastSlice->sliceGrp);
            free(lastSlice);
        }
        else
        {
            lastSlice->size -= shiftBack;
        }
    }
    list_add_tail(&slice->sliceGrp, &block->sliceGrp);
    return 0;
}

static int _SS_NALU_Parse(SS_NALU_Object_t *obj)
{
    const char *bs = NULL;
    const char *ls = NULL;
    SS_NALU_EsSliceData_t *slice = NULL, *pos = NULL, *posN = NULL;
    SS_NALU_DataBlockNode_t *block = NULL;

    if (!obj)
    {
        printf("Error input\n");
        return -1;
    }
    if (!obj->checkOpt.fpCheckIdr || !obj->checkOpt.fpCheckType
        || !obj->checkOpt.fpCheckPrefixInfo|| !obj->checkOpt.fpCheckSuffixInfo
        || !obj->checkOpt.fpCheckFrameStart1 || !obj->checkOpt.fpCheckFrameStart2)
    {
        printf("Error checkOpt\n");
        return -1;
    }
    if (list_empty(&obj->dataBlock.blockGrp))
    {
        printf("Block group is empty\n");
        return -1;
    }
    block = list_last_entry(&obj->dataBlock.blockGrp, SS_NALU_DataBlockNode_t, blockGrp);
    slice = (SS_NALU_EsSliceData_t *)malloc(sizeof(SS_NALU_EsSliceData_t));
    if (!slice)
    {
        printf("Alloc Slice data ERROR!!!\n");
        return -1;
    }
    memset(slice, 0, sizeof(SS_NALU_EsSliceData_t));
    slice->data          = block->data;
    slice->size          = block->size;
    slice->startCodeSize = 0;
    slice->bFrameStart   = 0;
    bs                   = slice->data;
    ls                   = slice->data + slice->size - 1;
    while (bs <= ls)
    {
        //printf("ST: %d bs %x\n", obj->checkOpt.state, *bs & 0xff);
        //getchar();
        switch (obj->checkOpt.state)
        {
            case E_SS_NALU_PARSE_STATE_0:
            case E_SS_NALU_PARSE_STATE_1:
                obj->checkOpt.startCodeSize = 0;
                obj->checkOpt.state = (0x00 == *bs) ? (obj->checkOpt.state + 1) : E_SS_NALU_PARSE_STATE_0;
                break;
            case E_SS_NALU_PARSE_STATE_2:
                if (*bs == 0x01)
                {
                    obj->checkOpt.state         = E_SS_NALU_PARSE_STATE_START_CODE;
                    obj->checkOpt.startCodeSize = 3;
                    break;
                }
                obj->checkOpt.state = (0x00 == *bs) ? (E_SS_NALU_PARSE_STATE_3) : 0;
                break;
            case E_SS_NALU_PARSE_STATE_3:
                if (0x01 == *bs)
                {
                    obj->checkOpt.state = E_SS_NALU_PARSE_STATE_START_CODE;
                    obj->checkOpt.startCodeSize = 4;
                    break;
                }
                obj->checkOpt.state = E_SS_NALU_PARSE_STATE_0;
                break;
            case E_SS_NALU_PARSE_STATE_START_CODE:
                if (bs - obj->checkOpt.startCodeSize > slice->data)
                {
                    slice->size = bs - obj->checkOpt.startCodeSize - slice->data;
                    list_add_tail(&slice->sliceGrp, &block->sliceGrp);
                    //if (slice->startCodeSize == 3 || slice->startCodeSize == 4)
                    //{
                    //    printf("0:ADD Slice Type %x slice size %d bFrameStart %d\n", *slice->data, slice->size, slice->bFrameStart);
                    //}
                    slice = (SS_NALU_EsSliceData_t *)malloc(sizeof(SS_NALU_EsSliceData_t));
                    if (!slice)
                    {
                        printf("Alloc Slice data ERROR!!!\n");
                        goto PARSE_ERROR;
                    }
                    memset(slice, 0, sizeof(SS_NALU_EsSliceData_t));
                }
                slice->startCodeSize = obj->checkOpt.startCodeSize;
                slice->data          = (char *)bs;
                slice->size          = ls - bs + 1;
                if (obj->checkOpt.fpCheckType(*bs))
                {
                    obj->checkOpt.state = E_SS_NALU_PARSE_STATE_FRAME_START1;
                    if (obj->checkOpt.fpCheckIdr(*bs))
                    {
                        slice->bFrameIdr = 1;
                    }
                    break;
                }
                if (obj->checkOpt.fpCheckPrefixInfo(*bs))
                {
                    slice->bPrefixInfo = 1;
                }
                else if (obj->checkOpt.fpCheckSuffixInfo(*bs))
                {
                    slice->bSuffixInfo = 1;
                }
                obj->checkOpt.state = E_SS_NALU_PARSE_STATE_0;
                break;
            case E_SS_NALU_PARSE_STATE_FRAME_START1:
                if (obj->checkOpt.fpCheckFrameStart1(*bs))
                {
                    slice->bFrameStart  = 1;
                    obj->checkOpt.state = E_SS_NALU_PARSE_STATE_0;
                    break;
                }
                obj->checkOpt.state = E_SS_NALU_PARSE_STATE_FRAME_START2;
                break;
            case E_SS_NALU_PARSE_STATE_FRAME_START2:
                if (obj->checkOpt.fpCheckFrameStart2(*bs))
                {
                    slice->bFrameStart = 1;
                }
                obj->checkOpt.state = E_SS_NALU_PARSE_STATE_0;
                break;
            default:
                free(slice);
                printf("Error state %d\n", obj->checkOpt.state);
                goto PARSE_ERROR;
        }
        bs++;
    }
    //if (slice->startCodeSize == 3 || slice->startCodeSize == 4)
    //{
    //    printf("1:ADD Slice Type %x slice size %d bFrameStart %d\n", *slice->data, slice->size, slice->bFrameStart);
    //}
    list_add_tail(&slice->sliceGrp, &block->sliceGrp);
    return 0;

PARSE_ERROR:
    list_for_each_entry_safe(pos, posN, &block->sliceGrp, sliceGrp)
    {
        list_del(&pos->sliceGrp);
        free(pos);
    }
    return -1;
}


static int _SS_NALU_ClearSliceData(struct list_head *sliceGrp)
{
    SS_NALU_EsSliceData_t *pos = NULL, *posN = NULL;

    list_for_each_entry_safe(pos, posN, sliceGrp, sliceGrp)
    {
        list_del(&pos->sliceGrp);
        free(pos->data);
        free(pos);
    }
    return 0;
}

static int _SS_NALU_CloneEsSliceData(struct list_head *sliceGrp, SS_NALU_EsSliceData_t *slice)
{
    SS_NALU_EsSliceData_t *copySlice = NULL;

    copySlice = (SS_NALU_EsSliceData_t *)malloc(sizeof(SS_NALU_EsSliceData_t));
    if (!copySlice)
    {
        printf("Copy slice alloc fail!\n");
        return -1;
    }
    memcpy(copySlice, slice, sizeof(SS_NALU_EsSliceData_t));
    copySlice->data = malloc(slice->size);
    if (!copySlice->data)
    {
        free(copySlice);
        printf("Copy slice data alloc fail!\n");
        return -1;
    }
    memcpy(copySlice->data, slice->data, slice->size);
    list_add_tail(&copySlice->sliceGrp, sliceGrp);
    return 0;
}

static int _SS_NALU_AddExtraEsSliceData(SS_NALU_EsFrame_t *listData, struct list_head *sliceGrpSrc)
{
    SS_NALU_EsSliceData_t *pos = NULL, *posN = NULL, *slice = NULL;

    list_for_each_entry_safe(pos, posN, sliceGrpSrc, sliceGrp)
    {
        slice = (SS_NALU_EsSliceData_t *)malloc(sizeof(SS_NALU_EsSliceData_t));
        if (!slice)
        {
            printf("Slice alloc fail!\n");
            return -1;
        }
        memcpy(slice, pos, sizeof(SS_NALU_EsSliceData_t));
        list_add_tail(&slice->sliceGrp, &listData->sliceGrp);
        listData->totalDataSize += slice->size + slice->startCodeSize;
    }
    return 0;
}

static enum SS_NALU_FrameState_e _SS_NALU_FindFrameEnd(SS_NALU_DataBlock_t *dataBlock, SS_NALU_EsFrame_t *listData,
                                                       enum SS_NALU_FrameState_e enFrameDataSt, unsigned char bRepeatInfo)
{
    SS_NALU_DataBlockNode_t *block = NULL, *blockN = NULL;
    SS_NALU_EsSliceData_t *pos = NULL, *posN = NULL;
    enum SS_NALU_FrameState_e retSt = enFrameDataSt;

    list_for_each_entry_safe(block, blockN, &dataBlock->blockGrp, blockGrp)
    {
        list_for_each_entry_safe(pos, posN, &block->sliceGrp, sliceGrp)
        {
            switch (retSt)
            {
                case E_SS_NALU_FRAME_IDLE:
                    if (pos->bFrameStart)
                    {
                        if (bRepeatInfo && pos->bFrameIdr && !list_empty(&dataBlock->repeatInfoSliceGrp))
                        {
                            /* Add prefix repeat */
                            _SS_NALU_AddExtraEsSliceData(listData, &dataBlock->repeatInfoSliceGrp);
                        }
                        retSt = E_SS_NALU_FRAME_START;
                        break;
                    }
                    if (pos->bPrefixInfo)
                    {
                        if (bRepeatInfo)
                        {
                            _SS_NALU_ClearSliceData(&dataBlock->repeatInfoSliceGrp);
                            if (_SS_NALU_CloneEsSliceData(&dataBlock->repeatInfoSliceGrp, pos) == -1)
                            {
                                retSt = E_SS_NALU_FRAME_DROP;
                                break;
                            }
                        }
                        retSt = E_SS_NALU_FRAME_PREFIX;
                        break;
                    }
                    break;
                case E_SS_NALU_FRAME_PREFIX:
                    if (pos->bFrameStart)
                    {
                        retSt = E_SS_NALU_FRAME_START;
                        break;
                    }
                    if (pos->bPrefixInfo && bRepeatInfo)
                    {
                        if (_SS_NALU_CloneEsSliceData(&dataBlock->repeatInfoSliceGrp, pos) == -1)
                        {
                            retSt = E_SS_NALU_FRAME_DROP;
                            break;
                        }
                    }
                    break;
                case E_SS_NALU_FRAME_START:
                    if (pos->bFrameStart || pos->bPrefixInfo)
                    {
                        return E_SS_NALU_FRAME_DONE;
                    }
                    if (pos->bSuffixInfo)
                    {
                        retSt = E_SS_NALU_FRAME_SUFFIX;
                    }
                    break;
                case E_SS_NALU_FRAME_SUFFIX:
                    if (pos->bFrameStart)
                    {
                        return E_SS_NALU_FRAME_DONE;
                    }
                    break;
                default:
                    break;
            }
            list_del(&pos->sliceGrp);
            if (retSt != E_SS_NALU_FRAME_DROP)
            {
                list_add_tail(&pos->sliceGrp, &listData->sliceGrp);
                listData->totalDataSize += pos->size + pos->startCodeSize;
            }
            //printf("1:Extra, Slice Type %d slice size %d bFrameStart %d\n", (*pos->data >> 1) & 0x3f, pos->size, pos->bFrameStart);
        }
    }
    return retSt;
}


static int _SS_NALU_FlushDataBlock(SS_NALU_DataBlock_t *dataBlock)
{
    SS_NALU_DataBlockNode_t *block = NULL, *blockN = NULL;

    list_for_each_entry_safe(block, blockN, &dataBlock->blockGrp, blockGrp)
    {
        if (list_empty(&block->sliceGrp))
        {
            list_del(&block->blockGrp);
            free(block->data);
            free(block);
        }
    }
    return 0;
}

static int _SS_NALU_DeleteDataBlock(SS_NALU_DataBlock_t *dataBlock)
{
    SS_NALU_DataBlockNode_t *block = NULL, *blockN = NULL;
    SS_NALU_EsSliceData_t *pos = NULL, *posN = NULL;


    list_for_each_entry_safe(pos, posN, &dataBlock->repeatInfoSliceGrp, sliceGrp)
    {
        list_del(&pos->sliceGrp);
        free(pos->data);
        free(pos);
    }

    list_for_each_entry_safe(block, blockN, &dataBlock->blockGrp, blockGrp)
    {
        list_for_each_entry_safe(pos, posN, &block->sliceGrp, sliceGrp)
        {
            list_del(&pos->sliceGrp);
            free(pos);
        }
        list_del(&block->blockGrp);
        free(block->data);
        free(block);
    }
    return 0;
}

void *SS_NALU_CreateParser(const SS_NALU_DataParse_t *parseConfig)
{
    SS_NALU_Object_t *obj;

    if (!parseConfig || !parseConfig->fpReadInputData || !parseConfig->dataBlockSize)
    {
        printf("input error\n");
        return NULL;
    }
    obj = (SS_NALU_Object_t *)malloc(sizeof(SS_NALU_Object_t));
    if (!obj)
    {
        printf("Alloc fail!\n");
        return NULL;
    }
    memset(obj, 0, sizeof(SS_NALU_Object_t));
    obj->parseCfg = *parseConfig;
    switch (obj->parseCfg.format)
    {
    case E_SS_NALU_DATA_FORMAT_H264:
        obj->checkOpt.fpCheckIdr         = _SS_NALU_H264CheckIdr;
        obj->checkOpt.fpCheckType        = _SS_NALU_H264CheckType;
        obj->checkOpt.fpCheckPrefixInfo  = _SS_NALU_H264CheckPrefixInfo;
        obj->checkOpt.fpCheckSuffixInfo  = _SS_NALU_H264CheckSuffixInfo;
        obj->checkOpt.fpCheckFrameStart1 = _SS_NALU_H264CheckFrameStart1;
        obj->checkOpt.fpCheckFrameStart2 = _SS_NALU_H264CheckFrameStart2;
        break;
    case E_SS_NALU_DATA_FORMAT_H265:
        obj->checkOpt.fpCheckIdr         = _SS_NALU_H265CheckIdr;
        obj->checkOpt.fpCheckType        = _SS_NALU_H265CheckType;
        obj->checkOpt.fpCheckPrefixInfo  = _SS_NALU_H265CheckPrefixInfo;
        obj->checkOpt.fpCheckSuffixInfo  = _SS_NALU_H265CheckSuffixInfo;
        obj->checkOpt.fpCheckFrameStart1 = _SS_NALU_H265CheckFrameStart1;
        obj->checkOpt.fpCheckFrameStart2 = _SS_NALU_H265CheckFrameStart2;
        break;
    default:
        printf("Format error!\n");
        free(obj);
        return NULL;
    }
    INIT_LIST_HEAD(&obj->dataBlock.blockGrp);
    INIT_LIST_HEAD(&obj->dataBlock.repeatInfoSliceGrp);
    return (void *)obj;
}

int SS_NALU_GetOneFrame(void *handle, SS_NALU_EsFrame_t *listData)
{
    SS_NALU_Object_t *obj = (SS_NALU_Object_t *)handle;
    SS_NALU_DataBlockNode_t *block = NULL;
    enum SS_NALU_FrameState_e frameDataState = E_SS_NALU_FRAME_IDLE;
    int ret = 0;

    if (!obj || !listData)
    {
        printf("handle is null!\n");
        return -1;
    }
    INIT_LIST_HEAD(&listData->sliceGrp);
    listData->totalDataSize = 0;
    while (1)
    {
        frameDataState = _SS_NALU_FindFrameEnd(&obj->dataBlock, listData, frameDataState,
                                               obj->parseCfg.bRepeatInfo);
        if (frameDataState == E_SS_NALU_FRAME_DONE)
        {
            break;
        }
        // Didn't find frame start.
        block = (SS_NALU_DataBlockNode_t *)malloc(sizeof(SS_NALU_DataBlockNode_t));
        if (!block)
        {
            printf("Alloc fail!\n");
            return -1;
        }
        memset(block, 0, sizeof(SS_NALU_DataBlockNode_t));
        block->data = (char *)malloc(obj->parseCfg.dataBlockSize);
        if (!block->data)
        {
            printf("Block data alloc fail!\n");
            free(block);
            return -1;
        }
        block->size = obj->parseCfg.dataBlockSize;
        INIT_LIST_HEAD(&block->sliceGrp);
        list_add_tail(&block->blockGrp, &obj->dataBlock.blockGrp);
        obj->dataBlock.nodeCnt++;
        block = list_last_entry(&obj->dataBlock.blockGrp, SS_NALU_DataBlockNode_t, blockGrp);
        ret   = obj->parseCfg.fpReadInputData(block->data, block->size, obj->parseCfg.userData);
        if (ret == -1)
        {
            printf("Read input data error\n");
            return -1;
        }
        if (ret == 0)
        {
            printf("Read input end!\n");
            return 0;
        }
        block->size = (unsigned int)ret;
        ret         = _SS_NALU_Parse(obj);
        if (ret == -1)
        {
            return -1;
        }
        ret = _SS_NALU_ContinueParsing(obj);
        if (ret == -1)
        {
            return -1;
        }
    }
    return 0;
}

int SS_NALU_PutOneFrame(void *handle, SS_NALU_EsFrame_t *listData)
{
    SS_NALU_Object_t *obj = (SS_NALU_Object_t *)handle;
    SS_NALU_EsSliceData_t *pos = NULL, *posN = NULL;

    if (!obj || !listData)
    {
        printf("handle is null!\n");
        return -1;
    }
    list_for_each_entry_safe(pos, posN, &listData->sliceGrp, sliceGrp)
    {
        list_del(&pos->sliceGrp);
        free(pos);
    }
    return _SS_NALU_FlushDataBlock(&obj->dataBlock);
}

int SS_NALU_DestroyParser(void *handle)
{
    SS_NALU_Object_t *obj = (SS_NALU_Object_t *)handle;

    if (!obj)
    {
        printf("handle is null!\n");
        return -1;
    }
    _SS_NALU_DeleteDataBlock(&obj->dataBlock);
    free(obj);
    return 0;
}
