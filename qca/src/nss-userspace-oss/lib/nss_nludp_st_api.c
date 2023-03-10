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

#include <nss_nlbase.h>
#include <nss_nlsock_api.h>
#include <nss_nludp_st_api.h>

/*
 * nss_nludp_st_sock_cb()
 *	Callback func for udp_st netlink socket
 */
int nss_nludp_st_sock_cb(struct nl_msg *msg, void *arg)
{
	pid_t pid = getpid();

	if(!msg) {
		nss_nlsock_log_error("%d:failed to get NSS NL msg\n", pid);
		return NL_SKIP;
	}

	struct nss_nludp_st_ctx *ctx = (struct nss_nludp_st_ctx *)arg;
	struct nss_nlsock_ctx *sock = &ctx->sock;

	struct nss_nludp_st_rule *rule = nss_nlsock_get_data(msg);
	if (!rule) {
		nss_nlsock_log_error("%d:failed to get NSS NL udp_st header\n", pid);
		return NL_SKIP;
	}

	uint8_t cmd = nss_nlcmn_get_cmd(&rule->cm);

	switch (cmd) {
	case NSS_UDP_ST_CFG_RULE_MSG:
	case NSS_UDP_ST_UNCFG_RULE_MSG:
	case NSS_UDP_ST_START_MSG:
	case NSS_UDP_ST_STOP_MSG:
	case NSS_UDP_ST_TX_CREATE_MSG:
	case NSS_UDP_ST_TX_DESTROY_MSG:
	case NSS_UDP_ST_TX_UPDATE_RATE_MSG:
	case NSS_UDP_ST_RX_MODE_SET_MSG:
	case NSS_UDP_ST_RESET_STATS_MSG:
	{
		void *cb_data = nss_nlcmn_get_cb_data(&rule->cm, sock->family_id);
		if (!cb_data) {
			return NL_SKIP;
		}

		/*
		 * Note: The callback user can modify the CB content so it
		 * needs to locally save the response data for further use
		 * after the callback is completed
		 */
		struct nss_nludp_st_resp resp;
		memcpy(&resp, cb_data, sizeof(struct nss_nludp_st_resp));

		/*
		 * clear the ownership of the CB so that callback user can
		 * use it if needed
		 */
		nss_nlcmn_clr_cb_owner(&rule->cm);

		if (!resp.cb) {
			nss_nlsock_log_info("%d:no UDP ST response callback for cmd(%d)\n", pid, cmd);
			return NL_SKIP;
		}

		resp.cb(sock->user_ctx, rule, resp.data);

		return NL_OK;
	}

	default:
		nss_nlsock_log_error("%d:unsupported message cmd type(%d)\n", pid, cmd);
		return NL_SKIP;
	}
}

/*
 * nss_nludp_st_sock_open()
 *	Opens the NSS udp_st NL socket for usage
 */
int nss_nludp_st_sock_open(struct nss_nludp_st_ctx *ctx, void *user_ctx, nss_nludp_st_event_t event_cb)
{
	pid_t pid = getpid();
	int error;

	if (!ctx) {
		nss_nlsock_log_error("%d: invalid parameters passed\n", pid);
		return -EINVAL;
	}

	memset(ctx, 0, sizeof(*ctx));

	nss_nlsock_set_family(&ctx->sock, NSS_NLUDP_ST_FAMILY);
	nss_nlsock_set_user_ctx(&ctx->sock, user_ctx);

	/*
	 * try opening the socket with Linux
	 */
	error = nss_nlsock_open(&ctx->sock, nss_nludp_st_sock_cb);
	if (error) {
		nss_nlsock_log_error("%d:unable to open NSS udp_st socket, error(%d)\n", pid, error);
		goto fail;
	}

	return 0;
fail:
	memset(ctx, 0, sizeof(*ctx));
	return error;
}

/*
 * nss_nludp_st_sock_close()
 *	Close the NSS udp_st NL socket
 */
void nss_nludp_st_sock_close(struct nss_nludp_st_ctx *ctx)
{
	if (!ctx) {
		return;
	}

	nss_nlsock_close(&ctx->sock);
	memset(ctx, 0, sizeof(struct nss_nludp_st_ctx));
}

/*
 * nss_nludp_st_sock_send()
 *	Send the udp_st message synchronously through the socket
 */
int nss_nludp_st_sock_send(struct nss_nludp_st_ctx *ctx, struct nss_nludp_st_rule *rule, nss_nludp_st_resp_t cb, void *data)
{
	int32_t family_id;
	struct nss_nludp_st_resp *resp;
	pid_t pid = getpid();
	bool has_resp = false;
	int error = 0;

	if (!ctx) {
		nss_nlsock_log_error("%d:invalid NSS udp_st ctx\n", pid);
		return -EINVAL;
	}

	if (!rule) {
		nss_nlsock_log_error("%d:invalid NSS udp_st rule\n", pid);
		return -EINVAL;
	}

	family_id = ctx->sock.family_id;

	if (cb) {
		nss_nlcmn_set_cb_owner(&rule->cm, family_id);

		resp = nss_nlcmn_get_cb_data(&rule->cm, family_id);
		assert(resp);

		resp->data = data;
		resp->cb = cb;
		has_resp = true;
	}

	error = nss_nlsock_send(&ctx->sock, &rule->cm, rule, has_resp);
	if (error) {
		nss_nlsock_log_error("%d:failed to send NSS udp_st rule, error(%d)\n", pid, error);
	}

	return error;
}

/*
 * nss_nludp_st_init_rule()
 *	Initialize the udp_st rule
 */
void nss_nludp_st_init_rule(struct nss_nludp_st_rule *rule, enum nss_udp_st_message_types type)
{
	nss_nludp_st_rule_init(rule, type);
}
