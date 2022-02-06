# SPDX-License-Identifier: Apache-2.0

prj_sources_ifdef(CONFIG_UART uart.c)
prj_sources_ifdef(CONFIG_UART_NS16550 uart_ns16550.c)
prj_sources_ifdef(CONFIG_UART_SIFIVE uart_sifive.c)
