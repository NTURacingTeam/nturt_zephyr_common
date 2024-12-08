#ifndef NTURT_FRONT_BOX_DASH_LEDS_H_
#define NTURT_FRONT_BOX_DASH_LEDS_H_

// std includes
#include <stdint.h>

// zephyr includes
#include <zephyr/device.h>

// libs includes
#include <canopennode.h>

/* macro ---------------------------------------------------------------------*/
#define DASH_LED_TO_DATA(led, cmd) (((cmd) << 6) | (led))
#define DASH_LED_TO_LED(data) ((data) & 0x3F)
#define DASH_LED_TO_CMD(data) ((data) >> 6)

/* type ----------------------------------------------------------------------*/
typedef uint8_t dash_led_data_t;

enum dash_led_cmd {
  DASH_LED_CMD_OFF = 0x00,
  DASH_LED_CMD_ON = 0x01,
  DASH_LED_CMD_BLINK = 0x02,
};

#endif  // NTURT_FRONT_BOX_DASH_LEDS_H_
