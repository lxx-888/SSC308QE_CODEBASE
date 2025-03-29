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
// AV1 video sources
// Implementation

#include "AV1VideoSource.hh"

AV1VideoSource::AV1VideoSource(UsageEnvironment& env)
  : FramedSource(env) {
}

AV1VideoSource::~AV1VideoSource() {
}

u_int8_t const* AV1VideoSource::quantizationTables(u_int8_t& precision,
						    u_int16_t& length) {
  // Default implementation
  precision = 0;
  length = 0;
  return NULL;
}

u_int16_t AV1VideoSource::restartInterval() {
  // Default implementation
  return 0;
}

Boolean AV1VideoSource::isAV1VideoSource() const {
  return True;
}
