# SPDX-License-Identifier: Apache-2.0

prj_sources(
  isr.S
  start.S
  cpu.c
  lock.c
  sync.c
  )

add_subdirectory(${CONFIG_SOC_NAME})
