#include "nturt/msg/msg.h"

// glibc includes
#include <stddef.h>
#include <time.h>

// zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/sys/clock.h>
#include <zephyr/zbus/zbus.h>

BUILD_ASSERT(CONFIG_NTURT_MSG_INIT_PRIORITY >
             CONFIG_ZBUS_CHANNELS_SYS_INIT_PRIORITY);

LOG_MODULE_REGISTER(nturt_msg, CONFIG_NTURT_MSG_LOG_LEVEL);

/* static variable -----------------------------------------------------------*/
MSG_ZBUS_CHAN_DEFINE(MSG_LIST);
MSG_SHELL_DEFINE(MSG_LIST);

/* function definition -------------------------------------------------------*/
void msg_header_init(struct msg_header *header) {
  if (IS_ENABLED(CONFIG_NTURT_RTC)) {
    struct timespec ts;
    sys_clock_gettime(SYS_CLOCK_REALTIME, &ts);
    header->timestamp_ns = (uint64_t)ts.tv_sec * 1000000000 + ts.tv_nsec;
  } else {
    header->timestamp_ns = k_ticks_to_ns_floor64(k_uptime_ticks());
  }
}
