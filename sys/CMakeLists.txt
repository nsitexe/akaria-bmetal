# SPDX-License-Identifier: Apache-2.0

prj_sources(
  file.c
  init.c
  intr.c
  lock.c
  printk.c
  string.c
  syscall.c
  thread.c
  )

add_subdirectory_ifdef(CONFIG_USE_GLIBC glibc)
add_subdirectory_ifdef(CONFIG_USE_MUSL musl)
add_subdirectory_ifdef(CONFIG_USE_NEWLIB newlib)
