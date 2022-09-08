# SPDX-License-Identifier: Apache-2.0

prj_sources(
  clock.c
  file.c
  init.c
  intr.c
  lock.c
  printk.c
  smp.c
  syscall.c
  thread.c
  vsprintf.c
  )

add_subdirectory_ifdef(CONFIG_USE_GLIBC glibc)
add_subdirectory_ifdef(CONFIG_USE_MUSL musl)
add_subdirectory_ifdef(CONFIG_USE_NEWLIB newlib)
add_subdirectory(sys)
