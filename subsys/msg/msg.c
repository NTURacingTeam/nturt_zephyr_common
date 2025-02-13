#include "nturt/msg/msg.h"

// glibc includes
#include <stddef.h>

// zephyr includes
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>

/* variable ------------------------------------------------------------------*/
LOG_MODULE_REGISTER(nturt_msg, CONFIG_NTURT_LOG_LEVEL);

MSG_ZBUS_CHAN_DEFINE(MSG_LIST);
