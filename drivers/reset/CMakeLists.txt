# SPDX-License-Identifier: Apache-2.0

prj_sources_ifdef(CONFIG_RESET reset.c)
prj_sources_ifdef(CONFIG_RESET_SIFIVE_QEMU_TEST sifive/qemu_test.c)
