#include "nturt/msg/msg.h"

// glibc includes
#include <stddef.h>
#include <time.h>

// zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/sys/clock.h>
#include <zephyr/sys/iterable_sections.h>
#include <zephyr/zbus/zbus.h>

BUILD_ASSERT(CONFIG_NTURT_MSG_INIT_PRIORITY >
             CONFIG_ZBUS_CHANNELS_SYS_INIT_PRIORITY);

LOG_MODULE_REGISTER(nturt_msg, CONFIG_NTURT_MSG_LOG_LEVEL);

/* static variable -----------------------------------------------------------*/
MSG_CHAN_DEFINE(MSG_LIST);

/* function definition -------------------------------------------------------*/
bool msg_chan_is_from_msg(const struct zbus_channel *chan) {
  STRUCT_SECTION_START_EXTERN(msg_chan_data);
  STRUCT_SECTION_END_EXTERN(msg_chan_data);

  void *data = zbus_chan_user_data(chan);

  return data >= (void *)STRUCT_SECTION_START(msg_chan_data) &&
         data < (void *)STRUCT_SECTION_END(msg_chan_data);
}

void msg_chan_print(const struct zbus_channel *chan, const void *data) {
  __ASSERT(msg_chan_is_from_msg(chan), "chan must be a message channel");

  struct msg_chan_data *chan_data = zbus_chan_user_data(chan);

  chan_data->print(data);
}

const char *msg_chan_csv_header(const struct zbus_channel *chan) {
  __ASSERT(msg_chan_is_from_msg(chan), "chan must be a message channel");

  struct msg_chan_data *chan_data = zbus_chan_user_data(chan);

  return chan_data->csv_header();
}

int msg_chan_csv_write(const struct zbus_channel *chan, const void *data,
                       char *buf, size_t len) {
  __ASSERT(msg_chan_is_from_msg(chan), "chan must be a message channel");

  struct msg_chan_data *chan_data = zbus_chan_user_data(chan);

  return chan_data->csv_write(buf, len, data);
}

void msg_header_init(struct msg_header *header) {
  if (IS_ENABLED(CONFIG_NTURT_RTC)) {
    struct timespec ts;
    sys_clock_gettime(SYS_CLOCK_REALTIME, &ts);
    header->timestamp_ns = (uint64_t)ts.tv_sec * 1000000000 + ts.tv_nsec;
  } else {
    header->timestamp_ns = k_ticks_to_ns_floor64(k_uptime_ticks());
  }
}

void msg_agg_publish(const void *data, void *user_data) {
  const struct zbus_channel *chan = user_data;

  struct msg_header *header = (struct msg_header *)data;
  msg_header_init(header);

  int ret;
  ret = zbus_chan_pub(chan, header, K_MSEC(5));
  if (ret < 0) {
    LOG_ERR("Failed to publish data: %s", strerror(-ret));
  }
}
