#include "sdo_srv.h"

// zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

// lib includes
#include <canopennode.h>

#if !(CO_CONFIG_SDO_SRV & CO_CONFIG_FLAG_CALLBACK_PRE)
#error \
    "CONFIG_NTURT_MSG_SDO_SRV requires CO_CONFIG_FLAG_CALLBACK_PRE to be set" \
"in CO_CONFIG_SDO_SRV"
#endif

LOG_MODULE_REGISTER(nturt_msg_sdo_srv, CONFIG_NTURT_LOG_LEVEL);

/* static function declaration -----------------------------------------------*/
/// @brief Callback function when receiving CANopen SDO server object.
static void sdo_srv_cb(void *arg);

/// @brief Bottom half of @ref sdo_srv_cb.
static void sdo_srv_work(struct k_work *work);

/* function definition -------------------------------------------------------*/
int sdo_srv_init(struct sdo_srv *sdo_srv, struct canopen *co) {
  sdo_srv->co = co;
  k_work_init(&sdo_srv->work, sdo_srv_work);

  CO_SDOserver_initCallbackPre(co->CO->SDOserver, sdo_srv, sdo_srv_cb);

  return 0;
}

/* static function definition ------------------------------------------------*/
static void sdo_srv_cb(void *arg) {
  struct sdo_srv *sdo_srv = arg;

  k_work_submit(&sdo_srv->work);
}

static void sdo_srv_work(struct k_work *work) {
  struct sdo_srv *sdo_srv = CONTAINER_OF(work, struct sdo_srv, work);

  CO_NMT_internalState_t NMTstate =
      CO_NMT_getInternalState(sdo_srv->co->CO->NMT);
  if (NMTstate != CO_NMT_PRE_OPERATIONAL && NMTstate != CO_NMT_OPERATIONAL) {
    return;
  }

  CO_SDOserver_process(sdo_srv->co->CO->SDOserver, true, 0, NULL);
}
