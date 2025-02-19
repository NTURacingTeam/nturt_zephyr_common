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

/* macros --------------------------------------------------------------------*/
/// @brief Flag indicating the error is set.
#define ERR_FLAG_SET BIT(4)

/// @brief Flag mask indicating the severity of the error.
#define ERR_FLAG_SEV_MASK (ERR_SEV_INFO | ERR_SEV_WARN | ERR_SEV_FATAL)

#define _ERR_DEFINE(_name, _errcode, _serverity, _desc, ...)                \
  STRUCT_SECTION_ITERABLE(err, _name) = {                                   \
      .errcode = _errcode,                                                  \
      .flags = _serverity | COND_CODE_1(__VA_OPT__(1), (__VA_ARGS__), (0)), \
      .desc = _desc,                                                        \
  }

/**
 * @brief Define an error.
 *
 * @param[in] errcode Code of the error.
 * @param[in] serverity Serverity of the error.
 * @param[in] desc Description of the error.
 * @param[in] ... Flags of the error.
 */
#define ERR_DEFINE(errcode, serverity, desc, ...) \
  _ERR_DEFINE(CONCAT(__err_, errcode), errcode, serverity, desc, __VA_ARGS__)

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

#define _ERR_CALLBACK_DEFINE(_name, _handler, _user_data, ...) \
  STRUCT_SECTION_ITERABLE(err_callback, _name) = {             \
      .handler = _handler,                                     \
      .user_data = _user_data,                                 \
      .filters = (struct err_filter[]){COND_CODE_1(            \
          __VA_OPT__(1), (__VA_ARGS__, _ERR_FILTER_LAST),      \
          (_ERR_FILTER_LAST))},                                \
  }

/**
 * @brief Define an error callback.
 * 
 * @param[in] handler Handler of the error.
 * @param[in] user_data Pointer to custom data for the callback.
 * @param[in] ... Filters for the error callback.
 */
#define ERR_CALLBACK_DEFINE(handler, user_data, ...)                        \
  _ERR_CALLBACK_DEFINE(CONCAT(__err_handler_, handler), handler, user_data, \
                       __VA_ARGS__)

/* types ---------------------------------------------------------------------*/
/**
 * @brief Error handler type.
 *
 * @param errcode Code of the error.
 * @param set True if the error is set, false if the error is cleared.
 * @param user_data Pointer to custom user data for the callback.
 */
typedef void (*err_handler_t)(uint32_t errcode, bool set, void* user_data);

/**
 * @brief Error severity.
 *
 */
enum err_sev {
  /** Error disabled, indicating this error is not used. */
  ERR_SEV_DISABLED = 0,

  /** Info serverity, the system operates normally. */
  ERR_SEV_INFO = BIT(0),

  /** Warning serverity, the system may continue running. */
  ERR_SEV_WARN = BIT(1),

  /** Fatal serverity, the system must be stopped. */
  ERR_SEV_FATAL = BIT(2),
};

/**
 * @brief Error filter type.
 *
 */
enum err_filter_type {
  /** Invalid filter. */
  ERR_FILTER_INVALID = 0,

  /** Filter for error codes. */
  ERR_FILTER_CODE = 1,

  /** Filter for severities. */
  ERR_FILTER_SEV = 2,
};

/**
 * @brief Error.
 *
 */
struct err {
  /** Code of the error. */
  uint32_t errcode;

  /** Description of the error. */
  const char* desc;

  /* Flags of the error. */
  uint32_t flags;

  /** List entry of the error. */
  TAILQ_ENTRY(err) next;
};

/**
 * @brief Error filter for error callbacks.
 *
 */
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

/**
 * @brief Error callback.
 *
 */
struct err_callback {
  /** Error handler */
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

/* function declaration ------------------------------------------------------*/
/**
 * @brief Set or clear error.
 *
 * @param errcode Error code to set or clear.
 * @param set True to set error, false to clear error.
 */
int err_report(uint32_t errcode, bool set);

/**
 * @brief Get the current errors.
 *
 * @return Current errors.
 */
// err_t err_get_errors();

/**
 * @} // Err
 */

#endif  // NTURT_ERR_H_
