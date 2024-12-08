#include "nturt/util.h"

// glibc includes
#include <stddef.h>

// zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(nturt_util, CONFIG_NTURT_LOG_LEVEL);

/* function definition -------------------------------------------------------*/
struct work_ctx* work_ctx_alloc(struct work_ctx* buf, size_t size) {
  for (size_t i = 0; i < size; i++) {
    if (!k_work_is_pending(&buf[i].work)) {
      return &buf[i];
    }
  }

  return NULL;
}
