/**
 * @file
 * @brief Error passing and handling.
 *
 * @author quantumspawner
 * @version 0.0.1
 * @date 2025-02-17
 * @copyright Copyright (c) 2025 NTU Racing Team
 */

#ifndef NTURT_ERR_H_
#define NTURT_ERR_H_

// glibc includes
#include <stdbool.h>
#include <stddef.h>
#include <sys/queue.h>

// zephyr includes
#include <zephyr/sys/iterable_sections.h>
#include <zephyr/sys/util.h>

/**
 * @addtogroup Err Error
 * @{
 */

/* macro ---------------------------------------------------------------------*/
/// @brief Flag indicating the error is disabled, meaning setting or clearing
/// the error will not have any effect.
#define ERR_FLAG_DISABLED BIT(3)

/// @brief Flag indicating the error is set. Can also be used in @ref ERR_DEFINE
/// to define an error that will be set after initialization.
#define ERR_FLAG_SET BIT(4)

/// @brief Flag mask indicating the severity of the error.
#define ERR_FLAG_SEV_MASK (ERR_SEV_INFO | ERR_SEV_WARN | ERR_SEV_FATAL)

/**
 * @brief Define an error.
 *
 * @param[in] _name Name of the error.
 * @param[in] _errcode Code of the error.
 * @param[in] _serverity Serverity of the error, must be one of @ref err_sev.
 * @param[in] _desc Description of the error.
 * @param[in] ... Optional flags of the error, multiple flags can be specified
 * by using the bitwise OR operator (|).
 */
#define ERR_DEFINE(_name, _errcode, _serverity, _desc, ...)                 \
  STRUCT_SECTION_ITERABLE(err, CONCAT(__err_, _name)) = {                   \
      .errcode = _errcode,                                                  \
      .flags = _serverity | COND_CODE_1(__VA_OPT__(1), (__VA_ARGS__), (0)), \
      .name = STRINGIFY(_name),                                             \
      .desc = _desc,                                                        \
  }

#define _ERR_FILTER_LAST          \
  {                               \
      .type = ERR_FILTER_INVALID, \
  }

/**
 * @brief Error filter for error codes.
 *
 * @param[in] ... Error codes to filter.
 */
#define ERR_FILTER_CODE(...)                 \
  {                                          \
      .type = ERR_FILTER_CODE,               \
      .size = NUM_VA_ARGS(__VA_ARGS__),      \
      .errcodes = (uint32_t[]){__VA_ARGS__}, \
  }

/**
 * @brief Error filter for severities.
 *
 * @param[in] ... Severities to filter.
 */
#define ERR_FILTER_SEV(...)                     \
  {                                             \
      .type = ERR_FILTER_SEV,                   \
      .size = NUM_VA_ARGS(__VA_ARGS__),         \
      .serverities = (uint32_t[]){__VA_ARGS__}, \
  }

/**
 * @brief Same as @ref ERR_CALLBACK_DEFINE, but with a custom name for the
 * callback.
 */
#define ERR_CALLBACK_DEFINE_NAMED(_name, _handler, _user_data, ...) \
  STRUCT_SECTION_ITERABLE(err_callback, _name) = {                  \
      .handler = _handler,                                          \
      .user_data = _user_data,                                      \
      .filters = (struct err_filter[]){COND_CODE_1(                 \
          __VA_OPT__(1), (__VA_ARGS__, _ERR_FILTER_LAST),           \
          (_ERR_FILTER_LAST))},                                     \
  }

/**
 * @brief Define an error callback.
 *
 * @param[in] handler Handler of the error.
 * @param[in] user_data Pointer to custom data for the callback.
 * @param[in] ... Optional filters for the error callback. If multiple filters
 * are specified, they are applied in the "and" manner.
 *
 * @note Since the name of the callback is derived from the name of @p handler ,
 * if handlers with the same name are used for multiple callbacks,
 * @ref ERR_CALLBACK_DEFINE_NAMED can be used instead to prevent linker
 * errors.
 */
#define ERR_CALLBACK_DEFINE(handler, user_data, ...)                  \
  ERR_CALLBACK_DEFINE_NAMED(CONCAT(__err_handler_, handler), handler, \
                            user_data, __VA_ARGS__)

/**
 * @brief Iterate over all set errors.
 *
 * @param[out] item @ref err pointer to the set errors, NULL if the loop exits
 * normally or no errors are set.
 */
#define ERR_FOREACH_SET(item) TAILQ_FOREACH(item, __err_errors, next)

/* type ----------------------------------------------------------------------*/
/**
 * @brief Error handler type.
 *
 * @param errcode Code of the error.
 * @param set True if the error is set, false if the error is cleared.
 * @param user_data Pointer to custom user data for the callback provided by
 * @ref ERR_CALLBACK_DEFINE.
 */
typedef void (*err_handler_t)(uint32_t errcode, bool set, void* user_data);

/// @brief Error severity.
enum err_sev {
  /** Info serverity, the system operates normally. */
  ERR_SEV_INFO = BIT(0),

  /** Warning serverity, the system may continue running. */
  ERR_SEV_WARN = BIT(1),

  /** Fatal serverity, the system must stop. */
  ERR_SEV_FATAL = BIT(2),
};

/// @brief Error filter type.
enum err_filter_type {
  /** Invalid filter, internal use only. */
  ERR_FILTER_INVALID = 0,

  /** Filter for error codes. */
  ERR_FILTER_CODE = 1,

  /** Filter for severities. */
  ERR_FILTER_SEV = 2,
};

/// @brief Error.
struct err {
  /** Code of the error. */
  uint32_t errcode;

  /* Flags of the error. */
  uint32_t flags;

  /** String representation of the error. */
  const char* name;

  /** Description of the error. */
  const char* desc;

  /** List entry of the error. */
  TAILQ_ENTRY(err) next;
};

/// @brief Error filter for error callbacks.
struct err_filter {
  /** Type of the filter. */
  enum err_filter_type type;

  /** Number of elements in the filter. */
  size_t size;

  union {
    /** Error codes to filter. */
    uint32_t* errcodes;

    /** Severities to filter. */
    uint32_t* serverities;
  };
};

/// @brief Error callback.
struct err_callback {
  /** Error handler. */
  err_handler_t handler;

  /** User data for the callback functions. */
  void* user_data;

  /** Number of error filters. */
  size_t size;

  /** Array of error filters. */
  struct err_filter* filters;
};

/// @cond

TAILQ_HEAD(err_list, err);

/// @endcond

/* exported variable ---------------------------------------------------------*/
extern const struct err_list* __err_errors;

/* function declaration ------------------------------------------------------*/
/**
 * @brief Set or clear error.
 *
 * @param[in] errcode Error code to set or clear.
 * @param[in] set True to set error, false to clear error.
 *
 * @retval 0 For success.
 * @retval -ENOENT If the error code does not exist.
 */
int err_report(uint32_t errcode, bool set);

/**
 * @brief Get the error with its code.
 *
 * @param[in] errcode Error code.
 * @return const struct err* Pointer to the error, NULL if the error code does
 * not exist.
 */
const struct err* err_get(uint32_t errcode);

/**
 * @} // Err
 */

#endif  // NTURT_ERR_H_
