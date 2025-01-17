#define DT_DRV_COMPAT nturt_accel

// glibc includes
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// zephyr includes
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/dt-bindings/input/input-error-codes.h>
#include <zephyr/input/input.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(accel, CONFIG_INPUT_LOG_LEVEL);

#define PLAUS_ACTIVATE_TH (25 / 100)
#define PLAUS_DEACTIVATE_TH (5 / 100)

struct accel_data {
  bool activated;
  bool brake_actuated;
  uint16_t error;
};

struct accel_config {
  const struct device* accel;
  const struct device* brake;
  int32_t min;
  int32_t max;
};

static int accel_init(const struct device* dev) {
  const struct accel_config* config = dev->config;

  if (!device_is_ready(config->accel)) {
    LOG_ERR("Accelerator %s not ready", config->accel->name);
    return -ENODEV;
  } else if (!device_is_ready(config->brake)) {
    LOG_ERR("Brake %s not ready", config->brake->name);
    return -ENODEV;
  }

  return 0;
}

static void error_update(const struct device* dev, uint16_t error) {
  struct accel_data* data = dev->data;

  int ret;
  if (data->error == INPUT_ERROR_NONE && error != INPUT_ERROR_NONE) {
    ret = input_report(dev, INPUT_EV_ERROR, error, true, true, K_NO_WAIT);

  } else if (data->error != INPUT_ERROR_NONE && error == INPUT_ERROR_NONE) {
    ret =
        input_report(dev, INPUT_EV_ERROR, data->error, false, true, K_NO_WAIT);

  } else {
    return;
  }

  if (ret < 0) {
    LOG_ERR("input_report failed: %s", strerror(-ret));
    return;
  }

  data->error = error;

  switch (error) {
    case INPUT_ERROR_NONE:
      LOG_INF("Pedal plausibility check cleared");
      break;

    case INPUT_ERROR_PEDAL_PLAUS:
      LOG_ERR("Pedal plausibility check failed");
      break;
  }
}

static int32_t update(const struct device* dev, int32_t val) {
  struct accel_data* data = dev->data;
  const struct accel_config* config = dev->config;

  int32_t range = config->max - config->min;

  if (data->activated) {
    if (val < range * PLAUS_DEACTIVATE_TH) {
      data->activated = false;
    }

    val = 0;
  } else {
    if (val > range * PLAUS_ACTIVATE_TH && data->brake_actuated) {
      data->activated = true;
      val = 0;
    }
  }

  error_update(dev,
               data->activated ? INPUT_ERROR_PEDAL_PLAUS : INPUT_ERROR_NONE);

  return val;
}

static void accel_cb(struct input_event* evt, void* user_data) {
  const struct device* dev = (const struct device*)user_data;

  if (!device_is_ready(dev)) {
    return;
  }

  int ret;
  if (evt->type == INPUT_EV_ABS && evt->code == INPUT_ABS_THROTTLE) {
    int32_t val = update(dev, evt->value);

    ret = input_report_abs(dev, evt->code, val, true, K_NO_WAIT);
    if (ret < 0) {
      goto err_input;
    }

  } else if (evt->type == INPUT_EV_ERROR) {
    ret = input_report(dev, evt->type, evt->code, evt->value, true, K_NO_WAIT);
    if (ret < 0) {
      goto err_input;
    }
  }

  return;

err_input:
  LOG_ERR("input_report failed: %s", strerror(-ret));
}

static void brake_cb(struct input_event* evt, void* user_data) {
  const struct device* dev = (const struct device*)user_data;
  struct accel_data* data = dev->data;

  if (!device_is_ready(dev)) {
    return;
  }

  if (evt->type == INPUT_EV_ABS && evt->code == INPUT_ABS_BRAKE) {
    data->brake_actuated = evt->value > 0;
  }
}

#define ACCEL_INIT(inst)                                                       \
  static struct accel_data accel_data_##inst = {                               \
      .brake_actuated = false,                                                 \
      .activated = false,                                                      \
      .error = INPUT_ERROR_NONE,                                               \
  };                                                                           \
                                                                               \
  const static struct accel_config accel_config_##inst = {                     \
      .accel = DEVICE_DT_GET(DT_INST_PHANDLE(inst, accel)),                    \
      .brake = DEVICE_DT_GET(DT_INST_PHANDLE(inst, brake)),                    \
      .min = DT_INST_PROP(inst, min),                                          \
      .max = DT_INST_PROP(inst, max),                                          \
  };                                                                           \
                                                                               \
  DEVICE_DT_INST_DEFINE(inst, accel_init, NULL, &accel_data_##inst,            \
                        &accel_config_##inst, POST_KERNEL,                     \
                        CONFIG_INPUT_INIT_PRIORITY, NULL);                     \
                                                                               \
  INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_INST_PHANDLE(inst, accel)), accel_cb, \
                        (void*)DEVICE_DT_INST_GET(inst));                      \
  INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_INST_PHANDLE(inst, brake)), brake_cb, \
                        (void*)DEVICE_DT_INST_GET(inst));

DT_INST_FOREACH_STATUS_OKAY(ACCEL_INIT)
