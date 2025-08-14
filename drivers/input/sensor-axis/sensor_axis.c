#include "sensor_axis.h"

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
#include <zephyr/dt-bindings/input/input-error-codes.h>
#include <zephyr/dt-bindings/input/sensor-axis.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(sensor_axis, CONFIG_INPUT_LOG_LEVEL);

/* macro ---------------------------------------------------------------------*/
#define WAIT_PERIOD K_MSEC(5)

#define INVALID_OUT (INT32_MAX)

/// @brief Hysteresis for checking if the value is in range. The final tolerance
/// should be (1 - RANGE_HYSTERESIS) when out of range.
#define RANGE_HYSTERESIS 10 / 100

/* static function declaration -----------------------------------------------*/
static int sensor_get_raw(const struct device* dev, struct sensor_value* val);

static int sensor_axis_sensor_get_curr(const struct device* dev, int times,
                                       k_timeout_t period, uint64_t* curr);

/* function definition -------------------------------------------------------*/
void sensor_axis_sensor_set_raw_cb(const struct device* dev,
                                   sensor_axis_sensor_raw_cb_t cb,
                                   void* user_data) {
  struct sensor_axis_sensor_data* data = dev->data;

  k_mutex_lock(&data->lock, K_FOREVER);
  data->cb = cb;
  data->user_data = user_data;
  k_mutex_unlock(&data->lock);
}

void sensor_axis_sensor_min_get(const struct device* dev,
                                struct sensor_value* val) {
  struct sensor_axis_sensor_data* data = dev->data;

  k_mutex_lock(&data->lock, K_FOREVER);
  *val = data->in_min;
  k_mutex_unlock(&data->lock);
}

void sensor_axis_sensor_max_get(const struct device* dev,
                                struct sensor_value* val) {
  struct sensor_axis_sensor_data* data = dev->data;

  k_mutex_lock(&data->lock, K_FOREVER);
  *val = data->in_max;
  k_mutex_unlock(&data->lock);
}

void sensor_axis_sensor_min_set(const struct device* dev,
                                const struct sensor_value* val) {
  struct sensor_axis_sensor_data* data = dev->data;

  k_mutex_lock(&data->lock, K_FOREVER);
  data->in_min = *val;
  k_mutex_unlock(&data->lock);
}

void sensor_axis_sensor_max_set(const struct device* dev,
                                const struct sensor_value* val) {
  struct sensor_axis_sensor_data* data = dev->data;

  k_mutex_lock(&data->lock, K_FOREVER);
  data->in_max = *val;
  k_mutex_unlock(&data->lock);
}

int sensor_axis_sensor_min_set_curr(const struct device* dev, int times,
                                    k_timeout_t interval) {
  uint64_t curr;
  int ret = sensor_axis_sensor_get_curr(dev, times, interval, &curr);
  if (ret < 0) {
    return ret;
  }

  struct sensor_value val;
  sensor_value_from_micro(&val, curr);
  sensor_axis_sensor_min_set(dev, &val);

  return 0;
}

int sensor_axis_sensor_max_set_curr(const struct device* dev, int times,
                                    k_timeout_t interval) {
  uint64_t curr;
  int ret = sensor_axis_sensor_get_curr(dev, times, interval, &curr);
  if (ret < 0) {
    return ret;
  }

  struct sensor_value val;
  sensor_value_from_micro(&val, curr);
  sensor_axis_sensor_max_set(dev, &val);

  return 0;
}

int sensor_axis_sensor_center_set_curr(const struct device* dev, int times,
                                       k_timeout_t interval) {
  struct sensor_axis_sensor_data* data = dev->data;

  uint64_t curr;
  int ret = sensor_axis_sensor_get_curr(dev, times, interval, &curr);
  if (ret < 0) {
    return ret;
  }

  k_mutex_lock(&data->lock, K_FOREVER);

  int64_t range = (sensor_value_to_micro(&data->in_max) -
                   sensor_value_to_micro(&data->in_min)) /
                  2;
  sensor_value_from_micro(&data->in_min, curr - range);
  sensor_value_from_micro(&data->in_max, curr + range);

  k_mutex_unlock(&data->lock);

  return 0;
}

int sensor_axis_sensor_range_set_curr(const struct device* dev, int times,
                                      k_timeout_t interval, bool is_min) {
  struct sensor_axis_sensor_data* data = dev->data;

  uint64_t curr;
  int ret = sensor_axis_sensor_get_curr(dev, times, interval, &curr);
  if (ret < 0) {
    return ret;
  }

  k_mutex_lock(&data->lock, K_FOREVER);

  int64_t center = (sensor_value_to_micro(&data->in_min) +
                    sensor_value_to_micro(&data->in_max)) /
                   2;
  int64_t range = is_min ? center - curr : curr - center;
  sensor_value_from_micro(&data->in_min, center - range);
  sensor_value_from_micro(&data->in_max, center + range);

  k_mutex_unlock(&data->lock);

  return 0;
}

int sensor_axis_channel_min_set_curr(const struct device* dev, int times,
                                     k_timeout_t interval) {
  const struct sensor_axis_channel_config* config = dev->config;

  for (int i = 0; i < config->num_sensor; i++) {
    int ret =
        sensor_axis_sensor_min_set_curr(config->sensors[i], times, interval);
    if (ret < 0) {
      return ret;
    }
  }

  return 0;
}

int sensor_axis_channel_max_set_curr(const struct device* dev, int times,
                                     k_timeout_t interval) {
  const struct sensor_axis_channel_config* config = dev->config;

  for (int i = 0; i < config->num_sensor; i++) {
    int ret =
        sensor_axis_sensor_max_set_curr(config->sensors[i], times, interval);
    if (ret < 0) {
      return ret;
    }
  }

  return 0;
}

int sensor_axis_channel_center_set_curr(const struct device* dev, int times,
                                        k_timeout_t interval) {
  const struct sensor_axis_channel_config* config = dev->config;

  for (int i = 0; i < config->num_sensor; i++) {
    int ret =
        sensor_axis_sensor_center_set_curr(config->sensors[i], times, interval);
    if (ret < 0) {
      return ret;
    }
  }

  return 0;
}

int sensor_axis_channel_range_set_curr(const struct device* dev, int times,
                                       k_timeout_t interval, bool is_min) {
  const struct sensor_axis_channel_config* config = dev->config;

  for (int i = 0; i < config->num_sensor; i++) {
    int ret = sensor_axis_sensor_range_set_curr(config->sensors[i], times,
                                                interval, is_min);
    if (ret < 0) {
      return ret;
    }
  }

  return 0;
}

/* static function definition ------------------------------------------------*/
static int sensor_get_raw(const struct device* dev, struct sensor_value* val) {
  const struct sensor_axis_sensor_config* config = dev->config;

  int ret;
  ret = sensor_sample_fetch(config->sensor);
  if (ret < 0) {
    return ret;
  }

  ret = sensor_channel_get(config->sensor, config->channel, val);
  return ret;
}

static int sensor_axis_sensor_get_curr(const struct device* dev, int times,
                                       k_timeout_t period, uint64_t* curr) {
  struct sensor_axis_sensor_data* data = dev->data;

  int64_t accum = 0;
  struct sensor_value val;

  for (int i = 0; i < times; i++) {
    k_mutex_lock(&data->lock, K_FOREVER);
    int ret = sensor_get_raw(dev, &val);
    k_mutex_unlock(&data->lock);

    if (ret < 0) {
      LOG_ERR("sensor_get_raw failed: %s", strerror(-ret));
      return ret;
    }

    accum += sensor_value_to_micro(&val);

    if (i < times - 1) {
      k_sleep(period);
    }
  }

  *curr = DIV_ROUND_CLOSEST(accum, times);

  return 0;
}

/**
 * @brief Update sensor error.
 *
 * @param[in] dev The sensor to update.
 * @param[in] error New error code to update.
 * @param[in] ret Current return value.
 * @param[in] info Additional information to report.
 *
 * @retval Original @p ret if success.
 * @retval others Negative error number if `input_report` fails.
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
      LOG_ERR("Sensor %s: underlying %s errors: %s", dev->name,
              config->sensor->name, strerror(-ret));
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
              DIV_ROUND_CLOSEST(*val, 10000) - 100);
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
  ret = k_mutex_lock(&data->lock, WAIT_PERIOD);
  if (ret < 0) {
    return sensor_error_update(dev, INPUT_ERROR_BUSY, ret, NULL);
  }

  struct sensor_value sensor_val;
  ret = sensor_get_raw(dev, &sensor_val);
  if (ret < 0) {
    k_mutex_unlock(&data->lock);
    return sensor_error_update(dev, INPUT_ERROR_IO, ret, NULL);
  }

  int64_t val = sensor_value_to_micro(&sensor_val);
  int64_t min = sensor_value_to_micro(&data->in_min);
  int64_t max = sensor_value_to_micro(&data->in_max);

  k_mutex_unlock(&data->lock);

  if (data->cb != NULL) {
    data->cb(dev, &sensor_val, data->user_data);
  }

  // devide range first to prevent overflow
  int64_t range = DIV_ROUND_CLOSEST(max - min, 1000);
  val = DIV_ROUND_CLOSEST((val - min) * 1000, range);

  if (config->range_tolerance >= 0) {
    int32_t tolerance = config->range_tolerance;
    if (data->error == INPUT_ERROR_UNDER || data->error == INPUT_ERROR_OVER) {
      tolerance -= config->range_tolerance * RANGE_HYSTERESIS;
    }

    if (val < -tolerance * 10000) {
      return sensor_error_update(dev, INPUT_ERROR_UNDER, -EINVAL, &val);

    } else if (val > 1000000 + tolerance * 10000) {
      return sensor_error_update(dev, INPUT_ERROR_OVER, -EINVAL, &val);
    }
  }

  val = CLAMP(val, 0, 1000000);

  ret = sensor_error_update(dev, INPUT_ERROR_NONE, 0, NULL);
  if (ret < 0) {
    return ret;
  }

  *_val = val;
  return 0;
}

/**
 * @brief Update channel error considering the time tolerance.
 *
 * @param[in] dev The channel to update.
 * @param[in] error New error code to update.
 * @param[in] ret Current return value.
 * @param[in] info Additional information to report.
 *
 * @retval Original @p ret if success.
 * @retval -EAGAIN If during accmulating error time decay, the error is still
 * set.
 * @retval others Negative error number if `input_report` fails.
 */
static int channel_error_update(const struct device* dev, uint16_t error,
                                int ret, const void* info) {
  struct sensor_axis_channel_data* data = dev->data;
  const struct sensor_axis_channel_config* config = dev->config;

  int ret2;
  if (error != INPUT_ERROR_NONE && data->error == INPUT_ERROR_NONE) {
    data->accum_error_time = MIN(data->accum_error_time + config->poll_period,
                                 config->time_tolerance);
    LOG_DBG("Channel %s accumulated error time increases to %d ms", dev->name,
            data->accum_error_time);

    if (data->accum_error_time >= config->time_tolerance) {
      ret2 = input_report(dev, INPUT_EV_ERROR, error, true, true, K_NO_WAIT);
      if (ret2 < 0) {
        goto err2;
      }

      data->prev_out = INVALID_OUT;

    } else {
      return ret;
    }

  } else if (error == INPUT_ERROR_NONE && data->error != INPUT_ERROR_NONE) {
    data->accum_error_time =
        MAX(data->accum_error_time -
                config->time_tolerance * config->time_tolerance_decay / 100,
            0);
    LOG_DBG("Channel %s accumulated error time decreases to %d ms", dev->name,
            data->accum_error_time);

    if (data->accum_error_time == 0) {
      ret2 = input_report(dev, INPUT_EV_ERROR, error, false, true, K_NO_WAIT);
      if (ret2 < 0) {
        goto err2;
      }

    } else {
      return -EAGAIN;
    }

  } else {
    return ret;
  }

  data->error = error;

  switch (error) {
    case INPUT_ERROR_NONE:
      LOG_INF("Channel %s error clears", dev->name);
      break;

    case INPUT_ERROR_IO: {
      const char* sensor_name = info;
      LOG_ERR("Channel %s: underlying sensor %s errors: %s", dev->name,
              sensor_name, strerror(-ret));
      break;
    }

    case INPUT_ERROR_NODEV: {
      const char* sensor_name = info;
      LOG_ERR("Sensor %s not ready, channel %s is disabled", sensor_name,
              dev->name);
      break;
    }

    case INPUT_ERROR_DEV: {
      const int32_t* val = info;
      LOG_ERR("Channel %s deviates by %d%%", dev->name,
              DIV_ROUND_CLOSEST(*val, 10000));
      break;
    }
  }

  return ret;

err2:
  LOG_ERR("input_report failed: %s", strerror(-ret2));
  return ret2;
}

static int32_t out_deadzone(const struct device* dev, int32_t out) {
  const struct sensor_axis_channel_config* config = dev->config;

  if (config->deadzone_size == 0) {
    return out;
  }

  switch (config->deadzone_mode) {
    case INPUT_DEADZONE_MODE_CENTER:
      if (IN_RANGE(out, 500000 - config->deadzone_size * 10000,
                   500000 + config->deadzone_size * 10000)) {
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
 * @brief Update channel.
 *
 * @param[in] dev The channel to update.
 * @param[in] axis The axis of the channel.
 *
 * @retval 1 If the channel value is updated.
 * @retval 0 If the channel value is not updated.
 * @retval others Negative error number.
 */
static int channel_update(const struct device* dev, uint16_t axis) {
  struct sensor_axis_channel_data* data = dev->data;
  const struct sensor_axis_channel_config* config = dev->config;

  int ret = 0;

  // normalized sensor values in one-millionth part
  int32_t val_accum = 0;
  int32_t val_min = INT32_MAX;
  int32_t val_max = INT32_MIN;
  for (int i = 0; i < config->num_sensor; i++) {
    const struct device* sensor = config->sensors[i];

    if (!device_is_ready(sensor)) {
      ret = channel_error_update(dev, INPUT_ERROR_NODEV, -ENODEV, sensor->name);

      // Using continue here instead of break to ensure all sensors are checked
      // and their raw data and errors are reported.
      continue;
    }

    int32_t val;
    int ret2 = sensor_get(sensor, &val);
    if (ret2 < 0) {
      ret = channel_error_update(dev, INPUT_ERROR_IO, ret2, sensor->name);
      continue;
    }

    val_accum += val * config->weights[i];
    val_min = MIN(val_min, val);
    val_max = MAX(val_max, val);
  }

  if (ret < 0) {
    return ret;
  }

  int32_t val_dev = val_max - val_min;
  if (config->dev_tolerance >= 0) {
    int tolerance = config->dev_tolerance;
    if (data->error == INPUT_ERROR_DEV) {
      tolerance -= config->dev_tolerance * RANGE_HYSTERESIS;
    }

    if (val_dev > tolerance * 10000) {
      return channel_error_update(dev, INPUT_ERROR_DEV, -EINVAL, &val_dev);

    } else {
      LOG_DBG("Channel %s deviates by %d%%", dev->name,
              DIV_ROUND_CLOSEST(val_dev, 10000));
    }
  }

  ret = channel_error_update(dev, INPUT_ERROR_NONE, 0, NULL);
  if (ret < 0) {
    return ret;
  }

  int32_t out =
      out_deadzone(dev, DIV_ROUND_CLOSEST(val_accum, config->total_weight));
  if (data->prev_out != INVALID_OUT &&
      abs(out - data->prev_out) < config->noise) {
    return 0;
  }

  // devide out first to prevent overflow
  int32_t _out = DIV_ROUND_CLOSEST(out, 1000);
  _out = DIV_ROUND_CLOSEST(_out * (config->out_max - config->out_min), 1000) +
         config->out_min;

  ret = input_report_abs(dev, axis, _out, true, K_NO_WAIT);
  if (ret < 0) {
    LOG_ERR("input_report failed: %s", strerror(-ret));
    return ret;
  }

  data->prev_out = out;

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
      channel_update(config->channels[i], config->axises[i]);
    }

    k_timer_status_sync(&data->timer);
  }
}

static int sensor_axis_sensor_init(const struct device* dev) {
  struct sensor_axis_sensor_data* data = dev->data;
  const struct sensor_axis_sensor_config* config = dev->config;

  k_mutex_init(&data->lock);

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
  snprintf(thread_name, sizeof(thread_name), "sensor_axis_%s", dev->name);
  k_thread_name_set(&data->thread, thread_name);
#endif  // CONFIG_THREAD_NAME

  k_timer_start(&data->timer, K_MSEC(config->poll_period),
                K_MSEC(config->poll_period));

  return 0;
}

#define SENSOR_AXIS_SENSOR_DEFINE(node_id)                                     \
  BUILD_ASSERT(DT_PROP_LEN(node_id, in_min) == 2,                              \
               "length of min-value should be two");                           \
  BUILD_ASSERT(DT_PROP_LEN(node_id, in_max) == 2,                              \
               "length of max-value should be two");                           \
                                                                               \
  static struct sensor_axis_sensor_data node_id##_data = {                     \
      .error = INPUT_ERROR_NONE,                                               \
      .in_min = DT_PROP(node_id, in_min),                                      \
      .in_max = DT_PROP(node_id, in_max),                                      \
      .cb = NULL,                                                              \
  };                                                                           \
                                                                               \
  const static struct sensor_axis_sensor_config node_id##_config = {           \
      .sensor = DEVICE_DT_GET(DT_PHANDLE(node_id, sensor)),                    \
      .channel = DT_PROP(node_id, zephyr_channel),                             \
      .range_tolerance = DT_PROP(node_id, range_tolerance),                    \
  };                                                                           \
                                                                               \
  DEVICE_DT_DEFINE(node_id, sensor_axis_sensor_init, NULL, &node_id##_data,    \
                   &node_id##_config, POST_KERNEL, CONFIG_INPUT_INIT_PRIORITY, \
                   NULL)

#define _SENSOR_AXIS_SENSOR_WEIGHT(node_id) DT_PROP(node_id, weight)

// `DT_FOREACH_CHILD_STATUS_OKAY_SEP` does not work within the same call to
// itself, define a new one with the same definition instead
#define _DT_FOREACH_CHILD_STATUS_OKAY_SEP(node_id, fn, sep) \
  DT_CAT(node_id, _FOREACH_CHILD_STATUS_OKAY_SEP)(fn, sep)

#define SENSOR_AXIS_CHANNEL_DEFINE(node_id)                                  \
  DT_CAT(node_id, _FOREACH_CHILD_STATUS_OKAY_SEP)(SENSOR_AXIS_SENSOR_DEFINE, \
                                                  (;));                      \
                                                                             \
  static struct sensor_axis_channel_data node_id##_data = {                  \
      .error = INPUT_ERROR_NONE,                                             \
      .prev_out = INVALID_OUT,                                               \
      .accum_error_time = 0,                                                 \
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
      .poll_period = DT_PROP(DT_PARENT(node_id), poll_period),               \
      .time_tolerance = DT_PROP(node_id, time_tolerance),                    \
      .time_tolerance_decay = DT_PROP(node_id, time_tolerance_decay),        \
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
