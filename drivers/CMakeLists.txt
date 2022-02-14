# SPDX-License-Identifier: Apache-2.0

prj_sources(
  device.c
  io.c
  )

add_subdirectory(cpu)
add_subdirectory_ifdef(CONFIG_CLK clk)
add_subdirectory_ifdef(CONFIG_UART uart)
