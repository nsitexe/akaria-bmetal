# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.16)
enable_language(C)

project(add_auxdata)
set(PRJ_NAME add_auxdata)
add_executable(${PRJ_NAME})
target_compile_options(${PRJ_NAME}
  PRIVATE -Wall
  )
target_sources(${PRJ_NAME}
  PRIVATE add_auxdata.c
  )
install(
  TARGETS ${PRJ_NAME}
  )
