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

#ifndef __EDMA_PPEDS_PRIV__
#define __EDMA_PPEDS_PRIV__

#include "nss_dp_ppeds.h"

#define EDMA_PPEDS_MAX_NODES	3	/* Maximum number of supported PPE-DS nodes */
#define EDMA_PPEDS_RX_WEIGHT	1	/* PPE-DS Rx processing budget */
#define EDMA_PPEDS_RXFILL_WEIGHT	128	/* PPE-DS Rxfill processing budget */

/*
 * EDMA PPE-DS node entry index definitions
 */
enum {
	EDMA_PPEDS_ENTRY_RXFILL_IDX,
	EDMA_PPEDS_ENTRY_TXCMPL_IDX,
	EDMA_PPEDS_ENTRY_RX_IDX,
	EDMA_PPEDS_ENTRY_TX_IDX,
	EDMA_PPEDS_ENTRY_QID_START_IDX,
	EDMA_PPEDS_ENTRY_QID_COUNT_IDX,
	EDMA_PPEDS_NUM_ENTRY
};

/*
 * edma_ppeds_node_state_t
 *	PPE-DS node state
 */
typedef enum {
	EDMA_PPEDS_NODE_STATE_NOT_AVAIL,
	EDMA_PPEDS_NODE_STATE_AVAIL,
	EDMA_PPEDS_NODE_STATE_ALLOC,
	EDMA_PPEDS_NODE_STATE_REG_IN_PROG,
	EDMA_PPEDS_NODE_STATE_REG_DONE,
	EDMA_PPEDS_NODE_STATE_START_IN_PROG,
	EDMA_PPEDS_NODE_STATE_START_DONE,
	EDMA_PPEDS_NODE_STATE_STOP_IN_PROG,
	EDMA_PPEDS_NODE_STATE_STOP_DONE,
	EDMA_PPEDS_NODE_STATE_FREE_IN_PROG,
} edma_ppeds_node_state_t;

/*
 * PPE-DS Interrupt's entry index in the node configuration
 */
enum {
	EDMA_PPEDS_TXCOMP_IRQ_IDX,
	EDMA_PPEDS_RXDESC_IRQ_IDX,
	EDMA_PPEDS_RXFILL_IRQ_IDX,
	EDMA_PPEDS_IRQ_NUM
};

/*
 * PPE-DS EDMA node descriptor
 */
struct edma_ppeds {
	const struct nss_dp_ppeds_cb *ops;	/* PPE-DS EDMA callback pointer */
	struct edma_rxfill_ring rxfill_ring;	/* PPE-DS EDMA Rxfill ring */
	struct edma_txcmpl_ring txcmpl_ring;	/* PPE-DS EDMA Tx complete ring */
	struct edma_rxdesc_ring rx_ring;	/* PPE-DS EDMA Rx ring */
	struct edma_txdesc_ring tx_ring;	/* PPE-DS EDMA Tx ring */
	struct net_device napi_ndev;		/* Dummy net_device for NAPI */
	uint32_t ppe_qid;			/* PPE-DS node start queue id */
	uint32_t ppe_num_queues;		/* PPE-DS node queue count */
	uint32_t txcmpl_intr;			/* PPE-DS EDMA Tx complete IRQ */
	uint32_t rxfill_intr;			/* PPE-DS EDMA Rxfill IRQ */
	uint32_t rxdesc_intr;			/* PPE-DS EDMA Rx IRQ */
	uint8_t db_idx;				/* PPE-DS node index */
	nss_dp_ppeds_handle_t ppeds_handle;	/* PPE-DS handle */
};

/*
 * edma_ppeds_node_cfg
 *	EDMA PPE-DS node configuration structure
 */
struct edma_ppeds_node_cfg {
	uint32_t node_map[EDMA_PPEDS_NUM_ENTRY];
					/* PPE-DS node configuration mapping */
	uint32_t irq_map[EDMA_PPEDS_IRQ_NUM];
					/* PPE-DS node IRQ mapping */
	edma_ppeds_node_state_t node_state;
					/* PPE-DS node state information */
	struct edma_ppeds *ppeds_db;	/* PPE-DS EDMA node pointer */
};

/*
 * edma_ppeds_drv
 *	PPE-DS EDMA configuration descriptor
 */
struct edma_ppeds_drv {
	uint32_t num_nodes;			/* Number of PPE-DS nodes */
	struct edma_ppeds_node_cfg ppeds_node_cfg[EDMA_PPEDS_MAX_NODES];
						/* PPE-DS node configuration information */
	rwlock_t lock;			/* Lock for accessing the PPE-DS node information */
};

int edma_ppeds_init(struct edma_ppeds_drv* drv);
void edma_ppeds_deinit(struct edma_ppeds_drv *drv);
#endif	/* __EDMA_PPEDS_PRIV__ */
