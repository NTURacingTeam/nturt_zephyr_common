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

/**
 * @defgroup msg_if Message Interface
 * @brief Message type definitions.
 *
 * @ingroup msg
 * @{
 */

/**
 * @defgroup msg_if_pri Message Printing
 * @brief Message printing format strings.
 *
 * @ingroup msg_if
 * @{
 */

/**
 * @} // msg_if_pri
 */

/**
 * @defgroup msg_if_csv Message CSV
 * @brief Message CSV format strings.
 *
 * @ingroup msg_if
 *  @{
 */

/**
 * @} // msg_if_csv
 */

/**
 * @} // msg_if
 */

#endif  // NTURT_MSG_INTERFACES_INTERFACES_H_
