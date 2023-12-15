# SPDX-License-Identifier: Apache-2.0

prj_sources_ifdef(CONFIG_UART uart.c)
prj_sources_ifdef(CONFIG_UART_NS16550 ns/16550.c)
prj_sources_ifdef(CONFIG_UART_SIFIVE sifive/uart0.c)
prj_sources_ifdef(CONFIG_UART_XILINX_UART_LITE xilinx/uart_lite.c)
