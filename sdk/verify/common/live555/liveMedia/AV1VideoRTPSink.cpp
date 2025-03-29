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
// Copyright (c) 1996-2019 Live Networks, Inc.  All rights reserved.
// RTP sink for AV1 video (RFC 2435)
// Implementation

#include "AV1VideoRTPSink.hh"
#include "AV1VideoSource.hh"

AV1VideoRTPSink
::AV1VideoRTPSink(UsageEnvironment& env, Groupsock* RTPgs)
  : VideoRTPSink(env, RTPgs, 35, 90000, "AV1") {
    setPacketSizes(1434, 1434);
}

AV1VideoRTPSink::~AV1VideoRTPSink() {
}

AV1VideoRTPSink*
AV1VideoRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs) {
  return new AV1VideoRTPSink(env, RTPgs);
}

Boolean AV1VideoRTPSink::sourceIsCompatibleWithUs(MediaSource& source) {
  return source.isAV1VideoSource();
}

Boolean AV1VideoRTPSink
::frameCanAppearAfterPacketStart(unsigned char const* /*frameStart*/,
				 unsigned /*numBytesInFrame*/) const {
  // A packet can contain only one frame
  return False;
}

void AV1VideoRTPSink
::doSpecialFrameHandling(unsigned fragmentationOffset,
			 unsigned char* frameStart,
			 unsigned numBytesInFrame,
			 struct timeval framePresentationTime,
			 unsigned numRemainingBytes) {
  // Our source is known to be a AV1VideoSource
  AV1VideoSource* source = (AV1VideoSource*)fSource;
  if (source == NULL) return; // sanity check

  // fill extra payload in a frame
  if (fragmentationOffset == 0) {
	fFramePresentationTime = framePresentationTime;  
  }
  if (numRemainingBytes == 0) {
    // This packet contains the last (or only) fragment of the frame.
    // Set the RTP 'M' ('marker') bit:
    setMarkerBit();
  }

  // Also set the RTP timestamp:
  setTimestamp(fFramePresentationTime);
}


unsigned AV1VideoRTPSink::specialHeaderSize() const {
  // Our source is known to be a AV1VideoSource
  AV1VideoSource* source = (AV1VideoSource*)fSource;
  if (source == NULL) return 0; // sanity check

  unsigned headerSize = 0; // for start code

  return headerSize;
}
