#ifndef NTURT_MSG_MSG_TIME_H_
#define NTURT_MSG_MSG_TIME_H_

// glibc includes
#include <stdbool.h>

// zephyr includes
#include <zephyr/kernel.h>

// lib includes
#include <canopennode.h>

/* type ----------------------------------------------------------------------*/
struct msg_time {
  struct canopen *co;

  bool set;
  struct k_work work;
};

/* function definition -------------------------------------------------------*/
/// @brief Initialization function for time module.
int msg_time_init(struct msg_time *time, struct canopen *co);

#endif // NTURT_MSG_MSG_TIME_H_
