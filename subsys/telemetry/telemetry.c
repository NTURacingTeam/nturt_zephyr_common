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

LOG_MODULE_REGISTER(nturt_tm, CONFIG_NTURT_LOG_LEVEL);

/* static function declaration -----------------------------------------------*/
static int init();

static struct tm_data *data_get(uint32_t addr);

static struct tm_group *group_get(uint32_t id);

static void group_access_end_impl(struct tm_group *group);

/* static variable -----------------------------------------------------------*/
SYS_INIT(init, APPLICATION, CONFIG_NTURT_TM_INIT_PRIORITY);

SYS_HASHMAP_DEFINE_STATIC(group_map);
SYS_HASHMAP_DEFINE_STATIC(data_map);

/* function definition -------------------------------------------------------*/
int tm_data_get(uint32_t addr, void *value) {
  struct tm_data *data = data_get(addr);
  if (data == NULL) {
    return -ENOENT;
  }

  if (data->has_group) {
    K_SPINLOCK(&data->group->lock) {
      memcpy(value, data->data[data->group->get_buf_idx], data->size);
    }
  } else {
    K_SPINLOCK(&data->lock) {
      memcpy(value, data->data[data->get_buf_idx], data->size);
    }
  }

  return 0;
}

int tm_data_update(uint32_t addr, const void *value) {
  struct tm_data *data = data_get(addr);
  if (data == NULL) {
    return -ENOENT;
  }

  if (data->has_group) {
    K_SPINLOCK(&data->group->lock) {
      memcpy(data->data[data->group->get_buf_idx ^ 1], value, data->size);

      data->group->updated |= BIT(data->data_idx);

      data->group->accessing++;
    }

    group_access_end_impl(data->group);
  } else {
    K_SPINLOCK(&data->lock) {
      memcpy(data->data[data->get_buf_idx ^ 1], value, data->size);

      data->get_buf_idx ^= 1;
    }
  }

  return 0;
}

int tm_group_access_begin(uint32_t id) {
  struct tm_group *group = group_get(id);
  if (group == NULL) {
    return -ENOENT;
  }

  K_SPINLOCK(&group->lock) { group->accessing++; }

  return 0;
}

int tm_group_access_end(uint32_t id) {
  struct tm_group *group = group_get(id);
  if (group == NULL) {
    return -ENOENT;
  }

  group_access_end_impl(group);

  return 0;
}

int tm_group_commit(uint32_t id) {
  struct tm_group *group = group_get(id);
  if (group == NULL) {
    return -ENOENT;
  }

  K_SPINLOCK(&group->lock) {
    int i;
    while ((i = __builtin_ffs(~group->updated & BIT_MASK(group->num_data)) -
                1) >= 0) {
      struct tm_data *data = data_get(group->data_addrs[i]);
      __ASSERT_NO_MSG(data != NULL);

      memcpy(data->data[group->get_buf_idx ^ 1], data->data[group->get_buf_idx],
             data->size);

      group->updated |= BIT(i);
    }

    group->accessing++;
  }

  group_access_end_impl(group);

  return 0;
}

/* static function definition ------------------------------------------------*/
static int init() {
  int ret;

  STRUCT_SECTION_FOREACH(tm_data, data) {
    ret = sys_hashmap_insert(&data_map, data->addr, (uintptr_t)data, NULL);
    if (ret < 0) {
      LOG_ERR("data_map insert failed: %s", strerror(-ret));
      return ret;

    } else if (ret == 0) {
      LOG_ERR("data with address 0x%x already exists", data->addr);
      return -EEXIST;
    }
  }

  STRUCT_SECTION_FOREACH(tm_group, group) {
    ret = sys_hashmap_insert(&group_map, group->id, (uintptr_t)group, NULL);
    if (ret < 0) {
      LOG_ERR("group_map insert failed: %s", strerror(-ret));
      return ret;

    } else if (ret == 0) {
      LOG_ERR("group with id %d already exists", group->id);
      return -EEXIST;
    }

    STAILQ_INIT(&group->backends);

    for (size_t i = 0; i < group->num_data; i++) {
      struct tm_data *data = data_get(group->data_addrs[i]);
      if (data == NULL) {
        LOG_ERR("data with address 0x%x does not exist", group->data_addrs[i]);
        return -ENOENT;
      }

      data->has_group = true;
      data->group = group;
      data->data_idx = i;
    }
  }

  STRUCT_SECTION_FOREACH(tm_backend, backend) {
    tm_backend_init(backend);

    for (size_t i = 0; i < backend->num_group; i++) {
      struct tm_group *group = group_get(backend->group_ids[i]);
      if (group == NULL) {
        LOG_ERR("group with id %d does not exist", backend->group_ids[i]);
        return -ENOENT;
      }

      backend->elements[i].backend = backend;
      STAILQ_INSERT_TAIL(&group->backends, &backend->elements[i], next);
    }
  }

  return 0;
}

static struct tm_data *data_get(uint32_t addr) {
  uint64_t value;
  if (!sys_hashmap_get(&data_map, addr, &value)) {
    return NULL;
  }

  return (struct tm_data *)(uintptr_t)value;
}

static struct tm_group *group_get(uint32_t id) {
  uint64_t value;
  if (!sys_hashmap_get(&group_map, id, &value)) {
    return NULL;
  }

  return (struct tm_group *)(uintptr_t)value;
}

static void group_access_end_impl(struct tm_group *group) {
  bool to_publish = false;

  do {
    K_SPINLOCK(&group->lock) {
      if (group->accessing == 1 &&
          group->updated == BIT_MASK(group->num_data)) {
        group->updated = 0;
        group->get_buf_idx ^= 1;

        to_publish = true;
      } else {
        group->accessing--;

        to_publish = false;
      }
    }

    if (to_publish) {
      struct tm_backend_list_elm *entry;
      STAILQ_FOREACH(entry, &group->backends, next) {
        tm_backend_publish(entry->backend, group->id);
      }
    }
  } while (to_publish);
}
