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

#ifndef __SS_RING_ALLOCATOR__
#define __SS_RING_ALLOCATOR__

#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <map>
#include <iostream>
#include "ss_auto_lock.h"
#include "ss_handle.h"

typedef long ring_addr;

class ss_ring_allocator
{
public:
    ss_ring_allocator() {}
    virtual ~ss_ring_allocator() {}
    void init_pool(unsigned int size, char *virt)
    {
        pool_size = size;
        virtual_addr = virt;
        read_offset = 0;
        prepare_offset = 0;
        write_offset = 0;
        ring_buf_mutex = PTHREAD_MUTEX_INITIALIZER;
    }
    ring_addr prepare_buf(int size)
    {
        ring_addr addr = 0;

        ss_auto_lock lock(ring_buf_mutex);
        if (!virtual_addr)
        {
            return -1;
        }
        if (prepare_offset + size  >= pool_size)
        {
            if (read_offset > prepare_offset || read_offset == 0)
            {
                cout << "ALLOC BUF FAIL! CASE(0)" << " ALLOC SIZE: " << size << " R: " << read_offset << " W: " << write_offset << " P: " << prepare_offset << endl;
                prepare_offset = read_offset = write_offset;
                return -1;
            }
            prepare_offset = 0;
        }
        addr = prepare_offset;
        if (read_offset > prepare_offset
            && prepare_offset + size > read_offset)
        {
            cout << "ALLOC BUF FAIL! CASE(1)" << " ALLOC SIZE: " << size << " R: " << read_offset << " W: " << write_offset << " P: " << prepare_offset << endl;
            prepare_offset = read_offset = write_offset;
            return -1;
        }
        prepare_offset += size;
        return addr;
    }
    ring_addr prepare_buf_to_end(const ring_addr &end)
    {
        ss_auto_lock lock(ring_buf_mutex);

        if (!virtual_addr)
        {
            return -1;
        }
        if (write_offset == read_offset)
        {
            if (end >= pool_size)
            {
                std::cout << "Prepare to end fail! Pool size exceed! " << " END: " << end << std::endl;
                return -1;
            }
        }
        else
    {
            if (!in_range(write_offset, end, write_offset, read_offset))
            {
                cout << "Prepare to end fail!" << " START: " << write_offset << " END: " << end << " RANGE_START: " << write_offset << " RANGE_END: " << read_offset << endl;
                return -1;
            }
        }
        prepare_offset = end;
        return write_offset;
    }
    int get_size(const ring_addr &start, const ring_addr &end)
    {
        unsigned int size[2];

        if (end >= start)
        {
            size[0] = end - start;
            size[1] = 0;
            return size[0];
        }
        size[0] = pool_size - start;
        size[1] = end;
        return size[0] + size[1];
    }
    void get_range_size(const ring_addr &start, const ring_addr &end, unsigned int(& size)[2])
    {
        if (end >= start)
        {
            size[0] = end - start;
            size[1] = 0;
        }
        size[0] = pool_size - start;
        size[1] = end;
    }
    // To get read/write offset;
    int use_all_ring(ring_addr &start, ring_addr &end)
    {
        ss_auto_lock lock(ring_buf_mutex);

        start = read_offset;
        end = write_offset;
        return write_offset;
    }
    bool in_range(const ring_addr &start, const ring_addr &end,
                  const ring_addr &range_start, const ring_addr &range_end)
    {
        if (range_start == range_end)
        {
            return false;
        }
        if (range_end > range_start)
        {
            if (start >= range_start && end > start && end <= range_end)
            {
                return true;
            }
            return false;
        }
        if (start >= range_start && end <= range_end)
        {
            return true;
        }
        if (end <= range_end && end > start)
        {
            return true;
        }
        if (start >= range_start && end > start)
        {
            return true;
        }
        return false;
    }
    ring_addr active_buf(void)
    {
        ss_auto_lock lock(ring_buf_mutex);

        write_offset = prepare_offset;
        return write_offset;
    }
    ring_addr deactive_buf(void)
    {
        ss_auto_lock lock(ring_buf_mutex);

        prepare_offset = write_offset;
        return write_offset;
    }
    void flip_buf(ring_addr flip)
    {
        ss_auto_lock lock(ring_buf_mutex);

        read_offset = flip;
    }
    void flip_all(void)
    {
        ss_auto_lock lock(ring_buf_mutex);
        prepare_offset = read_offset = write_offset;
    }
    void flip_by_set(ring_addr start, ring_addr end)
    {
        ss_auto_lock lock(ring_buf_mutex);

        read_offset = start;
        prepare_offset = write_offset = end;
    }
    int copy_to_ring(ring_addr dst_addr, char *src_addr, unsigned int size)
    {
        char *dst_virt_addr = NULL;

        ss_auto_lock lock(ring_buf_mutex);
        if (write_check(size, dst_addr) == -1)
            return -1;
        dst_virt_addr = dst_addr + virtual_addr;
        memcpy(dst_virt_addr, src_addr, size);
        return 0;
    }
    int copy_from_ring(char *dst_addr, ring_addr src_addr, unsigned int size)
    {
        char *src_virt_addr = NULL;
        ss_auto_lock lock(ring_buf_mutex);
        if (read_offset + (ring_addr)size >= pool_size)
            src_addr = 0;
        if (read_check(size, src_addr) == -1)
            return -1;
        src_virt_addr = (char *)(src_addr + virtual_addr);
        memcpy(dst_addr, src_virt_addr, size);
        return 0;
    }
    int copy_from_ring(char *dst_addr, unsigned int size)
    {
        ring_addr rd_addr = 0;
        char *src_virt_addr = NULL;
        ss_auto_lock lock(ring_buf_mutex);
        /*Grab function need check ring buffer size in normal flow.*/
        if (!get_size(read_offset, write_offset))
            return -1;
        rd_addr = read_offset;
        if (read_offset + (ring_addr)size >= pool_size)
            rd_addr = 0;
        if (read_check(size, rd_addr) == -1)
            return -1;
        src_virt_addr = (char *)(rd_addr + virtual_addr);
        memcpy(dst_addr, src_virt_addr, size);
        return 0;
    }
    void from_ring_buf(char *&addr, ring_addr ring_addr)
    {
        ss_auto_lock lock(ring_buf_mutex);

        addr = virtual_addr + ring_addr;
    }
private:
    int read_check(unsigned int size, ring_addr rd_start)
    {
        if (rd_start + (ring_addr)size  >= pool_size)
        {
            if (write_offset > rd_start || write_offset == 0)
            {
                std::cout << "Read check. CASE[0]" << " R: " << rd_start << " W: " << write_offset << std::endl;
                return -1;
            }
        }
        if (write_offset >= rd_start
            && rd_start + (ring_addr)size > write_offset)
        {
            std::cout << "Read check. CASE[1]" << " R: " << rd_start << " W: " << write_offset << std::endl;
            return -1;
        }
        return 0;
    }
    int write_check(unsigned int size, ring_addr wr_start)
    {
        if (wr_start + (ring_addr)size  >= pool_size)
        {
            if (read_offset > wr_start || read_offset == 0)
            {
                std::cout << "Write check. CASE[0]" << " R: " << read_offset << "W: " << wr_start << std::endl;
                return -1;
            }
        }
        if (read_offset > wr_start
            && wr_start + (ring_addr)size > read_offset)
        {
            std::cout << "Write check. CASE[1]" << " R: " << read_offset << "W: " << wr_start << std::endl;
            return -1;
        }
        return 0;
    }

    long pool_size;
    char * virtual_addr;
    ring_addr read_offset;
    ring_addr prepare_offset;
    ring_addr write_offset;
    pthread_mutex_t ring_buf_mutex;
};
#endif
