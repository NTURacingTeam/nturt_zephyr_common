/**
 * @file
 * @brief Tractive system message type definitions.
 *
 * @author quantumspawner
 * @version 0.0.1
 * @date 2025-07-05
 * @copyright Copyright (c) 2025 NTU Racing Team
 */

#ifndef NTURT_MSG_INTERFACES_TRACTIVE_H_
#define NTURT_MSG_INTERFACES_TRACTIVE_H_

// glibc includes
#include <stdbool.h>
#include <stdint.h>

// project includes
#include "nturt/msg/interfaces/common.h"

/**
 * @defgroup msg_interface_tractive Tractive System Message
 * @brief Tractive system message type definitions.
 *
 * @ingroup msg_interface
 * @{
 */

/* macro ---------------------------------------------------------------------*/
/// @brief List of tractive system messages.
#define MSG_TRACTIVE_LIST msg_acc, msg_inv

/* type ----------------------------------------------------------------------*/
/// @brief Accumulator message.
struct msg_acc {
  /** Message header. */
  struct msg_header header;

  /** Accumulator status. */
  bool ok;

  /** Accumulator voltage. Unit: V. */
  float volt;

  /** Accumulator current. Unit: A. */
  float current;

  /** Accumulator temperature. Unit: °C. */
  float temp;

  /** Accumulator state of charge (SOC). Unit: %. */
  int soc;

  /** Accumulator capacity. Unit: mAh. */
  float capacity;
};

/// @brief Inverter message.
struct msg_inv {
  /** Message header. */
  struct msg_header header;

  /** Inverter status word. */
  union msg_4wheel_flags status;

  /** DC bus voltage. Unit: V. */
  union msg_4wheel_data volt;

  /** DC bus current. Unit: A. */
  union msg_4wheel_data current;

  /** Inverter MOS temperature. Unit: °C. */
  union msg_4wheel_data inv_temp;

  /** Motor temperature. Unit: °C. */
  union msg_4wheel_data motor_temp;

  /** MCU temperature. Unit: °C. */
  union msg_4wheel_data mcu_temp;
};

/**
 * @} // msg_interface_tractive
 */

#endif  // NTURT_MSG_INTERFACES_TRACTIVE_H_
