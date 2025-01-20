#include "msg_time.h"

// glibc includes
#include <stdbool.h>
#include <time.h>

// zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

// lib includes
#include <canopennode.h>

// project includes
#include "nturt/msg.h"
#include "nturt/sys.h"

#if !((CO_CONFIG_TIME & CO_CONFIG_TIME_ENABLE) && \
      (CO_CONFIG_TIME & CO_CONFIG_FLAG_CALLBACK_PRE))
#error \
    "CONFIG_NTURT_MSG_TIME requires CO_CONFIG_TIME_ENABLE and" \
"CO_CONFIG_FLAG_CALLBACK_PRE to be set in CO_CONFIG_TIME"
#endif

LOG_MODULE_REGISTER(nturt_msg_time, CONFIG_NTURT_LOG_LEVEL);

/* macro ---------------------------------------------------------------------*/
/// @brief CANopen time reference in POSIX epoch time in seconds.
#define CO_SEC_REF 441763200UL

/// @brief Number of seconds in a day.
#define SEC_OF_DAY 86400

/* static function declaration -----------------------------------------------*/
/// @brief Callback function when receiving CANopen TIME object.
static void msg_time_cb(void *arg);

/// @brief Bottom half of @ref msg_time_cb.
static void msg_time_work(struct k_work *work);

/* function definition -------------------------------------------------------*/
int msg_time_init(struct msg_time *time, struct canopen *co) {
  time->co = co;
  k_work_init(&time->work, msg_time_work);

  CO_TIME_initCallbackPre(co->CO->TIME, time, msg_time_cb);

  return 0;
}

/* static function definition ------------------------------------------------*/
static void msg_time_cb(void *arg) {
  struct msg_time *time = arg;

  k_work_submit(&time->work);
}

static void msg_time_work(struct k_work *work) {
  struct msg_time *time = CONTAINER_OF(work, struct msg_time, work);

  int ret;

  CO_NMT_internalState_t NMTstate = CO_NMT_getInternalState(time->co->CO->NMT);
  if (NMTstate != CO_NMT_PRE_OPERATIONAL && NMTstate != CO_NMT_OPERATIONAL) {
    return;
  }

  CO_TIME_process(time->co->CO->TIME, true, 0);
  LOG_INF("Received TIME object, days: %d, ms: %d", time->co->CO->TIME->days,
          time->co->CO->TIME->ms);

  if (time->set) {
    return;
  }

  time_t epoch = ((time_t)time->co->CO->TIME->days * SEC_OF_DAY +
                  time->co->CO->TIME->ms / 1000) +
                 CO_SEC_REF;

  char time_str[] = "1970-01-01T00:00:00";
  strftime(time_str, sizeof(time_str), "%FT%T", gmtime(&epoch));
  LOG_INF("Setting system time to %lld (%s)", epoch, time_str);

  ret = sys_set_time(epoch);
  if (ret < 0) {
    LOG_ERR("Failed to set time: %s", strerror(-ret));
  } else {
    time->set = true;
  }
}
