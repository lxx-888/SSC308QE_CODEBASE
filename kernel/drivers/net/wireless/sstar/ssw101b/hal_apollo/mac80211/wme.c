/*
 * Copyright 2004, Instant802 Networks, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#include <linux/if_arp.h>
#include <linux/types.h>
#include <net/ip.h>
#include <net/pkt_sched.h>

#include <net/Sstar_mac80211.h>
#include "ieee80211_i.h"
#include "wme.h"

/* Default mapping in classifier to work with default
 * queue setup.
 */
const int ieee802_1d_to_ac[8] = { IEEE80211_AC_BE, IEEE80211_AC_BK,
				  IEEE80211_AC_BK, IEEE80211_AC_BE,
				  IEEE80211_AC_VI, IEEE80211_AC_VI,
				  IEEE80211_AC_VO, IEEE80211_AC_VO };

static int wme_downgrade_ac(struct sk_buff *skb)
{
	switch (skb->priority) {
	case 6:
	case 7:
		skb->priority = 5; /* VO -> VI */
		return 0;
	case 4:
	case 5:
		skb->priority = 3; /* VI -> BE */
		return 0;
	case 0:
	case 3:
		skb->priority = 2; /* BE -> BK */
		return 0;
	default:
		return -1;
	}
}

/* Indicate which queue to use. */
u16 ieee80211_select_queue(struct ieee80211_sub_if_data *sdata,
			   struct sk_buff *skb)
{
	struct ieee80211_local *local = sdata->local;
	struct sta_info *sta = NULL;
	const u8 *ra = NULL;
	bool qos = false;

	if (local->hw.queues < IEEE80211_NUM_ACS || skb->len < 6) {
		skb->priority = 0; /* required for correct WPA/11i MIC */
		return 0;
	}

	rcu_read_lock();
	switch (sdata->vif.type) {
	case NL80211_IFTYPE_AP_VLAN:
		sta = rcu_dereference(sdata->u.vlan.sta);
		if (sta) {
			qos = test_sta_flag(sta, WLAN_STA_WME);
			break;
		}
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 60))

		fallthrough;
#endif
	case NL80211_IFTYPE_AP:
		ra = skb->data;
		break;
#ifdef CONFIG_SSTAR_SUPPORT_WDS
	case NL80211_IFTYPE_WDS:
		ra = sdata->u.wds.remote_addr;
		break;
#endif
#ifdef CONFIG_MAC80211_SSTAR_MESH
	case NL80211_IFTYPE_MESH_POINT:
		ra = skb->data;
		break;
#endif
	case NL80211_IFTYPE_STATION:
		ra = sdata->u.mgd.bssid;
		break;
#ifdef CONFIG_SSTAR_SUPPORT_IBSS
	case NL80211_IFTYPE_ADHOC:
		ra = skb->data;
		break;
#endif
	default:
		break;
	}

	if (!sta && ra && !is_multicast_ether_addr(ra)) {
		sta = sta_info_get(sdata, ra);
		if (sta)
			qos = test_sta_flag(sta, WLAN_STA_WME);
	}
	rcu_read_unlock();

	if (!qos) {
		skb->priority = 0; /* required for correct WPA/11i MIC */
		return IEEE80211_AC_BE;
	}

/* use the data classifier to determine what 802.1d tag the
	 * data frame has */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 11, 0))
	skb->priority = cfg80211_classify8021d(skb);
#else
	skb->priority = cfg80211_classify8021d(skb, NULL);
#endif

	return ieee80211_downgrade_queue(local, skb);
}

u16 ieee80211_downgrade_queue(struct ieee80211_local *local,
			      struct sk_buff *skb)
{
	/* in case we are a client verify acm is not set for this ac */
#ifdef CONFIG_SSTAR_APOLLO_TESTMODE
	/*And if acm is set check whether the ac has been admitted */
	while (unlikely((local->wmm_acm & BIT(skb->priority)) &&
			!(local->wmm_admitted_ups & BIT(skb->priority)))) {
#else
	while (unlikely(local->wmm_acm & BIT(skb->priority))) {
#endif /*CONFIG_SSTAR_APOLLO_TESTMODE*/
		if (wme_downgrade_ac(skb)) {
			/*
			 * This should not really happen. The AP has marked all
			 * lower ACs to require admission control which is not
			 * a reasonable configuration. Allow the frame to be
			 * transmitted using AC_BK as a workaround.
			 */
			break;
		}
	}

	/* look up which queue to use for frames with this 1d tag */
	return ieee802_1d_to_ac[skb->priority];
}

void ieee80211_set_qos_hdr(struct ieee80211_sub_if_data *sdata,
			   struct sk_buff *skb)
{
	struct ieee80211_hdr *hdr = (void *)skb->data;

	/* Fill in the QoS header if there is one. */
	if (ieee80211_is_data_qos(hdr->frame_control)) {
		u8 *p = ieee80211_get_qos_ctl(hdr);
		u8 ack_policy, tid;

		tid = skb->priority & IEEE80211_QOS_CTL_TAG1D_MASK;

		/* preserve EOSP bit */
		ack_policy = *p & IEEE80211_QOS_CTL_EOSP;

		if (unlikely(sdata->local->wifi_wme_noack_test))
			ack_policy |= IEEE80211_QOS_CTL_ACK_POLICY_NOACK;
		/* qos header is 2 bytes */
		*p++ = ack_policy | tid;
		*p = ieee80211_vif_is_mesh(&sdata->vif) ?
				   (IEEE80211_QOS_CTL_MESH_CONTROL_PRESENT >> 8) :
				   0;
	}
}
