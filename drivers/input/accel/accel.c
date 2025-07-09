#define DT_DRV_COMPAT nturt_accel

// glibc includes
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// zephyr includes
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/dt-bindings/input/input-error-codes.h>
#include <zephyr/input/input.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(accel, CONFIG_INPUT_LOG_LEVEL);

#define PLAUS_ACTIVATE_TH (25 / 100)
#define PLAUS_DEACTIVATE_TH (5 / 100)
#define INVALID_OUT (INT32_MAX)

enum accel_device_state {
  INIT,
  NORMAL,
  ERR,
};

struct accel_data {
  enum accel_device_state accel_state;
  enum accel_device_state brake_state;

  int32_t accel_value;
  bool brake_actuated;
  bool plaus_failed;
  uint16_t error;

  int32_t prev_out;
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

static int error_update(const struct device* dev, uint16_t error) {
  const struct accel_config* config = dev->config;
  struct accel_data* data = dev->data;

  int ret;
  if (data->error == INPUT_ERROR_NONE && error != INPUT_ERROR_NONE) {
    if (data->prev_out != 0) {
      ret = input_report_abs(dev, INPUT_ABS_THROTTLE, 0, true, K_NO_WAIT);
      if (ret < 0) {
        goto err;
      }
    }

    data->prev_out = INVALID_OUT;

    ret = input_report(dev, INPUT_EV_ERROR, error, true, true, K_NO_WAIT);
    if (ret < 0) {
      goto err;
    }

  } else if (data->error != INPUT_ERROR_NONE && error == INPUT_ERROR_NONE) {
    ret =
        input_report(dev, INPUT_EV_ERROR, data->error, false, true, K_NO_WAIT);
    if (ret < 0) {
      goto err;
    }

  } else {
    return 0;
  }

  data->error = error;

  switch (error) {
    case INPUT_ERROR_NONE:
      LOG_INF("Pedal plausibility device error cleared");
      break;

    case INPUT_ERROR_IO:
      LOG_ERR(
          "Pedal plausibility device underlying %s device error",
          data->accel_state == ERR ? config->accel->name : config->brake->name);
      break;

    case INPUT_ERROR_PEDAL_PLAUS:
      LOG_WRN("Pedal plausibility check failed");
      break;
  }

  return 0;

err:
  LOG_ERR("input_report failed: %s", strerror(-ret));
  return ret;
}

static void update(const struct device* dev) {
  struct accel_data* data = dev->data;
  const struct accel_config* config = dev->config;

  if (data->accel_state == INIT || data->brake_state == INIT) {
    return;

  } else if (data->accel_state == ERR || data->brake_state == ERR) {
    error_update(dev, INPUT_ERROR_IO);
    return;
  }

  int32_t range = config->max - config->min;

  if (data->plaus_failed) {
    if (data->accel_value <= range * PLAUS_DEACTIVATE_TH) {
      data->plaus_failed = false;
    }

  } else {
    if (data->accel_value > range * PLAUS_ACTIVATE_TH && data->brake_actuated) {
      data->plaus_failed = true;
    }
  }

  int ret;
  ret = error_update(
      dev, data->plaus_failed ? INPUT_ERROR_PEDAL_PLAUS : INPUT_ERROR_NONE);
  if (ret < 0) {
    return;
  }

  if (!data->plaus_failed && data->accel_value != data->prev_out) {
    ret = input_report_abs(dev, INPUT_ABS_THROTTLE, data->accel_value, true,
                           K_NO_WAIT);
    if (ret < 0) {
      LOG_ERR("input_report failed: %s", strerror(-ret));
      return;
    }

    data->prev_out = data->accel_value;
  }
}

static void accel_cb(struct input_event* evt, void* user_data) {
  const struct device* dev = (const struct device*)user_data;
  struct accel_data* data = dev->data;

  if (!device_is_ready(dev)) {
    return;
  }

  if (evt->type == INPUT_EV_ABS && evt->code == INPUT_ABS_THROTTLE) {
    data->accel_state = NORMAL;
    data->accel_value = evt->value;

  } else if (evt->type == INPUT_EV_ERROR && evt->code) {
    data->accel_state = ERR;
  }

  update(dev);
}

static void brake_cb(struct input_event* evt, void* user_data) {
  const struct device* dev = (const struct device*)user_data;
  struct accel_data* data = dev->data;

  if (!device_is_ready(dev)) {
    return;
  }

  if (evt->type == INPUT_EV_ABS && evt->code == INPUT_ABS_BRAKE) {
    data->brake_state = NORMAL;
    data->brake_actuated = evt->value > 0;

  } else if (evt->type == INPUT_EV_ERROR && evt->code) {
    data->brake_state = ERR;
  }

  update(dev);
}

#define ACCEL_INIT(inst)                                                       \
  static struct accel_data accel_data_##inst = {                               \
      .accel_state = INIT,                                                     \
      .brake_state = INIT,                                                     \
      .plaus_failed = false,                                                   \
      .error = INPUT_ERROR_NONE,                                               \
      .prev_out = INVALID_OUT,                                                 \
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
