/*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#include "autoconf.h"

//==================================
//int-ll64.h
typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

//===================
//types.h	include\uapi\linux

typedef unsigned short __u16;

typedef unsigned int __u32;

typedef __u16  __le16;
typedef __u16  __be16;
typedef __u32  __le32;
typedef __u32  __be32;

//==================
//kobject.h	include\linux

//fake
struct kobject {
    char padding[36];
};

struct list_head {
    struct list_head *next, *prev;
};

//==================
// include\linux\sysfs.h
typedef unsigned short		umode_t;

#ifdef CONFIG_DEBUG_LOCK_ALLOC
struct hlist_head {
    struct hlist_node *first;
};

struct hlist_node {
    struct hlist_node *next, **pprev;
};

struct lockdep_subclass_key {
    char __one_byte;
} __attribute__ ((__packed__));

/* hash_entry is used to keep track of dynamically allocated keys. */
#define MAX_LOCKDEP_SUBCLASSES      8UL
struct lock_class_key {
    union {
        struct hlist_node       hash_entry;
        struct lockdep_subclass_key subkeys[MAX_LOCKDEP_SUBCLASSES];
    };
};
#endif

typedef struct attribute {
	const char		*name;
	umode_t			mode;
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	bool			ignore_lockdep:1;
	struct lock_class_key	*key;
	struct lock_class_key	skey;
#endif
}attribute;

struct bin_attribute { // 28
	struct attribute	attr;
	size_t			size;
	void			*private;
	int (*read)(void * para);
	int (*write)(void * para);
	int (*mmap)(void * para);
};


//==================================
//#include <linux/fwnode.h>
enum fwnode_type {
	FWNODE_INVALID = 0,
	FWNODE_OF,
	FWNODE_ACPI,
	FWNODE_ACPI_DATA,
	FWNODE_PDATA,
	FWNODE_IRQCHIP,
};

#define FWNODE_FLAG_LINKS_ADDED		BIT(0)
#define FWNODE_FLAG_NOT_DEVICE		BIT(1)
#define FWNODE_FLAG_INITIALIZED		BIT(2)
#define _ANDROID_KABI_RESERVE(n)		u64 android_kabi_reserved##n


#ifdef CONFIG_ANDROID_KABI_RESERVE
#define ANDROID_KABI_RESERVE(number)	_ANDROID_KABI_RESERVE(number)
#else
#define ANDROID_KABI_RESERVE(number)
#endif

struct fwnode_handle {
	struct fwnode_handle *secondary;
	const struct fwnode_operations *ops;
	struct device *dev;
	struct list_head suppliers;
	struct list_head consumers;
	u8 flags;
	ANDROID_KABI_RESERVE(1);
};
//==================================
//of.h

/* flag descriptions (need to be visible even when !CONFIG_OF) */
#define OF_DYNAMIC	1 /* node and properties were allocated via kmalloc */
#define OF_DETACHED	2 /* node has been detached from the device tree */
#define OF_POPULATED	3 /* device already created for the node */
#define OF_POPULATED_BUS	4 /* of_platform_populate recursed to children of this node */

#define for_each_property_of_node(dn, pp) \
	for (pp = dn->properties; pp != NULL; pp = pp->next)

#define OF_BAD_ADDR	((u64)-1)
typedef u32 phandle;
typedef u32 ihandle;
//typedef u32 bool;

struct property {
	char	*name;
	int	length;
	void	*value;
	struct property *next;
	struct bin_attribute attr;
};


struct device_node {
	const char *name;
	phandle phandle;
	const char *full_name;
	struct fwnode_handle fwnode;
	struct	property *properties;
	struct	property *deadprops;/* removed properties */
	struct	device_node *parent;
	struct	device_node *child;
	struct	device_node *sibling;
	struct	kobject kobj;
	unsigned long _flags;
	void	*data;

};

static inline const char *of_node_full_name(const struct device_node *np)
{
	return np ? np->full_name : "<no-node>";
}


#define BITS_PER_LONG 32

#define BIT_MASK(nr)		(1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)		((nr) / BITS_PER_LONG)
static inline void set_bit(int nr, volatile unsigned long *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);
	unsigned long flags;

//	_atomic_spin_lock_irqsave(p, flags);
	*p  |= mask;
//	_atomic_spin_unlock_irqrestore(p, flags);
}

static inline void of_node_set_flag(struct device_node *n, unsigned long flag)
{
	set_bit(flag, &n->_flags);
}

