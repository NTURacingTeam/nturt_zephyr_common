#ifndef NTURT_REAR_BOX_CTAL_H_
#define NTURT_REAR_BOX_CTAL_H_

// glibc includes
#include <stdint.h>

/* type ----------------------------------------------------------------------*/
/// @brief Rear box control modes.
enum ctrl_mode {
  CTRL_MODE_LOW = 0,
  CTRL_MODE_HIGH,
  CTRL_MODE_REVERSED,

  NUM_CTRL_MODES,
};

/* function declaration ------------------------------------------------------*/
int rb_ctrl_mode_qeury(enum ctrl_mode *mode);

int rb_ctrl_mode_set(enum ctrl_mode mode);

#endif  // NTURT_REAR_BOX_CTAL_H_
