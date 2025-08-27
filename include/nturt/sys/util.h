/**
 * @file
 * @brief Utility macros and functions.
 *
 * @author quantumspawner
 * @version 0.0.1
 * @date 2025-02-25
 * @copyright Copyright (c) 2025 NTU Racing Team
 */

#ifndef NTURT_SYS_UTIL_H_
#define NTURT_SYS_UTIL_H_

// glibc includes
#include <stddef.h>

// zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/sys/cbprintf.h>
#include <zephyr/sys/util.h>

// project includes
#include "nturt/sys/util_loops.h"

/**
 * @addtogroup util Utility
 * @brief Utility macros and functions.
 *
 * @ingroup sys
 * @{
 */

/* macros --------------------------------------------------------------------*/
/**
 * @brief Discards all arguments and expend to Zephyr `EMPTY`.
 *
 * @param[in] ... Arguments to discard.
 * @return Zephyr `EMPTY`.
 */
#define DISCARD(...) EMPTY

/**
 * @brief Same as Zephyr `GET_GET_ARG_N` but accepts macro expansion for @p N .
 *
 * @param[in] N The index of argument to fetch. Count from 1.
 * @param[in] ... Arguments from which one argument is returned.
 * @return Nth argument.
 */
#define GET_ARG_N_FIXED(N, ...) CONCAT(Z_GET_ARG_, N)(__VA_ARGS__)

#define _FOR_EACH_IDX_FIXED_ARG(idx, x, fixed_arg0, fixed_arg1) \
  fixed_arg0(idx, x, fixed_arg1)

/**
 * @brief Same as Zephyr `FOR_EACH_IDX_FIXED_ARG`, useful for nested `FOR_EACH`
 * macros.
 */
#define N_FOR_EACH_IDX_FIXED_ARG(F, sep, fixed_arg, ...) \
  N_FOR_EACH_ENGINE(_FOR_EACH_IDX_FIXED_ARG, sep, F, fixed_arg, __VA_ARGS__)

#define _FOR_EACH_FIXED_ARG(idx, x, fixed_arg0, fixed_arg1) \
  fixed_arg0(x, fixed_arg1)

/**
 * @brief Same as Zephyr `FOR_EACH_FIXED_ARG`, useful for nested `FOR_EACH`
 * macros.
 */
#define N_FOR_EACH_FIXED_ARG(F, sep, fixed_arg, ...) \
  N_FOR_EACH_ENGINE(_FOR_EACH_FIXED_ARG, sep, F, fixed_arg, __VA_ARGS__)

#define _FOR_EACH_IDX(idx, x, fixed_arg0, fixed_arg1) fixed_arg0(idx, x)

/**
 * @brief Same as Zephyr `FOR_EACH_IDX`, useful for nested `FOR_EACH` macros.
 */
#define N_FOR_EACH_IDX(F, sep, ...) \
  N_FOR_EACH_ENGINE(_FOR_EACH_IDX, sep, F, _, __VA_ARGS__)

#define _FOR_EACH(idx, x, fixed_arg0, fixed_arg1) fixed_arg0(x)

/**
 * @brief Same as Zephyr `FOR_EACH`, useful for nested `FOR_EACH` macros.
 */
#define N_FOR_EACH(F, sep, ...) \
  N_FOR_EACH_ENGINE(_FOR_EACH, sep, F, _, __VA_ARGS__)

/**
 * @brief Get the deferenced type of a pointer type.
 *
 * @param[in] type The pointer type.
 * @return The type @param type points to.
 */
#define DEREF_TYPE(type) __typeof__(*((type)0))

/**
 * @brief Get the type of a member in a structure (struct or union).
 *
 * @param[in] type The structure containing the member.
 * @param[in] member The member of the structure.
 * @return The type of the member.
 */
#define TYPEOF_FIELD(type, member) __typeof__(((type*)0)->member)

/**
 * @brief Logical XOR.
 *
 * @param[in] a First operand.
 * @param[in] b Second operand.
 * @return True if and only if one of the operands is true-like, i.e. not equal
 * to 0.
 */
#define XOR(a, b) (!(a) ^ !(b))

/**
 * @brief Check if all bits in @p mask are set in @p value .
 *
 * @param[in] value Value to check.
 * @param[in] mask Mask to check against.
 * @return True if all bits in @p mask are set in @p value .
 */
#define IS_MASK_SET(value, mask) (((value) & (mask)) == (mask))

/**
 * @brief Check if a flag is set and clear that flag if it so.
 *
 * @param[in] num Number to check.
 * @param[in] flag Flag to check.
 * @return True if the flag is set.
 */

#define FLAG_SET_AND_CLEAR(num, flag) ((num & flag) && ((num &= ~flag)))
/**
 * @brief Check if a bit is set and clear that bit if it is so.
 *
 * @param[in] num Number to check.
 * @param[in] bit Bit to check.
 * @return True if the bit is set.
 */
#define BIT_SET_AND_CLEAR(num, bit) FLAG_SET_AND_CLEAR(num, BIT(bit))

#define _LOG_THROTTLE(level, min_separation, ...)               \
  do {                                                          \
    static k_timepoint_t __log_throttle_next = {0};             \
    if (sys_timepoint_expired(__log_throttle_next)) {           \
      CONCAT(LOG_, level)(__VA_ARGS__);                         \
                                                                \
      __log_throttle_next = sys_timepoint_calc(min_separation); \
    }                                                           \
  } while (0)

/**
 * @brief Writes a DEBUG level message to the log with throttling.
 *
 * @param[in] min_separation Minimum time between log messages.
 * @param[in] ... Same as Zephyr `LOG_DBG`.
 */
#define LOG_DBG_THROTTLE(min_separation, ...) \
  _LOG_THROTTLE(DBG, min_separation, __VA_ARGS__)

/**
 * @brief Writes an INFO level message to the log with throttling.
 *
 * @param[in] min_separation Minimum time between log messages.
 * @param[in] ... Same as Zephyr `LOG_INF`.
 */
#define LOG_INF_THROTTLE(min_separation, ...) \
  _LOG_THROTTLE(INF, min_separation, __VA_ARGS__)

/** * @brief Writes a WARNING level message to the log with throttling.
 *
 * @param[in] min_separation Minimum time between log messages.
 * @param[in] ... Same as Zephyr `LOG_WRN`.
 */
#define LOG_WRN_THROTTLE(min_separation, ...) \
  _LOG_THROTTLE(WARN, min_separation, __VA_ARGS__)

/** @brief Writes an ERROR level message to the log with throttling.
 *
 * @param[in] min_separation Minimum time between log messages.
 * @param[in] ... Same as Zephyr `LOG_ERR`.
 */
#define LOG_ERR_THROTTLE(min_separation, ...) \
  _LOG_THROTTLE(ERR, min_separation, __VA_ARGS__)

/**
 * @brief Insert format string to print @p x .
 *
 * @param[in] x The data to print.
 */
#define PRI(x)                    \
  _Generic((x),                   \
      char: "%c",                 \
      signed char: "%hhd",        \
      unsigned char: "%hhu",      \
      short: "%hd",               \
      unsigned short: "%hu",      \
      int: "%d",                  \
      unsigned int: "%u",         \
      long: "%ld",                \
      unsigned long: "%lu",       \
      long long: "%lld",          \
      unsigned long long: "%llu", \
      float: "%f",                \
      double: "%f",               \
      long double: "%Lf",         \
      char*: "%s",                \
      void*: "%p",                \
      default: "%p")

/**
 * @brief Insert format string to scan @p x .
 *
 * @param[in] x The data to scan, NOT the pointer to it.
 */
#define SCN(x)                    \
  _Generic((x),                   \
      char: "%c",                 \
      signed char: "%hhd",        \
      unsigned char: "%hhu",      \
      short: "%hd",               \
      unsigned short: "%hu",      \
      int: "%d",                  \
      unsigned int: "%u",         \
      long: "%ld",                \
      unsigned long: "%lu",       \
      long long: "%lld",          \
      unsigned long long: "%llu", \
      float: "%f",                \
      double: "%lf",              \
      long double: "%Lf",         \
      char*: "%s",                \
      void*: "%p",                \
      default: "%p")

/**
 * @brief Insert @p x to printf format.
 *
 * @param[in] x The data to print.
 */
#define PRI_arg(x) _Generic(Z_ARGIFY(x), float: (double)(x), default: (x))

#define _WORK_CTX_DEFINE(_i, _work_handler, _ctx, _args) \
  [_i] = {                                               \
      .work = Z_WORK_INITIALIZER(_work_handler),         \
      .ctx = _ctx,                                       \
      .args = _args[_i],                                 \
  }

/**
 * @brief Define work context buffer for the bottom half of an ISR.
 *
 * @param[in] _name Name of the buffer.
 * @param[in] _size Size of the buffer, i.e. the number of simultaneous works
 * that can be submitted.
 * @param[in] _work_handler Work entry point.
 * @param[in] _ctx Context of the work.
 * @param[in] _args_type Type of the arguments for the work.
 */
#define WORK_CTX_BUF_DEFINE(_name, _size, _work_handler, _ctx, _args_type) \
  static _args_type CONCAT(_name, _work_args)[_size];                      \
  static struct work_ctx CONCAT(_name, _work_ctx)[] = {                    \
      LISTIFY(_size, _WORK_CTX_DEFINE, (, ), _work_handler, _ctx,          \
              &CONCAT(_name, _work_args)),                                 \
  };                                                                       \
  static struct work_ctx_buf _name = {                                     \
      .size = _size,                                                       \
      .work_ctxs = CONCAT(_name, _work_ctx),                               \
  }

/**
 * @brief Get the context of the work.
 *
 * @param[in] _work Pointer to work passed to work handler.
 * @return Context of the work.
 */
#define WORK_CTX(_work) \
  (((struct work_ctx*)CONTAINER_OF(_work, struct work_ctx, work))->ctx)

/**
 * @brief Get the arguments for the work.
 *
 * @param[in] _work Pointer to work passed to the work handler.
 * @return Arguments for the work.
 */
#define WORK_CTX_ARGS(_work) \
  (((struct work_ctx*)CONTAINER_OF(_work, struct work_ctx, work))->args)

/* types
   ---------------------------------------------------------------------*/
/// @brief Work context.
struct work_ctx {
  /// @brief Work.
  struct k_work work;

  /// @brief Pointer to context of the work.
  void* const ctx;

  /// @brief Pointer to arguments for the work.
  void* const args;
};

/// @brief Work context buffer.
struct work_ctx_buf {
  /// @brief Size of the work context buffer.
  const size_t size;

  /// @brief Array of work context buffer.
  struct work_ctx* const work_ctxs;
};

/* function declaration ------------------------------------------------------*/
/**
 * @brief Allocate a work context. Return NULL if all work context are in use.
 *
 * @param[in] buf Work context buffer to allocate from.
 * @return Allocated work context. NULL if all work context are in use.
 */
struct work_ctx* work_ctx_buf_alloc(struct work_ctx_buf* buf);

/**
 * @} // util
 */

#endif  // NTURT_SYS_UTIL_H_
