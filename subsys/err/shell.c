// glibc includes
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

// zephyr includes
#include <zephyr/shell/shell.h>
#include <zephyr/sys/iterable_sections.h>

// project includes
#include "nturt/err/err.h"
#include "nturt/sys/util.h"

/* static function declaration -----------------------------------------------*/
static void err_get_handler_impl(size_t idx, struct shell_static_entry *entry,
                                 bool is_set);
static void err_set_get_handler(size_t idx, struct shell_static_entry *entry);
static void err_clear_get_handler(size_t idx, struct shell_static_entry *entry);

static int err_list_cmd_handler(const struct shell *sh, size_t argc,
                                char **argv, void *data);
static int err_get_cmd_handler(const struct shell *sh, size_t argc, char **argv,
                               void *data);
static int err_set_cmd_handler(const struct shell *sh, size_t argc, char **argv,
                               void *data);
static int err_clear_cmd_handler(const struct shell *sh, size_t argc,
                                 char **argv, void *data);

/* static variable -----------------------------------------------------------*/
SHELL_DYNAMIC_CMD_CREATE(err_set_subcmd, err_set_get_handler);

SHELL_DYNAMIC_CMD_CREATE(err_clear_subcmd, err_clear_get_handler);

SHELL_STATIC_SUBCMD_SET_CREATE(
    canopen_cmd,
    SHELL_CMD_ARG(list, NULL, "List all registered errors.",
                  err_list_cmd_handler, 1, 0),
    SHELL_CMD_ARG(get, NULL, "Get currently set errors.", err_get_cmd_handler,
                  1, 0),
    SHELL_CMD_ARG(set, &err_set_subcmd,
                  "Set error.\n"
                  "Usage: set <error>",
                  err_set_cmd_handler, 2, 0),
    SHELL_CMD_ARG(clear, &err_clear_subcmd, "Clear error.",
                  err_clear_cmd_handler, 2, 0),
    SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(err, &canopen_cmd,
                   "Error module commands.\n"
                   "Usage: err <subcommand>",
                   NULL);

/* static function definition ------------------------------------------------*/
static void err_get_handler_impl(size_t idx, struct shell_static_entry *entry,
                                 bool is_set) {
  STRUCT_SECTION_FOREACH(err, err) {
    if (XOR(err->flags & ERR_FLAG_SET, is_set)) {
      if (idx == 0) {
        entry->syntax = err->name;
        entry->handler = NULL;
        entry->subcmd = NULL;
        entry->help = err->desc;
        return;

      } else {
        idx--;
      }
    }
  }

  entry->syntax = NULL;
}

static void err_set_get_handler(size_t idx, struct shell_static_entry *entry) {
  err_get_handler_impl(idx, entry, true);
}

static void err_clear_get_handler(size_t idx,
                                  struct shell_static_entry *entry) {
  err_get_handler_impl(idx, entry, false);
}

static int err_list_cmd_handler(const struct shell *sh, size_t argc,
                                char **argv, void *data) {
  (void)argc;
  (void)argv;
  (void)data;

  shell_print(sh, "Registered errors:");

  STRUCT_SECTION_FOREACH(err, err) {
    shell_print(sh, "\t%s(0x%X): %s", err->name, err->errcode, err->desc);
  }

  return 0;
}

static int err_get_cmd_handler(const struct shell *sh, size_t argc, char **argv,
                               void *data) {
  (void)argc;
  (void)argv;
  (void)data;

  bool has_error = false;
  struct err *err;
  ERR_FOREACH_SET(err) {
    if (!has_error) {
      shell_print(sh, "Currently set errors:");
      has_error = true;
    }

    shell_print(sh, "\t%s(0x%X): %s", err->name, err->errcode, err->desc);
  }

  if (!has_error) {
    shell_print(sh, "No errors are currently set.");
  }

  return 0;
}

static int err_set_cmd_handler(const struct shell *sh, size_t argc, char **argv,
                               void *data) {
  (void)argc;
  (void)data;

  STRUCT_SECTION_FOREACH(err, err) {
    if (!strcmp(err->name, argv[1])) {
      if (err->flags & ERR_FLAG_SET) {
        shell_warn(sh, "Error %s(0x%X) is already set.", err->name,
                   err->errcode);
        return -EALREADY;
      }

      err_report(err->errcode, true);

      shell_print(sh, "Error %s(0x%X) set.", err->name, err->errcode);
      return 0;
    }
  }

  shell_error(sh, "Unknown error: %s", argv[1]);
  return -ENOENT;
}

static int err_clear_cmd_handler(const struct shell *sh, size_t argc,
                                 char **argv, void *data) {
  (void)argc;
  (void)data;

  STRUCT_SECTION_FOREACH(err, err) {
    if (!strcmp(err->name, argv[1])) {
      if (!(err->flags & ERR_FLAG_SET)) {
        shell_warn(sh, "Error %s(0x%X) is not set.", err->name, err->errcode);
        return -EALREADY;
      }

      err_report(err->errcode, false);

      shell_print(sh, "Error %s(0x%X) cleared.", err->name, err->errcode);
      return 0;
    }
  }

  shell_error(sh, "Unknown error: %s", argv[1]);
  return -ENOENT;
}
