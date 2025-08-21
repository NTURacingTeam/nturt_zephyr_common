#include "nturt/err/err.h"

// glibc includes
#include <stdbool.h>
#include <stddef.h>
#include <sys/queue.h>

// zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/sys/hash_map.h>
#include <zephyr/sys/iterable_sections.h>
#include <zephyr/sys/util.h>

// project includes
#include "nturt/sys/util.h"

LOG_MODULE_REGISTER(nturt_err, CONFIG_NTURT_ERR_LOG_LEVEL);

/* type ----------------------------------------------------------------------*/
struct err_ctx {
  bool cb_dispatched;

  struct k_spinlock lock;

  /** List of current errors. */
  struct err_list errors;
};

/* static function declaration -----------------------------------------------*/
static struct err *err_get(uint32_t errcode);
static void err_notify(struct err *err);
static void err_log(struct err *err);

static int init();
static int cb_init();

/* static variable ----------------------------------------------------------*/
/// @brief State module context.
static struct err_ctx g_ctx = {
    .cb_dispatched = false,
    .errors = TAILQ_HEAD_INITIALIZER(g_ctx.errors),
};

SYS_HASHMAP_DEFINE_STATIC(g_err_map);

SYS_INIT(init, APPLICATION, CONFIG_NTURT_ERR_INIT_PRIORITY);
SYS_INIT(cb_init, APPLICATION, CONFIG_NTURT_ERR_CB_DISPATCH_PRIORITY);

/* exported variable ---------------------------------------------------------*/
const struct err_list *__err_errors = &g_ctx.errors;

/* function definition -------------------------------------------------------*/
void err_report(uint32_t errcode, bool set) {
  struct err *err = err_get(errcode);

  if (err->flags & ERR_FLAG_DISABLED) {
    return;
  }

  struct err err_copy;

  k_spinlock_key_t key = k_spin_lock(&g_ctx.lock);

  bool already_set = err->flags & ERR_FLAG_SET;

  if (!XOR(set, already_set)) {
    k_spin_unlock(&g_ctx.lock, key);
    return;
  }

  if (set) {
    TAILQ_INSERT_HEAD(&g_ctx.errors, err, next);
    err->flags |= ERR_FLAG_SET;

  } else {
    TAILQ_REMOVE(&g_ctx.errors, err, next);
    err->flags &= ~ERR_FLAG_SET;
  }

  // copy to prevent race conditions
  err_copy = *err;

  k_spin_unlock(&g_ctx.lock, key);

  if (g_ctx.cb_dispatched) {
    err_notify(&err_copy);
    err_log(&err_copy);
  }
}

bool err_is_set(uint32_t errcode) {
  return err_get(errcode)->flags & ERR_FLAG_SET;
}

/* static function definition ------------------------------------------------*/
static struct err *err_get(uint32_t errcode) {
  uint64_t value;
  if (!sys_hashmap_get(&g_err_map, errcode, &value)) {
    __ASSERT(0, "Error code 0x%X does not exist", errcode);
  }

  return UINT_TO_POINTER(value);
}

static void err_notify(struct err *err) {
  STRUCT_SECTION_FOREACH(err_callback, callback) {
    bool match = true;

    for (struct err_filter *filter = callback->filters;
         match && filter->type != ERR_FILTER_TYPE_INVALID; filter++) {
      match = false;

      for (size_t i = 0; !match && i < filter->size; i++) {
        switch (filter->type) {
          case ERR_FILTER_TYPE_CODE:
            if (filter->errcodes[i] == err->errcode) {
              match = true;
            }

            break;

          case ERR_FILTER_TYPE_SEV:
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
        LOG_INF("Error 0x%X (%s) set: %s", err->errcode, err->name, err->desc);
        break;

      case ERR_SEV_WARN:
        LOG_WRN("Error 0x%X (%s) set: %s", err->errcode, err->name, err->desc);
        break;

      case ERR_SEV_ERROR:
        LOG_ERR("Error 0x%X (%s) set: %s", err->errcode, err->name, err->desc);
        break;

      case ERR_SEV_FATAL:
        LOG_ERR(">>> FATAL ERROR 0x%X (%s) set: %s", err->errcode, err->name,
                err->desc);
        break;

      default:
        break;
    }

  } else {
    LOG_INF("Error 0x%X (%s) cleared", err->errcode, err->name);
  }
}

static int init() {
  int ret;

  STRUCT_SECTION_FOREACH(err, err) {
    __ASSERT(IS_POWER_OF_TWO(err->flags & ERR_FLAG_SEV_MASK),
             "Error must have one and only one severity.");

    ret = sys_hashmap_insert(&g_err_map, err->errcode, POINTER_TO_UINT(err),
                             NULL);

    __ASSERT(ret != 0, "Errors must not have the same error code: 0x%X",
             err->errcode);

    if (ret < 0) {
      LOG_ERR("g_err_map insert failed: %s", strerror(-ret));
      return ret;
    }

    if (!(err->flags & ERR_FLAG_DISABLED) && err->flags & ERR_FLAG_SET) {
      TAILQ_INSERT_HEAD(&g_ctx.errors, err, next);
    }
  }

  return 0;
}

static int cb_init() {
  g_ctx.cb_dispatched = true;

  struct err *err;
  ERR_FOREACH_SET(err) {
    err_notify(err);
    err_log(err);
  }

  return 0;
}
