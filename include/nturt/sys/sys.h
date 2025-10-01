/**
 * @file
 * @brief Basic system support.
 *
 * @author quantumspawner
 * @version 0.0.1
 * @date 2025-02-25
 * @copyright Copyright (c) 2025 NTU Racing Team
 */

#ifndef NTURT_SYS_H_
#define NTURT_SYS_H_

// glibc includes
#include <time.h>

// zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/sys/iterable_sections.h>

/**
 * @addtogroup sys System
 * @brief Basic system support.
 * @{
 */

/* macro ---------------------------------------------------------------------*/
/**
 * @brief Same as @ref SYS_SHUTDOWN_CALLBACK_DEFINE, but with a custom name for
 * the callback.
 */
#define SYS_SHUTDOWN_CALLBACK_DEFINE_NAMED(_name, _handler, _user_data, \
                                           _priority)                   \
  static const STRUCT_SECTION_ITERABLE(                                 \
      sys_shutdown_callback,                                            \
      CONCAT(__sys_shutdown_handler_, _priority, _name)) = {            \
      .handler = _handler,                                              \
      .user_data = _user_data,                                          \
  }

/**
 * @brief Define a shutdown callback.
 *
 * @param[in] handler Handler for shutting down.
 * @param[in] user_data Pointer to custom data for the callback.
 * @param[in] priority Priority of the callback.
 *
 * @note Since the name of the callback is derived from the name of @p handler ,
 * if the same handler is used for multiple callbacks,
 * @ref SYS_SHUTDOWN_CALLBACK_DEFINE_NAMED can be used instead to prevent linker
 * errors.
 */
#define SYS_SHUTDOWN_CALLBACK_DEFINE(handler, user_data, priority) \
  SYS_SHUTDOWN_CALLBACK_DEFINE_NAMED(handler, handler, user_data, priority)

/* type ----------------------------------------------------------------------*/
/**
 * @brief Shutdown handler type.
 *
 * @param[in,out] user_data Pointer to custom user data for the callback
 * provided by @ref SYS_SHUTDOWN_CALLBACK_DEFINE.
 */
typedef void (*sys_shutdown_handler_t)(void *user_data);

/// @brief Shutdown callback.
struct sys_shutdown_callback {
  /** Shutdown handler. */
  sys_shutdown_handler_t handler;

  /** User data for the callback functions. */
  void *user_data;
};

/* function declaration ------------------------------------------------------*/
/**
 * @brief Submit a work item to the system work queue.
 *
 * Different from the system work queue provided by Zephyr running in high
 * priority designed for bottom halves of interrupts, this work queue runs in
 * lower priority for less critical application level works.
 *
 * @param work Pointer to the queue item.
 * @return int Same as Zephyr `k_work_submit_to_queue`.
 */
int sys_work_submit(struct k_work *work);

/**
 * @brief Submit an idle work item to the system work queue after a delay.
 *
 * Different from the system work queue provided by Zephyr running in high
 * priority designed for bottom halves of interrupts, this work queue runs in
 * lower priority for less critical application level works.
 *
 * @param dwork Pointer to the delayable work item.
 * @param delay The time to wait before submitting the work item.
 * If `K_NO_WAIT` this is equivalent to @ref sys_work_submit.
 * @return int Same as Zephyr `k_work_schedule_for_queue`.
 */
int sys_work_schedule(struct k_work_delayable *dwork, k_timeout_t delay);

/**
 * @brief Reschedule a work item to the system work queue after a delay.
 *
 * Different from the system work queue provided by Zephyr running in high
 * priority designed for bottom halves of interrupts, this work queue runs in
 * lower priority for less critical application level works.
 *
 * @param dwork Pointer to the delayable work item.
 * @param delay The time to wait before submitting the work item.
 * @return int Same as Zephyr `k_work_reschedule_for_queue`.
 */
int sys_work_reschedule(struct k_work_delayable *dwork, k_timeout_t delay);

/**
 * @brief Shutdown the system.
 *
 */
void sys_shutdown();

/**
 * @brief Reset the system.
 *
 */
void sys_reset();

/**
 * @brief Set system time.
 *
 * @param[in] time Time to set.
 *
 * @retval 0 For success.
 * @retval others Negative error number.
 */
int sys_set_time(time_t time);

/**
 * @} // sys
 */

#endif  // NTURT_SYS_H_
