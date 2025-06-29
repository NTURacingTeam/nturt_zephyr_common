#include "nturt/sys/sys.h"

// glibc includes
#include <stdbool.h>
#include <string.h>

// zephyr includes
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/sys/reboot.h>

LOG_MODULE_REGISTER(nturt_sys, CONFIG_NTURT_SYS_LOG_LEVEL);

/* static function declaration -----------------------------------------------*/
static int init();

/* static varaible -----------------------------------------------------------*/
SYS_INIT(init, APPLICATION, CONFIG_NTURT_SYS_INIT_PRIORITY);

#ifdef CONFIG_NTURT_SYS_REBOOT_SOUND
static const struct gpio_dt_spec buzzer =
    GPIO_DT_SPEC_GET(DT_CHOSEN(nturt_buzzer), gpios);
#endif  // CONFIG_NTURT_SYS_REBOOT_SOUND

/* function definition -------------------------------------------------------*/
void sys_reset() {
  k_sched_lock();

  LOG_INF("System reset");
  log_panic();

#ifdef CONFIG_NTURT_SYS_REBOOT_SOUND
  gpio_pin_set_dt(&buzzer, true);
  k_busy_wait(300 * 1000);
  gpio_pin_set_dt(&buzzer, false);
  k_busy_wait(100 * 1000);
  gpio_pin_set_dt(&buzzer, true);
  k_busy_wait(100 * 1000);
  gpio_pin_set_dt(&buzzer, false);
  k_busy_wait(100 * 1000);
  gpio_pin_set_dt(&buzzer, true);
  k_busy_wait(100 * 1000);
  gpio_pin_set_dt(&buzzer, false);
#endif  // CONFIG_NTURT_SYS_REBOOT_SOUND

  sys_reboot(SYS_REBOOT_COLD);
}

/* static function definition ------------------------------------------------*/
static int init() {
#ifdef CONFIG_NTURT_SYS_REBOOT_SOUND
  if (!device_is_ready(buzzer.port)) {
    LOG_ERR("Buzzer device not ready");
    return -ENODEV;
  }

  int ret = gpio_pin_configure_dt(&buzzer, GPIO_OUTPUT_INACTIVE);
  if (ret < 0) {
    LOG_ERR("Failed to configure buzzer: %s", strerror(-ret));
    return ret;
  }
#endif  // CONFIG_NTURT_SYS_REBOOT_SOUND

  return 0;
}
