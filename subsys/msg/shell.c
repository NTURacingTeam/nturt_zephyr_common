// glibc includes
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/iterable_sections.h>
#include <zephyr/zbus/zbus.h>

// project includes
#include "nturt/msg/msg.h"

/* type ----------------------------------------------------------------------*/
struct msg_dump_get_data {
  size_t idx;
  const struct zbus_channel *chan;
};

struct msg_dump_data {
  const char *name;
  const struct zbus_channel *chan;
};

/* static function declaration -----------------------------------------------*/
static bool msg_dump_get_iter(const struct zbus_channel *chan, void *user_data);
static void msg_dump_get_handler(size_t idx, struct shell_static_entry *entry);

static bool msg_stats_iter(const struct zbus_channel *chan, void *user_data);
static int msg_stats_cmd_handler(const struct shell *sh, size_t argc,
                                 char **argv, void *data);

static bool msg_dump_iter(const struct zbus_channel *chan, void *user_data);
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
static bool msg_dump_get_iter(const struct zbus_channel *chan,
                              void *user_data) {
  struct msg_dump_get_data *data = user_data;

  if (!msg_chan_is_from_msg(chan)) {
    return true;
  }

  if (data->idx-- == 0) {
    data->chan = chan;
    return false;
  }

  return true;
}

static void msg_dump_get_handler(size_t idx, struct shell_static_entry *entry) {
  struct msg_dump_get_data data = {
      .idx = idx,
  };

  if (zbus_iterate_over_channels_with_user_data(msg_dump_get_iter, &data)) {
    entry->syntax = NULL;
    return;
  }

  entry->syntax = zbus_chan_name(data.chan);
  entry->handler = NULL;
  entry->subcmd = &msg_dump_cmd;
  entry->help = NULL;
}

static bool msg_stats_iter(const struct zbus_channel *chan, void *user_data) {
  const struct shell *sh = user_data;

  if (!msg_chan_is_from_msg(chan)) {
    return true;
  }

  shell_print(sh, "%s", zbus_chan_name(chan));
  if (zbus_chan_pub_stats_count(chan) == 0) {
    shell_print(sh, "\tNo messages published yet.\n");
    return true;
  }

  uint32_t diff = k_ticks_to_ms_floor32(k_uptime_ticks() -
                                        zbus_chan_pub_stats_last_time(chan));
  uint32_t avg = zbus_chan_pub_stats_avg_period(chan);
  shell_print(sh, "\tpublish count: %u", zbus_chan_pub_stats_count(chan));
  shell_print(sh, "\tlast published: %u.%03u s ago, average: %u.%03u s\n",
              diff / 1000, diff % 1000, avg / 1000, avg % 1000);

  return true;
}

static int msg_stats_cmd_handler(const struct shell *sh, size_t argc,
                                 char **argv, void *data) {
  (void)argc;
  (void)argv;
  (void)data;

  zbus_iterate_over_channels_with_user_data(msg_stats_iter, (void *)sh);

  return 0;
}

static bool msg_dump_iter(const struct zbus_channel *chan, void *user_data) {
  struct msg_dump_data *data = user_data;

  if (!msg_chan_is_from_msg(chan)) {
    return true;
  }

  if (strcmp(data->name, zbus_chan_name(chan)) == 0) {
    data->chan = chan;
    return false;
  }

  return true;
}

static int msg_dump_cmd_handler(const struct shell *sh, size_t argc,
                                char **argv, void *_data) {
  (void)argc;
  (void)_data;

  struct msg_dump_data data = {
      .name = argv[1],
  };

  if (zbus_iterate_over_channels_with_user_data(msg_dump_iter, &data)) {
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
    ret = zbus_chan_add_obs(data.chan, &msg_shell_listener, K_FOREVER);
    if (ret == -EALREADY) {
      shell_warn(sh, "Channel %s is already being dumped",
                 zbus_chan_name(data.chan));

    } else if (ret < 0) {
      shell_error(sh, "zbus_chan_add_obs(%s) failed: %s",
                  zbus_chan_name(data.chan), strerror(-ret));
    }

  } else {
    ret = zbus_chan_rm_obs(data.chan, &msg_shell_listener, K_FOREVER);
    if (ret == -ENODATA) {
      shell_error(sh, "Channel %s is not being dumped",
                  zbus_chan_name(data.chan));

    } else if (ret < 0) {
      shell_error(sh, "zbus_chan_rm_obs(%s) failed: %s",
                  zbus_chan_name(data.chan), strerror(-ret));
    }
  }

  return ret;
}

static void msg_cb(const struct zbus_channel *chan) {
  const void *data = zbus_chan_const_msg(chan);
  msg_chan_print(chan, data);
}
