#include "nturt/err.h"

// glibc includes
#include <stdbool.h>
#include <stddef.h>
#include <sys/queue.h>

// zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/hash_map.h>

// project includes
#include "nturt/sys/util.h"

LOG_MODULE_REGISTER(nturt_err, CONFIG_NTURT_LOG_LEVEL);

/* types ---------------------------------------------------------------------*/
struct context {
  struct k_spinlock lock;

  struct err_list errors;
};

/* static function declaration -----------------------------------------------*/
/// @brief Initialization function for error module.
static int init();

static struct err *err_get(uint32_t errcode);

static void err_notify(struct err *err);

static void err_log(struct err *err);

/* static varaibles ----------------------------------------------------------*/
static struct context context = {
    .errors = TAILQ_HEAD_INITIALIZER(context.errors),
};

SYS_INIT(init, APPLICATION, CONFIG_NTURT_ERR_INIT_PRIORITY);

SYS_HASHMAP_DEFINE_STATIC(err_map);

/* function definition -------------------------------------------------------*/
int err_report(uint32_t errcode, bool set) {
  struct err *err = err_get(errcode);
  if (err == NULL) {
    return -ENOENT;
  }

  if (!(err->flags | ERR_FLAG_SEV_MASK)) {
    return 0;
  }

  struct err err_copy;

  k_spinlock_key_t key = k_spin_lock(&context.lock);

  bool already_set = err->flags & ERR_FLAG_SET;

  if (!XOR(set, already_set)) {
    k_spin_unlock(&context.lock, key);
    return 0;
  }

  if (set) {
    TAILQ_INSERT_HEAD(&context.errors, err, next);
    err->flags |= ERR_FLAG_SET;

  } else {
    TAILQ_REMOVE(&context.errors, err, next);
    err->flags &= ~ERR_FLAG_SET;
  }

  // copy to prevent race conditions
  err_copy = *err;

  k_spin_unlock(&context.lock, key);

  err_notify(&err_copy);
  err_log(&err_copy);

  return 0;
}

/* static function definition ------------------------------------------------*/
static int init() {
  int ret;

  STRUCT_SECTION_FOREACH(err, err) {
    ret = sys_hashmap_insert(&err_map, err->errcode, (uintptr_t)err, NULL);
    if (ret < 0) {
      LOG_ERR("err_map insert failed: %s", strerror(-ret));
      return ret;

    } else if (ret == 0) {
      LOG_ERR("Error code 0x%x already exists", err->errcode);
      return -EEXIST;
    }
  }

  return 0;
}

static struct err *err_get(uint32_t errcode) {
  uint64_t value;
  if (!sys_hashmap_get(&err_map, errcode, &value)) {
    return NULL;
  }

  return (struct err *)(uintptr_t)value;
}

static void err_notify(struct err *err) {
  STRUCT_SECTION_FOREACH(err_callback, callback) {
    bool match = true;

    for (struct err_filter *filter = callback->filters;
         match && filter->type != ERR_FILTER_INVALID; filter++) {
      match = false;

      for (size_t i = 0; !match && i < filter->size; i++) {
        switch (filter->type) {
          case ERR_FILTER_CODE:
            if (filter->errcodes[i] == err->errcode) {
              match = true;
            }

            break;

          case ERR_FILTER_SEV:
            if (filter->serverities[i] == (err->flags & ERR_FLAG_SEV_MASK)) {
              match = true;
            }

            break;

          default:
            break;
        }
      }
    }

    if (match) {
      callback->handler(err->errcode, err->flags & ERR_FLAG_SET,
                        callback->user_data);
    }
  }
}

static void err_log(struct err *err) {
  if (err->flags & ERR_FLAG_SET) {
    switch (err->flags & ERR_FLAG_SEV_MASK) {
      case ERR_SEV_INFO:
        LOG_INF("Error 0x%x set: %s", err->errcode, err->desc);
        break;

      case ERR_SEV_WARN:
        LOG_WRN("Error 0x%x set: %s", err->errcode, err->desc);
        break;

      case ERR_SEV_FATAL:
        LOG_ERR("Error 0x%x set: %s", err->errcode, err->desc);
        break;

      default:
        break;
    }

  } else {
    LOG_INF("Error 0x%x cleared", err->errcode);
  }
}
