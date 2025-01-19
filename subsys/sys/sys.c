#include "nturt/sys.h"

// zephyr includes
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/sys/reboot.h>

LOG_MODULE_REGISTER(sys);

/* static varaible -----------------------------------------------------------*/
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
  gpio_pin_set_dt(&buzzer, 1);
  k_busy_wait(300 * 1000);
  gpio_pin_set_dt(&buzzer, 0);
  k_busy_wait(100 * 1000);
  gpio_pin_set_dt(&buzzer, 1);
  k_busy_wait(100 * 1000);
  gpio_pin_set_dt(&buzzer, 0);
  k_busy_wait(100 * 1000);
  gpio_pin_set_dt(&buzzer, 1);
  k_busy_wait(100 * 1000);
  gpio_pin_set_dt(&buzzer, 0);
#endif  // CONFIG_NTURT_SYS_REBOOT_SOUND

  sys_reboot(SYS_REBOOT_COLD);
}
