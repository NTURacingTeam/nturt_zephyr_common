#include "nturt/msg/logging.h"

// glibc includes
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// zephyr includes
#include <zephyr/fs/fs.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/zbus/zbus.h>

// project includes
#include "nturt/msg/msg.h"
#include "nturt/sys/fs.h"

LOG_MODULE_REGISTER(nturt_msg_logging, CONFIG_NTURT_MSG_LOG_LEVEL);

/* static function declaration -----------------------------------------------*/
static void thread(void *arg1, void *arg2, void *arg3);

/* static variable -----------------------------------------------------------*/
K_THREAD_DEFINE(msg_logging_thread, CONFIG_NTURT_MSG_LOGGING_THREAD_STACK_SIZE,
                thread, NULL, NULL, NULL,
                CONFIG_NTURT_MSG_LOGGING_THREAD_PRIORITY, 0, 0);

ZBUS_MSG_SUBSCRIBER_DEFINE(msg_logging_msg_subscriber);

/* function definition -------------------------------------------------------*/
int msg_chan_logging_start(const struct zbus_channel *chan, const char *file) {
  __ASSERT(is_msg_chan(chan), "chan must be a message channel");
  __ASSERT(strlen(file) < MAX_PATH_LEN, "file path too long");

  struct msg_chan_data *chan_data = zbus_chan_user_data(chan);
  struct msg_logging *logging = &chan_data->logging;

  k_mutex_lock(&logging->lock, K_FOREVER);

  int ret;
  if (logging->is_logging) {
    ret = -EALREADY;
    LOG_ERR("Channel %s is already being logged", zbus_chan_name(chan));
    goto out;
  }

  fs_file_t_init(&logging->file);
  ret = fs_open(&logging->file, file, FS_O_WRITE | FS_O_APPEND);
  if (ret == -ENOENT) {
    char dirname[MAX_PATH_LEN];
    size_t len = strrchr(file, '/') - file;
    strncpy(dirname, file, len);
    dirname[len] = '\0';

    ret = mkdir_p(dirname);
    if (ret < 0) {
      LOG_ERR("mkdir_p(%s) failed: %s", dirname, strerror(-ret));
      goto out;
    }

    ret = fs_open(&logging->file, file, FS_O_WRITE | FS_O_CREATE);
    if (ret < 0) {
      LOG_ERR("fs_open(%s) failed: %s", file, strerror(-ret));
      goto out;
    }

    const char *header = msg_chan_csv_header(chan);
    fs_write(&logging->file, header, strlen(header));

  } else if (ret < 0) {
    LOG_ERR("fs_open(%s) failed: %s", file, strerror(-ret));
    goto out;
  }

  ret = zbus_chan_add_obs(chan, &msg_logging_msg_subscriber, K_FOREVER);
  if (ret < 0) {
    LOG_ERR("zbus_chan_add_obs(%s) failed: %s", zbus_chan_name(chan),
            strerror(-ret));
    goto out_file;
  }

  logging->is_logging = true;

  goto out;

out_file:
  fs_close(&logging->file);

out:
  k_mutex_unlock(&logging->lock);
  return ret;
}

int msg_chan_logging_stop(const struct zbus_channel *chan) {
  __ASSERT(is_msg_chan(chan), "chan must be a message channel");

  struct msg_chan_data *chan_data = zbus_chan_user_data(chan);
  struct msg_logging *logging = &chan_data->logging;

  k_mutex_lock(&logging->lock, K_FOREVER);

  int ret;
  if (!logging->is_logging) {
    ret = -ENOTCONN;
    LOG_ERR("Channel %s is not being logged", zbus_chan_name(chan));
    goto out;
  }

  ret = zbus_chan_rm_obs(chan, &msg_logging_msg_subscriber, K_FOREVER);
  if (ret < 0) {
    LOG_ERR("zbus_chan_remove_obs(%s) failed: %s", zbus_chan_name(chan),
            strerror(-ret));
    goto out;
  }

  ret = fs_close(&logging->file);
  if (ret < 0) {
    LOG_ERR("fs_close failed: %s", strerror(-ret));
    goto out;
  }

  logging->is_logging = false;

out:
  k_mutex_unlock(&logging->lock);
  return ret;
}

/* static function definition ------------------------------------------------*/
static void thread(void *arg1, void *arg2, void *arg3) {
  const struct zbus_channel *chan;
  uint8_t msg[CONFIG_NTURT_MSG_MAX_SIZE];
  char buf[512];
  buf[0] = '\n';

  while (
      !zbus_sub_wait_msg(&msg_logging_msg_subscriber, &chan, msg, K_FOREVER)) {
    struct msg_chan_data *chan_data = zbus_chan_user_data(chan);
    struct msg_logging *logging = &chan_data->logging;

    int size = msg_chan_csv_write(chan, msg, buf + 1, sizeof(buf) - 1);
    if (size >= sizeof(buf) - 1) {
      LOG_ERR("msg_chan_csv_write(%s) failed: buffer too small",
              zbus_chan_name(chan));
      continue;
    }

    fs_write(&logging->file, buf, size + 1);
    fs_sync(&logging->file);
  }
}
