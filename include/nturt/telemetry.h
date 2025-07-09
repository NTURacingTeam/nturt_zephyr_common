/**
 * @file
 * @brief Telemetry support.
 *
 * @author quantumspawner
 * @version 0.0.1
 * @date 2024-12-27
 * @copyright Copyright (c) 2024 NTU Racing Team
 */

#ifndef NTURT_TELEMETRY_H_
#define NTURT_TELEMETRY_H_

// glibc includes
#include <stddef.h>
#include <sys/queue.h>

// zephyr include
#include <zephyr/kernel.h>
#include <zephyr/sys/iterable_sections.h>
#include <zephyr/sys/util.h>

// project includes
#include "nturt/msg/aggregation.h"

/**
 * @defgroup tm Telemetry
 * @brief Telemetry support.
 *
 * @{
 */

/* macro ---------------------------------------------------------------------*/
#define _TM_DATA_TYPE(name) CONCAT(__tm_data_, name, _t)

/**
 * @brief Declare a telemetry data, useful for header files.
 *
 * @param[in] name Name of the data.
 * @param[in] type Type of the data.
 *
 */
#define TM_DATA_DECLARE(name, type) \
  typedef type _TM_DATA_TYPE(name); \
  extern struct tm_data name

/**
 * @brief Define a telemetry data.
 *
 * @param[in] _name Name of the data.
 * @param[in] _type Type of the data.
 * @param[in] _addr Address of the data.
 *
 */
#define TM_DATA_DEFINE(_name, _type, _addr)   \
  TM_DATA_DECLARE(_name, _type);              \
                                              \
  STRUCT_SECTION_ITERABLE(tm_data, _name) = { \
      .type = TM_DATA_TYPE_NORMAL,            \
      .addr = _addr,                          \
      .name = STRINGIFY(_name),               \
      .size = sizeof(_type),                  \
      .data = &(_type){0},                    \
  };

/**
 * @brief Declare a telemetry data alias, useful for header files.
 *
 * @param[in] name Name of the alias.
 * @param[in] alias Telemetry data that this alias refers to.
 *
 */
#define TM_ALIAS_DECLARE(name, alias)               \
  typedef _TM_DATA_TYPE(alias) _TM_DATA_TYPE(name); \
  extern struct tm_data name

/**
 * @brief Define a telemetry data alias.
 *
 * @param[in] _name Name of the alias.
 * @param[in] _alias Telemetry data that this alias refers to.
 * @param[in] _addr Address of the alias.
 *
 */
#define TM_ALIAS_DEFINE(_name, _alias, _addr) \
  TM_ALIAS_DECLARE(_name, _alias);            \
                                              \
  STRUCT_SECTION_ITERABLE(tm_data, _name) = { \
      .type = TM_DATA_TYPE_ALIAS,             \
      .addr = _addr,                          \
      .alias = &_alias,                       \
  };

#define _TM_AGG_PUBLISH(name) CONCAT(__tm_agg_publish_, name)

#define __TM_AGG_PUBLISH_DEFINE(_idx, _data)                                 \
  group->publish(group->datas[_idx].data->addr, group->datas[_idx].pub_data, \
                 sizeof(_TM_DATA_TYPE(_data)), user_data)

#define _TM_AGG_PUBLISH_DEFINE(name, ...)                                \
  static void _TM_AGG_PUBLISH(name)(struct agg * agg, void *user_data) { \
    struct tm_group *group = CONTAINER_OF(agg, struct tm_group, agg);    \
                                                                         \
    tm_group_copy(group);                                                \
                                                                         \
    FOR_EACH_IDX(__TM_AGG_PUBLISH_DEFINE, (;), __VA_ARGS__);             \
  }

#define _TM_GROUP_DATA(_data, _group)         \
  {                                           \
      .group = &_group,                       \
      .data = &_data,                         \
      .pub_data = &(_TM_DATA_TYPE(_data)){0}, \
  }

/**
 * @brief Specify a telemetry data to be aggregated and published by a telemetry
 * group. Used in @ref TM_GROUP_DEFINE.
 *
 * @param[in] data Telemetry data to be aggregated and published.
 * @param[in] ... Optional flags of the data, the same ones and rules as
 * @ref AGG_MEMBER.
 */
#define TM_GROUP_DATA(data, ...) \
  (data, (COND_CODE_1(__VA_OPT__(1), (__VA_ARGS__), (0))))

#define _TM_GROUP_DATA_DATA(data) GET_ARG_N(1, __DEBRACKET data)
#define _TM_GROUP_DATA_FLAGS(data) GET_ARG_N(2, __DEBRACKET data)

/**
 * @brief Define a telemetry group to aggregrate and publish telementry data.
 *
 * @param[in] _name Name of the telemetry group.
 * @param[in] _period Period of data publishing.
 * @param[in] _min_separation Minimum separation time between two data
 * publishing.
 * @param[in] _watermark Watermark to wait for late-arriving members.
 * @param[in] _publish Function to publish the data, must be of type
 * @ref tm_publish_t.
 * @param[in] _user_data Pointer to custom data for the callback.
 * @param[in] ... Data to be aggregated and published, must be specified by
 * @ref TM_GROUP_DATA.
 */
#define TM_GROUP_DEFINE(_name, _period, _min_separation, _watermark, _publish, \
                        _user_data, ...)                                       \
  _TM_AGG_PUBLISH_DEFINE(_name,                                                \
                         FOR_EACH(_TM_GROUP_DATA_DATA, (, ), __VA_ARGS__));    \
                                                                               \
  STRUCT_SECTION_ITERABLE(tm_group, _name) = {                                 \
      .agg =                                                                   \
          AGG_INITIALIZER(_name.agg, _name, _period, _min_separation,          \
                          _watermark, _TM_AGG_PUBLISH(_name), _user_data,      \
                          FOR_EACH(_TM_GROUP_DATA_FLAGS, (, ), __VA_ARGS__)),  \
      .publish = _publish,                                                     \
      .num_data = NUM_VA_ARGS(__VA_ARGS__),                                    \
      .datas =                                                                 \
          (struct tm_group_data[]){                                            \
              FOR_EACH_FIXED_ARG(                                              \
                  _TM_GROUP_DATA, (, ), _name,                                 \
                  FOR_EACH(_TM_GROUP_DATA_DATA, (, ), __VA_ARGS__)),           \
          },                                                                   \
  }

/**
 * @brief Get the value of telemetry data using its name.
 *
 * @param[in] name Name of the telemetry data.
 *
 * @return Value of the telemetry data.
 */
#define TM_DATA_GET(name)                                                 \
  ({                                                                      \
    struct tm_data *__data = &name;                                       \
    if (__data->type == TM_DATA_TYPE_ALIAS) {                             \
      __data = __data->alias;                                             \
    }                                                                     \
                                                                          \
    k_spinlock_key_t key = k_spin_lock(&__data->lock);                    \
    _TM_DATA_TYPE(name) __value = *((_TM_DATA_TYPE(name) *)__data->data); \
    k_spin_unlock(&__data->lock, key);                                    \
                                                                          \
    __value;                                                              \
  })

/**
 * @brief Update telemetry data using its name and value.
 *
 * @param[in] name Name of the telemetry data.
 * @param[in] value New value of the telemetry data.
 */
#define TM_DATA_UPDATE(name, value)                   \
  do {                                                \
    struct tm_data *__data = &name;                   \
    if (__data->type == TM_DATA_TYPE_ALIAS) {         \
      __data = __data->alias;                         \
    }                                                 \
                                                      \
    K_SPINLOCK(&__data->lock) {                       \
      *((_TM_DATA_TYPE(name) *)__data->data) = value; \
      tm_data_notify_lock(__data);                    \
    }                                                 \
                                                      \
  } while (0)

/* type ----------------------------------------------------------------------*/
struct tm_data;
struct tm_group;

/**
 * @brief Function to publish the data.
 *
 * @param[in] addr Address of the data.
 * @param[in] data Pointer to the data to be published.
 * @param[in] size Size of the data to be published.
 * @param[in] user_data  Pointer to custom data for callback functions.
 */
typedef void (*tm_publish_t)(uint32_t addr, const void *data, size_t size,
                             void *user_data);

/// @brief Telemetry data type.
enum tm_data_type {
  TM_DATA_TYPE_NORMAL = 0,

  TM_DATA_TYPE_ALIAS,
};

/// @brief Telemetry group data.
struct tm_group_data {
  /** Pointer to the telemetry group. */
  struct tm_group *const group;

  /** Pointer to the telemetry data. */
  struct tm_data *const data;

  /** Pointer to the buffer for publishing. */
  void *const pub_data;

  /** List entry of the element. */
  STAILQ_ENTRY(tm_group_data) next;
};

/// @cond

STAILQ_HEAD(tm_group_list, tm_group_data);

/// @endcond

/// @brief Telemetry data.
struct tm_data {
  /** Type of the telemetry data. */
  const enum tm_data_type type;

  /** Address of the telemetry data. */
  const uint32_t addr;

  union {
    // normal
    struct {
      /** Name of the telementry data. */
      const char *name;

      /** Size of the telemetry data. */
      const size_t size;

      /** List of groups that publish the telemetry data. */
      struct tm_group_list groups;

      /** Spinlock to protect the following members. */
      struct k_spinlock lock;

      /** Pointer to the buffer of the telementry data. */
      void *const data;
    };

    // alias
    struct {
      /** Pointer to the telemetry data that this alias refers to. */
      struct tm_data *const alias;
    };
  };
};

/// @brief Telemetry publishing group.
struct tm_group {
  /** Function to publish the data. */
  const tm_publish_t publish;

  /** Number of data in the group. */
  const size_t num_data;

  /** Spinlock to protect the following members. */
  struct k_spinlock lock;

  /** Aggregation of the  data in the group. */
  struct agg agg;

  /** Array of data in the group. */
  struct tm_group_data *const datas;
};

/* function declaration ------------------------------------------------------*/
/**
 * @brief Get telemetry data using its address and pointer to value.
 *
 * @param[in] addr Address of the telemetry data.
 * @param[out] value Pointer to store the retrieved value.

 * @retval 0 For success.
 * @retval -ENOENT If the data does not exist.
 */
int tm_data_get(uint32_t addr, void *value);

/**
 * @brief Update telemetry data using its address and pointer to value.
 *
 * @param[in] addr Address of the telemetry data.
 * @param[in] value Pointer to the new value of the telemetry data.

 * @retval 0 For success.
 * @retval -ENOENT If the data does not exist.
 */
int tm_data_update(uint32_t addr, const void *value);

/**
 * @brief Notify the telemetry groups that this data has been updated. Must be
 * called while holding the lock.
 *
 * @param[in] data Pointer to @ref tm_data.
 *
 * @warning Internal use only.
 */
void tm_data_notify_lock(const struct tm_data *data);

/**
 * @brief Copy the data in the telemetry data to the group's publishing buffer.
 *
 * @param[in] group Pointer to @ref tm_group.
 *
 * @warning Internal use only.
 */
void tm_group_copy(struct tm_group *group);

/**
 * @} // tm
 */

#endif  // NTURT_TELEMETRY_H_
