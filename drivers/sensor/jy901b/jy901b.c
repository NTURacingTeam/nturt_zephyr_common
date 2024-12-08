#include <zephyr/drivers/sensor/jy901b.h>

#define DT_DRV_COMPAT witmotion_jy901b

// zephyr include
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>

LOG_MODULE_REGISTER(jy901b, CONFIG_SENSOR_LOG_LEVEL);

struct jy901b_data {
  int16_t accel_x;
  int16_t accel_y;
  int16_t accel_z;

  int16_t gyro_x;
  int16_t gyro_y;
  int16_t gyro_z;

  int16_t magn_x;
  int16_t magn_y;
  int16_t magn_z;

  int16_t roll;
  int16_t pitch;
  int16_t yaw;

  int16_t quat_w;
  int16_t quat_x;
  int16_t quat_y;
  int16_t quat_z;

  int16_t temp;
  int32_t press;
};

struct jy901b_config {
  const struct i2c_dt_spec i2c;
};

static void jy901b_convert_accel(struct sensor_value *val, int16_t raw) {
  int64_t conv;

  conv = ((int64_t)raw * SENSOR_G * JY901B_ACCEL_SCALE) >> JY901B_DATA_SHIFT;
  val->val1 = conv / 1000000;
  val->val2 = conv % 1000000;
}

static void jy901b_convert_gyro(struct sensor_value *val, int16_t raw) {
  int64_t conv;

  conv =
      ((int64_t)raw * SENSOR_PI * JY901B_GYRO_SCALE / 180) >> JY901B_DATA_SHIFT;
  val->val1 = conv / 1000000;
  val->val2 = conv % 1000000;
}

static void jy901b_convert_magn(struct sensor_value *val, int16_t raw) {
  int64_t conv;

  conv = (int64_t)raw * JY901B_MAGN_SCALE;
  val->val1 = conv / 1000000;
  val->val2 = conv % 1000000;
}

static void jy901b_convert_deg(struct sensor_value *val, int16_t raw) {
  int64_t conv;

  conv = ((int64_t)raw * 1000000 * JY901B_DEG_SCALE) >> JY901B_DATA_SHIFT;
  val->val1 = conv / 1000000;
  val->val2 = conv % 1000000;
}

static void jy901b_convert_quat(struct sensor_value *val, int16_t raw) {
  int64_t conv;

  conv = ((int64_t)raw * 1000000) >> JY901B_DATA_SHIFT;
  val->val1 = conv / 1000000;
  val->val2 = conv % 1000000;
}

static void jy901b_convert_temp(struct sensor_value *val, int16_t raw) {
  int64_t conv;

  conv = (int64_t)raw * 1000000 * JY901B_TEMP_SCALE;
  val->val1 = conv / 1000000;
  val->val2 = conv % 1000000;
}

static void jy901b_convert_press(struct sensor_value *val, int32_t raw) {
  int64_t conv;

  conv = (int64_t)raw * 1000000 * JY901B_PRESS_SCALE;
  val->val1 = conv / 1000000;
  val->val2 = conv % 1000000;
}

static int jy901b_sample_fetch(const struct device *dev,
                               enum sensor_channel chan) {
  const struct jy901b_config *config = (struct jy901b_config *)dev->config;
  struct jy901b_data *data = dev->data;

  int16_t buf[33];

  // data
  if (i2c_burst_read_dt(&config->i2c, JY901B_DATA_REG_START, (uint8_t *)buf,
                        33 * 2) < 0) {
    LOG_ERR("Failed to fetch data sample.");
    return -EIO;
  }

  // note: jy901b is in little endian format
  data->accel_x = sys_le16_to_cpu(buf[0]);
  data->accel_y = sys_le16_to_cpu(buf[1]);
  // for some reason, z axis is inverted
  data->accel_z = -sys_le16_to_cpu(buf[2]);

  data->gyro_x = sys_le16_to_cpu(buf[3]);
  data->gyro_y = sys_le16_to_cpu(buf[4]);
  data->gyro_z = sys_le16_to_cpu(buf[5]);

  data->magn_x = sys_le16_to_cpu(buf[6]);
  data->magn_y = sys_le16_to_cpu(buf[7]);
  data->magn_z = sys_le16_to_cpu(buf[8]);

  data->roll = sys_le16_to_cpu(buf[9]);
  data->pitch = sys_le16_to_cpu(buf[10]);
  data->yaw = sys_le16_to_cpu(buf[11]);

  data->temp = sys_le16_to_cpu(buf[12]);

  // pressure
  uint32_t press;
  if (i2c_burst_read_dt(&config->i2c, JY901B_PRESSURE_REG, (uint8_t *)&press,
                        JY901B_PRESSURE_LENGTH) < 0) {
    LOG_ERR("Failed to fetch data sample.");
    return -EIO;
  }

  data->press = sys_le32_to_cpu(press);

  // quaternion
  if (i2c_burst_read_dt(&config->i2c, JY901B_QUAT_REG, (uint8_t *)buf,
                        JY901B_QUAT_LENGTH) < 0) {
    LOG_ERR("Failed to fetch data sample.");
    return -EIO;
  }

  data->quat_w = sys_le16_to_cpu(buf[0]);
  data->quat_x = sys_le16_to_cpu(buf[1]);
  data->quat_y = sys_le16_to_cpu(buf[2]);
  data->quat_z = sys_le16_to_cpu(buf[3]);

  return 0;
}

static int jy901b_channel_get(const struct device *dev,
                              enum sensor_channel chan,
                              struct sensor_value *val) {
  struct jy901b_data *data = dev->data;

  // cast to int to prevent compilation warning
  switch ((int)chan) {
    case SENSOR_CHAN_ACCEL_XYZ:
      jy901b_convert_accel(val, data->accel_x);
      jy901b_convert_accel(val + 1, data->accel_y);
      jy901b_convert_accel(val + 2, data->accel_z);
      break;

    case SENSOR_CHAN_ACCEL_X:
      jy901b_convert_accel(val, data->accel_x);
      break;

    case SENSOR_CHAN_ACCEL_Y:
      jy901b_convert_accel(val, data->accel_y);
      break;

    case SENSOR_CHAN_ACCEL_Z:
      jy901b_convert_accel(val, data->accel_z);
      break;

    case SENSOR_CHAN_GYRO_XYZ:
      jy901b_convert_gyro(val, data->gyro_x);
      jy901b_convert_gyro(val + 1, data->gyro_y);
      jy901b_convert_gyro(val + 2, data->gyro_z);
      break;

    case SENSOR_CHAN_GYRO_X:
      jy901b_convert_gyro(val, data->gyro_x);
      break;

    case SENSOR_CHAN_GYRO_Y:
      jy901b_convert_gyro(val, data->gyro_y);
      break;

    case SENSOR_CHAN_GYRO_Z:
      jy901b_convert_gyro(val, data->gyro_z);
      break;

    case SENSOR_CHAN_MAGN_XYZ:
      jy901b_convert_magn(val, data->magn_x);
      jy901b_convert_magn(val + 1, data->magn_y);
      jy901b_convert_magn(val + 2, data->magn_z);
      break;

    case SENSOR_CHAN_MAGN_X:
      jy901b_convert_magn(val, data->magn_x);
      break;

    case SENSOR_CHAN_MAGN_Y:
      jy901b_convert_magn(val, data->magn_y);
      break;

    case SENSOR_CHAN_MAGN_Z:
      jy901b_convert_magn(val, data->magn_z);
      break;

    case SENSOR_CHAN_ORIEN_RPY:
      jy901b_convert_deg(val, data->roll);
      jy901b_convert_deg(val + 1, data->pitch);
      jy901b_convert_deg(val + 2, data->yaw);
      break;

    case SENSOR_CHAN_ORIEN_ROLL:
      jy901b_convert_deg(val, data->roll);
      break;

    case SENSOR_CHAN_ORIEN_PITCH:
      jy901b_convert_deg(val, data->pitch);
      break;

    case SENSOR_CHAN_ORIEN_YAW:
      jy901b_convert_deg(val, data->yaw);
      break;

    case SENSOR_CHAN_QUAT_WXYZ:
      jy901b_convert_quat(val, data->quat_w);
      jy901b_convert_quat(val + 1, data->quat_x);
      jy901b_convert_quat(val + 2, data->quat_y);
      jy901b_convert_quat(val + 3, data->quat_z);
      break;

    case SENSOR_CHAN_QUAT_W:
      jy901b_convert_quat(val, data->quat_w);
      break;

    case SENSOR_CHAN_QUAT_X:
      jy901b_convert_quat(val, data->quat_x);
      break;

    case SENSOR_CHAN_QUAT_Y:
      jy901b_convert_quat(val, data->quat_y);
      break;

    case SENSOR_CHAN_QUAT_Z:
      jy901b_convert_quat(val, data->quat_z);
      break;

    case SENSOR_CHAN_DIE_TEMP:
      jy901b_convert_temp(val, data->temp);
      break;

    case SENSOR_CHAN_PRESS:
      jy901b_convert_press(val, data->press);
      break;

    default:
      return -ENOTSUP;
  }

  return 0;
}

static const struct sensor_driver_api jy901b_api = {
    .sample_fetch = jy901b_sample_fetch,
    .channel_get = jy901b_channel_get,
};

static int jy901b_init(const struct device *dev) {
  const struct jy901b_config *config = dev->config;

  int ret;

  if (!device_is_ready(config->i2c.bus)) {
    LOG_ERR("I2C bus is not ready.");
    return -ENODEV;
  }

  uint8_t id;
  if (i2c_reg_read_byte_dt(&config->i2c, JY901B_I2C_ADDR_REG, &id) < 0) {
    LOG_ERR("Failed to read chip address.");
    return -EIO;
  }

  if (id != config->i2c.addr) {
    LOG_ERR("Invalid chip address.");
    return -EINVAL;
  }

  uint16_t cmd;
  cmd = sys_cpu_to_le16(JY901B_WRITE_ENABLE_KEY);
  if (i2c_burst_write_dt(&config->i2c, JY901B_WRITE_ENABLE_REG, (uint8_t *)&cmd,
                         JY901B_WRITE_ENABLE_LENGTH) < 0) {
    LOG_ERR("Failed to unlock chip write.");
    return -EIO;
  }

#ifdef CONFIG_JY901B_ORIEN_ABSLOUTE
  cmd = sys_cpu_to_le16(JY901B_ORIEN_MODE_ABSOLUTE);
#elif defined(CONFIG_JY901B_ORIEN_RELATIVE)
  cmd = sys_cpu_to_le16(JY901B_ORIEN_MODE_RELATIVE);
#endif  // CONFIG_JY901B_ORIEN_ABSLOUTE

  if (i2c_burst_write_dt(&config->i2c, JY901B_ORIEN_MODE_REG, (uint8_t *)&cmd,
                         JY901B_ORIEN_MODE_LENGTH) < 0) {
    LOG_ERR("Failed to set chip orientation mode.");
    return -EIO;
  }

  cmd = sys_cpu_to_le16(JY901B_SAVE_SETTING);
  if (i2c_burst_write_dt(&config->i2c, JY901B_SAVE_REG, (uint8_t *)&cmd,
                         JY901B_SAVE_LENGTH) < 0) {
    LOG_ERR("Failed to save chip setting.");
    return -EIO;
  }

  return 0;
}

#define JY901B_INIT(inst)                                             \
  static struct jy901b_data jy901b_data_##inst;                       \
                                                                      \
  static const struct jy901b_config jy901b_config_##inst = {          \
      .i2c = I2C_DT_SPEC_INST_GET(inst),                              \
  };                                                                  \
                                                                      \
  DEVICE_DT_INST_DEFINE(inst, jy901b_init, NULL, &jy901b_data_##inst, \
                        &jy901b_config_##inst, POST_KERNEL,           \
                        CONFIG_SENSOR_INIT_PRIORITY, &jy901b_api);

DT_INST_FOREACH_STATUS_OKAY(JY901B_INIT)
