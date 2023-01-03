CFLAGS ?= -O2 -g

BINS = dhrystone

DHRYSTONE_OBJS = \
	dhry_1.o dhry_2.o

OBJS = $(DHRYSTONE_OBJS)

CPPFLAGS += \
	-I ./ \
	-DTIMES

all: $(BINS)

dhrystone: $(DHRYSTONE_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

.PHONY: clean
clean:
	rm -f $(BINS) $(OBJS)

ifdef USE_SYSROOT
include ../bmetal.mk
endif

#DUMP_CFLAGS := $(CFLAGS)
#CFLAGS += -DFLAGS_STR="\"$(DUMP_CFLAGS)\""#
