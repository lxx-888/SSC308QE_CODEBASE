/*
 * firmwarefs.h - Sigmastar
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

#ifndef _FIRMWAREFS_H_
#define _FIRMWAREFS_H_

#include "fwfs.h"

#ifndef O_CREAT
#define O_CREAT     00000100
#endif
#ifndef O_PATH
#define O_PATH      010000000
#endif
#ifndef O_RDWR
#define O_RDWR      00000002
#endif
#ifndef O_WRONLY
#define O_WRONLY    00000001
#endif
#ifndef O_RDONLY
#define O_RDONLY    00000000
#endif
#ifndef O_ACCMODE
#define O_ACCMODE   00000003
#endif

#ifndef O_APPEND
#define O_APPEND    00002000
#endif

#ifndef O_TRUNC
#define O_TRUNC     00001000
#endif

#ifndef SEEK_SET
#define SEEK_SET    0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR    1
#endif
#ifndef SEEK_END
#define SEEK_END    2
#endif


int32_t firmwarefs_mount(char *partition, char *mnt_path);
void firmwarefs_unmount(void);
void* firmwarefs_open(char *filename, uint32_t  flags, uint32_t mode);
int32_t firmwarefs_close(void* fd);
int32_t firmwarefs_read(void* fd, void *buf, uint32_t count);
int32_t firmwarefs_write(void* fd, void *buf, uint32_t count);
int32_t firmwarefs_lseek(void* fd, int32_t offset, int32_t whence);

#endif /*_FIRMWAREFS_H_*/
