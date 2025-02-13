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
#include <string.h>

// zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

// nturt includes
#include <nturt/util.h>

/**
 * @defgroup msg_agg Message Aggregation
 * @brief Message aggregation.
 * @ingroup msg
 * @{
 */

/* macro ---------------------------------------------------------------------*/
#define _MSG_AGG_TYPE_MAP(idx, x, type) [offsetof(type, x)] = idx + 1

/**
 * @brief Declare a message aggregation struct type named @p _name from a
 * underlying @p _type with members @p __VA_ARGS__ monitored to be aggregated.
 *
 * @param[in] _name Name of the aggregation struct type.
 * @param[in] _type Underlying data type to be aggregated.
 * @param[in] ... Members of @p _type to be monitored.
 */
#define MSG_AGG_TYPE_DECLARE(_name, _type, ...)                                         \
  BUILD_ASSERT(NUM_VA_ARGS(__VA_ARGS__) < 32, "Number of fields must be less than 32"); \
                                                                                        \
  static const size_t __msg_agg_num_member_##_name = NUM_VA_ARGS(__VA_ARGS__);          \
  static const uint8_t __msg_agg_map_##_name[sizeof(_type)] = {                         \
      FOR_EACH_IDX_FIXED_ARG(_MSG_AGG_TYPE_MAP, (, ), _type, __VA_ARGS__),              \
  };                                                                                    \
                                                                                        \
  struct _name {                                                                        \
    struct msg_agg_ctrl ctrl;                                                           \
    _type *type;                                                                        \
  }

/**
 * @brief Intial value of data for message aggregation.
 *
 * @param[in] val Initialization list of the data.
 * @return Initial value of the data.
 */
#define MSG_AGG_DATA_INIT(val, ...) {val, __VA_ARGS__}

/**
 * @brief Define a message aggregation struct of type @p _type named @p _name .
 *
 * @param[in] _name Name of the aggregation struct.
 * @param[in] _struct_type Message aggregation struct type defined by
 * @ref MSG_AGG_TYPE_DECLARE.
 * @param[in] _publish Function to publish the data.
 * @param[in] _user_data Pointer to custom data for callback functions.
 * @param[in] _init_val Initial value of the data, must be defined by
 * @ref MSG_AGG_DATA_INIT.
 */
#define MSG_AGG_DEFINE(_name, _struct_type, _publish, _user_data, _init_val)                     \
  static DEREF_TYPE(TYPEOF_FIELD(struct _struct_type, type)) __msg_agg_data_##_name = _init_val; \
  static DEREF_TYPE(TYPEOF_FIELD(struct _struct_type, type)) __msg_agg_pub_data_##_name;         \
  struct _struct_type _name = {                                                                  \
      .ctrl =                                                                                    \
          {                                                                                      \
              .name = #_name,                                                                    \
              .data = &__msg_agg_data_##_name,                                                   \
              .pub_data = &__msg_agg_pub_data_##_name,                                           \
              .data_size = sizeof(DEREF_TYPE(TYPEOF_FIELD(struct _struct_type, type))),          \
              .num_member = __msg_agg_num_member_##_struct_type,                                 \
              .map = __msg_agg_map_##_struct_type,                                               \
                                                                                                 \
              .publish = _publish,                                                               \
              .user_data = _user_data,                                                           \
                                                                                                 \
              .updated = 0,                                                                      \
          },                                                                                     \
  }

/**
 * @brief Initialize a message aggregation.
 *
 * @param[in,out] agg Pointer to the message aggregation.
 */
#define MSG_AGG_INIT(agg) msg_agg_ctrl_init(&(agg)->ctrl)

/**
 * @brief Start a message aggregation to publish data periodically.
 *
 * @param[in,out] agg Pointer to the message aggregation.
 * @param[in] period Period of publishing.
 * @param[in] watermark Watermark to wait for late-arriving members.
 */
#define MSG_AGG_START(agg, period, watermark) msg_agg_ctrl_start(&(agg)->ctrl, period, watermark)

/**
 * @brief Stop a message aggregation from publishing data.
 *
 * @param[in,out] agg Pointer to the message aggregation.
 */
#define MSG_AGG_STOP(agg) msg_agg_ctrl_stop(&(agg)->ctrl)

#define _MSG_AGG_CTRL_UPDATE(ctrl, type, member, val)           \
  do {                                                          \
    K_SPINLOCK(&(ctrl)->lock) {                                 \
      ((type *)(ctrl)->data)->member = (val);                   \
                                                                \
      msg_agg_ctrl_update_lock((ctrl), offsetof(type, member)); \
    }                                                           \
  } while (0)

/**
 * @brief Update a member of a message aggregation data.
 *
 * @param[in,out] agg Pointer to the message aggregation.
 * @param[in] member Member of the data to be updated, must be listed in
 * @ref MSG_AGG_TYPE_DECLARE.
 * @param[in] val New value of the member.
 */
#define MSG_AGG_UPDATE(agg, member, val) \
  _MSG_AGG_CTRL_UPDATE(&(agg)->ctrl, DEREF_TYPE(__typeof__((agg)->type)), member, val)

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
struct msg_agg_ctrl {
  /** Name of the aggregation. */
  const char *name;

  /** Pointer to the buffer for updating. */
  void *const data;

  /** Pointer to the buffer for publishing. */
  void *const pub_data;

  /** Size of the data type. */
  const size_t data_size;

  /** Number of members of the data type to be monitored for updating. */
  const size_t num_member;

  /** Map from the byte offset of the member to the index of the member. */
  const uint8_t *const map;

  /** Function to publish the data. */
  const msg_agg_publish_t publish;

  /** User data for callback functions */
  void *const user_data;

  /** Spinlock to protect the following members. */
  struct k_spinlock lock;

  /** Watermark to wait for late-arriving mwmbers. */
  k_timeout_t watermark;

  /** Timer for periodic publishing. */
  struct k_timer timer;

  /** Work for the bottom half of publishing. */
  struct k_work_delayable work;

  /** Which member has been updated, each bit represents one member. */
  uint32_t updated;
};

/* function declaration ------------------------------------------------------*/
/**
 * @brief Implementation of @ref MSG_AGG_INIT.
 *
 * @warning Internal use only.
 */
void msg_agg_ctrl_init(struct msg_agg_ctrl *ctrl);

/**
 * @brief Implementation of @ref MSG_AGG_START.
 *
 * @warning Internal use only.
 */
void msg_agg_ctrl_start(struct msg_agg_ctrl *ctrl, k_timeout_t period, k_timeout_t watermark);

/**
 * @brief Implementation of @ref MSG_AGG_STOP.
 *
 * @warning Internal use only.
 */
void msg_agg_ctrl_stop(struct msg_agg_ctrl *ctrl);

/**
 * @brief Signal the update of a member, must be called while holding the lock.
 *
 * @param[in,out] ctrl Pointer to @ref msg_agg_ctrl.
 * @param[in] offset Byte offset of the member.
 *
 * @warning Internal use only.
 */
void msg_agg_ctrl_update_lock(struct msg_agg_ctrl *ctrl, size_t offset);

/**
 * @} // msg_agg
 */

#endif  // NTURT_MSG_AGGREGATION_H_
