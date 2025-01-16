#define DT_DRV_COMPAT diodes_74hc595

// glibc includes
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// zephyr includes
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/gpio/gpio_utils.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(gpio_74hc595, CONFIG_GPIO_LOG_LEVEL);

#if CONFIG_SPI_INIT_PRIORITY >= CONFIG_GPIO_74HC595_INIT_PRIORITY
#error SPI_INIT_PRIORITY must be lower than GPIO_74HC595_INIT_PRIORITY
#endif

struct gpio_74hc595_data {
  struct k_mutex lock;

  uint8_t* output;

  bool initialized;
};

struct gpio_74hc595_config {
  struct spi_dt_spec spi;

  size_t num_childs;
};

struct gpio_74hc595_child_data {
  /* gpio_driver_data needs to be first */
  struct gpio_driver_data gpio;

  struct gpio_74hc595_data* parent;
};

struct gpio_74hc595_child_config {
  /* gpio_driver_config needs to be first */
  struct gpio_driver_config gpio;

  /* index of the gpio is the reversed order of its register */
  int index;

  const struct gpio_74hc595_config* parent;
};

static int gpio_74hc595_write_output(const struct device* dev, uint8_t output) {
  const struct gpio_74hc595_child_config* config = dev->config;
  struct gpio_74hc595_data* prt_data =
      ((struct gpio_74hc595_child_data*)dev->data)->parent;
  const struct gpio_74hc595_config* prt_config =
      ((struct gpio_74hc595_child_config*)dev->config)->parent;

  if (k_is_in_isr()) {
    return -EWOULDBLOCK;
  }

  int ret = 0;

  // only flush if current output differs from new masked value
  if ((prt_data->output[config->index]) != output) {
    uint8_t orig = prt_data->output[config->index];
    prt_data->output[config->index] = output;

    struct spi_buf tx_buf[] = {
        {.buf = prt_data->output, .len = prt_config->num_childs}};
    const struct spi_buf_set tx = {.buffers = tx_buf, .count = 1};

    ret = spi_write_dt(&prt_config->spi, &tx);
    if (ret < 0) {
      prt_data->output[config->index] = orig;
    }
  }

  return ret;
}

static int gpio_74hc595_config(const struct device* dev, gpio_pin_t pin,
                               gpio_flags_t flags) {
  __ASSERT(flags & GPIO_OUTPUT && !(flags & GPIO_INPUT), "Must be output mode");
  __ASSERT(!(flags & (GPIO_PULL_UP | GPIO_PULL_DOWN)),
           "Bias flags not supported");

  const struct gpio_74hc595_child_config* config = dev->config;
  struct gpio_74hc595_data* prt_data =
      ((struct gpio_74hc595_child_data*)dev->data)->parent;

  k_mutex_lock(&prt_data->lock, K_FOREVER);

  uint8_t output = prt_data->output[config->index];
  if (flags & GPIO_OUTPUT_INIT_HIGH) {
    output |= BIT(pin);
  } else if (flags & GPIO_OUTPUT_INIT_LOW) {
    output &= ~BIT(pin);
  }

  int ret = gpio_74hc595_write_output(dev, output);

  k_mutex_unlock(&prt_data->lock);

  return ret;
}

static int gpio_74hc595_port_get_raw(const struct device* dev,
                                     uint32_t* value) {
  const struct gpio_74hc595_child_config* config = dev->config;
  struct gpio_74hc595_data* prt_data =
      ((struct gpio_74hc595_child_data*)dev->data)->parent;

  k_mutex_lock(&prt_data->lock, K_FOREVER);

  *value = prt_data->output[config->index];

  k_mutex_unlock(&prt_data->lock);
  return 0;
}

static int gpio_74hc595_port_set_masked_raw(const struct device* dev,
                                            uint32_t mask, uint32_t value) {
  const struct gpio_74hc595_child_config* config = dev->config;
  struct gpio_74hc595_data* prt_data =
      ((struct gpio_74hc595_child_data*)dev->data)->parent;

  k_mutex_lock(&prt_data->lock, K_FOREVER);

  int ret = gpio_74hc595_write_output(
      dev, (prt_data->output[config->index] & ~mask) | (mask & value));

  k_mutex_unlock(&prt_data->lock);
  return ret;
}

static int gpio_74hc595_port_set_bits_raw(const struct device* dev,
                                          uint32_t pins) {
  return gpio_74hc595_port_set_masked_raw(dev, pins, pins);
}

static int gpio_74hc595_port_clear_bits_raw(const struct device* dev,
                                            uint32_t pins) {
  return gpio_74hc595_port_set_masked_raw(dev, pins, 0U);
}

static int gpio_74hc595_port_toggle_bits(const struct device* dev,
                                         uint32_t pins) {
  const struct gpio_74hc595_child_config* config = dev->config;
  struct gpio_74hc595_data* prt_data =
      ((struct gpio_74hc595_child_data*)dev->data)->parent;

  k_mutex_lock(&prt_data->lock, K_FOREVER);

  int ret =
      gpio_74hc595_write_output(dev, prt_data->output[config->index] ^ pins);

  k_mutex_unlock(&prt_data->lock);
  return ret;
}

static int gpio_74hc595_init(const struct device* dev) {
  struct gpio_74hc595_data* prt_data =
      ((struct gpio_74hc595_child_data*)dev->data)->parent;
  const struct gpio_74hc595_config* prt_config =
      ((struct gpio_74hc595_child_config*)dev->config)->parent;

  if (prt_data->initialized) {
    return 0;
  }

  prt_data->initialized = true;

  if (!spi_is_ready_dt(&prt_config->spi)) {
    LOG_ERR("SPI bus %s is not ready", prt_config->spi.bus->name);
    return -ENODEV;
  }

  struct spi_buf tx_buf[] = {
      {.buf = prt_data->output, .len = prt_config->num_childs}};
  const struct spi_buf_set tx = {.buffers = tx_buf, .count = 1};
  return spi_write_dt(&prt_config->spi, &tx);
}

static const struct gpio_driver_api gpio_74hc595_api = {
    .pin_configure = gpio_74hc595_config,
    .port_get_raw = gpio_74hc595_port_get_raw,
    .port_set_masked_raw = gpio_74hc595_port_set_masked_raw,
    .port_set_bits_raw = gpio_74hc595_port_set_bits_raw,
    .port_clear_bits_raw = gpio_74hc595_port_clear_bits_raw,
    .port_toggle_bits = gpio_74hc595_port_toggle_bits,
};

#define ZERO_AND_COMMA(n) 0,

#define GPIO_74HC595_CHILD_INIT(id, inst, num)                           \
  static struct gpio_74hc595_child_data gpio_74hc595_child_data_##id = { \
      .parent = &gpio_74hc595_data_##inst,                               \
  };                                                                     \
                                                                         \
  static const struct gpio_74hc595_child_config                          \
      gpio_74hc595_child_config_##id = {                                 \
          .gpio =                                                        \
              {                                                          \
                  .port_pin_mask = GPIO_PORT_PIN_MASK_FROM_DT_NODE(id),  \
              },                                                         \
          .index = num - DT_REG_ADDR(id) - 1,                            \
          .parent = &gpio_74hc595_config_##inst,                         \
  };                                                                     \
                                                                         \
  DEVICE_DT_DEFINE(id, &gpio_74hc595_init, NULL,                         \
                   &gpio_74hc595_child_data_##id,                        \
                   &gpio_74hc595_child_config_##id, POST_KERNEL,         \
                   CONFIG_GPIO_74HC595_INIT_PRIORITY, &gpio_74hc595_api);

#define GPIO_74HC595_INIT(inst)                                                \
  static uint8_t output_##inst[] = {                                           \
      DT_INST_FOREACH_CHILD(inst, ZERO_AND_COMMA)};                            \
                                                                               \
  static struct gpio_74hc595_data gpio_74hc595_data_##inst = {                 \
      .lock = Z_MUTEX_INITIALIZER(gpio_74hc595_data_##inst.lock),              \
      .output = output_##inst,                                                 \
      .initialized = false,                                                    \
  };                                                                           \
                                                                               \
  static const struct gpio_74hc595_config gpio_74hc595_config_##inst = {       \
      .spi = SPI_DT_SPEC_INST_GET(                                             \
          inst, SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8), 0),   \
      .num_childs = sizeof(output_##inst) / sizeof(uint8_t),                   \
  };                                                                           \
                                                                               \
  DT_INST_FOREACH_CHILD_STATUS_OKAY_VARGS(inst, GPIO_74HC595_CHILD_INIT, inst, \
                                          DT_INST_CHILD_NUM_STATUS_OKAY(inst))

DT_INST_FOREACH_STATUS_OKAY(GPIO_74HC595_INIT)
