#ifndef ZEPHYR_DRIVERS_SENSOR_AMT21_H_
#define ZEPHYR_DRIVERS_SENSOR_AMT21_H_

// zephyr includes
#include <zephyr/drivers/sensor.h>

/* type ----------------------------------------------------------------------*/
enum amt21_attribute {
  SENSOR_ATTR_AMT21_RESET = SENSOR_ATTR_PRIV_START,
  SENSOR_ATTR_AMT21_SET_ZERO_POS,
};

#endif  // ZEPHYR_DRIVERS_SENSOR_AMT21_H_
