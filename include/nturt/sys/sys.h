#ifndef NTURT_SYS_H_
#define NTURT_SYS_H_

// glibc includes
#include <time.h>

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

#endif  // NTURT_SYS_H_
