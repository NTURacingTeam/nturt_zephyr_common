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
/// @brief Voltage physical data in V (float), convert to 0.02 V (uint8_t).
#define VOLTAGE_TO_CAN(PHY) (uint8_t)(50.0F * PHY)

/// @brief Current physical data in A (float), convert to 0.02 A (uint8_t).
#define CURRENT_TO_CAN(PHY) (uint8_t)(50.0F * PHY)

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

/// @brief Inverter motor speed data in rad/s (float), convert to RPM (int16_t).
#define INV_SPEED_PHY_TO_CAN(PHY) (int16_t)(95.493F * PHY)

/// @brief Inverter torque data in m*s (float), convert to 29.1 / 10000 m*s
/// (int16_t).
#define INV_TORQUE_PHY_TO_CAN(PHY) (int16_t)(PHY)

/// @brief Inverter voltage data in V (float), convert to 0.01 V (uint16_t).
#define INV_VOLTAGE_PHY_TO_CAN(PHY) (uin16_t)(100.0F * PHY)

/// @brief Inverter current data in A (float), convert to 0.01 A (int16_t).
#define INV_CURRENT_PHY_TO_CAN(PHY) (int16_t)(100.0F * PHY)

/* CAN to physical -----------------------------------------------------------*/
/// @brief Voltage CAN data in 0.02 V (uint8_t), convert to V (float).
#define VOLTAGE_CAN_TO_PHY(CAN) (0.02F * CAN)

/// @brief Current CAN data in 0.02 A (uint8_t), convert to A (float).
#define CURRENT_CAN_TO_PHY(CAN) (0.02F * CAN)

/// @brief Pedal travel CAN data in 0.5% (uint8_t), convert to 0~1 (float).
#define PEDAL_TRAV_CAN_TO_PHY(CAN) ((float)CAN / 200.0F)

/// @brief Wheel speed CAN data in RPM (uint16_t), convert to (float).
#define WHEEL_SPEED_CAN_TO_PHY(CAN) (float)(CAN)

/// @brief IMU CAN acceleration data in 0.001 g (int16_t), convert to m/s^2
/// (float).
#define IMU_ACCEL_CAN_TO_PHY(CAN) (0.00981F * CAN)

/// @brief IMU CAN gyro data in 0.1 deg/s (int16_t), convert to RPM
/// (float).
#define IMU_GYRO_CAN_TO_PHY(CAN) (CAN / 60.0F)

/// @brief Inverter motor speed data in RPM (int16_t), convert to (float).
#define INV_SPEED_CAN_TO_PHY(CAN) (float)(CAN)

/// @brief Inverter torque data in 0.001 m*s (int16_t), convert to m*s (float).
#define INV_TORQUE_CAN_TO_PHY(CAN) (0.001F * CAN)

/// @brief Inverter voltage data in 0.01 V (uint16_t), convert to V (float).
#define INV_VOLTAGE_CAN_TO_PHY(CAN) (0.01F * CAN)

/// @brief Inverter current data in 0.01 A (int16_t), convert to A (float).
#define INV_CURRENT_CAN_TO_PHY(CAN) (0.01F * CAN)

/**
 * @} // can_convert
 */

#endif  // NTURT_CANBUS_CONVERT_H_
