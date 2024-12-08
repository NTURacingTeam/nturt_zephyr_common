#define DT_DRV_COMPAT ssi_p51_curr

// glibc includes
#include <stdint.h>
#include <string.h>

// zephyr includes
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(p51_curr, CONFIG_SENSOR_LOG_LEVEL);

#define PSI_TO_MILLI_HPA 68948

#define P51_CURR_MIN_UA 4000
#define P51_CURR_MAX_UA 20000
#define P51_CURR_SCALE_UA (P51_CURR_MAX_UA - P51_CURR_MIN_UA)

struct p51_curr_data {
  struct sensor_value val;
};

struct p51_curr_config {
  const struct adc_dt_spec adc;

  int full_scale;

  int shunt_resistor;

  int tolerance;
};

static int p51_read_curr(const struct p51_curr_config* config, int* ua) {
  int ret;
  uint16_t raw;
  struct adc_sequence seq = {
      .buffer = &raw,
      .buffer_size = sizeof(raw),
  };

  adc_sequence_init_dt(&config->adc, &seq);
  ret = adc_read_dt(&config->adc, &seq);
  if (ret < 0) {
    return ret;
  }

  int mv = raw;
  adc_raw_to_millivolts_dt(&config->adc, &mv);
  *ua = (int64_t)mv * 1000000 / config->shunt_resistor;

  return 0;
}

static int p51_curr_init(const struct device* dev) {
  const struct p51_curr_config* config = dev->config;

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

static int p51_curr_sample_fetch(const struct device* dev,
                                 enum sensor_channel chan) {
  if (k_is_in_isr()) {
    return -EWOULDBLOCK;
  }

  if (chan != SENSOR_CHAN_ALL && chan != SENSOR_CHAN_PRESS) {
    return -ENOTSUP;
  }

  int ret = 0;
  struct p51_curr_data* data = dev->data;
  const struct p51_curr_config* config = dev->config;

  int ua;
  ret = p51_read_curr(config, &ua);
  if (ret < 0) {
    LOG_ERR("Failed to read ADC channel %s: %s", config->adc.dev->name,
            strerror(-ret));
    return ret;
  }

  if (ua < P51_CURR_MIN_UA * (100 - config->tolerance) / 100) {
    LOG_ERR("%s current below minimum: %d uA", dev->name, ua);
    return -EIO;
  } else if (ua < P51_CURR_MIN_UA) {
    ua = P51_CURR_MIN_UA;
  } else if (ua > P51_CURR_MAX_UA * (100 + config->tolerance) / 100) {
    LOG_ERR("%s current above maximum: %d uA", dev->name, ua);
    return -EIO;
  } else if (ua > P51_CURR_MAX_UA) {
    ua = P51_CURR_MAX_UA;
  }

  int64_t conv = (int64_t)(ua - P51_CURR_MIN_UA) * config->full_scale *
                 PSI_TO_MILLI_HPA / P51_CURR_SCALE_UA;
  sensor_value_from_milli(&data->val, conv);

  return 0;
}

static int p51_curr_channel_get(const struct device* dev,
                                enum sensor_channel chan,
                                struct sensor_value* val) {
  if (chan != SENSOR_CHAN_PRESS) {
    return -ENOTSUP;
  }

  struct p51_curr_data* data = dev->data;
  *val = data->val;

  return 0;
}

static const struct sensor_driver_api p51_curr_api = {
    .sample_fetch = p51_curr_sample_fetch,
    .channel_get = p51_curr_channel_get,
};

#define P51_CURR_INIT(inst)                                                \
  static struct p51_curr_data p51_curr_data_##inst;                        \
                                                                           \
  static const struct p51_curr_config p51_curr_config_##inst = {           \
      .adc = ADC_DT_SPEC_INST_GET_BY_IDX(inst, 0),                         \
      .full_scale = DT_INST_PROP(inst, full_scale),                        \
      .shunt_resistor = DT_INST_PROP(inst, shunt_resistor),                \
      .tolerance = DT_INST_PROP(inst, tolerance),                          \
  };                                                                       \
                                                                           \
  DEVICE_DT_INST_DEFINE(inst, &p51_curr_init, NULL, &p51_curr_data_##inst, \
                        &p51_curr_config_##inst, POST_KERNEL,              \
                        CONFIG_SENSOR_INIT_PRIORITY, &p51_curr_api);

DT_INST_FOREACH_STATUS_OKAY(P51_CURR_INIT)
