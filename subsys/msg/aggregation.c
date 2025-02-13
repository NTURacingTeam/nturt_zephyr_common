#include "nturt/msg/aggregation.h"

// glibc includes
#include <stdbool.h>

// zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>

LOG_MODULE_DECLARE(nturt_msg, CONFIG_NTURT_LOG_LEVEL);

/* static function declaration -----------------------------------------------*/
/**
 * @brief Timer callback function for periodic publishing.
 *
 * @param[in] timer Timer.
 */
static void msg_agg_timer_cb(struct k_timer *timer);

/**
 * @brief Work callback function for the bottom half of publishing.
 *
 * @param[in] work Work.
 */
static void msg_agg_work_cb(struct k_work *work);

/* function definition -------------------------------------------------------*/
void msg_agg_ctrl_init(struct msg_agg_ctrl *ctrl) {
  k_timer_init(&ctrl->timer, msg_agg_timer_cb, NULL);
  k_work_init_delayable(&ctrl->work, msg_agg_work_cb);
}

void msg_agg_ctrl_start(struct msg_agg_ctrl *ctrl, k_timeout_t period,
                        k_timeout_t watermark) {
  __ASSERT(period.ticks > watermark.ticks,
           "Watermark must be less than period");

  ctrl->watermark = watermark;
  k_timer_start(&ctrl->timer, period, period);
}

void msg_agg_ctrl_stop(struct msg_agg_ctrl *ctrl) {
  k_timer_stop(&ctrl->timer);
  k_work_cancel_delayable(&ctrl->work);
}

void msg_agg_ctrl_update_lock(struct msg_agg_ctrl *ctrl, size_t offset) {
  int idx = ctrl->map[offset] - 1;
  __ASSERT(idx >= 0, "Member not declared in MSG_AGG_TYPE_DECLARE");

  ctrl->updated |= BIT(idx);

  if (ctrl->updated == BIT_MASK(ctrl->num_member) &&
      k_work_delayable_is_pending(&ctrl->work)) {
    int ret = k_work_reschedule(&ctrl->work, K_NO_WAIT);
    if (ret == 0 || ret == 2) {
      LOG_WRN("Publishing %s took longer than the period", ctrl->name);
    }
  }
}

/* static function definition ------------------------------------------------*/
static void msg_agg_timer_cb(struct k_timer *timer) {
  struct msg_agg_ctrl *ctrl = CONTAINER_OF(timer, struct msg_agg_ctrl, timer);

  int ret = 1;
  K_SPINLOCK(&ctrl->lock) {
    if (ctrl->updated == BIT_MASK(ctrl->num_member)) {
      ret = k_work_schedule(&ctrl->work, K_NO_WAIT);

    } else {
      ret = k_work_schedule(&ctrl->work, ctrl->watermark);
    }
  }

  if (ret == 0 || ret == 2) {
    LOG_WRN("Publishing %s took longer than the period", ctrl->name);
  }
}

static void msg_agg_work_cb(struct k_work *work) {
  struct k_work_delayable *dwork = k_work_delayable_from_work(work);
  struct msg_agg_ctrl *ctrl = CONTAINER_OF(dwork, struct msg_agg_ctrl, work);

  bool fully_updated = true;
  K_SPINLOCK(&ctrl->lock) {
    if (ctrl->updated != BIT_MASK(ctrl->num_member)) {
      fully_updated = false;
    }

    memcpy(ctrl->pub_data, ctrl->data, ctrl->data_size);
    ctrl->updated = 0;
  }

  if (!fully_updated) {
    LOG_WRN(
        "Watermark reached while the data of %s is not fully updated, "
        "publishing anyway",
        ctrl->name);
  }

  ctrl->publish(ctrl->pub_data, ctrl->user_data);
}
