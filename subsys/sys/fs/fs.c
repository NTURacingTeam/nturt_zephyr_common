// glibc includes
#include <stdint.h>

// zephyr includes
#include <zephyr/fs/fs.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>

#ifdef CONFIG_NTURT_FS_FAT_FS
#include <ff.h>
#ifdef CONFIG_NTURT_FS_FAT_FS_HAS_RTC
#include <zephyr/posix/time.h>
#endif
#elif defined(CONFIG_NTURT_FS_LITTLE_FS)
#include <zephyr/fs/littlefs.h>
#endif

// project includes
#include "nturt/sys/sys.h"

LOG_MODULE_REGISTER(nturt_fs);

/* static function declaration -----------------------------------------------*/
/// @brief Initialization function for NTURT file system module.
static int init();

/* static varaible -----------------------------------------------------------*/
#ifdef CONFIG_NTURT_FS_FAT_FS
static FATFS fatfs;
static struct fs_mount_t fs_mnt = {
    .type = FS_FATFS,
    .fs_data = &fatfs,
    .mnt_point = CONFIG_NTURT_FS_MOUNT_POINT,
};
#elif defined(CONFIG_NTURT_FS_LITTLE_FS)
static struct fs_littlefs lfsfs;
static struct fs_mount_t fs_mnt = {
    .type = FS_LITTLEFS,
    .fs_data = &lfsfs,
    .flags = FS_MOUNT_FLAG_USE_DISK_ACCESS,
    .storage_dev = CONFIG_NTURT_FS_SD_DISK_NAME,
    .mnt_point = CONFIG_NTURT_FS_MOUNT_POINT,
};
#endif

SYS_INIT(init, APPLICATION, CONFIG_NTURT_SYS_INIT_PRIORITY);

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

/* static function definition ------------------------------------------------*/
static int init() {
  int ret;

  ret = fs_mount(&fs_mnt);
  if (ret < 0) {
    LOG_ERR("Fail to mount %s: %s", CONFIG_NTURT_FS_SD_DISK_NAME,
            strerror(-ret));
  } else {
    LOG_INF("Mounted %s at %s", CONFIG_NTURT_FS_SD_DISK_NAME,
            CONFIG_NTURT_FS_MOUNT_POINT);
  }

  return 0;
}
