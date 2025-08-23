#include "nturt/msg/logging.h"

// glibc includes
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// zephyr includes
#include <zephyr/fs/fs.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/sys/cbprintf.h>
#include <zephyr/sys/mpsc_pbuf.h>
#include <zephyr/sys/util.h>
#include <zephyr/zbus/zbus.h>

// project includes
#include "nturt/msg/msg.h"
#include "nturt/sys/fs.h"
#include "nturt/sys/sys.h"
#include "nturt/sys/util.h"

LOG_MODULE_REGISTER(nturt_msg_logging, CONFIG_NTURT_MSG_LOG_LEVEL);

/* macro ---------------------------------------------------------------------*/
// Use the same alignment as cbprintf as it is used to print the message.
#define MSG_ALIGNMENT CBPRINTF_PACKAGE_ALIGNMENT

#define MSG_PADDING                                                   \
  ((sizeof(struct msg_logging_packet_header) % MSG_ALIGNMENT) > 0     \
       ? (MSG_ALIGNMENT -                                             \
          (sizeof(struct msg_logging_packet_header) % MSG_ALIGNMENT)) \
       : 0)

/* type ----------------------------------------------------------------------*/
enum msg_logging_packet_type {
  PACKET_TYPE_DATA = 0,
  PACKET_TYPE_STOP,

  MAX_PACKET_TYPE,
};

struct msg_logging_packet_header {
  MPSC_PBUF_HDR;
  enum msg_logging_packet_type type : 2;
  uint32_t len : 30 - MPSC_PBUF_HDR_BITS;
  const struct zbus_channel *chan;
};

struct msg_logging_packet {
  struct msg_logging_packet_header header;
  uint8_t __padding[MSG_PADDING];
  uint8_t data[];
};

struct msg_logging_ctx {
  struct k_work logging_work;

  struct mpsc_pbuf_buffer mpsc_pbuf;

  // a word is assumed to be 4 bytes by mpsc_pbuf
  uint32_t __mpsc_pbuf_buf[CONFIG_NTURT_MSG_LOGGING_BUF_SIZE / 4];
};

/* static function declaration -----------------------------------------------*/
static void msg_logging_init(struct msg_logging_ctx *ctx);
static struct msg_logging_packet *msg_logging_packet_alloc(
    const struct zbus_channel *chan, enum msg_logging_packet_type type);

static int init();

static void msg_cb(const struct zbus_channel *chan);
static void logging_work(struct k_work *work);
static uint32_t msg_logging_packet_get_wlen(
    const union mpsc_pbuf_generic *packet);
static void msg_logging_packet_drop(const struct mpsc_pbuf_buffer *buffer,
                                    const union mpsc_pbuf_generic *packet);

/* static variable -----------------------------------------------------------*/
static struct msg_logging_ctx g_ctx = {
    .logging_work = Z_WORK_INITIALIZER(logging_work),
};

SYS_INIT(init, APPLICATION, CONFIG_NTURT_MSG_INIT_PRIORITY);

ZBUS_LISTENER_DEFINE(msg_logging_listener, msg_cb);

/* function definition -------------------------------------------------------*/
int msg_chan_logging_start(const struct zbus_channel *chan, const char *file) {
  __ASSERT(msg_chan_is_from_msg(chan), "chan must be a message channel");
  __ASSERT(strlen(file) < FS_MAX_PATH_LEN, "file path too long");

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
    char dirname[FS_MAX_PATH_LEN];
    size_t len = strrchr(file, '/') - file;
    strncpy(dirname, file, len);
    dirname[len] = '\0';

    ret = fs_mkdir_p(dirname);
    if (ret < 0) {
      LOG_ERR("fs_mkdir_p(%s) failed: %s", dirname, strerror(-ret));
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

  fs_sync(&logging->file);

  ret = zbus_chan_add_obs(chan, &msg_logging_listener, K_FOREVER);
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
  __ASSERT(msg_chan_is_from_msg(chan), "chan must be a message channel");

  struct msg_chan_data *chan_data = zbus_chan_user_data(chan);
  struct msg_logging *logging = &chan_data->logging;

  k_mutex_lock(&logging->lock, K_FOREVER);

  int ret;
  if (!logging->is_logging) {
    ret = -ENOTCONN;
    LOG_ERR("Channel %s is not being logged", zbus_chan_name(chan));
    goto out;
  }

  struct msg_logging_packet *packet =
      msg_logging_packet_alloc(chan, PACKET_TYPE_STOP);
  if (packet == NULL) {
    ret = -ENOMEM;
    goto out;
  }

  ret = zbus_chan_rm_obs(chan, &msg_logging_listener, K_FOREVER);
  if (ret < 0) {
    LOG_ERR("zbus_chan_remove_obs(%s) failed: %s", zbus_chan_name(chan),
            strerror(-ret));
    goto out;
  }

  logging->is_logging = false;

  mpsc_pbuf_commit(&g_ctx.mpsc_pbuf, (union mpsc_pbuf_generic *)packet);
  if (!k_work_is_pending(&g_ctx.logging_work)) {
    sys_work_submit(&g_ctx.logging_work);
  }

  // does not unlock the mutex so that other operations must wait until stop is
  // processed in the sys workqueue

  return 0;

out:
  k_mutex_unlock(&logging->lock);
  return ret;
}

void msg_logging_sync_work(struct k_work *work) {
  struct k_work_delayable *dwork = k_work_delayable_from_work(work);
  struct msg_logging *logging =
      CONTAINER_OF(dwork, struct msg_logging, sync_dwork);

  int ret = fs_sync(&logging->file);
  if (ret < 0) {
    LOG_ERR("fs_sync failed: %s", strerror(-ret));
  }
}

/* static function definition ------------------------------------------------*/
static void msg_logging_init(struct msg_logging_ctx *ctx) {
  struct mpsc_pbuf_buffer_config config = {
      .buf = ctx->__mpsc_pbuf_buf,
      .size = ARRAY_SIZE(ctx->__mpsc_pbuf_buf),
      .get_wlen = msg_logging_packet_get_wlen,
      .notify_drop = msg_logging_packet_drop,
      .flags = MPSC_PBUF_MODE_OVERWRITE,
  };

  mpsc_pbuf_init(&ctx->mpsc_pbuf, &config);
}

static struct msg_logging_packet *msg_logging_packet_alloc(
    const struct zbus_channel *chan, enum msg_logging_packet_type type) {
  size_t len = sizeof(struct msg_logging_packet);
  if (type == PACKET_TYPE_DATA) {
    len += zbus_chan_msg_size(chan);
  }
  len = DIV_ROUND_UP(len, 4);

  union mpsc_pbuf_generic *_packet =
      mpsc_pbuf_alloc(&g_ctx.mpsc_pbuf, len, K_NO_WAIT);
  if (_packet == NULL) {
    LOG_ERR("Failed to allocate packet for %s", zbus_chan_name(chan));
    return NULL;
  }

  struct msg_logging_packet *packet = (struct msg_logging_packet *)_packet;

  packet->header.type = type;
  packet->header.len = len;
  packet->header.chan = chan;

  return packet;
}

static int init() {
  msg_logging_init(&g_ctx);

  return 0;
}

static void msg_cb(const struct zbus_channel *chan) {
  struct msg_logging_packet *packet =
      msg_logging_packet_alloc(chan, PACKET_TYPE_DATA);
  if (packet == NULL) {
    return;
  }

  memcpy(packet->data, zbus_chan_const_msg(chan), zbus_chan_msg_size(chan));

  mpsc_pbuf_commit(&g_ctx.mpsc_pbuf, (union mpsc_pbuf_generic *)packet);
  if (!k_work_is_pending(&g_ctx.logging_work)) {
    sys_work_submit(&g_ctx.logging_work);
  }
}

static void logging_work(struct k_work *work) {
  struct msg_logging_ctx *ctx =
      CONTAINER_OF(work, struct msg_logging_ctx, logging_work);

  const union mpsc_pbuf_generic *_packet = mpsc_pbuf_claim(&ctx->mpsc_pbuf);
  if (_packet == NULL) {
    return;
  }

  const struct msg_logging_packet *packet =
      (const struct msg_logging_packet *)_packet;

  const struct zbus_channel *chan = packet->header.chan;
  struct msg_chan_data *chan_data = zbus_chan_user_data(chan);
  struct msg_logging *logging = &chan_data->logging;

  int ret;
  switch (packet->header.type) {
    case PACKET_TYPE_DATA: {
      char buf[512];
      buf[0] = '\n';

      int size =
          msg_chan_csv_write(chan, packet->data, buf + 1, sizeof(buf) - 1);
      if (size >= sizeof(buf) - 1) {
        LOG_ERR("msg_chan_csv_write(%s) failed: buffer too small",
                zbus_chan_name(chan));
        goto out;
      }

      ret = fs_write(&logging->file, buf, size + 1);
      if (ret < 0) {
        LOG_ERR("fs_write failed: %s", strerror(-ret));
        goto out;
      }

      sys_work_schedule(&logging->sync_dwork,
                        K_MSEC(CONFIG_VCU_MSG_LOGGING_SYNC_INTERVAL));
    } break;

    case PACKET_TYPE_STOP:
      ret = fs_close(&logging->file);
      if (ret < 0) {
        LOG_ERR("fs_close failed: %s", strerror(-ret));
      }

      k_work_cancel_delayable(&logging->sync_dwork);
      k_mutex_unlock(&logging->lock);

      break;

    default:
      break;
  }

out:
  mpsc_pbuf_free(&ctx->mpsc_pbuf, _packet);

  if (mpsc_pbuf_is_pending(&ctx->mpsc_pbuf)) {
    sys_work_submit(&ctx->logging_work);
  }
}

static uint32_t msg_logging_packet_get_wlen(
    const union mpsc_pbuf_generic *_packet) {
  const struct msg_logging_packet *packet =
      (const struct msg_logging_packet *)_packet;

  return packet->header.len;
}

static void msg_logging_packet_drop(const struct mpsc_pbuf_buffer *buffer,
                                    const union mpsc_pbuf_generic *_packet) {
  (void)buffer;

  const struct msg_logging_packet *packet =
      (const struct msg_logging_packet *)_packet;

  LOG_ERR_THROTTLE(K_MSEC(500), "Dropping packet from %s",
                   zbus_chan_name(packet->header.chan));
}
