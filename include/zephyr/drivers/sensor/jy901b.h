/**
 * @file jy901b.h
 * @author QuantumSpawner jet22854111@gmail.com
 * @brief
 *
 */
#ifndef ZEPHYR_DRIVERS_SENSOR_JY901B_H_
#define ZEPHYR_DRIVERS_SENSOR_JY901B_H_

// glibc include
#include <stdint.h>

// zephyr include
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>

// data std
#define JY901B_ACCEL_STD 0.001F           // g
#define JY901B_GYRO_STD 0.07F             // deg/s
#define JY901B_ORIEN_ROLL_PITCH_STD 0.2F  // deg
#ifdef CONFIG_JY901B_ORIEN_ABSLOUTE
#define JY901B_ORIEN_YAW_STD 1.0F  // deg
#elif defined(CONFIG_JY901B_ORIEN_RELATIVE)
#define JY901B_ORIEN_YAW_STD 0.5F  // deg
#endif

#define JY901B_PRESS_STD 6F  // Pa

// data scale
#define JY901B_DATA_SHIFT 15

#define JY901B_ACCEL_SCALE 16
#define JY901B_GYRO_SCALE 2000
#define JY901B_MAGN_SCALE 83 // determined experimentally
#define JY901B_DEG_SCALE 180
#define JY901B_QUAT_SCALE 1000

#define JY901B_TEMP_SCALE 1 / 100
#define JY901B_PRESS_SCALE 1 / 1000

// i2c register
// data
#define JY901B_DATA_REG_START 0x34
#define JY901B_DATA_LENGTH 26

#define JY901B_PRESSURE_REG 0x45
#define JY901B_PRESSURE_LENGTH 4

#define JY901B_QUAT_REG 0x51
#define JY901B_QUAT_LENGTH 8

// config
#define JY901B_SAVE_REG 0x00
#define JY901B_SAVE_LENGTH 2
#define JY901B_SAVE_SETTING 0x0000

#define JY901B_I2C_ADDR_REG 0x1A

#define JY901B_ORIEN_MODE_REG 0x24
#define JY901B_ORIEN_MODE_LENGTH 2
#define JY901B_ORIEN_MODE_ABSOLUTE 0x0000
#define JY901B_ORIEN_MODE_RELATIVE 0x0001

#define JY901B_WRITE_ENABLE_REG 0x69
#define JY901B_WRITE_ENABLE_LENGTH 2
#define JY901B_WRITE_ENABLE_KEY 0xB588

enum jy901b_channel {
  /// @brief Orientation in degrees.
  SENSOR_CHAN_ORIEN_RPY = SENSOR_CHAN_PRIV_START,
  SENSOR_CHAN_ORIEN_ROLL,
  SENSOR_CHAN_ORIEN_PITCH,
  SENSOR_CHAN_ORIEN_YAW,

  /// @brief Quaternion in 1 / 1000.
  SENSOR_CHAN_QUAT_WXYZ,
  SENSOR_CHAN_QUAT_W,
  SENSOR_CHAN_QUAT_X,
  SENSOR_CHAN_QUAT_Y,
  SENSOR_CHAN_QUAT_Z,
};

#endif /* ZEPHYR_DRIVERS_SENSOR_JY901B_H_ */
