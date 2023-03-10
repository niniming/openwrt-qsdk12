/*
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

/**
 * @defgroup
 * @{
 */
#ifndef _CPPE_PORTCTRL_REG_H_
#define _CPPE_PORTCTRL_REG_H_

/*[table] CPPE_MRU_MTU_CTRL_TBL*/
#define CPPE_MRU_MTU_CTRL_TBL
#define CPPE_MRU_MTU_CTRL_TBL_ADDRESS 0x3000
#define CPPE_MRU_MTU_CTRL_TBL_NUM     256
#define CPPE_MRU_MTU_CTRL_TBL_INC     0x10
#define CPPE_MRU_MTU_CTRL_TBL_TYPE    REG_TYPE_RW
#define CPPE_MRU_MTU_CTRL_TBL_DEFAULT 0x0
	/*[field] MRU*/
	#define CPPE_MRU_MTU_CTRL_TBL_MRU
	#define CPPE_MRU_MTU_CTRL_TBL_MRU_OFFSET  0
	#define CPPE_MRU_MTU_CTRL_TBL_MRU_LEN     14
	#define CPPE_MRU_MTU_CTRL_TBL_MRU_DEFAULT 0x0
	/*[field] MRU_CMD*/
	#define CPPE_MRU_MTU_CTRL_TBL_MRU_CMD
	#define CPPE_MRU_MTU_CTRL_TBL_MRU_CMD_OFFSET  14
	#define CPPE_MRU_MTU_CTRL_TBL_MRU_CMD_LEN     2
	#define CPPE_MRU_MTU_CTRL_TBL_MRU_CMD_DEFAULT 0x0
	/*[field] MTU*/
	#define CPPE_MRU_MTU_CTRL_TBL_MTU
	#define CPPE_MRU_MTU_CTRL_TBL_MTU_OFFSET  16
	#define CPPE_MRU_MTU_CTRL_TBL_MTU_LEN     14
	#define CPPE_MRU_MTU_CTRL_TBL_MTU_DEFAULT 0x0
	/*[field] MTU_CMD*/
	#define CPPE_MRU_MTU_CTRL_TBL_MTU_CMD
	#define CPPE_MRU_MTU_CTRL_TBL_MTU_CMD_OFFSET  30
	#define CPPE_MRU_MTU_CTRL_TBL_MTU_CMD_LEN     2
	#define CPPE_MRU_MTU_CTRL_TBL_MTU_CMD_DEFAULT 0x0
	/*[field] RX_CNT_EN*/
	#define CPPE_MRU_MTU_CTRL_TBL_RX_CNT_EN
	#define CPPE_MRU_MTU_CTRL_TBL_RX_CNT_EN_OFFSET  32
	#define CPPE_MRU_MTU_CTRL_TBL_RX_CNT_EN_LEN     1
	#define CPPE_MRU_MTU_CTRL_TBL_RX_CNT_EN_DEFAULT 0x0
	/*[field] TX_CNT_EN*/
	#define CPPE_MRU_MTU_CTRL_TBL_TX_CNT_EN
	#define CPPE_MRU_MTU_CTRL_TBL_TX_CNT_EN_OFFSET  33
	#define CPPE_MRU_MTU_CTRL_TBL_TX_CNT_EN_LEN     1
	#define CPPE_MRU_MTU_CTRL_TBL_TX_CNT_EN_DEFAULT 0x0
	/*[field] SRC_PROFILE*/
	#define CPPE_MRU_MTU_CTRL_TBL_SRC_PROFILE
	#define CPPE_MRU_MTU_CTRL_TBL_SRC_PROFILE_OFFSET  34
	#define CPPE_MRU_MTU_CTRL_TBL_SRC_PROFILE_LEN     2
	#define CPPE_MRU_MTU_CTRL_TBL_SRC_PROFILE_DEFAULT 0x0
	/*[field] PCP_QOS_GROUP_ID*/
	#define CPPE_MRU_MTU_CTRL_TBL_PCP_QOS_GROUP_ID
	#define CPPE_MRU_MTU_CTRL_TBL_PCP_QOS_GROUP_ID_OFFSET  36
	#define CPPE_MRU_MTU_CTRL_TBL_PCP_QOS_GROUP_ID_LEN     1
	#define CPPE_MRU_MTU_CTRL_TBL_PCP_QOS_GROUP_ID_DEFAULT 0x0
	/*[field] DSCP_QOS_GROUP_ID*/
	#define CPPE_MRU_MTU_CTRL_TBL_DSCP_QOS_GROUP_ID
	#define CPPE_MRU_MTU_CTRL_TBL_DSCP_QOS_GROUP_ID_OFFSET  37
	#define CPPE_MRU_MTU_CTRL_TBL_DSCP_QOS_GROUP_ID_LEN     1
	#define CPPE_MRU_MTU_CTRL_TBL_DSCP_QOS_GROUP_ID_DEFAULT 0x0
	/*[field] PCP_RES_PREC_FORCE*/
	#define CPPE_MRU_MTU_CTRL_TBL_PCP_RES_PREC_FORCE
	#define CPPE_MRU_MTU_CTRL_TBL_PCP_RES_PREC_FORCE_OFFSET  38
	#define CPPE_MRU_MTU_CTRL_TBL_PCP_RES_PREC_FORCE_LEN     1
	#define CPPE_MRU_MTU_CTRL_TBL_PCP_RES_PREC_FORCE_DEFAULT 0x0
	/*[field] DSCP_RES_PREC_FORCE*/
	#define CPPE_MRU_MTU_CTRL_TBL_DSCP_RES_PREC_FORCE
	#define CPPE_MRU_MTU_CTRL_TBL_DSCP_RES_PREC_FORCE_OFFSET  39
	#define CPPE_MRU_MTU_CTRL_TBL_DSCP_RES_PREC_FORCE_LEN     1
	#define CPPE_MRU_MTU_CTRL_TBL_DSCP_RES_PREC_FORCE_DEFAULT 0x0
	/*[field] PREHEADER_RES_PREC*/
	#define CPPE_MRU_MTU_CTRL_TBL_PREHEADER_RES_PREC
	#define CPPE_MRU_MTU_CTRL_TBL_PREHEADER_RES_PREC_OFFSET  40
	#define CPPE_MRU_MTU_CTRL_TBL_PREHEADER_RES_PREC_LEN     3
	#define CPPE_MRU_MTU_CTRL_TBL_PREHEADER_RES_PREC_DEFAULT 0x0
	/*[field] PCP_RES_PREC*/
	#define CPPE_MRU_MTU_CTRL_TBL_PCP_RES_PREC
	#define CPPE_MRU_MTU_CTRL_TBL_PCP_RES_PREC_OFFSET  43
	#define CPPE_MRU_MTU_CTRL_TBL_PCP_RES_PREC_LEN     3
	#define CPPE_MRU_MTU_CTRL_TBL_PCP_RES_PREC_DEFAULT 0x0
	/*[field] DSCP_RES_PREC*/
	#define CPPE_MRU_MTU_CTRL_TBL_DSCP_RES_PREC
	#define CPPE_MRU_MTU_CTRL_TBL_DSCP_RES_PREC_OFFSET  46
	#define CPPE_MRU_MTU_CTRL_TBL_DSCP_RES_PREC_LEN     3
	#define CPPE_MRU_MTU_CTRL_TBL_DSCP_RES_PREC_DEFAULT 0x0
	/*[field] FLOW_RES_PREC*/
	#define CPPE_MRU_MTU_CTRL_TBL_FLOW_RES_PREC
	#define CPPE_MRU_MTU_CTRL_TBL_FLOW_RES_PREC_OFFSET  49
	#define CPPE_MRU_MTU_CTRL_TBL_FLOW_RES_PREC_LEN     3
	#define CPPE_MRU_MTU_CTRL_TBL_FLOW_RES_PREC_DEFAULT 0x0
	/*[field] PRE_ACL_RES_PREC*/
	#define CPPE_MRU_MTU_CTRL_TBL_PRE_ACL_RES_PREC
	#define CPPE_MRU_MTU_CTRL_TBL_PRE_ACL_RES_PREC_OFFSET  52
	#define CPPE_MRU_MTU_CTRL_TBL_PRE_ACL_RES_PREC_LEN     3
	#define CPPE_MRU_MTU_CTRL_TBL_PRE_ACL_RES_PREC_DEFAULT 0x0
	/*[field] POST_ACL_RES_PREC*/
	#define CPPE_MRU_MTU_CTRL_TBL_POST_ACL_RES_PREC
	#define CPPE_MRU_MTU_CTRL_TBL_POST_ACL_RES_PREC_OFFSET  55
	#define CPPE_MRU_MTU_CTRL_TBL_POST_ACL_RES_PREC_LEN     3
	#define CPPE_MRU_MTU_CTRL_TBL_POST_ACL_RES_PREC_DEFAULT 0x0
	/*[field] SOURCE_FILTERING_BYPASS*/
	#define CPPE_MRU_MTU_CTRL_TBL_SOURCE_FILTERING_BYPASS
	#define CPPE_MRU_MTU_CTRL_TBL_SOURCE_FILTERING_BYPASS_OFFSET  58
	#define CPPE_MRU_MTU_CTRL_TBL_SOURCE_FILTERING_BYPASS_LEN     1
	#define CPPE_MRU_MTU_CTRL_TBL_SOURCE_FILTERING_BYPASS_DEFAULT 0x0
	/*[field] SOURCE_FILTERING_MODE*/
	#define CPPE_MRU_MTU_CTRL_TBL_SOURCE_FILTERING_MODE
	#define CPPE_MRU_MTU_CTRL_TBL_SOURCE_FILTERING_MODE_OFFSET  59
	#define CPPE_MRU_MTU_CTRL_TBL_SOURCE_FILTERING_MODE_LEN     1
	#define CPPE_MRU_MTU_CTRL_TBL_SOURCE_FILTERING_MODE_DEFAULT 0x0

struct cppe_mru_mtu_ctrl_tbl {
	a_uint32_t  mtu_cmd:2;
        a_uint32_t  mtu:14;
        a_uint32_t  mru_cmd:2;
        a_uint32_t  mru:14;
	a_uint32_t  _reserved0:4;
        a_uint32_t  source_filtering_mode:1;
        a_uint32_t  source_filtering_bypass:1;
        a_uint32_t  post_acl_res_prec:3;
        a_uint32_t  pre_acl_res_prec:3;
        a_uint32_t  flow_res_prec:3;
        a_uint32_t  dscp_res_prec:3;
        a_uint32_t  pcp_res_prec:3;
        a_uint32_t  preheader_res_prec:3;
        a_uint32_t  dscp_res_prec_force:1;
        a_uint32_t  pcp_res_prec_force:1;
        a_uint32_t  dscp_qos_group_id:1;
        a_uint32_t  pcp_qos_group_id:1;
        a_uint32_t  src_profile:2;
        a_uint32_t  tx_cnt_en:1;
        a_uint32_t  rx_cnt_en:1;
};

union cppe_mru_mtu_ctrl_tbl_u {
	a_uint32_t val[2];
	struct cppe_mru_mtu_ctrl_tbl bf;
};

struct cppe_port_phy_status_1 {
	a_uint32_t  _reserved0:8;
        a_uint32_t  port5_pcs1_phy_status:8;
        a_uint32_t  port4_pcs0_phy_status:8;
        a_uint32_t  port5_pcs0_phy_status:8;
};

union cppe_port_phy_status_1_u {
	a_uint32_t val;
	struct cppe_port_phy_status_1 bf;
};

#endif
