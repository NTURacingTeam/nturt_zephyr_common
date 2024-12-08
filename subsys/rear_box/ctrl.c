#include "nturt/rear_box/ctrl.h"

// glibc includes
#include <stddef.h>

// project includes
#include "nturt/msg.h"

/*macro ----------------------------------------------------------------------*/
#define CTRL_SDO_TIMEOUT K_MSEC(20)

#define CTRL_OD_INDEX 0x2081
#define CTRL_OD_SUBINDEX 0

/* function definition -------------------------------------------------------*/
int rb_ctrl_mode_qeury(enum ctrl_mode *mode) {
  uint8_t buf;
  struct sdo_cli_req req = {
      .node_id = CO_NODE_ID_RB,
      .index = CTRL_OD_INDEX,
      .subindex = CTRL_OD_SUBINDEX,
      .data = &buf,
      .size = sizeof(buf),
  };

  int ret = sdo_read(&req, CTRL_SDO_TIMEOUT, NULL, NULL);
  if (ret == 0) {
    *mode = buf;
  }

  return ret;
}

int rb_ctrl_mode_set(enum ctrl_mode mode) {
  uint8_t buf = mode;
  struct sdo_cli_req req = {
      .node_id = CO_NODE_ID_RB,
      .index = CTRL_OD_INDEX,
      .subindex = CTRL_OD_SUBINDEX,
      .data = &buf,
      .size = sizeof(buf),
  };

  return sdo_write(&req, CTRL_SDO_TIMEOUT, NULL, NULL);
}
