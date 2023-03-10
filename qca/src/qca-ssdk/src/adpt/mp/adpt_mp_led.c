/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
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

#include "sw.h"
#include "adpt.h"
#include "hsl.h"
#include "hsl_dev.h"
#include "hsl_phy.h"
#include "hsl_port_prop.h"
#include "adpt_mp.h"

static sw_error_t
adpt_mp_led_ctrl_pattern_set(a_uint32_t dev_id, led_pattern_group_t group,
	led_pattern_id_t led_pattern_id, led_ctrl_pattern_t * pattern)
{
	return hsl_port_phy_led_ctrl_pattern_set(dev_id, group, led_pattern_id, pattern);
}

static sw_error_t
adpt_mp_led_ctrl_pattern_get(a_uint32_t dev_id, led_pattern_group_t group,
	led_pattern_id_t led_pattern_id, led_ctrl_pattern_t * pattern)
{
	return hsl_port_phy_led_ctrl_pattern_get(dev_id, group, led_pattern_id, pattern);
}

static sw_error_t
adpt_mp_led_ctrl_source_set(a_uint32_t dev_id, a_uint32_t source_id,
	led_ctrl_pattern_t *pattern)
{
	return hsl_port_phy_led_ctrl_source_set(dev_id, source_id, pattern);
}

sw_error_t adpt_mp_led_init(a_uint32_t dev_id)
{
	adpt_api_t *p_adpt_api = NULL;

	p_adpt_api = adpt_api_ptr_get(dev_id);

	SW_RTN_ON_NULL (p_adpt_api);

	p_adpt_api->adpt_led_ctrl_pattern_set = adpt_mp_led_ctrl_pattern_set;
	p_adpt_api->adpt_led_ctrl_pattern_get = adpt_mp_led_ctrl_pattern_get;
	p_adpt_api->adpt_led_ctrl_source_set = adpt_mp_led_ctrl_source_set;

	return SW_OK;
}
/**
 * @}
 */
