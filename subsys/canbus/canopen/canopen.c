// glibc includes
#include <stddef.h>
#include <stdint.h>

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
// OD must be initialized before canopen is initialized in zephyr since PDOs
// will not use read/write callbacks registered afterwards.
SYS_INIT(od_init, APPLICATION, CONFIG_NTURT_CANOPEN_OD_INIT_PRIORITY);

/* function definition -------------------------------------------------------*/
#ifdef CONFIG_NTURT_CANOPEN_TM
void canopen_tm_publish(uint32_t addr, const void *data, size_t size,
                        void *user_data) {
  (void)user_data;

  uint16_t idx = addr >> 8;
  uint8_t sub = addr & 0xFF;

  OD_entry_t *entry = OD_find(OD, idx);
  __ASSERT(entry != NULL, "OD entry 0x%04X does not exist", idx);

  ODR_t ret = OD_set_value(entry, sub, (void *)data, size, false);
  __ASSERT(ret == ODR_OK,
           "Failed to set OD value, likely due to sub-index 0x%02X does not "
           "exist or size %zu mismatch",
           sub, size);

  OD_requestTPDO(OD_getFlagsPDO(entry), sub);
}
#endif  // CONFIG_NTURT_CANOPEN_TM

#ifdef CONFIG_NTURT_CANOPEN_MSG
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
#endif  // CONFIG_NTURT_CANOPEN_MSG

/* static function definition ------------------------------------------------*/
static int od_init() {
  // Currently TPDOs may define multiple `canopen_od_init` fo the same entry,
  // which is fine since OD_extension_init uses the last one. However if one
  // entry is used for both TPDO and RPDO, other mechanisms must be used to
  // ensure the `canopen_od_init` from RPDO is the last one.
  STRUCT_SECTION_FOREACH(canopen_od_init, init) {
    OD_entry_t *entry = OD_find(OD, init->idx);
    __ASSERT(entry != NULL, "OD entry 0x%04X does not exist", init->idx);

    OD_extension_init(entry, &init->extension);
  }

  return 0;
}
