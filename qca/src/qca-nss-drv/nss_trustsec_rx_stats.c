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
#include "nss_tx_rx_common.h"
#include "nss_trustsec_rx_stats.h"

/*
 * nss_trustsec_rx_stats_str
 *	Trustsec TX statistics strings.
 */

struct nss_stats_info nss_trustsec_rx_stats_str[NSS_TRUSTSEC_RX_STATS_MAX] = {
	{"Unknown ethernet type", NSS_STATS_TYPE_ERROR},
	{"Unknown packet"	, NSS_STATS_TYPE_ERROR},
	{"Unknown destination"	, NSS_STATS_TYPE_ERROR},
	{"IP parse failed"	, NSS_STATS_TYPE_ERROR},
	{"Wrong L4 type"	, NSS_STATS_TYPE_ERROR},
};

/*
 * trustsec_rx_stats
 *	Trustsec RX statistics.
 */
uint64_t trustsec_rx_stats[NSS_TRUSTSEC_RX_STATS_MAX];

/*
 * Trustsec RX statistics APIs
 */

/*
 * nss_trustsec_rx_stats_sync()
 *	Update trustsec_rx node statistics.
 */
void nss_trustsec_rx_stats_sync(struct nss_ctx_instance *nss_ctx, struct nss_trustsec_rx_stats_sync_msg *ntsm)
{
	struct nss_top_instance *nss_top = nss_ctx->nss_top;
	int j;

	spin_lock_bh(&nss_top->stats_lock);

	/*
	 * Update common node stats
	 */
	nss_top->stats_node[NSS_TRUSTSEC_RX_INTERFACE][NSS_STATS_NODE_RX_PKTS] += ntsm->node_stats.rx_packets;
	nss_top->stats_node[NSS_TRUSTSEC_RX_INTERFACE][NSS_STATS_NODE_RX_BYTES] += ntsm->node_stats.rx_bytes;
	nss_top->stats_node[NSS_TRUSTSEC_RX_INTERFACE][NSS_STATS_NODE_TX_PKTS] += ntsm->node_stats.tx_packets;
	nss_top->stats_node[NSS_TRUSTSEC_RX_INTERFACE][NSS_STATS_NODE_TX_BYTES] += ntsm->node_stats.tx_bytes;

	for (j = 0; j < NSS_MAX_NUM_PRI; j++) {
		nss_top->stats_node[NSS_TRUSTSEC_RX_INTERFACE][NSS_STATS_NODE_RX_QUEUE_0_DROPPED + j] += ntsm->node_stats.rx_dropped[j];
	}

	/*
	 * Update trustsec node stats
	 */
	trustsec_rx_stats[NSS_TRUSTSEC_RX_STATS_UNKNOWN_ETH_TYPE] += ntsm->unknown_eth_type;
	trustsec_rx_stats[NSS_TRUSTSEC_RX_STATS_UNKNOWN_PKT] += ntsm->unknown_pkt;
	trustsec_rx_stats[NSS_TRUSTSEC_RX_STATS_UNKNOWN_DEST] += ntsm->unknown_dest;
	trustsec_rx_stats[NSS_TRUSTSEC_RX_STATS_IP_PARSE_FAILED] += ntsm->ip_parse_failed;
	trustsec_rx_stats[NSS_TRUSTSEC_RX_STATS_WRONG_L4_TYPE] += ntsm->wrong_l4_type;

	spin_unlock_bh(&nss_top->stats_lock);
}

/*
 * nss_trustsec_rx_stats_read()
 *	Read trustsec_rx statiistics.
 */
static ssize_t nss_trustsec_rx_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	int32_t i;

	/*
	 * Max output lines = #stats + few blank lines for banner printing +
	 * Number of Extra outputlines for future reference to add new stats.
	 */
	uint32_t max_output_lines = NSS_STATS_NODE_MAX + NSS_TRUSTSEC_RX_STATS_MAX + NSS_STATS_EXTRA_OUTPUT_LINES;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	uint64_t *stats_shadow;

	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(lbuf == NULL)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	stats_shadow = kzalloc(NSS_STATS_NODE_MAX * 8, GFP_KERNEL);
	if (unlikely(stats_shadow == NULL)) {
		nss_warning("Could not allocate memory for local shadow buffer");
		kfree(lbuf);
		return 0;
	}

	size_wr += nss_stats_banner(lbuf, size_wr, size_al, "trustsec_rx", NSS_STATS_SINGLE_CORE);

	/*
	 * Common node stats
	 */
	size_wr += nss_stats_fill_common_stats(NSS_TRUSTSEC_RX_INTERFACE, NSS_STATS_SINGLE_INSTANCE, lbuf, size_wr, size_al, "trustsec_rx");

	/*
	 * TrustSec TX node stats
	 */
	spin_lock_bh(&nss_top_main.stats_lock);
	for (i = 0; (i < NSS_TRUSTSEC_RX_STATS_MAX); i++) {
		stats_shadow[i] = trustsec_rx_stats[i];
	}

	spin_unlock_bh(&nss_top_main.stats_lock);
	size_wr += nss_stats_print("trustsec_rx", NULL, NSS_STATS_SINGLE_INSTANCE
					, nss_trustsec_rx_stats_str
					, stats_shadow
					, NSS_TRUSTSEC_RX_STATS_MAX
					, lbuf, size_wr, size_al);
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, strlen(lbuf));
	kfree(lbuf);
	kfree(stats_shadow);

	return bytes_read;
}

/*
 * nss_trustsec_rx_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(trustsec_rx)

/*
 * nss_trustsec_rx_stats_dentry_create()
 *	Create trustsec_rx statistics debug entry.
 */
void nss_trustsec_rx_stats_dentry_create(void)
{
	nss_stats_create_dentry("trustsec_rx", &nss_trustsec_rx_stats_ops);
}
