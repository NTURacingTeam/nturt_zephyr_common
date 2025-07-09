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

// zephyr includes
#include <zephyr/sys/util.h>

// project includes
#include "nturt/msg/interfaces/sensor.h"
#include "nturt/msg/interfaces/tractive.h"

#ifdef CONFIG_NTURT_MSG_CHAN_STATES
#include "nturt/msg/interfaces/states.h"
#endif

/**
 * @defgroup msg_interface Message Interface
 * @brief Message type definitions.
 *
 * @ingroup msg
 * @{
 */

/* macro ---------------------------------------------------------------------*/
/// @brief List of all messages.
#define MSG_LIST                          \
  LIST_DROP_EMPTY(                        \
      MSG_SENSOR_LIST, MSG_TRACTIVE_LIST, \
      COND_CODE_1(CONFIG_NTURT_MSG_CHAN_STATES, (MSG_STATES_LIST), ()))

/**
 * @} // msg_interface
 */

#endif  // NTURT_MSG_INTERFACES_INTERFACES_H_
