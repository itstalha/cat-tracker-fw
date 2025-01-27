/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#ifndef ZEPHYR_INCLUDE_CLOUD_H_
#define ZEPHYR_INCLUDE_CLOUD_H_

/**
 * @brief Cloud API
 * @defgroup cloud_api Cloud API
 * @{
 */

#include <zephyr.h>

/**@brief Cloud backend states. */
enum cloud_state {
	CLOUD_STATE_DISCONNECTED,
	CLOUD_STATE_DISCONNECTING,
	CLOUD_STATE_CONNECTED,
	CLOUD_STATE_CONNECTING,
	CLOUD_STATE_BUSY,
	CLOUD_STATE_ERROR,
	CLOUD_STATE_COUNT
};

/**@brief Cloud events that can be notified asynchronously by the backend. */
enum cloud_event_type {
	CLOUD_EVT_CONNECTED,
	CLOUD_EVT_DISCONNECTED,
	CLOUD_EVT_READY,
	CLOUD_EVT_ERROR,
	CLOUD_EVT_DATA_SENT,
	CLOUD_EVT_DATA_RECEIVED,
	CLOUD_EVT_PAIR_REQUEST,
	CLOUD_EVT_PAIR_DONE,
	CLOUD_EVT_COUNT
};

/**@brief Quality of Service for message sent by a cloud backend. */
enum cloud_qos {
	CLOUD_QOS_AT_MOST_ONCE,
	CLOUD_QOS_AT_LEAST_ONCE,
	CLOUD_QOS_EXACTLY_ONCE,
	CLOUD_QOS_COUNT
};

/**@brief Cloud endpoint type. */
enum cloud_endpoint {
	CLOUD_EP_TOPIC_MSG,
	CLOUD_EP_TOPIC_STATE,
	CLOUD_EP_TOPIC_CONFIG,
	CLOUD_EP_TOPIC_PAIR,
	CLOUD_EP_TOPIC_BATCH,
	CLOUD_EP_URI,
	CLOUD_EP_COUNT,
};

/**@brief Cloud pairing type. */
enum cloud_pair_type {
	CLOUD_PAIR_SEQUENCE,
	CLOUD_PAIR_PIN,
};

/**@brief Cloud action type */
enum cloud_action_type {
	CLOUD_PAIR,
	CLOUD_REPORT,
};

/**@brief Cloud pairing data. */
struct cloud_pair_data {
	enum cloud_pair_type type;
	u8_t *buf;
	size_t len;
};

/** @brief Forward declaration of cloud backend type. */
struct cloud_backend;

/**@brief Cloud message type. */
struct cloud_msg {
	char *buf;
	size_t len;
	enum cloud_qos qos;
	struct {
		enum cloud_endpoint type;
		char *str;
		size_t len;
	} endpoint;
};

/**@brief Cloud event type. */
struct cloud_event {
	enum cloud_event_type type;
	union {
		struct cloud_msg msg;
		int err;
		struct cloud_pair_data pair_info;
	} data;
};

/**
 * @brief Cloud event handler function type.
 *
 * @param backend   Pointer to cloud backend.
 * @param evt       Pointer to cloud event.
 * @param user_data Pointer to user defined data that will be passed on as
 *                     argument to cloud event handler.
 */
typedef void (*cloud_evt_handler_t)(const struct cloud_backend *const backend,
				    const struct cloud_event *const evt,
				    void *user_data);

/**
 * @brief Cloud backend API.
 *
 * ping() and user_data_set() can be omitted, the other functions are mandatory.
 */
struct cloud_api {
	int (*init)(const struct cloud_backend *const backend,
		    cloud_evt_handler_t handler);
	int (*init_config)(const struct cloud_backend *const backend);
	int (*uninit)(const struct cloud_backend *const backend);
	int (*connect)(const struct cloud_backend *const backend);
	int (*disconnect)(const struct cloud_backend *const backend);
	int (*send)(const struct cloud_backend *const backend,
		    const struct cloud_msg *const msg);
	int (*ping)(const struct cloud_backend *const backend);
	int (*input)(const struct cloud_backend *const backend);
	int (*user_data_set)(const struct cloud_backend *const backend,
			     void *user_data);
	int (*report_and_update)(const struct cloud_backend *const backend,
				 const enum cloud_action_type action);
};

/**@brief Structure for cloud backend configuration. */
struct cloud_backend_config {
	char *name;
	cloud_evt_handler_t handler;
	int socket;
	void *user_data;
};

/**@brief Structure for cloud backend. */
struct cloud_backend {
	const struct cloud_api *const api;
	struct cloud_backend_config *const config;
};

/**@brief Initialize a cloud backend. Performs all necessary
 *	  configuration of the backend required to connect its online
 *	  counterpart.
 *
 * @param backend Pointer to cloud backend structure.
 * @param handler Handler to receive events from the backend.
 */
static inline int cloud_init(struct cloud_backend *const backend,
			     cloud_evt_handler_t handler)
{
	if (backend == NULL || backend->api == NULL ||
	    backend->api->init == NULL) {
		return -ENOTSUP;
	}

	if (handler == NULL) {
		return -EINVAL;
	}

	return backend->api->init(backend, handler);
}

static inline int cloud_init_config(struct cloud_backend *const backend)
{
	if (backend == NULL || backend->api == NULL ||
	    backend->api->init_config == NULL) {
		return -ENOTSUP;
	}

	return backend->api->init_config(backend);
}

/**@brief Uninitialize a cloud backend. Gracefully disconnects
 *        remote endpoint and releases memory.
 *
 * @param backend Pointer to cloud backend structure.
 */
static inline int cloud_uninit(const struct cloud_backend *const backend)
{
	if (backend == NULL || backend->api == NULL ||
	    backend->api->uninit == NULL) {
		return -ENOTSUP;
	}

	return backend->api->uninit(backend);
}

/**@brief Request connection to a cloud backend.
 *
 * @details The backend is required to expose the socket in use when this
 *	    function returns. The socket should then be available through
 *	    backend->config->socket and the application may start listening
 *	    for events on it.
 *
 * @param backend Pointer to a cloud backend structure.
 *
 * @return 0 or a negative error code indicating reason of failure.
 */
static inline int cloud_connect(const struct cloud_backend *const backend)
{
	if (backend == NULL || backend->api == NULL ||
	    backend->api->connect == NULL) {
		return -ENOTSUP;
	}

	return backend->api->connect(backend);
}

/**@brief Disconnect from a cloud backend.
 *
 * @param backend Pointer to a cloud backend structure.
 *
 * @return 0 or a negative error code indicating reason of failure.
 */
static inline int cloud_disconnect(const struct cloud_backend *const backend)
{
	if (backend == NULL || backend->api == NULL ||
	    backend->api->disconnect == NULL) {
		return -ENOTSUP;
	}

	return backend->api->disconnect(backend);
}

/**@brief Send data to a cloud.
 *
 * @param backend Pointer to a cloud backend structure.
 * @param msg     Pointer to cloud message structure.
 *
 * @return 0 or a negative error code indicating reason of failure.
 */
static inline int cloud_send(const struct cloud_backend *const backend,
			     struct cloud_msg *msg)
{
	if (backend == NULL || backend->api == NULL ||
	    backend->api->send == NULL) {
		return -ENOTSUP;
	}

	return backend->api->send(backend, msg);
}

/**
 * @brief Optional API to ping the cloud's remote endpoint periodically.
 *
 * @param backend Pointer to cloud backend.
 *
 * @return 0 or a negative error code indicating reason of failure.
 */
static inline int cloud_ping(const struct cloud_backend *const backend)
{
	if (backend == NULL || backend->api == NULL) {
		return -ENOTSUP;
	}

	/* Ping will only be sent if the backend has implemented it. */
	if (backend->api->ping != NULL) {
		return backend->api->ping(backend);
	}

	return 0;
}

/**
 * @brief Process incoming data to backend.
 *
 * @note This is a non-blocking call.
 *
 * @param backend Pointer to cloud backend.
 *
 * @return 0 or a negative error code indicating reason of failure.
 */
static inline int cloud_input(const struct cloud_backend *const backend)
{
	if (backend == NULL || backend->api == NULL ||
	    backend->api->input == NULL) {
		return -ENOTSUP;
	}

	return backend->api->input(backend);
}

static inline int cloud_report_and_update(struct cloud_backend *const backend,
					  const enum cloud_action_type action)
{
	if (backend == NULL || backend->api == NULL ||
	    backend->api->report_and_update == NULL) {
		return -ENOTSUP;
	}

	return backend->api->report_and_update(backend, action);
}

/**@brief Set the user-defined data that is passed as an argument to cloud event
 *        handler.
 *
 * @param backend   Pointer to cloud backend structure.
 * @param user_data Pointer to user defined data that will be passed on as
 *                     argument to cloud event handler.
 */
static inline int cloud_user_data_set(struct cloud_backend *const backend,
				      void *user_data)
{
	if (backend == NULL || backend->api == NULL ||
	    backend->api->user_data_set == NULL) {
		return -ENOTSUP;
	}

	if (user_data == NULL) {
		return -EINVAL;
	}

	return backend->api->user_data_set(backend, user_data);
}

/**
 * @brief Get binding (pointer) to cloud backend if a registered backend
 *	      matches the provided name.
 *
 * @param name String with the name of the cloud backend.
 *
 * @return Pointer to the cloud backend structure.
 **/
struct cloud_backend *cloud_get_binding(const char *name);

#define CLOUD_BACKEND_DEFINE(_name, _api)				     \
									     \
	static struct cloud_backend_config UTIL_CAT(_name, _config) =	     \
	{								     \
		.name = STRINGIFY(_name)				     \
	};								     \
									     \
	static const struct cloud_backend _name				     \
	__attribute__ ((section(".cloud_backends"))) __attribute__((used)) = \
	{								     \
		.api = &_api,						     \
		.config = &UTIL_CAT(_name, _config)			     \
	};

/**
 * @}
 */

#endif /* ZEPHYR_INCLUDE_CLOUD_H_ */
