# SPDX-License-Identifier: Apache-2.0

prj_sources(
  start.S
  init.c
  )

add_subdirectory(${CONFIG_SOC_NAME})
