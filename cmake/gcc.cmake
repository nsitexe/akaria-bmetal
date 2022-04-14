# SPDX-License-Identifier: Apache-2.0

# Common
prj_compile_options(
  -Wall -nostdlib -ffreestanding
  $<$<CONFIG:Debug>:>
  $<$<CONFIG:Release>:>
  $<$<CONFIG:MinSizeRel>:>
  $<$<CONFIG:RelWithDebInfo>:>
)

# Assembler
prj_compile_options($<$<COMPILE_LANGUAGE:ASM>:-DASMLANG>)

if("${ARCH}" STREQUAL "riscv")
  include(${CMAKE_SOURCE_DIR}/cmake/gcc_riscv.cmake)
endif()
