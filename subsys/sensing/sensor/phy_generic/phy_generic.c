#define DT_DRV_COMPAT zephyr_sensing_phy_generic

// glibc includes
#include <stdalign.h>
#include <stdbool.h>
#include <string.h>

// zephyr includes
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/rtio/rtio.h>
#include <zephyr/sensing/sensing_sensor.h>

LOG_MODULE_REGISTER(phy_generic_sensor, CONFIG_SENSING_LOG_LEVEL);

/* macro ---------------------------------------------------------------------*/
#define _NUM_TYPE(inst) DT_INST_PROP_LEN(inst, sensor_types)

#define NUM_TYPE \
  (FOR_EACH(_NUM_TYPE, (, ), DT_INST_FOREACH_STATUS_OKAY(IDENTITY)))

/* type ----------------------------------------------------------------------*/
struct phy_generic_config {
  const struct device *underlying_device;
  size_t num_type;
  uint16_t *sensor_types;
  struct rtio_iodev **sensor_iodevs;
  bool fake_interval;
  bool fake_sensitivity;
};

/* static function declaration -----------------------------------------------*/
static void phy_generic_thread(void *arg1, void *arg2, void *arg3);

/* static variable -----------------------------------------------------------*/
/// @brief RTIO for reading sensors.
RTIO_DEFINE_WITH_MEMPOOL(
    phy_generic_sensor_rtio, Z_POW2_CEIL(NUM_TYPE), Z_POW2_CEIL(NUM_TYPE),
    CONFIG_SENSING_PHY_GENERIC_RTIO_BLOCK_SIZE_PER_TYPE *NUM_TYPE, 8,
    alignof(void *));

K_THREAD_DEFINE(SENSING_PHY_GENERIC,
                CONFIG_SENSING_PHY_GENERIC_THREAD_STACK_SIZE,
                phy_generic_thread, NULL, NULL, NULL,
                CONFIG_SENSING_PHY_GENERIC_THREAD_PRIORITY, 0, 0);

/* static function definition ------------------------------------------------*/
static int phy_generic_init(const struct device *dev) {
  const struct phy_generic_config *config = dev->config;

  if (!device_is_ready(config->underlying_device)) {
    LOG_ERR("Underlying device %s is not ready",
            config->underlying_device->name);
    return -ENODEV;
  }

  for (int i = 0; i < config->num_type; i++) {
    struct rtio_iodev *sensor_iodev = config->sensor_iodevs[i];
    struct sensor_read_config *read_config = sensor_iodev->data;
    read_config->channels[0].chan_type =
        sensing_sensor_type_to_chan(config->sensor_types[i]);
  }

  return 0;
}

static int phy_generic_attr_set(const struct device *dev,
                                enum sensor_channel chan,
                                enum sensor_attribute attr,
                                const struct sensor_value *val) {
  const struct phy_generic_config *config = dev->config;

  switch (attr) {
    case SENSOR_ATTR_SAMPLING_FREQUENCY:
      if (config->fake_interval) {
        return 0;
      }

      break;

    case SENSOR_ATTR_HYSTERESIS:
      if (config->fake_sensitivity) {
        return 0;
      }

      break;

    default:
      break;
  }

  return sensor_attr_set(config->underlying_device, chan, attr, val);
}

static void phy_generic_submit(const struct device *dev,
                               struct rtio_iodev_sqe *sqe) {
  const struct phy_generic_config *config = dev->config;
  struct sensing_submit_config *submit_config = sqe->sqe.iodev->data;
  struct rtio_iodev *sensor_iodev =
      config->sensor_iodevs[submit_config->info_index];

  int ret;
  ret = sensor_read_async_mempool(sensor_iodev, &phy_generic_sensor_rtio, sqe);

  if (ret < 0) {
    rtio_iodev_sqe_err(sqe, ret);
  }
}

/**
 * @brief Decode the sensor data from the cqe buffer and copy it to the sqe
 * buffer, then complete the sqe and release the cqe and its buffer.
 *
 * @param dev The generic physical sensor device.
 * @param cqe The completion queue entry from sensor driver read.
 * @param sqe The submission queue entry from sensing sensor submit.
 */
static void phy_generic_decode(const struct device *dev, struct rtio_cqe *cqe,
                               struct rtio_iodev_sqe *sqe) {
  const struct phy_generic_config *config = dev->config;
  const struct device *underlying_device = config->underlying_device;
  struct sensing_submit_config *submit_config = sqe->sqe.iodev->data;
  struct sensor_chan_spec chan = {submit_config->chan, 0};

  int ret;
  if ((ret = cqe->result) < 0) {
    LOG_ERR("Failed to read sensor: %s", strerror(-ret));
    goto out;
  }

  uint8_t *cqe_buf;
  uint32_t cqe_buf_len;
  ret = rtio_cqe_get_mempool_buffer(&phy_generic_sensor_rtio, cqe, &cqe_buf,
                                    &cqe_buf_len);
  if (ret < 0) {
    LOG_ERR("Failed to get mempool buffer: %s", strerror(-ret));
    goto out;
  }

  const struct sensor_decoder_api *decoder;
  ret = sensor_get_decoder(underlying_device, &decoder);
  if (ret < 0) {
    LOG_ERR("Failed to get decoder: %s", strerror(-ret));
    goto out_cqe_buf;
  }

  uint16_t frame_count;
  ret = decoder->get_frame_count(cqe_buf, chan, &frame_count);
  if (ret < 0) {
    LOG_ERR("Failed to get frame count: %s", strerror(-ret));
    goto out_cqe_buf;
  }

  size_t base_size;
  size_t frame_size;
  decoder->get_size_info(chan, &base_size, &frame_size);

  uint8_t *sqe_buf;
  uint32_t sqe_buf_len;
  ret = rtio_sqe_rx_buf(sqe, base_size,
                        base_size + (frame_count - 1) * frame_size, &sqe_buf,
                        &sqe_buf_len);
  if (ret < 0) {
    LOG_ERR(
        "Failed to allocate RTIO buffer, considering increasing "
        "CONFIG_SENSING_PHY_GENERIC_RTIO_BLOCK_SIZE_PER_TYPE");
    goto out_cqe_buf;
  }

  // if the size of the allocated buffer is not multiple of the frame size,
  // the partial part that can't fit a whole frame will not be used since
  // integer devide always rounds down
  frame_count = (sqe_buf_len - base_size) / frame_size + 1;

  uint32_t fit;
  ret = decoder->decode(cqe_buf, chan, &fit, frame_count, sqe_buf);
  if (ret < 0) {
    LOG_ERR("Failed to decode: %s", strerror(-ret));
    goto err_sqe_buf;
  }

out_cqe_buf:
  rtio_release_buffer(&phy_generic_sensor_rtio, cqe_buf, cqe_buf_len);

out:
  rtio_cqe_release(&phy_generic_sensor_rtio, cqe);
  if (ret < 0) {
    rtio_iodev_sqe_err(sqe, ret);
  } else {
    rtio_iodev_sqe_ok(sqe, ret);
  }

err_sqe_buf:
  // buffer for the sqe is always allocated by the RTIO context, since the sqe
  // is submitted by sensor_read_async_mempool from sensing_sensor_polling_timer
  rtio_release_buffer(&sqe->r, sqe_buf, sqe_buf_len);
  goto out_cqe_buf;
}

static void phy_generic_thread(void *arg1, void *arg2, void *arg3) {
  (void)arg1;
  (void)arg2;
  (void)arg3;

  while (true) {
    struct rtio_cqe *cqe = rtio_cqe_consume_block(&phy_generic_sensor_rtio);

    struct rtio_iodev_sqe *sqe = cqe->userdata;
    const struct device *dev =
        ((struct sensing_sensor *)sqe->sqe.userdata)->dev;

    phy_generic_decode(dev, cqe, sqe);
  }
}

static const struct sensor_driver_api phy_generic_api = {
    .attr_set = phy_generic_attr_set,
    .submit = phy_generic_submit,
};

#define _SENSOR_IODEV_NAME(inst, idx) phy_generic_iodev_##inst##_##idx

#define _SENSOR_IODEV_DEFINE(node_id, prop, idx, inst) \
  SENSOR_DT_READ_IODEV(_SENSOR_IODEV_NAME(inst, idx),  \
                       DT_PHANDLE(node_id, underlying_device), {0, 0});

#define _SENSOR_IODEV_PTR(node_id, prop, idx, inst) \
  &_SENSOR_IODEV_NAME(inst, idx)

#define PHY_GENERIC_SENSOR_INIT(inst)                                       \
  DT_INST_FOREACH_PROP_ELEM_VARGS(inst, sensor_types, _SENSOR_IODEV_DEFINE, \
                                  inst);                                    \
                                                                            \
  static const struct phy_generic_config phy_generic_config_##inst = {      \
      .underlying_device =                                                  \
          DEVICE_DT_GET(DT_INST_PHANDLE(inst, underlying_device)),          \
      .num_type = DT_INST_PROP_LEN(inst, sensor_types),                     \
      .sensor_types = (uint16_t[])DT_INST_PROP(inst, sensor_types),         \
      .sensor_iodevs =                                                      \
          (struct rtio_iodev *[]){DT_INST_FOREACH_PROP_ELEM_SEP_VARGS(      \
              inst, sensor_types, _SENSOR_IODEV_PTR, (, ), inst)},          \
      .fake_interval = DT_INST_PROP(inst, fake_interval),                   \
      .fake_sensitivity = DT_INST_PROP(inst, fake_sensitivity),             \
  };                                                                        \
                                                                            \
  SENSING_SENSORS_DT_INST_DEFINE(inst, NULL, NULL, &phy_generic_init, NULL, \
                                 NULL, &phy_generic_config_##inst,          \
                                 POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY,  \
                                 &phy_generic_api);

DT_INST_FOREACH_STATUS_OKAY(PHY_GENERIC_SENSOR_INIT)
