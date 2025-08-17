// glibc includes
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

// zephyr includes
#include <zephyr/shell/shell.h>

// canopen includes
#include <canopennode.h>

/* static function declaration -----------------------------------------------*/
static int msg_stats_cmd_handler(const struct shell *sh, size_t argc,
                                 char **argv, void *data);

/* static variable -----------------------------------------------------------*/
SHELL_STATIC_SUBCMD_SET_CREATE(canopen_error_cmd,
                               SHELL_CMD_ARG(get, NULL,
                                             "Get currently set errors.",
                                             msg_stats_cmd_handler, 1, 0),
                               SHELL_SUBCMD_SET_END);

SHELL_STATIC_SUBCMD_SET_CREATE(canopen_cmd,
                               SHELL_CMD(error, &canopen_error_cmd,
                                         "CANopen errors.", NULL),
                               SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(canopen, &canopen_cmd,
                   "CANopen commands.\n"
                   "Usage: canopen <subcommand>",
                   NULL);

/* static function definition ------------------------------------------------*/
static int msg_stats_cmd_handler(const struct shell *sh, size_t argc,
                                 char **argv, void *data) {
  (void)argc;
  (void)argv;
  (void)data;

  bool has_error = false;
  for (int i = 0; i < CONFIG_CANOPENNODE_EM_ERR_STATUS_BITS_COUNT; i++) {
    if (CO_isError(CO->em, i)) {
      if (!has_error) {
        shell_print(sh, "Currently set errors (error register: 0x%X): 0x%X",
                    CO_getErrorRegister(CO->em), i);
        has_error = true;

      } else {
        shell_print(sh, ", 0x%X", i);
      }
    }
  }

  if (has_error) {
    shell_print(sh, "\n");

  } else {
    shell_print(sh, "No errors are currently set");
  }

  return 0;
}
