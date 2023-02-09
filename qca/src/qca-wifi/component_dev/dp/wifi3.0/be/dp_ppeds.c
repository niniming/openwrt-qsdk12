/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include <linux/netdevice.h>
#include <qdf_nbuf.h>
#include <dp_types.h>
#include <dp_internal.h>
#include <hal_be_hw_headers.h>
#include <dp_peer.h>
#include <dp_tx.h>
#include <dp_rx.h>
#include <be/dp_be_tx.h>
#include <hal_tx.h>
#include <hal_be_api.h>
#include <be/dp_be.h>
#include <ppe_vp_public.h>
#include "dp_ppeds.h"
#include <ppe_ds_wlan.h>
#include <wlan_osif_priv.h>
#include <dp_tx_desc.h>
#include "dp_rx.h"
#include "dp_rx_buffer_pool.h"
#include "dp_be_rx.h"

#define DS_NAPI_BUDGET_TO_INTERNAL_BUDGET(n, s) (((n) << (s)) - 1)
#define DS_INTERNAL_BUDGET_TO_NAPI_BUDGET(n, s) (((n) + 1) >> (s))

/**
 * dp_ppe_ds_ppe2tcl_irq_handler- Handle ppe2tcl ring interrupt
 * @irq: irq numer
 * @ctxt: context passed to the handler
 *
 * Handle ppe2tcl ring interrupt
 *
 * Return: IRQ_HANDLED
 */
irqreturn_t dp_ppe_ds_ppe2tcl_irq_handler(int irq, void *ctxt)
{
	ppe_ds_ppe2tcl_wlan_handle_intr(ctxt);

	return IRQ_HANDLED;
}

/*
 * dp_ppe_ds_reo2ppe_irq_handler: Handle reo2ppe ring interrupt
 * @irq: IRQ number
 * @ctxt: IRQ handler context
 *
 * Return: IRQ handle status
 */
irqreturn_t dp_ppe_ds_reo2ppe_irq_handler(int irq, void *ctxt)
{
	ppe_ds_reo2ppe_wlan_handle_intr(ctxt);

	return IRQ_HANDLED;
}

/**
 * dp_get_ppe_ds_ctxt - Get context from ppe ds driver
 * @soc: dp soc
 *
 * Get context from ppe driver
 *
 * Return: context
 */
void *dp_get_ppe_ds_ctxt(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	ppe_ds_wlan_handle_t *ppeds_handle =
				(ppe_ds_wlan_handle_t *)be_soc->ppeds_handle;

	if (!ppeds_handle)
		return NULL;

	return ppe_ds_wlan_get_intr_ctxt(ppeds_handle);
}

#if defined(QCA_DP_NBUF_FAST_PPEDS)
#define dp_nbuf_alloc_ppe_ds(d, s, r, a, p) \
	qdf_nbuf_alloc_ppe_ds(d, s, r, a, p)
#else
#define dp_nbuf_alloc_ppe_ds(d, s, r, a, p) \
	qdf_nbuf_alloc_simple(d, s, r, a, p)
#endif

/**
 * dp_ppeds_deinit_ppe_vp_tbl_be - PPE VP table dealoc
 * @be_soc: BE SoC
 *
 * Deallocate the VP table
 *
 * Return: QDF_STATUS
 */
static void dp_ppeds_deinit_ppe_vp_tbl_be(struct dp_soc_be *be_soc)
{
	if (be_soc->ppe_vp_tbl)
		qdf_mem_free(be_soc->ppe_vp_tbl);
	qdf_mutex_destroy(&be_soc->ppe_vp_tbl_lock);
}

/**
 * dp_ppeds_init_ppe_vp_tbl_be - PPE VP table alloc
 * @be_soc: BE SoC
 *
 * Allocate the VP table
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS dp_ppeds_init_ppe_vp_tbl_be(struct dp_soc_be *be_soc)
{
	int num_ppe_vp_entries, i;

	num_ppe_vp_entries =
		hal_tx_get_num_ppe_vp_tbl_entries(be_soc->soc.hal_soc);

	be_soc->ppe_vp_tbl =
		qdf_mem_malloc(num_ppe_vp_entries *
			       sizeof(*be_soc->ppe_vp_tbl));

	if (!be_soc->ppe_vp_tbl)
		return QDF_STATUS_E_NOMEM;

	qdf_mutex_create(&be_soc->ppe_vp_tbl_lock);

	for (i = 0; i < num_ppe_vp_entries; i++)
		be_soc->ppe_vp_tbl[i].is_configured = false;

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ppeds_get_vdev_vp_config_be() - Get the PPE vp config
 * @be_vdev: BE vdev
 * @vp_cfg: VP config
 *
 * Get the VDEV config for PPE
 *
 * Return: void
 */
static void dp_ppeds_get_vdev_vp_config_be(struct dp_vdev_be *be_vdev,
					    union hal_tx_ppe_vp_config *vp_cfg)
{
	struct dp_vdev *vdev = &be_vdev->vdev;

	/*
	 * Update the parameters frm profile.
	 */
	vp_cfg->vp_num = be_vdev->ppe_vp_profile.vp_num;

	/* This is valid when index based search is
	 * enabled for STA vap in bank register
	 */
	vp_cfg->search_idx_reg_num =
		be_vdev->ppe_vp_profile.search_idx_reg_num;

	vp_cfg->use_ppe_int_pri = be_vdev->ppe_vp_profile.use_ppe_int_pri;
	vp_cfg->to_fw = be_vdev->ppe_vp_profile.to_fw;
	vp_cfg->drop_prec_enable = be_vdev->ppe_vp_profile.drop_prec_enable;

	vp_cfg->bank_id = be_vdev->bank_id;
	vp_cfg->pmac_id = vdev->lmac_id;
	vp_cfg->vdev_id = vdev->vdev_id;
}

/**
 * dp_tx_ppeds_cfg_astidx_cache_mapping - Set ppe index mapping table value
 * @soc: DP SoC context
 * @vdev: DP vdev
 * @peer_map: map if true, unmap if false
 *
 * Return: void
 */
void dp_tx_ppeds_cfg_astidx_cache_mapping(struct dp_soc *soc,
					  struct dp_vdev *vdev, bool peer_map)
{
	union hal_tx_ppe_idx_map_config ppeds_idx_mapping = {0};

	if (vdev->opmode == wlan_op_mode_sta) {
		/*
		 * Peer map event provides bss_ast_idx and bss_ast_hash params
		 * which are used as search index and cache set numbers.
		 */
		if (peer_map) {
			ppeds_idx_mapping.search_idx = vdev->bss_ast_idx;
			ppeds_idx_mapping.cache_set = vdev->bss_ast_hash;
		}

		hal_ppeds_cfg_ast_override_map_reg(soc->hal_soc,
						   DP_PPEDS_STAMODE_ASTIDX_MAP_REG_IDX,
						   &ppeds_idx_mapping);
	}
}

/**
 * dp_ppeds_setup_vp_entry_be: Setup the PPE VP entry
 * be_soc: Be Soc
 * be_vdev: BE vdev
 *
 * Setup the PPE VP entry for HAL
 *
 * Return: void
 */
static void dp_ppeds_setup_vp_entry_be(struct dp_soc_be *be_soc,
					struct dp_vdev_be *be_vdev)
{
	int ppe_vp_num_idx;
	union hal_tx_ppe_vp_config vp_cfg = {0};

	dp_ppeds_get_vdev_vp_config_be(be_vdev, &vp_cfg);

	ppe_vp_num_idx = be_vdev->ppe_vp_profile.ppe_vp_num_idx;

	hal_tx_populate_ppe_vp_entry(be_soc->soc.hal_soc,
				     &vp_cfg,
				     ppe_vp_num_idx);
}

/**
 * dp_ppeds_alloc_vp_tbl_entry_be() - PPE VP entry alloc
 * @be_soc: BE SoC
 * @be_vdev: BE VAP
 *
 * PPE VP table entry alloc
 *
 * Return: Return PPE VP index to be used
 */
static int dp_ppeds_alloc_vp_tbl_entry_be(struct dp_soc_be *be_soc,
					   struct dp_vdev_be *be_vdev)
{
	int num_ppe_vp_max, i;

	num_ppe_vp_max =
		hal_tx_get_num_ppe_vp_tbl_entries(be_soc->soc.hal_soc);

	qdf_mutex_acquire(&be_soc->ppe_vp_tbl_lock);
	if (be_soc->num_ppe_vp_entries == num_ppe_vp_max) {
		qdf_mutex_release(&be_soc->ppe_vp_tbl_lock);
		dp_err("Maximum ppe_vp count reached for vap");
		return -ENOSPC;
	}

	/*
	 * Increase the count in atomic context.
	 */
	be_soc->num_ppe_vp_entries++;

	for (i = 0; i < num_ppe_vp_max; i++) {
		if (!be_soc->ppe_vp_tbl[i].is_configured)
			break;
	}

	/*
	 * As we already found the current count not maxed out, we should
	 * always find an non configured entry.
	 */
	qdf_assert_always(!(i == num_ppe_vp_max));

	be_soc->ppe_vp_tbl[i].is_configured = true;
	qdf_mutex_release(&be_soc->ppe_vp_tbl_lock);
	return i;
}

/**
 * dp_ppeds_dealloc_vp_tbl_entry_be() - PPE VP entry dealloc
 * @be_soc: BE SoC
 * @ppe_vp_num_idx: PPE VP index
 *
 * PPE VP table entry dealloc
 *
 * Return: void
 */
static void dp_ppeds_dealloc_vp_tbl_entry_be(struct dp_soc_be *be_soc,
					      int ppe_vp_num_idx)
{
	int num_ppe_vp_max;
	union hal_tx_ppe_vp_config vp_cfg = {0};

	num_ppe_vp_max =
		hal_tx_get_num_ppe_vp_tbl_entries(be_soc->soc.hal_soc);

	if (ppe_vp_num_idx < 0 || ppe_vp_num_idx >= num_ppe_vp_max) {
		dp_err("INvalid PPE VP free index");
		return;
	}

	/*
	 * Clear the HAL specific register for this PPE VP index.
	 */
	hal_tx_populate_ppe_vp_entry(be_soc->soc.hal_soc,
				     &vp_cfg,
				     ppe_vp_num_idx);

	/*
	 * Release the current index for reuse.
	 */
	qdf_mutex_acquire(&be_soc->ppe_vp_tbl_lock);
	if (!be_soc->ppe_vp_tbl[ppe_vp_num_idx].is_configured) {
		qdf_mutex_release(&be_soc->ppe_vp_tbl_lock);
		dp_err("PPE VP is not configured at idx:%d", ppe_vp_num_idx);
		return;
	}

	be_soc->ppe_vp_tbl[ppe_vp_num_idx].is_configured = false;
	be_soc->num_ppe_vp_entries--;
	qdf_mutex_release(&be_soc->ppe_vp_tbl_lock);
}

/**
 * dp_ppeds_tx_comp_poll() - ppeds tx completion napi poll
 * @napi: napi handle
 * @budget: Budget
 *
 * Ppeds tx completion napi poll funciton
 *
 * Return: Work done
 */
static int dp_ppeds_tx_comp_poll(struct napi_struct *napi, int budget)
{
	int work_done;
	int shift = 2;
	int norm_budget = DS_NAPI_BUDGET_TO_INTERNAL_BUDGET(budget, shift);
	struct dp_soc_be *be_soc =
		qdf_container_of(napi, struct dp_soc_be, ppeds_napi_ctxt.napi);

	work_done = dp_ppeds_tx_comp_handler(be_soc, norm_budget);
	work_done = DS_INTERNAL_BUDGET_TO_NAPI_BUDGET(work_done, shift);

	if (budget > work_done) {
		napi_complete(napi);
		dp_ppeds_enable_irq(&be_soc->soc,
				    &be_soc->ppeds_wbm_release_ring);
	}

	return (work_done > budget) ? budget : work_done;
}

/**
 * dp_ppeds_del_napi_ctxt() - delete ppeds tx completion napi context
 * @be_soc: BE SoC
 *
 * Delete ppeds tx completion napi context
 *
 * Return: None
 */
static void dp_ppeds_del_napi_ctxt(struct dp_soc_be *be_soc)
{
	struct dp_ppeds_napi *napi_ctxt = &be_soc->ppeds_napi_ctxt;

	netif_napi_del(&napi_ctxt->napi);
}

/**
 * dp_ppeds_add_napi_ctxt() - Add ppeds tx completion napi context
 * @be_soc: BE SoC
 *
 * Add ppeds tx completion napi context
 *
 * Return: None
 */
static void dp_ppeds_add_napi_ctxt(struct dp_soc_be *be_soc)
{
	struct dp_soc *soc = DP_SOC_BE_GET_SOC(be_soc);
	struct dp_ppeds_napi *napi_ctxt = &be_soc->ppeds_napi_ctxt;
	int napi_budget =
	wlan_cfg_get_dp_soc_ppeds_tx_comp_napi_budget(soc->wlan_cfg_ctx);

	qdf_net_if_create_dummy_if((struct qdf_net_if *)&napi_ctxt->ndev);

	netif_napi_add(&napi_ctxt->ndev, &napi_ctxt->napi,
		       dp_ppeds_tx_comp_poll, napi_budget);
}

/**
 * dp_ppeds_tx_desc_pool_alloc() - PPE DS allocate tx desc pool
 * @soc: SoC
 * @num_elem: num of dec elements
 *
 * PPE DS allocate tx desc pool
 *
 * Return: status
 */
static QDF_STATUS
dp_ppeds_tx_desc_pool_alloc(struct dp_soc *soc, uint16_t num_elem)
{
	uint32_t desc_size;
	struct dp_ppeds_tx_desc_pool_s *tx_desc_pool;
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);

	desc_size =  qdf_get_pwr2(sizeof(struct dp_tx_desc_s));
	tx_desc_pool = &be_soc->ppeds_tx_desc;
	dp_desc_multi_pages_mem_alloc(soc, DP_TX_PPEDS_DESC_TYPE,
				      &tx_desc_pool->desc_pages,
				      desc_size, num_elem,
				      0, true);

	if (!tx_desc_pool->desc_pages.num_pages) {
		dp_err("Multi page alloc fail, tx desc");
		return QDF_STATUS_E_NOMEM;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ppeds_tx_desc_pool_free() - PPE DS free tx desc pool
 * @soc: SoC
 *
 * PPE DS free tx desc pool
 *
 * Return: none
 */
static void dp_ppeds_tx_desc_pool_free(struct dp_soc *soc)
{
	struct dp_ppeds_tx_desc_pool_s *tx_desc_pool;
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);

	tx_desc_pool = &be_soc->ppeds_tx_desc;

	if (tx_desc_pool->desc_pages.num_pages)
		dp_desc_multi_pages_mem_free(soc, DP_TX_PPEDS_DESC_TYPE,
					     &tx_desc_pool->desc_pages, 0,
					     true);
}

/**
 * dp_ppeds_tx_desc_pool_setup() - PPE DS tx desc pool setup
 * @soc: SoC
 * @num_elem: num of dec elements
 * @pool_id: Pool id
 *
 * PPE DS tx desc pool setup
 *
 * Return: status
 */
static QDF_STATUS dp_ppeds_tx_desc_pool_setup(struct dp_soc *soc,
					      uint16_t num_elem,
					      uint8_t pool_id)
{
	struct dp_ppeds_tx_desc_pool_s *tx_desc_pool;
	struct dp_hw_cookie_conversion_t *cc_ctx;
	struct dp_soc_be *be_soc;
	struct dp_spt_page_desc *page_desc;
	struct dp_tx_desc_s *tx_desc;
	uint32_t ppt_idx = 0;
	uint32_t avail_entry_index = 0;

	if (!num_elem) {
		dp_err("desc_num 0 !!");
		return QDF_STATUS_E_FAILURE;
	}

	be_soc = dp_get_be_soc_from_dp_soc(soc);
	tx_desc_pool = &be_soc->ppeds_tx_desc;
	cc_ctx  = &be_soc->ppeds_tx_cc_ctx;

	tx_desc = tx_desc_pool->freelist;
	page_desc = &cc_ctx->page_desc_base[0];
	while (tx_desc) {
		if (avail_entry_index == 0) {
			if (ppt_idx >= cc_ctx->total_page_num) {
				dp_alert("insufficient secondary page tables");
				qdf_assert_always(0);
			}
			page_desc = &cc_ctx->page_desc_base[ppt_idx++];
		}

		/* put each TX Desc VA to SPT pages and
		 * get corresponding ID
		 */
		DP_CC_SPT_PAGE_UPDATE_VA(page_desc->page_v_addr,
					 avail_entry_index,
					 tx_desc);
		tx_desc->id =
			dp_cc_desc_id_generate(page_desc->ppt_index,
					       avail_entry_index);
		tx_desc->pool_id = pool_id;
		dp_tx_desc_set_magic(tx_desc, DP_TX_MAGIC_PATTERN_FREE);
		tx_desc = tx_desc->next;
		avail_entry_index = (avail_entry_index + 1) &
					DP_CC_SPT_PAGE_MAX_ENTRIES_MASK;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ppeds_tx_desc_pool_init() - PPE DS tx desc pool init
 * @soc: SoC
 *
 * PPE DS tx desc pool init
 *
 * Return: status
 */
static QDF_STATUS dp_ppeds_tx_desc_pool_init(struct dp_soc *soc)
{
	struct dp_ppeds_tx_desc_pool_s *tx_desc_pool;
	uint32_t desc_size, num_elem;
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);

	desc_size = qdf_get_pwr2(sizeof(struct dp_tx_desc_s));
	num_elem = wlan_cfg_get_dp_soc_ppeds_num_tx_desc(soc->wlan_cfg_ctx);
	tx_desc_pool = &be_soc->ppeds_tx_desc;

	if (qdf_mem_multi_page_link(soc->osdev,
				    &tx_desc_pool->desc_pages,
				    desc_size, num_elem, true)) {
		dp_err("invalid tx desc allocation -overflow num link");
		return QDF_STATUS_E_FAULT;
	}

	tx_desc_pool->freelist = (struct dp_tx_desc_s *)
		*tx_desc_pool->desc_pages.cacheable_pages;

	/* Set unique IDs for each Tx descriptor */
	if (dp_ppeds_tx_desc_pool_setup(soc, num_elem, DP_TX_PPEDS_POOL_ID) !=
					QDF_STATUS_SUCCESS) {
		dp_err("ppeds_tx_desc_pool_setup failed");
		return QDF_STATUS_E_FAULT;
	}

	tx_desc_pool->elem_size = desc_size;
	tx_desc_pool->num_free = num_elem;
	tx_desc_pool->num_allocated = 0;

	TX_DESC_LOCK_CREATE(&tx_desc_pool->lock);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ppeds_tx_desc_free_nolock() - PPE DS tx desc free nolocks
 * @be_soc: SoC
 * @tx_desc: tx desc
 *
 * PPE DS tx desc free witout locks
 *
 * Return: void
 */
static
void dp_ppeds_tx_desc_free_nolock(struct dp_soc *soc,
				  struct dp_tx_desc_s *tx_desc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_ppeds_tx_desc_pool_s *pool = NULL;

	tx_desc->nbuf = NULL;
	tx_desc->flags = 0;

	pool = &be_soc->ppeds_tx_desc;
	tx_desc->next = pool->freelist;
	pool->freelist = tx_desc;
	pool->num_allocated--;
	pool->num_free++;
}

/**
 * dp_ppeds_tx_desc_clean_up() -  Clean up the ppeds tx dexcriptors
 * @ctxt: context passed
 * @elem: element to be cleaned up
 * @elem_list: element list
 *
 */
static
void dp_ppeds_tx_desc_clean_up(void *ctxt, void *elem, void *elem_list)
{
	struct dp_soc *soc = (struct dp_soc *)ctxt;
	struct dp_tx_desc_s *tx_desc = (struct dp_tx_desc_s *)elem;

	if (tx_desc->nbuf) {
		qdf_nbuf_free(tx_desc->nbuf);
		dp_ppeds_tx_desc_free_nolock(soc, tx_desc);
	}
}

/**
 * dp_ppeds_tx_desc_pool_cleanup() - PPE DS tx desc pool cleanup
 * @soc: SoC
 * @tx_desc_pool: TX desc pool
 *
 * PPE DS tx desc pool cleanup
 *
 * Return: void
 */
static void
dp_ppeds_tx_desc_pool_cleanup(struct dp_soc_be *be_soc,
			      struct dp_ppeds_tx_desc_pool_s *tx_desc_pool)
{
	struct dp_spt_page_desc *page_desc;
	int i = 0;
	struct dp_hw_cookie_conversion_t *cc_ctx;
	struct dp_ppeds_tx_desc_pool_s *pool = &be_soc->ppeds_tx_desc;

	if (pool) {
		TX_DESC_LOCK_LOCK(&pool->lock);

		qdf_tx_desc_pool_free_bufs(&be_soc->soc, &pool->desc_pages,
					   pool->elem_size, pool->elem_count,
					   true, &dp_ppeds_tx_desc_clean_up,
					   NULL);

		pool->freelist = NULL;
		TX_DESC_LOCK_UNLOCK(&pool->lock);
	}

	cc_ctx  = &be_soc->ppeds_tx_cc_ctx;

	for (i = 0; i < cc_ctx->total_page_num; i++) {
		page_desc = &cc_ctx->page_desc_base[i];
		qdf_mem_zero(page_desc->page_v_addr, qdf_page_size);
	}
}

/**
 * dp_ppeds_tx_desc_pool_deinit() - PPE DS tx desc pool deinitialize
 * @be_soc: SoC
 *
 * PPE DS tx desc pool deinit
 *
 * Return: void
 */
static void dp_ppeds_tx_desc_pool_deinit(struct dp_soc_be *be_soc)
{
	struct dp_ppeds_tx_desc_pool_s *tx_desc_pool;

	tx_desc_pool = &be_soc->ppeds_tx_desc;
	dp_ppeds_tx_desc_pool_cleanup(be_soc, tx_desc_pool);
	TX_DESC_POOL_MEMBER_CLEAN(tx_desc_pool);
	TX_DESC_LOCK_DESTROY(&tx_desc_pool->lock);
}

/**
 * dp_ppeds_tx_desc_alloc() - PPE DS tx desc alloc
 * @be_soc: SoC
 *
 * PPE DS tx desc alloc
 *
 * Return: tx desc
 */
static inline
struct dp_tx_desc_s *dp_ppeds_tx_desc_alloc(struct dp_soc_be *be_soc)
{
	struct dp_tx_desc_s *tx_desc = NULL;
	struct dp_ppeds_tx_desc_pool_s *pool = &be_soc->ppeds_tx_desc;

	TX_DESC_LOCK_LOCK(&pool->lock);

	tx_desc = pool->freelist;

	/* Pool is exhausted */
	if (!tx_desc) {
		TX_DESC_LOCK_UNLOCK(&pool->lock);
		return NULL;
	}

	pool->freelist = pool->freelist->next;
	pool->num_allocated++;
	pool->num_free--;
	dp_tx_prefetch_desc(pool->freelist);

	tx_desc->flags = DP_TX_DESC_FLAG_ALLOCATED;

	TX_DESC_LOCK_UNLOCK(&pool->lock);

	return tx_desc;
}

/**
 * dp_ppeds_tx_desc_free() - PPE DS tx desc free
 * @be_soc: SoC
 * @tx_desc: tx desc
 *
 * PPE DS tx desc free
 *
 * Return: void
 */
void dp_ppeds_tx_desc_free(struct dp_soc *soc, struct dp_tx_desc_s *tx_desc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_ppeds_tx_desc_pool_s *pool = NULL;

	tx_desc->nbuf = NULL;
	tx_desc->flags = 0;

	pool = &be_soc->ppeds_tx_desc;
	TX_DESC_LOCK_LOCK(&pool->lock);
	tx_desc->next = pool->freelist;
	pool->freelist = tx_desc;
	pool->num_allocated--;
	pool->num_free++;
	TX_DESC_LOCK_UNLOCK(&pool->lock);
}

/**
 * dp_ppeds_get_batched_tx_desc() - Get Tx descriptors in a buffer array
 * @ppeds_handle: PPE-DS WLAN handle
 * @arr: Tx buffer array to be filled
 * @num_buff_req: Number of buffer to be allocated
 * @buff_size: Size of the buffer to be allocated
 * @headroom: Headroom required in the allocated buffer
 *
 * PPE-DS get buffer and the Tx descriptors in an array
 *
 * Return: Number of buffers allocated
 */
static
uint32_t dp_ppeds_get_batched_tx_desc(ppe_ds_wlan_handle_t *ppeds_handle,
				      struct ppe_ds_wlan_txdesc_elem *arr,
				      uint32_t num_buff_req,
				      uint32_t buff_size,
				      uint32_t headroom)
{
	uint32_t i = 0;
	struct dp_tx_desc_s *tx_desc;
	struct dp_soc *soc = *((struct dp_soc **)ppe_ds_wlan_priv(ppeds_handle));
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_ppeds_tx_desc_pool_s *pool = &be_soc->ppeds_tx_desc;
	qdf_dma_addr_t paddr;

	/*
	 * If the tx desc pool is empty, we need to return.
	 */
	if (!pool->freelist) {
		dp_err("ran out of txdesc");
		return 0;
	}

	for (i = 0; i < num_buff_req; i++) {
		qdf_nbuf_t nbuf = NULL;
		/*
		 * Get a free skb.
		 */
		nbuf = dp_nbuf_alloc_ppe_ds(soc->osdev, buff_size, 0, 0, 0);
		if (qdf_unlikely(!nbuf)) {
			break;
		}

		/*
		 * Reserve headrrom
		 */
		qdf_nbuf_reserve(nbuf, headroom);

		/*
		 * Map(Get Phys address)
		 */
		if (!nbuf->recycled_for_ds) {
			qdf_nbuf_dma_inv_range_no_dsb((void *)nbuf->data,
				(void *)(nbuf->data + buff_size - headroom));
			nbuf->recycled_for_ds = 1;
		}
		paddr = (qdf_dma_addr_t)qdf_mem_virt_to_phys(nbuf->data);

		tx_desc = dp_ppeds_tx_desc_alloc(be_soc);
		if (!tx_desc) {
			qdf_nbuf_free_simple(nbuf);
			dp_err("ran out of txdesc");
			break;
		}

		tx_desc->flags |= DP_TX_DESC_FLAG_PPEDS;
		tx_desc->pdev = NULL;
		tx_desc->nbuf = nbuf;

		/*
		 * If the skb is from recycler, then add the FAST flag in
		 * the Tx descriptor so that at the skb's can be freed in
		 * the batched mode rather than the conventional mode (freed
		 * one at a time).
		 */
		if (nbuf->is_from_recycler) {
			tx_desc->flags |= DP_TX_DESC_FLAG_FAST;
		}

		arr[i].opaque_lo = tx_desc->id;
		arr[i].opaque_hi = 0;
		arr[i].buff_addr = paddr;
	}

	return i;
}

/**
 * dp_ppeds_release_rx_desc() - Release the Rx descriptors from buffer array
 * @ppeds_handle: PPE-DS WLAN handle
 * @arr: Rx buffer array containing Rx descriptors
 * @count: Number of Rx descriptors to be freed
 *
 * Return: void
 */
static void dp_ppeds_release_rx_desc(ppe_ds_wlan_handle_t *ppeds_handle,
					struct ppe_ds_wlan_rxdesc_elem *arr, uint16_t count)
{
	struct dp_rx_desc *rx_desc;
	struct dp_soc *soc = *((struct dp_soc **)ppe_ds_wlan_priv(ppeds_handle));
	union dp_rx_desc_list_elem_t *head[WLAN_MAX_MLO_CHIPS][MAX_PDEV_CNT] = {0};
	union dp_rx_desc_list_elem_t *tail[WLAN_MAX_MLO_CHIPS][MAX_PDEV_CNT] = {0};
	uint32_t rx_bufs_reaped[WLAN_MAX_MLO_CHIPS][MAX_PDEV_CNT] = {0};
	struct dp_srng *dp_rxdma_srng;
	struct rx_desc_pool *rx_desc_pool;
	struct dp_soc *replenish_soc;
	uint32_t i, mac_id;
	uint8_t chip_id;
	qdf_nbuf_queue_head_t h;

	qdf_nbuf_queue_head_init(&h);
	for (i = 0; i < count; i++) {
		rx_desc = (struct dp_rx_desc *)arr[i].cookie;
		if (rx_desc == NULL) {
			dp_err("Rx descriptor is NULL\n");
			continue;
		}
		rx_bufs_reaped[rx_desc->chip_id][rx_desc->pool_id]++;

		qdf_nbuf_dev_queue_head(&h, rx_desc->nbuf);
		rx_desc->nbuf->recycled_for_ds = 1;

		dp_rx_add_to_free_desc_list(&head[rx_desc->chip_id][rx_desc->pool_id],
				&tail[rx_desc->chip_id][rx_desc->pool_id], rx_desc);
	}
	qdf_nbuf_dev_kfree_list(&h);

	for (chip_id = 0; chip_id < WLAN_MAX_MLO_CHIPS; chip_id++) {
		for (mac_id = 0; mac_id < MAX_PDEV_CNT; mac_id++) {

			if (!rx_bufs_reaped[chip_id][mac_id])
				continue;

			replenish_soc = dp_rx_replensih_soc_get(soc, chip_id);

			dp_rxdma_srng = &replenish_soc->rx_refill_buf_ring[mac_id];
			rx_desc_pool = &replenish_soc->rx_desc_buf[mac_id];

			dp_rx_buffers_replenish_simple(replenish_soc, mac_id, dp_rxdma_srng,
					rx_desc_pool, rx_bufs_reaped[chip_id][mac_id],
					&head[chip_id][mac_id], &tail[chip_id][mac_id]);
		}
	}
}

/**
 * dp_ppeds_release_tx_desc_single() - Release the Tx descriptor
 * @ppeds_handle: PPE-DS WLAN handle
 * @cookie: Cookie to get the descriptor to be freed
 *
 * PPE-DS release single Tx descriptor API
 *
 * Return: void
 */
static
void dp_ppeds_release_tx_desc_single(ppe_ds_wlan_handle_t *ppeds_handle,
				     uint32_t cookie)
{
	return;
}

/**
 * dp_ppeds_set_tcl_prod_idx() - Set produce index of the PPE2TCL ring
 * @ppeds_handle: PPE-DS WLAN handle
 * @tcl_prod_idx: Producer index to be set
 *
 * Set producer index of the PPE2TCL ring
 *
 * Return: void
 */
static void dp_ppeds_set_tcl_prod_idx(ppe_ds_wlan_handle_t *ppeds_handle,
				      uint16_t tcl_prod_idx)
{
	struct dp_soc_be *soc =
			*((struct dp_soc_be **)ppe_ds_wlan_priv(ppeds_handle));
	struct dp_soc *dpsoc = (struct dp_soc *)soc;
	struct dp_srng *ppe2tcl_ring = &soc->ppe2tcl_ring;

	hal_srng_src_set_hp(ppe2tcl_ring->hal_srng, tcl_prod_idx);
	dp_tx_ring_access_end_wrapper(dpsoc, ppe2tcl_ring->hal_srng, 0);
}

/**
 * dp_ppeds_get_tcl_cons_idx() - Get consumer index of the PPE2TCL ring
 * @ppeds_handle: PPE-DS WLAN handle
 *
 * Get consumer index of the PPE2TCL ring
 *
 * Return: Consumer index of the PPE2TCL ring
 */
static uint16_t dp_ppeds_get_tcl_cons_idx(ppe_ds_wlan_handle_t *ppeds_handle)
{
	struct dp_soc_be *soc =
			*((struct dp_soc_be **)ppe_ds_wlan_priv(ppeds_handle));
	struct dp_srng *ppe2tcl_ring = &soc->ppe2tcl_ring;

	return hal_srng_src_get_tpidx(ppe2tcl_ring->hal_srng);
}

/**
 * dp_ppeds_set_reo_cons_idx() - Set consumer index of the REO2PPE ring
 * @ppeds_handle: PPE-DS WLAN handle
 * @tcl_prod_idx: Producer index to be set
 *
 * Set the consumer index of the PPE2TCL ring
 *
 * Return: void
 */
static void dp_ppeds_set_reo_cons_idx(ppe_ds_wlan_handle_t *ppeds_handle,
				      uint16_t reo_cons_idx)
{
	struct dp_soc_be *soc =
			*((struct dp_soc_be **)ppe_ds_wlan_priv(ppeds_handle));
	struct dp_soc *dpsoc = (struct dp_soc *)soc;
	struct dp_srng *reo2ppe_ring = &soc->reo2ppe_ring;

	hal_srng_dst_set_tp(reo2ppe_ring->hal_srng, reo_cons_idx);
	hal_srng_access_end_unlocked(dpsoc->hal_soc, reo2ppe_ring->hal_srng);
}

/**
 * dp_ppeds_get_reo_prod_idx() - Get producer index of the REO2PPE ring
 * @ppeds_handle: PPE-DS WLAN handle
 *
 * Get producer index of the REO2PPE ring
 *
 * Return: Producer index of the REO2PPE ring
 */
static uint16_t dp_ppeds_get_reo_prod_idx(ppe_ds_wlan_handle_t *ppeds_handle)
{
	struct dp_soc_be *soc =
			*((struct dp_soc_be **)ppe_ds_wlan_priv(ppeds_handle));
	struct dp_srng *reo2ppe_ring = &soc->reo2ppe_ring;

	return hal_srng_dst_get_hpidx(reo2ppe_ring->hal_srng);
}

/**
 * dp_ppeds_toggle_srng_intr() - Enable/Disable irq of ppe2tcl ring
 * @ppeds_handle: PPE-DS WLAN handle
 * @enable: enable/disable bit
 *
 * Return: None
 */
static inline void
dp_ppeds_enable_srng_intr(ppe_ds_wlan_handle_t *ppeds_handle, bool enable)
{
	struct dp_soc *soc =
			*((struct dp_soc **)ppe_ds_wlan_priv(ppeds_handle));
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);

	if (enable)
		dp_ppeds_enable_irq(&be_soc->soc, &be_soc->ppe2tcl_ring);
	else
		dp_ppeds_disable_irq(&be_soc->soc, &be_soc->ppe2tcl_ring);
}

/**
 * ppeds_ops
 *	PPE-DS WLAN operations
 */
static struct ppe_ds_wlan_ops ppeds_ops = {
	.get_tx_desc_many = dp_ppeds_get_batched_tx_desc,
	.release_tx_desc_single = dp_ppeds_release_tx_desc_single,
	.enable_tx_consume_intr = dp_ppeds_enable_srng_intr,
	.set_tcl_prod_idx  = dp_ppeds_set_tcl_prod_idx,
	.set_reo_cons_idx = dp_ppeds_set_reo_cons_idx,
	.get_tcl_cons_idx = dp_ppeds_get_tcl_cons_idx,
	.get_reo_prod_idx = dp_ppeds_get_reo_prod_idx,
	.release_rx_desc = dp_ppeds_release_rx_desc,
};

/**
 * dp_ppeds_attach_vdev_be() - PPE DS table entry alloc
 * @soc_hdl: CDP SoC Tx/Rx handle
 * @vdev_id: vdev id
 * @vp_arg: PPE VP opaque
 * @ppe_vp_num: Allocated PPE VP number
 *
 * Allocate a DS VP port and attach to BE VAP
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ppeds_attach_vdev_be(struct cdp_soc_t *soc_hdl, uint8_t vdev_id,
				   void *vp_arg, int32_t *ppe_vp_num)
{
	ol_osif_vdev_handle osif;
	struct net_device *dev;
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_soc_be *be_soc;
	struct dp_vdev *vdev;
	struct dp_vdev_be *be_vdev;
	struct dp_ppe_vp_profile *vp_profile;
	int8_t ppe_vp_idx;

	be_soc = dp_get_be_soc_from_dp_soc(soc);

	/*
	 * Enable once PPEDS module is ready
	 */
	if (!be_soc->ppeds_handle) {
		dp_err("DS is not enabled on this SOC");
		return QDF_STATUS_E_INVAL;
	}

	vdev = dp_vdev_get_ref_by_id(soc, vdev_id, DP_MOD_ID_CDP);
	if (!vdev) {
		dp_err("%p: Could not find vdev for id:%d", be_soc, vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	/*
	 * Check if PPEDS could be enabled for Vdev
	 */
	if (vdev->wrap_vdev || vdev->nawds_enabled ||
	    vdev->opmode == wlan_op_mode_monitor ||
	    vdev->mesh_vdev || vdev->mesh_rx_filter) {
		dp_err("Unsupported Vdev config for id:%d", vdev_id);
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
		return QDF_STATUS_E_NOSUPPORT;
	}

	osif = vdev->osif_vdev;
	dev = OSIF_TO_NETDEV(osif);
	if (!dev) {
		dp_err("Could not find netdev");
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
		return QDF_STATUS_E_INVAL;
	}

	*ppe_vp_num = ppe_ds_wlan_vp_alloc(be_soc->ppeds_handle, dev, vp_arg);
	if (*ppe_vp_num < 0) {
		dp_err("vp alloc failed\n");
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
		return QDF_STATUS_E_FAULT;
	}

	/*
	 * Extract the VP profile from the BE vap.
	 */
	be_vdev = dp_get_be_vdev_from_dp_vdev(vdev);
	vp_profile = &be_vdev->ppe_vp_profile;

	/*
	 * Test and set the PPE-DS enabled value.
	 */
	if (be_vdev->ppe_vp_enabled == PPE_VP_USER_TYPE_DS) {
		dp_err("%p: PPE DS is already enabled on this vdev_id:%d",
		       be_soc, vdev_id);
		ppe_ds_wlan_vp_free(be_soc->ppeds_handle, *ppe_vp_num);
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
		return QDF_STATUS_E_ALREADY;
	} else
		be_vdev->ppe_vp_enabled = PPE_VP_USER_TYPE_DS;

	ppe_vp_idx = dp_ppeds_alloc_vp_tbl_entry_be(be_soc, be_vdev);
	if (ppe_vp_idx < 0) {
		dp_err("%p: Failed to allocate PPE VP idx for vdev_id:%d",
		       be_soc, vdev->vdev_id);
		ppe_ds_wlan_vp_free(be_soc->ppeds_handle, *ppe_vp_num);
		be_vdev->ppe_vp_enabled = 0;
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
		return QDF_STATUS_E_RESOURCES;
	}

	vp_profile->vp_num = *ppe_vp_num;
	vp_profile->ppe_vp_num_idx = ppe_vp_idx;
	vp_profile->to_fw = 0;
	vp_profile->use_ppe_int_pri = 0;
	vp_profile->drop_prec_enable = 0;

	if (vdev->opmode == wlan_op_mode_sta)
		be_vdev->ppe_vp_profile.search_idx_reg_num =
					DP_PPEDS_STAMODE_ASTIDX_MAP_REG_IDX;

	/*
	 * For the sta mode fill up the index reg number.
	 */
	dp_ppeds_setup_vp_entry_be(be_soc, be_vdev);
	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ppeds_detach_vdev_be() - Detach the PPE DS port from vdev
 * @soc_hdl: CDP SoC Tx/Rx handle
 * @vdev_id: vdev id
 *
 * Detach the PPE DS port from BE VAP
 *
 * Return: void
 */
void dp_ppeds_detach_vdev_be(struct cdp_soc_t *soc_hdl, uint8_t vdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_soc_be *be_soc;
	struct dp_ppe_vp_profile *vp_profile;
	struct dp_vdev *vdev;
	struct dp_vdev_be *be_vdev;

	if (!soc) {
		dp_err("CDP soch handle is NULL");
		return;
	}

	be_soc = dp_get_be_soc_from_dp_soc(soc);
	vdev = dp_vdev_get_ref_by_id(soc, vdev_id, DP_MOD_ID_CDP);
	if (!vdev) {
		dp_err("%p: Could not find vdev for id:%d", be_soc, vdev_id);
		return;
	}

	/*
	 * Extract the VP profile from the BE vap.
	 */
	be_vdev = dp_get_be_vdev_from_dp_vdev(vdev);
	vp_profile = &be_vdev->ppe_vp_profile;

	if (vp_profile->ppe_vp_num_idx == -1) {
		dp_err("%p: Invalid PPE VP num for vdev_id:%d",
			be_vdev, vdev_id);
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
		return;
	}

	ppe_ds_wlan_vp_free(be_soc->ppeds_handle, vp_profile->vp_num);

	dp_ppeds_dealloc_vp_tbl_entry_be(be_soc, vp_profile->ppe_vp_num_idx);

	/*
	 * Test and reset PPE-DS enabled value
	 */
	if (be_vdev->ppe_vp_enabled != PPE_VP_USER_TYPE_DS)
		dp_err("%p: Invalid vdev got detached with vdev_id:%d",
			be_vdev, vdev_id);
	else
		be_vdev->ppe_vp_enabled = 0;

	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
}

/*
 * dp_ppeds_set_int_pri2tid_be() - Set up INT_PRI to TID
 * @soc_hdl: CDP SoC Tx/Rx handle
 * @pri2tid: Priority to TID table
 *
 * Setup the INT_PRI to TID table
 *
 * Return: void
 */
void dp_ppeds_set_int_pri2tid_be(struct cdp_soc_t *soc_hdl,
				  uint8_t *pri2tid)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_soc_be *be_soc;
	union hal_tx_ppe_pri2tid_map0_config map0 = {0};
	union hal_tx_ppe_pri2tid_map1_config map1 = {0};
	uint32_t value = 0;

	if (!soc) {
		dp_err("CDP soch handle is NULL");
		return;
	}

	be_soc = dp_get_be_soc_from_dp_soc(soc);

	map0.int_pri0 = pri2tid[0];
	map0.int_pri1 = pri2tid[1];
	map0.int_pri2 = pri2tid[2];
	map0.int_pri3 = pri2tid[3];
	map0.int_pri4 = pri2tid[4];
	map0.int_pri5 = pri2tid[5];
	map0.int_pri6 = pri2tid[6];
	map0.int_pri7 = pri2tid[7];
	map0.int_pri8 = pri2tid[8];
	map0.int_pri9 = pri2tid[9];

	value = map0.val;

	hal_tx_set_int_pri2tid(be_soc->soc.hal_soc, value, 0);
	map1.int_pri10 = pri2tid[10];
	map1.int_pri11 = pri2tid[11];
	map1.int_pri12 = pri2tid[12];
	map1.int_pri13 = pri2tid[13];
	map1.int_pri14 = pri2tid[14];
	map1.int_pri15 = pri2tid[15];

	value = map1.val;

	hal_tx_set_int_pri2tid(be_soc->soc.hal_soc, value, 1);
}

/**
 * dp_ppeds_update_int_pri2tid_be() - Update INT_PRI to TID
 * @soc_hdl: CDP SoC Tx/Rx handle
 * @pri: Priority value
 * @tid: TID mapped to the priority value
 *
 * Update the tid for a specific INT_PRI
 *
 * Return: void
 */
void dp_ppeds_update_int_pri2tid_be(struct cdp_soc_t *soc_hdl,
				     uint8_t pri, uint8_t tid)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_soc_be *be_soc;

	if (!soc) {
		dp_err("CDP soch handle is NULL");
		return;
	}

	be_soc = dp_get_be_soc_from_dp_soc(soc);
	hal_tx_update_int_pri2tid(be_soc->soc.hal_soc, pri, tid);
}

/**
 * dp_ppeds_dump_ppe_vp_tbl_be() - Dump the PPE VP entries
 * @soc_hdl: CDP SoC Tx/Rx handle
 *
 * Dump the PPE VP entries of a SoC
 *
 * Return: void
 */
void dp_ppeds_dump_ppe_vp_tbl_be(struct cdp_soc_t *soc_hdl)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_soc_be *be_soc;

	if (!soc) {
		dp_err("CDP soch handle is NULL");
		return;
	}

	be_soc = dp_get_be_soc_from_dp_soc(soc);

	qdf_mutex_acquire(&be_soc->ppe_vp_tbl_lock);
	hal_tx_dump_ppe_vp_entry(be_soc->soc.hal_soc);
	qdf_mutex_release(&be_soc->ppe_vp_tbl_lock);
}

/**
 * dp_ppeds_vdev_enable_pri2tid_be() - Enable PPE PRI2TID conversion for a vap
 * @soc_hdl: CDP SoC Tx/Rx handle
 * @vdev_id : vdev id
 * @val: Boolean to enable/disable PRI2TID mapping
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ppeds_vdev_enable_pri2tid_be(struct cdp_soc_t *soc_hdl,
				       uint8_t vdev_id, bool val)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_soc_be *be_soc;
	struct dp_vdev *vdev;
	struct dp_vdev_be *be_vdev;
	struct dp_ppe_vp_profile *vp_profile;
	int8_t ppe_vp_idx;

	if (!soc) {
		dp_err("CDP soc handle is NULL");
		return QDF_STATUS_E_FAULT;
	}

	be_soc = dp_get_be_soc_from_dp_soc(soc);
	vdev = dp_vdev_get_ref_by_id(soc, vdev_id, DP_MOD_ID_CDP);
	if (!vdev) {
		dp_err("%p: Could not find vdev for id:%d", be_soc, vdev_id);
		return QDF_STATUS_E_FAULT;
	}

	/*
	 * Extract the VP profile from the BE vap.
	 */
	be_vdev = dp_get_be_vdev_from_dp_vdev(vdev);
	vp_profile = &be_vdev->ppe_vp_profile;

	/*
	 * Test PPE-DS enabled value
	 */
	if (be_vdev->ppe_vp_enabled != PPE_VP_USER_TYPE_DS) {
		dp_err("%p: PPE-DS is not enabled on this vdev_id:%d",
		       be_soc, vdev_id);
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
		return QDF_STATUS_E_FAULT;
	}

	ppe_vp_idx = vp_profile->ppe_vp_num_idx;

	hal_tx_enable_pri2tid_map(be_soc->soc.hal_soc, val, ppe_vp_idx);
	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ppeds_stop_soc_be() - Stop the PPE-DS instance
 * @soc: DP SoC
 *
 * Return: void
 */
void dp_ppeds_stop_soc_be(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_ppeds_napi *napi_ctxt = &be_soc->ppeds_napi_ctxt;

	if (!be_soc->ppeds_handle || be_soc->ppeds_stopped)
		return;

	be_soc->ppeds_stopped = 1;

	dp_info("stopping ppe ds for soc %pK ", soc);

	napi_disable(&napi_ctxt->napi);

	ppe_ds_wlan_inst_stop(be_soc->ppeds_handle);
	dp_ppeds_tx_desc_pool_deinit(be_soc);
}

/**
 * dp_ppeds_start_soc_be() - Start the PPE-DS instance
 * @soc: DP SoC
 *
 * Return: void
 */
QDF_STATUS dp_ppeds_start_soc_be(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_ppeds_napi *napi_ctxt = &be_soc->ppeds_napi_ctxt;

	if (!be_soc->ppeds_handle) {
		dp_err("%p: ppeds not allocated", soc);
		return QDF_STATUS_SUCCESS;
	}

	if (dp_ppeds_tx_desc_pool_init(soc) != QDF_STATUS_SUCCESS) {
		dp_err("%p: ppeds tx desc pool init failed", soc);
		return QDF_STATUS_SUCCESS;
	}

	napi_enable(&napi_ctxt->napi);

	be_soc->ppeds_stopped = 0;

	dp_info("starting ppe ds for soc %pK ", soc);

	if (ppe_ds_wlan_inst_start(be_soc->ppeds_handle) != 0) {
		dp_err("%p: ppeds start failed", soc);
		dp_ppeds_tx_desc_pool_deinit(be_soc);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ppeds_register_soc_be() - Register the PPE-DS instance
 * @be_soc: BE SoC
 * @idx: ppeds indices
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ppeds_register_soc_be(struct dp_soc_be *be_soc,
				    struct dp_ppe_ds_idxs *idx)
{
	struct ppe_ds_wlan_reg_info reg_info = {0};
	struct dp_soc *soc = DP_SOC_BE_GET_SOC(be_soc);

	if (!be_soc->ppeds_handle) {
		dp_err("%p: ppeds not attached", soc);
		return QDF_STATUS_SUCCESS;
	}

	reg_info.ppe2tcl_ba = be_soc->ppe2tcl_ring.base_paddr_aligned;
	reg_info.reo2ppe_ba = be_soc->reo2ppe_ring.base_paddr_aligned;
	reg_info.ppe2tcl_num_desc = be_soc->ppe2tcl_ring.num_entries;
	reg_info.reo2ppe_num_desc = be_soc->reo2ppe_ring.num_entries;
	if (ppe_ds_wlan_inst_register(be_soc->ppeds_handle, &reg_info) != true) {
		dp_err("%p: ppeds registeration failed", be_soc);
		return QDF_STATUS_SUCCESS;
	}

	idx->ppe2tcl_start_idx = reg_info.ppe2tcl_start_idx;
	idx->reo2ppe_start_idx = reg_info.reo2ppe_start_idx;
	be_soc->ppeds_int_mode_enabled = reg_info.ppe_ds_int_mode_enabled;

	dp_info("ppe2tcl_start_idx %d reo2ppe_start_idx %d\n",
		idx->ppe2tcl_start_idx, idx->reo2ppe_start_idx);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ppeds_detach_soc_be() - De-attach the PPE-DS instance
 * @be_soc: BE SoC
 *
 * Return: void
 */
void dp_ppeds_detach_soc_be(struct dp_soc_be *be_soc)
{
	struct dp_soc *soc = DP_SOC_BE_GET_SOC(be_soc);

	if (!be_soc->ppeds_handle)
		return;

	dp_ppeds_del_napi_ctxt(be_soc);
	dp_ppeds_tx_desc_pool_free(soc);
	dp_hw_cookie_conversion_detach(be_soc, &be_soc->ppeds_tx_cc_ctx);
	ppe_ds_wlan_inst_free(be_soc->ppeds_handle);
	be_soc->ppeds_handle = NULL;

	dp_ppeds_deinit_ppe_vp_tbl_be(be_soc);
}

/**
 * dp_ppeds_handle_tx_comp() - PPE DS handle irq
 * @irq: irq number
 * @ctxt: irq context
 *
 * PPE DS handle irq function
 *
 * Return: irq status
 */
irqreturn_t dp_ppeds_handle_tx_comp(int irq, void *ctxt)
{
	struct dp_soc_be *be_soc =
			dp_get_be_soc_from_dp_soc((struct dp_soc *)ctxt);
	struct napi_struct *napi = &be_soc->ppeds_napi_ctxt.napi;

	dp_ppeds_disable_irq(&be_soc->soc, &be_soc->ppeds_wbm_release_ring);

	napi_schedule(napi);

	return IRQ_HANDLED;
}

QDF_STATUS dp_ppeds_init_soc_be(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);

	if (!wlan_cfg_get_dp_soc_is_ppeds_enabled(soc->wlan_cfg_ctx))
		return QDF_STATUS_SUCCESS;

	return dp_hw_cookie_conversion_init(be_soc, &be_soc->ppeds_tx_cc_ctx);
}

QDF_STATUS dp_ppeds_deinit_soc_be(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);

	if (!wlan_cfg_get_dp_soc_is_ppeds_enabled(soc->wlan_cfg_ctx))
		return QDF_STATUS_SUCCESS;

	return dp_hw_cookie_conversion_deinit(be_soc, &be_soc->ppeds_tx_cc_ctx);
}

/**
 * dp_ppeds_attach_soc_be() - Attach the PPE-DS instance
 * @be_soc: BE SoC
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ppeds_attach_soc_be(struct dp_soc_be *be_soc)
{
	struct dp_soc *soc = DP_SOC_BE_GET_SOC(be_soc);
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	struct dp_soc_be **besocptr;
	uint32_t num_elem;

	num_elem = wlan_cfg_get_dp_soc_ppeds_num_tx_desc(soc->wlan_cfg_ctx);

	qdf_status =
		dp_hw_cookie_conversion_attach(be_soc, &be_soc->ppeds_tx_cc_ctx,
					       num_elem, DP_TX_PPEDS_DESC_TYPE,
					       DP_TX_PPEDS_POOL_ID);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status))
		goto fail1;

	if (dp_ppeds_tx_desc_pool_alloc(soc, num_elem) != QDF_STATUS_SUCCESS) {
		dp_err("%p: Failed to allocate ppeds tx desc pool", be_soc);
		goto fail2;
	}

	if (dp_ppeds_init_ppe_vp_tbl_be(be_soc) != QDF_STATUS_SUCCESS) {
		dp_err("%p: Failed to init ppe vp tbl", be_soc);
		goto fail3;
	}

	be_soc->ppeds_handle =
			ppe_ds_wlan_inst_alloc(&ppeds_ops,
						   sizeof(struct dp_soc_be *));
	if (!be_soc->ppeds_handle) {
		dp_err("%p: Failed to allocate ppeds soc instance", be_soc);
		goto fail4;
	}

	dp_ppeds_add_napi_ctxt(be_soc);

	dp_info("Allocated PPEDS handle\n");

	besocptr = (struct dp_soc_be **)ppe_ds_wlan_priv(be_soc->ppeds_handle);
	*besocptr = be_soc;

	 return QDF_STATUS_SUCCESS;

fail4:
	dp_ppeds_deinit_ppe_vp_tbl_be(be_soc);
fail3:
	dp_ppeds_tx_desc_pool_free(soc);
fail2:
	dp_hw_cookie_conversion_detach(be_soc, &be_soc->ppeds_tx_cc_ctx);
fail1:
	dp_err("Could not allocate PPEDS handle\n");
	return QDF_STATUS_E_FAILURE;

}
