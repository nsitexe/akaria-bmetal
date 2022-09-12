# SPDX-License-Identifier: Apache-2.0

# Common
prj_compile_options(
  -Wall -nostdlib -ffreestanding
  $<$<CONFIG:Debug>:-gdwarf-4>
  $<$<CONFIG:Release>:>
  $<$<CONFIG:MinSizeRel>:>
  $<$<CONFIG:RelWithDebInfo>:-gdwarf-4>
)

# Assembler
prj_compile_options($<$<COMPILE_LANGUAGE:ASM>:-DASMLANG>)

# Architecture dependent
set(SPACER "----")
message("${SPACER} arch  is ${ARCH}")
include(${CMAKE_SOURCE_DIR}/cmake/clang_${ARCH}.cmake)
