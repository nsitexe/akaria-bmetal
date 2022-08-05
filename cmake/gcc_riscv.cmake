# SPDX-License-Identifier: Apache-2.0

if(CONFIG_64BIT)
  set(RV_ARCH rv64im)
  set(RV_MABI lp64)
  prj_compile_options(-mcmodel=medany)
else()
  set(RV_ARCH rv32im)
  set(RV_MABI ilp32)
endif()

if(CONFIG_RISCV_ATOMIC)
  set(RV_ARCH ${RV_ARCH}a)
endif()

if(CONFIG_FPU)
  if(CONFIG_64BIT)
    set(RV_ARCH ${RV_ARCH}fd)
    set(RV_MABI ${RV_MABI}d)
  else()
    set(RV_ARCH ${RV_ARCH}f)
    set(RV_MABI ${RV_MABI}f)
  endif()
endif()

if(CONFIG_RISCV_COMPRESSED)
  set(RV_ARCH ${RV_ARCH}c)
endif()

# From binutils 2.38
set(RV_ARCH ${RV_ARCH}_zicsr_zifencei)

prj_compile_options(-march=${RV_ARCH})
prj_compile_options(-mabi=${RV_MABI})

message("${SPACER} march is '${RV_ARCH}'")
message("${SPACER} mabi  is '${RV_MABI}'")
