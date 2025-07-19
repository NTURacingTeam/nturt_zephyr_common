/**
 * @file
 * @brief Inter-thread communication support.
 *
 * @author quantumspawner
 * @version 0.0.1
 * @date 2025-01-20
 * @copyright Copyright (c) 2025 NTU Racing Team
 */

#ifndef NTURT_MSG_MSG_H_
#define NTURT_MSG_MSG_H_

// glibc includes
#include <stddef.h>

// zerphyr includes
#include <zephyr/sys/util.h>
#include <zephyr/zbus/zbus.h>

// project includes
#include "nturt/msg/aggregation.h"
#include "nturt/msg/interfaces/interfaces.h"

/**
 * @defgroup msg Message
 * @brief Inter-thread communication support.
 *
 * @{
 */

/* macro ---------------------------------------------------------------------*/
/// @brief List of all messages.
#define MSG_LIST MSG_SENSOR_LIST, MSG_TRACTIVE_LIST

#define _MSG_CHAN_NAME(msg) CONCAT(msg, _chan)

#define _MSG_ZBUS_CHAN_DEFINE(msg)                              \
  ZBUS_CHAN_DEFINE(_MSG_CHAN_NAME(msg), struct msg, NULL, NULL, \
                   ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(0))

/**
 * @brief Define one message channel for every message in @p list .
 *
 * @param[in] list List of messages to define channels.
 */
#define MSG_ZBUS_CHAN_DEFINE(list) FOR_EACH(_MSG_ZBUS_CHAN_DEFINE, (;), list)

/**
 * @brief Declare one message channel for every message in @p list .
 *
 * @param[in] list List of messages to define channels.
 */
#define MSG_ZBUS_CHAN_DECLARE(list) \
  ZBUS_CHAN_DECLARE(FOR_EACH(_MSG_CHAN_NAME, (, ), list))

/* exported variable ---------------------------------------------------------*/
MSG_ZBUS_CHAN_DECLARE(MSG_LIST);

/**
 * @} // msg
 */

#endif  // NTURT_MSG_MSG_H_
