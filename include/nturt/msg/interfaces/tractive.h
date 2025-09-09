/**
 * @file
 * @brief Tractive system message type definitions.
 *
 * @author quantumspawner
 * @version 0.0.1
 * @date 2025-07-05
 * @copyright Copyright (c) 2025 NTU Racing Team
 */

#ifndef NTURT_MSG_INTERFACES_TRACTIVE_H_
#define NTURT_MSG_INTERFACES_TRACTIVE_H_

// glibc includes
#include <stdbool.h>
#include <stdint.h>

// project includes
#include "nturt/msg/interfaces/common.h"

/**
 * @defgroup msg_if_tractive Tractive System Message
 * @brief Tractive system message type definitions.
 *
 * @ingroup msg_if
 * @{
 */

/* macro ---------------------------------------------------------------------*/
/// @brief List of tractive system messages.
#define MSG_TRACTIVE_LIST msg_ts_acc, msg_ts_inv

/**
 * @defgroup msg_if_tractive_pri Tractive System Message Printing
 * @brief Tractive system message printing format strings.
 *
 * @ingroup msg_if_pri
 * @{
 */

/// @brief Insert @ref msg_ts_acc printf format string.
#define PRImsg_ts_acc                                          \
  PRImsg_header                                                \
      "\n\r\tstatus: %hhu, voltage (V): %g, current (A): %g, " \
      "temperature (deg C): %g"                                \
      "\n\r\tsoc (%%): %d capacity (mAh): %g"

/**
 * @brief Insert @ref msg_ts_acc arguments to printf format.
 *
 * @param[in] data The accumulator message data.
 */
#define PRImsg_ts_acc_arg(data)                                 \
  PRImsg_header_arg((data).header), (data).status, (data).volt, \
      (data).current, (data).temp, (data).soc, (data).capacity

/// @brief Insert @ref msg_ts_inv printf format string.
#define PRImsg_ts_inv                                                   \
  PRImsg_header "\n\r\tstatus word: %" PRImsg_4wheel_flags              \
                "\n\r\tdc bus voltage (V): %" PRImsg_4wheel_data        \
                "\n\r\tdc bus current (A): %" PRImsg_4wheel_data        \
                "\n\r\tmos temperature (deg C): %" PRImsg_4wheel_data   \
                "\n\r\tmotor temperature (deg C): %" PRImsg_4wheel_data \
                "\n\r\tmcu temperature (deg C): %" PRImsg_4wheel_data

/**
 * @brief Insert @ref msg_ts_inv arguments to printf format.
 *
 * @param[in] data The inverter message data.
 */
#define PRImsg_ts_inv_arg(data)                                             \
  PRImsg_header_arg((data).header), PRImsg_4wheel_flags_arg((data).status), \
      PRImsg_4wheel_data_arg((data).volt),                                  \
      PRImsg_4wheel_data_arg((data).current),                               \
      PRImsg_4wheel_data_arg((data).inv_temp),                              \
      PRImsg_4wheel_data_arg((data).motor_temp),                            \
      PRImsg_4wheel_data_arg((data).mcu_temp)

/**
 * @} // msg_if_tractive_pri
 */

/**
 * @defgroup msg_if_csv_tractive Tractive System Message CSV
 * @brief Tractive system message CSV format strings.
 *
 * @ingroup msg_if_csv
 * @{
 */

/// @brief CSV header for @ref msg_ts_acc.
#define CSV_PRImsg_ts_acc_header \
  CSV_PRImsg_header_header ",status,volt,current,temp,soc,capacity"

/// @brief Insert @ref msg_ts_acc CSV format string.
#define CSV_PRImsg_ts_acc CSV_PRImsg_header ",%hhu,%f,%f,%f,%d,%f"

/**
 * @brief Insert @ref msg_ts_acc arguments to CSV print format.
 *
 * @param[in] data The accumulator message data.
 */
#define CSV_PRImsg_ts_acc_arg(data)                                 \
  CSV_PRImsg_header_arg((data).header), (data).status, (data).volt, \
      (data).current, (data).temp, (data).soc, (data).capacity

/// @brief CSV header for @ref msg_ts_inv.
#define CSV_PRImsg_ts_inv_header \
  CSV_PRImsg_header_header "," \
      CSV_PRImsg_4wheel_flags_header(status) ","    \
      CSV_PRImsg_4wheel_data_header(volt) ","       \
      CSV_PRImsg_4wheel_data_header(current) ","    \
      CSV_PRImsg_4wheel_data_header(inv_temp) ","   \
      CSV_PRImsg_4wheel_data_header(motor_temp) "," \
      CSV_PRImsg_4wheel_data_header(mcu_temp)

/// @brief Insert @ref msg_ts_inv CSV format string.
#define CSV_PRImsg_ts_inv                                                    \
  CSV_PRImsg_header ",%" CSV_PRImsg_4wheel_flags ",%" CSV_PRImsg_4wheel_data \
                    ",%" CSV_PRImsg_4wheel_data ",%" CSV_PRImsg_4wheel_data  \
                    ",%" CSV_PRImsg_4wheel_data ",%" CSV_PRImsg_4wheel_data

/**
 * @brief Insert @ref msg_ts_inv arguments to CSV print format.
 *
 * @param[in] data The inverter message data.
 */
#define CSV_PRImsg_ts_inv_arg(data)                  \
  CSV_PRImsg_header_arg((data).header),              \
      CSV_PRImsg_4wheel_flags_arg((data).status),    \
      CSV_PRImsg_4wheel_data_arg((data).volt),       \
      CSV_PRImsg_4wheel_data_arg((data).current),    \
      CSV_PRImsg_4wheel_data_arg((data).inv_temp),   \
      CSV_PRImsg_4wheel_data_arg((data).motor_temp), \
      CSV_PRImsg_4wheel_data_arg((data).mcu_temp)

/**
 * @} // msg_if_csv_tractive
 */

/* type
   ----------------------------------------------------------------------*/
/// @brief Accumulator message.
struct msg_ts_acc {
  /** Message header. */
  struct msg_header header;

  /** Accumulator status. */
  uint8_t status;

  /** Accumulator voltage. Unit: V. */
  double volt;

  /** Accumulator current. Unit: A. */
  double current;

  /** Accumulator temperature. Unit: °C. */
  double temp;

  /** Accumulator state of charge (SOC). Unit: %. */
  int soc;

  /** Accumulator capacity. Unit: mAh. */
  double capacity;
};

/// @brief Inverter message.
struct msg_ts_inv {
  /** Message header. */
  struct msg_header header;

  /** Inverter status word. */
  union msg_4wheel_flags status;

  /** DC bus voltage. Unit: V. */
  union msg_4wheel_data volt;

  /** DC bus current. Unit: A. */
  union msg_4wheel_data current;

  /** Inverter MOS temperature. Unit: °C. */
  union msg_4wheel_data inv_temp;

  /** Motor temperature. Unit: °C. */
  union msg_4wheel_data motor_temp;

  /** MCU temperature. Unit: °C. */
  union msg_4wheel_data mcu_temp;
};

/**
 * @} // msg_if_tractive
 */

#endif  // NTURT_MSG_INTERFACES_TRACTIVE_H_
