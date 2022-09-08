/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SYS_TIME_H_
#define BAREMETAL_CRT_SYS_TIME_H_

#include <stdint.h>

#include <bmetal/bmetal.h>
#include <bmetal/sys/types.h>

struct timespec {
	time_t tv_sec;
	long tv_nsec;
};

struct timespec64 {
	int64_t tv_sec;
	int64_t tv_nsec;
};

struct timespec32 {
	int32_t tv_sec;
	int32_t tv_nsec;
};

struct timeval {
	time_t tv_sec;
	suseconds_t tv_usec;
};

/* IDs of clockid_t (use same value as Linux) */
#define CLOCK_REALTIME              0
#define CLOCK_MONOTONIC             1
#define CLOCK_PROCESS_CPUTIME_ID    2
#define CLOCK_THREAD_CPUTIME_ID     3
#define CLOCK_MONOTONIC_RAW         4
#define CLOCK_REALTIME_COARSE       5
#define CLOCK_MONOTONIC_COARSE      6
#define CLOCK_BOOTTIME              7
#define CLOCK_REALTIME_ALARM        8
#define CLOCK_BOOTTIME_ALARM        9

#ifndef timespecadd
/* Same as timeradd() for struct timespec */
#define timespecadd(a, b, res)                                \
	do {                                                  \
		(res)->tv_sec = (a)->tv_sec + (b)->tv_sec;    \
		(res)->tv_nsec = (a)->tv_nsec + (b)->tv_nsec; \
		if ((res)->tv_nsec >= 1000000000L) {          \
			(res)->tv_sec += 1;                   \
			(res)->tv_nsec -= 1000000000L;        \
		}                                             \
	} while (0)
#endif /* timespecadd */

#ifndef timespecsub
/* Same as timersub() for struct timespec */
#define timespecsub(a, b, res)                                \
	do {                                                  \
		(res)->tv_sec = (a)->tv_sec - (b)->tv_sec;    \
		(res)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec; \
		if ((res)->tv_nsec < 0) {                     \
			(res)->tv_sec -= 1;                   \
			(res)->tv_nsec += 1000000000L;        \
		}                                             \
	} while (0)
#endif /* timespecsub */

#ifndef timespecclear
/* Same as timerclear() for struct timespec */
#define timespecclear(tsp)          \
	do {                        \
		(tsp)->tv_sec = 0;  \
		(tsp)->tv_nsec = 0; \
	} while (0)
#endif /* timespecclear */

#ifndef timespecisset
/* Same as timerisset() for struct timespec */
#define timespecisset(tsp) ((tsp)->tv_sec || (tsp)->tv_nsec)
#endif /* timespecisset */

#ifndef timespeccmp
/* Same as timercmp() for struct timespec */
#define timespeccmp(a, b, cmp)                    \
	((a)->tv_sec == (b)->tv_sec) ?            \
		((a)->tv_nsec cmp (b)->tv_nsec) : \
		((a)->tv_sec cmp (b)->tv_sec)
#endif /* timespecclear */

#endif /* BAREMETAL_CRT_SYS_TIME_H_ */
