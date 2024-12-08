board_runner_args(jlink "--device=STM32H562RG" "--speed=4000")
board_runner_args(pyocd "--target=stm32h562rgtx")

# include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
include(${ZEPHYR_BASE}/boards/common/pyocd.board.cmake)
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)
