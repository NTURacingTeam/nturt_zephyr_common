#include "nturt/msg.h"

#include "msg_time.h"
#include "node_mon.h"
#include "sdo_cli.h"
#include "sdo_srv.h"

// glibc includes
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

// zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/iterable_sections.h>
#include <zephyr/sys/util.h>
#include <zephyr/zbus/zbus.h>

// lib includes
#include <canopennode.h>

// project includes
#include "nturt/err.h"
#include "nturt/sys.h"

LOG_MODULE_REGISTER(nturt_msg, CONFIG_NTURT_LOG_LEVEL);

/* macro ---------------------------------------------------------------------*/
/// @brief Size of work buffer for EMCY callback.
#define EMCY_WORK_BUF_SIZE 5

/* type ----------------------------------------------------------------------*/
/// @brief Structure for global variables of message module.
struct msg {
  struct canopen *co;

#if IS_ENABLED(CONFIG_NTURT_MSG_HB)
  /// @todo Add node monitor object.
  struct node_mon node_mon;
#endif  // CONFIG_NTURT_MSG_HB

#if IS_ENABLED(CONFIG_NTURT_MSG_SDO_SRV)
  struct sdo_srv sdo_srv;
#endif  // CONFIG_NTURT_MSG_SDO_SRV

#if IS_ENABLED(CONFIG_NTURT_MSG_SDO_CLI)
  struct sdo_cli sdo_cli;
#endif  // CONFIG_NTURT_MSG_SDO_CLI

#if IS_ENABLED(CONFIG_NTURT_MSG_TIME)
  struct msg_time time;
#endif  // CONFIG_NTURT_MSG_TIME
};

/// @brief Arguments for bottom half of @ref emcy_cb.
struct emcy_cb_args {
  uint16_t ident;
  uint16_t errorCode;
  uint8_t errorRegister;
  uint8_t errorBit;
  uint32_t infoCode;
};

/* static function declaration -----------------------------------------------*/
/// @brief Initialization function for emergency module.
static int emcy_init();

/// @brief Callback function when receiving CANopen EMCY object.
static void emcy_cb(uint16_t ident, uint16_t errorCode, uint8_t errorRegister,
                    uint8_t errorBit, uint32_t infoCode);

/// @brief Bottom half of @ref emcy_cb.
static void emcy_work(struct k_work *work);

/// @brief Callback function when receiving from error channel.
static void err_chan_cb(const struct zbus_channel *chan);

/// @brief Initialization function for RPDO object dictionaries.
static int rpdo_od_init();

/// @brief Initialization function for TPDO object dictionaries.
static int tpdo_od_init();

/* static variable -----------------------------------------------------------*/
/// @brief Global variable of message module.
static struct msg msg;

WORK_CTX_BUF_DEFINE(emcy_ctx, EMCY_WORK_BUF_SIZE, emcy_work, &msg,
                    struct emcy_cb_args);

ZBUS_CHAN_DEFINE(msg_emcy_chan, struct msg_emcy, NULL, NULL,
                 ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(0));

ZBUS_LISTENER_DEFINE(nturt_msg_err_chan_listener, err_chan_cb);
ZBUS_CHAN_ADD_OBS(err_chan, nturt_msg_err_chan_listener, 0);

/* function definition -------------------------------------------------------*/
int msg_init(struct canopen *co) {
  int ret;

  msg.co = co;

  // must be called before canopen_init
  ret = rpdo_od_init();
  if (ret < 0) {
    LOG_ERR("Failed to initialize RPDO object dictionary: %s", strerror(-ret));
    goto err;
  }

  ret = tpdo_od_init();
  if (ret < 0) {
    LOG_ERR("Failed to initialize TPDO object dictionary: %s", strerror(-ret));
    goto err;
  }

  ret = canopen_init(co);
  if (ret < 0) {
    LOG_ERR("Failed to initialize CANopen: %s", strerror(-ret));
    goto err;
  }

  emcy_init();

#if IS_ENABLED(CONFIG_NTURT_MSG_HB)
  ret = node_mon_init(&msg.node_mon, co);
  if (ret < 0) {
    LOG_ERR("Failed to initialize message node monitor: %s", strerror(-ret));
    goto err;
  }
#endif  // CONFIG_NTURT_MSG_HB

#if IS_ENABLED(CONFIG_NTURT_MSG_SDO_SRV)
  ret = sdo_srv_init(&msg.sdo_srv, co);
  if (ret < 0) {
    LOG_ERR("Failed to initialize SDO server: %s", strerror(-ret));
    goto err;
  }
#endif  // CONFIG_NTURT_MSG_SDO_SRV

#if IS_ENABLED(CONFIG_NTURT_MSG_SDO_CLI)
  ret = sdo_cli_init(&msg.sdo_cli, co);
  if (ret < 0) {
    LOG_ERR("Failed to initialize SDO client: %s", strerror(-ret));
    goto err;
  }
#endif  // CONFIG_NTURT_MSG_SDO_CLI

#if IS_ENABLED(CONFIG_NTURT_MSG_TIME)
  ret = msg_time_init(&msg.time, co);
  if (ret < 0) {
    LOG_ERR("Failed to initialize message time module: %s", strerror(-ret));
    goto err;
  }
#endif  // CONFIG_NTURT_MSG_TIME

  return 0;

err:
  err_set_errors(ERR_CODE_CAN, true);
  return ret;
}

/* static function definition ------------------------------------------------*/
static void emcy_cb(uint16_t ident, uint16_t errorCode, uint8_t errorRegister,
                    uint8_t errorBit, uint32_t infoCode) {
  struct work_ctx *ctx = work_ctx_alloc(emcy_ctx, EMCY_WORK_BUF_SIZE);
  if (ctx != NULL) {
    struct emcy_cb_args *args = ctx->args;
    args->ident = ident;
    args->errorCode = errorCode;
    args->errorRegister = errorRegister;
    args->errorBit = errorBit;
    args->infoCode = infoCode;

    k_work_submit(&ctx->work);
  } else {
    LOG_ERR(
        "EMCY process queue full, dropping EMCY object from id: %d, err: %d.",
        ident, errorCode);
  }
}

static void emcy_work(struct k_work *work) {
  struct msg *msg = WORK_CTX(work);
  struct emcy_cb_args *args = WORK_CTX_ARGS(work);

  int ret;
  bool set = args->errorCode != CO_EMC_NO_ERROR;

  // frame id == 0 when EMCY from this node
  if (args->ident == 0) {
    // filter out manufacturer and heartbeat consumer errors since they are
    // handled by err module and heartbeat consumer callbacks, respectively
    if (args->errorBit >= CO_EM_MANUFACTURER_START ||
        args->errorBit == CO_EM_HEARTBEAT_CONSUMER ||
        args->errorBit == CO_EM_HB_CONSUMER_REMOTE_RESET) {
      return;
    }

    if (set) {
      LOG_ERR(
          "Received EMCY object from this node: error set: 0x%X, reg: 0x%X, "
          "error bit: 0x%X, info: %d",
          args->errorCode, args->errorRegister, args->errorBit, args->infoCode);
    } else {
      LOG_INF(
          "Received EMCY object from this node: error cleared, reg: 0x%X, "
          "error bit: 0x%X, info: %d",
          args->errorRegister, args->errorBit, args->infoCode);
    }

    static const uint8_t err_mask[CO_EM_MANUFACTURER_START / 8] = {0};
    bool err_status_set =
        memcmp(msg->co->CO->em->errorStatusBits, err_mask, sizeof(err_mask));
    err_t errors = err_get_errors();

    // have to check both set and err_status_set since we could only access
    // error status bits now and it might have been set and cleared before we
    // could process it
    if (set && err_status_set && !(errors & ERR_CODE_CAN)) {
      err_set_errors(ERR_CODE_CAN, true);
    } else if (!set && !err_status_set && errors & ERR_CODE_CAN) {
      err_set_errors(ERR_CODE_CAN, false);
    }

  } else {
    int id = args->ident - CO_CAN_ID_EMERGENCY;

    if (set) {
      LOG_ERR(
          "Received EMCY object from node 0x%X: error set: 0x%X, reg: 0x%X, "
          "error bit: 0x%X, info: %d",
          id, args->errorCode, args->errorRegister, args->errorBit,
          args->infoCode);
    } else {
      LOG_INF(
          "Received EMCY object from node 0x%X: error cleared, reg: 0x%X, "
          "error "
          "bit: 0x%X, info: %d",
          id, args->errorRegister, args->errorBit, args->infoCode);
    }

    struct msg_emcy emcy = {
        .node_id = id,
        .err_code = args->errorCode,
        .err_reg = args->errorRegister,
        .err_status = args->errorBit,
        .info = args->infoCode,
    };

    ret = zbus_chan_pub(&msg_emcy_chan, &emcy, K_MSEC(5));
    if (ret < 0) {
      LOG_ERR("Failed to publish EMCY object: %s", strerror(-ret));
    }
  }
}

static void err_chan_cb(const struct zbus_channel *chan) {
  if (msg.co == NULL || msg.co->CO == NULL ||
      !msg.co->CO->CANmodule->CANnormal) {
    return;
  }

  err_t errors = *(err_t *)zbus_chan_const_msg(chan);
  bool set = FLAG_SET_AND_CLEAR(errors, ERR_CODE_SET);

  enum err_code code;
  ERR_CODE_FOR_EACH(errors, code) {
    CO_error(msg.co->CO->em, set, ERR_CODE_TO_CO_ERR_STATUS(code),
             ERR_CODE_TO_CO_ERR_CODE(code), err_get_errors());
  }
}

static int emcy_init() {
  CO_EM_initCallbackRx(msg.co->CO->em, emcy_cb);

  return 0;
}

static int rpdo_od_init() {
  OD_entry_t *entry;
  STRUCT_SECTION_FOREACH(msg_rx_init, init) {
    if ((entry = OD_find(OD, init->idx)) == NULL) {
      LOG_ERR("OD entry not found: 0x%04X", init->idx);
      return -ENOENT;
    }

    OD_extension_init(entry, init->ext);
  }

  return 0;
}

static int tpdo_od_init() {
  OD_entry_t *entry;
  STRUCT_SECTION_FOREACH(msg_tx_init, init) {
    if ((entry = OD_find(OD, init->idx)) == NULL) {
      LOG_ERR("OD entry not found: 0x%04X", init->idx);
      return -ENOENT;
    }

    OD_extension_init(entry, &init->ext);
    init->ent = entry;
  }

  return 0;
}
