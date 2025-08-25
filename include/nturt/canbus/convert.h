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

/* Physical to CAN -----------------------------------------------------------*/
/// @brief Steering angle physical data in deg (double), convert to 0.01 deg
/// (int16_t).
#define STEER_PHY_TO_CAN(phy) ((int16_t)(100.0 * (phy)))

/// @brief Pedal travel physical data in % (double), convert to % (uint8_t).
#define PEDAL_TRAV_PHY_TO_CAN(phy) ((uint8_t)(phy))

/// @brief Accelerator pedal position physical data in deg (double), convert to
/// deg (int8_t).
#define APPS_RAW_PHY_TO_CAN(phy) ((int8_t)(phy))

/// @brief Brake pressure physical data in kPa (double), convert to bar
/// (uint8_t).
#define BSE_RAW_PHY_TO_CAN(phy) ((uint8_t)(0.01 * (phy)))

/// @brief Velocity physical data in m/s (double), convert to mm/s (int16_t).
#define VELOCITY_PHY_TO_CAN(phy) ((uint16_t)(1000.0 * (phy)))

/// @brief Angular velocity physical data in RPM (double), convert to RPM
/// (uint16_t).
#define ANGULAR_VELOCITY_PHY_TO_CAN(phy) ((uint16_t)(phy))

/// @todo
#define SUSP_PHY_TO_CAN(phy) (phy)

/// @brief Inverter torque physical data in 0~1 rated torque (double), convert
/// to 0.001 rated torque (int16_t).
#define INV_TORQ_PHY_TO_CAN(phy) ((int16_t)(1000.0 * (phy)))

/* CAN to physical -----------------------------------------------------------*/
/// @brief Steering angle CAN data in 0.01 deg (int16_t), convert to deg
/// (double).
#define STEER_CAN_TO_PHY(can) (0.01 * (can))

/// @brief Pedal travel CAN data in % (double), convert to % (uint8_t).
#define PEDAL_TRAV_CAN_TO_PHY(can) ((double)(can))

/// @brief Velocity CAN data in mm/s (uint16_t), convert to m/s (double).
#define VELOCITY_CAN_TO_PHY(can) (0.001 * (can))

/// @brief Angular velocity CAN data in RPM (uint16_t), convert to RPM (double).
#define ANGULAR_VELOCITY_CAN_TO_PHY(can) ((double)(can))

/// @brief Accumulator voltage CAN data in 1/1024 V (uint32_t), convert to V
/// (double).
#define ACC_VOLT_CAN_TO_PHY(can) ((double)(can) / 1024.0)

/// @brief Accumulator current CAN data in 0.01 A (int16_t), convert to A
/// (double).
#define ACC_CURRENT_CAN_TO_PHY(can) (0.01 * (can))

/// @brief Accumulator temperature CAN data in 0.125 °C (int16_t), convert to °C
/// (double).
#define ACC_TEMP_CAN_TO_PHY(can) (0.125 * (can))

/// @brief Accumulator capacity CAN data in 10 mAh (int16_t), convert to mAh
/// (double).
#define ACC_CAPACITY_CAN_TO_PHY(can) (10.0 * (can))

/// @brief Inverter torque CAN data in 0.001 rated torque (int16_t), convert to
/// 0~1 rated torque (double).
#define INV_TORQUE_CAN_TO_PHY(can) (0.001 * (can))

/// @brief Inverter voltage CAN data in 0.01 V (uint16_t), convert to V
/// (double).
#define INV_VOLT_CAN_TO_PHY(can) (0.01 * (can))

/// @brief Inverter current CAN data in 0.01 A (int16_t), convert to A (double).
#define INV_CURRENT_CAN_TO_PHY(can) (0.01 * (can))

/// @brief Inverter MOS temperature CAN data in 0.1 °C (int16_t), convert to °C
/// (double).
#define INV_TEMP_CAN_TO_PHY(can) (0.1 * (can))

/// @brief IMU acceleration CAN data in 0.001 g (int16_t), convert to m/s^2
/// (double).
#define IMU_ACCEL_CAN_TO_PHY(can) (0.00981 * (can))

/// @brief IMU angular velocity CAN data in 0.1 deg/s (int16_t), convert to
/// rad/s (double).
#define IMU_GYRO_CAN_TO_PHY(can) (0.001745 * (can))

/// @brief IMU orientation CAN data in 0.01 deg (int16_t), convert to deg
/// (double).
#define IMU_ORIENT_CAN_TO_PHY(can) (0.01 * (can))

/// @brief GPS longitude CAN data in 1E-7 deg (int32_t), convert to deg
/// (double).
#define GPS_LONGITUDE_CAN_TO_PHY(can) (1E-7 * (can))

/// @brief GPS latitude CAN data in 1E-7 deg (int32_t), convert to deg (double).
#define GPS_LATITUDE_CAN_TO_PHY(can) (1E-7 * (can))

/**
 * @} // can_convert
 */

#endif  // NTURT_CANBUS_CONVERT_H_
