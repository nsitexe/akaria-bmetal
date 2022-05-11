# SPDX-License-Identifier: Apache-2.0

prj_sources(
  isr.S
  start.S
  cpu.c
  intr.c
  lock.c
  sync.c
  thread.c
  )

add_subdirectory(${CONFIG_SOC_NAME})
