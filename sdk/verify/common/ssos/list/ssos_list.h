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
#ifndef SSOS_LIST_H
#define SSOS_LIST_H

#undef OFFSETOF
#ifdef size_t
#define OFFSETOF(TYPE, MEMBER) ((size_t) & ((TYPE *)0)->MEMBER)
#else
#define OFFSETOF(TYPE, MEMBER) ((unsigned long)&((TYPE *)0)->MEMBER)
#endif

#define CONTAINER_OF(ptr, type, member)                          \
    (                                                            \
        {                                                        \
            const typeof(((type *)0)->member) *__mptr = (ptr);   \
            ((type *)((char *)__mptr - OFFSETOF(type, member))); \
        })

struct SSOS_LIST_Head_s
{
    struct SSOS_LIST_Head_s *next, *prev;
};

#define SSOS_LIST_HEAD_INIT(name) \
    {                             \
        &(name), &(name)          \
    }

#define SSOS_LIST_HEAD(name) struct SSOS_LIST_Head_s name = SSOS_LIST_HEAD_INIT(name)
static inline void SSOS_LIST_InitHead(struct SSOS_LIST_Head_s *list) // NOLINT
{
    list->next = list;
    list->prev = list;
}
#define SSOS_LIST_FOR_EACH(pos, head) for ((pos) = (head)->next; (pos) != (head); (pos) = (pos)->next)
#define SSOS_LIST_FOR_EACH_SAFE(pos, n, head) \
    for (pos = (head)->next, n = (pos)->next; (pos) != (head); (pos) = (n), (n) = (pos)->next)
#define SSOS_LIST_ENTRY(ptr, type, member)       CONTAINER_OF(ptr, type, member)
#define SSOS_LIST_FIRST_ENTRY(ptr, type, member) SSOS_LIST_ENTRY((ptr)->next, type, member)
#define SSOS_LIST_LAST_ENTRY(ptr, type, member)  SSOS_LIST_ENTRY((ptr)->prev, type, member)
#define SSOS_LIST_FOR_EACH_ENTRY(pos, head, member)                                         \
    for (pos = SSOS_LIST_ENTRY((head)->next, typeof(*pos), member); &pos->member != (head); \
         pos = SSOS_LIST_ENTRY(pos->member.next, typeof(*pos), member))

#define SSOS_LIST_FOR_EACH_ENTRY_SAFE(pos, n, head, member)                                     \
    for (pos                         = SSOS_LIST_ENTRY((head)->next, typeof(*pos), member),     \
        n                            = SSOS_LIST_ENTRY(pos->member.next, typeof(*pos), member); \
         &pos->member != (head); pos = n, n = SSOS_LIST_ENTRY(n->member.next, typeof(*n), member))

#define SSOS_LIST_FOR_EACH_ENTRY_SAFE_REVERSE(pos, n, head, member)                             \
    for (pos                         = SSOS_LIST_ENTRY((head)->prev, typeof(*pos), member),     \
        n                            = SSOS_LIST_ENTRY(pos->member.prev, typeof(*pos), member); \
         &pos->member != (head); pos = n, n = SSOS_LIST_ENTRY(n->member.prev, typeof(*n), member))

static inline int SSOS_LIST_Empty(const struct SSOS_LIST_Head_s *head) // NOLINT
{
    return head->next == head;
}

static inline void _SSOS_LIST_Add(struct SSOS_LIST_Head_s *new_, struct SSOS_LIST_Head_s *prev, // NOLINT
                                  struct SSOS_LIST_Head_s *next)
{
    next->prev = new_;
    new_->next = next;
    new_->prev = prev;
    prev->next = new_;
}

static inline void SSOS_LIST_AddTail(struct SSOS_LIST_Head_s *new_, struct SSOS_LIST_Head_s *head) // NOLINT
{
    _SSOS_LIST_Add(new_, head->prev, head);
}

static inline void _SSOS_LIST_Del(struct SSOS_LIST_Head_s *prev, struct SSOS_LIST_Head_s *next)
{
    next->prev = prev;
    prev->next = next;
}

#define SSOS_LIST_POISON1 ((void *)0x00100100)
#define SSOS_LIST_POISON2 ((void *)0x00200200)
static inline void SSOS_LIST_Del(struct SSOS_LIST_Head_s *entry) // NOLINT
{
    _SSOS_LIST_Del(entry->prev, entry->next);
    entry->next = (struct SSOS_LIST_Head_s *)SSOS_LIST_POISON1;
    entry->prev = (struct SSOS_LIST_Head_s *)SSOS_LIST_POISON2;
}
static inline void SSOS_LIST_Add(struct SSOS_LIST_Head_s *new_, struct SSOS_LIST_Head_s *head) // NOLINT
{
    _SSOS_LIST_Add(new_, head, head->next);
}

#define SSOS_LIST_FOR_EACH_ENTRY_REVERSE(pos, head, member)                                 \
    for (pos = SSOS_LIST_ENTRY((head)->prev, typeof(*pos), member); &pos->member != (head); \
         pos = SSOS_LIST_ENTRY(pos->member.prev, typeof(*pos), member))
#endif
