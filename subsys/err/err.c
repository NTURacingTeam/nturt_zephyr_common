#include "nturt/err.h"

// glibc includes
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

// zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>

LOG_MODULE_REGISTER(nturt_err, CONFIG_NTURT_LOG_LEVEL);

/* types ---------------------------------------------------------------------*/
struct err {
  /// @brief Atomic variable to store errors.
  atomic_t errors;

  /// @brief Initialization flag.
  bool init;
};

/* static function declaration -----------------------------------------------*/
/// @brief Initialization function for states module.
static int init();

/* static varaibles ----------------------------------------------------------*/
static struct err err = {
    .errors = ATOMIC_INIT(0),
    .init = false,
};

SYS_INIT(init, APPLICATION, CONFIG_NTURT_ERR_INIT_PRIORITY);

ZBUS_CHAN_DEFINE(err_chan, err_t, NULL, NULL, ZBUS_OBSERVERS_EMPTY,
                 ZBUS_MSG_INIT(0));

/* function definition -------------------------------------------------------*/
void err_set_errors(err_t errors, bool set) {
  if (set) {
    atomic_val_t err_org = atomic_or(&err.errors, errors);
    if ((err_org & errors) == errors) {
      return;
    }

  } else {
    atomic_val_t err_org = atomic_and(&err.errors, ~errors);
    if ((err_org & errors) == 0) {
      return;
    }
  }

  if (err.init) {
    if (set) {
      LOG_ERR("Errors set: 0x%X, current errors: 0x%X", errors,
              err_get_errors());
      errors |= ERR_CODE_SET;
    } else {
      LOG_INF("Errors cleared: 0x%X, current errors: 0x%X", errors,
              err_get_errors());
      errors |= ERR_CODE_CLEAR;
    }

    int ret = zbus_chan_pub(&err_chan, &errors, K_FOREVER);
    if (ret < 0) {
      LOG_ERR("Failed to publish errors: %s", strerror(-ret));
    }
  }
}

err_t err_get_errors() { return atomic_get(&err.errors); }

/* static function definition ------------------------------------------------*/
static int init() {
  err_t errors = err_get_errors();
  if (errors != 0) {
    LOG_ERR("Set initial errors: 0x%X", errors);

    // publish to zbus
    err_set_errors(errors, true);
  }

  err.init = true;

  return 0;
}
