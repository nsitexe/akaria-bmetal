CROSS_COMPILE ?= riscv64-unknown-elf-

CC = $(CROSS_COMPILE)gcc

# 32bit
#CMNFLAGS = -march=rv32imafc -mabi=ilp32f -mcmodel=medany
# 64bit
CMNFLAGS = -march=rv64gc -mabi=lp64d -mcmodel=medany
CPPFLAGS = -Wall -O2 -g
CFLAGS   = $(CMNFLAGS)
LDFLAGS  = $(CMNFLAGS)

BINS = hello vecadd
OBJS = $(HELLO_OBJS) $(VECADD_OBJS)
HELLO_OBJS = hello.o
VECADD_OBJS = vecadd.o

all: $(BINS)

hello: $(HELLO_OBJS)
	$(CC) $(LDFLAGS) $< $(LDADD) -o $@

vecadd: $(VECADD_OBJS)
	$(CC) $(LDFLAGS) $< $(LDADD) -o $@

clean:
	rm -f $(BINS) $(OBJS)

# config
USE_NEWLIB  ?= y
USE_MUSL    ?= n
USE_SYSROOT ?= ./sysroot

include bmetal.mk
