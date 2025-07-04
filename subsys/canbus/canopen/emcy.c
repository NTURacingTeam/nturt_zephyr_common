// glibc includes
#include <stdbool.h>
#include <stdint.h>

// zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/sys/hash_map.h>
#include <zephyr/sys/iterable_sections.h>
#include <zephyr/sys/util.h>

// canopennode includes
#include <canopennode.h>

// project includes
#include "nturt/canbus/canopen.h"
#include "nturt/err/err.h"

LOG_MODULE_REGISTER(nturt_canopen_emcy, CONFIG_NTURT_CANOPEN_LOG_LEVEL);

/* type ----------------------------------------------------------------------*/
struct canopen_emcy_ctx {
  bool set;
  uint16_t ident;
  uint16_t errorCode;
  uint8_t errorRegister;
  uint8_t errorBit;
  uint32_t infoCode;

  struct k_work work;
};

/* static function definition ------------------------------------------------*/
static void emcy_cb(uint16_t ident, uint16_t errorCode, uint8_t errorRegister,
                    uint8_t errorBit, uint32_t infoCode);
static void emcy_work(struct k_work *work);

static void err_cb(uint32_t errcode, bool set, void *user_data);

static int init();

/* static variable -----------------------------------------------------------*/
struct canopen_emcy_ctx g_ctx = {
    .work = Z_WORK_INITIALIZER(emcy_work),
};

SYS_HASHMAP_DEFINE_STATIC(g_status_bit_map);
SYS_INIT(init, APPLICATION, CONFIG_NTURT_CANOPEN_INIT_PRIORITY);

ERR_DEFINE(canbus, ERR_CODE_CANBUS, ERR_SEV_FATAL, "CAN bus error");
ERR_CALLBACK_DEFINE_NAMED(canopen_emcy, err_cb, NULL);

/* static function declaration -----------------------------------------------*/
static void emcy_cb(uint16_t ident, uint16_t errorCode, uint8_t errorRegister,
                    uint8_t errorBit, uint32_t infoCode) {
  bool set = errorCode != CO_EMC_NO_ERROR;

  if (k_work_is_pending(&g_ctx.work)) {
    LOG_ERR(
        "Another EMCY is currently processing, dropping EMCY object from "
        "node 0x%X:",
        ident);

    if (set) {
      LOG_ERR("\tError set: 0x%X, reg: 0x%X, status bit: 0x%X, info: %d",
              errorCode, errorRegister, errorBit, infoCode);
    } else {
      LOG_INF("\tError cleared, reg: 0x%X, status bit: 0x%X, info: %d",
              errorRegister, errorBit, infoCode);
    }
    return;
  }

  g_ctx.ident = ident;
  g_ctx.errorCode = errorCode;
  g_ctx.errorRegister = errorRegister;
  g_ctx.errorBit = errorBit;
  g_ctx.infoCode = infoCode;

  k_work_submit(&g_ctx.work);
}

static void emcy_work(struct k_work *work) {
  struct canopen_emcy_ctx *ctx =
      CONTAINER_OF(work, struct canopen_emcy_ctx, work);
  bool set = ctx->errorCode != CO_EMC_NO_ERROR;

  // frame id == 0 when EMCY from this node
  if (ctx->ident == 0) {
    // filter out manufacturer and heartbeat consumer errors since they are
    // handled by other modules
    if (ctx->errorBit >= CO_EM_MANUFACTURER_START ||
        ctx->errorBit == CO_EM_HEARTBEAT_CONSUMER ||
        ctx->errorBit == CO_EM_HB_CONSUMER_REMOTE_RESET) {
      return;
    }

    if (set) {
      LOG_ERR(
          "Received EMCY object from this node:\n"
          "\tError set: 0x%X, reg: 0x%X, status bit: 0x%X, info: %d",
          g_ctx.errorCode, g_ctx.errorRegister, g_ctx.errorBit, g_ctx.infoCode);
    } else {
      LOG_INF(
          "Received EMCY object from this node:\n"
          "\tError cleared, reg: 0x%X, status bit: 0x%X, info: %d",
          g_ctx.errorRegister, g_ctx.errorBit, g_ctx.infoCode);
    }

    bool status_bit_set = false;
    CO_LOCK_EMCY(CO->em->CANdevTx);

    for (int i = 0; i < CO_EM_MANUFACTURER_START / 8; i++) {
      int byte = CO->em->errorStatusBits[i];
      while (byte) {
        int bit = find_lsb_set(byte) - 1;
        int status_bit = (i * 8) + bit;
        if (status_bit != CO_EM_HEARTBEAT_CONSUMER &&
            status_bit != CO_EM_HB_CONSUMER_REMOTE_RESET) {
          status_bit_set = true;
        }

        byte &= ~BIT(bit);
      }
    }

    CO_UNLOCK_EMCY(CO->em->CANdevTx);

    err_report(ERR_CODE_CANBUS, status_bit_set);

  } else {
    int id = g_ctx.ident - CO_CAN_ID_EMERGENCY;

    if (set) {
      LOG_ERR(
          "Received EMCY object from node 0x%X:\n"
          "\tError set: 0x%X, reg: 0x%X, status bit: 0x%X, info: %d",
          id, g_ctx.errorCode, g_ctx.errorRegister, g_ctx.errorBit,
          g_ctx.infoCode);
    } else {
      LOG_INF(
          "Received EMCY object from node 0x%X:\n"
          "\tError cleared, reg: 0x%X, status bit: 0x%X, info: %d",
          id, g_ctx.errorRegister, g_ctx.errorBit, g_ctx.infoCode);
    }
  }
}

static void err_cb(uint32_t errcode, bool set, void *user_data) {
  (void)user_data;

  uint64_t status_bit;
  sys_hashmap_get(&g_status_bit_map, errcode, &status_bit);

  CO_error(CO->em, set, status_bit, CO_EMC_NTURT, errcode);
}

static int init() {
  int status_bit = CO_EM_MANUFACTURER_START;

  STRUCT_SECTION_FOREACH(err, err) {
    __ASSERT(status_bit <= CO_EM_MANUFACTURER_END,
             "Too many errors defined, increase "
             "CO_CONFIG_EM_ERR_STATUS_BITS_COUNT to track them all.");

    sys_hashmap_insert(&g_status_bit_map, err->errcode, status_bit, NULL);
    status_bit++;
  }

  CO_EM_initCallbackRx(CO->em, emcy_cb);

  return 0;
}
