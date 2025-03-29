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

#ifndef _WW_AV1_VIDEO_STREAM_SOURCE
#define _WW_AV1_VIDEO_STREAM_SOURCE

#ifndef _AV1_VIDEO_SOURCE_HH
#include <AV1VideoSource.hh>
#endif

#include "libflv/aom-av1.h"
#include "librtp/rtp-payload.h"
extern "C" {
struct av1_rtp_test_t
{
	struct aom_av1_t av1;
	void* encoder;
	// void* decoder;

	uint32_t pts, dts;
	uint8_t* ptr;
	int bytes;

	int vcl;
	int keyframe;

	// FILE* wfp;
    uint8_t *str;
    long    str_bytes;
    char    sdp[1024];
    uint8_t *fTo;
    uint32_t offset;
};
}

class WW_AV1VideoStreamSource: public AV1VideoSource
{
public:
    static WW_AV1VideoStreamSource* createNew(UsageEnvironment& env, FramedSource* source);

private:
    WW_AV1VideoStreamSource(UsageEnvironment& env, FramedSource* source);
    // called only by createNew()

    virtual ~WW_AV1VideoStreamSource();

public: // redefined virtual functions
    virtual void doGetNextFrame();

    virtual u_int8_t type()
    {
        return fType;
    };
    virtual u_int8_t qFactor()
    {
        return 255;
    };
    virtual u_int8_t width()
    {
        return fWidth;
    };
    virtual u_int8_t height()
    {
        return fHeight;
    };
    virtual u_int16_t widthPixels()
    {
        return fWidthPixels;
    };
    virtual u_int16_t heightPixels()
    {
        return fHeightPixels;
    };
    virtual u_int8_t const* quantizationTables(u_int8_t& precision,
            u_int16_t& length);
private:
    static void afterGettingFrame(void* clientData, unsigned frameSize,
                                  unsigned numTruncatedBytes,
                                  struct timeval presentationTime,
                                  unsigned durationInMicroseconds);
    void afterGettingFrame1(unsigned frameSize, unsigned numTruncatedBytes,
                            struct timeval presentationTime,
                            unsigned durationInMicroseconds);

private:
    FramedSource* fSource;
    u_int8_t fType;
    u_int8_t fWidth, fHeight; // actual dimensions /8
    u_int8_t fPrecision;
    u_int8_t fQuantizationTable[128];
    u_int16_t fQtableLength;
    u_int16_t fWidthPixels, fHeightPixels; // actual pixels, can support width/height more than 2048
                                           //
    //
    struct av1_rtp_test_t   *ctx;
};

#endif

