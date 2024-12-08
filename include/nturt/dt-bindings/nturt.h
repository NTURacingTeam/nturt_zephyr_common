#ifndef NTURT_INCLUDE_DT_BINDINGS_NTURT_H_
#define NTURT_INCLUDE_DT_BINDINGS_NTURT_H_

// zephyr includes
#include <zephyr/dt-bindings/input/input-event-codes.h>

/// @brief Dashboard LED numbers.
#define LED_NUM_RTD 0

#define LED_NUM_MATRIX_OFFSET 1
#define LED_NUM_ERR (LED_NUM_MATRIX_OFFSET + 5)
#define LED_NUM_RUNNING (LED_NUM_MATRIX_OFFSET + 13)
#define LED_NUM_PEDAL_PLAUS (LED_NUM_MATRIX_OFFSET + 9)
#define LED_NUM_MODE (LED_NUM_MATRIX_OFFSET + 1)

/// @brief Input codes.
#define INPUT_NTURT_START 1024

/// @brief Dashboard button numbers.
#define BTN_NUM_RTD 0
#define BTN_NUM_RESET 1
#define BTN_NUM_MODE 2
#define BTN_NUM_SET_HOME 3

#endif  // NTURT_INCLUDE_DT_BINDINGS_NTURT_H_
