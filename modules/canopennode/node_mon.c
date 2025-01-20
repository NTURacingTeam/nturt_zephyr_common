#include "node_mon.h"

// zephyr includes
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <zephyr/zbus/zbus.h>

// project includes
#include "nturt/err.h"
#include "nturt/msg.h"

#if !((CO_CONFIG_HB_CONS & CO_CONFIG_HB_CONS_ENABLE) &&         \
      (CO_CONFIG_HB_CONS & CO_CONFIG_HB_CONS_CALLBACK_MULTI) && \
      (CO_CONFIG_HB_CONS & CO_CONFIG_HB_CONS_QUERY_FUNCT))
#error \
    "CONFIG_NTURT_MSG_HB requires CO_CONFIG_HB_CONS_ENABLE," \
"CO_CONFIG_HB_CONS_CALLBACK_MULTI, and CO_CONFIG_HB_CONS_QUERY_FUNCT to be" \
"set in CO_CONFIG_HB_CONS"
#endif

#if OD_CNT_ARR_1016 == 0
#error "No nodes to be monitored defined in the object dictionary"
#endif

LOG_MODULE_REGISTER(nturt_msg_node_mon, CONFIG_NTURT_LOG_LEVEL);

/* static function definition ------------------------------------------------*/
/// @brief Callback function when monitored node NMT state changed.
static void mnt_changed_cb(uint8_t id, uint8_t idx,
                           CO_NMT_internalState_t state, void *arg);

/// @brief Callback function when monitored node heartbeat started for the first
/// time after timeout.
static void hb_started_cb(uint8_t id, uint8_t idx, void *arg);

/// @brief Callback function when monitored node heartbeat timed out for the
/// first time.
static void hb_timeout_cb(uint8_t id, uint8_t idx, void *arg);

/// @brief Callback function when monitored node reset.
static void node_reset_cb(uint8_t id, uint8_t idx, void *arg);

/// @brief Callback function when receiving from message emengency channel.
static void msg_emcy_chan_cb(const struct zbus_channel *chan);

/**
 * @brief Convert node ID to corresponding error code.
 *
 * @param id Node ID.
 * @return Corresponding error code.
 */
static enum err_code node_mon_id_to_err_code(enum co_node_id id);

/**
 * @brief Update error code based on monitored node state.
 *
 * @param id Node ID.
 * @param states Node state.
 */
static void node_mon_update_err_code(enum co_node_id id,
                                     struct node_mon_state *states);

/* static variable -----------------------------------------------------------*/
static struct node_mon *node_mon;

ZBUS_CHAN_DEFINE(msg_node_mon_chan, struct msg_node_mon, NULL, NULL,
                 ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(0));

ZBUS_LISTENER_DEFINE(msg_node_mon_msg_emcy_chan_listener, msg_emcy_chan_cb);
ZBUS_CHAN_ADD_OBS(msg_emcy_chan, msg_node_mon_msg_emcy_chan_listener, 0);

/* function definition -------------------------------------------------------*/
int node_mon_init(struct node_mon *node_mon_, struct canopen *co) {
  node_mon = node_mon_;

  node_mon->co = co;
  for (int i = 0; i < OD_CNT_ARR_1016; i++) {
    node_mon->states[i].nmt_state = NMT_TIMEOUT;
    node_mon->states[i].is_err = false;
  }

  CO_HBconsumer_t *HBcons = co->CO->HBcons;
  for (int i = 0; i < HBcons->numberOfMonitoredNodes; i++) {
    CO_HBconsumer_initCallbackNmtChanged(HBcons, i, node_mon, mnt_changed_cb);
    CO_HBconsumer_initCallbackHeartbeatStarted(HBcons, i, node_mon,
                                               hb_started_cb);
    CO_HBconsumer_initCallbackTimeout(HBcons, i, node_mon, hb_timeout_cb);
    CO_HBconsumer_initCallbackRemoteReset(HBcons, i, node_mon, node_reset_cb);

    // set initial monitor node state to error
    uint32_t id;
    OD_get_u32(OD_ENTRY_H1016, i + 1, &id, true);
    id = (id >> 16) & 0xFF;

    enum err_code err = node_mon_id_to_err_code(id);
    if (err != 0) {
      err_set_errors(err, true);
    } else {
      LOG_WRN("No error code corresponds to node ID: %u at subindex: 0x%02X",
              id, i + 1);
    }
  }

  return 0;
}

/* static function definition ------------------------------------------------*/
static void mnt_changed_cb(uint8_t id, uint8_t idx,
                           CO_NMT_internalState_t co_state, void *arg) {
  struct node_mon *node_mon = arg;
  enum nmt_state state;

  switch (co_state) {
    case CO_NMT_INITIALIZING:
      // node reset already handled by hb_started_cb()
      return;

    case CO_NMT_PRE_OPERATIONAL:
      state = NMT_PREOPERATIONAL;
      break;

    case CO_NMT_OPERATIONAL:
      state = NMT_OPERATIONAL;
      break;

    case CO_NMT_STOPPED:
      state = NMT_STOPPED;
      break;

    default:
      state = NMT_TIMEOUT;
      break;
  }

  node_mon->states[idx].nmt_state = state;

  if (state != NMT_OPERATIONAL) {
    LOG_WRN("Node %d NMT state changed to non-operational", id);

  } else {
    LOG_INF("Node %d NMT state changed to operational", id);
  }

  node_mon_update_err_code(id, &node_mon->states[idx]);
}

static void hb_started_cb(uint8_t id, uint8_t idx, void *arg) {
  (void)arg;

  // nmt state internal to CANopenNode will be set to unknown when a node
  // timeouts, so heartbeat active will be followed by nmt state change to
  // update nmt state

  LOG_INF("Node %d heartbeat active", id);
}

static void hb_timeout_cb(uint8_t id, uint8_t idx, void *arg) {
  struct node_mon *node_mon = arg;

  node_mon->states[idx].nmt_state = NMT_TIMEOUT;
  LOG_ERR("Node %d heartbeat timeout", id);

  node_mon_update_err_code(id, &node_mon->states[idx]);
}

static void node_reset_cb(uint8_t id, uint8_t idx, void *arg) {
  struct node_mon *node_mon = arg;

  node_mon->states[idx].nmt_state = NMT_INITIALIZING;
  node_mon->states[idx].is_err = false;
  LOG_INF("Node %d reset", id);

  node_mon_update_err_code(id, &node_mon->states[idx]);
}

static void msg_emcy_chan_cb(const struct zbus_channel *chan) {
  const struct msg_emcy *emcy = zbus_chan_const_msg(chan);

  int idx =
      CO_HBconsumer_getIdxByNodeId(node_mon->co->CO->HBcons, emcy->node_id);
  if (idx != -1) {
    if (emcy->err_reg != 0) {
      node_mon->states[idx].is_err = true;
    } else {
      node_mon->states[idx].is_err = false;
    }

    node_mon_update_err_code(emcy->node_id, &node_mon->states[idx]);
  }
}

static enum err_code node_mon_id_to_err_code(enum co_node_id id) {
  switch (id) {
    case CO_NODE_ID_FB:
      return ERR_CODE_NODE_FB;

    case CO_NODE_ID_RB:
      return ERR_CODE_NODE_RB;

    case CO_NODE_ID_ACC:
      return ERR_CODE_NODE_ACC;

    case CO_NODE_ID_INV_FL:
      return ERR_CODE_NODE_INV_FL;

    case CO_NODE_ID_INV_FR:
      return ERR_CODE_NODE_INV_FR;

    case CO_NODE_ID_INV_RL:
      return ERR_CODE_NODE_INV_RL;

    case CO_NODE_ID_INV_RR:
      return ERR_CODE_NODE_INV_RR;

    default:
      return 0;
  }
}

static void node_mon_update_err_code(enum co_node_id id,
                                     struct node_mon_state *states) {
  int ret;
  enum err_code err = node_mon_id_to_err_code(id);
  err_t errors = err_get_errors();

  if (((states->is_err) || (states->nmt_state != NMT_OPERATIONAL)) &&
      !(errors & err)) {
    err_set_errors(err, true);
  } else if (!states->is_err && states->nmt_state == NMT_OPERATIONAL &&
             (errors & err)) {
    err_set_errors(err, false);
  }

  struct msg_node_mon msg = {
      .node_id = id,
      .state = *states,
  };

  ret = zbus_chan_pub(&msg_node_mon_chan, &msg, K_FOREVER);
  if (ret != 0) {
    LOG_ERR("Failed to publish node monitor message: %d", ret);
  }
}
