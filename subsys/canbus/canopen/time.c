// glibc includes
#include <stdbool.h>
#include <time.h>

// zephyr includes
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

// lib includes
#include <canopennode.h>

// project includes
#include "nturt/sys/sys.h"

LOG_MODULE_REGISTER(nturt_canopen_time, CONFIG_NTURT_CANOPEN_LOG_LEVEL);

/* type ----------------------------------------------------------------------*/
struct canopen_time_ctx {
  bool set;
};

/* static function declrartion -----------------------------------------------*/
static int init();
static void time_cb(time_t epoch, void *user_data);

/* static variable -----------------------------------------------------------*/
static struct canopen_time_ctx g_ctx = {
    .set = false,
};

SYS_INIT(init, APPLICATION, CONFIG_NTURT_CANOPEN_INIT_PRIORITY);

/* static function definition ------------------------------------------------*/
static int init() {
  canopen_time_set_callback(time_cb, &g_ctx);

  return 0;
}

static void time_cb(time_t epoch, void *user_data) {
  struct canopen_time_ctx *ctx = user_data;

  if (ctx->set) {
    return;
  }

  char time_str[] = "1970-01-01T00:00:00";
  strftime(time_str, sizeof(time_str), "%FT%T", gmtime(&epoch));
  LOG_INF("Setting system time to %lld (%s)", epoch, time_str);

  int ret = sys_set_time(epoch);
  if (ret < 0) {
    LOG_ERR("Failed to set time: %s", strerror(-ret));
  } else {
    ctx->set = true;
  }
}
