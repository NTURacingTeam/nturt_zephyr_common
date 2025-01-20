#ifndef NTURT_MSG_SDO_SRV_H_
#define NTURT_MSG_SDO_SRV_H_

// zephyr includes
#include <zephyr/kernel.h>

// lib includes
#include <canopennode.h>

/* type ----------------------------------------------------------------------*/
struct sdo_srv {
  struct canopen *co;

  struct k_work work;
};

/* function definition -------------------------------------------------------*/
/// @brief Initialization function for SDO server module.
int sdo_srv_init(struct sdo_srv *sdo_srv, struct canopen *co);

#endif  // NTURT_MSG_SDO_SRV_H_
