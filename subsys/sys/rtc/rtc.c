// glibc includes
#include <errno.h>
#include <string.h>
#include <time.h>

// zephyr includes
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/rtc.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/posix/time.h>

// project includes
#include "nturt/sys/sys.h"

LOG_MODULE_REGISTER(nturt_rtc, CONFIG_NTURT_LOG_LEVEL);

/* static function declaration -----------------------------------------------*/
/// @brief Initialization function for NTURT RTC system module.
static int init();

/* static varaible -----------------------------------------------------------*/
static const struct device* rtc = DEVICE_DT_GET(DT_CHOSEN(nturt_rtc));

SYS_INIT(init, APPLICATION, CONFIG_NTURT_SYS_INIT_PRIORITY);

int sys_set_time(time_t time) {
  int ret;

  struct timespec ts = {
      .tv_sec = time,
      .tv_nsec = 0,
  };
  ret = clock_settime(CLOCK_REALTIME, &ts);
  if (ret < 0) {
    LOG_ERR("Fail to set posix real-time clock time: %s", strerror(errno));
    return -errno;
  }

  ret = rtc_set_time(rtc, (struct rtc_time*)gmtime(&time));
  if (ret < 0) {
    LOG_ERR("Fail to set RTC time: %s", strerror(-ret));
    return ret;
  }

  return 0;
}

/* static function definition ------------------------------------------------*/
static int init() {
  int ret;

  if (!device_is_ready(rtc)) {
    LOG_ERR("RTC device %s is not ready", rtc->name);
    return -ENODEV;
  }

  struct rtc_time time;
  ret = rtc_get_time(rtc, &time);
  if (ret == -ENODATA) {
    LOG_WRN("RTC time not set, initializing to dummy value");
    struct rtc_time dummy_time = {
        .tm_year = 2025 - 1900,
        .tm_mon = 0,
        .tm_mday = 1,
        .tm_wday = 0,
        .tm_hour = 0,
        .tm_min = 0,
        .tm_sec = 0,
    };

    ret = rtc_set_time(rtc, &dummy_time);
    if (ret < 0) {
      LOG_ERR("Fail to set RTC time: %s", strerror(-ret));
      return ret;
    }

    ret = rtc_get_time(rtc, &time);
  }

  if (ret < 0) {
    LOG_ERR("Fail to get RTC time: %s", strerror(-ret));
    return ret;
  }

  struct timespec ts = {
      .tv_sec = mktime(rtc_time_to_tm(&time)),
      .tv_nsec = 0,
  };

  char time_str[] = "1970-01-01T00:00:00";
  strftime(time_str, sizeof(time_str), "%FT%T", rtc_time_to_tm(&time));
  LOG_INF("Set posix real-time clock to %lld.%06ld (%s)", ts.tv_sec, ts.tv_nsec,
          time_str);

  ret = clock_settime(CLOCK_REALTIME, &ts);
  if (ret < 0) {
    LOG_ERR("Fail to set posix real-time clock time: %s", strerror(errno));
    return errno;
  }

  return 0;
}
