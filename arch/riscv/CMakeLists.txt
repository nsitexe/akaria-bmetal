# SPDX-License-Identifier: Apache-2.0

prj_sources(
  isr.S
  start.S
  switch.S
  cpu.c
  intr.c
  lock.c
  sync.c
  thread.c
  )

add_subdirectory_ifdef(CONFIG_USE_NEWLIB newlib)
add_subdirectory(${CONFIG_SOC_NAME})
