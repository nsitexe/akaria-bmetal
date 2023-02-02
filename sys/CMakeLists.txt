# SPDX-License-Identifier: Apache-2.0

prj_sources(
  clock.c
  event.c
  file.c
  fini.c
  init.c
  intr.c
  lock.c
  printk.c
  smp.c
  syscall.c
  thread.c
  vsprintf.c
  )

add_subdirectory_ifdef(CONFIG_USE_LIBC libc)
add_subdirectory_ifdef(CONFIG_USE_NEWLIB newlib)
add_subdirectory(sys)
