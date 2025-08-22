#ifndef ZEPHYR_DRIVERS_INPUT_SENSOR_AXIS_H_
#define ZEPHYR_DRIVERS_INPUT_SENSOR_AXIS_H_

// glibc includes
#include <stddef.h>
#include <stdint.h>

// zephyr includes
#include <zephyr/device.h>
#include <zephyr/drivers/input/sensor_axis.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>

/* macro ---------------------------------------------------------------------*/
#define DT_DRV_COMPAT sensor_axis

/* type ----------------------------------------------------------------------*/
struct sensor_axis_sensor_data {
  /**
   * Current error value. Not protected by lock since only the sensor-axis
   * thread will update it.
   */
  uint16_t error;

  /** Lock to protect the following members and the underlying sensor. */
  struct k_mutex lock;

  /** Minimum input value. */
  struct sensor_value in_min;

  /** Maximum input value. */
  struct sensor_value in_max;

  /** Callback for raw sensor data. */
  sensor_axis_sensor_raw_cb_t cb;

  /** User data for the callback. */
  void* user_data;
};

struct sensor_axis_sensor_config {
  const struct device* sensor;
  enum sensor_channel channel;
  int32_t range_tolerance;
};

struct sensor_axis_channel_data {
  uint16_t error;
  int32_t prev_out;
  int accum_error_time;
};

struct sensor_axis_channel_config {
  size_t num_sensor;
  const struct device** sensors;
  int total_weight;
  int* weights;
  int32_t out_min;
  int32_t out_max;
  int deadzone_mode;
  int deadzone_size;
  int noise;
  int dev_tolerance;
  int poll_period;
  int time_tolerance;
  int time_tolerance_decay;
};

struct sensor_axis_data {
  struct k_timer timer;
  struct k_thread thread;
  K_KERNEL_STACK_MEMBER(thread_stack, CONFIG_INPUT_SENSOR_AXIS_STACK_SIZE);
};

struct sensor_axis_config {
  size_t num_channel;
  const struct device** channels;
  uint16_t* axises;
  int poll_period;
};

#endif  // ZEPHYR_DRIVERS_INPUT_SENSOR_AXIS_H_
