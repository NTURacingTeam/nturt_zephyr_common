// zephyr includes
#include <zephyr/fs/fs.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>

#if IS_ENABLED(CONFIG_NTURT_SYS_FS_FAT_FS)
#include <ff.h>
#elif IS_ENABLED(CONFIG_NTURT_SYS_FS_LITTLE_FS)
#include <zephyr/fs/littlefs.h>
#endif

// project includes
#include "nturt/sys.h"

LOG_MODULE_REGISTER(nturt_sys_fs);

/* static function declaration -----------------------------------------------*/
/// @brief Initialization function for NTURT file system module.
static int init();

/* static varaible -----------------------------------------------------------*/
#if IS_ENABLED(CONFIG_NTURT_SYS_FS_FAT_FS)
static FATFS fatfs;
static struct fs_mount_t fs_mnt = {
    .type = FS_FATFS,
    .fs_data = &fatfs,
    .mnt_point = CONFIG_NTURT_SYS_FS_MOUNT_POINT,
};
#elif IS_ENABLED(CONFIG_NTURT_SYS_FS_LITTLE_FS)
static struct fs_littlefs lfsfs;
static struct fs_mount_t fs_mnt = {
    .type = FS_LITTLEFS,
    .fs_data = &lfsfs,
    .flags = FS_MOUNT_FLAG_USE_DISK_ACCESS,
    .storage_dev = CONFIG_NTURT_SYS_FS_SD_DISK_NAME,
    .mnt_point = CONFIG_NTURT_SYS_FS_MOUNT_POINT,
};
#endif

SYS_INIT(init, APPLICATION, CONFIG_NTURT_SYS_INIT_PRIORITY);

/* static function definition ------------------------------------------------*/
static int init(void) {
  int ret;

  ret = fs_mount(&fs_mnt);
  if (ret < 0) {
    LOG_ERR("Fail to mount %s: %s", CONFIG_NTURT_SYS_FS_SD_DISK_NAME,
            strerror(-ret));
  }

  return 0;
}
