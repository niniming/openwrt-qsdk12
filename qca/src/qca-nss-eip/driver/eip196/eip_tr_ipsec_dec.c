/*
 * Copyright (c) 2023, Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/crypto.h>
#include <linux/slab.h>
#include <linux/completion.h>
#include <asm/cacheflush.h>

#include "eip_priv.h"

/*
 * eip_tr_ipsec_dec_dummy_pre()
 * 	Dummy function
 */
static void eip_tr_ipsec_dec_dummy_pre(struct eip_tr *tr, struct sk_buff *skb)
{
	/*
	 * Not doing anything here
	 */
}

/*
 * eip_tr_ipsec_dec_dummy_post()
 * 	Dummy function
 */
static void eip_tr_ipsec_dec_dummy_post(struct eip_tr *tr, struct sk_buff *skb)
{
	/*
	 * Not doing anything here
	 */
}

/*
 * eip_tr_ipsec_dec_err()
 *	Hardware error callback API for Decapsulation.
 */
void eip_tr_ipsec_dec_err(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw, uint16_t cle_err, uint16_t tr_err)
{
	struct eip_tr_ipsec *ipsec = &tr->ipsec;
	struct eip_ctx *ctx = tr->ctx;
	int err;

	/*
	 * Call the client callback.
	 */
	err = ((tr_err << EIP_TR_ERR_SHIFT) | cle_err);
	ipsec->ops.err_cb(ipsec->ops.app_data, sw->req, err);

	kmem_cache_free(ctx->tk_cache, sw->tk);
	kmem_cache_free(ctx->sw_cache, sw);
	eip_tr_deref(tr);
}

/*
 * eip_tr_ipsec_dec_done()
 *	Hardware completion callback API for Decapsulation.
 */
void eip_tr_ipsec_dec_done(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw)
{
	uint16_t out_len = EIP_HW_RES_DATA_LEN(hw->token[0]);
	struct eip_tr_ipsec *ipsec = &tr->ipsec;
	struct eip_ctx *ctx = tr->ctx;
	struct sk_buff *skb;

	/*
	 * After successful Transformation SKB data length needs to be decreased.
	 * TODO: Fix for nonlinear skb
	 */
	skb = eip_req2skb(sw->req);
	skb_trim(skb, out_len);
	skb_reset_network_header(skb);

	/*
	 * Call the client callback.
	 */
	ipsec->ops.cb(ipsec->ops.app_data, sw->req);

	kmem_cache_free(ctx->tk_cache, sw->tk);
	kmem_cache_free(ctx->sw_cache, sw);
	eip_tr_deref(tr);
}

/*
 * eip_tr_ipsec_dec()
 *	IPsec Decapsulation.
 */
int eip_tr_ipsec_dec(struct eip_tr *tr, struct sk_buff *skb)
{
	int (*dma_tx)(struct eip_dma *dma, struct eip_sw_desc *sw, struct sk_buff *skb);
	struct eip_ctx *ctx = tr->ctx;
	struct eip_sw_desc *sw;
	uint32_t tk_hdr = 0;
	struct eip_dma *dma;
	struct eip_tk *tk;
	uint32_t tk_words;
	int status = 0;

	/*
	 * Allocate HW token.
	 */
	tk = kmem_cache_alloc(ctx->tk_cache, GFP_NOWAIT | __GFP_NOWARN);
	if (!tk) {
		pr_err("%px: Failed to allocate token.\n", tr);
		return -ENOMEM;
	}

	/*
	 * Allocate SW descriptor.
	 */
	sw = kmem_cache_alloc(ctx->sw_cache, GFP_NOWAIT | __GFP_NOWARN);
	if (!sw) {
		pr_err("%px: Failed to allocate SW descriptor.\n", tr);
		kmem_cache_free(ctx->tk_cache, tk);
		return -ENOMEM;
	}

	skb_pull(skb, tr->ipsec.hdr_len);

	/*
	 * Fill token for decryption and hash for ipsec
	 */
	tk_words = eip_tr_fill_token(tr, &tr->ipsec.ops, tk, skb, &tk_hdr);
	dmac_clean_range(tk, tk + 1);

	dma_tx = skb_is_nonlinear(skb) ? eip_dma_tx_nonlinear_skb : eip_dma_tx_linear_skb;

	/*
	 * Fill software descriptor.
	 * Dereference: eip_tr_ipsec_dec_done() / eip_tr_ipsec_dec_err()
	 */
	sw->tr = eip_tr_ref(tr);

	sw->tk = tk;
	sw->tk_hdr = tk_hdr;
	sw->tk_words = tk_words;
	sw->tk_addr = virt_to_phys(tk);
	sw->tr_addr_type = tr->tr_addr_type;

	sw->req = skb;
	sw->hw_svc = EIP_HW_CMD_HWSERVICE_LAC;

	sw->comp = eip_tr_ipsec_dec_done;
	sw->err_comp = eip_tr_ipsec_dec_err;

	dma = &ctx->dma[smp_processor_id()];

	status = dma_tx(dma, sw, skb);
	if (status < 0) {
		dma->stats.tx_error++;
		goto fail_tx;
	}

	return 0;

fail_tx:
	eip_tr_deref(tr);
	kmem_cache_free(ctx->sw_cache, sw);
	kmem_cache_free(ctx->tk_cache, tk);
	return status;
}
EXPORT_SYMBOL(eip_tr_ipsec_dec);

/*
 * eip_tr_ipsec_get_replay_words()
 */
static uint32_t eip_tr_ipsec_get_replay_words(enum eip_ipsec_replay replay)
{
        switch (replay) {
        case EIP_IPSEC_REPLAY_NONE:
                return 0;
        case EIP_IPSEC_REPLAY_32:
                return 1;
        case EIP_IPSEC_REPLAY_64:
                return 2;
        case EIP_IPSEC_REPLAY_128:
                return 4;
        case EIP_IPSEC_REPLAY_384:
        default:
                return 12;
        }
}

/*
 * eip_tr_ipsec_get_replay_seq_mask()
 */
static uint32_t eip_tr_ipsec_get_replay_seq_mask(enum eip_ipsec_replay replay)
{
        switch (replay) {
        case EIP_IPSEC_REPLAY_NONE:
                return 0;
        case EIP_IPSEC_REPLAY_32:
                return EIP_TR_IPSEC_SEQ_NUM_MASK_32;
        case EIP_IPSEC_REPLAY_64:
                return EIP_TR_IPSEC_SEQ_NUM_MASK_64;
        case EIP_IPSEC_REPLAY_128:
                return EIP_TR_IPSEC_SEQ_NUM_MASK_128;
        case EIP_IPSEC_REPLAY_384:
        default:
                return EIP_TR_IPSEC_SEQ_NUM_MASK_384;
        }
}

/*
 * eip_tr_ipsec_dec_cmn_init()
 *      Initialize the transform record for ipsec Decapsulation.
 */
void eip_tr_ipsec_dec_cmn_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo, bool nonce_en)
{
        struct eip_tr_info_ipsec *ipsec = &info->ipsec;
	struct eip_tr_ipsec *tr_ipsec = &tr->ipsec;
        struct eip_tr_base *crypto = &info->base;
        uint32_t *crypt_words = tr->hw_words;
        uint32_t *tr_words = tr->hw_words;
        uint8_t seq_offset;
	bool esn, natt;
        uint32_t size;

        esn = ipsec->sa_flags & EIP_TR_IPSEC_FLAG_ESN;
	natt =ipsec->sa_flags & EIP_TR_IPSEC_FLAG_UDP;

	tr->ipsec.ops.tk_fill = algo->dec_tk_fill;

        /*
         * First two words are Control words.
         */
        tr_words[0] = algo->ctrl_words_0;
        tr_words[1] = algo->ctrl_words_1;

        /*
         * Enable IPsec specific fields in control words 0.
         */
	tr_words[0] |= EIP_TK_CTRL_OP_HMAC_DEC;
        tr_words[0] |= eip_tr_ipsec_get_replay_seq_mask(ipsec->replay);
        tr_words[0] |= EIP_TR_IPSEC_EXT_SEQ_NUM(esn);
        tr_words[1] |= EIP_TR_IPSEC_PAD_TYPE;

        /*
         * Crypto variable words starts from third words.
         */
        crypt_words = &tr_words[2];

        /*
         * Fill cipher key.
         */
        memcpy(crypt_words, crypto->cipher.key_data, crypto->cipher.key_len);
        crypt_words += (crypto->cipher.key_len /  sizeof(uint32_t));

        /*
         * Leave the space for inner & outer digest.
         */
        size = algo->auth_state_len;
        crypt_words += (size / sizeof(uint32_t)); /* ipad */
        crypt_words += (size / sizeof(uint32_t)); /* opad */

        /*
         * Fill nonce.
         */
        if (nonce_en)
                *crypt_words++ = crypto->nonce;

        /*
         * IPsec specific fields for EIP96.
         * - SPI in Host order (1 word)
         * - Sequence number (2 words for ESN else 1 word)
         * - Sequence mask to support window size.
         */
        *crypt_words++ = ntohl(ipsec->spi_idx);
        *crypt_words++ = 0x1;
        seq_offset = crypt_words - tr_words - 1;
        if (esn)
                *crypt_words++ = 0;

        crypt_words += eip_tr_ipsec_get_replay_words(ipsec->replay);

        /*
         * Done with EIP crypto context words.
         * Update relative information in control words.
         */
        tr_words[0] |= EIP_TR_CTRL_CONTEXT_WORDS(crypt_words - tr_words - 2);

        /*
         * Store the seq no offset, icv length and cipher block length
         * to be used during token fill
         */
        tr_ipsec->seq_offset = seq_offset;
        tr->digest_len = ipsec->icv_len;
        tr->blk_len = algo->cipher_blk_len;
	tr->iv_len = algo->iv_len;

	/*
	 * Assign the pre & post procesing callback for IP header
	 *
	 * For transport mode decap we will bypass the IP header
	 * Note for NATT we are removing UDP header before decap,
	 * so the bypass len does not include UDP header length.
	 */
	tr_ipsec->ops.pre = eip_tr_ipsec_dec_dummy_pre;
	tr_ipsec->ops.post = eip_tr_ipsec_dec_dummy_post;
	tr_ipsec->bypass_len = 0;

	switch (ipsec->ip_ver) {
	case IPVERSION:
		tr_ipsec->hdr_len = sizeof(struct iphdr) + (natt ? sizeof(struct udphdr) : 0);
		break;
        default:
		tr_ipsec->hdr_len = sizeof(struct ipv6hdr);
		break;
	}
}

