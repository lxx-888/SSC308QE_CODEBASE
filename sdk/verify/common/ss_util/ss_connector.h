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
#ifndef __SS_CONNECTOR__
#define __SS_CONNECTOR__

#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <atomic>
#include <list>
#include <map>
#include <iostream>
#include <memory>
#include "ss_auto_lock.h"
#include "ss_handle.h"
#include "ss_packet.h"

enum en_state
{
   EN_CONNECTION_IN    = 0x1,
   EN_CONNECTION_OUT   = 0x2,
   EN_CONNECTION_BUILD = 0x3
};
typedef int (*approch_cb)(void *user);
typedef int (*packet_to_cb)(stream_packet_obj &packet, void *user);
typedef int (*connection_cb)(void *user, unsigned int ref);
typedef stream_packet_obj (*packet_get_cb)(void *user, int delay_ms);
typedef void (*packet_put_cb)(stream_packet_obj &packet, void *user);
struct packet_to
{
    approch_cb   approch;
    packet_to_cb send;
    void         *user;
    packet_to()
        : approch(NULL), send(NULL), user(NULL)
    {}
};
struct packet_from
{
    packet_get_cb recv_get;
    packet_put_cb recv_put;
    void          *user;
    packet_from()
        : recv_get(NULL), recv_put(NULL), user(NULL)
    {}
};
struct connection_info
{
    unsigned int              state;
    unsigned int              refer_count;
    void                      *user;
    connection_cb             in_access;
    connection_cb             in_leave;
    struct stream_packet_info packet_info;
    connection_info ()
    : state(0), refer_count(0), user(NULL), in_access(0), in_leave(0)
    {
    }
};
class ss_connector_base : public ss_handle
{
public:
    explicit ss_connector_base()
    {
        connector_mutex       = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
        connector_access_lock = PTHREAD_RWLOCK_INITIALIZER;
        connector_data_lock   = PTHREAD_RWLOCK_INITIALIZER;
        pthread_condattr_init(&connector_cond_attr);
        pthread_condattr_setclock(&connector_cond_attr, CLOCK_MONOTONIC);
        pthread_cond_init(&connector_cond, &connector_cond_attr);
    }
    virtual ~ss_connector_base()
    {
        disconnect_in();
        pthread_cond_broadcast(&connector_cond);
        pthread_cond_destroy(&connector_cond);
        pthread_condattr_destroy(&connector_cond_attr);
    }
    bool connection_check()
    {
        ss_auto_rdlock lock(connector_access_lock);
        return (connector_info.state == EN_CONNECTION_BUILD) ?
        true : false;
    }
    void access()
    {
        ss_auto_rwlock lock(connector_access_lock);
        connector_info.state |= EN_CONNECTION_OUT;
        if (connector_info.state == EN_CONNECTION_BUILD)
        {
            if (connector_info.in_access)
            {
                connector_info.in_access(connector_info.user, connector_info.refer_count);
                wake_up_cond();
            }
            connector_info.refer_count++;
        }
    }
    void leave(bool b_force = false)
    {
        ss_auto_rwlock lock(connector_access_lock);
        if (b_force)
        {
            connector_info.refer_count = 0;
        }
        else if (connector_info.refer_count)
        {
            connector_info.refer_count--;
        }
        if (connector_info.state == EN_CONNECTION_BUILD)
        {
            if (connector_info.in_leave)
            {
                connector_info.in_leave(connector_info.user, connector_info.refer_count);
            }
        }
        if (connector_info.refer_count == 0)
        {
            connector_info.state &= (~(EN_CONNECTION_OUT & 0xFFFFFFFF));
            wake_up_cond();
        }
    }
    void connect_in(connection_cb connect, connection_cb disconnect, void *user)
    {
        ss_auto_rwlock lock(connector_access_lock);
        connector_info.user        = user;
        connector_info.in_access   = connect;
        connector_info.in_leave    = disconnect;
        connector_info.state |=  EN_CONNECTION_IN;
        if (connector_info.state == EN_CONNECTION_BUILD
            && connector_info.in_access)
        {
            connector_info.in_access(connector_info.user, connector_info.refer_count);
            wake_up_cond();
        }
    }
    void disconnect_in()
    {
        ss_auto_rwlock lock(connector_access_lock);
        if (connector_info.state == EN_CONNECTION_BUILD
            && connector_info.in_leave)
        {
            connector_info.in_leave(connector_info.user, 0);
        }
        connector_info.user        = NULL;
        connector_info.in_access   = NULL;
        connector_info.in_leave    = NULL;
        connector_info.state &= (~(EN_CONNECTION_IN & 0xFFFFFFFF));
        wake_up_cond();
    }
    virtual int approch(void)
    {
        //Check writer state.
        ss_auto_rdlock lock(connector_data_lock);
        if (sends.size())
        {
            return check();
        }
        return 0;
    }
    void set_packet_from(const packet_get_cb &get_cb, const packet_put_cb &put_cb, void *user)
    {
        ss_auto_rwlock lock(connector_data_lock);
        if (!get_cb || !put_cb)
        {
            recvs.recv_get = NULL;
            recvs.recv_put = NULL;
            recvs.user     = NULL;
        }
        recvs.recv_get = get_cb;
        recvs.recv_put = put_cb;
        recvs.user = user;
    }
    void set_packet_to(const std::string &ref, const packet_to_cb &send,
                       const approch_cb &approch, void *user)
    {
        ss_auto_rwlock lock(connector_data_lock);
        if (!send)
        {
            auto iter = sends.find(ref);
            if (iter != sends.end())
            {
                sends.erase(iter);
            }
            return;
        }
        sends[ref].send    = send;
        sends[ref].approch = approch;
        sends[ref].user    = user;
    }
    void set_packet_info(const stream_packet_info &p_info)
    {
        ss_auto_rwlock lock(connector_access_lock);
        connector_info.packet_info = p_info;
    }
    const stream_packet_info &get_packet_info()
    {
        ss_auto_rdlock lock(connector_access_lock);
        return connector_info.packet_info;
    }

    //Input virtual function.
    virtual int come(stream_packet_obj &packet) = 0;
    virtual stream_packet_obj getbuf(const stream_packet_info &packet_info) = 0;
    virtual void update(stream_packet_obj &packet) = 0;
    virtual void drop(stream_packet_obj &packet) = 0;

    //Output virtual function.
    virtual stream_packet_obj grab(int delay_ms = 100) = 0;
    virtual void back(stream_packet_obj &packet) = 0;

protected:
    bool go_accross(stream_packet_obj &packet, int &ret)
    {
        ss_auto_rdlock lock(connector_data_lock);
        if (sends.size())
        {
            ret = go(packet);
            return true;
        }
        return false;
    }
    bool check_accross(int &ret)
    {
        ss_auto_rdlock lock(connector_data_lock);
        if (sends.size())
        {
            ret = check();
            return true;
        }
        return false;
    }
    stream_packet_obj take_previous(bool &b_sync, int delay_ms)
    {
        ss_auto_rdlock lock(connector_data_lock);
        if (recvs.recv_get && recvs.recv_put)
        {
            b_sync = true;
            return take(delay_ms);
        }
        b_sync = false;
        return nullptr;
    }
    bool put_previous(stream_packet_obj &packet)
    {
        ss_auto_rdlock lock(connector_data_lock);
        if (recvs.recv_get && recvs.recv_put)
        {
            put(packet);
            return true;
        }
        return false;
    }
    bool can_send(void) const
    {
        return sends.size() ? true : false;
    }
    bool can_recv(void) const
    {
        return (recvs.recv_get && recvs.recv_put) ? true : false;
    }
    int check()
    {
        int ret = 0;
        for (auto iter = sends.begin(); iter != sends.end(); iter++)
        {
            if (!iter->second.approch)
            {
                continue;
            }
            ret |= iter->second.approch(iter->second.user);
        }
        return ret;
    }
    stream_packet_obj take(int delay_ms)
    {
        if (recvs.recv_get)
        {
            return recvs.recv_get(recvs.user, delay_ms);
        }
        return nullptr;
    }
    void put(stream_packet_obj &packet)
    {
        if (recvs.recv_put)
        {
            recvs.recv_put(packet, recvs.user);
        }
    }
    int go(stream_packet_obj &packet)
    {
        int ret = 0;
        for (auto iter = sends.begin(); iter != sends.end(); iter++)
        {
            if (!iter->second.send)
            {
                std::cout << "SEND CB IS NULL" << std::endl;
                return -1;
            }
            ret |= iter->second.send(packet, iter->second.user);
        }
        return ret;
    }
    int wake_up_cond(void)
    {
        return pthread_cond_signal(&connector_cond);
    }
    int wait_cond(int time_out_ms)
    {
        int ret = 0;
        struct timespec future;
        clock_gettime(CLOCK_MONOTONIC, &future);
        if (time_out_ms < 0)
        {
            // If time_out_ms < 0, wait no timeout
            ret = pthread_cond_wait(&connector_cond, &connector_mutex);
            return ret;
        }
        if((time_out_ms) >= 1000)
        {
            future.tv_sec += (time_out_ms) / 1000;
        }
        future.tv_nsec += (((time_out_ms) % 1000)*1000000);
        if(future.tv_nsec >= 1000000000)
        {
            future.tv_sec++;
            future.tv_nsec = future.tv_nsec - 1000000000;
        }
        ret = pthread_cond_timedwait(&connector_cond, &connector_mutex, &future);
        return ret;
    }
    pthread_mutex_t        connector_mutex;
    struct connection_info connector_info;

private:
    pthread_condattr_t                 connector_cond_attr;
    pthread_cond_t                     connector_cond;
    pthread_rwlock_t                   connector_access_lock;
    pthread_rwlock_t                   connector_data_lock;
    std::map<std::string, struct packet_to> sends;
    struct packet_from                 recvs;
};

class ss_connector : public ss_connector_base
{
public:
    explicit ss_connector() : packet_depth(8), bDropMsg(true)
    {
    }
    virtual ~ss_connector()
    {
    }
    void set_packet_depth(unsigned int depth)
    {
        packet_depth = depth;
    }
    void set_packet_drop_warning(bool bMsg)
    {
        bDropMsg = bMsg;
    }
    static ss_connector_base *create(const std::string &handle)
    {
        ss_connector *connector = NULL;
        connector = new ss_connector();
        assert(connector);
        if (!ss_handle::install(handle, connector))
        {
            delete connector;
            return NULL;
        }
        return connector;
    }
    virtual int come(stream_packet_obj &packet) override
    {
        int ret = 0;
        bool b_sync = go_accross(packet, ret);
        if (b_sync)
        {
            return ret;
        }
        auto new_pack = stream_packet_base::dup(packet);
        assert(new_pack);

        ss_auto_lock lock(connector_mutex);
        if (packet_list.size() > packet_depth)
        {
            if(bDropMsg)
            {
                std::cout << "[WARNING]: Packet list element's count exceed the max vaule of packet depth(" << packet_depth << ")." << std::endl;
            }
            return -1;
        }
        packet_list.push_back(new_pack);
        wake_up_cond();
        return 0;
    }
    virtual stream_packet_obj getbuf(const stream_packet_info &packet_info) override
    {
        auto new_pack = stream_packet_base::make<stream_packet>(packet_info);
        assert(new_pack);
        return new_pack;
    }
    virtual void update(stream_packet_obj &packet) override
    {
        int ret = 0;
        if (go_accross(packet, ret))
        {
            return;
        }
        ss_auto_lock lock(connector_mutex);
        if (packet_list.size() > packet_depth)
        {
            if(bDropMsg)
            {
                std::cout << "[WARNING]: Packet list element's count exceed the max vaule of packet depth(" << packet_depth << std::endl;
            }
            return;
        }
        packet_list.push_back(packet);
        wake_up_cond();
    }
    virtual void drop(stream_packet_obj &packet) override
    {
    }
    virtual stream_packet_obj grab(int delay_ms = 100) override
    {
        bool b_sync = false;
        auto packet = take_previous(b_sync, delay_ms);
        if (b_sync)
        {
            return packet;
        }
        ss_auto_lock lock(connector_mutex);
        int wait_ret = 0;
        do
        {
            if (!packet_list.empty())
            {
                auto packet_front = packet_list.front();
                packet_list.pop_front();
                return packet_front;
            }
            wait_ret = wait_cond(delay_ms);
        } while (wait_ret != ETIMEDOUT && this->connector_info.state == EN_CONNECTION_BUILD);
        return nullptr;
    }
    virtual void back(stream_packet_obj &packet) override
    {
        if (put_previous(packet))
        {
            return;
        }
    }
private:
    std::list<stream_packet_obj> packet_list;
    unsigned packet_depth;
    bool bDropMsg;
};
#endif

