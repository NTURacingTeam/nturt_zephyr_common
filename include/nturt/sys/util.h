/**
 * @file
 * @brief Utility.
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
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/util.h>

/**
 * @addtogroup Util Utility
 * @{
 */

/* macros --------------------------------------------------------------------*/
/**
 * @brief Logical XOR.
 *
 * @param[in] a First operand.
 * @param[in] b Second operand.
 * @return True if only one of the operands is true-like, i.e. not equal to 0.
 */
#define XOR(a, b) (!(a) ^ !(b))

/**
 * @brief Get the deferenced type of a pointer type.
 *
 * @param[in] type The pointer type.
 * @return The type of the pointer type.
 */
#define DEREF_TYPE(type) __typeof__(*((type)0))

/**
 * @brief Get the type of a struct field.
 *
 * @param[in] type The structure cintaining the field of interest.
 * @param[in] member The field to return the type of.
 * @return The type of the field.
 */
#define TYPEOF_FIELD(type, member) __typeof__(((type*)0)->member)

/**
 * @brief Discards all arguments and expend to Zephyr `EMPTY`.
 *
 * @param ... Variable list of arguments to discard.
 * @return Zephyr `EMPTY`.
 */
#define DISCARD(...) EMPTY

/**
 * @brief Same as zephyr GET_GET_ARG_N but accepts macro expansion for @p N .
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

/* types ---------------------------------------------------------------------*/
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
 * @} // Utility
 */

#endif  // NTURT_SYS_UTIL_H_
