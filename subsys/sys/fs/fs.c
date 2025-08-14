#include "nturt/sys/fs.h"

// glibc includes
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

// zephyr includes
#include <zephyr/devicetree.h>
#include <zephyr/fs/fs.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>
#include <zephyr/sys/clock.h>

// project includes
#include "nturt/sys/sys.h"

LOG_MODULE_REGISTER(nturt_fs, CONFIG_NTURT_SYS_LOG_LEVEL);

/* macro ---------------------------------------------------------------------*/
#define FSTAB_UNMOUNT(node_id)                                               \
  do {                                                                       \
    FS_FSTAB_DECLARE_ENTRY(node_id);                                         \
    int ret = fs_unmount(&FS_FSTAB_ENTRY(node_id));                          \
    if (ret < 0) {                                                           \
      LOG_ERR("Failed to unmount %s: %s", FS_FSTAB_ENTRY(node_id).mnt_point, \
              strerror(-ret));                                               \
    }                                                                        \
  } while (0)

/* static function declaration -----------------------------------------------*/
static void shutdown_cb(void *user_data);

/* static variable -----------------------------------------------------------*/
SYS_SHUTDOWN_CALLBACK_DEFINE(shutdown_cb, NULL,
                             CONFIG_NTURT_FS_SHUTDOWN_PRIORITY);

/* function definition -------------------------------------------------------*/
// copied from zephyr/subsys/logging/backends/log_backend_fs.c create_log_dir()
int fs_mkdir_p(const char *path) {
  const char *next;
  const char *last = path + (strlen(path) - 1);
  char w_path[FS_MAX_PATH_LEN];
  int rc, len;
  struct fs_dir_t dir;

  fs_dir_t_init(&dir);

  /* the fist directory name is the mount point*/
  /* the firs path's letter might be meaningless `/`, let's skip it */
  next = strchr(path + 1, '/');
  if (!next) {
    return 0;
  }

  while (true) {
    next++;
    if (next > last) {
      return 0;
    }
    next = strchr(next, '/');
    if (!next) {
      next = last;
      len = last - path + 1;
    } else {
      len = next - path;
    }

    memcpy(w_path, path, len);
    w_path[len] = 0;

    rc = fs_opendir(&dir, w_path);
    if (rc) {
      /* assume directory doesn't exist */
      rc = fs_mkdir(w_path);
      if (rc) {
        break;
      }
    }
    rc = fs_closedir(&dir);
    if (rc) {
      break;
    }
  }

  return rc;
}

/* static function definition ------------------------------------------------*/
static void shutdown_cb(void *user_data) {
  (void)user_data;

#if DT_HAS_CHOSEN(nturt_fstab)
  DT_FOREACH_CHILD_STATUS_OKAY_SEP(DT_CHOSEN(nturt_fstab), FSTAB_UNMOUNT, (;));
#endif
}

#ifdef CONFIG_NTURT_FS_FAT_FS_HAS_RTC

/// @brief Get the current time in FAT file system format, declared in ff.h.
uint32_t get_fattime() {
  struct timespec ts;
  sys_clock_gettime(SYS_CLOCK_REALTIME, &ts);

  struct tm *tm = gmtime(&ts.tv_sec);
  return ((tm->tm_year - 80) << 25) | ((tm->tm_mon + 1) << 21) |
         (tm->tm_mday << 16) | (tm->tm_hour << 11) | (tm->tm_min << 5) |
         (tm->tm_sec / 2);
}

#endif  // CONFIG_NTURT_FS_FAT_FS_HAS_RTC

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
