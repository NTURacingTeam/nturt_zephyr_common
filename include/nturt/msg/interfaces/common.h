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

/**
 * @addtogroup msg_interface
 * @{
 */

/* macro ---------------------------------------------------------------------*/
/// @brief Insert @ref msg_header format string.
#define PRImsg_header "s%llu.%06llu s"

/**
 * @brief Insert @ref msg_header arguments to print format.
 *
 * @param[in] header The message header.
 */
#define PRImsg_header_arg(header)                    \
  "timestamp: ", (header).timestamp_ns / 1000000000, \
      ((header).timestamp_ns % 1000000000) / 1000

/// @brief Insert @ref msg_3d_data format string.
#define PRImsg_3d_data "s(%g, %g, %g)"

/**
 * @brief Insert @ref msg_3d_data arguments to print format.
 *
 * @param[in] data The 3D data.
 */
#define PRImsg_3d_data_arg(data)                          \
  "", (double)(data).values[0], (double)(data).values[1], \
      (double)(data).values[2]

/// @brief Insert @ref msg_4wheel_data format string.
#define PRImsg_4wheel_data "s(%g, %g, %g, %g)"

/**
 * @brief Insert @ref msg_4wheel_data arguments to print format.
 *
 * @param[in] data The 4-wheel data.
 */
#define PRImsg_4wheel_data_arg(data)                      \
  "", (double)(data).values[0], (double)(data).values[1], \
      (double)(data).values[2], (double)(data).values[3]

/// @brief Insert @ref msg_4wheel_flags format string.
#define PRImsg_4wheel_flags "s(0x%X, 0x%X, 0x%X, 0x%X)"

/**
 * @brief Insert @ref msg_4wheel_flags arguments to print format.
 *
 * @param[in] flags The 4-wheel flags.
 */
#define PRImsg_4wheel_flags_arg(flags) \
  "", (flags).values[0], (flags).values[1], (flags).values[2], (flags).values[3]

/* type ----------------------------------------------------------------------*/
/// @brief Message header.
struct msg_header {
  /**
   * Timestamp when the msg is generated as attained by
   * `k_ticks_to_ns_floor64(k_uptime_ticks())`. Same convention as sensor
   * drivers. When `CONFIG_NTURT_RTC` is enabled, should be derived from
   * `clock_gettime()` with `CLOCK_REALTIME`.
   */
  uint64_t timestamp_ns;
};

/// @brief 3D data.
union msg_3d_data {
  /** 3D data in an array. */
  float values[3];

  struct {
    /** X-axis value. */
    float x;

    /** Y-axis value. */
    float y;

    /** Z-axis value. */
    float z;
  };
};

/// @brief 4-wheel data.
union msg_4wheel_data {
  /** 4-wheel data in an array. */
  float values[4];

  struct {
    /** Front left wheel data */
    float fl;

    /** Front right wheel data */
    float fr;

    /** Rear left wheel data */
    float rl;

    /** Rear right wheel data */
    float rr;
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
 * @} // msg_interface
 */

#endif  // NTURT_MSG_INTERFACES_COMMON_H_
