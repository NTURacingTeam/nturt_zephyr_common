#include "nturt/sys/sys.h"

// glibc includes
#include <stdbool.h>
#include <string.h>

// zephyr includes
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/sys/iterable_sections.h>
#include <zephyr/sys/poweroff.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(nturt_sys, CONFIG_NTURT_SYS_LOG_LEVEL);

/* macro ---------------------------------------------------------------------*/
#define RESET_TIMEOUT K_SECONDS(2)

#define RESET_SOUND_COUNT 5

/* type ----------------------------------------------------------------------*/
struct sys_ctx {
  struct k_work_q work_q;
};

/* static function declaration -----------------------------------------------*/
static void shutdown();
static void reset();

static int init();

static void reset_cb(struct k_timer *timer);

/* static variable -----------------------------------------------------------*/
#ifdef CONFIG_NTURT_SYS_REBOOT_SOUND
static const struct gpio_dt_spec buzzer =
    GPIO_DT_SPEC_GET(DT_CHOSEN(nturt_buzzer), gpios);
#endif

static struct sys_ctx g_ctx;

SYS_INIT(init, APPLICATION, CONFIG_NTURT_SYS_INIT_PRIORITY);

static K_THREAD_STACK_DEFINE(nturt_work_q_stack,
                             CONFIG_NTURT_WORKQUEUE_STACK_SIZE);

static K_TIMER_DEFINE(reset_timer, reset_cb, NULL);

/* function definition -------------------------------------------------------*/
int sys_work_submit(struct k_work *work) {
  return k_work_submit_to_queue(&g_ctx.work_q, work);
}

int sys_work_schedule(struct k_work_delayable *dwork, k_timeout_t delay) {
  return k_work_schedule_for_queue(&g_ctx.work_q, dwork, delay);
}

int sys_work_reschedule(struct k_work_delayable *dwork, k_timeout_t delay) {
  return k_work_reschedule_for_queue(&g_ctx.work_q, dwork, delay);
}

void sys_shutdown() {
  LOG_INF("system shutdown");
  shutdown();

  if (IS_ENABLED(CONFIG_POWEROFF)) {
    sys_poweroff();
  } else {
    while (true) {
      // spin
    }
  }
}

void sys_reset() {
  k_timer_start(&reset_timer, RESET_TIMEOUT, K_FOREVER);

  LOG_INF("System reset");
  shutdown();

  k_timer_stop(&reset_timer);

  reset();
}

/* static function definition ------------------------------------------------*/
static void shutdown() {
  STRUCT_SECTION_FOREACH(sys_shutdown_callback, cb) {
    cb->handler(cb->user_data);
  }

  log_panic();
}

static void reset() {
  irq_lock();

#ifdef CONFIG_NTURT_SYS_REBOOT_SOUND
  for (int i = 0; i < RESET_SOUND_COUNT; i++) {
    gpio_pin_set_dt(&buzzer, true);
    k_busy_wait(100 * 1000);
    gpio_pin_set_dt(&buzzer, false);

    if (i != RESET_SOUND_COUNT - 1) {
      k_busy_wait(100 * 1000);
    }
  }
#endif  // CONFIG_NTURT_SYS_REBOOT_SOUND

  sys_reboot(SYS_REBOOT_COLD);
}

static int init() {
  struct k_work_queue_config config = {
      .name = "nturt_work_q",
  };

  k_work_queue_init(&g_ctx.work_q);
  k_work_queue_start(&g_ctx.work_q, nturt_work_q_stack,
                     K_THREAD_STACK_SIZEOF(nturt_work_q_stack),
                     CONFIG_NTURT_WORKQUEUE_THREAD_PRIORITY, &config);
  return 0;
}

static void reset_cb(struct k_timer *timer) {
  (void)timer;

  reset();
}

#ifdef CONFIG_NTURT_SYS_REBOOT_SOUND

static int gpio_init() {
  if (!device_is_ready(buzzer.port)) {
    LOG_ERR("Buzzer device not ready");
    return -ENODEV;
  }

  int ret = gpio_pin_configure_dt(&buzzer, GPIO_OUTPUT_INACTIVE);
  if (ret < 0) {
    LOG_ERR("Failed to configure buzzer: %s", strerror(-ret));
    return ret;
  }

  return 0;
}

// use the same init priority as the LEDs since they are used in the same way
SYS_INIT(gpio_init, POST_KERNEL, CONFIG_LED_INIT_PRIORITY);

#endif  // CONFIG_NTURT_SYS_REBOOT_SOUND
