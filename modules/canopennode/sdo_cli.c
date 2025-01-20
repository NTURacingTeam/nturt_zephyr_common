#include "sdo_cli.h"

// glibc includes
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

// lib includes
#include <canopennode.h>

// project includes
#include "nturt/msg.h"

#if !((CO_CONFIG_SDO_CLI & CO_CONFIG_SDO_CLI_ENABLE) && \
      (CO_CONFIG_SDO_CLI & CO_CONFIG_FLAG_CALLBACK_PRE))
#error \
    "CONFIG_NTURT_MSG_SDO_CLI requires CO_CONFIG_SDO_CLI_ENABLE and" \
"CO_CONFIG_FLAG_CALLBACK_PRE to be set in CO_CONFIG_SDO_CLI"
#endif

LOG_MODULE_REGISTER(nturt_msg_sdo_cli, CONFIG_NTURT_LOG_LEVEL);

/* type ----------------------------------------------------------------------*/
struct sdo_cli_default_cb_ctx {
  struct k_sem done;
  int ret;
};

/* static function declaration -----------------------------------------------*/
static void sdo_cli_cb(void *arg);

static void sdo_cli_work(struct k_work *work);

static void sdo_cli_timeout_work(struct k_work *work);

static void sdo_cli_default_cb(const struct sdo_cli_req *req, int ret,
                               void *user_data);

static int sdo_read_write(struct sdo_cli *sdo_cli,
                          const struct sdo_cli_req *req, k_timeout_t timeout,
                          sdo_cli_callback_t callback, void *user_data,
                          bool read);

static int req_setup(struct sdo_cli *sdo_cli, struct sdo_cli_req_ctx *ctx);

static int req_send(struct sdo_cli *sdo_cli, struct sdo_cli_req_ctx *ctx);

static void req_finish(struct sdo_cli *sdo_cli, struct sdo_cli_req_ctx *ctx,
                       int ret);

static void req_start_next(struct sdo_cli *sdo_cli);

/* static variable -----------------------------------------------------------*/
static struct sdo_cli *sdo_cli = NULL;

/* function definition -------------------------------------------------------*/
int sdo_cli_init(struct sdo_cli *sdo_cli_, struct canopen *co) {
  sdo_cli = sdo_cli_;

  sdo_cli->co = co;
  k_work_init(&sdo_cli->work, sdo_cli_work);

  sdo_cli->size = 0;
  sdo_cli->head = 0;
  sdo_cli->tail = 0;
  for (int i = 0; i < CONFIG_NTURT_MSG_SDO_CLI_BUF_SIZE; i++) {
    sdo_cli->ctxes[i].sdo_cli = sdo_cli;
    k_work_init_delayable(&sdo_cli->ctxes[i].timeout_work,
                          sdo_cli_timeout_work);
  }

  CO_SDOclient_initCallbackPre(co->CO->SDOclient, sdo_cli, sdo_cli_cb);

  return 0;
}

int sdo_read(const struct sdo_cli_req *req, k_timeout_t timeout,
             sdo_cli_callback_t callback, void *user_data) {
  if (sdo_cli == NULL || sdo_cli->co->CO == NULL ||
      !sdo_cli->co->CO->CANmodule->CANnormal) {
    return -ENODEV;
  }

  return sdo_read_write(sdo_cli, req, timeout, callback, user_data, true);
}

int sdo_write(const struct sdo_cli_req *req, k_timeout_t timeout,
              sdo_cli_callback_t callback, void *user_data) {
  if (sdo_cli == NULL || sdo_cli->co->CO == NULL ||
      !sdo_cli->co->CO->CANmodule->CANnormal) {
    return -ENODEV;
  }

  return sdo_read_write(sdo_cli, (struct sdo_cli_req *)req, timeout, callback,
                        user_data, false);
}

/* static function definition ------------------------------------------------*/

static void sdo_cli_cb(void *arg) {
  struct sdo_cli *sdo_cli = (struct sdo_cli *)arg;

  k_work_submit(&sdo_cli->work);
}

static void sdo_cli_work(struct k_work *work) {
  struct sdo_cli *sdo_cli = CONTAINER_OF(work, struct sdo_cli, work);

  int ret;

  ret = req_send(sdo_cli, &sdo_cli->ctxes[sdo_cli->tail]);

  // CANopenNode returns > 0 if not finished
  if (ret <= 0) {
    K_SPINLOCK(&sdo_cli->lock) {
      req_finish(sdo_cli, &sdo_cli->ctxes[sdo_cli->tail], ret);
      req_start_next(sdo_cli);
    }
  }
}

static void sdo_cli_timeout_work(struct k_work *work) {
  struct k_work_delayable *dwork = k_work_delayable_from_work(work);
  struct sdo_cli_req_ctx *ctx =
      CONTAINER_OF(dwork, struct sdo_cli_req_ctx, timeout_work);
  struct sdo_cli *sdo_cli = ctx->sdo_cli;

  if (ctx == &sdo_cli->ctxes[sdo_cli->tail]) {
    CO_SDOclient_t *SDO_C = sdo_cli->co->CO->SDOclient;
    CO_SDO_abortCode_t abort_code;

    if (ctx->read) {
      CO_SDOclientUpload(SDO_C, 0, true, &abort_code, NULL, NULL, NULL);
    } else {
      CO_SDOclientDownload(SDO_C, 0, true, false, &abort_code, NULL, NULL);
    }

    K_SPINLOCK(&sdo_cli->lock) {
      req_finish(sdo_cli, ctx, -EAGAIN);
      req_start_next(sdo_cli);
    }

  } else {
    ctx->callback(ctx->req, -EAGAIN, ctx->user_data);
  }
}

static void sdo_cli_default_cb(const struct sdo_cli_req *req, int ret,
                               void *user_data) {
  struct sdo_cli_default_cb_ctx *ctx = user_data;

  ctx->ret = ret;
  k_sem_give(&ctx->done);
}

static int sdo_read_write(struct sdo_cli *sdo_cli,
                          const struct sdo_cli_req *req, k_timeout_t timeout,
                          sdo_cli_callback_t callback, void *user_data,
                          bool read) {
  int ret = 0;
  struct sdo_cli_default_cb_ctx cb_ctx;
  k_sem_init(&cb_ctx.done, 0, 1);

  K_SPINLOCK(&sdo_cli->lock) {
    if (sdo_cli->size == CONFIG_NTURT_MSG_SDO_CLI_BUF_SIZE) {
      ret = -ENOMEM;
      K_SPINLOCK_BREAK;
    }

    struct sdo_cli_req_ctx *ctx = &sdo_cli->ctxes[sdo_cli->head];
    sdo_cli->head = (sdo_cli->head + 1) % CONFIG_NTURT_MSG_SDO_CLI_BUF_SIZE;
    sdo_cli->size++;

    ctx->req = req;
    ctx->offset = 0;
    ctx->partial = true;

    ctx->read = read;
    ctx->callback = callback != NULL ? callback : sdo_cli_default_cb;
    ctx->user_data = callback != NULL ? user_data : &cb_ctx;
    ctx->deadline = sys_timepoint_calc(timeout);

    k_work_reschedule(&ctx->timeout_work, timeout);

    if (sdo_cli->size == 1 && (ret = req_setup(sdo_cli, ctx)) < 0) {
      req_finish(sdo_cli, ctx, 1);
      K_SPINLOCK_BREAK;
    }
  }

  if (ret == 0 && callback == NULL) {
    k_sem_take(&cb_ctx.done, K_FOREVER);
    ret = cb_ctx.ret;
  }

  return ret;
}

static int req_setup(struct sdo_cli *sdo_cli, struct sdo_cli_req_ctx *ctx) {
  CO_SDOclient_t *SDO_C = sdo_cli->co->CO->SDOclient;

  if (SDO_C->state != CO_SDO_ST_IDLE) {
    LOG_ERR(
        "SDO client busy, is SDO server being used by other library other than "
        "this?");
    return -EBUSY;
  }

  int ret;
  const struct sdo_cli_req *req = ctx->req;

  ret = CO_SDOclient_setup(SDO_C, CO_CAN_ID_SDO_CLI + req->node_id,
                           CO_CAN_ID_SDO_SRV + req->node_id, req->node_id);
  if (ret != CO_SDO_RT_ok_communicationEnd) {
    LOG_ERR("Failed to setup SDO client: %d", ret);
    return -EIO;
  }

  if (ctx->read) {
    ret =
        CO_SDOclientUploadInitiate(SDO_C, req->index, req->subindex, 1, false);
    if (ret != CO_SDO_RT_ok_communicationEnd) {
      LOG_ERR("Failed to initiate SDO upload: %d", ret);
      return -EIO;
    }

  } else {
    ret = CO_SDOclientDownloadInitiate(SDO_C, req->index, req->subindex,
                                       req->size, 1, false);
    if (ret != CO_SDO_RT_ok_communicationEnd) {
      LOG_ERR("Failed to initiate SDO download: %d", ret);
      return -EIO;
    }
  }

  ret = req_send(sdo_cli, ctx);
  if (ret > 0) {
    return 0;
  }

  return ret;
}

static int req_send(struct sdo_cli *sdo_cli, struct sdo_cli_req_ctx *ctx) {
  int ret;
  CO_SDOclient_t *SDO_C = sdo_cli->co->CO->SDOclient;
  CO_SDO_abortCode_t abort_code;

  if (ctx->read) {
    // time difference of 0 effectively disables timeout
    ret = CO_SDOclientUpload(SDO_C, 0, false, &abort_code, NULL, NULL, NULL);
    if (ret < 0) {
      LOG_ERR("Failed to upload SDO: %d", ret);
      return -EIO;
    }

    ctx->offset += CO_SDOclientUploadBufRead(
        SDO_C, (uint8_t *)ctx->req->data + ctx->offset,
        ctx->req->size - ctx->offset);
  } else {
    ctx->offset += CO_SDOclientDownloadBufWrite(
        SDO_C, (uint8_t *)ctx->req->data + ctx->offset,
        ctx->req->size - ctx->offset);

    if (ctx->partial && ctx->offset == ctx->req->size) {
      ctx->partial = false;
    }

    // time difference of 0 effectively disables timeout
    ret = CO_SDOclientDownload(SDO_C, 0, false, ctx->partial, &abort_code, NULL,
                               NULL);
    if (ret < 0) {
      LOG_ERR("Failed to download SDO: %d", ret);
      return -EIO;
    }
  }

  // does not handle when buffer is full
  if (ret == CO_SDO_RT_transmittBufferFull) {
    LOG_ERR("SDO client transmitt buffer full");
    return -ENOMEM;
  }

  return ret;
}

static void req_finish(struct sdo_cli *sdo_cli, struct sdo_cli_req_ctx *ctx,
                       int ret) {
  k_work_cancel_delayable(&ctx->timeout_work);

  if (!(ret > 0)) {
    ctx->callback(ctx->req, ret, ctx->user_data);
  }

  sdo_cli->tail = (sdo_cli->tail + 1) % CONFIG_NTURT_MSG_SDO_CLI_BUF_SIZE;
  sdo_cli->size--;
}

static void req_start_next(struct sdo_cli *sdo_cli) {
  while (sdo_cli->size > 0) {
    int ret;
    struct sdo_cli_req_ctx *ctx = &sdo_cli->ctxes[sdo_cli->tail];

    if (sys_timepoint_expired(ctx->deadline)) {
      sdo_cli->tail = (sdo_cli->tail + 1) % CONFIG_NTURT_MSG_SDO_CLI_BUF_SIZE;
      sdo_cli->size--;

    } else if ((ret = req_setup(sdo_cli, ctx)) < 0) {
      req_finish(sdo_cli, ctx, ret);

    } else {
      break;
    }
  }
}
