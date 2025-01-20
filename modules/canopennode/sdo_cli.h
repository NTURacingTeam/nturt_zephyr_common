#ifndef NTURT_MSG_SDO_CLI_H_
#define NTURT_MSG_SDO_CLI_H_

// glibc includes
#include <stdbool.h>
#include <stddef.h>

// zephyr includes
#include <zephyr/kernel.h>

// lib includes
#include <canopennode.h>

// project includes
#include "nturt/msg.h"

/* type ----------------------------------------------------------------------*/
struct sdo_cli_req_ctx {
  struct sdo_cli *sdo_cli;

  const struct sdo_cli_req *req;
  size_t offset;
  bool partial;

  bool read;
  k_timepoint_t deadline;
  sdo_cli_callback_t callback;
  void *user_data;

  struct k_work_delayable timeout_work;
};

struct sdo_cli {
  struct canopen *co;

  struct k_work work;

  struct k_spinlock lock;

  int size;
  int head;
  int tail;
  struct sdo_cli_req_ctx ctxes[CONFIG_NTURT_MSG_SDO_CLI_BUF_SIZE];
};

/* function declaration ------------------------------------------------------*/
/// @brief Initialization function for SDO client module.
int sdo_cli_init(struct sdo_cli *sdo_cli, struct canopen *co);

#endif  // NTURT_MSG_SDO_CLI_H_
