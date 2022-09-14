PORT_DIR ?= posix
CFLAGS ?= -O2 -g

BINS = coremark

COREMARK_OBJS = \
	core_main.o core_list_join.o core_util.o \
	core_matrix.o core_state.o \
	$(PORT_DIR)/core_portme.o

OBJS = $(COREMARK_OBJS)

CPPFLAGS += -I ./ -I $(PORT_DIR)/

all: $(BINS)

coremark: $(COREMARK_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

clean:
	rm -f $(BINS) $(OBJS)

ifdef USE_SYSROOT
include ../bmetal.mk
endif

DUMP_CFLAGS := $(CFLAGS)
CFLAGS += -DFLAGS_STR="\"$(DUMP_CFLAGS)\""
