/*
 **************************************************************************
 * Copyright (c) 2017-2020, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/*
 * nss_wifili_stats.h
 *	NSS wifili statistics header file.
 */

#ifndef __NSS_WIFILI_STATS_H
#define __NSS_WIFILI_STATS_H

#define NSS_WIFILI_TARGET_TYPE_QCA8074   20
#define NSS_WIFILI_TARGET_TYPE_QCA8074V2 24
#define NSS_WIFILI_TARGET_TYPE_QCA6018   25
#define NSS_WIFILI_TARGET_TYPE_QCN9000   26
#define NSS_WIFILI_TARGET_TYPE_QCA5018   29
#define NSS_WIFILI_TARGET_TYPE_QCN6122   30

#include "nss_core.h"
#include "nss_wifili_if.h"

/*
 * NSS wifili statistics APIs
 */
extern void nss_wifili_stats_notify(struct nss_ctx_instance *nss_ctx, uint32_t if_num);
extern void nss_wifili_stats_sync(struct nss_ctx_instance *nss_ctx, struct nss_wifili_stats_sync_msg *wlsoc_stats, uint16_t interface);
extern void nss_wifili_stats_dentry_create(void);

#endif /* __NSS_WIFILI_STATS_H */
