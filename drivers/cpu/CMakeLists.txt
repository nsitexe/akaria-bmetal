# SPDX-License-Identifier: Apache-2.0

prj_sources(
  cpu.c
  )

prj_sources_ifdef(CONFIG_RISCV riscv.c)
