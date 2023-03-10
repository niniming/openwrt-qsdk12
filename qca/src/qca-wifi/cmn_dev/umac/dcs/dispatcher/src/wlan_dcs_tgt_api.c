/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
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

/**
 * DOC:wlan_dcs_tgt_api.c
 *
 * This file provide API definitions to update dcs from southbound interface
 */

#include "wlan_dcs_tgt_api.h"
#include "../../core/src/wlan_dcs.h"

QDF_STATUS tgt_dcs_process_event(struct wlan_objmgr_psoc *psoc,
				 struct wlan_host_dcs_event *event)
{
	return wlan_dcs_process(psoc, event);
}

#ifdef CONFIG_AFC_SUPPORT
QDF_STATUS tgt_afc_trigger_dcs(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_host_dcs_event *event;
	QDF_STATUS status;

	psoc = wlan_pdev_get_psoc(pdev);

	event = qdf_mem_malloc(sizeof(*event));
	if (!event)
		return QDF_STATUS_E_NOMEM;

	event->dcs_param.interference_type = WLAN_HOST_DCS_AFC;
	event->dcs_param.pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	status = wlan_dcs_process(psoc, event);

	qdf_mem_free(event);
	return status;
}
#endif
