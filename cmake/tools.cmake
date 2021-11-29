# SPDX-License-Identifier: Apache-2.0

# Do add_subdirectory()
# if conf is ON, YES, TRUE, Y or non-zero.
function(add_subdirectory_ifdef conf dir)
  if(${${conf}})
    add_subdirectory(${dir} ${ARGN})
  endif()
endfunction()

# Do target_sources() for project
function(prj_sources src)
  target_sources(${PRJ_LIB_NAME} PRIVATE ${src} ${ARGN})
endfunction()

function(prj_sources_ifdef conf src)
  if(${${conf}})
    prj_sources(${src} ${ARGN})
  endif()
endfunction()

# Do target_include_directories() for project
function(prj_include_directories scope item)
  target_include_directories(${PRJ_LIB_NAME} ${scope} ${item} ${ARGN})
endfunction()

function(prj_include_directories_ifdef conf scope item)
  if(${${conf}})
    prj_include_directories(${scope} ${item} ${ARGN})
  endif()
endfunction()

# Do target_compile_options() for project
function(prj_compile_options opt)
  target_compile_options(${PRJ_LIB_NAME} PRIVATE ${opt} ${ARGN})
endfunction()

function(prj_compile_options_ifdef conf opt)
  if(${${conf}})
    prj_compile_options(${opt} ${ARGN})
  endif()
endfunction()

# Import Kconfig-like config variables to cmake environment
# from a specified config file.
#
# Comment:
#   Start with '#' character.
# Config:
#   Start with CONFIG_ prefix. Cannot use '=' for config name.
# Value:
#   Enable to use bool (y is defined, n is not defined), number
#   and string. Value is placed after '=' character.
#
# Example:
#   CONFIG_AAA=y
#   CONFIG_BBB=n
#   CONFIG_CCC=100
#   CONFIG_DDD="string"
function(import_config config_file)
  # Remove comments
  file(
    STRINGS
    ${config_file}
    CONFIG_LIST
    REGEX "^CONFIG_"
  )

  foreach(CONFIG ${CONFIG_LIST})
    # Separate name and value
    string(REGEX MATCH "[^=]+" CONFIG_NAME ${CONFIG})
    string(REGEX REPLACE "[^=]+=" "" CONFIG_VAL ${CONFIG})

    # Remove quotation marks
    string(REGEX REPLACE "^\"" "" CONFIG_VAL ${CONFIG_VAL})
    string(REGEX REPLACE "\"$" "" CONFIG_VAL ${CONFIG_VAL})

    set(${CONFIG_NAME} ${CONFIG_VAL} PARENT_SCOPE)
  endforeach()
endfunction()

# [Tentative]
# Write autoconf.h from a specified config file.
#
# A config macro is ...:
#   - defined as 1 if the config has boolean value 'y'.
#   - not defined if the config boolean value 'n'.
#   - defined as number if the config has numerical value.
#   - defined as string if the config has string value.
function(write_autoconf config_file autoconf_file)
  # Remove comments
  file(
    STRINGS
    ${config_file}
    CONFIG_LIST
    REGEX "^CONFIG_"
  )

  # Clear output file
  file(
    WRITE
    ${autoconf_file}
    ""
  )

  foreach(CONFIG ${CONFIG_LIST})
    # Separate name and value
    string(REGEX MATCH "[^=]+" CONFIG_NAME ${CONFIG})
    string(REGEX REPLACE "[^=]+=" "" CONFIG_VAL ${CONFIG})

    if(${CONFIG_VAL} STREQUAL "n")
      continue()
    endif()
    if(${CONFIG_VAL} STREQUAL "y")
      set(CONFIG_VAL 1)
    endif()

    file(
      APPEND
      ${autoconf_file}
      "#define ${CONFIG_NAME} ${CONFIG_VAL}\n"
    )
  endforeach()
endfunction()

# Show toolchain settings
set(SPACER "----")

if(NOT DEFINED CC)
  set(CC gcc)
endif()
set(CMAKE_C_COMPILER ${CROSS_COMPILE}${CC})
message("${SPACER} CC is '${CC}'")
message("${SPACER} Compiler is '${CMAKE_C_COMPILER}'")

if(NOT DEFINED CCASM)
  set(CCASM ${CC})
endif()
set(CMAKE_ASM_COMPILER ${CROSS_COMPILE}${CCASM})
message("${SPACER} CCASM is '${CCASM}'")
message("${SPACER} Assembler is '${CMAKE_ASM_COMPILER}'")
