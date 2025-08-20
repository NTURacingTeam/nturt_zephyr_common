// glibc includes
#include <stddef.h>
#include <stdint.h>

// zephyr includes
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/sys/util.h>

// canopennode includes
#include <canopennode.h>

// project includes
#include "nturt/canbus/canopen.h"
#include "nturt/err/err.h"

BUILD_ASSERT(IS_ENABLED(OD_CNT_HB_CONS),
             "OD 0x1016 must be defined to use heartbeat consumer");
BUILD_ASSERT(IS_ENABLED(CONFIG_CANOPENNODE_HB_CONS_SINGLE_CALLBACK),
             "Single callback must be enabled");

LOG_MODULE_REGISTER(nturt_canopen_hb_cons, CONFIG_NTURT_CANOPEN_LOG_LEVEL);

/* static function definition ------------------------------------------------*/
static uint32_t node_id_to_err_code(enum canopen_node_id id);

static int init();

static void mnt_changed_cb(uint8_t nodeid, uint8_t idx,
                           CO_NMT_internalState_t NMTstate, void *object);

/* static variable -----------------------------------------------------------*/
SYS_INIT(init, APPLICATION, CONFIG_NTURT_CANOPEN_INIT_PRIORITY);

ERR_DEFINE(hb_vcu, ERR_CODE_HB_VCU, ERR_SEV_FATAL,
           "VCU not operational or heartbeat lost");
ERR_DEFINE(hb_sensors, ERR_CODE_HB_SENSORS, ERR_SEV_FATAL,
           "Sensor box not operational or heartbeat lost");
ERR_DEFINE(hb_rpi, ERR_CODE_HB_RPI, ERR_SEV_WARN,
           "Raspberry Pi not operational or heartbeat lost");
ERR_DEFINE(hb_imu, ERR_CODE_HB_IMU, ERR_SEV_FATAL,
           "IMU not operational or heartbeat lost");
ERR_DEFINE(hb_acc, ERR_CODE_HB_ACC, ERR_SEV_FATAL,
           "Accumulator not operational or heartbeat lost");
ERR_DEFINE(hb_inv_fl, ERR_CODE_HB_INV_FL, ERR_SEV_FATAL,
           "Inverter FL not operational or heartbeat lost");
ERR_DEFINE(hb_inv_fr, ERR_CODE_HB_INV_FR, ERR_SEV_FATAL,
           "Inverter FR not operational or heartbeat lost");
ERR_DEFINE(hb_inv_rl, ERR_CODE_HB_INV_RL, ERR_SEV_FATAL,
           "Inverter RL not operational or heartbeat lost");
ERR_DEFINE(hb_inv_rr, ERR_CODE_HB_INV_RR, ERR_SEV_FATAL,
           "Inverter RR not operational or heartbeat lost");

/* static function definition ------------------------------------------------*/
static uint32_t node_id_to_err_code(enum canopen_node_id id) {
  switch (id) {
    case NODE_ID_VCU:
      return ERR_CODE_HB_VCU;

    case NODE_ID_SENSORS:
      return ERR_CODE_HB_SENSORS;

    case NODE_ID_RPI:
      return ERR_CODE_HB_RPI;

    case NODE_ID_IMU:
      return ERR_CODE_HB_IMU;

    case NODE_ID_ACC:
      return ERR_CODE_HB_ACC;

    case NODE_ID_INV_FL:
      return ERR_CODE_HB_INV_FL;

    case NODE_ID_INV_FR:
      return ERR_CODE_HB_INV_FR;

    case NODE_ID_INV_RL:
      return ERR_CODE_HB_INV_RL;

    case NODE_ID_INV_RR:
      return ERR_CODE_HB_INV_RR;

    default:
      __ASSERT(false, "Unknown node ID: %u", id);
  }
}

static int init() {
  // set initial errors for monitored nodes
  for (int i = 0; i < CO->HBcons->numberOfMonitoredNodes; i++) {
    CO_HBconsNode_t *node = &CO->HBcons->monitoredNodes[i];
    if (node->nodeId == 0 || node->nodeId > 127) {
      continue;
    }

    uint32_t err_code = node_id_to_err_code(node->nodeId);
    err_report(err_code, true);
  }

  // idx is ignored in single callback mode
  CO_HBconsumer_initCallbackNmtChanged(CO->HBcons, 0, NULL, mnt_changed_cb);

  return 0;
}

static void mnt_changed_cb(uint8_t nodeid, uint8_t idx,
                           CO_NMT_internalState_t NMTstate, void *object) {
  (void)object;

  err_report(node_id_to_err_code(nodeid), NMTstate != CO_NMT_OPERATIONAL);
}
