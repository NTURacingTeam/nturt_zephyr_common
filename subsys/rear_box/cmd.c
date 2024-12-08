#include "nturt/rear_box/cmd.h"

// glibc includes
#include <stddef.h>

// zephyr includes
#include <zephyr/logging/log.h>

// project includes
#include "nturt/msg.h"

LOG_MODULE_REGISTER(nturt_rb_cmd);

/*macro ----------------------------------------------------------------------*/
#define CMD_SDO_TIMEOUT K_MSEC(20)

#define CMD_OD_INDEX 0x2080
#define CMD_OD_SUBINDEX 0

/* function definition -------------------------------------------------------*/
int rb_cmd(enum rb_cmd cmd) {
  uint8_t buf = cmd;
  struct sdo_cli_req req = {
      .node_id = CO_NODE_ID_RB,
      .index = CMD_OD_INDEX,
      .subindex = CMD_OD_SUBINDEX,
      .data = &buf,
      .size = sizeof(buf),
  };

  return sdo_write(&req, CMD_SDO_TIMEOUT, NULL, NULL);
}
