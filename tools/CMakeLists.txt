# SPDX-License-Identifier: Apache-2.0

project(add_auxdata)

cmake_minimum_required(VERSION 3.16)
enable_language(C)

set(PRJ_NAME add_auxdata)

add_executable(${PRJ_NAME})

target_sources(${PRJ_NAME}
  PRIVATE add_auxdata.c
  )
