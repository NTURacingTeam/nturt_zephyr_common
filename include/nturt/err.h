#ifndef NTURT_ERR_H_
#define NTURT_ERR_H_

// glibc includes
#include <stdbool.h>

// zephyr includes
#include <zephyr/sys/util.h>
#include <zephyr/zbus/zbus.h>

// project incldues
#include "nturt/util.h"

/**
 * @addtogroup Err Error
 *
 * @{
 */

/* macros --------------------------------------------------------------------*/
/**
 * @addtogroup ErrMacro Error Macros
 *
 * @{
 */

/// @brief Error severity. Warning means that the system can continue to
/// operate.
#define ERR_SEV_WARN false

/// @brief Error severity. Fatal means that the system must stop.
#define ERR_SEV_FATAL true

/// @brief Error code set bit.
#define ERR_CODE_SET BIT(31)

/// @brief Error code clear bit.
#define ERR_CODE_CLEAR 0

/// @brief Invalid error code.
#define ERR_CODE_INVALID BIT(31)

/// @brief Prefix for error code names.
#define ERR_CODE_PREFIX ERR_CODE_

/**
 * @brief Define an error code.
 *
 * @param CODE Error code.
 * @param SER Error severity, one of @ref ERR_SEV_WARN or @ref ERR_SEV_FATAL.
 */
#define ERR_CODE(CODE, SER) CODE, SER

/**
 * @brief Listify error codes defined by @ref ERR_CODE.
 *
 * @param ... List of error codes.
 */
#define ERR_CODE_LISTIFY(...) __VA_ARGS__

/**
 * @brief Check if error code is valid.
 *
 * @param LIST List of error codes defined by @ref ERR_CODE_LISTIFY.
 */
#define ERR_CODE_VALIDATE(LIST)             \
  BUILD_ASSERT(NUM_VA_ARGS(LIST) / 2 <= 31, \
               "Too many error codes. Only supports up to 31.")

#define _ERR_CODE_DECLARE(I, X, _) CONCAT(ERR_CODE_PREFIX, X) = BIT(I)

/**
 * @brief Declare the values of the error codes using enum.
 *
 * @param LIST List of error codes defined by @ref ERR_CODE_LISTIFY.
 */
#define ERR_CODE_DECLARE(LIST) \
  enum err_code { FOR_EACH_PAIR_IDX(_ERR_CODE_DECLARE, (, ), LIST) }

#define _ERR_CODE_STR_MAP_DEFINE(X, _) #X

/**
 * @brief Define the error code name string map, which can be accessed by
 * @p NAME [err_index]. Where err_index is LOG2(error_code).
 *
 * @param NAME Name of the map.
 * @param LIST List of error codes defined by @ref ERR_CODE_LISTIFY.
 */
#define ERR_CODE_STR_MAP_DEFINE(NAME, LIST)                \
  static const char* NAME[] = {                            \
      FOR_EACH_PAIR(_ERR_CODE_STR_MAP_DEFINE, (, ), LIST), \
  }

#define _ERR_CODE_GET_FATAL_MASK(I, _, Y) COND_CODE_1(Y, (BIT(I)), (0))

/**
 * @brief Get the mask of fatal error codes.
 *
 * @param LIST List of error codes defined by @ref ERR_CODE_LISTIFY.
 */
#define ERR_CODE_GET_FATAL_MASK(LIST) \
  (FOR_EACH_PAIR_IDX(_ERR_CODE_GET_FATAL_MASK, (|), LIST))

#define _ERR_CODE_GET_WARN_MASK(I, _, Y) COND_CODE_0(Y, (BIT(I)), (0))

/**
 * @brief Get the mask of warning error codes.
 *
 * @param LIST List of error codes defined by @ref ERR_CODE_LISTIFY.
 */
#define ERR_CODE_GET_WARN_MASK(LIST) \
  (FOR_EACH_PAIR_IDX(_ERR_CODE_GET_WARN_MASK, (|), LIST))

/**
 * @brief Iterate over each individual error code in a combined error.
 *
 * @param ERR Combined error code.
 * @param CODE Individual error code.
 */
#define ERR_CODE_FOR_EACH(ERR, CODE)  \
  for (int __i = 0; ERR >> __i != 0;) \
    if ((__i += __builtin_ffs(ERR >> __i)) && (CODE = BIT(__i - 1)))

/// @brief Define the error code list.
#define ERR_CODE_LIST                                                          \
  ERR_CODE_LISTIFY(                                                            \
      ERR_CODE(NODE_FB, ERR_SEV_FATAL), ERR_CODE(NODE_RB, ERR_SEV_FATAL),      \
      ERR_CODE(NODE_RPI, ERR_SEV_FATAL), ERR_CODE(NODE_ACC, ERR_SEV_FATAL),    \
      ERR_CODE(NODE_INV_FL, ERR_SEV_FATAL),                                    \
      ERR_CODE(NODE_INV_FR, ERR_SEV_FATAL),                                    \
      ERR_CODE(NODE_INV_RL, ERR_SEV_FATAL),                                    \
      ERR_CODE(NODE_INV_RR, ERR_SEV_FATAL), ERR_CODE(CAN, ERR_SEV_FATAL),      \
      ERR_CODE(STEER, ERR_SEV_FATAL), ERR_CODE(APPS1, ERR_SEV_FATAL),          \
      ERR_CODE(APPS2, ERR_SEV_FATAL), ERR_CODE(APPS_PLAUS, ERR_SEV_FATAL),     \
      ERR_CODE(BSE_F, ERR_SEV_FATAL), ERR_CODE(BSE_R, ERR_SEV_FATAL),          \
      ERR_CODE(PEDAL_PLAUS, ERR_SEV_WARN),                                     \
      ERR_CODE(WHEEL_SPEED_L, ERR_SEV_FATAL),                                  \
      ERR_CODE(WHEEL_SPEED_R, ERR_SEV_FATAL),                                  \
      ERR_CODE(SUSP_DIVE, ERR_SEV_WARN), ERR_CODE(SUSP_ROLL, ERR_SEV_WARN),    \
      ERR_CODE(STAT_ACC, ERR_SEV_FATAL), ERR_CODE(STAT_INV_FL, ERR_SEV_FATAL), \
      ERR_CODE(STAT_INV_FR, ERR_SEV_FATAL),                                    \
      ERR_CODE(STAT_INV_RL, ERR_SEV_FATAL),                                    \
      ERR_CODE(STAT_INV_RR, ERR_SEV_FATAL))

#define ERR_CODE_APPS_MASK \
  (ERR_CODE_APPS1 | ERR_CODE_APPS2 | ERR_CODE_APPS_PLAUS)

#define ERR_CODE_BSE_MASK (ERR_CODE_BSE_F | ERR_CODE_BSE_R)

#define ERR_CODE_PEDALS_MASK \
  (ERR_CODE_APPS_MASK | ERR_CODE_BSE_MASK | ERR_CODE_PEDAL_PLAUS)

/**
 * @} // ErrMacro
 */

ERR_CODE_VALIDATE(ERR_CODE_LIST);

#define ERR_CODE_FATAL_MASK ERR_CODE_GET_FATAL_MASK(ERR_CODE_LIST)

#define ERR_CODE_WARN_MASK ERR_CODE_GET_WARN_MASK(ERR_CODE_LIST)

/* types
 * ---------------------------------------------------------------------*/
typedef uint32_t err_t;

ERR_CODE_DECLARE(ERR_CODE_LIST);

/* exported varaibles
 * --------------------------------------------------------*/
ZBUS_CHAN_DECLARE(err_chan);

/* function declaration
 * ------------------------------------------------------*/
/**
 * @brief Set or clear errors. Multiple error codes can be set at once.
 * Must be ones defined by @ref ERR_CODE_LISTIFY.
 *
 * @param errors Errors to set or clear.
 * @param set Set or clear errors.
 */
void err_set_errors(err_t errors, bool set);

/**
 * @brief Get the current errors.
 *
 * @return Current errors.
 */
err_t err_get_errors();

/**
 * @} // Err
 */

#endif  // NTURT_ERR_H_
