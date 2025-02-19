// glibc include
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// zephyr include
#include <zephyr/ztest.h>
#include <zephyr/ztest_mock.h>

// project include
#include "nturt/err.h"

/* macro ---------------------------------------------------------------------*/
#define _ERR_NAME(idx, sev) CONCAT(ERR_, sev, _, idx)

#define _ERR_ENUM_ELM(idx, sev, offset) _ERR_NAME(idx, sev) = offset + idx

#define _ERR_LISTIFY(idx, sev) \
  ERR_DEFINE(_ERR_NAME(idx, sev), ERR_SEV_##sev, "ERR_" #sev "_" #idx)

/**
 * @brief Define an array of error codes.
 *
 * @param[in] sev Severity of the error codes.
 * @param[in] offset Offset of the error codes.
 * @param[in] len Length of the error code array.
 */
#define ERR_ARRAY_DEFINE(sev, offset, len)                 \
  enum { LISTIFY(len, _ERR_ENUM_ELM, (, ), sev, offset) }; \
                                                           \
  LISTIFY(len, _ERR_LISTIFY, (;), sev);

/**
 * @brief List the errors defined by @ref ERR_ARRAY_DEFINE.
 *
 * @param[in] sev Severity of the error codes.
 * @param[in] len Length of the error code array.
 * @return List of error codes seperated by comma.
 */
#define ERR_LIST(sev, len) LISTIFY(len, _ERR_NAME, (, ), sev)

/**
 * @brief Define a filter for error codes.
 *
 * @param[in] name Name of the filter.
 * @param[in] user_data User data to be passed to the callback.
 * @param[in] ... Errors to be filtered.
 */
#define ERR_FILTER_DEFINE(_name, _user_data, ...)               \
  static err_handler_t __err_handler_impl_##_name = NULL;       \
  static void __err_handler_##_name(uint32_t errcode, bool set, \
                                    void *user_data) {          \
    if (__err_handler_impl_##_name != NULL) {                   \
      __err_handler_impl_##_name(errcode, set, _user_data);     \
    }                                                           \
  }                                                             \
                                                                \
  ERR_CALLBACK_DEFINE(__err_handler_##_name, _user_data, __VA_ARGS__);

/**
 * @brief Set the callback of the filter.
 *
 * @param[in] name Name of the filter.
 * @param[in] fn Callback function.
 */
#define ERR_FILTER_SET(name, fn) __err_handler_impl_##name = fn;

/**
 * @brief Unset the callback of the filter.
 *
 * @param[in] name Name of the filter.
 */
#define ERR_FILTER_UNSET(name) __err_handler_impl_##name = NULL;

#define LEN 4
#define HALF 2

ERR_ARRAY_DEFINE(INFO, 0x100, LEN);
ERR_ARRAY_DEFINE(WARN, 0x200, LEN);
ERR_ARRAY_DEFINE(FATAL, 0x300, LEN);
ERR_ARRAY_DEFINE(DISABLED, 0x400, LEN);

static void clear_errors() {
  uint32_t errcodes[] = {ERR_LIST(INFO, LEN), ERR_LIST(WARN, LEN),
                         ERR_LIST(FATAL, LEN), ERR_LIST(DISABLED, LEN)};

  for (int i = 0; i < ARRAY_SIZE(errcodes); i++) {
    err_report(errcodes[i], false);
  }
}

/* suite: err ----------------------------------------------------------------*/
ERR_FILTER_DEFINE(no, NULL);

static void err_cb(uint32_t errcode, bool set, void *user_data) {
  ztest_check_expected_value(errcode);
  ztest_check_expected_value(set);
}

static void err_before(void *_fixture) {
  (void)_fixture;

  ERR_FILTER_SET(no, err_cb);
}

static void err_after(void *_fixture) {
  (void)_fixture;

  ERR_FILTER_UNSET(no);

  clear_errors();
}

ZTEST_SUITE(err, NULL, NULL, err_before, err_after, NULL);

/**
 * @brief Test setting errors.
 *
 */
ZTEST(err, test_set) {
  uint32_t errcodes[] = {ERR_LIST(INFO, LEN), ERR_LIST(WARN, LEN),
                         ERR_LIST(FATAL, LEN)};

  for (int i = 0; i < ARRAY_SIZE(errcodes); i++) {
    ztest_expect_value(err_cb, errcode, errcodes[i]);
    ztest_expect_value(err_cb, set, true);

    err_report(errcodes[i], true);
  }
}

/**
 * @brief Test clearing errors.
 *
 */
ZTEST(err, test_clear) {
  uint32_t errcodes[] = {ERR_LIST(INFO, LEN), ERR_LIST(WARN, LEN),
                         ERR_LIST(FATAL, LEN)};

  for (int i = 0; i < ARRAY_SIZE(errcodes); i++) {
    ztest_expect_value(err_cb, errcode, errcodes[i]);
    ztest_expect_value(err_cb, set, true);

    err_report(errcodes[i], true);

    ztest_expect_value(err_cb, errcode, errcodes[i]);
    ztest_expect_value(err_cb, set, false);

    err_report(errcodes[i], false);
  }
}

/* suite: err_filters --------------------------------------------------------*/
ERR_FILTER_DEFINE(half_info, NULL, ERR_FILTER_CODE(ERR_LIST(INFO, HALF)));
ERR_FILTER_DEFINE(info, NULL, ERR_FILTER_CODE(ERR_LIST(INFO, LEN)));

static void err_filters_cb(uint32_t errcode, bool set, void *user_data) {
  ztest_check_expected_value(errcode);
  ztest_check_expected_value(set);
}

static void err_filters_after(void *_fixture) {
  (void)_fixture;

  clear_errors();
}

ZTEST_SUITE(err_filters, NULL, NULL, NULL, err_filters_after, NULL);

/**
 * @brief Test error code filters.
 *
 */
ZTEST(err_filters, test_code) {
  uint32_t errcodes[] = {ERR_LIST(INFO, LEN)};

  ERR_FILTER_SET(half_info, err_filters_cb);

  for (int i = 0; i < ARRAY_SIZE(errcodes); i++) {
    if (i < HALF) {
      ztest_expect_value(err_filters_cb, errcode, errcodes[i]);
      ztest_expect_value(err_filters_cb, set, true);
    }

    err_report(errcodes[i], true);
  }

  ERR_FILTER_UNSET(half_info);
}

/**
 * @brief Test error severity filters.
 *
 */
ZTEST(err_filters, test_severity) {
  uint32_t infos[] = {ERR_LIST(INFO, LEN)};
  uint32_t others[] = {ERR_LIST(WARN, LEN), ERR_LIST(FATAL, LEN)};

  ERR_FILTER_SET(info, err_filters_cb);

  for (int i = 0; i < ARRAY_SIZE(infos); i++) {
    ztest_expect_value(err_filters_cb, errcode, infos[i]);
    ztest_expect_value(err_filters_cb, set, true);

    err_report(infos[i], true);
  }

  for (int i = 0; i < ARRAY_SIZE(others); i++) {
    err_report(others[i], true);
  }

  ERR_FILTER_UNSET(info);
}
