# SPDX-License-Identifier: Apache-2.0

project(baremetal_crt)

cmake_minimum_required(VERSION 3.16)
enable_language(C ASM)

# Additional host tools
# Need to add these dirs before changing CMAKE_C_COMPILER.
add_subdirectory(tool)

# Check ARCH
if(NOT DEFINED ARCH)
  message(FATAL_ERROR "ARCH is not defined. Please set -DARCH= option.")
endif()
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/arch/${ARCH})
  message(FATAL_ERROR "ARCH '${ARCH}' is not supported.")
endif()

# Check CROSS_COMPILE
if(NOT DEFINED CROSS_COMPILE)
  message(FATAL_ERROR "CROSS_COMPILE is not defined. Please set -DCROSS_COMPILE= option.")
endif()

# Adjust find_something commands for cross compile
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Common settings
set(PRJ_LIB_NAME bmetal_crt)
set(PRJ_CONFIG_NAME config_cur)
set(PRJ_CONFIG_CACHE_NAME config_cache)
set(PRJ_CONFIG_NOTIFY_NAME config_notify.cmake)
set(PRJ_DEFCONFIG_NAME defconfig)
set(PRJ_AUTOCONF_H_DIR include/bmetal/generated)
set(PRJ_AUTOCONF_H_NAME autoconf.h)
set(PRJ_LINKER_SCRIPT_NAME linker.ld)
set(PRJ_GEN_LINKER_SCRIPT_NAME linker_gen.ld)

# CMake basic tools to add/use directories, sources and configs
include(${CMAKE_SOURCE_DIR}/cmake/tools.cmake)

# Target: Library
add_library(${PRJ_LIB_NAME} STATIC)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Default config directory
set(CONF_DIR ${CMAKE_SOURCE_DIR}/config)
message("${SPACER} DEFCONF is '${DEFCONF}'")
message("${SPACER} CONF dir is '${CONF_DIR}'")

# Use Kconfig like configuration system.
# After that, we can use CONFIG_ABCD variables in CMake
include(${CMAKE_SOURCE_DIR}/cmake/kconfig.cmake)

set(ARCH_DIR ${CMAKE_SOURCE_DIR}/arch/${ARCH})
set(SOC_DIR ${ARCH_DIR}/${CONFIG_SOC_NAME})
set(BOARD_DIR ${CMAKE_SOURCE_DIR}/board/${ARCH}/${CONFIG_BOARD_NAME})

message("${SPACER} ARCH dir is '${ARCH_DIR}'")
message("${SPACER} SOC dir is '${SOC_DIR}'")
message("${SPACER} BOARD dir is '${BOARD_DIR}'")


# File path for linker script
set(BOARD_LINKER_SCRIPT_FILE ${BOARD_DIR}/${PRJ_LINKER_SCRIPT_NAME})
set(SOC_LINKER_SCRIPT_FILE ${SOC_DIR}/${PRJ_LINKER_SCRIPT_NAME})
if(EXISTS ${BOARD_LINKER_SCRIPT_FILE})
  set(PRJ_LINKER_SCRIPT_FILE ${BOARD_LINKER_SCRIPT_FILE})
else()
  set(PRJ_LINKER_SCRIPT_FILE ${SOC_LINKER_SCRIPT_FILE})
endif()
if(NOT EXISTS ${PRJ_LINKER_SCRIPT_FILE})
  message(FATAL_ERROR "A linker script '${PRJ_LINKER_SCRIPT_FILE}' is not exists.")
endif()

set(PRJ_GEN_LINKER_SCRIPT_FILE ${CMAKE_BINARY_DIR}/include/bmetal/generated/${PRJ_GEN_LINKER_SCRIPT_NAME})

# Target: Linker script
add_custom_command(
  OUTPUT ${PRJ_GEN_LINKER_SCRIPT_FILE}
  DEPENDS ${PRJ_LINKER_SCRIPT_FILE}
  COMMAND
    ${CMAKE_C_COMPILER}
    -I ${CMAKE_SOURCE_DIR}/include
    -I ${CMAKE_BINARY_DIR}/include
    -I ${ARCH_DIR}/include
    -DLINKER_SCRIPT
    -E -P -x c -o ${PRJ_GEN_LINKER_SCRIPT_FILE}
    ${PRJ_LINKER_SCRIPT_FILE}
  )
prj_sources(
  ${PRJ_GEN_LINKER_SCRIPT_FILE}
  )


# Compiler settings
if("${CC}" STREQUAL "gcc")
  include(${CMAKE_SOURCE_DIR}/cmake/gcc.cmake)
endif()
if("${CC}" STREQUAL "clang")
  include(${CMAKE_SOURCE_DIR}/cmake/clang.cmake)
endif()

# Install, include settings
install(
  TARGETS ${PRJ_LIB_NAME}
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )
install(
  DIRECTORY
  ${CMAKE_SOURCE_DIR}/include/bmetal/app/bmetal
  TYPE INCLUDE
  )
install(
  DIRECTORY
  ${CMAKE_BINARY_DIR}/include/bmetal
  TYPE INCLUDE
  )
prj_include_directories(
  PRIVATE ${CMAKE_SOURCE_DIR}/include
  PRIVATE ${CMAKE_BINARY_DIR}/include
  PRIVATE ${ARCH_DIR}/include
  )

add_subdirectory(arch)
add_subdirectory(sys)
add_subdirectory(board)
add_subdirectory(driver)

# Document settings
set(DOXYGEN_GENERATE_HTML YES)
set(DOXYGEN_GENERATE_MAN NO)

find_package(
  Doxygen
  REQUIRED dot
  )

if (DEFINED DOXYGEN_FOUND)
  doxygen_add_docs(
    doxygen
    arch board driver include sys
    )
endif()
