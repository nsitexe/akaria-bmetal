# SPDX-License-Identifier: Apache-2.0

# Check variables
if(NOT DEFINED PRJ_CONFIG_NAME)
  message(FATAL_ERROR "PRJ_CONFIG_NAME is not defined.")
endif()
if(NOT DEFINED PRJ_CONFIG_CACHE_NAME)
  message(FATAL_ERROR "PRJ_CONFIG_CACHE_NAME is not defined.")
endif()
if(NOT DEFINED PRJ_CONFIG_NOTIFY_NAME)
  message(FATAL_ERROR "PRJ_CONFIG_NOTIFY_NAME is not defined.")
endif()
if(NOT DEFINED PRJ_AUTOCONF_H_NAME)
  message(FATAL_ERROR "PRJ_AUTOCONF_H_NAME is not defined.")
endif()
if(NOT DEFINED PRJ_DEFCONFIG_NAME)
  message(FATAL_ERROR "PRJ_DEFCONFIG_NAME is not defined.")
endif()

# File path for Kconfig
set(PRJ_CONFIG_FILE ${CMAKE_BINARY_DIR}/${PRJ_CONFIG_NAME})
set(PRJ_CONFIG_CACHE_FILE ${CMAKE_BINARY_DIR}/${PRJ_CONFIG_CACHE_NAME})
set(PRJ_CONFIG_NOTIFY_FILE ${CMAKE_BINARY_DIR}/${PRJ_CONFIG_NOTIFY_NAME})
set(PRJ_AUTOCONF_H_FILE ${CMAKE_BINARY_DIR}/include/bmetal/generated/${PRJ_AUTOCONF_H_NAME})
set(PRJ_DEFCONFIG_FILE ${CMAKE_SOURCE_DIR}/${PRJ_DEFCONFIG_NAME})

# Target: Default config
# TODO: Use suitable tools for Kconfig
add_custom_target(
  defconfig
  COMMAND cp ${CMAKE_SOURCE_DIR}/config ${PRJ_CONFIG_FILE}
  COMMAND touch ${PRJ_CONFIG_NOTIFY_FILE}
  )

# Target: Config
# TODO: Use suitable tools for Kconfig
add_custom_target(
  config
  COMMAND touch ${PRJ_CONFIG_NOTIFY_FILE}
  )

# Run defconfig silently and create cache if each file does not exist
# TODO: Use suitable tools for Kconfig
if(NOT EXISTS ${PRJ_CONFIG_FILE})
  execute_process(
    COMMAND cp ${CMAKE_SOURCE_DIR}/config ${PRJ_CONFIG_FILE}
    COMMAND cp ${CMAKE_SOURCE_DIR}/config ${PRJ_CONFIG_CACHE_FILE}
    )
endif()
execute_process(
  COMMAND cp ${PRJ_CONFIG_FILE} ${PRJ_CONFIG_CACHE_FILE}
  )
file(WRITE ${PRJ_CONFIG_NOTIFY_FILE} "")
include(${PRJ_CONFIG_NOTIFY_FILE})

# Target: Cache of config
# If config is changed, run cmake again.
add_custom_target(
  cache_config
  COMMAND
    ${CMAKE_COMMAND} -E compare_files
      "${PRJ_CONFIG_FILE}" "${PRJ_CONFIG_CACHE_FILE}" ";"
    if [ $$? -ne 0 ]; then
      ${CMAKE_COMMAND} -E copy
        ${PRJ_CONFIG_FILE} ${PRJ_CONFIG_CACHE_FILE} ";"
      ${CMAKE_COMMAND} -E touch
        ${PRJ_CONFIG_NOTIFY_FILE} ";"
      ${CMAKE_MAKE_PROGRAM} ";"
    fi
  )
add_dependencies(${PRJ_LIB_NAME} cache_config)

# Import configs
# After that, we can use CONFIG_ABCD variables in CMake
import_config(${PRJ_CONFIG_FILE})
