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
#include <zephyr/sys/reboot.h>

LOG_MODULE_REGISTER(nturt_sys, CONFIG_NTURT_SYS_LOG_LEVEL);

#ifdef CONFIG_NTURT_SYS_REBOOT_SOUND

static const struct gpio_dt_spec buzzer =
    GPIO_DT_SPEC_GET(DT_CHOSEN(nturt_buzzer), gpios);

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

/* macro ---------------------------------------------------------------------*/
#define RESET_TIMEOUT K_SECONDS(2)

#define RESET_SOUND_COUNT 5

/* static function declaration -----------------------------------------------*/
static void reset();

static void reset_cb(struct k_timer *timer);

/* static variable -----------------------------------------------------------*/
static K_TIMER_DEFINE(reset_timer, reset_cb, NULL);

/* function definition -------------------------------------------------------*/
void sys_reset() {
  k_timer_start(&reset_timer, RESET_TIMEOUT, K_FOREVER);

  LOG_INF("System reset");
  log_panic();

  k_timer_stop(&reset_timer);

  reset();
}

/* static function definition ------------------------------------------------*/
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

static void reset_cb(struct k_timer *timer) {
  (void)timer;

  reset();
}
