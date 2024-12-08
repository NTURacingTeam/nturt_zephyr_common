#include "nturt/sensors.h"

// glibc includes
#include <errno.h>
#include <stdbool.h>

// zephyr includes
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

// project includes
#include "nturt/err.h"

LOG_MODULE_REGISTER(nturt_sensors, CONFIG_NTURT_LOG_LEVEL);

/* static function declaration -----------------------------------------------*/
/**
 * @brief Update the fail count and set error if the tolerance is reached.
 *
 * @param tol Sensor tolerant read structure.
 * @param fail If sensor failed.
 */
static void sensor_tol_update_fail(struct sensor_tol *tol, bool fail);

/* function definition -------------------------------------------------------*/
int sensor_tol_init(struct sensor_tol *tol) {
  if (!device_is_ready(tol->dev)) {
    err_set_errors(tol->err, true);
    LOG_ERR("%s not ready", tol->dev->name);

    return -ENODEV;
  }

  return 0;
}

bool sensor_tol_is_ok(struct sensor_tol *tol) {
  return !(err_get_errors() & tol->err);
}

int sensor_tol_chan_read(struct sensor_tol *tol, enum sensor_channel chan,
                         struct sensor_value *val) {
  if (!sensor_tol_is_ok(tol)) {
    return -ENODEV;
  }

  int ret;
  bool fail = (ret = sensor_sample_fetch_chan(tol->dev, chan)) < 0 ||
              (ret = sensor_channel_get(tol->dev, chan, val)) < 0;
  sensor_tol_update_fail(tol, fail);
  return ret;
}

void sensor_tol_report_fail(struct sensor_tol *tol) {
  sensor_tol_update_fail(tol, true);
}

void sensor_tol_report_error(struct sensor_tol *tol) {
  err_set_errors(tol->err, true);
}

/* static function definition ------------------------------------------------*/
static void sensor_tol_update_fail(struct sensor_tol *tol, bool fail) {
  if (fail) {
    tol->level += tol->weight;

    if (tol->level >= tol->thres) {
      err_set_errors(tol->err, true);
      LOG_ERR("Sensor %s error level %d reaches threshold %d", tol->dev->name,
              tol->level, tol->thres);

    } else {
      LOG_WRN("Sensor %s error level increased to %d", tol->dev->name,
              tol->level);
    }

  } else {
    tol->level = MAX(0, tol->level - 1);
  }
}
