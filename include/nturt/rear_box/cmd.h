#ifndef NTURT_REAR_BOX_CMD_H_
#define NTURT_REAR_BOX_CMD_H_

/* type ----------------------------------------------------------------------*/
enum rb_cmd {
  RB_CMD_NONE = 0,
  RB_CMD_RTD,
  RB_CMD_DISABLE,
  RB_CMD_SET_HOME,
  RB_CMD_RESET,
  RB_CMD_INV_RESET,
};

/* function declaration ------------------------------------------------------*/
int rb_cmd(enum rb_cmd cmd);

#endif  // NTURT_REAR_BOX_CMD_H_
