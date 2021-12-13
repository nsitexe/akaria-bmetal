# SPDX-License-Identifier: Apache-2.0

prj_sources(
  init.c
  printk.c
  syscall.c
  thread.c
  )

add_subdirectory_ifdef(CONFIG_USE_MUSL musl)
add_subdirectory_ifdef(CONFIG_USE_NEWLIB newlib)
