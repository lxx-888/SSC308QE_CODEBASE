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

#include "ssos_def.h"
#include "ptree_log.h"
#include "ssos_mem.h"
#include "ptree_message.h"
#include "ptree_packer.h"
#include "ssos_task.h"
#include "ptree_packet.h"
#include "ptree_packet_raw.h"
#include "ptree_packet_video.h"
#include "ptree_packet_audio.h"
#include "ptree_linker.h"
#include "ptree_nlinker_out.h"
#include "ptree_mod.h"
#include "ptree_mod_file.h"
#include "ptree_sur_file.h"
#include "ptree_maker.h"
#include "ssos_time.h"
#include "ssos_thread.h"
#include "ptree_mod_sys.h"

#define U32VALUE(__pu8Data, __index)                                                            \
    (__pu8Data[__index] << 24) | (__pu8Data[__index + 1] << 16) | (__pu8Data[__index + 2] << 8) \
        | (__pu8Data[__index + 3])

#define PCM_PACKET_SIZE_ALIGNMENT_BYTE 1024

typedef struct PTREE_MOD_FILE_Obj_s    PTREE_MOD_FILE_Obj_t;
typedef struct PTREE_MOD_FILE_InObj_s  PTREE_MOD_FILE_InObj_t;
typedef struct PTREE_MOD_FILE_OutObj_s PTREE_MOD_FILE_OutObj_t;

enum PTREE_MOD_FILE_PacketType_e
{
    E_PTREE_MOD_FILE_PACKET_TYPE_RAW,
    E_PTREE_MOD_FILE_PACKET_TYPE_VIDEO,
    E_PTREE_MOD_FILE_PACKET_TYPE_AUDIO,
};
PTREE_ENUM_DEFINE(PTREE_MOD_FILE_PacketType_e, {E_PTREE_MOD_FILE_PACKET_TYPE_RAW, "raw"},
                  {E_PTREE_MOD_FILE_PACKET_TYPE_VIDEO, "video"}, {E_PTREE_MOD_FILE_PACKET_TYPE_AUDIO, "audio"}, )

struct PTREE_MOD_FILE_Obj_s
{
    PTREE_MOD_Obj_t base;
};
struct PTREE_MOD_FILE_InObj_s
{
    PTREE_MOD_InObj_t  base;
    PTREE_LINKER_Obj_t linker;
    SSOS_IO_File_t     fileWriteFd;
    SSOS_THREAD_Cond_t frameCond;
    int                frameCntLimit;
    int                isFinish;
    unsigned char      bAddHead;
};
struct PTREE_MOD_FILE_OutObj_s
{
    PTREE_MOD_OutObj_t base;

    SSOS_IO_File_t fileReadFd;
    unsigned int   lastTimeSec;
    unsigned int   lastTimeNsec;
    unsigned int   gapTimeNs;
    unsigned int   gapDataSize;
    void *         taskHandle;
    int            frameCntLimit;

    enum PTREE_MOD_FILE_PacketType_e packetType;
    union
    {
        PTREE_PACKET_RAW_Info_t   rawInfo;
        PTREE_PACKET_VIDEO_Info_t videoInfo;
        PTREE_PACKET_AUDIO_Info_t audioInfo;
    };
    PTREE_NLINKER_OUT_Obj_t linker;
};

typedef struct PTREE_MOD_FILE_WaveFileHeader_s
{
    char         riff[4];
    unsigned int riffLen;
    char         wave[4];
    char         fmt[4];
    unsigned int fmtLen;
    struct PTREE_MOD_FILE_WaveFormat_s
    {
        signed short formatTag;
        signed short channels;
        unsigned int samplesPerSec;
        unsigned int avgBytesPerSec;
        signed short blockAlign;
        signed short bitsPerSample;
    } waveInfo;
    char         data[4];
    unsigned int dataLen;
} PTREE_MOD_FILE_WaveFileHeader_t;

static int                 _PTREE_MOD_FILE_InLinkerEnqueue(PTREE_LINKER_Obj_t *linker, PTREE_PACKET_Obj_t *packet);
static PTREE_PACKET_Obj_t *_PTREE_MOD_FILE_InLinkerDequeue(PTREE_LINKER_Obj_t *linker, int ms);

static int _PTREE_MOD_FILE_OutLinkerEnqueue(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet);
static PTREE_PACKET_Obj_t *_PTREE_MOD_FILE_OutLinkerDequeueFromInside(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, int ms);
static int _PTREE_MOD_FILE_OutLinkerDequeueOut(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet);

static int                 _PTREE_MOD_FILE_IsDelayLink(PTREE_MOD_InObj_t *modIn);
static int                 _PTREE_MOD_FILE_InLinked(PTREE_MOD_InObj_t *modIn, unsigned int ref);
static int                 _PTREE_MOD_FILE_InUnlinked(PTREE_MOD_InObj_t *modIn, unsigned int ref);
static int                 _PTREE_MOD_FILE_InGetType(PTREE_MOD_InObj_t *modIn);
static PTREE_LINKER_Obj_t *_PTREE_MOD_FILE_InCreateNLinker(PTREE_MOD_InObj_t *modIn);
static PTREE_PACKER_Obj_t *_PTREE_MOD_FILE_InCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast);
static void                _PTREE_MOD_FILE_InDestruct(PTREE_MOD_InObj_t *modIn);
static void                _PTREE_MOD_FILE_InFree(PTREE_MOD_InObj_t *modIn);

static int                  _PTREE_MOD_FILE_OutStart(PTREE_MOD_OutObj_t *modOut);
static int                  _PTREE_MOD_FILE_OutStop(PTREE_MOD_OutObj_t *modOut);
static int                  _PTREE_MOD_FILE_OutGetType(PTREE_MOD_OutObj_t *modOut);
static PTREE_LINKER_Obj_t * _PTREE_MOD_FILE_OutCreateNLinker(PTREE_MOD_OutObj_t *modOut);
static PTREE_PACKET_Info_t *_PTREE_MOD_FILE_OutGetPacketInfo(PTREE_MOD_OutObj_t *modOut);
static int                  _PTREE_MOD_FILE_OutLinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref);
static int                  _PTREE_MOD_FILE_OutUnlinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref);
static void                 _PTREE_MOD_FILE_OutDestruct(PTREE_MOD_OutObj_t *modOut);
static void                 _PTREE_MOD_FILE_OutFree(PTREE_MOD_OutObj_t *modOut);

static PTREE_MOD_InObj_t * _PTREE_MOD_FILE_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static PTREE_MOD_OutObj_t *_PTREE_MOD_FILE_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static void                _PTREE_MOD_FILE_Free(PTREE_MOD_Obj_t *mod);

static const PTREE_LINKER_Ops_t G_PTREE_MOD_FILE_IN_LINKER_OPS = {
    .enqueue = _PTREE_MOD_FILE_InLinkerEnqueue,
    .dequeue = _PTREE_MOD_FILE_InLinkerDequeue,
};
static const PTREE_LINKER_Hook_t G_PTREE_MOD_FILE_IN_LINKER_HOOK = {};

static const PTREE_NLINKER_OUT_Ops_t G_PTREE_MOD_FILE_OUT_LINKER_OPS = {
    .enqueue           = _PTREE_MOD_FILE_OutLinkerEnqueue,
    .dequeueOut        = _PTREE_MOD_FILE_OutLinkerDequeueOut,
    .dequeueFromInside = _PTREE_MOD_FILE_OutLinkerDequeueFromInside,
};
static const PTREE_NLINKER_OUT_Hook_t G_PTREE_MOD_FILE_OUT_LINKER_HOOK = {};

static const PTREE_MOD_InOps_t G_PTREE_MOD_FILE_IN_OPS = {
    .isDelayLink   = _PTREE_MOD_FILE_IsDelayLink,
    .linked        = _PTREE_MOD_FILE_InLinked,
    .unlinked      = _PTREE_MOD_FILE_InUnlinked,
    .getType       = _PTREE_MOD_FILE_InGetType,
    .createNLinker = _PTREE_MOD_FILE_InCreateNLinker,
    .createPacker  = _PTREE_MOD_FILE_InCreatePacker,
};
static const PTREE_MOD_InHook_t G_PTREE_MOD_FILE_IN_HOOK = {
    .destruct = _PTREE_MOD_FILE_InDestruct,
    .free     = _PTREE_MOD_FILE_InFree,
};

static const PTREE_MOD_OutOps_t G_PTREE_MOD_FILE_OUT_OPS = {
    .start         = _PTREE_MOD_FILE_OutStart,
    .stop          = _PTREE_MOD_FILE_OutStop,
    .linked        = _PTREE_MOD_FILE_OutLinked,
    .unlinked      = _PTREE_MOD_FILE_OutUnlinked,
    .getType       = _PTREE_MOD_FILE_OutGetType,
    .createNLinker = _PTREE_MOD_FILE_OutCreateNLinker,
    .getPacketInfo = _PTREE_MOD_FILE_OutGetPacketInfo,
};
static const PTREE_MOD_OutHook_t G_PTREE_MOD_FILE_OUT_HOOK = {
    .destruct = _PTREE_MOD_FILE_OutDestruct,
    .free     = _PTREE_MOD_FILE_OutFree,
};

static const PTREE_MOD_Ops_t G_PTREE_MOD_FILE_OPS = {
    .createModIn  = _PTREE_MOD_FILE_CreateModIn,
    .createModOut = _PTREE_MOD_FILE_CreateModOut,
};
static const PTREE_MOD_Hook_t G_PTREE_MOD_FILE_HOOK = {
    .free = _PTREE_MOD_FILE_Free,
};

void PTREE_MOD_FILE_SetLeftFrame(PTREE_MOD_InObj_t *inMod, unsigned int frameCnt)
{
    PTREE_MOD_FILE_InObj_t *fileInObj = CONTAINER_OF(inMod, PTREE_MOD_FILE_InObj_t, base);

    fileInObj->frameCntLimit = frameCnt;
}

int PTREE_MOD_FILE_WaitFrame(PTREE_MOD_InObj_t *inMod, unsigned int waitMs)
{
    SSOS_TIME_Spec_t        time;
    PTREE_MOD_FILE_InObj_t *fileInObj = CONTAINER_OF(inMod, PTREE_MOD_FILE_InObj_t, base);

    time.tvSec  = waitMs / 1000;
    time.tvNSec = (waitMs % 1000) * 1000000;
    return SSOS_THREAD_COND_TIME_WAIT(&fileInObj->frameCond, fileInObj->isFinish, time);
}

static void _PTREE_MOD_FILE_WriteRawFile(SSOS_IO_File_t fileWriteFd, const PTREE_PACKET_RAW_RawData_t *rawData)
{
    if (rawData->data[0])
    {
        SSOS_IO_FileWrite(fileWriteFd, rawData->data[0], rawData->size[0]);
    }
    if (rawData->data[1])
    {
        SSOS_IO_FileWrite(fileWriteFd, rawData->data[1], rawData->size[1]);
    }
}
static void _PTREE_MOD_FILE_WriteEsVideoFile(SSOS_IO_File_t fileWriteFd, unsigned char bAddHead,
                                             const PTREE_PACKET_VIDEO_PacketInfo_t *esInfo,
                                             const PTREE_PACKET_VIDEO_PacketData_t *esData)
{
    unsigned int i = 0;
    if (bAddHead)
    {
        unsigned char header[16];
        unsigned int  dataSize = 0;
        for (i = 0; i < esInfo->pktCount; i++)
        {
            dataSize += esInfo->pktInfo[i].size;
        }
        memset(header, 0, 16);
        header[0] = 0x1;
        header[4] = ((dataSize) >> 24) & 0xFF;
        header[5] = ((dataSize) >> 16) & 0xFF;
        header[6] = ((dataSize) >> 8) & 0xFF;
        header[7] = (dataSize)&0xFF;
        SSOS_IO_FileWrite(fileWriteFd, header, 16);
    }
    for (i = 0; i < esInfo->pktCount; i++)
    {
        SSOS_IO_FileWrite(fileWriteFd, esData->pktData[i].data, esInfo->pktInfo[i].size);
    }
}
static inline int _PTREE_MOD_FILE_FillWaveHeader(PTREE_MOD_FILE_WaveFileHeader_t *header, unsigned int sampleRate,
                                                 unsigned int channels, unsigned int size)
{
    header->riff[0]                 = 'R';
    header->riff[1]                 = 'I';
    header->riff[2]                 = 'F';
    header->riff[3]                 = 'F';
    header->wave[0]                 = 'W';
    header->wave[1]                 = 'A';
    header->wave[2]                 = 'V';
    header->wave[3]                 = 'E';
    header->fmt[0]                  = 'f';
    header->fmt[1]                  = 'm';
    header->fmt[2]                  = 't';
    header->fmt[3]                  = 0x20;
    header->waveInfo.channels       = channels;
    header->waveInfo.formatTag      = 0x1;
    header->waveInfo.bitsPerSample  = 16; /* 16bit */
    header->waveInfo.samplesPerSec  = sampleRate;
    header->waveInfo.avgBytesPerSec = (header->waveInfo.bitsPerSample * sampleRate * channels) / 8;
    header->waveInfo.blockAlign     = 1024;
    header->data[0]                 = 'd';
    header->data[1]                 = 'a';
    header->data[2]                 = 't';
    header->data[3]                 = 'a';
    header->dataLen                 = header->dataLen + size;
    header->riffLen                 = header->dataLen + sizeof(PTREE_MOD_FILE_WaveFileHeader_t) - 8;
    return SSOS_DEF_OK;
}

static void _PTREE_MOD_FILE_WritePcmAudioFile(SSOS_IO_File_t                         fileWriteFd,
                                              const PTREE_PACKET_AUDIO_PacketInfo_t *esAudInfo,
                                              const PTREE_PACKET_AUDIO_PacketData_t *esAudData)
{
    unsigned int i = 0;
    for (i = 0; i < esAudInfo->pktCount; i++)
    {
        PTREE_MOD_FILE_WaveFileHeader_t wavHeader;
        memset(&wavHeader, 0, sizeof(PTREE_MOD_FILE_WaveFileHeader_t));
        _PTREE_MOD_FILE_FillWaveHeader(&wavHeader, esAudInfo->sampleRate, esAudInfo->channels,
                                       esAudInfo->pktInfo[i].size);
        SSOS_IO_FileSeek(fileWriteFd, 0, SSOS_IO_SEEK_SET);
        SSOS_IO_FileWrite(fileWriteFd, &wavHeader, sizeof(PTREE_MOD_FILE_WaveFileHeader_t));
        SSOS_IO_FileSeek(fileWriteFd, 0, SSOS_IO_SEEK_END);
        SSOS_IO_FileWrite(fileWriteFd, esAudData->pktData[i].data, esAudInfo->pktInfo[i].size);
    }
}
static void _PTREE_MOD_FILE_ReadRawFile(SSOS_IO_File_t fd, const PTREE_PACKET_RAW_RawInfo_t *info,
                                        PTREE_PACKET_RAW_RawData_t *data)
{
    switch (info->fmt)
    {
        case E_PTREE_PACKET_RAW_FORMAT_YUV422_YUYV:
        case E_PTREE_PACKET_RAW_FORMAT_YUV422_UYVY:
        case E_PTREE_PACKET_RAW_FORMAT_YUV422_YVYU:
        case E_PTREE_PACKET_RAW_FORMAT_YUV422_VYUY:
        case E_PTREE_PACKET_RAW_FORMAT_RGB888:
        case E_PTREE_PACKET_RAW_FORMAT_BGR888:
        case E_PTREE_PACKET_RAW_FORMAT_ARGB8888:
        case E_PTREE_PACKET_RAW_FORMAT_ABGR8888:
        case E_PTREE_PACKET_RAW_FORMAT_BGRA8888:
        case E_PTREE_PACKET_RAW_FORMAT_RGB565:
        case E_PTREE_PACKET_RAW_FORMAT_ARGB1555:
        case E_PTREE_PACKET_RAW_FORMAT_ARGB4444:
        case E_PTREE_PACKET_RAW_FORMAT_I2:
        case E_PTREE_PACKET_RAW_FORMAT_I4:
        case E_PTREE_PACKET_RAW_FORMAT_I8:
        case E_PTREE_PACKET_RAW_FORMAT_BAYER_BASE ... E_PTREE_PACKET_RAW_FORMAT_BAYER_NUM:
        {
            int curr = SSOS_IO_FileSeek(fd, 0, SEEK_CUR);
            int end  = SSOS_IO_FileSeek(fd, 0, SEEK_END);
            if (end - curr < (int)data->size[0])
            {
                curr = 0;
            }
            SSOS_IO_FileSeek(fd, curr, SEEK_SET);
            if (data->size[0] != (unsigned int)SSOS_IO_FileRead(fd, data->data[0], data->size[0]))
            {
                PTREE_ERR("Bad file,");
                return;
            }
        }
        break;
        case E_PTREE_PACKET_RAW_FORMAT_YUV420SP:
        case E_PTREE_PACKET_RAW_FORMAT_YUV420SP_NV21:
        {
            int curr = SSOS_IO_FileSeek(fd, 0, SEEK_CUR);
            int end  = SSOS_IO_FileSeek(fd, 0, SEEK_END);
            if (end - curr < (int)(data->size[0] + data->size[1]))
            {
                curr = 0;
            }
            SSOS_IO_FileSeek(fd, curr, SEEK_SET);
            if (data->size[0] != (unsigned int)SSOS_IO_FileRead(fd, data->data[0], data->size[0])
                || data->size[1] != (unsigned int)SSOS_IO_FileRead(fd, data->data[1], data->size[1]))
            {
                PTREE_ERR("Bad file,");
                return;
            }
        }
        default:
            break;
    }
}

static int _PTREE_MOD_FILE_CheckEsVideoFile(SSOS_IO_File_t fd, PTREE_PACKET_VIDEO_PacketInfo_t *info)
{
    unsigned char header[16];
    int           curr = SSOS_IO_FileSeek(fd, 0, SSOS_IO_SEEK_CUR);
    int           end  = SSOS_IO_FileSeek(fd, 0, SSOS_IO_SEEK_END);

    memset(header, 0, 16);
    if (end - curr < 16)
    {
        curr = 0;
    }
    SSOS_IO_FileSeek(fd, curr, SSOS_IO_SEEK_SET);
    if (16 != SSOS_IO_FileRead(fd, header, 16))
    {
        PTREE_ERR("Bad file,");
        return SSOS_DEF_FAIL;
    }
    if (header[0] != 1 && header[1] != 0 && header[2] != 0 && header[3] != 0)
    {
        PTREE_ERR("ES File Header error!");
        return SSOS_DEF_FAIL;
    }
    info->pktCount        = 1;
    info->pktInfo[0].bEnd = 1;
    info->pktInfo[0].size = U32VALUE(header, 4);
    return SSOS_DEF_OK;
}
static void _PTREE_MOD_FILE_ReadEsVideoFile(SSOS_IO_File_t fd, const PTREE_PACKET_VIDEO_PacketInfo_t *info,
                                            const PTREE_PACKET_VIDEO_PacketData_t *data)
{
    unsigned int i = 0;
    for (; i < info->pktCount; ++i)
    {
        if (info->pktInfo[i].size != (unsigned int)SSOS_IO_FileRead(fd, data->pktData[i].data, info->pktInfo[i].size))
        {
            PTREE_ERR("Bad file, fd %p sz %d address %p", fd, info->pktInfo[i].size, data->pktData[i].data);
            break;
        }
    }
}
static void _PTREE_MOD_FILE_ReadPcmAudioFile(SSOS_IO_File_t fd, unsigned char isSkipWavHeader,
                                             const PTREE_PACKET_AUDIO_PacketInfo_t *info,
                                             const PTREE_PACKET_AUDIO_PacketData_t *data)
{
    unsigned int i = 0;
    for (; i < info->pktCount; i++)
    {
        if (info->pktInfo[i].size != (unsigned int)SSOS_IO_FileRead(fd, data->pktData[i].data, info->pktInfo[i].size))
        {
            SSOS_IO_FileSeek(fd, (isSkipWavHeader) ? 44 : 0, SSOS_IO_SEEK_SET);
            return;
        }
    }
}

static void *_PTREE_MOD_FILE_RawFileReader(SSOS_TASK_Buffer_t *taskBuf)
{
    PTREE_MOD_FILE_OutObj_t *fileModOut = NULL;
    PTREE_PACKET_RAW_Obj_t * rawPacket  = NULL;
    PTREE_PACKET_Obj_t *     packet     = NULL;

    fileModOut = (PTREE_MOD_FILE_OutObj_t *)taskBuf->buf;
    SSOS_ASSERT(fileModOut->packetType == E_PTREE_MOD_FILE_PACKET_TYPE_RAW);
    if (fileModOut->frameCntLimit == 0)
    {
        PTREE_DBG("RawFile readcnt reach frameCntLimit [finish......]\n");
        return 0;
    }
    if (fileModOut->frameCntLimit > 0)
    {
        fileModOut->frameCntLimit--;
    }
    packet = PTREE_PACKER_Make(&fileModOut->base.packer.base, &fileModOut->rawInfo.base);
    if (!packet)
    {
        return NULL;
    }
    rawPacket = CONTAINER_OF(packet, PTREE_PACKET_RAW_Obj_t, base);
    _PTREE_MOD_FILE_ReadRawFile(fileModOut->fileReadFd, &rawPacket->info.rawInfo, &rawPacket->rawData);
    PTREE_LINKER_Enqueue(&fileModOut->base.plinker.base, packet);
    PTREE_PACKET_Del(packet);
    return NULL;
}

static unsigned int _PTREE_MOD_FILE_PcmLoopFileSize(PTREE_MOD_FILE_OutObj_t *fileModOut)
{
    SSOS_TIME_Spec_t curTime;
    unsigned int     uCycleMs = 10;
    unsigned int     retSize  = 0;
    unsigned int     curSec   = 0;
    unsigned int     curNsec  = 0;

    SSOS_TIME_Get(&curTime);
    /*Value < 1000000000, 32bit can handle this.*/
    curSec  = curTime.tvSec;
    curNsec = curTime.tvNSec;
    if (fileModOut->lastTimeNsec || fileModOut->lastTimeSec)
    {
        unsigned int timeNs = 0;

        timeNs   = (fileModOut->lastTimeNsec < curSec)
                       ? ((curNsec - fileModOut->lastTimeNsec) + (curSec - fileModOut->lastTimeSec) * 1000000000)
                       : ((1000000000 - fileModOut->lastTimeNsec + curNsec)
                        + (curSec - fileModOut->lastTimeSec - 1) * 1000000000);
        uCycleMs = timeNs / 10000000 * 10;
        fileModOut->gapTimeNs += (timeNs % 10000000);
        if (fileModOut->gapTimeNs >= 10000000)
        {
            fileModOut->gapTimeNs -= 10000000;
            uCycleMs += 10;
        }
        // PTREE_DBG("CurTime: %u,%u DiffNs: %u GapNs: %u DataMs: %u", curSec, curNsec, timeNs,
        // fileModOut->gapTimeNs, uCycleMs);
    }
    fileModOut->lastTimeSec  = curTime.tvSec;
    fileModOut->lastTimeNsec = curTime.tvNSec;
    retSize                  = fileModOut->audioInfo.packetInfo.sampleRate * fileModOut->audioInfo.packetInfo.channels
              * (fileModOut->audioInfo.packetInfo.sampleWidth / 8) * uCycleMs / 1000;
    fileModOut->gapDataSize += retSize % (PCM_PACKET_SIZE_ALIGNMENT_BYTE);
    /* Audio data algogrithem only accept byte alignment of pcm data.*/
    retSize = retSize / (PCM_PACKET_SIZE_ALIGNMENT_BYTE) * (PCM_PACKET_SIZE_ALIGNMENT_BYTE);
    if (fileModOut->gapDataSize >= (PCM_PACKET_SIZE_ALIGNMENT_BYTE))
    {
        fileModOut->gapDataSize -= PCM_PACKET_SIZE_ALIGNMENT_BYTE;
        retSize += PCM_PACKET_SIZE_ALIGNMENT_BYTE;
    }
    return retSize;
}

static void *_PTREE_MOD_FILE_PcmFileReader(SSOS_TASK_Buffer_t *taskBuf)
{
    PTREE_MOD_FILE_OutObj_t * fileModOut  = NULL;
    PTREE_PACKET_AUDIO_Obj_t *audioPacket = NULL;
    PTREE_PACKET_Obj_t *      packet      = NULL;
    unsigned int              size        = 0;

    fileModOut = (PTREE_MOD_FILE_OutObj_t *)taskBuf->buf;
    SSOS_ASSERT(fileModOut->packetType == E_PTREE_MOD_FILE_PACKET_TYPE_AUDIO);
    if (fileModOut->frameCntLimit == 0)
    {
        PTREE_DBG("EsAudioFile readcnt reach frameCntLimit [finish......]\n");
        return 0;
    }
    if (fileModOut->frameCntLimit > 0)
    {
        fileModOut->frameCntLimit--;
    }
    size = _PTREE_MOD_FILE_PcmLoopFileSize(fileModOut);
    if (!size)
    {
        return NULL;
    }
    fileModOut->audioInfo.packetInfo.pktInfo[0].size = size;
    packet = PTREE_PACKER_Make(&fileModOut->base.packer.base, &fileModOut->audioInfo.base);
    if (!packet)
    {
        return NULL;
    }
    audioPacket = CONTAINER_OF(packet, PTREE_PACKET_AUDIO_Obj_t, base);
    _PTREE_MOD_FILE_ReadPcmAudioFile(fileModOut->fileReadFd, 1, &audioPacket->info.packetInfo,
                                     &audioPacket->packetData);
    PTREE_LINKER_Enqueue(&fileModOut->base.plinker.base, packet);
    PTREE_PACKET_Del(packet);
    return NULL;
}

static void *_PTREE_MOD_FILE_EsFileReader(SSOS_TASK_Buffer_t *taskBuf)
{
    PTREE_MOD_FILE_OutObj_t * fileModOut  = NULL;
    PTREE_PACKET_VIDEO_Obj_t *videoPacket = NULL;
    PTREE_PACKET_Obj_t *      packet      = NULL;

    fileModOut = (PTREE_MOD_FILE_OutObj_t *)taskBuf->buf;
    SSOS_ASSERT(fileModOut->packetType == E_PTREE_MOD_FILE_PACKET_TYPE_VIDEO);
    if (fileModOut->frameCntLimit == 0)
    {
        PTREE_DBG("EsVideoFile readcnt reach frameCntLimit [finish......]\n");
        return 0;
    }
    if (fileModOut->frameCntLimit > 0)
    {
        fileModOut->frameCntLimit--;
    }
    if (_PTREE_MOD_FILE_CheckEsVideoFile(fileModOut->fileReadFd, &fileModOut->videoInfo.packetInfo) == -1)
    {
        return NULL;
    }
    packet = PTREE_PACKER_Make(&fileModOut->base.packer.base, &fileModOut->videoInfo.base);
    if (!packet)
    {
        int curr = SSOS_IO_FileSeek(fileModOut->fileReadFd, 0, SSOS_IO_SEEK_CUR);
        if (curr >= 16)
        {
            SSOS_IO_FileSeek(fileModOut->fileReadFd, curr - 16, SSOS_IO_SEEK_SET);
        }
        return NULL;
    }
    videoPacket = CONTAINER_OF(packet, PTREE_PACKET_VIDEO_Obj_t, base);
    _PTREE_MOD_FILE_ReadEsVideoFile(fileModOut->fileReadFd, &videoPacket->info.packetInfo, &videoPacket->packetData);
    PTREE_LINKER_Enqueue(&fileModOut->base.plinker.base, packet);
    PTREE_PACKET_Del(packet);
    return NULL;
}

static int _PTREE_MOD_FILE_InLinkerEnqueue(PTREE_LINKER_Obj_t *linker, PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_FILE_InObj_t *fileInObj = CONTAINER_OF(linker, PTREE_MOD_FILE_InObj_t, linker);

    if (fileInObj->frameCntLimit == 0)
    {
        return SSOS_DEF_OK;
    }
    if (fileInObj->frameCntLimit > 0)
    {
        fileInObj->frameCntLimit--;
        if (!fileInObj->frameCntLimit)
        {
            PTREE_LOG("Got enough packet preview reader thread will stop. time %luus", PTREE_MOD_GetTimer());
            PTREE_MESSAGE_Suspend(&fileInObj->base.message);
            fileInObj->isFinish = SSOS_DEF_TRUE;
        }
    }
    else if (fileInObj->frameCntLimit < 0)
    {
        fileInObj->isFinish = SSOS_DEF_TRUE;
    }
    if (PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(raw)))
    {
        PTREE_PACKET_RAW_Obj_t *rawPacket = CONTAINER_OF(packet, PTREE_PACKET_RAW_Obj_t, base);
        _PTREE_MOD_FILE_WriteRawFile(fileInObj->fileWriteFd, &rawPacket->rawData);
        SSOS_THREAD_COND_WAKE(&fileInObj->frameCond);
        return SSOS_DEF_OK;
    }
    if (PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(video)))
    {
        PTREE_PACKET_VIDEO_Obj_t *videoPacket = CONTAINER_OF(packet, PTREE_PACKET_VIDEO_Obj_t, base);
        _PTREE_MOD_FILE_WriteEsVideoFile(fileInObj->fileWriteFd, fileInObj->bAddHead, &videoPacket->info.packetInfo,
                                         &videoPacket->packetData);
        SSOS_THREAD_COND_WAKE(&fileInObj->frameCond);
        if (!fileInObj->frameCntLimit)
        {
            PTREE_LOG("File write done time is %luus", PTREE_MOD_GetTimer());
        }
        return SSOS_DEF_OK;
    }
    if (PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(audio)))
    {
        PTREE_PACKET_AUDIO_Obj_t *audioPacket = CONTAINER_OF(packet, PTREE_PACKET_AUDIO_Obj_t, base);
        _PTREE_MOD_FILE_WritePcmAudioFile(fileInObj->fileWriteFd, &audioPacket->info.packetInfo,
                                          &audioPacket->packetData);
        SSOS_THREAD_COND_WAKE(&fileInObj->frameCond);
        return SSOS_DEF_OK;
    }
    PTREE_ERR("packet info type %s is not support", packet->info->type);
    return SSOS_DEF_FAIL;
}
static PTREE_PACKET_Obj_t *_PTREE_MOD_FILE_InLinkerDequeue(PTREE_LINKER_Obj_t *linker, int ms)
{
    (void)linker;
    (void)ms;
    return NULL;
}

static int _PTREE_MOD_FILE_OutLinkerEnqueue(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_FILE_OutObj_t *fileModOut = CONTAINER_OF(nlinkerOut, PTREE_MOD_FILE_OutObj_t, linker);
    if (E_PTREE_MOD_FILE_PACKET_TYPE_VIDEO == fileModOut->packetType
        && PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(video)))
    {
        return SSOS_DEF_OK;
    }
    if (E_PTREE_MOD_FILE_PACKET_TYPE_AUDIO == fileModOut->packetType
        && PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(audio)))
    {
        return SSOS_DEF_OK;
    }
    if (E_PTREE_MOD_FILE_PACKET_TYPE_RAW == fileModOut->packetType
        && PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(raw)))
    {
        return SSOS_DEF_OK;
    }
    return SSOS_DEF_FAIL;
}
static PTREE_PACKET_Obj_t *_PTREE_MOD_FILE_OutLinkerDequeueFromInside(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, int ms)
{
    PTREE_MOD_FILE_OutObj_t *fileModOut = CONTAINER_OF(nlinkerOut, PTREE_MOD_FILE_OutObj_t, linker);
    PTREE_PACKET_Obj_t *     packet     = NULL;

    (void)ms;
    if (fileModOut->frameCntLimit == 0)
    {
        PTREE_DBG("FILE readcnt reach frameCntLimit [finish......]\n");
        return 0;
    }
    if (fileModOut->frameCntLimit > 0)
    {
        fileModOut->frameCntLimit--;
    }
    if (E_PTREE_MOD_FILE_PACKET_TYPE_VIDEO == fileModOut->packetType)
    {
        PTREE_PACKET_VIDEO_Obj_t *videoPacket = NULL;
        if (_PTREE_MOD_FILE_CheckEsVideoFile(fileModOut->fileReadFd, &fileModOut->videoInfo.packetInfo) == -1)
        {
            PTREE_ERR("Check fail port %d", fileModOut->base.info->port);
            return NULL;
        }
        packet = PTREE_PACKER_Make(&fileModOut->base.packer.base, &fileModOut->videoInfo.base);
        if (!packet)
        {
            int curr = SSOS_IO_FileSeek(fileModOut->fileReadFd, 0, SEEK_CUR);
            if (curr >= 16)
            {
                SSOS_IO_FileSeek(fileModOut->fileReadFd, curr - 16, SEEK_SET);
            }
            return NULL;
        }
        videoPacket = CONTAINER_OF(packet, PTREE_PACKET_VIDEO_Obj_t, base);
        _PTREE_MOD_FILE_ReadEsVideoFile(fileModOut->fileReadFd, &videoPacket->info.packetInfo,
                                        &videoPacket->packetData);
        return packet;
    }
    if (E_PTREE_MOD_FILE_PACKET_TYPE_AUDIO == fileModOut->packetType)
    {
        PTREE_PACKET_AUDIO_Obj_t *audioPacket = NULL;
        unsigned int              size        = 0;
        size                                  = _PTREE_MOD_FILE_PcmLoopFileSize(fileModOut);
        if (!size)
        {
            return NULL;
        }
        fileModOut->audioInfo.packetInfo.pktInfo[0].size = size;
        packet = PTREE_PACKER_Make(&fileModOut->base.packer.base, &fileModOut->audioInfo.base);
        if (!packet)
        {
            return NULL;
        }
        audioPacket = CONTAINER_OF(packet, PTREE_PACKET_AUDIO_Obj_t, base);
        _PTREE_MOD_FILE_ReadPcmAudioFile(fileModOut->fileReadFd, 1, &audioPacket->info.packetInfo,
                                         &audioPacket->packetData);
        return packet;
    }
    if (E_PTREE_MOD_FILE_PACKET_TYPE_RAW == fileModOut->packetType)
    {
        PTREE_PACKET_RAW_Obj_t *rawPacket = NULL;
        packet                            = PTREE_PACKER_Make(&fileModOut->base.packer.base, &fileModOut->rawInfo.base);
        if (!packet)
        {
            return NULL;
        }
        rawPacket = CONTAINER_OF(packet, PTREE_PACKET_RAW_Obj_t, base);
        _PTREE_MOD_FILE_ReadRawFile(fileModOut->fileReadFd, &rawPacket->info.rawInfo, &rawPacket->rawData);
        return packet;
    }
    return NULL;
}
static int _PTREE_MOD_FILE_OutLinkerDequeueOut(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_FILE_OutObj_t *fileModOut = CONTAINER_OF(nlinkerOut, PTREE_MOD_FILE_OutObj_t, linker);

    if (E_PTREE_MOD_FILE_PACKET_TYPE_VIDEO == fileModOut->packetType
        && PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(video)))
    {
        PTREE_PACKET_VIDEO_Obj_t *videoPacket = CONTAINER_OF(packet, PTREE_PACKET_VIDEO_Obj_t, base);
        if (_PTREE_MOD_FILE_CheckEsVideoFile(fileModOut->fileReadFd, &fileModOut->videoInfo.packetInfo) == -1)
        {
            PTREE_ERR("Check fail port %d", fileModOut->base.info->port);
            return SSOS_DEF_FAIL;
        }
        _PTREE_MOD_FILE_ReadEsVideoFile(fileModOut->fileReadFd, &videoPacket->info.packetInfo,
                                        &videoPacket->packetData);
        return SSOS_DEF_OK;
    }
    if (E_PTREE_MOD_FILE_PACKET_TYPE_AUDIO == fileModOut->packetType
        && PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(audio)))
    {
        PTREE_PACKET_AUDIO_Obj_t *audioPacket = CONTAINER_OF(packet, PTREE_PACKET_AUDIO_Obj_t, base);
        _PTREE_MOD_FILE_ReadPcmAudioFile(fileModOut->fileReadFd, 1, &audioPacket->info.packetInfo,
                                         &audioPacket->packetData);
        return SSOS_DEF_OK;
    }
    if (E_PTREE_MOD_FILE_PACKET_TYPE_RAW == fileModOut->packetType
        && PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(raw)))
    {
        PTREE_PACKET_RAW_Obj_t *rawPacket = CONTAINER_OF(packet, PTREE_PACKET_RAW_Obj_t, base);
        _PTREE_MOD_FILE_ReadRawFile(fileModOut->fileReadFd, &rawPacket->info.rawInfo, &rawPacket->rawData);
        return SSOS_DEF_OK;
    }
    return SSOS_DEF_FAIL;
}
static int _PTREE_MOD_FILE_IsDelayLink(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_FILE_InObj_t * fileInObj  = CONTAINER_OF(modIn, PTREE_MOD_FILE_InObj_t, base);
    PTREE_SUR_FILE_InInfo_t *fileInInfo = CONTAINER_OF(fileInObj->base.info, PTREE_SUR_FILE_InInfo_t, base);
    return fileInInfo->frameCntLimit == 0;
}
static int _PTREE_MOD_FILE_InLinked(PTREE_MOD_InObj_t *modIn, unsigned int ref)
{
    PTREE_MOD_FILE_InObj_t * fileInObj  = NULL;
    PTREE_SUR_FILE_InInfo_t *fileInInfo = NULL;

    if (ref > 0)
    {
        return SSOS_DEF_OK;
    }
    fileInObj = CONTAINER_OF(modIn, PTREE_MOD_FILE_InObj_t, base);
    if (!fileInObj)
    {
        PTREE_ERR("fileModIn is null");
        return SSOS_DEF_FAIL;
    }
    fileInInfo = CONTAINER_OF(fileInObj->base.info, PTREE_SUR_FILE_InInfo_t, base);
    if (!fileInInfo)
    {
        PTREE_ERR("fileInInfo is null");
        return SSOS_DEF_FAIL;
    }
    fileInObj->fileWriteFd = SSOS_IO_FileOpen(fileInInfo->fileName, SSOS_IO_O_CREAT | SSOS_IO_O_WRONLY, 0755);
    if (!fileInObj->fileWriteFd)
    {
        PTREE_ERR("File %s open error", fileInInfo->fileName);
        return SSOS_DEF_FAIL;
    }
    if (0 != fileInInfo->frameCntLimit)
    {
        fileInObj->frameCntLimit = fileInInfo->frameCntLimit;
    }
    fileInObj->bAddHead = fileInInfo->bAddHead;
    fileInObj->isFinish = SSOS_DEF_FALSE;
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_FILE_InUnlinked(PTREE_MOD_InObj_t *modIn, unsigned int ref)
{
    PTREE_MOD_FILE_InObj_t *fileModIn = CONTAINER_OF(modIn, PTREE_MOD_FILE_InObj_t, base);
    if (ref > 0)
    {
        return SSOS_DEF_OK;
    }
    return SSOS_IO_FileClose(fileModIn->fileWriteFd);
}
static int _PTREE_MOD_FILE_InGetType(PTREE_MOD_InObj_t *modIn)
{
    (void)modIn;
    return E_PTREE_MOD_BIND_TYPE_USER;
}
static PTREE_LINKER_Obj_t *_PTREE_MOD_FILE_InCreateNLinker(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_FILE_InObj_t *fileModIn = CONTAINER_OF(modIn, PTREE_MOD_FILE_InObj_t, base);
    return PTREE_LINKER_Dup(&fileModIn->linker);
}
static PTREE_PACKER_Obj_t *_PTREE_MOD_FILE_InCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast)
{
    (void)modIn;
    *isFast = SSOS_DEF_FALSE;
    return PTREE_PACKER_NormalNew();
}
static void _PTREE_MOD_FILE_InDestruct(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_FILE_InObj_t *fileModIn = CONTAINER_OF(modIn, PTREE_MOD_FILE_InObj_t, base);
    PTREE_LINKER_Del(&fileModIn->linker);
    SSOS_THREAD_COND_DEINIT(&fileModIn->frameCond);
}
static void _PTREE_MOD_FILE_InFree(PTREE_MOD_InObj_t *modIn)
{
    SSOS_MEM_Free(CONTAINER_OF(modIn, PTREE_MOD_FILE_InObj_t, base));
}
static PTREE_MOD_InObj_t *_PTREE_MOD_FILE_InNew(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_FILE_InObj_t *fileModIn = NULL;

    fileModIn = SSOS_MEM_Alloc(sizeof(PTREE_MOD_FILE_InObj_t));
    if (!fileModIn)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(fileModIn, 0, sizeof(PTREE_MOD_FILE_InObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_InObjInit(&fileModIn->base, &G_PTREE_MOD_FILE_IN_OPS, mod, loopId))
    {
        goto ERR_MOD_IN_INIT;
    }
    if (SSOS_DEF_OK != PTREE_LINKER_Init(&fileModIn->linker, &G_PTREE_MOD_FILE_IN_LINKER_OPS))
    {
        goto ERR_LINKER_INIT;
    }

    PTREE_LINKER_Register(&fileModIn->linker, &G_PTREE_MOD_FILE_IN_LINKER_HOOK);
    SSOS_THREAD_COND_INIT(&fileModIn->frameCond);
    PTREE_MOD_InObjRegister(&fileModIn->base, &G_PTREE_MOD_FILE_IN_HOOK);
    return &fileModIn->base;

ERR_LINKER_INIT:
    PTREE_MOD_InObjDel(&fileModIn->base);
ERR_MOD_IN_INIT:
    SSOS_MEM_Free(fileModIn);
ERR_MEM_ALLOC:
    return NULL;
}

static int _PTREE_MOD_FILE_SetFilePacketInfo(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_FILE_OutObj_t * fileModOut  = CONTAINER_OF(modOut, PTREE_MOD_FILE_OutObj_t, base);
    PTREE_SUR_FILE_OutInfo_t *fileOutInfo = CONTAINER_OF(fileModOut->base.info, PTREE_SUR_FILE_OutInfo_t, base);

    if (fileModOut->fileReadFd != NULL)
    {
        PTREE_LOG("fileReadFd is already success");
        return SSOS_DEF_OK;
    }

    fileModOut->fileReadFd = SSOS_IO_FileOpen(fileOutInfo->fileName, SSOS_IO_O_RDONLY, 0);
    if (!fileModOut->fileReadFd)
    {
        PTREE_ERR("File %s open error", fileOutInfo->fileName);
        return SSOS_DEF_FAIL;
    }
    fileModOut->frameCntLimit = fileOutInfo->frameCntLimit;
    fileModOut->packetType    = PTREE_ENUM_FROM_STR(PTREE_MOD_FILE_PacketType_e, fileOutInfo->outType);
    switch (fileModOut->packetType)
    {
        case E_PTREE_MOD_FILE_PACKET_TYPE_RAW:
        {
            PTREE_PACKET_RAW_RawInfo_t rawInfo;
            memset(&rawInfo, 0, sizeof(PTREE_PACKET_RAW_RawInfo_t));
            rawInfo.fmt    = PTREE_ENUM_FROM_STR(PTREE_PACKET_RAW_VideoFmt_e, fileOutInfo->outFmt);
            rawInfo.width  = fileOutInfo->videoWidth;
            rawInfo.height = fileOutInfo->videoHeight;
            PTREE_PACKET_RAW_InfoInit(&fileModOut->rawInfo, &rawInfo);
        }
        break;
        case E_PTREE_MOD_FILE_PACKET_TYPE_VIDEO:
        {
            PTREE_PACKET_VIDEO_PacketInfo_t videoInfo;
            memset(&videoInfo, 0, sizeof(PTREE_PACKET_VIDEO_PacketInfo_t));
            videoInfo.fmt    = PTREE_ENUM_FROM_STR(PTREE_PACKET_VIDEO_Fmt_e, fileOutInfo->outFmt);
            videoInfo.width  = fileOutInfo->videoWidth;
            videoInfo.height = fileOutInfo->videoHeight;
            videoInfo.bHead  = 0;
            PTREE_PACKET_VIDEO_InfoInit(&fileModOut->videoInfo, &videoInfo);
        }
        break;
        case E_PTREE_MOD_FILE_PACKET_TYPE_AUDIO:
        {
            PTREE_PACKET_AUDIO_PacketInfo_t audioInfo;
            PTREE_MOD_FILE_WaveFileHeader_t wavHeader;
            memset(&audioInfo, 0, sizeof(PTREE_PACKET_AUDIO_PacketInfo_t));
            memset(&wavHeader, 0, sizeof(PTREE_MOD_FILE_WaveFileHeader_t));

            SSOS_IO_FileRead(fileModOut->fileReadFd, &wavHeader, sizeof(PTREE_MOD_FILE_WaveFileHeader_t));

            audioInfo.sampleRate  = wavHeader.waveInfo.samplesPerSec;
            audioInfo.sampleWidth = wavHeader.waveInfo.bitsPerSample;
            audioInfo.channels    = wavHeader.waveInfo.channels;
            audioInfo.fmt         = E_PTREE_PACKET_AUDIO_STREAM_PCM;
            audioInfo.pktCount    = 1;

            PTREE_PACKET_AUDIO_InfoInit(&fileModOut->audioInfo, &audioInfo);

            fileModOut->gapDataSize  = 0;
            fileModOut->lastTimeSec  = 0;
            fileModOut->lastTimeNsec = 0;
            fileModOut->gapTimeNs    = 0;
        }
        break;
        default:
            PTREE_ERR("File fmt %d not support", fileModOut->packetType);
            SSOS_IO_FileClose(fileModOut->fileReadFd);
            fileModOut->fileReadFd = NULL;
            return SSOS_DEF_FAIL;
    }
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_FILE_OutStart(PTREE_MOD_OutObj_t *modOut)
{
    return _PTREE_MOD_FILE_SetFilePacketInfo(modOut);
}
static int _PTREE_MOD_FILE_OutStop(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_FILE_OutObj_t *fileModOut = NULL;

    fileModOut = CONTAINER_OF(modOut, PTREE_MOD_FILE_OutObj_t, base);
    if (!fileModOut->fileReadFd)
    {
        PTREE_SUR_FILE_OutInfo_t *fileOutInfo = CONTAINER_OF(fileModOut->base.info, PTREE_SUR_FILE_OutInfo_t, base);
        PTREE_ERR("File %s did not open.", fileOutInfo->fileName);
        return SSOS_DEF_FAIL;
    }
    switch (fileModOut->packetType)
    {
        case E_PTREE_MOD_FILE_PACKET_TYPE_RAW:
            PTREE_PACKET_InfoDel(&fileModOut->rawInfo.base);
            break;
        case E_PTREE_MOD_FILE_PACKET_TYPE_VIDEO:
            PTREE_PACKET_InfoDel(&fileModOut->videoInfo.base);
            break;
        case E_PTREE_MOD_FILE_PACKET_TYPE_AUDIO:
            PTREE_PACKET_InfoDel(&fileModOut->audioInfo.base);
            break;
        default:
            break;
    }
    SSOS_IO_FileClose(fileModOut->fileReadFd);
    fileModOut->fileReadFd = NULL;
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_FILE_OutGetType(PTREE_MOD_OutObj_t *modOut)
{
    (void)modOut;
    return E_PTREE_MOD_BIND_TYPE_USER;
}
static PTREE_LINKER_Obj_t *_PTREE_MOD_FILE_OutCreateNLinker(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_FILE_OutObj_t *fileModOut = CONTAINER_OF(modOut, PTREE_MOD_FILE_OutObj_t, base);
    return &fileModOut->linker.base;
}
static PTREE_PACKET_Info_t *_PTREE_MOD_FILE_OutGetPacketInfo(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_FILE_OutObj_t *fileModOut = CONTAINER_OF(modOut, PTREE_MOD_FILE_OutObj_t, base);
    if (_PTREE_MOD_FILE_SetFilePacketInfo(modOut) != SSOS_DEF_OK)
    {
        return NULL;
    }

    switch (fileModOut->packetType)
    {
        case E_PTREE_MOD_FILE_PACKET_TYPE_RAW:
            return PTREE_PACKET_InfoDup(&fileModOut->rawInfo.base);
        case E_PTREE_MOD_FILE_PACKET_TYPE_VIDEO:
            return PTREE_PACKET_InfoDup(&fileModOut->videoInfo.base);
        case E_PTREE_MOD_FILE_PACKET_TYPE_AUDIO:
            return PTREE_PACKET_InfoDup(&fileModOut->audioInfo.base);
        default:
            return NULL;
    }
}
static int _PTREE_MOD_FILE_OutLinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref)
{
    PTREE_MOD_FILE_OutObj_t *fileModOut = NULL;
    SSOS_TASK_Attr_t         taskAttr;
    char                     taskName[32];

    fileModOut = CONTAINER_OF(modOut, PTREE_MOD_FILE_OutObj_t, base);
    if (!fileModOut->fileReadFd)
    {
        PTREE_SUR_FILE_OutInfo_t *fileOutInfo = CONTAINER_OF(fileModOut->base.info, PTREE_SUR_FILE_OutInfo_t, base);
        PTREE_ERR("File %s did not open.", fileOutInfo->fileName);
        return SSOS_DEF_FAIL;
    }
    if (ref > 0)
    {
        return SSOS_DEF_OK;
    }

    if (PTREE_PLINKER_GROUP_Empty(&modOut->plinker) || modOut->nlinker)
    {
        return SSOS_DEF_OK;
    }
    memset(&taskAttr, 0, sizeof(SSOS_TASK_Attr_t));
    switch (fileModOut->packetType)
    {
        case E_PTREE_MOD_FILE_PACKET_TYPE_RAW:
            taskAttr.doMonitor = _PTREE_MOD_FILE_RawFileReader;
            break;
        case E_PTREE_MOD_FILE_PACKET_TYPE_VIDEO:
            taskAttr.doMonitor = _PTREE_MOD_FILE_EsFileReader;
            break;
        case E_PTREE_MOD_FILE_PACKET_TYPE_AUDIO:
            taskAttr.doMonitor = _PTREE_MOD_FILE_PcmFileReader;
            break;
        default:
            PTREE_ERR("File fmt %d not support", fileModOut->packetType);
            return SSOS_DEF_FAIL;
    }
    if (modOut->info->fps == 1)
    {
        taskAttr.monitorCycleSec  = 1;
        taskAttr.monitorCycleNsec = 0;
    }
    else
    {
        taskAttr.monitorCycleSec  = 0;
        taskAttr.monitorCycleNsec = 1000000000 / ((int)modOut->info->fps);
    }
    taskAttr.inBuf.buf       = (void *)fileModOut;
    taskAttr.threadAttr.name = PTREE_MOD_OutKeyStr(modOut, taskName, 32);
    fileModOut->taskHandle   = SSOS_TASK_Open(&taskAttr);
    if (!fileModOut->taskHandle)
    {
        PTREE_ERR("File reader task open failed");
        return SSOS_DEF_FAIL;
    }
    SSOS_TASK_StartMonitor(fileModOut->taskHandle);
    PTREE_DBG("File %s Out %d reader start.", fileModOut->base.thisMod->info->sectionName, fileModOut->base.info->port);
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_FILE_OutUnlinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref)
{
    PTREE_MOD_FILE_OutObj_t *fileModOut = CONTAINER_OF(modOut, PTREE_MOD_FILE_OutObj_t, base);
    if (ref > 0)
    {
        return SSOS_DEF_OK;
    }
    if (!fileModOut->taskHandle)
    {
        return SSOS_DEF_OK;
    }
    SSOS_TASK_Stop(fileModOut->taskHandle);
    SSOS_TASK_Close(fileModOut->taskHandle);
    fileModOut->taskHandle = NULL;
    PTREE_DBG("File %s Out %d reader stop.", fileModOut->base.thisMod->info->sectionName, fileModOut->base.info->port);
    return SSOS_DEF_OK;
}
static void _PTREE_MOD_FILE_OutDestruct(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_FILE_OutObj_t *fileModOut = CONTAINER_OF(modOut, PTREE_MOD_FILE_OutObj_t, base);
    PTREE_LINKER_Del(&fileModOut->linker.base);
}
static void _PTREE_MOD_FILE_OutFree(PTREE_MOD_OutObj_t *modOut)
{
    SSOS_MEM_Free(CONTAINER_OF(modOut, PTREE_MOD_FILE_OutObj_t, base));
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_FILE_OutNew(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_FILE_OutObj_t *fileModOut = NULL;

    fileModOut = SSOS_MEM_Alloc(sizeof(PTREE_MOD_FILE_OutObj_t));
    if (!fileModOut)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(fileModOut, 0, sizeof(PTREE_MOD_FILE_OutObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_OutObjInit(&fileModOut->base, &G_PTREE_MOD_FILE_OUT_OPS, mod, loopId))
    {
        goto ERR_MOD_OUT_INIT;
    }
    if (SSOS_DEF_OK != PTREE_NLINKER_OUT_Init(&fileModOut->linker, &G_PTREE_MOD_FILE_OUT_LINKER_OPS))
    {
        goto ERR_LINKER_INIT;
    }

    PTREE_NLINKER_OUT_Register(&fileModOut->linker, &G_PTREE_MOD_FILE_OUT_LINKER_HOOK);
    PTREE_MOD_OutObjRegister(&fileModOut->base, &G_PTREE_MOD_FILE_OUT_HOOK);
    return &fileModOut->base;

ERR_LINKER_INIT:
    PTREE_MOD_OutObjDel(&fileModOut->base);
ERR_MOD_OUT_INIT:
    SSOS_MEM_Free(fileModOut);
ERR_MEM_ALLOC:
    return NULL;
}

static PTREE_MOD_InObj_t *_PTREE_MOD_FILE_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    return _PTREE_MOD_FILE_InNew(mod, loopId);
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_FILE_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    return _PTREE_MOD_FILE_OutNew(mod, loopId);
}
static void _PTREE_MOD_FILE_Free(PTREE_MOD_Obj_t *mod)
{
    SSOS_MEM_Free(CONTAINER_OF(mod, PTREE_MOD_FILE_Obj_t, base));
}
PTREE_MOD_Obj_t *PTREE_MOD_FILE_New(PARENA_Tag_t *tag)
{
    PTREE_MOD_FILE_Obj_t *fileMod = NULL;

    fileMod = SSOS_MEM_Alloc(sizeof(PTREE_MOD_FILE_Obj_t));
    if (!fileMod)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(fileMod, 0, sizeof(PTREE_MOD_FILE_Obj_t));
    if (SSOS_DEF_OK != PTREE_MOD_ObjInit(&fileMod->base, &G_PTREE_MOD_FILE_OPS, tag))
    {
        SSOS_MEM_Free(fileMod);
        return NULL;
    }
    PTREE_MOD_ObjRegister(&fileMod->base, &G_PTREE_MOD_FILE_HOOK);
    return &fileMod->base;
}

PTREE_MAKER_MOD_INIT(FILE, PTREE_MOD_FILE_New);
