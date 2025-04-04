/*
 * Copyright 2002-2005, Instant802 Networks, Inc.
 * Copyright 2005-2006, Devicescape Software, Inc.
 * Copyright 2006-2007	Jiri Benc <jbenc@suse.cz>
 * Copyright 2008-2010	Johannes Berg <johannes@sipsolutions.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/export.h>
#include <net/Sstar_mac80211.h>
#include "ieee80211_i.h"
#include "rate.h"
#include "mesh.h"
#include "led.h"
#include "wme.h"

#if 0
void ieee80211_tx_status_irqsafe(struct ieee80211_hw *hw,
				 struct sk_buff *skb)
{
	struct ieee80211_local *local = hw_to_local(hw);
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	int tmp;

	skb->pkt_type = IEEE80211_TX_STATUS_MSG;
	Sstar_skb_queue_tail(info->flags & IEEE80211_TX_CTL_REQ_TX_STATUS ?
		       &local->skb_queue : &local->skb_queue_unreliable, skb);
	tmp = Sstar_skb_queue_len(&local->skb_queue) +
		Sstar_skb_queue_len(&local->skb_queue_unreliable);
	while (tmp > IEEE80211_IRQSAFE_QUEUE_LIMIT &&
	       (skb = Sstar_skb_dequeue(&local->skb_queue_unreliable))) {
		Sstar_dev_kfree_skb_irq(skb);
		tmp--;
		I802_DEBUG_INC(local->tx_status_drop);
	}
	tasklet_schedule(&local->tasklet);
}
#endif
//EXPORT_SYMBOL(ieee80211_tx_status_irqsafe);
#ifndef CONFIG_TX_NO_CONFIRM
static void ieee80211_handle_filtered_frame(struct ieee80211_local *local,
					    struct sta_info *sta,
					    struct sk_buff *skb)
{
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	struct ieee80211_hdr *hdr = (void *)skb->data;
	int ac;

	/*
	 * This skb 'survived' a round-trip through the driver, and
	 * hopefully the driver didn't mangle it too badly. However,
	 * we can definitely not rely on the control information
	 * being correct. Clear it so we don't get junk there, and
	 * indicate that it needs new processing, but must not be
	 * modified/encrypted again.
	 */
	memset(&info->control, 0, sizeof(info->control));

	info->control.jiffies = jiffies;
	info->control.vif = &sta->sdata->vif;
	info->flags |= IEEE80211_TX_INTFL_NEED_TXPROCESSING |
		       IEEE80211_TX_INTFL_RETRANSMISSION;
	info->flags &= ~IEEE80211_TX_TEMPORARY_FLAGS;

	sta->tx_filtered_count++;

	/*
	 * Clear more-data bit on filtered frames, it might be set
	 * but later frames might time out so it might have to be
	 * clear again ... It's all rather unlikely (this frame
	 * should time out first, right?) but let's not confuse
	 * peers unnecessarily.
	 */
	if (hdr->frame_control & cpu_to_le16(IEEE80211_FCTL_MOREDATA))
		hdr->frame_control &= ~cpu_to_le16(IEEE80211_FCTL_MOREDATA);

	if (ieee80211_is_data_qos(hdr->frame_control)) {
		u8 *p = ieee80211_get_qos_ctl(hdr);
		int tid = *p & IEEE80211_QOS_CTL_TID_MASK;

		/*
		 * Clear EOSP if set, this could happen e.g.
		 * if an absence period (us being a P2P GO)
		 * shortens the SP.
		 */
		if (*p & IEEE80211_QOS_CTL_EOSP)
			*p &= ~IEEE80211_QOS_CTL_EOSP;
		ac = ieee802_1d_to_ac[tid & 7];
	} else {
		ac = IEEE80211_AC_BE;
	}

	/*
	 * Clear the TX filter mask for this STA when sending the next
	 * packet. If the STA went to power save mode, this will happen
	 * when it wakes up for the next time.
	 */
	set_sta_flag(sta, WLAN_STA_CLEAR_PS_FILT);

	/*
	 * This code races in the following way:
	 *
	 *  (1) STA sends frame indicating it will go to sleep and does so
	 *  (2) hardware/firmware adds STA to filter list, passes frame up
	 *  (3) hardware/firmware processes TX fifo and suppresses a frame
	 *  (4) we get TX status before having processed the frame and
	 *	knowing that the STA has gone to sleep.
	 *
	 * This is actually quite unlikely even when both those events are
	 * processed from interrupts coming in quickly after one another or
	 * even at the same time because we queue both TX status events and
	 * RX frames to be processed by a tasklet and process them in the
	 * same order that they were received or TX status last. Hence, there
	 * is no race as long as the frame RX is processed before the next TX
	 * status, which drivers can ensure, see below.
	 *
	 * Note that this can only happen if the hardware or firmware can
	 * actually add STAs to the filter list, if this is done by the
	 * driver in response to set_tim() (which will only reduce the race
	 * this whole filtering tries to solve, not completely solve it)
	 * this situation cannot happen.
	 *
	 * To completely solve this race drivers need to make sure that they
	 *  (a) don't mix the irq-safe/not irq-safe TX status/RX processing
	 *	functions and
	 *  (b) always process RX events before TX status events if ordering
	 *      can be unknown, for example with different interrupt status
	 *	bits.
	 *  (c) if PS mode transitions are manual (i.e. the flag
	 *      %IEEE80211_HW_AP_LINK_PS is set), always process PS state
	 *      changes before calling TX status events if ordering can be
	 *	unknown.
	 */
	if (test_sta_flag(sta, WLAN_STA_PS_STA) &&
	    Sstar_skb_queue_len(&sta->tx_filtered[ac]) < STA_MAX_TX_BUFFER) {
		Sstar_skb_queue_tail(&sta->tx_filtered[ac], skb);
		sta_info_recalc_tim(sta);

		if (!Sstar_timer_pending(&local->sta_cleanup))
			Sstar_mod_timer(
				&local->sta_cleanup,
				round_jiffies(jiffies +
					      STA_INFO_CLEANUP_INTERVAL));
		return;
	}

	if (!test_sta_flag(sta, WLAN_STA_PS_STA) &&
	    !(info->flags & IEEE80211_TX_INTFL_RETRIED)) {
		/* Software retry the packet once */
		info->flags |= IEEE80211_TX_INTFL_RETRIED;
		ieee80211_add_pending_skb(local, skb);
		return;
	}

#ifdef CONFIG_MAC80211_SSTAR_VERBOSE_DEBUG
	if (net_ratelimit())
		wiphy_debug(
			local->hw.wiphy,
			"dropped TX filtered frame, queue_len=%d PS=%d @%lu\n",
			Sstar_skb_queue_len(&sta->tx_filtered[ac]),
			!!test_sta_flag(sta, WLAN_STA_PS_STA), jiffies);
#endif
	Sstar_dev_kfree_skb(skb);
}
#endif
#ifdef CONFIG_SSTAR_SW_AGGTX
static void ieee80211_check_pending_bar(struct sta_info *sta, u8 *addr, u8 tid)
{
	struct tid_ampdu_tx *tid_tx;

	tid_tx = rcu_dereference(sta->ampdu_mlme.tid_tx[tid]);
	if (!tid_tx || !tid_tx->bar_pending)
		return;

	tid_tx->bar_pending = false;
	ieee80211_send_bar(&sta->sdata->vif, addr, tid, tid_tx->failed_bar_ssn);
}
static void ieee80211_set_bar_pending(struct sta_info *sta, u8 tid, u16 ssn)
{
	struct tid_ampdu_tx *tid_tx;

	tid_tx = rcu_dereference(sta->ampdu_mlme.tid_tx[tid]);
	if (!tid_tx)
		return;

	tid_tx->failed_bar_ssn = ssn;
	tid_tx->bar_pending = true;
}
#endif

static void ieee80211_frame_acked(struct sta_info *sta, struct sk_buff *skb)
{
#if defined(CONFIG_SSTAR_SW_AGGTX) || defined(CONFIG_SSTAR_SMPS)
	struct Sstar_ieee80211_mgmt *mgmt = (void *)skb->data;
#endif
#ifdef CONFIG_SSTAR_SMPS
	struct ieee80211_local *local = sta->local;
	struct ieee80211_sub_if_data *sdata = sta->sdata;
#endif
#ifdef CONFIG_SSTAR_SW_AGGTX
	if (ieee80211_is_data_qos(mgmt->frame_control)) {
		struct ieee80211_hdr *hdr = (void *)skb->data;
		u8 *qc = ieee80211_get_qos_ctl(hdr);
		u16 tid = qc[0] & 0xf;

		ieee80211_check_pending_bar(sta, hdr->addr1, tid);
	}
#endif

#ifdef CONFIG_SSTAR_SMPS
	if (ieee80211_is_action(mgmt->frame_control) &&
	    sdata->vif.type == NL80211_IFTYPE_STATION &&
	    mgmt->u.action.category == SSTAR_WLAN_CATEGORY_HT &&
	    mgmt->u.action.u.ht_smps.action == WLAN_HT_ACTION_SMPS) {
		/*
		 * This update looks racy, but isn't -- if we come
		 * here we've definitely got a station that we're
		 * talking to, and on a managed interface that can
		 * only be the AP. And the only other place updating
		 * this variable is before we're associated.
		 */
		switch (mgmt->u.action.u.ht_smps.smps_control) {
		case WLAN_HT_SMPS_CONTROL_DYNAMIC:
			sta->sdata->u.mgd.ap_smps = IEEE80211_SMPS_DYNAMIC;
			break;
		case WLAN_HT_SMPS_CONTROL_STATIC:
			sta->sdata->u.mgd.ap_smps = IEEE80211_SMPS_STATIC;
			break;
		case WLAN_HT_SMPS_CONTROL_DISABLED:
		default: /* shouldn't happen since we don't send that */
			sta->sdata->u.mgd.ap_smps = IEEE80211_SMPS_OFF;
			break;
		}

		ieee80211_queue_work(&local->hw, &local->recalc_smps);
	}
#endif
}

static int ieee80211_tx_radiotap_len(struct ieee80211_tx_info *info)
{
	int len = sizeof(struct ieee80211_radiotap_header);

	/* IEEE80211_RADIOTAP_RATE rate */
	if (info->status.rates[0].idx >= 0 &&
	    !(info->status.rates[0].flags & IEEE80211_TX_RC_MCS))
		len += 2;

	/* IEEE80211_RADIOTAP_TX_FLAGS */
	len += 2;

	/* IEEE80211_RADIOTAP_DATA_RETRIES */
	len += 1;

	/* IEEE80211_TX_RC_MCS */
	if (info->status.rates[0].idx >= 0 &&
	    info->status.rates[0].flags & IEEE80211_TX_RC_MCS)
		len += 3;

	return len;
}

static void
ieee80211_add_tx_radiotap_header(struct ieee80211_supported_band *sband,
				 struct sk_buff *skb, int retry_count,
				 int rtap_len)
{
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	struct ieee80211_radiotap_header *rthdr;
	unsigned char *pos;
	u16 txflags;

	rthdr = (struct ieee80211_radiotap_header *)Sstar_skb_push(skb,
								   rtap_len);

	memset(rthdr, 0, rtap_len);
	rthdr->it_len = cpu_to_le16(rtap_len);
	rthdr->it_present = cpu_to_le32((1 << IEEE80211_RADIOTAP_TX_FLAGS) |
					(1 << IEEE80211_RADIOTAP_DATA_RETRIES));
	pos = (unsigned char *)(rthdr + 1);

	/*
	 * XXX: Once radiotap gets the bitmap reset thing the vendor
	 *	extensions proposal contains, we can actually report
	 *	the whole set of tries we did.
	 */

	/* IEEE80211_RADIOTAP_RATE */
	if (info->status.rates[0].idx >= 0 &&
	    !(info->status.rates[0].flags & IEEE80211_TX_RC_MCS)) {
		rthdr->it_present |= cpu_to_le32(1 << IEEE80211_RADIOTAP_RATE);
		*pos = sband->bitrates[info->status.rates[0].idx].bitrate / 5;
		/* padding for tx flags */
		pos += 2;
	}

	/* IEEE80211_RADIOTAP_TX_FLAGS */
	txflags = 0;
	if (!(info->flags & IEEE80211_TX_STAT_ACK) &&
	    !is_multicast_ether_addr(hdr->addr1))
		txflags |= IEEE80211_RADIOTAP_F_TX_FAIL;

	if ((info->status.rates[0].flags & IEEE80211_TX_RC_USE_RTS_CTS) ||
	    (info->status.rates[0].flags & IEEE80211_TX_RC_USE_CTS_PROTECT))
		txflags |= IEEE80211_RADIOTAP_F_TX_CTS;
	else if (info->status.rates[0].flags & IEEE80211_TX_RC_USE_RTS_CTS)
		txflags |= IEEE80211_RADIOTAP_F_TX_RTS;

	put_unaligned_le16(txflags, pos);
	pos += 2;

	/* IEEE80211_RADIOTAP_DATA_RETRIES */
	/* for now report the total retry_count */
	*pos = retry_count;
	pos++;

	/* IEEE80211_TX_RC_MCS */
	if (info->status.rates[0].idx >= 0 &&
	    info->status.rates[0].flags & IEEE80211_TX_RC_MCS) {
		rthdr->it_present |= cpu_to_le32(1 << IEEE80211_RADIOTAP_MCS);
		pos[0] = IEEE80211_RADIOTAP_MCS_HAVE_MCS |
			 IEEE80211_RADIOTAP_MCS_HAVE_GI |
			 IEEE80211_RADIOTAP_MCS_HAVE_BW;
		if (info->status.rates[0].flags & IEEE80211_TX_RC_SHORT_GI)
			pos[1] |= IEEE80211_RADIOTAP_MCS_SGI;
		if (info->status.rates[0].flags & IEEE80211_TX_RC_40_MHZ_WIDTH)
			pos[1] |= IEEE80211_RADIOTAP_MCS_BW_40;
		if (info->status.rates[0].flags & IEEE80211_TX_RC_GREEN_FIELD)
			pos[1] |= IEEE80211_RADIOTAP_MCS_FMT_GF;
		pos[2] = info->status.rates[0].idx;
		pos += 3;
	}
}

/*
 * Use a static threshold for now, best value to be determined
 * by testing ...
 * Should it depend on:
 *  - on # of retransmissions
 *  - current throughput (higher value for higher tpt)?
 */
#define STA_LOST_PKT_THRESHOLD 50

void ieee80211_tx_status(struct ieee80211_hw *hw, struct sk_buff *skb)
{
	struct sk_buff *skb2;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	struct ieee80211_local *local = hw_to_local(hw);
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	u16 frag, type;
#if defined(CONFIG_SSTAR_MAC80211_NO_USE) || defined(CONFIG_SSTAR_SW_AGGTX)
	__le16 fc;
#endif
	struct ieee80211_supported_band *sband;
	struct ieee80211_sub_if_data *sdata;
	struct net_device *prev_dev = NULL;
	struct sta_info *sta, *tmp;
	int retry_count = -1, i;
	int rates_idx = -1;
	bool send_to_cooked;
	bool acked;
#ifdef CONFIG_SSTAR_SW_AGGTX
	struct ieee80211_bar *bar;
	u16 tid;
#endif
	int rtap_len;

	for (i = 0; i < IEEE80211_TX_MAX_RATES; i++) {
		if (info->status.rates[i].idx < 0) {
			break;
		} else if (i >= hw->max_report_rates) {
			/* the HW cannot have attempted that rate */
			info->status.rates[i].idx = -1;
			info->status.rates[i].count = 0;
			break;
		}

		retry_count += info->status.rates[i].count;
	}
	rates_idx = i - 1;

	if (retry_count < 0)
		retry_count = 0;

	rcu_read_lock();
	sdata = vif_to_sdata(info->control.vif);
	sband = local->hw.wiphy->bands[info->band];
#if defined(CONFIG_SSTAR_MAC80211_NO_USE) || defined(CONFIG_SSTAR_SW_AGGTX)
	fc = hdr->frame_control;
#endif
	for_each_sta_info(local, hdr->addr1, sta, tmp)
	{
		/* skip wrong virtual interface */
		if (memcmp(hdr->addr2, sta->sdata->vif.addr, ETH_ALEN))
			continue;

		if (info->flags & IEEE80211_TX_STATUS_EOSP)
			clear_sta_flag(sta, WLAN_STA_SP);
		acked = !!(info->flags & IEEE80211_TX_STAT_ACK);
#ifndef CONFIG_TX_NO_CONFIRM
		if (!acked && test_sta_flag(sta, WLAN_STA_PS_STA)) {
			/*
			 * The STA is in power save mode, so assume
			 * that this TX packet failed because of that.
			 */
			ieee80211_handle_filtered_frame(local, sta, skb);
			rcu_read_unlock();
			return;
		}
#endif
		if ((local->hw.flags & IEEE80211_HW_HAS_RATE_CONTROL) &&
		    (rates_idx != -1))
			sta->last_tx_rate = info->status.rates[rates_idx];
#ifdef CONFIG_SSTAR_SW_AGGTX
		if ((info->flags & IEEE80211_TX_STAT_AMPDU_NO_BACK) &&
		    (ieee80211_is_data_qos(fc))) {
			u16 tid, ssn;
			u8 *qc;

			qc = ieee80211_get_qos_ctl(hdr);
			tid = qc[0] & 0xf;
			ssn = ((le16_to_cpu(hdr->seq_ctrl) + 0x10) &
			       IEEE80211_SCTL_SEQ);
			ieee80211_send_bar(&sta->sdata->vif, hdr->addr1, tid,
					   ssn);
		}

		if (!acked && ieee80211_is_back_req(fc)) {
			u16 control;

			/*
			 * BAR failed, store the last SSN and retry sending
			 * the BAR when the next unicast transmission on the
			 * same TID succeeds.
			 */
			bar = (struct ieee80211_bar *)skb->data;
			control = le16_to_cpu(bar->control);
			if (!(control & IEEE80211_BAR_CTRL_MULTI_TID)) {
				u16 ssn = le16_to_cpu(bar->start_seq_num);

				tid = (control &
				       IEEE80211_BAR_CTRL_TID_INFO_MASK) >>
				      IEEE80211_BAR_CTRL_TID_INFO_SHIFT;

				ieee80211_set_bar_pending(sta, tid, ssn);
			}
		}
#endif
#ifndef CONFIG_TX_NO_CONFIRM
		if (info->flags & IEEE80211_TX_STAT_TX_FILTERED) {
			ieee80211_handle_filtered_frame(local, sta, skb);
			rcu_read_unlock();
			return;
		} else
#endif
		{
			if (!acked)
				sta->tx_retry_failed++;
			sta->tx_retry_count += retry_count;
		}
		if (sta->sdata->vif.type == NL80211_IFTYPE_STATION) {
			if (info->flags & IEEE80211_TX_HANDSHAKE) {
				struct sk_buff *buffed_skb = NULL;
				clear_sta_flag(sta,
					       WLAN_STA_HANDSHAKE4OF4_SENDING);
				set_sta_flag(sta,
					     WLAN_STA_HANDSHAKE4OF4_SUCCESS);
				smp_mb();
				while ((buffed_skb = Sstar_skb_dequeue(
						&sta->handshake_buffed))) {
					struct ieee80211_tx_info *buffed_info =
						IEEE80211_SKB_CB(buffed_skb);
					Sstar_printk_mgmt(
						"%s:release buffed skb\n",
						__func__);
					buffed_info->flags |=
						IEEE80211_TX_INTFL_NEED_TXPROCESSING;
					ieee80211_add_pending_skb(local,
								  buffed_skb);
				}
				if (acked) {
					Sstar_printk_mgmt(
						"%s:4/4 Pairwise Succeed\n",
						sta->sdata->name);

				} else {
					Sstar_printk_err(
						"%s:4/4 Pairwise Failed\n",
						sta->sdata->name);
					/*
					*Here Maybe needed deauthen
					*/
					if (sta->sdata)
						ieee80211_connection_loss(
							&sta->sdata->vif);
				}
			}
		}
#ifndef CONFIG_RATE_HW_CONTROL
		rate_control_tx_status(local, sband, sta, skb);
#endif
#ifdef CONFIG_MAC80211_SSTAR_MESH
		if (ieee80211_vif_is_mesh(&sta->sdata->vif))
			ieee80211s_update_metric(local, sta, skb);
#endif

		if (!(info->flags & IEEE80211_TX_CTL_INJECTED) && acked)
			ieee80211_frame_acked(sta, skb);
#ifdef CONFIG_SSTAR_MAC80211_NO_USE
		if ((sta->sdata->vif.type == NL80211_IFTYPE_STATION) &&
		    (local->hw.flags & IEEE80211_HW_REPORTS_TX_ACK_STATUS))
			ieee80211_sta_tx_notify(sta->sdata, (void *)skb->data,
						acked);
#endif
#ifndef CONFIG_TX_NO_CONFIRM
		if (local->hw.flags & IEEE80211_HW_REPORTS_TX_ACK_STATUS) {
			if (info->flags & IEEE80211_TX_STAT_ACK) {
				if (sta->lost_packets)
					sta->lost_packets = 0;
			} else if (++sta->lost_packets >=
				   STA_LOST_PKT_THRESHOLD) {
				cfg80211_cqm_pktloss_notify(sta->sdata->dev,
							    sta->sta.addr,
							    sta->lost_packets,
							    GFP_ATOMIC);
				sta->lost_packets = 0;
			}
		}
#endif
	}
#ifdef SSTAR_AP_SME
	if (info->flags & IEEE80211_TX_AP_HANDLE_STATUS) {
		if (!ieee80211_ap_sme_tx_mgmt_status(sdata, skb)) {
			rcu_read_unlock();
			return;
		}
	}
#endif
	rcu_read_unlock();

	ieee80211_led_tx(local, 0);

	/* SNMP counters
	 * Fragments are passed to low-level drivers as separate skbs, so these
	 * are actually fragments, not frames. Update frame counters only for
	 * the first fragment of the frame. */

	frag = le16_to_cpu(hdr->seq_ctrl) & IEEE80211_SCTL_FRAG;
	type = le16_to_cpu(hdr->frame_control) & IEEE80211_FCTL_FTYPE;

	if (info->flags & IEEE80211_TX_STAT_ACK) {
		if (frag == 0) {
			local->dot11TransmittedFrameCount++;
			if (is_multicast_ether_addr(hdr->addr1))
				local->dot11MulticastTransmittedFrameCount++;
			if (retry_count > 0)
				local->dot11RetryCount++;
			if (retry_count > 1)
				local->dot11MultipleRetryCount++;
		}

		/* This counter shall be incremented for an acknowledged MPDU
		 * with an individual address in the address 1 field or an MPDU
		 * with a multicast address in the address 1 field of type Data
		 * or Management. */
		if (!is_multicast_ether_addr(hdr->addr1) ||
		    type == IEEE80211_FTYPE_DATA ||
		    type == IEEE80211_FTYPE_MGMT)
			local->dot11TransmittedFragmentCount++;
	} else {
		if (frag == 0)
			local->dot11FailedCount++;
	}
#ifdef CONFIG_SSTAR_MAC80211_NO_USE
	if (ieee80211_is_nullfunc(fc) && ieee80211_has_pm(fc) &&
	    (local->hw.flags & IEEE80211_HW_REPORTS_TX_ACK_STATUS) &&
	    !(info->flags & IEEE80211_TX_CTL_INJECTED) && sdata->ps_allowed &&
	    !(local->scanning)) {
		if (info->flags & IEEE80211_TX_STAT_ACK)
			sdata->u.mgd.flags |= IEEE80211_STA_NULLFUNC_ACKED;
		else
			Sstar_mod_timer(&sdata->dynamic_ps_timer,
					jiffies + msecs_to_jiffies(10));
	}
#endif
	if (info->flags & IEEE80211_TX_INTFL_NL80211_FRAME_TX) {
		u64 cookie = (unsigned long)skb;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0))
		cfg80211_mgmt_tx_status(skb->dev, cookie, skb->data, skb->len,
					!!(info->flags & IEEE80211_TX_STAT_ACK),
					GFP_ATOMIC);
#else
		cfg80211_mgmt_tx_status(&sdata->wdev, cookie, skb->data,
					skb->len,
					!!(info->flags & IEEE80211_TX_STAT_ACK),
					GFP_ATOMIC);
#endif
	}

	/* this was a transmitted frame, but now we want to reuse it */
	Sstar_skb_orphan(skb);

	/* Need to make a copy before skb->cb gets cleared */
	send_to_cooked = !!(info->flags & IEEE80211_TX_CTL_INJECTED) ||
			 (type != IEEE80211_FTYPE_DATA);

	/*
	 * This is a bit racy but we can avoid a lot of work
	 * with this test...
	 */
	if (!local->monitors && (!send_to_cooked || !local->cooked_mntrs)) {
		Sstar_dev_kfree_skb(skb);
		return;
	}

	/* send frame to monitor interfaces now */
	rtap_len = ieee80211_tx_radiotap_len(info);
	if (WARN_ON_ONCE(Sstar_skb_headroom(skb) < rtap_len)) {
		Sstar_dev_kfree_skb(skb);
		return;
	}
	ieee80211_add_tx_radiotap_header(sband, skb, retry_count, rtap_len);

	/* XXX: is this sufficient for BPF? */
	Sstar_skb_set_mac_header(skb, 0);
	skb->ip_summed = CHECKSUM_UNNECESSARY;
	skb->pkt_type = PACKET_OTHERHOST;
	skb->protocol = htons(ETH_P_802_2);
	memset(skb->cb, 0, sizeof(skb->cb));

	rcu_read_lock();
	list_for_each_entry_rcu (sdata, &local->interfaces, list) {
		if (sdata->vif.type == NL80211_IFTYPE_MONITOR) {
			if (!ieee80211_sdata_running(sdata))
				continue;

			if ((sdata->u.mntr_flags & MONITOR_FLAG_COOK_FRAMES) &&
			    !send_to_cooked)
				continue;

			if (prev_dev) {
				skb2 = Sstar_skb_clone(skb, GFP_ATOMIC);
				if (skb2) {
					skb2->dev = prev_dev;
					Sstar_netif_rx(skb2);
				}
			}

			prev_dev = sdata->dev;
		}
	}
	if (prev_dev) {
		skb->dev = prev_dev;
		Sstar_netif_rx(skb);
		skb = NULL;
	}
	rcu_read_unlock();
	Sstar_dev_kfree_skb(skb);
}
//EXPORT_SYMBOL(ieee80211_tx_status);
#if defined(CONFIG_SSTAR_MAC80211_NO_USE)

void ieee80211_report_low_ack(struct ieee80211_sta *pubsta, u32 num_packets)
{
	struct sta_info *sta = container_of(pubsta, struct sta_info, sta);
	cfg80211_cqm_pktloss_notify(sta->sdata->dev, sta->sta.addr, num_packets,
				    GFP_ATOMIC);
}
//EXPORT_SYMBOL(ieee80211_report_low_ack);
char *ieee80211_vif_name_get(struct ieee80211_vif *vif)
{
	return vif_to_sdata(vif)->name;
}
//EXPORT_SYMBOL(ieee80211_vif_name_get);
#endif
