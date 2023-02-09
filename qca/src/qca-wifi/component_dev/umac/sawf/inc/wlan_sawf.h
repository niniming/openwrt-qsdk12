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

/**
 * DOC: wlan_sawf.h
 * This file defines data structure &  prototypes of the functions
 * needed by the SAWF framework.
 */

#ifndef _WLAN_SAWF_H_
#define _WLAN_SAWF_H_

#include <qdf_status.h>
#include "qdf_atomic.h"
#include "qdf_lock.h"

#define SAWF_SVC_CLASS_MIN 1
#define SAWF_SVC_CLASS_MAX 128
#define WLAN_MAX_SVC_CLASS_NAME 64

#define SAWF_LINE_FORMAT "================================================"

#define SAWF_DEF_PARAM_VAL 0xFFFFFFFF
/*
 * Min throughput limit 0 - 10gbps
 * Granularity: 1Kbps
 */
#define SAWF_MIN_MIN_THROUGHPUT 0
#define SAWF_MAX_MIN_THROUGHPUT (10 * 1204 * 1024)

/*
 * Max throughput limit 0 - 10gbps.
 * Granularity: 1Kbps
 */
#define SAWF_MIN_MAX_THROUGHPUT 0
#define SAWF_MAX_MAX_THROUGHPUT (10 * 1204 * 1024)

/*
 * Service interval limit 0 - 10secs.
 * Granularity: 100µs
 */
#define SAWF_MIN_SVC_INTERVAL 0
#define SAWF_MAX_SVC_INTERVAL (10 * 100 * 100)

/*
 * Burst size 0 - 16Mbytes.
 * Granularity: 1byte
 */
#define SAWF_MIN_BURST_SIZE 0
#define SAWF_MAX_BURST_SIZE (16 * 1024 * 1024)

/*
 * Delay bound limit 0 - 10secs
 * Granularity: 100µs
 */
#define SAWF_MIN_DELAY_BOUND 0
#define SAWF_MAX_DELAY_BOUND (10 * 100 * 100)

/*
 * Msdu TTL limit 0 - 10secs.
 * Granularity: 100µs
 */
#define SAWF_MIN_MSDU_TTL 0
#define SAWF_MAX_MSDU_TTL (10 * 100 * 100)

/*
 * Priority limit 0 - 127.
 */
#define SAWF_MIN_PRIORITY 0
#define SAWF_MAX_PRIORITY 127

/*
 * TID limit 0 - 7
 */
#define SAWF_MIN_TID 0
#define SAWF_MAX_TID 7

/*
 * MSDU Loss Rate limit 0 - 1000.
 * Granularity: 0.01%
 */
#define SAWF_MIN_MSDU_LOSS_RATE 0
#define SAWF_MAX_MSDU_LOSS_RATE 10000

#define DEF_SAWF_CONFIG_VALUE 0xFFFFFFFF

#define SAWF_INVALID_TID 0xFF

#define SERVICE_CLASS_TYPE_SAWF     1
#define SERVICE_CLASS_TYPE_SCS      2
#define SERVICE_CLASS_TYPE_SAWF_SCS 3
#define SAWF_INVALID_TYPE 0xFF

#define SAWF_INVALID_SERVICE_CLASS_ID                  0xFF
#define SAWF_SVC_CLASS_PARAM_DEFAULT_MIN_THRUPUT       0
#define SAWF_SVC_CLASS_PARAM_DEFAULT_MAX_THRUPUT       0xFFFFFFFF
#define SAWF_SVC_CLASS_PARAM_DEFAULT_BURST_SIZE        0
#define SAWF_SVC_CLASS_PARAM_DEFAULT_SVC_INTERVAL      0xFFFFFFFF
#define SAWF_SVC_CLASS_PARAM_DEFAULT_DELAY_BOUND       0xFFFFFFFF
#define SAWF_SVC_CLASS_PARAM_DEFAULT_TIME_TO_LIVE      0xFFFFFFFF
#define SAWF_SVC_CLASS_PARAM_DEFAULT_PRIORITY          0
#define SAWF_SVC_CLASS_PARAM_DEFAULT_TID               0xFFFFFFFF
#define SAWF_SVC_CLASS_PARAM_DEFAULT_MSDU_LOSS_RATE    0

/**
 * struct wlan_sawf_scv_class_params- Service Class Parameters
 * @svc_id: Service ID
 * @app_name: Service class name
 * @min_thruput_rate: min throughput in kilobits per second
 * @max_thruput_rate: max throughput in kilobits per second
 * @burst_size:  burst size in bytes
 * @service_interval: service interval
 * @delay_bound: delay bound in in milli seconds
 * @msdu_ttl: MSDU Time-To-Live
 * @priority: Priority
 * @tid: TID
 * @msdu_rate_loss: MSDU loss rate in parts per million
 * @configured: indicating if the serivice class is configured.
 * @ul_service_interval: Uplink service interval
 * @ul_burst_size: Uplink Burst Size
 * @type: type of service class
 * @ref_count: Number of sawf/scs procedures using the service class
 * @peer_count: Number of peers having initialized a flow in this service class
 */

struct wlan_sawf_scv_class_params {
	uint8_t svc_id;
	char app_name[WLAN_MAX_SVC_CLASS_NAME];
	uint32_t min_thruput_rate;
	uint32_t max_thruput_rate;
	uint32_t burst_size;
	uint32_t service_interval;
	uint32_t delay_bound;
	uint32_t msdu_ttl;
	uint32_t priority;
	uint32_t tid;
	uint32_t msdu_rate_loss;
	bool configured;
	uint32_t ul_service_interval;
	uint32_t ul_burst_size;
	uint8_t type;
	uint32_t ref_count;
	uint32_t peer_count;
};

/**
 * struct sawf_ctx- SAWF context
 * @lock: Lock to add or delete entry from sawf params structure
 * @svc_classes: List of all service classes
 */
struct sawf_ctx {
	qdf_spinlock_t lock;
	struct wlan_sawf_scv_class_params svc_classes[SAWF_SVC_CLASS_MAX];
};

struct psoc_peer_iter {
	uint8_t *mac_addr;
	bool set_clear;
	uint8_t svc_id;
	uint8_t param;
	uint8_t tid;
};

/* wlan_sawf_init() - Initialize SAWF subsytem
 *
 * Initialize the SAWF context
 *
 * Return: QDF_STATUS
 */

QDF_STATUS wlan_sawf_init(void);

/* wlan_sawf_deinit() - Deinitialize SAWF subsystem
 *
 * Deinnitialize the SAWF context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_sawf_deinit(void);

/* wlan_get_sawf_ctx() - Get service aware wifi context
 *
 * Get Service Aware Wifi Context
 *
 * Return: SAWF context
 */
struct sawf_ctx *wlan_get_sawf_ctx(void);

/* wlan_service_id_valid() - Validate the service ID
 *
 * Validate the service ID
 *
 * Return: true or false
 */
bool wlan_service_id_valid(uint8_t svc_id);

/* wlan_service_id_configured() - Is service ID configured
 *
 * Is the service ID configured
 *
 * Return: true or false
 */
bool wlan_service_id_configured(uint8_t svc_id);

/* wlan_service_id_tid() - TID for the service class
 *
 * TID for a service class
 *
 * Return: TID
 */
uint8_t wlan_service_id_tid(uint8_t svc_id);

/* wlan_delay_bound_configured() - Is delay-bound configured
 *
 * Is the service ID configured
 * @svc_id : service-class id
 *
 * Return: true or false
 */
bool wlan_delay_bound_configured(uint8_t svc_id);

/* wlan_get_svc_class_params() - Get service-class params
 *
 * @svc_id : service-class id
 *
 * Return: pointer to service-class params
 * NULL otherwise
 */
struct wlan_sawf_scv_class_params *
wlan_get_svc_class_params(uint8_t svc_id);

/* wlan_print_service_class() - Print service class params
 *
 * Print service class params
 *
 * Return: none
 */
void wlan_print_service_class(struct wlan_sawf_scv_class_params *params);

/* wlan_update_sawf_params() - Update service class params
 *
 * Update service class params
 *
 * Return: none
 */
void wlan_update_sawf_params(struct wlan_sawf_scv_class_params *params);

/* wlan_update_sawf_params_nolock() - Update service class params
 *
 * Update service class params
 * Caller has to take care of acquiring lock
 *
 * Return: none
 */
void wlan_update_sawf_params_nolock(struct wlan_sawf_scv_class_params *params);

/* wlan_validate_sawf_params() - Validate service class params
 *
 * Validate service class params
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_validate_sawf_params(struct wlan_sawf_scv_class_params *params);

/* wlan_sawf_get_uplink_params() - Get service class uplink parameters
 *
 * @svc_id: service class ID
 * @tid: pointer to update TID
 * @service_interval: Pointer to update uplink Service Interval
 * @burst_size: Pointer to update uplink Burst Size
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_sawf_get_uplink_params(uint8_t svc_id, uint8_t *tid,
			    uint32_t *service_interval, uint32_t *burst_size);

/* wlan_sawf_sla_process_sla_event() - Process SLA-related nl-event
 *
 * @svc_id: service class ID
 * @peer_mac: pointer to peer mac-addr
 * @peer_mld_mac: pointer to peer mld mac-addr
 * @flag: flag to denote set or clear
 *
 * Return: 0 on success
 */
int
wlan_sawf_sla_process_sla_event(uint8_t svc_id, uint8_t *peer_mac,
				uint8_t *peer_mld_mac, uint8_t flag);

/* wlan_service_id_configured_nolock() - Is service ID configured
 *
 * @svc_id : service-class id
 * Caller has to take care of acquiring lock
 *
 * Return: true or false
 */
bool wlan_service_id_configured_nolock(uint8_t svc_id);

/* wlan_service_id_tid_nolock() - TID for the service class
 *
 * @svc_id : service-class id
 * Caller has to take care of acquiring lock
 *
 * Return: TID
 */
uint8_t wlan_service_id_tid_nolock(uint8_t svc_id);

/* wlan_service_id_get_type() - get type for the service class
 *
 * @svc_id : service-class id
 *
 * Return: type
 */
uint8_t wlan_service_id_get_type(uint8_t svc_id);

/* wlan_service_id_get_type_nolock() - get type for the service class
 *
 * @svc_id : service-class id
 * Caller has to take care of acquiring lock
 *
 * Return: type
 */
uint8_t wlan_service_id_get_type_nolock(uint8_t svc_id);

/* wlan_service_id_set_type() - set type for the service class
 *
 * @svc_id : service-class id
 * @type : service-class type
 *
 * Return: void
 */
void wlan_service_id_set_type(uint8_t svc_id, uint8_t type);

/* wlan_service_id_set_type_nolock() - set type for the service class
 *
 * @svc_id : service-class id
 * @type : service-class type
 * Caller has to take care of acquiring lock
 *
 * Return: void
 */
void wlan_service_id_set_type_nolock(uint8_t svc_id, uint8_t type);

/* wlan_service_id_get_ref_count_nolock() - Get ref count
 *
 * @svc_id : service-class id
 * Caller has to take care of acquiring lock
 *
 * Return: ref_count
 */
uint32_t wlan_service_id_get_ref_count_nolock(uint8_t svc_id);

/* wlan_service_id_dec_ref_count_nolock() - Decrement ref count
 *
 * @svc_id : service-class id
 * Caller has to take care of acquiring lock
 *
 * Return: void
 */
void wlan_service_id_dec_ref_count_nolock(uint8_t svc_id);

/* wlan_service_id_inc_ref_count_nolock() - Increment ref count
 *
 * @svc_id : service-class id
 * Caller has to take care of acquiring lock
 *
 * Return: void
 */
void wlan_service_id_inc_ref_count_nolock(uint8_t svc_id);

/* wlan_service_id_get_peer_count_nolock() - Get peer count
 *
 * @svc_id : service-class id
 * Caller has to take care of acquiring lock
 *
 * Return: peer_count
 */
uint32_t wlan_service_id_get_peer_count_nolock(uint8_t svc_id);

/* wlan_service_id_dec_peer_count_nolock() - Decrement peer count
 *
 * @svc_id : service-class id
 * Caller has to take care of acquiring lock
 *
 * Return: void
 */
void wlan_service_id_dec_peer_count_nolock(uint8_t svc_id);

/* wlan_service_id_inc_peer_count_nolock() - Increment peer count
 *
 * @svc_id : service-class id
 * Caller has to take care of acquiring lock
 *
 * Return: void
 */
void wlan_service_id_inc_peer_count_nolock(uint8_t svc_id);

/* wlan_service_id_get_ref_count() - Get ref count
 *
 * @svc_id : service-class id
 *
 * Return: ref_count
 */
uint32_t wlan_service_id_get_ref_count(uint8_t svc_id);

/* wlan_service_id_dec_ref_count() - Decrement ref count
 *
 * @svc_id : service-class id
 *
 * Return: void
 */
void wlan_service_id_dec_ref_count(uint8_t svc_id);

/* wlan_service_id_inc_ref_count() - Increment ref count
 *
 * @svc_id : service-class id
 *
 * Return: void
 */
void wlan_service_id_inc_ref_count(uint8_t svc_id);

/* wlan_service_id_get_peer_count() - Get peer count
 *
 * @svc_id : service-class id
 *
 * Return: peer_count
 */
uint32_t wlan_service_id_get_peer_count(uint8_t svc_id);

/* wlan_service_id_dec_peer_count() - Decrement peer count
 *
 * @svc_id : service-class id
 *
 * Return: void
 */
void wlan_service_id_dec_peer_count(uint8_t svc_id);

/* wlan_service_id_inc_peer_count() - Increment peer count
 *
 * @svc_id : service-class id
 *
 * Return: void
 */
void wlan_service_id_inc_peer_count(uint8_t svc_id);
#endif
