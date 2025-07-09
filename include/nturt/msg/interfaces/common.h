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
