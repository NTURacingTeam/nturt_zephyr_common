// glibc includes
#include <math.h>
#include <stdbool.h>
#include <stddef.h>

// zephyr includes
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc/adc_emul.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/ztest.h>

// project includes
#include "dt-settings.h"

#define VAL_TOL 0.005

#define VOLTAGE_RANGE (double)(MAX_VOLTAGE - MIN_VOLTAGE)
#define VALUE_RANGE (double)(MAX_VALUE - MIN_VALUE)

static const struct device* adc = DEVICE_DT_GET(DT_NODELABEL(adc));
static const struct device* sensor = DEVICE_DT_GET(DT_NODELABEL(sensor));

static bool value_equal(double a, double b) {
  return fabs((a - b) / VALUE_RANGE) < VAL_TOL;
}

ZTEST_SUITE(analog_generic, NULL, NULL, NULL, NULL, NULL);

ZTEST(analog_generic, test_range) {
  struct sensor_value value;

  for (int i = 0; i < 100; i++) {
    adc_emul_const_value_set(adc, 0, MIN_VOLTAGE + VOLTAGE_RANGE * i / 100);
    zassert_ok(sensor_sample_fetch(sensor));
    zassert_ok(sensor_channel_get(sensor, SENSOR_CHAN, &value));
    zassert_true(value_equal(sensor_value_to_double(&value),
                             MIN_VALUE + VALUE_RANGE * i / 100));
  }
}

ZTEST(analog_generic, test_clamp) {
  struct sensor_value value;

  adc_emul_const_value_set(adc, 0,
                           MIN_VOLTAGE - VOLTAGE_RANGE * (TOLARENCE - 1) / 100);
  zassert_ok(sensor_sample_fetch(sensor));
  zassert_ok(sensor_channel_get(sensor, SENSOR_CHAN, &value));
  zassert_equal(value.val1, MIN_VALUE);

  adc_emul_const_value_set(adc, 0,
                           MAX_VOLTAGE + VOLTAGE_RANGE * (TOLARENCE - 1) / 100);
  zassert_ok(sensor_sample_fetch(sensor));
  zassert_ok(sensor_channel_get(sensor, SENSOR_CHAN, &value));
  zassert_equal(value.val1, MAX_VALUE);
}

ZTEST(analog_generic, test_out_of_range) {
  adc_emul_const_value_set(adc, 0,
                           MIN_VOLTAGE - VOLTAGE_RANGE * (TOLARENCE + 1) / 100);
  zassert_not_ok(sensor_sample_fetch(sensor));

  adc_emul_const_value_set(adc, 0,
                           MAX_VOLTAGE + VOLTAGE_RANGE * (TOLARENCE + 1) / 100);
  zassert_not_ok(sensor_sample_fetch(sensor));
}
