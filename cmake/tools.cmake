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
