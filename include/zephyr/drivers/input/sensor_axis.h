#ifndef ZEPHYR_INCLUDE_DRIVERS_INPUT_SENSOR_AXIS_H_
#define ZEPHYR_INCLUDE_DRIVERS_INPUT_SENSOR_AXIS_H_

// zephyr includes
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/util.h>

/**
 * @brief Sensor axis API
 * @defgroup input_sensor_axis Sensor axis API
 * @ingroup input_interface
 * @{
 */

/* type ----------------------------------------------------------------------*/
/**
 * @brief Sensor axis sensor raw callback type.
 *
 * @param[in] dev The device that generated the event.
 * @param[in] val The sensor value.
 * @param[in] user_data User data to pass to the callback.
 */
typedef void (*sensor_axis_sensor_raw_cb_t)(const struct device* dev,
                                            const struct sensor_value* val,
                                            void* user_data);

/* function declaration ------------------------------------------------------*/
/**
 * @brief Set callback function for raw sensor value.
 *
 * @param[in] dev The device to set the callback.
 * @param[in] cb The callback function.
 * @param[in] user_data User data to pass to the callback.
 */
void sensor_axis_sensor_set_raw_cb(const struct device* dev,
                                   sensor_axis_sensor_raw_cb_t cb,
                                   void* user_data);

/**
 * @brief Get sensor setpoint that will report minimum value.
 * 
 * @param[in] dev The sensor to get the setpoint.
 * @param[out] val The sensor value to get.
 */
void sensor_axis_sensor_min_get(const struct device* dev,
                                struct sensor_value* val);

/**
 * @brief Get sensor setpoint that will report maximum value.
 * 
 * @param[in] dev The sensor to get the setpoint.
 * @param[out] val The sensor value to get.
 */
void sensor_axis_sensor_max_get(const struct device* dev,
                                struct sensor_value* val);

/**
 * @brief Set sensor setpoint that will report minimum value.
 * 
 * @param[in] dev The sensor to set the setpoint.
 * @param[in] val The sensor value to set.
 */
void sensor_axis_sensor_min_set(const struct device* dev,
                                const struct sensor_value* val);

/**
 * @brief Set sensor setpoint that will report maximum value.
 * 
 * @param[in] dev The sensor to set the setpoint.
 * @param[in] val The sensor value to set.
 */
void sensor_axis_sensor_max_set(const struct device* dev,
                                const struct sensor_value* val);

/**
 * @brief Use current sensor value as setpoint that will report minimum value.
 * 
 * @param[in] dev The sensor to set the setpoint.
 * @param[in] times The number of times to sample the sensor.
 * @param[in] interval The interval between each sample.
 * 
 * @return 0 If success, negative error number otherwise.
 */
int sensor_axis_sensor_min_set_curr(const struct device* dev, int times,
                                    k_timeout_t interval);

/**
 * @brief Use current sensor value as setpoint that will report maximum value.
 * 
 * @param[in] dev The sensor to set the setpoint.
 * @param[in] times The number of times to sample the sensor.
 * @param[in] interval The interval between each sample.
 * 
 * @return 0 If success, negative error number otherwise.
 */
int sensor_axis_sensor_max_set_curr(const struct device* dev, int times,
                                    k_timeout_t interval);

#ifdef CONFIG_INPUT_SENSOR_AXIS_SETTINGS

/**
 * @brief Save sensor calibration data to settings.
 *
 * @param[in] dev The device to save the calibration data.
 * @return 0 if successful, negative error number otherwise.
 */
int sensor_axis_sensor_calib_save(const struct device* dev);

#endif  // CONFIG_INPUT_SENSOR_AXIS_SETTINGS

/**
 * @}
 */

#endif  // ZEPHYR_INCLUDE_DRIVERS_INPUT_SENSOR_AXIS_H_
