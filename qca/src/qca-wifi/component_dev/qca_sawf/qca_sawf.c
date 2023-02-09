/*
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef CONFIG_SAWF
#include <dp_sawf.h>
#include <wlan_sawf.h>
#include <cdp_txrx_sawf.h>

uint16_t qca_sawf_get_msduq(struct net_device *netdev, uint8_t *peer_mac,
			    uint32_t service_id)
{
	if (!wlan_service_id_valid(service_id) ||
	    !wlan_service_id_configured(service_id)) {
	   return DP_SAWF_PEER_Q_INVALID;
	}

	return dp_sawf_get_msduq(netdev, peer_mac, service_id);
}

struct psoc_sawf_ul_itr {
	uint8_t *mac_addr;
	uint32_t service_interval;
	uint32_t burst_size;
	uint8_t tid;
	uint8_t add_or_sub;
};

static void qca_sawf_psoc_ul_cb(struct wlan_objmgr_psoc *psoc, void *cbd,
				uint8_t index)
{
	ol_txrx_soc_handle soc_txrx_handle;
	struct psoc_sawf_ul_itr *param = (struct psoc_sawf_ul_itr *)cbd;

	soc_txrx_handle = wlan_psoc_get_dp_handle(psoc);
	cdp_sawf_peer_config_ul(soc_txrx_handle,
				param->mac_addr, param->tid,
				param->service_interval, param->burst_size,
				param->add_or_sub);
}

static inline
void qca_sawf_peer_config_ul(uint8_t *mac_addr, uint8_t tid,
			     uint32_t service_interval, uint32_t burst_size,
			     uint8_t add_or_sub)
{
	struct psoc_sawf_ul_itr param = {
		.mac_addr = mac_addr,
		.service_interval = service_interval,
		.burst_size = burst_size,
		.tid = tid,
		.add_or_sub = add_or_sub
	};

	wlan_objmgr_iterate_psoc_list(qca_sawf_psoc_ul_cb, &param,
				      WLAN_SAWF_ID);
}

void qca_sawf_config_ul(uint8_t *dst_mac, uint8_t *src_mac,
			uint8_t fw_service_id, uint8_t rv_service_id,
			uint8_t add_or_sub)
{
	uint32_t svc_interval = 0, burst_size = 0;
	uint8_t tid = 0;

	qdf_info("src " QDF_MAC_ADDR_FMT " dst " QDF_MAC_ADDR_FMT
		 " fw_service_id %u rv_service_id %u",
		 QDF_MAC_ADDR_REF(src_mac), QDF_MAC_ADDR_REF(dst_mac),
		 fw_service_id, rv_service_id);

	if (QDF_IS_STATUS_SUCCESS(wlan_sawf_get_uplink_params(fw_service_id,
							      &tid,
							      &svc_interval,
							      &burst_size))) {
		if (svc_interval && burst_size)
			qca_sawf_peer_config_ul(src_mac, tid,
						svc_interval, burst_size,
						add_or_sub);
	}

	svc_interval = 0;
	burst_size = 0;
	tid = 0;
	if (QDF_IS_STATUS_SUCCESS(wlan_sawf_get_uplink_params(rv_service_id,
							      &tid,
							      &svc_interval,
							      &burst_size))) {
		if (svc_interval && burst_size)
			qca_sawf_peer_config_ul(dst_mac, tid,
						svc_interval, burst_size,
						add_or_sub);
	}
}
#else

#include "qdf_module.h"
#define DP_SAWF_PEER_Q_INVALID 0xffff
uint16_t qca_sawf_get_msduq(struct net_device *netdev, uint8_t *peer_mac,
			    uint32_t service_id)
{
	return DP_SAWF_PEER_Q_INVALID;
}

void qca_sawf_config_ul(uint8_t *dst_mac, uint8_t *src_mac,
			uint8_t fw_service_id, uint8_t rv_service_id,
			uint8_t add_or_sub)
{}
#endif

qdf_export_symbol(qca_sawf_get_msduq);
qdf_export_symbol(qca_sawf_config_ul);
