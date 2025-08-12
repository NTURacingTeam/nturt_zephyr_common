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

/**
 * @addtogroup sys System
 * @brief Basic system support.
 * @{
 */

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
 * @brief Reset system.
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
