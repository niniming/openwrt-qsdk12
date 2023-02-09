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

#include <linux/kernel.h>
#include <linux/module.h>

#include "ppe_ds_stats.h"
#include "ppe_ds.h"

/*
 * PPE-DS ring processing mode selection parameter.
 * The default mode PPE_DS_INTR_MODE (0) will use interrupt mode
 * and PPE_DS_POLL_MODE (1) will use polling mode.
 *
 */
unsigned int polling_for_idx_update = PPE_DS_INTR_MODE;
module_param(polling_for_idx_update,
		uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(polling_for_idx_update, "Enable/Disable PPE DS poll mode");

/*
 * PPE-DS timer frequency for polling function.
 * The final timer frequency will be in nanosecond and calculated as
 * NSEC_PER_SEC/idx_mgmt_freq
 */
unsigned int idx_mgmt_freq = 32768;
module_param(idx_mgmt_freq, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(idx_mgmt_freq, "Idx Management hrtimer freq");

/*
 * Maximum PPE2TCL producer index count to be indicated to the Waikiki
 * hardware to handle its slow Tx complete
 */
unsigned int max_move = 1024;
module_param(max_move, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(max_move, "Max movement of the Prod idx that is allowed");

unsigned int cpu_mask_2g = 0x2;
module_param(cpu_mask_2g, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(cpu_mask_2g, "CPU mask for the 2G radio VAP");

unsigned int cpu_mask_5g = 0x2;
module_param(cpu_mask_5g, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(cpu_mask_5g, "CPU mask for the 5G radio VAP");

unsigned int cpu_mask_6g = 0x1;
module_param(cpu_mask_6g, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(cpu_mask_6g, "CPU mask for the 6G radio VAP");

struct ppe_ds_node_config ppe_ds_node_cfg[PPE_DS_MAX_NODE];

/*
 * ppe_ds_module_init()
 *	Module init for PPE-DS driver
 */
static int __init ppe_ds_module_init(void)
{
	uint32_t i;

	for (i = 0; i < PPE_DS_MAX_NODE; i++) {
		rwlock_init(&ppe_ds_node_cfg[i].lock);
		ppe_ds_node_cfg[i].node_state = PPE_DS_NODE_STATE_AVAIL;
	}
	ppe_ds_node_stats_debugfs_init();
	ppe_ds_info("PPE-DS module loaded successfully %d", idx_mgmt_freq);
	return 0;
}
module_init(ppe_ds_module_init);

/*
 * ppe_ds_module_exit()
 *	Module exit for PPE-DS driver
 */
static void __exit ppe_ds_module_exit(void)
{
	uint32_t i;

	ppe_ds_node_stats_debugfs_exit();
	for (i = 0; i < PPE_DS_MAX_NODE; i++) {
		ppe_ds_node_cfg[i].node_state = PPE_DS_NODE_STATE_AVAIL;
	}
	ppe_ds_info("PPE-DS module unloaded successfully %d", idx_mgmt_freq);
}
module_exit(ppe_ds_module_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("QCA PPE DS driver");
