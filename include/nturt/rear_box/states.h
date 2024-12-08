#ifndef NTURT_REAR_BOX_STATES_H_
#define NTURT_REAR_BOX_STATES_H_

// glibc includes
#include <stdint.h>

// zephyr includes
#include <zephyr/sys/util.h>

// project includes
#include "nturt/err.h"

/* macro ---------------------------------------------------------------------*/
#define STATE_RTD_MASK GENMASK(STATE_RTD_SOUND, STATE_RTD_BLINK)

/* type ----------------------------------------------------------------------*/
typedef uint16_t states_t;

/// @brief Rear box hierarchical state machine states.
enum states_state {
  STATE_ROOT = 0,
  STATE_ERR_FREE,
  STATE_READY,
  STATE_RTD_BLINK,
  STATE_RTD_STEADY,
  STATE_RTD_SOUND,
  STATE_RUNNING,
  STATE_ERROR,

  NUM_STATES,
};

/* function declaration ------------------------------------------------------*/
int rb_states_querry(states_t* states);

int rb_err_querry(err_t* err);

#endif  // NTURT_REAR_BOX_STATES_H_
