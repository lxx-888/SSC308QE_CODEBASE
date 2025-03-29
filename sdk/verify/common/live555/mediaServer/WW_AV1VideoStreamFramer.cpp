/*
 * Copyright (C) 2005-2006 WIS Technologies International Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and the associated README documentation file (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
// A class that encapsulates a 'FramedSource' object that
// returns AV1 video frames.
// C++ header

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "assert.h"
#include "WW_AV1VideoStreamFramer.hh"

enum {
	OBU_SEQUENCE_HEADER = 1,
	OBU_TEMPORAL_DELIMITER = 2,
	OBU_FRAME_HEADER = 3,
	OBU_TILE_GROUP = 4,
	OBU_METADATA = 5,
	OBU_FRAME = 6,
	OBU_REDUNDANT_FRAME_HEADER = 7,
	OBU_TILE_LIST = 8,
	// 9-14	Reserved
	OBU_PADDING = 15,
};

#define RTP_FIXED_HEADER 12
// #define verbose     fprintf
#define verbose(...)    do {  } while (0)
#if 0
static void* rtp_alloc(void* param, int bytes)
{
	// struct av1_rtp_test_t* ctx = (struct av1_rtp_test_t*)param;
    uint8_t *buffer;

    buffer = (uint8_t*)malloc(bytes + 4);
	assert(!!buffer);
    *(uint32_t*)buffer = 1; // 00, 00, 00, 01
	return buffer + 4;
}

static void rtp_free(void* param, void* packet)
{
	// struct av1_rtp_test_t* ctx = (struct av1_rtp_test_t*)param;
    uint8_t *buffer;
	assert(!!packet);
    buffer = (uint8_t*)packet - 4;
    free(buffer);
}

static int rtp_encoded_packet(void* param, const void* packet, int bytes, uint32_t timestamp, int flags)
{
	struct av1_rtp_test_t* ctx = (struct av1_rtp_test_t*)param;
    const uint8_t   *data = (const uint8_t*)packet;

    memcpy(ctx->fTo + ctx->offset, (uint8_t*)packet + RTP_FIXED_HEADER, bytes - RTP_FIXED_HEADER);
    ctx->offset += (bytes - RTP_FIXED_HEADER);
	verbose(stderr, ">> RTP Encode: timestamp: %u, bytes: %d\n", (unsigned int)timestamp, bytes);
    verbose(stderr, "   %02x %02x %02x %02x %02x %02x %02x %02x\n",
            *data, *(data+1), *(data+2), *(data+3), *(data+4), *(data+5), *(data+6), *(data+7));
    data += 8;
    verbose(stderr, "   %02x %02x %02x %02x %02x %02x %02x %02x\n",
            *data, *(data+1), *(data+2), *(data+3), *(data+4), *(data+5), *(data+6), *(data+7));
    data += 8;
    verbose(stderr, "   %02x %02x %02x %02x %02x %02x %02x %02x\n",
            *data, *(data+1), *(data+2), *(data+3), *(data+4), *(data+5), *(data+6), *(data+7));
    verbose(stderr, "   seq_profile %u, seq_level %u, seq_tier %u\n",
                (unsigned int)ctx->av1.seq_profile,
                (unsigned int)ctx->av1.seq_level_idx_0,
                (unsigned int)ctx->av1.seq_tier_0);
    verbose(stderr, "   size %dx%d\n", ctx->av1.width, ctx->av1.height);

    return 0;
}
#endif 
WW_AV1VideoStreamSource*
WW_AV1VideoStreamSource::createNew(UsageEnvironment& env, FramedSource* source)
{
    return new WW_AV1VideoStreamSource(env, source);
}

WW_AV1VideoStreamSource::WW_AV1VideoStreamSource(UsageEnvironment& env, FramedSource* source)
    : AV1VideoSource(env), fSource(source), fType(0), fWidth(0), fHeight(0), fPrecision(0), fQtableLength(0),
    fWidthPixels(0), fHeightPixels(0)
{
	//struct rtp_payload_t handler2;
	//handler2.alloc  = rtp_alloc;
	//handler2.free   = rtp_free;
	//handler2.packet = rtp_encoded_packet;

    ctx = (struct av1_rtp_test_t*)malloc(sizeof(struct av1_rtp_test_t));
	// ctx->encoder = rtp_payload_encode_create(35, "AV1", 0, 0, &handler2, ctx);
}

WW_AV1VideoStreamSource::~WW_AV1VideoStreamSource()
{
    if (ctx) {
        free(ctx);
    }
    Medium::close(fSource);
}

u_int8_t const* WW_AV1VideoStreamSource::quantizationTables(u_int8_t& precision, u_int16_t& length)
{
    precision = fPrecision;
    length = fQtableLength;
    return fQuantizationTable;
}

void WW_AV1VideoStreamSource::doGetNextFrame()
{
    if(fSource)
    {
        fSource->getNextFrame(fTo, fMaxSize,
                              afterGettingFrame, this,
                              FramedSource::handleClosure, this);
    }
}

void WW_AV1VideoStreamSource::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
        struct timeval presentationTime, unsigned durationInMicroseconds)
{
    WW_AV1VideoStreamSource* source = (WW_AV1VideoStreamSource*) clientData;
    verbose(stderr, "====> %s %d frameSize %u %ld:%ld dur %u\n", __func__, __LINE__,
                    frameSize, presentationTime.tv_sec, presentationTime.tv_usec, durationInMicroseconds);
    while ((int)frameSize > 0) {
        // manuplate data
        uint8_t *ext = (uint8_t*)malloc(frameSize);
        uint32_t dts = 0;   // TODO:
        if (!ext) {
            fprintf(stderr, "%s %d:No memory\n", __func__, __LINE__);
            break;
        }
        uint8_t *ptr = (uint8_t*)source->fTo;
	    uint8_t obu_type = (*ptr >> 3) & 0x0F;
	    if (OBU_SEQUENCE_HEADER == obu_type) {
            aom_av1_codec_configuration_record_init(&source->ctx->av1, ptr, frameSize);
        }
#if 0
        memcpy(ext, source->fTo, frameSize);
        source->ctx->offset = 0;
        source->ctx->fTo = (uint8_t*)source->fTo;
		if (rtp_payload_encode_input(source->ctx->encoder, ext, frameSize, (unsigned int)dts) != 0) {
            fprintf(stderr, "%s %d:payload encode error\n", __func__, __LINE__);
            break;
        }
        frameSize = source->ctx->offset;
#endif
        free(ext);
        break;
    }
    source->afterGettingFrame1(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
    verbose(stderr, "<==== %s %d\n", __func__, __LINE__);
}

void WW_AV1VideoStreamSource::afterGettingFrame1(unsigned frameSize, unsigned numTruncatedBytes,
        struct timeval presentationTime, unsigned durationInMicroseconds)
{
    fNumTruncatedBytes = 0;
    fFrameSize = frameSize;
    fPresentationTime = presentationTime;
	// TODO: What is fType? Canlet
    fType = 1;  // YUV 4:2:0
    verbose(stderr, "++++> %s %d frameSize %u %ld:%ld dur %u\n", __func__, __LINE__,
                    frameSize, presentationTime.tv_sec, presentationTime.tv_usec, durationInMicroseconds);
    afterGetting(this);
    verbose(stderr, "<++++ %s %d\n", __func__, __LINE__);
}

