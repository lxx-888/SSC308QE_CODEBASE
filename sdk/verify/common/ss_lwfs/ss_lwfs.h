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

#ifndef __LWFS__
#define __LWFS__

typedef long SS_LWFS_FD;

typedef enum
{
    EN_LWFS_FILE_READ_WRITE_TRUNC,
    EN_LWFS_FILE_READ_ONLY,
    EN_LWFS_FILE_READ_WRITE
} SS_LWFS_FileOpertion_e;

#define EN_LWFS_SEEK_SET 0 /* seek relative to beginning of file */
#define EN_LWFS_SEEK_CUR 1 /* seek relative to current file position */
#define EN_LWFS_SEEK_END 2 /* seek relative to end of file */

int        SS_LWFS_List(const char *s8MtdPath);
SS_LWFS_FD SS_LWFS_Open(const char *s8MtdPath, const char *s8FileName, SS_LWFS_FileOpertion_e);
int        SS_LWFS_Read(SS_LWFS_FD lwfsFd, void *buf, int size);
int        SS_LWFS_Write(SS_LWFS_FD lwfsFd, void *buf, int size);
int        SS_LWFS_Sync(SS_LWFS_FD lwfsFd);
int        SS_LWFS_Seek(SS_LWFS_FD lwfsFd, int s32Offset, int s32Whence);
int        SS_LWFS_Close(SS_LWFS_FD lwfsFd);

#endif //LWFS
