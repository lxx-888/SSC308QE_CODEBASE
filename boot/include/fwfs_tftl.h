/*
 * fwfs_tftl.h - Sigmastar
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

//TFTL:Tiny flash translation layer
#ifndef FWFS_TFTL_H
#define FWFS_TFTL_H

#include <stdint.h>
#include <stdbool.h>

#include "fwfs.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// Version info ///

// Version of TFTL On-disk data structures
// Major (top-nibble), incremented on backwards incompatible changes
// Minor (bottom-nibble), incremented on feature additions
#define FWFS_TFTL_DISK_VERSION 0x00040000
#define FWFS_TFTL_DISK_VERSION_MAJOR (0xffff & (FWFS_TFTL_DISK_VERSION >> 16))
#define FWFS_TFTL_DISK_VERSION_MINOR (0xffff & (FWFS_TFTL_DISK_VERSION >>  0))

#define FWFS_TFTL_GC_FILE       ".fwfs_gcstate"

#define TFTL_MAGIC_METADATA     (0xa)
#define TFTL_MAGIC_CTZ          (0xb)
#define TFTL_MAGIC_NULL         (0xc)
#define TFTL_MAGIC_INLINE       (0xd)
#define TFTL_MAGIC_GC           (0xe)

    /**
     * tftl header on start of every subblock
     *
     * Change its size need to increase FWFS_TFTL_DISK_VERSION_MAJOR
     * Backwards compatible changes need to increase FWFS_TFTL_DISK_VERSION_MINOR
     */
    typedef struct fwfs_tftl_header {
        uint32_t tag;
        uint32_t reserved;
    } fwfs_tftl_header_t;

#define FWFS_TFTL_HEADER_SIZE  ((fwfs_size_t)sizeof(fwfs_tftl_header_t))

    typedef struct fwfs_tftl_gcstate {
        fwfs_block_t gc_target;
        fwfs_block_t gc_block;
        uint32_t map[2];
        uint32_t crc;
    } fwfs_tftl_gcstate_t;

    int fwfs_tftl_init(fwfs_t *fwfs);

    int fwfs_tftl_deinit(fwfs_t *fwfs);

    fwfs_size_t fwfs_tftl_header_size(fwfs_t *fwfs, fwfs_block_t block);

    int fwfs_tftl_header_fill(fwfs_t *fwfs, fwfs_tftl_header_t *header,
                              fwfs_block_t block, fwfs_off_t index,
                              uint32_t fid);

    void fwfs_tftl_gc_init(fwfs_t *fwfs);

    static inline bool fwfs_tftl_gcstate_hasgc(fwfs_tftl_gcstate_t *gcstate) {
        return (gcstate->gc_target & 0xf0000000) >> 28 == TFTL_MAGIC_GC;
    }

    static inline fwfs_block_t fwfs_tftl_block_id_raw(fwfs_block_t block) {
        return block & 0x003fffff;
    }

    /**
     * fwfs:
     * fwfs_t pointer
     *
     * block:
     * Logical erasable block (LEB)
     *
     *
     * return the PEB id encoded in LEB
     */
    static inline fwfs_block_t fwfs_tftl_block_id(const fwfs_t *fwfs, fwfs_block_t block) {
        fwfs_block_t id = fwfs_tftl_block_id_raw(block);
        fwfs_tftl_gcstate_t *gcstate = (fwfs_tftl_gcstate_t *)fwfs->tftl.priv;

        if (!fwfs_tftl_gcstate_hasgc(gcstate) ||
            id != fwfs_tftl_block_id_raw(gcstate->gc_target)) {
            return id;
        } else {
            return fwfs_tftl_block_id_raw(gcstate->gc_block);
        }
    }

    /**
     * fwfs:
     * fwfs_t pointer
     *
     * block:
     * Logical erasable block (LEB)
     *
     *
     * return block size of LEB
     */
    fwfs_size_t fwfs_tftl_block_size(fwfs_t *fwfs, fwfs_block_t block);

    /**
     * block:
     * when ctz is true, it is a in&out paramter,
     * it pass in the pointer of allocated sub-block(LEB),
     * and pass out PEB+LEB through it after return.
     *
     * when ctz is false, sub_block is a out paramter,
     * pass out allocated PEB through it after return.
     *
     * ctz:
     * the user is metdata pairs or CTZ skip-lists.
     *
     *
     * return 0 or err.
     */
    int fwfs_tftl_alloc(fwfs_t *fwfs, fwfs_block_t *block);

    int fwfs_tftl_dir_alloc(fwfs_t *fwfs, fwfs_block_t *blocks, uint32_t num);

    /**
     * seed:
     * filesystem seed
     *
     */
    void fwfs_tftl_seed(uint32_t seed);

    int fwfs_tftl_fs_size(fwfs_t *fwfs, fwfs_size_t *size);

    int fwfs_tftl_debug(fwfs_t *fwfs);

    // Read a region in a LEB block. Negative error codes are propogated
    // to the user.
    int fwfs_tftl_read (const fwfs_t *fwfs, fwfs_block_t block,
                       fwfs_off_t off, void *buffer, fwfs_size_t size);

    // Program a region in a LEB block. The block must have previously
    // been erased. Negative error codes are propogated to the user.
    // May return FWFS_ERR_CORRUPT if the block should be considered bad.
    int fwfs_tftl_prog(const fwfs_t *fwfs, fwfs_block_t block,
                      fwfs_off_t off, const void *buffer, fwfs_size_t size);

    // Erase a LEB block. a LEB block must be erased before being programmed.
    // The state of an erased block is undefined. Negative error codes
    // are propogated to the user.
    // May return FWFS_ERR_CORRUPT if the block should be considered bad.
    int fwfs_tftl_erase(const fwfs_t *fwfs, fwfs_block_t block);

    // Sync the state of the underlying block device. Negative error codes
    // are propogated to the user.
    int fwfs_tftl_sync(const fwfs_t *fwfs);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
