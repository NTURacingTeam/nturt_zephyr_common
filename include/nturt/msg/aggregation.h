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
/// @brief Flag indicating the member is registered to the aggregation.
#define AGG_FLAG_VALID BIT(4)

/// @brief Flag indicating the aggregation will not wait for the member to be
/// updated before publishing.
#define AGG_FLAG_OPTIONAL BIT(5)

/// @brief Flag mask indicating the index of the member in the aggregation.
#define AGG_FLAG_IDX_MASK BIT_MASK(4)

/**
 * @brief Declare a member of a struct to be monitored for aggregation.
 *
 * @param[in] member Member of the struct to be monitored.
 * @param[in] ... Optional flags of the member, multiple flags can be specified
 * by using the bitwise OR operator (|).
 */
#define AGG_MEMBER(member, ...) \
  (member, (COND_CODE_1(__VA_OPT__(1), (AGG_FLAG_VALID | __VA_ARGS__), (AGG_FLAG_VALID))))

#define __AGG_FLAGS(idx, member, flags, type) [offsetof(type, member)] = flags | idx

#define _AGG_FLAGS(idx, args, type) \
  __AGG_FLAGS(idx, GET_ARG_N(1, __DEBRACKET args), GET_ARG_N(2, __DEBRACKET args), type)

#define _ARG_FULLY_UPDATED(idx, args) \
  (GET_ARG_N(2, __DEBRACKET args) & AGG_FLAG_OPTIONAL ? 0 : BIT(idx))

/**
 * @brief Declare a struct type for data aggregation named @p _name for a
 * underlying @p _type with members @p __VA_ARGS__ monitored to be aggregated.
 *
 * @param[in] _name Name of the aggregation struct type.
 * @param[in] _type Underlying data type to be aggregated.
 * @param[in] ... Members of @p _type to be monitored, must be declared by
 * @ref AGG_MEMBER.
 */
#define AGG_TYPE_DECLARE(_name, _type, ...)                              \
  BUILD_ASSERT(NUM_VA_ARGS(__VA_ARGS__) < 32,                            \
               "Number of fields to be monitored must be less than 32"); \
                                                                         \
  static const uint8_t __agg_flags_map_##_name[sizeof(_type)] = {        \
      FOR_EACH_IDX_FIXED_ARG(_AGG_FLAGS, (, ), _type, __VA_ARGS__),      \
  };                                                                     \
  static const uint32_t __agg_fully_updated_##_name =                    \
      FOR_EACH_IDX(_ARG_FULLY_UPDATED, (|), __VA_ARGS__);                \
                                                                         \
  struct _name {                                                         \
    struct agg_ctrl ctrl;                                                \
    _type *type;                                                         \
  }

/**
 * @brief Intial value of the data.
 *
 * @param[in] val Initialization list of the data.
 * @return Initial value of the data.
 */
#define AGG_DATA_INIT(val, ...) {val, __VA_ARGS__}

/**
 * @brief Define a message aggregation struct of type @p _type named @p _name .
 *
 * @param[in] _name Name of the data aggregation.
 * @param[in] _struct_type Struct type of the data aggregation defined by
 * @ref AGG_TYPE_DECLARE.
 * @param[in] _init_val Initial value of the data, must be defined by
 * @ref AGG_DATA_INIT.
 * @param[in] _period Period of data publishing.
 * @param[in] _min_separation Minimum separation time between two data
 * publishing.
 * @param[in] _watermark Watermark to wait for late-arriving members.
 * @param[in] _publish Function to publish the data.
 * @param[in] _user_data Pointer to custom data for the callback.
 */
#define AGG_DEFINE(_name, _struct_type, _init_val, _period, _min_separation, _watermark, _publish, \
                   _user_data)                                                                     \
  struct _struct_type _name = {                                                                    \
      .ctrl =                                                                                      \
          {                                                                                        \
              .name = #_name,                                                                      \
              .data_size = sizeof(DEREF_TYPE(TYPEOF_FIELD(struct _struct_type, type))),            \
              .data = &(DEREF_TYPE(TYPEOF_FIELD(struct _struct_type, type)))_init_val,             \
              .pub_data = &(DEREF_TYPE(TYPEOF_FIELD(struct _struct_type, type))){},                \
              .flags_map = __agg_flags_map_##_struct_type,                                         \
              .fully_updated = __agg_fully_updated_##_struct_type,                                 \
                                                                                                   \
              .period = _period,                                                                   \
              .min_separation = _min_separation,                                                   \
              .watermark = _watermark,                                                             \
              .publish = _publish,                                                                 \
              .user_data = _user_data,                                                             \
                                                                                                   \
              .early_timer =                                                                       \
                  Z_TIMER_INITIALIZER(_name.ctrl.early_timer, agg_early_timer_cb, NULL),           \
              .period_timer =                                                                      \
                  Z_TIMER_INITIALIZER(_name.ctrl.period_timer, agg_period_timer_cb, NULL),         \
              .work = Z_WORK_DELAYABLE_INITIALIZER(agg_work_cb),                                   \
              .updated = 0,                                                                        \
          },                                                                                       \
  }

#define _AGG_CTRL_UPDATE(ctrl, type, member, val)           \
  do {                                                      \
    K_SPINLOCK(&(ctrl)->lock) {                             \
      ((type *)(ctrl)->data)->member = (val);               \
                                                            \
      agg_ctrl_update_lock((ctrl), offsetof(type, member)); \
    }                                                       \
  } while (0)

/**
 * @brief Update a member of a message aggregation data.
 *
 * @param[in,out] agg Pointer to the message aggregation.
 * @param[in] member Member of the data to be updated, must be listed in
 * @ref AGG_TYPE_DECLARE.
 * @param[in] val New value of the member.
 */
#define AGG_UPDATE(agg, member, val) \
  _AGG_CTRL_UPDATE(&(agg)->ctrl, DEREF_TYPE(__typeof__((agg)->type)), member, val)

/* type ----------------------------------------------------------------------*/
/**
 * @brief Function to publish the data.
 *
 * @param[in] data Pointer to the data to be published.
 * @param[in] user_data  Pointer to custom data for callback functions.
 */
typedef void (*msg_agg_publish_t)(void *data, void *user_data);

/**
 * @brief Message aggregation control.
 */
struct agg_ctrl {
  /** Name of the aggregation. */
  const char *name;

  /** Size of the data. */
  const size_t data_size;

  /** Pointer to the buffer for updating. */
  struct msg_susp_data *const data;

  /** Pointer to the buffer for publishing. */
  struct msg_susp_data *const pub_data;

  /** Map from the byte offset of the member to its flags. */
  const uint8_t *const flags_map;

  /** Number of members of the data type to be monitored for updating. */
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

/* function declaration ------------------------------------------------------*/
/**
 * @brief Signal the update of a member, must be called while holding
 * @ref agg_ctrl::lock.
 *
 * @param[in,out] ctrl Pointer to @ref agg_ctrl.
 * @param[in] offset Byte offset of the member.
 *
 * @warning Internal use only.
 */
void agg_ctrl_update_lock(struct agg_ctrl *ctrl, size_t offset);

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
 * @} // msg_agg
 */

#endif  // NTURT_MSG_AGGREGATION_H_
