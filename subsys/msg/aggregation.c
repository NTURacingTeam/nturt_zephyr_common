#include "nturt/msg/aggregation.h"

// glibc includes
#include <stdbool.h>
#include <string.h>

// zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>

// nturt includes
#include <nturt/sys/util.h>

LOG_MODULE_DECLARE(nturt_msg, CONFIG_NTURT_MSG_LOG_LEVEL);

/* function definition -------------------------------------------------------*/
void agg_update(struct agg *agg, int idx) {
  uint8_t flag = agg->member_flags[idx];
  if (flag & AGG_MEMBER_FLAG_IGNORED) {
    return;
  }

  K_SPINLOCK(&agg->lock) {
    agg->updated |= BIT(idx);

    // cold start
    if (k_timer_remaining_ticks(&agg->period_timer) == 0 &&
        !k_work_delayable_is_pending(&agg->work)) {
      k_work_schedule(&agg->work, agg->watermark);
    }

    // fully updated after minimum separation time has elapsed
    if (k_timer_remaining_ticks(&agg->early_timer) == 0 &&
        IS_MASK_SET(agg->updated, agg->fully_updated)) {
      k_timer_start(&agg->period_timer, agg->period, K_FOREVER);
      k_timer_start(&agg->early_timer, agg->min_separation, K_FOREVER);

      int ret = k_work_reschedule(&agg->work, K_NO_WAIT);
      if (ret == 0 || ret == 2) {
        LOG_WRN("Publishing %s took longer than minimum separation time",
                agg->name);
      }
    }
  }
}

void agg_period_timer_cb(struct k_timer *timer) {
  struct agg *agg = CONTAINER_OF(timer, struct agg, period_timer);

  K_SPINLOCK(&agg->lock) { k_work_schedule(&agg->work, agg->watermark); }
}

void agg_early_timer_cb(struct k_timer *timer) {
  struct agg *agg = CONTAINER_OF(timer, struct agg, early_timer);

  K_SPINLOCK(&agg->lock) {
    // early publish
    if (IS_MASK_SET(agg->updated, agg->fully_updated)) {
      k_timer_start(&agg->period_timer, agg->period, K_FOREVER);
      k_timer_start(&agg->early_timer, agg->min_separation, K_FOREVER);
      int ret = k_work_reschedule(&agg->work, K_NO_WAIT);
      if (ret == 0 || ret == 2) {
        LOG_WRN("Publishing %s took longer than minimum separation time",
                agg->name);
      }
    }
  }
}

void agg_work_cb(struct k_work *work) {
  struct k_work_delayable *dwork = k_work_delayable_from_work(work);
  struct agg *agg = CONTAINER_OF(dwork, struct agg, work);

  k_spinlock_key_t key = k_spin_lock(&agg->lock);

  if (!(agg->flag & AGG_FLAG_ALWAYS_PUBLISH) && agg->updated == 0) {
    // no member has been updated, stop publishing
    k_spin_unlock(&agg->lock, key);

    LOG_DBG("No member has been updated, stop publishing %s", agg->name);
    return;

  } else if (!IS_MASK_SET(agg->updated, agg->fully_updated)) {
    // not fully updated means this is late publishing
    // timers may not be started right after watermark has elapsed due to the
    // inevitable delay of workqueue
    k_timer_start(&agg->period_timer, agg->period, K_FOREVER);
    k_timer_start(&agg->early_timer, agg->min_separation, K_FOREVER);

    LOG_DBG(
        "Watermark reached while the data of %s is not fully updated, "
        "publishing anyway",
        agg->name);
  }

  agg->updated = 0;
  k_spin_unlock(&agg->lock, key);

  agg->publish(agg, agg->user_data);
}

void agg_typed_publish(struct agg *agg, void *user_data) {
  struct agg_typed *agg_typed = CONTAINER_OF(agg, struct agg_typed, agg);

  K_SPINLOCK(&agg_typed->lock) {
    // there will be no race condition for pub_data since work is run in series
    // and the next memcpy will not happen until the previous publish is done
    // accessing pub_data
    memcpy(agg_typed->pub_data, agg_typed->data, agg_typed->data_size);
  }

  agg_typed->publish(agg_typed->pub_data, user_data);
}
