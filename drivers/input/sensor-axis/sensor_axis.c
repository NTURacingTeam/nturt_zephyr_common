#define DT_DRV_COMPAT sensor_axis

#include <zephyr/drivers/input/sensor_axis.h>

// glibc includes
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// zephyr includes
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/dt-bindings/input/deadzone-modes.h>
#include <zephyr/dt-bindings/input/input-error-codes.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(sensor_axis, CONFIG_INPUT_LOG_LEVEL);

/* type ----------------------------------------------------------------------*/
struct sensor_axis_sensor_raw_cb {
  struct k_mutex lock;
  sensor_axis_sensor_raw_cb_t cb;
  void* user_data;
};

struct sensor_axis_sensor_data {
  uint16_t error;
  struct sensor_axis_sensor_raw_cb raw_cb;
};

struct sensor_axis_sensor_calib {
  struct sensor_value in_min;
  struct sensor_value in_max;
};

struct sensor_axis_sensor_config {
  const struct device* sensor;
  enum sensor_channel channel;
  struct sensor_axis_sensor_calib* calib;
  int32_t range_tolerance;
};

struct sensor_axis_channel_data {
  uint16_t error;
  int32_t prev_out;
};

struct sensor_axis_channel_config {
  size_t num_sensor;
  const struct device** sensors;
  int total_weight;
  int* weights;
  int32_t out_min;
  int32_t out_max;
  int32_t deadzone_mode;
  int32_t deadzone_size;
  int32_t noise;
  int32_t dev_tolerance;
};

struct sensor_axis_data {
  struct k_timer timer;
  struct k_thread thread;
  K_KERNEL_STACK_MEMBER(thread_stack,
                        CONFIG_INPUT_SENSOR_AXIS_THREAD_STACK_SIZE);
};

struct sensor_axis_config {
  size_t num_channel;
  const struct device** channels;
  uint16_t* axises;
  int poll_period;
};

/* static function declaration -----------------------------------------------*/
static int sensor_get_raw(const struct device* dev, struct sensor_value* val);

/* function definition -------------------------------------------------------*/
void sensor_axis_sensor_set_raw_cb(const struct device* dev,
                                   sensor_axis_sensor_raw_cb_t cb,
                                   void* user_data) {
  struct sensor_axis_sensor_data* data = dev->data;

  data->raw_cb.cb = cb;
  data->raw_cb.user_data = user_data;
}

void sensor_axis_sensor_min_get(const struct device* dev,
                                struct sensor_value* val) {
  struct sensor_axis_sensor_data* data = dev->data;
  const struct sensor_axis_sensor_config* config = dev->config;

  k_mutex_lock(&data->raw_cb.lock, K_FOREVER);
  *val = config->calib->in_min;
  k_mutex_unlock(&data->raw_cb.lock);
}

void sensor_axis_sensor_max_get(const struct device* dev,
                                struct sensor_value* val) {
  struct sensor_axis_sensor_data* data = dev->data;
  const struct sensor_axis_sensor_config* config = dev->config;

  k_mutex_lock(&data->raw_cb.lock, K_FOREVER);
  *val = config->calib->in_max;
  k_mutex_unlock(&data->raw_cb.lock);
}

void sensor_axis_sensor_min_set(const struct device* dev,
                                const struct sensor_value* val) {
  struct sensor_axis_sensor_data* data = dev->data;
  const struct sensor_axis_sensor_config* config = dev->config;

  k_mutex_lock(&data->raw_cb.lock, K_FOREVER);
  config->calib->in_min = *val;
  k_mutex_unlock(&data->raw_cb.lock);
}

void sensor_axis_sensor_max_set(const struct device* dev,
                                const struct sensor_value* val) {
  struct sensor_axis_sensor_data* data = dev->data;
  const struct sensor_axis_sensor_config* config = dev->config;

  k_mutex_lock(&data->raw_cb.lock, K_FOREVER);
  config->calib->in_max = *val;
  k_mutex_unlock(&data->raw_cb.lock);
}

static int sensor_axis_sensor_set_curr_impl(const struct device* dev, int times,
                                            k_timeout_t period, bool is_min) {
  struct sensor_axis_sensor_data* data = dev->data;
  const struct sensor_axis_sensor_config* config = dev->config;
  struct sensor_axis_sensor_calib* calib = config->calib;

  k_mutex_lock(&data->raw_cb.lock, K_FOREVER);

  int64_t accum = 0;
  struct sensor_value val;

  int ret = 0;
  for (int i = 0; i < times; i++) {
    ret = sensor_get_raw(dev, &val);
    if (ret < 0) {
      LOG_ERR("sensor_get_raw failed: %s", strerror(-ret));
      goto out;
    }

    accum += sensor_value_to_micro(&val);

    if (i < times - 1) {
      k_sleep(period);
    }
  }

  accum = DIV_ROUND_CLOSEST(accum, times);
  sensor_value_from_micro(is_min ? &calib->in_min : &calib->in_max, accum);

out:
  k_mutex_unlock(&data->raw_cb.lock);
  return ret;
}

int sensor_axis_sensor_min_set_curr(const struct device* dev, int times,
                                    k_timeout_t period) {
  return sensor_axis_sensor_set_curr_impl(dev, times, period, true);
}

int sensor_axis_sensor_max_set_curr(const struct device* dev, int times,
                                    k_timeout_t period) {
  return sensor_axis_sensor_set_curr_impl(dev, times, period, false);
}

/* static function definition ------------------------------------------------*/
static int sensor_get_raw(const struct device* dev, struct sensor_value* val) {
  struct sensor_axis_sensor_data* data = dev->data;
  const struct sensor_axis_sensor_config* config = dev->config;

  k_mutex_lock(&data->raw_cb.lock, K_FOREVER);

  int ret;
  ret = sensor_sample_fetch(config->sensor);
  if (ret < 0) {
    goto out;
  }

  ret = sensor_channel_get(config->sensor, config->channel, val);
  if (ret < 0) {
    goto out;
  }

out:
  k_mutex_unlock(&data->raw_cb.lock);
  return ret;
}

/**
 * @brief Update sensor error.
 *
 * @param[in] dev The sensor to update.
 * @param[in] error New error code to update.
 * @param[in] ret Current return value.
 * @param[in] info Additional information to report.
 *
 * @return Original @p ret if success, negative error number otherwise.
 */
static int sensor_error_update(const struct device* dev, uint16_t error,
                               int ret, const void* info) {
  struct sensor_axis_sensor_data* data = dev->data;
  const struct sensor_axis_sensor_config* config = dev->config;

  int ret2;
  if (data->error == INPUT_ERROR_NONE && error != INPUT_ERROR_NONE) {
    ret2 = input_report(dev, INPUT_EV_ERROR, error, true, true, K_NO_WAIT);

  } else if (data->error != INPUT_ERROR_NONE && error == INPUT_ERROR_NONE) {
    ret2 =
        input_report(dev, INPUT_EV_ERROR, data->error, false, true, K_NO_WAIT);

  } else {
    return ret;
  }

  if (ret2 < 0) {
    LOG_ERR("input_report failed: %s", strerror(-ret2));
    return ret2;
  }

  data->error = error;

  switch (error) {
    case INPUT_ERROR_NONE:
      LOG_INF("Sensor %s error clears", dev->name);
      break;

    case INPUT_ERROR_IO:
      LOG_ERR("Sensor %s errors: %s", config->sensor->name, strerror(-ret));
      break;

    case INPUT_ERROR_BUSY:
      LOG_ERR("Sensor %s busy", dev->name);
      break;

    case INPUT_ERROR_UNDER: {
      const int64_t* val = info;
      LOG_ERR("Sensor %s value below range by %lld%%", dev->name,
              DIV_ROUND_CLOSEST(-*val, 10000));
      break;
    }

    case INPUT_ERROR_OVER: {
      const int64_t* val = info;
      LOG_ERR("Sensor %s value above range by %lld%%", dev->name,
              DIV_ROUND_CLOSEST(*val, 10000));
      break;
    }
  }

  return ret;
}

/**
 * @brief Get sensor value.
 *
 * @param[in] dev The sensor to get.
 * @param[out] _val The sensor value in one-millionth part.
 *
 * @return 0 If success, negative error number otherwise.
 */
static int sensor_get(const struct device* dev, int32_t* _val) {
  struct sensor_axis_sensor_data* data = dev->data;
  const struct sensor_axis_sensor_config* config = dev->config;

  int ret;
  ret = k_mutex_lock(&data->raw_cb.lock, K_NO_WAIT);
  if (ret < 0) {
    return sensor_error_update(dev, INPUT_ERROR_BUSY, ret, NULL);
  }

  struct sensor_value sensor_val;
  ret = sensor_get_raw(config->sensor, &sensor_val);
  if (ret < 0) {
    k_mutex_unlock(&data->raw_cb.lock);
    return sensor_error_update(dev, INPUT_ERROR_IO, ret, NULL);
  }

  int64_t val = sensor_value_to_micro(&sensor_val);
  int64_t min = sensor_value_to_micro(&config->calib->in_min);
  int64_t max = sensor_value_to_micro(&config->calib->in_max);

  k_mutex_unlock(&data->raw_cb.lock);

  if (data->raw_cb.cb != NULL) {
    data->raw_cb.cb(dev, &sensor_val, data->raw_cb.user_data);
  }

  /* devide range first to prevent overflow */
  int64_t range = DIV_ROUND_CLOSEST(max - min, 1000);
  val = DIV_ROUND_CLOSEST((val - min) * 1000, range);

  if (config->range_tolerance >= 0) {
    if (val < -config->range_tolerance * 10000) {
      return sensor_error_update(dev, INPUT_ERROR_UNDER, -EINVAL, &sensor_val);

    } else if (val > 1000000 + config->range_tolerance * 10000) {
      return sensor_error_update(dev, INPUT_ERROR_OVER, -EINVAL, &sensor_val);
    }
  }

  if (val < 0) {
    val = 0;
  } else if (val > 1000000) {
    val = 1000000;
  }

  ret = sensor_error_update(dev, INPUT_ERROR_NONE, 0, NULL);
  if (ret < 0) {
    return ret;
  }

  *_val = val;
  return 0;
}

static int32_t out_deadzone(const struct device* dev, int32_t out) {
  const struct sensor_axis_channel_config* config = dev->config;

  if (config->deadzone_size == 0) {
    return out;
  }

  switch (config->deadzone_mode) {
    case INPUT_DEADZONE_MODE_CENTER:
      if (out < 500000 + config->deadzone_size * 10000 &&
          out > 500000 - config->deadzone_size * 10000) {
        return 500000;
      }

      break;

    case INPUT_DEADZONE_MODE_EDGE:
      if (out < config->deadzone_size * 10000) {
        return 0;
      }

      break;

    default:
      return -1;
  }

  return out;
}

/**
 * @brief Update channel error.
 *
 * @param[in] dev The channel to update.
 * @param[in] error New error code to update.
 * @param[in] ret Current return value.
 * @param[in] info Additional information to report.
 *
 * @return Original @p ret if success, negative error number otherwise.
 */
static int channel_error_update(const struct device* dev, uint16_t error,
                                int ret, const void* info) {
  struct sensor_axis_channel_data* data = dev->data;

  int ret2;
  if (data->error == INPUT_ERROR_NONE && error != INPUT_ERROR_NONE) {
    ret2 = input_report(dev, INPUT_EV_ERROR, error, true, true, K_NO_WAIT);

  } else if (data->error != INPUT_ERROR_NONE && error == INPUT_ERROR_NONE) {
    ret2 =
        input_report(dev, INPUT_EV_ERROR, data->error, false, true, K_NO_WAIT);

  } else {
    return ret;
  }

  if (ret2 < 0) {
    LOG_ERR("input_report failed: %s", strerror(-ret2));
    return ret2;
  }

  data->error = error;

  switch (error) {
    case INPUT_ERROR_NONE:
      LOG_INF("Channel %s error clears", dev->name);
      break;

    case INPUT_ERROR_IO: {
      const char* sensor_name = info;
      LOG_ERR("Sensor %s errors: %s", sensor_name, strerror(-ret));
      break;
    }

    case INPUT_ERROR_NODEV: {
      const char* sensor_name = info;
      LOG_ERR("Sensor %s not ready, channel %s is disabled", sensor_name,
              dev->name);
      break;
    }

    case INPUT_ERROR_OVER: {
      const int32_t* val = info;
      LOG_ERR("Channel %s deviates by %d%%", dev->name,
              DIV_ROUND_CLOSEST(*val, 10000));
      break;
    }
  }

  return ret;
}

/**
 * @brief Get channel output.
 *
 * @param[in] dev The channel to get.
 * @param[out] _out The channel output.
 *
 * @retval 1 If the channel value is updated.
 * @retval 0 If the channel value is not updated.
 * @retval others Negative error number if error occurs.
 */
static int channel_get(const struct device* dev, int32_t* _out) {
  struct sensor_axis_channel_data* data = dev->data;
  const struct sensor_axis_channel_config* config = dev->config;

  int ret;

  /* normalized sensor values in one-millionth part */
  int32_t val_accum = 0;
  int32_t val_min = INT32_MAX;
  int32_t val_max = INT32_MIN;
  for (int i = 0; i < config->num_sensor; i++) {
    const struct device* sensor = config->sensors[i];

    if (!device_is_ready(sensor)) {
      return channel_error_update(dev, INPUT_ERROR_NODEV, -ENODEV,
                                  sensor->name);
    }

    int32_t val;
    ret = sensor_get(sensor, &val);
    if (ret < 0) {
      return channel_error_update(dev, INPUT_ERROR_IO, ret, sensor->name);
    }

    val_accum += val * config->weights[i];
    val_min = MIN(val_min, val);
    val_max = MAX(val_max, val);
  }

  int32_t val_dev = val_max - val_min;
  if (val_dev > config->dev_tolerance * 10000) {
    return channel_error_update(dev, INPUT_ERROR_DEV, -EINVAL, &val_dev);
  }

  ret = channel_error_update(dev, INPUT_ERROR_NONE, 0, NULL);
  if (ret < 0) {
    return ret;
  }

  int32_t out = out_deadzone(dev, val_accum / config->total_weight);
  if (abs(out - data->prev_out) < config->noise) {
    return 0;
  }

  /* devide out first to prevent overflow */
  out = DIV_ROUND_CLOSEST(out, 1000);
  *_out = DIV_ROUND_CLOSEST(out * (config->out_max - config->out_min), 1000) +
          config->out_min;
  return 1;
}

static void sensor_axis_thread(void* arg1, void* arg2, void* arg3) {
  (void)arg2;
  (void)arg3;

  const struct device* dev = arg1;
  struct sensor_axis_data* data = dev->data;
  const struct sensor_axis_config* config = dev->config;

  while (true) {
    for (int i = 0; i < config->num_channel; i++) {
      const struct device* channel = config->channels[i];

      int ret;
      int32_t out;
      ret = channel_get(channel, &out);
      if (ret > 0) {
        ret =
            input_report_abs(channel, config->axises[i], out, true, K_NO_WAIT);
        if (ret < 0) {
          LOG_ERR("input_report failed: %s", strerror(-ret));
        }
      }
    }

    k_timer_status_sync(&data->timer);
  }
}

static int sensor_axis_sensor_init(const struct device* dev) {
  struct sensor_axis_sensor_data* data = dev->data;
  const struct sensor_axis_sensor_config* config = dev->config;

  k_mutex_init(&data->raw_cb.lock);

  if (!device_is_ready(config->sensor)) {
    LOG_ERR("Sensor %s not ready", config->sensor->name);
    return -ENODEV;
  }

  return 0;
}

static int sensor_axis_init(const struct device* dev) {
  struct sensor_axis_data* data = dev->data;
  const struct sensor_axis_config* config = dev->config;

  k_timer_init(&data->timer, NULL, NULL);

  k_tid_t tid = k_thread_create(&data->thread, data->thread_stack,
                                K_KERNEL_STACK_SIZEOF(data->thread_stack),
                                sensor_axis_thread, (void*)dev, NULL, NULL,
                                CONFIG_INPUT_SENSOR_AXIS_THREAD_PRIORITY, 0,
                                K_MSEC(config->poll_period));
  if (!tid) {
    LOG_ERR("thread creation failed");
    return -ENODEV;
  }

#ifdef CONFIG_THREAD_NAME
  char thread_name[CONFIG_THREAD_MAX_NAME_LEN];
  snprintf(thread_name, sizeof(thread_name), "input_sensor_axis_%s", dev->name);
  k_thread_name_set(&data->thread, thread_name);
#endif // CONFIG_THREAD_NAME

  k_timer_start(&data->timer, K_MSEC(config->poll_period),
                K_MSEC(config->poll_period));

  return 0;
}

#ifdef CONFIG_INPUT_SENSOR_AXIS_SETTINGS

/* macro ---------------------------------------------------------------------*/
#define SENSOR_AXIS_SETTINGS_ROOT "sensor_axis"

/* static function definition ------------------------------------------------*/
static int sensor_axis_calib_load(const char* key, size_t len_rd,
                                  settings_read_cb read_cb, void* cb_arg);

/* static variable -----------------------------------------------------------*/
SETTINGS_STATIC_HANDLER_DEFINE(sensor_axis, SENSOR_AXIS_SETTINGS_ROOT, NULL,
                               sensor_axis_calib_load, NULL, NULL);

/* function definition -------------------------------------------------------*/
int sensor_axis_sensor_calib_save(const struct device* dev) {
  int ret;
  struct sensor_value val;
  char path[SETTINGS_MAX_NAME_LEN + 1];
  ret = snprintf(path, sizeof(path), SENSOR_AXIS_SETTINGS_ROOT "/%s/in_min",
                 dev->name);
  __ASSERT(ret < SETTINGS_MAX_NAME_LEN, "Path too long");

  sensor_axis_sensor_min_get(dev, &val);
  ret = settings_save_one(path, &val, sizeof(struct sensor_value));
  if (ret < 0) {
    goto err_settings;
  }

  ret = snprintf(path, sizeof(path), SENSOR_AXIS_SETTINGS_ROOT "/%s/in_max",
                 dev->name);
  __ASSERT(ret < SETTINGS_MAX_NAME_LEN, "Path too long");

  sensor_axis_sensor_max_get(dev, &val);
  ret = settings_save_one(path, &val, sizeof(struct sensor_value));
  if (ret < 0) {
    goto err_settings;
  }

  return 0;

err_settings:
  LOG_ERR("settings_save failed: %d", ret);
  return ret;
}

/* static function definition ------------------------------------------------*/
static int sensor_axis_calib_load(const char* key, size_t len,
                                  settings_read_cb read_cb, void* cb_arg) {
  if (len != sizeof(struct sensor_value)) {
    return -EINVAL;
  }

  ssize_t ret;
  struct sensor_value val;
  ret = read_cb(cb_arg, &val, sizeof(struct sensor_value));
  if (ret < 0) {
    return ret;
  }

  int nlen;
  const char* next;
  nlen = settings_name_next(key, &next);

  char dev_name[Z_DEVICE_MAX_NAME_LEN];
  memcpy(dev_name, key, nlen);
  dev_name[nlen] = '\0';

  const struct device* dev = device_get_binding(dev_name);
  if (dev == NULL) {
    LOG_ERR("Calibration restore failed: device %s not available", dev_name);
    return -ENODEV;
  }

  key = next;
  if (settings_name_steq(key, "in_min", &next) && !next) {
    sensor_axis_sensor_min_set(dev, &val);
  } else if (settings_name_steq(key, "in_max", &next) && !next) {
    sensor_axis_sensor_max_set(dev, &val);
  } else {
    return -ENOENT;
  }

  return 0;
}

#endif  // CONFIG_INPUT_SENSOR_AXIS_SETTINGS

#define SENSOR_AXIS_SENSOR_DEFINE(node_id)                                     \
  static struct sensor_axis_sensor_data node_id##_data = {                     \
      .error = INPUT_ERROR_NONE,                                               \
      .raw_cb =                                                                \
          {                                                                    \
              .cb = NULL,                                                      \
          },                                                                   \
  };                                                                           \
                                                                               \
  const static struct sensor_axis_sensor_config node_id##_config = {           \
      .sensor = DEVICE_DT_GET(DT_PHANDLE(node_id, sensor)),                    \
      .channel = DT_PROP(node_id, zephyr_channel),                             \
      .calib = (struct sensor_axis_sensor_calib[]){{                           \
          .in_min =                                                            \
              {                                                                \
                  .val1 = DT_PROP_BY_IDX(node_id, in_min, 0) +                 \
                          ZERO_OR_COMPILE_ERROR(                               \
                              DT_PROP_LEN(node_id, in_min) == 2),              \
                  .val2 = DT_PROP_BY_IDX(node_id, in_min, 1),                  \
              },                                                               \
          .in_max =                                                            \
              {                                                                \
                  .val1 = DT_PROP_BY_IDX(node_id, in_max, 0) +                 \
                          ZERO_OR_COMPILE_ERROR(                               \
                              DT_PROP_LEN(node_id, in_max) == 2),              \
                  .val2 = DT_PROP_BY_IDX(node_id, in_max, 1),                  \
              },                                                               \
      }},                                                                      \
      .range_tolerance = DT_PROP(node_id, range_tolerance),                    \
  };                                                                           \
                                                                               \
  DEVICE_DT_DEFINE(node_id, sensor_axis_sensor_init, NULL, &node_id##_data,    \
                   &node_id##_config, POST_KERNEL, CONFIG_INPUT_INIT_PRIORITY, \
                   NULL)

#define _SENSOR_AXIS_SENSOR_WEIGHT(node_id) DT_PROP(node_id, weight)

/*
 * DT_FOREACH_CHILD_STATUS_OKAY_SEP does not work within the same call to
 * itself, define a new one with the same definition instead
 */
#define _DT_FOREACH_CHILD_STATUS_OKAY_SEP(node_id, fn, sep) \
  DT_CAT(node_id, _FOREACH_CHILD_STATUS_OKAY_SEP)(fn, sep)

#define SENSOR_AXIS_CHANNEL_DEFINE(node_id)                                  \
  DT_CAT(node_id, _FOREACH_CHILD_STATUS_OKAY_SEP)(SENSOR_AXIS_SENSOR_DEFINE, \
                                                  (;));                      \
                                                                             \
  static struct sensor_axis_channel_data node_id##_data = {                  \
      .error = INPUT_ERROR_NONE,                                             \
      .prev_out = 0,                                                         \
  };                                                                         \
                                                                             \
  const static struct sensor_axis_channel_config node_id##_config = {        \
      .num_sensor = DT_CHILD_NUM_STATUS_OKAY(node_id),                       \
      .sensors = (const struct device*[]){_DT_FOREACH_CHILD_STATUS_OKAY_SEP( \
          node_id, DEVICE_DT_GET, (, ))},                                    \
      .total_weight = (_DT_FOREACH_CHILD_STATUS_OKAY_SEP(                    \
          node_id, _SENSOR_AXIS_SENSOR_WEIGHT, (+))),                        \
      .weights = (int[]){_DT_FOREACH_CHILD_STATUS_OKAY_SEP(                  \
          node_id, _SENSOR_AXIS_SENSOR_WEIGHT, (, ))},                       \
      .out_min = DT_PROP(node_id, out_min),                                  \
      .out_max = DT_PROP(node_id, out_max),                                  \
      .deadzone_mode = DT_PROP(node_id, deadzone_mode),                      \
      .deadzone_size = DT_PROP(node_id, deadzone_size),                      \
      .noise =                                                               \
          DT_PROP(node_id, noise) +                                          \
          ZERO_OR_COMPILE_ERROR(DT_PROP(node_id, deadzone_size) == 0 ||      \
                                DT_PROP(node_id, deadzone_size) * 10000 >=   \
                                    DT_PROP(node_id, noise)),                \
      .dev_tolerance = DT_PROP(node_id, dev_tolerance),                      \
  };                                                                         \
                                                                             \
  DEVICE_DT_DEFINE(node_id, NULL, NULL, &node_id##_data, &node_id##_config,  \
                   POST_KERNEL, CONFIG_INPUT_INIT_PRIORITY, NULL)

#define _SENSOR_AXIS_CHANNEL_AXIS(node_id) DT_PROP(node_id, zephyr_axis)

#define SENSOR_AXIS_INIT(inst)                                                \
  DT_INST_FOREACH_CHILD_STATUS_OKAY_SEP(inst, SENSOR_AXIS_CHANNEL_DEFINE,     \
                                        (;));                                 \
                                                                              \
  static struct sensor_axis_data sensor_axis_data_##inst = {};                \
                                                                              \
  const static struct sensor_axis_config sensor_axis_config_##inst = {        \
      .num_channel = DT_INST_CHILD_NUM_STATUS_OKAY(inst),                     \
      .channels =                                                             \
          (const struct device*[]){DT_INST_FOREACH_CHILD_STATUS_OKAY_SEP(     \
              inst, DEVICE_DT_GET, (, ))},                                    \
      .axises = (uint16_t[]){DT_INST_FOREACH_CHILD_STATUS_OKAY_SEP(           \
          inst, _SENSOR_AXIS_CHANNEL_AXIS, (, ))},                            \
      .poll_period = DT_INST_PROP(inst, poll_period),                         \
  };                                                                          \
                                                                              \
  DEVICE_DT_INST_DEFINE(inst, sensor_axis_init, NULL,                         \
                        &sensor_axis_data_##inst, &sensor_axis_config_##inst, \
                        POST_KERNEL, CONFIG_INPUT_INIT_PRIORITY, NULL);

DT_INST_FOREACH_STATUS_OKAY(SENSOR_AXIS_INIT)
