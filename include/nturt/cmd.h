/**
 * @file
 * @brief Command module.
 *
 * @author quantumspawner
 * @version 0.0.1
 * @date 2025-01-05
 * @copyright Copyright (c) 2025 NTU Racing Team
 */

#ifndef NTURT_CMD_H_
#define NTURT_CMD_H_

// glibc include
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// zephyr include
#include <zephyr/kernel.h>
#include <zephyr/sys/iterable_sections.h>
#include <zephyr/sys/util.h>

/**
 * @addtogroup cmd Command
 * @{
 */

/* macro ---------------------------------------------------------------------*/
/**
 * @brief Indicate that the command has no handler of this type.
 *
 */
#define CMD_NO_HANDLER 0

/**
 * @brief Define a command.
 *
 * @param[in] opcode Command opcode.
 * @param[in] immed Immediate handler of type @ref cmd_immed_handler_t, or @ref
 * CMD_NO_HANDLER if not required.
 * @param[in] deffered Deffered handler of type @ref cmd_deffered_handler_t, or
 * @ref CMD_NO_HANDLER if not required.
 * @param[in] user_data Pointer to custom data of the command.
 *
 * @note Since the name of the command is automatically generated using
 * @p opcode , the name would be illegal if @p opcode contains characters other
 * than alphanumericals or underscores. To avoid this, _CMD_DEFINE can be used
 * directly by providing a unique name.
 */
#define CMD_DEFINE(opcode, immed, deffered, user_data) \
  _CMD_DEFINE(__cmd_##opcode, opcode, immed, deffered, user_data)

#define _CMD_DEFINE(_name, _opcode, _immed, _deffered, _user_data)            \
  STRUCT_SECTION_ITERABLE(cmd, _name) = {                                     \
      .opcode = _opcode,                                                      \
      .immed = COND_CODE_1(IS_EQ(_immed, CMD_NO_HANDLER), (NULL), (_immed)),  \
      .deffered =                                                             \
          COND_CODE_1(IS_EQ(_deffered, CMD_NO_HANDLER), (NULL), (_deffered)), \
      .user_data = _user_data,                                                \
  }

#define _CMD_DEFINE_TYPED(name, opcode, immed, deffered, user_data, type)    \
  static int CONCAT(__cmd_immed_, name)(                                     \
      void *_user_data, const void *_operand, size_t _operand_size) {        \
    if (_operand_size != sizeof(type)) {                                     \
      return -EINVAL;                                                        \
    }                                                                        \
    COND_CODE_1(IS_EQ(immed, CMD_NO_HANDLER), (return 0;),                   \
                (type operand; memcpy(&operand, _operand, sizeof(type));     \
                 return immed(operand, _user_data);))                        \
  }                                                                          \
  IF_DISABLED(                                                               \
      IS_EQ(deffered, CMD_NO_HANDLER),                                       \
      (static void CONCAT(__cmd_deffered_, name)(                            \
          void *_user_data, void const *_operand, size_t _operand_size) {    \
        (void)_operand_size;                                                 \
        type operand;                                                        \
        memcpy(&operand, _operand, sizeof(type));                            \
        deffered(operand, _user_data);                                       \
      }))                                                                    \
  _CMD_DEFINE(name, opcode, CONCAT(__cmd_immed_, name),                      \
              COND_CODE_1(IS_EQ(deffered, CMD_NO_HANDLER), (CMD_NO_HANDLER), \
                          (CONCAT(__cmd_deffered_, name))),                  \
              user_data)

/**
 * @brief Invoke a command with specific operand type.
 *
 * @param[in] _opcode Command opcode.
 * @param[in] _operand Operand for the command.
 * @param[in] _operand_type Type of the operand.
 *
 * @return Same as @ref cmd_invoke.
 */
#define cmd_invoke_typed(_opcode, _operand, _operand_type) \
  __extension__({                                          \
    _operand_type operand = _operand;                      \
    cmd_invoke(_opcode, &operand, sizeof(_operand_type));  \
  })

/**
 * @brief Invoke a command with operand type inferred from @p operand .
 *
 * @param[in] opcode Command opcode.
 * @param[in] operand Operand for the command.
 *
 * @return Same as @ref cmd_invoke.
 */
#define cmd_invoke_auto(opcode, operand) \
  cmd_invoke(opcode, &operand, sizeof(operand))

/* type ----------------------------------------------------------------------*/
/**
 * @brief Immediate handler of a command.
 *
 * @param[in] opcode Command opcode.
 * @param[in] operand Operand for the command.
 * @param[in] user_data Custom data of the command.
 *
 * @retval 0 For success.
 * @retval -EINVAL If the operand is invalid.
 * @retval others Negative error number.
 */
typedef int (*cmd_immed_handler_t)(uint32_t opcode, const void *operand,
                                   size_t operand_size, void *user_data);

/**
 * @brief Deffered handler of a command.
 *
 * @param[in] opcode Command opcode.
 * @param[in] operand Operand for the command.
 * @param[in] user_data Custom data of the command.
 */
typedef void (*cmd_deffered_handler_t)(uint32_t opcode, const void *operand,
                                       size_t operand_size, void *user_data);

/**
 * @brief Command.
 *
 */
struct cmd {
  /// @brief Opcode of the command.
  const uint32_t opcode;

  /// @brief Immediate handler.
  const cmd_immed_handler_t immed;

  /// @brief Deffered handler.
  const cmd_deffered_handler_t deffered;

  /// @brief Custom data of the command.
  void *const user_data;
};

/* function declaration ------------------------------------------------------*/
/**
 * @brief Invoke a command.
 *
 * @param[in] opcode Command opcode.
 * @param[in] operand Operand for the command.
 * @param[in] operand_size Size of the operand.
 *
 * @retval 0 For success.
 * @retval -ENOENT If the opcode does not exist.
 * @retval -EINVAL If the operand is invalid.
 * @retval -ENOMEM If the buffer for deffered handler is full.
 * @retval others Negative error number returned by immediate handler.
 */
int cmd_invoke(uint32_t opcode, void *operand, size_t operand_size);

/**
 * @brief Invoke a command without operand.
 *
 * @param[in] opcode Command opcode.
 *
 * @return Same as @ref cmd_invoke.
 */
static inline int cmd_invoke_void(uint32_t opcode) {
  return cmd_invoke(opcode, NULL, 0);
}

/**
 * @} // Command
 */

#endif  // NTURT_CMD_H_
