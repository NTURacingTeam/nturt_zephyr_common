#include "sensor_axis.h"

// glibc includes
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// zephyr includes
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>
#include <zephyr/sys/__assert.h>

LOG_MODULE_DECLARE(sensor_axis);

/* static function definition ------------------------------------------------*/
static int calib_load(const char* key, size_t len_rd, settings_read_cb read_cb,
                      void* cb_arg);
static int calib_export(int (*export_func)(const char* name, const void* val,
                                           size_t val_len));

/* static variable -----------------------------------------------------------*/
#define _SENSOR_AXIS_GET(inst) DEVICE_DT_GET(DT_DRV_INST(inst)),

static const struct device* axises[] = {
    DT_INST_FOREACH_STATUS_OKAY(_SENSOR_AXIS_GET)};

SETTINGS_STATIC_HANDLER_DEFINE(_SENSOR_AXIS_SETTINGS_ROOT,
                               SENSOR_AXIS_SETTINGS_ROOT, NULL, calib_load,
                               NULL, calib_export);

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

int sensor_axis_channel_calib_save(const struct device* dev) {
  const struct sensor_axis_channel_config* config = dev->config;

  for (int i = 0; i < config->num_sensor; i++) {
    int ret = sensor_axis_sensor_calib_save(config->sensors[i]);
    if (ret < 0) {
      return ret;
    }
  }

  return 0;
}

/* static function definition ------------------------------------------------*/
static int calib_load(const char* key, size_t len, settings_read_cb read_cb,
                      void* cb_arg) {
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

static int calib_export(int (*export_func)(const char* name, const void* val,
                                           size_t val_len)) {
  char path[SETTINGS_MAX_NAME_LEN + 1];

  for (int i = 0; i < ARRAY_SIZE(axises); i++) {
    const struct sensor_axis_config* axis_config = axises[i]->config;

    for (int j = 0; j < axis_config->num_channel; j++) {
      const struct sensor_axis_channel_config* channel_config =
          axis_config->channels[j]->config;

      for (int k = 0; k < channel_config->num_sensor; k++) {
        const struct device* sensor = channel_config->sensors[k];
        struct sensor_axis_sensor_data* sensor_data = sensor->data;

        k_mutex_lock(&sensor_data->lock, K_FOREVER);

        int ret =
            snprintf(path, sizeof(path), SENSOR_AXIS_SETTINGS_ROOT "/%s/in_min",
                     sensor->name);
        __ASSERT(ret < SETTINGS_MAX_NAME_LEN, "Path too long");

        ret = export_func(path, &sensor_data->in_min,
                          sizeof(sensor_data->in_min));
        if (ret < 0) {
          k_mutex_unlock(&sensor_data->lock);
          return ret;
        }

        ret = snprintf(path, sizeof(path),
                       SENSOR_AXIS_SETTINGS_ROOT "/%s/in_max", sensor->name);
        __ASSERT(ret < SETTINGS_MAX_NAME_LEN, "Path too long");

        ret = export_func(path, &sensor_data->in_max,
                          sizeof(sensor_data->in_max));
        if (ret < 0) {
          k_mutex_unlock(&sensor_data->lock);
          return ret;
        }

        k_mutex_unlock(&sensor_data->lock);
      }
    }
  }

  return 0;
}
