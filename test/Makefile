
CC = $(CROSS_COMPILE)gcc

# Config for bmetal.mk
USE_GLIBC   ?= n
USE_MUSL    ?= n
USE_NEWLIB  ?= n
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

BINS = \
	args clock ctype \
	flush hello malloc \
	saxpy saxpyf \
	vecadd vecaddf
ifeq ($(USE_GLIBC),y)
BINS += \
	mt_clock pthread
endif

ARGS_OBJS = args.o
CLOCK_OBJS = clock.o
CTYPE_OBJS = ctype.o
FLUSH_OBJS = flush.o
HELLO_OBJS = hello.o
MALLOC_OBJS = malloc.o
MT_CLOCK_OBJS = mt_clock.o
PTHREAD_OBJS = pthread.o
SAXPY_OBJS = saxpy.o
SAXPYF_OBJS = saxpyf.o
VECADD_OBJS = vecadd.o
VECADDF_OBJS = vecaddf.o
OBJS = \
       $(ARGS_OBJS) $(CLOCK_OBJS) $(CTYPE_OBJS) \
       $(FLUSH_OBJS) $(HELLO_OBJS) $(MALLOC_OBJS) \
       $(SAXPY_OBJS) $(SAXPYF_OBJS) \
       $(VECADD_OBJS) $(VECADDF_OBJS)
ifeq ($(USE_GLIBC),y)
OBJS += \
	$(MT_CLOCK_OBJS) $(PTHREAD_OBJS)
endif

all: $(BINS)

args: $(ARGS_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

clock: $(CLOCK_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

ctype: $(CTYPE_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

flush: $(FLUSH_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

hello: $(HELLO_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

malloc: $(MALLOC_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

mt_clock: $(MT_CLOCK_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

pthread: $(PTHREAD_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -lpthread -o $@

saxpy: $(SAXPY_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

saxpyf: $(SAXPYF_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

vecadd: $(VECADD_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

vecaddf: $(VECADDF_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

clean:
	rm -f $(BINS) $(OBJS)

include bmetal.mk
