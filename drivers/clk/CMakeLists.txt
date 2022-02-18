# SPDX-License-Identifier: Apache-2.0

prj_sources_ifdef(CONFIG_CLK clk.c)
prj_sources_ifdef(CONFIG_CLK clk_fixed.c)
prj_sources_ifdef(CONFIG_CLK_SIFIVE_PRCI clk_sifive_prci.c)
