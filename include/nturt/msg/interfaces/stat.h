/**
 * @file
 * @brief State message type definitions.
 *
 * @author quantumspawner
 * @version 0.0.1
 * @date 2025-01-22
 * @copyright Copyright (c) 2025 NTU Racing Team
 */

#ifndef NTURT_MSG_INTERFACES_STAT_H_
#define NTURT_MSG_INTERFACES_STAT_H_

// glibc includes
#include <stdint.h>

// project includes
#include "nturt/msg/interfaces/common.h"

/**
 * @defgroup msg_interface_stat State Message
 * @brief State message type definitions.
 * @ingroup msg_interface
 * @{
 */

/* macro ---------------------------------------------------------------------*/
/// @brief List of state messages without msg_ prefix.
#define MSG_STAT_LIST vehicle_stat

/* type ----------------------------------------------------------------------*/
/**
 * @brief Vehicle state message.
 *
 * @todo
 */
struct msg_vehicle_stat {
  /** Message header. */
  struct msg_header header;
};

/**
 * @} // msg_interface_stat
 */

#endif  // NTURT_MSG_INTERFACES_STAT_H_
