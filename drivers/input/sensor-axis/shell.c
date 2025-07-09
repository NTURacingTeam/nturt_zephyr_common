#define DT_DRV_COMPAT sensor_axis

#include <zephyr/drivers/input/sensor_axis.h>

// zephyr includes
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/util.h>

/* static function declaration -----------------------------------------------*/
static int sensor_axis_set_cmd_handler(const struct shell* sh, size_t argc,
                                       char** argv, void* data);

/* static variable -----------------------------------------------------------*/
#define __DEVICE_GET(node_id) \
  DT_CAT(node_id, _FOREACH_CHILD_STATUS_OKAY_SEP)(DEVICE_DT_GET, (, ))

#define _DEVICE_GET(inst) \
  DT_INST_FOREACH_CHILD_STATUS_OKAY_SEP(inst, __DEVICE_GET, (, ))

const struct device* devices[] = {DT_INST_FOREACH_STATUS_OKAY(_DEVICE_GET)};

SHELL_STATIC_SUBCMD_SET_CREATE(sensor_axis_set,
                               SHELL_CMD(set_min, NULL, "", NULL),
                               SHELL_CMD(set_max, NULL, "", NULL),
                               SHELL_SUBCMD_SET_END);

#define ___SUBCMD(node_id)                                                 \
  SHELL_CMD_ARG(DT_NODE_FULL_NAME_UNQUOTED(node_id), &sensor_axis_set, "", \
                NULL, 1, 0)
#define __SUBCMD(node_id) \
  DT_CAT(node_id, _FOREACH_CHILD_STATUS_OKAY_SEP)(___SUBCMD, (, ))
#define _SUBCMD(inst) \
  DT_INST_FOREACH_CHILD_STATUS_OKAY_SEP(inst, __SUBCMD, (, ))

SHELL_STATIC_SUBCMD_SET_CREATE(sensor_axis_cmd,
                               DT_INST_FOREACH_STATUS_OKAY(_SUBCMD),
                               SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(sensor_axis, &sensor_axis_cmd, "Sensor Axis Settings",
                   sensor_axis_set_cmd_handler);

/* static function definition ------------------------------------------------*/
static int sensor_axis_set_cmd_handler(const struct shell* sh, size_t argc,
                                       char** argv, void* data) {
  const struct device* dev = NULL;
  for (int i = 0; i < ARRAY_SIZE(devices); i++) {
    if (!strcmp(argv[1], devices[i]->name)) {
      dev = devices[i];
    }
  }

  if (!dev) {
    shell_error(sh, "Unknown device: %s", argv[1]);
    return -ENOENT;
  }

  if (!strcmp(argv[2], "set_min")) {
    sensor_axis_sensor_min_set_curr(dev, 10, K_MSEC(10));

  } else if (!strcmp(argv[2], "set_max")) {
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
