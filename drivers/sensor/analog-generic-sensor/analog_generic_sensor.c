#define DT_DRV_COMPAT analog_generic_sensor

// glibc includes
#include <stdint.h>
#include <string.h>

// zephyr includes
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

// project includes
#include "nturt/sys/util.h"

LOG_MODULE_REGISTER(analog_generic_sensor, CONFIG_SENSOR_LOG_LEVEL);

struct analog_generic_data {
  struct sensor_value val;
};

struct analog_generic_config {
  const struct adc_dt_spec adc;
  int channel;
  int min_voltage;
  int max_voltage;
  int voltage_range;
  struct sensor_value min_value;
  struct sensor_value max_value;
  int tolerance;
};

static int analog_generic_init(const struct device* dev) {
  const struct analog_generic_config* config = dev->config;

  int ret;
  if (!adc_is_ready_dt(&config->adc)) {
    LOG_ERR("ADC channel %s is not ready", config->adc.dev->name);
    return -ENODEV;
  }

  ret = adc_channel_setup_dt(&config->adc);
  if (ret < 0) {
    LOG_ERR("Failed to setup ADC channel %s: %s", config->adc.dev->name,
            strerror(-ret));
    return ret;
  }

  return 0;
}

static int analog_generic_sample_fetch(const struct device* dev,
                                       enum sensor_channel chan) {
  struct analog_generic_data* data = dev->data;
  const struct analog_generic_config* config = dev->config;

  if (k_is_in_isr()) {
    return -EWOULDBLOCK;
  }

  if (chan != SENSOR_CHAN_ALL && chan != config->channel) {
    return -ENOTSUP;
  }

  uint16_t raw;
  struct adc_sequence seq = {
      .buffer = &raw,
      .buffer_size = sizeof(raw),
  };

  int ret = 0;
  adc_sequence_init_dt(&config->adc, &seq);
  ret = adc_read_dt(&config->adc, &seq);
  if (ret < 0) {
    LOG_ERR("Failed to read ADC channel %s: %s", config->adc.dev->name,
            strerror(-ret));
    return ret;
  }

  int mv = raw;
  adc_raw_to_millivolts_dt(&config->adc, &mv);
  LOG_DBG("Sensor %s raw %d mv", dev->name, mv);

  if (mv <
      config->min_voltage - config->voltage_range * config->tolerance / 100) {
    goto err_range;
  } else if (mv < config->min_voltage) {
    mv = config->min_voltage;
  } else if (mv > config->max_voltage +
                      config->voltage_range * config->tolerance / 100) {
    goto err_range;
  } else if (mv > config->max_voltage) {
    mv = config->max_voltage;
  }

  int64_t val_range = sensor_value_to_micro(&config->max_value) -
                      sensor_value_to_micro(&config->min_value);
  int64_t val =
      DIV_ROUND_CLOSEST((int64_t)(mv - config->min_voltage) * val_range,
                        config->voltage_range) +
      sensor_value_to_micro(&config->min_value);
  sensor_value_from_micro(&data->val, val);

  return 0;

err_range:
  LOG_ERR_THROTTLE(K_MSEC(500), "Sensor %s voltage out of range: %d mV",
                   dev->name, mv);
  return -EIO;
}

static int analog_generic_channel_get(const struct device* dev,
                                      enum sensor_channel chan,
                                      struct sensor_value* val) {
  struct analog_generic_data* data = dev->data;
  const struct analog_generic_config* config = dev->config;

  if (chan != config->channel) {
    return -ENOTSUP;
  }

  *val = data->val;

  return 0;
}

static DEVICE_API(sensor, analog_generic_api) = {
    .sample_fetch = analog_generic_sample_fetch,
    .channel_get = analog_generic_channel_get,
};

#define ANALOG_GENERIC_INIT(inst)                                              \
  BUILD_ASSERT(                                                                \
      DT_INST_PROP(inst, min_voltage) < DT_INST_PROP(inst, max_voltage),       \
      "min-voltage should be less then max-voltage");                          \
  BUILD_ASSERT(DT_INST_PROP_LEN(inst, min_value) == 2,                         \
               "length of min-value should be two");                           \
  BUILD_ASSERT(DT_INST_PROP_LEN(inst, max_value) == 2,                         \
               "length of max-value should be two");                           \
                                                                               \
  static struct analog_generic_data analog_generic_data_##inst;                \
                                                                               \
  static const struct analog_generic_config analog_generic_config_##inst = {   \
      .adc = ADC_DT_SPEC_INST_GET_BY_IDX(inst, 0),                             \
      .channel = DT_INST_PROP(inst, zephyr_channel),                           \
      .min_voltage = DT_INST_PROP(inst, min_voltage),                          \
      .max_voltage = DT_INST_PROP(inst, max_voltage),                          \
      .voltage_range =                                                         \
          DT_INST_PROP(inst, max_voltage) - DT_INST_PROP(inst, min_voltage),   \
      .min_value = DT_INST_PROP(inst, min_value),                              \
      .max_value = DT_INST_PROP(inst, max_value),                              \
      .tolerance = DT_INST_PROP(inst, tolerance),                              \
  };                                                                           \
                                                                               \
  SENSOR_DEVICE_DT_INST_DEFINE(                                                \
      inst, analog_generic_init, NULL, &analog_generic_data_##inst,            \
      &analog_generic_config_##inst, POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY, \
      &analog_generic_api);

DT_INST_FOREACH_STATUS_OKAY(ANALOG_GENERIC_INIT)
