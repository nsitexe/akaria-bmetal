
CC = $(CROSS_COMPILE)gcc

# Config for bmetal.mk
USE_GLIBC   ?= n
USE_MUSL    ?= n
USE_NEWLIB  ?= y
USE_SYSROOT ?= ./sysroot

# 32bit
#CMNFLAGS = -march=rv32imafc -mabi=ilp32f -mcmodel=medany
#CMNFLAGS = -march=rv32imac -mabi=ilp32 -mcmodel=medany
# 64bit
CMNFLAGS = -march=rv64gc -mabi=lp64d -mcmodel=medany
#CMNFLAGS = -march=rv64imac -mabi=lp64 -mcmodel=medany
CPPFLAGS = -Wall -O2 -gdwarf-4
CFLAGS   = $(CMNFLAGS)
LDFLAGS  = $(CMNFLAGS)

BINS = hello ctype vecadd
OBJS = $(HELLO_OBJS) $(CTYPE_OBJS) $(PTHREAD_OBJS) $(VECADD_OBJS)
ifeq ($(USE_GLIBC),y)
BINS += pthread
endif

HELLO_OBJS = hello.o
CTYPE_OBJS = ctype.o
PTHREAD_OBJS = pthread.o
VECADD_OBJS = vecadd.o

all: $(BINS)

hello: $(HELLO_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

ctype: $(CTYPE_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

pthread: $(PTHREAD_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -lpthread -o $@

vecadd: $(VECADD_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

clean:
	rm -f $(BINS) $(OBJS)

include bmetal.mk
