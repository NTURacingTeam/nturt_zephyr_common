/**
 * @file
 * @brief CAN bus data conversion macros.
 *
 * @author quantumspawner
 * @version 0.0.1
 * @date 2025-07-04
 * @copyright Copyright (c) 2025 NTU Racing Team
 */

#ifndef NTURT_CANBUS_CONVERT_H_
#define NTURT_CANBUS_CONVERT_H_

// glibc includes
#include <stdint.h>

/**
 * @defgroup can_convert CAN bus data conversion.
 * @brief CAN bus data conversion macros.
 *
 * @ingroup can
 * @{
 */

#define INV_DIR_L -1.0F
#define INV_DIR_R 1.0F
#define INV_RATED_TORQUE 20.0F
#define MOTOR_REDUCTION_RATIO 13.1F

/* Physical to CAN -----------------------------------------------------------*/
/// @brief Steering angle physical data in deg (float), convert to 0.01 deg
/// (int16_t).
#define STEER_PHY_TO_CAN(phy) ((int16_t)(100.0F * (phy)))

/// @brief Pedal travel physical data in % (float), convert to % (uint8_t).
#define PEDAL_TRAV_PHY_TO_CAN(phy) ((uint8_t)(phy))

/// @brief Accelerator pedal position physical data in deg (float), convert to
/// deg (int8_t).
#define APPS_RAW_PHY_TO_CAN(phy) ((int8_t)(phy))

/// @brief Brake pressure physical data in kPa (float), convert to bar
/// (uint8_t).
#define BSE_RAW_PHY_TO_CAN(phy) ((uint8_t)(0.01F * (phy)))

/// @brief Velocity physical data in m/s (float), convert to mm/s (int16_t).
#define VELOCITY_PHY_TO_CAN(phy) ((uint16_t)(1000.0F * (phy)))

/// @brief Angular velocity physical data in RPM (float), convert to RPM
/// (uint16_t).
#define ANGULAR_VELOCITY_PHY_TO_CAN(phy) ((uint16_t)(phy))

/// @todo
#define SUSP_PHY_TO_CAN(phy) (phy)

/// @brief Left torque command physical data in Nm (float), convert to 0.001
/// rated torque (int16_t).
#define INV_TORQUE_PHY_TO_CAN_L(phy) \
  ((int16_t)(INV_DIR_L * 1000.0F / INV_RATED_TORQUE * (phy)))

/// @brief Right torque command physical data in Nm (float), convert to 0.001
/// rated torque (int16_t).
#define INV_TORQUE_PHY_TO_CAN_R(phy) \
  ((int16_t)(INV_DIR_R * 1000.0F / INV_RATED_TORQUE * (phy)))

/* CAN to physical -----------------------------------------------------------*/
/// @brief Steering angle CAN data in 0.01 deg (int16_t), convert to deg
/// (float).
#define STEER_CAN_TO_PHY(can) (0.01F * (can))

/// @brief Pedal travel CAN data in % (float), convert to % (uint8_t).
#define PEDAL_TRAV_CAN_TO_PHY(can) ((float)(can))

/// @brief Velocity CAN data in mm/s (uint16_t), convert to m/s (float).
#define VELOCITY_CAN_TO_PHY(can) (0.001F * (can))

/// @brief Angular velocity CAN data in RPM (uint16_t), convert to RPM (float).
#define ANGULAR_VELOCITY_CAN_TO_PHY(can) ((float)(can))

/// @brief Accumulator voltage CAN data in 1/1024 V (uint32_t), convert to V
/// (float).
#define ACC_VOLT_CAN_TO_PHY(can) ((float)(can) / 1024.0F)

/// @brief Accumulator current CAN data in 0.01 A (int16_t), convert to A
/// (float).
#define ACC_CURRENT_CAN_TO_PHY(can) (0.01F * (can))

/// @brief Accumulator temperature CAN data in 0.125 °C (int16_t), convert to °C
/// (float).
#define ACC_TEMP_CAN_TO_PHY(can) (0.125F * (can))

/// @brief Accumulator capacity CAN data in 10 mAh (int16_t), convert to mAh
/// (float).
#define ACC_CAPACITY_CAN_TO_PHY(can) (10.0F * (can))

/// @brief Left inverter speed CAN data in RPM (int16_t) before reduction,
/// convert to RPM (float) after reduction.
#define INV_SPEED_CAN_TO_PHY_L(can) \
  (INV_DIR_L * ANGULAR_VELOCITY_CAN_TO_PHY(can) / MOTOR_REDUCTION_RATIO)

/// @brief Right inverter speed CAN data in RPM (int16_t) before reduction,
/// convert to RPM (float) after reduction.
#define INV_SPEED_CAN_TO_PHY_R(can) \
  (INV_DIR_R * ANGULAR_VELOCITY_CAN_TO_PHY(can) / MOTOR_REDUCTION_RATIO)

/// @brief Left inverter torque CAN data in 0.001 rated torque (int16_t),
/// convert to Nm (float).
#define INV_TORQUE_CAN_TO_PHY_L(can) \
  (INV_DIR_L * 0.001F * INV_RATED_TORQUE * (can))

/// @brief Right inverter torque CAN data in 0.001 rated torque (int16_t),
/// convert to Nm (float).
#define INV_TORQUE_CAN_TO_PHY_R(can) \
  (INV_DIR_R * 0.001F * INV_RATED_TORQUE * (can))

/// @brief Inverter voltage CAN data in 0.01 V (uint16_t), convert to V (float).
#define INV_VOLT_CAN_TO_PHY(can) (0.01F * (can))

/// @brief Inverter current CAN data in 0.01 A (int16_t), convert to A (float).
#define INV_CURRENT_CAN_TO_PHY(can) (0.01F * (can))

/// @brief Inverter MOS temperature CAN data in 0.1 °C (int16_t), convert to °C
/// (float).
#define INV_TEMP_CAN_TO_PHY(can) (0.1F * (can))

/// @brief IMU acceleration CAN data in 0.001 g (int16_t), convert to m/s^2
/// (float).
#define IMU_ACCEL_CAN_TO_PHY(can) (0.00981F * (can))

/// @brief IMU angular velocity CAN data in 0.1 deg/s (int16_t), convert to
/// rad/s (float).
#define IMU_GYRO_CAN_TO_PHY(can) (0.001745F * (can))

/// @brief IMU orientation CAN data in 0.01 deg (int16_t), convert to deg
/// (float).
#define IMU_ORIENT_CAN_TO_PHY(can) (0.01F * (can))

/// @brief GPS longitude CAN data in 1E-7 deg (int32_t), convert to deg (float).
#define GPS_LONGITUDE_CAN_TO_PHY(can) (1E-7 * (can))

/// @brief GPS latitude CAN data in 1E-7 deg (int32_t), convert to deg (float).
#define GPS_LATITUDE_CAN_TO_PHY(can) (1E-7 * (can))

/**
 * @} // can_convert
 */

#endif  // NTURT_CANBUS_CONVERT_H_
