/*
 * fwfs_tftl.c - Sigmastar
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

#include <linux/string.h>
#include "fwfs_util.h"
#include "fwfs_tftl.h"

#define TFTL_STATE_BITS_NUM     (0x4)
#define TFTL_STATE_BITS_MASK    (0xf)

#define TFTL_STATE_FREE         (0x0)
#define TFTL_STATE_CTZ          (0x1)
#define TFTL_STATE_METADATA     (0x2)
#define TFTL_STATE_ABANDON      (0x3)
#define TFTL_STATE_UNKNOWN      (0x4)
#define TFTL_STATE_GC           (0x5)
#define TFTL_STATE_RESERVE      (0x6)
#define TFTL_STATE_INVALID      (0xf)

#define TFTL_MKLEB(MAGIC, SUBBLOCK_ID, BLOCK_ID)                \
    (((MAGIC & 0xf) << 28) | ((SUBBLOCK_ID & 0x3f) << 22) |     \
     (BLOCK_ID & 0x3fffff))

static void fwfs_tftl_gc_dump_gcstate(const fwfs_tftl_gcstate_t *gcstate);
static int fwfs_tftl_gc_write_gcstate(fwfs_t *fwfs, fwfs_tftl_gcstate_t *gcstate);
static int fwfs_tftl_gc_read_gcstate(fwfs_t *fwfs, fwfs_tftl_gcstate_t *gcstate);

static int fwfs_tftl_gc_alloc_subblock(fwfs_t *fwfs, fwfs_block_t *block);
static int fwfs_tftl_gc_visitor_commit(fwfs_t *fwfs);

static int fwfs_tftl_gc_select_dirty_block(fwfs_t *fwfs, fwfs_block_t *block);
static int fwfs_tftl_gc_begin(fwfs_t *fwfs, fwfs_block_t block,
                              fwfs_block_t gc_block);
static int fwfs_tftl_gc_end(fwfs_t *fwfs);

static bool fwfs_tftl_header_check(fwfs_t *fwfs, fwfs_block_t block,
                                   fwfs_off_t index, uint32_t fid);
static void fwfs_tftl_visitor_prepare(fwfs_t *fwfs);
static void fwfs_tftl_visitor_commit(fwfs_t *fwfs);
static int fwfs_tftl_visitor(void *p, fwfs_block_t block,
                             fwfs_off_t index, uint32_t fid);
static int fwfs_tftl_dump_states(fwfs_t *fwfs);

static inline void fwfs_tftl_gc_gcstate_clear(fwfs_tftl_gcstate_t *a) {
    memset(a, 0, sizeof(fwfs_tftl_gcstate_t));
}

static inline int fwfs_tftl_return_nospc(fwfs_t *fwfs) {
    FWFS_ERROR("No more free space %"PRIu32,
               fwfs->free.i + fwfs->free.off);
    if (fwfs->cfg->dump_on_full || fwfs->cfg->debug) {
        fwfs_tftl_dump_states(fwfs);
    }
    return FWFS_ERR_NOSPC;
}

static inline fwfs_block_t fwfs_tftl_subblock_count(fwfs_t *fwfs) {
    return fwfs->cfg->block_size / fwfs->cfg->subblock_size;
}

static inline fwfs_block_t fwfs_tftl_subblock_id_raw(fwfs_block_t block) {
    return (block & 0x0fc00000) >> 22;
}

static inline fwfs_block_t fwfs_tftl_subblock_id(const fwfs_t *fwfs, fwfs_block_t block) {
    fwfs_tftl_gcstate_t *gcstate = (fwfs_tftl_gcstate_t *)fwfs->tftl.priv;
    uint8_t id = fwfs_tftl_subblock_id_raw(block);

    if (!fwfs_tftl_gcstate_hasgc(gcstate) ||
        fwfs_tftl_block_id_raw(block) !=
        fwfs_tftl_block_id_raw(gcstate->gc_target)) {
        return id;
    } else {
        if (id < 8) {
            return (gcstate->map[0] >> (id * 4)) & 0xf;
        } else {
            id -= 8;
            return (gcstate->map[1] >> (id * 4)) & 0xf;
        }
    }
}

static inline uint8_t fwfs_tftl_magic(fwfs_block_t block) {
    return (block & 0xf0000000) >> 28;
}

static inline bool fwfs_tftl_leb_check(fwfs_block_t block, bool ext) {
    uint8_t magic = fwfs_tftl_magic(block);

    switch (magic) {
    case TFTL_MAGIC_METADATA:
    case TFTL_MAGIC_CTZ:
        return true;
    case TFTL_MAGIC_NULL:
    case TFTL_MAGIC_INLINE:
        return ext;
    default:
        return false;
    }
}

static uint8_t fwfs_tftl_get_state(fwfs_t *fwfs, uint8_t *states,
                                   fwfs_block_t block_id,
                                   fwfs_block_t subblock_id)
{
    FWFS_TRACE("fwfs_tftl_get_state(%p, %p, 0x%"PRIx32", 0x%"PRIx32")",
               (void*)fwfs, (void*)states, block_id, subblock_id);
    FWFS_ASSERT(fwfs->cfg->subblock_size != 0);
    uint32_t i, j;
    uint32_t mask;

    // subblocks per block
    i = fwfs_tftl_subblock_count(fwfs);
    // subblocks number need to skip
    i = i * block_id;
    // bits need to skip
    i = i * TFTL_STATE_BITS_NUM;
    // extra bits implied by subblock_id need to skip
    i += subblock_id * TFTL_STATE_BITS_NUM;

    j = i % 8;
    i = i / 8;
    mask = TFTL_STATE_BITS_MASK << j;

    FWFS_TRACE("fwfs_tftl_get_state %"PRIu32" %"PRIu32" 0x%"PRIx32" -> "
               "0x%"PRIx32, i, j, mask, (states[i] & mask) >> j);
    return (states[i] & mask) >> j;

}

static void fwfs_tftl_set_state(fwfs_t *fwfs, uint8_t *states,
                                fwfs_block_t block_id,
                                fwfs_block_t subblock_id,
                                uint8_t state)
{
    FWFS_TRACE("fwfs_tftl_set_state(%p, %p, 0x%"PRIx32", 0x%"PRIx32", "
               "0x%"PRIx32")",
               (void*)fwfs, (void*)states, block_id, subblock_id,
               (uint32_t)state);
    FWFS_ASSERT(fwfs->cfg->subblock_size != 0);
    uint32_t i, j;
    uint32_t mask, umask;
    uint8_t nstate;

    // subblocks per block
    i = fwfs_tftl_subblock_count(fwfs);
    // subblocks number need to skip
    i = i * block_id;
    // bits need to skip
    i = i * TFTL_STATE_BITS_NUM;
    // extra bits implied by subblock_id need to skip
    i += subblock_id * TFTL_STATE_BITS_NUM;

    j = i % 8;
    i = i / 8;

    mask = TFTL_STATE_BITS_MASK << j;
    umask = ~mask;

    nstate = (states[i] & umask) | ((state & TFTL_STATE_BITS_MASK) << j);
    FWFS_TRACE("fwfs_tftl_set_state %"PRIu32" %"PRIu32" 0x%"PRIx32" "
               "0x%"PRIx32" -> 0x%"PRIx32,
               i, j, mask,
               (uint32_t)((states[i] & mask) >> j),
               (uint32_t)((nstate & mask) >> j));
    states[i] = nstate;
}

static inline void fwfs_tftl_setmetadata(fwfs_t *fwfs, uint8_t *states,
                                         fwfs_block_t block_id)
{
    uint32_t i;

    for (i = 0; i < fwfs_tftl_subblock_count(fwfs); i++)
        fwfs_tftl_set_state(fwfs, states, block_id, i, TFTL_STATE_METADATA);
}

static inline void fwfs_tftl_setreserve(fwfs_t *fwfs, uint8_t *states,
                                            fwfs_block_t block_id)
{
    uint32_t i;

    for (i = 0; i < fwfs_tftl_subblock_count(fwfs); i++)
        fwfs_tftl_set_state(fwfs, states, block_id, i, TFTL_STATE_RESERVE);
}

static inline void fwfs_tftl_setctz(fwfs_t *fwfs, uint8_t *states,
                                    fwfs_block_t block_id,
                                    fwfs_block_t subblock_id)
{
    fwfs_tftl_set_state(fwfs, states, block_id, subblock_id, TFTL_STATE_CTZ);
}

static inline void fwfs_tftl_setfree(fwfs_t *fwfs, uint8_t *states,
                                     fwfs_block_t block_id)
{
    uint32_t i;

    for (i = 0; i < fwfs_tftl_subblock_count(fwfs); i++)
        fwfs_tftl_set_state(fwfs, states, block_id, i, TFTL_STATE_FREE);
}

static inline bool fwfs_tftl_subblock_inuse(fwfs_t *fwfs, uint8_t state) {
    return (state == TFTL_STATE_METADATA ||
            state == TFTL_STATE_RESERVE ||
            state == TFTL_STATE_CTZ);
}

static inline void fwfs_tftl_free_to_unknown(fwfs_t *fwfs, uint8_t *states,
                                             fwfs_block_t block_id)
{
    uint32_t i;
    uint8_t state;

    for (i = 0; i < fwfs_tftl_subblock_count(fwfs); i++) {
        state = fwfs_tftl_get_state(fwfs, fwfs->tftl.states, block_id, i);
        if (state == TFTL_STATE_FREE)
            fwfs_tftl_set_state(fwfs, states, block_id, i, TFTL_STATE_UNKNOWN);
    }
}

static inline void fwfs_tftl_setgc(fwfs_t *fwfs, uint8_t *states,
                                   fwfs_block_t block_id)
{
    uint32_t i;

    for (i = 0; i < fwfs_tftl_subblock_count(fwfs); i++)
        fwfs_tftl_set_state(fwfs, states, block_id, i, TFTL_STATE_GC);
}

static inline bool fwfs_tftl_hasctz(fwfs_t *fwfs, fwfs_block_t block_id)
{
    uint32_t i;

    for (i = 0; i < fwfs_tftl_subblock_count(fwfs); i++) {
        if (fwfs_tftl_get_state(fwfs, fwfs->tftl.states, block_id, i) ==
            TFTL_STATE_CTZ)
            return true;
    }

    return false;
}

static bool fwfs_tftl_isfree(fwfs_t *fwfs, fwfs_block_t block_id)
{
    uint32_t i;
    uint8_t state;

    for (i = 0; i < fwfs_tftl_subblock_count(fwfs); i++) {
        state = fwfs_tftl_get_state(fwfs, fwfs->tftl.states, block_id, i);
        if (state != TFTL_STATE_FREE && state != TFTL_STATE_ABANDON &&
            state != TFTL_STATE_UNKNOWN)
            return false;
    }

    return true;
}

fwfs_size_t fwfs_tftl_states_size(fwfs_t *fwfs) {
    uint32_t i;

    // Subblocks per block
    i = fwfs_tftl_subblock_count(fwfs);

    // Total subblocks
    i = i * fwfs->cfg->block_count;

    // Total bits
    i = i * TFTL_STATE_BITS_NUM;

    // Align to byte boundary
    if ((i % 8) != 0)
        i += 8 - (i % 8);

    return i = i / 8;
}

int fwfs_tftl_init(fwfs_t *fwfs) {
    fwfs->b0 = TFTL_MKLEB(TFTL_MAGIC_METADATA, 0, 0);
    fwfs->b1 = TFTL_MKLEB(TFTL_MAGIC_METADATA, 0, 1);
    fwfs->bn = TFTL_MKLEB(TFTL_MAGIC_NULL, 0, 0);
    fwfs->bi = TFTL_MKLEB(TFTL_MAGIC_INLINE, 0, 0);

    if (fwfs_tftl_subblock_count(fwfs) > 16) {
        FWFS_ERROR("The map in gcstate can't support subblock count "
                   "> 16 (0x%"PRIx32")", fwfs_tftl_subblock_count(fwfs));
        return FWFS_ERR_INVAL;
    }

    fwfs->tftl.size = fwfs_tftl_states_size(fwfs);

    fwfs->tftl.states = (uint8_t  *)fwfs_malloc(fwfs->tftl.size);
    if (!fwfs->tftl.states)
        return FWFS_ERR_NOMEM;
    memset(fwfs->tftl.states, 0, fwfs->tftl.size);


    fwfs->tftl.snapshot = (uint8_t  *)fwfs_malloc(fwfs->tftl.size);
    if (!fwfs->tftl.snapshot) {
        fwfs_free(fwfs->tftl.states);
        fwfs->tftl.states = NULL;
        return FWFS_ERR_NOMEM;
    }
    memset(fwfs->tftl.snapshot, 0, fwfs->tftl.size);

    fwfs->tftl.priv = (void *)fwfs_malloc(sizeof(fwfs_tftl_gcstate_t));
    if (!fwfs->tftl.priv) {
        fwfs_free(fwfs->tftl.states);
        fwfs->tftl.states = NULL;

        fwfs_free(fwfs->tftl.snapshot);
        fwfs->tftl.snapshot = NULL;
        return FWFS_ERR_NOMEM;
    }
    memset(fwfs->tftl.priv, 0, sizeof(fwfs_tftl_gcstate_t));

    fwfs->tftl.checked = false;
    fwfs->tftl.header_size = FWFS_TFTL_HEADER_SIZE;
    fwfs->tftl.free_blocks = fwfs->cfg->block_count;
    return 0;
}

int fwfs_tftl_deinit(fwfs_t *fwfs) {
    if (!fwfs->tftl.states) {
        fwfs_free(fwfs->tftl.states);
        fwfs->tftl.states = NULL;
    }

    if (!fwfs->tftl.snapshot) {
        fwfs_free(fwfs->tftl.snapshot);
        fwfs->tftl.snapshot = NULL;
    }

    if (!fwfs->tftl.priv) {
        fwfs_free(fwfs->tftl.priv);
        fwfs->tftl.priv = NULL;
    }

    return 0;
}

void fwfs_tftl_visitor_prepare(fwfs_t *fwfs) {
    memset(fwfs->tftl.snapshot, 0, fwfs->tftl.size);
}

int fwfs_tftl_visitor(void *p, fwfs_block_t block, fwfs_off_t index,
                      uint32_t fid) {
    FWFS_TRACE("fwfs_tftl_visitor(%p, 0x%"PRIx32")", (void*)p, block);
    fwfs_t *fwfs = (fwfs_t*)p;
    (void)index;

    if (fwfs_tftl_magic(block) == TFTL_MAGIC_METADATA) {
        fwfs_block_t block_id;

        block_id = fwfs_tftl_block_id(fwfs, block);
        fwfs_tftl_setmetadata(fwfs, fwfs->tftl.snapshot, block_id);
    } else if (fwfs_tftl_magic(block) == TFTL_MAGIC_CTZ) {
        fwfs_block_t block_id;
        fwfs_block_t subblock_id;

        if (fwfs->cfg->debug) {
            fwfs_tftl_header_check(fwfs, block, index, fid);
        }

        block_id = fwfs_tftl_block_id_raw(block);
        subblock_id = fwfs_tftl_subblock_id_raw(block);
        fwfs_tftl_setctz(fwfs, fwfs->tftl.snapshot, block_id, subblock_id);
    } else {
        FWFS_ERROR("Invalid LEB 0x%"PRIx32" (0x%x)", block, fwfs_tftl_magic(block));
        FWFS_ASSERT(false);
    }

    return 0;
}

static inline uint8_t fwfs_tftl_state_transfer(uint8_t state, uint8_t snap) {
    switch (state) {
    case TFTL_STATE_FREE:
        switch (snap) {
        case TFTL_STATE_FREE:
            return TFTL_STATE_FREE;
        case TFTL_STATE_CTZ:
            return TFTL_STATE_CTZ;
        case TFTL_STATE_METADATA:
            return TFTL_STATE_METADATA;
        default:
            return TFTL_STATE_INVALID;
        }
        break;
    case TFTL_STATE_CTZ:
        switch (snap) {
        case TFTL_STATE_FREE:
            return TFTL_STATE_ABANDON;
        case TFTL_STATE_CTZ:
            return TFTL_STATE_CTZ;
        default:
            return TFTL_STATE_INVALID;
        }
        break;
    case TFTL_STATE_METADATA:
        switch (snap) {
        case TFTL_STATE_FREE:
            return TFTL_STATE_FREE;
        case TFTL_STATE_METADATA:
            return TFTL_STATE_METADATA;
        default:
            return TFTL_STATE_INVALID;
        }
        break;
    case TFTL_STATE_ABANDON:
        switch (snap) {
        case TFTL_STATE_FREE:
            return TFTL_STATE_ABANDON;
        default:
            return TFTL_STATE_INVALID;
        }
        break;
    case TFTL_STATE_UNKNOWN:
        switch (snap) {
        case TFTL_STATE_FREE:
            return TFTL_STATE_UNKNOWN;
        default:
            return TFTL_STATE_INVALID;
        }
        break;
    case TFTL_STATE_GC:
        switch (snap) {
        case TFTL_STATE_FREE:
            return TFTL_STATE_GC;
        default:
            return TFTL_STATE_INVALID;
        }
        break;
    case TFTL_STATE_RESERVE:
        switch (snap) {
        case TFTL_STATE_FREE:
            return TFTL_STATE_RESERVE;
        default:
            return TFTL_STATE_INVALID;
        }
        break;
    default:
        break;
    }
    return TFTL_STATE_INVALID;
}

static bool fwfs_tftl_partial_write_check(fwfs_t *fwfs, fwfs_block_t block) {
    FWFS_TRACE("fwfs_tftl_partial_write_check(%p, 0x%"PRIx32")", (void*)fwfs, block);
    fwfs_tftl_header_t header;
    int err;

    err = fwfs_bd_read_ext(fwfs, sizeof(header), block, 0, &header,
                           sizeof(header));
    if (err) {
        FWFS_TRACE("fwfs_tftl_partial_write_check err -> %d", err);
        return false;
    }

    header.tag = fwfs_frombe32(header.tag);

    if (fwfs->cfg->debug >= 3)
        FWFS_INFO("fwfs_tftl_partial_write_check %d %d -> 0x%"PRIx32"",
                  fwfs_tftl_block_id(fwfs, block), fwfs_tftl_subblock_id(fwfs, block),
                  header.tag);

    if (header.tag & 0x80000000)
        return true;
    return false;
}

static bool fwfs_tftl_header_check(fwfs_t *fwfs, fwfs_block_t block,
                                   fwfs_off_t index, uint32_t fid) {
    FWFS_TRACE("fwfs_tftl_index_check(%p, 0x%"PRIx32", %"PRIu32")", (void*)fwfs,
               block, index);
    fwfs_tftl_header_t header;
    int err;
    fwfs_off_t tmp;

    err = fwfs_bd_read_ext(fwfs, sizeof(header), block, 0, &header,
                           sizeof(header));
    if (err) {
        FWFS_TRACE("fwfs_tftl_index_check err -> %d", err);
        return false;
    }
    header.tag = fwfs_frombe32(header.tag);

    if (header.tag & 0x80000000) {
        FWFS_ERROR("Invalid header 0x%"PRIx32" in block 0x%"PRIx32
                   "(index:0x%"PRIx32",fid:0x%"PRIx32")",
                   header.tag, block, index, fid);
        FWFS_ASSERT(false);
        return false;
    }

    tmp = header.tag & 0x0000ffff;
    FWFS_TRACE("fwfs_tftl_index_check 0x%"PRIx32" -> %"PRIu32, header.tag, tmp);

    if ((header.tag & 0x7fff0000) != (fid & 0x7fff0000)) {
        FWFS_ERROR("Recorded fid %"PRIu32" doesn't match real "
                   "fid %"PRIu32" for block 0x%"PRIx32"(0x%"PRIx32")",
                   header.tag & 0x7fff0000,
                   fid & 0x7fff0000,
                   block, index);
        FWFS_ASSERT(false);
        return false;
    } else {
        FWFS_TRACE("Recorded fid 0x%"PRIx32" matches real "
                   "fid 0x%"PRIx32"(0x%"PRIx32" for block 0x%"PRIx32,
                   header.tag & 0x7fff0000,
                   fid, (fid & 0x7fff0000), block);
    }

    if (tmp != index) {
        FWFS_ERROR("Recorded index %"PRIu32" doesn't match real "
                   "index %"PRIu32" for block 0x%"PRIx32"(0x%"PRIx32")",
                   tmp, index, block, fid);
        FWFS_ASSERT(false);
        return false;
    } else {
        FWFS_TRACE("Recorded index %"PRIu32" matches real "
                   "index %"PRIu32" for block 0x%"PRIx32,
                   tmp, index, block);
    }
    return true;
}

static bool fwfs_tftl_erased_check(fwfs_t *fwfs, fwfs_block_t block) {
    FWFS_TRACE("fwfs_tftl_erased_check(%p, %"PRIu32")", (void*)fwfs, block);
    int err;
    uint32_t data;
    fwfs_size_t block_size;
    fwfs_off_t off;
    int len;

    if (fwfs_tftl_magic(block) == TFTL_MAGIC_METADATA) {
        block_size = fwfs->cfg->block_size;
    } else if (fwfs_tftl_magic(block) == TFTL_MAGIC_CTZ) {
        block_size = fwfs->cfg->subblock_size;
    } else {
        FWFS_ERROR("Invalid LEB 0x%x (0x%x)", block, fwfs_tftl_magic(block));
        FWFS_ASSERT(false);
        return false;
    }

    len = block_size / sizeof(data);
    off = 0;
    for (int i = 0; i < len; i++) {
        err = fwfs_bd_read_ext(fwfs, block_size, block, off, &data,
                               sizeof(data));
        if (err) {
            FWFS_TRACE("fwfs_tftl_erased_check err -> %d", err);
            return false;
        }

        if (data != 0xffffffff) {
            FWFS_ERROR("Bad data 0x%"PRIx32" at 0x%"PRIx32" in erased block "
                       "0x%"PRIx32, data, off, block);
            FWFS_ASSERT(false);
            return false;
        }
        off += sizeof(data);
    }
    return true;
}

static void fwfs_tftl_visitor_commit(fwfs_t *fwfs) {
    uint32_t i, j;
    uint8_t snap;
    uint8_t state;
    uint8_t nstate;
    fwfs_block_t blocks = fwfs->cfg->block_count;
    bool inuse;

    for (i = 0; i < fwfs->cfg->block_count; i++) {
        inuse = false;
        for (j = 0; j < fwfs_tftl_subblock_count(fwfs); j++) {
            snap = fwfs_tftl_get_state(fwfs, fwfs->tftl.snapshot, i, j);
            state = fwfs_tftl_get_state(fwfs, fwfs->tftl.states, i, j);

            nstate = fwfs_tftl_state_transfer(state, snap);
            if (nstate == TFTL_STATE_INVALID) {
                FWFS_ERROR("Invalid states (0x%x, 0x%x) for %"PRIu32"-%"PRIu32,
                           state, snap, i, j);
                continue;
            }

            if (nstate != state)
                fwfs_tftl_set_state(fwfs, fwfs->tftl.states, i, j, nstate);

            if ((!inuse) && fwfs_tftl_subblock_inuse(fwfs, nstate))
                inuse = true;
        }

        if (!fwfs->tftl.checked) {
            if (fwfs_tftl_hasctz(fwfs, i)) {
                fwfs_tftl_free_to_unknown(fwfs, fwfs->tftl.states, i);
            }
        }

        if (inuse && blocks > 0)
            blocks -= 1;
    }
    fwfs->tftl.free_blocks = blocks;

    fwfs_tftl_gc_visitor_commit(fwfs);

    if (!fwfs->tftl.checked) {
        fwfs->tftl.checked = true;
    }
}

static int fwfs_tftl_dump_states(fwfs_t *fwfs) {
    uint32_t i, j, index = 0;
    uint8_t state;
    char *buffer;
    uint32_t buflen = 512;

    fwfs_tftl_header_t header;
    int err;
    fwfs_block_t block;
    fwfs_off_t idx = 0;
    uint32_t fid = 0;
    uint32_t tag = 0;

    buffer = (char *)fwfs_malloc(buflen);
    if (!buffer) {
        FWFS_ERROR("Out of memory!");
        return FWFS_ERR_NOMEM;
    }
    memset(buffer, 0, buflen);

    FWFS_INFO("Firmwarefs states dump:");
    for (i = 0; i < fwfs->cfg->block_count; i++) {
        for (j = 0; j < fwfs_tftl_subblock_count(fwfs); j++) {
            state = fwfs_tftl_get_state(fwfs, fwfs->tftl.states, i, j);
            index = strlen(buffer);
            if (j == 0) {
                snprintf(buffer + index, buflen - index, "%04d:", (int)i);
                index += 5;
            }

            if (state != TFTL_STATE_METADATA) {
                block = TFTL_MKLEB(TFTL_MAGIC_CTZ, j, i);
                err = fwfs_bd_read_ext(fwfs, sizeof(header), block, 0, &header,
                                       sizeof(header));
                if (err) {
                    FWFS_ERROR("tftl header read err -> %d", err);
                }

                tag = fwfs_frombe32(header.tag);
                idx = tag & 0x0000ffff;
                fid = (tag & 0x7fff0000) >> 16;
            } else {
                tag = 0x0;
                idx = 0x0;
                fid = 0x0;
            }
            snprintf(buffer + index, buflen - index, "%d(0x%04x,0x%01x,0x%04"PRIx32
                     ",0x%04"PRIx32") ",
                     (int)j, state, (int)(tag & 0x80000000 ? 0x1 : 0x0), fid, idx);
        }
        FWFS_INFO("%s", buffer);
        memset(buffer, 0, buflen);
    }
    fwfs_free(buffer);
    return FWFS_ERR_OK;
}

static void fwfs_tftl_clear_reserve(fwfs_t *fwfs,
                                    fwfs_block_t *blocks, uint32_t num) {
    uint8_t state;
    uint32_t i, j;
    fwfs_block_t subcount;

    subcount = fwfs_tftl_subblock_count(fwfs);
    for (i = 0; i < num; i++) {
        for (j = 0; j < subcount; j++) {
            state = fwfs_tftl_get_state(fwfs, fwfs->tftl.states, blocks[i], j);

            if (state == TFTL_STATE_RESERVE)
                fwfs_tftl_set_state(fwfs, fwfs->tftl.states, blocks[i], j, TFTL_STATE_FREE);
        }
    }
    return;
}

int fwfs_tftl_alloc_block(fwfs_t *fwfs, fwfs_block_t *blocks, uint32_t num, bool ctz) {
    FWFS_TRACE("fwfs_tftl_alloc_block(%p, %p(0x%"PRIx32"))",
               (void*)fwfs, (void*)blocks, num);
    uint32_t allocated = 0;
    bool resync = false;

    while (true) {
        while (fwfs->free.i != fwfs->free.size) {
            fwfs_block_t off = fwfs->free.i;

            fwfs->free.i += 1;
            fwfs->free.ack -= 1;
            if (!(fwfs->free.buffer[off / 32] & (1U << (off % 32)))) {
                // Reserve last free block for CTZ garbage collection
                if (!ctz && fwfs->tftl.free_blocks == 1) {
                    fwfs_tftl_clear_reserve(fwfs, blocks, allocated);
                    fwfs_alloc_reset(fwfs);

                    if (!resync) {
                        // Resync the state info of filesystem
                        // to make sure there is only one free
                        // block
                        resync = true;
                        allocated = 0;
                        break;
                    }
                    return fwfs_tftl_return_nospc(fwfs);
                }

                // found a free block
                if (fwfs->tftl.free_blocks > 0) {
                    fwfs->tftl.free_blocks -= 1;

                    if (!resync && fwfs->tftl.free_blocks == 0) {
                        // Resync the state info of filesystem
                        // to make sure there is no free block
                        resync = true;
                        fwfs_tftl_clear_reserve(fwfs, blocks, allocated);
                        fwfs_alloc_reset(fwfs);
                        allocated = 0;
                        break;
                    }
                }

                blocks[allocated++] = fwfs->free.off + off;
                fwfs_tftl_setreserve(fwfs, fwfs->tftl.states, fwfs->free.off + off);

                // eagerly find next off so an alloc ack can
                // discredit old lookahead blocks
                while (fwfs->free.i != fwfs->free.size &&
                       (fwfs->free.buffer[fwfs->free.i / 32]
                        & (1U << (fwfs->free.i % 32)))) {
                    fwfs->free.i += 1;
                    fwfs->free.ack -= 1;
                }

                if (allocated >= num) {
                    fwfs_tftl_clear_reserve(fwfs, blocks, allocated);
                    return 0;
                }
            }
        }

        if (fwfs->free.ack == 0) {
            fwfs_tftl_clear_reserve(fwfs, blocks, allocated);
            fwfs_alloc_reset(fwfs);
            return fwfs_tftl_return_nospc(fwfs);
        }

        fwfs->free.off = (fwfs->free.off + fwfs->free.size)
            % fwfs->cfg->block_count;
        fwfs->free.size = fwfs_min(fwfs_min(8*fwfs->cfg->lookahead_size,
                                            fwfs->free.ack),
                                   fwfs->cfg->block_count - fwfs->free.off);
        fwfs->free.i = 0;

        fwfs_tftl_visitor_prepare(fwfs);
        int err = fwfs_fs_traverse(fwfs, fwfs_tftl_visitor, fwfs);
        if (err) {
            fwfs_tftl_clear_reserve(fwfs, blocks, allocated);
            fwfs_alloc_reset(fwfs);
            return err;
        }
        fwfs_tftl_visitor_commit(fwfs);

        // find mask of free blocks from tree
        memset(fwfs->free.buffer, 0, fwfs->cfg->lookahead_size);
        for (uint32_t i = 0; i < fwfs->free.size; i++) {
            fwfs_block_t off = i + fwfs->free.off;
            if (!fwfs_tftl_isfree(fwfs, (fwfs_block_t)off)) {
                fwfs->free.buffer[i / 32] |= 1U << (i % 32);
            }
        }
    }
}

int fwfs_tftl_dir_alloc(fwfs_t *fwfs, fwfs_block_t *blocks, uint32_t num) {
    FWFS_TRACE("fwfs_tftl_alloc(%p, %p, %d)", (void*)fwfs, (void*)blocks, num);
    fwfs_block_t nblocks[2];
    int err;

    if (num != 2 && num != 1) {
        FWFS_ERROR("Invalid num value %d\n", num);
        FWFS_ASSERT(false);
    }

    err = fwfs_tftl_alloc_block(fwfs, nblocks, num, false);
    if (err)
        return err;

    fwfs_tftl_setmetadata(fwfs, fwfs->tftl.states, nblocks[0]);
    blocks[0] = TFTL_MKLEB(TFTL_MAGIC_METADATA, 0x0, nblocks[0]);
    FWFS_TRACE("fwfs_tftl_alloc 0x%"PRIx32" -> 0x%"PRIx32"",
               nblocks[0], blocks[0]);

    if (num >= 2) {
        fwfs_tftl_setmetadata(fwfs, fwfs->tftl.states, nblocks[1]);
        blocks[1] = TFTL_MKLEB(TFTL_MAGIC_METADATA, 0x0, nblocks[1]);
        FWFS_TRACE("fwfs_tftl_alloc 0x%"PRIx32" -> 0x%"PRIx32"",
                   nblocks[1], blocks[1]);
    }

    return 0;
}

int fwfs_tftl_alloc(fwfs_t *fwfs, fwfs_block_t *block) {
    FWFS_TRACE("fwfs_tftl_alloc(%p, %p)", (void*)fwfs, (void*)block);
    uint8_t state, j, count;
    uint32_t i;
    fwfs_block_t nblock;
    int err;
    fwfs_tftl_gcstate_t *gcstate = (fwfs_tftl_gcstate_t *)fwfs->tftl.priv;

    if (!fwfs->tftl.checked) {
        fwfs_tftl_visitor_prepare(fwfs);
        err = fwfs_fs_traverse(fwfs, fwfs_tftl_visitor, fwfs);
        if (err) {
            fwfs_alloc_reset(fwfs);
            return err;
        }
        fwfs_tftl_visitor_commit(fwfs);
    }

    if (fwfs_tftl_gcstate_hasgc(gcstate)) {
        err = fwfs_tftl_gc_alloc_subblock(fwfs, block);
        if ((!err) || (err && err != FWFS_ERR_NOSPC))
            return err;
    }

    count = fwfs_tftl_subblock_count(fwfs);
 again:
    for (i = 0; i < fwfs->cfg->block_count; i++) {
        if (!fwfs_tftl_hasctz(fwfs, i))
            continue;

        for (j = 0; j < count; j++) {
            state = fwfs_tftl_get_state(fwfs, fwfs->tftl.states, i, j);

            // Do partial write check for unknown state subblock.
            if (state == TFTL_STATE_UNKNOWN) {
                fwfs_block_t leb = TFTL_MKLEB(TFTL_MAGIC_CTZ, j, i);

                if (!fwfs_tftl_partial_write_check(fwfs, leb)) {
                    state = TFTL_STATE_ABANDON;
                } else {
                    state = TFTL_STATE_FREE;
                }
                fwfs_tftl_set_state(fwfs, fwfs->tftl.states, i, j, state);
            }

            // Check whether all following subblocks are in free state or
            // unknown state.
            if (state == TFTL_STATE_FREE) {
                if (j != (count - 1)) {
                    uint8_t tmp;
                    uint8_t m = j + 1;

                    for (; m < count; m++) {
                        tmp = fwfs_tftl_get_state(fwfs, fwfs->tftl.states,
                                                  i, j);
                        if (tmp != TFTL_STATE_FREE &&
                            tmp != TFTL_STATE_UNKNOWN) {
                            FWFS_ERROR("Found in use or abandoned subblock "
                                       "%d after free subblock %d in %"PRIu32,
                                       m, j, i);
                            FWFS_ASSERT(false);
                        }
                    }
                }

                if (fwfs->cfg->debug >= 2) {
                    fwfs_tftl_erased_check(fwfs,
                                           TFTL_MKLEB(TFTL_MAGIC_CTZ, j, i));
                }

                fwfs_tftl_setctz(fwfs, fwfs->tftl.states, i, j);
                *block = TFTL_MKLEB(TFTL_MAGIC_CTZ, j, i);
                FWFS_TRACE("fwfs_tftl_alloc 0x%"PRIx32" 0x%d -> 0x%"PRIx32,
                           i, j, *block);
                return 0;
            }
        }
    }

    err = fwfs_tftl_alloc_block(fwfs, &nblock, 1, true);
    if (err)
        return err;

    if (fwfs->tftl.free_blocks == 0) {
        fwfs_block_t target = fwfs->bn;

        err = fwfs_tftl_gc_select_dirty_block(fwfs, &target);
        if (err) {
            return fwfs_tftl_return_nospc(fwfs);
        }

        if (target == fwfs->bn) {
            FWFS_ERROR("gc select return true, but target is invalid!\n");
            return fwfs_tftl_return_nospc(fwfs);
        }

        err = fwfs_tftl_gc_begin(fwfs, target, nblock);
        if (err) {
            return fwfs_tftl_return_nospc(fwfs);
        }

        if (fwfs_tftl_gcstate_hasgc(gcstate)) {
            return fwfs_tftl_gc_alloc_subblock(fwfs, block);
        } else {
            goto again;
        }
    }

    fwfs->cfg->erase(fwfs->cfg, nblock);
    fwfs_tftl_setfree(fwfs, fwfs->tftl.states, nblock);

    fwfs_tftl_setctz(fwfs, fwfs->tftl.states, nblock, 0);
    *block = TFTL_MKLEB(TFTL_MAGIC_CTZ, 0, nblock);
    FWFS_TRACE("fwfs_tftl_alloc 0x%"PRIx32" 0x%"PRIx32" -> 0x%"PRIx32,
               nblock, (fwfs_block_t)0, *block);
    return 0;
}

static int fwfs_tftl_gc_select_dirty_block(fwfs_t *fwfs, fwfs_block_t *block) {
    uint32_t i, j;
    uint8_t state;
    uint32_t score, best_score = 0;
    fwfs_block_t count = fwfs_tftl_subblock_count(fwfs);

    for (i = 0; i < fwfs->cfg->block_count; i++) {
        if (!fwfs_tftl_hasctz(fwfs, i))
            continue;

        score = 0;
        for (j = 0; j < count; j++) {
            state = fwfs_tftl_get_state(fwfs, fwfs->tftl.states, i, j);
            if (state != TFTL_STATE_CTZ)
                score++;
        }

        if (score > best_score) {
            *block = i;
            best_score = score;
        }

        if (best_score == (count - 1))
            break;
    }

    return best_score != 0 ? 0 : FWFS_ERR_INVAL;
}

static int fwfs_tftl_gc_write_gcstate(fwfs_t *fwfs, fwfs_tftl_gcstate_t *gcstate) {
    fwfs_file_t file;
    char path[FWFS_NAME_MAX+1];
    int err;
    uint32_t crc = 0xffffffff;

    snprintf(path, sizeof(path),"%s", FWFS_TFTL_GC_FILE);
    if (gcstate == NULL) {
        err = fwfs_remove(fwfs, path);
        if (err)
            FWFS_ERROR("fwfs_remove return err %d\n", err);
        return err;
    }

    crc = fwfs_crc(crc, gcstate, sizeof(fwfs_tftl_gcstate_t) - 4);
    gcstate->crc = crc;

    err = fwfs_file_open(fwfs, &file, path, FWFS_O_WRONLY | FWFS_O_CREAT);
    if (err) {
        FWFS_ERROR("fwfs_file_open return err %d\n", err);
        return err;
    }

    err = fwfs_file_write(fwfs, &file, gcstate, sizeof(fwfs_tftl_gcstate_t));
    if (err != sizeof(fwfs_tftl_gcstate_t)) {
        FWFS_ERROR("fwfs_file_write return %d(%zu)\n", err,
                   sizeof(fwfs_tftl_gcstate_t));
        fwfs_file_close(fwfs, &file);
        return FWFS_ERR_IO;
    }

    err = fwfs_file_close(fwfs, &file);
    if (err) {
        FWFS_ERROR("fwfs_file_close return err %d\n", err);
    }
    return err;
}

static int fwfs_tftl_gc_read_gcstate(fwfs_t *fwfs, fwfs_tftl_gcstate_t *gcstate) {
    fwfs_file_t file;
    char path[FWFS_NAME_MAX+1];
    uint32_t crc = 0xffffffff;
    int err;

    snprintf(path, sizeof(path),"%s", FWFS_TFTL_GC_FILE);
    err = fwfs_file_open(fwfs, &file, path, FWFS_O_RDONLY);
    if (err) {
        if (err != FWFS_ERR_NOENT) {
            FWFS_ERROR("fwfs_file_open return err %d\n", err);
        }
        return err;
    }

    err = fwfs_file_read(fwfs, &file, gcstate, sizeof(fwfs_tftl_gcstate_t));
    if (err != sizeof(fwfs_tftl_gcstate_t)) {
        FWFS_ERROR("fwfs_file_read return %d(%zu)\n", err,
                   sizeof(fwfs_tftl_gcstate_t));
        fwfs_file_close(fwfs, &file);
        return FWFS_ERR_IO;
    }

    err = fwfs_file_close(fwfs, &file);
    if (err) {
        FWFS_ERROR("failed to close gc state file:%d\n", err);
        return err;
    }

    crc = fwfs_crc(crc, gcstate, sizeof(fwfs_tftl_gcstate_t) - 4);
    if (crc != gcstate->crc) {
        FWFS_ERROR("bad crc value: want 0x%x, get 0x%x\n",
                   gcstate->crc, crc);
        return FWFS_ERR_INVAL;
    }

    return 0;
}

static void fwfs_tftl_gc_dump_gcstate(const fwfs_tftl_gcstate_t *a) {
    FWFS_INFO("gc state dump begin:%p\n", (void *)a);
    FWFS_INFO("gc_target: 0x%08x\n", a->gc_target);
    FWFS_INFO("gc_block : 0x%08x\n", a->gc_block);
    FWFS_INFO("map[0]   : 0x%08x\n", a->map[0]);
    FWFS_INFO("map[1]   : 0x%08x\n", a->map[1]);
    FWFS_INFO("gc state dump end:%p\n", (void *)a);
}

static inline fwfs_block_t fwfs_tftl_gc_count_ctz_from(fwfs_t *fwfs,
                                                       fwfs_block_t block_id,
                                                       fwfs_block_t subblock_id) {
    fwfs_block_t i, j;
    uint8_t state;

    for (i = subblock_id, j = 0; i < fwfs_tftl_subblock_count(fwfs); i++) {
        state = fwfs_tftl_get_state(fwfs, fwfs->tftl.states, block_id, i);
        if (state == TFTL_STATE_CTZ)
            j++;
    }

    return j;
}

static int fwfs_tftl_gc_copy_subblock(fwfs_t *fwfs,
                                      fwfs_block_t src_block, fwfs_off_t src_offset,
                                      fwfs_block_t dst_block, fwfs_off_t dst_offset) {
    fwfs_size_t off;
    int ret;
    void *buffer;

    buffer = (void *)fwfs_malloc(fwfs->cfg->prog_size);
    if (buffer == NULL)
        return FWFS_ERR_NOMEM;

    for (off = 0; off < fwfs->cfg->subblock_size; off += fwfs->cfg->prog_size) {
        ret = fwfs->cfg->read(fwfs->cfg, src_block, off + src_offset,
                              buffer, fwfs->cfg->prog_size);
        if (ret) {
            fwfs_free(buffer);
            FWFS_ERROR("read return err %d\n", ret);
            return ret;
        }

        ret  = fwfs->cfg->prog(fwfs->cfg, dst_block, off + dst_offset,
                               buffer, fwfs->cfg->prog_size);
        if (ret) {
            fwfs_free(buffer);
            FWFS_ERROR("prog return err %d\n", ret);
            return ret;
        }
    }
    fwfs_free(buffer);

    return 0;
}

static int fwfs_tftl_gc_begin(fwfs_t *fwfs, fwfs_block_t block,
                              fwfs_block_t gc_block) {
    fwfs_off_t src_offset, dst_offset;
    fwfs_block_t subcount;
    fwfs_tftl_gcstate_t gcstate;

    fwfs_block_t last_ctz;
    fwfs_block_t first_non_ctz;
    uint8_t state;
    int i, j, m;

    int err;
    bool fastpath = false;

    subcount = fwfs_tftl_subblock_count(fwfs);
    fwfs_tftl_gc_gcstate_clear(&gcstate);

    err = fwfs->cfg->erase(fwfs->cfg, gc_block);
    if (err) {
        FWFS_ERROR("erase return err %d\n", err);
        return err;
    }

    // Assign invalid values first
    last_ctz = subcount;
    first_non_ctz = subcount;
    for (i = 0; i < subcount; i++) {
        state = fwfs_tftl_get_state(fwfs, fwfs->tftl.states, block, i);
        if (state == TFTL_STATE_CTZ) {
            last_ctz = i;
        } else if (first_non_ctz == subcount) {
            first_non_ctz = i;
        }
    }

    if (first_non_ctz == subcount || last_ctz == subcount) {
        FWFS_ERROR("Invalid valid: first_non_ctz=0x%"PRIx32", last_ctz=0x%"PRIx32"\n",
                   first_non_ctz, last_ctz);
        return FWFS_ERR_INVAL;
    }

    if ((last_ctz + 1) == first_non_ctz) {
        fastpath = true;
    }

    // Build the map of subblock id
    for (i = 0, j = 0; i < subcount; i++) {
        state = fwfs_tftl_get_state(fwfs, fwfs->tftl.states, block, i);
        if (state != TFTL_STATE_CTZ) {
            if(i < last_ctz) {
                m = fwfs_tftl_gc_count_ctz_from(fwfs, block, i);
                if (i < 8) {
                    gcstate.map[0] |= (i + m) << (i * 4);
                } else {
                    gcstate.map[1] |= (i + m) << ((i - 8) * 4);
                }
            } else {
                if (i < 8) {
                    gcstate.map[0] |= (i) << (i * 4);
                } else {
                    gcstate.map[1] |= (i) << ((i - 8) * 4);
                }
            }
            continue;
        }

        src_offset = i * fwfs->cfg->subblock_size;
        dst_offset = j * fwfs->cfg->subblock_size;

        err = fwfs_tftl_gc_copy_subblock(fwfs, block, src_offset, gc_block, dst_offset);
        if (err)
            return err;

        if (i < 8) {
            gcstate.map[0] |= j << (i * 4);
        } else {
            gcstate.map[1] |= j << ((i - 8) * 4);
        }
        j++;
    }

    FWFS_ASSERT(j > 0);

    gcstate.gc_target = TFTL_MKLEB(TFTL_MAGIC_GC, last_ctz, block);
    gcstate.gc_block = TFTL_MKLEB(TFTL_MAGIC_GC, 0x0, gc_block);

    err = fwfs_tftl_gc_write_gcstate(fwfs, &gcstate);
    if (err)
        return err;

    memcpy(fwfs->tftl.priv, &gcstate, sizeof(fwfs_tftl_gcstate_t));
    fwfs_tftl_setgc(fwfs, fwfs->tftl.states, gcstate.gc_block);

    if (fastpath) {
        err = fwfs_tftl_gc_end(fwfs);
        return err;
    }

    for (i = 0; i < last_ctz; i++) {
        state = fwfs_tftl_get_state(fwfs, fwfs->tftl.states, block, i);
        if (state == TFTL_STATE_CTZ)
            continue;
        fwfs_tftl_set_state(fwfs, fwfs->tftl.states, block, i, TFTL_STATE_FREE);
    }
    return 0;
}

static int fwfs_tftl_gc_end(fwfs_t *fwfs) {
    fwfs_block_t src_block, dst_block;
    fwfs_off_t src_offset, dst_offset;

    fwfs_block_t last_ctz;
    int i, err;
    fwfs_tftl_gcstate_t *gcstate;

    gcstate = (fwfs_tftl_gcstate_t *)fwfs->tftl.priv;

    src_block = fwfs_tftl_block_id_raw(gcstate->gc_block);
    dst_block = fwfs_tftl_block_id_raw(gcstate->gc_target);

    last_ctz = fwfs_tftl_subblock_id_raw(gcstate->gc_target);

    err = fwfs->cfg->erase(fwfs->cfg, dst_block);
    if (err) {
        FWFS_ERROR("erase return err %d\n", err);
        return err;
    }

    for (i = 0; i <= last_ctz; i++) {
        if (i < 8) {
            src_offset = ((gcstate->map[0] >> (i * 4)) & 0xf) * fwfs->cfg->subblock_size;
        } else {
            src_offset = ((gcstate->map[1] >> ((i - 8) * 4)) & 0xf) * fwfs->cfg->subblock_size;
        }
        dst_offset = i * fwfs->cfg->subblock_size;
        err = fwfs_tftl_gc_copy_subblock(fwfs, src_block, src_offset, dst_block, dst_offset);
        if (err)
            return err;
    }

    err = fwfs_tftl_gc_write_gcstate(fwfs, NULL);
    if (err)
        return err;

    fwfs_tftl_gc_gcstate_clear(gcstate);

    for (i = (last_ctz + 1); i < fwfs_tftl_subblock_count(fwfs); i++) {
        fwfs_tftl_set_state(fwfs, fwfs->tftl.states, dst_block, i, TFTL_STATE_FREE);
    }
    fwfs_tftl_setfree(fwfs, fwfs->tftl.states, src_block);
    return 0;
}

static int fwfs_tftl_gc_visitor_commit(fwfs_t *fwfs) {
    fwfs_block_t block_id;
    fwfs_block_t last_ctz;
    uint8_t state;
    int err;
    bool writeback = true;
    fwfs_tftl_gcstate_t *gcstate;
    fwfs_block_t i;

    gcstate = (fwfs_tftl_gcstate_t *)fwfs->tftl.priv;
    if (!fwfs_tftl_gcstate_hasgc(gcstate))
        return 0;

    block_id = fwfs_tftl_block_id_raw(gcstate->gc_target);
    last_ctz = fwfs_tftl_subblock_id_raw(gcstate->gc_target);

    for (i = 0; i <= last_ctz; i++) {
        if (fwfs_tftl_get_state(fwfs, fwfs->tftl.states, block_id, i) ==
            TFTL_STATE_CTZ) {
            writeback = false;
            break;
        }
    }

    if (writeback) {
        fwfs_block_t gc_block;

        err = fwfs_tftl_gc_write_gcstate(fwfs, NULL);
        if (err) {
            FWFS_ERROR("fwfs_tftl_gc_write_gcstate return false!\n");
            return err;
        }

        gc_block = fwfs_tftl_block_id_raw(gcstate->gc_block);
        fwfs_tftl_setfree(fwfs, fwfs->tftl.states, gc_block);

        fwfs_tftl_gc_gcstate_clear(gcstate);
        return 0;
    }

    if (fwfs->tftl.checked)
        return 0;

    writeback = true;
    for (i = 0; i <= last_ctz; i++) {
        state = fwfs_tftl_get_state(fwfs, fwfs->tftl.states, block_id, i);
        if (state == TFTL_STATE_UNKNOWN) {
            fwfs_block_t leb = TFTL_MKLEB(TFTL_MAGIC_CTZ, i, block_id);

            if (!fwfs_tftl_partial_write_check(fwfs, leb)) {
                state = TFTL_STATE_ABANDON;
            } else {
                state = TFTL_STATE_FREE;
                writeback = false;
            }
            fwfs_tftl_set_state(fwfs, fwfs->tftl.states, block_id, i, state);
        }
    }

    if (writeback) {
        err = fwfs_tftl_gc_end(fwfs);
        if (err) {
            return err;
        }
    }

    return 0;
}

static int fwfs_tftl_gc_alloc_subblock(fwfs_t *fwfs, fwfs_block_t *block) {
    fwfs_block_t block_id;
    fwfs_block_t subblock_id;
    fwfs_block_t last_ctz;
    uint8_t state = TFTL_STATE_INVALID;
    fwfs_block_t i;
    int err;
    fwfs_tftl_gcstate_t *gcstate = (fwfs_tftl_gcstate_t *)fwfs->tftl.priv;

    block_id = fwfs_tftl_block_id_raw(gcstate->gc_target);
    last_ctz = fwfs_tftl_subblock_id_raw(gcstate->gc_target);

    for (i = 0; i < last_ctz; i++) {
        state = fwfs_tftl_get_state(fwfs, fwfs->tftl.states, block_id, i);
        if (state == TFTL_STATE_FREE) {
            subblock_id = i;
            break;
        }
    }

    if (state != TFTL_STATE_FREE) {
        err = fwfs_tftl_gc_end(fwfs);
        return err ? err : fwfs_tftl_return_nospc(fwfs);
    }

    FWFS_ASSERT(state == TFTL_STATE_FREE);

    fwfs_tftl_setctz(fwfs, fwfs->tftl.states, block_id, subblock_id);
    *block = TFTL_MKLEB(TFTL_MAGIC_CTZ, subblock_id, block_id);
    return 0;
}

void fwfs_tftl_gc_init(fwfs_t *fwfs)
{
    fwfs_tftl_gcstate_t tmp;
    fwfs_tftl_gcstate_t *gcstate;
    int err;

    gcstate = &tmp;
    err = fwfs_tftl_gc_read_gcstate(fwfs, gcstate);
    if (err)
        return;

    if (fwfs_tftl_gcstate_hasgc(gcstate)) {
        fwfs_block_t gc_block = fwfs_tftl_block_id_raw(gcstate->gc_block);

        if (fwfs->cfg->debug >= 2)
            fwfs_tftl_gc_dump_gcstate(gcstate);
        fwfs_tftl_setgc(fwfs, fwfs->tftl.states, gc_block);
        memcpy(fwfs->tftl.priv, gcstate, sizeof(fwfs_tftl_gcstate_t));
    }
}

fwfs_size_t fwfs_tftl_header_size(fwfs_t *fwfs, fwfs_block_t block) {
    (void)fwfs;
    if (fwfs_tftl_magic(block) == TFTL_MAGIC_CTZ) {
        return FWFS_TFTL_HEADER_SIZE;
    } else if (fwfs_tftl_magic(block) == TFTL_MAGIC_METADATA) {
        return 0;
    } else {
        FWFS_ERROR("Invalid LEB 0x%"PRIx32" (0x%x)", block, fwfs_tftl_magic(block));
        FWFS_ASSERT(false);
        return 0;
    }
}

fwfs_size_t fwfs_tftl_block_size(fwfs_t *fwfs, fwfs_block_t block) {
    if (fwfs_tftl_magic(block) == TFTL_MAGIC_CTZ) {
        return fwfs->cfg->subblock_size;
    } else if (fwfs_tftl_magic(block) == TFTL_MAGIC_METADATA) {
        return fwfs->cfg->block_size;
    } else if (fwfs_tftl_magic(block) == TFTL_MAGIC_INLINE) {
        return fwfs->cfg->block_size;
    } else {
        FWFS_ERROR("Invalid LEB 0x%"PRIx32" (0x%x)", block, fwfs_tftl_magic(block));
        FWFS_ASSERT(false);
        return 0;
    }
}

int fwfs_tftl_fs_size(fwfs_t *fwfs, fwfs_size_t *size) {
    uint32_t i, j;
    uint8_t state;
    int err;

    fwfs_tftl_visitor_prepare(fwfs);
    err = fwfs_fs_traverseraw(fwfs, fwfs_tftl_visitor, fwfs, false);
    if (err)
        return err;

    *size = 0;
    for (i = 0; i < fwfs->cfg->block_count; i++) {
        state = TFTL_STATE_INVALID;
        for (j = 0; j < fwfs_tftl_subblock_count(fwfs); j++) {
            state = fwfs_tftl_get_state(fwfs, fwfs->tftl.snapshot, i, j);
            if (state == TFTL_STATE_METADATA) {
                *size += fwfs_tftl_subblock_count(fwfs);
                break;
            } else if (state == TFTL_STATE_CTZ) {
                *size += 1;
            }
        }
    }

    // add gc block
    *size += fwfs_tftl_subblock_count(fwfs);
    if (*size > (fwfs->cfg->block_count * fwfs_tftl_subblock_count(fwfs)))
        FWFS_ERROR("Invalid used size %d\n", *size);
    return err;
}

int fwfs_tftl_header_fill(fwfs_t *fwfs, fwfs_tftl_header_t *header,
                          fwfs_block_t block, fwfs_off_t index,
                          uint32_t fid)
{
    (void)fwfs;
    if (!fwfs_tftl_leb_check(block, false)) {
        FWFS_ERROR("Invalid LEB 0x%"PRIx32" (0x%x)", block,
                   fwfs_tftl_magic(block));
        FWFS_ASSERT(false);
        return 0;
    }

    if (fwfs_tftl_magic(block) != TFTL_MAGIC_CTZ) {
        return 0;
    }

    header->tag = index & 0x0000ffff;
    header->tag = header->tag | (fid & 0x7fff0000);
    //header->tag = 0x7f7f7f7f;
    header->tag = header->tag & 0x7fffffff;
    header->tag = fwfs_tobe32(header->tag);
    header->reserved = 0x0;

    return 0;
}

int fwfs_tftl_debug(fwfs_t *fwfs)
{
    if (fwfs->cfg->debug >= 3) {
        int err;

        fwfs_tftl_visitor_prepare(fwfs);
        err = fwfs_fs_traverse(fwfs, fwfs_tftl_visitor, fwfs);
        if (err) {
            fwfs_alloc_reset(fwfs);
            return err;
        }
        fwfs_tftl_visitor_commit(fwfs);

        fwfs_tftl_dump_states(fwfs);
    }
    return 0;
}

int fwfs_tftl_read (const fwfs_t *fwfs, fwfs_block_t block,
                    fwfs_off_t off, void *buffer, fwfs_size_t size) {
    FWFS_TRACE("fwfs_tftl_read(%p, 0x%"PRIx32", 0x%"PRIx32", %p, "
               "%"PRIu32")", (void*)fwfs->cfg,
               block, off, (void*)buffer, size);
    fwfs_block_t block_id = fwfs_tftl_block_id(fwfs, block);

    if (fwfs_tftl_magic(block) == TFTL_MAGIC_CTZ) {
        fwfs_block_t subblock_id = fwfs_tftl_subblock_id(fwfs, block);
        fwfs_off_t offset = subblock_id * fwfs->cfg->subblock_size;

        FWFS_TRACE("fwfs_tftl_read(%p, 0x%"PRIx32", 0x%"PRIx32", 0x%"PRIx32")",
                   (void*)fwfs->cfg, block_id, subblock_id, offset);
        return fwfs->cfg->read(fwfs->cfg, block_id, off + offset, buffer, size);
    } else if (fwfs_tftl_magic(block) == TFTL_MAGIC_METADATA) {
        FWFS_TRACE("fwfs_tftl_read(%p, 0x%"PRIx32", 0x%"PRIx32")",
                   (void*)fwfs->cfg, block_id, off);
        return fwfs->cfg->read(fwfs->cfg, block_id, off, buffer, size);
    } else {
        FWFS_ERROR("Invalid LEB 0x%"PRIx32" (0x%x)", block, fwfs_tftl_magic(block));
        FWFS_ASSERT(false);
        return FWFS_ERR_INVAL;
    }
}

int fwfs_tftl_prog(const fwfs_t *fwfs, fwfs_block_t block,
                   fwfs_off_t off, const void *buffer, fwfs_size_t size) {
    FWFS_TRACE("fwfs_tftl_prog(%p, 0x%"PRIx32", 0x%"PRIx32", %p, %"PRIu32")",
               (void*)fwfs->cfg, block, off, (void*)buffer, size);
    fwfs_block_t block_id = fwfs_tftl_block_id(fwfs, block);

    if (fwfs_tftl_magic(block) == TFTL_MAGIC_CTZ) {
        fwfs_block_t subblock_id = fwfs_tftl_subblock_id(fwfs, block);
        fwfs_off_t offset = subblock_id * fwfs->cfg->subblock_size;

        FWFS_TRACE("fwfs_tftl_prog(%p, 0x%"PRIx32", 0x%"PRIx32", 0x%"PRIx32")",
                   (void*)fwfs->cfg, block_id, subblock_id, offset);
        return fwfs->cfg->prog(fwfs->cfg, block_id, off + offset, buffer, size);
    } else if (fwfs_tftl_magic(block) == TFTL_MAGIC_METADATA) {
        FWFS_TRACE("fwfs_tftl_prog(%p, 0x%"PRIx32", 0x%"PRIx32")",
                   (void*)fwfs->cfg, block_id, off);
        return fwfs->cfg->prog(fwfs->cfg, block_id, off, buffer, size);
    } else {
        FWFS_ERROR("Invalid LEB 0x%"PRIx32" (0x%x)", block, fwfs_tftl_magic(block));
        FWFS_ASSERT(false);
        return FWFS_ERR_INVAL;
    }
}

int fwfs_tftl_erase(const fwfs_t *fwfs, fwfs_block_t block) {
    FWFS_TRACE("fwfs_tftl_erase(%p, 0x%"PRIx32")", (void*)fwfs->cfg, block);

    if (fwfs_tftl_magic(block) == TFTL_MAGIC_CTZ) {
        return 0;
    } else if (fwfs_tftl_magic(block) == TFTL_MAGIC_METADATA) {
        return fwfs->cfg->erase(fwfs->cfg, fwfs_tftl_block_id(fwfs, block));
    } else {
        FWFS_ERROR("Invalid LEB 0x%"PRIx32" (0x%x)", block, fwfs_tftl_magic(block));
        FWFS_ASSERT(false);
        return FWFS_ERR_INVAL;
    }
}

int fwfs_tftl_sync(const fwfs_t *fwfs) {
    FWFS_TRACE("fwfs_tftl_sync(%p)", (void*)fwfs->cfg);
    return fwfs->cfg->sync(fwfs->cfg);
}
