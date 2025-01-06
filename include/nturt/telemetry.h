/**
 * @file
 * @brief Telemetry module.
 *
 * @author quantumspawner
 * @version 0.0.1
 * @date 2024-12-27
 * @copyright Copyright (c) 2024 NTU Racing Team
 */

#ifndef NTURT_TELEMETRY_H_
#define NTURT_TELEMETRY_H_

// glibc include
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/queue.h>

// zephyr include
#include <zephyr/kernel.h>
#include <zephyr/sys/iterable_sections.h>
#include <zephyr/sys/util.h>

/**
 * @addtogroup tm Telemetry
 *
 * @{
 */

/* macro ---------------------------------------------------------------------*/
#define _TM_DATA_BUF_NAME(name, i) CONCAT(name, _buf, i)

#define _TM_DATA_BUF_DEFINE(name, size)                         \
  static uint8_t __aligned(8) _TM_DATA_BUF_NAME(name, 0)[size], \
      _TM_DATA_BUF_NAME(name, 1)[size]

#define _TM_DATA_DEFINE(_name, _addr, _size)                              \
  _TM_DATA_BUF_DEFINE(_name, _size);                                      \
  STRUCT_SECTION_ITERABLE(tm_data, _name) = {                             \
      .addr = _addr,                                                      \
      .size = _size,                                                      \
      .data = {_TM_DATA_BUF_NAME(_name, 0), _TM_DATA_BUF_NAME(_name, 1)}, \
  }

/**
 * @brief Instantiate telemetry data.
 *
 * @param[in] addr Address of the data.
 * @param[in] size Size of the data.
 *
 * @note Since the name of the data is automatically generated using @p addr ,
 * the name would be illegal if @p addr contains numeric operations. To avoid
 * this, _TM_DATA_DEFINE can be used directly by providing a unique name.
 */
#define TM_DATA_DEFINE(addr, size) \
  _TM_DATA_DEFINE(CONCAT(__tm_data, addr), addr, size)

#define _TM_GROUP_DATA_ADDRS_NAME(name) CONCAT(name, _data_addrs)

#define _TM_GROUP_DEFINE(_name, _id, ...)                                     \
  BUILD_ASSERT(                                                               \
      NUM_VA_ARGS(__VA_ARGS__) < sizeof(((struct tm_group *)0)->updated) * 8, \
      "number of data in telemetry group of id " #_id                         \
      " is greater then the number of bits of tm_group::updated");            \
  static const uint32_t _TM_GROUP_DATA_ADDRS_NAME(_name)[] = {__VA_ARGS__};   \
  STRUCT_SECTION_ITERABLE(tm_group, _name) = {                                \
      .id = _id,                                                              \
      .num_data = NUM_VA_ARGS(__VA_ARGS__),                                   \
      .data_addrs = _TM_GROUP_DATA_ADDRS_NAME(_name),                         \
  }

/**
 * @brief Instantiate a telemetry group.
 *
 * @param[in] id ID of the group.
 * @param[in] ... Addresses of the data in the group.
 *
 * @note Since the name of the data is automatically generated using @p id ,
 * the name would be illegal if @p id contains numeric operations. To avoid
 * this, _TM_DATA_DEFINE can be used directly by providing a unique name.
 */
#define TM_GROUP_DEFINE(id, ...) \
  _TM_GROUP_DEFINE(CONCAT(__tm_group, id), id, __VA_ARGS__)

/**
 * @brief Instantiate a telemetry backend.
 *
 * @param[in] _name Name of the backend.
 * @param[in] _api Pointer to telemetry backend API.
 * @param[in] _data Pointer to custom data of the backend.
 * @param[in] ... Group IDs associated with the backend.
 */
#define TM_BACKEND_DEFINE(_name, _api, _data, ...)                             \
  static const uint32_t _name##_group_ids[] = {__VA_ARGS__};                   \
  static struct tm_backend_list_elm _name##_entries[NUM_VA_ARGS(__VA_ARGS__)]; \
  STRUCT_SECTION_ITERABLE(tm_backend, _name) = {                               \
      .num_group = NUM_VA_ARGS(__VA_ARGS__),                                   \
      .group_ids = _name##_group_ids,                                          \
      .api = _api,                                                             \
      .elements = _name##_entries,                                             \
      .data = _data,                                                           \
  }

/* type ----------------------------------------------------------------------*/
struct tm_backend;
typedef void (*tm_backend_init_t)(struct tm_backend *backend);
typedef void (*tm_backend_publish_t)(struct tm_backend *backend,
                                     const uint32_t group_id);

/**
 * @brief List element for telemetry backend.
 *
 */
struct tm_backend_list_elm {
  /// @brief Pointer to the backend.
  struct tm_backend *backend;

  /// @brief List entry of the element.
  STAILQ_ENTRY(tm_backend_list_elm) next;
};

/// @cond

STAILQ_HEAD(tm_backend_list, tm_backend_list_elm);

/// @endcond

/**
 * @brief Telemetry data.
 *
 */
struct tm_data {
  /// @brief Address of the data.
  const uint32_t addr;

  /// @brief Size of the data.
  const size_t size;

  /// @brief Buffer for the data.
  void *const data[2];

  /// @brief Indicates if the data belongs to a group.
  bool has_group;

  union {
    struct {
      /// @brief Pointer to the group the data belongs to.
      struct tm_group *group;

      /// @brief Index of the data in the group.
      uint8_t data_idx;
    };

    struct {
      /// @brief Spinlock.
      struct k_spinlock lock;

      /// @brief Index of the buffer to get data from.
      uint8_t get_buf_idx;
    };
  };
};

/**
 * @brief Telemetry group.
 *
 */
struct tm_group {
  /// @brief ID of the group.
  const uint32_t id;

  /// @brief Number of data in the group.
  const size_t num_data;

  /// @brief Array of addresses of the data in the group.
  const uint32_t *const data_addrs;

  /// @brief List of backends associated with the group.
  struct tm_backend_list backends;

  /// @brief Spinlock.
  struct k_spinlock lock;

  /// @brief Which data in the group has been updated, each bit represents one
  /// data.
  uint32_t updated;

  /// @brief Number of times the group is currently being accessed.
  uint8_t accessing;

  /// @brief Index of the buffer to get data from.
  uint8_t get_buf_idx;
};

/**
 * @brief Telemetry backend API.
 *
 */
struct tm_backend_api {
  /// @brief Initialization function.
  tm_backend_init_t init;

  /// @brief Publish function.
  tm_backend_publish_t publish;
};

/**
 * @brief Telemetry backend.
 *
 */
struct tm_backend {
  /// @brief Number of groups associated with the backend.
  const size_t num_group;

  /// @brief Array of group IDs associated with the backend.
  const uint32_t *const group_ids;

  /// @brief Backend API.
  const struct tm_backend_api *const api;

  /// @brief List elements for @ref tm_group::backends.
  struct tm_backend_list_elm *const elements;

  /// @brief Custom data of the backend.
  void *const data;
};

/* function definition -------------------------------------------------------*/
/**
 * @brief Get telemetry data.
 *
 * @param[in] addr Address of the data.
 * @param[out] value Pointer to store the retrieved value.
 *
 * @retval 0 For success.
 * @retval -ENOENT If the data does not exist.
 */
int tm_data_get(uint32_t addr, void *value);

/**
 * @brief Update telemetry data.
 *
 * @param[in] addr Address of the data.
 * @param[in] value Pointer to the new value.
 *
 * @retval 0 For success.
 * @retval -ENOENT If the data does not exist.
 */
int tm_data_update(uint32_t addr, const void *value);

/**
 * @brief Begin access to a telemetry group.
 *
 * @param[in] id ID of the group.
 *
 * @retval 0 For success.
 * @retval -ENOENT If the group does not exist.
 */
int tm_group_access_begin(uint32_t id);

/**
 * @brief End access to a telemetry group.
 *
 * @param[in] id ID of the group.
 *
 * @retval 0 For success.
 * @retval -ENOENT If the group does not exist.
 */
int tm_group_access_end(uint32_t id);

/**
 * @brief Commit telemetry data for a group.
 *
 * @param[in] id ID of the group.
 *
 * @retval 0 For success.
 * @retval -ENOENT If the group does not exist.
 */
int tm_group_commit(uint32_t id);

/**
 * @brief Initialize a telemetry backend.
 *
 * @param[in] backend Pointer to the backend.
 */
static inline void tm_backend_init(struct tm_backend *backend) {
  backend->api->init(backend);
}

/**
 * @brief Publish telemetry data.
 *
 * @param[in] backend Pointer to the backend.
 * @param[in] group_id ID of the group to publish.
 */
static inline void tm_backend_publish(struct tm_backend *backend,
                                      const uint32_t group_id) {
  backend->api->publish(backend, group_id);
}

/**
 * @} // Telemetry
 */

#endif  // NTURT_TELEMETRY_H_
