#ifndef NTURT_SYS_H_
#define NTURT_SYS_H_

// glibc includes
#include <time.h>

/**
 * @brief Set system time.
 * 
 * @param time Time to set.
 * @return int 0 if success, negative error code otherwise.
 */
int sys_set_time(time_t time);

#endif  // NTURT_SYS_H_
