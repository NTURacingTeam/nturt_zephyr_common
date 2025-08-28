/**
 * @file
 * @brief Common message type definitions.
 *
 * @author quantumspawner
 * @version 0.0.1
 * @date 2025-01-20
 * @copyright Copyright (c) 2025 NTU Racing Team
 */

#ifndef NTURT_MSG_INTERFACES_COMMON_H_
#define NTURT_MSG_INTERFACES_COMMON_H_

// glibc includes
#include <stdint.h>

// zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

/**
 * @addtogroup msg_if
 * @{
 */

/* macro ---------------------------------------------------------------------*/
/**
 * @addtogroup msg_if_pri
 * @{
 */

/// @brief Insert @ref msg_header format string.
#define PRImsg_header "s%llu.%06llu s"

/**
 * @brief Insert @ref msg_header arguments to print format.
 *
 * @param[in] data The message header.
 */
#define PRImsg_header_arg(data)                    \
  "timestamp: ", (data).timestamp_ns / 1000000000, \
      ((data).timestamp_ns % 1000000000) / 1000

/// @brief Insert @ref msg_2d_data format string.
#define PRImsg_2d_data "s(%g, %g)"

/**
 * @brief Insert @ref msg_2d_data arguments to print format.
 *
 * @param[in] data The 2D data.
 */
#define PRImsg_2d_data_arg(data) "", (data).x, (data).y

/// @brief Insert @ref msg_3d_data format string.
#define PRImsg_3d_data "s(%g, %g, %g)"

/**
 * @brief Insert @ref msg_3d_data arguments to print format.
 *
 * @param[in] data The 3D data.
 */
#define PRImsg_3d_data_arg(data) "", (data).x, (data).y, (data).z

/// @brief Insert @ref msg_4d_data format string.
#define PRImsg_4d_data "s(%g, %g, %g, %g)"

/// @brief Insert @ref msg_4d_data arguments to print format.
#define PRImsg_4d_data_arg(data) "", (data).w, (data).x, (data).y, (data).z

/// @brief Insert @ref msg_4wheel_data format string.
#define PRImsg_4wheel_data "s(%g, %g, %g, %g)"

/**
 * @brief Insert @ref msg_4wheel_data arguments to print format.
 *
 * @param[in] data The 4-wheel data.
 */
#define PRImsg_4wheel_data_arg(data) \
  "", (data).fl, (data).fr, (data).rl, (data).rr

/// @brief Insert @ref msg_4wheel_flags format string.
#define PRImsg_4wheel_flags "s(0x%X, 0x%X, 0x%X, 0x%X)"

/**
 * @brief Insert @ref msg_4wheel_flags arguments to print format.
 *
 * @param[in] data The 4-wheel flags.
 */
#define PRImsg_4wheel_flags_arg(data) \
  "", (data).fl, (data).fr, (data).rl, (data).rr

/**
 * @} // msg_if_pri
 */

/**
 * @addtogroup msg_if_csv
 * @{
 */

/// @brief CSV header for @ref msg_header.
#define CSV_PRImsg_header_header "timestamp"

/// @brief Insert @ref msg_header CSV format string.
#define CSV_PRImsg_header "llu.%06llu"

/**
 * @brief Insert @ref msg_header arguments to CSV print format.
 *
 * @param[in] data The message header.
 */
#define CSV_PRImsg_header_arg(data) \
  (data).timestamp_ns / 1000000000, ((data).timestamp_ns % 1000000000) / 1000

/** @brief CSV header for @ref msg_2d_data.
 *
 * @param[in] data The 2D data.
 */
#define CSV_PRImsg_2d_data_header(data) \
  STRINGIFY(data) "_x," STRINGIFY(data) "_y"

/// @brief Insert @ref msg_2d_data CSV format string.
#define CSV_PRImsg_2d_data "f,%f"

/**
 * @brief Insert @ref msg_2d_data arguments to CSV print format.
 *
 * @param[in] data The 2D data.
 */
#define CSV_PRImsg_2d_data_arg(data) (data).x, (data).y

/** @brief CSV header for @ref msg_3d_data.
 *
 * @param[in] data The 3D data.
 */
#define CSV_PRImsg_3d_data_header(data) \
  STRINGIFY(data) "_x," STRINGIFY(data) "_y," STRINGIFY(data) "_z"

/// @brief Insert @ref msg_3d_data CSV format string.
#define CSV_PRImsg_3d_data "f,%f,%f"

/**
 * @brief Insert @ref msg_3d_data arguments to CSV print format.
 *
 * @param[in] data The 3D data.
 */
#define CSV_PRImsg_3d_data_arg(data) (data).x, (data).y, (data).z

/** @brief CSV header for @ref msg_4d_data.
 *
 * @param[in] data The 4D data.
 */
#define CSV_PRImsg_4d_data_header(data) \
  STRINGIFY(data) "_w," STRINGIFY(data) "_x," STRINGIFY(data) "_y," STRINGIFY(data) "_z"

/// @brief Insert @ref msg_4d_data CSV format string.
#define CSV_PRImsg_4d_data "f,%f,%f,%f"

/**
 * @brief Insert @ref msg_4d_data arguments to CSV print format.
 *
 * @param[in] data The 4D data.
 */
#define CSV_PRImsg_4d_data_arg(data) (data).w, (data).x, (data).y, (data).z

/** @brief CSV header for @ref msg_4wheel_data.
 *
 * @param[in] data The 4-wheel data.
 */
#define CSV_PRImsg_4wheel_data_header(data) \
  STRINGIFY(data)                           \
  "_fl," STRINGIFY(data) "_fr," STRINGIFY(data) "_rl," STRINGIFY(data) "_rr"

/// @brief Insert @ref msg_4wheel_data CSV format string.
#define CSV_PRImsg_4wheel_data "f,%f,%f,%f"

/**
 * @brief Insert @ref msg_4wheel_data arguments to CSV print format.
 *
 * @param[in] data The 4-wheel data.
 */
#define CSV_PRImsg_4wheel_data_arg(data) \
  (data).fl, (data).fr, (data).rl, (data).rr

/** @brief CSV header for @ref msg_4wheel_flags.
 *
 * @param[in] data The 4-wheel flags.
 */
#define CSV_PRImsg_4wheel_flags_header(data) \
  STRINGIFY(data)                            \
  "_fl," STRINGIFY(data) "_fr," STRINGIFY(data) "_rl," STRINGIFY(data) "_rr"

/// @brief Insert @ref msg_4wheel_flags CSV format string.
#define CSV_PRImsg_4wheel_flags "u,%u,%u,%u"

/**
 * @brief Insert @ref msg_4wheel_flags arguments to CSV print format.
 *
 * @param[in] data The 4-wheel flags.
 */
#define CSV_PRImsg_4wheel_flags_arg(data) \
  (data).fl, (data).fr, (data).rl, (data).rr

/**
 * @} // msg_if_csv
 */

/* type ----------------------------------------------------------------------*/
/// @brief Message header.
struct msg_header {
  /**
   * Timestamp when the msg is generated as attained by
   * `k_ticks_to_ns_floor64(k_uptime_ticks())`. Same convention as sensor
   * drivers. When `CONFIG_NTURT_RTC` is enabled, should be derived from
   * `sys_clock_gettime()` with `CLOCK_REALTIME`.
   */
  uint64_t timestamp_ns;
};

/// @brief 2D data.
union msg_2d_data {
  /** 2D data in an array. */
  double values[2];

  struct {
    /** X-axis value. */
    double x;

    /** Y-axis value. */
    double y;
  };
};

/// @brief 3D data.
union msg_3d_data {
  /** 3D data in an array. */
  double values[3];

  struct {
    /** X-axis value. */
    double x;

    /** Y-axis value. */
    double y;

    /** Z-axis value. */
    double z;
  };
};

/// @brief 4D data.
union msg_4d_data {
  /** 4D data in an array. */
  double values[4];

  struct {
    /** W-axis value. */
    double w;

    /** X-axis value. */
    double x;

    /** Y-axis value. */
    double y;

    /** Z-axis value. */
    double z;
  };
};

/// @brief 4-wheel data.
union msg_4wheel_data {
  /** 4-wheel data in an array. */
  double values[4];

  struct {
    /** Front left wheel data. */
    double fl;

    /** Front right wheel data. */
    double fr;

    /** Rear left wheel data. */
    double rl;

    /** Rear right wheel data. */
    double rr;
  };
};

/// @brief 4-wheel flags.
union msg_4wheel_flags {
  /** 4-wheel flags in an array. */
  uint32_t values[4];

  struct {
    /** Front left wheel flags */
    uint32_t fl;

    /** Front right wheel flags */
    uint32_t fr;

    /** Rear left wheel flags */
    uint32_t rl;

    /** Rear right wheel flags */
    uint32_t rr;
  };
};

/* function declarations ---------------------------------------------------*/
/**
 * @brief Initialize a message header.
 *
 * @param[out] header Pointer to the message header.
 */
void msg_header_init(struct msg_header *header);

/**
 * @} // msg_if
 */

#endif  // NTURT_MSG_INTERFACES_COMMON_H_
