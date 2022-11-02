
CC  = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
ADD_AUXDATA ?= $(USE_SYSROOT)/bin/add_auxdata

# Config for bmetal.mk
USE_GLIBC   ?= n
USE_MUSL    ?= n
USE_NEWLIB  ?= n
USE_SYSROOT ?= ./sysroot

# 32bit
#APP_CFLAGS_RV = -march=rv32imafc -mabi=ilp32f -mcmodel=medany
#APP_CFLAGS_RV = -march=rv32imac -mabi=ilp32 -mcmodel=medany
# 64bit
APP_CFLAGS_RV = -march=rv64gc -mabi=lp64d -mcmodel=medany
#APP_CFLAGS_RV = -march=rv64imac -mabi=lp64 -mcmodel=medany

APP_CPPFLAGS = -Wall
APP_CFLAGS   = -O2 -gdwarf-4 $(APP_CFLAGS_RV)
APP_CXXFLAGS = $(APP_CFLAGS)
APP_LDFLAGS  = $(APP_CFLAGS_RV)

CPPFLAGS = $(APP_CPPFLAGS)
CFLAGS   = $(APP_CFLAGS)
CXXFLAGS = $(APP_CXXFLAGS)
LDFLAGS  = $(APP_LDFLAGS)

BINS = \
	args clock \
	crt_string \
	ctype \
	flush hello \
	malloc md5 \
	saxpy saxpyf \
	sgemm \
	vecadd vecaddf \
	coremark/coremark
ifeq ($(USE_GLIBC),y)
BINS += \
	hello_cpp mt_clock \
	mt_vecadd mt_vecaddf \
	pthread
endif

ARGS_OBJS = args.o
CLOCK_OBJS = clock.o
CRT_STRING_OBJS = crt_string.o
CTYPE_OBJS = ctype.o
FLUSH_OBJS = flush.o
HELLO_OBJS = hello.o
HELLO_CPP_OBJS = hello_cpp.o
MALLOC_OBJS = malloc.o
MD5_OBJS = md5.o
MT_CLOCK_OBJS = mt_clock.o
MT_VECADD_OBJS = mt_vecadd.o cmn_vecaddf.o
MT_VECADDF_OBJS = mt_vecaddf.o cmn_vecaddf.o
PTHREAD_OBJS = pthread.o
SAXPY_OBJS = saxpy.o
SAXPYF_OBJS = saxpyf.o
SGEMM_OBJS = sgemm.o
VECADD_OBJS = vecadd.o cmn_vecaddf.o
VECADDF_OBJS = vecaddf.o cmn_vecaddf.o
OBJS = \
       $(ARGS_OBJS) $(CLOCK_OBJS) \
       $(CRT_STRING_OBJS) \
       $(CTYPE_OBJS) \
       $(FLUSH_OBJS) $(HELLO_OBJS) $(HELLO_CPP_OBJS) \
       $(MALLOC_OBJS) $(MD5_OBJS) \
       $(SAXPY_OBJS) $(SAXPYF_OBJS) \
       $(SGEMM_OBJS) \
       $(VECADD_OBJS) $(VECADDF_OBJS)
ifeq ($(USE_GLIBC),y)
OBJS += \
	$(MT_CLOCK_OBJS) \
	$(MT_VECADD_OBJS) $(MT_VECADDF_OBJS) \
	$(PTHREAD_OBJS)
endif

all: all_auxdata

# Add ELF and program headers into special section 'auxdata' of binary
all_auxdata: $(BINS)
	$(ADD_AUXDATA) $(BINS)

args: $(ARGS_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

clock: $(CLOCK_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

crt_string: $(CRT_STRING_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

ctype: $(CTYPE_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

flush: $(FLUSH_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

hello: $(HELLO_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

hello_cpp: $(HELLO_CPP_OBJS)
	$(CXX) $(LDFLAGS) $^ $(LDADD) -o $@

malloc: $(MALLOC_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

md5: $(MD5_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

mt_clock: $(MT_CLOCK_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

mt_vecadd: $(MT_VECADD_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

mt_vecaddf: $(MT_VECADDF_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

pthread: $(PTHREAD_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

saxpy: $(SAXPY_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

saxpyf: $(SAXPYF_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

sgemm: $(SGEMM_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

vecadd: $(VECADD_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

vecaddf: $(VECADDF_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDADD) -o $@

coremark/coremark:
	USE_SYSROOT=../$(USE_SYSROOT) \
	USE_GLIBC=$(USE_GLIBC) \
	USE_NEWLIB=$(USE_NEWLIB) \
	CC="$(CC)" \
	CPPFLAGS="$(APP_CPPFLAGS)" \
	CFLAGS="$(APP_CFLAGS)" \
	LDFLAGS="$(APP_LDFLAGS)" \
	$(MAKE) -C coremark

clean:
	rm -f $(BINS) $(OBJS)
	$(MAKE) -C coremark $@

include bmetal.mk
