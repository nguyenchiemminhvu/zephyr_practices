# Example utilizing J-Link configurations for Cortex-R5 targets
board_runner_args(jlink "--device=J721E_CORTEX_R5" "--speed=4000")
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)