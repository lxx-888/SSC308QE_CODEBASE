/*
 * Copyright 2002-2005, Instant802 Networks, Inc.
 * Copyright 2005-2006, Devicescape Software, Inc.
 * Copyright 2006-2007	Jiri Benc <jbenc@suse.cz>
 * Copyright 2007	Johannes Berg <johannes@sipsolutions.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * utilities for mac80211
 */

#include <net/Sstar_mac80211.h>
#include <linux/netdevice.h>
#include <linux/export.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/if_arp.h>
#include <linux/bitmap.h>
#include <linux/crc32.h>
#include <net/net_namespace.h>
#include <net/cfg80211.h>
#include <net/rtnetlink.h>

#include "ieee80211_i.h"
#include "driver-ops.h"
#include "rate.h"
#include "wme.h"
#include "Sstar_common.h"
enum {
	IEEE80211_SPECIAL_RX_PACKAGE_MSG = 1,
	IEEE80211_SPECIAL_FRAME_FILTER_REGISTER_MSG = 2,
	IEEE80211_SPECIAL_FRAME_FILTER_CLEAE_MSG = 3,
	IEEE80211_SPECIAL_FRAME_FILTER_REQUEST_MSG = 4,
	IEEE80211_SPECIAL_MAX_MSG,
};
struct ieee80211_special_filter_request {
	struct ieee80211_special_filter_table *tables;
};
union ieee80211_special_filter_cb {
	struct ieee80211_special_filter_request request;
};
extern int
Sstar_internal_recv_6441_vendor_ie(struct Sstar_vendor_cfg_ie *recv_ie);
extern int ieee80211_send_L2_2_hight_layer(struct sk_buff *skb,
					   struct net_device *dev);

static void
ieee80211_special_filter_rx_package_handle(struct ieee80211_sub_if_data *sdata,
					   struct sk_buff *skb)
{
	struct Sstar_ieee80211_mgmt *mgmt =
		(struct Sstar_ieee80211_mgmt *)skb->data;
	u8 *elements;
	int baselen;
	struct ieee802_Sstar_11_elems elems;
	struct ieee80211_rx_status *rx_status = IEEE80211_SKB_RXCB(skb);
	int freq;
	char ssid[32] = { 0 };
	const u8 *ie = NULL;
	if (ieee80211_is_beacon(mgmt->frame_control)) {
		//ie = Sstar_ieee80211_find_ie(SSTAR_WLAN_EID_SSID,mgmt->u.beacon.variable,
		//		                   skb->len-offsetof(struct Sstar_ieee80211_mgmt, u.beacon.variable));
		baselen = offsetof(struct Sstar_ieee80211_mgmt,
				   u.beacon.variable);
		if (baselen > skb->len) {
			Sstar_printk_err("[beacon] error ! \n");
		}
		elements = mgmt->u.beacon.variable;
		ieee802_11_parse_elems(elements, skb->len - baselen, &elems);
		if (elems.ds_params && elems.ds_params_len == 1)
			freq = ieee80211_channel_to_frequency(
				elems.ds_params[0], rx_status->band);
		else
			freq = rx_status->freq;

		memcpy(ssid, elems.ssid, elems.ssid_len);
		freq = (freq - 2407) / 5;

		ie = Sstar_ieee80211_find_ie(
			SSTAR_WLAN_EID_PRIVATE, mgmt->u.beacon.variable,
			skb->len - offsetof(struct Sstar_ieee80211_mgmt,
					    u.beacon.variable));

		if (ie) {
			char special_data[255] = { 0 };
			memcpy(special_data, ie + 2, ie[1]);
			Sstar_printk_err(
				"[beacon] from [%pM] channel[%d] ssid[%s] ie[%d][%d][%s]\n",
				mgmt->bssid, freq, ssid, ie[0], ie[1],
				special_data);
		} else {
			Sstar_printk_err(
				"[beacon] from [%pM] channel[%d] ssid[%s] \n",
				mgmt->bssid, freq, ssid);
		}

	} else if (ieee80211_is_probe_req(mgmt->frame_control)) {
		struct Sstar_vendor_cfg_ie *private_ie;
		u8 OUI[4];
		private_ie =
			(struct Sstar_vendor_cfg_ie *)Sstar_ieee80211_find_ie(
				221, mgmt->u.probe_req.variable,
				skb->len - offsetof(struct Sstar_ieee80211_mgmt,
						    u.probe_req.variable));

		if (private_ie) {
			OUI[0] = (SSTAR_6441_PRIVATE_OUI >> 24) & 0xFF;
			OUI[1] = (SSTAR_6441_PRIVATE_OUI >> 16) & 0xFF;
			OUI[2] = (SSTAR_6441_PRIVATE_OUI >> 8) & 0xFF;
			OUI[3] = SSTAR_6441_PRIVATE_OUI & 0xFF;
			if (memcmp(private_ie->OUI, OUI, 4) == 0) {
				Sstar_printk_err(
					"recv from ssid[%s],psk[%s]  \n",
					private_ie->ssid, private_ie->password);
				Sstar_internal_recv_6441_vendor_ie(private_ie);
				/* send data to up layer*/
				ieee80211_send_L2_2_hight_layer(skb,
								sdata->dev);
			}
		} else {
			Sstar_printk_err("[probe_req]  \n");
		}
	} else if (ieee80211_is_action(mgmt->frame_control)) {
		Sstar_printk_err("[action][%x] from [%pM]\n",
				 mgmt->frame_control, mgmt->sa);
	} else {
		Sstar_printk_err("[others][%x] from [%pM]\n",
				 mgmt->frame_control, mgmt->sa);
	}
	Sstar_dev_kfree_skb(skb);
}

static void
ieee80211_special_filter_register_handle(struct ieee80211_sub_if_data *sdata,
					 struct sk_buff *skb)
{
	struct ieee80211_special_filter_list *filter_list = NULL;
	struct ieee80211_special_filter_list *filter_temp = NULL;
	struct ieee80211_special_filter *filter_table = NULL;

	int n_filters = 0;

	filter_list = Sstar_kzalloc(
		sizeof(struct ieee80211_special_filter_list), GFP_KERNEL);

	if (filter_list == NULL) {
		goto exit;
	}

	memcpy(&filter_list->filter, skb->data,
	       sizeof(struct ieee80211_special_filter));
	list_add_tail(&filter_list->list, &sdata->filter_list);

	filter_table = Sstar_kzalloc(
		sizeof(struct ieee80211_special_filter) * 16, GFP_KERNEL);

	if (filter_table == NULL)
		goto exit;

	list_for_each_entry (filter_temp, &sdata->filter_list, list) {
		if (n_filters >= 16)
			break;
		Sstar_printk_debug("%s:flags[%x],action[%d],oui[%d:%d:%d]\n",
				   __func__, filter_temp->filter.flags,
				   filter_temp->filter.filter_action,
				   filter_temp->filter.oui[0],
				   filter_temp->filter.oui[1],
				   filter_temp->filter.oui[2]);
		memcpy(&filter_table[n_filters], &filter_temp->filter,
		       sizeof(struct ieee80211_special_filter));
		n_filters++;
	}

	if (n_filters && sdata->local->ops->set_frame_filter)
		sdata->local->ops->set_frame_filter(&sdata->local->hw,
						    &sdata->vif, n_filters,
						    filter_table, true);

exit:
	if (filter_table)
		Sstar_kfree(filter_table);
	Sstar_dev_kfree_skb(skb);
}

static void
ieee80211_special_filter_clear_handle(struct ieee80211_sub_if_data *sdata,
				      struct sk_buff *skb)
{
	struct ieee80211_special_filter_list *filter_temp = NULL;

	while (!list_empty(&sdata->filter_list)) {
		filter_temp =
			list_first_entry(&sdata->filter_list,
					 struct ieee80211_special_filter_list,
					 list);
		Sstar_printk_debug("%s:action(%d),oui[%d:%d:%d]\n", __func__,
				   filter_temp->filter.filter_action,
				   filter_temp->filter.oui[0],
				   filter_temp->filter.oui[1],
				   filter_temp->filter.oui[2]);
		list_del(&filter_temp->list);
		Sstar_kfree(filter_temp);
	}

	if (sdata->local->ops->set_frame_filter)
		sdata->local->ops->set_frame_filter(
			&sdata->local->hw, &sdata->vif, 0, NULL, false);

	if (skb)
		Sstar_dev_kfree_skb(skb);
}

static void
ieee80211_special_filter_request_handle(struct ieee80211_sub_if_data *sdata,
					struct sk_buff *skb)
{
	union ieee80211_special_filter_cb *request =
		(union ieee80211_special_filter_cb *)skb->cb;
	struct ieee80211_special_filter_table *tables = request->request.tables;
	struct ieee80211_special_filter_list *filter_temp = NULL;
	int n_filters = 0;

	if (tables == NULL) {
		return;
	}

	list_for_each_entry (filter_temp, &sdata->filter_list, list) {
		if (n_filters >= 16)
			break;
		Sstar_printk_debug("%s:flags[%x],action[%d],oui[%d:%d:%d]\n",
				   __func__, filter_temp->filter.flags,
				   filter_temp->filter.filter_action,
				   filter_temp->filter.oui[0],
				   filter_temp->filter.oui[1],
				   filter_temp->filter.oui[2]);
		memcpy(&tables->table[n_filters], &filter_temp->filter,
		       sizeof(struct ieee80211_special_filter));
		n_filters++;
	}
	tables->n_filters = n_filters;

	Sstar_dev_kfree_skb(skb);
}
static void ieee80211_special_filter_work(struct Sstar_work_struct *work)
{
	struct ieee80211_sub_if_data *sdata = container_of(
		work, struct ieee80211_sub_if_data, special_filter_work);
	struct sk_buff *skb;
	struct sk_buff_head local_list;
	unsigned long flags;

	if (!ieee80211_sdata_running(sdata))
		return;

	__Sstar_skb_queue_head_init(&local_list);

	spin_lock_irqsave(&sdata->special_filter_skb_queue.lock, flags);
	sdata->special_running = true;
restart:
	Sstar_skb_queue_splice_tail_init(&sdata->special_filter_skb_queue,
					 &local_list);
	spin_unlock_irqrestore(&sdata->special_filter_skb_queue.lock, flags);

	while ((skb = __Sstar_skb_dequeue(&local_list)) != NULL) {
		switch (skb->pkt_type) {
		case IEEE80211_SPECIAL_RX_PACKAGE_MSG:
			skb->pkt_type = 0;
			ieee80211_special_filter_rx_package_handle(sdata, skb);
			break;
		case IEEE80211_SPECIAL_FRAME_FILTER_REGISTER_MSG:
			skb->pkt_type = 0;
			ieee80211_special_filter_register_handle(sdata, skb);
			break;
		case IEEE80211_SPECIAL_FRAME_FILTER_CLEAE_MSG:
			skb->pkt_type = 0;
			ieee80211_special_filter_clear_handle(sdata, skb);
			break;
		case IEEE80211_SPECIAL_FRAME_FILTER_REQUEST_MSG:
			skb->pkt_type = 0;
			ieee80211_special_filter_request_handle(sdata, skb);
			break;
		case IEEE80211_SPECIAL_MAX_MSG:
		default:
			WARN(1, "unknown type %d\n", skb->pkt_type);
			Sstar_dev_kfree_skb(skb);
			break;
		}
	}

	spin_lock_irqsave(&sdata->special_filter_skb_queue.lock, flags);
	if (!Sstar_skb_queue_empty(&sdata->special_filter_skb_queue))
		goto restart;
	sdata->special_running = false;
	spin_unlock_irqrestore(&sdata->special_filter_skb_queue.lock, flags);
}

static void
ieee80211_special_filter_queue_request(struct ieee80211_sub_if_data *sdata,
				       struct sk_buff *skb)
{
	unsigned long flags;
	bool work_runing = false;

	spin_lock_irqsave(&sdata->special_filter_skb_queue.lock, flags);
	__Sstar_skb_queue_tail(&sdata->special_filter_skb_queue, skb);
	work_runing = sdata->special_running;
	spin_unlock_irqrestore(&sdata->special_filter_skb_queue.lock, flags);

	if (work_runing == false)
		ieee80211_queue_work(&sdata->local->hw,
				     &sdata->special_filter_work);
}
void ieee80211_special_filter_init(struct ieee80211_sub_if_data *sdata)
{
	Sstar_skb_queue_head_init(&sdata->special_filter_skb_queue);
	SSTAR_INIT_WORK(&sdata->special_filter_work,
			ieee80211_special_filter_work);
	atomic_set(&sdata->special_enable, 1);
	INIT_LIST_HEAD(&sdata->filter_list);
}

void ieee80211_special_filter_exit(struct ieee80211_sub_if_data *sdata)
{
	atomic_set(&sdata->special_enable, 0);
	Sstar_cancel_work_sync(&sdata->special_filter_work);
	Sstar_skb_queue_purge(&sdata->special_filter_skb_queue);
	ieee80211_special_filter_clear_handle(sdata, NULL);
}
bool ieee80211_special_filter_register(struct ieee80211_sub_if_data *sdata,
				       struct ieee80211_special_filter *filter)
{
	struct sk_buff *skb;

	if (!ieee80211_sdata_running(sdata))
		return false;

	skb = Sstar_dev_alloc_skb(sizeof(struct ieee80211_special_filter));

	if (skb == NULL)
		return false;

	skb->pkt_type = IEEE80211_SPECIAL_FRAME_FILTER_REGISTER_MSG;
	memcpy(skb->data, filter, sizeof(struct ieee80211_special_filter));
	Sstar_skb_put(skb, sizeof(struct ieee80211_special_filter));

	ieee80211_special_filter_queue_request(sdata, skb);

	return true;
}
bool ieee80211_special_filter_clear(struct ieee80211_sub_if_data *sdata)
{
	struct sk_buff *skb;

	if (!ieee80211_sdata_running(sdata))
		return false;

	skb = Sstar_dev_alloc_skb(0);

	if (skb == NULL)
		return false;

	skb->pkt_type = IEEE80211_SPECIAL_FRAME_FILTER_CLEAE_MSG;

	ieee80211_special_filter_queue_request(sdata, skb);

	return true;
}
bool ieee80211_special_filter_request(
	struct ieee80211_sub_if_data *sdata,
	struct ieee80211_special_filter_table *tables)
{
	struct sk_buff *skb;
	union ieee80211_special_filter_cb *request;
	if (!ieee80211_sdata_running(sdata))
		return false;

	skb = Sstar_dev_alloc_skb(
		sizeof(struct ieee80211_special_filter_table *));

	if (skb == NULL)
		return false;

	request = (union ieee80211_special_filter_cb *)skb->cb;
	skb->pkt_type = IEEE80211_SPECIAL_FRAME_FILTER_REQUEST_MSG;
	request->request.tables = tables;

	ieee80211_special_filter_queue_request(sdata, skb);

	Sstar_flush_workqueue(sdata->local->workqueue);

	return true;
}
struct sk_buff *ieee80211_special_queue_package(struct ieee80211_vif *vif,
						struct sk_buff *skb)
{
	struct ieee80211_sub_if_data *sdata = vif_to_sdata(vif);
	if (!ieee80211_sdata_running(sdata))
		return skb;

	skb->pkt_type = IEEE80211_SPECIAL_RX_PACKAGE_MSG;

	ieee80211_special_filter_queue_request(sdata, skb);

	return NULL;
}

void ieee80211_special_check_package(struct ieee80211_local *local,
				     struct sk_buff *skb)
{
	struct ieee80211_sub_if_data *sdata = NULL;
	struct Sstar_ieee80211_mgmt *mgmt =
		(struct Sstar_ieee80211_mgmt *)skb->data;
	u8 is_beacon = ieee80211_is_beacon(mgmt->frame_control);
	u8 is_probereq = ieee80211_is_probe_req(mgmt->frame_control);
	u8 is_proberesp = ieee80211_is_probe_resp(mgmt->frame_control);
	u8 is_action = ieee80211_is_action(mgmt->frame_control);
	u8 keep = 0;

	if (!(is_beacon || is_probereq)) {
		return;
	}

	list_for_each_entry_rcu (sdata, &local->interfaces, list) {
		struct ieee80211_special_filter_list *filter_temp = NULL;

		if (!ieee80211_sdata_running(sdata))
			continue;

		if (list_empty(&sdata->filter_list))
			continue;

		keep = 0;

		list_for_each_entry (filter_temp, &sdata->filter_list, list) {
			if (filter_temp->filter.flags &
			    SPECIAL_F_FLAGS_FRAME_TYPE) {
				if ((is_beacon &&
				     filter_temp->filter.filter_action ==
					     0x80) ||
				    (is_probereq &&
				     filter_temp->filter.filter_action ==
					     0x40) ||
				    (is_proberesp &&
				     filter_temp->filter.filter_action ==
					     0x50) ||
				    (is_action)) {
					keep = 1;
					break;
				}
			} else {
				const u8 *pos;
				const u8 *target;
				int ie_len = 0;

				if (is_beacon) {
					pos = mgmt->u.beacon.variable;
					ie_len =
						skb->len -
						offsetof(
							struct Sstar_ieee80211_mgmt,
							u.beacon.variable);
				} else if (is_probereq) {
					pos = mgmt->u.probe_req.variable;
					ie_len =
						skb->len -
						offsetof(
							struct Sstar_ieee80211_mgmt,
							u.probe_req.variable);
				} else {
					pos = mgmt->u.probe_resp.variable;
					ie_len =
						skb->len -
						offsetof(
							struct Sstar_ieee80211_mgmt,
							u.probe_resp.variable);
				}
			next:
				//Sstar_printk_debug("%s:pos(%p),ie_len(%d)\n",__func__,pos,ie_len);
				target = Sstar_ieee80211_find_ie(
					filter_temp->filter.filter_action, pos,
					ie_len);

				if (target == NULL)
					continue;

				if (filter_temp->filter.flags &
				    SPECIAL_F_FLAGS_FRAME_OUI) {
					if ((target[2] ==
					     filter_temp->filter.oui[0]) &&
					    (target[3] ==
					     filter_temp->filter.oui[1]) &&
					    (target[4] ==
					     filter_temp->filter.oui[2])) {
						keep = 1;
						break;
					}
					ie_len -= (target - pos);
					ie_len -= (target[1] + 2);
					pos = target + target[1] + 2;

					if (ie_len < 2)
						continue;

					goto next;
				} else {
					keep = 1;
					break;
				}
			}
		}
		if (keep) {
			struct sk_buff *skb_cp = NULL;
			skb_cp = Sstar_skb_copy(skb, GFP_KERNEL);
			if (skb_cp) {
				if (ieee80211_special_queue_package(&sdata->vif,
								    skb_cp))
					Sstar_kfree_skb(skb_cp);
			}
		}
	}
}
