/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2017 Live Networks, Inc.  All rights reserved.
// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand, from a H265 video file.
// Implementation

#include "WW_JPEGVideoFileServerMediaSubsession.hh"
#include "JPEGVideoRTPSink.hh"
#include "WW_ByteStreamFileSource.hh"
#include "WW_JPEGVideoStreamFramer.hh"

WW_JPEGVideoFileServerMediaSubsession*
WW_JPEGVideoFileServerMediaSubsession::createNew(UsageEnvironment& env
										, char const* fileName, OPENSTREAM  OpenStreamFun
										, READSTREAM  ReadStreamFun, CLOSESTREAM CloseStreamFun
										, double iFrameRate
										, void* CallbackFunArg, int iCallbackFunArgSize
										, Boolean copyArgFlag, Boolean reuseFirstSource
                                                    , unsigned maxSize
                                                    , unsigned bitRate)
{
	if ( OpenStreamFun==NULL||ReadStreamFun==NULL||CloseStreamFun==NULL) {
		return NULL;
	}
	return new WW_JPEGVideoFileServerMediaSubsession(env, fileName, reuseFirstSource
									, OpenStreamFun, ReadStreamFun, CloseStreamFun
									, iFrameRate
									, CallbackFunArg, iCallbackFunArgSize, copyArgFlag
                            , maxSize, bitRate);
}

WW_JPEGVideoFileServerMediaSubsession::WW_JPEGVideoFileServerMediaSubsession(UsageEnvironment& env
										, char const* fileName, Boolean reuseFirstSource
										, OPENSTREAM  OpenStreamFun, READSTREAM  ReadStreamFun
										, CLOSESTREAM CloseStreamFun, double iFrameRate
										, void* CallbackFunArg
										, int iCallbackFunArgSize, Boolean copyArgFlag
                                                    , unsigned maxSize
                                                    , unsigned bitRate)
	: OnDemandServerMediaSubsession(env, reuseFirstSource)
	, fAuxSDPLine(NULL)
	, fDoneFlag(0)
    , fDummySource(NULL)
	, fMaxInputBufSize(maxSize)
    , fStreamBitRate(bitRate)
    , fFileSize(0)
{
	m_OpenStreamFun       = OpenStreamFun;
	m_ReadStreamFun       = ReadStreamFun;
	m_CloseStreamFun      = CloseStreamFun;
	m_CallbackFunArg      = CallbackFunArg;
	m_iCallbackFunArgSize = iCallbackFunArgSize;
	m_CopyArgFlag         = copyArgFlag;
	m_fFrameRate          = iFrameRate;
	if ( m_CallbackFunArg&&(m_iCallbackFunArgSize>0)&&m_CopyArgFlag ) {
		m_CallbackFunArg = calloc(1, m_iCallbackFunArgSize);
		memcpy(m_CallbackFunArg, CallbackFunArg, m_iCallbackFunArgSize);
	}
	memset(fFileName, 0, sizeof(fFileName));
	strcpy(fFileName, fileName);
}

WW_JPEGVideoFileServerMediaSubsession::~WW_JPEGVideoFileServerMediaSubsession()
{
	if ( fAuxSDPLine ) {
		delete[] fAuxSDPLine;
		fAuxSDPLine = NULL;
	}
	if ( m_CallbackFunArg&&(m_iCallbackFunArgSize>0)&&m_CopyArgFlag ) {
		free(m_CallbackFunArg);
		m_CallbackFunArg = NULL;
	}
}

static void afterPlayingDummy(void* clientData)
{
	WW_JPEGVideoFileServerMediaSubsession* subsess
		= (WW_JPEGVideoFileServerMediaSubsession*)clientData;
	subsess->afterPlayingDummy1();
}

void WW_JPEGVideoFileServerMediaSubsession::afterPlayingDummy1()
{
	// Unschedule any pending 'checking' task:
	envir().taskScheduler().unscheduleDelayedTask(nextTask());
	// Signal the event loop that we're done:
	setDoneFlag();
}

static void checkForAuxSDPLine(void* clientData) {
    WW_JPEGVideoFileServerMediaSubsession* subsess = (WW_JPEGVideoFileServerMediaSubsession*)clientData;
    subsess->checkForAuxSDPLine1();
}

void WW_JPEGVideoFileServerMediaSubsession::checkForAuxSDPLine1() {
    nextTask() = NULL;

    if (fAuxSDPLine != NULL) {
        // Signal the event loop that we're done:
        setDoneFlag();
    } else if (fDummySource != NULL && this->getAuxSDP() != NULL) {
        // Signal the event loop that we're done:
        fDummySource = NULL;
        setDoneFlag();
    } else if (!fDoneFlag) {
        // try again after a brief delay:
        int uSecsToDelay = 100000; // 100 ms
        nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay,
                    (TaskFunc*)checkForAuxSDPLine, this);
    }
}

char const* WW_JPEGVideoFileServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource)
{
	if (fAuxSDPLine != NULL) return fAuxSDPLine; // it's already been set up (for a previous client)

    if (fDummySource == NULL)
    {
        fDummySource = (WW_JPEGVideoStreamSource *)inputSource;
        // JPEG Need get w / h from SOF_MARKER of jpeg stream data
        rtpSink->startPlaying(*inputSource, afterPlayingDummy, this);
        // check fAuxSDPLine ready
        checkForAuxSDPLine(this);
    }

    // Wait setDoneFlag
    envir().taskScheduler().doEventLoop(&fDoneFlag);

	return fAuxSDPLine;
}

char const* WW_JPEGVideoFileServerMediaSubsession::getAuxSDP()
{
    unsigned short width, height;
    width = fDummySource->widthPixels();
    height = fDummySource->heightPixels();

    if (!width || !height)
    {
        return fAuxSDPLine;
    }

	char const* addSDPFormat = "a=x-dimensions:%d,%d\r\n";
	unsigned addSDPFormatSize = strlen(addSDPFormat) + 4 + 4 + 1;
	char* fmtp = new char[addSDPFormatSize];
	sprintf(fmtp, addSDPFormat, width, height);
    fAuxSDPLine = strdup(fmtp);
    delete []fmtp;
    return fAuxSDPLine;
}

FramedSource* WW_JPEGVideoFileServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate)
{
	estBitrate = fStreamBitRate; // kbps, estimate
	// Create the video source:
	WW_ByteStreamFileSource* fileSource = WW_ByteStreamFileSource::createNew(envir(), fFileName
						, m_OpenStreamFun, m_ReadStreamFun, m_CloseStreamFun, m_CallbackFunArg);
	if (fileSource == NULL) return NULL;
	//fFileSize = fileSource->fileSize();

	// Create a framer for the Video Elementary Stream:
	return WW_JPEGVideoStreamSource::createNew(envir(), fileSource);
}

RTPSink* WW_JPEGVideoFileServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock
					, unsigned char rtpPayloadTypeIfDynamic, FramedSource* /*inputSource*/)
{
	OutPacketBuffer::maxSize = fMaxInputBufSize;
	return JPEGVideoRTPSink::createNew(envir(), rtpGroupsock);
}

