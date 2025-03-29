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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//#include <libfdt_env.h>
//#include <fdt.h>
#include <libfdt.h>
#include <util.h>

#include "dtb2unfdt.h"

//#include <uapi/asm-generic/errno-base.h>
#define	EINVAL		22	/* Invalid argument */

//===================
#define ALIGN(x, a)	(((x) + ((a) - 1)) & ~((a) - 1))
#define PALIGN(p, a)	((void *)(ALIGN((unsigned long)(p), (a))))
#define GET_CELL(p)	(p += 4, *((const uint32_t *)(p-4)))
#define PTR_ALIGN(p, a)		((typeof(p))ALIGN((unsigned long)(p), (a)))  //linux/kernel.h

//===================
//#define DEBUG
#ifdef DEBUG
#define pr_warn printf
#define pr_warning printf
#define pr_err printf
#else
#define pr_warn //printf
#define pr_warning //printf
#define pr_err //printf
#endif
#define WARN_ON_ONCE(x)  (x)
//===================
//convert be32_to_cpu/cpu_to_be32
#include <arpa/inet.h>
#define  cpu_to_be32(x) htonl((uint32_t )x)
#define  be32_to_cpu(x) ntohl((uint32_t )x)

//===================
#define of_compat_cmp(s1, s2, l)	strcasecmp((s1), (s2))
#define of_prop_cmp(s1, s2)		strcmp((s1), (s2))
#define of_node_cmp(s1, s2)		strcasecmp((s1), (s2))

struct device_node *of_root;
bool of_fdt_device_is_available(const void *blob, unsigned long node);
static struct property *__of_find_property(const struct device_node *np,
		const char *name, int *lenp)
{
	struct property *pp;

	if (!np)
		return NULL;

	for (pp = np->properties; pp; pp = pp->next) {
		if (of_prop_cmp(pp->name, name) == 0) {
			if (lenp)
				*lenp = pp->length;
			break;
		}
	}

	return pp;
}
struct property *of_find_property(const struct device_node *np,
		const char *name,
		int *lenp)
{
	struct property *pp;
	//unsigned long flags;

	//raw_spin_lock_irqsave(&devtree_lock, flags);
	pp = __of_find_property(np, name, lenp);
	//raw_spin_unlock_irqrestore(&devtree_lock, flags);

	return pp;
}
const void *of_get_property(const struct device_node *np, const char *name,
		int *lenp)
{
	struct property *pp = of_find_property(np, name, lenp);

	return pp ? pp->value : NULL;
}
//=================
static void *unflatten_dt_alloc(void **mem, unsigned long size,
		unsigned long align)
{
	void *res;

	*mem = PTR_ALIGN(*mem, align);
	res = *mem;
	*mem += size;

	return res;
}

static void populate_properties(const void *blob,
		int offset,
		void **mem,
		struct device_node *np,
		const char *nodename,
		bool dryrun)
{
	struct property *pp, **pprev = NULL;
	int cur;
	bool has_name = false;

	pprev = &np->properties;
	for (cur = fdt_first_property_offset(blob, offset);
			cur >= 0;
			cur = fdt_next_property_offset(blob, cur)) {
		const __be32 *val;
		const char *pname;
		u32 sz;

		val = fdt_getprop_by_offset(blob, cur, &pname, &sz);
		if (!val) {
			pr_warn("Cannot locate property at 0x%x\n", cur);
			continue;
		}

		if (!pname) {
			pr_warn("Cannot find property name at 0x%x\n", cur);
			continue;
		}

		if (!strcmp(pname, "name"))
			has_name = true;

		pp = unflatten_dt_alloc(mem, sizeof(struct property),
				__alignof__(struct property));
		if (dryrun)
			continue;

		/* We accept flattened tree phandles either in
		 * ePAPR-style "phandle" properties, or the
		 * legacy "linux,phandle" properties.  If both
		 * appear and have different values, things
		 * will get weird. Don't do that.
		 */
		if (!strcmp(pname, "phandle") ||
				!strcmp(pname, "linux,phandle")) {
			if (!np->phandle)
				np->phandle = be32_to_cpu(*val);
		}

		/* And we process the "ibm,phandle" property
		 * used in pSeries dynamic device tree
		 * stuff
		 */
		if (!strcmp(pname, "ibm,phandle"))
			np->phandle = be32_to_cpu(*val);

		pp->name   = (char *)pname;
		pp->length = sz;
		pp->value  = (__be32 *)val;
		*pprev     = pp;
		pprev      = &pp->next;
	}

	/* With version 0x10 we may not have the name property,
	 * recreate it here from the unit name if absent
	 */
	if (!has_name) {
		const char *p = nodename, *ps = p, *pa = NULL;
		int len;

		while (*p) {
			if ((*p) == '@')
				pa = p;
			else if ((*p) == '/')
				ps = p + 1;
			p++;
		}

		if (pa < ps)
			pa = p;
		len = (pa - ps) + 1;
		pp = unflatten_dt_alloc(mem, sizeof(struct property) + len,
				__alignof__(struct property));
		if (!dryrun) {
			pp->name   = "name";
			pp->length = len;
			pp->value  = pp + 1;
			*pprev     = pp;
			pprev      = &pp->next;
			memcpy(pp->value, ps, len - 1);
			((char *)pp->value)[len - 1] = 0;
			pr_err("fixed up name for %s -> %s\n",
					nodename, (char *)pp->value);
		}
	}

	if (!dryrun)
		*pprev = NULL;
}

static bool populate_node(const void *blob,
			  int offset,
			  void **mem,
			  struct device_node *dad,
			  struct device_node **pnp,
			  bool dryrun)
{
	struct device_node *np;
	const char *pathp;
	unsigned int l, allocl;

	pathp = fdt_get_name(blob, offset, &l);
	if (!pathp) {
		*pnp = NULL;
		return false;
	}

	allocl = ++l;

	np = unflatten_dt_alloc(mem, sizeof(struct device_node) + allocl,
				__alignof__(struct device_node));
	if (!dryrun) {
		char *fn;
		//of_node_init(np);
		np->full_name = fn = ((char *)np) + sizeof(*np);

		memcpy(fn, pathp, l);

		if (dad != NULL) {
			np->parent = dad;
			np->sibling = dad->child;
			dad->child = np;
		}
	}

	populate_properties(blob, offset, mem, np, pathp, dryrun);
	if (!dryrun) {
		np->name = of_get_property(np, "name", NULL);
		if (!np->name)
			np->name = "<NULL>";
	}

	*pnp = np;
	return true;
}

static void reverse_nodes(struct device_node *parent)
{
	struct device_node *child, *next;

	/* In-depth first */
	child = parent->child;
	while (child) {
		reverse_nodes(child);

		child = child->sibling;
	}

	/* Reverse the nodes in the child list */
	child = parent->child;
	parent->child = NULL;
	while (child) {
		next = child->sibling;

		child->sibling = parent->child;
		parent->child = child;
		child = next;
	}
}

struct device_node *__of_find_all_nodes(struct device_node *prev)
{
	struct device_node *np;
	if (!prev) {
		np = of_root;
	} else if (prev->child) {
		np = prev->child;
	} else {
		/* Walk back up looking for a sibling, or the end of the structure */
		np = prev;
		while (np->parent && !np->sibling)
			np = np->parent;
		np = np->sibling; /* Might be null at the end of the tree */
	}
	return np;
}



void show_unfdt(struct device_node *dad)
{
	struct device_node *np;
	struct property *pp;
		int x=0;
	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

	//revise root all of trees
	for (np = dad; np; np = __of_find_all_nodes(np)) {
		if(x<=10){
			printf("\nnp: %px  \n", np);
			if (np->name)        printf("name      :%px %s\n", np->name, np->name);
			if (np->phandle)     printf("phandle   :%x\n", np->phandle);
			if (np->full_name)   printf("full_name :%px %s\n", np->full_name,np->full_name);
			//if (np->deadprops)   printf("deadprops :%px\n", np->deadprops);
			if (np->parent)      printf("parent    :%px\n", np->parent);
			if (np->child)       printf("child     :%px\n", np->child);
			if (np->sibling)     printf("sibling   :%px\n", np->sibling);
			if (np->data)        printf("data      :%px\n", np->data);
			if (np->_flags)      printf("_flags    :0x%lx\n", np->_flags);
			if (np->properties)  printf("properties:%px\n", np->properties);
#if 1
			if (np->properties) {
				for_each_property_of_node(np, pp) {
					if (!strcmp(pp->name ,"compatible"))
						printf(">> pp:%px len:(%2d) (%s)\t[%s]\n", pp, pp->length, pp->name,(char*) pp->value);
					else if (!pp->length)
						printf(">> pp:%px len:(%2d) (%s)\n", pp, pp->length, pp->name);
					else if ( pp->length==4 )
						printf(">> pp:%px len:(%2d) (%s)\t[%08x]\n", pp, pp->length, pp->name, be32_to_cpu( *(int*)pp->value));
					else if ( pp->length==8 )
						printf(">> pp:%px len:(%2d) (%s)\t[%08x %08x]\n", pp, pp->length, pp->name, be32_to_cpu(*(int*)pp->value),be32_to_cpu( *(int*)(pp->value+4)));
					else if ( !(pp->length%4) )
						printf(">> pp:%px len:(%2d) (%s)\t[%08x %08x %08x]\n", pp, pp->length, pp->name, be32_to_cpu(*(int*)pp->value),be32_to_cpu( *(int*)(pp->value+4)), be32_to_cpu(*(int*)(pp->value+8)));
					else
						printf(">> pp:%px len:(%2d) (%s)\t[%s]\n", pp, pp->length, pp->name,(char*) pp->value);
					if (pp->attr.attr.name)  printf("      >>pp->attr.attr.name:%px\n",pp->attr.attr.name);
					if (pp->attr.write)      printf("      >>pp->attr.write:%px\n", pp->attr.write);
					if (pp->attr.private)    printf("      >>pp->attr.private:%px\n", pp->attr.private);
					if (pp->attr.read)       printf("      >>pp->attr.read:%px\n", pp->attr.read);
					if (pp->attr.mmap)       printf("      >>pp->attr.mmap:%px\n", pp->attr.mmap);

				}
			}
#endif
		}
		x++;
	}
	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
}

/**
 * unflatten_dt_nodes - Alloc and populate a device_node from the flat tree
 * @blob: The parent device tree blob
 * @mem: Memory chunk to use for allocating device nodes and properties
 * @dad: Parent struct device_node
 * @nodepp: The device_node tree created by the call
 *
 * It returns the size of unflattened device tree or error code
 */
static int unflatten_dt_nodes(const void *blob,
		void *mem,
		struct device_node *dad,
		struct device_node **nodepp)
{
	struct device_node *root;
	int offset = 0, depth = 0, initial_depth = 0;
#define FDT_MAX_DEPTH	64
	struct device_node *nps[FDT_MAX_DEPTH];
	void *base = mem;
	bool dryrun = !base;

	if (nodepp)
		*nodepp = NULL;

	/*
	 * We're unflattening device sub-tree if @dad is valid. There are
	 * possibly multiple nodes in the first level of depth. We need
	 * set @depth to 1 to make fdt_next_node() happy as it bails
	 * immediately when negative @depth is found. Otherwise, the device
	 * nodes except the first one won't be unflattened successfully.
	 */
	if (dad)
		depth = initial_depth = 1;

	root = dad;
	nps[depth] = dad;

	for (offset = 0;
		offset >= 0 && depth >= initial_depth;
		offset = fdt_next_node(blob, offset, &depth)) {
		if (WARN_ON_ONCE(depth >= FDT_MAX_DEPTH))
			continue;

		if (0 &&
		    !of_fdt_device_is_available(blob, offset))
			continue;

		if (!populate_node(blob, offset, &mem, nps[depth],
				   &nps[depth+1], dryrun))
			return mem - base;

		if (!dryrun && nodepp && !*nodepp)
			*nodepp = nps[depth+1];
		if (!dryrun && !root)
			root = nps[depth+1];
	}

	if (offset < 0 && offset != -FDT_ERR_NOTFOUND) {
		pr_err("Error %d processing FDT\n", offset);
		return -EINVAL;
	}

	/*
	 * Reverse the child list. Some drivers assumes node order matches .dts
	 * node order
	 */
	if (!dryrun)
		reverse_nodes(root);

	return mem - base;
}

/**
 * __unflatten_device_tree - create tree of device_nodes from flat blob
 *
 * unflattens a device-tree, creating the
 * tree of struct device_node. It also fills the "name" and "type"
 * pointers of the nodes so the normal device-tree walking functions
 * can be used.
 * @blob: The blob to expand
 * @dad: Parent device node
 * @mynodes: The device_node tree created by the call
 * @dt_alloc: An allocator that provides a virtual address to memory
 * for the resulting tree
 *
 * Returns NULL on failure or the memory chunk containing the unflattened
 * device tree on success.
 */
void *__unflatten_device_tree(const void *blob,
			      struct device_node *dad,
			      struct device_node **mynodes,
			      void *(*dt_alloc)(u64 size, u64 align),
			      bool detached,
				  int  *unfdt_size)
{
	int size;
	void *mem;

	pr_err(" -> unflatten_device_tree()\n");

	if (!blob) {
		pr_err("No device tree pointer\n");
		return NULL;
	}


	pr_err("blob size: 0x%x\n", fdt_totalsize(blob));
	pr_err("blob base: 0x%08x\n", (unsigned int)blob);
	pr_err("  magic: 0x%08x\n", fdt_magic(blob));
	pr_err("version: 0x%08x\n", fdt_version(blob));

	if (fdt_check_header(blob)) {
		pr_err("Invalid device tree blob header\n");
		return NULL;
	}

	/* First pass, scan for size */
	size = unflatten_dt_nodes(blob, NULL, dad, NULL);
	if (size < 0)
		return NULL;


	size = ALIGN(size, 4);
	pr_err("unflattened size: 0x%x\n", size);
	*unfdt_size = size;

	/* Allocate memory for the expanded device tree */
	mem = malloc(size + 4); //mem = dt_alloc(size + 4, __alignof__(struct device_node));
	if (!mem)
		return NULL;

	memset(mem, 0, size);

	*(__be32 *)(mem + size) = cpu_to_be32(0xdeadbeef);

	pr_err("unflattened base: 0x%08x\n", (unsigned int)mem);

	/* Second pass, do actual unflattening */
	unflatten_dt_nodes(blob, mem, dad, mynodes);
	if (be32_to_cpu(*(__be32 *)(mem + size)) != 0xdeadbeef)
		pr_warning("End of tree marker overwritten: %08x\n",
				be32_to_cpu(*(__be32 *)(mem + size)));

	if (detached && mynodes) {
		of_node_set_flag(*mynodes, OF_DETACHED);
		pr_err("unflattened tree is detached\n");
	}

	pr_err(" <- unflatten_device_tree()\n");
	return mem;
}


int main(int argc, char *argv[])
{
	int fd = 0;	/* assume stdin */
	char *blob_buf;
	int unfdt_size;
    size_t size_fdt;
	void *unfdt_buf;

	char *input_name;
	char *output_name;
	FILE *outf = NULL;

	if (argc < 3) {
		printf("supply input filename\n");
		return 5;
	}

	input_name = argv[1];
	output_name = argv[2];
	printf ("%s: %s\n", " input", input_name);
	printf ("%s: %s\n", "output", output_name);
	blob_buf = (char *)utilfdt_read(input_name, &size_fdt);
	unfdt_buf = __unflatten_device_tree(blob_buf, NULL, &of_root, false, true, &unfdt_size);
	free(blob_buf);
	printf ("unfdt_size %x\n", unfdt_size);
#if 1

	outf = fopen(output_name, "wb");
	if (! outf)
		printf("Couldn't open output file %s\n",    output_name);

	if (fwrite(&unfdt_buf, 4, 1, outf) != 1)
		printf("Error writing \n");

	if (fwrite(&blob_buf, 4, 1, outf) != 1)
		printf("Error writing \n");

	if (fwrite(unfdt_buf, unfdt_size, 1, outf) != 1)
		printf("Error writing \n");
#ifdef DEBUG
	show_unfdt(of_root);
#endif
	fclose(outf);
#endif

#if 0
	{
		char (*__kobject)[sizeof(struct kobject)];
		printf("__kobject %d \r\n", sizeof(struct kobject) ); //36
	}
	{
		char (*__property)[sizeof(struct property)];
		printf("__property %d \r\n", sizeof(struct property) );//target 52
	}
	{
		char (*__device_node)[sizeof(struct device_node)];
		printf("__device_node%d \r\n", sizeof(struct device_node) );//144 target 88
	}
	{
		char (*__void_p)[sizeof(void	*)];
		printf("__void_p %d \r\n", sizeof(void	*) );//144 target 88
		printf("fwnode_handle %d \r\n", sizeof(struct	fwnode_handle) );
	}
#endif
	return 0;
}
