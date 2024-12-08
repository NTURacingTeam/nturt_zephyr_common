#define DT_DRV_COMPAT res_zx

// glibc includes
#include <stdint.h>
#include <string.h>

// zephyr includes
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(zx, CONFIG_SENSOR_LOG_LEVEL);

#define ZX_MAX_MV 3300

struct zx_data {
  struct sensor_value val;
};

struct zx_config {
  const struct adc_dt_spec adc;

  int stroke;
};

static int zx_init(const struct device* dev) {
  const struct zx_config* config = dev->config;

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

static int zx_sample_fetch(const struct device* dev, enum sensor_channel chan) {
  if (k_is_in_isr()) {
    return -EWOULDBLOCK;
  }

  if (chan != SENSOR_CHAN_ALL && chan != SENSOR_CHAN_DISTANCE) {
    return -ENOTSUP;
  }

  int ret = 0;
  struct zx_data* data = dev->data;
  const struct zx_config* config = dev->config;

  uint16_t raw;
  struct adc_sequence seq = {
      .buffer = &raw,
      .buffer_size = sizeof(raw),
  };

  adc_sequence_init_dt(&config->adc, &seq);
  ret = adc_read_dt(&config->adc, &seq);
  if (ret < 0) {
    LOG_ERR("Failed to read ADC channel %s: %s", config->adc.dev->name,
            strerror(-ret));
    return ret;
  }

  int mv = raw;
  adc_raw_to_millivolts_dt(&config->adc, &mv);
  sensor_value_from_micro(&data->val, (int64_t)config->stroke *
                                          (ZX_MAX_MV - mv) * 1000 / ZX_MAX_MV);

  return 0;
}

static int zx_channel_get(const struct device* dev, enum sensor_channel chan,
                          struct sensor_value* val) {
  if (chan != SENSOR_CHAN_DISTANCE) {
    return -ENOTSUP;
  }

  struct zx_data* data = dev->data;
  *val = data->val;

  return 0;
}

static const struct sensor_driver_api zx_api = {
    .sample_fetch = zx_sample_fetch,
    .channel_get = zx_channel_get,
};

#define ZX_INIT(inst)                                          \
  static struct zx_data zx_data_##inst;                        \
                                                               \
  static const struct zx_config zx_config_##inst = {           \
      .adc = ADC_DT_SPEC_INST_GET_BY_IDX(inst, 0),             \
      .stroke = DT_INST_PROP(inst, stroke),                    \
  };                                                           \
                                                               \
  DEVICE_DT_INST_DEFINE(inst, &zx_init, NULL, &zx_data_##inst, \
                        &zx_config_##inst, POST_KERNEL,        \
                        CONFIG_SENSOR_INIT_PRIORITY, &zx_api);

DT_INST_FOREACH_STATUS_OKAY(ZX_INIT)
