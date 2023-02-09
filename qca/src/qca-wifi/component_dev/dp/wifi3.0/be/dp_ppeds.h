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

#ifndef _DP_PPEDS_H_
#define _DP_PPEDS_H_

struct dp_ppe_ds_idxs {
	uint32_t ppe2tcl_start_idx;
	uint32_t reo2ppe_start_idx;
};

/**
 * dp_ppeds_init_ppe_vp_tbl_be - Attach ppeds soc instance
 * @be_soc: BE SoC
 *
 * Attach ppeds soc instance
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ppeds_attach_soc_be(struct dp_soc_be *be_soc);

/**
 * dp_ppeds_detach_soc_be - Detach ppeds soc instance
 * @be_soc: BE SoC
 *
 * Detach ppeds soc instance
 *
 * Return: void
 */
void dp_ppeds_detach_soc_be(struct dp_soc_be *be_soc);

/**
 * dp_ppeds_init_soc_be - Initialize ppeds soc instance
 * @soc: SoC
 *
 * Initialize ppeds soc instance
 *
 * Return: status
 */
QDF_STATUS dp_ppeds_init_soc_be(struct dp_soc *soc);

/**
 * dp_ppeds_deinit_soc_be - Deinitialize ppeds soc instance
 * @soc: SoC
 *
 * Deinitialize ppeds soc instance
 *
 * Return: status
 */
QDF_STATUS dp_ppeds_deinit_soc_be(struct dp_soc *soc);

/**
 * dp_ppeds_register_soc_be - Registers ppeds soc instance
 * @be_soc: BE SoC
 *
 * Registers ppeds soc instance
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ppeds_register_soc_be(struct dp_soc_be *be_soc, struct dp_ppe_ds_idxs *idx);

/**
 * dp_ppeds_start_soc_be - Starts ppeds txrx
 * @soc: DP SoC
 *
 * Starts ppeds txrx for the given soc instance
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ppeds_start_soc_be(struct dp_soc *soc);

/**
 * dp_ppeds_stop_soc_be - Stops ppeds txrx
 * @soc: DP SoC
 *
 * Stops ppeds txrx for the given soc instance
 *
 * Return: void
 */
void dp_ppeds_stop_soc_be(struct dp_soc *soc);

/**
 * dp_ppeds_detach_vdev_be() - Deattach the VP port from vdev
 * @soc_hdl: CDP SoC Tx/Rx handle
 * @vdev_id: vdev id
 *
 * Detach the VP port from BE VAP
 *
 * Return: void
 */
void dp_ppeds_detach_vdev_be(struct cdp_soc_t *soc, uint8_t vdev_id);

/**
 * dp_ppeds_attach_vdev_be - PPE DS table entry alloc
 * @soc: CDP SoC Tx/Rx handle
 * @vdev_id: vdev_id
 * @vp_arg: PPE VP opaque
 * @ppe_vp_num: PPE VP number
 *
 * Allocate a DS VP port and attach to BE VAP
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ppeds_attach_vdev_be(struct cdp_soc_t *soc, uint8_t vdev_id,
				   void *vp_arg, int32_t *ppe_vp_num);

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
				  uint8_t *pri2tid);

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
				     uint8_t pri, uint8_t tid);

/*
 * dp_ppeds_dump_ppe_vp_tbl_be() - Dump the PPE VP entries
 * @soc_hdl: CDP SoC Tx/Rx handle
 *
 * Dump the PPE VP entries of a SoC
 *
 * Return: void
 */
void dp_ppeds_dump_ppe_vp_tbl_be(struct cdp_soc_t *soc_hdl);

/*
 * dp_ppeds_vdev_enable_pri2tid_be() - Enable PPE PRI2TID conversion for a vap
 * @soc_hdl: CDP SoC Tx/Rx handle
 * @vdev_id : vdev id
 * @val: Boolean to enable/disable PRI2TID mapping
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ppeds_vdev_enable_pri2tid_be(struct cdp_soc_t *soc_hdl,
				       uint8_t vdev_id,
				       bool val);

/*
 * dp_ppeds_handle_tx_comp: Handle tx completions interrupt
 * @irq: IRQ number
 * @ctxr: IRQ handler context
 *
 * Return: IRQ handle status
 */
irqreturn_t dp_ppeds_handle_tx_comp(int irq, void *ctxr);

/*
 * dp_ppe_ds_ppe2tcl_irq_handler: Handle ppe2tcl ring interrupt
 * @irq: IRQ number
 * @ctxr: IRQ handler context
 *
 * Return: IRQ handle status
 */
irqreturn_t dp_ppe_ds_ppe2tcl_irq_handler(int irq, void *ctxr);

/*
 * dp_ppe_ds_reo2ppe_irq_handler: Handle reo2ppe ring interrupt
 * @irq: IRQ number
 * @ctxr: IRQ handler context
 *
 * Return: IRQ handle status
 */
irqreturn_t dp_ppe_ds_reo2ppe_irq_handler(int irq, void *ctxr);

/*
 * dp_get_ppe_ds_ctxt: Get context from ppe ds driver
 * @soc: CDP SoC Tx/Rx handle
 *
 * Return: ppeds handle
 */
void *dp_get_ppe_ds_ctxt(struct dp_soc *soc);

/**
 * dp_tx_ppeds_cfg_astidx_cache_mapping - Set ppe index mapping table value
 * @soc: DP SoC context
 * @vdev: DP vdev
 * @peer_map: map if true, unmap if false
 *
 * Return: void
 */
void dp_tx_ppeds_cfg_astidx_cache_mapping(struct dp_soc *soc,
					  struct dp_vdev *vdev, bool peer_map);
#endif /* _DP_PPEDS_H_ */
