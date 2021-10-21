# SPDX-License-Identifier: Apache-2.0

prj_compile_options(-Wall)

if("${ARCH}" STREQUAL "riscv")
  include(${CMAKE_SOURCE_DIR}/cmake/gcc_riscv.cmake)
endif()
