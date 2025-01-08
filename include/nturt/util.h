#ifndef NTURT_UTIL_H_
#define NTURT_UTIL_H_

// glibc includes
#include <stddef.h>

// zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/util.h>

// project includes
#include "nturt/util_loops.h"

/**
 * @addtogroup Util Util
 *
 */

/* macros --------------------------------------------------------------------*/
/**
 * @addtogroup UtilMacro Utility Macros
 *
 * @{
 */

/**
 * @brief Macro that discards all arguments and expend to zephyr @ref EMPTY.
 *
 * @param ... Variable list of arguments to discard.
 * @return zephyr @ref EMPTY.
 */
#define DISCARD(...) EMPTY

/**
 * @brief Similar to Zephyr @ref REVERSE_ARGS but reverses the arguments in
 * pairs.
 *
 * @note Only accepts even number of arguments and up to 64 pairs.
 * @param ... Variable argument list.
 */
#define REVERSE_PAIRS(...)                                             \
  Z_FOR_EACH_PAIR_ENGINE(Z_FOR_EACH_PAIR_EXEC, (, ), Z_BYPASS_PAIR, _, \
                         __VA_ARGS__)

/**
 * @brief Similar to Zephyr @ref FOR_EACH but iterates over pairs of arguments.
 *
 * @note Only accepts even number of arguments and up to 64 pairs.
 * @param F Macro to invoke.
 * @param sep Separator (e.g. comma or semicolon). Must be in parentheses; this
 * is required to enable providing a comma as separator.
 * @param ... Variable argument list. The macro @p F is invoked as <tt>F(x,
 * y)</tt> for each pair (x, y) in the list.
 */
#define FOR_EACH_PAIR(F, sep, ...) \
  Z_FOR_EACH_PAIR(F, sep, REVERSE_PAIRS(__VA_ARGS__))

/**
 * @brief Similar to Zephyr @ref FOR_EACH_IDX but iterates over pairs of
 * arguments.
 *
 * @note Only accepts even number of arguments and up to 64 pairs.
 * @param F Macro to invoke.
 * @param sep Separator (e.g. comma or semicolon). Must be in parentheses; this
 * is required to enable providing a comma as separator.
 * @param ... Variable argument list. The macro @p F is invoked as <tt>F(index,
 * x, y)</tt> for each pair (x, y) in the list.
 */
#define FOR_EACH_PAIR_IDX(F, sep, ...) \
  Z_FOR_EACH_PAIR_IDX(F, sep, REVERSE_PAIRS(__VA_ARGS__))

/**
 * @brief Similar to Zephyr @ref FOR_EACH_FIXED_ARG but iterates over pairs of
 * arguments.
 *
 * @note Only accepts even number of arguments and up to 64 pairs.
 * @param F Macro to invoke.
 * @param sep Separator (e.g. comma or semicolon). Must be in parentheses; this
 * is required to enable providing a comma as separator.
 * @param fixed_arg Fixed argument passed to @p F as the second macro parameter.
 * @param ... Variable argument list. The macro @p F is invoked as
 * <tt>F(x, y, fixed_arg)</tt> for each pair (x, y) in the list.
 */
#define FOR_EACH_PAIR_FIXED_ARG(F, sep, fixed_arg, ...) \
  Z_FOR_EACH_PAIR_FIXED_ARG(F, sep, fixed_arg, REVERSE_PAIRS(__VA_ARGS__))

/**
 * @brief Similar to Zephyr @ref FOR_EACH_IDX_FIXED_ARG but iterates over pairs
 * of arguments.
 *
 * @note Only accepts even number of arguments and up to 64 pairs.
 * @param F Macro to invoke.
 * @param sep Separator (e.g. comma or semicolon). Must be in parentheses; this
 * is required to enable providing a comma as separator.
 * @param fixed_arg Fixed argument passed to @p F as the second macro parameter.
 * @param ... Variable argument list. The macro @p F is invoked as
 * <tt>F(index, x, y, fixed_arg)</tt> for each pair (x, y) in the list.
 */
#define FOR_EACH_PAIR_IDX_FIXED_ARG(F, sep, fixed_arg, ...) \
  Z_FOR_EACH_PAIR_IDX_FIXED_ARG(F, sep, fixed_arg, REVERSE_PAIRS(__VA_ARGS__))

/**
 * @brief Same as zephyr @ref GET_GET_ARG_N but accepts macro expansion for @p N
 * .
 *
 * @param N Argument index to fetch. Counter from 1.
 * @param ... Variable list of arguments from which one argument is returned.
 * @return Nth argument.
 */
#define GET_ARG_N_FIXED(N, ...) UTIL_CAT(Z_GET_ARG_, N)(__VA_ARGS__)

/**
 * @brief Check if a flag is set and clear that flag if it so.
 *
 * @param NUM Number to check.
 * @param FLAG Flag to check.
 * @return True if the flag is set.
 */
#define FLAG_SET_AND_CLEAR(NUM, FLAG) ((NUM & FLAG) && ((NUM &= ~FLAG)))

/**
 * @brief Check if a bit is set and clear that bit if it so.
 *
 * @param NUM Number to check.
 * @param BIT_ Bit to check.
 * @return True if the bit is set.
 */
#define BIT_SET_AND_CLEAR(NUM, BIT_) FLAG_SET_AND_CLEAR(NUM, BIT(BIT_))

/**
 * @} // UtilMacro
 */

/**
 * @addtogroup WorkCtxBuf Work Context Buffer
 *
 * @{
 */

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
 * @param[in] _cts Context of the work.
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

/**
 * @} // WorkCtxBuf
 */

/* types ---------------------------------------------------------------------*/
/**
 * @addtogroup WorkCtxBuf
 *
 * @{
 */

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

/**
 * @} // WorkCtxBuf
 */

/* function declaration ------------------------------------------------------*/
/**
 * @addtogroup WorkCtxBuf
 *
 * @{
 */

/**
 * @brief Allocate a work context. Return NULL if all work context are in use.
 *
 * @param[in] buf Work context buffer to allocate from.
 * @return Allocated work context. NULL if all work context are in use.
 */
struct work_ctx* work_ctx_buf_alloc(struct work_ctx_buf* buf);

/**
 * @} // WorkCtxBuf
 */

/**
 * @} // Util
 */

#endif  // NTURT_UTIL_H_
