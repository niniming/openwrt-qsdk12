/*
 **************************************************************************
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#ifndef __NSS_UDP_ST_STRINGS_H
#define __NSS_UDP_ST_STRINGS_H

extern struct nss_stats_info nss_udp_st_strings_error_stats[NSS_UDP_ST_ERROR_MAX];
extern struct nss_stats_info nss_udp_st_strings_rx_time_stats[NSS_UDP_ST_STATS_TIME_MAX];
extern struct nss_stats_info nss_udp_st_strings_tx_time_stats[NSS_UDP_ST_STATS_TIME_MAX];
extern struct nss_stats_info nss_udp_st_strings_timestamp_stats[NSS_UDP_ST_STATS_TIMESTAMP_MAX];
extern void nss_udp_st_strings_dentry_create(void);

#endif /* __NSS_UDP_ST_STRINGS_H */
