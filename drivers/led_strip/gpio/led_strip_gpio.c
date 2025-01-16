#define DT_DRV_COMPAT gpio_led_strip

// glibc includea
#include <assert.h>
#include <stdbool.h>
#include <string.h>

// zephyr includes
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(led_strip_gpio, CONFIG_AUXDISPLAY_LOG_LEVEL);

struct gpio_map {
  const struct device *gpio_port;

  gpio_port_pins_t mask;

  gpio_port_pins_t value;
};

struct led_gpio {
  int index;

  gpio_pin_t pin;
};

struct led_strip_gpio_data {
  struct gpio_map *gpio_map;

  struct led_gpio *red_gpios;

  struct led_gpio *green_gpios;

  struct led_gpio *blue_gpios;
};

struct led_strip_gpio_config {
  const struct gpio_dt_spec *red_gpios;

  const struct gpio_dt_spec *green_gpios;

  const struct gpio_dt_spec *blue_gpios;

  int num_led;
};

static int set_gpios(const struct device *dev) {
  struct led_strip_gpio_data *data = dev->data;

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

static int led_strip_gpio_update_rgb(const struct device *dev,
                                     struct led_rgb *pixels,
                                     size_t num_pixels) {
  struct led_strip_gpio_data *data = dev->data;

  for (int i = 0; i < num_pixels; i++) {
    if (data->red_gpios != NULL) {
      if (pixels[i].r > 0) {
        data->gpio_map[data->red_gpios[i].index].value |=
            BIT(data->red_gpios[i].pin);
      } else {
        data->gpio_map[data->red_gpios[i].index].value &=
            ~BIT(data->red_gpios[i].pin);
      }
    }

    if (data->green_gpios != NULL) {
      if (pixels[i].g > 0) {
        data->gpio_map[data->green_gpios[i].index].value |=
            BIT(data->green_gpios[i].pin);
      } else {
        data->gpio_map[data->green_gpios[i].index].value &=
            ~BIT(data->green_gpios[i].pin);
      }
    }

    if (data->blue_gpios != NULL) {
      if (pixels[i].b > 0) {
        data->gpio_map[data->blue_gpios[i].index].value |=
            BIT(data->blue_gpios[i].pin);
      } else {
        data->gpio_map[data->blue_gpios[i].index].value &=
            ~BIT(data->blue_gpios[i].pin);
      }
    }
  }

  return set_gpios(dev);
}

static size_t led_strip_gpio_length(const struct device *dev) {
  const struct led_strip_gpio_config *config = dev->config;

  return config->num_led;
}

static int led_strip_gpio_init(const struct device *dev) {
  const struct led_strip_gpio_config *config = dev->config;
  struct led_strip_gpio_data *data = dev->data;

  int ret;
  int gpio_map_index = 0;
  struct led_gpio *led_gpios[] = {data->red_gpios, data->green_gpios,
                                  data->blue_gpios};
  const struct gpio_dt_spec *gpios[] = {config->red_gpios, config->green_gpios,
                                        config->blue_gpios};

  for (int i = 0; i < 3; i++) {
    if (gpios[i] == NULL) {
      continue;
    }

    for (int j = 0; j < config->num_led; j++) {
      if (!gpio_is_ready_dt(&gpios[i][j])) {
        LOG_ERR("GPIO port %s is not ready", gpios[i][j].port->name);
        return -ENODEV;
      }

      ret = gpio_pin_configure_dt(&gpios[i][j], GPIO_OUTPUT);
      if (ret < 0) {
        LOG_ERR("Failed to configure GPIO port %s: %s", gpios[i][j].port->name,
                strerror(-ret));
        return ret;
      }

      led_gpios[i][j].pin = gpios[i][j].pin;

      int k = 0;
      while (true) {
        if (data->gpio_map[k].gpio_port == gpios[i][j].port) {
          break;
        } else if (k == gpio_map_index) {
          data->gpio_map[k].gpio_port = gpios[i][j].port;
          gpio_map_index++;
          break;
        }

        k++;
      }

      data->gpio_map[k].mask |= BIT(led_gpios[i][j].pin);
      led_gpios[i][j].index = k;
    }
  }

  return 0;
}

static const struct led_strip_driver_api led_strip_gpio_api = {
    .update_rgb = led_strip_gpio_update_rgb,
    .length = led_strip_gpio_length,
};

#define _COLOR_GPIOS(i, inst, gpio) GPIO_DT_SPEC_INST_GET_BY_IDX(inst, gpio, i)

#define COLOR_GPIOS(inst, gpios)                                             \
  IF_ENABLED(DT_INST_NODE_HAS_PROP(inst, gpios),                             \
             (static_assert(DT_INST_PROP_LEN(inst, gpios) ==                 \
                                DT_INST_PROP(inst, chain_length) &&          \
                            "Invalid number of " #gpios);                    \
              static struct led_gpio                                         \
                  led_##gpios##_##inst[DT_INST_PROP_LEN(inst, gpios)];       \
              const static struct gpio_dt_spec gpios##_##inst[] = {          \
                  LISTIFY(DT_INST_PROP_LEN(inst, gpios), _COLOR_GPIOS, (, ), \
                          inst, gpios),                                      \
              }))

#define LED_STRIP_GPIO_INIT(inst)                                             \
  COLOR_GPIOS(inst, red_gpios);                                               \
  COLOR_GPIOS(inst, green_gpios);                                             \
  COLOR_GPIOS(inst, blue_gpios);                                              \
                                                                              \
  struct gpio_map gpio_map_##inst[(DT_INST_NODE_HAS_PROP(inst, red_gpios) +   \
                                   DT_INST_NODE_HAS_PROP(inst, green_gpios) + \
                                   DT_INST_NODE_HAS_PROP(inst, blue_gpios)) * \
                                  DT_INST_PROP(inst, chain_length)];          \
                                                                              \
  static struct led_strip_gpio_data led_strip_gpio_data_##inst = {            \
      .gpio_map = gpio_map_##inst,                                            \
      .red_gpios = COND_CODE_1(DT_INST_NODE_HAS_PROP(inst, red_gpios),        \
                               (led_red_gpios_##inst), (NULL)),               \
      .green_gpios = COND_CODE_1(DT_INST_NODE_HAS_PROP(inst, green_gpios),    \
                                 (led_green_gpios_##inst), (NULL)),           \
      .blue_gpios = COND_CODE_1(DT_INST_NODE_HAS_PROP(inst, blue_gpios),      \
                                (led_blue_gpios_##inst), (NULL)),             \
  };                                                                          \
                                                                              \
  const static struct led_strip_gpio_config led_strip_gpio_config_##inst = {  \
      .red_gpios = COND_CODE_1(DT_INST_NODE_HAS_PROP(inst, red_gpios),        \
                               (red_gpios_##inst), (NULL)),                   \
      .green_gpios = COND_CODE_1(DT_INST_NODE_HAS_PROP(inst, green_gpios),    \
                                 (red_gpios_##inst), (NULL)),                 \
      .blue_gpios = COND_CODE_1(DT_INST_NODE_HAS_PROP(inst, blue_gpios),      \
                                (red_gpios_##inst), (NULL)),                  \
      .num_led = DT_INST_PROP(inst, chain_length),                            \
  };                                                                          \
                                                                              \
  DEVICE_DT_INST_DEFINE(inst, &led_strip_gpio_init, NULL,                     \
                        &led_strip_gpio_data_##inst,                          \
                        &led_strip_gpio_config_##inst, POST_KERNEL,           \
                        CONFIG_LED_STRIP_INIT_PRIORITY, &led_strip_gpio_api);

DT_INST_FOREACH_STATUS_OKAY(LED_STRIP_GPIO_INIT)
