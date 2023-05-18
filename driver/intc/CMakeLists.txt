# SPDX-License-Identifier: Apache-2.0

prj_sources_ifdef(CONFIG_INTC intc.c)
prj_sources_ifdef(CONFIG_INTC_RISCV_PRIV riscv/rv_priv.c)
prj_sources_ifdef(CONFIG_INTC_SIFIVE_CLINT sifive/clint.c)
prj_sources_ifdef(CONFIG_INTC_SIFIVE_PLIC sifive/plic.c)
