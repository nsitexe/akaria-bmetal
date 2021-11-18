CROSS_COMPILE ?= riscv64-unknown-elf-

CC = $(CROSS_COMPILE)gcc

CMNFLAGS = -march=rv64gc -mabi=lp64d -mcmodel=medany
CPPFLAGS = -Wall -O2 -g
CFLAGS   = $(CMNFLAGS)
LDFLAGS  = $(CMNFLAGS)

BINS = hello
HELLO_OBJS = hello.o

all: $(BINS)

hello: $(HELLO_OBJS)
	$(CC) $(LDFLAGS) $< $(LDADD) -o $@

clean:
	rm -f $(BINS) $(HELLO_OBJS)

# config
USE_NEWLIB  ?= y
USE_MUSL    ?= n
USE_SYSROOT ?= ./sysroot

include bmetal.mk
