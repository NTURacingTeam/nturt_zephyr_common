/**
 * @file
 * @brief Inter-thread communication logging support.
 *
 * @author quantumspawner
 * @version 0.0.1
 * @date 2025-08-03
 * @copyright Copyright (c) 2025 NTU Racing Team
 */

#ifndef NTURT_MSG_LOGGING_H_
#define NTURT_MSG_LOGGING_H_

// glibc includes
#include <stdbool.h>

// zephyr includes
#include <zephyr/fs/fs.h>
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>

/**
 * @defgroup msg_logging Message Logging
 * @brief Inter-thread communication logging support.
 *
 * @ingroup msg
 * @{
 */

/* macro ---------------------------------------------------------------------*/
#define MSG_LOGGING

/* type ----------------------------------------------------------------------*/

/// @brief Message logging support.
struct msg_logging {
  /** Lock to protect the following members. */
  struct k_mutex lock;
  bool is_logging;

  /** File for logging. */
  struct fs_file_t file;
};

/* function declaration ------------------------------------------------------*/
/**
 * @brief Start logging messages on a channel to a file.
 * 
 * @param[in] chan Message channel to log.
 * @param[in] file Path to the file to log messages to, will create the
 * directory to the file if it does not exist.
 * @retval 0 on success.
 * @retval -EALREADY if logging is already started on the channel.
 * @retval others Negative error number on file operations failure.
 */
int msg_chan_logging_start(const struct zbus_channel *chan, const char *file);

/**
 * @brief Stop logging messages on a channel.
 * 
 * @param[in] chan Message channel to stop logging.
 * @retval 0 on success.
 * @retval -ENOTCONN if logging is not started on the channel.
 * @retval others Negative error number on file operations failure.
 */
int msg_chan_logging_stop(const struct zbus_channel *chan);
/**
 * @} // msg_logging
 */

#endif  // NTURT_MSG_LOGGING_H_
