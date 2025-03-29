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

#ifndef __AMIGOS_ENV_H__
#define __AMIGOS_ENV_H__

#include <time.h>
#include <chrono>
#include <map>
#include <queue>
#include <string>
#include "ss_auto_lock.h"

class AmigosEnv
{
public:
    class ValueBuffer
    {
    public:
        ValueBuffer(char *v);
        ValueBuffer &operator=(const char *v);
        ValueBuffer &operator=(const std::string &v);
        // make this to be right value like this std::string str = env["ssss"];
        operator std::string () const;
    private:
        char *value;
    };
    AmigosEnv(const std::string &modName);
    ~AmigosEnv();
    ValueBuffer operator [](const std::string &key);
    void Dump(std::map<std::string, std::string> &mapEnv);
    AmigosEnv &In(unsigned int port);
    AmigosEnv &Out(unsigned int port);
    AmigosEnv &Ext(const char *extSec);

    void MonitorOut(unsigned int portId, unsigned int size, bool isIdr, bool bFrameEnd);
    void MonitorOut(unsigned int portId, unsigned int size);
    void MonitorOut(unsigned int portId);

    static int EnableShm(bool isFather);
    static int DisableShm();

private:
    struct AMIGOS_ENV_ModTag_s    *AllocModTag(const std::string &modName);
    struct AMIGOS_ENV_KeyValTag_s *AllocEnvTag(const std::string &key);
    struct AMIGOS_ENV_ModTag_s *modTag;
    pthread_mutex_t mutexSubEnv;
    std::map<std::string, AmigosEnv *> inEnv;
    std::map<std::string, AmigosEnv *> outEnv;
    std::map<std::string, AmigosEnv *> extEnv;
    std::map<unsigned int, struct AMIGOS_ENV_FrameContext_s> mapFpsCtx;
};

static inline std::ostream &operator<<(std::ostream &lval, const AmigosEnv::ValueBuffer& rval)
{
    return lval << (std::string)rval;
}
#endif /* __AMIGOS_ENV_H__ */
