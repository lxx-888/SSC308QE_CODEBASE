/*
 * riu.c- Sigmastar
 *
 * Copyright (c) [2019~2020] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */

#include <mdrv_msys_io.h>
#include <mdrv_msys_io_st.h>
#include <mdrv_verchk.h>
#include <riu.h>

#define MSYS_PATH "/dev/msys"
#define MEM_PATH  "/dev/mem"

bool riu_write(unsigned long phy_addr, unsigned short val, unsigned int mask, bool mask_flag)
{
    bool           ret      = true;
    int            msys_fd  = 0;
    int            mem_fd   = 0;
    unsigned short value    = 0;
    unsigned long  addr     = 0;
    unsigned long *map_base = NULL;
    MSYS_MMIO_INFO info     = {0};

    FILL_VERCHK_TYPE(info, info.VerChk_Version, info.VerChk_Size, IOCTL_MSYS_VERSION);

    msys_fd = open(MSYS_PATH, O_RDWR | O_SYNC);
    if (-1 == msys_fd)
    {
        printf("open %s fail\n", MSYS_PATH);
        return false;
    }

    mem_fd = open(MEM_PATH, O_RDWR | O_SYNC);
    if (-1 == mem_fd)
    {
        printf("open %s fail\n", MEM_PATH);
        ret = false;
        goto out_msys_close;
    }

    if (0 != ioctl(msys_fd, IOCTL_MSYS_GET_RIU_MAP, &info))
    {
        printf("ioctl %s fail\n", MSYS_PATH);
        ret = false;
        goto out_mem_close;
    }

    map_base = (unsigned long *)mmap(NULL, info.size, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, info.addr);
    if (map_base == NULL)
    {
        printf("mmap fail\n");
        ret = false;
        goto out_mem_close;
    }

    addr = (unsigned long)((unsigned long)map_base + phy_addr);
    if (mask_flag)
    {
        value                   = *(unsigned short *)addr;
        value                   = (value & (~mask)) | (val & mask);
        *(unsigned short *)addr = value;
        val                     = *(unsigned short *)addr;
    }
    else
    {
        *(unsigned short *)addr = val;
        val                     = *(unsigned short *)addr;
    }

    if (map_base != NULL)
    {
        munmap(map_base, 0xff);
    }

out_mem_close:
    close(mem_fd);
out_msys_close:
    close(msys_fd);

    return ret;
}

unsigned long riu_read(unsigned long phy_addr, unsigned int mask, bool mask_flag)
{
    int            value    = 0;
    int            msys_fd  = 0;
    int            mem_fd   = 0;
    unsigned long  addr     = 0;
    unsigned long *map_base = NULL;
    MSYS_MMIO_INFO info     = {0};

    FILL_VERCHK_TYPE(info, info.VerChk_Version, info.VerChk_Size, IOCTL_MSYS_VERSION);

    msys_fd = open(MSYS_PATH, O_RDWR | O_SYNC);
    if (-1 == msys_fd)
    {
        printf("open %s fail\n", MEM_PATH);
        return 0;
    }

    mem_fd = open(MEM_PATH, O_RDWR | O_SYNC);
    if (-1 == mem_fd)
    {
        printf("open %s fail\n", MEM_PATH);
        goto out_msys_close;
    }

    if (0 != ioctl(msys_fd, IOCTL_MSYS_GET_RIU_MAP, &info))
    {
        printf("ioctl %s fail\n", MSYS_PATH);
        goto out_mem_close;
    }

    map_base = (unsigned long *)mmap(NULL, info.size, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, info.addr);
    if (map_base == NULL)
    {
        printf("mmap fail\n");
        goto out_mem_close;
    }

    addr  = (unsigned long)((unsigned long)map_base + phy_addr);
    value = *(unsigned short *)addr;

    if (mask_flag)
        value &= mask;

    if (map_base != NULL)
    {
        munmap(map_base, info.size);
    }

out_mem_close:
    close(mem_fd);
out_msys_close:
    close(msys_fd);
    return value;
}

bool riu_w(unsigned long bank, unsigned long offset, unsigned short content, bool show_tag)
{
    bool           ret      = true;
    int            msys_fd  = 0;
    int            mem_fd   = 0;
    unsigned long  addr     = 0;
    unsigned long *map_base = NULL;
    MSYS_MMIO_INFO info     = {0};

    FILL_VERCHK_TYPE(info, info.VerChk_Version, info.VerChk_Size, IOCTL_MSYS_VERSION);

    msys_fd = open(MSYS_PATH, O_RDWR | O_SYNC);
    if (-1 == msys_fd)
    {
        printf("open %s fail\n", MSYS_PATH);
        return false;
    }

    mem_fd = open(MEM_PATH, O_RDWR | O_SYNC);
    if (-1 == mem_fd)
    {
        printf("open %s fail\n", MEM_PATH);
        ret = false;
        goto out_msys_close;
    }

    if (0 != ioctl(msys_fd, IOCTL_MSYS_GET_RIU_MAP, &info))
    {
        printf("ioctl %s fail\n", MSYS_PATH);
        ret = false;
        goto out_mem_close;
    }

    map_base = (unsigned long *)mmap(NULL, info.size, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, info.addr);
    if (map_base == NULL)
    {
        printf("mmap fail\n");
        ret = false;
        goto out_mem_close;
    }

    addr                    = (unsigned long)((unsigned long)map_base + bank * 0x200 + offset * 4);
    *(unsigned short *)addr = content;
    content                 = *(unsigned short *)addr;

    if (show_tag)
    {
        printf("0x%04X\n", (unsigned int)content);
    }

    if (map_base != NULL)
    {
        munmap(map_base, info.size);
    }

out_mem_close:
    close(mem_fd);
out_msys_close:
    close(msys_fd);
    return ret;
}

bool riu_r(unsigned long bank, unsigned long offset, unsigned short *content, bool show_tag)
{
    bool           ret      = true;
    int            msys_fd  = 0;
    int            mem_fd   = 0;
    unsigned long  addr     = 0;
    unsigned long *map_base = NULL;
    MSYS_MMIO_INFO info     = {0};

    FILL_VERCHK_TYPE(info, info.VerChk_Version, info.VerChk_Size, IOCTL_MSYS_VERSION);

    msys_fd = open(MSYS_PATH, O_RDWR | O_SYNC);
    if (-1 == msys_fd)
    {
        printf("open %s fail\n", MEM_PATH);
        return false;
    }

    mem_fd = open(MEM_PATH, O_RDWR | O_SYNC);
    if (-1 == mem_fd)
    {
        printf("open %s fail\n", MEM_PATH);
        ret = false;
        goto out_msys_close;
    }

    if (0 != ioctl(msys_fd, IOCTL_MSYS_GET_RIU_MAP, &info))
    {
        printf("ioctl %s fail\n", MSYS_PATH);
        ret = false;
        goto out_mem_close;
    }

    map_base = (unsigned long *)mmap(NULL, info.size, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, info.addr);
    if (map_base == NULL)
    {
        printf("mmap fail\n");
        ret = false;
        goto out_mem_close;
    }

    addr     = (unsigned long)((unsigned long)map_base + bank * 0x200 + offset * 4);
    *content = *(unsigned short *)addr;

    if (show_tag)
    {
        printf("0x%04X\n", (unsigned int)*content);
    }

    if (map_base != NULL)
    {
        munmap(map_base, info.size);
    }

out_mem_close:
    close(mem_fd);
out_msys_close:
    close(msys_fd);
    return ret;
}
