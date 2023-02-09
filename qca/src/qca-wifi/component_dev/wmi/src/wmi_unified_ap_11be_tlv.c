/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <osdep.h>
#include <wmi.h>
#include <wmi_unified_priv.h>
#include <wmi_unified_ap_api.h>

uint8_t *bcn_offload_quiet_add_ml_partner_links(
		uint8_t *buf_ptr,
		struct set_bcn_offload_quiet_mode_params *param)
{
	wmi_vdev_bcn_offload_ml_quiet_config_params *ml_quiet;
	struct wmi_mlo_bcn_offload_partner_links *partner_link;
	uint8_t i;

	if (param->mlo_partner.num_links > WLAN_UMAC_MLO_MAX_VDEVS) {
		wmi_err("mlo_partner.num_link(%d) are greater than supported partner links(%d)",
				param->mlo_partner.num_links, WLAN_UMAC_MLO_MAX_VDEVS);
		return buf_ptr;
	}

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       (param->mlo_partner.num_links *
			sizeof(wmi_vdev_bcn_offload_ml_quiet_config_params)));
	buf_ptr += sizeof(uint32_t);

	partner_link = &param->mlo_partner;
	ml_quiet = (wmi_vdev_bcn_offload_ml_quiet_config_params *)buf_ptr;
	for (i = 0; i < partner_link->num_links; i++) {
		WMITLV_SET_HDR(&ml_quiet->tlv_header,
			       WMITLV_TAG_STRUC_wmi_vdev_bcn_offload_ml_quiet_config_params,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_vdev_bcn_offload_ml_quiet_config_params)
			       );
		ml_quiet->vdev_id = partner_link->partner_info[i].vdev_id;
		ml_quiet->hw_link_id = partner_link->partner_info[i].hw_link_id;
		ml_quiet->beacon_interval =
			partner_link->partner_info[i].bcn_interval;
		ml_quiet->period = partner_link->partner_info[i].period;
		ml_quiet->duration = partner_link->partner_info[i].duration;
		ml_quiet->next_start = partner_link->partner_info[i].next_start;
		ml_quiet->flags = partner_link->partner_info[i].flag;
		ml_quiet++;
	}

	return buf_ptr + (param->mlo_partner.num_links *
			sizeof(wmi_vdev_bcn_offload_ml_quiet_config_params));
}

size_t quiet_mlo_params_size(struct set_bcn_offload_quiet_mode_params *params)
{
	size_t quiet_mlo_size;

	quiet_mlo_size = WMI_TLV_HDR_SIZE +
			 (sizeof(wmi_vdev_bcn_offload_ml_quiet_config_params) *
			  params->mlo_partner.num_links);

	return quiet_mlo_size;
}
