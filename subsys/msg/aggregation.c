#include "nturt/msg/aggregation.h"

// glibc includes
#include <stdbool.h>
#include <string.h>

// zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>

LOG_MODULE_DECLARE(nturt_msg, CONFIG_NTURT_LOG_LEVEL);

/* function definition -------------------------------------------------------*/
void agg_ctrl_update_lock(struct agg_ctrl *ctrl, size_t offset) {
  uint8_t flags = ctrl->flags_map[offset];

  __ASSERT(flags & AGG_FLAG_VALID,
           "Members must declared in AGG_TYPE_DECLARE to be updated");

  if (!(flags & AGG_FLAG_OPTIONAL)) {
    ctrl->updated |= BIT(flags & AGG_FLAG_IDX_MASK);
  }

  // cold start
  if (k_timer_remaining_ticks(&ctrl->period_timer) == 0 &&
      !k_work_delayable_is_pending(&ctrl->work)) {
    k_work_schedule(&ctrl->work, ctrl->watermark);
  }

  // fully updated after minimum separation time
  if (k_timer_remaining_ticks(&ctrl->early_timer) == 0 &&
      ctrl->updated == ctrl->fully_updated) {
    k_timer_start(&ctrl->period_timer, ctrl->period, K_FOREVER);
    k_timer_start(&ctrl->early_timer, ctrl->min_separation, K_FOREVER);
    int ret = k_work_reschedule(&ctrl->work, K_NO_WAIT);
    if (ret == 0 || ret == 2) {
      LOG_WRN("Publishing %s took longer than minimum separation time",
              ctrl->name);
    }
  }
}

void agg_period_timer_cb(struct k_timer *timer) {
  struct agg_ctrl *ctrl = CONTAINER_OF(timer, struct agg_ctrl, period_timer);

  K_SPINLOCK(&ctrl->lock) { k_work_schedule(&ctrl->work, ctrl->watermark); }
}

void agg_early_timer_cb(struct k_timer *timer) {
  struct agg_ctrl *ctrl = CONTAINER_OF(timer, struct agg_ctrl, early_timer);

  K_SPINLOCK(&ctrl->lock) {
    if (ctrl->updated == ctrl->fully_updated) {
      k_timer_start(&ctrl->period_timer, ctrl->period, K_FOREVER);
      k_timer_start(&ctrl->early_timer, ctrl->min_separation, K_FOREVER);
      int ret = k_work_reschedule(&ctrl->work, K_NO_WAIT);
      if (ret == 0 || ret == 2) {
        LOG_WRN("Publishing %s took longer than minimum separation time",
                ctrl->name);
      }
    }
  }
}

void agg_work_cb(struct k_work *work) {
  struct k_work_delayable *dwork = k_work_delayable_from_work(work);
  struct agg_ctrl *ctrl = CONTAINER_OF(dwork, struct agg_ctrl, work);

  k_spinlock_key_t key = k_spin_lock(&ctrl->lock);

  if (ctrl->updated == 0) {
    // no member has been updated, stop publishing
    k_spin_unlock(&ctrl->lock, key);
    return;

  } else if (ctrl->updated != ctrl->fully_updated) {
    // not fully updated means this is late publishing
    // timers are not started right after watermark due to the inevitable
    // delay of work queue
    k_timer_start(&ctrl->period_timer, ctrl->period, K_FOREVER);
    k_timer_start(&ctrl->early_timer, ctrl->min_separation, K_FOREVER);

    LOG_WRN(
        "Watermark reached while the data of %s is not fully updated, "
        "publishing anyway",
        ctrl->name);
  }

  // there will be no race condition for pub_data since work is run in series
  // and the next memcpy will not happen until the previous publish is done
  // accessing pub_data
  memcpy(ctrl->pub_data, ctrl->data, ctrl->data_size);
  ctrl->updated = 0;

  k_spin_unlock(&ctrl->lock, key);

  ctrl->publish(ctrl->pub_data, ctrl->user_data);
}
