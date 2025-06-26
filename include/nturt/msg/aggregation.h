/**
 * @file
 * @brief Message agregation.
 *
 * @author quantumspawner
 * @version 0.0.1
 * @date 2025-02-13
 * @copyright Copyright (c) 2025 NTU Racing Team
 */

#ifndef NTURT_MSG_AGGREGATION_H_
#define NTURT_MSG_AGGREGATION_H_

// glibc includes
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

// nturt includes
#include <nturt/sys/util.h>

/**
 * @defgroup msg_agg Aggregation
 * @brief Data aggregation.
 * @ingroup msg
 * @{
 */

/* macro ---------------------------------------------------------------------*/
/// @brief Flag indicating the aggregation will not wait for the member to be
/// updated before publishing.
#define AGG_FLAG_OPTIONAL BIT(0)

#define _ARG_FULLY_UPDATED(idx, flags) \
  (flags & AGG_FLAG_OPTIONAL ? 0 : BIT(idx))

/**
 * @brief Static initializer for a dataa aggregation. Refer to @ref AGG_DEFINE
 * for detailed parameter descriptions.
 *
 * @param[in] _obj Object to be initialized.
 *
 */
#define AGG_INITIALIZER(_obj, _name, _period, _min_separation, _watermark,   \
                        _publish, _user_data, ...)                           \
  {                                                                          \
      .name = STRINGIFY(_name),                                              \
      .num_data = NUM_VA_ARGS(__VA_ARGS__) +                                 \
                  ZERO_OR_COMPILE_ERROR(NUM_VA_ARGS(__VA_ARGS__) < 32),      \
      .flags = (uint8_t[]){__VA_ARGS__},                                     \
      .fully_updated = FOR_EACH_IDX(_ARG_FULLY_UPDATED, (|), __VA_ARGS__),   \
                                                                             \
      .period = _period,                                                     \
      .min_separation = _min_separation,                                     \
      .watermark = _watermark,                                               \
      .publish = _publish,                                                   \
      .user_data = _user_data,                                               \
                                                                             \
      .early_timer =                                                         \
          Z_TIMER_INITIALIZER(_obj.early_timer, agg_early_timer_cb, NULL),   \
      .period_timer =                                                        \
          Z_TIMER_INITIALIZER(_obj.period_timer, agg_period_timer_cb, NULL), \
      .work = Z_WORK_DELAYABLE_INITIALIZER(agg_work_cb),                     \
      .updated = 0,                                                          \
  }

/**
 * @brief Define a data aggregation named @p _name to monitor the update of
 * data.
 *
 * @param[in] _name Name of the data aggregation.
 * @param[in] _period Period of data publishing.
 * @param[in] _min_separation Minimum separation time between two data
 * publishing.
 * @param[in] _watermark Watermark to wait for late-arriving members.
 * @param[in] _publish Function to publish the data, must be of type
 * @ref msg_agg_publish_t.
 * @param[in] _user_data Pointer to custom data for the callback.
 * @param[in] ... Flags of the data to be monitored, where each flag represents
 * a data to be monitored. If the data does not require flag, 0 should be
 * specified. The flags can be combined by using the bitwise OR operator (|).
 */
#define AGG_DEFINE(_name, _period, _min_separation, _watermark, _publish, \
                   _user_data, ...)                                       \
  struct agg _name =                                                      \
      AGG_INITIALIZER(_name, _name, _period, _min_separation, _watermark, \
                      _publish, _user_data, __VA_ARGS__)

/**
 * @brief Specify a member of a struct to be monitored for aggregation.
 *
 * @param[in] member Member of the struct to be monitored.
 * @param[in] ... Optional flags of the member, multiple flags can be specified
 * by using the bitwise OR operator (|).
 */
#define AGG_MEMBER(member, ...) \
  (member, (COND_CODE_1(__VA_OPT__(1), (__VA_ARGS__), (0))))

#define _AGG_MEMBER_MEMBER(member) GET_ARG_N(1, __DEBRACKET member)
#define _AGG_MEMBER_FLAGS(member) GET_ARG_N(2, __DEBRACKET member)

/**
 * @brief Intial value of the data.
 *
 * @param[in] val Initialization list of the data.
 * @return Initial value of the data.
 */
#define AGG_DATA_INIT(val, ...) {val, __VA_ARGS__}

#define __AGG_MAP(idx, offset, type) [offset] = idx + 1
#define _AGG_MAP(idx, member, type) \
  __AGG_MAP(idx, offsetof(type, _AGG_MEMBER_MEMBER(member)), type)

/**
 * @brief Define a data aggregation named @p _name to monitor the update of
 * members within a data type.
 *
 * @param[in] _name Name of the data aggregation.
 * @param[in] _type Data type to be monitored.
 * @param[in] _init_val Initial value of the data, must be a specified by
 * @ref AGG_DATA_INIT.
 * @param[in] _period Period of data publishing.
 * @param[in] _min_separation Minimum separation time between two data
 * publishing.
 * @param[in] _watermark Watermark to wait for late-arriving members.
 * @param[in] _publish Function to publish the data, must be of type
 * @ref msg_agg_typed_publish_t.
 * @param[in] _user_data Pointer to custom data for the callback.
 * @param[in] ... Members of @p _type to be monitored, must be specified by
 * @ref AGG_MEMBER.
 */
#define AGG_TYPED_DEFINE(_name, _type, _init_val, _period, _min_separation,   \
                         _watermark, _publish, _user_data, ...)               \
  struct agg_typed _name = {                                                  \
      .agg = AGG_INITIALIZER(_name.agg, _name, _period, _min_separation,      \
                             _watermark, agg_typed_publish, _user_data,       \
                             FOR_EACH(_AGG_MEMBER_FLAGS, (, ), __VA_ARGS__)), \
      .data_size = sizeof(_type),                                             \
      .map =                                                                  \
          (uint8_t[sizeof(_type)]){                                           \
              FOR_EACH_IDX_FIXED_ARG(_AGG_MAP, (, ), _type, __VA_ARGS__),     \
          },                                                                  \
                                                                              \
      .publish = _publish,                                                    \
                                                                              \
      .data = &(_type)_init_val,                                              \
      .pub_data = &(_type){},                                                 \
  }

/**
 * @brief Update a member of data.
 *
 * @param[in,out] agg_typed Pointer to the data aggregation.
 * @param[in] type Type of the data, must be the same as the type specified in
 * @ref AGG_TYPED_DEFINE.
 * @param[in] member Member of the data to be updated, must be listed in
 * @ref AGG_TYPE_DEFINE.
 * @param[in] value New value of the member.
 */
#define AGG_TYPED_UPDATE(agg_typed, type, member, value)                   \
  do {                                                                     \
    K_SPINLOCK(&(agg_typed)->lock) {                                       \
      int idx = (agg_typed)->map[offsetof(type, member)];                  \
      __ASSERT(idx > 0,                                                    \
               "Members must declared in AGG_TYPED_DEFINE to be updated"); \
      ((type *)(agg_typed)->data)->member = (value);                       \
                                                                           \
      agg_update(&(agg_typed)->agg, idx - 1);                              \
    }                                                                      \
  } while (0)

/* type ----------------------------------------------------------------------*/
struct agg;

/**
 * @brief Function to publish the data.
 *
 * @param[in] data Pointer to the data to be published.
 * @param[in] user_data  Pointer to custom data for callback functions.
 */
typedef void (*msg_agg_publish_t)(struct agg *agg, void *user_data);

/**
 * @brief Function to publish the data.
 *
 * @param[in] data Pointer to the data to be published.
 * @param[in] user_data  Pointer to custom data for callback functions.
 */
typedef void (*msg_agg_typed_publish_t)(const void *data, void *user_data);

/// @brief Message aggregation control.
struct agg {
  /** Name of the aggregation. */
  const char *name;

  /** Number of data to be monitored for updating. */
  const size_t num_data;

  /** Flags of the data. */
  const uint8_t *const flags;

  /** What @ref agg::updated should when every data is updated. */
  const uint32_t fully_updated;

  /** Period of data publishing. */
  const k_timeout_t period;

  /** Minimum separation time between two data publishing. */
  const k_timeout_t min_separation;

  /** Watermark to wait for late-arriving members. */
  const k_timeout_t watermark;

  /** Function to publish the data. */
  const msg_agg_publish_t publish;

  /** User data for the callback. */
  void *const user_data;

  /** Spinlock to protect the following members. */
  struct k_spinlock lock;

  /** Timer for periodic publishing. */
  struct k_timer period_timer;

  /** Timer for tracking minimum separation time. */
  struct k_timer early_timer;

  /** Work for the bottom half of publishing, also used for late publishing. */
  struct k_work_delayable work;

  /** Which member has been updated, each bit represents one member. */
  uint32_t updated;
};

struct agg_typed {
  /** Size of the data. */
  const size_t data_size;

  /**
   * Map from the offset of member in the type to the index of data in
   * @ref agg::agg.
   */
  const uint8_t *const map;

  /** Function to publish the data. */
  const msg_agg_typed_publish_t publish;

  /** Spinlock to protect the following members. */
  struct k_spinlock lock;

  /** Data aggregation. */
  struct agg agg;

  /** Pointer to the buffer for updating. */
  void *const data;

  /** Pointer to the buffer for publishing. */
  void *const pub_data;
};

/* function declaration ------------------------------------------------------*/
/**
 * @brief Signal the update of a data in aggregation.
 *
 * @param[in,out] agg Pointer to @ref agg.
 * @param[in] idx Index of the data that is updated.
 */
void agg_update(struct agg *agg, int idx);

/**
 * @brief Timer callback function for periodic publishing.
 *
 * @param[in,out] timer Timer.
 *
 * @warning Internal use only.
 */
void agg_period_timer_cb(struct k_timer *timer);

/**
 * @brief Timer callback function for tracking minimum separation time.
 *
 * @param[in,out] timer Timer.
 *
 * @warning Internal use only.
 */
void agg_early_timer_cb(struct k_timer *timer);

/**
 * @brief Work callback function for the bottom half of publishing, also used
 * for late publishing.
 *
 * @param[in,out] work Work.
 *
 * @warning Internal use only.
 */
void agg_work_cb(struct k_work *work);

/**
 * @brief Publish function for data aggregation.
 *
 * @param[in,out] agg Pointer to @ref agg.
 * @param[in] user_data Pointer to custom data for callback functions.
 *
 * @warning Internal use only.
 */
void agg_typed_publish(struct agg *agg, void *user_data);

/**
 * @} // msg_agg
 */

#endif  // NTURT_MSG_AGGREGATION_H_
