# Check configs
ifeq ($(strip $(USE_SYSROOT)),)
  $(error USE_SYSROOT is empty. Please set a path of sysroot)
endif
ifeq ($(strip $(USE_NEWLIB)$(USE_MUSL)),)
  $(error Neither USE_NEWLIB nor USE_MUSL is set)
endif

# Make additional CFLAGS, LDFLAGS
BAREMETAL_CMNFLAGS = -static -nostdlib --sysroot=$(USE_SYSROOT)
BAREMETAL_CMNLDFLAGS = \
	-Wl,-T,generated/linker_gen.ld \
	-L $(USE_SYSROOT)/include/bmetal

LDADD += -Wl,--whole-archive,-lbmetal_crt,--no-whole-archive,-lgcc
ifeq ($(USE_NEWLIB),y)
  CPPFLAGS +=
  CFLAGS   += $(BAREMETAL_CMNFLAGS)
  LDFLAGS  += $(BAREMETAL_CMNFLAGS) $(BAREMETAL_CMNLDFLAGS)
  LDADD    += -Wl,-lc,-lgloss
endif
ifeq ($(USE_MUSL),y)
  CPPFLAGS +=
  CFLAGS   += $(BAREMETAL_CMNFLAGS)
  LDFLAGS  += $(BAREMETAL_CMNFLAGS) $(BAREMETAL_CMNLDFLAGS)
  LDADD    += -Wl,-lc
endif
LDADD += -Wl,-lbmetal_crt,-lgcc
