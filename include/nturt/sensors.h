#ifndef NTURT_SENSORS_H_
#define NTURT_SENSORS_H_

// glibc includes
#include <stdbool.h>

// zephyr includes
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>

// project includes
#include "nturt/err.h"

/* macro ---------------------------------------------------------------------*/
/**
 * @brief Designated initializer for @ref sensor_tol.
 *
 * @param NODE_ID Node ID of the sensor device.
 * @param THRES Error level when greater or equal is considered an error.
 * @param WEIGHT Error level increase of one sensor failure.
 * @param ERR Error code corresponding to the sensor.
 */
#define SENSOR_TOL_INITIALIZER(NODE_ID, THRES, WEIGHT, ERR) \
  {                                                         \
      .dev = DEVICE_DT_GET(NODE_ID),                        \
      .thres = THRES,                                       \
      .weight = WEIGHT,                                     \
      .err = ERR,                                           \
      .level = 0,                                           \
  }

/**
 * @brief Instantiate a @ref sensor_tol.
 *
 * @param NAME Name of the sensor tolerant read structure.
 * @param NODE_ID Node ID of the sensor device.
 * @param THRES Error level when greater or equal is considered an error.
 * @param WEIGHT Error level increase of one sensor failure.
 * @param ERR Error code corresponding to the sensor.
 */
#define SENSOR_TOL_DEFINE(NAME, NODE_ID, THRES, WEIGHT, ERR) \
  static struct sensor_tol NAME =                            \
      SENSOR_TOL_INITIALIZER(NODE_ID, THRES, WEIGHT, ERR)

/* type ----------------------------------------------------------------------*/
/// @brief Sensor tolerant structure.
struct sensor_tol {
  /// @brief Sensor device.
  const struct device *dev;

  /// @brief Error level when greater or equal is considered an error.
  const int thres;

  /// @brief Error level increase of one sensor failure.
  const int weight;

  /// @brief Error code corresponding to the sensor.
  const enum err_code err;

  /// @brief Error level.
  int level;
};

/* function declaration ------------------------------------------------------*/
int sensor_tol_init(struct sensor_tol *tol);

bool sensor_tol_is_ok(struct sensor_tol *tol);

/**
 * @brief Read sensor data with tolerance. Sets error when error level reaches
 * threshold. Does nothing if the sensor error is set and returns -ENODEV.
 *
 * @param tol Sensor tolerant read structure.
 * @param chan Sensor channel.
 * @param val Sensor value.
 * @return 0 if read successful, negative error code otherwise.
 */
int sensor_tol_chan_read(struct sensor_tol *tol, enum sensor_channel chan,
                         struct sensor_value *val);

/**
 * @brief Report sensor failure.
 *
 * @param tol Sensor tolerant read structure.
 */
void sensor_tol_report_fail(struct sensor_tol *tol);

void sensor_tol_report_error(struct sensor_tol *tol);

#endif  // NTURT_SENSORS_H_
