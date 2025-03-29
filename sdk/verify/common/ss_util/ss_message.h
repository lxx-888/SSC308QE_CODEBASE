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
#ifndef __SS_MESSAGE__
#define __SS_MESSAGE__

#include "ss_auto_lock.h"
#include "ss_packet.h"

enum
{
   E_SS_MSG_CONNECTION_IN    = 0x1,
   E_SS_MSG_CONNECTION_OUT   = 0x2,
   E_SS_MSG_CONNECTION_BUILD = 0x3
};
struct ss_message_info
{
    unsigned int state;
    unsigned int refer_count;
    ss_message_info()
        : state(0), refer_count(0)
    {}
};
class ss_message
{
public:
    explicit ss_message()
    {
        access_lock = PTHREAD_RWLOCK_INITIALIZER;
    }
    virtual ~ss_message()
    {

    }
    virtual void connected(unsigned int ref) = 0;
    virtual void disconnected(unsigned int ref) = 0;
    bool connection_check()
    {
        ss_auto_rdlock lock(access_lock);
        return (message_info.state == E_SS_MSG_CONNECTION_BUILD) ?
            true : false;
    }
    void access()
    {
        ss_auto_rwlock lock(access_lock);
        message_info.state |= E_SS_MSG_CONNECTION_OUT;
        message_info.refer_count++;
        if (message_info.state == E_SS_MSG_CONNECTION_BUILD)
        {
            connected(message_info.refer_count - 1);
        }
    }
    void leave(bool b_force = false)
    {
        ss_auto_rwlock lock(access_lock);
        if (b_force)
        {
            message_info.refer_count = 0;
        }
        else if (message_info.refer_count)
        {
            message_info.refer_count--;
        }
        if (message_info.state == E_SS_MSG_CONNECTION_BUILD)
        {
            disconnected(message_info.refer_count);
        }
        if (message_info.refer_count == 0)
        {
            message_info.state &= (~(E_SS_MSG_CONNECTION_OUT& 0xFFFFFFFF));
        }
    }
    void connect_in()
    {
        ss_auto_rwlock lock(access_lock);
        message_info.state |=  E_SS_MSG_CONNECTION_IN;
        if (message_info.state == E_SS_MSG_CONNECTION_BUILD)
        {
            for (unsigned int ref = 0; ref < message_info.refer_count; ++ref)
            {
                connected(ref);
            }
        }
    }
    void disconnect_in()
    {
        ss_auto_rwlock lock(access_lock);
        if (message_info.state == E_SS_MSG_CONNECTION_BUILD)
        {
            for (unsigned int ref = message_info.refer_count; ref > 0; --ref)
            {
                disconnected(ref - 1);
            }
        }
        message_info.state &= (~(E_SS_MSG_CONNECTION_IN & 0xFFFFFFFF));
    }
    virtual stream_packet_info get_packet_info() = 0;
private:
    struct ss_message_info message_info;
    pthread_rwlock_t access_lock;

};
#endif

