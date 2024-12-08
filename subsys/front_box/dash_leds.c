#define DT_DRV_COMPAT nturt_dash_leds

#include "nturt/front_box/dash_leds.h"

// glibc includes
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// zephyr incldues
#include <zephyr/device.h>
#include <zephyr/drivers/led.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

// project includes
#include "nturt/msg.h"

LOG_MODULE_REGISTER(nturt_fb_dash_led, CONFIG_LED_LOG_LEVEL);

#define DASH_LED_BUF_SIZE CONFIG_NTURT_MSG_SDO_CLI_BUF_SIZE
#define DASH_LED_SDO_TIMEOUT K_MSEC(10)

#define DASH_LED_OD_INDEX 0x2120
#define DASH_LED_OD_SUBINDEX 0

struct dash_led_data {
  uint8_t buf[DASH_LED_BUF_SIZE];
  struct sdo_cli_req reqs[DASH_LED_BUF_SIZE];
  atomic_t *full;
};

static void sdo_cli_cb(const struct sdo_cli_req *req, int ret, void *user_data) {
  const struct device *dev = user_data;
  struct dash_led_data *data = dev->data;

  if (ret < 0) {
    LOG_ERR("Failed to send LED command: %s", strerror(-ret));
  }

  atomic_clear_bit(data->full, req - data->reqs);
}

static int dash_led_send_cmd(const struct device *dev, int led,
                             enum dash_led_cmd cmd) {
  struct dash_led_data *data = dev->data;

  struct sdo_cli_req *req = NULL;
  for (int i = 0; i < DASH_LED_BUF_SIZE; i++) {
    if (!atomic_test_and_set_bit(data->full, i)) {
      req = &data->reqs[i];
      break;
    }
  }

  if (req == NULL) {
    LOG_ERR("No available buffer for LED command");
    return -ENOMEM;
  }

  *((dash_led_data_t *)req->data) = DASH_LED_TO_DATA(led, cmd);

  int ret;
  ret = sdo_write(req, DASH_LED_SDO_TIMEOUT, sdo_cli_cb, (void *)dev);
  if (ret < 0) {
    LOG_ERR("Failed to send LED command: %s", strerror(-ret));
    atomic_clear_bit(data->full, req - data->reqs);
    return ret;
  }

  return 0;
}

int dash_led_blink(const struct device *dev, uint32_t led, uint32_t delay_on,
                   uint32_t delay_off) {
  (void)delay_on;
  (void)delay_off;

  return dash_led_send_cmd(dev, led, DASH_LED_CMD_BLINK);
}

static int dash_led_set_brightness(const struct device *dev, uint32_t led,
                                   uint8_t value) {
  return dash_led_send_cmd(dev, led,
                           value > 0 ? DASH_LED_CMD_ON : DASH_LED_CMD_OFF);
}

static int dash_led_on(const struct device *dev, uint32_t led) {
  return dash_led_send_cmd(dev, led, DASH_LED_CMD_ON);
}

static int dash_led_off(const struct device *dev, uint32_t led) {
  return dash_led_send_cmd(dev, led, DASH_LED_CMD_OFF);
}

static const struct led_driver_api dash_led_api = {
    .blink = dash_led_blink,
    .set_brightness = dash_led_set_brightness,
    .on = dash_led_on,
    .off = dash_led_off,
};

#define _REQ_INITIALIZER(i, led_data)   \
  {                                     \
      .node_id = CO_NODE_ID_FB,         \
      .index = DASH_LED_OD_INDEX,       \
      .subindex = DASH_LED_OD_SUBINDEX, \
      .data = &led_data.buf[i],         \
      .size = sizeof(led_data.buf[i]),  \
  }

#define DASH_LED_INIT(inst)                                            \
  ATOMIC_DEFINE(dash_led_full_##inst, DASH_LED_BUF_SIZE);              \
                                                                       \
  struct dash_led_data dash_led_data_##inst = {                        \
      .reqs =                                                          \
          {                                                            \
              LISTIFY(DASH_LED_BUF_SIZE, _REQ_INITIALIZER, (, ),       \
                      dash_led_data_##inst),                           \
          },                                                           \
      .full = dash_led_full_##inst,                                    \
  };                                                                   \
                                                                       \
  DEVICE_DT_INST_DEFINE(inst, NULL, NULL, &dash_led_data_##inst, NULL, \
                        POST_KERNEL, CONFIG_LED_INIT_PRIORITY, &dash_led_api);

DT_INST_FOREACH_STATUS_OKAY(DASH_LED_INIT)
