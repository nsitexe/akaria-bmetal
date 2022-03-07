# SPDX-License-Identifier: Apache-2.0

prj_sources(
  start.S
  cpu.c
  )

add_subdirectory(${CONFIG_SOC_NAME})
