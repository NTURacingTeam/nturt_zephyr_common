#define DT_DRV_COMPAT cui_amt21

#include "zephyr/drivers/sensor/amt21.h"

// glibc includes
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// zephyr include
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(amt21, CONFIG_SENSOR_LOG_LEVEL);

#define AMT21_ANG_RES 14

#define AMT21_CMD_READ_POS 0x0
#define AMT21_CMD_READ_TURNS 0x1
#define AMT21_CMD_EXT 0x2

#define AMT21_EXT_CMD_RESET 0x75
#define AMT21_EXT_CMD_SET_ZERO 0x5E

struct amt21_data {
  struct sensor_value val;
};

struct amt21_config {
  const struct device* uart;

  uint8_t addr;
  bool multi_turn;
  k_timeout_t read_timeout;
};

static int uart_poll_in_timeout(const struct device* dev, uint8_t* c,
                                k_timeout_t timeout) {
  k_timepoint_t end = sys_timepoint_calc(timeout);

  do {
    int ret = uart_poll_in(dev, c);
    if (ret != -1) {
      return ret;
    }
  } while (!sys_timepoint_expired(end));

  return -ETIMEDOUT;
}

static int amt21_read(const struct device* dev, uint8_t cmd, uint16_t* data) {
  const struct amt21_config* config = dev->config;

  int ret;
  uint8_t buf[2];

  uart_poll_out(config->uart, config->addr + cmd);
  for (int j = 0; j < sizeof(buf); j++) {
    ret = uart_poll_in_timeout(config->uart, &buf[j], config->read_timeout);
    if (ret < 0) {
      return ret;
    }
  }

  uint16_t raw = sys_get_le16(buf);

  bool even = false;
  bool odd = false;
  for (int i = 0; i < 8; i++) {
    even ^= (raw >> (2 * i)) & 0x1;
    odd ^= (raw >> (2 * i + 1)) & 0x1;
  }

  if (!(even && odd)) {
    return -EILSEQ;
  }

  *data = raw & 0x3FFF;
  return 0;
}

static int amt21_fetch_once(const struct device* dev,
                            struct sensor_value* val) {
  const struct amt21_config* config = dev->config;

  int ret;
  int16_t turns = 0;
  uint16_t pos;

  if (config->multi_turn) {
    ret = amt21_read(dev, AMT21_CMD_READ_TURNS, &turns);
    if (ret < 0) {
      return ret;
    }

    // sign extend from 14 bits to 16 bits
    if (turns & 0x2000) {
      turns |= 0xC000;
    }
  }

  ret = amt21_read(dev, AMT21_CMD_READ_POS, &pos);
  if (ret < 0) {
    return ret;
  }

  int64_t conv = (int64_t)turns * 1000000 * 360 +
                 (((int64_t)pos * 1000000 * 360) >> AMT21_ANG_RES);
  sensor_value_from_micro(val, conv);

  return 0;
}

static int amt21_init(const struct device* dev) {
  const struct amt21_config* config = dev->config;

  int ret;

  if (!device_is_ready(config->uart)) {
    LOG_ERR("UART bus %s is not ready", config->uart->name);
    return -ENODEV;
  }

  // check if the device is connected
  uint16_t pos;
  for (int i = 0; i < CONFIG_AMT21_READ_TRIES; i++) {
    ret = amt21_read(dev, AMT21_CMD_READ_POS, &pos);
    if (ret == 0 || ret == -EILSEQ) {
      return 0;
    }
  }

  LOG_ERR("Failed to pin AMT21 after %d ties: %s", CONFIG_AMT21_READ_TRIES,
          strerror(-ret));
  return ret;
}

static int amt21_sample_fetch(const struct device* dev,
                              enum sensor_channel chan) {
  if (k_is_in_isr()) {
    return -EWOULDBLOCK;
  }

  if (chan != SENSOR_CHAN_ALL && chan != SENSOR_CHAN_ROTATION) {
    return -ENOTSUP;
  }

  struct amt21_data* data = dev->data;

  int ret = 0;
  int success = 0;
  struct sensor_value val[3];
  for (int i = 0; i < CONFIG_AMT21_READ_TRIES; i++) {
    ret = amt21_fetch_once(dev, &val[success]);
    if (ret == 0 && ++success == 1) {
      data->val = val[0];
      // break;
      return 0;
    }

    k_busy_wait(100);
  }

  // if (success != 3) {
  //   LOG_ERR("%s too few samples: Read failed %d out of %d (requires 3)",
  //           dev->name, CONFIG_AMT21_READ_TRIES - success,
  //           CONFIG_AMT21_READ_TRIES);
  //   return -EIO;
  // }

  // get median of the three
  // if ((sensor_val_cmp(&val[0], &val[1]) > 0) !=
  //     (sensor_val_cmp(&val[0], &val[2]) > 0)) {
  //   data->val = val[0];
  // } else if ((sensor_val_cmp(&val[1], &val[0]) > 0) !=
  //            (sensor_val_cmp(&val[1], &val[2]) > 0)) {
  //   data->val = val[1];
  // } else {
  //   data->val = val[2];
  // }

  LOG_ERR("Failed to fetch %s once out of %d", dev->name,
          CONFIG_AMT21_READ_TRIES);

  return -EIO;
}

static int amt21_channel_get(const struct device* dev, enum sensor_channel chan,
                             struct sensor_value* val) {
  if (chan != SENSOR_CHAN_ROTATION) {
    return -ENOTSUP;
  }

  struct amt21_data* data = dev->data;
  *val = data->val;

  return 0;
}

static int amt21_attribute_set(const struct device* dev,
                               enum sensor_channel chan,
                               enum sensor_attribute attr,
                               const struct sensor_value* val) {
  (void)chan;
  (void)val;

  const struct amt21_config* config = dev->config;

  switch ((int)attr) {
    case SENSOR_ATTR_AMT21_RESET:
      uart_poll_out(config->uart, config->addr + AMT21_CMD_EXT);
      uart_poll_out(config->uart, AMT21_EXT_CMD_RESET);
      break;

    case SENSOR_ATTR_AMT21_SET_ZERO_POS:
      uart_poll_out(config->uart, config->addr + AMT21_CMD_EXT);
      uart_poll_out(config->uart, AMT21_EXT_CMD_SET_ZERO);
      break;

    default:
      return -ENOTSUP;
  }

  return 0;
}

static const struct sensor_driver_api amt21_api = {
    .sample_fetch = amt21_sample_fetch,
    .channel_get = amt21_channel_get,
    .attr_set = amt21_attribute_set,
};

#define AMT21_INIT(inst)                                                    \
  static struct amt21_data amt21_data_##inst = {                            \
      .val = {0},                                                           \
  };                                                                        \
                                                                            \
  static const struct amt21_config amt21_config_##inst = {                  \
      .uart = DEVICE_DT_GET(DT_INST_BUS(inst)),                             \
      .addr = DT_INST_PROP(inst, addr),                                     \
      .multi_turn = DT_INST_PROP(inst, multi_turn),                         \
      .read_timeout =                                                       \
          K_USEC(30 * 1000000 / DT_PROP(DT_INST_BUS(inst), current_speed)), \
  };                                                                        \
                                                                            \
  DEVICE_DT_INST_DEFINE(inst, &amt21_init, NULL, &amt21_data_##inst,        \
                        &amt21_config_##inst, POST_KERNEL,                  \
                        CONFIG_SENSOR_INIT_PRIORITY, &amt21_api);

DT_INST_FOREACH_STATUS_OKAY(AMT21_INIT)
