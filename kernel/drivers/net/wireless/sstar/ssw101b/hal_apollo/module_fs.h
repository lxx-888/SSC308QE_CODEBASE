#ifndef __MODULE_FS_H__
#define __MODULE_FS_H__
#include <linux/hash.h>
#include "mac80211/ieee80211_i.h"
#ifdef CONFIG_SSTAR_MOULE_FS
extern int Sstar_module_attribute_init(void);
extern void Sstar_module_attribute_exit(void);
struct Sstar_module_scan_node {
	struct ieee80211_internal_scan_sta sta;
	struct hlist_node hnode;
};
#else
#define Sstar_module_attribute_init()
#define Sstar_module_attribute_exit()
#endif
#endif
