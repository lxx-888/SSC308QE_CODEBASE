
/*
 * Mac80211 STA API for sigmastar APOLLO drivers
 *
 * Copyright (c) 2016, sigmastar
 *
 * Based on:
 * Copyright (c) 2010, stericsson
 * Author: Dmitry Tarnyagin <dmitry.tarnyagin@stericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
#include <linux/firmware.h>
#include <linux/if_arp.h>
#include <linux/ipv6.h>
#include <linux/icmpv6.h>
#include <net/ndisc.h>

#include "apollo.h"
#include "sta.h"
#include "ap.h"
#include "fwio.h"
#include "bh.h"
#include "debug.h"
#include "wsm.h"
#include "hwio.h"
#ifdef CONFIG_SSTAR_SUPPORT_SCHED_SCAN
#ifdef ROAM_OFFLOAD
#include <net/netlink.h>
#endif /*ROAM_OFFLOAD*/
#endif
//#ifdef CONFIG_SSTAR_APOLLO_TESTMODE
#include "Sstar_testmode.h"
#include <net/netlink.h>
//#endif /* CONFIG_SSTAR_APOLLO_TESTMODE */

#include "net/Sstar_mac80211.h"
#include "dbg_event.h"
#include "smartconfig.h"

#include "mac80211/ieee80211_i.h"
#include "mac80211/Sstar_common.h"
#include "internal_cmd.h"
#ifdef CONFIG_SSTAR_DEV_IOCTL
#define SSTAR_DEV_IOCTL_DEBUG 1

#if SSTAR_DEV_IOCTL_DEBUG
#define dev_printk(...) Sstar_printk_always(__VA_ARGS__)
#else
#define dev_printk(...)
#endif

#define DEV_MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define DEV_MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
enum Sstar_msg_type {
	SSTAR_DEV_IO_GET_STA_STATUS = 0,
	SSTAR_DEV_IO_GET_STA_RSSI = 1, //STA connected AP's RSSI
	SSTAR_DEV_IO_GET_AP_INFO = 2, //STA or AP
	SSTAR_DEV_IO_GET_STA_INFO = 3,
	SSTAR_DEV_IO_SET_STA_SCAN = 4,
	SSTAR_DEV_IO_SET_FREQ = 5,
	SSTAR_DEV_IO_SET_SPECIAL_OUI = 6, //use for Beacon and Probe package
	SSTAR_DEV_IO_SET_STA_DIS = 7,
	SSTAR_DEV_IO_SET_IF_TYPE = 8,
	SSTAR_DEV_IO_SET_ADAPTIVE = 9,
	SSTAR_DEV_IO_SET_TXPWR_DCXO = 10,
	SSTAR_DEV_IO_SET_TXPWR = 11,
	SSTAR_DEV_IO_GET_WORK_CHANNEL = 12,
	SSTAR_DEV_IO_SET_BEST_CHANNEL_SCAN = 13,
	SSTAR_DEV_IO_GET_AP_LIST = 14,
	SSTAR_DEV_IO_GET_TP_RATE = 15,
	SSTAR_DEV_IO_ETF_TEST = 16,
	SSTAR_DEV_IO_ETF_GET_RESULT = 17,
	SSTAR_DEV_IO_ETF_START_TX = 18,
	SSTAR_DEV_IO_ETF_STOP_TX = 19,
	SSTAR_DEV_IO_ETF_START_RX = 20,
	SSTAR_DEV_IO_ETF_STOP_RX = 21,
	SSTAR_DEV_IO_FIX_TX_RATE = 22,
	SSTAR_DEV_IO_MAX_TX_RATE = 23,
	SSTAR_DEV_IO_TX_RATE_FREE = 24,
	SSTAR_DEV_IO_SET_EFUSE_MAC = 25,
	SSTAR_DEV_IO_SET_EFUSE_DCXO = 26,
	SSTAR_DEV_IO_SET_EFUSE_DELTAGAIN = 27,
	SSTAR_DEV_IO_MIN_TX_RATE = 28,
	SSTAR_DEV_IO_SET_RATE_POWER = 29,
#ifdef CONFIG_IEEE80211_SPECIAL_FILTER

	SSTAR_DEV_IO_SET_SPECIAL_FILTER = 30,
#endif
	SSTAR_DEV_IO_SET_COUNTRY_CODE = 31,

	SSTAR_DEV_IO_GET_DRIVER_VERSION = 32,
	SSTAR_DEV_IO_GET_EFUSE = 33,
	SSTAR_DEV_IO_GET_ETF_START_RX_RESULTS = 34,
	SSTAR_DEV_IO_SET_UPERR_PROCESS_PID = 35,
#ifdef CONFIG_SSTAR_SUPPORT_AP_CONFIG
	SSTAR_DEV_IO_SET_FIX_SCAN_CHANNEL = 36,
#endif
	SSTAR_DEV_IO_SET_AUTO_CALI_PPM = 37,
	SSTAR_DEV_IO_GET_CALI_REAULTS = 38,
	SSTAR_DEV_IO_SET_EFUSE_GAIN_COMPENSATION_VALUE = 39,
	SSTAR_DEV_IO_GET_VENDOR_SPECIAL_IE = 40,
};

#define WSM_MAX_NUM_LINK_AP 14 //Lmac support station number is 4;
typedef struct _Sstar_wifi_ap_info_ {
	int wext_rssi;
	unsigned long rx_packets;
	unsigned long tx_packets;
	unsigned long tx_retry_count;
	int last_rx_rate_idx;
	unsigned char wext_mac[ETH_ALEN];
	unsigned char sta_cnt;
} Sstar_wifi_ap_info;

typedef struct _Sstar_wifi_sta_info_ {
	int rssi;
	unsigned long rx_packets;
	unsigned long tx_packets;
	unsigned long tx_retry_count;
	u8 bssid[ETH_ALEN];
	u8 ssid[IEEE80211_MAX_SSID_LEN];
	size_t ssid_len;
} Sstar_wifi_sta_info;

struct altm_wext_msg {
	int type;
	int value;
	char externData[256];
};

extern unsigned int Sstar_wifi_status_get(void);
extern int Sstar_change_iface_to_monitor(struct net_device *dev);
extern void ieee80211_connection_loss(struct ieee80211_vif *vif);
extern int ieee80211_set_channel(struct wiphy *wiphy, struct net_device *netdev,
				 struct ieee80211_channel *chan,
				 enum nl80211_channel_type channel_type);
extern int Sstar_find_link_id(struct Sstar_vif *priv, const u8 *mac);
extern int str2mac(char *dst_mac, char *src_str);
extern void Sstar_set_freq(struct ieee80211_local *local);
extern void Sstar_set_tx_power(struct Sstar_common *hw_priv, int txpw);
extern u8 ETF_bStartTx;
extern u8 ETF_bStartRx;
extern char ch_and_type[20];

extern u8 ucWriteEfuseFlag;
extern u32 chipversion;
extern struct rxstatus_signed gRxs_s;
extern struct etf_test_config etf_config;
extern u32 MyRand(void);
extern int wsm_start_tx_v2(struct Sstar_common *hw_priv,
			   struct ieee80211_vif *vif);
extern int wsm_start_tx(struct Sstar_common *hw_priv,
			struct ieee80211_vif *vif);
extern int wsm_stop_tx(struct Sstar_common *hw_priv);
extern u32 GetChipVersion(struct Sstar_common *hw_priv);
extern void Sstar_set_special_oui(struct Sstar_common *hw_priv, char *pdata,
				  int len);
extern int DCXOCodeWrite(struct Sstar_common *hw_priv, u8 data);
extern u8 DCXOCodeRead(struct Sstar_common *hw_priv);

static int Sstar_dev_get_ap_info(struct net_device *dev,
				 struct altm_wext_msg *msg)
{
	int i = 0;
	int ret = 0;
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_local *local = sdata->local;
	struct Sstar_vif *priv = (struct Sstar_vif *)sdata->vif.drv_priv;
	struct Sstar_common *hw_priv = priv->hw_priv;
	struct sta_info *sta = NULL;
	Sstar_wifi_ap_info *p_info;

	char *pdata = NULL;

	dev_printk("%s\n", __func__);

	if (msg == NULL) {
		return -1;
	}
	if (!(p_info = (Sstar_wifi_ap_info *)Sstar_kzalloc(
		      WSM_MAX_NUM_LINK_AP * sizeof(Sstar_wifi_ap_info),
		      GFP_KERNEL))) {
		return -ENOMEM;
	}

	dev_printk("@@@ sta cnt %d\n", hw_priv->connected_sta_cnt);

	rcu_read_lock();

	list_for_each_entry_rcu (sta, &local->sta_list, list) {
		if (sta != NULL) {
			if (sta->sdata->vif.type == NL80211_IFTYPE_AP) {
				p_info[i].wext_rssi =
					(s8)-Sstar_ewma_read(&sta->avg_signal);
				p_info[i].rx_packets = sta->rx_packets;
				p_info[i].tx_packets = sta->tx_packets;
				p_info[i].tx_retry_count = sta->tx_retry_count;
				p_info[i].last_rx_rate_idx =
					sta->last_rx_rate_idx;

				memcpy(p_info[i].wext_mac, sta->sta.addr,
				       ETH_ALEN);
				p_info[i].sta_cnt = hw_priv->connected_sta_cnt;

				dev_printk("%d: MAC " DEV_MACSTR "\n", i,
					   DEV_MAC2STR(p_info[i].wext_mac));
				dev_printk("    RSSI %d\n",
					   p_info[i].wext_rssi);
				dev_printk("    RX Pkts %ld\n",
					   p_info[i].rx_packets);
				dev_printk("    TX Pkts %ld\n",
					   p_info[i].tx_packets);
				dev_printk("    TX Retry %ld\n",
					   p_info[i].tx_retry_count);

				++i;
			} else {
				msg->value =
					(s8)-Sstar_ewma_read(&sta->avg_signal);
				Sstar_printk_wext("# rssi %d\n", msg->value);
				break;
			}
		}
	}

	rcu_read_unlock();

	memcpy(&pdata, &msg->externData[0], 4);

	if (pdata != NULL) {
		if (copy_to_user(pdata, p_info,
				 WSM_MAX_NUM_LINK_AP *
					 sizeof(Sstar_wifi_ap_info))) {
			ret = -1;
		}
	}

	if (p_info != NULL)
		Sstar_kfree(p_info);

	return ret;
}

static int Sstar_dev_get_sta_info(struct net_device *dev,
				  struct altm_wext_msg *msg)
{
	int ret = 0;
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	//struct ieee80211_bss_conf *p_bss_conf = &sdata->vif.bss_conf;
	struct ieee80211_local *local = sdata->local;
	struct Sstar_vif *priv = (struct Sstar_vif *)sdata->vif.drv_priv;
	//struct Sstar_common *hw_priv = priv->hw_priv;
	struct sta_info *sta = NULL;
	Sstar_wifi_sta_info *p_info;

	dev_printk("%s\n", __func__);

	if (msg == NULL) {
		return -1;
	}

	if (priv->mode != NL80211_IFTYPE_STATION) {
		return -1;
	}

	if (priv->join_status != SSTAR_APOLLO_JOIN_STATUS_STA) {
		return -1;
	}

	if (!(p_info = (Sstar_wifi_sta_info *)Sstar_kzalloc(
		      sizeof(Sstar_wifi_sta_info), GFP_KERNEL))) {
		return -ENOMEM;
	}

	rcu_read_lock();
	list_for_each_entry_rcu (sta, &local->sta_list, list) {
		if (sta != NULL) {
			if (sta->sdata->vif.type == NL80211_IFTYPE_STATION) {
				//packets num
				struct ieee80211_if_managed *ifmgd =
					&sdata->u.mgd;
				struct cfg80211_bss *cbss;

				cbss = ifmgd->associated;

				if (cbss) {
					const char *ssid = NULL;

					ssid = ieee80211_bss_get_ie(
						cbss, SSTAR_WLAN_EID_SSID);

					if (ssid) {
						memcpy(p_info->ssid, &ssid[2],
						       ssid[1]);
						p_info->ssid_len = ssid[1];
					}
				}
				memcpy(p_info->bssid, sta->sta.addr, ETH_ALEN);
				p_info->rssi =
					(s8)-Sstar_ewma_read(&sta->avg_signal);
				p_info->rx_packets = sta->rx_packets;
				p_info->tx_packets = sta->tx_packets;
				p_info->tx_retry_count = sta->tx_retry_count;
				break;
			}
		}
	}
	rcu_read_unlock();

	dev_printk("    SSID %s\n", p_info->ssid);
	dev_printk("    BSSID " DEV_MACSTR "\n", DEV_MAC2STR(p_info->bssid));
	dev_printk("    RSSI %d\n", p_info->rssi);
	dev_printk("    RX Pkts %ld\n", p_info->rx_packets);
	dev_printk("    TX Pkts %ld\n", p_info->tx_packets);
	dev_printk("    TX Retry %ld\n", p_info->tx_retry_count);

	memcpy((u8 *)msg->externData, p_info, sizeof(Sstar_wifi_sta_info));

	if (p_info != NULL)
		Sstar_kfree(p_info);

	return ret;
}
static bool
Sstar_dev_handle_scan_sta(struct ieee80211_hw *hw,
			  struct Sstar_internal_scan_results_req *req,
			  struct ieee80211_internal_scan_sta *sta)
{
	Wifi_Recv_Info_t *info = (Wifi_Recv_Info_t *)req->priv;
	Wifi_Recv_Info_t *pos_info = NULL;

	if (req->n_stas >= MAC_FILTER_NUM) {
		return false;
	}

	if ((sta->ie == NULL) || (sta->ie_len == 0)) {
		return true;
	}
	pos_info = info + req->n_stas;
	req->n_stas++;
	pos_info->channel = sta->channel;
	pos_info->Rssi = sta->signal;
	memcpy(pos_info->Bssid, sta->bssid, 6);
	if (sta->ssid_len)
		memcpy(pos_info->Ssid, sta->ssid, sta->ssid_len);
	if (sta->ie && sta->ie_len)
		memcpy(pos_info->User_data, sta->ie, sta->ie_len);
	return true;
}
static int scan_result_filter_single_channel(u8 *recv_info, int channel)
{
	Wifi_Recv_Info_t *recv = NULL;
	//Wifi_Recv_Info_t recv_bak[MAC_FILTER_NUM];
	Wifi_Recv_Info_t *recv_bak = NULL;
	int i = 0, j = 0;
	if (!recv_info) {
		Sstar_printk_err(
			"scan_result_filter_channel recv_info NULL \n");
		return -1;
	}
	recv_bak = Sstar_kmalloc(sizeof(Wifi_Recv_Info_t) * MAC_FILTER_NUM,
				 GFP_KERNEL);
	if (!recv_bak) {
		Sstar_printk_err("scan_result_filter_channel recv_bak NULL \n");
		return -1;
	}
	memset(recv_bak, 0, sizeof(Wifi_Recv_Info_t) * MAC_FILTER_NUM);
	recv = (Wifi_Recv_Info_t *)recv_info;

	for (i = 0; i < MAC_FILTER_NUM; i++) {
		if (recv[i].channel == channel) {
			memcpy(&recv_bak[j], &recv[i],
			       sizeof(Wifi_Recv_Info_t));
			j++;
		}
	}
	memcpy(recv_info, recv_bak, sizeof(Wifi_Recv_Info_t) * MAC_FILTER_NUM);
	if (recv_bak)
		Sstar_kfree(recv_bak);
	return 0;
}

static int Sstar_dev_set_sta_scan(struct net_device *dev,
				  struct altm_wext_msg *msg)
{
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	char zero_mac[ETH_ALEN] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	unsigned short channel;
	int ret = 0;
	struct ieee80211_internal_scan_request internal_scan;
	struct Sstar_internal_scan_results_req req;
	//Wifi_Recv_Info_t *recv_data = NULL;
	u8 *user_pos = NULL;
	u8 *recv_info = NULL;
	u8 scan_ch = 0, scan_ch_list[3];

	memset(&internal_scan, 0,
	       sizeof(struct ieee80211_internal_scan_request));

	if (!ieee80211_sdata_running(sdata)) {
		ret = -ENETDOWN;
		goto exit;
	}

	if (sdata->vif.type != NL80211_IFTYPE_STATION) {
		ret = -EOPNOTSUPP;
		goto exit;
	}
	recv_info = Sstar_kzalloc(sizeof(Wifi_Recv_Info_t) * MAC_FILTER_NUM,
				  GFP_KERNEL);

	if (recv_info == NULL) {
		ret = -ENOMEM;
		goto exit;
	}

	memcpy(&channel, &msg->externData[0], sizeof(channel));
	scan_ch = (u8)channel;

	if (memcmp(&msg->externData[2], zero_mac, ETH_ALEN) != 0) {
		u8 *mac = &msg->externData[2];
		u8 index = 0;

		internal_scan.macs = Sstar_kzalloc(
			sizeof(struct ieee80211_internal_mac) * MAC_FILTER_NUM,
			GFP_KERNEL);

		if (internal_scan.macs == NULL) {
			ret = -ENOMEM;
			goto exit;
		}

		for (index = 0; index < MAC_FILTER_NUM; index++) {
			memcpy(internal_scan.macs[index].mac, &mac[6 * index],
			       6);
		}
		internal_scan.n_macs = MAC_FILTER_NUM;
	}
	if (scan_ch != 0) {
		scan_ch_list[0] = scan_ch_list[1] = scan_ch_list[2] = scan_ch;
		internal_scan.channels = scan_ch_list;
		internal_scan.n_channels = 3;

	} else {
		internal_scan.channels = NULL;
		internal_scan.n_channels = 0;
	}

	if (Sstar_internal_cmd_scan_triger(sdata, &internal_scan) == false) {
		ret = -EOPNOTSUPP;
		goto exit;
	}

	memcpy(&user_pos, &msg->externData[50], sizeof(void *));

	req.flush = true;
	req.n_stas = 0;
	req.priv = recv_info;
	req.result_handle = Sstar_dev_handle_scan_sta;

	ieee80211_scan_internal_req_results(sdata->local, &req);

	if (internal_scan.n_channels == 1)
		scan_result_filter_single_channel(recv_info, scan_ch);

	if (copy_to_user(user_pos, recv_info,
			 MAC_FILTER_NUM * sizeof(Wifi_Recv_Info_t)) != 0) {
		ret = -EINVAL;
	}
exit:

	if (internal_scan.macs)
		Sstar_kfree(internal_scan.macs);
	if (recv_info) {
		Sstar_kfree(recv_info);
	}
	return ret;
}
static int Sstar_dev_set_freq(struct net_device *dev, struct altm_wext_msg *msg)
{
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_internal_set_freq_req req;
	int ret = 0;
	unsigned short channel = 0;
	int freq;

	if (!ieee80211_sdata_running(sdata)) {
		ret = -ENETDOWN;
		goto exit;
	}

	memcpy(&channel, &msg->externData[0], sizeof(unsigned short));
	memcpy(&freq, &msg->externData[2], sizeof(int));

	req.channel_num = channel;
	req.freq = freq;
	req.set = true;

	if (Sstar_internal_freq_set(&sdata->local->hw, &req) == false) {
		ret = -EINVAL;
	}

	if (ret == 0)
		Sstar_set_freq(sdata->local);
exit:
	return ret;
}
static int Sstar_dev_set_special_oui(struct net_device *dev,
				     struct altm_wext_msg *msg)
{
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_sub_if_data *sdata_update;

	int ret = 0;
	char *special = NULL;
	int len = 0;

	if (!ieee80211_sdata_running(sdata)) {
		ret = -ENETDOWN;
		goto exit;
	}

	len = strlen(msg->externData);

	if ((len < 0) || (len > 255)) {
		ret = -EINVAL;
		goto exit;
	}

	special = Sstar_kzalloc(len, GFP_KERNEL);

	if (special == NULL) {
		ret = -EINVAL;
		goto exit;
	}
	memcpy(special, msg->externData, len);

	{
		//  struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
		struct Sstar_vif *priv =
			(struct Sstar_vif *)sdata->vif.drv_priv;
		Sstar_set_special_oui(priv->hw_priv, msg->externData, len);
	}

	list_for_each_entry (sdata_update, &local->interfaces, list) {
		bool res = true;

		if (!ieee80211_sdata_running(sdata_update)) {
			continue;
		}

		if (sdata_update->vif.type == NL80211_IFTYPE_STATION) {
			res = ieee80211_ap_update_special_probe_request(
				sdata_update, special, len);
		} else if ((sdata_update->vif.type == NL80211_IFTYPE_AP) &&
			   (rtnl_dereference(sdata_update->u.ap.beacon))) {
			res = ieee80211_ap_update_special_beacon(sdata_update,
								 special, len);
			if (res == true) {
				res = ieee80211_ap_update_special_probe_response(
					sdata_update, special, len);
			}
		}
		if (res == false) {
			ret = -EOPNOTSUPP;
			goto exit;
		}
	}
exit:
	if (special)
		Sstar_kfree(special);
	return ret;
}

static int Sstar_dev_set_sta_dis(struct net_device *dev,
				 struct altm_wext_msg *msg)
{
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct Sstar_vif *priv = (struct Sstar_vif *)sdata->vif.drv_priv;

	dev_printk("%s\n", __func__);

	ieee80211_connection_loss(priv->vif);
	return 0;
}

static int Sstar_dev_set_iftype(struct net_device *dev,
				struct altm_wext_msg *msg)
{
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_internal_iftype_req req;

	dev_printk("%s\n", __func__);

	req.if_type = (enum ieee80211_internal_iftype)msg->externData[0];
	req.channel = msg->externData[1];

	if (Sstar_internal_cmd_req_iftype(sdata, &req) == false)
		return -1;

	return 0;
}
/*
msg.type:
	1 filter_frame

	2 filter_ie

	3 filter clean

	4 filter show

msg.value:
	filter_frame : 80 or 40
	filter_ie	 : ie

msg.externData:
	filter_ie	 : oui1 oui2 pui3
*/
#ifdef CONFIG_IEEE80211_SPECIAL_FILTER

enum SPECIAL_FILTER_TYPE {
	FILTER_FRAME = 1,
	FILTER_IE = 2,
	FILTER_CLEAR = 3,
	FILTER_SHOW = 4,
};
static int Sstar_dev_set_special_filter(struct net_device *dev,
					struct altm_wext_msg *msg)
{
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_special_filter filter;
	struct ieee80211_special_filter_table *tables = NULL;
	char *results = NULL;
	int copy_len = 0;
	int total_len = 0, i = 0;
	int ret = -1;

	memset(&filter, 0, sizeof(struct ieee80211_special_filter));
	switch (msg->value) {
	case FILTER_FRAME: {
		filter.filter_action = (msg->externData[0] / 10 * 16) +
				       (msg->externData[0] % 10);
		filter.flags = SPECIAL_F_FLAGS_FRAME_TYPE;
		Sstar_printk_err("%s:action(%x)\n", __func__,
				 filter.filter_action);
		ret = ieee80211_special_filter_register(sdata, &filter);
	} break;
	case FILTER_IE: {
		filter.filter_action = msg->externData[0];
		filter.flags = SPECIAL_F_FLAGS_FRAME_IE;

		if (msg->externData[1] > 0 || msg->externData[2] > 0 ||
		    msg->externData[3] > 0) {
			filter.oui[0] = msg->externData[1];
			filter.oui[1] = msg->externData[2];
			filter.oui[2] = msg->externData[3];
			filter.flags |= SPECIAL_F_FLAGS_FRAME_OUI;
		}
		Sstar_printk_err("%s:ie[%d],oui[%d:%d:%d]\n", __func__,
				 filter.filter_action, filter.oui[0],
				 filter.oui[1], filter.oui[2]);
		ret = ieee80211_special_filter_register(sdata, &filter);
	} break;
	case FILTER_CLEAR: {
		ret = ieee80211_special_filter_clear(sdata);
	} break;
	case FILTER_SHOW: {
		results = Sstar_kzalloc(255, GFP_KERNEL);

		if (results == NULL) {
			ret = -ENOMEM;
			Sstar_printk_err(
				"FILTER_SHOW :results malloc err! \n ");
			goto exit;
		}
		tables = Sstar_kzalloc(
			sizeof(struct ieee80211_special_filter_table),
			GFP_KERNEL);

		if (tables == NULL) {
			ret = -ENOMEM;
			Sstar_printk_err("FILTER_SHOW :tables malloc err! \n ");
			goto exit;
		}
		ret = ieee80211_special_filter_request(sdata, tables);

		copy_len = scnprintf(results + total_len, 255 - total_len,
				     "filter table --->\n");
		total_len += copy_len;

		for (i = 0; i < tables->n_filters; i++) {
			if ((tables->table[i].flags &
			     IEEE80211_SPECIAL_FILTER_MASK) ==
			    SPECIAL_F_FLAGS_FRAME_TYPE)
				copy_len = scnprintf(
					results + total_len, 255 - total_len,
					"filter[%d]: frame [%x]\n", i,
					tables->table[i].filter_action);
			else if ((tables->table[i].flags &
				  IEEE80211_SPECIAL_FILTER_MASK) ==
				 SPECIAL_F_FLAGS_FRAME_IE)
				copy_len = scnprintf(
					results + total_len, 255 - total_len,
					"filter[%d]: ie[%d]\n", i,
					tables->table[i].filter_action);
			else if ((tables->table[i].flags &
				  IEEE80211_SPECIAL_FILTER_MASK) ==
				 (SPECIAL_F_FLAGS_FRAME_IE |
				  SPECIAL_F_FLAGS_FRAME_OUI)) {
				copy_len = scnprintf(
					results + total_len, 255 - total_len,
					"filter[%d]: ie[%d] oui[%d:%d:%d]\n", i,
					tables->table[i].filter_action,
					tables->table[i].oui[0],
					tables->table[i].oui[1],
					tables->table[i].oui[2]);
			} else {
				copy_len = scnprintf(results + total_len,
						     255 - total_len,
						     "filter[%d]: unkown\n", i);
			}
			if (copy_len > 0) {
				total_len += copy_len;
				memcpy(msg->externData, results, total_len);
			}
		}
	} break;
	default: {
		Sstar_printk_err("msg->value[%d] data err! \n", msg->value);
		ret = -ENOMEM;
	} break;
	}
exit:
	if (results)
		Sstar_kfree(results);
	if (tables)
		Sstar_kfree(tables);
	return ret;
}
#endif
static int Sstar_dev_set_adaptive(struct net_device *dev,
				  struct altm_wext_msg *msg)
{
	int ret = 0;
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct Sstar_vif *priv = (struct Sstar_vif *)sdata->vif.drv_priv;
	struct Sstar_common *hw_priv = priv->hw_priv;

	int adaptive;
	char cmd[32];

	dev_printk("%s\n", __func__);

	/*
    0: disable
    1: enable
    */
	//adaptive = msg->value;
	memcpy(&adaptive, msg->externData, sizeof(int));

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "set_adaptive %d ", adaptive);

	dev_printk("Sstar: %s\n", cmd);
	ret = wsm_write_mib(hw_priv, WSM_MIB_ID_FW_CMD, cmd, strlen(cmd),
			    priv->if_id);
	if (ret < 0) {
	}

	return ret;
}
static int Sstar_dev_set_txpwr_dcxo(struct net_device *dev,
				    struct altm_wext_msg *msg)
{
	int ret = 0;
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct Sstar_vif *priv = (struct Sstar_vif *)sdata->vif.drv_priv;
	struct Sstar_common *hw_priv = priv->hw_priv;

	int txpwr_L, txpwr_M, txpwr_H;
	int dcxo;
	char cmd[32];

	dev_printk("%s\n", __func__);

	memcpy(&txpwr_L, &msg->externData[0], 4); /*detla gain & txpwr*/
	if (txpwr_L > 32 || txpwr_L < -32) {
		Sstar_printk_err("error, txpwr_L %d\n", txpwr_L);
		return -1;
	}

	memcpy(&txpwr_M, &msg->externData[4], 4); /*detla gain & txpwr*/
	if (txpwr_M > 32 || txpwr_M < -32) {
		Sstar_printk_err("error, txpwr_M %d\n", txpwr_M);
		return -1;
	}

	memcpy(&txpwr_H, &msg->externData[8], 4); /*detla gain & txpwr*/
	if (txpwr_H > 32 || txpwr_H < -32) {
		Sstar_printk_err("error, txpwr_H %d\n", txpwr_H);
		return -1;
	}

	memcpy(&dcxo, &msg->externData[12], 4);
	if (dcxo > 127 || dcxo < 0) {
		Sstar_printk_err("error, dcxo %d\n", dcxo);
		return -1;
	}

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "set_txpwr_and_dcxo,%d,%d,%d,%d ", txpwr_L, txpwr_M,
		txpwr_H, dcxo);

	dev_printk("Sstar: %s\n", cmd);
	ret = wsm_write_mib(hw_priv, WSM_MIB_ID_FW_CMD, cmd, strlen(cmd),
			    priv->if_id);
	if (ret < 0) {
	}

	return ret;
}

int Sstar_dev_set_txpwr(struct net_device *dev, struct altm_wext_msg *msg)
{
	int ret = 0;
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct Sstar_vif *priv = (struct Sstar_vif *)sdata->vif.drv_priv;
	struct Sstar_common *hw_priv = priv->hw_priv;

	int txpwr_indx = 0;
	char cmd[32];

	dev_printk("%s\n", __func__);

	// memcpy(&txpwr_indx, &msg->externData[0], sizeof(int));
	txpwr_indx = msg->externData[0];

	if (txpwr_indx > 127) {
		txpwr_indx = txpwr_indx - 256;
	}

	if (txpwr_indx > 16 || txpwr_indx < -16) {
		dev_printk("txpwr_indx = %d , super range\n", txpwr_indx);
		return -1;
	}

	memset(cmd, 0, sizeof(cmd));

	/*
    *0,3,15,63
    */
	sprintf(cmd, "set_rate_txpower_mode %d ", txpwr_indx);

	Sstar_printk_err("Sstar: %s , %d,txpwr_indx = %d\n", cmd, strlen(cmd),
			 txpwr_indx);
	ret = wsm_write_mib(hw_priv, WSM_MIB_ID_FW_CMD, cmd, strlen(cmd),
			    priv->if_id);
	if (ret < 0) {
	}

	return ret;
}
static int Sstar_dev_get_work_channel(struct net_device *dev,
				      struct altm_wext_msg *msg)
{
	unsigned short channel = 0;
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_channel_state *chan_state = NULL;
	struct ieee80211_sub_if_data *sdata_update = NULL;
	mutex_lock(&local->mtx);
	switch (sdata->vif.type) {
	case NL80211_IFTYPE_STATION: {
		if (Sstar_wifi_status_get() == 1) {
			chan_state = ieee80211_get_channel_state(local,
								 sdata_update);
			if (chan_state)
				channel = channel_hw_value(
					chan_state->oper_channel);
			else {
				channel = 0;
			}
		} else {
#ifdef CONFIG_SSTAR_STA_LISTEN
			if (local->listen_channel)
				channel =
					channel_hw_value(local->listen_channel);
			else
				channel = 0;
#endif
		}
	} break;
	case NL80211_IFTYPE_AP: {
		chan_state = ieee80211_get_channel_state(local, sdata_update);
		if (chan_state)
			channel = channel_hw_value(chan_state->oper_channel);
		else
			channel = 0;
	} break;
	case NL80211_IFTYPE_MONITOR: {
		Sstar_printk_err("NL80211_IFTYPE_MONITOR \n");
		chan_state = ieee80211_get_channel_state(local, sdata_update);
		if (chan_state)
			channel = channel_hw_value(chan_state->oper_channel);
		else
			channel = 0;
	} break;
	default:
		Sstar_printk_err("other type(%d) \n", sdata->vif.type);
		break;
	}

	mutex_unlock(&local->mtx);
	Sstar_printk_err("current work channel is [%d] \n", channel);
	//msg->value = hw_priv->channel->hw_value;
	memcpy(&msg->externData[0], &channel, sizeof(unsigned short));

	return 0;
}

static int Sstar_dev_set_best_channel_scan(struct net_device *dev,
					   struct altm_wext_msg *msg)
{
	int ret = 0;
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_internal_channel_auto_select_req req;
	struct ieee80211_internal_channel_auto_select_results results;
	Best_Channel_Scan_Result scan_result;
	u8 i = 0, j = 0, k = 0, start_channel, end_channel;
	u8 all_channels[18] = { 1,  2,	3,  4,	5,  6,	7,  8,	9,
				10, 11, 12, 13, 14, 36, 38, 40, 42 };
	//	bool support_special=false;
	u8 all_n_channels = 0;
	u8 *ignor_channels = NULL, *vaid_channels = NULL;
	if (!ieee80211_sdata_running(sdata)) {
		ret = -ENETDOWN;
		goto exit;
	}
	if (ieee8011_channel_valid(&local->hw, 36) == true) {
		//		support_special = true;
		all_n_channels = 18;
		Sstar_printk_err("support_special! \n");
	} else {
		//		support_special = false;
		all_n_channels = 14;
		Sstar_printk_err("not support_special! \n");
	}
	if (sdata->vif.type != NL80211_IFTYPE_STATION) {
		ret = -EOPNOTSUPP;
		goto exit;
	}
	memset(&req, 0,
	       sizeof(struct ieee80211_internal_channel_auto_select_req));

	if (Sstar_internal_channel_auto_select(sdata, &req) == false) {
		ret = -EOPNOTSUPP;
	}

	memset(&scan_result, 0, sizeof(Best_Channel_Scan_Result));
	memset(&results, 0,
	       sizeof(struct ieee80211_internal_channel_auto_select_results));
	results.version = 0; //use version 0

	/*
		start_channel
		end_channel
		2.4G & 5G
		if support 5G but not confine channel, default return 1~14 channel value , suggest_ch range 1~14

	*/
	start_channel = msg->externData[0];
	end_channel = msg->externData[1];
	//Determine channel validity

	for (i = 0; i < 18; i++) {
		if (all_channels[i] == start_channel) {
			j = i + 1; //start channel valid
		} else if ((all_channels[i] == end_channel) && (i > j)) {
			k = i + 1; //end_channel valid
		}
	}
	Sstar_printk_err("start_channel index[%d] end_index[%d] \n", j, k);
	// channel valid
	if ((j != 0) && (k != 0)) {
		results.ignore_n_channels = all_n_channels - (k - j + 1);
		ignor_channels = (u8 *)Sstar_kmalloc(results.ignore_n_channels,
						     GFP_KERNEL);
		if (ignor_channels == NULL) {
			ret = false;
			goto exit;
		}
		results.n_channels = k - j + 1;
		vaid_channels =
			(u8 *)Sstar_kmalloc(results.n_channels, GFP_KERNEL);
		if (vaid_channels == NULL) {
			ret = false;
			goto exit;
		}
		j = 0;
		k = 0;
		for (i = 0; i < all_n_channels; i++) {
			if ((all_channels[i] < start_channel) ||
			    (all_channels[i] > end_channel)) {
				ignor_channels[j++] = all_channels[i];

				Sstar_printk_err("ignor_channels[%d] : %d \n",
						 j - 1, ignor_channels[j - 1]);
			} else {
				vaid_channels[k++] = all_channels[i];

				Sstar_printk_err("vaid_channels[%d] : %d \n",
						 k - 1, vaid_channels[k - 1]);
			}
		}
		results.ignore_channels = ignor_channels;
		//results.ignore_n_channels = all_n_channels - (end_channel - start_channel + 1);
		results.channels = vaid_channels;
		//results.n_channels = end_channel - start_channel + 1;

	} else {
		start_channel = 1;
		end_channel = 14;
		if (all_n_channels == 18) {
			ignor_channels = (u8 *)Sstar_kmalloc(
				all_n_channels - 14, GFP_KERNEL);
			if (ignor_channels == NULL) {
				ret = false;
				goto exit;
			}
		}

		vaid_channels = (u8 *)Sstar_kmalloc(14, GFP_KERNEL);
		if (vaid_channels == NULL) {
			ret = false;
			goto exit;
		}
		j = 0;
		k = 0;
		for (i = 0; i < all_n_channels; i++) {
			if ((all_channels[i] < start_channel) ||
			    (all_channels[i] > end_channel)) {
				if (all_n_channels == 18) {
					ignor_channels[j++] = all_channels[i];

					Sstar_printk_err(
						"ignor_channels[%d] : %d \n",
						j - 1, ignor_channels[j - 1]);
				}
			} else {
				vaid_channels[k++] = all_channels[i];

				Sstar_printk_err("vaid_channels[%d] : %d \n",
						 k - 1, vaid_channels[k - 1]);
			}
		}
		results.ignore_channels = ignor_channels;
		results.ignore_n_channels = all_n_channels - 14;
		results.channels = vaid_channels;
		results.n_channels = 14;
	}

	Sstar_printk_err(
		"[%s] start channel[%d] end channel[%d] results.ignore_n_channels[%d] results.n_channels[%d]\n",
		__func__, start_channel, end_channel, results.ignore_n_channels,
		results.n_channels);

	if (Sstar_internal_channel_auto_select_results(sdata, &results) ==
	    false) {
		ret = -EINVAL;
		goto exit;
	}

	for (i = 0; i < all_n_channels; i++) {
		scan_result.channel_ap_num[i] = results.n_aps[i];
		scan_result.busy_ratio[i] = results.busy_ratio[i];
		scan_result.weight[i] = results.weight[i];
	}
	scan_result.suggest_ch = results.susgest_channel;

	Sstar_printk_err("auto_select channel %d\n", scan_result.suggest_ch);
	memcpy(msg->externData, &scan_result, sizeof(scan_result));
exit:
	if (results.ignore_channels)
		Sstar_kfree(results.ignore_channels);
	if (results.channels)
		Sstar_kfree(results.channels);
	return ret;
}
static bool
Sstar_dev_handle_ap_list(struct ieee80211_hw *hw,
			 struct Sstar_internal_scan_results_req *req,
			 struct ieee80211_internal_scan_sta *sta)
{
	BEST_CHANNEL_INFO *info = (BEST_CHANNEL_INFO *)req->priv;
	BEST_CHANNEL_INFO *pos_info = NULL;
	int i = 0;

	if (req->n_stas >= CHANNEL_NUM * AP_SCAN_NUM_MAX) {
		return false;
	}

	if (sta->channel > 14) {
		return false;
	}

	pos_info = info + (sta->channel - 1) * AP_SCAN_NUM_MAX;

	for (i = 0; i < AP_SCAN_NUM_MAX; i++) {
		if (pos_info->flag == 1) {
			pos_info++;
			continue;
		}

		req->n_stas++;
		pos_info->enc_type = (u8)sta->enc_type;
		pos_info->enc_type_name = sta->ieee80211_enc_type_name;
		pos_info->rssi = sta->signal;
		memcpy(pos_info->mac_addr, sta->bssid, 6);
		if (sta->ssid_len)
			memcpy(pos_info->ssid, sta->ssid, sta->ssid_len);
		pos_info->flag = 1;

		break;
	}

	return true;
}

int Sstar_dev_get_ap_list(struct net_device *dev, struct altm_wext_msg *msg)
{
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	int ret = 0;
	struct ieee80211_internal_scan_request internal_scan;
	struct Sstar_internal_scan_results_req req;
	u8 *user_pos = NULL;
	u8 *recv_info = NULL;

	memset(&internal_scan, 0,
	       sizeof(struct ieee80211_internal_scan_request));

	if (!ieee80211_sdata_running(sdata)) {
		ret = -ENETDOWN;
		goto exit;
	}

	if (sdata->vif.type != NL80211_IFTYPE_STATION) {
		ret = -EOPNOTSUPP;
		goto exit;
	}
	recv_info = Sstar_kzalloc(sizeof(BEST_CHANNEL_INFO) * CHANNEL_NUM *
					  AP_SCAN_NUM_MAX,
				  GFP_KERNEL);
	if (recv_info == NULL) {
		ret = -ENOMEM;
		goto exit;
	}

	if (Sstar_internal_cmd_scan_triger(sdata, &internal_scan) == false) {
		ret = -EOPNOTSUPP;
		goto exit;
	}

	memcpy(&user_pos, &msg->externData[0], sizeof(void *));

	req.flush = true;
	req.n_stas = 0;
	req.priv = recv_info;
	req.result_handle = Sstar_dev_handle_ap_list;

	ieee80211_scan_internal_req_results(sdata->local, &req);

	if (copy_to_user(user_pos, recv_info,
			 sizeof(BEST_CHANNEL_INFO) * CHANNEL_NUM *
				 AP_SCAN_NUM_MAX) != 0) {
		ret = -EINVAL;
	}
exit:

	if (internal_scan.macs)
		Sstar_kfree(internal_scan.macs);
	if (recv_info) {
		Sstar_kfree(recv_info);
	}
	return ret;
}
int Sstar_dev_get_tp_rate(struct net_device *dev, struct altm_wext_msg *msg)
{
	int ret = 0;
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct Sstar_vif *priv = (struct Sstar_vif *)sdata->vif.drv_priv;
	struct Sstar_common *hw_priv = priv->hw_priv;
	char mac_addr[6];
	int sta_id = 0;
	unsigned int rate_val = 0;

	dev_printk("%s sdata->vif.type = %d NL80211_IFTYPE_STATION = %d \n",
		   __func__, sdata->vif.type, NL80211_IFTYPE_STATION);

	//ap mode
	if (sdata->vif.type != NL80211_IFTYPE_STATION) {
		//clear mac addr buffer
		memset(mac_addr, 0, 6);

		//convert mac string to mac hex format
		str2mac(mac_addr, msg->externData);

		//according to mac hex, find out sta link id
		sta_id = Sstar_find_link_id(priv, mac_addr);

		Sstar_printk_wext("sta_id %d\n", sta_id);
		wsm_write_mib(hw_priv, WSM_MIB_ID_GET_RATE, &sta_id, 1,
			      priv->if_id);
	}

	wsm_read_mib(hw_priv, WSM_MIB_ID_GET_RATE, &rate_val,
		     sizeof(unsigned int), priv->if_id);

	//convert bits/s
	rate_val = rate_val / 2;
	Sstar_printk_wext("rate: %d bits/s\n", rate_val);

	memcpy(&msg->externData[0], &rate_val, sizeof(unsigned int));

	return ret;
}
extern void etf_param_init(struct Sstar_common *hw_priv);
int Sstar_dev_etf_test(struct net_device *dev, struct altm_wext_msg *msg)
{
	int i = 0;
	u8 chipid = 0;
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct Sstar_vif *priv = (struct Sstar_vif *)sdata->vif.drv_priv;
	struct Sstar_common *hw_priv = ABwifi_vifpriv_to_hwpriv(priv);

	etf_param_init(hw_priv);
	chipid = GetChipVersion(hw_priv);

	ucWriteEfuseFlag = msg->value;
	Sstar_printk_always("ucWriteEfuseFlag:%d\n", ucWriteEfuseFlag);

	etf_config.freq_ppm = 7000;
	etf_config.rssifilter = -100;
	etf_config.txevm = 400;
	etf_config.txevmthreshold = 400;
	if (chipid == 0x24) //athenaB(101A)
	{
		etf_config.rxevm = 400;
		etf_config.rxevmthreshold = 400;
	} else if (chipid == 0x49) //AresB(101B)
	{
		etf_config.rxevm = 25;
		etf_config.rxevmthreshold = 25;
	}
	etf_config.cableloss = 30 * 4;
	etf_config.featureid = MyRand();

	Sstar_printk_always("featureid:%d\n", etf_config.featureid);
	Sstar_printk_always(
		"Freq:%d,txEvm:%d,rxEvm:%d,txevmthreshold:%d,rxevmthreshold:%d,Txpwrmax:%d,Txpwrmin:%d,Rxpwrmax:%d,Rxpwrmin:%d,rssifilter:%d,cableloss:%d,default_dcxo:%d\n",
		etf_config.freq_ppm, etf_config.txevm, etf_config.rxevm,
		etf_config.txevmthreshold, etf_config.rxevmthreshold,
		etf_config.txpwrmax, etf_config.txpwrmin, etf_config.rxpwrmax,
		etf_config.rxpwrmin, etf_config.rssifilter,
		etf_config.cableloss, etf_config.default_dcxo);

	hw_priv->etf_channel = 7;
	hw_priv->etf_channel_type = 0;
	hw_priv->etf_rate = 21;
	hw_priv->etf_len = 1000;
	hw_priv->etf_greedfiled = 0;

	Sstar_for_each_vif(hw_priv, priv, i)
	{
		if ((priv != NULL)) {
			Sstar_printk_wext("device ioctl etf test\n");
			down(&hw_priv->scan.lock);
			mutex_lock(&hw_priv->conf_mutex);
			//ETF_bStartTx = 1;
			wsm_start_tx_v2(hw_priv, priv->vif);

			mutex_unlock(&hw_priv->conf_mutex);
			break;
		}
	}

	return 0;
}

int Sstar_dev_etf_get_result(struct net_device *dev, struct altm_wext_msg *msg)
{
	int ret = 0;
	u8 chipid = 0;
	char *buff = NULL;
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_local *local = sdata->local;
	struct Sstar_common *hw_priv = local->hw.priv;

	chipid = GetChipVersion(hw_priv);

	if (!(buff = (char *)Sstar_kzalloc(256, GFP_KERNEL))) {
		return -ENOMEM;
	}
	memset(buff, 0, 256);

	sprintf(buff,
		"%dcfo:%d,txevm:%d,rxevm:%d,dcxo:%d,txrssi:%d,rxrssi:%d,result:%d (0:OK; -1:FreqOffset Error; -2:efuse hard error;"
		" -3:efuse no written; -4:efuse anaysis failed; -5:efuse full; -6:efuse version change; -7:rx null)",
		chipid, gRxs_s.Cfo, gRxs_s.txevm, gRxs_s.evm, gRxs_s.dcxo,
		gRxs_s.TxRSSI, gRxs_s.RxRSSI, gRxs_s.result);

	if (&msg->externData[0] != NULL) {
		memcpy(&msg->externData[0], buff, strlen(buff));
	} else {
		Sstar_printk_always("error:msg->externData[0] is null\n");
	}

	if (buff)
		Sstar_kfree(buff);

	return ret;
}
/*

channel = msg->externData[0];
band_value = msg->externData[1~2];
len = msg->externData[3~4];
is_40M = msg->externData[5];
greedfiled = msg->externData[6];

*/
static int Sstar_dev_stop_tx(struct net_device *dev, struct altm_wext_msg *msg);

static int Sstar_dev_start_tx(struct net_device *dev, struct altm_wext_msg *msg)
{
	int i = 0;
	int ret = 0;
	int len = 0;
	int channel = 0;
	u32 rate = 0;
	u32 is_40M = 0;
	int band_value = 0;
	int greedfiled = 0;
	u8 ucDbgPrintOpenFlag = 1;
	short *len_p;
	//	char *extra = NULL;
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_local *local = sdata->local;
	struct Sstar_common *hw_priv = local->hw.priv;
	struct Sstar_vif *vif;

	if (ETF_bStartTx || ETF_bStartRx) {
		if (ETF_bStartTx) {
			Sstar_dev_stop_tx(dev, NULL);
			msleep(500);
		} else {
			Sstar_printk_err(
				"Error! already start_tx, please stop_rx first!\n");
			return 0;
		}
	}
	if (strlen(msg->externData) == 0) {
		Sstar_printk_err("Invalid parameters\n");
		return 0;
	}
	/*
	if(!(extra = Sstar_kmalloc(strlen(msg->externData), GFP_KERNEL)))
		return -ENOMEM;

	memcpy(extra, msg->externData, strlen(msg->externData));
	Sstar_printk_err("%s\n", extra);

	sscanf(extra, "%d %d %d %d %d", &channel, &band_value, &len, &is_40M, &greedfiled);
*/
	channel = msg->externData[0];
	len_p = (short *)&msg->externData[1];
	band_value = *len_p;
	len_p = (short *)&msg->externData[3];
	len = *len_p;
	is_40M = msg->externData[5];
	greedfiled = msg->externData[6];

	Sstar_printk_err(
		"Sstar_dev_start_tx:channel[%d],rate[%d].len[%d],is_40M[%d],greedfiled[%d]\n",
		channel, band_value, len, is_40M, greedfiled);

	if (channel < 0 || channel > 14) {
		Sstar_printk_err("invalid channel[%d]!\n", channel);
		ret = -EINVAL;
		goto exit;
	}
	if (is_40M)
		is_40M = 1;
	else
		is_40M = 0;

	//check rate
	switch (band_value) {
	case 10:
		rate = WSM_TRANSMIT_RATE_1;
		break;
	case 20:
		rate = WSM_TRANSMIT_RATE_2;
		break;
	case 55:
		rate = WSM_TRANSMIT_RATE_5;
		break;
	case 110:
		rate = WSM_TRANSMIT_RATE_11;
		break;
	case 60:
		rate = WSM_TRANSMIT_RATE_6;
		break;
	case 90:
		rate = WSM_TRANSMIT_RATE_9;
		break;
	case 120:
		rate = WSM_TRANSMIT_RATE_12;
		break;
	case 180:
		rate = WSM_TRANSMIT_RATE_18;
		break;
	case 240:
		rate = WSM_TRANSMIT_RATE_24;
		break;
	case 360:
		rate = WSM_TRANSMIT_RATE_36;
		break;
	case 480:
		rate = WSM_TRANSMIT_RATE_48;
		break;
	case 540:
		rate = WSM_TRANSMIT_RATE_54;
		break;
	case 65:
		rate = WSM_TRANSMIT_RATE_HT_6;
		break;
	case 130:
		rate = WSM_TRANSMIT_RATE_HT_13;
		break;
	case 195:
		rate = WSM_TRANSMIT_RATE_HT_19;
		break;
	case 260:
		rate = WSM_TRANSMIT_RATE_HT_26;
		break;
	case 390:
		rate = WSM_TRANSMIT_RATE_HT_39;
		break;
	case 520:
		rate = WSM_TRANSMIT_RATE_HT_52;
		break;
	case 585:
		rate = WSM_TRANSMIT_RATE_HT_58;
		break;
	case 650:
		rate = WSM_TRANSMIT_RATE_HT_65;
		break;
	default: {
		Sstar_printk_err("invalid rate!\n");
		ret = -EINVAL;
		goto exit;
	}
	}

	Sstar_printk_err("rate:%d\n", rate);

	if (is_40M == 1) {
		is_40M = NL80211_CHAN_HT40PLUS; //
		channel -= 2;
	}

	Sstar_printk_wext("NL80211_CHAN_HT40PLUS:%d\n", NL80211_CHAN_HT40PLUS);

	//printk("%d, %d, %d, %d\n", channel, rate, len, is_40M);
	hw_priv->etf_channel = channel;
	hw_priv->etf_channel_type = is_40M;
	hw_priv->etf_rate = rate;
	hw_priv->etf_len = len;
	hw_priv->etf_greedfiled = greedfiled;

	Sstar_for_each_vif(hw_priv, vif, i)
	{
		if ((vif != NULL)) {
			Sstar_printk_wext("*******\n");

			down(&hw_priv->scan.lock);
			WARN_ON(wsm_write_mib(
				hw_priv, WSM_MIB_ID_DBG_PRINT_TO_HOST,
				&ucDbgPrintOpenFlag, sizeof(ucDbgPrintOpenFlag),
				vif->if_id));
			mutex_lock(&hw_priv->conf_mutex);

			ETF_bStartTx = 1;
			if (wsm_start_tx(hw_priv, vif->vif) != 0) {
				up(&hw_priv->scan.lock);
			}

			mutex_unlock(&hw_priv->conf_mutex);
			break;
		}
	}
exit:
	return ret;
}

static int Sstar_dev_stop_tx(struct net_device *dev, struct altm_wext_msg *msg)
{
	int i = 0;
	int ret = 0;
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_local *local = sdata->local;
	struct Sstar_common *hw_priv = local->hw.priv;
	struct Sstar_vif *vif;

	if (0 == ETF_bStartTx) {
		Sstar_printk_err("please start start_rx first,then stop_tx\n");
		return -EINVAL;
	}

	mutex_lock(&hw_priv->conf_mutex);
	ETF_bStartTx = 0;
	mutex_unlock(&hw_priv->conf_mutex);

	Sstar_for_each_vif(hw_priv, vif, i)
	{
		if ((vif != NULL)) {
			wsm_oper_unlock(hw_priv);
			wsm_stop_tx(hw_priv);
			wsm_stop_scan(hw_priv, i);
			up(&hw_priv->scan.lock);
		}
	}

	return ret;
}
/*
channel ----- msg->externData[0]
is_40M ------ msg->externData[1]

*/
static int Sstar_dev_stop_rx(struct net_device *dev, struct altm_wext_msg *msg);

static int Sstar_dev_start_rx(struct net_device *dev, struct altm_wext_msg *msg)
{
	int i = 0;
	int ret = 0;
	char cmd[20] = "monitor 1 ";
	u8 ucDbgPrintOpenFlag = 1;
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_local *local = sdata->local;
	struct Sstar_common *hw_priv = local->hw.priv;
	struct Sstar_vif *vif;
	int channel, is_40M;
	if (ETF_bStartTx || ETF_bStartRx) {
		if (ETF_bStartRx) {
			Sstar_printk_err(
				"start rx : %s ,stop now and change chan[%d],is_40M[%d]\n",
				ch_and_type, msg->externData[0],
				msg->externData[1]);
			Sstar_dev_stop_rx(dev, NULL);
			msleep(500);
		} else {
			Sstar_printk_err(
				"Error! already ETF_bStartRx, please stop_tx first!\n");
			return 0;
		}
	}

	if (strlen(msg->externData) == 0) {
		Sstar_printk_err("Invalid parameters\n");
		return 0;
	}
	channel = msg->externData[0];
	is_40M = msg->externData[1];
	if (channel < 0 || channel > 14) {
		Sstar_printk_err("channel[%d] not support \n", channel);
		return -EINVAL;
	}
	if (is_40M)
		is_40M = 1;
	else
		is_40M = 0;
	//./iwpriv wlan0 fwdbg 1
	Sstar_for_each_vif(hw_priv, vif, i)
	{
		if (vif != NULL) {
			WARN_ON(wsm_write_mib(
				hw_priv, WSM_MIB_ID_DBG_PRINT_TO_HOST,
				&ucDbgPrintOpenFlag, sizeof(ucDbgPrintOpenFlag),
				vif->if_id));
			break;
		}
	}
	sprintf(cmd, "monitor 1 %d %d", channel, is_40M);
	//memcpy(cmd+10, msg->externData, strlen(msg->externData)+1);
	memset(ch_and_type, 0, 20);
	//	memcpy(ch_and_type, msg->externData, strlen(msg->externData)+1);
	sprintf(ch_and_type, "%d %d", channel, is_40M);
	Sstar_printk_wext("CMD:%s,ch_and_type:%s\n", cmd, ch_and_type);
	i = 0;
	Sstar_for_each_vif(hw_priv, vif, i)
	{
		if (vif != NULL) {
			ETF_bStartRx = 1;
			WARN_ON(wsm_write_mib(hw_priv, WSM_MIB_ID_FW_CMD, cmd,
					      strlen(cmd) + 1, vif->if_id));
			break;
		}
	}

	return ret;
}

static int Sstar_dev_stop_rx(struct net_device *dev, struct altm_wext_msg *msg)
{
	int i = 0;
	int ret = 0;
	char cmd[20] = "monitor 0 ";
	u8 ucDbgPrintOpenFlag = 0;
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_local *local = sdata->local;
	struct Sstar_common *hw_priv = local->hw.priv;
	struct Sstar_vif *vif;

	if (0 == ETF_bStartRx) {
		Sstar_printk_err("please start start_rx first,then stop_rx\n");
		return -EINVAL;
	}

	ETF_bStartRx = 0;

	memcpy(cmd + 10, ch_and_type, strlen(ch_and_type));
	//printk("cmd %s\n", cmd);
	i = 0;
	Sstar_for_each_vif(hw_priv, vif, i)
	{
		if (vif != NULL) {
			WARN_ON(wsm_write_mib(hw_priv, WSM_MIB_ID_FW_CMD, cmd,
					      13, vif->if_id));
			break;
		}
	}

	Sstar_for_each_vif(hw_priv, vif, i)
	{
		if (vif != NULL) {
			WARN_ON(wsm_write_mib(
				hw_priv, WSM_MIB_ID_DBG_PRINT_TO_HOST,
				&ucDbgPrintOpenFlag, sizeof(ucDbgPrintOpenFlag),
				vif->if_id));
			break;
		}
	}
	return ret;
}

static int Sstar_dev_get_etf_rx_results(struct net_device *dev,
					struct altm_wext_msg *msg)
{
	u32 rx_status[3] = { 0, 0, 0 };
	//	int len = 0;
	int ret = 0;
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_local *local = sdata->local;
	struct Sstar_common *hw_priv = local->hw.priv;
	struct rx_results {
		u32 rxSuccess;
		u32 FcsErr;
		u32 PlcpErr;
	};
	struct rx_results rx_results_t;
	if (ETF_bStartRx == 0) {
		Sstar_printk_wext("%s:start rx not running\n", __func__);
		ret = -EOPNOTSUPP;
		goto exit;
	}

	ret = wsm_read_shmem(hw_priv, (u32)0x161001d8 /*RX_STATUS_ADDR*/,
			     rx_status, sizeof(rx_status));

	if (ret != 0) {
		ret = -EINVAL;
		goto exit;
	}

	rx_results_t.rxSuccess = rx_status[0] - rx_status[1];
	rx_results_t.FcsErr = rx_status[1];
	rx_results_t.PlcpErr = rx_status[2];
	memcpy(msg->externData, &rx_results_t, sizeof(struct rx_results));

exit:
	return ret;
}
#ifdef CONFIG_SSTAR_SUPPORT_AP_CONFIG

static int Sstar_dev_set_fix_scan_channel(struct net_device *dev,
					  struct altm_wext_msg *msg)
{
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_internal_ap_conf *conf_req;
	int ret = -1;

	conf_req = (struct ieee80211_internal_ap_conf *)msg->externData;

	if (!conf_req) {
		Sstar_printk_err(
			"Sstar_dev_set_fix_scan_channel : set fix scan channel error ! \n");
		return -1;
	}

	if (conf_req->channel < 0 || conf_req->channel > 14) {
		conf_req->channel = 0;
	}

	ret = Sstar_internal_update_ap_conf(
		sdata, conf_req, conf_req->channel == 0 ? true : false);

	return ret;
}

#endif

static int Sstar_dev_set_country_code(struct net_device *dev,
				      struct altm_wext_msg *msg)
{
#ifdef CONFIG_SSTAR_5G_PRETEND_2G
	Sstar_printk_err("this mode not support! \n");

	return 0;
#else
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_local *local = sdata->local;
	//unsigned char country_code[3] = {0};
	int ret = 0;

	if (!ieee80211_sdata_running(sdata)) {
		ret = -ENETDOWN;
		goto exit;
	}
	memcpy(local->country_code, &msg->externData[0], 2);
	Sstar_printk_err(
		"Sstar_dev_set_country_code:country_code = %c%c---------------\n",
		local->country_code[0], local->country_code[1]);
exit:
	return ret;
#endif
}

static int Sstar_dev_get_driver_version(struct net_device *dev,
					struct altm_wext_msg *msg)
{
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_local *local = sdata->local;
	struct Sstar_common *hw_priv = local->hw.priv;
	short *p;
	p = (short *)&msg->externData[0];
	Sstar_get_drv_version(p);
	p = (short *)&msg->externData[2];
	*p = (short)hw_priv->wsm_caps.firmwareVersion;

	return 0;
}
static int Sstar_dev_get_efuse(struct net_device *dev,
			       struct altm_wext_msg *msg)
{
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_local *local = sdata->local;
	struct Sstar_common *hw_priv = local->hw.priv;
	struct efuse_headr efuse_data;
	int ret = -1;

	if ((ret = wsm_get_efuse_data(hw_priv, &efuse_data,
				      sizeof(efuse_data))) == 0) {
		Sstar_printk_init(
			"Get efuse data is [%d,%d,%d,%d,%d,%d,%d,%d,%02x:%02x:%02x:%02x:%02x:%02x]\n",
			efuse_data.version, efuse_data.dcxo_trim,
			efuse_data.delta_gain1, efuse_data.delta_gain2,
			efuse_data.delta_gain3, efuse_data.Tj_room,
			efuse_data.topref_ctrl_bias_res_trim,
			efuse_data.PowerSupplySel, efuse_data.mac[0],
			efuse_data.mac[1], efuse_data.mac[2], efuse_data.mac[3],
			efuse_data.mac[4], efuse_data.mac[5]);
		memcpy(&hw_priv->efuse, &efuse_data,
		       sizeof(struct efuse_headr));
	} else {
		Sstar_printk_err("read efuse failed\n");
		return -1;
	}

	memcpy(msg->externData, &efuse_data, sizeof(struct efuse_headr));

	return 0;
}

static int Sstar_dev_set_cail_auto_ppm(struct net_device *dev,
				       struct altm_wext_msg *msg)
{
	char *ptr = NULL;
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_local *local = sdata->local;
	struct Sstar_common *hw_priv = local->hw.priv;
	int i = 0;
	struct Sstar_vif *vif;
	char buff_on[] = "cfo 1";
	char buff_off[] = "cfo 0";
	i = 0;

	if (msg->value == 0) {
		ptr = buff_off;
	} else {
		ptr = buff_on;
	}

	Sstar_for_each_vif(hw_priv, vif, i)
	{
		if (vif != NULL) {
			WARN_ON(wsm_write_mib(hw_priv, WSM_MIB_ID_FW_CMD, ptr,
					      5, vif->if_id));
			break;
		}
	}
	return 0;
}

static int Sstar_dev_get_vendor_special_ie(struct net_device *dev,
					   struct altm_wext_msg *msg)
{
	struct Sstar_vendor_cfg_ie *private_ie = NULL;

	private_ie = Sstar_internal_get_6441_vendor_ie();
	if (private_ie) {
		memcpy(msg->externData, private_ie,
		       sizeof(struct Sstar_vendor_cfg_ie));
	}

	return 0;
}

/*
	crystal_type :
		2 Ϊ������
		����ֵ��Ϊ��������

	ֻ�ж�������dcxo����Ч��������ppm����dcxo����

*/
extern u32 GetChipCrystalType(struct Sstar_common *hw_priv);
static int Sstar_dev_get_cail_ppm_results(struct net_device *dev,
					  struct altm_wext_msg *msg)
{
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_local *local = sdata->local;
	struct Sstar_common *hw_priv = local->hw.priv;
	u8 dcxo = 0;
	u8 crystal_type = 0;
	int cfo_val = 0;

	crystal_type = GetChipCrystalType(hw_priv);

	dcxo = DCXOCodeRead(hw_priv);

	wsm_get_cfo_ppm_correction_value(hw_priv, &cfo_val, sizeof(int));

	Sstar_printk_always("%s,dcxo:%d,cfo_val:%d ppm\n",
			    crystal_type == 2 ? "share crystal" :
						      "independent crystal",
			    dcxo, cfo_val);
	msg->externData[0] = crystal_type;
	msg->externData[1] = dcxo;
	msg->externData[2] = cfo_val;

	return 0;
}
static int
Sstar_dev_set_efuse_gain_compensation_value(struct net_device *dev,
					    struct altm_wext_msg *msg)
{
	int gain_for = 0;
	char delta_gain[3], deal_gain[3];
	int prv_deltagain;
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_local *local = sdata->local;
	struct Sstar_common *hw_priv = local->hw.priv;
	struct efuse_headr efuse_data;
	//	int compensation_data;
	int compensation_val = 0, ret, write_flag;
	char writebuf[128] = "";
	if ((ret = wsm_get_efuse_data(hw_priv, &efuse_data,
				      sizeof(efuse_data))) == 0) {
		Sstar_printk_init(
			"dcxo:%d,gain1:%d,gain2:%d,gain3:%d,[%02x:%02x:%02x:%02x:%02x:%02x]\n",
			efuse_data.dcxo_trim, efuse_data.delta_gain1,
			efuse_data.delta_gain2, efuse_data.delta_gain3,
			efuse_data.mac[0], efuse_data.mac[1], efuse_data.mac[2],
			efuse_data.mac[3], efuse_data.mac[4],
			efuse_data.mac[5]);
		memcpy(&hw_priv->efuse, &efuse_data,
		       sizeof(struct efuse_headr));
	} else {
		Sstar_printk_err("read efuse failed\n");
		return -1;
	}

	compensation_val = msg->externData[0];
	if (compensation_val > 128)
		compensation_val -= 256;
	Sstar_printk_err("compensation_val = %d\n", compensation_val);
	delta_gain[0] = hw_priv->efuse.delta_gain1;
	delta_gain[1] = hw_priv->efuse.delta_gain2;
	delta_gain[2] = hw_priv->efuse.delta_gain3;

	for (gain_for = 0; gain_for < 3; gain_for++) {
		prv_deltagain = delta_gain[gain_for];
		if (prv_deltagain < 16) { //����

			if (compensation_val >= 0) { // gain add
				prv_deltagain =
					prv_deltagain + compensation_val;
				if (prv_deltagain > 128)
					prv_deltagain -= 256;

				if (prv_deltagain > 15) {
					prv_deltagain = 15;
				}

			} else { //gain sub
				prv_deltagain =
					prv_deltagain + compensation_val;

				if (prv_deltagain > 128)
					prv_deltagain -= 256;

				if (prv_deltagain < 0) {
					prv_deltagain += 32;
					if (prv_deltagain < 17)
						prv_deltagain = 17;
				}
			}

		} else if (prv_deltagain >= 16) { //����

			if (compensation_val >= 0) { // gain add
				prv_deltagain += compensation_val;
				if (prv_deltagain > 128)
					prv_deltagain -= 256;

				if (prv_deltagain > 32) {
					prv_deltagain = (prv_deltagain - 32);
				}

			} else { //gain sub
				prv_deltagain += compensation_val;
				if (prv_deltagain > 128)
					prv_deltagain -= 256;
				if (prv_deltagain < 17) {
					prv_deltagain = 17;
				}
			}
		}
		if (prv_deltagain > 128)
			prv_deltagain -= 256;
		deal_gain[gain_for] = prv_deltagain;
		//		hw_priv->efuse.delta_gain1 = prv_deltagain1;
		Sstar_printk_err(
			" prv_deltagain = %d ,gain[%d] = %d compensation_val = %d\n",
			prv_deltagain, gain_for, deal_gain[gain_for],
			compensation_val);
	}

	hw_priv->efuse.delta_gain1 = deal_gain[0];
	hw_priv->efuse.delta_gain2 = deal_gain[1];
	hw_priv->efuse.delta_gain3 = deal_gain[2];

	write_flag = msg->value;

	memset(writebuf, 0, sizeof(writebuf));
	sprintf(writebuf, "set_txpwr_and_dcxo,%d,%d,%d,%d ",
		hw_priv->efuse.delta_gain1, hw_priv->efuse.delta_gain2,
		hw_priv->efuse.delta_gain3, hw_priv->efuse.dcxo_trim);

	Sstar_printk_init("cmd: %s\n", writebuf);
	ret = wsm_write_mib(hw_priv, WSM_MIB_ID_FW_CMD, writebuf,
			    strlen(writebuf), 0);
	if (ret < 0) {
		Sstar_printk_err("%s: write mib failed(%d). \n", __func__, ret);
	}

	if (write_flag == 1) {
		if (Sstar_save_efuse(hw_priv, &hw_priv->efuse) != 0) {
			hw_priv->efuse.delta_gain1 = delta_gain[0];
			hw_priv->efuse.delta_gain2 = delta_gain[1];
			hw_priv->efuse.delta_gain3 = delta_gain[2];
			ret = -EINVAL;
		}

		Sstar_printk_init(
			"dcxo:%d,gain1:%d,gain2:%d,gain3:%d,[%02x:%02x:%02x:%02x:%02x:%02x]\n",
			hw_priv->efuse.dcxo_trim, hw_priv->efuse.delta_gain1,
			hw_priv->efuse.delta_gain2, hw_priv->efuse.delta_gain3,
			hw_priv->efuse.mac[0], hw_priv->efuse.mac[1],
			hw_priv->efuse.mac[2], hw_priv->efuse.mac[3],
			hw_priv->efuse.mac[4], hw_priv->efuse.mac[5]);
	}

	return ret;
}

int Sstar_wext_cmd(struct net_device *dev, void *data, int len)
{
	int ret = 0;
	struct ieee80211_sub_if_data *sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	struct ieee80211_local *local = sdata->local;
	struct Sstar_vif *priv = (struct Sstar_vif *)sdata->vif.drv_priv;
	struct ieee80211_hw *hw = &local->hw;

	struct Sstar_common *hw_priv = priv->hw_priv;
	struct altm_wext_msg *msg = NULL;

	if (atomic_read(&priv->enabled) == 0) {
		Sstar_printk_err("Sstar_wext_cmd() priv is disabled\n");
		ret = -1;
		goto __Exit__;
	}

	if (hw_priv == NULL) {
		Sstar_printk_err("error, netdev NULL\n");
		ret = -1;
		goto __Exit__;
	}

	if (hw->vendcmd_nl80211 == 0) {
		struct nlattr *data_p = nla_find(data, len, SSTAR_TM_MSG_DATA);
		if (!data_p) {
			ret = -1;
			goto __Exit__;
		}
		msg = (struct altm_wext_msg *)nla_data(data_p);
	} else
		msg = (struct altm_wext_msg *)data;

	dev_printk("cmd type: %d\n", msg->type);

	switch (msg->type) {
	case SSTAR_DEV_IO_GET_STA_STATUS:
		memcpy(msg->externData, &sdata->vif.bss_conf.assoc, 4);
		break;
	case SSTAR_DEV_IO_GET_STA_RSSI: {
		struct sta_info *sta = NULL;
		int rssi = 0;
		rcu_read_lock();
		list_for_each_entry_rcu (sta, &local->sta_list, list) {
			if (sta->sdata->vif.type == NL80211_IFTYPE_STATION) {
				rssi = (s8)-Sstar_ewma_read(&sta->avg_signal);
				memcpy(msg->externData, &rssi, 4);
				break;
			}
		}
		rcu_read_unlock();
	} break;
	case SSTAR_DEV_IO_GET_AP_INFO:
		Sstar_printk_wext("%s: get sta info(%d)\n", __func__,
				  hw_priv->connected_sta_cnt);
		Sstar_dev_get_ap_info(dev, msg);
		break;
	case SSTAR_DEV_IO_GET_STA_INFO:
		Sstar_dev_get_sta_info(dev, msg);
		break;
	case SSTAR_DEV_IO_SET_STA_SCAN:
		Sstar_dev_set_sta_scan(dev, msg);
		break;
	case SSTAR_DEV_IO_SET_FREQ:
		Sstar_dev_set_freq(dev, msg);
		break;
	case SSTAR_DEV_IO_SET_SPECIAL_OUI:
		Sstar_dev_set_special_oui(dev, msg);
		break;
	case SSTAR_DEV_IO_SET_STA_DIS:
		Sstar_dev_set_sta_dis(dev, msg);
		break;
	case SSTAR_DEV_IO_SET_IF_TYPE:
		Sstar_dev_set_iftype(dev, msg);
		break;
	case SSTAR_DEV_IO_SET_ADAPTIVE:
		Sstar_dev_set_adaptive(dev, msg);
		break;
	case SSTAR_DEV_IO_SET_TXPWR_DCXO:
		Sstar_dev_set_txpwr_dcxo(dev, msg);
		break;
#ifdef CONFIG_IEEE80211_SPECIAL_FILTER

	case SSTAR_DEV_IO_SET_SPECIAL_FILTER:
		Sstar_dev_set_special_filter(dev, msg);
		break;
#endif
	case SSTAR_DEV_IO_SET_TXPWR: {
#ifdef CONFIG_JUAN_MISC
		Sstar_dev_set_txpwr(dev, msg);
#else
		struct ieee80211_internal_wsm_txpwr txpwr;

		memcpy(&txpwr.txpwr_indx, &msg->externData[0], sizeof(int));
		if (Sstar_internal_wsm_txpwr(hw_priv, &txpwr) == false) {
			ret = -EINVAL;
		}
#endif
		break;
	}
	case SSTAR_DEV_IO_GET_WORK_CHANNEL:
		Sstar_dev_get_work_channel(dev, msg);
		break;
	case SSTAR_DEV_IO_SET_BEST_CHANNEL_SCAN:
		Sstar_dev_set_best_channel_scan(dev, msg);
		break;
	case SSTAR_DEV_IO_GET_AP_LIST:
		Sstar_dev_get_ap_list(dev, msg);
		break;
	case SSTAR_DEV_IO_GET_TP_RATE:
		Sstar_dev_get_tp_rate(dev, msg);
		break;
	case SSTAR_DEV_IO_ETF_TEST:
		Sstar_dev_etf_test(dev, msg);
		break;
	case SSTAR_DEV_IO_ETF_GET_RESULT:
		Sstar_dev_etf_get_result(dev, msg);
		break;
	case SSTAR_DEV_IO_ETF_START_TX:
		Sstar_dev_start_tx(dev, msg);
		break;
	case SSTAR_DEV_IO_ETF_STOP_TX:
		Sstar_dev_stop_tx(dev, msg);
		break;
	case SSTAR_DEV_IO_ETF_START_RX:
		Sstar_dev_start_rx(dev, msg);
		break;
	case SSTAR_DEV_IO_ETF_STOP_RX:
		Sstar_dev_stop_rx(dev, msg);
		break;
	case SSTAR_DEV_IO_FIX_TX_RATE:
	case SSTAR_DEV_IO_MAX_TX_RATE:
	case SSTAR_DEV_IO_TX_RATE_FREE:
	case SSTAR_DEV_IO_MIN_TX_RATE: {
		struct ieee80211_internal_rate_req req;
		req.flags = 0;
		req.rate = 0;

		if (msg->type == SSTAR_DEV_IO_FIX_TX_RATE) {
			req.flags |= IEEE80211_INTERNAL_RATE_FLAGS_SET_TX_RATE;
			req.flags |=
				IEEE80211_INTERNAL_RATE_FLAGS_CLEAE_TOP_RATE;
		}

		if (msg->type == SSTAR_DEV_IO_MAX_TX_RATE) {
			req.flags |= IEEE80211_INTERNAL_RATE_FLAGS_SET_TOP_RATE;
			req.flags |=
				IEEE80211_INTERNAL_RATE_FLAGS_CLEAR_TX_RATE;
		}

		if (msg->type == SSTAR_DEV_IO_TX_RATE_FREE) {
			req.flags |=
				IEEE80211_INTERNAL_RATE_FLAGS_CLEAE_TOP_RATE;
			req.flags |=
				IEEE80211_INTERNAL_RATE_FLAGS_CLEAR_TX_RATE;
		}

		if (msg->type == SSTAR_DEV_IO_MIN_TX_RATE) {
			req.flags |= IEEE80211_INTERNAL_RATE_FLAGS_SET_MIN_RATE;
		}
		req.rate = msg->value;

		if (Sstar_internal_wsm_set_rate(hw_priv, &req) == false) {
			ret = -EINVAL;
		} else {
			ret = 0;
		}
		break;
	}
	case SSTAR_DEV_IO_SET_RATE_POWER: {
		struct ieee80211_internal_rate_power_req power_req;

		memset(&power_req, 0,
		       sizeof(struct ieee80211_internal_rate_power_req));

		power_req.rate_index = msg->externData[0];
		power_req.power = msg->externData[1];

		if (Sstar_internal_wsm_set_rate_power(hw_priv, &power_req) ==
		    false) {
			ret = -EINVAL;
		} else {
			ret = 0;
		}

		break;
	}
	case SSTAR_DEV_IO_SET_EFUSE_MAC: {
		u8 prv_mac[ETH_ALEN];

		memcpy(prv_mac, hw_priv->efuse.mac, ETH_ALEN);
		memcpy(hw_priv->efuse.mac, msg->externData, ETH_ALEN);

		if (Sstar_save_efuse(hw_priv, &hw_priv->efuse) != 0) {
			memcpy(hw_priv->efuse.mac, prv_mac, ETH_ALEN);
			ret = -EINVAL;
		}

		break;
	}

	case SSTAR_DEV_IO_SET_EFUSE_DCXO: {
		u8 prv_dcxo = hw_priv->efuse.dcxo_trim;

		hw_priv->efuse.dcxo_trim = msg->externData[0];

		if (Sstar_save_efuse(hw_priv, &hw_priv->efuse) != 0) {
			hw_priv->efuse.dcxo_trim = prv_dcxo;
			ret = -EINVAL;
		}

		break;
	}

	case SSTAR_DEV_IO_SET_EFUSE_DELTAGAIN: {
		u8 prv_deltagain1 = hw_priv->efuse.delta_gain1;
		u8 prv_deltagain2 = hw_priv->efuse.delta_gain2;
		u8 prv_deltagain3 = hw_priv->efuse.delta_gain3;

		hw_priv->efuse.delta_gain1 = msg->externData[0];
		hw_priv->efuse.delta_gain2 = msg->externData[1];
		hw_priv->efuse.delta_gain3 = msg->externData[2];

		if (Sstar_save_efuse(hw_priv, &hw_priv->efuse) != 0) {
			hw_priv->efuse.delta_gain1 = prv_deltagain1;
			hw_priv->efuse.delta_gain2 = prv_deltagain2;
			hw_priv->efuse.delta_gain3 = prv_deltagain3;
			ret = -EINVAL;
		}

		break;
	}
	case SSTAR_DEV_IO_SET_COUNTRY_CODE: {
		Sstar_dev_set_country_code(dev, msg);
	} break;
	case SSTAR_DEV_IO_GET_DRIVER_VERSION: {
		if (Sstar_dev_get_driver_version(dev, msg) != 0)
			ret = -EINVAL;
	} break;
	case SSTAR_DEV_IO_GET_EFUSE: {
		if (Sstar_dev_get_efuse(dev, msg) != 0)
			ret = -EINVAL;

	} break;
	case SSTAR_DEV_IO_GET_ETF_START_RX_RESULTS: {
		if (Sstar_dev_get_etf_rx_results(dev, msg) != 0)
			ret = -EINVAL;
	} break;
	case SSTAR_DEV_IO_SET_UPERR_PROCESS_PID: {
		local->upper_pid = msg->value;
		Sstar_printk_err("%s : set upper process pid = %d \n", __func__,
				 local->upper_pid);

	} break;
#ifdef CONFIG_SSTAR_SUPPORT_AP_CONFIG

	case SSTAR_DEV_IO_SET_FIX_SCAN_CHANNEL: {
		ret = Sstar_dev_set_fix_scan_channel(dev, msg);
	} break;
#endif
	case SSTAR_DEV_IO_SET_AUTO_CALI_PPM: {
		Sstar_dev_set_cail_auto_ppm(dev, msg);
	} break;
	case SSTAR_DEV_IO_GET_CALI_REAULTS: {
		ret = Sstar_dev_get_cail_ppm_results(dev, msg);
	} break;
	case SSTAR_DEV_IO_SET_EFUSE_GAIN_COMPENSATION_VALUE: {
		ret = Sstar_dev_set_efuse_gain_compensation_value(dev, msg);
	} break;
	case SSTAR_DEV_IO_GET_VENDOR_SPECIAL_IE: {
		ret = Sstar_dev_get_vendor_special_ie(dev, msg);
	} break;
	default:
		Sstar_printk_err("%s: not found. %d\n", __func__, msg->type);
		break;
	}

__Exit__:
	return ret;
}
#endif
