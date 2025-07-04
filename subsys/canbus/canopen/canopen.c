// glibc includes
#include <stddef.h>

// zephyr includes
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/sys/iterable_sections.h>
#include <zephyr/sys/util.h>
#include <zephyr/zbus/zbus.h>

// canopennode includes
#include <canopennode.h>

// project includes
#include "nturt/canbus/canopen.h"

BUILD_ASSERT(CONFIG_NTURT_CANOPEN_INIT_PRIORITY >
             CONFIG_CANOPENNODE_INIT_PRIORITY);

LOG_MODULE_REGISTER(nturt_canopen, CONFIG_NTURT_LOG_LEVEL);

/* static function definition ------------------------------------------------*/
static int od_init();

/* static variable -----------------------------------------------------------*/
SYS_INIT(od_init, APPLICATION, CONFIG_NTURT_CANOPEN_OD_INIT_PRIORITY);

/* function definition -------------------------------------------------------*/
void canopen_od_agg_publish(const void *data, void *user_data) {
  const struct zbus_channel *chan = user_data;

  struct msg_header *header = (struct msg_header *)data;
  msg_header_init(header);

  int ret;
  ret = zbus_chan_pub(chan, header, K_MSEC(5));
  if (ret < 0) {
    LOG_ERR("Failed to publish data: %s", strerror(-ret));
  }
}

/* static function definition ------------------------------------------------*/
static int od_init() {
  STRUCT_SECTION_FOREACH(canopen_od_init, init) {
    OD_entry_t *entry = OD_find(OD, init->idx);
    __ASSERT(entry != NULL, "OD entry 0x%04X does not exist", init->idx);

    OD_extension_init(entry, &init->ext);
  }

  return 0;
}
