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
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>

// project includes
#include "nturt/msg/aggregation.h"
#include "nturt/msg/interfaces/interfaces.h"
#include "nturt/sys/util.h"

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

#define _MSG_SHELL_PRINT(msg) CONCAT(__msg_shell_print_, msg)

#define _MSG_SHELL_DEFINE(msg)                                            \
  static bool _MSG_SHELL_PRINT(msg)(const struct zbus_channel *chan) {    \
    if (chan != &_MSG_CHAN_NAME(msg)) {                                   \
      return false;                                                       \
    }                                                                     \
                                                                          \
    const struct msg *data = zbus_chan_const_msg(chan);                   \
    LOG_INF("%s:\n\r\t%" CONCAT(PRI, msg), #msg,                          \
            CONCAT(PRI, msg, _arg)(*data));                               \
    return true;                                                          \
  }                                                                       \
                                                                          \
  const STRUCT_SECTION_ITERABLE(msg_shell, CONCAT(__msg_shell_, msg)) = { \
      .chan = &_MSG_CHAN_NAME(msg),                                       \
      .print = _MSG_SHELL_PRINT(msg),                                     \
  }

/**
 * @brief Define one message shell support for every message in @p list .
 *
 * @param[in] list List of messages to define shell support.
 *
 * @note The messages are printed using logging, so a log module must be
 * registered or declared before using this macro.
 */
#define MSG_SHELL_DEFINE(list) N_FOR_EACH(_MSG_SHELL_DEFINE, (;), list)

/* type ----------------------------------------------------------------------*/
/**
 * @brief Print function of a message.
 *
 * @param[in] chan Message channel.
 *
 * @return true if the message was printed, false otherwise.
 */
typedef bool (*msg_print_t)(const struct zbus_channel *chan);

/// @brief Message shell support.
struct msg_shell {
  /** Message channel. */
  const struct zbus_channel *chan;

  /** Print function for the message. */
  msg_print_t print;
};

/* exported variable ---------------------------------------------------------*/
MSG_ZBUS_CHAN_DECLARE(MSG_LIST);

/**
 * @} // msg
 */

#endif  // NTURT_MSG_MSG_H_
