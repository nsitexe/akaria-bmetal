# SPDX-License-Identifier: Apache-2.0

prj_sources_ifdef(CONFIG_TIMER timer.c)
prj_sources_ifdef(CONFIG_TIMER_SIFIVE_CLINT_TIMER sifive/clint_timer.c)
