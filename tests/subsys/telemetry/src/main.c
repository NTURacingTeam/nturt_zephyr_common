// glibc include
#include <stddef.h>

// zephyr include
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>
#include <zephyr/ztest_mock.h>

// nturt include
#include "nturt/telemetry.h"

/* macro ---------------------------------------------------------------------*/
#define SINGLE_DATA_NAME single_data
#define SINGLE_DATA_ADDR 0x10

#define ALIAS_DATA_NAME alias_data
#define ALIAS_DATA_ADDR 0x100

#define GROUP_DATA_SIZE 10
#define GROUP_DATA_NAME(i) CONCAT(group_data, i)
#define GROUP_DATA_ADDR_START 0x1000
#define GROUP_DATA_ADDR(i) (GROUP_DATA_ADDR_START + i)

#define SINGLE_GROUP_NAME single_group
#define ALIAS_GROUP_NAME alias_group
#define GROUP_GROUP_NAME group_group

// time granularity of 10 ms
#define MULTI (CONFIG_SYS_CLOCK_TICKS_PER_SEC / 100)

// in ticks
#define PERIOD (20 * MULTI)
#define MIN_SEPARATION (10 * MULTI)
#define WATERMARK (10 * MULTI)
#define TOLERANCE (2 * MULTI)

#define EARLY (MIN_SEPARATION + WATERMARK / 2)
#define LATE (PERIOD + WATERMARK / 2)
#define SLEEP (PERIOD + WATERMARK + TOLERANCE)

#define _DATA_LISTIFY(i, ...) \
  TM_DATA_DEFINE(GROUP_DATA_NAME(i), int, GROUP_DATA_ADDR(i))

#define _GROUP_DATA_LISTIFY(i, ...) TM_GROUP_DATA(GROUP_DATA_NAME(i))

/* type ----------------------------------------------------------------------*/
struct tm_fixture {
  tm_publish_t single_publish;

  tm_publish_t alias_publish;

  tm_publish_t group_publish;

  int group_publish_called;
};

/* static function declaration -----------------------------------------------*/
static void publish(uint32_t addr, const void *data, size_t size,
                    void *user_data);
static void group_publish(uint32_t addr, const void *data, size_t size,
                          void *user_data);
static void __single_publish(uint32_t addr, const void *data, size_t size,
                             void *user_data);
static void __alias_publish(uint32_t addr, const void *data, size_t size,
                            void *user_data);
static void __group_publish(uint32_t addr, const void *data, size_t size,
                            void *user_data);

/* static variables ----------------------------------------------------------*/
static struct tm_fixture tm_fixture = {
    .single_publish = NULL,
    .alias_publish = NULL,
    .group_publish = NULL,
    .group_publish_called = 0,
};

TM_DATA_DEFINE(SINGLE_DATA_NAME, int, SINGLE_DATA_ADDR);
TM_GROUP_DEFINE(SINGLE_GROUP_NAME, K_TICKS(PERIOD), K_TICKS(MIN_SEPARATION),
                K_TICKS(WATERMARK), __single_publish, &tm_fixture,
                TM_GROUP_DATA(SINGLE_DATA_NAME));

TM_ALIAS_DEFINE(ALIAS_DATA_NAME, SINGLE_DATA_NAME, ALIAS_DATA_ADDR);
TM_GROUP_DEFINE(ALIAS_GROUP_NAME, K_TICKS(PERIOD), K_TICKS(MIN_SEPARATION),
                K_TICKS(WATERMARK), __alias_publish, &tm_fixture,
                TM_GROUP_DATA(ALIAS_DATA_NAME));

LISTIFY(GROUP_DATA_SIZE, _DATA_LISTIFY, (;));
TM_GROUP_DEFINE(GROUP_GROUP_NAME, K_TICKS(PERIOD), K_TICKS(MIN_SEPARATION),
                K_TICKS(WATERMARK), __group_publish, &tm_fixture,
                LISTIFY(GROUP_DATA_SIZE, _GROUP_DATA_LISTIFY, (, )));

/* static function definition ------------------------------------------------*/
static void publish(uint32_t addr, const void *data, size_t size,
                    void *user_data) {
  (void)user_data;

  ztest_check_expected_value(addr);
  ztest_check_expected_value(size);
  ztest_check_expected_data(data, size);
}

static void group_publish(uint32_t addr, const void *data, size_t size,
                          void *user_data) {
  struct tm_fixture *fixture = user_data;

  zassert_equal(GROUP_DATA_ADDR(fixture->group_publish_called), addr);
  zassert_equal(size, sizeof(int));

  int value = 100 + fixture->group_publish_called;
  zassert_mem_equal(data, &value, sizeof(int));

  fixture->group_publish_called++;
}

static void __single_publish(uint32_t addr, const void *data, size_t size,
                             void *user_data) {
  struct tm_fixture *fixture = user_data;

  if (fixture->single_publish) {
    fixture->single_publish(addr, data, size, user_data);
  }
}

static void __alias_publish(uint32_t addr, const void *data, size_t size,
                            void *user_data) {
  struct tm_fixture *fixture = user_data;

  if (fixture->alias_publish) {
    fixture->alias_publish(addr, data, size, user_data);
  }
}

static void __group_publish(uint32_t addr, const void *data, size_t size,
                            void *user_data) {
  struct tm_fixture *fixture = user_data;

  if (fixture->group_publish) {
    fixture->group_publish(addr, data, size, user_data);
  }
}

/* tm ------------------------------------------------------------------------*/
static void *tm_setup() {
  // ensure to cold start aggregation
  k_sleep(K_TICKS(SLEEP));

  return &tm_fixture;
}

static void tm_before(void *_fixture) {
  (void)_fixture;

  int value = 0;

  tm_data_update(SINGLE_DATA_ADDR, &value);

  for (int i = 0; i < GROUP_DATA_SIZE; i++) {
    tm_data_update(GROUP_DATA_ADDR(i), &value);
  }

  // sleep to wait for publishing
  k_sleep(K_TICKS(MULTI));
}

static void tm_after(void *_fixture) {
  struct tm_fixture *fixture = _fixture;

  fixture->single_publish = NULL;
  fixture->alias_publish = NULL;
  fixture->group_publish = NULL;
  fixture->group_publish_called = 0;

  // force to cold start aggregration everytime
  k_sleep(K_TICKS(SLEEP));
}

ZTEST_SUITE(tm, NULL, tm_setup, tm_before, tm_after, NULL);

/**
 * @brief Test if tm_data_update() updates the data.
 *
 */
ZTEST(tm, test_update) {
  int value = 100;
  zassert_ok(tm_data_update(SINGLE_DATA_ADDR, &value));

  zassert_ok(tm_data_get(SINGLE_DATA_ADDR, &value));
  zassert_equal(value, 100);
}

/**
 * @brief Test if TM_DATA_UPDATE() updates the data.
 *
 */
ZTEST(tm, test_UPDATE) {
  int value = 100;
  TM_DATA_UPDATE(SINGLE_DATA_NAME, value);

  value = TM_DATA_GET(SINGLE_DATA_NAME);
  zassert_equal(value, 100);
}

/**
 * @brief Test if updating alias updates itself and the original data and vice
 * versa.
 *
 */
ZTEST(tm, test_alias_update) {
  int value = 100;
  zassert_ok(tm_data_update(ALIAS_DATA_ADDR, &value));

  value = 0;
  zassert_ok(tm_data_get(ALIAS_DATA_ADDR, &value));
  zassert_equal(value, 100);

  value = 0;
  zassert_ok(tm_data_get(SINGLE_DATA_ADDR, &value));
  zassert_equal(value, 100);

  value = 200;
  zassert_ok(tm_data_update(SINGLE_DATA_ADDR, &value));

  value = 0;
  zassert_ok(tm_data_get(ALIAS_DATA_ADDR, &value));
  zassert_equal(value, 200);
}

/**
 * @brief Test if publish() is called after update.
 *
 */
ZTEST_F(tm, test_publish) {
  fixture->single_publish = publish;

  int value = 100;
  ztest_expect_value(publish, addr, SINGLE_DATA_ADDR);
  ztest_expect_value(publish, size, sizeof(value));
  ztest_expect_data(publish, data, &value);

  zassert_ok(tm_data_update(SINGLE_DATA_ADDR, &value));

  k_sleep(K_TICKS(SLEEP));
}

/**
 * @brief Test if publish() is called after alias update for itself and the
 * original data and vice versa.
 *
 */
ZTEST_F(tm, test_alias_publish) {
  fixture->alias_publish = publish;

  int value = 100;
  ztest_expect_value(publish, addr, ALIAS_DATA_ADDR);
  ztest_expect_value(publish, size, sizeof(value));
  ztest_expect_data(publish, data, &value);

  zassert_ok(tm_data_update(ALIAS_DATA_ADDR, &value));

  k_sleep(K_TICKS(2 * SLEEP));

  fixture->alias_publish = NULL;
  fixture->single_publish = publish;

  value = 200;
  ztest_expect_value(publish, addr, SINGLE_DATA_ADDR);
  ztest_expect_value(publish, size, sizeof(value));
  ztest_expect_data(publish, data, &value);

  zassert_ok(tm_data_update(ALIAS_DATA_ADDR, &value));

  k_sleep(K_TICKS(2 * SLEEP));

  fixture->single_publish = NULL;
  fixture->alias_publish = publish;

  value = 300;
  ztest_expect_value(publish, addr, ALIAS_DATA_ADDR);
  ztest_expect_value(publish, size, sizeof(value));
  ztest_expect_data(publish, data, &value);

  zassert_ok(tm_data_update(SINGLE_DATA_ADDR, &value));

  k_sleep(K_TICKS(SLEEP));
}

ZTEST_F(tm, test_group_publish) {
  fixture->group_publish = group_publish;

  for (int i = 0; i < GROUP_DATA_SIZE; i++) {
    int value = 100 + i;
    zassert_ok(tm_data_update(GROUP_DATA_ADDR(i), &value));
  }

  k_sleep(K_TICKS(SLEEP));

  zassert_equal(fixture->group_publish_called, GROUP_DATA_SIZE);
}
