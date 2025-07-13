/**
 * @file
 * @brief Vehicle control message type definitions.
 *
 * @author quantumspawner
 * @version 0.0.1
 * @date 2025-07-13
 * @copyright Copyright (c) 2025 NTU Racing Team
 */

#ifndef NTURT_MSG_INTERFACES_CTRL_H_
#define NTURT_MSG_INTERFACES_CTRL_H_

// project includes
#include "nturt/msg/interfaces/common.h"

/**
 * @defgroup msg_interface_ctrl Vehicle Control Message
 * @brief Vehicle control message type definitions.
 *
 * @ingroup msg_interface
 * @{
 */

/* macro ---------------------------------------------------------------------*/
#define MSG_CTRL_LIST msg_ctrl_word, msg_ctrl_torque

#define PRImsg_ctrl_word \
  PRImsg_header "\n\r\tcontrol word: %" PRImsg_4wheel_flags

#define PRImsg_ctrl_word_arg(data) \
  PRImsg_header_arg((data).header), PRImsg_4wheel_flags_arg((data).ctrl)

#define PRImsg_ctrl_torque \
  PRImsg_header "\n\r\ttarget torque: %" PRImsg_4wheel_data

#define PRImsg_ctrl_torque_arg(data) \
  PRImsg_header_arg((data).header), PRImsg_4wheel_data_arg((data).torque)

/* type ----------------------------------------------------------------------*/
/// @brief Inverter control word message.
struct msg_ctrl_word {
  struct msg_header header;

  /** Inverter control word. */
  union msg_4wheel_flags ctrl;
};

/// @brief Inverter target torque message.
struct msg_ctrl_torque {
  struct msg_header header;

  /** Inverter target torque. */
  union msg_4wheel_data torque;
};

/**
 * @} // msg_interface_ctrl
 */

#endif  // NTURT_MSG_INTERFACES_CTRL_H_
