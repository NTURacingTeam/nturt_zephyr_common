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

/**
 * @defgroup msg_interface_sensor Sensor Message
 * @brief Sensor message type definitions.
 *
 * @ingroup msg_interface
 * @{
 */

/* macro ---------------------------------------------------------------------*/
/// @brief List of sensor messages.
#define MSG_SENSOR_LIST \
  msg_cockpit_data, msg_wheel_data, msg_susp_data, msg_imu_data, msg_gps_data

#define PRImsg_cockpit_data                        \
  PRImsg_header                                    \
      "\n\r\tsteer (deg): %g"                      \
      "\n\r\taccel (%%): %g, apps (deg): (%g, %g)" \
      "\n\r\tbrake (%%): %g, bse (kPa): (%g, %g)"

#define PRImsg_cockpit_data_arg(data)                                         \
  PRImsg_header_arg((data).header), (double)((data).steer),                   \
      (double)((data).accel), (double)((data).apps1), (double)((data).apps2), \
      (double)((data).brake), (double)((data).bse1), (double)((data).bse2)

#define PRImsg_wheel_data                                 \
  PRImsg_header "\n\r\tspeed (rpm): %" PRImsg_4wheel_data \
                "\n\r\ttorque (Nm): %" PRImsg_4wheel_data \
                "\n\r\ttire temp (°C): %" PRImsg_4wheel_data

#define PRImsg_wheel_data_arg(data)                                       \
  PRImsg_header_arg((data).header), PRImsg_4wheel_data_arg((data).speed), \
      PRImsg_4wheel_data_arg((data).torque),                              \
      PRImsg_4wheel_data_arg((data).tire_temp)

#define PRImsg_susp_data PRImsg_header "\n\r\ttravel (m): %" PRImsg_4wheel_data

#define PRImsg_susp_data_arg(data) \
  PRImsg_header_arg((data).header), PRImsg_4wheel_data_arg((data).travel)

#define PRImsg_imu_data                                        \
  PRImsg_header "\n\r\tacceleration (m/s^2): %" PRImsg_3d_data \
                "\n\r\tgyration (rad/s): %" PRImsg_3d_data     \
                "\n\r\teular angle (deg): %" PRImsg_3d_data

#define PRImsg_imu_data_arg(data)                                     \
  PRImsg_header_arg((data).header), PRImsg_3d_data_arg((data).accel), \
      PRImsg_3d_data_arg((data).gyro), PRImsg_3d_data_arg((data).orient)

#define PRImsg_gps_data PRImsg_header "\n\r\tTODO"

#define PRImsg_gps_data_arg(data) PRImsg_header_arg((data).header)

/* type ----------------------------------------------------------------------*/
struct msg_cockpit_data {
  /** Message header. */
  struct msg_header header;

  /** Steering angle. Unit: degree */
  float steer;

  /** Accelerator pedal travel. Unit: % */
  float accel;

  /** Accelerator pedal position sensor (APPS) 1. Unit: degree */
  float apps1;

  /** Accelerator pedal position sensor (APPS) 2. Unit: degree */
  float apps2;

  /** Brake pedal travel. Unit: % */
  float brake;

  /** Brake system encoder (BSE) 1. Unit: kPa */
  float bse1;

  /** Brake system encoder (BSE) 2. Unit: kPa */
  float bse2;
};

/// @brief Wheel data message.
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

  /** Tire temperature. Unit: °C */
  union msg_4wheel_data tire_temp;
};

/// @brief Suspension data message.
struct msg_susp_data {
  /** Message header. */
  struct msg_header header;

  /**
   * Suspension travel. Postive for compression, negative for expansion.
   * Unit: m.
   */
  union msg_4wheel_data travel;
};

/// @brief IMU data message.
struct msg_imu_data {
  /** Message header. */
  struct msg_header header;

  /** Acceleration. Unit: m/s^2 */
  union msg_3d_data accel;

  /** Angular velocity. Unit: rad/s */
  union msg_3d_data gyro;

  /** Orientation in Eular angles. Unit: angle */
  union msg_3d_data orient;
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
 * @} // msg_interface_sensor
 */

#endif  // NTURT_MSG_INTERFACES_SENSOR_H_
