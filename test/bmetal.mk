# Check configs
ifeq ($(strip $(USE_SYSROOT)),)
  $(error USE_SYSROOT is empty. Please set a path of sysroot)
endif
ifeq ($(strip $(USE_NEWLIB)$(USE_MUSL)),)
  $(error Neither USE_NEWLIB nor USE_MUSL is set)
endif

# Make additional CFLAGS, LDFLAGS
BAREMETAL_CMNFLAGS = \
	-static \
	-I $(USE_SYSROOT)/include
BAREMETAL_CMNLDFLAGS = \
	-Wl,-T,generated/linker_gen.ld \
	-Wl,--print-memory-usage \
	-L $(USE_SYSROOT)/include/bmetal \
	-L $(USE_SYSROOT)/lib

LDADD = -Wl,--whole-archive,-lbmetal_crt,--no-whole-archive
ifeq ($(USE_GLIBC),y)
  CROSS_COMPILE ?= riscv64-unknown-linux-gnu-
  CPPFLAGS +=
  CFLAGS   += $(BAREMETAL_CMNFLAGS)
  LDFLAGS  += $(BAREMETAL_CMNFLAGS) $(BAREMETAL_CMNLDFLAGS)
endif
ifeq ($(USE_MUSL),y)
  CROSS_COMPILE ?= riscv64-unknown-linux-musl-
  CPPFLAGS +=
  CFLAGS   += $(BAREMETAL_CMNFLAGS)
  LDFLAGS  += $(BAREMETAL_CMNFLAGS) $(BAREMETAL_CMNLDFLAGS)
endif
ifeq ($(USE_NEWLIB),y)
  CROSS_COMPILE ?= riscv64-unknown-elf-
  CPPFLAGS +=
  CFLAGS   += $(BAREMETAL_CMNFLAGS)
  LDFLAGS  += $(BAREMETAL_CMNFLAGS) $(BAREMETAL_CMNLDFLAGS)
endif
