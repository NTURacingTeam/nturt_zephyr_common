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
#include <stdio.h>

// zerphyr includes
#include <zephyr/logging/log.h>
#include <zephyr/sys/iterable_sections.h>
#include <zephyr/zbus/zbus.h>

// project includes
#include "nturt/msg/aggregation.h"
#include "nturt/msg/interfaces/interfaces.h"
#include "nturt/msg/logging.h"
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

#define _MSG_CHAN(msg) CONCAT(msg, _chan)
#define _MSG_PRINT(msg) CONCAT(__msg_print_, msg)
#define _MSG_CSV_HEADER(msg) CONCAT(__msg_csv_header_, msg)
#define _MSG_CSV_WRITE(msg) CONCAT(__msg_csv_write_, msg)
#define _MSG_CHAN_DATA(msg) CONCAT(__msg_chan_data_, msg)

#define _MSG_CHAN_DEFINE(msg)                                                 \
  BUILD_ASSERT(sizeof(struct msg) <= CONFIG_NTURT_MSG_MAX_SIZE,               \
               "Message size exceeds CONFIG_NTURT_MSG_MAX_SIZE, increase it " \
               "to accommodate " #msg);                                       \
                                                                              \
  static void _MSG_PRINT(msg)(const void *data) {                             \
    LOG_INF("%s:\n\r\t%" CONCAT(PRI, msg), #msg,                              \
            CONCAT(PRI, msg, _arg)(*(const struct msg *)data));               \
  }                                                                           \
                                                                              \
  static const char *_MSG_CSV_HEADER(msg)() {                                 \
    return CONCAT(CSV_PRI, msg, _header);                                     \
  }                                                                           \
                                                                              \
  static int _MSG_CSV_WRITE(msg)(char *buf, size_t len, const void *data) {   \
    return snprintf(buf, len, "%" CONCAT(CSV_PRI, msg),                       \
                    CONCAT(CSV_PRI, msg, _arg)(*(const struct msg *)data));   \
  }                                                                           \
                                                                              \
  static STRUCT_SECTION_ITERABLE(msg_chan_data, _MSG_CHAN_DATA(msg)) = {      \
      .print = _MSG_PRINT(msg),                                               \
      .csv_header = _MSG_CSV_HEADER(msg),                                     \
      .csv_write = _MSG_CSV_WRITE(msg),                                       \
  };                                                                          \
                                                                              \
  ZBUS_CHAN_DEFINE(_MSG_CHAN(msg), struct msg, NULL, &_MSG_CHAN_DATA(msg),    \
                   ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(0))

/**
 * @brief Define one message channel for every message in @p list .
 *
 * @param[in] list List of messages to define channels.
 *
 * @note This macro uses logging to print messages, so a log module must be
 * registered or declared before using this macro.
 */
#define MSG_CHAN_DEFINE(list) N_FOR_EACH(_MSG_CHAN_DEFINE, (;), list)

/**
 * @brief Declare one message channel for every message in @p list .
 *
 * @param[in] list List of messages to define channels.
 */
#define MSG_CHAN_DECLARE(list) \
  ZBUS_CHAN_DECLARE(FOR_EACH(_MSG_CHAN, (, ), list))

/* type ----------------------------------------------------------------------*/
/**
 * @brief Print function of a message.
 *
 * @param[in] data Pointer to the message data.
 */
typedef void (*msg_print_t)(const void *data);

/**
 * @brief Function to get the CSV header of a message.
 *
 * @return Pointer to the CSV header string.
 */
typedef const char *(*msg_csv_header_t)();

/**
 * @brief Function to write a message to a CSV format.
 *
 * @param[in] buf Buffer to write the CSV data to.
 * @param[in] len Length of the buffer.
 * @param[in] data Pointer to the message data.
 * @return Number of bytes that would be written to the buffer if it were large
 * enough, excluding the null terminator.
 */
typedef int (*msg_csv_write_t)(char *buf, size_t len, const void *data);

/// @brief Message channel data.
struct msg_chan_data {
  /** Print function for the message. */
  msg_print_t print;

  /** Function to get the CSV header of the message. */
  msg_csv_header_t csv_header;

  /** Function to write the message to a CSV format. */
  msg_csv_write_t csv_write;

#ifdef CONFIG_NTURT_MSG_LOGGING
  struct msg_logging logging;
#endif
};

/* exported variable ---------------------------------------------------------*/
MSG_CHAN_DECLARE(MSG_LIST);

/* function declaration ------------------------------------------------------*/
/**
 * @brief Check if the given zbus channel is a message channel.
 *
 * @param chan Zbus channel to check.
 * @return True if the channel is a message channel, false otherwise.
 */
bool is_msg_chan(const struct zbus_channel *chan);

/**
 * @brief Print a message based on the channel.
 *
 * @param[in] chan Message channel.
 * @param[in] data Pointer to the message data.
 *
 * @warning Only zbus channels defined with @ref MSG_CHAN_DEFINE can be used
 * with this function.
 */
void msg_chan_print(const struct zbus_channel *chan, const void *data);

/**
 * @brief Get the CSV header of a message channel.
 *
 * @param[in] chan Message channel.
 * @return Pointer to the CSV header string.
 *
 * @warning Only zbus channels defined with @ref MSG_CHAN_DEFINE can be used
 * with this function.
 */
const char *msg_chan_csv_header(const struct zbus_channel *chan);

/**
 * @brief Write a message in CSV format to a buffer based on the channel.
 *
 * @param[in] chan Message channel.
 * @param[in] data Pointer to the message data.
 * @param[in] buf Buffer to write the CSV data to.
 * @param[in] len Length of the buffer.
 * @return Number of bytes that would be written to the buffer if it were large
 * enough, excluding the null terminator.
 *
 * @warning Only zbus channels defined with @ref MSG_CHAN_DEFINE can be used
 * with this function.
 */
int msg_chan_csv_write(const struct zbus_channel *chan, const void *data,
                       char *buf, size_t len);

/**
 * @} // msg
 */

#endif  // NTURT_MSG_MSG_H_
