#include "nturt/telemetry.h"

// glibc include
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/queue.h>

// zephyr include
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/sys/hash_map.h>
#include <zephyr/sys/iterable_sections.h>

LOG_MODULE_REGISTER(nturt_tm2, CONFIG_NTURT_LOG_LEVEL);

/* static function declaration -----------------------------------------------*/
static struct tm_data *data_get(uint32_t addr);

static int init();

/* static variable -----------------------------------------------------------*/
SYS_INIT(init, APPLICATION, CONFIG_NTURT_TM_INIT_PRIORITY);

SYS_HASHMAP_DEFINE_STATIC(data_map);

/* function definition -------------------------------------------------------*/
int tm_data_get(uint32_t addr, void *value) {
  struct tm_data *data = data_get(addr);
  if (data == NULL) {
    return -ENOENT;
  }

  if (data->type != TM_DATA_TYPE_NORMAL) {
    data = data->alias;
  }

  K_SPINLOCK(&data->lock) { memcpy(value, data->data, data->size); }

  return 0;
}

int tm_data_update(uint32_t addr, const void *value) {
  struct tm_data *data = data_get(addr);
  if (data == NULL) {
    return -ENOENT;
  }

  if (data->type != TM_DATA_TYPE_NORMAL) {
    data = data->alias;
  }

  K_SPINLOCK(&data->lock) {
    memcpy(data->data, value, data->size);
    tm_data_notify_lock(data);
  }

  return 0;
}

void tm_data_notify_lock(const struct tm_data *data) {
  struct tm_group_data *group_data;
  STAILQ_FOREACH(group_data, &data->groups, next) {
    agg_update(&group_data->group->agg, group_data - group_data->group->datas);
  }
}

void tm_group_copy(struct tm_group *group) {
  K_SPINLOCK(&group->lock) {
    for (size_t i = 0; i < group->num_data; i++) {
      const struct tm_group_data *group_data = &group->datas[i];

      struct tm_data *data = group_data->data;
      if (data->type == TM_DATA_TYPE_ALIAS) {
        data = data->alias;
      }

      K_SPINLOCK(&data->lock) {
        memcpy(group_data->pub_data, data->data, data->size);
      }
    }
  }
}

/* static function definition ------------------------------------------------*/
static struct tm_data *data_get(uint32_t addr) {
  uint64_t value;
  if (!sys_hashmap_get(&data_map, addr, &value)) {
    return NULL;
  }

  return (struct tm_data *)(uintptr_t)value;
}

static int init() {
  int ret;

  STRUCT_SECTION_FOREACH(tm_data, data) {
    ret = sys_hashmap_insert(&data_map, data->addr, (uintptr_t)data, NULL);

    __ASSERT(ret != 0, "Data must not have same address: 0x%x", data->addr);

    if (ret < 0) {
      LOG_ERR("data_map insert failed: %s", strerror(-ret));
      return ret;
    }

    if (data->type == TM_DATA_TYPE_NORMAL) {
      STAILQ_INIT(&data->groups);

    } else if (data->type == TM_DATA_TYPE_ALIAS) {
      __ASSERT(data->alias->type == TM_DATA_TYPE_NORMAL,
               "Alias must point to normal data");
    }
  }

  STRUCT_SECTION_FOREACH(tm_group, group) {
    for (size_t i = 0; i < group->num_data; i++) {
      struct tm_data *data = group->datas[i].data;

      if (data->type == TM_DATA_TYPE_ALIAS) {
        data = data->alias;
      }

      STAILQ_INSERT_TAIL(&data->groups, &group->datas[i], next);
    }
  }

  return 0;
}
