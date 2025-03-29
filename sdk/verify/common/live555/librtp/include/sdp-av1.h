#ifndef _sdp_payload_h_
#define _sdp_payload_h_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int sdp_av1(uint8_t* data, int bytes, const char* proto, unsigned short port, int payload, int frequence, const void* extra, int extra_size);
/// @param[in] rtp rtp payload, see more @rtp-profile.h and @rtsp-payloads.h
int sdp_payload_video(uint8_t* data, int bytes, int rtp, const char* proto, unsigned short port, int payload, int frequence, const void* extra, int extra_size);

#ifdef __cplusplus
}
#endif
#endif /* !_sdp_payload_h_ */
