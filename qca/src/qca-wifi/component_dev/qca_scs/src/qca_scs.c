/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <qdf_module.h>
#include "qca_scs_if.h"

#ifdef WLAN_SUPPORT_SCS

#include <ol_if_athvar.h>

#define QCA_SCS_RULE_SOC_ID_MASK  0xF0000000
#define QCA_SCS_RULE_SOC_ID_SHIFT 28

extern ol_ath_soc_softc_t *ol_global_soc[GLOBAL_SOC_SIZE];

/**
 * qca_scs_fetch_soc_id_frm_rule_id() - Fetch soc_id from SCS rule_id
 *
 * rule_id: uint32_t SCS rule_id
 *
 * Return: soc_id of type uint8_t
 */
static inline uint8_t qca_scs_fetch_soc_id_frm_rule_id(uint32_t rule_id)
{
	return ((rule_id & QCA_SCS_RULE_SOC_ID_MASK) >> QCA_SCS_RULE_SOC_ID_SHIFT);
}

/**
 * qca_scs_peer_lookup_n_rule_match() - Find peer using dst mac addr and check
 *     if peer is SCS capable and rule matches for that peer or not
 *
 * @rule_id: SCS rule_id
 * @dst_mac_addr: destination mac address to be used for peer lookup
 *
 * Return: true on success and false on failure
 */
bool qca_scs_peer_lookup_n_rule_match(uint32_t rule_id, uint8_t *dst_mac_addr)
{
	uint8_t soc_id = 0;
	ol_ath_soc_softc_t *soc = NULL;
	ol_txrx_soc_handle soc_txrx_handle = NULL;

	/* Fetch soc_id from scs rule_id*/
	soc_id = qca_scs_fetch_soc_id_frm_rule_id(rule_id);

	if (!ol_global_soc[soc_id])
		return false;

	soc = ol_global_soc[soc_id];
	soc_txrx_handle = wlan_psoc_get_dp_handle(soc->psoc_obj);

	return cdp_scs_peer_lookup_n_rule_match(soc_txrx_handle, rule_id,
						dst_mac_addr);
}
#else
bool qca_scs_peer_lookup_n_rule_match(uint32_t rule_id, uint8_t *dst_mac_addr)
{
	return false;
}
#endif

qdf_export_symbol(qca_scs_peer_lookup_n_rule_match);
