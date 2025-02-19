#include "nturt/msg/msg.h"

// glibc includes
#include <stddef.h>

// zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>

BUILD_ASSERT(CONFIG_NTURT_MSG_INIT_PRIORITY >
             CONFIG_ZBUS_CHANNELS_SYS_INIT_PRIORITY);

LOG_MODULE_REGISTER(nturt_msg, CONFIG_NTURT_LOG_LEVEL);

/* static variable -----------------------------------------------------------*/
MSG_ZBUS_CHAN_DEFINE(MSG_LIST);
