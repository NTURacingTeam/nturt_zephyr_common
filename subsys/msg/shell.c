// glibc includes
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/iterable_sections.h>
#include <zephyr/sys/util.h>
#include <zephyr/zbus/zbus.h>

// project includes
#include "nturt/msg/msg.h"
#include "nturt/sys/util.h"

/* static function declaration -----------------------------------------------*/
static void msg_dump_get_handler(size_t idx, struct shell_static_entry *entry);

static int msg_stats_cmd_handler(const struct shell *sh, size_t argc,
                                 char **argv, void *data);
static int msg_dump_cmd_handler(const struct shell *sh, size_t argc,
                                char **argv, void *data);

static void msg_cb(const struct zbus_channel *chan);

/* static variable -----------------------------------------------------------*/
SHELL_STATIC_SUBCMD_SET_CREATE(
    msg_dump_cmd, SHELL_CMD(on, NULL, "Enable dumping message.", NULL),
    SHELL_CMD(off, NULL, "Disable dumping message.", NULL),
    SHELL_SUBCMD_SET_END);

SHELL_DYNAMIC_CMD_CREATE(msg_dump_subcmd, msg_dump_get_handler);

SHELL_STATIC_SUBCMD_SET_CREATE(msg_cmd,
                               SHELL_CMD_ARG(stats, NULL,
                                             "Print message statistics.",
                                             msg_stats_cmd_handler, 1, 0),
                               SHELL_CMD_ARG(dump, &msg_dump_subcmd,
                                             "Enable message dumping.\n"
                                             "Usage: dump <channel> <on|off>",
                                             msg_dump_cmd_handler, 3, 0),
                               SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(msg, &msg_cmd,
                   "Inter-thread communication commands.\n"
                   "Usage: msg <subcommand>",
                   NULL);

ZBUS_LISTENER_DEFINE(msg_shell_listener, msg_cb);

/* static function definition ------------------------------------------------*/
static void msg_dump_get_handler(size_t idx, struct shell_static_entry *entry) {
  size_t count;
  STRUCT_SECTION_COUNT(msg_shell, &count);
  if (idx >= count) {
    entry->syntax = NULL;
    return;
  }

  const struct msg_shell *shell;
  STRUCT_SECTION_GET(msg_shell, idx, &shell);

  entry->syntax = zbus_chan_name(shell->chan);
  entry->handler = NULL;
  entry->subcmd = &msg_dump_cmd;
  entry->help = NULL;
}

static int msg_stats_cmd_handler(const struct shell *sh, size_t argc,
                                 char **argv, void *data) {
  (void)argc;
  (void)argv;
  (void)data;

  STRUCT_SECTION_FOREACH(msg_shell, shell) {
    shell_print(sh, "%s", zbus_chan_name(shell->chan));
    if (zbus_chan_pub_stats_count(shell->chan) == 0) {
      shell_print(sh, "\tNo messages published yet.\n");
      continue;
    }

    uint32_t diff = k_ticks_to_ms_floor32(
        k_uptime_ticks() - zbus_chan_pub_stats_last_time(shell->chan));
    uint32_t avg = zbus_chan_pub_stats_avg_period(shell->chan);
    shell_print(sh, "\tpublish count: %u",
                zbus_chan_pub_stats_count(shell->chan));
    shell_print(sh, "\tlast published: %u.%03u s ago, average: %u.%03u s\n",
                diff / 1000, diff % 1000, avg / 1000, avg % 1000);
  }

  return 0;
}

static int msg_dump_cmd_handler(const struct shell *sh, size_t argc,
                                char **argv, void *data) {
  (void)argc;
  (void)data;

  const struct zbus_channel *chan = NULL;
  STRUCT_SECTION_FOREACH(msg_shell, shell) {
    if (!strcmp(argv[1], zbus_chan_name(shell->chan))) {
      chan = shell->chan;
      break;
    }
  }

  if (chan == NULL) {
    shell_error(sh, "Unknown channel: %s", argv[1]);
    return -ENOENT;
  }

  int ret = 0;
  bool is_on = shell_strtobool(argv[2], 0, &ret);
  if (ret) {
    shell_error(sh, "Invalid command: %s", argv[2]);
    return ret;
  }

  if (is_on) {
    ret = zbus_chan_add_obs(chan, &msg_shell_listener, K_FOREVER);
    if (ret == -EALREADY) {
      shell_warn(sh, "Channel %s is already being dumped",
                 zbus_chan_name(chan));

    } else if (ret < 0) {
      shell_error(sh, "zbus_chan_add_obs failed: %s", strerror(-ret));
    }

  } else {
    ret = zbus_chan_rm_obs(chan, &msg_shell_listener, K_FOREVER);
    if (ret == -ENODATA) {
      shell_error(sh, "Channel %s is not being dumped", zbus_chan_name(chan));

    } else if (ret < 0) {
      shell_error(sh, "zbus_chan_rm_obs failed: %s", strerror(-ret));
    }
  }

  return ret;
}

static void msg_cb(const struct zbus_channel *chan) {
  STRUCT_SECTION_FOREACH(msg_shell, shell) {
    if (shell->print(chan)) {
      return;
    }
  }
}
