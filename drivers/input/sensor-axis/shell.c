#include "sensor_axis.h"

// glibc includes
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

// zephyr includes
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/util.h>

LOG_MODULE_DECLARE(sensor_axis);

/* macro ---------------------------------------------------------------------*/
#define CALIB_TIMES 10
#define CALIB_INTERVAL K_MSEC(10)

/* static function declaration -----------------------------------------------*/
static void shell_print_sensor_value(const struct shell* sh,
                                     const struct sensor_value* val);

// Shell provides `shell_device_filter` to get devices. However, since the
// channels and sensors here are not regular devices, it is difficult to filter
// them using for example `DEVICE_API_IS`.
const struct device* sensor_axis_shell_channel(size_t idx);
const struct device* sensor_axis_shell_sensor(size_t idx);

static void sensor_axis_channel_list_get_handler(
    size_t idx, struct shell_static_entry* entry);
static void sensor_axis_channel_calib_set_get_handler(
    size_t idx, struct shell_static_entry* entry);
static void sensor_axis_sensor_calib_get_get_handler(
    size_t idx, struct shell_static_entry* entry);
static void sensor_axis_sensor_calib_set_get_handler(
    size_t idx, struct shell_static_entry* entry);
static void sensor_axis_sensor_dump_raw_get_handler(
    size_t idx, struct shell_static_entry* entry);

static int sensor_axis_channel_list_cmd_handler(const struct shell* sh,
                                                size_t argc, char** argv,
                                                void* data);
static int sensor_axis_channel_calib_set_cmd_handler(const struct shell* sh,
                                                     size_t argc, char** argv,
                                                     void* data);
static int sensor_axis_sensor_calib_get_cmd_handler(const struct shell* sh,
                                                    size_t argc, char** argv,
                                                    void* data);
static int sensor_axis_sensor_calib_set_cmd_handler(const struct shell* sh,
                                                    size_t argc, char** argv,
                                                    void* data);
static int sensor_axis_sensor_dump_raw_cmd_handler(const struct shell* sh,
                                                   size_t argc, char** argv,
                                                   void* data);

void sensor_axis_sensor_raw_cb(const struct device* dev,
                               const struct sensor_value* val, void* user_data);

/* static variable -----------------------------------------------------------*/
#define _SENSOR_AXIS_GET(inst) DEVICE_DT_GET(DT_DRV_INST(inst)),

static const struct device* axises[] = {
    DT_INST_FOREACH_STATUS_OKAY(_SENSOR_AXIS_GET)};

SHELL_DYNAMIC_CMD_CREATE(sensor_axis_channel_list_subcmd,
                         sensor_axis_channel_list_get_handler);

SHELL_STATIC_SUBCMD_SET_CREATE(
    sensor_axis_channel_calib_set_cmd,
    SHELL_CMD(curr_as_min, NULL, "Use current position as minimum travel.",
              NULL),
    SHELL_CMD(curr_as_max, NULL, "Use current position as maximum travel.",
              NULL),
    SHELL_CMD(curr_as_center, NULL, "Use current position as center.", NULL),
    SHELL_CMD(curr_as_max_range, NULL, "Use current position as maximum range.",
              NULL),
    SHELL_CMD(curr_as_min_range, NULL, "Use current position as minimum range.",
              NULL),
    SHELL_SUBCMD_SET_END);

SHELL_DYNAMIC_CMD_CREATE(sensor_axis_channel_calib_set_subcmd,
                         sensor_axis_channel_calib_set_get_handler);

SHELL_STATIC_SUBCMD_SET_CREATE(
    sensor_axis_channel_cmd,
    SHELL_CMD_ARG(list, &sensor_axis_channel_list_subcmd,
                  "List underlying sensors of a channel.\n"
                  "Usage: list <channel>",
                  sensor_axis_channel_list_cmd_handler, 2, 0),
    SHELL_CMD_ARG(calib_set, &sensor_axis_channel_calib_set_subcmd,
                  "Set sensor calibration data of a channel.\n"
                  "Usage: calib_set <channel> <curr_as_min|curr_as_max|"
                  "curr_as_center|curr_as_max_range|curr_as_min_range>",
                  sensor_axis_channel_calib_set_cmd_handler, 3, 0),
    SHELL_SUBCMD_SET_END);

SHELL_STATIC_SUBCMD_SET_CREATE(
    sensor_axis_sensor_calib_get_cmd,
    SHELL_CMD(min, NULL, "Sensor value at minimum travel.", NULL),
    SHELL_CMD(max, NULL, "Sensor value at maximum travel.", NULL),
    SHELL_SUBCMD_SET_END);

SHELL_DYNAMIC_CMD_CREATE(sensor_axis_sensor_calib_get_subcmd,
                         sensor_axis_sensor_calib_get_get_handler);

SHELL_STATIC_SUBCMD_SET_CREATE(
    sensor_axis_sensor_calib_set_cmd,
    SHELL_CMD(curr_as_min, NULL, "Use current position as minimum travel.",
              NULL),
    SHELL_CMD(curr_as_max, NULL, "Use current position as maximum travel.",
              NULL),
    SHELL_CMD(curr_as_center, NULL, "Use current position as center.", NULL),
    SHELL_CMD(curr_as_max_range, NULL, "Use current position as maximum range.",
              NULL),
    SHELL_CMD(curr_as_min_range, NULL, "Use current position as minimum range.",
              NULL),
    SHELL_SUBCMD_SET_END);

SHELL_DYNAMIC_CMD_CREATE(sensor_axis_sensor_calib_set_subcmd,
                         sensor_axis_sensor_calib_set_get_handler);

SHELL_STATIC_SUBCMD_SET_CREATE(
    sensor_axis_sensor_dump_raw_cmd,
    SHELL_CMD(on, NULL, "Enable dumping raw sensor output.", NULL),
    SHELL_CMD(off, NULL, "Disable dumping raw sensor output.", NULL),
    SHELL_SUBCMD_SET_END);

SHELL_DYNAMIC_CMD_CREATE(sensor_axis_sensor_dump_raw_subcmd,
                         sensor_axis_sensor_dump_raw_get_handler);

SHELL_STATIC_SUBCMD_SET_CREATE(
    sensor_axis_sensor_cmd,
    SHELL_CMD_ARG(calib_get, &sensor_axis_sensor_calib_get_subcmd,
                  "Get sensor calibration data.\n"
                  "Usage: calib_get <sensor> <min|max>",
                  sensor_axis_sensor_calib_get_cmd_handler, 3, 0),
    SHELL_CMD_ARG(calib_set, &sensor_axis_sensor_calib_set_subcmd,
                  "Set sensor calibration data.\n"
                  "Usage: calib_set <sensor> <curr_as_min|curr_as_max|"
                  "curr_as_center|curr_as_max_range|curr_as_min_range>",
                  sensor_axis_sensor_calib_set_cmd_handler, 3, 0),
    SHELL_CMD_ARG(dump_raw, &sensor_axis_sensor_dump_raw_subcmd,
                  "Enable raw sensor output dumping.\n"
                  "Usage: dump_raw <sensor> <on|off>",
                  sensor_axis_sensor_dump_raw_cmd_handler, 3, 0),
    SHELL_SUBCMD_SET_END);

SHELL_STATIC_SUBCMD_SET_CREATE(sensor_axis_cmd,
                               SHELL_CMD(channel, &sensor_axis_channel_cmd,
                                         "Channel commands.", NULL),
                               SHELL_CMD(sensor, &sensor_axis_sensor_cmd,
                                         "Sensor commands.", NULL),
                               SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(sensor_axis, &sensor_axis_cmd,
                   "Sensor axis commands.\n"
                   "Usage: sensor_axis <sensor|channel> <subcommand>",
                   NULL);

/* static function definition ------------------------------------------------*/
static void shell_print_sensor_value(const struct shell* sh,
                                     const struct sensor_value* val) {
  if (val->val2 >= 0) {
    shell_print(sh, "%d.%06d", val->val1, val->val2);
  } else {
    shell_print(sh, "%d.%06d", val->val1, -val->val2);
  }
}

const struct device* sensor_axis_shell_channel(size_t idx) {
  for (int i = 0; i < ARRAY_SIZE(axises); i++) {
    const struct sensor_axis_config* config = axises[i]->config;

    if (idx < config->num_channel) {
      return config->channels[idx];
    } else {
      idx -= config->num_channel;
    }
  }

  return NULL;
}

const struct device* sensor_axis_shell_sensor(size_t idx) {
  for (int i = 0; i < ARRAY_SIZE(axises); i++) {
    const struct sensor_axis_config* axis_config = axises[i]->config;

    for (int j = 0; j < axis_config->num_channel; j++) {
      const struct sensor_axis_channel_config* channel_config =
          axis_config->channels[j]->config;

      if (idx < channel_config->num_sensor) {
        return channel_config->sensors[idx];
      } else {
        idx -= channel_config->num_sensor;
      }
    }
  }

  return NULL;
}

static void sensor_axis_channel_list_get_handler(
    size_t idx, struct shell_static_entry* entry) {
  const struct device* channel = sensor_axis_shell_channel(idx);
  if (channel == NULL) {
    entry->syntax = NULL;
    return;
  }

  entry->syntax = channel->name;
  entry->handler = NULL;
  entry->subcmd = NULL;
  entry->help = NULL;
}

static void sensor_axis_channel_calib_set_get_handler(
    size_t idx, struct shell_static_entry* entry) {
  const struct device* channel = sensor_axis_shell_channel(idx);
  if (channel == NULL) {
    entry->syntax = NULL;
    return;
  }

  entry->syntax = channel->name;
  entry->handler = NULL;
  entry->subcmd = &sensor_axis_channel_calib_set_cmd;
  entry->help = NULL;
}

static void sensor_axis_sensor_calib_get_get_handler(
    size_t idx, struct shell_static_entry* entry) {
  const struct device* sensor = sensor_axis_shell_sensor(idx);
  if (sensor == NULL) {
    entry->syntax = NULL;
    return;
  }

  entry->syntax = sensor->name;
  entry->handler = NULL;
  entry->subcmd = &sensor_axis_sensor_calib_get_cmd;
  entry->help = NULL;
}

static void sensor_axis_sensor_calib_set_get_handler(
    size_t idx, struct shell_static_entry* entry) {
  const struct device* sensor = sensor_axis_shell_sensor(idx);
  if (sensor == NULL) {
    entry->syntax = NULL;
    return;
  }

  entry->syntax = sensor->name;
  entry->handler = NULL;
  entry->subcmd = &sensor_axis_sensor_calib_set_cmd;
  entry->help = NULL;
}

static void sensor_axis_sensor_dump_raw_get_handler(
    size_t idx, struct shell_static_entry* entry) {
  const struct device* sensor = sensor_axis_shell_sensor(idx);
  if (sensor == NULL) {
    entry->syntax = NULL;
    return;
  }

  entry->syntax = sensor->name;
  entry->handler = NULL;
  entry->subcmd = &sensor_axis_sensor_dump_raw_cmd;
  entry->help = NULL;
}

static int sensor_axis_channel_list_cmd_handler(const struct shell* sh,
                                                size_t argc, char** argv,
                                                void* data) {
  (void)argc;
  (void)data;

  const struct device* dev = shell_device_get_binding(argv[1]);
  if (dev == NULL) {
    shell_error(sh, "Unknown channel: %s", argv[1]);
    return -ENOENT;
  }

  shell_print(sh, "Sensors in %s:", dev->name);

  const struct sensor_axis_channel_config* config = dev->config;
  for (int i = 0; i < config->num_sensor; i++) {
    shell_print(sh, " %s", config->sensors[i]->name);
  }

  shell_print(sh, "\n");

  return 0;
}

static int sensor_axis_channel_calib_set_cmd_handler(const struct shell* sh,
                                                     size_t argc, char** argv,
                                                     void* data) {
  (void)argc;
  (void)data;

  const struct device* dev = shell_device_get_binding(argv[1]);
  if (dev == NULL) {
    shell_error(sh, "Unknown channel: %s", argv[1]);
    return -ENOENT;
  }

  int ret = 0;
  if (!strcmp(argv[2], "curr_as_min")) {
    ret = sensor_axis_channel_min_set_curr(dev, CALIB_TIMES, CALIB_INTERVAL);

  } else if (!strcmp(argv[2], "curr_as_max")) {
    ret = sensor_axis_channel_max_set_curr(dev, CALIB_TIMES, CALIB_INTERVAL);

  } else if (!strcmp(argv[2], "curr_as_center")) {
    ret = sensor_axis_channel_center_set_curr(dev, CALIB_TIMES, CALIB_INTERVAL);

  } else if (!strcmp(argv[2], "curr_as_max_range")) {
    ret = sensor_axis_channel_range_set_curr(dev, CALIB_TIMES, CALIB_INTERVAL,
                                             false);

  } else if (!strcmp(argv[2], "curr_as_min_range")) {
    ret = sensor_axis_channel_range_set_curr(dev, CALIB_TIMES, CALIB_INTERVAL,
                                             true);

  } else {
    shell_error(sh, "Invalid command: %s", argv[2]);
    return -EINVAL;
  }

  if (ret < 0) {
    shell_error(sh, "Calibration failed: %s", strerror(-ret));
    return ret;
  }

  if (IS_ENABLED(CONFIG_INPUT_SENSOR_AXIS_SETTINGS)) {
    ret = sensor_axis_channel_calib_save(dev);
    if (ret < 0) {
      shell_error(sh, "Calibration save failed: %s", strerror(-ret));
    }
  }

  return 0;
}

static int sensor_axis_sensor_calib_get_cmd_handler(const struct shell* sh,
                                                    size_t argc, char** argv,
                                                    void* data) {
  (void)argc;
  (void)data;

  const struct device* dev = shell_device_get_binding(argv[1]);
  if (dev == NULL) {
    shell_error(sh, "Unknown sensor: %s", argv[1]);
    return -ENOENT;
  }

  struct sensor_value val;
  if (!strcmp(argv[2], "min")) {
    sensor_axis_sensor_min_get(dev, &val);

    shell_print(sh, "%s min value: ", dev->name);
    shell_print_sensor_value(sh, &val);
    shell_print(sh, "\n");

  } else if (!strcmp(argv[2], "max")) {
    sensor_axis_sensor_max_get(dev, &val);

    shell_print(sh, "%s max value: ", dev->name);
    shell_print_sensor_value(sh, &val);
    shell_print(sh, "\n");

  } else {
    shell_error(sh, "Invalid command: %s", argv[2]);
    return -EINVAL;
  }

  return 0;
}

static int sensor_axis_sensor_calib_set_cmd_handler(const struct shell* sh,
                                                    size_t argc, char** argv,
                                                    void* data) {
  (void)argc;
  (void)data;

  const struct device* dev = shell_device_get_binding(argv[1]);
  if (dev == NULL) {
    shell_error(sh, "Unknown sensor: %s", argv[1]);
    return -ENOENT;
  }

  int ret = 0;
  if (!strcmp(argv[2], "curr_as_min")) {
    ret = sensor_axis_sensor_min_set_curr(dev, CALIB_TIMES, CALIB_INTERVAL);

  } else if (!strcmp(argv[2], "curr_as_max")) {
    ret = sensor_axis_sensor_max_set_curr(dev, CALIB_TIMES, CALIB_INTERVAL);

  } else if (!strcmp(argv[2], "curr_as_center")) {
    ret = sensor_axis_sensor_center_set_curr(dev, CALIB_TIMES, CALIB_INTERVAL);

  } else if (!strcmp(argv[2], "curr_as_max_range")) {
    ret = sensor_axis_sensor_range_set_curr(dev, CALIB_TIMES, CALIB_INTERVAL,
                                            false);

  } else if (!strcmp(argv[2], "curr_as_min_range")) {
    ret = sensor_axis_sensor_range_set_curr(dev, CALIB_TIMES, CALIB_INTERVAL,
                                            true);

  } else {
    shell_error(sh, "Invalid command: %s", argv[2]);
    return -EINVAL;
  }

  if (ret < 0) {
    shell_error(sh, "Calibration failed: %s", strerror(-ret));
    return ret;
  }

  if (IS_ENABLED(CONFIG_INPUT_SENSOR_AXIS_SETTINGS)) {
    ret = sensor_axis_sensor_calib_save(dev);
    if (ret < 0) {
      shell_error(sh, "Calibration save failed: %s", strerror(-ret));
    }
  }

  return 0;
}

static int sensor_axis_sensor_dump_raw_cmd_handler(const struct shell* sh,
                                                   size_t argc, char** argv,
                                                   void* data) {
  (void)argc;
  (void)data;

  const struct device* dev = shell_device_get_binding(argv[1]);
  if (dev == NULL) {
    shell_error(sh, "Unknown sensor: %s", argv[1]);
    return -ENOENT;
  }

  int ret = 0;
  bool is_on = shell_strtobool(argv[2], 0, &ret);
  if (ret) {
    shell_error(sh, "Invalid command: %s", argv[2]);
    return ret;
  }

  if (is_on) {
    sensor_axis_sensor_set_raw_cb(dev, sensor_axis_sensor_raw_cb, NULL);
  } else {
    sensor_axis_sensor_set_raw_cb(dev, NULL, NULL);
  }

  return 0;
}

void sensor_axis_sensor_raw_cb(const struct device* dev,
                               const struct sensor_value* val,
                               void* user_data) {
  (void)user_data;

  if (val->val2 >= 0) {
    LOG_INF("Raw %s data: %d.%06d", dev->name, val->val1, val->val2);

  } else {
    LOG_INF("Raw %s data: %d.%06d", dev->name, val->val1, -val->val2);
  }
}
