#define DT_DRV_COMPAT hall_rpm

// glibc includes
#include <stdint.h>
#include <string.h>

// zephyr includes
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(hall_rpm, CONFIG_SENSOR_LOG_LEVEL);

struct hall_rpm_data {
  const struct hall_rpm_config* config;

  struct gpio_callback cb_data;

  struct k_timer timer;

  /// @brief Sampling interval in milliseconds.
  int sampling_int;

  /// @brief Exponential average filter smoothing factor in milli.
  int alpha;

  /// @brief Pulse count since last timer update.
  atomic_t count;

  /// @brief Last cycle time in HW cycles. Counter overflow is handled by
  /// unsigned integer underflow when subtracting.
  uint32_t cycle;

  /// @brief Last calculated RPM in milli RPM.
  int64_t last;

  struct sensor_value val;
};

struct hall_rpm_config {
  const struct gpio_dt_spec gpio;

  int num_tooth;

  /// @brief Exponential average filter time constant in milliseconds.
  int time_const;
};

void hall_rpm_gpio_cb(const struct device* dev, struct gpio_callback* cb,
                      uint32_t pins) {
  ARG_UNUSED(dev);
  ARG_UNUSED(pins);

  struct hall_rpm_data* data = CONTAINER_OF(cb, struct hall_rpm_data, cb_data);

  atomic_inc(&data->count);
}

void hall_rpm_timer_cb(struct k_timer* timer) {
  struct hall_rpm_data* data = CONTAINER_OF(timer, struct hall_rpm_data, timer);

  uint64_t cycle = k_cycle_get_32();
  int64_t raw = (int64_t)atomic_get(&data->count) * 1000 * 60 *
                sys_clock_hw_cycles_per_sec() / (cycle - data->cycle) /
                data->config->num_tooth;

  data->cycle = cycle;
  atomic_set(&data->count, 0);

  data->last = (raw * data->alpha + data->last * (1000 - data->alpha)) / 1000;
}

static int hall_rpm_init(const struct device* dev) {
  struct hall_rpm_data* data = dev->data;
  const struct hall_rpm_config* config = dev->config;

  int ret;
  if (!gpio_is_ready_dt(&config->gpio)) {
    LOG_ERR("GPIO %s is not ready", config->gpio.port->name);
    return -ENODEV;
  }

  ret = gpio_pin_configure_dt(&config->gpio, GPIO_INPUT);
  if (ret < 0) {
    LOG_ERR("Failed to configure GPIO port %s pin %d: %s",
            config->gpio.port->name, config->gpio.pin, strerror(-ret));
    return ret;
  }

  ret = gpio_pin_interrupt_configure_dt(&config->gpio, GPIO_INT_EDGE_TO_ACTIVE);
  if (ret < 0) {
    LOG_ERR("Failed to configure GPIO port %s pin %d interrupt: %s",
            config->gpio.port->name, config->gpio.pin, strerror(-ret));
    return ret;
  }

  gpio_init_callback(&data->cb_data, hall_rpm_gpio_cb, BIT(config->gpio.pin));
  gpio_add_callback_dt(&config->gpio, &data->cb_data);

  k_timer_init(&data->timer, hall_rpm_timer_cb, NULL);
  k_timer_start(&data->timer, K_MSEC(data->sampling_int),
                K_MSEC(data->sampling_int));

  data->cycle = k_cycle_get_32();
  data->alpha = data->sampling_int * 1000 / config->time_const;

  return 0;
}

static int hall_rpm_sample_fetch(const struct device* dev,
                                 enum sensor_channel chan) {
  if (chan != SENSOR_CHAN_ALL && chan != SENSOR_CHAN_RPM) {
    return -ENOTSUP;
  }

  struct hall_rpm_data* data = dev->data;
  sensor_value_from_milli(&data->val, data->last);

  return 0;
}

static int hall_rpm_channel_get(const struct device* dev,
                                enum sensor_channel chan,
                                struct sensor_value* val) {
  if (chan != SENSOR_CHAN_RPM) {
    return -ENOTSUP;
  }

  struct hall_rpm_data* data = dev->data;
  *val = data->val;

  return 0;
}

static int hall_rpm_attr_set(const struct device* dev, enum sensor_channel chan,
                             enum sensor_attribute attr,
                             const struct sensor_value* val) {
  if (attr != SENSOR_ATTR_SAMPLING_FREQUENCY) {
    return -ENOTSUP;
  }

  struct hall_rpm_data* data = dev->data;
  const struct hall_rpm_config* config = dev->config;

  data->sampling_int = 1000000 / sensor_value_to_milli(val);

  k_timer_start(&data->timer, K_MSEC(data->sampling_int),
                K_MSEC(data->sampling_int));

  data->cycle = k_cycle_get_32();
  data->alpha = data->sampling_int * 1000 / config->time_const;

  return 0;
}

static int hall_rpm_attr_get(const struct device* dev, enum sensor_channel chan,
                             enum sensor_attribute attr,
                             struct sensor_value* val) {
  if (attr != SENSOR_ATTR_SAMPLING_FREQUENCY) {
    return -ENOTSUP;
  }

  struct hall_rpm_data* data = dev->data;

  sensor_value_from_milli(val, 1000000 / data->sampling_int);

  return 0;
}

static const struct sensor_driver_api hall_rpm_api = {
    .sample_fetch = hall_rpm_sample_fetch,
    .channel_get = hall_rpm_channel_get,
    .attr_set = hall_rpm_attr_set,
    .attr_get = hall_rpm_attr_get,
};

#define HALL_RPM_INIT(inst)                                                \
  static const struct hall_rpm_config hall_rpm_config_##inst = {           \
      .gpio = GPIO_DT_SPEC_INST_GET_BY_IDX(inst, gpios, 0),                \
      .num_tooth = DT_INST_PROP(inst, num_tooth),                          \
      .time_const = DT_INST_PROP(inst, time_const),                        \
  };                                                                       \
                                                                           \
  static struct hall_rpm_data hall_rpm_data_##inst = {                     \
      .config = &hall_rpm_config_##inst,                                   \
      .sampling_int = 1000 / DT_INST_PROP(inst, sampling_freq),            \
  };                                                                       \
                                                                           \
  DEVICE_DT_INST_DEFINE(inst, &hall_rpm_init, NULL, &hall_rpm_data_##inst, \
                        &hall_rpm_config_##inst, POST_KERNEL,              \
                        CONFIG_SENSOR_INIT_PRIORITY, &hall_rpm_api);

DT_INST_FOREACH_STATUS_OKAY(HALL_RPM_INIT)
