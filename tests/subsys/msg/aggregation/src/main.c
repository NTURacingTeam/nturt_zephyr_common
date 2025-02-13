// glibc include
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// zephyr include
#include <zephyr/zbus/zbus.h>
#include <zephyr/ztest.h>
#include <zephyr/ztest_mock.h>

// project include
#include "nturt/msg/msg.h"

/* macro ---------------------------------------------------------------------*/
// time granularity of 10 ms
#define MULTI (CONFIG_SYS_CLOCK_TICKS_PER_SEC / 100)

// in ticks
#define PERIOD (10 * MULTI)
#define WATERMARK (5 * MULTI)
#define TOLERANCE (1 * MULTI)

#define INVALID (-1)

#define SLEEP_TIME K_TICKS(PERIOD + WATERMARK + 3 * TOLERANCE)

/* type ----------------------------------------------------------------------*/
MSG_AGG_TYPE_DECLARE(agg_susp, struct msg_susp_data, travel.fl, travel.fr,
                     travel.rl, travel.rr);

struct agg_fixture {
  struct agg_susp *agg_susp;
  int64_t start_time;
  int64_t end_time;
};

/* static function declaration -----------------------------------------------*/
void check_time(struct agg_fixture *fixture, int64_t expected);

void susp_publish(void *data, void *user_data);

/* static function definition ------------------------------------------------*/
void check_time(struct agg_fixture *fixture, int64_t expected) {
  zassert_true(fixture->end_time != INVALID);
  zassert_within(fixture->end_time, fixture->start_time + expected, TOLERANCE);
}

void susp_publish(void *data, void *user_data) {
  struct agg_fixture *fixture = user_data;

  fixture->end_time = k_uptime_ticks();

  ztest_check_expected_data(data, sizeof(struct msg_susp_data));
}

/* static variable -----------------------------------------------------------*/
static struct agg_fixture agg_fixture;

MSG_AGG_DEFINE(agg_susp, agg_susp, susp_publish, &agg_fixture,
               MSG_AGG_DATA_INIT(0));

/* suite: agg ----------------------------------------------------------------*/
static void *agg_setup() {
  MSG_AGG_INIT(&agg_susp);

  agg_fixture.agg_susp = &agg_susp;

  return &agg_fixture;
}

static void agg_before(void *_fixture) {
  struct agg_fixture *fixture = _fixture;

  memset(fixture->agg_susp->ctrl.data, 0, fixture->agg_susp->ctrl.data_size);

  MSG_AGG_START(fixture->agg_susp, K_TICKS(PERIOD), K_TICKS(WATERMARK));
  fixture->start_time = k_uptime_ticks();
}

static void agg_after(void *_fixture) {
  struct agg_fixture *fixture = _fixture;

  MSG_AGG_STOP(fixture->agg_susp);
}

ZTEST_SUITE(agg, NULL, agg_setup, agg_before, agg_after, NULL);

/**
 * @brief Test MSG_AGG_UPDATE() with all members.
 *
 */
ZTEST_F(agg, test_agg_update_all) {
  struct msg_susp_data data = {.travel = {.fl = 1, .fr = 2, .rl = 3, .rr = 4}};

  MSG_AGG_UPDATE(fixture->agg_susp, travel.fl, data.travel.fl);
  MSG_AGG_UPDATE(fixture->agg_susp, travel.fr, data.travel.fr);
  MSG_AGG_UPDATE(fixture->agg_susp, travel.rl, data.travel.rl);
  MSG_AGG_UPDATE(fixture->agg_susp, travel.rr, data.travel.rr);

  ztest_expect_data(susp_publish, data, &data);

  k_sleep(SLEEP_TIME);

  check_time(fixture, PERIOD);
}

/**
 * @brief Test MSG_AGG_UPDATE() with not all members.
 *
 */
ZTEST_F(agg, test_agg_update_not_all) {
  struct msg_susp_data data = {.travel = {.fl = 1, .fr = 2, .rl = 3}};

  MSG_AGG_UPDATE(fixture->agg_susp, travel.fl, data.travel.fl);
  MSG_AGG_UPDATE(fixture->agg_susp, travel.fr, data.travel.fr);
  MSG_AGG_UPDATE(fixture->agg_susp, travel.rl, data.travel.rl);

  ztest_expect_data(susp_publish, data, &data);

  k_sleep(SLEEP_TIME);

  check_time(fixture, PERIOD + WATERMARK);
}
