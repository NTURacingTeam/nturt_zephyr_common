#include "nturt/rear_box/states.h"

// project includes
#include "nturt/msg.h"

/*macro ----------------------------------------------------------------------*/
#define STATES_SDO_TIMEOUT K_MSEC(20)

#define STATES_OD_INDEX 0x20F0
#define STATES_OD_SUBINDEX 0

#define ERRORS_OD_INDEX 0x20F0
#define ERRORS_OD_SUBINDEX 1

/* function definition -------------------------------------------------------*/
int rb_states_querry(states_t* states) {
  struct sdo_cli_req req = {
      .node_id = CO_NODE_ID_RB,
      .index = STATES_OD_INDEX,
      .subindex = STATES_OD_SUBINDEX,
      .data = states,
      .size = sizeof(*states),
  };

  return sdo_read(&req, STATES_SDO_TIMEOUT, NULL, NULL);
}

int rb_err_querry(err_t* err) {
  struct sdo_cli_req req = {
      .node_id = CO_NODE_ID_RB,
      .index = ERRORS_OD_INDEX,
      .subindex = ERRORS_OD_SUBINDEX,
      .data = err,
      .size = sizeof(*err),
  };

  return sdo_read(&req, STATES_SDO_TIMEOUT, NULL, NULL);
}
