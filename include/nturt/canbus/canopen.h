/**
 * @file
 * @brief CANopen support.
 *
 * @author quantumspawner
 * @version 0.0.1
 * @date 2025-07-01
 * @copyright Copyright (c) 2025 NTU Racing Team
 */

#ifndef NTURT_CANBUS_CANOPEN_H_
#define NTURT_CANBUS_CANOPEN_H_

// glibc includes
#include <stdint.h>

// zephyr includes
#include <zephyr/sys/iterable_sections.h>

// project includes
#include "nturt/msg/aggregation.h"
#include "nturt/msg/msg.h"
#include "nturt/sys/util.h"

/**
 * @defgroup can_open CANopen
 * @brief CANopen support.
 *
 * @ingroup can
 * @{
 */

/* macro----------------------------------------------------------------------*/
/// @brief CANopen error code for reporting NTURT errors.
#define CO_EMC_NTURT 0xFF10

/**
 * @brief Specify the data in an OD entry to be aggregated. Used
 * in @ref OD_AGG_ENTRY.
 *
 * @param[in] subidx Subindex of the data in the OD entry. If the entry is of
 * type `VAR`, this must be 0.
 * @param[in] type Type of the data.
 * @param[in] convert Function to convert the raw data into physical data, which
 * is useful if the raw data have different scale or offset. If no conversion is
 * needed, use Zephyr `IDENTITY`.
 * @param[in] member Member in the message that the data corresponds to and will
 * be updated to when the OD data is received, must be specified by
 * @ref AGG_MEMBER.
 */
#define OD_AGG_DATA(subidx, type, convert, member) \
  (subidx, type, convert, member)

#define _OD_AGG_DATA_SUBIDX(data) GET_ARG_N(1, __DEBRACKET data)
#define _OD_AGG_DATA_TYPE(data) GET_ARG_N(2, __DEBRACKET data)
#define _OD_AGG_DATA_CONVERT(data) GET_ARG_N(3, __DEBRACKET data)
#define _OD_AGG_DATA_MEMBER(data) GET_ARG_N(4, __DEBRACKET data)

/**
 * @brief Specify an OD entry to be aggregated. Used in
 * @ref CANOPEN_OD_AGG_TO_MSG_DEFINE.
 *
 * @param[in] idx Index of the OD entry.
 * @param[in] ... Data to be aggregated, must be specified by @ref OD_AGG_DATA.
 */
#define OD_AGG_ENTRY(idx, ...) (idx, __VA_ARGS__)

#define _OD_AGG_ENTRY_IDX(entry) GET_ARG_N(1, __DEBRACKET entry)
#define _OD_AGG_ENTRY_DATA(entry) GET_ARGS_LESS_N(1, __DEBRACKET entry)

#define __OD_AGG_MEMBERS(...) FOR_EACH(_OD_AGG_ENTRY_DATA, (, ), __VA_ARGS__)
#define _OD_AGG_MEMBERS(...) \
  FOR_EACH(_OD_AGG_DATA_MEMBER, (, ), __OD_AGG_MEMBERS(__VA_ARGS__))

#define _OD_AGG_NAME(name) CONCAT(__od_agg_, name)
#define _OD_AGG_WRITE(name) CONCAT(__od_write_, _OD_AGG_NAME(name))

#define _OD_WRITE_CASE(data, name)                                  \
  case _OD_AGG_DATA_SUBIDX(data): {                                 \
    _OD_AGG_DATA_TYPE(data) __buf;                                  \
    memcpy(&__buf, stream->dataOrig, stream->dataLength);           \
                                                                    \
    AGG_TYPED_UPDATE(&_OD_AGG_NAME(name), struct name,              \
                     _AGG_MEMBER_MEMBER(_OD_AGG_DATA_MEMBER(data)), \
                     _OD_AGG_DATA_CONVERT(data)(__buf));            \
  } break

#define _OD_AGG_WRITE_DEFINE(entry, name)                                   \
  static ODR_t _OD_AGG_WRITE(_OD_AGG_ENTRY_IDX(entry))(                     \
      OD_stream_t * stream, const void *buf, OD_size_t size,                \
      OD_size_t *size_written) {                                            \
    ODR_t ret = OD_writeOriginal(stream, buf, size, size_written);          \
    if (ret != ODR_OK) {                                                    \
      return ret;                                                           \
    }                                                                       \
                                                                            \
    switch (stream->subIndex) {                                             \
      N_FOR_EACH_FIXED_ARG(_OD_WRITE_CASE, (;), name,                       \
                           _OD_AGG_ENTRY_DATA(entry));                      \
                                                                            \
      default:                                                              \
        break;                                                              \
    }                                                                       \
                                                                            \
    return ODR_OK;                                                          \
  }                                                                         \
                                                                            \
  STRUCT_SECTION_ITERABLE(canopen_od_init,                                  \
                          CONCAT(__od_init_, _OD_AGG_ENTRY_IDX(entry))) = { \
      .idx = _OD_AGG_ENTRY_IDX(entry),                                      \
      .ext =                                                                \
          {                                                                 \
              .read = OD_readOriginal,                                      \
              .write = _OD_AGG_WRITE(_OD_AGG_ENTRY_IDX(entry)),             \
          },                                                                \
  }

/**
 * @brief Define a data aggregration for aggregating OD writes (PDO or SDO) to
 * message @p _name .
 *
 * @param[in] _name Name of the message.
 * @param[in] _init_val Initial value of the message, must be a specified by
 * @ref AGG_DATA_INIT.
 * @param[in] _period Period of data publishing.
 * @param[in] _min_separation Minimum separation time between two data
 * publishing.
 * @param[in] _watermark Watermark to wait for late-arriving members.
 * @param[in] ... Object dictionary entries to be aggregated, must be specified
 * by @ref OD_AGG_ENTRY.
 */
#define CANOPEN_OD_AGG_TO_MSG_DEFINE(_name, _init_val, _period,           \
                                     _min_separation, _watermark, ...)    \
  AGG_TYPED_DEFINE(_OD_AGG_NAME(_name), struct _name, _init_val, _period, \
                   _min_separation, _watermark, canopen_od_agg_publish,   \
                   (void *)&_MSG_CHAN_NAME(_name),                        \
                   _OD_AGG_MEMBERS(__VA_ARGS__));                         \
                                                                          \
  FOR_EACH_FIXED_ARG(_OD_AGG_WRITE_DEFINE, (;), _name, __VA_ARGS__);

/* type ----------------------------------------------------------------------*/
/// @brief Struct for initializing OD entries.
struct canopen_od_init {
  /// @brief Index of the OD entry.
  uint16_t idx;

  /// @brief Extension for the OD entry.
  OD_extension_t ext;
};

/* function declaration ------------------------------------------------------*/
/**
 * @brief Publish function for OD aggregation.
 *
 * @warning Internal use only.
 */
void canopen_od_agg_publish(const void *data, void *user_data);

/**
 * @} // can_open
 */

#endif  // NTURT_CANBUS_CANOPEN_H_
