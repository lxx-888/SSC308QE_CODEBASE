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

#ifndef __SS_DMABUF_ALLOCATOR__
#define __SS_DMABUF_ALLOCATOR__

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <iostream>
#include "ss_connector.h"

typedef struct{
    uint64_t len;
    uint32_t fd;
    uint32_t fd_flags;
    uint64_t heap_flags;
}dma_heap_allocation_data;

#define DMA_HEAP_IOC_MAGIC    'H'
#define DMA_HEAP_IOCTL_ALLOC    _IOWR(DMA_HEAP_IOC_MAGIC,0,dma_heap_allocation_data)

class ss_dmabuf_allocator
{
public:
    ss_dmabuf_allocator(){}
    virtual ~ss_dmabuf_allocator() {}
    int alloc_dmabuf(uint32_t size)
    {
        int dev_fd = 0, Res = -1;
        dma_heap_allocation_data data = {
            .len = 0,
            .fd = 0,
            .fd_flags = 2,
            .heap_flags = 0,
        };
        dev_fd = open("/dev/mma", O_RDWR);
        if (dev_fd <= 0)
        {
            return 0;
        }
        data.fd_flags =(O_CLOEXEC | O_ACCMODE);
        data.len = size;
        Res = ioctl(dev_fd, DMA_HEAP_IOCTL_ALLOC, &data);
        close(dev_fd);
        if(Res != EXIT_SUCCESS || data.fd <= 0)
        {
            std::cout << "ss_dmabuf_allocator: alloc dma-buf fail Res=" << std::hex <<  Res << std::endl;
            return 0;
        }
        return data.fd;
    }

    void free_dmabuf(int fd)
    {
        close(fd);
    }
};


#endif



