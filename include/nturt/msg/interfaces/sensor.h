/**
 * @file
 * @brief Sensor message type definitions.
 *
 * @author quantumspawner
 * @version 0.0.1
 * @date 2025-01-20
 * @copyright Copyright (c) 2025 NTU Racing Team
 */

#ifndef NTURT_MSG_INTERFACES_SENSOR_H_
#define NTURT_MSG_INTERFACES_SENSOR_H_

// project includes
#include "nturt/msg/interfaces/common.h"

/* macro ---------------------------------------------------------------------*/
/// @brief List of sensor messages without msg_ prefix.
#define MSG_SENSOR_LIST imu_data, gps_data, wheel_data, susp_data

/* type ----------------------------------------------------------------------*/
/**
 * @defgroup msg_interface_sensor Sensor Message
 * @brief Sensor message type definitions.
 * @ingroup msg_interface
 * @{
 */

/**
 * @brief IMU data message.
 *
 */
struct msg_imu_data {
  /** Message header. */
  struct msg_header header;

  /** Orientation in Eular angles. Unit: angle */
  union msg_3d_data orient;

  /** Acceleration. Unit: m/s^2 */
  union msg_3d_data accel;

  /** Angular velocity. Unit: rad/s */
  union msg_3d_data gyro;
};

/**
 * @brief GPS data message.
 *
 * @todo
 */
struct msg_gps_data {
  /** Message header. */
  struct msg_header header;
};

/**
 * @brief Wheel data message.
 *
 */
struct msg_wheel_data {
  /** Message header. */
  struct msg_header header;

  /** Wheel speed. Unit: rpm */
  union msg_4wheel_data speed;

  /**
   * Wheel feedback torque. Postive for asserting torque to wheel, negative
   * for braking. Unit: Nm
   */
  union msg_4wheel_data torque;
};

/**
 * @brief Suspension data message.
 *
 */
struct msg_susp_data {
  /** Message header. */
  struct msg_header header;

  /**
   * Suspension travel. Postive for compression, negative for expansion.
   * Unit: m.
   */
  union msg_4wheel_data travel;
};

/**
 * @} // msg_interface_sensor
 */

#endif  // NTURT_MSG_INTERFACES_SENSOR_H_
