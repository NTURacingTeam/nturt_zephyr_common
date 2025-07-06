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
/// @brief Steering angle physical data in deg (float), convert to 0.01 deg
/// (int16_t).
#define STEER_PHY_TO_CAN(PHY) (int16_t)(PHY / 0.01F)

/// @brief Pedal travel physical data in 0~1 (float), convert to 0.5% (uint8_t).
#define PEDAL_TRAV_PHY_TO_CAN(PHY) (uint8_t)(200.0F * PHY)

/// @todo
#define APPS_RAW_PHY_TO_CAN(PHY) (int8_t)(10.0F * PHY)

/// @brief Brake pressure physical data in hPa (float), convert to bar
/// (uint8_t).
#define BSE_RAW_PHY_TO_CAN(PHY) (uint8_t)(PHY / 1000.0F)

/// @brief Wheel speed physical data in RPM (float), convert to RPM (uint16_t).
#define WHEEL_SPEED_PHY_TO_CAN(PHY) (uint16_t)(PHY)

/// @todo
#define SUSP_PHY_TO_CAN(PHY) (PHY)

/* CAN to physical -----------------------------------------------------------*/
/// @brief Accumulator voltage CAN data in 1/1024 V (uint32_t), convert to V
/// (float).
#define ACC_VOLT_CAN_TO_PHY(CAN) ((float)CAN / 1024.0F)

/// @brief Accumulator current CAN data in 0.01 A (int16_t), convert to A
/// (float).
#define ACC_CURRENT_CAN_TO_PHY(CAN) (0.01F * CAN)

/// @brief Accumulator temperature CAN data in 0.125 °C (int16_t), convert to °C
/// (float).
#define ACC_TEMP_CAN_TO_PHY(CAN) (0.125F * CAN)

/// @brief Accumulator capacity CAN data in 10 mAh (int16_t), convert to mAh
/// (float).
#define ACC_CAPACITY_CAN_TO_PHY(CAN) (10.0F * CAN)

/// @brief Inverter motor speed data in RPM (int16_t), convert to (float).
#define INV_SPEED_CAN_TO_PHY(CAN) (float)(CAN)

/// @brief Inverter torque data in 0.001 m*s (int16_t), convert to m*s (float).
#define INV_TORQUE_CAN_TO_PHY(CAN) (0.001F * CAN)

/// @brief Inverter voltage data in 0.01 V (uint16_t), convert to V (float).
#define INV_VOLT_CAN_TO_PHY(CAN) (0.01F * CAN)

/// @brief Inverter current data in 0.01 A (int16_t), convert to A (float).
#define INV_CURRENT_CAN_TO_PHY(CAN) (0.01F * CAN)

/// @brief Inverter MOS temperature data in 0.1 °C (int16_t), convert to °C
/// (float).
#define INV_TEMP_CAN_TO_PHY(CAN) (0.1F * CAN)

/// @brief IMU CAN acceleration data in 0.001 g (int16_t), convert to m/s^2
/// (float).
#define IMU_ACCEL_CAN_TO_PHY(CAN) (0.00981F * CAN)

/// @brief IMU CAN gyro data in 0.1 deg/s (int16_t), convert to RPM
/// (float).
#define IMU_GYRO_CAN_TO_PHY(CAN) (CAN / 60.0F)

/// @brief IMU CAN orientation data in 0.01 deg (int16_t), convert to
/// degrees (float).
#define IMU_ORIENT_CAN_TO_PHY(CAN) (CAN / 100.0F)

/**
 * @} // can_convert
 */

#endif  // NTURT_CANBUS_CONVERT_H_
