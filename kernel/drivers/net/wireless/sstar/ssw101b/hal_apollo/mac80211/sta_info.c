/*
 * Copyright 2002-2005, Instant802 Networks, Inc.
 * Copyright 2006-2007	Jiri Benc <jbenc@suse.cz>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/skbuff.h>
#include <linux/if_arp.h>
#include <linux/timer.h>
#include <linux/rtnetlink.h>

#include <net/Sstar_mac80211.h>
#include "ieee80211_i.h"
#include "driver-ops.h"
#include "rate.h"
#include "sta_info.h"
#include "debugfs_sta.h"
#include "mesh.h"
#include "wme.h"

/**
 * DOC: STA information lifetime rules
 *
 * STA info structures (&struct sta_info) are managed in a hash table
 * for faster lookup and a list for iteration. They are managed using
 * RCU, i.e. access to the list and hash table is protected by RCU.
 *
 * Upon allocating a STA info structure with sta_info_alloc(), the caller
 * owns that structure. It must then insert it into the hash table using
 * either sta_info_insert() or sta_info_insert_rcu(); only in the latter
 * case (which acquires an rcu read section but must not be called from
 * within one) will the pointer still be valid after the call. Note that
 * the caller may not do much with the STA info before inserting it, in
 * particular, it may not start any mesh peer link management or add
 * encryption keys.
 *
 * When the insertion fails (sta_info_insert()) returns non-zero), the
 * structure will have been freed by sta_info_insert()!
 *
 * Station entries are added by mac80211 when you establish a link with a
 * peer. This means different things for the different type of interfaces
 * we support. For a regular station this mean we add the AP sta when we
 * receive an association response from the AP. For IBSS this occurs when
 * get to know about a peer on the same IBSS. For WDS we add the sta for
 * the peer immediately upon device open. When using AP mode we add stations
 * for each respective station upon request from userspace through nl80211.
 *
 * In order to remove a STA info structure, various sta_info_destroy_*()
 * calls are available.
 *
 * There is no concept of ownership on a STA entry, each structure is
 * owned by the global hash table/list until it is removed. All users of
 * the structure need to be RCU protected so that the structure won't be
 * freed before they are done using it.
 */

/* Caller must hold local->sta_lock */
static int sta_info_hash_del(struct ieee80211_local *local,
			     struct sta_info *sta)
{
	struct sta_info *s;

	s = rcu_dereference_protected(local->sta_hash[STA_HASH(sta->sta.addr)],
				      lockdep_is_held(&local->sta_lock));
	if (!s)
		return -ENOENT;
	if (s == sta) {
		rcu_assign_pointer(local->sta_hash[STA_HASH(sta->sta.addr)],
				   s->hnext);
		return 0;
	}

	while (rcu_access_pointer(s->hnext) &&
	       rcu_access_pointer(s->hnext) != sta)
		s = rcu_dereference_protected(
			s->hnext, lockdep_is_held(&local->sta_lock));
	if (rcu_access_pointer(s->hnext)) {
		rcu_assign_pointer(s->hnext, sta->hnext);
		return 0;
	}

	return -ENOENT;
}

/* protected by RCU */
struct sta_info *sta_info_get(struct ieee80211_sub_if_data *sdata,
			      const u8 *addr)
{
	struct ieee80211_local *local = sdata->local;
	struct sta_info *sta;

	sta = rcu_dereference_check(local->sta_hash[STA_HASH(addr)],
				    lockdep_is_held(&local->sta_lock) ||
					    lockdep_is_held(&local->sta_mtx));
	while (sta) {
		if (sta->sdata == sdata && !sta->dummy &&
		    memcmp(sta->sta.addr, addr, ETH_ALEN) == 0)
			break;
		sta = rcu_dereference_check(
			sta->hnext, lockdep_is_held(&local->sta_lock) ||
					    lockdep_is_held(&local->sta_mtx));
	}
	return sta;
}

/* get a station info entry even if it is a dummy station*/
struct sta_info *sta_info_get_rx(struct ieee80211_sub_if_data *sdata,
				 const u8 *addr)
{
	struct ieee80211_local *local = sdata->local;
	struct sta_info *sta;

	sta = rcu_dereference_check(local->sta_hash[STA_HASH(addr)],
				    lockdep_is_held(&local->sta_lock) ||
					    lockdep_is_held(&local->sta_mtx));
	while (sta) {
		if (sta->sdata == sdata &&
		    memcmp(sta->sta.addr, addr, ETH_ALEN) == 0)
			break;
		sta = rcu_dereference_check(
			sta->hnext, lockdep_is_held(&local->sta_lock) ||
					    lockdep_is_held(&local->sta_mtx));
	}
	return sta;
}

/*
 * Get sta info either from the specified interface
 * or from one of its vlans
 */
struct sta_info *sta_info_get_bss(struct ieee80211_sub_if_data *sdata,
				  const u8 *addr)
{
	struct ieee80211_local *local = sdata->local;
	struct sta_info *sta;

	sta = rcu_dereference_check(local->sta_hash[STA_HASH(addr)],
				    lockdep_is_held(&local->sta_lock) ||
					    lockdep_is_held(&local->sta_mtx));
	while (sta) {
		if ((sta->sdata == sdata ||
		     (sta->sdata->bss && sta->sdata->bss == sdata->bss)) &&
		    !sta->dummy && memcmp(sta->sta.addr, addr, ETH_ALEN) == 0)
			break;
		sta = rcu_dereference_check(
			sta->hnext, lockdep_is_held(&local->sta_lock) ||
					    lockdep_is_held(&local->sta_mtx));
	}
	return sta;
}

/*
 * Get sta info either from the specified interface
 * or from one of its vlans (including dummy stations)
 */
struct sta_info *sta_info_get_bss_rx(struct ieee80211_sub_if_data *sdata,
				     const u8 *addr)
{
	struct ieee80211_local *local = sdata->local;
	struct sta_info *sta;

	sta = rcu_dereference_check(local->sta_hash[STA_HASH(addr)],
				    lockdep_is_held(&local->sta_lock) ||
					    lockdep_is_held(&local->sta_mtx));
	while (sta) {
		if ((sta->sdata == sdata ||
		     (sta->sdata->bss && sta->sdata->bss == sdata->bss)) &&
		    memcmp(sta->sta.addr, addr, ETH_ALEN) == 0)
			break;
		sta = rcu_dereference_check(
			sta->hnext, lockdep_is_held(&local->sta_lock) ||
					    lockdep_is_held(&local->sta_mtx));
	}
	return sta;
}

struct sta_info *sta_info_get_by_idx(struct ieee80211_sub_if_data *sdata,
				     int idx)
{
	struct ieee80211_local *local = sdata->local;
	struct sta_info *sta;
	int i = 0;

	list_for_each_entry_rcu (sta, &local->sta_list, list) {
		if (sdata != sta->sdata)
			continue;
		if (i < idx) {
			++i;
			continue;
		}
		return sta;
	}

	return NULL;
}
static void sta_info_flush_cache_frag(struct ieee80211_sub_if_data *sdata,
				      u8 *mac)
{
	struct ieee80211_fragment_entry *entry;
	int i, idx;

	idx = sdata->fragment_next;
	for (i = 0; i < IEEE80211_FRAGMENT_MAX; i++) {
		struct ieee80211_hdr *f_hdr;

		idx--;
		if (idx < 0)
			idx = IEEE80211_FRAGMENT_MAX - 1;

		entry = &sdata->fragments[idx];

		if (Sstar_skb_queue_empty(&entry->skb_list))
			continue;

		f_hdr = (struct ieee80211_hdr *)entry->skb_list.next->data;

		/*
		 * Check ftype and addresses are equal, else check next fragment
		 */
		if (Sstar_compare_ether_addr(mac, f_hdr->addr1) != 0 &&
		    Sstar_compare_ether_addr(mac, f_hdr->addr2) != 0)
			continue;
		Sstar_printk_err("%s:[%pM] fragment flush start\n", __func__,
				 mac);
		__Sstar_skb_queue_purge(&entry->skb_list);
		Sstar_printk_err("%s:[%pM] fragment flush end\n", __func__,
				 mac);
	}
}

/**
 * __sta_info_free - internal STA free helper
 *
 * @local: pointer to the global information
 * @sta: STA info to free
 *
 * This function must undo everything done by sta_info_alloc()
 * that may happen before sta_info_insert().
 */
static void __sta_info_free(struct ieee80211_local *local, struct sta_info *sta)
{
	Sstar_skb_queue_purge(&sta->handshake_buffed);
#ifndef CONFIG_RATE_HW_CONTROL
	if (sta->rate_ctrl) {
		rate_control_free_sta(sta);
		rate_control_put(sta->rate_ctrl);
	}
#endif
#ifdef CONFIG_MAC80211_SSTAR_VERBOSE_DEBUG
	wiphy_debug(local->hw.wiphy, "Destroyed STA %pM\n", sta->sta.addr);
#endif /* CONFIG_MAC80211_SSTAR_VERBOSE_DEBUG */

	Sstar_kfree(sta);
}

#ifdef CONFIG_MAC80211_SSTAR_ROAMING_CHANGES
static void sta_free_work(struct Sstar_work_struct *wk)
{
	struct sta_info *sta = container_of(wk, struct sta_info, sta_free_wk);
	struct ieee80211_local *local = sta->local;
#ifdef CONFIG_SSTAR_MAC80211_NO_USE
	Sstar_cancel_work_sync(&sta->drv_unblock_wk);
#endif
	__sta_info_free(local, sta);
}

/**
 * sta_info_free_rcu - STA free rcu callback
 */
void sta_info_free_rcu(struct rcu_head *rcu_h)
{
	struct sta_info *sta = container_of(rcu_h, struct sta_info, rcu);
	struct ieee80211_local *local = sta->local;
	struct ieee80211_hw *hw = local_to_hw(local);
#ifndef CONFIG_RATE_HW_CONTROL
	rate_control_remove_sta_debugfs(sta);
#endif
	ieee80211_sta_debugfs_remove(sta);
	/*
	*when the phy has been suspened,the sta_free_wk
	*can not to be queued
	*/
	if (local->suspended && !local->resuming) {
		sta_free_work(&sta->sta_free_wk);
	} else {
		ieee80211_queue_work(hw, &sta->sta_free_wk);
	}
}
#endif
/* Caller must hold local->sta_lock */
static void sta_info_hash_add(struct ieee80211_local *local,
			      struct sta_info *sta)
{
	sta->hnext = local->sta_hash[STA_HASH(sta->sta.addr)];
	rcu_assign_pointer(local->sta_hash[STA_HASH(sta->sta.addr)], sta);
}
#ifdef CONFIG_SSTAR_MAC80211_NO_USE
static void sta_unblock(struct Sstar_work_struct *wk)
{
	struct sta_info *sta;

	sta = container_of(wk, struct sta_info, drv_unblock_wk);

	if (sta->dead)
		return;

	if (!test_sta_flag(sta, WLAN_STA_PS_STA))
		ieee80211_sta_ps_deliver_wakeup(sta);
	else if (test_and_clear_sta_flag(sta, WLAN_STA_PSPOLL)) {
		clear_sta_flag(sta, WLAN_STA_PS_DRIVER);

		local_bh_disable();
		ieee80211_sta_ps_deliver_poll_response(sta);
		local_bh_enable();
	} else if (test_and_clear_sta_flag(sta, WLAN_STA_UAPSD)) {
		clear_sta_flag(sta, WLAN_STA_PS_DRIVER);

		local_bh_disable();
		ieee80211_sta_ps_deliver_uapsd(sta);
		local_bh_enable();
	} else
		clear_sta_flag(sta, WLAN_STA_PS_DRIVER);
}
#endif
#ifndef CONFIG_RATE_HW_CONTROL
static int sta_prepare_rate_control(struct ieee80211_local *local,
				    struct sta_info *sta, gfp_t gfp)
{
	if (local->hw.flags & IEEE80211_HW_HAS_RATE_CONTROL)
		return 0;

	sta->rate_ctrl = rate_control_get(local->rate_ctrl);
	sta->rate_ctrl_priv =
		rate_control_alloc_sta(sta->rate_ctrl, &sta->sta, gfp);
	if (!sta->rate_ctrl_priv) {
		rate_control_put(sta->rate_ctrl);
		return -ENOMEM;
	}

	return 0;
}
#endif
struct sta_info *sta_info_alloc(struct ieee80211_sub_if_data *sdata, u8 *addr,
				gfp_t gfp)
{
	struct ieee80211_local *local = sdata->local;
	struct sta_info *sta;
	struct timespec uptime;
	int i;

	sta = Sstar_kzalloc(sizeof(*sta) + local->hw.sta_data_size, gfp);
	if (!sta)
		return NULL;

	spin_lock_init(&sta->lock);
#ifdef CONFIG_SSTAR_MAC80211_NO_USE
	SSTAR_INIT_WORK(&sta->drv_unblock_wk, sta_unblock);
#endif
#ifdef CONFIG_MAC80211_SSTAR_ROAMING_CHANGES
	SSTAR_INIT_WORK(&sta->sta_free_wk, sta_free_work);
#endif
	SSTAR_INIT_WORK(&sta->ampdu_mlme.work, ieee80211_ba_session_work);
	mutex_init(&sta->ampdu_mlme.mtx);

	memcpy(sta->sta.addr, addr, ETH_ALEN);
	sta->local = local;
	sta->sdata = sdata;
	sta->last_rx = jiffies;

	do_posix_clock_monotonic_gettime(&uptime);
	sta->last_connected = uptime.tv_sec;
	Sstar_ewma_init(&sta->avg_signal, 1024, 8);
	Sstar_ewma_init(&sta->avg_signal2, 1024, 8);
#ifndef CONFIG_RATE_HW_CONTROL
	if (sta_prepare_rate_control(local, sta, gfp)) {
		Sstar_kfree(sta);
		return NULL;
	}
#endif
	for (i = 0; i < STA_TID_NUM; i++) {
		/*
		 * timer_to_tid must be initialized with identity mapping
		 * to enable session_timer's data differentiation. See
		 * sta_rx_agg_session_timer_expired for usage.
		 */
		sta->timer_to_tid[i] = i;
	}
	for (i = 0; i < IEEE80211_NUM_ACS; i++) {
		Sstar_skb_queue_head_init(&sta->ps_tx_buf[i]);
		Sstar_skb_queue_head_init(&sta->tx_filtered[i]);
	}
	Sstar_skb_queue_head_init(&sta->handshake_buffed);
	for (i = 0; i < NUM_RX_DATA_QUEUES; i++)
		sta->last_seq_ctrl[i] = cpu_to_le16(USHRT_MAX);

#ifdef CONFIG_MAC80211_SSTAR_VERBOSE_DEBUG
	wiphy_debug(local->hw.wiphy, "Allocated STA %pM\n", sta->sta.addr);
#endif /* CONFIG_MAC80211_SSTAR_VERBOSE_DEBUG */

#ifdef CONFIG_MAC80211_SSTAR_MESH
	sta->plink_state = NL80211_PLINK_LISTEN;
	Sstar_init_timer(&sta->plink_timer);
#endif

#ifdef SSTAR_AP_SME
	ieee80211_ap_sme_sta_session_timer_init(sta);
#endif

	return sta;
}
void sta_info_set_mgmt_suit(struct sta_info *sta,
			    struct cfg80211_crypto_settings *settings)
{
	u8 i = 0;

	clear_sta_flag(sta, WLAN_STA_WPA_RSN);
	clear_sta_flag(sta, WLAN_STA_HANDSHAKE4OF4_SENDING);
	clear_sta_flag(sta, WLAN_STA_HANDSHAKE4OF4_SUCCESS);

	for (i = 0; i < settings->n_ciphers_pairwise; i++) {
		if ((settings->ciphers_pairwise[i] !=
		     WLAN_CIPHER_SUITE_WEP40) &&
		    (settings->ciphers_pairwise[i] !=
		     WLAN_CIPHER_SUITE_WEP104)) {
			set_sta_flag(sta, WLAN_STA_WPA_RSN);
			break;
		}
	}

	if (!test_sta_flag(sta, WLAN_STA_WPA_RSN)) {
		Sstar_printk_mgmt("%s:not wpa_rsn\n", __func__);
		return;
	}

	sta->mic_len = 16;
	for (i = 0; i < settings->n_akm_suites; i++) {
		u32 new_mic_len = 0;

		switch (settings->akm_suites[i]) {
		case SSTAR_WLAN_AKM_SUITE_802_1X_SUITE_B_192:
			new_mic_len = 24;
			break;
		case SSTAR_WLAN_AKM_SUITE_FILS_SHA256:
		case SSTAR_WLAN_AKM_SUITE_FILS_SHA384:
		case SSTAR_WLAN_AKM_SUITE_FT_FILS_SHA256:
		case SSTAR_WLAN_AKM_SUITE_FT_FILS_SHA384:
			new_mic_len = 0;
			break;
		default:
			new_mic_len = 16;
			break;
		}

		if (new_mic_len != sta->mic_len)
			sta->mic_len = new_mic_len;
	}
	Sstar_printk_mgmt("%s:sta->mic_len(%d)\n", __func__, sta->mic_len);
}
static int sta_info_finish_insert(struct sta_info *sta, bool async,
				  bool dummy_reinsert)
{
	struct ieee80211_local *local = sta->local;
	struct ieee80211_sub_if_data *sdata = sta->sdata;
	struct station_info sinfo;
	unsigned long flags;
	int err = 0;

	lockdep_assert_held(&local->sta_mtx);

	if (!sta->dummy || dummy_reinsert) {
		/* notify driver */
		if (sdata->vif.type == NL80211_IFTYPE_AP_VLAN)
			sdata = container_of(
				sdata->bss, struct ieee80211_sub_if_data, u.ap);
		err = drv_sta_add(local, sdata, &sta->sta);
		if (err) {
			if (!async)
				return err;
			Sstar_printk_sta("%s: failed to add IBSS STA %pM to "
					 "driver (%d) - keeping it anyway.\n",
					 sdata->name, sta->sta.addr, err);
		} else {
			sta->uploaded = true;
#ifdef CONFIG_MAC80211_SSTAR_VERBOSE_DEBUG
			if (async)
				wiphy_debug(local->hw.wiphy,
					    "Finished adding IBSS STA %pM\n",
					    sta->sta.addr);
#endif
		}

		sdata = sta->sdata;
	}

	if (!dummy_reinsert) {
		if (!async) {
			local->num_sta++;
			local->sta_generation++;
			smp_mb();

			/* make the station visible */
			spin_lock_irqsave(&local->sta_lock, flags);
			sta_info_hash_add(local, sta);
			spin_unlock_irqrestore(&local->sta_lock, flags);
		}

		list_add(&sta->list, &local->sta_list);
	} else {
		sta->dummy = false;
	}

	if (!sta->dummy) {
		ieee80211_sta_debugfs_add(sta);
#ifndef CONFIG_RATE_HW_CONTROL
		rate_control_add_sta_debugfs(sta);
#endif
		memset(&sinfo, 0, sizeof(sinfo));
		sinfo.filled = 0;
		sinfo.generation = local->sta_generation;

#ifdef SSTAR_AP_SME
		if (!((sdata->vif.type == NL80211_IFTYPE_AP) ||
		      (sdata->vif.type == NL80211_IFTYPE_P2P_GO))) {
			cfg80211_new_sta(sdata->dev, sta->sta.addr, &sinfo,
					 GFP_KERNEL);
		}
#else
		cfg80211_new_sta(sdata->dev, sta->sta.addr, &sinfo, GFP_KERNEL);
#endif
	}

	return 0;
}

static void sta_info_finish_pending(struct ieee80211_local *local)
{
	struct sta_info *sta;
	unsigned long flags;

	spin_lock_irqsave(&local->sta_lock, flags);
	while (!list_empty(&local->sta_pending_list)) {
		sta = list_first_entry(&local->sta_pending_list,
				       struct sta_info, list);
		list_del(&sta->list);
		spin_unlock_irqrestore(&local->sta_lock, flags);

		sta_info_finish_insert(sta, true, false);

		spin_lock_irqsave(&local->sta_lock, flags);
	}
	spin_unlock_irqrestore(&local->sta_lock, flags);
}

static void sta_info_finish_work(struct Sstar_work_struct *work)
{
	struct ieee80211_local *local =
		container_of(work, struct ieee80211_local, sta_finish_work);

	mutex_lock(&local->sta_mtx);
	sta_info_finish_pending(local);
	mutex_unlock(&local->sta_mtx);
}

static int sta_info_insert_check(struct sta_info *sta)
{
	struct ieee80211_sub_if_data *sdata = sta->sdata;

	/*
	 * Can't be a WARN_ON because it can be triggered through a race:
	 * something inserts a STA (on one CPU) without holding the RTNL
	 * and another CPU turns off the net device.
	 */
	if (unlikely(!ieee80211_sdata_running(sdata)))
		return -ENETDOWN;

	if (WARN_ON(Sstar_compare_ether_addr(sta->sta.addr, sdata->vif.addr) ==
			    0 ||
		    is_multicast_ether_addr(sta->sta.addr)))
		return -EINVAL;

	return 0;
}
#ifdef CONFIG_SSTAR_SUPPORT_IBSS
static int sta_info_insert_ibss(struct sta_info *sta) __acquires(RCU)
{
	struct ieee80211_local *local = sta->local;
	struct ieee80211_sub_if_data *sdata = sta->sdata;
	unsigned long flags;

	spin_lock_irqsave(&local->sta_lock, flags);
	/* check if STA exists already */
	if (sta_info_get_bss_rx(sdata, sta->sta.addr)) {
		spin_unlock_irqrestore(&local->sta_lock, flags);
		rcu_read_lock();
		return -EEXIST;
	}

	local->num_sta++;
	local->sta_generation++;
	smp_mb();
	sta_info_hash_add(local, sta);

	list_add_tail(&sta->list, &local->sta_pending_list);

	rcu_read_lock();
	spin_unlock_irqrestore(&local->sta_lock, flags);

#ifdef CONFIG_MAC80211_SSTAR_VERBOSE_DEBUG
	wiphy_debug(local->hw.wiphy, "Added IBSS STA %pM\n", sta->sta.addr);
#endif /* CONFIG_MAC80211_SSTAR_VERBOSE_DEBUG */

	ieee80211_queue_work(&local->hw, &local->sta_finish_work);

	return 0;
}
#endif
/*
 * should be called with sta_mtx locked
 * this function replaces the mutex lock
 * with a RCU lock
 */
static int sta_info_insert_non_ibss(struct sta_info *sta) __acquires(RCU)
{
	struct ieee80211_local *local = sta->local;
	struct ieee80211_sub_if_data *sdata = sta->sdata;
	unsigned long flags;
	struct sta_info *exist_sta;
	bool dummy_reinsert = false;
	int err = 0;

	lockdep_assert_held(&local->sta_mtx);

	/*
	 * On first glance, this will look racy, because the code
	 * in this function, which inserts a station with sleeping,
	 * unlocks the sta_lock between checking existence in the
	 * hash table and inserting into it.
	 *
	 * However, it is not racy against itself because it keeps
	 * the mutex locked.
	 */

	spin_lock_irqsave(&local->sta_lock, flags);
	/*
	 * check if STA exists already.
	 * only accept a scenario of a second call to sta_info_insert_non_ibss
	 * with a dummy station entry that was inserted earlier
	 * in that case - assume that the dummy station flag should
	 * be removed.
	 */
	exist_sta = sta_info_get_bss_rx(sdata, sta->sta.addr);
	if (exist_sta) {
		if (exist_sta == sta && sta->dummy) {
			dummy_reinsert = true;
		} else {
			spin_unlock_irqrestore(&local->sta_lock, flags);
			mutex_unlock(&local->sta_mtx);
			rcu_read_lock();
			return -EEXIST;
		}
	}

	spin_unlock_irqrestore(&local->sta_lock, flags);

	err = sta_info_finish_insert(sta, false, dummy_reinsert);
	if (err) {
		mutex_unlock(&local->sta_mtx);
		rcu_read_lock();
		return err;
	}

#ifdef CONFIG_MAC80211_SSTAR_VERBOSE_DEBUG
	wiphy_debug(local->hw.wiphy, "Inserted %sSTA %pM\n",
		    sta->dummy ? "dummy " : "", sta->sta.addr);
#endif /* CONFIG_MAC80211_SSTAR_VERBOSE_DEBUG */

	/* move reference to rcu-protected */
	rcu_read_lock();
	mutex_unlock(&local->sta_mtx);
#ifdef CONFIG_MAC80211_SSTAR_MESH
	if (ieee80211_vif_is_mesh(&sdata->vif))
		mesh_accept_plinks_update(sdata);
#endif

	return 0;
}

int sta_info_insert_rcu(struct sta_info *sta) __acquires(RCU)
{
	struct ieee80211_local *local = sta->local;
#ifdef CONFIG_SSTAR_SUPPORT_IBSS
	struct ieee80211_sub_if_data *sdata = sta->sdata;
#endif
	int err = 0;

	err = sta_info_insert_check(sta);
	if (err) {
		rcu_read_lock();
		goto out_free;
	}

	/*
	 * In ad-hoc mode, we sometimes need to insert stations
	 * from tasklet context from the RX path. To avoid races,
	 * always do so in that case -- see the comment below.
	 */
#ifdef CONFIG_SSTAR_SUPPORT_IBSS
	if (sdata->vif.type == NL80211_IFTYPE_ADHOC) {
		err = sta_info_insert_ibss(sta);
		if (err)
			goto out_free;

		return 0;
	}
#endif
	/*
	 * It might seem that the function called below is in race against
	 * the function call above that atomically inserts the station... That,
	 * however, is not true because the above code can only
	 * be invoked for IBSS interfaces, and the below code will
	 * not be -- and the two do not race against each other as
	 * the hash table also keys off the interface.
	 */

	might_sleep();

	mutex_lock(&local->sta_mtx);

	err = sta_info_insert_non_ibss(sta);
	if (err)
		goto out_free;

	return 0;
out_free:
	BUG_ON(!err);
	__sta_info_free(local, sta);
	return err;
}

int sta_info_insert(struct sta_info *sta)
{
	int err = sta_info_insert_rcu(sta);

	rcu_read_unlock();

	return err;
}

/* Caller must hold sta->local->sta_mtx */
int sta_info_reinsert(struct sta_info *sta)
{
	struct ieee80211_local *local = sta->local;
	int err = 0;

	err = sta_info_insert_check(sta);
	if (err) {
		mutex_unlock(&local->sta_mtx);
		return err;
	}

	might_sleep();

	err = sta_info_insert_non_ibss(sta);
	rcu_read_unlock();
	return err;
}

static inline void __bss_tim_set(struct ieee80211_if_ap *bss, u16 aid)
{
	/*
	 * This format has been mandated by the IEEE specifications,
	 * so this line may not be changed to use the __set_bit() format.
	 */
	bss->tim[aid / 8] |= (1 << (aid % 8));
}

static inline void __bss_tim_clear(struct ieee80211_if_ap *bss, u16 aid)
{
	/*
	 * This format has been mandated by the IEEE specifications,
	 * so this line may not be changed to use the __clear_bit() format.
	 */
	bss->tim[aid / 8] &= ~(1 << (aid % 8));
}

static unsigned long ieee80211_tids_for_ac(int ac)
{
	/* If we ever support TIDs > 7, this obviously needs to be adjusted */
	switch (ac) {
	case IEEE80211_AC_VO:
		return BIT(6) | BIT(7);
	case IEEE80211_AC_VI:
		return BIT(4) | BIT(5);
	case IEEE80211_AC_BE:
		return BIT(0) | BIT(3);
	case IEEE80211_AC_BK:
		return BIT(1) | BIT(2);
	default:
		WARN_ON(1);
		return 0;
	}
}

void sta_info_recalc_tim(struct sta_info *sta)
{
	struct ieee80211_local *local = sta->local;
	struct ieee80211_if_ap *bss = sta->sdata->bss;
	unsigned long flags;
	bool indicate_tim = false;
	u8 ignore_for_tim = sta->sta.uapsd_queues;
	int ac;

	if (WARN_ON_ONCE(!sta->sdata->bss))
		return;

	/* No need to do anything if the driver does all */
	if (local->hw.flags & IEEE80211_HW_AP_LINK_PS)
		return;

	if (sta->dead)
		goto done;

	/*
	 * If all ACs are delivery-enabled then we should build
	 * the TIM bit for all ACs anyway; if only some are then
	 * we ignore those and build the TIM bit using only the
	 * non-enabled ones.
	 */
	if (ignore_for_tim == BIT(IEEE80211_NUM_ACS) - 1)
		ignore_for_tim = 0;

	for (ac = 0; ac < IEEE80211_NUM_ACS; ac++) {
		unsigned long tids;

		if (ignore_for_tim & BIT(ac))
			continue;

		indicate_tim |= !Sstar_skb_queue_empty(&sta->tx_filtered[ac]) ||
				!Sstar_skb_queue_empty(&sta->ps_tx_buf[ac]);
		if (indicate_tim)
			break;

		tids = ieee80211_tids_for_ac(ac);

		indicate_tim |= sta->driver_buffered_tids & tids;
	}

done:
	spin_lock_irqsave(&local->sta_lock, flags);

	if (indicate_tim)
		__bss_tim_set(bss, sta->sta.aid);
	else
		__bss_tim_clear(bss, sta->sta.aid);

	if (local->ops->set_tim) {
		local->tim_in_locked_section = true;
		drv_set_tim(local, &sta->sta, indicate_tim);
		local->tim_in_locked_section = false;
	}

	spin_unlock_irqrestore(&local->sta_lock, flags);
}

static bool sta_info_buffer_expired(struct sta_info *sta, struct sk_buff *skb)
{
	struct ieee80211_tx_info *info;
	int timeout;

	if (!skb)
		return false;

	info = IEEE80211_SKB_CB(skb);

	/* Timeout: (2 * listen_interval * beacon_int * 1024 / 1000000) sec */
	timeout = (sta->listen_interval * sta->sdata->vif.bss_conf.beacon_int *
		   32 / 15625) *
		  HZ;
	/* Timeout limited to range 5-10sec */
	if (timeout < STA_TX_BUFFER_EXPIRE)
		timeout = STA_TX_BUFFER_EXPIRE;
	else if (timeout > (10 * HZ))
		timeout = 10 * HZ;
	return time_after(jiffies, info->control.jiffies + timeout);
}

static bool sta_info_cleanup_expire_buffered_ac(struct ieee80211_local *local,
						struct sta_info *sta, int ac)
{
	unsigned long flags;
	struct sk_buff *skb;

	/*
	 * First check for frames that should expire on the filtered
	 * queue. Frames here were rejected by the driver and are on
	 * a separate queue to avoid reordering with normal PS-buffered
	 * frames. They also aren't accounted for right now in the
	 * total_ps_buffered counter.
	 */
	for (;;) {
		spin_lock_irqsave(&sta->tx_filtered[ac].lock, flags);
		skb = Sstar_skb_peek(&sta->tx_filtered[ac]);
		if (sta_info_buffer_expired(sta, skb))
			skb = __Sstar_skb_dequeue(&sta->tx_filtered[ac]);
		else
			skb = NULL;
		spin_unlock_irqrestore(&sta->tx_filtered[ac].lock, flags);

		/*
		 * Frames are queued in order, so if this one
		 * hasn't expired yet we can stop testing. If
		 * we actually reached the end of the queue we
		 * also need to stop, of course.
		 */
		if (!skb)
			break;
		Sstar_dev_kfree_skb(skb);
	}

	/*
	 * Now also check the normal PS-buffered queue, this will
	 * only find something if the filtered queue was emptied
	 * since the filtered frames are all before the normal PS
	 * buffered frames.
	 */
	for (;;) {
		spin_lock_irqsave(&sta->ps_tx_buf[ac].lock, flags);
		skb = Sstar_skb_peek(&sta->ps_tx_buf[ac]);
		if (sta_info_buffer_expired(sta, skb))
			skb = __Sstar_skb_dequeue(&sta->ps_tx_buf[ac]);
		else
			skb = NULL;
		spin_unlock_irqrestore(&sta->ps_tx_buf[ac].lock, flags);

		/*
		 * frames are queued in order, so if this one
		 * hasn't expired yet (or we reached the end of
		 * the queue) we can stop testing
		 */
		if (!skb)
			break;

		local->total_ps_buffered--;
#ifdef CONFIG_MAC80211_SSTAR_VERBOSE_PS_DEBUG
		Sstar_printk_debug("Buffered frame expired (STA %pM)\n",
				   sta->sta.addr);
#endif
		Sstar_dev_kfree_skb(skb);
	}

	/*
	 * Finally, recalculate the TIM bit for this station -- it might
	 * now be clear because the station was too slow to retrieve its
	 * frames.
	 */
	sta_info_recalc_tim(sta);

	/*
	 * Return whether there are any frames still buffered, this is
	 * used to check whether the cleanup timer still needs to run,
	 * if there are no frames we don't need to rearm the timer.
	 */
	return !(Sstar_skb_queue_empty(&sta->ps_tx_buf[ac]) &&
		 Sstar_skb_queue_empty(&sta->tx_filtered[ac]));
}

static bool sta_info_cleanup_expire_buffered(struct ieee80211_local *local,
					     struct sta_info *sta)
{
	bool have_buffered = false;
	int ac;

	/* This is only necessary for stations on BSS interfaces */
	if (!sta->sdata->bss)
		return false;

#ifdef SSTAR_AP_SME
	if (!test_sta_flag(sta, WLAN_STA_ASSOC_AP))
		return false;
#endif

	for (ac = 0; ac < IEEE80211_NUM_ACS; ac++)
		have_buffered |=
			sta_info_cleanup_expire_buffered_ac(local, sta, ac);

	return have_buffered;
}

static int __must_check __sta_info_destroy(struct sta_info *sta)
{
	struct ieee80211_local *local;
	struct ieee80211_sub_if_data *sdata;
	unsigned long flags;
	int ret, i, ac;

	might_sleep();

	if (!sta)
		return -ENOENT;

	local = sta->local;
	sdata = sta->sdata;

/*
	 * Before removing the station from the driver and
	 * rate control, it might still start new aggregation
	 * sessions -- block that to make sure the tear-down
	 * will be sufficient.
	 */
#ifdef SSTAR_AP_SME
	Sstar_del_timer_sync(&sta->sta_session_timer);
	ieee80211_ap_sme_free_aid(sdata, sta);
	if (test_sta_flag(sta, WLAN_STA_DEAUTHENNING))
		ieee80211_ap_sme_sta_sync_unlock(sdata);

	if (test_sta_flag(sta, WLAN_STA_SHORT_PREAMBLE))
		sdata->vif.bss_conf.num_sta_no_short_preamble--;
	if (test_sta_flag(sta, WLAN_STA_SHORT_SLOT))
		sdata->vif.bss_conf.num_sta_no_short_slot_time--;
#endif
	set_sta_flag(sta, WLAN_STA_BLOCK_BA);
	ieee80211_sta_tear_down_BA_sessions(sta, true);

	spin_lock_irqsave(&local->sta_lock, flags);
	ret = sta_info_hash_del(local, sta);
	/* this might still be the pending list ... which is fine */
	if (!ret)
		list_del(&sta->list);
	spin_unlock_irqrestore(&local->sta_lock, flags);
	if (ret)
		return ret;

	mutex_lock(&local->key_mtx);
	for (i = 0; i < NUM_DEFAULT_KEYS; i++)
		__ieee80211_key_free(key_mtx_dereference(local, sta->gtk[i]));
	if (sta->ptk)
		__ieee80211_key_free(key_mtx_dereference(local, sta->ptk));
	mutex_unlock(&local->key_mtx);

	sta->dead = true;

	if (test_sta_flag(sta, WLAN_STA_PS_STA) ||
	    test_sta_flag(sta, WLAN_STA_PS_DRIVER)) {
		BUG_ON(!sdata->bss);

		clear_sta_flag(sta, WLAN_STA_PS_STA);
		clear_sta_flag(sta, WLAN_STA_PS_DRIVER);

		atomic_dec(&sdata->bss->num_sta_ps);
		sta_info_recalc_tim(sta);
	}
	if (sdata->vif.type == NL80211_IFTYPE_AP) {
		/*
		*no function to cancle the sta package in the lower driver,so
		*call this function.
		*/
		synchronize_rcu();
		drv_flush(local, sdata, false);
	}
	local->num_sta--;
	local->sta_generation++;

	if (sdata->vif.type == NL80211_IFTYPE_AP_VLAN)
		RCU_INIT_POINTER(sdata->u.vlan.sta, NULL);

	if (sta->uploaded) {
		if (sdata->vif.type == NL80211_IFTYPE_AP_VLAN)
			sdata = container_of(
				sdata->bss, struct ieee80211_sub_if_data, u.ap);
		drv_sta_remove(local, sdata, &sta->sta);
		sdata = sta->sdata;
	}

	sta_info_flush_cache_frag(sdata, sta->sta.addr);
#ifdef CONFIG_MAC80211_SSTAR_ROAMING_CHANGES
	/*
	 * In STA mode use non blocking rcu to decrease
	 * roaming time.
	 */
	if (sdata->vif.type == NL80211_IFTYPE_STATION) {
		cfg80211_del_sta(sdata->dev, sta->sta.addr, GFP_KERNEL);

#ifdef CONFIG_MAC80211_SSTAR_VERBOSE_DEBUG
		wiphy_debug(local->hw.wiphy, "Removed STA %pM\n",
			    sta->sta.addr);
#endif /* CONFIG_MAC80211_SSTAR_VERBOSE_DEBUG */

		call_rcu(&sta->rcu, sta_info_free_rcu);
		return 0;
	}
#endif

	/*
	 * At this point, after we wait for an RCU grace period,
	 * neither mac80211 nor the driver can reference this
	 * sta struct any more except by still existing timers
	 * associated with this station that we clean up below.
	 */
	synchronize_rcu();

	for (ac = 0; ac < IEEE80211_NUM_ACS; ac++) {
		local->total_ps_buffered -=
			Sstar_skb_queue_len(&sta->ps_tx_buf[ac]);
		__Sstar_skb_queue_purge(&sta->ps_tx_buf[ac]);
		__Sstar_skb_queue_purge(&sta->tx_filtered[ac]);
	}

#ifdef CONFIG_MAC80211_SSTAR_MESH
	if (ieee80211_vif_is_mesh(&sdata->vif))
		mesh_accept_plinks_update(sdata);
#endif

#ifdef CONFIG_MAC80211_SSTAR_VERBOSE_DEBUG
	wiphy_debug(local->hw.wiphy, "Removed STA %pM\n", sta->sta.addr);
#endif /* CONFIG_MAC80211_SSTAR_VERBOSE_DEBUG */
#ifdef CONFIG_SSTAR_MAC80211_NO_USE
	Sstar_cancel_work_sync(&sta->drv_unblock_wk);
#endif

	cfg80211_del_sta(sdata->dev, sta->sta.addr, GFP_KERNEL);
#ifndef CONFIG_RATE_HW_CONTROL
	rate_control_remove_sta_debugfs(sta);
#endif
	ieee80211_sta_debugfs_remove(sta);

#ifdef CONFIG_MAC80211_SSTAR_MESH
	if (ieee80211_vif_is_mesh(&sta->sdata->vif)) {
		mesh_plink_deactivate(sta);
		Sstar_del_timer_sync(&sta->plink_timer);
	}
#endif
#ifdef SSTAR_AP_SME
	if (sta->associate_ie) {
		Sstar_kfree(sta->associate_ie);
		sta->associate_ie = NULL;
	}
	if (sta->challenge) {
		Sstar_kfree(sta->challenge);
		sta->challenge = NULL;
	}
#endif
	__sta_info_free(local, sta);

	return 0;
}

int sta_info_destroy_addr(struct ieee80211_sub_if_data *sdata, const u8 *addr)
{
	struct sta_info *sta;
	int ret;

	mutex_lock(&sdata->local->sta_mtx);
	sta = sta_info_get_rx(sdata, addr);
	ret = __sta_info_destroy(sta);
	mutex_unlock(&sdata->local->sta_mtx);

	return ret;
}

int sta_info_destroy_addr_bss(struct ieee80211_sub_if_data *sdata,
			      const u8 *addr)
{
	struct sta_info *sta;
	int ret;

	mutex_lock(&sdata->local->sta_mtx);
	sta = sta_info_get_bss_rx(sdata, addr);
	ret = __sta_info_destroy(sta);
	mutex_unlock(&sdata->local->sta_mtx);

	return ret;
}

static void sta_info_cleanup(unsigned long data)
{
	struct ieee80211_local *local = (struct ieee80211_local *)data;
	struct sta_info *sta;
	bool timer_needed = false;

	rcu_read_lock();
	list_for_each_entry_rcu (sta, &local->sta_list, list)
		if (sta_info_cleanup_expire_buffered(local, sta))
			timer_needed = true;
	rcu_read_unlock();

	if (local->quiescing)
		return;

	if (!timer_needed)
		return;

	Sstar_mod_timer(&local->sta_cleanup,
			round_jiffies(jiffies + STA_INFO_CLEANUP_INTERVAL));
}

void sta_info_init(struct ieee80211_local *local)
{
	spin_lock_init(&local->sta_lock);
	mutex_init(&local->sta_mtx);
	INIT_LIST_HEAD(&local->sta_list);
	INIT_LIST_HEAD(&local->sta_pending_list);
	SSTAR_INIT_WORK(&local->sta_finish_work, sta_info_finish_work);

	Sstar_setup_timer(&local->sta_cleanup, sta_info_cleanup,
			  (unsigned long)local);
}

void sta_info_stop(struct ieee80211_local *local)
{
	Sstar_del_timer(&local->sta_cleanup);
	sta_info_flush(local, NULL);
}

/**
 * sta_info_flush - flush matching STA entries from the STA table
 *
 * Returns the number of removed STA entries.
 *
 * @local: local interface data
 * @sdata: matching rule for the net device (sta->dev) or %NULL to match all STAs
 */
int sta_info_flush(struct ieee80211_local *local,
		   struct ieee80211_sub_if_data *sdata)
{
	struct sta_info *sta, *tmp;
	int ret = 0;

	might_sleep();

	mutex_lock(&local->sta_mtx);

	sta_info_finish_pending(local);

	list_for_each_entry_safe (sta, tmp, &local->sta_list, list) {
		if (!sdata || sdata == sta->sdata)
			WARN_ON(__sta_info_destroy(sta));
	}
	mutex_unlock(&local->sta_mtx);

	return ret;
}

void ieee80211_sta_expire(struct ieee80211_sub_if_data *sdata,
			  unsigned long exp_time)
{
	struct ieee80211_local *local = sdata->local;
	struct sta_info *sta, *tmp;

	mutex_lock(&local->sta_mtx);
	list_for_each_entry_safe (sta, tmp, &local->sta_list, list)
		if (time_after(jiffies, sta->last_rx + exp_time)) {
#ifdef CONFIG_MAC80211_SSTAR_IBSS_DEBUG
			Sstar_printk_debug("%s: expiring inactive STA %pM\n",
					   sdata->name, sta->sta.addr);
#endif
			WARN_ON(__sta_info_destroy(sta));
		}
	mutex_unlock(&local->sta_mtx);
}
struct ieee80211_sta *ieee80211_find_sta(struct ieee80211_vif *vif,
					 const u8 *addr)
{
	struct sta_info *sta;

	if (!vif)
		return NULL;

	sta = sta_info_get_bss(vif_to_sdata(vif), addr);
	if (!sta)
		return NULL;

	if (!sta->uploaded)
		return NULL;

	return &sta->sta;
}
//EXPORT_SYMBOL(ieee80211_find_sta);

static void clear_sta_ps_flags(void *_sta)
{
	struct sta_info *sta = _sta;

	clear_sta_flag(sta, WLAN_STA_PS_DRIVER);
	clear_sta_flag(sta, WLAN_STA_PS_STA);
}

/* powersave support code */
void ieee80211_sta_ps_deliver_wakeup(struct sta_info *sta)
{
	struct ieee80211_sub_if_data *sdata = sta->sdata;
	struct ieee80211_local *local = sdata->local;
	struct sk_buff_head pending;
	int filtered = 0, buffered = 0, ac;

	clear_sta_flag(sta, WLAN_STA_SP);

	BUILD_BUG_ON(BITS_TO_LONGS(STA_TID_NUM) > 1);
	sta->driver_buffered_tids = 0;

	if (!(local->hw.flags & IEEE80211_HW_AP_LINK_PS))
		drv_sta_notify(local, sdata, STA_NOTIFY_AWAKE, &sta->sta);

	Sstar_skb_queue_head_init(&pending);

	/* Send all buffered frames to the station */
	for (ac = 0; ac < IEEE80211_NUM_ACS; ac++) {
		int count = Sstar_skb_queue_len(&pending), tmp;

		Sstar_skb_queue_splice_tail_init(&sta->tx_filtered[ac],
						 &pending);
		tmp = Sstar_skb_queue_len(&pending);
		filtered += tmp - count;
		count = tmp;

		Sstar_skb_queue_splice_tail_init(&sta->ps_tx_buf[ac], &pending);
		tmp = Sstar_skb_queue_len(&pending);
		buffered += tmp - count;
	}

	ieee80211_add_pending_skbs_fn(local, &pending, clear_sta_ps_flags, sta);

	local->total_ps_buffered -= buffered;

	sta_info_recalc_tim(sta);

#ifdef CONFIG_MAC80211_SSTAR_VERBOSE_PS_DEBUG
	Sstar_printk_debug(
		"%s: STA %pM aid %d sending %d filtered/%d PS frames "
		"since STA not sleeping anymore\n",
		sdata->name, sta->sta.addr, sta->sta.aid, filtered, buffered);
#endif /* CONFIG_MAC80211_SSTAR_VERBOSE_PS_DEBUG */
}

static void
ieee80211_send_null_response(struct ieee80211_sub_if_data *sdata,
			     struct sta_info *sta, int tid,
			     enum ieee80211_frame_release_type reason)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_qos_hdr *nullfunc;
	struct sk_buff *skb;
	int size = sizeof(*nullfunc);
	__le16 fc;
	bool qos = test_sta_flag(sta, WLAN_STA_WME);
	struct ieee80211_tx_info *info;

	if (qos) {
		fc = cpu_to_le16(IEEE80211_FTYPE_DATA |
				 IEEE80211_STYPE_QOS_NULLFUNC |
				 IEEE80211_FCTL_FROMDS);
	} else {
		size -= 2;
		fc = cpu_to_le16(IEEE80211_FTYPE_DATA |
				 IEEE80211_STYPE_NULLFUNC |
				 IEEE80211_FCTL_FROMDS);
	}

	skb = Sstar_dev_alloc_skb(local->hw.extra_tx_headroom + size);
	if (!skb)
		return;

	Sstar_skb_reserve(skb, local->hw.extra_tx_headroom);

	nullfunc = (void *)Sstar_skb_put(skb, size);
	nullfunc->frame_control = fc;
	nullfunc->duration_id = 0;
	memcpy(nullfunc->addr1, sta->sta.addr, ETH_ALEN);
	memcpy(nullfunc->addr2, sdata->vif.addr, ETH_ALEN);
	memcpy(nullfunc->addr3, sdata->vif.addr, ETH_ALEN);

	skb->priority = tid;
	Sstar_skb_set_queue_mapping(skb, ieee802_1d_to_ac[tid]);
	if (qos) {
		nullfunc->qos_ctrl = cpu_to_le16(tid);

		if (reason == IEEE80211_FRAME_RELEASE_UAPSD)
			nullfunc->qos_ctrl |=
				cpu_to_le16(IEEE80211_QOS_CTL_EOSP);
	}

	info = IEEE80211_SKB_CB(skb);

	/*
	 * Tell TX path to send this frame even though the
	 * STA may still remain is PS mode after this frame
	 * exchange. Also set EOSP to indicate this packet
	 * ends the poll/service period.
	 */
	info->flags |= IEEE80211_TX_CTL_POLL_RESPONSE |
		       IEEE80211_TX_STATUS_EOSP |
		       IEEE80211_TX_CTL_REQ_TX_STATUS;

	drv_allow_buffered_frames(local, sta, BIT(tid), 1, reason, false);

	ieee80211_xmit(sdata, skb);
}

static void
ieee80211_sta_ps_deliver_response(struct sta_info *sta, int n_frames,
				  u8 ignored_acs,
				  enum ieee80211_frame_release_type reason)
{
	struct ieee80211_sub_if_data *sdata = sta->sdata;
	struct ieee80211_local *local = sdata->local;
	bool found = false;
	bool more_data = false;
	int ac;
	unsigned long driver_release_tids = 0;
	struct sk_buff_head frames;

	/* Service or PS-Poll period starts */
	set_sta_flag(sta, WLAN_STA_SP);

	__Sstar_skb_queue_head_init(&frames);

	/*
	 * Get response frame(s) and more data bit for it.
	 */
	for (ac = 0; ac < IEEE80211_NUM_ACS; ac++) {
		unsigned long tids;

		if (ignored_acs & BIT(ac))
			continue;

		tids = ieee80211_tids_for_ac(ac);

		if (!found) {
			driver_release_tids = sta->driver_buffered_tids & tids;
			if (driver_release_tids) {
				found = true;
			} else {
				struct sk_buff *skb;

				while (n_frames > 0) {
					skb = Sstar_skb_dequeue(
						&sta->tx_filtered[ac]);
					if (!skb) {
						skb = Sstar_skb_dequeue(
							&sta->ps_tx_buf[ac]);
						if (skb)
							local->total_ps_buffered--;
					}
					if (!skb)
						break;
					n_frames--;
					found = true;
					__Sstar_skb_queue_tail(&frames, skb);
				}
			}

			/*
			 * If the driver has data on more than one TID then
			 * certainly there's more data if we release just a
			 * single frame now (from a single TID).
			 */
			if (reason == IEEE80211_FRAME_RELEASE_PSPOLL &&
			    hweight16(driver_release_tids) > 1) {
				more_data = true;
				driver_release_tids =
					BIT(ffs(driver_release_tids) - 1);
				break;
			}
		}

		if (!Sstar_skb_queue_empty(&sta->tx_filtered[ac]) ||
		    !Sstar_skb_queue_empty(&sta->ps_tx_buf[ac])) {
			more_data = true;
			break;
		}
	}

	if (!found) {
		int tid;

		/*
		 * For PS-Poll, this can only happen due to a race condition
		 * when we set the TIM bit and the station notices it, but
		 * before it can poll for the frame we expire it.
		 *
		 * For uAPSD, this is said in the standard (11.2.1.5 h):
		 *	At each unscheduled SP for a non-AP STA, the AP shall
		 *	attempt to transmit at least one MSDU or MMPDU, but no
		 *	more than the value specified in the Max SP Length field
		 *	in the QoS Capability element from delivery-enabled ACs,
		 *	that are destined for the non-AP STA.
		 *
		 * Since we have no other MSDU/MMPDU, transmit a QoS null frame.
		 */

		/* This will evaluate to 1, 3, 5 or 7. */
		tid = 7 - ((ffs(~ignored_acs) - 1) << 1);

		ieee80211_send_null_response(sdata, sta, tid, reason);
		return;
	}

	if (!driver_release_tids || !local->ops->release_buffered_frames) {
		struct sk_buff_head pending;
		struct sk_buff *skb;
		int num = 0;
		u16 tids = 0;

		Sstar_skb_queue_head_init(&pending);

		while ((skb = __Sstar_skb_dequeue(&frames))) {
			struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
			struct ieee80211_hdr *hdr = (void *)skb->data;
			u8 *qoshdr = NULL;

			num++;

			/*
			 * Tell TX path to send this frame even though the
			 * STA may still remain is PS mode after this frame
			 * exchange.
			 */
			info->flags |= IEEE80211_TX_CTL_POLL_RESPONSE;

			/*
			 * Use MoreData flag to indicate whether there are
			 * more buffered frames for this STA
			 */
			if (more_data || !Sstar_skb_queue_empty(&frames))
				hdr->frame_control |=
					cpu_to_le16(IEEE80211_FCTL_MOREDATA);
			else
				hdr->frame_control &=
					cpu_to_le16(~IEEE80211_FCTL_MOREDATA);

			if (ieee80211_is_data_qos(hdr->frame_control) ||
			    ieee80211_is_qos_nullfunc(hdr->frame_control))
				qoshdr = ieee80211_get_qos_ctl(hdr);

			/* set EOSP for the frame */
			if (reason == IEEE80211_FRAME_RELEASE_UAPSD && qoshdr &&
			    Sstar_skb_queue_empty(&frames))
				*qoshdr |= IEEE80211_QOS_CTL_EOSP;

			info->flags |= IEEE80211_TX_STATUS_EOSP |
				       IEEE80211_TX_CTL_REQ_TX_STATUS;

			if (qoshdr)
				tids |= BIT(*qoshdr &
					    IEEE80211_QOS_CTL_TID_MASK);
			else
				tids |= BIT(0);

			__Sstar_skb_queue_tail(&pending, skb);
		}

		drv_allow_buffered_frames(local, sta, tids, num, reason,
					  more_data);

		ieee80211_add_pending_skbs(local, &pending);

		sta_info_recalc_tim(sta);
	} else {
		/*
		 * We need to release a frame that is buffered somewhere in the
		 * driver ... it'll have to handle that.
		 * Note that, as per the comment above, it'll also have to see
		 * if there is more than just one frame on the specific TID that
		 * we're releasing from, and it needs to set the more-data bit
		 * accordingly if we tell it that there's no more data. If we do
		 * tell it there's more data, then of course the more-data bit
		 * needs to be set anyway.
		 */
		drv_release_buffered_frames(local, sta, driver_release_tids,
					    n_frames, reason, more_data);

		/*
		 * Note that we don't recalculate the TIM bit here as it would
		 * most likely have no effect at all unless the driver told us
		 * that the TID became empty before returning here from the
		 * release function.
		 * Either way, however, when the driver tells us that the TID
		 * became empty we'll do the TIM recalculation.
		 */
	}
}

void ieee80211_sta_ps_deliver_poll_response(struct sta_info *sta)
{
	u8 ignore_for_response = sta->sta.uapsd_queues;

	/*
	 * If all ACs are delivery-enabled then we should reply
	 * from any of them, if only some are enabled we reply
	 * only from the non-enabled ones.
	 */
	if (ignore_for_response == BIT(IEEE80211_NUM_ACS) - 1)
		ignore_for_response = 0;

	ieee80211_sta_ps_deliver_response(sta, 1, ignore_for_response,
					  IEEE80211_FRAME_RELEASE_PSPOLL);
}

void ieee80211_sta_ps_deliver_uapsd(struct sta_info *sta)
{
	int n_frames = sta->sta.max_sp;
	u8 delivery_enabled = sta->sta.uapsd_queues;

	/*
	 * If we ever grow support for TSPEC this might happen if
	 * the TSPEC update from hostapd comes in between a trigger
	 * frame setting WLAN_STA_UAPSD in the RX path and this
	 * actually getting called.
	 */
	if (!delivery_enabled)
		return;

	switch (sta->sta.max_sp) {
	case 1:
		n_frames = 2;
		break;
	case 2:
		n_frames = 4;
		break;
	case 3:
		n_frames = 6;
		break;
	case 0:
		/* XXX: what is a good value? */
		n_frames = 8;
		break;
	}

	ieee80211_sta_ps_deliver_response(sta, n_frames, ~delivery_enabled,
					  IEEE80211_FRAME_RELEASE_UAPSD);
}
#ifdef CONFIG_SSTAR_MAC80211_NO_USE
void ieee80211_sta_block_awake(struct ieee80211_hw *hw,
			       struct ieee80211_sta *pubsta, bool block)
{
	struct sta_info *sta = container_of(pubsta, struct sta_info, sta);

	trace_api_sta_block_awake(sta->local, pubsta, block);

	if (block)
		set_sta_flag(sta, WLAN_STA_PS_DRIVER);
	else if (test_sta_flag(sta, WLAN_STA_PS_DRIVER))
		ieee80211_queue_work(hw, &sta->drv_unblock_wk);
}
//EXPORT_SYMBOL(ieee80211_sta_block_awake);

void ieee80211_sta_eosp_irqsafe(struct ieee80211_sta *pubsta)
{
	struct sta_info *sta = container_of(pubsta, struct sta_info, sta);
	struct ieee80211_local *local = sta->local;
	struct sk_buff *skb;
	struct skb_eosp_msg_data *data;

	trace_api_eosp(local, pubsta);

	skb = Sstar_alloc_skb(0, GFP_ATOMIC);
	if (!skb) {
		/* too bad ... but race is better than loss */
		clear_sta_flag(sta, WLAN_STA_SP);
		return;
	}

	data = (void *)skb->cb;
	memcpy(data->sta, pubsta->addr, ETH_ALEN);
	memcpy(data->iface, sta->sdata->vif.addr, ETH_ALEN);
	skb->pkt_type = IEEE80211_EOSP_MSG;
	Sstar_skb_queue_tail(&local->skb_queue, skb);
	tasklet_schedule(&local->tasklet);
}
//EXPORT_SYMBOL(ieee80211_sta_eosp_irqsafe);
struct ieee80211_sta *ieee80211_find_sta_by_ifaddr(struct ieee80211_hw *hw,
						   const u8 *addr,
						   const u8 *localaddr)
{
	struct sta_info *sta, *nxt;

	/*
	 * Just return a random station if localaddr is NULL
	 * ... first in list.
	 */
	for_each_sta_info(hw_to_local(hw), addr, sta, nxt)
	{
		if (localaddr && Sstar_compare_ether_addr(sta->sdata->vif.addr,
							  localaddr) != 0)
			continue;
		if (!sta->uploaded)
			return NULL;
		return &sta->sta;
	}

	return NULL;
}
//EXPORT_SYMBOL_GPL(ieee80211_find_sta_by_ifaddr);
#endif
void ieee80211_sta_set_buffered(struct ieee80211_sta *pubsta, u8 tid,
				bool buffered)
{
	struct sta_info *sta = container_of(pubsta, struct sta_info, sta);

	if (WARN_ON(tid >= STA_TID_NUM))
		return;

	if (buffered)
		set_bit(tid, &sta->driver_buffered_tids);
	else
		clear_bit(tid, &sta->driver_buffered_tids);
#ifndef CONFIG_TX_NO_CONFIRM
#pragma message("set buffered recalc time")
	sta_info_recalc_tim(sta);
#else
#pragma message("set buffered not recalc time")
#endif
}
//EXPORT_SYMBOL(ieee80211_sta_set_buffered);
