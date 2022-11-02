# Check configs
ifeq ($(strip $(USE_SYSROOT)),)
  $(error USE_SYSROOT is empty. Please set a path of sysroot)
endif
ifeq ($(strip $(USE_GLIBC)$(USE_MUSL)$(USE_NEWLIB)),nnn)
  $(error Please set USE_GLIBC or USE_MUSL or USE_NEWLIB to y)
endif

# Make additional CPPFLAGS, CFLAGS, CXXFLAGS, LDFLAGS
BAREMETAL_CMNCPPFLAGS = \
	-I $(USE_SYSROOT)/include
BAREMETAL_CMNCFLAGS =
BAREMETAL_CMNCXXFLAGS =
BAREMETAL_CMNLDFLAGS = \
	-static \
	-L $(USE_SYSROOT)/lib \
	-Wl,-T,$(USE_SYSROOT)/include/bmetal/generated/linker_gen.ld \
	-Wl,--print-memory-usage

LDADD = -Wl,--whole-archive,-lbmetal_crt,--no-whole-archive
ifeq ($(USE_GLIBC),y)
  CROSS_COMPILE ?= riscv64-unknown-linux-gnu-
  LDADD += -lpthread
endif
ifeq ($(USE_MUSL),y)
  CROSS_COMPILE ?= riscv64-unknown-linux-musl-
endif
ifeq ($(USE_NEWLIB),y)
  CROSS_COMPILE ?= riscv64-unknown-elf-
endif

CPPFLAGS += $(BAREMETAL_CMNCPPFLAGS)
CFLAGS   += $(BAREMETAL_CMNCFLAGS)
CXXFLAGS += $(BAREMETAL_CMNCXXFLAGS)
LDFLAGS  += $(BAREMETAL_CMNLDFLAGS)
