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

// project includes
#include "nturt/msg/interfaces/common.h"

/* type ----------------------------------------------------------------------*/
/**
 * @defgroup msg_interface_stat State Message
 * @brief State message type definitions.
 * @ingroup msg_interface
 * @{
 */

/**
 * @brief Vehicle state message.
 *
 * @todo
 */
struct msg_vehicle_state {
  /** Message header. */
  struct msg_header header;
};

/**
 * @} // msg_interface_stat
 */

#endif  // NTURT_MSG_INTERFACES_STAT_H_
