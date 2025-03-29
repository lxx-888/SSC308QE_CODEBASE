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

#ifndef __AUDIO_CODE_H__
#define __AUDIO_CODE_H__

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "ss_enum_cast.hpp"
#include "g711.h"
#include "g726.h"

typedef enum
{
    E_G711U,
    E_G711A,
    E_G726_16,
    E_G726_24,
    E_G726_32,
    E_G726_40,
    E_AAC
} EN_CODE_TYPE;
SS_ENUM_CAST_STR(EN_CODE_TYPE,
{
    {E_G711U,   "g711u"   },
    {E_G711A,   "g711a"   },
    {E_G726_16, "g726_16" },
    {E_G726_24, "g726_24" },
    {E_G726_32, "g726_32" },
    {E_G726_40, "g726_40" },
    {E_AAC,     "aac"     },
});

class AudioCode
{
public:
    virtual unsigned int decode(short *pcm, const unsigned char *code, unsigned int len) = 0;
    virtual unsigned int encode(unsigned char *code, const short *pcm, unsigned int len) = 0;
    virtual ~AudioCode() {}
};
class G711U : public AudioCode
{
public:
    G711U() {};
    ~G711U() override {};
    unsigned int decode(short *pcm, const unsigned char *code, unsigned int len) override
    {
        unsigned int i;
        if (nullptr == pcm || nullptr == code || 0 == len)
        {
            return -1;
        }
        for (i = 0; i < len; i++)
        {
            *(pcm++) = ulaw2linear(*(code++));
        }
        return len*2;
    };
    unsigned int encode(unsigned char *code, const short *pcm, unsigned int len) override
    {
        unsigned int i;
        for (i = 0;  i < len/2;  i++)
        {
            *(code++) = linear2ulaw(*(pcm++));
        }
        return len/2;
    }
};
class G711A : public AudioCode
{
public:
    G711A() {};
    ~G711A() override {};
    unsigned int decode(short *pcm, const unsigned char *code, unsigned int len) override
    {
        unsigned int i;
        if (nullptr == pcm || nullptr == code || 0 == len)
        {
            return -1;
        }
        for (i = 0; i < len; i++)
        {
            *(pcm++) = alaw2linear(*(code++));
        }
        return len*2;
    };
    unsigned int encode(unsigned char *code, const short *pcm, unsigned int len) override
    {
        unsigned int i;
        for (i = 0;  i < len/2;  i++)
        {
            *(code++) = linear2alaw(*(pcm++));
        }
        return len/2;
    };
};

typedef enum
{
    E_AENC_TYPE_G726_16 = 2, // convenient calculation
    E_AENC_TYPE_G726_24,
    E_AENC_TYPE_G726_32,
    E_AENC_TYPE_G726_40,
} g726_type_e;
class G726 : public AudioCode
{
public:
    G726(g726_type_e bps):g726_state(nullptr)
    {
        g726_state = (g726_state_t *)malloc(sizeof(g726_state_t));
        if (g726_state)
        {
            g726_init(g726_state, 8000*bps);
        }
    };
    ~G726() override
    {
        free(g726_state);
        g726_state = nullptr;
    };
    unsigned int decode(short *pcm, const unsigned char *code, unsigned int len) override
    {
        if (g726_state && pcm && code && len > 0)
        {
            return (2 * g726_decode(g726_state, pcm, code, len));
        }
        return -1;
    };
    unsigned int encode(unsigned char *code, const short *pcm, unsigned int len) override
    {
        if (g726_state && pcm && code && len > 0)
        {
            return g726_encode(g726_state, code, pcm, len/2);
        }
        return -1;
    };
private:
    g726_state_t *g726_state;
};
#endif // !__AUDIO_CODE_H__
