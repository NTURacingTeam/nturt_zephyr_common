// glibc include
#include <stddef.h>

// zephyr include
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>
#include <zephyr/ztest_mock.h>

// nturt include
#include "nturt/telemetry.h"

/* macro ---------------------------------------------------------------------*/
#define SINGLE_DATA 0x10

#define GROUP_DATA_START 0x1000
#define GROUP_DATA_SIZE 10

#define GROUP_DATA_ADDR(i, ...) (GROUP_DATA_START + i)

#define GROUP_ID 0x20

#define _GROUP_DATA_LISTIFY(i, ...) \
  _TM_DATA_DEFINE(CONCAT(__tm_group, i), GROUP_DATA_START + i, sizeof(int))

/* type ----------------------------------------------------------------------*/
struct tm_backend_fixture {
  int init_times;

  struct tm_backend_api api;
};

/* static function declaration -----------------------------------------------*/
static void backend_init(void *user_data);
static void backend_publish_dummy(uint32_t group_id, void *user_data);
static void backend_publish(uint32_t group_id, void *user_data);

/* static variables ----------------------------------------------------------*/
static struct tm_backend_fixture tm_backend_fixture = {
    .api =
        {
            .init = backend_init,
            .publish = backend_publish_dummy,
        },
};

TM_DATA_DEFINE(SINGLE_DATA, sizeof(int));

LISTIFY(GROUP_DATA_SIZE, _GROUP_DATA_LISTIFY, (;));
TM_GROUP_DEFINE(GROUP_ID, LISTIFY(GROUP_DATA_SIZE, GROUP_DATA_ADDR, (, )));

TM_BACKEND_DEFINE(tm_backend, &tm_backend_fixture.api, &tm_backend_fixture,
                  GROUP_ID);

/* static function definition ------------------------------------------------*/
static void backend_publish_dummy(uint32_t group_id, void *user_data) {
  (void)group_id;
  (void)user_data;
}

static void backend_init(void *user_data) {
  struct tm_backend_fixture *fixture = user_data;
  fixture->init_times++;
}

static void backend_publish(uint32_t group_id, void *user_data) {
  (void)user_data;

  ztest_check_expected_value(group_id);
}

/* tm ------------------------------------------------------------------------*/
static void tm_before(void *fixture) {
  (void)fixture;

  int value = 0;

  tm_data_update(SINGLE_DATA, &value);

  for (int i = 0; i < GROUP_DATA_SIZE; i++) {
    tm_data_update(GROUP_DATA_ADDR(i), &value);
  }
}

ZTEST_SUITE(tm, NULL, NULL, tm_before, NULL, NULL);

/**
 * @brief Test if tm_data_update() updates the data.
 *
 */
ZTEST(tm, test_data_update) {
  int value = 100;
  zassert_ok(tm_data_update(SINGLE_DATA, &value));

  zassert_ok(tm_data_get(SINGLE_DATA, &value));
  zassert_equal(value, 100);
}

/**
 * @brief Test if data updates only after the whole group is updated.
 *
 */
ZTEST(tm, test_group_update) {
  for (int i = 0; i < GROUP_DATA_SIZE; i++) {
    int value = i + 100;
    zassert_ok(tm_data_update(GROUP_DATA_ADDR(i), &value));

    zassert_ok(tm_data_get(GROUP_DATA_ADDR(i), &value));
    if (i < GROUP_DATA_SIZE - 1) {
      zassert_equal(value, 0);
    }
  }

  for (int i = 0; i < GROUP_DATA_SIZE; i++) {
    int value = 0;
    zassert_ok(tm_data_get(GROUP_DATA_ADDR(i), &value));
    zassert_equal(value, i + 100);
  }
}

/**
 * @brief Test if tm_group_commit() updates the data even if the group is not
 * fully updated.
 *
 */
ZTEST(tm, test_group_commit) {
  for (int i = 0; i < GROUP_DATA_SIZE / 2; i++) {
    int value = i + 100;
    zassert_ok(tm_data_update(GROUP_DATA_ADDR(i), &value));
  }

  zassert_ok(tm_group_commit(GROUP_ID));

  for (int i = 0; i < GROUP_DATA_SIZE / 2; i++) {
    int value = 0;
    zassert_ok(tm_data_get(GROUP_DATA_ADDR(i), &value));
    zassert_equal(value, i + 100);
  }
}

/**
 * @brief Test if tm_group_commit() preserves the last updated data for data not
 * updated in the last commit.
 *
 */
ZTEST(tm, test_group_commit_preserve_last_data) {
  for (int i = 0; i < GROUP_DATA_SIZE; i++) {
    int value = i + 100;
    zassert_ok(tm_data_update(GROUP_DATA_ADDR(i), &value));
  }

  for (int i = 0; i < GROUP_DATA_SIZE; i++) {
    int value = i + 200;
    zassert_ok(tm_data_update(GROUP_DATA_ADDR(i), &value));
  }

  for (int i = 0; i < GROUP_DATA_SIZE / 2; i++) {
    int value = i + 300;
    zassert_ok(tm_data_update(GROUP_DATA_ADDR(i), &value));
  }

  zassert_ok(tm_group_commit(GROUP_ID));

  for (int i = 0; i < GROUP_DATA_SIZE / 2; i++) {
    int value = 0;
    zassert_ok(tm_data_get(GROUP_DATA_ADDR(i), &value));
    zassert_equal(value, i + 300);
  }

  for (int i = GROUP_DATA_SIZE / 2; i < GROUP_DATA_SIZE; i++) {
    int value = 0;
    zassert_ok(tm_data_get(GROUP_DATA_ADDR(i), &value));
    zassert_equal(value, i + 200);
  }
}

/**
 * @brief Test if tm_group_access_begin() prevents data being updated and
 * tm_group_access_end() allows data to be updated.
 *
 */
ZTEST(tm, test_group_access) {
  zassert_ok(tm_group_access_begin(GROUP_ID));

  for (int i = 0; i < GROUP_DATA_SIZE; i++) {
    int value = i + 100;
    zassert_ok(tm_data_update(GROUP_DATA_ADDR(i), &value));
  }

  for (int i = 0; i < GROUP_DATA_SIZE; i++) {
    int value = 100;
    zassert_ok(tm_data_get(GROUP_DATA_ADDR(i), &value));
    zassert_equal(value, 0);
  }

  zassert_ok(tm_group_access_end(GROUP_ID));

  for (int i = 0; i < GROUP_DATA_SIZE; i++) {
    int value = 0;
    zassert_ok(tm_data_get(GROUP_DATA_ADDR(i), &value));
    zassert_equal(value, i + 100);
  }
}

/**
 * @brief Test if tm_group_access_begin() prevents data being updated even with
 * and tm_group_commit() and tm_group_access_end() updates it after the commit.
 *
 */
ZTEST(tm, test_group_access_commit) {
  zassert_ok(tm_group_access_begin(GROUP_ID));

  for (int i = 0; i < GROUP_DATA_SIZE / 2; i++) {
    int value = i + 100;
    zassert_ok(tm_data_update(GROUP_DATA_ADDR(i), &value));
  }

  zassert_ok(tm_group_commit(GROUP_ID));

  for (int i = 0; i < GROUP_DATA_SIZE / 2; i++) {
    int value = 100;
    zassert_ok(tm_data_get(GROUP_DATA_ADDR(i), &value));
    zassert_equal(value, 0);
  }

  zassert_ok(tm_group_access_end(GROUP_ID));

  for (int i = 0; i < GROUP_DATA_SIZE / 2; i++) {
    int value = 0;
    zassert_ok(tm_data_get(GROUP_DATA_ADDR(i), &value));
    zassert_equal(value, i + 100);
  }
}

/* tm_backend ----------------------------------------------------------------*/
static void *tm_backend_setup() { return &tm_backend_fixture; }

static void tm_backend_before(void *_fixture) {
  struct tm_backend_fixture *fixture = (struct tm_backend_fixture *)_fixture;

  tm_before(NULL);

  fixture->api.publish = backend_publish;
}

static void tm_backend_after(void *_fixture) {
  struct tm_backend_fixture *fixture = (struct tm_backend_fixture *)_fixture;

  fixture->api.publish = backend_publish_dummy;
}

ZTEST_SUITE(tm_backend, NULL, tm_backend_setup, tm_backend_before,
            tm_backend_after, NULL);

/**
 * @brief Test if tm_backend_init() is called once and only once.
 *
 */
ZTEST_F(tm_backend, test_backend_init) {
  zassert_equal(fixture->init_times, 1);
}

/**
 * @brief Test if backend_publish() is called every data update.
 *
 */
ZTEST_F(tm_backend, test_backend_publish) {
  ztest_expect_value(backend_publish, group_id, GROUP_ID);
  for (int i = 0; i < GROUP_DATA_SIZE; i++) {
    int value = i + 100;
    zassert_ok(tm_data_update(GROUP_DATA_ADDR(i), &value));
  }

  ztest_expect_value(backend_publish, group_id, GROUP_ID);
  zassert_ok(tm_group_commit(GROUP_ID));
}
