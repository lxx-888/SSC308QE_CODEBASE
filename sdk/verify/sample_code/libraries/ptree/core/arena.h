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

#ifndef __ARENA_H__
#define __ARENA_H__

/*
 * arena
 * ---
 *  Arena Memory Pool is a memory management technique that efficiently handles
 * long-lived objects by pre-allocating a fixed-size block of memory. No
 * individual memory deallocation is supported; the entire memory pool is
 * released when no longer needed.
 */

/*
 * ARENA_Create - Create arena memory pool
 * ---
 *  @handle   - Arena memory pool description
 *  @capacity - Initialized memory pool size
 * ---
 *  @return   - return 0 if success, or return -1
 */
int ARENA_Create(void **handle, unsigned int capacity);

/*
 * ARENA_MapUnused  - Map the specific buffer to arena without allocation
 * ---
 *  @handle - Arena memory pool description
 *  @size   - Buffer size
 * ---
 *  @return - return memory address if success, or return NULL
 */
int ARENA_MapUnused(void **ppHandle, void *pBuf, unsigned int u32Capacity);

/*
 * ARENA_Map  - Map the specific buffer to arena
 * ---
 *  @handle - Arena memory pool description
 *  @size   - Buffer size
 * ---
 *  @return - return memory address if success, or return NULL
 */
int ARENA_Map(void **handle, void *buf, unsigned int capacity);

/*
 * ARENA_Map  - Unmap the buffer.
 * ---
 *  @handle - Arena memory pool description
 * ---
 *  @return - return memory address if success, or return NULL
 */
int ARENA_Unmap(void *handle);

/*
 * ARENA_Get  - Get buffer from arena memory pool
 * tips: If size is overflow, capacity will increase auto.
 * ---
 *  @handle - Arena memory pool description
 *  @size   - Buffer size
 * ---
 *  @return - return memory address if success, or return NULL
 */
void *ARENA_Get(void *handle, unsigned int size);

/*
 * ARENA_Addr - Convert offset of address to address
 * ---
 *  @handle - Arena memory pool description
 *  @offset - offset of address in arena
 * ---
 *  @return - return memory address if success, or return NULL
 */
void *ARENA_Addr(void *handle, unsigned int offset);

/*
 * ARENA_Offset - Convert address to offset of address
 * ---
 *  @handle - Arena memory pool description
 *  @addr   - address of buffer
 * ---
 *  @return - return offset of address if success, or return 0
 */
unsigned int ARENA_Offset(void *handle, void *addr);

/*
 * ARENA_Size - Get used arena memory pool size
 * ---
 *  @hanele - Arena memory pool description
 * ---
 *  @return - size of used memory.
 */
unsigned int ARENA_Size(void *handle);

/*
 * ARENA_Destroy - Destroy arena memory pool
 * ---
 *  @hanele - Arena memory pool description
 */
int ARENA_Destroy(void *handle);

#endif /* __ARENA_H__ */
