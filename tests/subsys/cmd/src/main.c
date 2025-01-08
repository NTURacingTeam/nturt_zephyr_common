// glibc include
#include <stddef.h>
#include <stdint.h>

// zephyr include
#include <zephyr/ztest.h>
#include <zephyr/ztest_mock.h>

// nturt include
#include "nturt/cmd.h"

/* macro ---------------------------------------------------------------------*/
#define OPCODE 0x10
#define OPCODE_QUEUED 0x20

#define SIZE 10

/* type ----------------------------------------------------------------------*/

/* static function declaration -----------------------------------------------*/
int immed_handler(uint32_t opcode, const void *operand, size_t operand_size,
                  void *user_data);

void deffered_handler(uint32_t opcode, const void *operand, size_t operand_size,
                      void *user_data);

void deffered_handler_queued(uint32_t opcode, const void *operand,
                             size_t operand_size, void *user_data);

/* static variables ----------------------------------------------------------*/
CMD_DEFINE(OPCODE, immed_handler, deffered_handler, NULL);

CMD_DEFINE(OPCODE_QUEUED, immed_handler, deffered_handler_queued, NULL);

/* static function definition ------------------------------------------------*/
int immed_handler(uint32_t opcode, const void *operand, size_t operand_size,
                  void *user_data) {
  (void)user_data;

  ztest_check_expected_value(operand_size);
  if (operand_size != 0) {
    ztest_check_expected_data(operand, operand_size);
  }

  return 0;
}

void deffered_handler(uint32_t opcode, const void *operand, size_t operand_size,
                      void *user_data) {
  (void)user_data;

  ztest_check_expected_value(operand_size);
  if (operand_size != 0) {
    ztest_check_expected_data(operand, operand_size);
  }
}

void deffered_handler_queued(uint32_t opcode, const void *operand,
                             size_t operand_size, void *user_data) {
  static uint8_t operand_next[SIZE];
  static int count = 0;

  (void)opcode;
  (void)user_data;

  ztest_check_expected_value(operand_size);
  ztest_check_expected_data(operand, operand_size);

  if (++count < SIZE) {
    ztest_expect_value(deffered_handler_queued, operand_size, SIZE);

    memset(operand_next, 100 + count + 1, SIZE);
    ztest_expect_data(deffered_handler_queued, operand, operand_next);
  } else {
    count = 0;
  }
}

/* cmd -----------------------------------------------------------------------*/
ZTEST_SUITE(cmd, NULL, NULL, NULL, NULL, NULL);

/**
 * @brief Test cmd_invoke() with various operand sizes.
 *
 */
ZTEST(cmd, test_operand_size) {
  ztest_expect_value(immed_handler, operand_size, 0);
  zassert_ok(cmd_invoke(OPCODE, NULL, 0));

  ztest_expect_value(deffered_handler, operand_size, 0);
  k_sleep(K_MSEC(1));

  uint8_t operand[SIZE] = {0};
  for (int i = 1; i <= SIZE; i++) {
    memset(operand, 100 + i, i);
    ztest_expect_value(immed_handler, operand_size, i);
    ztest_expect_data(immed_handler, operand, operand);
    zassert_ok(cmd_invoke(OPCODE, operand, i));

    ztest_expect_value(deffered_handler, operand_size, i);
    ztest_expect_data(deffered_handler, operand, operand);
    k_sleep(K_MSEC(1));
  }
}

BUILD_ASSERT(CONFIG_NTURT_CMD_PROCESS_THREAD_PRIORITY >
                 CONFIG_MAIN_THREAD_PRIORITY,
             "this test requires cmd process thread has lower priority than "
             "main thread");

ZTEST(cmd, test_exe_queue) {
  uint8_t operand[SIZE];
  for (int i = 1; i <= SIZE; i++) {
    memset(operand, 100 + i, SIZE);
    ztest_expect_value(immed_handler, operand_size, SIZE);
    ztest_expect_data(immed_handler, operand, operand);
    zassert_ok(cmd_invoke(OPCODE_QUEUED, operand, SIZE));
  }

  memset(operand, 100 + 1, SIZE);
  ztest_expect_value(deffered_handler_queued, operand_size, SIZE);
  ztest_expect_data(deffered_handler_queued, operand, operand);
  k_sleep(K_MSEC(1));
}
