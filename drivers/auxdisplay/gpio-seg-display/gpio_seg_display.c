#define DT_DRV_COMPAT gpio_seg_display

// glibc includea
#include <assert.h>
#include <stdbool.h>
#include <string.h>

// zephyr includes
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/auxdisplay.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(gpio_seg_display, CONFIG_AUXDISPLAY_LOG_LEVEL);

/*
 *    0
 *   ---
 * 5|   |1
 *   -6-
 * 4|   |2
 *   ---  .
 *    3    7
 */

#define BRIGHTNESS_MIN (0)
#define BRIGHTNESS_MAX (100)

#define CHAR_OFF (0)

#define CHAR_0 (BIT(0) | BIT(1) | BIT(2) | BIT(3) | BIT(4) | BIT(5))
#define CHAR_1 (BIT(1) | BIT(2))
#define CHAR_2 (BIT(0) | BIT(1) | BIT(3) | BIT(4) | BIT(6))
#define CHAR_3 (BIT(0) | BIT(1) | BIT(2) | BIT(3) | BIT(6))
#define CHAR_4 (BIT(1) | BIT(2) | BIT(5) | BIT(6))
#define CHAR_5 (BIT(0) | BIT(2) | BIT(3) | BIT(5) | BIT(6))
#define CHAR_6 (BIT(0) | BIT(2) | BIT(3) | BIT(4) | BIT(5) | BIT(6))
#define CHAR_7 (BIT(0) | BIT(1) | BIT(2) | BIT(5))
#define CHAR_8 (BIT(0) | BIT(1) | BIT(2) | BIT(3) | BIT(4) | BIT(5) | BIT(6))
#define CHAR_9 (BIT(0) | BIT(1) | BIT(2) | BIT(3) | BIT(5) | BIT(6))

#define CHAR_A (BIT(0) | BIT(1) | BIT(2) | BIT(4) | BIT(5) | BIT(6))
#define CHAR_C (BIT(0) | BIT(3) | BIT(4) | BIT(5))
#define CHAR_E (BIT(0) | BIT(3) | BIT(4) | BIT(5) | BIT(6))
#define CHAR_F (BIT(0) | BIT(4) | BIT(5) | BIT(6))
#define CHAR_H (BIT(1) | BIT(2) | BIT(4) | BIT(5) | BIT(6))
#define CHAR_L (BIT(3) | BIT(4) | BIT(5))
#define CHAR_P (BIT(0) | BIT(1) | BIT(4) | BIT(5) | BIT(6))
#define CHAR_U (BIT(1) | BIT(2) | BIT(3) | BIT(4) | BIT(5))

#define CHAR_b (BIT(2) | BIT(3) | BIT(4) | BIT(5) | BIT(6))
#define CHAR_d (BIT(1) | BIT(2) | BIT(3) | BIT(4) | BIT(6))
#define CHAR_h (BIT(2) | BIT(4) | BIT(5) | BIT(6))
#define CHAR_i (BIT(2))
#define CHAR_o (BIT(2) | BIT(3) | BIT(4) | BIT(6))
#define CHAR_r (BIT(4) | BIT(6))
#define CHAR_t (BIT(3) | BIT(4) | BIT(5) | BIT(6))
#define CHAR_u (BIT(2) | BIT(3) | BIT(4))

#define CHAR_DASH (BIT(6))
#define CHAR_OVERLINE (BIT(0))
#define CHAR_UNDERSCORE (BIT(3))
#define CHAR_PIPE (BIT(4) | BIT(5))

struct gpio_map {
  const struct device *gpio_port;
  gpio_port_pins_t mask;
  gpio_port_pins_t value;
};

struct seg_gpio {
  int index;
  gpio_pin_t pin;
};

struct gpio_seg_display_data {
  struct gpio_map *gpio_map;
  struct seg_gpio (*gpios)[8];
  int brightness;
};

struct gpio_seg_display_config {
  const struct gpio_dt_spec (*gpios)[8];
  int num_disp;
  bool decimal_point;
  struct pwm_dt_spec brightness_pwm;
  struct auxdisplay_capabilities capabilities;
};

static const uint8_t num_map[] = {
    CHAR_0, CHAR_1, CHAR_2, CHAR_3, CHAR_4,
    CHAR_5, CHAR_6, CHAR_7, CHAR_8, CHAR_9,
};

static int set_gpios(const struct device *dev) {
  struct gpio_seg_display_data *data = dev->data;

  int ret;
  for (int i = 0; data->gpio_map[i].gpio_port != NULL; i++) {
    ret = gpio_port_set_masked(data->gpio_map[i].gpio_port,
                               data->gpio_map[i].mask, data->gpio_map[i].value);
    if (ret < 0) {
      LOG_ERR("Failed to set GPIO port %s: %s",
              data->gpio_map[i].gpio_port->name, strerror(-ret));
      return ret;
    }
  }

  return 0;
}

static int gpio_seg_display_write(const struct device *dev, const uint8_t *chr,
                                  uint16_t len) {
  struct gpio_seg_display_data *data = dev->data;
  const struct gpio_seg_display_config *config = dev->config;

  uint8_t val[config->num_disp];
  for (int i = 0; i < config->num_disp; i++) {
    val[i] = CHAR_OFF;
  }

  int chr_index = 0;
  for (int i = 0; i < len; i++) {
    if (chr_index >= config->num_disp &&
        (!config->decimal_point || chr[i] != '.')) {
      break;

      LOG_WRN("Not enough digits to display. Truncating the rest");
    }

    switch (chr[i]) {
      case ' ':
        val[chr_index++] = CHAR_OFF;
        break;

      case '0' ... '9':
        val[chr_index++] = num_map[chr[i] - '0'];
        break;

      case 'A':
        val[chr_index++] = CHAR_A;
        break;

      case 'C':
        val[chr_index++] = CHAR_C;
        break;

      case 'E':
        val[chr_index++] = CHAR_E;
        break;

      case 'F':
        val[chr_index++] = CHAR_F;
        break;

      case 'H':
        val[chr_index++] = CHAR_H;
        break;

      case 'L':
        val[chr_index++] = CHAR_L;
        break;

      case 'P':
        val[chr_index++] = CHAR_P;
        break;

      case 'U':
        val[chr_index++] = CHAR_U;
        break;

      case 'b':
        val[chr_index++] = CHAR_b;
        break;

      case 'd':
        val[chr_index++] = CHAR_d;
        break;

      case 'h':
        val[chr_index++] = CHAR_h;
        break;

      case 'i':
        val[chr_index++] = CHAR_i;
        break;

      case 'o':
        val[chr_index++] = CHAR_o;
        break;

      case 'r':
        val[chr_index++] = CHAR_r;
        break;

      case 't':
        val[chr_index++] = CHAR_t;
        break;

      case 'u':
        val[chr_index++] = CHAR_u;
        break;

      case '-':
        val[chr_index++] = CHAR_DASH;
        break;

      case '_':
        val[chr_index++] = CHAR_UNDERSCORE;
        break;

      case '|':
        val[chr_index++] = CHAR_PIPE;
        break;

      case '.':
        if (config->decimal_point) {
          val[i == 0 ? chr_index++ : chr_index - 1] |= BIT(7);
        } else {
          LOG_ERR("Decimal point is not supported");
          return -EINVAL;
        }
        break;

      default:
        LOG_ERR("Invalid character: %c", chr[i]);
        return -EINVAL;
    }
  }

  int num_gpio = config->decimal_point ? 8 : 7;
  for (int i = 0; i < config->num_disp; i++) {
    for (int j = 0; j < num_gpio; j++) {
      if (val[i] & BIT(j)) {
        data->gpio_map[data->gpios[i][j].index].value |=
            BIT(data->gpios[i][j].pin);
      } else {
        data->gpio_map[data->gpios[i][j].index].value &=
            ~BIT(data->gpios[i][j].pin);
      }
    }
  }

  return set_gpios(dev);
}

static int gpio_seg_display_on(const struct device *dev) {
  (void)dev;

  return 0;
}

int gpio_seg_display_brightness_get(const struct device *dev,
                                    uint8_t *brightness) {
  struct gpio_seg_display_data *data = dev->data;
  const struct gpio_seg_display_config *config = dev->config;

  if (!config->brightness_pwm.dev) {
    return -ENOTSUP;
  }

  *brightness = data->brightness;
  return 0;
}

int gpio_seg_display_brightness_set(const struct device *dev,
                                    uint8_t brightness) {
  struct gpio_seg_display_data *data = dev->data;
  const struct gpio_seg_display_config *config = dev->config;

  if (!config->brightness_pwm.dev) {
    return -ENOTSUP;
  }

  if (brightness < BRIGHTNESS_MIN || brightness > BRIGHTNESS_MAX) {
    return -EINVAL;
  }

  data->brightness = brightness;
  pwm_set_pulse_dt(&config->brightness_pwm,
                   config->brightness_pwm.period * brightness / BRIGHTNESS_MAX);

  return 0;
}

static int gpio_seg_display_clear(const struct device *dev) {
  const struct gpio_seg_display_config *config = dev->config;
  char blank[config->num_disp + 1];
  memset(blank, ' ', config->num_disp);

  return gpio_seg_display_write(dev, blank, config->num_disp);
}

static int gpio_seg_display_off(const struct device *dev) {
  return gpio_seg_display_clear(dev);
}

static int gpio_seg_display_capabilities_get(
    const struct device *dev, struct auxdisplay_capabilities *capabilities) {
  const struct gpio_seg_display_config *config = dev->config;

  memcpy(capabilities, &config->capabilities,
         sizeof(struct auxdisplay_capabilities));
  return 0;
}

static int gpio_seg_display_is_busy(const struct device *dev) {
  (void)dev;

  return 0;
}

static int gpio_seg_display_init(const struct device *dev) {
  struct gpio_seg_display_data *data = dev->data;
  const struct gpio_seg_display_config *config = dev->config;

  int ret;
  int num_gpio = config->decimal_point ? 8 : 7;

  int gpio_map_index = 0;
  for (int i = 0; i < config->num_disp; i++) {
    for (int j = 0; j < num_gpio; j++) {
      if (!gpio_is_ready_dt(&config->gpios[i][j])) {
        LOG_ERR("GPIO port %s is not ready", config->gpios[i][j].port->name);
        return -ENODEV;
      }

      ret = gpio_pin_configure_dt(&config->gpios[i][j], GPIO_OUTPUT);
      if (ret < 0) {
        LOG_ERR("Failed to configure GPIO port %s: %s",
                config->gpios[i][j].port->name, strerror(-ret));
        return ret;
      }

      data->gpios[i][j].pin = config->gpios[i][j].pin;

      int k = 0;
      while (true) {
        if (data->gpio_map[k].gpio_port == config->gpios[i][j].port) {
          break;
        } else if (k == gpio_map_index) {
          data->gpio_map[k].gpio_port = config->gpios[i][j].port;
          gpio_map_index++;
          break;
        }

        k++;
      }

      data->gpio_map[k].mask |= BIT(data->gpios[i][j].pin);
      data->gpios[i][j].index = k;
    }
  }

  ret = gpio_seg_display_clear(dev);
  if (ret < 0) {
    return ret;
  }

  return 0;
}

static const struct auxdisplay_driver_api gpio_seg_display_api = {
    .display_on = gpio_seg_display_on,
    .display_off = gpio_seg_display_off,
    .capabilities_get = gpio_seg_display_capabilities_get,
    .clear = gpio_seg_display_clear,
    .brightness_get = gpio_seg_display_brightness_get,
    .brightness_set = gpio_seg_display_brightness_set,
    .is_busy = gpio_seg_display_is_busy,
    .write = gpio_seg_display_write,
};

#define VALIDATE_GPIO_NUM(id, decimal_point)                          \
  static_assert(COND_CODE_1(decimal_point,                            \
                            (DT_PROP_LEN(id, segment_gpios) == 8),    \
                            (DT_PROP_LEN(id, segment_gpios) == 7)) && \
                "Invalid number of segment gpios")

#define _DISP_GPIO(i, id) GPIO_DT_SPEC_GET_BY_IDX(id, segment_gpios, i)

#define DISP_GPIO(id)                                                \
  [DT_REG_ADDR(id)] = {                                              \
      LISTIFY(DT_PROP_LEN(id, segment_gpios), _DISP_GPIO, (, ), id), \
  }

#define GPIO_SEG_DISPLAY_INIT(inst)                                            \
  DT_INST_FOREACH_CHILD_SEP_VARGS(inst, VALIDATE_GPIO_NUM, (;),                \
                                  DT_INST_PROP(inst, decimal_point));          \
                                                                               \
  static struct gpio_map                                                       \
      gpio_map_##inst[8 * DT_INST_CHILD_NUM_STATUS_OKAY(inst)];                \
  static struct seg_gpio seg_gpios_##inst[DT_INST_CHILD_NUM_STATUS_OKAY(inst)] \
                                         [8];                                  \
                                                                               \
  static struct gpio_seg_display_data gpio_seg_display_data##inst = {          \
      .gpio_map = gpio_map_##inst,                                             \
      .gpios = seg_gpios_##inst,                                               \
  };                                                                           \
                                                                               \
  static const struct gpio_dt_spec gpios_##inst[][8] = {                       \
      DT_INST_FOREACH_CHILD_STATUS_OKAY_SEP(inst, DISP_GPIO, (, )),            \
  };                                                                           \
                                                                               \
  static const struct gpio_seg_display_config gpio_seg_display_config##inst =  \
      {                                                                        \
          .gpios = gpios_##inst,                                               \
          .num_disp = DT_INST_CHILD_NUM_STATUS_OKAY(inst),                     \
          .decimal_point = DT_INST_PROP(inst, decimal_point),                  \
          .brightness_pwm = PWM_DT_SPEC_INST_GET_OR(inst, {}),                 \
          .capabilities =                                                      \
              {                                                                \
                  .columns = DT_INST_PROP(inst, columns),                      \
                  .rows = DT_INST_PROP(inst, rows),                            \
                  .brightness =                                                \
                      {                                                        \
                          .minimum =                                           \
                              COND_CODE_1(DT_INST_NODE_HAS_PROP(inst, pwms),   \
                                          BRIGHTNESS_MIN,                      \
                                          (AUXDISPLAY_LIGHT_NOT_SUPPORTED)),   \
                          .maximum =                                           \
                              COND_CODE_1(DT_INST_NODE_HAS_PROP(inst, pwms),   \
                                          BRIGHTNESS_MAX,                      \
                                          (AUXDISPLAY_LIGHT_NOT_SUPPORTED)),   \
                      },                                                       \
                  .backlight =                                                 \
                      {                                                        \
                          .minimum = AUXDISPLAY_LIGHT_NOT_SUPPORTED,           \
                          .maximum = AUXDISPLAY_LIGHT_NOT_SUPPORTED,           \
                      },                                                       \
              },                                                               \
  };                                                                           \
                                                                               \
  DEVICE_DT_INST_DEFINE(                                                       \
      inst, &gpio_seg_display_init, NULL, &gpio_seg_display_data##inst,        \
      &gpio_seg_display_config##inst, POST_KERNEL,                             \
      CONFIG_AUXDISPLAY_INIT_PRIORITY, &gpio_seg_display_api);

DT_INST_FOREACH_STATUS_OKAY(GPIO_SEG_DISPLAY_INIT)
