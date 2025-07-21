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
#define MSG_SENSOR_LIST                                                  \
  msg_sensor_cockpit, msg_sensor_wheel, msg_sensor_susp, msg_sensor_imu, \
      msg_sensor_gps, msg_sensor_pow

/// @brief Insert @ref msg_sensor_cockpit printf format string.
#define PRImsg_sensor_cockpit                      \
  PRImsg_header                                    \
      "\n\r\tsteer (deg): %g"                      \
      "\n\r\taccel (%%): %g, apps (deg): (%g, %g)" \
      "\n\r\tbrake (%%): %g, bse (kPa): (%g, %g)"

/**
 * @brief Insert @ref msg_sensor_cockpit arguments to printf format.
 *
 * @param[in] data The cockpit sensor data.
 */
#define PRImsg_sensor_cockpit_arg(data)                                       \
  PRImsg_header_arg((data).header), (double)((data).steer),                   \
      (double)((data).accel), (double)((data).apps1), (double)((data).apps2), \
      (double)((data).brake), (double)((data).bse1), (double)((data).bse2)

/// @brief Insert @ref msg_sensor_wheel printf format string.
#define PRImsg_sensor_wheel                               \
  PRImsg_header "\n\r\tspeed (rpm): %" PRImsg_4wheel_data \
                "\n\r\ttorque (Nm): %" PRImsg_4wheel_data \
                "\n\r\ttire temp (°C): %" PRImsg_4wheel_data

/**
 * @brief Insert @ref msg_sensor_wheel arguments to printf format.
 *
 * @param[in] data The wheel sensor data.
 */
#define PRImsg_sensor_wheel_arg(data)                                     \
  PRImsg_header_arg((data).header), PRImsg_4wheel_data_arg((data).speed), \
      PRImsg_4wheel_data_arg((data).torque),                              \
      PRImsg_4wheel_data_arg((data).tire_temp)

/// @brief Insert @ref msg_sensor_susp printf format string.
#define PRImsg_sensor_susp \
  PRImsg_header "\n\r\ttravel (m): %" PRImsg_4wheel_data

/**
 * @brief Insert @ref msg_sensor_susp arguments to printf format.
 *
 * @param[in] data The suspension sensor data.
 */
#define PRImsg_sensor_susp_arg(data) \
  PRImsg_header_arg((data).header), PRImsg_4wheel_data_arg((data).travel)

/// @brief Insert @ref msg_sensor_imu printf format string.
#define PRImsg_sensor_imu                                      \
  PRImsg_header "\n\r\tacceleration (m/s^2): %" PRImsg_3d_data \
                "\n\r\tgyration (rad/s): %" PRImsg_3d_data     \
                "\n\r\teular angle (deg): %" PRImsg_3d_data

/**
 * @brief Insert @ref msg_sensor_imu arguments to printf format.
 *
 * @param[in] data The IMU sensor data.
 */
#define PRImsg_sensor_imu_arg(data)                                   \
  PRImsg_header_arg((data).header), PRImsg_3d_data_arg((data).accel), \
      PRImsg_3d_data_arg((data).gyro), PRImsg_3d_data_arg((data).orient)

/// @brief Insert @ref msg_sensor_gps printf format string.
#define PRImsg_sensor_gps PRImsg_header "\n\r\tTODO"

/**
 * @brief Insert @ref msg_sensor_gps arguments to printf format.
 *
 * @param[in] data The GPS sensor data.
 */
#define PRImsg_sensor_gps_arg(data) PRImsg_header_arg((data).header)

/// @brief Insert @ref msg_sensor_pow printf format string.
#define PRImsg_sensor_pow           \
  PRImsg_header                     \
      "\n\r\tinput voltage (V): %g" \
      "\n\r\t5V current (A): %g"    \
      "\n\r\t5V RPi current (A): %g"

/**
 * @brief Insert @ref msg_sensor_pow arguments to printf format.
 *
 * @param[in] data The power sensor data.
 */
#define PRImsg_sensor_pow_arg(data)                           \
  PRImsg_header_arg((data).header), (double)((data).in_volt), \
      (double)((data).v5_curr), (double)((data).v5_rpi_curr)

/* type ----------------------------------------------------------------------*/
/// @brief Cockpit sensors message.
struct msg_sensor_cockpit {
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

/// @brief Wheel sensors message.
struct msg_sensor_wheel {
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

/// @brief Suspension sensors message.
struct msg_sensor_susp {
  /** Message header. */
  struct msg_header header;

  /**
   * Suspension travel. Postive for compression, negative for expansion.
   * Unit: m.
   */
  union msg_4wheel_data travel;
};

/// @brief IMU message.
struct msg_sensor_imu {
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
 * @brief GPS message.
 *
 * @todo
 */
struct msg_sensor_gps {
  /** Message header. */
  struct msg_header header;
};

/// @brief Power sensors message.
struct msg_sensor_pow {
  /** Message header. */
  struct msg_header header;

  /** Input voltage. Unit: V */
  float in_volt;

  /** 5V current. Unit: A */
  float v5_curr;

  /** 5V Raspberry Pi current. Unit: A */
  float v5_rpi_curr;
};

/**
 * @} // msg_interface_sensor
 */

#endif  // NTURT_MSG_INTERFACES_SENSOR_H_
