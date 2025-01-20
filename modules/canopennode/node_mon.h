#ifndef NTURT_MSG_NODE_MON_H_
#define NTURT_MSG_NODE_MON_H_

// glibc includes
#include <stdbool.h>
#include <stdint.h>

// lib includes
#include <canopennode.h>

// project includes
#include "nturt/msg.h"

/* type ----------------------------------------------------------------------*/
struct node_mon {
  struct canopen *co;

  struct node_mon_state states[OD_CNT_ARR_1016];
};

/* function definition -------------------------------------------------------*/
/// @brief Initialization function for node monitor module.
int node_mon_init(struct node_mon *node_mon, struct canopen *co);

#endif  // NTURT_MSG_NODE_MON_H_
