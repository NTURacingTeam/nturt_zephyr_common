// glibc include
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// zephyr include
#include <zephyr/ztest.h>
#include <zephyr/ztest_mock.h>

// project include
#include "nturt/msg/msg.h"

/* macro ---------------------------------------------------------------------*/
// time granularity of 10 ms
#define MULTI (CONFIG_SYS_CLOCK_TICKS_PER_SEC / 100)

// in ticks
#define PERIOD (20 * MULTI)
#define MIN_SEPARATION (10 * MULTI)
#define WATERMARK (10 * MULTI)
#define TOLERANCE (2 * MULTI)

#define EARLY (MIN_SEPARATION + WATERMARK / 2)
#define LATE (PERIOD + WATERMARK / 2)
#define SLEEP (PERIOD + WATERMARK + 3 * TOLERANCE)

#define INVALID (-1)

/* type ----------------------------------------------------------------------*/
AGG_TYPE_DECLARE(agg_susp, struct msg_susp_data,
                 AGG_MEMBER(header, AGG_FLAG_OPTIONAL), AGG_MEMBER(travel.fl),
                 AGG_MEMBER(travel.fr), AGG_MEMBER(travel.rl),
                 AGG_MEMBER(travel.rr));

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

AGG_DEFINE(agg_susp, agg_susp, AGG_DATA_INIT(0), K_TICKS(PERIOD),
           K_TICKS(MIN_SEPARATION), K_TICKS(WATERMARK), susp_publish,
           &agg_fixture);

/* suite: agg ----------------------------------------------------------------*/
static void *agg_setup() {
  k_sleep(K_TICKS(SLEEP));

  agg_fixture.agg_susp = &agg_susp;

  return &agg_fixture;
}

static void agg_before(void *_fixture) {
  static const struct msg_susp_data data = {0};

  struct agg_fixture *fixture = _fixture;

  // reset data and cold start
  AGG_UPDATE(fixture->agg_susp, header, data.header);
  AGG_UPDATE(fixture->agg_susp, travel.fl, data.travel.fl);
  AGG_UPDATE(fixture->agg_susp, travel.fr, data.travel.fr);
  AGG_UPDATE(fixture->agg_susp, travel.rl, data.travel.rl);
  AGG_UPDATE(fixture->agg_susp, travel.rr, data.travel.rr);

  ztest_expect_data(susp_publish, data, &data);

  k_sleep(K_TICKS(MULTI));

  fixture->start_time = k_uptime_ticks();
}

static void agg_after(void *_fixture) {
  (void)_fixture;

  // force to cold start everytime
  k_sleep(K_TICKS(SLEEP));
}

ZTEST_SUITE(agg, NULL, agg_setup, agg_before, agg_after, NULL);

/**
 * @brief Test publish after minimum separation time with all members updated.
 *
 */
ZTEST_F(agg, test_early_publish) {
  struct msg_susp_data data = {.travel = {.fl = 1, .fr = 2, .rl = 3, .rr = 4}};

  AGG_UPDATE(fixture->agg_susp, travel.fl, data.travel.fl);
  AGG_UPDATE(fixture->agg_susp, travel.fr, data.travel.fr);
  AGG_UPDATE(fixture->agg_susp, travel.rl, data.travel.rl);
  AGG_UPDATE(fixture->agg_susp, travel.rr, data.travel.rr);

  ztest_expect_data(susp_publish, data, &data);

  k_sleep(K_TICKS(SLEEP));

  check_time(fixture, MIN_SEPARATION);
}

/**
 * @brief Test publish between minimum separartion and period immediately after
 * all members updated.
 *
 */
ZTEST_F(agg, test_before_period_publish) {
  struct msg_susp_data data = {.travel = {.fl = 1, .fr = 2, .rl = 3, .rr = 4}};

  AGG_UPDATE(fixture->agg_susp, travel.fl, data.travel.fl);
  AGG_UPDATE(fixture->agg_susp, travel.fr, data.travel.fr);
  AGG_UPDATE(fixture->agg_susp, travel.rl, data.travel.rl);

  ztest_expect_data(susp_publish, data, &data);

  k_sleep(K_TICKS(EARLY));

  AGG_UPDATE(fixture->agg_susp, travel.rr, data.travel.rr);

  k_sleep(K_TICKS(SLEEP));

  check_time(fixture, EARLY);
}

/**
 * @brief Test publish between period and watermark immediately after
 * late-arrival members updated.
 *
 */
ZTEST_F(agg, test_after_period_publish) {
  struct msg_susp_data data = {.travel = {.fl = 1, .fr = 2, .rl = 3, .rr = 4}};

  AGG_UPDATE(fixture->agg_susp, travel.fl, data.travel.fl);
  AGG_UPDATE(fixture->agg_susp, travel.fr, data.travel.fr);
  AGG_UPDATE(fixture->agg_susp, travel.rl, data.travel.rl);

  ztest_expect_data(susp_publish, data, &data);

  k_sleep(K_TICKS(LATE));

  AGG_UPDATE(fixture->agg_susp, travel.rr, data.travel.rr);

  k_sleep(K_TICKS(SLEEP));

  check_time(fixture, LATE);
}

/**
 * @brief Test publish after period + water with not all members updated.
 *
 */
ZTEST_F(agg, test_late_publish) {
  struct msg_susp_data data = {.travel = {.fl = 1, .fr = 2, .rl = 3}};

  AGG_UPDATE(fixture->agg_susp, travel.fl, data.travel.fl);
  AGG_UPDATE(fixture->agg_susp, travel.fr, data.travel.fr);
  AGG_UPDATE(fixture->agg_susp, travel.rl, data.travel.rl);

  ztest_expect_data(susp_publish, data, &data);

  k_sleep(K_TICKS(SLEEP));

  check_time(fixture, PERIOD + WATERMARK);
}
