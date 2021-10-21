# SPDX-License-Identifier: Apache-2.0

if(CONFIG_64BIT)
  prj_compile_options(-march=rv64gc)
  prj_compile_options(-mabi=lp64)
  prj_compile_options(-mcmodel=medany)
endif()

if(CONFIG_32BIT)
  prj_compile_options(-march=rv32imafc)
  prj_compile_options(-mabi=ilp32)
endif()
