# SPDX-License-Identifier: Apache-2.0

# Common
prj_compile_options(
  -Wall -nostdlib -ffreestanding -gdwarf-4
  $<$<CONFIG:Debug>:>
  $<$<CONFIG:Release>:>
  $<$<CONFIG:MinSizeRel>:>
  $<$<CONFIG:RelWithDebInfo>:>
)

# Assembler
prj_compile_options($<$<COMPILE_LANGUAGE:ASM>:-DASMLANG>)

# Architecture depenent
set(SPACER "----")
message("${SPACER} arch  is ${ARCH}")
include(${CMAKE_SOURCE_DIR}/cmake/gcc_${ARCH}.cmake)
