#include "nturt/cmd.h"

// glibc include
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// zephyr include
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/hash_map.h>
#include <zephyr/sys/iterable_sections.h>
#include <zephyr/sys/mpsc_pbuf.h>

LOG_MODULE_REGISTER(nturt_cmd, CONFIG_NTURT_LOG_LEVEL);

/* type ----------------------------------------------------------------------*/
struct cmd_deffered {
  MPSC_PBUF_HDR;

  uint32_t operand_size : 32 - MPSC_PBUF_HDR_BITS;

  struct cmd *cmd;

  uint8_t operand[];
};

/* static function declaration -----------------------------------------------*/
static int init();

static struct cmd *cmd_get(uint32_t opcode);

static uint32_t cmd_deffered_get_wlen(const union mpsc_pbuf_generic *item);

static void cmd_process_thread(void *arg1, void *arg2, void *arg3);

/* static variable -----------------------------------------------------------*/
static uint32_t deffered_pubf_buf[CONFIG_NTURT_CMD_BUF_SIZE / sizeof(int)];

static struct mpsc_pbuf_buffer deffered_pbuf;

static struct mpsc_pbuf_buffer_config deffered_pbuf_config = {
    .buf = deffered_pubf_buf,
    .size = ARRAY_SIZE(deffered_pubf_buf),
    .get_wlen = cmd_deffered_get_wlen,
};

SYS_INIT(init, APPLICATION, CONFIG_NTURT_CMD_INIT_PRIORITY);

K_THREAD_DEFINE(cmd_process_thread_tid,
                CONFIG_NTURT_CMD_PROCESS_THREAD_STACK_SIZE, cmd_process_thread,
                NULL, NULL, NULL, CONFIG_NTURT_CMD_PROCESS_THREAD_PRIORITY, 0,
                0);

K_SEM_DEFINE(cmd_process_sem, 0, K_SEM_MAX_LIMIT);

SYS_HASHMAP_DEFINE_STATIC(cmd_map);

/* function definition -------------------------------------------------------*/
int cmd_invoke(uint32_t opcode, void *operand, size_t operand_size) {
  struct cmd *cmd = cmd_get(opcode);
  if (cmd == NULL) {
    return -ENOENT;
  }

  int ret;
  if (cmd->immed != NULL &&
      (ret = cmd->immed(cmd->opcode, operand, operand_size, cmd->user_data)) !=
          0) {
    return ret;
  }

  union mpsc_pbuf_generic *packet = mpsc_pbuf_alloc(
      &deffered_pbuf,
      DIV_ROUND_UP(sizeof(struct cmd_deffered) + operand_size, sizeof(int)),
      K_NO_WAIT);
  if (packet == NULL) {
    return -ENOMEM;
  }

  struct cmd_deffered *deffered = (struct cmd_deffered *)packet;
  deffered->cmd = cmd;
  deffered->operand_size = operand_size;
  memcpy(deffered->operand, operand, operand_size);

  mpsc_pbuf_commit(&deffered_pbuf, packet);

  k_sem_give(&cmd_process_sem);
  return 0;
}

/* static function definition ------------------------------------------------*/
static int init() {
  int ret;

  STRUCT_SECTION_FOREACH(cmd, cmd) {
    ret = sys_hashmap_insert(&cmd_map, cmd->opcode, (uintptr_t)cmd, NULL);
    if (ret < 0) {
      LOG_ERR("cmd_map insert failed: %s", strerror(-ret));
      return ret;

    } else if (ret == 0) {
      LOG_ERR("command opcode 0x%x already exists", cmd->opcode);
      return -EEXIST;
    }
  }

  mpsc_pbuf_init(&deffered_pbuf, &deffered_pbuf_config);

  return 0;
}

static struct cmd *cmd_get(uint32_t opcode) {
  uint64_t value;
  if (!sys_hashmap_get(&cmd_map, opcode, &value)) {
    return NULL;
  }

  return (struct cmd *)(uintptr_t)value;
}

static uint32_t cmd_deffered_get_wlen(const union mpsc_pbuf_generic *item) {
  const struct cmd_deffered *deffered = (const struct cmd_deffered *)item;
  return DIV_ROUND_UP(sizeof(struct cmd_deffered) + deffered->operand_size,
                      sizeof(uint32_t));
}

static void cmd_process_thread(void *arg1, void *arg2, void *arg3) {
  (void)arg1;
  (void)arg2;
  (void)arg3;

  while (true) {
    k_sem_take(&cmd_process_sem, K_FOREVER);

    const union mpsc_pbuf_generic *packet = mpsc_pbuf_claim(&deffered_pbuf);

    const struct cmd_deffered *deffered = (struct cmd_deffered *)packet;
    deffered->cmd->deffered(deffered->cmd->opcode, deffered->operand,
                            deffered->operand_size, deffered->cmd->user_data);

    mpsc_pbuf_free(&deffered_pbuf, packet);
  }
}
