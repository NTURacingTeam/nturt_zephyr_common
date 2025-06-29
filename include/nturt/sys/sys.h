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

/**
 * @addtogroup sys System
 * @brief Basic system support.
 * @{
 */

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
