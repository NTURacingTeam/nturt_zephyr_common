#define DT_DRV_COMPAT sensor_axis

#include <zephyr/drivers/input/sensor_axis.h>

// glibc includes
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

// zephyr includes
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/util.h>

LOG_MODULE_DECLARE(sensor_axis);

/* static function declaration -----------------------------------------------*/
static const struct device* get_sensor_by_name(const char* name);
static void shell_print_sensor_value(const struct shell* sh,
                                     const struct sensor_value* val);

static void sensor_axis_calib_get_get_handler(size_t idx,
                                              struct shell_static_entry* entry);
static void sensor_axis_calib_set_get_handler(size_t idx,
                                              struct shell_static_entry* entry);
static void sensor_axis_dump_raw_get_handler(size_t idx,
                                             struct shell_static_entry* entry);

static int sensor_axis_calib_get_cmd_handler(const struct shell* sh,
                                             size_t argc, char** argv,
                                             void* data);
static int sensor_axis_calib_set_cmd_handler(const struct shell* sh,
                                             size_t argc, char** argv,
                                             void* data);
static int sensor_axis_dump_raw_cmd_handler(const struct shell* sh, size_t argc,
                                            char** argv, void* data);

void sensor_axis_sensor_raw_cb(const struct device* dev,
                               const struct sensor_value* val, void* user_data);

/* static variable -----------------------------------------------------------*/
#define __SENSOR_GET(node_id) \
  DT_CAT(node_id, _FOREACH_CHILD_STATUS_OKAY_SEP)(DEVICE_DT_GET, (, ))

#define _SENSOR_GET(inst) \
  DT_INST_FOREACH_CHILD_STATUS_OKAY_SEP(inst, __SENSOR_GET, (, ))

const struct device* sensors[] = {DT_INST_FOREACH_STATUS_OKAY(_SENSOR_GET)};

SHELL_STATIC_SUBCMD_SET_CREATE(
    sensor_axis_calib_get_cmd,
    SHELL_CMD(min, NULL, "Sensor value at minimum travel.", NULL),
    SHELL_CMD(max, NULL, "Sensor value at maximum travel.", NULL),
    SHELL_SUBCMD_SET_END);

SHELL_DYNAMIC_CMD_CREATE(sensor_axis_calib_get_subcmd,
                         sensor_axis_calib_get_get_handler);

SHELL_STATIC_SUBCMD_SET_CREATE(
    sensor_axis_calib_set_cmd,
    SHELL_CMD(curr_as_min, NULL, "Use current position as minimum travel.",
              NULL),
    SHELL_CMD(curr_as_max, NULL, "Use current position as maximum travel.",
              NULL),
    SHELL_SUBCMD_SET_END);

SHELL_DYNAMIC_CMD_CREATE(sensor_axis_calib_set_subcmd,
                         sensor_axis_calib_set_get_handler);

SHELL_STATIC_SUBCMD_SET_CREATE(
    sensor_axis_dump_raw_cmd,
    SHELL_CMD(on, NULL, "Dump raw sensor output.", NULL),
    SHELL_CMD(off, NULL, "Disable dumping raw sensor output.", NULL),
    SHELL_SUBCMD_SET_END);

SHELL_DYNAMIC_CMD_CREATE(sensor_axis_dump_raw_subcmd,
                         sensor_axis_dump_raw_get_handler);

SHELL_STATIC_SUBCMD_SET_CREATE(
    sensor_axis_cmd,
    SHELL_CMD_ARG(calib_get, &sensor_axis_calib_get_subcmd,
                  "Get sensor calibration data.\n"
                  "Usage: calib_get <sensor> <min|max>",
                  sensor_axis_calib_get_cmd_handler, 3, 0),
    SHELL_CMD_ARG(calib_set, &sensor_axis_calib_set_subcmd,
                  "Set sensor calibration data.\n"
                  "Usage: calib_set <sensor> <curr_as_min|curr_as_max>",
                  sensor_axis_calib_set_cmd_handler, 3, 0),
    SHELL_CMD_ARG(dump_raw, &sensor_axis_dump_raw_subcmd,
                  "Print raw sensor data.\n"
                  "Usage: raw_print <sensor> <on|off>",
                  sensor_axis_dump_raw_cmd_handler, 3, 0),
    SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(sensor_axis, &sensor_axis_cmd,
                   "Sensor axis sensors commands.\n"
                   "Usage: sensor_axis <subcommand>",
                   NULL);

/* static function definition ------------------------------------------------*/
static const struct device* get_sensor_by_name(const char* name) {
  for (int i = 0; i < ARRAY_SIZE(sensors); i++) {
    if (!strcmp(name, sensors[i]->name)) {
      return sensors[i];
    }
  }

  return NULL;
}

static void shell_print_sensor_value(const struct shell* sh,
                                     const struct sensor_value* val) {
  if (val->val2 >= 0) {
    shell_fprintf(sh, SHELL_INFO, "%d.%06d", val->val1, val->val2);
  } else {
    shell_fprintf(sh, SHELL_INFO, "%d.%06d", val->val1, -val->val2);
  }
}

static void sensor_axis_calib_get_get_handler(
    size_t idx, struct shell_static_entry* entry) {
  if (idx >= ARRAY_SIZE(sensors)) {
    entry->syntax = NULL;
    return;
  }

  entry->syntax = sensors[idx]->name;
  entry->handler = NULL;
  entry->subcmd = &sensor_axis_calib_get_cmd;
  entry->help = NULL;
}

static void sensor_axis_calib_set_get_handler(
    size_t idx, struct shell_static_entry* entry) {
  if (idx >= ARRAY_SIZE(sensors)) {
    entry->syntax = NULL;
    return;
  }

  entry->syntax = sensors[idx]->name;
  entry->handler = NULL;
  entry->subcmd = &sensor_axis_calib_set_cmd;
  entry->help = NULL;
}

static void sensor_axis_dump_raw_get_handler(size_t idx,
                                             struct shell_static_entry* entry) {
  if (idx >= ARRAY_SIZE(sensors)) {
    entry->syntax = NULL;
    return;
  }

  entry->syntax = sensors[idx]->name;
  entry->handler = NULL;
  entry->subcmd = &sensor_axis_dump_raw_cmd;
  entry->help = NULL;
}

static int sensor_axis_calib_get_cmd_handler(const struct shell* sh,
                                             size_t argc, char** argv,
                                             void* data) {
  (void)argc;
  (void)data;

  const struct device* dev = get_sensor_by_name(argv[1]);
  if (dev == NULL) {
    shell_error(sh, "Unknown device: %s", argv[1]);
    return -ENOENT;
  }

  struct sensor_value val;
  if (!strcmp(argv[2], "min")) {
    sensor_axis_sensor_min_get(dev, &val);

    shell_fprintf(sh, SHELL_INFO, "%s min value: ", dev->name);
    shell_print_sensor_value(sh, &val);
    shell_fprintf(sh, SHELL_INFO, "\n");

  } else if (!strcmp(argv[2], "max")) {
    sensor_axis_sensor_max_get(dev, &val);

    shell_fprintf(sh, SHELL_INFO, "%s max value: ", dev->name);
    shell_print_sensor_value(sh, &val);
    shell_fprintf(sh, SHELL_INFO, "\n");

  } else {
    shell_error(sh, "Invalid command: %s", argv[2]);
    return -EINVAL;
  }

  return 0;
}

static int sensor_axis_calib_set_cmd_handler(const struct shell* sh,
                                             size_t argc, char** argv,
                                             void* data) {
  (void)argc;
  (void)data;

  const struct device* dev = get_sensor_by_name(argv[1]);
  if (dev == NULL) {
    shell_error(sh, "Unknown device: %s", argv[1]);
    return -ENOENT;
  }

  if (!strcmp(argv[2], "curr_as_min")) {
    sensor_axis_sensor_min_set_curr(dev, 10, K_MSEC(10));

  } else if (!strcmp(argv[2], "curr_as_max")) {
    sensor_axis_sensor_max_set_curr(dev, 10, K_MSEC(10));

  } else {
    shell_error(sh, "Invalid command: %s", argv[2]);
    return -EINVAL;
  }

  if (IS_ENABLED(CONFIG_INPUT_SENSOR_AXIS_SETTINGS)) {
    sensor_axis_sensor_calib_save(dev);
  }

  return 0;
}

static int sensor_axis_dump_raw_cmd_handler(const struct shell* sh, size_t argc,
                                            char** argv, void* data) {
  (void)argc;
  (void)data;

  const struct device* dev = get_sensor_by_name(argv[1]);
  if (dev == NULL) {
    shell_error(sh, "Unknown device: %s", argv[1]);
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
    sensor_axis_sensor_reset_raw_cb(dev);
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
