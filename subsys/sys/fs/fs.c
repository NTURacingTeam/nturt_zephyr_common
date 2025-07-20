// glibc includes
#include <stdint.h>

// zephyr includes
#include <zephyr/fs/fs.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>

#ifdef CONFIG_NTURT_FS_FAT_FS
#include <ff.h>
#ifdef CONFIG_NTURT_FS_FAT_FS_HAS_RTC
#include <zephyr/posix/time.h>
#endif
#elif defined(CONFIG_NTURT_FS_LITTLE_FS)
#include <zephyr/fs/littlefs.h>
#endif

LOG_MODULE_REGISTER(nturt_fs, CONFIG_NTURT_SYS_LOG_LEVEL);

/* function definition -------------------------------------------------------*/
#ifdef CONFIG_NTURT_FS_FAT_FS_HAS_RTC
/// @brief Get the current time in FAT file system format, declared in ff.h.
uint32_t get_fattime() {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);

  struct tm tm;
  localtime_r(&ts.tv_sec, &tm);

  return ((tm.tm_year - 80) << 25) | ((tm.tm_mon + 1) << 21) |
         (tm.tm_mday << 16) | (tm.tm_hour << 11) | (tm.tm_min << 5) |
         (tm.tm_sec / 2);
}
#endif

#ifdef CONFIG_SETTINGS

static int settings_init() {
  settings_subsys_init();
  int ret = settings_load();
  if (ret < 0) {
    LOG_ERR("Failed to load settings: %s", strerror(-ret));
    return ret;

  } else {
    LOG_INF("Loaded settings from %s", CONFIG_SETTINGS_FILE_PATH);
  }

  return 0;
}

SYS_INIT(settings_init, APPLICATION, CONFIG_NTURT_SETTINGS_INIT_PRIORITY);

#endif  // CONFIG_SETTINGS
