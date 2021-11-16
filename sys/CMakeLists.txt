# SPDX-License-Identifier: Apache-2.0

prj_sources(
  init.c
  )

add_subdirectory_ifdef(CONFIG_USE_NEWLIB newlib)
