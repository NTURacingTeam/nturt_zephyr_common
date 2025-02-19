/**
 * @file
 * @brief Message type definitions.
 *
 * @author quantumspawner
 * @version 0.0.1
 * @date 2025-01-20
 * @copyright Copyright (c) 2025 NTU Racing Team
 */

#ifndef NTURT_MSG_INTERFACES_INTERFACES_H_
#define NTURT_MSG_INTERFACES_INTERFACES_H_

// glibc includes
#include <stdint.h>

// project includes
#include "nturt/msg/interfaces/sensor.h"
#include "nturt/msg/interfaces/stat.h"

/**
 * @defgroup msg_interface Message Interface
 * @brief Message type definitions.
 * @ingroup msg
 * @{
 */

/* macro ---------------------------------------------------------------------*/
/// @brief List of all messages without msg_ prefix.
#define MSG_LIST MSG_SENSOR_LIST, MSG_STAT_LIST

/**
 * @} // msg_interface
 */

#endif  // NTURT_MSG_INTERFACES_INTERFACES_H_
